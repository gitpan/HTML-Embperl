/*###################################################################################
#
#   Embperl - Copyright (c) 1997-1998 Gerald Richter / ECOS
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



static struct tCmd * pCurrCmd ;    /* Current cmd which is excuted */


/*
   Stack 
*/



struct tStackEntry Stack [nStackMax] ; /* Stack for if, while, etc. */

int nStack = 0 ;                /* Stackpointer */

struct tStackEntry State ;             /* current State */

char ArgStack [16384] ;

char * pArgStack = ArgStack ;


/* */
/* Stack for dynamic table counters */
/* */


struct tTableStackEntry TableStack [nStackMax] ; /* Stack for table */

int nTableStack = 0 ;                /* Stackpointer */

struct tTableStackEntry TableState ;             /* current State */


int nTabMode    ;    /* mode for next table (only takes affect after next <TABLE> */
int nTabMaxRow  ;    /* maximum rows for next table (only takes affect after next <TABLE> */
int nTabMaxCol  ;    /* maximum columns for next table (only takes affect after next <TABLE> */

/* ---------------------------------------------------------------------------- */
/* Commandtable...                                                              */
/* ---------------------------------------------------------------------------- */


static int CmdIf (/*in*/ const char *   sArg) ;
static int CmdElse (/*in*/ const char *   sArg) ;
static int CmdElsif (/*in*/ const char *   sArg) ;
static int CmdEndif (/*in*/ const char *   sArg) ;

static int CmdWhile (/*in*/ const char *   sArg) ;
static int CmdEndwhile (/*in*/ const char *   sArg) ;
static int CmdHidden (/*in*/ const char *   sArg) ;
static int CmdVar    (/*in*/ const char *   sArg) ;

static int HtmlTable    (/*in*/ const char *   sArg) ;
static int HtmlTableHead(/*in*/ const char *   sArg) ;
static int HtmlSelect   (/*in*/ const char *   sArg) ;
static int HtmlOption   (/*in*/ const char *   sArg) ;
static int HtmlEndtable (/*in*/ const char *   sArg) ;
static int HtmlRow      (/*in*/ const char *   sArg) ;
static int HtmlEndrow   (/*in*/ const char *   sArg) ;
static int HtmlInput    (/*in*/ const char *   sArg) ;
static int HtmlTextarea   (/*in*/ const char *   sArg) ;
static int HtmlEndtextarea(/*in*/ const char *   sArg) ;
static int HtmlBody       (/*in*/ const char *   sArg) ;
static int HtmlA         (/*in*/ const char *   sArg) ;
static int HtmlMeta      (/*in*/ const char *   sArg) ;

    

