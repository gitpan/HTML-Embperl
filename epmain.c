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

int  bSafeEval = 0 ;  


int  nIOType   = epIOPerl ;

static char sCmdFifo [] = "/local/www/cgi-bin/embperl/cmd.fifo" ;
static char sRetFifo [1024] ;

static char sRetFifoName [] = "__RETFIFO" ;

char cMultFieldSep = '\t' ;  /* Separator if a form filed is multiplie defined */

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
static char sEscModeName   [] = "HTML::Embperl::escmode" ;

static char sNameSpaceHashName [] = "HTML::Embperl::NameSpace" ;
       char sLogfileURLName[] = "HTML::Embperl::LogfileURL" ;
static char sCacheHashName[]  = "HTML::Embperl::cache" ;
static char sPackageName[]    = "HTML::Embperl::package" ;


HV *    pEnvHash ;   /* environement from CGI Script */
HV *    pFormHash ;  /* Formular data */
HV *    pInputHash ; /* Data of input fields */
HV *    pNameSpaceHash ; /* Hash of NameSpace Objects */
AV *    pFormArray ; /* Fieldnames */

HV *    pCacheHash ; /* Hash containing CVs to precompiled subs */

SV *    pNameSpace ; /* Currently active Namespace */


static char *  sCurrPackage ; /* Name of package to eval everything */

char * pBuf ;        /* Buffer which holds the html source file */
char * pCurrPos ;    /* Current position in html file */
char * pCurrStart ;  /* Current start position of html tag / eval expression */
char * pEndPos ;     /* end of html file */
char * pCurrTag ;    /* Current start position of html tag */

/*
   Additional Error info   
*/


char errdat1 [ERRDATLEN]  ;
char errdat2 [ERRDATLEN]  ;


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
        case ok:                        msg ="[%d]ERR:  %d: ok%s%s%c" ; break ;
        case rcStackOverflow:           msg ="[%d]ERR:  %d: Stack Overflow%s%s%c" ; break ;
        case rcArgStackOverflow:        msg ="[%d]ERR:  %d: Argumnet Stack Overflow (%s)%s%c" ; break ;
        case rcStackUnderflow:          msg ="[%d]ERR:  %d: Stack Underflow%s%s%c" ; break ;
        case rcEndifWithoutIf:          msg ="[%d]ERR:  %d: endif without if%s%s%c" ; break ;
        case rcElseWithoutIf:           msg ="[%d]ERR:  %d: else without if%s%s%c" ; break ;
        case rcEndwhileWithoutWhile:    msg ="[%d]ERR:  %d: endwhile without while%s%s%c" ; break ;
        case rcEndtableWithoutTable:    msg ="[%d]ERR:  %d: blockend <%s> does not match blockstart <%s>%c" ; break ;
        case rcTablerowOutsideOfTable:  msg ="[%d]ERR:  %d: <tr> outside of table%s%s%c" ; break ;
        case rcCmdNotFound:             msg ="[%d]ERR:  %d: Unknown Command %s%s%c" ; break ;
        case rcOutOfMemory:             msg ="[%d]ERR:  %d: Out of memory%s%s%c" ; break ;
        case rcPerlVarError:            msg ="[%d]ERR:  %d: Perl variable error %s%s%c" ; break ;
        case rcHashError:               msg ="[%d]ERR:  %d: Perl hash error %s%s%c" ; break ;
        case rcArrayError:              msg ="[%d]ERR:  %d: Perl array error %s%s%c" ; break ;
        case rcFileOpenErr:             msg ="[%d]ERR:  %d: File %s open error: %s%c" ; break ;    
        case rcLogFileOpenErr:          msg ="[%d]ERR:  %d: Logfile %s open error: %s%c" ; break ;    
        case rcMissingRight:            msg ="[%d]ERR:  %d: Missing right %s%s%c" ; break ;
        case rcNoRetFifo:               msg ="[%d]ERR:  %d: No Return Fifo%s%s%c" ; break ;
        case rcMagicError:              msg ="[%d]ERR:  %d: Perl Magic Error%s%s%c" ; break ;
        case rcWriteErr:                msg ="[%d]ERR:  %d: File write Error%s%s%c" ; break ;
        case rcUnknownNameSpace:        msg ="[%d]ERR:  %d: Namespace %s unknown%s%c" ; break ;
        case rcInputNotSupported:       msg ="[%d]ERR:  %d: Input not supported in mod_perl mode%s%s%c" ; break ;
        case rcCannotUsedRecursive:     msg ="[%d]ERR:  %d: Cannot be called recursivly in mod_perl mode%s%s%c" ; break ;
        case rcEndtableWithoutTablerow: msg ="[%d]ERR:  %d: </tr> without <tr>%s%s%c" ; break ;
        case rcEndtextareaWithoutTextarea: msg ="[%d]ERR:  %d: </textarea> without <textarea>%s%s%c" ; break ;
        case rcEvalErr:                 msg ="[%d]ERR:  %d: Error in Perl code %s%s%c" ; break ;
        case rcExecCGIMissing:          msg ="[%d]ERR:  %d: Forbidden %s: Options ExecCGI not set in your Apache configs%s%c" ; break ;
        case rcIsDir:                   msg ="[%d]ERR:  %d: Forbidden %s is a directory%s%c" ; break ;
        case rcXNotSet:                 msg ="[%d]ERR:  %d: Forbidden %s X Bit not set%s%c" ; break ;
        case rcNotFound:                msg ="[%d]ERR:  %d: Not found %s%s%c" ; break ;
        default:                        msg ="[%d]ERR:  %d: Error %s%s%c" ; break ; 
        }
    
    lprintf (msg, nPid , rc, errdat1, errdat2, '\n') ;
