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

// Macros

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif


// Apache Request Record

#ifdef APACHE
request_rec * pReq = NULL ;
#endif


//

int  bDebug = dbgAll & ~dbgMem & ~dbgEnv;

int  bSafeEval = 0 ;

int  nIOType   = epIOPerl ;

char sCmdFifo [] = "/local/www/cgi-bin/embperl/cmd.fifo" ;
char sRetFifo [1024] ;

char sRetFifoName [] = "__RETFIFO" ;

char sEnvHashName   [] = "ENV" ;
char sFormHashName  [] = "HTML::Embperl::fdat" ;
char sFormArrayName [] = "HTML::Embperl::ffld" ;
char sInputHashName [] = "HTML::Embperl::idat" ;
char sTabCountName  [] = "HTML::Embperl::cnt" ;
char sTabRowName    [] = "HTML::Embperl::row" ;
char sTabColName    [] = "HTML::Embperl::col" ;
char sTabMaxRowName [] = "HTML::Embperl::maxrow" ;
char sTabMaxColName [] = "HTML::Embperl::maxcol" ;
char sTabModeName   [] = "HTML::Embperl::tabmode" ;


HV *    pEnvHash ;   // environement from CGI Script
HV *    pFormHash ;  // Formular data
HV *    pInputHash ; // Data of input fields
AV *    pFormArray ; // Fieldnames


SV *    pNameSpace ; // Currently active Namespace


char * pCurrPos ;    // Current position in html file
char * pCurrStart ;  // Current start position of html tag / eval expression
char * pEndPos ;     // end of html file
char * pCurrTag ;    // Current start position of html tag

//
// Additional Error info
//


char errdat1 [1024]  ;


//
// Commandtypes
//

enum tCmdType
    {
    cmdNorm = 1,
    cmdIf   = 2,
    cmdEndif = 4,
    cmdWhile = 8,
    cmdTable = 16,
    cmdTablerow = 32,
    cmdList  = 64,
    cmdTextarea  = 128,

    cmdAll   = 255
    } ;


//
// Stack
//

struct tStackEntry
    {
    enum tCmdType   nCmdType ;      // Type of the command which the pushed the entry on the stack
    char *          pStart ;        // Startposition fpr loops
    long            bProcessCmds ;  // Process corresponding cmds
    int             nResult ;       // Result of Command which starts the block
    int             nCount ;        // Count for tables, lists etc
    int             nCountUsed ;    // Count for tables, lists is used in Table 
    int             nRow ;          // Row Count for tables, lists etc
    int             nRowUsed ;      // Row Count for tables, lists is used in Table 
    int             nCol ;          // Column Count for tables, lists etc
    int             nColUsed ;      // Column Count for tables, lists is used in Table 
    int             nMaxRow ;       // maximum rows
    int             nMaxCol ;       // maximum columns
    int             nTabMode ;      // table mode
    char *          sArg ;          // Argument of Command which starts the block
    struct tBuf *   pBuf ;          // Output buf for table rollback         
    } ;

#define nStackMax 100           // Max level of nesting

struct tStackEntry Stack [nStackMax] ; // Stack for if, while, etc.

int nStack = 0 ;                // Stackpointer

struct tStackEntry State ;             // current State

char ArgStack [16384] ;

char * pArgStack = ArgStack ;

int nTabMode    ;    // mode for next table (only takes affect after next <TABLE>
int nTabMaxRow  ;    // maximum rows for next table (only takes affect after next <TABLE>
int nTabMaxCol  ;    // maximum columns for next table (only takes affect after next <TABLE>

//
// Commands
//

struct tCmd
    {
    const char *    sCmdName ;     // Commandname
    int            ( *pProc)(/*in*/ const char *   sArg) ;   // pointer to the procedure
    bool            bPush ;         // Push current state?
    bool            bPop ;          // Pop last state?
    enum tCmdType   nCmdType ;      // Type of the command 
    } ;


//// ----------------------------------------------------------------------------
//// Commandtable...
////


int CmdIf (/*in*/ const char *   sArg) ;
int CmdElse (/*in*/ const char *   sArg) ;
int CmdElsif (/*in*/ const char *   sArg) ;
int CmdEndif (/*in*/ const char *   sArg) ;

int CmdWhile (/*in*/ const char *   sArg) ;
int CmdEndwhile (/*in*/ const char *   sArg) ;
int CmdHidden (/*in*/ const char *   sArg) ;

int HtmlTable    (/*in*/ const char *   sArg) ;
int HtmlList     (/*in*/ const char *   sArg) ;
int HtmlEndtable (/*in*/ const char *   sArg) ;
int HtmlRow      (/*in*/ const char *   sArg) ;
int HtmlEndrow   (/*in*/ const char *   sArg) ;
int HtmlInput    (/*in*/ const char *   sArg) ;
int HtmlTextarea   (/*in*/ const char *   sArg) ;
int HtmlEndtextarea(/*in*/ const char *   sArg) ;

    

struct tCmd CmdTab [] =
    {
        { "/dir",     HtmlEndtable, 0, 1, cmdTable } ,
        { "/dl",      HtmlEndtable, 0, 1, cmdTable } ,
        { "/menu",    HtmlEndtable, 0, 1, cmdTable } ,
        { "/ol",      HtmlEndtable, 0, 1, cmdTable } ,
        { "/table",   HtmlEndtable, 0, 1, cmdTable } ,
        { "/textarea", HtmlEndtextarea, 0, 1, cmdTextarea } ,
        { "/tr",      HtmlEndrow,   0, 1, cmdTablerow } ,
        { "/ul",      HtmlEndtable, 0, 1, cmdTable } ,
        { "dir",      HtmlList,     1, 0, cmdList } ,
        { "dl",       HtmlList,     1, 0, cmdList } ,
        { "else",     CmdElse,      0, 0, cmdIf } ,
        { "elsif",    CmdElsif,     0, 0, cmdIf } ,
        { "endif",    CmdEndif,     0, 1, cmdIf | cmdEndif } ,
        { "endwhile", CmdEndwhile,  0, 1, cmdWhile } ,
        { "hidden",   CmdHidden,    0, 0, cmdNorm } ,
        { "if",       CmdIf,        1, 0, cmdIf } ,
        { "input",    HtmlInput,    0, 0, cmdNorm } ,
        { "menu",     HtmlList,     1, 0, cmdList } ,
        { "ol",       HtmlList,     1, 0, cmdList } ,
        { "table",    HtmlTable,    1, 0, cmdTable } ,
        { "textarea", HtmlTextarea, 1, 0, cmdTextarea } ,
        { "tr",       HtmlRow,      1, 0, cmdTablerow } ,
        { "ul",       HtmlList,     1, 0, cmdList } ,
        { "while",    CmdWhile,     1, 0, cmdWhile } ,

    } ;


