/*###################################################################################
#
#   Embperl - Copyright (c) 1997-1999 Gerald Richter / ECOS
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


#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <memory.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#if !defined(PERLIO_IS_STDIO)
#define PERLIO_IS_STDIO
#endif

#ifdef __cplusplus
extern "C" {
#endif
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"
#ifdef __cplusplus
}
#endif


#ifdef APACHE
/* form mod_perl.h ->
 * perl hides it's symbols in libperl when these macros are 
 * expanded to Perl_foo
 * but some cause conflict when expanded in other headers files
 */
#undef S_ISREG
#undef DIR
#undef VOIDUSED
#undef pregexec
#undef pregfree
#undef pregcomp
#undef setregid
#undef setreuid
#undef sync
#undef my_memcmp
#undef RETURN
#undef die
#undef __attribute__

#if defined(EPAPACHE_SSL) || defined(APACHE_SSL) 
#undef TRUE
#undef FALSE
#endif

#include <httpd.h>
#include <http_config.h>
#include <http_protocol.h>
#include <http_log.h>
#if MODULE_MAGIC_NUMBER >= 19980713
#include "ap_compat.h"
#elif MODULE_MAGIC_NUMBER >= 19980413
#include "compat.h"
#endif
#endif


#include "epnames.h"

#include "epdat.h"
#include "embperl.h"

#ifdef WIN32
#define PATH_MAX _MAX_DIR
#endif

#ifndef PATH_MAX
#define PATH_MAX 512
#endif


/* ---- global variables ---- */

extern tReq * pCurrReq ;                  /* Set before every eval (NOT thread safe!!) */ 
extern tReq   InitialReq ;                /* Initial request - holds default values */


/* ---- from epmain.c ----- */

                     
#define nInitialScanOutputSize 2048

int Init          (int          nIOType,
                   const char * sLogFile,
                   int          nDebugDefault) ;

int ResetHandler (/*in*/ SV * pApacheReqSV) ;

int Term (void) ;

int ExecuteReq (/*i/o*/ register req * r,
                /*in*/  SV *           pReqSV) ;

tReq * SetupRequest (/*in*/ SV *    pApacheReqSV,
                     /*in*/ char *  sSourcefile,
                     /*in*/ double  mtime,
                     /*in*/ long    nFilesize,
                     /*in*/ int     nFirstLine,
                     /*in*/ char *  sOutputfile,
                     /*in*/ tConf * pConf,
                     /*in*/ int     nIOType,
                     /*in*/ SV *    pIn,
                     /*in*/ SV *    pOut,
    		     /*in*/ char *  sSubName,
		     /*in*/ char *  sImport,
		     /*in*/ int	    nSessionMgnt) ;

void FreeRequest (/*i/o*/ register req * r) ;
                   
tFile * SetupFileData   (/*i/o*/ register req * r,
                         /*in*/ char *  sSourcefile,
                         /*in*/ double  mtime,
                         /*in*/ long    nFilesize,
                         /*in*/ int     nFirstLine,
                         /*in*/ tConf * pConf) ;

tConf * SetupConfData   (/*in*/ HV *   pReqInfo,
                         /*in*/ SV *   pOpcodeMask) ;
                         

int ScanCmdEvalsInString (/*i/o*/ register req * r,
			/*in*/  char *   pIn,
                          /*out*/ char * * pOut,
                          /*in*/  size_t   nSize,
                          /*out*/ char * * pFree) ;
    
char * LogError (/*i/o*/ register req * r,
			/*in*/ int   rc) ;
void CommitError (/*i/o*/ register req * r) ;
void RollbackError (/*i/o*/ register req * r) ;

int ProcessBlock	(/*i/o*/ register req * r,
			 /*in*/  int	 nBlockStart,
			 /*in*/  int	 nBlockSize,
                         /*in*/  int     nBlockNo) ;

