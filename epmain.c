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


/* Version */

static char sVersion [] = VERSION ;


/* pid */

pid_t nPid ;

/* Apache Request Record */

#ifdef APACHE
request_rec * pReq = NULL ;
#endif


/* Debugging */

int  bDebug = dbgAll & ~dbgMem & ~dbgEnv;


/* Should we eval everything in a safe namespace? */

static int  bSafeEval = 0 ;  


int  nIOType   = epIOPerl ;

static char sCmdFifo [] = "/local/www/cgi-bin/embperl/cmd.fifo" ;
static char sRetFifo [1024] ;

static char sRetFifoName [] = "__RETFIFO" ;

static char cMultFieldSep = '\t' ;  /* Separator if a form filed is multiplie defined */

static char sEvalErrName   [] = "@" ;
static char sEnvHashName   [] = "ENV" ;
static char sFormHashName  [] = "HTML::Embperl::fdat" ;
static char sFormArrayName [] = "HTML::Embperl::ffld" ;
static char sInputHashName [] = "HTML::Embperl::idat" ;
static char sTabCountName  [] = "HTML::Embperl::cnt" ;
static char sTabRowName    [] = "HTML::Embperl::row" ;
static char sTabColName    [] = "HTML::Embperl::col" ;
static char sTabMaxRowName [] = "HTML::Embperl::maxrow" ;
static char sTabMaxColName [] = "HTML::Embperl::maxcol" ;
static char sTabModeName   [] = "HTML::Embperl::tabmode" ;

static char sNameSpaceHashName [] = "HTML::Embperl::NameSpace" ;
static char sLogfileURLName[] = "HTML::Embperl::LogfileURL" ;
static char sCacheHashName[]  = "HTML::Embperl::cache" ;
static char sPackageName[]    = "HTML::Embperl::package" ;


static HV *    pEnvHash ;   /* environement from CGI Script */
static HV *    pFormHash ;  /* Formular data */
static HV *    pInputHash ; /* Data of input fields */
static HV *    pNameSpaceHash ; /* Hash of NameSpace Objects */
static AV *    pFormArray ; /* Fieldnames */
static SV *    pEvalErr ;   /* $@ -> Result of Eval */
static HV *    pCacheHash ; /* Hash containing CVs to precompiled subs */

static SV *    pNameSpace ; /* Currently active Namespace */


static char *  sCurrPackage ; /* Name of package to eval everything */

static char * pCurrPos ;    /* Current position in html file */
static char * pCurrStart ;  /* Current start position of html tag / eval expression */
static char * pEndPos ;     /* end of html file */
static char * pCurrTag ;    /* Current start position of html tag */

/*
   Additional Error info   
*/


char errdat1 [ERRDATLEN]  ;
char errdat2 [ERRDATLEN]  ;


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
    enum tCmdNo     nCmdNo ;        /* number of command to catch mismatch in start/end */
    } ;

static struct tCmd * pCurrCmd ;    /* Current cmd which is excuted */


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
    struct tBuf *   pBuf ;          /* Output buf for table rollback          */
    struct tCmd *   pCmd ;          /* pointer to command infos */
    } ;

#define nStackMax 100           /* Max level of nesting */

struct tStackEntry Stack [nStackMax] ; /* Stack for if, while, etc. */

int nStack = 0 ;                /* Stackpointer */

struct tStackEntry State ;             /* current State */

char ArgStack [16384] ;

char * pArgStack = ArgStack ;


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
    } ;


struct tTableStackEntry TableStack [nStackMax] ; /* Stack for table */

int nTableStack = 0 ;                /* Stackpointer */

struct tTableStackEntry TableState ;             /* current State */


int nTabMode    ;    /* mode for next table (only takes affect after next <TABLE> */
int nTabMaxRow  ;    /* maximum rows for next table (only takes affect after next <TABLE> */
int nTabMaxCol  ;    /* maximum columns for next table (only takes affect after next <TABLE> */

/*// ---------------------------------------------------------------------------- */
/*// Commandtable... */
/*// */


int CmdIf (/*in*/ const char *   sArg) ;
int CmdElse (/*in*/ const char *   sArg) ;
int CmdElsif (/*in*/ const char *   sArg) ;
int CmdEndif (/*in*/ const char *   sArg) ;

int CmdWhile (/*in*/ const char *   sArg) ;
int CmdEndwhile (/*in*/ const char *   sArg) ;
int CmdHidden (/*in*/ const char *   sArg) ;

int HtmlTable    (/*in*/ const char *   sArg) ;
int HtmlEndtable (/*in*/ const char *   sArg) ;
int HtmlRow      (/*in*/ const char *   sArg) ;
int HtmlEndrow   (/*in*/ const char *   sArg) ;
int HtmlInput    (/*in*/ const char *   sArg) ;
int HtmlTextarea   (/*in*/ const char *   sArg) ;
int HtmlEndtextarea(/*in*/ const char *   sArg) ;
int HtmlBody       (/*in*/ const char *   sArg) ;

    

struct tCmd CmdTab [] =
    {
        { "/dir",     HtmlEndtable, 0, 1, cmdTable,     0, cnDir } ,
        { "/dl",      HtmlEndtable, 0, 1, cmdTable,     0, cnDl  } ,
        { "/menu",    HtmlEndtable, 0, 1, cmdTable,     0, cnMenu  } ,
        { "/ol",      HtmlEndtable, 0, 1, cmdTable,     0, cnOl  } ,
        { "/select",  HtmlEndtable, 0, 1, cmdTable,     0, cnSelect  } ,
        { "/table",   HtmlEndtable, 0, 1, cmdTable,     0, cnTable  } ,
        { "/textarea", HtmlEndtextarea, 0, 1, cmdTextarea, 0, cnNop  } ,
        { "/tr",      HtmlEndrow,   0, 1, cmdTablerow,  0, cnTr  } ,
        { "/ul",      HtmlEndtable, 0, 1, cmdTable,     0, cnUl  } ,
        { "body",     HtmlBody,     0, 0, cmdNorm,      1, cnNop   } ,
        { "dir",      HtmlTable,    1, 0, cmdTable,     1, cnDir   } ,
        { "dl",       HtmlTable,    1, 0, cmdTable,     1, cnDl   } ,
        { "else",     CmdElse,      0, 0, cmdIf,        0, cnNop   } ,
        { "elsif",    CmdElsif,     0, 0, cmdIf,        0, cnNop   } ,
        { "endif",    CmdEndif,     0, 1, cmdIf | cmdEndif, 0, cnNop   } ,
        { "endwhile", CmdEndwhile,  0, 1, cmdWhile,     0, cnNop   } ,
        { "hidden",   CmdHidden,    0, 0, cmdNorm,      0, cnNop   } ,
        { "if",       CmdIf,        1, 0, cmdIf | cmdEndif, 0, cnNop   } ,
        { "input",    HtmlInput,    0, 0, cmdNorm,      1, cnNop   } ,
        { "menu",     HtmlTable,    1, 0, cmdTable,     1, cnMenu   } ,
        { "ol",       HtmlTable,    1, 0, cmdTable,     1, cnOl   } ,
        { "select",   HtmlTable,    1, 0, cmdTable,     1, cnSelect   } ,
        { "table",    HtmlTable,    1, 0, cmdTable,     1, cnTable   } ,
        { "textarea", HtmlTextarea, 1, 0, cmdTextarea,  0, cnNop   } ,
        { "tr",       HtmlRow,      1, 0, cmdTablerow,  1, cnTr   } ,
        { "ul",       HtmlTable,    1, 0, cmdTable,     1, cnUl   } ,
        { "while",    CmdWhile,     1, 0, cmdWhile,     0, cnNop   } ,

    } ;

/* */
/* forward definition for function prototypes */
/* */

static int ScanCmdEvalsInString (/*in*/  char *   pIn,
                                 /*out*/ char * * pOut,
                                 /*in*/  size_t   nSize) ;

/* */
/* print error */
/* */

void LogError (/*in*/ int   rc)

    {
    const char * msg ;
    
    EPENTRY (LogError) ;
    
    errdat1 [sizeof (errdat1) - 1] = '\0' ;
    errdat2 [sizeof (errdat2) - 1] = '\0' ;
    
    switch (rc)
        {
        case ok:                        msg ="[%d]ERR:  %d: ok\n" ; break ;
        case rcStackOverflow:           msg ="[%d]ERR:  %d: Stack Overflow\n" ; break ;
        case rcArgStackOverflow:        msg ="[%d]ERR:  %d: Argumnet Stack Overflow (%s)\n" ; break ;
        case rcStackUnderflow:          msg ="[%d]ERR:  %d: Stack Underflow\n" ; break ;
        case rcEndifWithoutIf:          msg ="[%d]ERR:  %d: endif without if\n" ; break ;
        case rcElseWithoutIf:           msg ="[%d]ERR:  %d: else without if\n" ; break ;
        case rcEndwhileWithoutWhile:    msg ="[%d]ERR:  %d: endwhile without while\n" ; break ;
        case rcEndtableWithoutTable:    msg ="[%d]ERR:  %d: blockend <%s> does not match blockstart <%s>\n" ; break ;
        case rcCmdNotFound:             msg ="[%d]ERR:  %d: Unknown Command %s\n" ; break ;
        case rcOutOfMemory:             msg ="[%d]ERR:  %d: Out of memory\n" ; break ;
        case rcPerlVarError:            msg ="[%d]ERR:  %d: Perl variable error %s\n" ; break ;
        case rcHashError:               msg ="[%d]ERR:  %d: Perl hash error %s\n" ; break ;
        case rcArrayError:              msg ="[%d]ERR:  %d: Perl array error %s\n" ; break ;
        case rcFileOpenErr:             msg ="[%d]ERR:  %d: File %s open error: %s\n" ; break ;    
        case rcLogFileOpenErr:          msg ="[%d]ERR:  %d: Logfile %s open error: %s\n" ; break ;    
        case rcMissingRight:            msg ="[%d]ERR:  %d: Missing right %s\n" ; break ;
        case rcNoRetFifo:               msg ="[%d]ERR:  %d: No Return Fifo\n" ; break ;
        case rcMagicError:              msg ="[%d]ERR:  %d: Perl Magic Error\n" ; break ;
        case rcWriteErr:                msg ="[%d]ERR:  %d: File write Error\n" ; break ;
        case rcUnknownNameSpace:        msg ="[%d]ERR:  %d: Namespace %s unknown\n" ; break ;
        case rcInputNotSupported:       msg ="[%d]ERR:  %d: Input not supported in mod_perl mode\n" ; break ;
        case rcCannotUsedRecursive:     msg ="[%d]ERR:  %d: Cannot be called recursivly in mod_perl mode\n" ; break ;
        case rcEndtableWithoutTablerow: msg ="[%d]ERR:  %d: </tr> without <tr>\n" ; break ;
        case rcEndtextareaWithoutTextarea: msg ="[%d]ERR:  %d: </textarea> without <textarea>\n" ; break ;
        case rcEvalErr:                 msg ="[%d]ERR:  %d: Error in Perl code %s\n" ; break ;
        case rcExecCGIMissing:          msg ="[%d]ERR:  %d: Forbidden %s: Options ExecCGI not set in your Apache configs\n" ; break ;
        case rcIsDir:                   msg ="[%d]ERR:  %d: Forbidden %s is a directory\n" ; break ;
        case rcXNotSet:                 msg ="[%d]ERR:  %d: Forbidden %s X Bit not set\n" ; break ;
        case rcNotFound:                msg ="[%d]ERR:  %d: Not found %s\n" ; break ;
        default:                        msg ="[%d]ERR:  %d: Error %s\n" ; break ; 
        }
    
    if (bDebug)
        lprintf (msg, nPid , rc, errdat1, errdat2) ;
    fprintf (stderr, msg, nPid , rc, errdat1, errdat2) ;
        
    errdat1[0] = '\0' ;
    errdat2[0] = '\0' ;
    }


