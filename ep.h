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

#if !defined(__FreeBSD__)
extern int errno;
extern int sys_nerr;
extern char *sys_errlist[];
#endif

/* ---- from epmain.c ----- */

/* Debug Flags */

extern int  bDebug ;

/* Apache Request Record */

#ifdef APACHE
extern request_rec * pReq ;
#endif

extern pid_t nPid ;

extern HV *    pCacheHash ; /* Hash containing CVs to precompiled subs */
extern HV *    pFormHash ;  /* Formular data */
extern HV *    pInputHash ; /* Data of input fields */

extern SV *    pNameSpace ; /* Currently active Namespace */

extern char sLogfileURLName[] ;

extern char * pBuf ;        /* Buffer which holds the html source file */
extern char * pCurrPos ;    /* Current position in html file */
extern char * pCurrStart ;  /* Current start position of html tag / eval expression */
extern char * pEndPos ;     /* end of html file */
extern char * pCurrTag ;    /* Current start position of html tag */

extern int  bSafeEval ;     /* Should we eval everything in a safe namespace? */

int iembperl_init (int  nIOType,
                   const char * sLogFile) ;
int iembperl_setreqrec  (/*in*/ SV *   pReqSV) ;
int iembperl_resetreqrec  () ;
int iembperl_term (void) ;
int iembperl_req  (/*in*/ char *  sInputfile,
                   /*in*/ char *  sOutputfile,
                   /*in*/ int     bDebugFlags,
                   /*in*/ char *  pNameSpaceName,
                   /*in*/ int     nFileSize,
                   /*in*/ HV *    pCache) ;


/* ---- from epio.c ----- */


/*
// Datastructure for buffering output
*/

struct tBuf
    {
    struct tBuf *   pNext ;     /* Next buffer  */
    int             nSize ;     /* Size in bytes */
    int             nMarker ;   /* nesting level */
    int             nCount ;    /* output count including this buffer */
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

int GetContentLength () ;

int OpenLog     (/*in*/ const char *  sFilename) ;
int CloseLog    () ;
int FlushLog    () ;
int lprintf     (/*in*/ const char *  sFormat,
                 /*in*/ ...) ;


/* Memory Allocation */

void _free (void * p) ;
void * _malloc (size_t  size) ;

/* ---- from epchar.c ----- */


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
                         /*in*/  const char *    sArg,
                         /*in*/  int             bIgnore,
                         /*out*/ struct tCmd * * ppCmd) ;

int  ProcessCmd        (/*in*/ struct tCmd *  pCmd,
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

const char * GetHtmlArg (/*in*/  const char *    pTag,
                         /*in*/  const char *    pArg,
                         /*out*/ int *           pLen) ;

void TransHtml (/*i/o*/ char *  sData) ;

#define strnicmp strncasecmp



/* ---- from epeval.c ----- */

extern int numEvals ;
extern int numCacheHits ;

int EvalNum (/*in*/  char *        sArg,
             /*in*/  int           nFilepos,
             /*out*/ int *         pNum) ;            

int Eval (/*in*/  const char *  sArg,
          /*in*/  int           nFilepos,
          /*out*/ SV **         pRet) ;

int EvalTrans (/*in*/  char *   sArg,
               /*in*/  int      nFilepos,
               /*out*/ SV **    pRet) ;


