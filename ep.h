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


#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <memory.h>
#include <string.h>
#include <ctype.h>
#include <time.h>



#include <EXTERN.h>               /* from the Perl distribution     */
#include <perl.h>                 /* from the Perl distribution     */

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
#if MODULE_MAGIC_NUMBER >= 19980413
#include "compat.h"
#endif
#endif


#include "epnames.h"

#include "embperl.h"

#ifdef WIN32
#define PATH_MAX _MAX_DIR
#endif

#ifndef PATH_MAX
#define PATH_MAX 512
#endif

/* ---- from epmain.c ----- */

/* Debug Flags */

extern int  bDebug ;

/* Options */

extern int  bOptions ;


extern int  bError ;       /* Error has occured somewhere */
extern char lastwarn [ERRDATLEN]  ;

/* Apache Request Record */

#ifdef APACHE
extern request_rec * pReq ;
#endif


extern char cMultFieldSep ;  /* Separator if a form filed is multiplie defined */



extern HV *    pCacheHash ; /* Hash containing CVs to precompiled subs */
extern HV *    pFormHash ;  /* Formular data */
extern AV *    pFormArray ; /* Fieldnames */
extern HV *    pInputHash ; /* Data of input fields */

extern char *  sEvalPackage ; /* Currently active Package */
extern STRLEN  nEvalPackage ; /* Currently active Package (length) */

extern char sLogfileURLName[] ;

extern char * pBuf ;        /* Buffer which holds the html source file */
extern char * pCurrPos ;    /* Current position in html file */
extern char * pCurrStart ;  /* Current start position of html tag / eval expression */
extern char * pEndPos ;     /* end of html file */
extern char * pCurrTag ;    /* Current start position of html tag */

extern char * sSourcefile ; /* Name of sourcefile */
extern int    nSourceline ; /* Currentline in sourcefile */
extern char * pSourcelinePos ; /* Positon of nSourceline in sourcefile */
extern char * pLineNoCurrPos ; /* save pCurrPos for line no calculation */                     
                     

int iembperl_init (int  nIOType,
                   const char * sLogFile) ;
int iembperl_setreqrec  (/*in*/ SV *   pReqSV) ;
int iembperl_resetreqrec  (void) ;
int iembperl_term (void) ;
int iembperl_req  (/*in*/ char *  sInputfile,
                   /*in*/ char *  sOutputfile,
                   /*in*/ int     bDebugFlags,
                   /*in*/ int     bOptionFlags,
                   /*in*/ int     nFileSize,
                   /*in*/ HV *    pCache,
                   /*in*/ SV *    pInData,
                   /*in*/ SV *    pOutData) ;
int ScanCmdEvalsInString (/*in*/  char *   pIn,
                          /*out*/ char * * pOut,
                          /*in*/  size_t   nSize) ;
    
char * LogError (/*in*/ int   rc) ;
void CommitError (void) ;
void RollbackError (void) ;

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


extern int     nMarker ;


/* i/o functions */


int OpenInput   (/*in*/ const char *  sFilename) ;
int CloseInput  (void) ;
int iread       (/*in*/ void * ptr, size_t size, size_t nmemb) ;
char * igets    (/*in*/ char * s,   int    size) ;


int OpenOutput  (/*in*/ const char *  sFilename) ;
int CloseOutput (void) ;
int owrite      (/*in*/ const void * ptr, size_t size, size_t nmemb) ;
void oputc       (/*in*/ char c) ;
int oputs (/*in*/ const char *  str) ;

void OutputToMemBuf (/*in*/ char *  pBuf,
                     /*in*/ size_t  nBufSize) ;
void OutputToStd (void) ;


             
struct tBuf *   oBegin (void) ;
void oRollback (struct tBuf *   pBuf) ;
void oRollbackOutput (struct tBuf *   pBuf) ;
void oCommit (struct tBuf *   pBuf) ;
void oCommitToMem (struct tBuf *   pBuf,
                   char *          pOut) ;

int GetContentLength (void) ;

int OpenLog     (/*in*/ const char *  sFilename,
                 /*in*/ int           nMode) ;
int CloseLog    (void) ;
int FlushLog    (void) ;
int lprintf     (/*in*/ const char *  sFormat,
                 /*in*/ ...) ;

long GetLogFilePos (void) ;
int GetLogHandle (void) ;


/* Memory Allocation */

void _free (void * p) ;
void * _malloc (size_t  size) ;

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
extern struct tCharTrans * pCurrEscape ;
extern int bEscMode ;

/* ---- from epcmd.c ----- */

/*
   Commandtypes 
*/

enum tCmdType
    {
    cmdNorm = 1,
    cmdIf   = 2,
    cmdEndif = 4,
    cmdWhile = 8,
    cmdTable = 16,
    cmdTablerow = 32,
    cmdTextarea  = 64,

    cmdAll   = 255
    } ;

