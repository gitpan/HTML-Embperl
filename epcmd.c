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

/*// ---------------------------------------------------------------------------- */
/*// Commandtable... */
/*// */


static int CmdIf (/*in*/ const char *   sArg) ;
static int CmdElse (/*in*/ const char *   sArg) ;
static int CmdElsif (/*in*/ const char *   sArg) ;
static int CmdEndif (/*in*/ const char *   sArg) ;

static int CmdWhile (/*in*/ const char *   sArg) ;
static int CmdEndwhile (/*in*/ const char *   sArg) ;
static int CmdHidden (/*in*/ const char *   sArg) ;

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

    

struct tCmd CmdTab [] =
    {
        /* cmdname    function        psuh pop  type         scan save no */
        { "/dir",     HtmlEndtable,     0, 1, cmdTable,         0, 0, cnDir } ,
        { "/dl",      HtmlEndtable,     0, 1, cmdTable,         0, 0, cnDl  } ,
        { "/menu",    HtmlEndtable,     0, 1, cmdTable,         0, 0, cnMenu  } ,
        { "/ol",      HtmlEndtable,     0, 1, cmdTable,         0, 0, cnOl  } ,
        { "/select",  HtmlEndtable,     0, 1, cmdTable,         0, 0, cnSelect  } ,
        { "/table",   HtmlEndtable,     0, 1, cmdTable,         0, 0, cnTable  } ,
        { "/textarea", HtmlEndtextarea, 0, 1, cmdTextarea,      0, 0, cnNop  } ,
        { "/tr",      HtmlEndrow,       0, 1, cmdTablerow,      0, 0, cnTr  } ,
        { "/ul",      HtmlEndtable,     0, 1, cmdTable,         0, 0, cnUl  } ,
        { "body",     HtmlBody,         0, 0, cmdNorm,          1, 0, cnNop   } ,
        { "dir",      HtmlTable,        1, 0, cmdTable,         1, 0, cnDir   } ,
        { "dl",       HtmlTable,        1, 0, cmdTable,         1, 0, cnDl   } ,
        { "else",     CmdElse,          0, 0, cmdIf,            0, 0, cnNop   } ,
        { "elsif",    CmdElsif,         0, 0, cmdIf,            0, 0, cnNop   } ,
        { "endif",    CmdEndif,         0, 1, cmdIf | cmdEndif, 0, 0, cnNop   } ,
        { "endwhile", CmdEndwhile,      0, 1, cmdWhile,         0, 0, cnNop   } ,
        { "hidden",   CmdHidden,        0, 0, cmdNorm,          0, 0, cnNop   } ,
        { "if",       CmdIf,            1, 0, cmdIf | cmdEndif, 0, 0, cnNop   } ,
        { "input",    HtmlInput,        0, 0, cmdNorm,          1, 0, cnNop   } ,
        { "menu",     HtmlTable,        1, 0, cmdTable,         1, 0, cnMenu   } ,
        { "ol",       HtmlTable,        1, 0, cmdTable,         1, 0, cnOl   } ,
        { "option",   HtmlOption,       0, 0, cmdNorm,          1, 0, cnNop   } ,
        { "select",   HtmlSelect,       1, 0, cmdTable,         1, 0, cnSelect   } ,
        { "table",    HtmlTable,        1, 0, cmdTable,         1, 0, cnTable   } ,
        { "textarea", HtmlTextarea,     1, 0, cmdTextarea,      0, 1, cnNop   } ,
        { "th",       HtmlTableHead,    0, 0, cmdNorm,          1, 0, cnNop   } ,
        { "tr",       HtmlRow,          1, 0, cmdTablerow,      1, 0, cnTr   } ,
        { "ul",       HtmlTable,        1, 0, cmdTable,         1, 0, cnUl   } ,
        { "while",    CmdWhile,         1, 0, cmdWhile,         0, 1, cnNop   } ,

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
        strncpy (errdat1, sCmdName, sizeof (errdat1) - 1) ;
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
            State.bProcessCmds = Stack[nStack-1].bProcessCmds ;
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




/*// ---------------------------------------------------------------------------- */
/*// if command ... */
/*// */

static int CmdIf (/*in*/ const char *   sArg)
    {
    int rc = ok ;

    EPENTRY (CmdIf) ;
    
    if (State.bProcessCmds == cmdAll)
        {
        rc = EvalNum ((char *)sArg, (sArg - pBuf), &State.nResult) ;
    
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
        rc = EvalNum ((char *)sArg, (sArg - pBuf), &State.nResult) ;
    
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
                        

/*// ---------------------------------------------------------------------------- */
/*// endif befehl ... */
/*// */

static int CmdEndif (/*in*/ const char *   sArg)
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

    rc = EvalNum ((char *)sArg, (State.pStart - pBuf), &State.nResult) ;
    
    if (State.nResult) 
        State.bProcessCmds = cmdAll ;
    else
        State.bProcessCmds = cmdWhile ;

    return rc ;
    }

/*// ---------------------------------------------------------------------------- */
/*// endwhile command ... */
/*// */

static int CmdEndwhile (/*in*/ const char *   sArg)
    {
    int rc = ok ;

    EPENTRY (CmdEndwhile) ;


    if (State.nCmdType != cmdWhile)
        return rcEndwhileWithoutWhile ;

    
    if (State.nResult)
        {
        rc = EvalNum (State.sArg, (State.pStart - pBuf), &State.nResult) ;
    
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

static int CmdHidden (/*in*/ const char *   sArg)

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
/* body tag ... */
/* */
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
    
    
    pCurrPos = NULL ;
    	
    pSV = perl_get_sv (sLogfileURLName, FALSE) ;
    
    if (pSV)
        oputs (SvPV (pSV, na)) ;

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
    
    pCurrPos = NULL ;
    	
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
    return ok ;
    }


/*// ---------------------------------------------------------------------------- */
/*// /table tag ... (and end of list) */
/*// */

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

/*// ---------------------------------------------------------------------------- */
/*// tr tag ... */
/*// */

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

    pCurrPos = NULL ;
    
    TableState.nResult    = 1 ;
    TableState.nCol       = 0 ; 
    TableState.nColUsed   = 0 ; 
    TableState.bHead      = TableState.bRowHead = 0 ;

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
	char sName [256] ;

        if (bDebug & dbgInput)
            {
            if (nlen >= sizeof (sName))
                nlen = sizeof (sName) - 1 ;
            strncpy (sName, pName, nlen) ;
            sName [nlen] = '\0' ;        
	    }
        ppSV = hv_fetch(pFormHash, (char *)pName, nlen, 0) ;  
        if (ppSV == NULL)
            {
            if (bDebug & dbgInput)
                lprintf ("[%d]INPU: Select %s: no data available in form data\n", nPid, sName) ; 
            }
        else
            {
            State.pSV = *ppSV ;
            SvREFCNT_inc (State.pSV) ;
            if (bDebug & dbgInput)
                lprintf ("[%d]INPU: Select %s = %s\n", nPid, sName, SvPV(State.pSV, na)) ; 
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
    char *        pData ;
    STRLEN        dlen ;


    EPENTRY (HtmlOption) ;

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
            lprintf ("[%d]INPU: <Option> has no value\n", nPid) ; 
        
        return ok ; /* has no value */
        }

    pSelected = GetHtmlArg (sArg, "SELECTED", &slen) ;
    pData = SvPV (State.pSV, dlen) ;
    if (dlen == vlen && strncmp (pVal, pData, dlen) == 0)
        { /* -> selected */
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
        
        return ok ; /* has already a value */
        }


    ppSV = hv_fetch(pFormHash, (char *)pName, nlen, 0) ;  
    if (ppSV == NULL)
        {
        if (bDebug & dbgInput)
            lprintf ("[%d]INPU: %s: no data available in form data\n", nPid, sName) ; 
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

