/*###################################################################################
#
#   Embperl - Copyright (c) 1997-1998 Gerald Richter / ECOS
#
#   You may distribute under the terms of either the GNU General Public
#   License or the Artistic License, as specified in the Perl README file.
#   For use with Apache httpd and mod_perl, see also Apache copyright.
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


static char sLogFilename [512] = "" ;

#ifndef PerlIO

/* define same helper macros to let it run with plain perl 5.003 */
/* where no PerlIO is present */

#define PerlIO_stdinF stdin
#define PerlIO_stdoutF stdout
#define PerlIO_stderrF stderr
#define PerlIO_close fclose
#define PerlIO_open fopen
#define PerlIO_flush fflush
#define PerlIO_vprintf vfprintf
#define PerlIO_fileno fileno
#define PerlIO_tell ftell

#define PerlIO_read(f,buf,cnt) fread(buf,1,cnt,f)
#define PerlIO_write(f,buf,cnt) fwrite(buf,1,cnt,f)

#define PerlIO_putc(f,c) fputc(c,f)

#else

#define PerlIO_stdinF PerlIO_stdin ()
#define PerlIO_stdoutF PerlIO_stdout ()
#define PerlIO_stderrF PerlIO_stderr ()


#endif



#ifdef APACHE
#define DefaultLog "/tmp/embperl.log"


static request_rec * pAllocReq = NULL ;
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


static char * pMemBuf ;         /* temporary output */
static char * pMemBufPtr ;      /* temporary output */
static size_t nMemBufSize ;     /* size of pMemBuf */
static size_t nMemBufSizeFree ; /* remaining space in pMemBuf */


/*
*  Makers for rollback output
*/


int     nMarker ;


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

void oRollbackOutput (struct tBuf *   pBuf) 

    {
    EPENTRY1N (oRollback, nMarker) ;

    if (pBuf == NULL)
        {
        if (pLastFreeBuf)
            pLastFreeBuf -> pNext = pFirstBuf ;
        else 
            pFreeBuf = pFirstBuf ;
        
        pLastFreeBuf = pLastBuf ;
        
        pFirstBuf   = NULL ;
        nMarker     = 0 ;
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

/* -------------------------------------------------------------------------------------
*
*  rollback output transaction  and errors(throw away all the output since corresponding
*  begin)
*
-------------------------------------------------------------------------------------- */

void oRollback (struct tBuf *   pBuf) 

    {
    oRollbackOutput (pBuf) ;
    
    RollbackError () ;
    }

/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* commit output transaction (all the output since corresponding begin is vaild)*/
/*                                                                              */
/* ---------------------------------------------------------------------------- */

void oCommitToMem (struct tBuf *   pBuf,
                   char *          pOut) 

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
        
        if (pOut)
            {
            while (pBuf)
                {
                memmove (pOut, pBuf + 1, pBuf -> nSize) ;
                pOut += pBuf -> nSize ;
                pBuf = pBuf -> pNext ;
                }
            *pOut = '\0' ;                
            }
        else
            {
            while (pBuf)
                {
                owrite (pBuf + 1, 1, pBuf -> nSize) ;
                pBuf = pBuf -> pNext ;
                }
            }
        }

    CommitError () ;
    }

/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* commit output transaction (all the output since corresponding begin is vaild)*/
/*                                                                              */
/* ---------------------------------------------------------------------------- */

void oCommit (struct tBuf *   pBuf) 

    {
    EPENTRY1N (oCommit, nMarker) ;

    oCommitToMem (pBuf, NULL) ;
    }

/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* write to a buffer                                                            */
/*                                                                              */
/* we will alloc a new buffer for every write                                   */
/* this is fast with apache palloc or for malloc if no free is call in between  */
/*                                                                              */
/* ---------------------------------------------------------------------------- */


