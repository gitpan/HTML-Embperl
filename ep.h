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


#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <memory.h>
#include <string.h>
#include <ctype.h>
#include <time.h>


#ifdef APACHE
#include <httpd.h>
#include <http_protocol.h>
#endif



#include <EXTERN.h>               /* from the Perl distribution     */
#include <perl.h>                 /* from the Perl distribution     */

#include "epnames.h"

#include "embperl.h"


/* declare error information here, since it's not on all systems in stdlib.h */

extern int errno;
extern int sys_nerr;
extern char *sys_errlist[];


/* Debug Flags */

extern int  bDebug ;

/* Apache Request Record */

#ifdef APACHE
extern request_rec * pReq ;
#endif



int iembperl_init (int  nIOType,
                   const char * sLogFile) ;
int iembperl_setreqrec  (/*in*/ SV *   pReqSV) ;
int iembperl_resetreqrec  () ;
int iembperl_term (void) ;
int iembperl_req  (/*in*/ char *  sInputfile,
                   /*in*/ char *  sOutputfile,
                   /*in*/ int     bDebugFlags,
                   /*in*/ char *  pNameSpaceName,
                   /*in*/ int     nFileSize) ;

/*
// Datastructure for buffering output
*/

struct tBuf
    {
    struct tBuf *   pNext ;     /* Next buffer  */
    int             nSize ;     /* Size in bytes */
    int             nMarker ;   /* nesting level */
    } ;



/* i/o functions */


int OpenInput   (/*in*/ const char *  sFilename) ;
int CloseInput  () ;
int iread       (/*in*/ void * ptr, size_t size, size_t nmemb) ;
char * igets    (/*in*/ char * s,   int    size) ;


int OpenOutput  (/*in*/ const char *  sFilename) ;
int CloseOutput () ;
int owrite      (/*in*/ const void * ptr, size_t size, size_t nmemb) ;
void oputc       (/*in*/ char c) ;
int oputs (/*in*/ const char *  str) ;

void OutputToMemBuf (/*in*/ char *  pBuf,
                     /*in*/ size_t  nBufSize) ;
void OutputToStd () ;


             
struct tBuf *   oBegin () ;
void oRollback (struct tBuf *   pBuf) ;
void oCommit (struct tBuf *   pBuf) ;


int OpenLog     (/*in*/ const char *  sFilename) ;
int CloseLog    () ;
int FlushLog    () ;
int lprintf     (/*in*/ const char *  sFormat,
                 /*in*/ ...) ;


/* Memory Allocation */

void _free (void * p) ;
void * _malloc (size_t  size) ;


/*
// Character Translation
*/

struct tCharTrans
    {
    char    c ;
    char *  sHtml ;
    } ;


extern struct tCharTrans Char2Html [] ;
extern struct tCharTrans Html2Char [] ;
extern int sizeHtml2Char ;


extern pid_t nPid ;