#ifdef APACHE
    if (pReq)
        {
        char sText [2048] ;

        if (strlen (msg) + strlen (errdat1) + strlen (errdat2) > sizeof (sText) - 64)
            strcpy (sText, msg) ;
        else
            sprintf (sText, msg, nPid , rc, errdat1, errdat2, ' ') ;
        
        log_error (sText, pReq -> server) ;
        }
    else
#endif
        {
        fprintf (stderr, msg, nPid , rc, errdat1, errdat2, '\n') ;
        fflush (stderr) ;
        }

    errdat1[0] = '\0' ;
    errdat2[0] = '\0' ;
    }



    
/* */
/* Magic */
/* */

static void NewEscMode ()

    {
    if (bEscMode & escHtml)
        pCurrEscape = Char2Html ;
    else if (bEscMode & escUrl)
        pCurrEscape = Char2Url ;
    else 
        pCurrEscape = NULL ;
    }

static void NOP ()

    {
    }


static int notused ;

INTMG (Count, TableState.nCount, TableState.nCountUsed, NOP) 
INTMG (Row, TableState.nRow, TableState.nRowUsed, NOP) 
INTMG (Col, TableState.nCol, TableState.nColUsed, NOP) 
INTMG (MaxRow, nTabMaxRow, notused,  NOP) 
INTMG (MaxCol, nTabMaxCol, notused, NOP) 
INTMG (TabMode, nTabMode, notused, NOP) 
INTMG (EscMode, bEscMode, notused, NewEscMode) 


/* ---------------------------------------------------------------------------- */
/* read form input from http server... */
/* */

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

/* ---------------------------------------------------------------------------- */
/* read input from cgi process... */
/* */


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
                        