struct tCmd CmdTab [] =
    {
        /* cmdname    function        push pop  type         scan save no      disable */
        { "/dir",     HtmlEndtable,     0, 1, cmdTable,         0, 0, cnDir    , optDisableTableScan } ,
        { "/dl",      HtmlEndtable,     0, 1, cmdTable,         0, 0, cnDl     , optDisableTableScan } ,
        { "/menu",    HtmlEndtable,     0, 1, cmdTable,         0, 0, cnMenu   , optDisableTableScan } ,
        { "/ol",      HtmlEndtable,     0, 1, cmdTable,         0, 0, cnOl     , optDisableTableScan } ,
        { "/select",  HtmlEndtable,     0, 1, cmdTable,         0, 0, cnSelect , optDisableTableScan } ,
        { "/table",   HtmlEndtable,     0, 1, cmdTable,         0, 0, cnTable  , optDisableTableScan } ,
        { "/textarea", HtmlEndtextarea, 0, 1, cmdTextarea,      0, 0, cnNop    , optDisableInputScan } ,
        { "/tr",      HtmlEndrow,       0, 1, cmdTablerow,      0, 0, cnTr     , optDisableTableScan } ,
        { "/ul",      HtmlEndtable,     0, 1, cmdTable,         0, 0, cnUl     , optDisableTableScan } ,
        { "a",        HtmlA,            0, 0, cmdNorm,          0, 0, cnNop    , 0 } ,
        { "body",     HtmlBody,         0, 0, cmdNorm,          1, 0, cnNop    , 0 } ,
        { "dir",      HtmlTable,        1, 0, cmdTable,         1, 0, cnDir    , optDisableTableScan } ,
        { "dl",       HtmlTable,        1, 0, cmdTable,         1, 0, cnDl     , optDisableTableScan } ,
        { "else",     CmdElse,          0, 0, cmdIf,            0, 0, cnNop    , 0 } ,
        { "elsif",    CmdElsif,         0, 0, cmdIf,            0, 0, cnNop    , 0 } ,
        { "endif",    CmdEndif,         0, 1, cmdIf | cmdEndif, 0, 0, cnNop    , 0 } ,
        { "endwhile", CmdEndwhile,      0, 1, cmdWhile,         0, 0, cnNop    , 0 } ,
        { "hidden",   CmdHidden,        0, 0, cmdNorm,          0, 0, cnNop    , 0 } ,
        { "if",       CmdIf,            1, 0, cmdIf | cmdEndif, 0, 0, cnNop    , 0 } ,
        { "input",    HtmlInput,        0, 0, cmdNorm,          1, 0, cnNop    , optDisableInputScan } ,
        { "menu",     HtmlTable,        1, 0, cmdTable,         1, 0, cnMenu   , optDisableTableScan } ,
        { "meta",     HtmlMeta,         0, 0, cmdNorm,          1, 0, cnNop    , optDisableMetaScan  } ,
        { "ol",       HtmlTable,        1, 0, cmdTable,         1, 0, cnOl     , optDisableTableScan } ,
        { "option",   HtmlOption,       0, 0, cmdNorm,          1, 0, cnNop    , optDisableInputScan } ,
        { "select",   HtmlSelect,       1, 0, cmdTable,         1, 0, cnSelect , optDisableTableScan } ,
        { "table",    HtmlTable,        1, 0, cmdTable,         1, 0, cnTable  , optDisableTableScan } ,
        { "textarea", HtmlTextarea,     1, 0, cmdTextarea,      1, 1, cnNop    , optDisableInputScan } ,
        { "th",       HtmlTableHead,    0, 0, cmdNorm,          1, 0, cnNop    , optDisableTableScan } ,
        { "tr",       HtmlRow,          1, 0, cmdTablerow,      1, 0, cnTr     , optDisableTableScan } ,
        { "ul",       HtmlTable,        1, 0, cmdTable,         1, 0, cnUl     , optDisableTableScan } ,
        { "var",      CmdVar,           0, 0, cmdNorm,          0, 0, cnNop    , 0 } ,
        { "while",    CmdWhile,         1, 0, cmdWhile,         0, 1, cnNop    , 0 } ,

    } ;



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

int  SearchCmd          (/*in*/  const char *    sCmdName,
                         /*in*/  int             nCmdLen,
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
    while (nCmdLen-- > 0 && --i > 0)
        if ((*p++ = tolower (*sCmdName++)) == '\0')
            break ;

    *p = '\0' ;
    
    p = sCmdLwr ;
    pCmd = (struct tCmd *)bsearch (&p, CmdTab, sizeof (CmdTab) / sizeof (struct tCmd), sizeof (struct tCmd), CmpCmd) ;
    if (pCmd && (pCmd -> bDisableOption & bOptions))
        pCmd = NULL ; /* command is disabled */

    if (bDebug & dbgAllCmds)
        if (sArg && *sArg != '\0')
            lprintf ("[%d]CMD%c:  +%02d Cmd = '%s' Arg = '%s'\n", nPid, (pCmd == NULL)?'-':'+', nStack, sCmdLwr, sArg) ;
        else
            lprintf ("[%d]CMD%c:  +%02d Cmd = '%s'\n", nPid, (pCmd == NULL)?'-':'+', nStack, sCmdLwr) ;

    if (pCmd == NULL && bIgnore)
        return rcCmdNotFound ;

    if ((bDebug & dbgCmd) && (bDebug & dbgAllCmds) == 0)
        if (sArg && *sArg != '\0')
            lprintf ("[%d]CMD:  +%02d Cmd = '%s' Arg = '%s'\n", nPid, nStack, sCmdLwr, sArg) ;
        else
            lprintf ("[%d]CMD:  +%02d Cmd = '%s'\n", nPid, nStack, sCmdLwr) ;
    
    if (pCmd == NULL)
        {
        strncpy (errdat1, sCmdLwr, sizeof (errdat1) - 1) ;
        return rcCmdNotFound ;
        }

    
    *ppCmd = pCmd ;
    
    return ok ;
    }