/* ---------------------------------------------------------------------------- */
/* Output a string and escapte html special character to html special representation (&xxx;) */
/* */
/* i/o sData     = input:  perl string */
/* */
/* ---------------------------------------------------------------------------- */

void OutputToHtml (/*i/o*/ const char *  sData)

    {
    char * pHtml  ;
    const char * p = sData ;
    
    EPENTRY (OutputToHtml) ;

    while (*sData)
        {
        if (*sData == '\\')
            {
            if (p != sData)
                owrite (p, 1, sData - p) ;
            sData++ ;
            p = sData ;
            }
        else
            {
            pHtml = Char2Html[(unsigned char)(*sData)].sHtml ;
            if (*pHtml)
                {
                if (p != sData)
                    owrite (p, 1, sData - p) ;
                oputs (pHtml) ;
                p = sData + 1;
                }
            }
        sData++ ;
        }
    if (p != sData)
        owrite (p, 1, sData - p) ;
    }


/* */
/* Eval PERL Statements */
/* */
/* Return Perl Scalar Value */
/* */

#define EVAL_SUB

int EvalAll (/*in*/  const char *  sArg,
             /*out*/ SV **   pRet)             
    {
    int   num ;         
    int   nCountUsed = TableState.nCountUsed ;
    int   nRowUsed   = TableState.nRowUsed ;
    int   nColUsed   = TableState.nColUsed ;
#ifndef EVAL_SUB    
    SV *  pSVArg ;
#endif
    dSP;                            /* initialize stack pointer      */

    EPENTRY (EvalAll) ;

    if (bDebug & dbgEval)
        lprintf ("[%d]EVAL< %s\n", nPid, sArg) ;

    tainted = 0 ;

#ifdef EVAL_SUB    

    ENTER;                          /* everything created after here */
    SAVETMPS;                       /* ...is a temporary variable.   */
    PUSHMARK(sp);                   /* remember the stack pointer    */
    XPUSHs(sv_2mortal(newSVpv((char *)sArg, strlen (sArg)))); /* push the base onto the stack  */
    PUTBACK;                        /* make local stack pointer global */
    num = perl_call_pv ("HTML::Embperl::_eval_", G_SCALAR | G_EVAL) ; /* call the function             */
#else
    
    pSVArg = sv_2mortal(newSVpv((char *)sArg, strlen (sArg))) ;

    /*num = perl_eval_sv (pSVArg, G_SCALAR) ; /* call the function             */ */
    num = perl_eval_sv (pSVArg, G_DISCARD) ; /* call the function             */
    num = 0 ;
#endif    
    SPAGAIN;                        /* refresh stack pointer         */
    
    if (bDebug & dbgMem)
        lprintf ("[%d]SVs:  %d\n", nPid, sv_count) ;
    /* pop the return value from stack */
    if (num == 1)   
        {
        *pRet = POPs ;
        SvREFCNT_inc (*pRet) ;
        if ((nCountUsed != TableState.nCountUsed ||
             nColUsed != TableState.nColUsed ||
             nRowUsed != TableState.nRowUsed) &&
              !SvOK (*pRet))
            TableState.nResult = 0 ;

        if ((bDebug & dbgTab) &&
            (TableState.nCountUsed ||
             TableState.nColUsed ||
             TableState.nRowUsed))
            lprintf ("[%d]TAB:  nResult = %d\n", nPid, TableState.nResult) ;

        if (bDebug & dbgEval)
            if (SvOK (*pRet))
                lprintf ("[%d]EVAL> %s\n", nPid, SvPV (*pRet, na)) ;
            else
                lprintf ("[%d]EVAL> <undefined>\n", nPid) ;
        }
     else
        {
        *pRet = NULL ;
        if (bDebug & dbgEval)
            lprintf ("[%d]EVAL> <NULL>\n", nPid) ;
        }

     PUTBACK;

     if (SvOK (pEvalErr) && *(SvPV (pEvalErr, na)) != '\0')
         {
         strncpy (errdat1, SvPV (pEvalErr, na), sizeof (errdat1) - 1) ;
         LogError (rcEvalErr) ;
         }


#ifdef EVAL_SUB    
    FREETMPS;                       /* free that return value        */
    LEAVE;                       /* ...and the XPUSHed "mortal" args.*/
#endif
    
    return num ;
    }


/* */
/* Eval PERL Statements in safe namespace */
/* */
/* Return Perl Scalar Value */
/* */


int EvalSafe (/*in*/  const char *  sArg,
              /*out*/ SV **   pRet)             
    {
    int   num ;         
    int   nCountUsed = TableState.nCountUsed ;
    int   nRowUsed   = TableState.nRowUsed ;
    int   nColUsed   = TableState.nColUsed ;
    dSP;                            /* initialize stack pointer      */

    EPENTRY (EvalSafe) ;

    if (bDebug & dbgEval)
        lprintf ("[%d]EVAL< %s\n", nPid, sArg) ;


    ENTER;                          /* everything created after here */
    SAVETMPS;                       /* ...is a temporary variable.   */
    PUSHMARK(sp);                   /* remember the stack pointer    */
    XPUSHs(pNameSpace) ;
    XPUSHs(sv_2mortal(newSVpv((char *)sArg, strlen (sArg)))); 
    PUTBACK;                        /* make local stack pointer global */
    num = perl_call_method ("reval", G_SCALAR | G_EVAL) ;  /* call the function             */
    SPAGAIN;                        /* refresh stack pointer         */
    
    if (bDebug & dbgMem)
        lprintf ("[%d]SVs:  %d\n", nPid, sv_count) ;
    /* pop the return value from stack */
    if (num == 1)   
        {
        *pRet = POPs ;
        SvREFCNT_inc (*pRet) ;
        if ((nCountUsed != TableState.nCountUsed ||
             nColUsed != TableState.nColUsed ||
             nRowUsed != TableState.nRowUsed) &&
              !SvOK (*pRet))
            TableState.nResult = 0 ;

        if ((bDebug & dbgTab) &&
            (TableState.nCountUsed ||
             TableState.nColUsed ||
             TableState.nRowUsed))
            lprintf ("[%d]TAB:  nResult = %d\n", nPid, TableState.nResult) ;

        if (bDebug & dbgEval)
            if (SvOK (*pRet))
                lprintf ("[%d]EVAL> %s\n", nPid, SvPV (*pRet, na)) ;
            else
                lprintf ("[%d]EVAL> <undefined>\n",nPid) ;
        }
     else
        {
        *pRet = NULL ;
        if (bDebug & dbgEval)
            lprintf ("[%d]EVAL> <NULL>\n", nPid) ;
        }

    PUTBACK;

     if (SvOK (pEvalErr) && *(SvPV (pEvalErr, na)) != '\0')
         {
         strncpy (errdat1, SvPV (pEvalErr, na), sizeof (errdat1) - 1) ;
         LogError (rcEvalErr) ;
         }

    
    
    FREETMPS;                       /* free that return value        */
    LEAVE;                       /* ...and the XPUSHed "mortal" args.*/
    
    return num ;
    }


int Eval (/*in*/  const char *  sArg,
          /*out*/ SV **   pRet)             


    {
    EPENTRY (Eval) ;

    
    if (bSafeEval)
        return EvalSafe (sArg, pRet) ;
    else
        return EvalAll (sArg, pRet) ;
    }




/* */
/* Eval PERL Statements */
/* */
/* Return int */
/* */



int EvalNum (/*in*/  const char *  sArg,
             /*out*/ int *   pNum)             
    {
    SV * pRet ;
    int  n ;

    EPENTRY (EvalNum) ;


    n = Eval (sArg, &pRet) ;
    
    if (pRet)
        {
        *pNum = SvIV (pRet) ;
        SvREFCNT_dec (pRet) ;
        }
    else
        pNum = 0 ;

    return ok ;
    }
    
/* */
/* Magic */
/* */


static int notused ;