enum tCmdNo
    {
    cnNop,       
    cnTable,
    cnTr,
    cnDir,
    cnMenu,
    cnOl,
    cnUl,
    cnDl,
    cnSelect
    } ;

/* */
/* Commands */
/* */

struct tCmd
    {
    const char *    sCmdName ;     /* Commandname */
    int            ( *pProc)(/*in*/ const char *   sArg) ;   /* pointer to the procedure */
    bool            bPush ;         /* Push current state? */
    bool            bPop ;          /* Pop last state? */
    enum tCmdType   nCmdType ;      /* Type of the command  */
    bool            bScanArg ;      /* is it nessesary to scan the command arg */
    bool            bSaveArg ;      /* is it nessesary to save the command arg for later use */
    enum tCmdNo     nCmdNo ;        /* number of command to catch mismatch in start/end */
    int             bDisableOption ; /* option bit which disables this cmd */
    } ;


/*
   Stack 
*/

struct tStackEntry
    {
    enum tCmdType   nCmdType ;      /* Type of the command which the pushed the entry on the stack */
    char *          pStart ;        /* Startposition fpr loops */
    long            bProcessCmds ;  /* Process corresponding cmds */
    int             nResult ;       /* Result of Command which starts the block */
    char *          sArg ;          /* Argument of Command which starts the block */
    SV *            pSV ;           /* Additional Data */
    struct tBuf *   pBuf ;          /* Output buf for table rollback          */
    struct tCmd *   pCmd ;          /* pointer to command infos */
    } ;

#define nStackMax 100           /* Max level of nesting */


extern int nStack  ;                /* Stackpointer */
extern struct tStackEntry State ;             /* current State */
extern char ArgStack [16384] ;

extern char * pArgStack  ;

/* */
/* Stack for dynamic table counters */
/* */

struct tTableStackEntry
    {
    int             nResult ;       /* Result of Command which starts the block */
    int             nCount ;        /* Count for tables, lists etc */
    int             nCountUsed ;    /* Count for tables, lists is used in Table  */
    int             nRow ;          /* Row Count for tables, lists etc */
    int             nRowUsed ;      /* Row Count for tables, lists is used in Table  */
    int             nCol ;          /* Column Count for tables, lists etc */
    int             nColUsed ;      /* Column Count for tables, lists is used in Table  */
    int             nMaxRow ;       /* maximum rows */
    int             nMaxCol ;       /* maximum columns */
    int             nTabMode ;      /* table mode */
    int             bHead ;         /* this row contains a heading */
    int             bRowHead ;      /* the entire row is made of th tags */
    int             nStackTable ;   /* stack entry for table tag */
    } ;


extern struct tTableStackEntry TableStack [nStackMax] ; /* Stack for table */

extern int nTableStack  ;                /* Stackpointer */

extern struct tTableStackEntry TableState ;             /* current State */


extern int nTabMode    ;    /* mode for next table (only takes affect after next <TABLE> */
extern int nTabMaxRow  ;    /* maximum rows for next table (only takes affect after next <TABLE> */
extern int nTabMaxCol  ;    /* maximum columns for next table (only takes affect after next <TABLE> */


int  SearchCmd          (/*in*/  const char *    sCmdName,
                         /*in*/  int             nCmdLen,
                         /*in*/  const char *    sArg,
                         /*in*/  int             bIgnore,
                         /*out*/ struct tCmd * * ppCmd) ;

int  ProcessCmd        (/*in*/ struct tCmd *  pCmd,
                        /*in*/ const char *    sArg) ;

extern int  bStrict ; /* aply use strict in each eval */

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

const char * GetHtmlArg (/*in*/  const char *    pTag,
                         /*in*/  const char *    pArg,
                         /*out*/ int *           pLen) ;

void TransHtml (/*i/o*/ char *  sData) ;


int GetLineNo (void) ;

#ifndef WIN32
#define strnicmp strncasecmp
#endif

void Dirname (/*in*/ const char * filename,
              /*out*/ char *      dirname,
              /*in*/  int         size) ;


/* ---- from epeval.c ----- */

extern int numEvals ;
extern int numCacheHits ;

int EvalDirect (/*in*/  SV * pArg) ;

int EvalNum (/*in*/  char *        sArg,
             /*in*/  int           nFilepos,
             /*out*/ int *         pNum) ;            

int Eval (/*in*/  const char *  sArg,
          /*in*/  int           nFilepos,
          /*out*/ SV **         pRet) ;

int EvalTrans (/*in*/  char *   sArg,
               /*in*/  int      nFilepos,
               /*out*/ SV **    pRet) ;

int EvalTransOnFirstCall (/*in*/  char *   sArg,
                          /*in*/  int      nFilepos,
                          /*out*/ SV **    pRet) ;            