int ProcessSub		(/*i/o*/ register req * r,
			 /*in*/  tFile * pFile,
			 /*in*/  int	 nBlockStart,
                         /*in*/  int     nBlockNo) ;

/* ---- from epio.c ----- */


/*
    Datastructure for buffering output
*/

struct tBuf
    {
    struct tBuf *   pNext ;     /* Next buffer  */
    int             nSize ;     /* Size in bytes */
    int             nMarker ;   /* nesting level */
    int             nCount ;    /* output count including this buffer */
    } ;



/* i/o functions */


int OpenInput (/*i/o*/ register req * r,
			/*in*/ const char *  sFilename) ;
int CloseInput (/*i/o*/ register req * r) ;
int iread (/*i/o*/ register req * r,
			/*in*/ void * ptr,
                        /*in*/ size_t size) ;
char * igets (/*i/o*/ register req * r,
			/*in*/ char * s,
                        /*in*/ int    size) ;

int ReadHTML (/*i/o*/ register req * r,
	      /*in*/    char *    sInputfile,
              /*in*/    size_t *  nFileSize,
              /*out*/   SV   * *  ppBuf) ;


int OpenOutput (/*i/o*/ register req * r,
			/*in*/ const char *  sFilename) ;
int CloseOutput (/*i/o*/ register req * r) ;
int owrite (/*i/o*/ register req * r,
			/*in*/ const void * ptr, 
                        /*in*/ size_t size) ;
void oputc (/*i/o*/ register req * r,
			/*in*/ char c) ;
int oputs (/*i/o*/ register req * r,
			/*in*/ const char *  str) ;

void OutputToMemBuf (/*i/o*/ register req * r,
			/*in*/ char *  pBuf,
                     /*in*/ size_t  nBufSize) ;
char * OutputToStd (/*i/o*/ register req * r) ;


             
struct tBuf *   oBegin (/*i/o*/ register req * r) ;
void oRollback (/*i/o*/ register req * r,
			struct tBuf *   pBuf) ;
void oRollbackOutput (/*i/o*/ register req * r,
			struct tBuf *   pBuf) ;
void oCommit (/*i/o*/ register req * r,
			struct tBuf *   pBuf) ;
void oCommitToMem (/*i/o*/ register req * r,
			struct tBuf *   pBuf,
                   char *          pOut) ;

int GetContentLength (/*i/o*/ register req * r) ;

int OpenLog (/*i/o*/ register req * r,
			/*in*/ const char *  sFilename,
                 /*in*/ int           nMode) ;
int CloseLog (/*i/o*/ register req * r) ;
int FlushLog (/*i/o*/ register req * r) ;
int lprintf (/*i/o*/ register req * r,
			/*in*/ const char *  sFormat,
                 /*in*/ ...) ;
int lwrite (/*i/o*/ register req * r,
	    /*in*/  const void * ptr, 
            /*in*/  size_t size) ;

long GetLogFilePos (/*i/o*/ register req * r) ;
int GetLogHandle (/*i/o*/ register req * r) ;

/* Memory Allocation */

void _free          (/*i/o*/ register req * r,
			     void * p) ;
void * _malloc      (/*i/o*/ register req * r, size_t  size) ;
void * _realloc     (/*i/o*/ register req * r,  void * ptr, size_t oldsize, size_t  size) ;
char * _memstrcat   (/*i/o*/ register req * r,
			     const char *s, ...) ;
char * _ep_strdup   (/*i/o*/ register req * r,
                     /*in*/  const char * str) ;
char * _ep_strndup  (/*i/o*/ register req * r,
                     /*in*/  const char *   str,
                     /*in*/  int            len) ;


/* ---- from epchar.c ----- */


/*
    Character Translation
*/

struct tCharTrans
    {
    char    c ;
    char *  sHtml ;
    } ;


extern struct tCharTrans Char2Html [] ;
extern struct tCharTrans Char2Url  [] ; 
extern struct tCharTrans Html2Char [] ;
extern int sizeHtml2Char ;