INTMG (Count, TableState.nCount, TableState.nCountUsed) 
INTMG (Row, TableState.nRow, TableState.nRowUsed) 
INTMG (Col, TableState.nCol, TableState.nColUsed) 
INTMG (MaxRow, nTabMaxRow, notused) 
INTMG (MaxCol, nTabMaxCol, notused) 
INTMG (TabMode, nTabMode, notused) 


/* */
/* compare commands */
/* */

static int CmpCmd (/*in*/ const void *  p1,
            /*in*/ const void *  p2)

    {
    return strcmp (*((const char * *)p1), *((const char * *)p2)) ;
    }



/* */
/* Search Command in Commandtable */
/* */

static int  SearchCmd   (/*in*/  const char *    sCmdName,
                         /*in*/  const char *    sArg,
                         /*in*/  int             bIgnore,
                         /*out*/ struct tCmd * * ppCmd)

    {
    struct tCmd *  pCmd ;
    int            rc ;
    char           sCmdLwr [64] ;
    char *         p ;
    int            i ;


    EPENTRY (SearchCmd) ;

    i = sizeof (sCmdLwr) - 1 ;
    p = sCmdLwr ;
    while (--i > 0 && (*p++ = tolower (*sCmdName++)) != '\0')
        ;
    *p = '\0' ;
    
    p = sCmdLwr ;
    pCmd = (struct tCmd *)bsearch (&p, CmdTab, sizeof (CmdTab) / sizeof (struct tCmd), sizeof (struct tCmd), CmpCmd) ;

    if (bDebug & dbgAllCmds)
        lprintf ("[%d]CMD%c:  +%02d Cmd = '%s' Arg = '%s'\n", nPid, (pCmd == NULL)?'-':'+', nStack, sCmdLwr, sArg) ;

    if (pCmd == NULL && bIgnore)
        return rcCmdNotFound ;

    if ((bDebug & dbgCmd) && (bDebug & dbgAllCmds) == 0)
        lprintf ("[%d]CMD:  +%02d Cmd = '%s' Arg = '%s'\n", nPid, nStack, sCmdName, sArg) ;
    
    if (pCmd == NULL)
        {
        strncpy (errdat1, sCmdName, sizeof (errdat1) - 1) ;
        return rcCmdNotFound ;
        }

    
    *ppCmd = pCmd ;
    
    return ok ;
    }



/* */
/* Process a Command */
/* */



static int  ProcessCmd (/*in*/ struct tCmd *  pCmd,
                        /*in*/ const char *    sArg)

    {                        
    int            rc ;
    int            nArgLen ;

    EPENTRY (ProcessCmd) ;
    
    
    if ((pCmd -> nCmdType & State.bProcessCmds) == 0)
        return ok ; /* ignore it */


    if (pCmd -> bPush)
        if (nStack > nStackMax - 2)
            return rcStackOverflow ;
        else
            {
            nArgLen = strlen (sArg) + 1 ;
            if (pArgStack + nArgLen >= ArgStack + sizeof (ArgStack))
                {
                sprintf (errdat1, "nArgLen=%d, pArgStack=%d",nArgLen,  pArgStack - ArgStack) ;
                return rcArgStackOverflow ;
                }

            Stack[nStack++] = State ;
            
            State.nCmdType  = pCmd -> nCmdType ;
            State.bProcessCmds = Stack[nStack-1].bProcessCmds ;
            State.pStart    = pCurrPos ;
            State.sArg      = strcpy (pArgStack, sArg) ;
            pArgStack += nArgLen ;
            State.pBuf      = NULL ;
            State.pCmd      = pCmd ;
            }

    pCurrCmd = pCmd ;

    rc = (*pCmd -> pProc)(sArg) ;
    

    if (pCmd -> bPop && State.pStart == NULL)
        if (nStack < 1)
            return rcStackUnderflow ;
        else
            {
            if (State.sArg)
                pArgStack = State.sArg ;
            
            State = Stack[--nStack];
            }

    return rc ;
    }




/*// ---------------------------------------------------------------------------- */
/*// if command ... */
/*// */

int CmdIf (/*in*/ const char *   sArg)
    {
    int rc = ok ;

    EPENTRY (CmdIf) ;
    
    if (State.bProcessCmds == cmdAll)
        {
        rc = EvalNum (sArg, &State.nResult) ;
    
        if (State.nResult) 
            {
            State.bProcessCmds = cmdAll ;
            }
        else
            {
            State.bProcessCmds = cmdIf ;
            }
        }
    else
        State.nResult = -1 ;

    return rc ;
    }

/*// ---------------------------------------------------------------------------- */
/*// elsif command ... */
/*// */

int CmdElsif  (/*in*/ const char *   sArg)
    {
    int rc = ok ;


    EPENTRY (CmdElsif) ;
    
    if ((State.nCmdType & cmdIf) == 0)
        return rcElseWithoutIf ;
    
        
    if (State.nResult == -1)
        return ok ;

    if (State.nResult == 0)
        {    
        int nArgLen = strlen (sArg) + 1 ;
        pArgStack = State.sArg ;
        if (pArgStack + nArgLen >= ArgStack + sizeof (ArgStack))
                {
                sprintf (errdat1, "nArgLen=%d, pArgStack=%d",nArgLen,  pArgStack - ArgStack) ;
                return rcArgStackOverflow ;
                }
        State.sArg      = strcpy (pArgStack, sArg) ;
        pArgStack += nArgLen ;


        rc = EvalNum (sArg, &State.nResult) ;
    
        if (State.nResult) 
            State.bProcessCmds = cmdAll ;
        else
            State.bProcessCmds = cmdIf ;
        }
    else
        {
        State.bProcessCmds = cmdEndif ;
        State.nResult      = 0 ;
        }

    return rc ;
    }

/*// ---------------------------------------------------------------------------- */
/*// else command ... */
/*// */

int CmdElse  (/*in*/ const char *   sArg)
    {

    EPENTRY (CmdElse) ;
    
    
    if ((State.nCmdType & cmdIf) == 0)
        return rcElseWithoutIf ;

    if (State.nResult == -1)
        return ok ;


    
    if (State.nResult)
        {
        State.bProcessCmds = cmdIf ;
        State.nResult      = 0 ;
        }
    else
        {
        State.bProcessCmds = cmdAll ;
        State.nResult      = 1 ;
        }

    return ok ;
    }
                        

/*// ---------------------------------------------------------------------------- */
/*// endif befehl ... */
/*// */

int CmdEndif (/*in*/ const char *   sArg)
    {
    EPENTRY (CmdEndif) ;

    
    
    State.pStart    = NULL ;

    if ((State.nCmdType & cmdIf) == 0)
        return rcEndifWithoutIf ;

    return ok ;
    }
                        


/*// ---------------------------------------------------------------------------- */
/*// while command ... */
/*// */

int CmdWhile (/*in*/ const char *   sArg)
    {
    int rc ;
    
    EPENTRY (CmdWhile) ;

    rc = EvalNum (sArg, &State.nResult) ;
    
    if (State.nResult) 
        State.bProcessCmds = cmdAll ;
    else
        State.bProcessCmds = cmdWhile ;

    return rc ;
    }

/*// ---------------------------------------------------------------------------- */
/*// endwhile command ... */
/*// */

int CmdEndwhile (/*in*/ const char *   sArg)
    {
    int rc = ok ;

    EPENTRY (CmdEndwhile) ;


    if (State.nCmdType != cmdWhile)
        return rcEndwhileWithoutWhile ;

    
    if (State.nResult)
        {
        rc = EvalNum (State.sArg, &State.nResult) ;
    
        if (State.nResult) 
            {
            pCurrPos = State.pStart ;        
            return rc ;
            }
        }

    State.pStart    = NULL ;

    return rc ;
    }


/*// ---------------------------------------------------------------------------- */
/*// hidden command ... */
/*// */

int CmdHidden (/*in*/ const char *   sArg)

    {
    char * pSub ;
    char * pEnd ;
    char * pKey ;
    SV *   psv ;
    HV *   pSubHash ;
    HV *   pAddHash ;
    HE *   pEntry ;
    I32    l ;


    EPENTRY (CmdHidden) ;
    
    pSub = strchr (sArg, ',') ;
    if (pSub)
        {
        pEnd = pSub ;
        if (pEnd > sArg)
            {
            pEnd-- ;
            while (pEnd > sArg && isspace (*pEnd))
                pEnd-- ;
            pEnd++ ;
            }
        *pEnd = '\0' ;
        pSub++ ;
        while (*pSub && isspace (*pSub))
            pSub++ ;
        }
    else
        pSub = "" ;

    if (sArg [0] != '\0')
        {
        if ((pAddHash = perl_get_hv ((char *)sArg, TRUE)) == NULL)
            return rcHashError ;
        }
    else
        pAddHash = pFormHash ;

    if (pSub [0] != '\0')
        {
        if ((pSubHash = perl_get_hv (pSub, TRUE)) == NULL)
            return rcHashError ;
        }
    else
        pSubHash = pInputHash ;

    oputc ('\n') ;
    hv_iterinit (pAddHash) ;
    while (pEntry = hv_iternext (pAddHash))
        {
        pKey = hv_iterkey (pEntry, &l) ;
        if (!hv_exists (pSubHash, pKey, strlen (pKey)))
            {
            psv = hv_iterval (pAddHash, pEntry) ;

            
            oputs ("<input type=\"hidden\" name=\"") ;
            oputs (pKey) ;
            oputs ("\" value=\"") ;
            OutputToHtml (SvPV (psv, na)) ;
            oputs ("\">\n") ;
            }
        }

    return ok ;
    }




/* ---------------------------------------------------------------------------- */
/* find substring ignore case */
/* */
/* in  pSring  = string to search in (any case) */
/* in  pSubStr = string to search for (must be upper case) */
/* */
/* out ret  = pointer to pSubStr in pStringvalue or NULL if not found */
/* */
/* ---------------------------------------------------------------------------- */


#define strnicmp strncasecmp