static int bufwrite (/*in*/ const void * ptr, size_t size) 


    {
    struct tBuf * pBuf ;

    EPENTRY1N (bufwrite, nMarker) ;

    pBuf = (struct tBuf *)_malloc (size + sizeof (struct tBuf)) ;

    if (pBuf == NULL)
        return 0 ;

    memcpy (pBuf + 1,  ptr, size) ;
    pBuf -> pNext   = NULL ;
    pBuf -> nSize   = size ;
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


/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* free buffers                                                                 */
/*                                                                              */
/* free all buffers                                                             */
/* note: this is not nessecary for apache palloc, because all buffers are freed */
/*       at the end of the request                                              */
/*                                                                              */
/* ---------------------------------------------------------------------------- */


static void buffree ()

    {
    struct tBuf * pNext = NULL ;
    struct tBuf * pBuf ;

#ifdef APACHE
    if ((bDebug & dbgMem) == 0 && pAllocReq != NULL)
        {
        pFirstBuf    = NULL ;
        pLastBuf     = NULL ;
        pFreeBuf     = NULL ;
        pLastFreeBuf = NULL ;
        return ; /* no need for apache to free memory */
        }
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

/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* get the length outputed to buffers so far                                    */
/*                                                                              */
/* ---------------------------------------------------------------------------- */

int GetContentLength ()
    {
    if (pLastBuf)
        return pLastBuf -> nCount ;
    else
        return 0 ;
    
    }

/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* set the name of the input file and open it                                   */
/*                                                                              */
/* ---------------------------------------------------------------------------- */


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
        strncpy (errdat2, Strerror(errno), sizeof (errdat2) - 1) ; 
        return rcFileOpenErr ;
        }

    return ok ;
    }


/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* close input file                                                             */
/*                                                                              */
/* ---------------------------------------------------------------------------- */


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



/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* read block of data from input (web client)                                   */
/*                                                                              */
/* ---------------------------------------------------------------------------- */


int iread (/*in*/ void * ptr, size_t size) 

    {
    if (size == 0)
        return 0 ;

#if defined (APACHE)
    if (pReq)
        {
        setup_client_block(pReq, REQUEST_CHUNKED_ERROR); 
        if(should_client_block(pReq))
            {
            int c ;
            int n = 0 ;
            while (1)
                {
                c = get_client_block(pReq, ptr, size); 
                if (c < 0 || c == 0)
                    return n ;
                n    	     += c ;
                (char *)ptr  += c ;
                size         -= c ;
                }
            }
        else
            return 0 ;
        } 
#endif

    return PerlIO_read (ifd, ptr, size) ;
    }


/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* read line of data from input (web client)                                    */
/*                                                                              */
/* ---------------------------------------------------------------------------- */


char * igets    (/*in*/ char * s,   int    size) 

    {
#if defined (APACHE)
    if (pReq)
        return NULL ;
#endif

#ifdef PerlIO
        {
        /*
        FILE * f = PerlIO_exportFILE (ifd, 0) ;
        char * p = fgets (s, size, f) ;
        PerlIO_releaseFILE (ifd, f) ;
        return p ;
        */
        return NULL ;
        }
#else
    return fgets (s, size, ifd) ;
#endif
    }

/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* read HTML File into pBuf                                                     */
/*                                                                              */
/* ---------------------------------------------------------------------------- */

int ReadHTML (/*in*/    char *    sInputfile,
              /*in*/    size_t *  nFileSize,
              /*out*/   SV   * *  ppBuf)

    {              
    SV   * pBufSV ;
    char * pData ;
#ifdef PerlIO
    PerlIO * ifd ;
#else
    FILE *   ifd ;
#endif
    
    if (bDebug)
        lprintf ("[%d]Reading %s as input using %s ...\n", nPid, sInputfile, FILEIOTYPE) ;

#ifdef WIN32
    if ((ifd = PerlIO_open (sInputfile, "rb")) == NULL)
#else
    if ((ifd = PerlIO_open (sInputfile, "r")) == NULL)
#endif        
        {
        strncpy (errdat1, sInputfile, sizeof (errdat1) - 1) ;
        strncpy (errdat2, Strerror(errno), sizeof (errdat2) - 1) ; 
        return rcFileOpenErr ;
        }

    pBufSV = sv_2mortal (newSV(*nFileSize + 1)) ;
    pData = SvPVX(pBufSV) ;

    *nFileSize = PerlIO_read (ifd, pData, *nFileSize) ;

    PerlIO_close (ifd) ;
    
    pData [*nFileSize] = '\0' ;
    SvCUR_set (pBufSV, *nFileSize) ;
    SvTEMP_off (pBufSV) ;
    SvPOK_on   (pBufSV) ;

    *ppBuf = pBufSV ;

    return ok ;
    }


/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* set the name of the output file and open it                                  */
/*                                                                              */
/* ---------------------------------------------------------------------------- */



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

#ifdef WIN32
    if ((ofd = PerlIO_open (sFilename, "wb")) == NULL)
#else
    if ((ofd = PerlIO_open (sFilename, "w")) == NULL)
#endif        
        {
        strncpy (errdat1, sFilename, sizeof (errdat1) - 1) ;
        strncpy (errdat2, Strerror(errno), sizeof (errdat2) - 1) ; 
        return rcFileOpenErr ;
        }

    return ok ;
    }


