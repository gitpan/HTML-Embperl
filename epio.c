/*###################################################################################
#
#   Embperl - Copyright (c) 1997 Gerald Richter / ECOS
#
#   You may distribute under the terms of either the GNU General Public
#   License or the Artistic License, as specified in the Perl README file.
#   For use with Apache httpd and mod_perl, see also Apache copyright.
#
#   THIS IS BETA SOFTWARE!
#
#   THIS PACKAGE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR
#   IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
#   WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
#
###################################################################################*/


#include "ep.h"
#include "epmacro.h"



/*
*   Files
*/

#ifdef PerlIO

#define FILEIOTYPE "PerlIO"

static PerlIO *  ifd = NULL ;  /* input file */
static PerlIO *  ofd = NULL ;  /* output file */
static PerlIO *  lfd = NULL ;  /* log file */
#else

#define FILEIOTYPE "StdIO"

static FILE *  ifd = NULL ;  /* input file */
static FILE *  ofd = NULL ;  /* output file */
static FILE *  lfd = NULL ;  /* log file */
#endif


#ifndef PerlIO

/* define same helper macros to let it run with plain perl 5.003 */
/* where no PerlIO is present */

/*
#undef PerlIO_close 
#undef PerlIO_open 
#undef PerlIO_flush 
#undef PerlIO_vprintf
#undef PerlIO_fileno

#undef PerlIO_read
#undef PerlIO_write

#undef PerlIO_putc
*/

#define PerlIO_stdinF stdin
#define PerlIO_stdoutF stdout
#define PerlIO_stderrF stderr
#define PerlIO_close fclose
#define PerlIO_open fopen
#define PerlIO_flush fflush
#define PerlIO_vprintf vfprintf
#define PerlIO_fileno fileno

#define PerlIO_read(f,buf,cnt) fread(buf,cnt,1,f)
#define PerlIO_write(f,buf,cnt) fwrite(buf,cnt,1,f)

#define PerlIO_putc(f,c) fputc(c,f)

#else

#define PerlIO_stdinF PerlIO_stdin ()
#define PerlIO_stdoutF PerlIO_stdout ()
#define PerlIO_stderrF PerlIO_stderr ()


#endif



#ifdef APACHE
#define DefaultLog "/tmp/embperl.log"
#endif

/*
*  Alloced memory for debugging
*/

size_t nAllocSize = 0 ;


/*
*  Datastructure for buffering output
*/



static struct tBuf *    pFirstBuf = NULL ;  /* First buffer */
static struct tBuf *    pLastBuf  = NULL ;  /* Last written buffer */
static struct tBuf *    pFreeBuf  = NULL ;  /* List of unused buffers */
static struct tBuf *    pLastFreeBuf  = NULL ;  /* End of list of unused buffers */


static char * pMemBuf ;     /* temporary output */
static size_t nMemBufSize ; /* remaining space in pMemBuf */


/*
*  Makers for rollback output
*/


static int     nMarker ;


/* -------------------------------------------------------------------------------------
*
* begin output transaction
*
-------------------------------------------------------------------------------------- */

struct tBuf *   oBegin ()

    {
    EPENTRY1N (oBegin, nMarker) ;
    
    nMarker++ ;
    return pLastBuf ;
    }

/* -------------------------------------------------------------------------------------
*
*  rollback output transaction (throw away all the output since corresponding begin)
*
-------------------------------------------------------------------------------------- */

void oRollback (struct tBuf *   pBuf) 

    {
    EPENTRY1N (oRollback, nMarker) ;

    if (pBuf == NULL)
        {
        if (pLastFreeBuf)
            pLastFreeBuf -> pNext = pFirstBuf ;
        else 
            pFreeBuf = pFirstBuf ;
        
        pLastFreeBuf = pLastBuf ;
        
        pFirstBuf = NULL ;
        nMarker = 0 ;
        }
    else
	{
        if (pLastBuf == pBuf || pBuf -> pNext == NULL)
            nMarker-- ;
        else
            {
            nMarker = pBuf -> pNext -> nMarker - 1 ;
            if (pLastFreeBuf)
                pLastFreeBuf -> pNext = pBuf -> pNext ;
            else
                pFreeBuf = pBuf -> pNext ;
            pLastFreeBuf = pLastBuf ;
            }
        pBuf -> pNext = NULL ;
        }
        
    pLastBuf = pBuf ;
    }

