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


/* this was private in http_protocol.c */


#define SET_BYTES_SENT(r) do {if (r->sent_bodyct) bgetopt (r->connection->client, BO_BYTECT, &r->bytes_sent); } while (0)


//
// Files
//


FILE *  ifd = NULL ;  // input file
FILE *  ofd = NULL ;  // output file
FILE *  lfd = NULL ;  // log file



#ifdef APACHE
#define DefaultLog "/tmp/embperl.log"
#endif


//
// Datastructure for buffering output
//



struct tBuf *    pFirstBuf = NULL ;  // First buffer
struct tBuf *    pLastBuf  = NULL ;  // Last written buffer


//
// Makers for rollback output
//


int     nMarker ;


////////////////////////////////////////////////////////////////////////////////////
//
// begin output transaction
//

struct tBuf *   oBegin ()

    {
    nMarker++ ;
    return pLastBuf ;
    }

////////////////////////////////////////////////////////////////////////////////////
//
// rollback output transaction (throw away all the output since corresponding begin)
//

void oRollback (struct tBuf *   pBuf) 

    {
    if (pBuf == NULL)
        {
        pFirstBuf = NULL ;
        nMarker = 0 ;
        }
    else
        if (pLastBuf == pBuf)
            nMarker-- ;
        else
            nMarker = pBuf -> pNext -> nMarker - 1 ;
        
    pLastBuf = pBuf ;
    pBuf -> pNext = NULL ;
    }

////////////////////////////////////////////////////////////////////////////////////
//
// commit output transaction (all the output since corresponding begin is vaild)
//

void oCommit (struct tBuf *   pBuf) 

    {
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


////////////////////////////////////////////////////////////////////////////////////
//
// write to a buffer
//
// we will alloc a new buffer for every write
// this is fast with apache palloc or for malloc if no free is call in between
//


int bufwrite (/*in*/ const void * ptr, size_t size) 


    {
    struct tBuf * pBuf = (struct tBuf *)_malloc (size + sizeof (struct tBuf)) ;


    if (pBuf == NULL)
        return 0 ;

    memcpy (pBuf + 1,  ptr, size) ;
    pBuf -> pNext = NULL ;
    pBuf -> nSize = size ;
    pBuf -> nMarker = nMarker ;

    if (pLastBuf)
        pLastBuf -> pNext = pBuf ;
    if (pFirstBuf == NULL)
        pFirstBuf = pBuf ;
    pLastBuf = pBuf ;


    return size ;
    }




////////////////////////////////////////////////////////////////////////////////////
//
// set the name of the input file and open it
//


int OpenInput (/*in*/ const char *  sFilename)

    {
#ifdef APACHE
    if (pReq)
        return rcInputNotSupported ;
#endif
    
    if (ifd)
        fclose (ifd) ;

    ifd = NULL ;

    if (sFilename == NULL || *sFilename == '\0')
        {
        ifd = stdin ;
        return ok ;
        }

    if ((ifd = fopen (sFilename, "r")) == NULL)
        {
        strncpy (errdat1, sFilename, sizeof (sFilename)) ;
        return rcFileOpenErr ;
        }

    return ok ;
    }


////////////////////////////////////////////////////////////////////////////////////
//
// close the input file 
//


int CloseInput ()

    {
#ifdef APACHE
    if (pReq)
        return rcInputNotSupported ;
#endif

    if (ifd)
        fclose (ifd) ;

    ifd = NULL ;

    return ok ;
    }



////////////////////////////////////////////////////////////////////////////////////
//
// read block of data from input (web client)
//


int iread (/*in*/ void * ptr, size_t size, size_t nmemb) 

    {
#ifdef APACHE
    if (pReq)
        return rcInputNotSupported ;
#endif

    return fread (ptr, size, nmemb, ifd) ;
    }


////////////////////////////////////////////////////////////////////////////////////
//
// read line of data from input (web client)
//


char * igets    (/*in*/ char * s,   int    size) 

    {
#ifdef APACHE
    if (pReq)
        return NULL ;
#endif

    return fgets (s, size, ifd) ;
    }


////////////////////////////////////////////////////////////////////////////////////
//
// set the name of the output file and open it
//


int OpenOutput (/*in*/ const char *  sFilename)

    {
    pFirstBuf = NULL ; 
    pLastBuf  = NULL ; 
    nMarker   = 0 ;


#ifdef APACHE
    if (pReq)
        return ok ;
#endif

    
    if (ofd)
        fclose (ofd) ;

    ofd = NULL ;

    if (sFilename == NULL || *sFilename == '\0')
        {
        ofd = stdout ;
        return ok ;
        }

    if ((ofd = fopen (sFilename, "w")) == NULL)
        {
        strncpy (errdat1, sFilename, sizeof (sFilename)) ;
        return rcFileOpenErr ;
        }

    return ok ;
    }


////////////////////////////////////////////////////////////////////////////////////
//
// close the output file 
//


int CloseOutput ()

    {
#ifdef APACHE
    if (pReq)
        return ok ;
#endif

    if (ofd)
        fclose (ofd) ;

    ofd = NULL ;

    return ok ;
    }


////////////////////////////////////////////////////////////////////////////////////
//
// puts to output (web client)
//



int oputs (/*in*/ const char *  str) 

    {
    return owrite (str, 1, strlen (str)) ;
    }


////////////////////////////////////////////////////////////////////////////////////
//
// printf to output (web client)
//



int oprintf (/*in*/ const char *  sFormat,
             /*in*/ ...) 

    {
    va_list  ap ;
    int      n ;
    char     buf [1024] ;


    va_start (ap, sFormat) ;
    
    n = vsnprintf (buf, sizeof (buf) - 1, sFormat, ap) ;

    return owrite (buf, 1, n) ;
    }

/*
#ifdef APACHE
    if (pReq)
        {
        if(pReq->connection->aborted) return EOF;
        n=vbprintf(pReq->connection->client,sFormat,ap);
        SET_BYTES_SENT(pReq);
        if (bDebug & dbgFlushOutput)
            rflush (pReq) ;
        }
    else
#endif
        {
        n = vfprintf (ofd, sFormat, ap) ;
        if (bDebug & dbgFlushOutput)
            fflush (ofd) ;
        }


    va_end (ap) ;

    return n ;
    }
*/

////////////////////////////////////////////////////////////////////////////////////
//
// write block of data to output (web client)
//


int owrite (/*in*/ const void * ptr, size_t size, size_t nmemb) 

    {
    int n = size * nmemb ;

    if (nMarker)
        return bufwrite (ptr, n) / size ;

#ifdef APACHE
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
        n = fwrite ((void *)ptr, size, nmemb, ofd) ;

        if (bDebug & dbgFlushOutput)
            fflush (ofd) ;
        }

    return n ;
    }