/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* close the output file                                                        */
/*                                                                              */
/* ---------------------------------------------------------------------------- */


int CloseOutput ()

    {
    
    /* make sure all buffers are freed */

    /* buffree () ; */

#if defined (APACHE)
    if (pReq)
        return ok ;
#endif

    if (ofd && ofd != PerlIO_stdoutF)
        PerlIO_close (ofd) ;

    ofd = NULL ;

    return ok ;
    }


/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* set output to memory buffer                                                  */
/*                                                                              */
/* ---------------------------------------------------------------------------- */



void OutputToMemBuf (/*in*/ char *  pBuf,
                     /*in*/ size_t  nBufSize)

    {
    pMemBuf         = pBuf ;
    pMemBufPtr      = pBuf ;
    nMemBufSize     = nBufSize ;
    nMemBufSizeFree = nBufSize ;
    }


/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* set output to standard                                                       */
/*                                                                              */
/* ---------------------------------------------------------------------------- */


char * OutputToStd    ()

    {
    char * p = pMemBuf ;
    pMemBuf         = NULL ;
    nMemBufSize     = 0 ;
    nMemBufSizeFree = 0 ;
    return p ;
    }



/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* puts to output (web client)                                                  */
/*                                                                              */
/* ---------------------------------------------------------------------------- */

int oputs (/*in*/ const char *  str) 

    {
    return owrite (str, 1, strlen (str)) ;
    }


/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* write block of data to output (web client)                                   */
/*                                                                              */
/* ---------------------------------------------------------------------------- */