const char * stristr (/*in*/ const char *   pString,
                      /*in*/ const char *   pSubString)

    {
    char c = *pSubString ;
    int  l = strlen (pSubString) ;
    
    do
        {
        while (*pString && toupper (*pString) != c)
            pString++ ;

        if (*pString == '\0')
            return NULL ;

        if (strnicmp (pString, pSubString, l) == 0)
            return pString ;
        pString++ ;
        }
    
    while (TRUE) ;
    }

/* ---------------------------------------------------------------------------- */
/* make string lower case */
/* */
/* i/o  pSring  = string to search in (any case) */
/* */
/* ---------------------------------------------------------------------------- */


char * strlower (/*in*/ char *   pString)

    {
    char * p = pString ;
    
    while (*p)
        {
        *p = tolower (*p) ;
        p++ ;
        }

    return pString ;
    }


/* */
/* compare commands */
/* */

static int CmpCharTrans (/*in*/ const void *  pKey,
                         /*in*/ const void *  pEntry)

    {
    return strcmp ((const char *)pKey, ((struct tCharTrans *)pEntry) -> sHtml) ;
    }



/* ---------------------------------------------------------------------------- */
/* replace html special character representation (&xxx;) with correct chars */
/* and delete all html tags */
/* The Replacement is done in place, the whole string will become shorter */
/* and is padded with spaces */
/* tags and special charcters which are preceeded by a \ are not translated */
/* carrige returns are replaced by spaces */
/* */
/* i/o sData     = input:  html string */
/*                 output: perl string */
/* */
/* ---------------------------------------------------------------------------- */

void TransHtml (/*i/o*/ char *  sData)

    {
    char * p = sData ;
    char * s = NULL ;
    char * e = sData + strlen (sData) ;
    struct tCharTrans * pChar ;

    EPENTRY (TransHtml) ;

    while (*p)
        {
        if (*p == '\\')
            {
            if (p[1] == '<')
                { /*  Quote next HTML tag */
                p += 2 ;
                while (*p && *p != '>')
                    p++ ;
                }
            else if (p[1] == '&')
                { /*  Quote next HTML char */
                p += 2 ;
                while (*p && *p != ';')
                    p++ ;
                }
            else
                p++ ; /* Nothing to quote */
            }
        else if (*p == '\r')
            {
            *p++ = ' ' ;
            }
        else
            {
            if (p[0] == '<')
                { /*  count HTML tag length */
                s = p ;
                p++ ;
                while (*p && *p != '>')
                    p++ ;
                if (*p)
                    p++ ;
                else
                    { /* missing left '>' -> no html tag  */
                    p = s ;
                    s = NULL ;
                    }
                }
            else if (p[0] == '&')
                { /*  count HTML char length */
                s = p ;
                p++ ;
                while (*p && *p != ';')
                    p++ ;

                if (*p)
                    {
                    *p = '\0' ;
                    p++ ;
                    }
                pChar = (struct tCharTrans *)bsearch (s, Html2Char, sizeHtml2Char, sizeof (struct tCharTrans), CmpCharTrans) ;
                if (pChar)
                    *s = pChar -> c ;
                else
                    *s = '?' ;
                s++ ;
                
                }

            if (s && (p - s) > 0)
                { /* copy rest of string, pad with spaces */
                memmove (s, p, e - p + 1) ;
                memset (e - (p - s), ' ', (p - s)) ;
                p = s ;
                s = NULL ;
                }
            else
                if (*p)
                    p++ ;
            
            }
        }
    }

                        
/* ---------------------------------------------------------------------------- */
/* get argument from html tag  */
/* */
/* in  pTag = html tag args  (eg. arg=val arg=val .... >) */
/* in  pArg = name of argument (must be upper case) */
/* */
/* out pLen = length of value */
/* out ret  = pointer to value or NULL if not found */
/* */
/* ---------------------------------------------------------------------------- */

const char * GetHtmlArg (/*in*/  const char *    pTag,
                         /*in*/  const char *    pArg,
                         /*out*/ int *           pLen)

    {
    const char * pVal ;
    const char * pEnd ;
    const char * pName ;
    int l ;

    EPENTRY (GetHtmlArg) ;

    l = strlen (pArg) ;
    while (*pTag)
        {
        *pLen = 0 ;

        while (*pTag && !isalpha (*pTag))
            pTag++ ; 

        pVal = pTag ;
        while (*pVal && !isspace (*pVal) && *pVal != '=' && *pVal != '>')
            pVal++ ;

        while (*pVal && isspace (*pVal))
            pVal++ ;

        if (*pVal == '=')
            {
            pVal++ ;
            while (*pVal && isspace (*pVal))
                pVal++ ;
        
            pEnd = pVal ;
            if (*pVal == '"' || *pVal == '\'')
                {
                char q = *pVal++ ;
                pEnd++ ;
                while (*pEnd && *pEnd != q)
                    pEnd++ ;
                }
            else
                {
                while (*pEnd && !isspace (*pEnd) && *pEnd != '>')
                    pEnd++ ;
                }

            *pLen = pEnd - pVal ;
            }
        else
            pEnd = pVal ;

        
        if (strnicmp (pTag, pArg, l) == 0 && (pTag[l] == '=' || isspace (pTag[l]) || pTag[l] == '>' || pTag[l] == '\0'))
            if (*pLen > 0)
                return pVal ;
            else
                return pTag ;

        pTag = pEnd ;
        }

    *pLen = 0 ;
    return NULL ;
    }


/* ---------------------------------------------------------------------------- */
/* body tag ... */
/* */
/* ---------------------------------------------------------------------------- */


int HtmlBody (/*in*/ const char *   sArg)
    {
    SV * pSV ;

    
    EPENTRY (HtmlBody) ;

    if ((bDebug & dbgLogLink) == 0)
        return ok ;

    oputs (pCurrTag) ;
    if (*sArg != '\0')
        {
        oputc (' ') ;
        oputs (sArg) ;
        }
    oputc ('>') ;
    
    
    pCurrPos = NULL ;
    	
    pSV = perl_get_sv (sLogfileURLName, FALSE) ;
    
    if (pSV)
        oputs (SvPV (pSV, na)) ;

    return ok ;
    }

/*// ---------------------------------------------------------------------------- */
/*// table tag ... */
/*// and various list tags ... (dir, menu, ol, ul) */
/*// */

int HtmlTable (/*in*/ const char *   sArg)
    {
    EPENTRY (HtmlTable) ;

    oputs (pCurrTag) ;
    if (*sArg != '\0')
        {
        oputc (' ') ;
        oputs (sArg) ;
        }
    oputc ('>') ;
    
    pCurrPos = NULL ;
    	
    if (nTableStack > nStackMax - 2)
        return rcStackOverflow ;

    TableStack[nTableStack++] = TableState ;

    memset (&TableState, 0, sizeof (TableState)) ;
    TableState.nResult   = 1 ;
    TableState.nTabMode  = nTabMode ;
    TableState.nMaxRow   = nTabMaxRow ;
    TableState.nMaxCol   = nTabMaxCol ;
    
    if ((TableState.nTabMode & epTabRow) == epTabRowDef)
        State.pBuf = oBegin () ;
    return ok ;
    }


/*// ---------------------------------------------------------------------------- */
/*// /table tag ... (and end of list) */
/*// */

int HtmlEndtable (/*in*/ const char *   sArg)
    {
    EPENTRY (HtmlEndtable) ;

    if (State.nCmdType != cmdTable || State.pCmd -> nCmdNo != pCurrCmd -> nCmdNo)
        {
        strncpy (errdat1, pCurrTag + 1, sizeof (errdat1) - 1) ; 
        if (State.pCmd)
            strcpy (errdat2, State.pCmd -> sCmdName) ; 
        else
            strcpy (errdat2, "NO TAG") ; 
        
        return rcEndtableWithoutTable ;
        }

    if (bDebug & dbgTab)
        lprintf ("[%d]TAB:  nResult=%d nRow=%d Used=%d nCol=%d Used=%d nCnt=%d Used=%d \n",
               nPid, TableState.nTabMode, TableState.nResult, TableState.nRow, TableState.nRowUsed, TableState.nCol, TableState.nColUsed, TableState.nCount, TableState.nCountUsed) ;

    if ((TableState.nTabMode & epTabRow) == epTabRowDef)
        if (TableState.nResult || TableState.nCol > 0)
            oCommit (State.pBuf) ;
        else
            oRollback (State.pBuf) ;

    TableState.nRow++ ;
    if (((TableState.nTabMode & epTabRow) == epTabRowMax ||
         ((TableState.nResult || TableState.nCol > 0) && (TableState.nRowUsed || TableState.nCountUsed) )) &&
          TableState.nRow < TableState.nMaxRow)
        {
        pCurrPos = State.pStart ;        
        if ((TableState.nTabMode & epTabRow) == epTabRowDef)
            State.pBuf = oBegin () ;

        return ok ;
        }

    State.pStart    = NULL ;
    TableState = TableStack[--nTableStack];

    return ok ;
    }

/*// ---------------------------------------------------------------------------- */
/*// tr tag ... */
/*// */

int HtmlRow (/*in*/ const char *   sArg)
    {
    EPENTRY (HtmlRow) ;

    oputs (pCurrTag) ;
    if (*sArg != '\0')
        {
        oputc (' ') ;
        oputs (sArg) ;
        }
    oputc ('>') ;

    pCurrPos = NULL ;
    
    TableState.nResult    = 1 ;
    TableState.nCol       = 0 ; 
    TableState.nColUsed   = 0 ; 

    if ((TableState.nTabMode & epTabCol) == epTabColDef)
        State.pBuf = oBegin () ;
    
    return ok ;
    }

/*// ---------------------------------------------------------------------------- */
/*// /tr tag ... */
/*// */