//
// print error
//

void LogError (/*in*/ int   rc)

    {
    errdat1 [sizeof (errdat1) - 1] = '\0' ;
    
    switch (rc)
        {
        case ok:                    lprintf ("ERR:  %d: ok\n", rc, errdat1) ; break ;
        case rcStackOverflow:       lprintf ("ERR:  %d: Stack Overflow\n", rc, errdat1) ; break ;
        case rcArgStackOverflow:    lprintf ("ERR:  %d: Argumnet Stack Overflow (%s)\n", rc, errdat1) ; break ;
        case rcStackUnderflow:      lprintf ("ERR:  %d: Stack Underflow\n", rc, errdat1) ; break ;
        case rcEndifWithoutIf:      lprintf ("ERR:  %d: endif without if\n", rc, errdat1) ; break ;
        case rcElseWithoutIf:       lprintf ("ERR:  %d: else without if\n", rc, errdat1) ; break ;
        case rcEndwhileWithoutWhile: lprintf ("ERR:  %d: endwhile without while\n", rc, errdat1) ; break ;
        case rcEndtableWithoutTable: lprintf ("ERR:  %d: </table> without <table>\n", rc, errdat1) ; break ;
        case rcCmdNotFound:         lprintf ("ERR:  %d: Unknown Command %s\n", rc, errdat1) ; break ;
        case rcOutOfMemory:         lprintf ("ERR:  %d: Out of memory\n", rc, errdat1) ; break ;
        case rcPerlVarError:        lprintf ("ERR:  %d: Perl variable error %s\n", rc, errdat1) ; break ;
        case rcHashError:           lprintf ("ERR:  %d: Perl hash error %s\n", rc, errdat1) ; break ;
        case rcArrayError:          lprintf ("ERR:  %d: Perl array error %s\n", rc, errdat1) ; break ;
        case rcFileOpenErr:         lprintf ("ERR:  %d: File %s open error\n", rc, errdat1) ; break ;    
        case rcMissingRight:        lprintf ("ERR:  %d: Missing right %s\n", rc, errdat1) ; break ;
        case rcNoRetFifo:           lprintf ("ERR:  %d: No Return Fifo\n", rc, errdat1) ; break ;
        case rcMagicError:          lprintf ("ERR:  %d: Perl Magic Error\n", rc, errdat1) ; break ;
        case rcWriteErr:            lprintf ("ERR:  %d: File write Error\n", rc, errdat1) ; break ;
        case rcUnknownNameSpace:    lprintf ("ERR:  %d: Namespace %s unknown\n", rc, errdat1) ; break ;
        case rcInputNotSupported:   lprintf ("ERR:  %d: Input not supported in mod_perl mode\n", rc, errdat1) ; break ;
        case rcCannotUsedRecursive: lprintf ("ERR:  %d: Cannot be called recursivly in mod_perl mode\n", rc, errdat1) ; break ;
        case rcEndtableWithoutTablerow: lprintf ("ERR:  %d: </tr> without <tr>\n", rc, errdat1) ; break ;
        case rcEndtextareaWithoutTextarea: lprintf ("ERR:  %d: </textarea> without <textarea>\n", rc, errdat1) ; break ;
        default: lprintf ("ERR:  %d: Error %s\n", rc, errdat1) ; break ; break ;
        }
    strcpy (errdat1, "") ;
    }


// ----------------------------------------------------------------------------
// Output a string and escapte html special character to html special representation (&xxx;)
//
// i/o sData     = input:  perl string
//
// ----------------------------------------------------------------------------

void OutputToHtml (/*i/o*/ const char *  sData)

    {
    char * pHtml  ;
    const char * p = sData ;
    
    while (*sData)
        {
        pHtml = Char2Html[(unsigned char)(*sData)].sHtml ;
        if (*pHtml)
            {
            if (p != sData)
                owrite (p, 1, sData - p) ;
            oputs (pHtml) ;
            p = sData + 1;
            }
        sData++ ;
        }
    if (p != sData)
        owrite (p, 1, sData - p) ;
    }


//
// Eval PERL Statements
//
// Return Perl Scalar Value
//

#define EVAL_SUB