int owrite (/*in*/ const void * ptr, size_t size, size_t nmemb) 

    {
    int n = size * nmemb ;

    if (n == 0)
        return 0 ;

    if (pMemBuf)
        {
        char * p ;
        int s = nMemBufSize ;
        if (n >= nMemBufSizeFree)
            {
            if (s < n)
                s = n + nMemBufSize ;
            
            nMemBufSize      += s ;
            nMemBufSizeFree  += s ;
            /*lprintf ("[%d]MEM:  Realloc pMemBuf, nMemSize = %d\n", nPid, nMemBufSize) ; */
            p = _realloc (pMemBuf, nMemBufSize) ;
            if (p == NULL)
                {
                nMemBufSize      -= s ;
                nMemBufSizeFree  -= s ;
                return 0 ;
                }
            pMemBufPtr = p + (pMemBufPtr - pMemBuf) ;
            pMemBuf = p ;
            }
                
        memcpy (pMemBufPtr, ptr, n) ;
        pMemBufPtr += n ;
        *pMemBufPtr = '\0' ;
        nMemBufSizeFree -= n ;
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



/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* write one char to output (web client)                                        */
/*                                                                              */
/* ---------------------------------------------------------------------------- */


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



/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* set the name of the log file and open it                                     */
/*                                                                              */
/* nMode = 0 open later, save filename only										*/
/* nMode = 1 open now															*/
/* nMode = 2 open with saved filename											*/
/*                                                                              */
/* ---------------------------------------------------------------------------- */


int OpenLog (/*in*/ const char *  sFilename,
             /*in*/ int           nMode)

    {
    if (sFilename == NULL)
        sFilename = "" ;

    if (lfd && (nMode == 2 || strcmp (sLogFilename, sFilename) == 0))
        return ok ; /*already open */

    if (lfd && lfd != PerlIO_stdoutF)
        PerlIO_close (lfd) ;  /* close old logfile */
   
    lfd = NULL ;

    if (nMode != 2)
        {
        strncpy (sLogFilename, sFilename, sizeof (sLogFilename) - 1) ;
        sLogFilename[sizeof (sLogFilename) - 1] = '\0' ;
        }

    if (*sLogFilename == '\0')
        {
        sLogFilename[0] = '\0' ;
        lfd = PerlIO_stdoutF ;
        return ok ;
        }

    if (nMode == 0)
        return ok ;
    
    if ((lfd = PerlIO_open (sLogFilename, "a")) == NULL)
        {
        strncpy (errdat1, sLogFilename, sizeof (errdat1) - 1) ;
        strncpy (errdat2, Strerror(errno), sizeof (errdat2) - 1) ; 
        return rcLogFileOpenErr ;
        }

    return ok ;
    }

/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* return the handle of the log file                                            */
/*                                                                              */
/* ---------------------------------------------------------------------------- */


int GetLogHandle ()

    {
    if (lfd)
	return PerlIO_fileno (lfd) ;

    return 0 ;
    }


/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* return the current posittion of the log file                                 */
/*                                                                              */
/* ---------------------------------------------------------------------------- */


long GetLogFilePos ()

    {
    if (lfd)
	return PerlIO_tell (lfd) ;

    return 0 ;
    }


/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* close the log file                                                           */
/*                                                                              */
/* ---------------------------------------------------------------------------- */


int CloseLog ()

    {
    if (lfd && lfd != PerlIO_stdoutF)
        PerlIO_close (lfd) ;

    lfd = NULL ;

    return ok ;
    }



/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* flush the log file                                                           */
/*                                                                              */
/* ---------------------------------------------------------------------------- */


int FlushLog ()

    {
    if (lfd != NULL)
        PerlIO_flush (lfd) ;

    return ok ;
    }



/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* printf to log file                                                           */
/*                                                                              */
/* ---------------------------------------------------------------------------- */

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


/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* write block of data to log file                                              */
/*                                                                              */
/* ---------------------------------------------------------------------------- */


int lwrite (/*in*/ const void * ptr, size_t size, size_t nmemb) 

    {
    int n ;

    if (lfd == NULL)
        return 0 ;
    
    n = PerlIO_write (lfd, (void *)ptr, size * nmemb) ;

    if (bDebug & dbgFlushLog)
        PerlIO_flush (lfd) ;

    return n ;
    }




/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* Memory Allocation                                                            */
/*                                                                              */
/* ---------------------------------------------------------------------------- */


void _free (void * p)

    {
    size_t size ;
    size_t * ps ;

#ifdef APACHE
    if (pAllocReq && !(bDebug & dbgMem))
        return ;
#endif


    /* we do it a bit complicted so it compiles also on aix */
    ps = (size_t *)p ;
    ps-- ;
    size = *ps ;
    p = ps ;
    if (bDebug & dbgMem)
        {
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
    size_t * ps ;

#ifdef APACHE
    pAllocReq = pReq ;

    if (pReq)
        {
        p = palloc (pReq -> pool, size + sizeof (size)) ;
        }
    else
#endif
        
        p = malloc (size + sizeof (size)) ;

    /* we do it a bit complicted so it compiles also on aix */
    ps = (size_t *)p ;
    *ps = size ;
    p = ps + 1 ;

    if (bDebug & dbgMem)
        {
        nAllocSize += size ;
        lprintf ("[%d]MEM:  Alloc %d Bytes at %08x   Allocated so far %d Bytes\n" ,nPid, size, p, nAllocSize) ;
        }

    return p ;
    }

void * _realloc (void * ptr, size_t  size)

    {
    void * p ;
    size_t * ps ;
    size_t sizeold ;
    
#ifdef APACHE
    if (pReq)
        {
        p = palloc (pReq -> pool, size + sizeof (size)) ;
        if (p == NULL)
            return NULL ;
        
        /* we do it a bit complicted so it compiles also on aix */
        ps = (size_t *)p ;
        *ps = size ;
        p = ps + 1;
        
        ps = (size_t *)ptr ;
        ps-- ;
        sizeold = *ps ;

        memcpy (p, ptr, sizeold) ; 
        }
    else
#endif
        {        
        ps = (size_t *)ptr ;
        ps-- ;
        p = realloc (ps, size + sizeof (size)) ;
        if (p == NULL)
            return NULL ;

        /* we do it a bit complicted so it compiles also on aix */
        ps = (size_t *)p ;
        *ps = size ;
        p = ps + 1;
        }

    if (bDebug & dbgMem)
        {
        nAllocSize += size ;
        lprintf ("[%d]MEM:  ReAlloc %d Bytes at %08x   Allocated so far %d Bytes\n" ,nPid, size, p, nAllocSize) ;
        }

    return p ;
    }


char *  _memstrcat (const char *s, ...) 

    {
    va_list ap ;
    char *  p ;
    char *  str ;
    char *  sp ;
    int     l ;
    int     sum ;

    EPENTRY(_memstrcat) ;

    va_start(ap, s) ;

    p = (char *)s ;
    sum = 0 ;
    while (p)
        {
        sum += va_arg (ap, int) ;
        p = va_arg (ap, char *) ;
        }
    sum++ ;

    va_end (ap) ;

    sp = str = _malloc (sum+1) ;

    va_start(ap, s) ;

    p = (char *)s ;
    while (p)
        {
        l = va_arg (ap, int) ;
        memcpy (str, p, l) ;
        str += l ;
        p = va_arg (ap, char *) ;
        }
    *str = '\0' ;

    va_end (ap) ;


    return sp ;
    }