int HtmlEndrow (/*in*/ const char *   sArg)
    {
    EPENTRY (HtmlEndrow) ;

    
    if (State.nCmdType != cmdTablerow)
        return rcEndtableWithoutTablerow ;

    if (bDebug & dbgTab)
        lprintf ("[%d]TAB:  nTabMode=%d nResult=%d nRow=%d Used=%d nCol=%d Used=%d nCnt=%d Used=%d \n",
               nPid, TableState.nTabMode, TableState.nResult, TableState.nRow, TableState.nRowUsed, TableState.nCol, TableState.nColUsed, TableState.nCount, TableState.nCountUsed) ;

    
    if ((TableState.nTabMode & epTabCol) == epTabColDef)
        if (TableState.nResult || (!TableState.nColUsed && !TableState.nCountUsed && !TableState.nRowUsed))
            oCommit (State.pBuf) ;
        else
            oRollback (State.pBuf), TableState.nCol-- ;

        

    TableState.nCount++ ;
    TableState.nCol++ ;
    if (((TableState.nTabMode & epTabCol) == epTabColMax ||
         (TableState.nResult && (TableState.nColUsed || TableState.nCountUsed)))
        && TableState.nCol < TableState.nMaxCol)
        {
        pCurrPos = State.pStart ;        
        if ((TableState.nTabMode & epTabCol) == epTabColDef)
            State.pBuf = oBegin () ;
        }
    else
        State.pStart    = NULL ;


    return ok ;
    }
                        

                        

/* ---------------------------------------------------------------------------- */
/* input tag ... */
/* */
/* ---------------------------------------------------------------------------- */


int HtmlInput (/*in*/ const char *   sArg)
    {
    const char *  pName ;
    const char *  pVal ;
    const char *  pData ;
    const char *  pType ;
    const char *  pCheck ;
    int           nlen ;
    int           vlen ;
    STRLEN        dlen ;
    int           tlen ;
    int           clen ;
    SV *          pSV ;
    SV **         ppSV ;
    char          sName [256] ;
    int           bCheck ;
    int           bEqual ;
    SV *          pRet ;
    int           rc ;
    char          ArgBuf [2048] ;
    char *        pArgBuf ;


    EPENTRY (HtmlInput) ;

    pName = GetHtmlArg (sArg, "NAME", &nlen) ;
    if (nlen == 0)
        {
        if (bDebug & dbgInput)
            lprintf ("[%d]INPU: has no name\n", nPid) ; 
        if (sArg == ArgBuf)
            { /* write translated values */
            oputs ("<INPUT ") ;
            oputs (sArg) ;
            oputc ('>') ;
            pCurrPos = NULL ;
            }
        return ok ; /* no Name */
        }

    if (nlen >= sizeof (sName))
        nlen = sizeof (sName) - 1 ;
    strncpy (sName, pName, nlen) ;
    sName [nlen] = '\0' ;        

    pType = GetHtmlArg (sArg, "TYPE", &tlen) ;
    if (nlen >= 0 && (strnicmp (pType, "RADIO", 5) == 0 || strnicmp (pType, "CHECKBOX", 8) == 0))
        bCheck = 1 ;
    else    
        bCheck = 0 ;
    
    
    
    pVal = GetHtmlArg (sArg, "VALUE", &vlen) ;
    if (vlen != 0 && bCheck == 0)    
        {
        pSV = newSVpv ((char *)pVal, vlen) ;
        TransHtml (SvPV (pSV, na)) ;

        if (bDebug & dbgInput)
            lprintf ("[%d]INPU: %s already has a value = %s\n", nPid, sName, SvPV (pSV, na)) ; 
        
        if (hv_store (pInputHash, sName, strlen (sName), pSV, 0) == NULL)
            return rcHashError ;
        
        if (sArg == ArgBuf)
            { /* write translated values */
            oputs ("<INPUT ") ;
            oputs (sArg) ;
            oputc ('>') ;
            pCurrPos = NULL ;
            }
        return ok ; /* has already a value */
        }


    ppSV = hv_fetch(pFormHash, (char *)pName, nlen, 0) ;  
    if (ppSV == NULL)
        {
        if (bDebug & dbgInput)
            lprintf ("[%d]INPU: %s: no data available in form data\n", nPid, sName) ; 
        if (sArg == ArgBuf)
            { /* write translated values */
            oputs ("<INPUT ") ;
            oputs (sArg) ;
            oputc ('>') ;
            pCurrPos = NULL ;
            }
        return ok ; /* no data available */
        }

    pData = SvPV (*ppSV, dlen) ;
    

    if (bCheck)
        {
        if (vlen == strlen (pData))
            bEqual = strncmp (pData, pVal, vlen) == 0 ; 
        else
            bEqual = 0 ;
        
        pCheck = GetHtmlArg (sArg, "CHECKED", &clen) ;
        if (pCheck)
            {
            if (bEqual)
                { /* Everything ok */
                if (sArg == ArgBuf)
                    { /* write translated values */
                    oputs ("<INPUT ") ;
                    oputs (sArg) ;
                    oputc ('>') ;
                    pCurrPos = NULL ;
                    }
                }
            else
                { /* Remove "checked" */
                oputs ("<INPUT ") ;
                
                owrite (sArg, pCheck - sArg, 1) ;
    
                oputs (pCheck + 7) ; /* write rest of html tag */
                oputc ('>') ;

                pCurrPos = NULL ; /* nothing more left of html tag */
                }
            }
        else
            {
            if (bEqual)
                { /* Insert "checked" */
                oputs ("<INPUT ") ;
                oputs (sArg) ;
                oputs (" CHECKED>") ;
    
                pCurrPos = NULL ; /* nothing more left of html tag */
                }
            else
                {
                if (sArg == ArgBuf)
                    { /* write translated values */
                    oputs ("<INPUT ") ;
                    oputs (sArg) ;
                    oputc ('>') ;
                    pCurrPos = NULL ;
                    }
                }
            }
        }
    else if (pVal)
        {
        oputs ("<INPUT ") ;

        owrite (sArg, pVal - sArg, 1) ;

        oputs (" VALUE=\"") ;
        OutputToHtml (pData) ;
        oputs ("\" ") ;

        while (*pVal && !isspace(*pVal))
            pVal++ ;
        
        oputs (pVal) ; /* write rest of html tag */
        oputc ('>') ;

        pCurrPos = NULL ; /* nothing more left of html tag */
        }
    else
        {
        oputs ("<INPUT ") ;
        oputs (sArg) ;
        oputs (" VALUE=\"") ;
        OutputToHtml (pData) ;
        oputs ("\">") ;
    
        pCurrPos = NULL ; /* nothing more left of html tag */
        }


    if (bDebug & dbgInput)
        {
        lprintf ("[%d]INPU: %s=%s %s\n", nPid, sName, pData, bCheck?(bEqual?"CHECKED":"NOT CHECKED"):"") ; 
        }
    
    pSV = newSVpv ((char *)pData, dlen) ;
    if (hv_store (pInputHash, sName, strlen (sName), pSV, 0) == NULL)
        return rcHashError ;

    return ok ;
    }

/* ---------------------------------------------------------------------------- */
/* textarea tag ... */
/* */
/* ---------------------------------------------------------------------------- */


int HtmlTextarea (/*in*/ const char *   sArg)
    {
    EPENTRY (HtmlTextarea) ;

    return ok ;
    }

    
/* ---------------------------------------------------------------------------- */
/* /textarea tag ... */
/* */
/* ---------------------------------------------------------------------------- */


int HtmlEndtextarea (/*in*/ const char *   sArg)
    {
    const char *  pName ;
    const char *  pVal ;
    const char *  pEnd ;
    const char *  pData ;
    int           nlen ;
    int           vlen ;
    STRLEN        dlen ;
    int           clen ;
    SV *          pSV ;
    SV **         ppSV ;
    char          sName [256] ;


    EPENTRY (HtmlEndtextarea) ;
    
    
    pVal = State.pStart ;

    State.pStart = NULL ;

    if (State.nCmdType != cmdTextarea)
        return rcEndtextareaWithoutTextarea ;


    pName = GetHtmlArg (State.sArg, "NAME", &nlen) ;
    if (nlen == 0)
        {
        if (bDebug & dbgInput)
            lprintf ("[%d]TEXT: has no name\n", nPid) ; 
        return ok ; /* no Name */
        }

    if (nlen >= sizeof (sName))
        nlen = sizeof (sName) - 1 ;
    strncpy (sName, pName, nlen) ;
    sName [nlen] = '\0' ;        


    pEnd = pCurrTag - 1 ;
    while (pVal <= pEnd && isspace (*pVal))
        pVal++ ;

    while (pVal <= pEnd && isspace (*pEnd))
        pEnd-- ;

    vlen = pEnd - pVal + 1 ;

    if (vlen != 0)    
        {
        pSV = newSVpv ((char *)pVal, vlen) ;
        TransHtml (SvPV (pSV, na)) ;

        if (bDebug & dbgInput)
            lprintf ("[%d]TEXT: %s already has a value = %s\n", nPid, sName, SvPV (pSV, na)) ; 
        
        if (hv_store (pInputHash, sName, strlen (sName), pSV, 0) == NULL)
            return rcHashError ;
        
        return ok ; /* has already a value */
        }


    ppSV = hv_fetch(pFormHash, (char *)pName, nlen, 0) ;  
    if (ppSV == NULL)
        {
        if (bDebug & dbgInput)
            lprintf ("[%d]TEXT: %s: no data available in form data\n", nPid, sName) ; 
        return ok ; /* no data available */
        }

    pData = SvPV (*ppSV, dlen) ;
    
    if (pVal)
        {
        OutputToHtml (pData) ;
        }

    if (bDebug & dbgInput)
        {
        lprintf ("[%d]TEXT: %s=%s\n", nPid, sName, pData) ; 
        }
    
    pSV = newSVpv ((char *)pData, dlen) ;
    if (hv_store (pInputHash, sName, strlen (sName), pSV, 0) == NULL)
        return rcHashError ;

    return ok ;
    }

    
    
/*//////////////////////////////////////////////////////////////////////////////////////// */
/*  */
/* Get a Value out of a perl hash */
/* */