/* */
/* Process a Command */
/* */



int  ProcessCmd        (/*in*/ struct tCmd *  pCmd,
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
            if (pCmd -> bSaveArg)
                {
                nArgLen = strlen (sArg) + 1 ;
                if (pArgStack + nArgLen >= ArgStack + sizeof (ArgStack))
                    {
                    sprintf (errdat1, "nArgLen=%d, pArgStack=%d",nArgLen,  pArgStack - ArgStack) ;
                    return rcArgStackOverflow ;
                    }
                }

            Stack[nStack++] = State ;
            
            State.nCmdType  = pCmd -> nCmdType ;
            /*State.bProcessCmds = Stack[nStack-1].bProcessCmds ;*/
            State.pStart    = pCurrPos ;
            if (pCmd -> bSaveArg)
                {
                State.sArg      = strcpy (pArgStack, sArg) ;
                pArgStack += nArgLen ;
                }
            else
                State.sArg = NULL ;
            State.pSV       = NULL ;
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
            if (State.pSV)
                SvREFCNT_dec (State.pSV) ;

            State = Stack[--nStack];
            }

    return rc ;
    }




/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* if command ...                                                               */
/*                                                                              */
/* ---------------------------------------------------------------------------- */

static int CmdIf (/*in*/ const char *   sArg)
    {
    int rc = ok ;

    EPENTRY (CmdIf) ;
    
    if (State.bProcessCmds == cmdAll)
        {
        rc = EvalBool ((char *)sArg, (sArg - pBuf), &State.nResult) ;
    
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

/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* elsif command ...                                                            */
/*                                                                              */
/* ---------------------------------------------------------------------------- */

static int CmdElsif  (/*in*/ const char *   sArg)
    {
    int rc = ok ;


    EPENTRY (CmdElsif) ;
    
    if ((State.nCmdType & cmdIf) == 0)
        return rcElseWithoutIf ;
    
        
    if (State.nResult == -1)
        return ok ;

    if (State.nResult == 0)
        {    
        rc = EvalBool ((char *)sArg, (sArg - pBuf), &State.nResult) ;
    
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

/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* else command ...                                                             */
/*                                                                              */
/* ---------------------------------------------------------------------------- */

static int CmdElse  (/*in*/ const char *   sArg)
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
                        

/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* endif command ...                                                            */
/*                                                                              */
/* ---------------------------------------------------------------------------- */

static int CmdEndif (/*in*/ const char *   sArg)
    {
    EPENTRY (CmdEndif) ;

    
    
    State.pStart    = NULL ;

    if ((State.nCmdType & cmdIf) == 0)
        return rcEndifWithoutIf ;

    return ok ;
    }
                        


/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* while command ...                                                            */
/*                                                                              */
/* ---------------------------------------------------------------------------- */

int CmdWhile (/*in*/ const char *   sArg)
    {
    int rc ;
    
    EPENTRY (CmdWhile) ;

    if (State.bProcessCmds == cmdWhile)
        return ok ;

    rc = EvalBool ((char *)sArg, (State.pStart - pBuf), &State.nResult) ;
    
    if (State.nResult) 
        State.bProcessCmds = cmdAll ;
    else
        State.bProcessCmds = cmdWhile ;

    return rc ;
    }

/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* endwhile command ...                                                         */
/*                                                                              */
/* ---------------------------------------------------------------------------- */

static int CmdEndwhile (/*in*/ const char *   sArg)
    {
    int rc = ok ;

    EPENTRY (CmdEndwhile) ;


    if (State.nCmdType != cmdWhile)
        return rcEndwhileWithoutWhile ;

    
    if (State.nResult)
        {
        rc = EvalBool (State.sArg, (State.pStart - pBuf), &State.nResult) ;
    
        if (State.nResult) 
            {
            pCurrPos = State.pStart ;        
            return rc ;
            }
        }

    State.pStart    = NULL ;

    return rc ;
    }


/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* hidden command ...                                                           */
/*                                                                              */
/* ---------------------------------------------------------------------------- */

static int CmdHidden (/*in*/ const char *   sArg)

    {
    char *  pKey ;
    SV *    psv ;
    SV * *  ppsv ;
    HV *    pAddHash = pFormHash ;
    HV *    pSubHash = pInputHash ;
    AV *    pSort    = NULL ;
    HE *    pEntry ;
    I32     l ;
    char *  sArgs ;
    int     nArgLen ;
    char *  sVarName ;
    char    sVar[512] ;
    int     nMax ;
    STRLEN  nKey ;

    EPENTRY (CmdHidden) ;

    
    nArgLen = strlen (sArg) + 1 ;
    if (pArgStack + nArgLen >= ArgStack + sizeof (ArgStack))
        {
        sprintf (errdat1, "nArgLen=%d, pArgStack=%d",nArgLen,  pArgStack - ArgStack) ;
        return rcArgStackOverflow ;
        }
    if (nArgLen > 1)
        {            
        strncpy (sVar, sEvalPackage, sizeof (sVar) - 5) ;
        sVar[nEvalPackage] = ':' ;
        sVar[nEvalPackage+1] = ':' ;
        sVar[sizeof(sVar) - 1] = '\0' ;
        nMax = sizeof(sVar) - nEvalPackage - 3 ;
        
        sArgs = strcpy (pArgStack, sArg) ;
        if (sVarName = strtok (sArgs, ", \t\n"))
            {
            if (*sVarName == '%')
                sVarName++ ;
        
            nArgLen = strlen (sVarName) ;
            strncpy (sVar + nEvalPackage + 2, sVarName, nMax) ;
            
            if ((pAddHash = perl_get_hv ((char *)sVar, FALSE)) == NULL)
                {
                strncpy (errdat1, sVar, sizeof (errdat1) - 1) ;
                return rcHashError ;
                }

            if (sVarName = strtok (NULL, ", \t\n"))
                {
                if (*sVarName == '%')
                    sVarName++ ;
        
                nArgLen = strlen (sVarName) ;
                strncpy (sVar + nEvalPackage + 2, sVarName, nMax) ;
        
                if ((pSubHash = perl_get_hv ((char *)sVar, FALSE)) == NULL)
                    {
                    strncpy (errdat1, sVar, sizeof (errdat1) - 1) ;
                    return rcHashError ;
                    }

                if (sVarName = strtok (NULL, ", \t\n"))
                    {
                    if (*sVarName == '@')
                        sVarName++ ;
        
                    nArgLen = strlen (sVarName) ;
                    strncpy (sVar + nEvalPackage + 2, sVarName, nMax) ;
        
                    if ((pSort = perl_get_av ((char *)sVar, FALSE)) == NULL)
                        {
                        strncpy (errdat1, sVar, sizeof (errdat1) - 1) ;
                        return rcArrayError ;
                        }
                    }
                }
            }
        }
    else
        pSort = pFormArray ;


    oputc ('\n') ;
    if (pSort)
        {
        int n = AvFILL (pSort) + 1 ;
        int i ;

        for (i = 0; i < n; i++)
            {
            ppsv = av_fetch (pSort, i, 0) ;
            if (ppsv && (pKey = SvPV(*ppsv, nKey)) && !hv_exists (pSubHash, pKey, nKey))
                {
                ppsv = hv_fetch (pAddHash, pKey, nKey, 0) ;
                
                if (ppsv)
                    {
                    oputs ("<input type=\"hidden\" name=\"") ;
                    oputs (pKey) ;
                    oputs ("\" value=\"") ;
                    OutputToHtml (SvPV (*ppsv, na)) ;
                    oputs ("\">\n") ;
                    }
                }
            }
        }
    else
        {
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
        }

    return ok ;
    }


/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* var command ...                                                              */
/*                                                                              */
/* ---------------------------------------------------------------------------- */

static int CmdVar (/*in*/ const char *   sArg)

    {
    int    rc ;
    char * pVarName ;
    void * p ;
    SV **  ppSV ;
    int    nFilepos = (sArg - pBuf) ;
    SV *   pSV ;


    EPENTRY (CmdVar) ;
    
    bStrict = HINT_STRICT_REFS | HINT_STRICT_SUBS | HINT_STRICT_VARS ;
    
    /* Already compiled ? */
    
    ppSV = hv_fetch(pCacheHash, (char *)&nFilepos, sizeof (nFilepos), 1) ;  
    if (ppSV == NULL)
        return rcHashError ;
    
    if (SvTRUE(*ppSV))
        return ok ;
    
    sv_setiv (*ppSV, 1) ;
    
    pSV = newSVpvf("package %s ; \n#line %d %s\n use vars qw(%s);\n",sEvalPackage, nSourceline, sSourcefile, sArg) ;
    EvalDirect (pSV) ;
    SvREFCNT_dec(pSV);

    return ok ;
    }

/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* meta tag ...                                                                 */
/*                                                                              */
/*  set http headers on meta http-equiv                                         */
/*                                                                              */
/* ---------------------------------------------------------------------------- */


static int HtmlMeta (/*in*/ const char *   sArg)
    
    {
    const char *  pType ;
    const char *  pContent ;
    int     tlen ;
    int     clen ;
    
    
    
    EPENTRY (HtmlMeta) ;

#ifdef APACHE
    if (pReq == NULL)
        return ok ;
    
    pType = GetHtmlArg (sArg, "HTTP-EQUIV", &tlen) ;
    if (tlen == 0)
        return ok ; /* no http-equiv */

    pContent = GetHtmlArg (sArg, "CONTENT", &clen) ;
    if (clen == 0)
        return ok ; /* missing content for http-equiv */


    if (strnicmp (pType, "content-type", tlen) == 0)
        {
        pReq->content_type = pstrndup(pReq->pool, pContent, clen);
        return ok ;
        }

    table_set(pReq->headers_out, pstrndup (pReq->pool, pType, tlen), 
                                 pstrndup (pReq->pool, pContent, clen));
    
    
#endif
    return ok ;
    }

/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* body tag ...                                                                 */
/*                                                                              */
/* ---------------------------------------------------------------------------- */


static int HtmlBody (/*in*/ const char *   sArg)
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
    
    
    pSV = perl_get_sv (sLogfileURLName, FALSE) ;
    
    if (pSV)
        oputs (SvPV (pSV, na)) ;

    pCurrPos = NULL ;
    	
    return ok ;
    }

/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* A tag ...                                                                    */
/*                                                                              */
/* ---------------------------------------------------------------------------- */


static int HtmlA (/*in*/ const char *   sArg)
    {
    int    rc ;
    char   ArgBuf [2048] ;
    char * pArgBuf = ArgBuf ;

    
    EPENTRY (HtmlA) ;

    if (pCurrEscape == NULL)
        return ok ;
    
    if (*sArg != '\0')
    	{
        struct tCharTrans * pCurrEscapeSave = pCurrEscape ;
        if (bEscMode & escUrl)
            pCurrEscape = Char2Url ;
        
        if ((rc = ScanCmdEvalsInString ((char *)sArg, &pArgBuf, sizeof (ArgBuf))) != ok)
            {
            pCurrEscape = pCurrEscapeSave ;
            return rc ;
            }
        pCurrEscape = pCurrEscapeSave ;
    	}
    else
    	pArgBuf = (char *)sArg ;

    oputs (pCurrTag) ;
    if (*pArgBuf != '\0')
        {
        oputc (' ') ;
        oputs (pArgBuf) ;
        }
    oputc ('>') ;
    
    
    pCurrPos = NULL ;
    	
    return ok ;
    }
/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* table tag ...                                                                */
/* and various list tags ... (dir, menu, ol, ul)                                */
/*                                                                              */
/* ---------------------------------------------------------------------------- */

static int HtmlTable (/*in*/ const char *   sArg)
    {
    EPENTRY (HtmlTable) ;

    oputs (pCurrTag) ;
    if (*sArg != '\0')
        {
        oputc (' ') ;
        oputs (sArg) ;
        }
    oputc ('>') ;
    
    if (nTableStack > nStackMax - 2)
        return rcStackOverflow ;

    TableStack[nTableStack++] = TableState ;

    memset (&TableState, 0, sizeof (TableState)) ;
    TableState.nResult   = 1 ;
    TableState.nTabMode  = nTabMode ;
    TableState.nMaxRow   = nTabMaxRow ;
    TableState.nMaxCol   = nTabMaxCol ;
    TableState.nStackTable = nStack ;
    
    if ((TableState.nTabMode & epTabRow) == epTabRowDef)
        State.pBuf = oBegin () ;

    pCurrPos = NULL ;
    	
    return ok ;
    }

/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* table tag ... (and end of list)                                              */
/*                                                                              */
/* ---------------------------------------------------------------------------- */


static int HtmlEndtable (/*in*/ const char *   sArg)
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

/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* tr tag ...                                                                   */
/*                                                                              */
/* ---------------------------------------------------------------------------- */

static int HtmlRow (/*in*/ const char *   sArg)
    {
    EPENTRY (HtmlRow) ;

            
    if (TableState.nStackTable <= 0 || 
        nStack <= TableState.nStackTable ||
        Stack[TableState.nStackTable].pCmd -> nCmdNo  != cnTable)
        return rcTablerowOutsideOfTable ;

    
    oputs (pCurrTag) ;
    if (*sArg != '\0')
        {
        oputc (' ') ;
        oputs (sArg) ;
        }
    oputc ('>') ;

    TableState.nResult    = 1 ;
    TableState.nCol       = 0 ; 
    TableState.nColUsed   = 0 ; 
    TableState.bHead      = TableState.bRowHead = 0 ;

    if ((TableState.nTabMode & epTabCol) == epTabColDef)
        State.pBuf = oBegin () ;
    
    pCurrPos = NULL ;
    
    return ok ;
    }

/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* /tr tag ...                                                                  */
/*                                                                              */
/* ---------------------------------------------------------------------------- */

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

    if (TableState.bRowHead)    
        Stack[TableState.nStackTable].pStart = pCurrPos ;

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
/*                                                                              */
/* table head tag ...                                                           */
/*                                                                              */
/* ---------------------------------------------------------------------------- */

static int HtmlTableHead (/*in*/ const char *   sArg)
    {
    EPENTRY (HtmlTableHead) ;

    if (TableState.nCol == 0)
        TableState.bHead = TableState.bRowHead = 1 ;
    else
        TableState.bRowHead = 0 ;

    return ok ;
    }


/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* select tag ...                                                               */
/*                                                                              */
/* ---------------------------------------------------------------------------- */

static int HtmlSelect (/*in*/ const char *   sArg)
    {
    const char * pName ;
    int          nlen ;
    SV * *       ppSV ;

    
    EPENTRY (HtmlSelect) ;

    pName = GetHtmlArg (sArg, "NAME", &nlen) ;
    if (nlen == 0)
        {
        if (bDebug & dbgInput)
            lprintf ("[%d]INPU: Select has no name\n", nPid) ; 
        }
    else
        {
        if (pArgStack + nlen + 1 >= ArgStack + sizeof (ArgStack))
           {
           sprintf (errdat1, "nArgLen=%d, pArgStack=%d",nlen + 1,  pArgStack - ArgStack) ;
           return rcArgStackOverflow ;
           }
        State.sArg   = strncpy (pArgStack, pName, nlen) ;
        State.sArg[nlen] = '\0' ;
        pArgStack   += nlen + 1 ;

        ppSV = hv_fetch(pFormHash, (char *)pName, nlen, 0) ;  
        if (ppSV == NULL)
            {
            if (bDebug & dbgInput)
                lprintf ("[%d]INPU: Select %s: no data available in form data\n", nPid, State.sArg) ; 
            }
        else
            {
            STRLEN dlen ;
            char * pData = SvPV (*ppSV, dlen) ;
            char * s = pData ;
            char * p ;
            if (p = strchr (s, cMultFieldSep))
                { /* Multiple values -> put them into a hash */
                HV * pHV = newHV () ;
                SV * pSV ;
                int l ;

                while (p)
                    {
                    hv_store (pHV, pData, p - pData, &sv_undef, 0) ;
                    s = p + 1 ;
                    p = strchr (s, cMultFieldSep) ;
                    }

                l = dlen - (s - pData) ;
                if (l > 0)
                    hv_store (pHV, s, l, &sv_undef, 0) ;
                State.pSV = (SV *)pHV ;
                if (bDebug & dbgInput)
                    lprintf ("[%d]INPU: Select %s = <mult values>\n", nPid, State.sArg) ; 
                }
            else
                {
                State.pSV = *ppSV ;
                SvREFCNT_inc (State.pSV) ;
                if (bDebug & dbgInput)
                    lprintf ("[%d]INPU: Select %s = %s\n", nPid, State.sArg, SvPV(State.pSV, na)) ; 
                }
           }    
        }


    return HtmlTable (sArg) ;
    }


/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* option tag ...                                                               */
/*                                                                              */
/* ---------------------------------------------------------------------------- */


static int HtmlOption (/*in*/ const char *   sArg)
    {
    int           rc ;
    const char *  pVal ;
    const char *  pSelected ;
    int           nlen ;
    int           vlen ;
    int           slen ;
    char *        pName ;
    char *        pData ;
    STRLEN        dlen ;
    int           bSel ;


    EPENTRY (HtmlOption) ;

    pName = State.sArg?State.sArg:"" ;

    if (State.pSV == NULL)
        {
        /*if (bDebug & dbgInput)
            lprintf ("[%d]INPU: <Select>/<Option> no data available\n", nPid) ; */
        
        return ok ; /* no name or no data for select */
        }

    pVal = GetHtmlArg (sArg, "VALUE", &vlen) ;
    if (vlen == 0)    
        {
        if (bDebug & dbgInput)
            lprintf ("[%d]INPU: <Option> for Select %s has no value\n", nPid, pName) ; 
        
        return ok ; /* has no value */
        }

    pSelected = GetHtmlArg (sArg, "SELECTED", &slen) ;
    bSel = 0 ;
    if (SvTYPE (State.pSV) == SVt_PVHV)
        { /* -> Hash -> check if key exists */
        if (hv_exists ((HV *)State.pSV, (char *)pVal, vlen))
            bSel = 1 ;
        }
    else
        {
        pData = SvPV (State.pSV, dlen) ;
        if (dlen == vlen && strncmp (pVal, pData, dlen) == 0)
            bSel = 1 ;
        }
            
    if (bSel)
        { /* -> selected */
        SV * pSV = newSVpv ((char *)pVal, vlen) ;
        if (hv_store (pInputHash, pName, strlen (pName), pSV, 0) == NULL)
            return rcHashError ;

        if (pSelected)
            return ok ;
        else
            {
            oputs (pCurrTag) ;
            if (*sArg != '\0')
                {
                oputc (' ') ;
                oputs (sArg) ;
                }
            oputs (" SELECTED>") ;
            pCurrPos = NULL ; /* nothing more left of html tag */
            return ok ;
            }
        }
    else
        {
        if (pSelected == NULL)
            return ok ;
        else
            {
            oputs (pCurrTag) ;
            oputc (' ') ;
            owrite (sArg, pSelected - sArg, 1) ;
            oputs (pSelected + 8) ;
            oputc ('>') ;
            pCurrPos = NULL ; /* nothing more left of html tag */
            return ok ;
            }
        }
    }


                        

/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* input tag ...                                                                */
/*                                                                              */
/* ---------------------------------------------------------------------------- */


static int HtmlInput (/*in*/ const char *   sArg)
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

    EPENTRY (HtmlInput) ;

    pName = GetHtmlArg (sArg, "NAME", &nlen) ;
    if (nlen == 0)
        {
        if (bDebug & dbgInput)
            lprintf ("[%d]INPU: has no name\n", nPid) ; 
        return ok ; /* no Name */
        }

    if (nlen >= sizeof (sName))
        nlen = sizeof (sName) - 1 ;
    strncpy (sName, pName, nlen) ;
    sName [nlen] = '\0' ;        

    pType = GetHtmlArg (sArg, "TYPE", &tlen) ;
    if (tlen > 0 && (strnicmp (pType, "RADIO", 5) == 0 || strnicmp (pType, "CHECKBOX", 8) == 0))
        bCheck = 1 ;
    else    
        bCheck = 0 ;
    
    
    
    pVal = GetHtmlArg (sArg, "VALUE", &vlen) ;
    if (vlen != 0 && bCheck == 0)    
        {
        pSV = newSVpv ((char *)pVal, vlen) ;

        if (bDebug & dbgInput)
            lprintf ("[%d]INPU: %s already has a value = %s\n", nPid, sName, SvPV (pSV, na)) ; 
        
        if (hv_store (pInputHash, sName, strlen (sName), pSV, 0) == NULL)
            return rcHashError ;
        
        return ok ; /* has already a value */
        }


    ppSV = hv_fetch(pFormHash, (char *)pName, nlen, 0) ;  
    if (ppSV == NULL)
        {
        if (bDebug & dbgInput)
            lprintf ("[%d]INPU: %s: no data available in form data\n", nPid, sName) ; 

        if (vlen != 0)    
            {
            pSV = newSVpv ((char *)pVal, vlen) ;

            if (hv_store (pInputHash, sName, strlen (sName), pSV, 0) == NULL)
                return rcHashError ;
            }

        return ok ; /* no data available */
        }

    pData = SvPV (*ppSV, dlen) ;
    

    if (bCheck)
        {
        if (pVal && vlen == strlen (pData))
            bEqual = strncmp (pData, pVal, vlen) == 0 ; 
        else
            bEqual = 0 ;
        
        pCheck = GetHtmlArg (sArg, "CHECKED", &clen) ;
        if (pCheck)
            {
            if (!bEqual)
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


static int HtmlTextarea (/*in*/ const char *   sArg)
    {
    EPENTRY (HtmlTextarea) ;

    return ok ;
    }

    
/* ---------------------------------------------------------------------------- */
/* /textarea tag ... */
/* */
/* ---------------------------------------------------------------------------- */


static int HtmlEndtextarea (/*in*/ const char *   sArg)
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