int EvalAll (/*in*/  const char *  sArg,
             /*out*/ SV **   pRet)             
    {
    int   num ;         
    int   nCountUsed = State.nCountUsed ;
    int   nRowUsed   = State.nRowUsed ;
    int   nColUsed   = State.nColUsed ;
#ifndef EVAL_SUB    
    SV *  pSVArg ;
#endif
    dSP;                            /* initialize stack pointer      */


    if (bDebug & dbgEval)
        lprintf ("EVAL< %s\n", sArg) ;



#ifdef EVAL_SUB    



    ENTER;                          /* everything created after here */
    SAVETMPS;                       /* ...is a temporary variable.   */
    PUSHMARK(sp);                   /* remember the stack pointer    */
    XPUSHs(sv_2mortal(newSVpv((char *)sArg, strlen (sArg)))); /* push the base onto the stack  */
    PUTBACK;                        /* make local stack pointer global */
    num = perl_call_pv ("HTML::Embperl::_eval_", G_SCALAR | G_EVAL) ; /* call the function             */
#else
    
    pSVArg = sv_2mortal(newSVpv((char *)sArg, strlen (sArg))) ;

    //num = perl_eval_sv (pSVArg, G_SCALAR) ; /* call the function             */
    num = perl_eval_sv (pSVArg, G_DISCARD) ; /* call the function             */
    num = 0 ;
#endif    
    SPAGAIN;                        /* refresh stack pointer         */
    
    if (bDebug & dbgMem)
        lprintf ("SVs:  %d\n", sv_count) ;
    /* pop the return value from stack */
    if (num == 1)   
        {
        *pRet = POPs ;
        SvREFCNT_inc (*pRet) ;
        if ((nCountUsed != State.nCountUsed ||
             nColUsed != State.nColUsed ||
             nRowUsed != State.nRowUsed) &&
              !SvOK (*pRet))
            State.nResult = 0 ;

        if ((bDebug & dbgTab) &&
            (State.nCountUsed ||
             State.nColUsed ||
             State.nRowUsed))
            lprintf ("TAB:  nResult = %d\n", State.nResult) ;

        if (bDebug & dbgEval)
            if (SvOK (*pRet))
                lprintf ("EVAL> %s\n", SvPV (*pRet, na)) ;
            else
                lprintf ("EVAL> <undefined>\n") ;
        }
     else
        {
        *pRet = NULL ;
        if (bDebug & dbgEval)
            lprintf ("EVAL> <NULL>\n") ;
        }

     PUTBACK;

#ifdef EVAL_SUB    
    FREETMPS;                       /* free that return value        */
    LEAVE;                       /* ...and the XPUSHed "mortal" args.*/
#endif
    
    return num ;
    }


//
// Eval PERL Statements in safe namespace
//
// Return Perl Scalar Value
//


int EvalSafe (/*in*/  const char *  sArg,
              /*out*/ SV **   pRet)             
    {
    int   num ;         
    int   nCountUsed = State.nCountUsed ;
    int   nRowUsed   = State.nRowUsed ;
    int   nColUsed   = State.nColUsed ;
    dSP;                            /* initialize stack pointer      */


    if (bDebug & dbgEval)
        lprintf ("EVAL< %s\n", sArg) ;


    ENTER;                          /* everything created after here */
    SAVETMPS;                       /* ...is a temporary variable.   */
    PUSHMARK(sp);                   /* remember the stack pointer    */
    XPUSHs(pNameSpace) ;
    XPUSHs(sv_2mortal(newSVpv((char *)sArg, strlen (sArg)))); 
    PUTBACK;                        /* make local stack pointer global */
    num = perl_call_method ("reval", G_SCALAR | G_EVAL) ;  /* call the function             */
    SPAGAIN;                        /* refresh stack pointer         */
    
    if (bDebug & dbgMem)
        lprintf ("SVs:  %d\n", sv_count) ;
    /* pop the return value from stack */
    if (num == 1)   
        {
        *pRet = POPs ;
        SvREFCNT_inc (*pRet) ;
        if ((nCountUsed != State.nCountUsed ||
             nColUsed != State.nColUsed ||
             nRowUsed != State.nRowUsed) &&
              !SvOK (*pRet))
            State.nResult = 0 ;

        if ((bDebug & dbgTab) &&
            (State.nCountUsed ||
             State.nColUsed ||
             State.nRowUsed))
            lprintf ("TAB:  nResult = %d\n", State.nResult) ;

        if (bDebug & dbgEval)
            if (SvOK (*pRet))
                lprintf ("EVAL> %s\n", SvPV (*pRet, na)) ;
            else
                lprintf ("EVAL> <undefined>\n") ;
        }
     else
        {
        *pRet = NULL ;
        if (bDebug & dbgEval)
            lprintf ("EVAL> <NULL>\n") ;
        }

    PUTBACK;
    FREETMPS;                       /* free that return value        */
    LEAVE;                       /* ...and the XPUSHed "mortal" args.*/
    
    return num ;
    }


int Eval (/*in*/  const char *  sArg,
          /*out*/ SV **   pRet)             


    {
    if (bSafeEval)
        return EvalSafe (sArg, pRet) ;
    else
        return EvalAll (sArg, pRet) ;
    }




//
// Eval PERL Statements
//
// Return int
//