char * GetHashValueLen (/*in*/  HV *           pHash,
                        /*in*/  const char *   sKey,
                        /*in*/  int            nLen,
                        /*in*/  int            nMaxLen,
                        /*out*/ char *         sValue)

    {
    SV **   ppSV ;
    char *  p ;
    STRLEN  len ;        

    EPENTRY (GetHashValueLen) ;

    ppSV = hv_fetch(pHash, (char *)sKey, nLen, 0) ;  
    if (ppSV != NULL)
        {
        p = SvPV (*ppSV ,len) ;
        if (len >= nMaxLen)
            len = nMaxLen - 1 ;        
        strncpy (sValue, p, len) ;
        }
    else
        len = 0 ;

    sValue[len] = '\0' ;
        
    return sValue ;
    }


char * GetHashValue (/*in*/  HV *           pHash,
                     /*in*/  const char *   sKey,
                     /*in*/  int            nMaxLen,
                     /*out*/ char *         sValue)
    {
    return GetHashValueLen (pHash, sKey, strlen (sKey), nMaxLen, sValue) ;
    }



/*// ---------------------------------------------------------------------------- */
/*// read form input from http server... */
/*// */

static int GetFormData (/*in*/ char * pQueryString,
                        /*in*/ int    nLen)

    {
    int     num ;
    char *  p ;
    char *  pMem ;
    int     nVal ;
    int     nKey ;
    char *  pKey ;
    char *  pVal ;
    SV *    pSVV ;
    SV *    pSVK ;
    SV * *  ppSV ;

    EPENTRY (GetFormData) ;

    hv_clear (pFormHash) ;
    

    if (nLen == 0)
        return ok ;
    
    if ((pMem = _malloc (nLen + 4)) == NULL)
        return rcOutOfMemory ;

    p = pMem ;


    nKey = nVal = 0 ;
    pKey = pVal = p ;
    while (1)
        {
        switch (*pQueryString)
            {
            case '+':
                pQueryString++ ;
                *p++ = ' ' ;
                break ;
            
            case '%':
                pQueryString++ ;
                num = 0 ;
                if (*pQueryString)
                    {
                    if (toupper (*pQueryString) >= 'A')
                        num += (toupper (*pQueryString) - 'A' + 10) << 4 ;
                    else
                        num += (toupper (*pQueryString) - '0') << 4 ;
                    pQueryString++ ;
                    }
                if (*pQueryString)
                    {
                    if (toupper (*pQueryString) >= 'A')
                        num += (toupper (*pQueryString) - 'A' + 10) ;
                    else
                        num += (toupper (*pQueryString) - '0') ;
                    pQueryString++ ;
                    }
                *p++ = num ;
                break ;
            case '=':
                nKey = p - pKey ;
                *p++ = '\0' ;
                nVal = 0 ;
                pVal = p ;
                pQueryString++ ;
                break ;
            case '&':
                pQueryString++ ;
            case '\0':
                nVal = p - pVal ;
                *p++ = '\0' ;
            
                if (nVal > 0)
                    {
                    if (ppSV = hv_fetch (pFormHash, pKey, nKey, 0))
                        { /* Field exists already -> append separator and field value */
                        sv_catpvn (*ppSV, &cMultFieldSep, 1) ;
                        sv_catpvn (*ppSV, pVal, nVal) ;
                        }
                    else
                        { /* New Field -> store it */
                        pSVV = newSVpv (pVal, nVal) ;

                        if (hv_store (pFormHash, pKey, nKey, pSVV, 0) == NULL)
                            {
                            _free (pMem) ;
                            return rcHashError ;
                            }

                        }

                    pSVK = newSVpv (pKey, nKey) ;

                    av_push (pFormArray, pSVK) ;
                
                    if (bDebug & dbgForm)
                        lprintf ("[%d]FORM: %s=%s\n", nPid, pKey, pVal) ; 

                    }
                pKey = pVal = p ;
                nKey = nVal = 0 ;
                
                if (*pQueryString == '\0')
                    {
                    _free (pMem) ;
                    return ok ;
                    }
                
                
                break ;
            default:
                *p++ = *pQueryString++ ;
                break ;
            }
        }

    }

/*// ---------------------------------------------------------------------------- */
/*// read input from cgi process... */
/*// */


static int GetInputData_CGIProcess ()

    {
    char *  p ;
    int     rc = ok ;
    int  state = 0 ;
    int  len   = 0 ;
    char sLine [1024] ;
    SV * pSVE ;
    
    EPENTRY (GetInputData_CGIProcess) ;

    hv_clear (pEnvHash) ;


    if (bDebug)
        lprintf ("\n[%d]Waiting for Request... SVs: %d OBJs: %d\n", nPid, sv_count, sv_objcount) ;

    if ((rc = OpenInput (sCmdFifo)) != ok)
        {
        return rc ;
        }


    if (bDebug)
        lprintf ("[%d]Processing Request...\n", nPid) ;
    
    while (igets (sLine, sizeof (sLine)))
        {
        len = strlen (sLine) ; 
        while (len >= 0 && isspace (sLine [--len]))
            ;
        sLine [len + 1] = '\0' ;
        

        if (strcmp (sLine, "----") == 0)
            { state = 1 ; if (bDebug) lprintf ("[%d]Environement...\n", nPid) ;}
        else if (strcmp (sLine, "****") == 0)
            { state = 2 ;  if (bDebug) lprintf ( "[%d]Formdata...\n", nPid) ;}
        else if (state == 1)
            {
            p = strchr (sLine, '=') ;
            *p = '\0' ;
            p++ ;

            pSVE = newSVpv (p, strlen (p)) ;

            if (hv_store (pEnvHash, sLine, strlen (sLine), pSVE, 0) == NULL)
                {
                return rcHashError ;
                }
            if (bDebug & dbgEnv)
                lprintf ( "[%d]ENV:  %s=%s\n", nPid, sLine, p) ;
            }
        else if (state == 2)
            {
            len = atoi (sLine) ;
            if ((p = _malloc (len + 1)) == NULL)
                return rcOutOfMemory ;
            iread (p, len, 1) ;
            p[len] = '\0' ;
            rc = GetFormData (p, len) ;
            _free (p) ;
            break ;
            }
        else
            { if (bDebug) lprintf ("[%d]Unknown Input: %s\n", nPid, sLine) ;}

        }
        
    CloseInput () ;
    
    return rc ;
    }
                        

/*// ---------------------------------------------------------------------------- */
/*// get form data when running as cgi script... */
/*// */


static int GetInputData_CGIScript ()

    {
    char *  p ;
    char *  f ;
    int     rc = ok ;
    int     len   = 0 ;
    char    sQuery [2048] ;
    char    sLen [20] ;
    

    EPENTRY (GetInputData_CGIScript) ;

    if (bDebug & dbgEnv)
        {
        SV *   psv ;
        HE *   pEntry ;
        char * pKey ;
        I32    l ;
        
        hv_iterinit (pEnvHash) ;
        while (pEntry = hv_iternext (pEnvHash))
            {
            pKey = hv_iterkey (pEntry, &l) ;
            psv  = hv_iterval (pEnvHash, pEntry) ;

                lprintf ( "[%d]ENV:  %s=%s\n", nPid, pKey, SvPV (psv, na)) ; 
            }
        }

    sLen [0] = '\0' ;
    GetHashValue (pEnvHash, "CONTENT_LENGTH", sizeof (sLen) - 1, sLen) ;

    if ((len = atoi (sLen)) == 0)
        {
        GetHashValue (pEnvHash, "QUERY_STRING", sizeof (sQuery) - 1, sQuery) ;
        p = sQuery ;
        len = strlen (sQuery) ;
        f = NULL ;
        }
    else
        {
        if ((p = _malloc (len + 1)) == NULL)
            return rcOutOfMemory ;

        if ((rc = OpenInput (NULL)) != ok)
            {
            _free (p) ;
            return rc ;
            }
        iread (p, len, 1) ;
        CloseInput () ;
        
        p[len] = '\0' ;
        f = p ;
        }
        
    if (bDebug)
        lprintf ( "[%d]Formdata... length = %d\n", nPid, len) ;    

    rc = GetFormData (p, len) ;
    
    if (f)
        _free (f) ;
        
    
    return rc ;
    }


/* ---------------------------------------------------------------------------- */
/* scan commands and evals ([x ... x] sequenz) ... */
/* */
/* p points to '[' */
/* */


    
static int ScanCmdEvals (/*in*/ char *   p)
    
    
    { 
    int     rc ;
    int     len ;
    int     n ;
    char *  c ;
    char *  a ;
    char    as ;
    char    nType ;
    SV *    pRet ;
    struct tCmd * pCmd ;


    EPENTRY (ScanCmdEvals) ;
    
    p++ ;

    pCurrPos = p ;

    if ((nType = *p++) == '\0')
        return ok ;

    pCurrPos = p ;

    if (nType != '+' && nType != '-' && nType != '$' )
        { /* escape (for [[ -> [) */
        if (State.bProcessCmds == cmdAll)
            {
            if (nType != '[') 
                oputc ('[') ;
            oputc (nType) ;
            }
        return ok ;
        }


    do
        { /* search end  */
        p++ ;
        if ((p = strchr (p, ']')) == NULL)
            break ;
        }   
    while (p[-1] != nType) ;
    if (p == NULL)
        { /* end not found */
        sprintf (errdat1, "%c]", nType) ; 
        return rcMissingRight ;
        }
    p [-1] = '\0' ;
    p++ ;

    /* strip off all <HTML> Tags */

    TransHtml (pCurrPos) ;

    switch (nType)
        {
        case '+':
            if (State.bProcessCmds == cmdAll)
                {
                Eval (pCurrPos, &pRet) ;
        
                OutputToHtml (SvPV (pRet, na)) ;
                SvREFCNT_dec (pRet) ;
                }

            p [-2] = nType ;
            pCurrPos = p ;

        
            break ;
        case '-':
            if (State.bProcessCmds == cmdAll)
                {
                Eval (pCurrPos, &pRet) ;
                SvREFCNT_dec (pRet) ;
                }

            p [-2] = nType ;
            pCurrPos = p ;

            break ;
        case '$':
            while (*pCurrPos != '\0' && isspace (*pCurrPos))
                    pCurrPos++ ;

            a = c = pCurrPos ;
            while (*a != '\0' && !isspace (*a))
                a++ ;

            as = '\0' ;
            if (*a != '\0')
                {
                as = *a ;
                *a = '\0' ;
                a++ ;
                }

            pCurrPos = p ;

            if ((rc = SearchCmd (c, a, FALSE, &pCmd)) != ok)
                return rc ;
        
        
            if ((rc = ProcessCmd (pCmd, a)) != ok)
                return rc ;
        
            if (as != '\0')
                a [-1] = as ;
            p [-2] = nType ;

            break ;
        }

    return ok ;
    }

    