////////////////////////////////////////////////////////////////////////////////////
//
// write one char to output (web client)
//


void oputc (/*in*/ char c)

    {
    if (nMarker)
        {
        bufwrite (&c, 1) ;
        return ;
        }

#ifdef APACHE
    if (pReq)
        {
        rputc (c, pReq) ;
        if (bDebug & dbgFlushOutput)
            rflush (pReq) ;
        }
#endif
    fputc (c, ofd) ;

    if (bDebug & dbgFlushOutput)
        fflush (ofd) ;
    }



////////////////////////////////////////////////////////////////////////////////////
//
// set the name of the log file and open it
//


int OpenLog (/*in*/ const char *  sFilename)

    {
    if (lfd)
        fclose (lfd) ;

    lfd = NULL ;

    if (sFilename == NULL || *sFilename == '\0')
        {
            {
            lfd = stdout ;
            return ok ;
            }
        }

    if ((lfd = fopen (sFilename, "a")) == NULL)
        {
        strncpy (errdat1, sFilename, sizeof (sFilename)) ;
        return rcFileOpenErr ;
        }

    return ok ;
    }


////////////////////////////////////////////////////////////////////////////////////
//
// close the log file 
//


int CloseLog ()

    {
    if (lfd)
        fclose (lfd) ;

    lfd = NULL ;

    return ok ;
    }



////////////////////////////////////////////////////////////////////////////////////
//
// flush the log file 
//


int FlushLog ()

    {
    fflush (lfd) ;

    return ok ;
    }



////////////////////////////////////////////////////////////////////////////////////
//
// printf to log file
//

int lprintf (/*in*/ const char *  sFormat,
             /*in*/ ...) 

    {
    va_list  ap ;
    int      n ;

    va_start (ap, sFormat) ;
    
        {
        n = vfprintf (lfd, sFormat, ap) ;
        if (bDebug & dbgFlushLog)
            fflush (lfd) ;
        }


    va_end (ap) ;

    return n ;
    }


////////////////////////////////////////////////////////////////////////////////////
//
// write block of data to log file
//


int lwrite (/*in*/ const void * ptr, size_t size, size_t nmemb) 

    {
    int n ;

    n = fwrite ((void *)ptr, size, nmemb, lfd) ;

    if (bDebug & dbgFlushLog)
        fflush (lfd) ;

    return n ;
    }




////////////////////////////////////////////////////////////////////////////////////
//
// Memory Allocation
//


void _free (void * p)

    {
    if (bDebug & dbgMem)
        lprintf ("MEM:  Free at %08x\n" ,p) ;

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
        p = palloc (pReq -> pool, size) ;
    else
#endif
        
        p = malloc (size) ;

    if (bDebug & dbgMem)
        lprintf ("MEM:  Alloc %d Bytes at %08x\n" ,size, p) ;

    return p ;
    }