/* ///////////////////////////////////////////////////////////////////////////////////
//
// commit output transaction (all the output since corresponding begin is vaild)
*/

void oCommit (struct tBuf *   pBuf) 

    {
    EPENTRY1N (oCommit, nMarker) ;

    
    if (pBuf == NULL)
        nMarker = 0 ;
    else
        if (pLastBuf == pBuf)
            nMarker-- ;
        else
            nMarker = pBuf -> pNext -> nMarker - 1 ;
    
    if (nMarker == 0)
        {
        if (pBuf == NULL)
            pBuf = pFirstBuf ;
        else
            pBuf = pBuf -> pNext ;
        
        while (pBuf)
            {
            owrite (pBuf + 1, 1, pBuf -> nSize) ;
            pBuf = pBuf -> pNext ;
            }
        }
    }


/* ///////////////////////////////////////////////////////////////////////////////////
//
// write to a buffer
//
// we will alloc a new buffer for every write
// this is fast with apache palloc or for malloc if no free is call in between
*/


static int bufwrite (/*in*/ const void * ptr, size_t size) 


    {
    struct tBuf * pBuf ;

    EPENTRY1N (bufwrite, nMarker) ;

    pBuf = (struct tBuf *)_malloc (size + sizeof (struct tBuf)) ;

    if (pBuf == NULL)
        return 0 ;

    memcpy (pBuf + 1,  ptr, size) ;
    pBuf -> pNext = NULL ;
    pBuf -> nSize = size ;
    pBuf -> nMarker = nMarker ;

    if (pLastBuf)
        {
        pLastBuf -> pNext = pBuf ;
        pBuf -> nCount    = pLastBuf -> nCount + size ;
        }
    else
        pBuf -> nCount    = size ;
        
    if (pFirstBuf == NULL)
        pFirstBuf = pBuf ;
    pLastBuf = pBuf ;


    return size ;
    }


/* ///////////////////////////////////////////////////////////////////////////////////
//
// free buffers
//
// free all buffers
// note: this is not nessecary for apache palloc, because all buffers are freed
//       at the end of the request
*/


static void buffree ()

    {
    struct tBuf * pNext = NULL ;
    struct tBuf * pBuf ;

#ifdef APACHE
    if ((bDebug & dbgMem) == 0 && pReq != NULL)
        return ; /* no need for apache to free memory */
#endif
        
    /* first walk thru the used buffers */

    pBuf = pFirstBuf ;
    while (pBuf)
        {
        pNext = pBuf -> pNext ;
        _free (pBuf) ;
        pBuf = pNext ;
        }

    pFirstBuf = NULL ;
    pLastBuf  = NULL ;


    /* now walk thru the unused buffers */
    
    pBuf = pFreeBuf ;
    while (pBuf)
        {
        pNext = pBuf -> pNext ;
        _free (pBuf) ;
        pBuf = pNext ;
        }

    pFreeBuf = NULL ;
    pLastFreeBuf  = NULL ;
    }

/* ///////////////////////////////////////////////////////////////////////////////////
//
// get the length outputed to buffers so far
*/

int GetContentLength ()
    {
    if (pLastBuf)
        return pLastBuf -> nCount ;
    else
        return 0 ;
    
    }

/* ///////////////////////////////////////////////////////////////////////////////////
//
// set the name of the input file and open it
*/