int EvalNum (/*in*/  const char *  sArg,
             /*out*/ int *   pNum)             
    {
    SV * pRet ;
    int  n ;

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
    
//
// Magic
//


int notused ;

INTMG (Count, State.nCount, State.nCountUsed) ;
INTMG (Row, State.nRow, State.nRowUsed) ;
INTMG (Col, State.nCol, State.nColUsed) ;
INTMG (MaxRow, nTabMaxRow, notused) ;
INTMG (MaxCol, nTabMaxCol, notused) ;
INTMG (TabMode, nTabMode, notused) ;


//
// compare commands
//

int CmpCmd (/*in*/ const void *  p1,
            /*in*/ const void *  p2)

    {
    return strcmp (*((const char * *)p1), *((const char * *)p2)) ;
    }



//
// Command found
//

int  ProcessCmd (/*in*/ const char *    sCmdName,
                 /*in*/ const char *    sArg,
                 /*in*/ int             bIgnore)

    {
    struct tCmd *  pCmd ;
    int            rc ;
    int            nArgLen ;

    pCmd = bsearch (&sCmdName, CmdTab, sizeof (CmdTab) / sizeof (struct tCmd), sizeof (struct tCmd), CmpCmd) ;

    if (bDebug & dbgAllCmds)
        lprintf ("CMD%c:  +%02d Cmd = '%s' Arg = '%s'\n", (pCmd == NULL)?'-':'+', nStack, sCmdName, sArg) ;

    if (pCmd == NULL && bIgnore)
        return rcCmdNotFound ;

    if ((bDebug & dbgCmd) && (bDebug & dbgAllCmds) == 0)
        lprintf ("CMD:  +%02d Cmd = '%s' Arg = '%s'\n", nStack, sCmdName, sArg) ;
    
    if (pCmd == NULL)
        {
        strncpy (errdat1, sCmdName, sizeof (errdat1)) ;
        return rcCmdNotFound ;
        }

    if ((pCmd -> nCmdType & State.bProcessCmds) == 0)
        return ok ; // ignore it


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
            
            memset (&State, 0, sizeof (State)) ;
            State.nCmdType  = pCmd -> nCmdType ;
            State.bProcessCmds = Stack[nStack-1].bProcessCmds ;
            State.pStart    = pCurrPos ;
            State.sArg      = strcpy (pArgStack, sArg) ;
            pArgStack += nArgLen ;
            State.nTabMode  = nTabMode ;
            State.nMaxRow   = nTabMaxRow ;
            State.nMaxCol   = nTabMaxCol ;
            }

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




//// ----------------------------------------------------------------------------
//// if command ...
////

int CmdIf (/*in*/ const char *   sArg)
    {
    int rc = EvalNum (sArg, &State.nResult) ;
    
    if (State.nResult) 
        {
        State.bProcessCmds = cmdAll ;
        }
    else
        {
        State.bProcessCmds = cmdIf ;
        }

    return rc ;
    }

//// ----------------------------------------------------------------------------
//// elsif command ...
////

int CmdElsif  (/*in*/ const char *   sArg)
    {
    int rc = ok ;

    if (State.nCmdType != cmdIf)
        return rcElseWithoutIf ;
    
        
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

//// ----------------------------------------------------------------------------
//// else command ...
////

int CmdElse  (/*in*/ const char *   sArg)
    {
    if (State.nCmdType != cmdIf)
        return rcElseWithoutIf ;

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
                        

//// ----------------------------------------------------------------------------
//// endif befehl ...
////

int CmdEndif (/*in*/ const char *   sArg)
    {
    State.pStart    = NULL ;

    if (State.nCmdType != cmdIf)
        return rcEndifWithoutIf ;

    return ok ;
    }
                        


//// ----------------------------------------------------------------------------
//// while command ...
////

int CmdWhile (/*in*/ const char *   sArg)
    {
    int rc = EvalNum (sArg, &State.nResult) ;
    
    if (State.nResult) 
        State.bProcessCmds = cmdAll ;
    else
        State.bProcessCmds = cmdWhile ;

    return rc ;
    }

//// ----------------------------------------------------------------------------
//// endwhile command ...
////

int CmdEndwhile (/*in*/ const char *   sArg)
    {
    int rc = ok ;

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


//// ----------------------------------------------------------------------------
//// hidden command ...
////

int CmdHidden (/*in*/ const char *   sArg)

    {
    char * pSub = strchr (sArg, ',') ;
    char * pEnd ;
    char * pKey ;
    SV *   psv ;
    HV *   pSubHash ;
    HV *   pAddHash ;
    HE *   pEntry ;
    I32    l ;

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

            oprintf ("<input type=\"hidden\" name=\"%s\" value=\"", pKey) ;
            OutputToHtml (SvPV (psv, na)) ;
            oputs ("\">\n") ;
            }
        }

    return ok ;
    }




// ----------------------------------------------------------------------------
// find substring ignore case
//
// in  pSring  = string to search in (any case)
// in  pSubStr = string to search for (must be upper case)
//
// out ret  = pointer to pSubStr in pStringvalue or NULL if not found
//
// ----------------------------------------------------------------------------


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

// ----------------------------------------------------------------------------
// make string lower case
//
// i/o  pSring  = string to search in (any case)
//
// ----------------------------------------------------------------------------


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


//
// compare commands
//

int CmpCharTrans (/*in*/ const void *  pKey,
                  /*in*/ const void *  pEntry)

    {
    return strcmp ((const char *)pKey, ((struct tCharTrans *)pEntry) -> sHtml) ;
    }



// ----------------------------------------------------------------------------
// replace html special character representation (&xxx;) with correct chars
// and delete all html tags
// The Replacement is done in place, the whole string will become shorter
// and is padded with spaces
// tags and special charcters which are preceeded by a \ are not translated
//
// i/o sData     = input:  html string
//                 output: perl string
//
// ----------------------------------------------------------------------------

void TransHtml (/*i/o*/ char *  sData)

    {
    char * p = sData ;
    char * s = NULL ;
    char * e = sData + strlen (sData) ;
    struct tCharTrans * pChar ;


    while (*p)
        {
        if (*p == '\\')
            {
            if (p[1] == '<')
                { //  Quote next HTML tag
                p += 2 ;
                while (*p && *p != '>')
                    p++ ;
                }
            else if (p[1] == '&')
                { //  Quote next HTML char
                p += 2 ;
                while (*p && *p != ';')
                    p++ ;
                }
            else
                p++ ; // Nothing to quote
            }
        else
            {
            if (p[0] == '<')
                { //  count HTML tag length
                s = p ;
                p++ ;
                while (*p && *p != '>')
                    p++ ;
                if (*p)
                    p++ ;
                }
            else if (p[0] == '&')
                { //  count HTML char length
                s = p ;
                p++ ;
                while (*p && *p != ';')
                    p++ ;

                if (*p)
                    {
                    *p = '\0' ;
                    p++ ;
                    }
                pChar = bsearch (s, Html2Char, sizeHtml2Char, sizeof (struct tCharTrans), CmpCharTrans) ;
                if (pChar)
                    *s = pChar -> c ;
                else
                    *s = '?' ;
                s++ ;
                
                }

            if (s && (p - s) > 0)
                { // copy rest of string, pad with spaces
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

                        
// ----------------------------------------------------------------------------
// get argument from html tag 
//
// in  pTag = html tag args  (eg. arg=val arg=val .... >)
// in  pArg = name of argument (must be upper case)
//
// out pLen = length of value
// out ret  = pointer to value or NULL if not found
//
// ----------------------------------------------------------------------------

const char * GetHtmlArg (/*in*/  const char *    pTag,
                         /*in*/  const char *    pArg,
                         /*out*/ int *           pLen)

    {
    const char * pVal ;
    const char * pEnd ;
    const char * pName ;
    int l = strlen (pArg) ;


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

        
        if (strnicmp (pTag, pArg, l) == 0 && (pTag[l] == '=' || isspace (pTag[l]) || pTag[l] == '>'))
            if (*pLen > 0)
                return pVal ;
            else
                return pTag ;

        pTag = pEnd ;
        }

    return NULL ;
    }



//// ----------------------------------------------------------------------------
//// table tag ...
////

int HtmlTable (/*in*/ const char *   sArg)
    {
    State.nResult = 1 ;
    if ((State.nTabMode & epTabRow) == epTabRowDef)
        State.pBuf = oBegin () ;
    return ok ;
    }

//// ----------------------------------------------------------------------------
//// various list tags ... (dir, menu, ol, ul)
////

int HtmlList (/*in*/ const char *   sArg)
    {
    State.nResult = 1 ;
    State.nCol    = 1 ;
    if ((State.nTabMode & epTabRow) == epTabRowDef)
        State.pBuf = oBegin () ;
    return ok ;
    }

//// ----------------------------------------------------------------------------
//// /table tag ... (and end of list)
////

int HtmlEndtable (/*in*/ const char *   sArg)
    {
    if (State.nCmdType != cmdTable)
        return rcEndtableWithoutTable ;


    if ((State.nTabMode & epTabRow) == epTabRowDef)
        if (State.nResult || State.nCol > 0)
            oCommit (State.pBuf) ;
        else
            oRollback (State.pBuf) ;

    State.nRow++ ;
    if (((State.nTabMode & epTabRow) == epTabRowMax ||
         ((State.nResult || State.nCol > 0) && (State.nRowUsed || State.nCountUsed) )) &&
          State.nRow < State.nMaxRow)
        {
        pCurrPos = State.pStart ;        
        if ((State.nTabMode & epTabRow) == epTabRowDef)
            State.pBuf = oBegin () ;

        return ok ;
        }

    State.pStart    = NULL ;

    return ok ;
    }

//// ----------------------------------------------------------------------------
//// tr tag ...
////

int HtmlRow (/*in*/ const char *   sArg)
    {
    State.nResult = 1 ;
    if (Stack[nStack-1].nCmdType == cmdTable)
        {
        State.nCount     = Stack[nStack-1].nCount ;
        State.nCountUsed = Stack[nStack-1].nCountUsed ;
        State.nRow       = Stack[nStack-1].nRow ;
        State.nRowUsed   = Stack[nStack-1].nRowUsed ;
        }

    if ((State.nTabMode & epTabCol) == epTabColDef)
        State.pBuf = oBegin () ;
    
    return ok ;
    }

//// ----------------------------------------------------------------------------
//// /tr tag ...
////

int HtmlEndrow (/*in*/ const char *   sArg)
    {
    if (State.nCmdType != cmdTablerow)
        return rcEndtableWithoutTablerow ;

    if ((State.nTabMode & epTabCol) == epTabColDef)
        if (State.nResult || (!State.nColUsed && !State.nCountUsed && !State.nRowUsed))
            oCommit (State.pBuf) ;
        else
            oRollback (State.pBuf), State.nCol-- ;

        

    State.nCount++ ;
    State.nCol++ ;
    if (((State.nTabMode & epTabCol) == epTabColMax ||
         (State.nResult && (State.nColUsed || State.nCountUsed)))
        && State.nCol < State.nMaxCol)
        {
        pCurrPos = State.pStart ;        
        if ((State.nTabMode & epTabCol) == epTabColDef)
            State.pBuf = oBegin () ;
        }
    else
        State.pStart    = NULL ;

    if (Stack[nStack-1].nCmdType == cmdTable)
        {
        Stack[nStack-1].nCount      = State.nCount     ;
        Stack[nStack-1].nCountUsed  = State.nCountUsed ;
        Stack[nStack-1].nRow        = State.nRow     ;
        Stack[nStack-1].nRowUsed    = State.nRowUsed ;
        Stack[nStack-1].nCol        = State.nCol     ;
        Stack[nStack-1].nColUsed    = State.nColUsed ;
        Stack[nStack-1].nResult     = State.nResult ;
        }

    return ok ;
    }
                        

                        

// ----------------------------------------------------------------------------
// input tag ...
//
// ----------------------------------------------------------------------------


int HtmlInput (/*in*/ const char *   sArg)
    {
    const char *  pName ;
    const char *  pVal ;
    const char *  pData ;
    const char *  pType ;
    const char *  pCheck ;
    int           nlen ;
    int           vlen ;
    int           dlen ;
    int           tlen ;
    int           clen ;
    SV *          pSV ;
    SV **         ppSV ;
    char          sName [256] ;
    int           bCheck ;
    int           bEqual ;


    pName = GetHtmlArg (sArg, "NAME", &nlen) ;
    if (nlen == 0)
        {
        if (bDebug & dbgInput)
            lprintf ("INPU: has no name\n") ; 
        return ok ; // no Name
        }

    nlen = min (nlen, sizeof (sName) - 1) ;
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
            lprintf ("INPU: %s already has a value = %s\n", sName, SvPV (pSV, na)) ; 
        
        if (hv_store (pInputHash, sName, strlen (sName), pSV, 0) == NULL)
            return rcHashError ;
        
        return ok ; // has already a value
        }


    ppSV = hv_fetch(pFormHash, (char *)pName, nlen, 0) ;  
    if (ppSV == NULL)
        {
        if (bDebug & dbgInput)
            lprintf ("INPU: %s: no data available in form data\n", sName) ; 
        return ok ; // no data available
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
                ; // Everything ok
            else
                { // Remove "checked"
                oputs ("<INPUT ") ;
                
                if (owrite (sArg, pCheck - sArg, 1) != 1)
                    return rcWriteErr ;
    
                pCurrPos = (char *)pCheck + 7 ; // let main programm write rest of html tag
                }
            }
        else
            {
            if (bEqual)
                { // Insert "checked"
                oputs ("<INPUT ") ;

                if (owrite (sArg, (pCurrPos - 1) - sArg, 1) != 1)
                    return rcWriteErr ;

                oputs (" CHECKED>") ;
    
                pCurrPos = NULL ; // nothing more left of html tag
                }
            }
        }
    else if (pVal)
        {
        oputs ("<INPUT ") ;

        if (owrite (sArg, pVal - sArg, 1) != 1)
            return rcWriteErr ;
    
    
        //if (owrite (pData, dlen, 1) != 1)
        //    return rcWriteErr ;

        OutputToHtml (pData) ;

        pCurrPos = (char *)pVal + 1 ; // let main programm write rest of html tag
        }
    else
        {
        oputs ("<INPUT ") ;

        if (owrite (sArg, (pCurrPos - 1) - sArg, 1) != 1)
            return rcWriteErr ;

        oputs (" VALUE=\"") ;
        OutputToHtml (pData) ;
        oputs ("\">") ;
    
        pCurrPos = NULL ; // nothing more left of html tag
        }


    if (bDebug & dbgInput)
        {
        lprintf ("INPU: %s=%s %s\n", sName, pData, bCheck?(bEqual?"CHECKED":"NOT CHECKED"):"") ; 
        }
    
    pSV = newSVpv ((char *)pData, dlen) ;
    if (hv_store (pInputHash, sName, strlen (sName), pSV, 0) == NULL)
        return rcHashError ;

    return ok ;
    }

// ----------------------------------------------------------------------------
// textarea tag ...
//
// ----------------------------------------------------------------------------


int HtmlTextarea (/*in*/ const char *   sArg)
    {

    return ok ;
    }

    
// ----------------------------------------------------------------------------
// /textarea tag ...
//
// ----------------------------------------------------------------------------


int HtmlEndtextarea (/*in*/ const char *   sArg)
    {
    const char *  pName ;
    const char *  pVal ;
    const char *  pEnd ;
    const char *  pData ;
    int           nlen ;
    int           vlen ;
    int           dlen ;
    int           clen ;
    SV *          pSV ;
    SV **         ppSV ;
    char          sName [256] ;

    pVal = State.pStart ;

    State.pStart = NULL ;

    if (State.nCmdType != cmdTextarea)
        return rcEndtextareaWithoutTextarea ;


    pName = GetHtmlArg (State.sArg, "NAME", &nlen) ;
    if (nlen == 0)
        {
        if (bDebug & dbgInput)
            lprintf ("TEXT: has no name\n") ; 
        return ok ; // no Name
        }

    nlen = min (nlen, sizeof (sName) - 1) ;
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
            lprintf ("TEXT: %s already has a value = %s\n", sName, SvPV (pSV, na)) ; 
        
        if (hv_store (pInputHash, sName, strlen (sName), pSV, 0) == NULL)
            return rcHashError ;
        
        return ok ; // has already a value
        }


    ppSV = hv_fetch(pFormHash, (char *)pName, nlen, 0) ;  
    if (ppSV == NULL)
        {
        if (bDebug & dbgInput)
            lprintf ("TEXT: %s: no data available in form data\n", sName) ; 
        return ok ; // no data available
        }

    pData = SvPV (*ppSV, dlen) ;
    
    if (pVal)
        {
        OutputToHtml (pData) ;
        }

    if (bDebug & dbgInput)
        {
        lprintf ("TEXT: %s=%s\n", sName, pData) ; 
        }
    
    pSV = newSVpv ((char *)pData, dlen) ;
    if (hv_store (pInputHash, sName, strlen (sName), pSV, 0) == NULL)
        return rcHashError ;

    return ok ;
    }

    
    
//////////////////////////////////////////////////////////////////////////////////////////
// 
// Get a Value out of a perl hash
//


char * GetHashValueLen (/*in*/  HV *           pHash,
                        /*in*/  const char *   sKey,
                        /*in*/  int            nLen,
                        /*in*/  int            nMaxLen,
                        /*out*/ char *         sValue)

    {
    SV **   ppSV ;
    char *  p ;
    int     len ;        


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



//// ----------------------------------------------------------------------------
//// read form input from http server...
////

int GetFormData (/*in*/ char * pQueryString,
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
                    pSVV = newSVpv (pVal, nVal) ;
                    pSVK = newSVpv (pKey, nKey) ;

                    if (hv_store (pFormHash, pKey, nKey, pSVV, 0) == NULL)
                        {
                        _free (pMem) ;
                        return rcHashError ;
                        }

                
                    av_push (pFormArray, pSVK) ;
                
                    if (bDebug & dbgForm)
                        lprintf ("FORM: %s=%s\n", pKey, pVal) ; 

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

    _free (pMem) ;

    return ok ;    
    }

//// ----------------------------------------------------------------------------
//// read input from cgi process...
////


int GetInputData_CGIProcess ()

    {
    char *  p ;
    int     rc = ok ;
    int  state = 0 ;
    int  len   = 0 ;
    char sLine [1024] ;
    SV * pSVE ;
    

    hv_clear (pEnvHash) ;


    if (bDebug)
        lprintf ("\nWaiting for Request... SVs: %d OBJs: %d\n", sv_count, sv_objcount) ;

    if ((rc = OpenInput (sCmdFifo)) != ok)
        {
        return rc ;
        }


    if (bDebug)
        lprintf ("Processing Request...\n") ;
    
    while (igets (sLine, sizeof (sLine)))
        {
        len = strlen (sLine) ; 
        while (len >= 0 && isspace (sLine [--len]))
            ;
        sLine [len + 1] = '\0' ;
        

        if (strcmp (sLine, "----") == 0)
            { state = 1 ; if (bDebug) lprintf ("Environement...\n") ;}
        else if (strcmp (sLine, "****") == 0)
            { state = 2 ;  if (bDebug) lprintf ( "Formdata...\n") ;}
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
                lprintf ( "ENV:  %s=%s\n", sLine, p) ;
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
            { if (bDebug) lprintf ("Unknown Input: %s\n", sLine) ;}

        }
        
    CloseInput () ;
    
    return rc ;
    }
                        

//// ----------------------------------------------------------------------------
//// get form data when running as cgi script...
////


int GetInputData_CGIScript ()

    {
    char *  p ;
    char *  f ;
    int     rc = ok ;
    int     len   = 0 ;
    char    sQuery [2048] ;
    char    sLen [20] ;
    

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

                lprintf ( "ENV:  %s=%s\n", pKey, SvPV (psv, na)) ; 
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
        lprintf ( "Formdata... length = %d\n", len) ;    

    rc = GetFormData (p, len) ;
    
    if (f)
        _free (f) ;
        
    
    return rc ;
    }

// ----------------------------------------------------------------------------
// Process Formdata when running under mod_perl
//

int GetInputData_Mod_Perl ()

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

        if (bDebug & dbgEnv)
            lprintf ( "ENV:  %s=%s\n", pKey, SvPV (psv, na)) ; 
        }

    hv_iterinit (pFormHash) ;
    while (pEntry = hv_iternext (pFormHash))
        {
        pKey = hv_iterkey (pEntry, &l) ;
        psv  = hv_iterval (pFormHash, pEntry) ;

        if (bDebug & dbgForm)
            lprintf ("FORM: %s=%s\n", pKey, SvPV (psv, na)) ; 
        }

    return ok ;
    }



// ----------------------------------------------------------------------------
// scan html tag ...
//
// p points to '<'
//

int ScanHtmlTag (/*in*/ char *   p)

    { 
    int  rc ;
    char ec ;
    char ea ;
    char * pec ;
    char * pea ;
    char * pCmd ;
    char * pArg ;

    pCurrTag = p ;     // save start of html tag

    // skip space
    p++ ;
    while (*p != '\0' && isspace (*p))
            p++ ;
    
    pCmd = p ;               // start of tag name
    while (*p != '\0' && !isspace (*p) && *p != '>')
        p++ ;

    ec = *p ;              // save first char after tag name
    pec = p ;
    *p = '\0' ;            // set end of tag name to \0
    if (p)
        p++ ;
    pArg = p ;             // start of arguments

    if (ec == '>')
        { // No Arguments
        pArg = p - 1 ;
        pea = NULL ;
        }
    else
        {
        p = strchr (p, '>') ; // get end of tag

        if (p)
            {
            ea = *p ;
            pea = p ;
            *p = '\0' ;            // set end of tag arguments to \0
            p++ ;
            }
        else
            {
            p = pArg + strlen (pArg) ;
            pea = NULL ;
            }
        }

    pCurrPos = p ;    // pCurrPos = first char after whole tag

    strlower (pCmd) ;         // see if knwon html tag and execute
    if ((rc = ProcessCmd (pCmd, pArg, TRUE)) != ok)
        {
        if (rc == rcCmdNotFound)
            {
              // only write html tag start char and
            p = pCurrPos = pCurrTag + 1 ;       // check if more to exceute within html tag
            }
        else
            return rc ;
        }


    *pec = ec ;              // restore first char after tag name
    if (pea)
        *pea = ea ;              // restore first char after tag arguments

    if (p == pCurrPos && pCurrPos) // if CurrPos didn't change write out html tag as it is
        owrite (pCurrTag, 1, pCurrPos - pCurrTag) ;
    
    if (pCurrPos == NULL)
        pCurrPos = p ; // html tag is written by command handler

    pCurrTag = NULL ;

    return ok ;    
    }


// ----------------------------------------------------------------------------
// scan commands and evals ([x ... x] sequenz) ...
//
// p points to '['
//


    
int ScanCmdEvals (/*in*/ char *   p)
    
    
    { 
    int     rc ;
    int     len ;
    int     n ;
    char *  c ;
    char *  a ;
    char    nType ;
    SV *    pRet ;

    
    p++ ;

    pCurrPos = p ;

    if ((nType = *p++) == '\0')
        return ok ;

    pCurrPos = p ;

    if (nType != '+' && nType != '-' && nType != '$' )
        { // escape (for [[ -> [)
        oputc (nType) ;
        return ok ;
        }


    do
        { // search end 
        p++ ;
        if ((p = strchr (p, ']')) == NULL)
            break ;
        }   
    while (p[-1] != nType) ;
    if (p == NULL)
        { // end not found
        sprintf (errdat1, "%c]", nType) ; 
        return rcMissingRight ;
        }
    p [-1] = '\0' ;
    p++ ;

    // strip off all <HTML> Tags

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

            if (*a != '\0')
                {
                *a = '\0' ;
                a++ ;
                }

            pCurrPos = p ;
        
            if ((rc = ProcessCmd (c, a, FALSE)) != ok)
                return rc ;
        
            a [-1] = ' ' ;
            p [-2] = nType ;

            break ;
        }

    return ok ;
    }