/* ---------------------------------------------------------------------------- */
/* process commands and evals in a string ... */
/* */
/* pIn   points to the string to process */
/* pOut  pointer to a pointer to a buffer for the output, maybe point to pIn at */
/*       exit if nothing to do or the buffer is filled with processed output */
/* nSize size of outputbuffer */
/* */


    
static int ScanCmdEvalsInString (/*in*/  char *   pIn,
                                 /*out*/ char * * pOut,
                                 /*in*/  size_t   nSize)
    
    
    { 
    int    rc ;
    char * pSaveCurrPos  ;
    char * pSaveCurrStart ;
    char * pSaveEndPos ;
    char * p = strchr (pIn, '[');    


    EPENTRY (ScanCmdEvalsInString) ;

    if (p == NULL)
        {
        lprintf ("SCEV nothing sArg = %s\n", pIn) ; 
        *pOut = pIn ; /* Nothing to do */
        return ok ;
        }
    lprintf ("SCEV sArg = %s, p = %s\n", pIn, p) ; 

    /* save global vars */
    pSaveCurrPos   = pCurrPos ;
    pSaveCurrStart = pCurrStart ;
    pSaveEndPos    = pEndPos ;
    
    pCurrPos = pIn ;
    pEndPos  = pIn + strlen (pIn) ;

    OutputToMemBuf (*pOut, nSize) ;

    rc = ok ;
    while (pCurrPos < pEndPos && rc == ok)
        {
        /* */
        /* execute [x ... x] and replace them if nessecary */
        /* */
        if (p == NULL || *p == '\0')
            { /* output the rest of html */
            owrite (pCurrPos, 1, pEndPos - pCurrPos) ;
            break ;
            }
        
        if (State.bProcessCmds == cmdAll)
            /* output until next cmd */
            owrite (pCurrPos, 1, p - pCurrPos) ;
        
        if (bDebug & dbgSource)
            {
            char * s = p ;
            char * n ;

            while (*s && isspace (*s))
                s++ ;
            
            if (*s)
                {
                n = strchr (s, '\n') ;
                if (n)
                    lprintf ("[%d]SRC: %*.*s\n", nPid, n-s, n-s, s) ;
                else
                    lprintf ("[%d]SRC: %70.70s\n", nPid, s) ;

                }
            }        

        
        pCurrStart = p ;
        rc = ScanCmdEvals (p) ;

        p = strchr (pCurrPos, '[') ;
        }
    
    OutputToStd () ;
    
    pCurrPos   = pSaveCurrPos ;
    pCurrStart = pSaveCurrStart ;
    pEndPos    = pSaveEndPos ;
    
    return ok ;
    }
            
/* ---------------------------------------------------------------------------- */
/* scan html tag ... */
/* */
/* p points to '<' */
/* */

static int ScanHtmlTag (/*in*/ char *   p)

    { 
    int  rc ;
    char ec ;
    char ea ;
    char * pec ;
    char * pea ;
    char * pCmd ;
    char * pArg ;
    char   ArgBuf [2048] ;
    char * pArgBuf = ArgBuf ;
    struct tCmd * pCmdInfo ;



    EPENTRY (ScanHtmlTag) ;
    
    
    pCurrTag = p ;     /* save start of html tag */

    /* skip space */
    p++ ;
    while (*p != '\0' && isspace (*p))
            p++ ;
    
    pCmd = p ;               /* start of tag name */
    while (*p != '\0' && !isspace (*p) && *p != '>')
        p++ ;

    ec = *p ;              /* save first char after tag name */
    pec = p ;
    *p++ = '\0' ;          /* set end of tag name to \0 */

    if ((rc = SearchCmd (pCmd, "", TRUE, &pCmdInfo)) != ok)
        {
        *pec = ec ;
        oputc (*pCurrTag) ;
        pCurrPos = pCurrTag + 1 ;  
        if (rc == rcCmdNotFound)
            return ok ;    /* ignore this html tag */
        return rc ;
        }


    /* look if there are any arguments */    
    
    pArg = p ;             /* start of arguments */
    if (ec == '>')
        { /* No Arguments */
        pArg = p - 1 ;
        pea = NULL ;
        }
    else
        {
        p = strchr (p, '>') ; /* get end of tag */

        if (p)
            {
            ea = *p ;
            pea = p ;
            *p = '\0' ;            /* set end of tag arguments to \0 */
            p++ ;
            }
        else
            {
            p = pArg + strlen (pArg) ;
            pea = NULL ;
            }
        }

    pCurrPos = p ;    /* pCurrPos = first char after whole tag */

    
    if (*pArg != '\0' && pCmdInfo -> bScanArg)
    	{
        if ((rc = ScanCmdEvalsInString ((char *)pArg, &pArgBuf, sizeof (ArgBuf))) != ok)
            return rc ;
    	}
    else
    	pArgBuf = pArg ;
    
    
    /* see if knwon html tag and execute */

    if ((rc = ProcessCmd (pCmdInfo, pArgBuf)) != ok)
        {
        if (rc == rcCmdNotFound)
            {
              /* only write html tag start char and */
            /*p = pCurrPos = pCurrTag + 1 ;   */    /* check if more to exceute within html tag */
            }
        else
            return rc ;
        }


    if (p == pCurrPos && pCurrPos) /* if CurrPos didn't change write out html tag as it is */
        {
        if (pArg == pArgBuf)
            { /* write unmodified tag */    
            *pec = ec ;              /* restore first char after tag name */
            if (pea)
                *pea = ea ;              /* restore first char after tag arguments */

            oputc (*pCurrTag) ;
            pCurrPos = pCurrTag + 1 ;
            }
        else
            { /* write tag with interpreted args */
            oputs (pCurrTag) ;
            oputc (' ') ;
            oputs (pArgBuf) ;
            oputc ('>') ;
            *pec = ec ;              /* restore first char after tag name */
            if (pea)
                *pea = ea ;              /* restore first char after tag arguments */

            }
        }
    else
        {
        *pec = ec ;              /* restore first char after tag name */
        if (pea)
            *pea = ea ;              /* restore first char after tag arguments */
        }

    if (pCurrPos == NULL)
        pCurrPos = p ; /* html tag is written by command handler */

    pCurrTag = NULL ;

    return ok ;    
    }

    
/* ---------------------------------------------------------------------------- */
/* add magic to integer var */
/* */
/* in  sVarName = Name of varibale */
/* in  pVirtTab = pointer to virtual table */
/* */
/* ---------------------------------------------------------------------------- */

static int AddMagic (/*in*/ char *     sVarName,
                     /*in*/ MGVTBL *   pVirtTab) 

    {
    SV * pSV ;
    struct magic * pMagic ;

    EPENTRY (AddMagic) ;

    
    pSV = perl_get_sv (sVarName, TRUE) ;
    sv_magic (pSV, NULL, 0, sVarName, strlen (sVarName)) ;
    pMagic = mg_find (pSV, 0) ;

    if (pMagic)
        pMagic -> mg_virtual = pVirtTab ;
    else
        {
        LogError ( rcMagicError) ;
        return 1 ;
        }


    return ok ;
    }


/* ---------------------------------------------------------------------------- */
/* init embperl module */
/* */
/* in  nIOType = type of requested i/o */
/* */
/* ---------------------------------------------------------------------------- */

int iembperl_init (/*in*/ int     _nIOType,
                   /*in*/ const char *  sLogFile) 

    {
    int     len ;
    char *  p ;
    int     n ;
    char *  c ;
    char *  a ;
    SV **   ppSV ;
    int     rc ;
    struct stat st ;
    char    nType ;
    SV *    pRet ;
    SV *    pSVMode  = newSViv (0) ;


    nIOType = _nIOType ;

#ifdef APACHE
    pReq = NULL ;
#endif

    nPid = getpid () ;


    if ((rc = OpenLog (sLogFile)) != ok)
        { 
        bDebug = 0 ; /* Turn debbuging of, only errors will go to stderr */
        LogError (rc) ;
        }

    EPENTRY (iembperl_init) ;

    if (bDebug)
        {
        char * p ;

        switch (nIOType)
            {
        #ifdef APACHE
            case epIOMod_Perl: p = "mod_perl"; break ;
        #else
            case epIOMod_Perl: p = "mod_perl UNSUPPORTED"; break ;
        #endif
            case epIOPerl:     p = "Offline"; break ;
            case epIOCGI:      p = "CGI-Script"; break ;
            case epIOProcess:  p = "Demon"; break ;
            default: p = "unknown" ; break ;
            }
        
        lprintf ("[%d]INIT: Embperl %s starting... mode = %s (%d)\n", nPid, sVersion, p, nIOType) ;
        }


#ifndef APACHE
    if (nIOType == epIOMod_Perl)
        {
        LogError (rcNotCompiledForModPerl) ;
        return 1 ;
        }
#endif


    if ((pEvalErr = perl_get_sv (sEvalErrName, FALSE)) == NULL)
        {
        LogError (rcPerlVarError) ;
        return 1 ;
        }

    
    if ((pFormHash = perl_get_hv (sFormHashName, TRUE)) == NULL)
        {
        LogError (rcHashError) ;
        return 1 ;
        }


    if ((pFormArray = perl_get_av (sFormArrayName, TRUE)) == NULL)
        {
        LogError (rcArrayError) ;
        return 1 ;
        }

    if ((pInputHash = perl_get_hv (sInputHashName, TRUE)) == NULL)
        {
        LogError ( rcHashError) ;
        return 1 ;
        }



    if ((pEnvHash = perl_get_hv (sEnvHashName, TRUE)) == NULL)
        {
        LogError ( rcHashError) ;
        return 1 ;
        }

    if ((pNameSpaceHash = perl_get_hv (sNameSpaceHashName, TRUE)) == NULL)
        {
        LogError ( rcHashError) ;
        return 1 ;
        }
    
    rc = AddMagic (sTabCountName, &EMBPERL_mvtTabCount) ;
    if (rc == 0)
        rc = AddMagic (sTabRowName, &EMBPERL_mvtTabRow) ;
    if (rc == 0)
        rc = AddMagic (sTabColName, &EMBPERL_mvtTabCol) ;
    if (rc == 0)
        rc = AddMagic (sTabMaxRowName, &EMBPERL_mvtTabMaxRow) ;
    if (rc == 0)
        rc = AddMagic (sTabMaxColName, &EMBPERL_mvtTabMaxCol) ;
    if (rc == 0)
        rc = AddMagic (sTabModeName, &EMBPERL_mvtTabTabMode) ;

    return rc ;
    }