int OpenInput (/*in*/ const char *  sFilename)

    {
#ifdef APACHE
    if (pReq)
        return ok ;
#endif
    
    if (ifd && ifd != PerlIO_stdinF)
        PerlIO_close (ifd) ;

    ifd = NULL ;

    if (sFilename == NULL || *sFilename == '\0')
        {
        /*
        GV * io = gv_fetchpv("STDIN", TRUE, SVt_PVIO) ;
        if (io == NULL || (ifd = IoIFP(io)) == NULL)
            {
            if (bDebug)
                lprintf ("[%d]Cannot get Perl STDIN, open os stdin\n", nPid) ;
            ifd = PerlIO_stdinF ;
            }
        */
        
        ifd = PerlIO_stdinF ;

        return ok ;
        }

    if ((ifd = PerlIO_open (sFilename, "r")) == NULL)
        {
        strncpy (errdat1, sFilename, sizeof (errdat1) - 1) ;
        if (errno >= 0 && errno < sys_nerr)
            strncpy (errdat2, sys_errlist[errno], sizeof (errdat2) - 1) ; 
        return rcFileOpenErr ;
        }

    return ok ;
    }


/* ///////////////////////////////////////////////////////////////////////////////////
//
// close the input file 
*/


int CloseInput ()

    {
#ifdef APACHE
    if (pReq)
        return ok ;
#endif

    if (ifd && ifd != PerlIO_stdinF)
        PerlIO_close (ifd) ;

    ifd = NULL ;

    return ok ;
    }



/* ///////////////////////////////////////////////////////////////////////////////////
//
// read block of data from input (web client)
*/


int iread (/*in*/ void * ptr, size_t size, size_t nmemb) 

    {
    int n = size * nmemb ;

    if (n == 0)
        return 0 ;

#if defined (APACHE)
    if (pReq)
        {
        setup_client_block(pReq, REQUEST_CHUNKED_ERROR); 
        if(should_client_block(pReq))
            {
            int c = get_client_block(pReq, ptr, n); 
            return c / size ;
            }
        else
            return 0 ;
        } 
#endif

    return PerlIO_read (ifd, ptr, n) ;
    }


/* ///////////////////////////////////////////////////////////////////////////////////
//
// read line of data from input (web client)
*/


char * igets    (/*in*/ char * s,   int    size) 

    {
#if defined (APACHE)
    if (pReq)
        return NULL ;
#endif

#ifdef PerlIO
        {
        FILE * f = PerlIO_exportFILE (ifd, 0) ;
        char * p = fgets (s, size, f) ;
        PerlIO_releaseFILE (ifd, f) ;
        return p ;
        }
#else
    return fgets (s, size, ifd) ;
#endif
    }

/* ///////////////////////////////////////////////////////////////////////////////////
//
// read HTML File into PBuf
*/

int ReadHTML (/*in*/    char *  sInputfile,
              /*in*/    size_t  nFileSize,
              /*out*/   char * * ppBuf)

    {              
    char * pBuf ;
#ifdef PerlIO
    PerlIO * ifd ;
#else
    FILE *   ifd ;
#endif
    
    if (bDebug)
        lprintf ("[%d]Reading %s as input using %s ...\n", nPid, sInputfile, FILEIOTYPE) ;

    if ((ifd = PerlIO_open (sInputfile, "r")) == NULL)
        {
        strncpy (errdat1, sInputfile, sizeof (errdat1) - 1) ;
        if (errno >= 0 && errno < sys_nerr)
            strncpy (errdat2, sys_errlist[errno], sizeof (errdat2) - 1) ; 
        return rcFileOpenErr ;
        }

    if ((pBuf = _malloc (nFileSize + 1)) == NULL)
        {
        return rcOutOfMemory ;
        }

    PerlIO_read (ifd, pBuf, nFileSize) ;

    PerlIO_close (ifd) ;
    
    pBuf [nFileSize] = '\0' ;

    *ppBuf = pBuf ;

    return ok ;
    }


/* ///////////////////////////////////////////////////////////////////////////////////
//
// set the name of the output file and open it
*/