// ----------------------------------------------------------------------------
// add magic to integer var
//
// in  sVarName = Name of varibale
// in  pVirtTab = pointer to virtual table
//
// ----------------------------------------------------------------------------

int AddMagic (/*in*/ char *     sVarName,
              /*in*/ MGVTBL *   pVirtTab) 

    {
    SV * pSV ;
    struct magic * pMagic ;

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


// ----------------------------------------------------------------------------
// init embperl module
//
// in  nIOType = type of requested i/o
//
// ----------------------------------------------------------------------------

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
    
    OpenLog (sLogFile) ;

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
        
        lprintf ("INIT: Embperl starting... mode = %s (%d)\n", p, nIOType) ;
        }

    if ((pFormHash = perl_get_hv (sFormHashName, TRUE)) == NULL)
        {
        LogError ( rcHashError) ;
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
    
    rc = AddMagic (sTabCountName, &mvtTabCount) ;
    if (rc == 0)
        rc = AddMagic (sTabRowName, &mvtTabRow) ;
    if (rc == 0)
        rc = AddMagic (sTabColName, &mvtTabCol) ;
    if (rc == 0)
        rc = AddMagic (sTabMaxRowName, &mvtTabMaxRow) ;
    if (rc == 0)
        rc = AddMagic (sTabMaxColName, &mvtTabMaxCol) ;
    if (rc == 0)
        rc = AddMagic (sTabModeName, &mvtTabTabMode) ;

    return rc ;
    }