/* ---------------------------------------------------------------------------- */
/* get form data when running as cgi script... */
/* */


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


    switch (nType)
        {
        case '+':
            if (State.bProcessCmds == cmdAll)
                {
                EvalTrans (pCurrPos, (pCurrPos - pBuf), &pRet) ;
        
                if (pRet)
                    {
                    OutputToHtml (SvPV (pRet, na)) ;
                    SvREFCNT_dec (pRet) ;
                    }
                }

            p [-2] = nType ;
            pCurrPos = p ;

        
            break ;
        case '-':
            if (State.bProcessCmds == cmdAll)
                {
                EvalTrans (pCurrPos, (pCurrPos - pBuf), &pRet) ;
                if (pRet)
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


    
int ScanCmdEvalsInString (/*in*/  char *   pIn,
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
        /* lprintf ("SCEV nothing sArg = %s\n", pIn) ; */
        *pOut = pIn ; /* Nothing to do */
        return ok ;
        }
    /* lprintf ("SCEV sArg = %s, p = %s\n", pIn, p) ; */

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
/*                                                                              */
/* scan html tag ...                                                            */
/*                                                                              */
/* p points to '<'                                                              */
/*                                                                              */
/* ---------------------------------------------------------------------------- */

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
        /* get end of tag, skip everything inside [+/- ... -/+] */

        char nType = '\0';
        while ((*p != '>' || nType) && *p != '\0')
            {
            if (*p == '[' && (p[1] == '+' || p[1] == '-'))
                nType = *++p;
            else if (nType && *p == nType && p[1] == ']')
                {
                nType = '\0';
                p++ ;
                }

            p++;
            }

        if (*p == '>')
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
    if (rc == 0)
        rc = AddMagic (sEscModeName, &EMBPERL_mvtTabEscMode) ;

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
                   /*in*/ int     nFileSize,
                   /*in*/ HV *    pCache) 




    {
    clock_t startclock = clock () ;
    int     rc ;
    int     len ;
    char *  p ;
    char *  sMode ;
    int     n ;
    char *  c ;
    char *  a ;
    SV **   ppSV ;
    struct stat st ;
    char    nType ;
    SV *    pRet ;
    int     ifd ;
    I32     stsv_count = sv_count ;
    I32     stsv_objcount = sv_objcount ;
    I32     lstsv_count = sv_count ;
    I32     lstsv_objcount = sv_objcount ;

    nPid = getpid () ; /* reget pid, because it could be chaned when loaded with PerlModule */

    EPENTRY (iembperl_req) ;


    bDebug     = bDebugFlags ;
    pCacheHash = pCache ;

    
    if (bDebug)
        {
        time_t t ;
        struct tm * tm ;
        time (&t) ;        
        tm =localtime (&t) ;
        lprintf ("[%d]REQ:  starting... %s\n", nPid, asctime(tm)) ;
        numEvals = 0  ;
        numCacheHits = 0 ;
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
        
    if (bDebug)
        {
        char * p ;

        switch (nIOType)
            {
            case epIOMod_Perl: sMode = "mod_perl"; break ;
            case epIOPerl:     sMode = "Offline"; break ;
            case epIOCGI:      sMode = "CGI-Script"; break ;
            case epIOProcess:  sMode = "Demon"; break ;
            default: sMode = "unknown" ; break ;
            }
        
        lprintf ("[%d]REQ:  %s %s", nPid, (bSafeEval)?"Namespace = ":"No Safe Eval", pNameSpaceName?pNameSpaceName:"") ;
        lprintf (" mode = %s (%d)\n", sMode, nIOType) ;
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
    bEscMode    = escHtml | escUrl ;
    NewEscMode () ;

    /* */
    /* read data from cgi script */
    /* */

    rc = ok ;
    if (av_len (pFormArray) == -1)
        { /* Not already read by perl part */
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


    if (bDebug & dbgEarlyHttpHeader)
        {
#ifdef APACHE
        if (pReq == NULL)
            {
#endif
            if (nIOType != epIOPerl)
                oputs ("Content-type: text/html\n\n") ;

#ifdef APACHE
            }
        else
            {
            send_http_header (pReq) ;
            if (pReq -> header_only)
                {
                CloseOutput () ;
                pReq = NULL ;
                return ok ;
                }
            }
#endif
        }
    else
        {
#ifdef APACHE
        if (pReq == NULL && nIOType != epIOPerl)
#else
        if (nIOType != epIOPerl)
#endif
            oputs ("Content-type: text/html\n\n") ;

        oBegin () ;
        }


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
    /* Process the file... */
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
        

    if (!(bDebug & dbgEarlyHttpHeader))
        {
#ifdef APACHE
        if (pReq)
            {
            set_content_length (pReq, GetContentLength () + 2) ;
            send_http_header (pReq) ;
            }
#endif
    
#ifdef APACHE
        if (pReq == NULL || !pReq -> header_only)
#endif
            oCommit (NULL) ;
        }

    
    
    oputs ("\r\n") ;
    CloseOutput () ;

    if (rc != ok)
        {
        LogError (rc) ;
        }

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
        
        lprintf ("[%d]PERF: mode = %s  input = %s\n", nPid, sMode, sInputfile) ;
#ifdef CLOCKS_PER_SEC
        lprintf ("[%d]PERF: Time: %d ms ", nPid, ((cl - startclock) * 1000 / CLOCKS_PER_SEC)) ;
#else
        lprintf ("[%d]PERF: ", nPid) ;
#endif        
        lprintf ("Evals: %d ", numEvals) ;
        if (bDebug & dbgCacheDisable)
            lprintf ("Cache disabled") ;
        else
            if (numEvals == 0)
                lprintf ("No Evals to cache") ;
            else
                lprintf ("Cache Hits: %d (%d%%)", numCacheHits, numCacheHits * 100 / numEvals) ;

        lprintf ("\n") ;    
        lprintf ("[%d]Request finished. %s. Entry-SVs: %d -OBJs: %d Exit-SVs: %d -OBJs: %d\n", nPid, asctime(tm), stsv_count, stsv_objcount, sv_count, sv_objcount) ;
        }

    if (pBuf)
        _free (pBuf) ;


    FlushLog () ;


#ifdef APACHE
    /* This must be the very very very last !!!!! */
    pReq = NULL ;
#endif

    return ok ;
    }