int OpenOutput (/*in*/ const char *  sFilename)

    {
    pFirstBuf = NULL ; 
    pLastBuf  = NULL ; 
    nMarker   = 0 ;
    pMemBuf   = NULL ;
    nMemBufSize = 0 ;


    /* make sure all old buffers are freed */

    buffree () ;


#if defined (APACHE)
    if (pReq)
        {
        if (bDebug)
            lprintf ("[%d]Using APACHE for output...\n", nPid) ;
        return ok ;
        }
#endif

    
    if (ofd && ofd != PerlIO_stdoutF)
        PerlIO_close (ofd) ;

    ofd = NULL ;

    if (sFilename == NULL || *sFilename == '\0')
        {
        /*
        GV * io = gv_fetchpv("STDOUT", TRUE, SVt_PVIO) ;
        if (io == NULL || (ofd = IoOFP(io)) == NULL)
            {
            if (bDebug)
                lprintf ("[%d]Cannot get Perl STDOUT, open os stdout\n", nPid) ;
            ofd = PerlIO_stdoutF ;
            }
        */

        ofd = PerlIO_stdoutF ;
        
        if (bDebug)
            {
#ifdef APACHE
             if (pReq)
                lprintf ("[%d]Open STDOUT to Apache for output...\n", nPid) ;
             else
#endif
             lprintf ("[%d]Open STDOUT for output...\n", nPid) ;
            }
        return ok ;
        }

    if (bDebug)
        lprintf ("[%d]Open %s for output...\n", nPid, sFilename) ;

    if ((ofd = PerlIO_open (sFilename, "w")) == NULL)
        {
        strncpy (errdat1, sFilename, sizeof (errdat1) - 1) ;
        if (errno >= 0 && errno < sys_nerr)
            strncpy (errdat2, sys_errlist[errno], sizeof (errdat2) - 1) ; 
        return rcFileOpenErr ;
        }

    return ok ;
    }


/* ///////////////////////////////////////////////////////////////////////////////////
//
// close the output file 
*/


int CloseOutput ()

    {
    
    /* make sure all buffers are freed */

    buffree () ;

#if defined (APACHE)
    if (pReq)
        return ok ;
#endif

    if (ofd && ofd != PerlIO_stdoutF)
        PerlIO_close (ofd) ;

    ofd = NULL ;

    return ok ;
    }


/* ///////////////////////////////////////////////////////////////////////////////////
//
// set output to memory buffer
*/


void OutputToMemBuf (/*in*/ char *  pBuf,
                     /*in*/ size_t  nBufSize)

    {
    pMemBuf     = pBuf ;
    nMemBufSize = nBufSize ;
    }


/* ///////////////////////////////////////////////////////////////////////////////////
//
// set output to stdandard
*/


void OutputToStd    ()

    {
    pMemBuf     = NULL ;
    nMemBufSize = 0 ;
    }



/* ///////////////////////////////////////////////////////////////////////////////////
//
// puts to output (web client)
*/



int oputs (/*in*/ const char *  str) 

    {
    return owrite (str, 1, strlen (str)) ;
    }


/* ///////////////////////////////////////////////////////////////////////////////////
//
// write block of data to output (web client)
*/


int owrite (/*in*/ const void * ptr, size_t size, size_t nmemb) 

    {
    int n = size * nmemb ;

    if (n == 0)
        return 0 ;

    if (pMemBuf)
        {
        if (n >= nMemBufSize)
            n = nMemBufSize - 1 ;
        memcpy (pMemBuf, ptr, n) ;
        pMemBuf += n ;
        *pMemBuf = '\0' ;
        nMemBufSize -= n ;
        return n / size ;
        }

    
    if (nMarker)
        return bufwrite (ptr, n) / size ;

#if defined (APACHE)
    if (pReq)
        {
        if (n > 0)
            {
            n = rwrite (ptr, n, pReq) ;
            if (bDebug & dbgFlushOutput)
                rflush (pReq) ;
            return n / size ;
            }
        else
            return 0 ;
        }
#endif
    if (n > 0)
        {
        n = PerlIO_write (ofd, (void *)ptr, size * nmemb) ;

        if (bDebug & dbgFlushOutput)
            PerlIO_flush (ofd) ;
        }

    return n / size ;
    }



/* ///////////////////////////////////////////////////////////////////////////////////
//
// write one char to output (web client)
*/