// ----------------------------------------------------------------------------
// clean up embperl module
//
// ----------------------------------------------------------------------------

int iembperl_term (void) 


    {
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
#endif

    return ok ;
    }









int iembperl_req  (/*in*/ int     bDebugFlags,
                   /*in*/ char *  pNameSpaceName)




    {
    clock_t startclock = clock () ;
    int     rc ;
    int     len ;
    char *  p ;
    int     n ;
    char *  c ;
    char *  a ;
    SV **   ppSV ;
    char    sInputfile [256] ;
    char *  pBuf = NULL ;
    struct stat st ;
    char    nType ;
    SV *    pRet ;
    int     ifd ;
    I32     stsv_count = sv_count ;
    I32     stsv_objcount = sv_objcount ;
    I32     lstsv_count = sv_count ;
    I32     lstsv_objcount = sv_objcount ;


    if (bDebug)
        lprintf ("REQ:  starting...\n") ;


    if (pNameSpaceName && *pNameSpaceName != '\0')
        {
        if ((pNameSpace = perl_get_sv (pNameSpaceName, FALSE)) == NULL)
            {
            LogError (rcUnknownNameSpace) ;
#ifdef APACHE
            pReq = NULL ;
#endif
            return rcUnknownNameSpace ;
            }
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
        
        lprintf ("REQ:  %s %s", (bSafeEval)?"Namespace = ":"No Safe Eval", pNameSpaceName?pNameSpaceName:"") ;
        lprintf (" mode = %s (%d)\n", p, nIOType) ;
        }
    nStack    = 0 ;
    pArgStack = ArgStack ;

    memset (&State, 0, sizeof (State)) ;
    
    State.nCmdType      = cmdNorm ;
    State.bProcessCmds  = cmdAll ;
    State.sArg          = strcpy (pArgStack, "") ;
    pArgStack += 1 ;
    nTabMode    = epTabRowDef | epTabColDef ;
    nTabMaxRow  = 100 ;
    nTabMaxCol  = 10 ;
    pCurrTag    = NULL ;

    if (pBuf)
        {
        _free (pBuf) ;
        pBuf = NULL ;
        }

    //
    // read data from cgi script
    //

    rc = ok ;
    switch (nIOType)
        {
        case epIOPerl:
        case epIOCGI:
            rc = GetInputData_CGIScript () ;
            break ;
        case epIOProcess:
            rc = GetInputData_CGIProcess () ;
            break ;
        case epIOMod_Perl:
            rc = GetInputData_Mod_Perl () ;
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
    
    if (bDebug)
        lprintf ("Open %s for output...\n", sRetFifo) ;

    if ((rc = OpenOutput (sRetFifo)) != ok)
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
        oputs ("Content-type: text/html\n\n") ;

    GetHashValue (pEnvHash, "PATH_TRANSLATED", sizeof (sInputfile), sInputfile) ;

    //
    // read HTML File into PBuf
    //

    if (bDebug)
        lprintf ("Reading %s as input...\n", sInputfile) ;

    if ((ifd = open (sInputfile, O_RDONLY)) < 0)
        {
        strncpy (errdat1, sInputfile, sizeof (errdat1)) ;
        LogError (rcFileOpenErr) ;
#ifdef APACHE
        pReq = NULL ;
#endif
        return rcFileOpenErr ;
        }

    fstat (ifd, &st) ;
    
    if ((pBuf = _malloc (st.st_size + 1)) == NULL)
        {
        LogError (rcOutOfMemory) ;
#ifdef APACHE
        pReq = NULL ;
#endif
        return rcOutOfMemory ;
        }

    read (ifd, pBuf, st.st_size) ;

    close (ifd) ;
    
    pBuf [st.st_size] = '\0' ;

    //
    // Datei bearbeiten...
    //

    pCurrPos = pBuf ;
    pEndPos  = pBuf + st.st_size ;

    rc = ok ;
    while (pCurrPos < pEndPos && rc == ok)
        {
        if ((bDebug & dbgMem) && (sv_count != lstsv_count || sv_objcount != lstsv_objcount))
            {
            lprintf ("SVs:  Entry-SVs: %d -OBJs: %d Curr-SVs: %d -OBJs: %d\n", stsv_count, stsv_objcount, sv_count, sv_objcount) ;
            lstsv_count = sv_count ;
            lstsv_objcount = sv_objcount ;
            }
        
        
        if (bDebug & dbgSource)
            lprintf ("SRC: %70.70s\n", pCurrPos) ;
        
        //
        // execute [x ... x] and special html tags and replace them if nessecary
        //

        if (State.bProcessCmds == cmdAll)
            {
            n = strcspn (pCurrPos, "[<") ;
            p = pCurrPos + n ;
            }
        else
            p = strchr (pCurrPos, '[') ;
            
            
        if (p == NULL || *p == '\0')
            { // output the rest of html
            owrite (pCurrPos, 1, pEndPos - pCurrPos) ;
            break ;
            }
        
        if (State.bProcessCmds == cmdAll)
            // output until next cmd
            owrite (pCurrPos, 1, p - pCurrPos) ;
        
        pCurrStart = p ;
        if (*p == '<')
            { // HTML Tag
            rc = ScanHtmlTag (p) ;
            }
         else
            { // [x ... x] sequenz
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

        
        lprintf ("Request finished. Entry-SVs: %d -OBJs: %d Exit-SVs: %d -OBJs: %d  Time: %d ms (cps %d)\n", stsv_count, stsv_objcount, sv_count, sv_objcount, ((cl - startclock) * 1000 / CLOCKS_PER_SEC), CLOCKS_PER_SEC) ;
        }

    FlushLog () ;


#ifdef APACHE
    pReq = NULL ;
#endif

    return ok ;
    }