/* ---- from epcmd.c ----- */



int SearchCmd (/*i/o*/ register req * r,
			/*in*/  const char *    sCmdName,
                         /*in*/  int             nCmdLen,
                         /*in*/  const char *    sArg,
                         /*in*/  int             bIgnore,
                         /*out*/ struct tCmd * * ppCmd) ;

int ProcessCmd (/*i/o*/ register req * r,
			/*in*/ struct tCmd *  pCmd,
                        /*in*/ const char *    sArg) ;

/* ---- from eputil.c ----- */


char * GetHashValue (/*in*/  HV *           pHash,
                     /*in*/  const char *   sKey,
                     /*in*/  int            nMaxLen,
                     /*out*/ char *         sValue) ;

char * GetHashValueLen (/*in*/  HV *           pHash,
                        /*in*/  const char *   sKey,
                        /*in*/  int            nLen,
                        /*in*/  int            nMaxLen,
                        /*out*/ char *         sValue) ;

int    GetHashValueInt (/*in*/  HV *           pHash,
                        /*in*/  const char *   sKey,
                        /*in*/  int            nDefault) ;

char * GetHashValueStr (/*in*/  HV *           pHash,
                        /*in*/  const char *   sKey,
                        /*in*/  char *         sDefault) ;


const char * GetHtmlArg (/*in*/  const char *    pTag,
                         /*in*/  const char *    pArg,
                         /*out*/ int *           pLen) ;

void OutputToHtml (/*i/o*/ register req * r,
 		   /*i/o*/ const char *   sData) ;

int TransHtml (/*i/o*/ register req * r,
		/*i/o*/ char *         sData,
		/*in*/   int           nLen) ;

void TransHtmlSV (/*i/o*/ register req * r,
		  /*i/o*/ SV *           pSV) ;

int GetLineNo (/*i/o*/ register req * r) ;

#ifndef WIN32
#define strnicmp strncasecmp
#endif

void Dirname (/*in*/ const char * filename,
              /*out*/ char *      dirname,
              /*in*/  int         size) ;

char * sstrdup (/*in*/ char *   pString) ;


int SetSubTextPos (/*i/o*/ register req * r,
		   /*in*/  const char *   sName,
		   /*in*/  int		  sPos) ;

int GetSubTextPos (/*i/o*/ register req * r,
		   /*in*/  const char *   sName) ;



/* ---- from epeval.c ----- */


int EvalDirect (/*i/o*/ register req * r,
			/*in*/  SV * pArg) ;

int EvalNum (/*i/o*/ register req * r,
	     /*in*/  char *        sArg,
             /*in*/  int           nFilepos,
             /*out*/ int *         pNum) ;            

int Eval (/*i/o*/ register req * r,
	  /*in*/  const char *  sArg,
          /*in*/  int           nFilepos,
          /*out*/ SV **         pRet) ;

int EvalTrans (/*i/o*/ register req * r,
	       /*in*/  char *   sArg,
               /*in*/  int      nFilepos,
               /*out*/ SV **    pRet) ;

int EvalTransFlags (/*i/o*/ register req * r,
 		    /*in*/  char *   sArg,
                   /*in*/  int      nFilepos,
                   /*in*/  int      flags,
                   /*out*/ SV **    pRet) ;

int EvalTransOnFirstCall (/*i/o*/ register req * r,
			  /*in*/  char *   sArg,
                          /*in*/  int      nFilepos,
                          /*out*/ SV **    pRet) ;            

int EvalBool (/*i/o*/ register req * r,
	      /*in*/  char *        sArg,
              /*in*/  int           nFilepos,
              /*out*/ int *         pTrue) ;

int EvalSub (/*i/o*/ register req * r,
	    /*in*/  const char *  sArg,
	    /*in*/  int           nFilepos,
	    /*in*/  const char *  sName) ;

/* ---- from epdbg.c ----- */

int SetupDebugger (/*i/o*/ register req * r) ;