void oputc (/*in*/ char c)

    {
    if (nMarker || pMemBuf)
        {
        owrite (&c, 1, 1) ;
        return ;
        }

#if defined (APACHE)
    if (pReq)
        {
        rputc (c, pReq) ;
        if (bDebug & dbgFlushOutput)
            rflush (pReq) ;
        return ;
        }
#endif
    PerlIO_putc (ofd, c) ;

    if (bDebug & dbgFlushOutput)
        PerlIO_flush (ofd) ;
    }



/* ///////////////////////////////////////////////////////////////////////////////////
//
// set the name of the log file and open it
*/


int OpenLog (/*in*/ const char *  sFilename)

    {
    if (lfd && lfd != PerlIO_stdoutF)
        PerlIO_close (lfd) ;

    lfd = NULL ;

    if (sFilename == NULL || *sFilename == '\0')
        {
            {
            lfd = PerlIO_stdoutF ;
            return ok ;
            }
        }

    if ((lfd = PerlIO_open (sFilename, "a")) == NULL)
        {
        strncpy (errdat1, sFilename, sizeof (errdat1) - 1) ;
        if (errno >= 0 && errno < sys_nerr)
            strncpy (errdat2, sys_errlist[errno], sizeof (errdat2) - 1) ; 
        return rcLogFileOpenErr ;
        }

    return ok ;
    }

/* ///////////////////////////////////////////////////////////////////////////////////
//
// return the handle of the log file 
*/


int GetLogHandle ()

    {
    return PerlIO_fileno (lfd) ;
    }


/* ///////////////////////////////////////////////////////////////////////////////////
//
// close the log file 
*/


int CloseLog ()

    {
    if (lfd && lfd != PerlIO_stdoutF)
        PerlIO_close (lfd) ;

    lfd = NULL ;

    return ok ;
    }



/* ///////////////////////////////////////////////////////////////////////////////////
//
// flush the log file 
*/


int FlushLog ()

    {
    PerlIO_flush (lfd) ;

    return ok ;
    }



/* ///////////////////////////////////////////////////////////////////////////////////
//
// printf to log file
*/

int lprintf (/*in*/ const char *  sFormat,
             /*in*/ ...) 

    {
    va_list  ap ;
    int      n ;

    if (lfd == NULL)
        return 0 ;
    
    va_start (ap, sFormat) ;
    
        {
        n = PerlIO_vprintf (lfd, sFormat, ap) ;
        if (bDebug & dbgFlushLog)
            PerlIO_flush (lfd) ;
        }


    va_end (ap) ;

    return n ;
    }


/* ///////////////////////////////////////////////////////////////////////////////////
//
// write block of data to log file
*/


int lwrite (/*in*/ const void * ptr, size_t size, size_t nmemb) 

    {
    int n ;

    n = PerlIO_write (lfd, (void *)ptr, size * nmemb) ;

    if (bDebug & dbgFlushLog)
        PerlIO_flush (lfd) ;

    return n ;
    }




/* ///////////////////////////////////////////////////////////////////////////////////
//
// Memory Allocation
*/


void _free (void * p)

    {
    if (bDebug & dbgMem)
        {
        size_t size ;
        ((size_t *)p)-- ;
        size = *(size_t *)p ;
        nAllocSize -= size ;
        lprintf ("[%d]MEM:  Free %d Bytes at %08x  Allocated so far %d Bytes\n" ,nPid, size, p, nAllocSize) ;
        }

#ifdef APACHE
    if (pReq == NULL)
#endif
        free (p) ;
    }

void * _malloc (size_t  size)

    {
    void * p ;
    
#ifdef APACHE
    if (pReq)
        {
        p = palloc (pReq -> pool, size + sizeof (size)) ;
        }
    else
#endif
        
        p = malloc (size + sizeof (size)) ;

    if (bDebug & dbgMem)
        {
        nAllocSize += size ;
        *(size_t *)p = size ;
        ((size_t *)p)++ ;
        lprintf ("[%d]MEM:  Alloc %d Bytes at %08x   Allocated so far %d Bytes\n" ,nPid, size, p, nAllocSize) ;
        }

    return p ;
    }