/* ---------------------------------------------------------------------------- */
/* clean up embperl module */
/* */
/* ---------------------------------------------------------------------------- */

int iembperl_term (void) 


    {

    EPENTRY (iembperl_term) ;
    
    CloseLog () ;
    CloseOutput () ;
    
    return ok ;
    }


int iembperl_setreqrec  (/*in*/ SV *   pReqSV)
    {
#ifdef APACHE
    if (pReq)
        {
        LogError (rcCannotUsedRecursive);            
        return rcCannotUsedRecursive ;            
        }    
    
    pReq = (request_rec *)SvIV((SV*)SvRV(pReqSV));
    bDebug = 0 ; /* set it to nothing for output of logfiles */
#endif

    return ok ;
    }



int iembperl_resetreqrec  ()
    {
#ifdef APACHE
    pReq = NULL ;
#endif

    return ok ;
    }









int iembperl_req  (/*in*/ char *  sInputfile,
                   /*in*/ char *  sOutputfile,
                   /*in*/ int     bDebugFlags,
                   /*in*/ char *  pNameSpaceName,
                   /*in*/ int     nFileSize) 




    {
    clock_t startclock = clock () ;
    int     rc ;
    int     len ;
    char *  p ;
    int     n ;
    char *  c ;
    char *  a ;
    SV **   ppSV ;
    char *  pBuf = NULL ;
    struct stat st ;
    char    nType ;
    SV *    pRet ;
    int     ifd ;
    I32     stsv_count = sv_count ;
    I32     stsv_objcount = sv_objcount ;
    I32     lstsv_count = sv_count ;
    I32     lstsv_objcount = sv_objcount ;


    EPENTRY (iembperl_req) ;

   
    if (bDebug)
        {
        time_t t ;
        struct tm * tm ;
        time (&t) ;        
        tm =localtime (&t) ;
        lprintf ("[%d]REQ:  starting... %s\n", nPid, asctime(tm)) ;
        }
    
    tainted = 0 ;


    if (pNameSpaceName && *pNameSpaceName != '\0')
        {
    	SV * * ppSV = hv_fetch(pNameSpaceHash, pNameSpaceName, strlen (pNameSpaceName), 0) ;  
    	if (ppSV == NULL)
            {
            LogError (rcUnknownNameSpace) ;
#ifdef APACHE
            pReq = NULL ;
#endif
            return rcUnknownNameSpace ;
            }
	pNameSpace = * ppSV ;
        bSafeEval = TRUE ;
        }
    else
        {
        pNameSpace = NULL ;
        bSafeEval = FALSE ;
        }
        
    bDebug     = bDebugFlags ;

    if (bDebug)
        {
        char * p ;

        switch (nIOType)
            {
            case epIOMod_Perl: p = "mod_perl"; break ;
            case epIOPerl:     p = "Offline"; break ;
            case epIOCGI:      p = "CGI-Script"; break ;
            case epIOProcess:  p = "Demon"; break ;
            default: p = "unknown" ; break ;
            }
        
        lprintf ("[%d]REQ:  %s %s", nPid, (bSafeEval)?"Namespace = ":"No Safe Eval", pNameSpaceName?pNameSpaceName:"") ;
        lprintf (" mode = %s (%d)\n", p, nIOType) ;
        }
    nStack      = 0 ;
    nTableStack = 0 ;
    pArgStack = ArgStack ;

    memset (&State, 0, sizeof (State)) ;
    memset (&TableState, 0, sizeof (TableState)) ;
    
    State.nCmdType      = cmdNorm ;
    State.bProcessCmds  = cmdAll ;
    State.sArg          = strcpy (pArgStack, "") ;
    pArgStack += 1 ;
    nTabMode    = epTabRowDef | epTabColDef ;
    nTabMaxRow  = 100 ;
    nTabMaxCol  = 10 ;
    pCurrTag    = NULL ;

    /* */
    /* read data from cgi script */
    /* */

    rc = ok ;
    switch (nIOType)
        {
        case epIOPerl:
        case epIOCGI:
        case epIOMod_Perl:
            rc = GetInputData_CGIScript () ;
            break ;
        case epIOProcess:
            rc = GetInputData_CGIProcess () ;
            break ;
        }

            
    if (rc != ok)
        {
        LogError (rc);
#ifdef APACHE
        pReq = NULL ;
#endif
        return rc ;
        }
    
    /*
    sRetFifo [0] = '\0' ;
    if (nIOType == epIOProcess)
        {
        GetHashValue (pEnvHash, sRetFifoName, sizeof (sRetFifo), sRetFifo) ;

        if (sRetFifo [0] == '\0')
            {
            LogError (rcNoRetFifo) ;
#ifdef APACHE
        pReq = NULL ;
#endif
            return rcNoRetFifo ;
            }
        }
    */


    if ((rc = OpenOutput (sOutputfile)) != ok)
        {
        LogError (rc) ;
#ifdef APACHE
        pReq = NULL ;
#endif
        return rc ;
        }

#ifdef APACHE
    if (pReq == NULL)
#endif
        if (nIOType != epIOPerl)
            oputs ("Content-type: text/html\n\n") ;




    /* Read HTML file */

    if ((rc = ReadHTML (sInputfile, nFileSize, &pBuf)) != ok)
        {
        LogError (rc) ;
#ifdef APACHE
        pReq = NULL ;
#endif
        return rc ;
        }




    /* */
    /* Datei bearbeiten... */
    /* */

    pCurrPos = pBuf ;
    pEndPos  = pBuf + nFileSize ;

    rc = ok ;
    while (pCurrPos < pEndPos && rc == ok)
        {
        if ((bDebug & dbgMem) && (sv_count != lstsv_count || sv_objcount != lstsv_objcount))
            {
            lprintf ("[%d]SVs:  Entry-SVs: %d -OBJs: %d Curr-SVs: %d -OBJs: %d\n", nPid, stsv_count, stsv_objcount, sv_count, sv_objcount) ;
            lstsv_count = sv_count ;
            lstsv_objcount = sv_objcount ;
            }
        
        
        /* */
        /* execute [x ... x] and special html tags and replace them if nessecary */
        /* */

        if (State.bProcessCmds == cmdAll)
            {
            n = strcspn (pCurrPos, "[<") ;
            p = pCurrPos + n ;
            }
        else
            p = strchr (pCurrPos, '[') ;
            
            
        if (p == NULL || *p == '\0')
            { /* output the rest of html */
            owrite (pCurrPos, 1, pEndPos - pCurrPos) ;
            break ;
            }
        
        if (State.bProcessCmds == cmdAll)
            /* output until next cmd */
            owrite (pCurrPos, 1, p - pCurrPos) ;
        
        if (bDebug & dbgSource)
            {
            char * s = p ;
            char * n ;

            while (*s && isspace (*s))
                s++ ;
            
            if (*s)
                {
                n = strchr (s, '\n') ;
                if (n)
                    lprintf ("[%d]SRC: %*.*s\n", nPid, n-s, n-s, s) ;
                else
                    lprintf ("[%d]SRC: %70.70s\n", nPid, s) ;

                }
            }        

        
        pCurrStart = p ;
        if (*p == '<')
            { /* HTML Tag */
            rc = ScanHtmlTag (p) ;
            }
         else
            { /* [x ... x] sequenz */
            rc = ScanCmdEvals (p) ;
            }
        }
        
    
    oputs ("\r\n") ;
    CloseOutput () ;

    if (rc != ok)
        {
        LogError (rc) ;
        }

    hv_clear (pEnvHash) ;
    hv_clear (pFormHash) ;
    av_clear (pFormArray) ;
    hv_clear (pInputHash) ;

    if (bDebug)
        {
        clock_t cl = clock () ;
        time_t t ;
        struct tm * tm ;
        time (&t) ;        
        tm =localtime (&t) ;
        
#ifdef CLOCKS_PER_SEC
        lprintf ("[%d]Request finished. %s. Entry-SVs: %d -OBJs: %d Exit-SVs: %d -OBJs: %d  Time: %d ms (cps %d)\n", nPid, asctime(tm), stsv_count, stsv_objcount, sv_count, sv_objcount, ((cl - startclock) * 1000 / CLOCKS_PER_SEC), CLOCKS_PER_SEC) ;
#else
        lprintf ("[%d]Request finished. %s. Entry-SVs: %d -OBJs: %d Exit-SVs: %d -OBJs: %d\n", nPid, asctime(tm), stsv_count, stsv_objcount, sv_count, sv_objcount) ;
#endif        
        }

    FlushLog () ;



    if (pBuf)
        _free (pBuf) ;

#ifdef APACHE
    /* This must be the very very very last !!!!! */
    pReq = NULL ;
#endif

    return ok ;
    }

