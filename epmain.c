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


#include "ep.h"
#include "epmacro.h"


/* Version */

static char sVersion [] = VERSION ;


static int  bInitDone = 0 ; /* c part is already initialized */

static char sEnvHashName   [] = "ENV" ;
static char sFormHashName  [] = "HTML::Embperl::fdat" ;
static char sUserHashName  [] = "HTML::Embperl::udat" ;
static char sModHashName  []  = "HTML::Embperl::mdat" ;
static char sFormSplitHashName [] = "HTML::Embperl::fsplitdat" ;
static char sFormArrayName [] = "HTML::Embperl::ffld" ;
static char sInputHashName [] = "HTML::Embperl::idat" ;
static char sErrArrayName  [] = "HTML::Embperl::errors" ;
static char sErrFillName   [] = "HTML::Embperl::errfill" ;
static char sErrStateName  [] = "HTML::Embperl::errstate" ;
static char sHeaderArrayName  [] = "HTML::Embperl::headers" ;
static char sTabCountName  [] = "HTML::Embperl::cnt" ;
static char sTabRowName    [] = "HTML::Embperl::row" ;
static char sTabColName    [] = "HTML::Embperl::col" ;
static char sTabMaxRowName [] = "HTML::Embperl::maxrow" ;
static char sTabMaxColName [] = "HTML::Embperl::maxcol" ;
static char sTabModeName   [] = "HTML::Embperl::tabmode" ;
static char sEscModeName   [] = "HTML::Embperl::escmode" ;


       char sLogfileURLName[] = "HTML::Embperl::LogfileURL" ;
static char sOpcodeMaskName[] = "HTML::Embperl::opcodemask" ;
static char sPackageName[]    = "HTML::Embperl::package" ;
static char sEvalPackageName[]= "HTML::Embperl::evalpackage" ;
static char sDefaultPackageName [] = "HTML::Embperl::DOC::_%d" ;

static char sUIDName [] = "_ID" ;
static char sSetCookie [] = "Set-Cookie" ;
static char sCookieName [] = "EMBPERL_UID" ;


static int      nPackNo = 1 ;       /* Number for createing unique package names */
static tReq *   pReqFree = NULL ;   /* Chain of unused req structures */
tReq     InitialReq ;               /* Initial request - holds default values */
tReq * pCurrReq ;                   /* Set before every eval (NOT thread safe!!) */ 

static HV * pCacheHash ;            /* Hash which holds all cached data (key=>filename, value=>cache hash for file) */

/* */
/* print error */
/* */

char * LogError (/*i/o*/ register req * r,
			/*in*/ int   rc)

    {
    const char * msg ;
    char * sText ;
    SV *   pSV ;
    SV **  ppSV ;
    int    n ;

    
    EPENTRY (LogError) ;
    
    r -> errdat1 [sizeof (r -> errdat1) - 1] = '\0' ;
    r -> errdat2 [sizeof (r -> errdat2) - 1] = '\0' ;

    GetLineNo (r) ;
    
    if (rc != rcPerlWarn)
        r -> bError = 1 ;

    switch (rc)
        {
        case ok:                        msg ="[%d]ERR:  %d: Line %d: ok%s%s" ; break ;
        case rcStackOverflow:           msg ="[%d]ERR:  %d: Line %d: Stack Overflow%s%s" ; break ;
        case rcArgStackOverflow:        msg ="[%d]ERR:  %d: Line %d: Argumnet Stack Overflow (%s)%s" ; break ;
        case rcStackUnderflow:          msg ="[%d]ERR:  %d: Line %d: Stack Underflow%s%s" ; break ;
        case rcEndifWithoutIf:          msg ="[%d]ERR:  %d: Line %d: endif without if%s%s" ; break ;
        case rcElseWithoutIf:           msg ="[%d]ERR:  %d: Line %d: else without if%s%s" ; break ;
        case rcEndwhileWithoutWhile:    msg ="[%d]ERR:  %d: Line %d: endwhile without while%s%s" ; break ;
        case rcEndtableWithoutTable:    msg ="[%d]ERR:  %d: Line %d: blockend <%s> does not match blockstart <%s>" ; break ;
        case rcTablerowOutsideOfTable:  msg ="[%d]ERR:  %d: Line %d: <tr> outside of table%s%s" ; break ;
        case rcCmdNotFound:             msg ="[%d]ERR:  %d: Line %d: Unknown Command %s%s" ; break ;
        case rcOutOfMemory:             msg ="[%d]ERR:  %d: Line %d: Out of memory%s%s" ; break ;
        case rcPerlVarError:            msg ="[%d]ERR:  %d: Line %d: Perl variable error %s%s" ; break ;
        case rcHashError:               msg ="[%d]ERR:  %d: Line %d: Perl hash error, %%%s does not exist%s" ; break ;
        case rcArrayError:              msg ="[%d]ERR:  %d: Line %d: Perl array error , @%s does not exist%s" ; break ;
        case rcFileOpenErr:             msg ="[%d]ERR:  %d: Line %d: File %s open error: %s" ; break ;    
        case rcLogFileOpenErr:          msg ="[%d]ERR:  %d: Line %d: Logfile %s open error: %s" ; break ;    
        case rcMissingRight:            msg ="[%d]ERR:  %d: Line %d: Missing right %s%s" ; break ;
        case rcNoRetFifo:               msg ="[%d]ERR:  %d: Line %d: No Return Fifo%s%s" ; break ;
        case rcMagicError:              msg ="[%d]ERR:  %d: Line %d: Perl Magic Error%s%s" ; break ;
        case rcWriteErr:                msg ="[%d]ERR:  %d: Line %d: File write Error%s%s" ; break ;
        case rcUnknownNameSpace:        msg ="[%d]ERR:  %d: Line %d: Namespace %s unknown%s" ; break ;
        case rcInputNotSupported:       msg ="[%d]ERR:  %d: Line %d: Input not supported in mod_perl mode%s%s" ; break ;
        case rcCannotUsedRecursive:     msg ="[%d]ERR:  %d: Line %d: Cannot be called recursivly in mod_perl mode%s%s" ; break ;
        case rcEndtableWithoutTablerow: msg ="[%d]ERR:  %d: Line %d: </tr> without <tr>%s%s" ; break ;
        case rcEndtextareaWithoutTextarea: msg ="[%d]ERR:  %d: Line %d: </textarea> without <textarea>%s%s" ; break ;
        case rcEvalErr:                 msg ="[%d]ERR:  %d: Line %d: Error in Perl code: %s%s" ; break ;
        case rcExecCGIMissing:          msg ="[%d]ERR:  %d: Line %d: Forbidden %s: Options ExecCGI not set in your Apache configs%s" ; break ;
        case rcIsDir:                   msg ="[%d]ERR:  %d: Line %d: Forbidden %s is a directory%s" ; break ;
        case rcXNotSet:                 msg ="[%d]ERR:  %d: Line %d: Forbidden %s X Bit not set%s" ; break ;
        case rcNotFound:                msg ="[%d]ERR:  %d: Line %d: Not found %s%s" ; break ;
        case rcUnknownVarType:          msg ="[%d]ERR:  %d: Line %d: Type for Variable %s is unknown %s" ; break ;
        case rcPerlWarn:                msg ="[%d]ERR:  %d: Line %d: Warning in Perl code: %s%s" ; break ;
        case rcVirtLogNotSet:           msg ="[%d]ERR:  %d: Line %d: EMBPERL_VIRTLOG must be set, when dbgLogLink is set %s%s" ; break ;
        case rcMissingInput:            msg ="[%d]ERR:  %d: Line %d: Sourcedaten fehlen %s%s" ; break ;
        case rcUntilWithoutDo:          msg ="[%d]ERR:  %d: Line %d: until without do%s%s" ; break ;
        case rcEndforeachWithoutForeach:msg ="[%d]ERR:  %d: Line %d: endforeach without foreach%s%s" ; break ;
        case rcMissingArgs:             msg ="[%d]ERR:  %d: Line %d: Too few arguments%s%s" ; break ;
        case rcNotAnArray:              msg ="[%d]ERR:  %d: Line %d: Second Argument must be array/list%s%s" ; break ;
        case rcCallInputFuncFailed:     msg ="[%d]ERR:  %d: Line %d: Call to Input Function failed: %s%s" ; break ;
        case rcCallOutputFuncFailed:    msg ="[%d]ERR:  %d: Line %d: Call to Output Function failed: %s%s" ; break ;
        default:                        msg ="[%d]ERR:  %d: Line %d: Error %s%s" ; break ; 
        }

    pSV = newSVpvf (msg, r -> nPid , rc, r -> Buf.nSourceline, r -> errdat1, r -> errdat2) ;

    sText = SvPV (pSV, na) ;    
    
    lprintf (r, "%s\n", sText) ;

#ifdef APACHE
    if (r -> pApacheReq)
#ifdef APLOG_ERR
        if (rc != rcPerlWarn)
            aplog_error (APLOG_MARK, APLOG_ERR | APLOG_NOERRNO, r -> pApacheReq -> server, sText) ;
        else
            aplog_error (APLOG_MARK, APLOG_WARNING | APLOG_NOERRNO, r -> pApacheReq -> server, sText) ;
#else
        log_error (sText, r -> pApacheReq -> server) ;
#endif
    else
#endif
        {
#ifdef WIN32
        if (r -> nIOType != epIOCGI)
#endif
            {
            fprintf (stderr, "%s\n", sText) ;
            fflush (stderr) ;
            }
        }
    
    if (rc == rcPerlWarn)
        strncpy (r -> lastwarn, r -> errdat1, sizeof (r -> lastwarn) - 1) ;

    if (r -> pErrArray)
        {
        /*lprintf ("DIS: AvFILL (pErrArray) = %d, nMarker = %d,  nLastErrFill= %d , bLastErrState = %d\n" , AvFILL (pErrArray), nMarker, nLastErrFill, bLastErrState) ;*/
        av_push (r -> pErrArray, pSV) ;
    
        av_store (r -> pErrFill, r -> nMarker, newSViv (AvFILL(r -> pErrArray))) ;
        av_store (r -> pErrState, r -> nMarker, newSViv (r -> bError)) ;
        n = r -> nMarker ;
        while (n-- > 0)
            {
            ppSV = av_fetch (r -> pErrFill, n, 0) ;
            if (ppSV && SvOK (*ppSV))
                break ;
            av_store (r -> pErrFill, n, newSViv (r -> nLastErrFill)) ;
            av_store (r -> pErrState, n, newSViv (r -> bLastErrState)) ;
            }
        
        r -> nLastErrFill  = AvFILL(r -> pErrArray) ;
        r -> bLastErrState = r -> bError ;
        }

    r -> errdat1[0] = '\0' ;
    r -> errdat2[0] = '\0' ;

    return sText ;
    }


/* */
/* begin for error rollback */
/* */

void CommitError (/*i/o*/ register req * r)
    
    {
    int f = AvFILL(r -> pErrArray)  ;
    
    if (f == -1)
        return ; /* no errors -> nothing to do */

    /*lprintf ("DIS: Commit AvFILL (pErrArray) = %d, nMarker = %d,  nLastErrFill= %d , bLastErrState = %d\n" , AvFILL (pErrArray), nMarker, nLastErrFill, bLastErrState) ;*/

    av_store (r -> pErrFill, r -> nMarker, newSViv (f)) ;
    av_store (r -> pErrState, r -> nMarker, newSViv (r -> bError)) ;
    }

    
    
/* */
/* rollback error */
/* */

void RollbackError (/*i/o*/ register req * r)

    {
    SV *  pFill ;
    SV *  pState ;
    SV ** ppSV ;
    I32   f = AvFILL (r -> pErrFill) ;
    int   n ;
    int   i ;

    if (f < r -> nMarker)
        return ;
    
    /*lprintf ("DIS: AvFILL (pErrFill) = %d, nMarker = %d\n" , f, nMarker) ;*/
    
    for (i = f; i > r -> nMarker; i--)
        {
        pFill  = av_pop(r -> pErrFill) ;
        pState = av_pop(r -> pErrState) ;
        SvREFCNT_dec (pFill) ;
        SvREFCNT_dec (pState) ;
        }
    ppSV   = av_fetch(r -> pErrFill, r -> nMarker, 0) ;
    if (ppSV)
        n = SvIV (*ppSV) ;
    else
        n = 0 ;
    ppSV = av_fetch(r -> pErrState, r -> nMarker, 0) ;
    if (ppSV)
        r -> bError = SvIV (*ppSV) ;
    else
        r -> bError = 1 ;
    f = AvFILL (r -> pErrArray) ;
    /*lprintf ("DIS: AvFILL (pErrArray) = %d, n = %d\n" , f, n) ;*/
    if (f > n)
        lprintf (r, "[%d]ERR:  Discard the last %d errormessages, because they occured after the end of a table\n", r -> nPid, f - n) ;
    for (i = f; i > n; i--)
        {
        SvREFCNT_dec (av_pop(r -> pErrArray)) ;
        }

    r -> nLastErrFill  = AvFILL(r -> pErrArray) ;
    r -> bLastErrState = r -> bError ;
    }


    
/* */
/* Magic */
/* */

static void NewEscMode (/*i/o*/ register req * r,
			SV * pSV)

    {
    if (r -> nEscMode & escHtml)
        r -> pNextEscape = Char2Html ;
    else if (r -> nEscMode & escUrl)
        r -> pNextEscape = Char2Url ;
    else 
        r -> pNextEscape = NULL ;

    if (r -> bEscModeSet < 1)
        r -> pCurrEscape = r -> pNextEscape ;

    if (r -> bEscModeSet < 0 && SvOK (pSV))
        r -> bEscModeSet = 1 ;
    }



static int notused ;

INTMG (TabCount, pCurrReq -> TableStack.State.nCount, pCurrReq -> TableStack.State.nCountUsed, ;) 
INTMG (TabRow, pCurrReq -> TableStack.State.nRow, pCurrReq -> TableStack.State.nRowUsed, ;) 
INTMG (TabCol, pCurrReq -> TableStack.State.nCol, pCurrReq -> TableStack.State.nColUsed, ;) 
INTMG (TabMaxRow, pCurrReq -> nTabMaxRow, notused,  ;) 
INTMG (TabMaxCol, pCurrReq -> nTabMaxCol, notused, ;) 
INTMG (TabMode, pCurrReq -> nTabMode, notused, ;) 
INTMG (EscMode, pCurrReq -> nEscMode, notused, NewEscMode (pCurrReq, pSV)) 

OPTMGRD (optDisableVarCleanup      , pCurrReq -> bOptions) ;
OPTMG   (optDisableEmbperlErrorPage, pCurrReq -> bOptions) ;
OPTMGRD (optSafeNamespace          , pCurrReq -> bOptions) ;
OPTMGRD (optOpcodeMask             , pCurrReq -> bOptions) ;
OPTMG   (optRawInput               , pCurrReq -> bOptions) ;
OPTMG   (optSendHttpHeader         , pCurrReq -> bOptions) ;
OPTMGRD (optDisableChdir           , pCurrReq -> bOptions) ;
OPTMG   (optDisableHtmlScan        , pCurrReq -> bOptions) ;
OPTMGRD (optEarlyHttpHeader        , pCurrReq -> bOptions) ;
OPTMGRD (optDisableFormData        , pCurrReq -> bOptions) ;
OPTMG   (optDisableInputScan       , pCurrReq -> bOptions) ;
OPTMG   (optDisableTableScan       , pCurrReq -> bOptions) ;
OPTMG   (optDisableMetaScan        , pCurrReq -> bOptions) ;
OPTMGRD (optAllFormData            , pCurrReq -> bOptions) ;
OPTMGRD (optRedirectStdout         , pCurrReq -> bOptions) ;
OPTMG   (optUndefToEmptyValue      , pCurrReq -> bOptions) ;



OPTMG   (dbgStd          , pCurrReq -> bDebug) ;
OPTMG   (dbgMem          , pCurrReq -> bDebug) ;
OPTMG   (dbgEval         , pCurrReq -> bDebug) ;
OPTMG   (dbgCmd          , pCurrReq -> bDebug) ;
OPTMG   (dbgEnv          , pCurrReq -> bDebug) ;
OPTMG   (dbgForm         , pCurrReq -> bDebug) ;
OPTMG   (dbgTab          , pCurrReq -> bDebug) ;
OPTMG   (dbgInput        , pCurrReq -> bDebug) ;
OPTMG   (dbgFlushOutput  , pCurrReq -> bDebug) ;
OPTMG   (dbgFlushLog     , pCurrReq -> bDebug) ;
OPTMG   (dbgAllCmds      , pCurrReq -> bDebug) ;
OPTMG   (dbgSource       , pCurrReq -> bDebug) ;
OPTMG   (dbgFunc         , pCurrReq -> bDebug) ;
OPTMG   (dbgLogLink      , pCurrReq -> bDebug) ;
OPTMG   (dbgDefEval      , pCurrReq -> bDebug) ;
OPTMG   (dbgCacheDisable , pCurrReq -> bDebug) ;
OPTMG   (dbgWatchScalar  , pCurrReq -> bDebug) ;
OPTMG   (dbgHeadersIn    , pCurrReq -> bDebug) ;
OPTMG   (dbgShowCleanup  , pCurrReq -> bDebug) ;

/* ---------------------------------------------------------------------------- */
/* read form input from http server... */
/* */

static int GetFormData (/*i/o*/ register req * r,
			/*in*/ char * pQueryString,
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

    hv_clear (r -> pFormHash) ;
    hv_clear (r -> pFormSplitHash) ;
    

    if (nLen == 0)
        return ok ;
    
    if ((pMem = _malloc (r, nLen + 4)) == NULL)
        return rcOutOfMemory ;

    p = pMem ;


    nKey = nVal = 0 ;
    pKey = pVal = p ;
    while (1)
        {
        switch (nLen > 0?*pQueryString:'\0')
            {
            case '+':
                pQueryString++ ;
                nLen-- ;
                *p++ = ' ' ;
                break ;
            
            case '%':
                pQueryString++ ;
                nLen-- ;
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
                    nLen-- ;
                    }
                *p++ = num ;
                break ;
            case '=':
                nKey = p - pKey ;
                *p++ = r -> pConf -> cMultFieldSep ;
                nVal = 0 ;
                pVal = p ;
                pQueryString++ ;
                nLen-- ;
                break ;
            case '&':
                pQueryString++ ;
                nLen-- ;
            case '\0':
                nVal = p - pVal ;
                *p++ = '\0' ;
            
                if (nKey > 0 && (nVal > 0 || (r -> bOptions & optAllFormData)))
                    {
                    if (pVal > pKey)
                        pVal[-1] = '\0' ;
                    
                    if (ppSV = hv_fetch (r -> pFormHash, pKey, nKey, 0))
                        { /* Field exists already -> append separator and field value */
                        sv_catpvn (*ppSV, &r ->  pConf -> cMultFieldSep, 1) ;
                        sv_catpvn (*ppSV, pVal, nVal) ;
                        }
                    else
                        { /* New Field -> store it */
                        pSVV = newSVpv (pVal, nVal) ;
                        if (hv_store (r -> pFormHash, pKey, nKey, pSVV, 0) == NULL)
                            {
                            _free (r, pMem) ;
                            return rcHashError ;
                            }

                        }

                    pSVK = newSVpv (pKey, nKey) ;

                    av_push (r -> pFormArray, pSVK) ;
                
                    if (r -> bDebug & dbgForm)
                        lprintf (r, "[%d]FORM: %s=%s\n", r -> nPid, pKey, pVal) ; 

                    }
                pKey = pVal = p ;
                nKey = nVal = 0 ;
                
                if (*pQueryString == '\0')
                    {
                    _free (r, pMem) ;
                    return ok ;
                    }
                
                
                break ;
            default:
                *p++ = *pQueryString++ ;
                nLen-- ;
                break ;
            }
        }

    }

#ifdef comment

/* ---------------------------------------------------------------------------- */
/* read input from cgi process... */
/* */


static int GetInputData_CGIProcess (/*i/o*/ register req * r)

    {
    char *  p ;
    int     rc = ok ;
    int  state = 0 ;
    int  len   = 0 ;
    char sLine [1024] ;
    SV * pSVE ;
    
    EPENTRY (GetInputData_CGIProcess) ;

    hv_clear (r -> pEnvHash) ;


    if (r -> bDebug)
        lprintf (r, "\n[%d]Waiting for Request... SVs: %d OBJs: %d\n", r -> nPid, sv_count, sv_objcount) ;

    if ((rc = OpenInput (r, sCmdFifo)) != ok)
        {
        return rc ;
        }


    if (r -> bDebug)
        lprintf (r, "[%d]Processing Request...\n", r -> nPid) ;
    
    while (igets (sLine, sizeof (sLine)))
        {
        len = strlen (sLine) ; 
        while (len >= 0 && isspace (sLine [--len]))
            ;
        sLine [len + 1] = '\0' ;
        

        if (strcmp (sLine, "----") == 0)
            { state = 1 ; if (r -> bDebug) lprintf (r, "[%d]Environement...\n", r -> nPid) ;}
        else if (strcmp (sLine, "****") == 0)
            { state = 2 ;  if (r -> bDebug) lprintf (r,  "[%d]Formdata...\n", r -> nPid) ;}
        else if (state == 1)
            {
            p = strchr (sLine, '=') ;
            *p = '\0' ;
            p++ ;

            pSVE = newSVpv (p, strlen (p)) ;

            if (hv_store (r -> pEnvHash, sLine, strlen (sLine), pSVE, 0) == NULL)
                {
                return rcHashError ;
                }
            if (r -> bDebug & dbgEnv)
                lprintf (r,  "[%d]ENV:  %s=%s\n", r -> nPid, sLine, p) ;
            }
        else if (state == 2)
            {
            len = atoi (sLine) ;
            if ((p = _malloc (len + 1)) == NULL)
                return rcOutOfMemory ;
            iread (p, len) ;
            p[len] = '\0' ;
            rc = GetFormData (p, len) ;
            _free (p) ;
            break ;
            }
        else
            { if (r -> bDebug) lprintf (r, "[%d]Unknown Input: %s\n", r -> nPid, sLine) ;}

        }
        
    CloseInput () ;
    
    return rc ;
    }
                        
#endif

/* ---------------------------------------------------------------------------- */
/* get form data when running as cgi script... */
/* */


static int GetInputData_CGIScript (/*i/o*/ register req * r)

    {
    char *  p ;
    char *  f ;
    int     rc = ok ;
    STRLEN  len   = 0 ;
    char    sLen [20] ;
    

    EPENTRY (GetInputData_CGIScript) ;

#ifdef APACHE
    if (r -> pApacheReq && (r -> bDebug & dbgHeadersIn))
        {
        int i;
        array_header *hdrs_arr;
        table_entry  *hdrs;

        hdrs_arr = table_elts (r -> pApacheReq->headers_in);
        hdrs = (table_entry *)hdrs_arr->elts;

        lprintf (r,  "[%d]HDR:  %d\n", r -> nPid, hdrs_arr->nelts) ; 
        for (i = 0; i < hdrs_arr->nelts; ++i)
	    if (hdrs[i].key)
                lprintf (r,  "[%d]HDR:  %s=%s\n", r -> nPid, hdrs[i].key, hdrs[i].val) ; 
        }
#endif

    if (r -> bDebug & dbgEnv)
        {
        SV *   psv ;
        HE *   pEntry ;
        char * pKey ;
        I32    l ;
        
        hv_iterinit (r -> pEnvHash) ;
        while (pEntry = hv_iternext (r -> pEnvHash))
            {
            pKey = hv_iterkey (pEntry, &l) ;
            psv  = hv_iterval (r -> pEnvHash, pEntry) ;

                lprintf (r,  "[%d]ENV:  %s=%s\n", r -> nPid, pKey, SvPV (psv, na)) ; 
            }
        }

    sLen [0] = '\0' ;
    GetHashValue (r -> pEnvHash, "CONTENT_LENGTH", sizeof (sLen) - 1, sLen) ;

    if ((len = atoi (sLen)) == 0)
        {
        SV * * ppSV = hv_fetch(r -> pEnvHash, "QUERY_STRING", sizeof ("QUERY_STRING") - 1, 0) ;  
        if (ppSV != NULL)
            {
            p = SvPV (*ppSV ,len) ;
            }
        else
            len = 0 ;
        f = NULL ;
        }
    else
        {
        if ((p = _malloc (r, len + 1)) == NULL)
            return rcOutOfMemory ;

        if ((rc = OpenInput (r, NULL)) != ok)
            {
            _free (r, p) ;
            return rc ;
            }
        iread (r, p, len) ;
        CloseInput (r) ;
        
        p[len] = '\0' ;
        f = p ;
        }
        
    if (r -> bDebug)
        lprintf (r,  "[%d]Formdata... length = %d\n", r -> nPid, len) ;    

    rc = GetFormData (r, p, len) ;
    
    if (f)
        _free (r, f) ;
        
    
    return rc ;
    }


/* ---------------------------------------------------------------------------- */
/* scan commands and evals ([x ... x] sequenz) ... */
/* */
/* p points to '[' */
/* */


    
static int ScanCmdEvals (/*i/o*/ register req * r,
			/*in*/ char *   p)
    
    
    { 
    int     rc ;
    int     len ;
    int     n ;
    char *  c ;
    char *  a ;
    char    nType ;
    SV *    pRet ;
    struct tCmd * pCmd ;


    EPENTRY (ScanCmdEvals) ;
    
    p++ ;

    r -> Buf.pCurrPos = p ;

    if ((nType = *p++) == '\0')
        return ok ;

    r -> Buf.pCurrPos = p ;

    if (nType != '+' && nType != '-' && nType != '$' && nType != '!')
        { /* escape (for [[ -> [) */
        if (r -> CmdStack.State.bProcessCmds == cmdAll)
            {
            if (nType != '[') 
                oputc (r, '[') ;
            oputc (r, nType) ;
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
        sprintf (r -> errdat1, "%c]", nType) ; 
        return rcMissingRight ;
        }
    p [-1] = '\0' ;
    p++ ;


    switch (nType)
        {
        case '+':
            if (r -> CmdStack.State.bProcessCmds == cmdAll)
                {
                r -> bEscModeSet = -1 ;
                r -> pNextEscape = r -> pCurrEscape ;
                rc = EvalTrans (r, r -> Buf.pCurrPos, (r -> Buf.pCurrPos - r -> Buf.pBuf), &pRet) ;
                if (rc != ok && rc != rcEvalErr)
                    return rc ;

                if (pRet)
                    {
                    OutputToHtml (r, SvPV (pRet, na)) ;
                    SvREFCNT_dec (pRet) ;
                    }
                r -> pCurrEscape = r -> pNextEscape ;
                r -> bEscModeSet = 0 ;
                }

            p [-2] = nType ;
            r -> Buf.pCurrPos = p ;

        
            break ;
        case '-':
            if (r -> CmdStack.State.bProcessCmds == cmdAll)
                {
                rc = EvalTrans (r, r -> Buf.pCurrPos, (r -> Buf.pCurrPos - r -> Buf.pBuf), &pRet) ;
                if (rc != ok && rc != rcEvalErr)
                    return rc ;
                if (pRet)
                    SvREFCNT_dec (pRet) ;
                }

            p [-2] = nType ;
            r -> Buf.pCurrPos = p ;

            break ;
        case '!':
            if (r -> CmdStack.State.bProcessCmds == cmdAll)
                {
                rc = EvalTransOnFirstCall (r, r -> Buf.pCurrPos, (r -> Buf.pCurrPos - r -> Buf.pBuf), &pRet) ;
                if (rc != ok && rc != rcEvalErr)
                    return rc ;
                if (pRet)
                    SvREFCNT_dec (pRet) ;
                }

            p [-2] = nType ;
            r -> Buf.pCurrPos = p ;

            break ;
        case '$':
            TransHtml (r, r -> Buf.pCurrPos) ;

            /* skip spaces before command */
            while (*r -> Buf.pCurrPos != '\0' && isspace (*r -> Buf.pCurrPos))
                    r -> Buf.pCurrPos++ ;

            /* c holds the start of the command */
            a = c = r -> Buf.pCurrPos ;
            while (*a != '\0' && isalpha (*a))
                a++ ;

            /* a points to first char after command */

            r -> Buf.pCurrPos = p ;

            if ((rc = SearchCmd (r, c, a-c, a, FALSE, &pCmd)) != ok)
                return rc ;
        
        
            if ((rc = ProcessCmd (r, pCmd, a)) != ok)
                return rc ;
        
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


    
int ScanCmdEvalsInString (/*i/o*/ register req * r,
			/*in*/  char *   pIn,
                          /*out*/ char * * pOut,
                          /*in*/  size_t   nSize,
                          /*out*/ char * * pFree)
    
    
    { 
    int    rc ;
    char * pSaveCurrPos  ;
    char * pSaveCurrStart ;
    char * pSaveEndPos ;
    char * pSaveLineNo ;
    char * p = strchr (pIn, '[');    


    EPENTRY (ScanCmdEvalsInString) ;

    *pFree = NULL ;
    if (p == NULL)
        {
        /* lprintf (r, "SCEV nothing sArg = %s\n", pIn) ; */
        *pOut = pIn ; /* Nothing to do */
        return ok ;
        }
    /* lprintf (r, "SCEV sArg = %s, p = %s\n", pIn, p) ; */

    /* save global vars */
    pSaveCurrPos   = r -> Buf.pCurrPos ;
    pSaveCurrStart = r -> Buf.pCurrStart ;
    pSaveEndPos    = r -> Buf.pEndPos ;
    pSaveLineNo    = r -> Buf.pLineNoCurrPos ;
    if (r -> Buf.pLineNoCurrPos == NULL)
        r -> Buf.pLineNoCurrPos = r -> Buf.pCurrPos ; /* save it for line no calculation */
    

    r -> Buf.pCurrPos = pIn ;
    r -> Buf.pEndPos  = pIn + strlen (pIn) ;

    *pOut = _malloc (r, nSize) ;
    if (*pOut == NULL)
        return rcOutOfMemory ;

    OutputToMemBuf (r, *pOut, nSize) ;

    rc = ok ;
    while (r -> Buf.pCurrPos < r -> Buf.pEndPos && rc == ok)
        {
        /* */
        /* execute [x ... x] and replace them if nessecary */
        /* */
        if (p == NULL || *p == '\0')
            { /* output the rest of html */
            owrite (r, r -> Buf.pCurrPos, r -> Buf.pEndPos - r -> Buf.pCurrPos) ;
            break ;
            }
        
        if (r -> CmdStack.State.bProcessCmds == cmdAll)
            /* output until next cmd */
            owrite (r, r -> Buf.pCurrPos, p - r -> Buf.pCurrPos) ;
        
        if (r -> bDebug & dbgSource)
            {
            char * s = p ;
            char * n ;

            while (*s && isspace (*s))
                s++ ;
            
            if (*s)
                {
                n = strchr (s, '\n') ;
                if (n)
                    lprintf (r, "[%d]SRC: %*.*s\n", r -> nPid, n-s, n-s, s) ;
                else
                    lprintf (r, "[%d]SRC: %70.70s\n", r -> nPid, s) ;

                }
            }        

        
        r -> Buf.pCurrStart = p ;
        rc = ScanCmdEvals (r, p) ;

        p = strchr (r -> Buf.pCurrPos, '[') ;
        }
    
    *pFree = *pOut = OutputToStd (r) ;

    r -> Buf.pCurrPos   = pSaveCurrPos ;
    r -> Buf.pCurrStart = pSaveCurrStart ;
    r -> Buf.pEndPos    = pSaveEndPos ;
    r -> Buf.pLineNoCurrPos = pSaveLineNo ;
    
    return rc ;
    }
            
/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* scan html tag ...                                                            */
/*                                                                              */
/* p points to '<'                                                              */
/*                                                                              */
/* ---------------------------------------------------------------------------- */

static int ScanHtmlTag (/*i/o*/ register req * r,
			/*in*/ char *   p)

    { 
    int  rc ;
    char ec ;
    char ea ;
    char * pec ;
    char * pea ;
    char * pCmd ;
    char * pArg ;
    char * pArgBuf  = NULL ;
    char * pFreeBuf = NULL ;
    struct tCmd * pCmdInfo ;



    EPENTRY (ScanHtmlTag) ;
    
    
    r -> Buf.pCurrTag = p ;     /* save start of html tag */

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

    if ((rc = SearchCmd (r, pCmd, pec - pCmd, "", TRUE, &pCmdInfo)) != ok)
        {
        *pec = ec ;
        oputc (r, *r -> Buf.pCurrTag) ;
        r -> Buf.pCurrPos = r -> Buf.pCurrTag + 1 ;  
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
            if (nType == '\0' && *p == '[' && (p[1] == '+' || p[1] == '-' || p[1] == '$' || p[1] == '!'))
                nType = *++p ;
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

    r -> Buf.pCurrPos = p ;    /* r -> Buf.pCurrPos = first char after whole tag */

    
    if (*pArg != '\0' && pCmdInfo -> bScanArg)
    	{
        if ((rc = ScanCmdEvalsInString (r, (char *)pArg, &pArgBuf, nInitialScanOutputSize, &pFreeBuf)) != ok)
            return rc ;
    	}
    else
    	pArgBuf = pArg ;
    
    
    /* see if knwon html tag and execute */

    if ((rc = ProcessCmd (r, pCmdInfo, pArgBuf)) != ok)
        {
        if (rc == rcCmdNotFound)
            {
              /* only write html tag start char and */
            /*p = pCurrPos = pCurrTag + 1 ;   */    /* check if more to exceute within html tag */
            }
        else
            {
            if (pFreeBuf)
                _free (r, pFreeBuf) ;
            
            return rc ;
            }
        }


    if (p == r -> Buf.pCurrPos && r -> Buf.pCurrPos) /* if CurrPos didn't change write out html tag as it is */
        {
        if (pArg == pArgBuf)
            { /* write unmodified tag */    
            *pec = ec ;              /* restore first char after tag name */
            if (pea)
                *pea = ea ;              /* restore first char after tag arguments */

            oputc (r, *r -> Buf.pCurrTag) ;
            r -> Buf.pCurrPos = r -> Buf.pCurrTag + 1 ;
            }
        else
            { /* write tag with interpreted args */
            oputs (r, r -> Buf.pCurrTag) ;
            oputc (r, ' ') ;
            oputs (r, pArgBuf) ;
            oputc (r, '>') ;
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

    if (r -> Buf.pCurrPos == NULL)
        r -> Buf.pCurrPos = p ; /* html tag is written by command handler */

    if (pFreeBuf)
        _free (r, pFreeBuf) ;

    r -> Buf.pCurrTag = NULL ;

    return ok ;    
    }

    
/* ---------------------------------------------------------------------------- */
/* add magic to integer var */
/* */
/* in  sVarName = Name of varibale */
/* in  pVirtTab = pointer to virtual table */
/* */
/* ---------------------------------------------------------------------------- */

static int AddMagic (/*i/o*/ register req * r,
			/*in*/ char *     sVarName,
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
        LogError (r, rcMagicError) ;
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

int Init        (/*in*/ int           _nIOType,
                 /*in*/ const char *  sLogFile, 
                 /*in*/ int           nDebugDefault)

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

    req * r = &InitialReq ;
    
    pCurrReq = r ;

    r -> nIOType = _nIOType ;

#ifdef APACHE
    r -> pApacheReq = NULL ;
#endif
    r -> bReqRunning = 0 ;
    
    r -> bDebug = nDebugDefault ;
    
    r -> nPid = getpid () ;

    r -> Buf.nSourceline = 1 ;
    r -> Buf.pSourcelinePos = NULL ;    
    r -> Buf.pLineNoCurrPos = NULL ;    

    r -> nEscMode = escStd ;

    if ((rc = OpenLog (r, sLogFile, (r -> bDebug & dbgFunc)?1:0)) != ok)
        { 
        r -> bDebug = 0 ; /* Turn debbuging off, only errors will go to stderr */
        LogError (r, rc) ;
        }

    EPENTRY (iembperl_init) ;

    if (r -> bDebug)
        {
        char * p ;

        switch (r -> nIOType)
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
        
        /* lprintf (r, "[%d]INIT: Embperl %s starting... mode = %s (%d)\n", nPid, sVersion, p, nIOType) ; */
        }


#ifndef APACHE
    if (r -> nIOType == epIOMod_Perl)
        {
        LogError (r, rcNotCompiledForModPerl) ;
        return 1 ;
        }
#endif

    if (bInitDone)
        return ok ; /* the rest was alreay done */

    if ((r -> pFormHash = perl_get_hv (sFormHashName, TRUE)) == NULL)
        {
        LogError (r, rcHashError) ;
        return 1 ;
        }

    if ((r -> pUserHash = perl_get_hv (sUserHashName, TRUE)) == NULL)
        {
        LogError (r, rcHashError) ;
        return 1 ;
        }

    if ((r -> pModHash = perl_get_hv (sModHashName, TRUE)) == NULL)
        {
        LogError (r, rcHashError) ;
        return 1 ;
        }


    if ((r -> pFormSplitHash = perl_get_hv (sFormSplitHashName, TRUE)) == NULL)
        {
        LogError (r, rcHashError) ;
        return 1 ;
        }

    if ((r -> pFormArray = perl_get_av (sFormArrayName, TRUE)) == NULL)
        {
        LogError (r, rcArrayError) ;
        return 1 ;
        }

    /*
    if ((r -> pErrArray = perl_get_av (sErrArrayName, TRUE)) == NULL)
        {
        LogError (r, rcArrayError) ;
        return 1 ;
        }

    if ((r -> pErrFill = perl_get_av (sErrFillName, TRUE)) == NULL)
        {
        LogError (r, rcArrayError) ;
        return 1 ;
        }

    if ((r -> pErrState = perl_get_av (sErrStateName, TRUE)) == NULL)
        {
        LogError (r, rcArrayError) ;
        return 1 ;
        }

    if ((pHeaderArray = perl_get_av (sHeaderArrayName, TRUE)) == NULL)
        {
        LogError (r, rcArrayError) ;
        return 1 ;
        }
    */

    if ((r -> pInputHash = perl_get_hv (sInputHashName, TRUE)) == NULL)
        {
        LogError (r,  rcHashError) ;
        return 1 ;
        }

    if ((r -> pEnvHash = perl_get_hv (sEnvHashName, TRUE)) == NULL)
        {
        LogError (r,  rcHashError) ;
        return 1 ;
        }

    if (!(pCacheHash = newHV ()))
        {
        LogError (r,  rcHashError) ;
        return 1 ;
        }

    if (!(r -> pErrFill = newAV ()))
        {
        LogError (r, rcArrayError) ;
        return 1 ;
        }
    
    if (!(r -> pErrState = newAV ()))
        {
        LogError (r, rcArrayError) ;
        return 1 ;
        }
    
    if (!(r -> pErrArray = newAV ()))
        {
        LogError (r, rcArrayError) ;
        return 1 ;
        }
    
    
    rc = 0 ;

    ADDINTMG (TabCount) ;
    ADDINTMG (TabRow) ;
    ADDINTMG (TabCol) ;
    ADDINTMG (TabMaxRow) ;
    ADDINTMG (TabMaxCol) ;
    ADDINTMG (TabMode) ;
    ADDINTMG (EscMode) ;
    
    ADDOPTMG (optDisableVarCleanup      ) ;
    ADDOPTMG (optDisableEmbperlErrorPage) ;
    ADDOPTMG (optSafeNamespace          ) ;
    ADDOPTMG (optOpcodeMask             ) ;
    ADDOPTMG (optRawInput               ) ;
    ADDOPTMG (optSendHttpHeader         ) ;
    ADDOPTMG (optDisableChdir           ) ;
    ADDOPTMG (optDisableHtmlScan        ) ;
    ADDOPTMG (optEarlyHttpHeader        ) ;
    ADDOPTMG (optDisableFormData        ) ;
    ADDOPTMG (optDisableInputScan       ) ;
    ADDOPTMG (optDisableTableScan       ) ;
    ADDOPTMG (optDisableMetaScan        ) ;
    ADDOPTMG (optAllFormData            ) ;
    ADDOPTMG (optRedirectStdout         ) ;
    ADDOPTMG (optUndefToEmptyValue      ) ;

    ADDOPTMG   (dbgStd         ) ;
    ADDOPTMG   (dbgMem         ) ;
    ADDOPTMG   (dbgEval        ) ;
    ADDOPTMG   (dbgCmd         ) ;
    ADDOPTMG   (dbgEnv         ) ;
    ADDOPTMG   (dbgForm        ) ;
    ADDOPTMG   (dbgTab         ) ;
    ADDOPTMG   (dbgInput       ) ;
    ADDOPTMG   (dbgFlushOutput ) ;
    ADDOPTMG   (dbgFlushLog    ) ;
    ADDOPTMG   (dbgAllCmds     ) ;
    ADDOPTMG   (dbgSource      ) ;
    ADDOPTMG   (dbgFunc        ) ;
    ADDOPTMG   (dbgLogLink     ) ;
    ADDOPTMG   (dbgDefEval     ) ;
    ADDOPTMG   (dbgCacheDisable) ;
    ADDOPTMG   (dbgWatchScalar ) ;
    ADDOPTMG   (dbgHeadersIn   ) ;
    ADDOPTMG   (dbgShowCleanup ) ;
   
    bInitDone = 1 ;

    return rc ;
    }

/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* clean up embperl module                                                      */
/*                                                                              */
/* ---------------------------------------------------------------------------- */

int Term ()

    {
    req * r = pCurrReq ;

    EPENTRY (iembperl_term) ;
    
    if (!bInitDone)
        return ok ; 

    CloseLog (r) ;
    CloseOutput (r) ;
    
    return ok ;
    }



int ResetHandler (/*in*/ SV * pApacheReqSV)
    {
#ifdef APACHE
    request_rec * pReq = (request_rec *)SvIV((SV*)SvRV(pApacheReqSV));
    pReq -> handler = NULL ;
#endif

    return ok ;
    }


/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* Localise op_mask then opmask_add()                                           */
/*                                                                              */
/* Just copied from Opcode.xs                                                   */
/*                                                                              */
/* ---------------------------------------------------------------------------- */


static void
opmask_addlocal(SV *   opset,
                char * op_mask_buf) 
    {
    char *orig_op_mask = op_mask;
    int i,j;
    char *bitmask;
    STRLEN len;
    int myopcode  = 0;
    int opset_len = (maxo + 7) / 8 ;

    SAVEPPTR(op_mask);
    op_mask = &op_mask_buf[0];
    if (orig_op_mask)
	Copy(orig_op_mask, op_mask, maxo, char);
    else
	Zero(op_mask, maxo, char);


    /* OPCODES ALREADY MASKED ARE NEVER UNMASKED. See opmask_addlocal()	*/

    bitmask = SvPV(opset, len);
    for (i=0; i < opset_len; i++)
        {
	U16 bits = bitmask[i];
	if (!bits)
            {	/* optimise for sparse masks */
	    myopcode += 8;
	    continue;
	    }
	for (j=0; j < 8 && myopcode < maxo; )
	    op_mask[myopcode++] |= bits & (1 << j++);
        }
    }

/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* Setup Configuration specficy data                                            */
/*                                                                              */
/* ---------------------------------------------------------------------------- */


tConf * SetupConfData   (/*in*/ HV *   pReqInfo,
                         /*in*/ SV *   pOpcodeMask)

    {
    tConf *  pConf = malloc (sizeof (tConf)) ;
    
    if (!pConf)
        return NULL ;

    pConf -> bDebug =	    GetHashValueInt (pReqInfo, "debug", pCurrReq -> pConf?pCurrReq -> pConf -> bDebug:pCurrReq -> bDebug) ;	    /* Debugging options */
    pConf -> bOptions =	    GetHashValueInt (pReqInfo, "options",  pCurrReq -> pConf?pCurrReq -> pConf -> bOptions:pCurrReq -> bOptions) ;  /* Options */
    pConf -> nEscMode =	    GetHashValueInt (pReqInfo, "escmode",  pCurrReq -> pConf?pCurrReq -> pConf -> nEscMode:escStd) ;  /* EscMode */
    pConf -> sPackage =	    sstrdup (GetHashValueStr (pReqInfo, "package", NULL)) ;         /* Packagename */
    pConf -> sLogFilename = sstrdup (GetHashValueStr (pReqInfo, "log",  NULL)) ;            /* name of logfile */
    pConf -> sVirtLogURI  = sstrdup (GetHashValueStr (pReqInfo, "virtlog",  NULL)) ;        /* name of logfile */
    pConf -> pOpcodeMask  = pOpcodeMask ;                                                   /* Opcode mask (if any) */
    pConf -> cMultFieldSep = '\t' ;

    return pConf ;
    }

/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* Free Configuration specficy data                                             */
/*                                                                              */
/* ---------------------------------------------------------------------------- */


void FreeConfData       (/*in*/ tConf *   pConf)

    {
    if (!pConf)
        return ;

    if (pConf -> sPackage)
        free (pConf -> sPackage) ;
    
    if (pConf -> sLogFilename)
        free (pConf -> sLogFilename) ;

    if (pConf -> sVirtLogURI)
        free (pConf -> sVirtLogURI) ;

    free (pConf) ;
    }


/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* Setup File specficy data                                                     */
/*                                                                              */
/* ---------------------------------------------------------------------------- */


tFile * SetupFileData   (/*i/o*/ register req * r,
                         /*in*/  char *  sSourcefile,
                         /*in*/  long    mtime,
                         /*in*/  long    nFilesize,
                         /*in*/  tConf * pConf)

    {
    SV * *      ppSV ;
    tFile *     f ;
    char txt [sizeof (sDefaultPackageName) + 50] ;
    
    EPENTRY (SetupFileData) ;

    /* Have we seen this sourcefile already ? */
    ppSV = hv_fetch(pCacheHash, (char *)sSourcefile, strlen (sSourcefile), 0) ;  
    if (ppSV && *ppSV)
        {
        f = (tFile *)SvIV((SV*)SvRV(*ppSV)) ;
        
        if (f -> mtime != mtime)
            {
            hv_clear (f -> pCacheHash) ;
        
            if (r -> bDebug)
                lprintf (r, "[%d]MEM: Reload %s in %s\n", r -> nPid,  sSourcefile, f -> sCurrPackage) ;
            }
        }
    else
        { /* create new file structure */
        if ((f = malloc (sizeof (*f))) == NULL)
            return NULL ;

        f -> sSourcefile = sstrdup (sSourcefile) ; /* Name of sourcefile */
        f -> mtime       = mtime ;	 /* last modification time of file */
        f -> nFilesize   = nFilesize ;	 /* size of File */

        f -> pCacheHash  = newHV () ;    /* Hash containing CVs to precompiled subs */

        if (pConf -> sPackage)
            f -> sCurrPackage = strdup (pConf -> sPackage) ; /* Package of file  */
        else
            {
            sprintf (txt, sDefaultPackageName, nPackNo++ ) ;
            f -> sCurrPackage = strdup (txt) ; /* Package of file  */
            }
        f -> nCurrPackage = strlen (f -> sCurrPackage); /* Package of file (length) */

        hv_store(pCacheHash, (char *)sSourcefile, strlen (sSourcefile), newRV_noinc (newSViv ((IV)f)), 0) ;  
    
        if (r -> bDebug)
            lprintf (r, "[%d]MEM: Load %s in %s\n", r -> nPid,  sSourcefile, f -> sCurrPackage) ;
        }

    return f ;
    }

/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* Setup Request                                                                */
/*                                                                              */
/* ---------------------------------------------------------------------------- */


tReq * SetupRequest (/*in*/ SV *    pApacheReqSV,
                     /*in*/ char *  sSourcefile,
                     /*in*/ long    mtime,
                     /*in*/ long    nFilesize,
                     /*in*/ char *  sOutputfile,
                     /*in*/ tConf * pConf,
                     /*in*/ int     nIOType,
                     /*in*/ SV *    pIn,
                     /*in*/ SV *    pOut)

    {
    int     rc ;
    tReq *  r ;
    char *  sMode ;
    tFile * pFile ;

    dTHR ;

    EPENTRY (SetupRequest) ;
	
    tainted         = 0 ;

    if ((rc = OpenLog (pCurrReq, NULL, 2)) != ok)
        { 
        LogError (pCurrReq, rc) ;
        }

    if (pReqFree)
        {
        r = pReqFree ;
        pReqFree = pReqFree -> pNext ;
        memcpy (r, pCurrReq, (char *)&r -> zeroend - (char *)r) ;
        r -> pNext = NULL ;
        }
    else
        {
        if ((r = malloc (sizeof (tReq))) == NULL)
            return NULL ;
        
        memcpy (r, pCurrReq, sizeof (*r)) ;
        }
    
    r -> bSubReq = !(pCurrReq == &InitialReq) ;

    r -> pLastReq = pCurrReq ;
    pCurrReq = r ;
    
#ifdef APACHE
    if (SvROK (pApacheReqSV))
        r -> pApacheReq = (request_rec *)SvIV((SV*)SvRV(pApacheReqSV));
    else
        r -> pApacheReq = NULL ;
    r -> pApacheReqSV = pApacheReqSV ;
#endif

    r -> startclock      = clock () ;
    r -> stsv_count      = sv_count ;
    r -> stsv_objcount   = sv_objcount ;
    r -> lstsv_count     = sv_count ;
    r -> lstsv_objcount  = sv_objcount ;

    r -> nPid            = getpid () ; /* reget pid, because it could be chaned when loaded with PerlModule */
    r -> bDebug          = pConf -> bDebug ;
    if (rc != ok)
        r -> bDebug = 0 ; /* Turn debbuging off, only errors will go to stderr if logfile not open */
    r -> bOptions        = pConf -> bOptions ;
    /*r -> nIOType         = InitialReq.nIOType ;*/

    r -> pConf           = pConf ;

    if ((pFile = SetupFileData (r, sSourcefile, mtime, nFilesize, pConf)) == NULL)
        return NULL ;
    
    if (r -> bSubReq && sOutputfile[0] == 1 && r -> pLastReq && !SvROK (pOut))
        {
        r -> sOutputfile      = r -> pLastReq -> sOutputfile ;
        r -> bAppendToMainReq = TRUE ;
        }
    else
        {
        if (sOutputfile[0] == 1)
            r -> sOutputfile      = "" ;
        else
            r -> sOutputfile      = sOutputfile ;
        r -> bAppendToMainReq = FALSE ;
        }
    
    r -> bReqRunning     = 1 ;

    r -> Buf.pFile = pFile ;

    r -> pOutData        = pOut ;
    r -> pInData         = pIn ;

    
    r -> CmdStack.State.nCmdType      = cmdNorm ;
    r -> CmdStack.State.bProcessCmds  = cmdAll ;
    r -> HtmlStack.State.nCmdType      = cmdNorm ;
    r -> HtmlStack.State.bProcessCmds  = cmdAll ;
    r -> nTabMode        = epTabRowDef | epTabColDef ;
    r -> nTabMaxRow      = 100 ;
    r -> nTabMaxCol      = 10 ;


    r -> nEscMode = pConf -> nEscMode ;
    NewEscMode (r, NULL) ;
    r -> bEscModeSet = 0 ;


    if (r -> bOptions & optSafeNamespace)
	{
        r -> Buf.sEvalPackage = "main" ;
        r -> Buf.nEvalPackage = sizeof ("main") - 1 ;
        }
    else
	{
        r -> Buf.sEvalPackage = r -> Buf.pFile -> sCurrPackage ;
        r -> Buf.nEvalPackage = r -> Buf.pFile -> nCurrPackage ;
        }
    
    r -> Buf.nSourceline = 1 ;
    r -> Buf.pSourcelinePos = NULL ;    
    r -> Buf.pLineNoCurrPos = NULL ;    

    r -> bStrict       = FALSE ;

    r -> errdat1 [0]  = '\0' ; /* Additional error information */
    r -> errdat2 [0]  = '\0' ;
    r -> lastwarn [0] = '\0' ; /* last warning */
    
    if (!r -> bSubReq)
        {
        r -> bError = FALSE ;
        av_clear (r -> pErrFill) ;
        av_clear (r -> pErrState) ;
        av_clear (r -> pErrArray) ;
        r -> nLastErrFill  = AvFILL(r -> pErrArray) ;
        r -> bLastErrState = r -> bError ;
        r -> nLogFileStartPos = GetLogFilePos (r) ;
        }


    if (r -> bDebug)
        {
        time_t t ;
        struct tm * tm ;
        time (&t) ;        
        tm =localtime (&t) ;
        lprintf (r, "[%d]REQ:  Embperl %s starting... %s\n", r -> nPid,  sVersion, asctime(tm)) ;
        r -> numEvals = 0  ;
        r -> numCacheHits = 0 ;
        }
    
    /* for backwards compability */
    if (r -> bDebug & dbgEarlyHttpHeader)
        r -> bOptions |= optEarlyHttpHeader ;
        
    if (r -> bDebug)
        {
        char * p ;

        switch (r -> nIOType)
            {
            case epIOMod_Perl: sMode = "mod_perl"; break ;
            case epIOPerl:     sMode = "Offline"; break ;
            case epIOCGI:      sMode = "CGI-Script"; break ;
            case epIOProcess:  sMode = "Demon"; break ;
            default: sMode = "unknown" ; break ;
            }
        
        lprintf (r, "[%d]REQ:  %s  %s  ", r -> nPid, (r -> bOptions & optSafeNamespace)?"SafeNamespace":"No Safe Eval", (r -> bOptions & optOpcodeMask)?"OpcodeMask":"All Opcode allowed") ;
        lprintf (r, " mode = %s (%d)\n", sMode, r -> nIOType) ;
        lprintf (r, "[%d]REQ:  Package = %s\n", r -> nPid, r -> Buf.pFile -> sCurrPackage) ;
        }

    return r ;
    }


/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* Free Request                                                                 */
/*                                                                              */
/* ---------------------------------------------------------------------------- */


void FreeRequest (/*i/o*/ register req * r)

    {
    FreeConfData (r -> pConf) ;
    r -> pConf = NULL ;

    if (!r -> bAppendToMainReq && r -> ofd)
        CloseOutput (r) ; /* just to be sure */
    
    if (r -> bSubReq)
        {
        tReq * l = r -> pLastReq ;

        l -> bError      = r -> bError ;
        l -> nLastErrFill= r -> nLastErrFill ;
        l -> bLastErrState= r -> bLastErrState ;
        }

    pCurrReq = r -> pLastReq ;

    r -> pNext = pReqFree ;
    pReqFree = r ;
    }


                     
/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* Setup Safe Namespace                                                         */
/*                                                                              */
/* ---------------------------------------------------------------------------- */


static void SetupSafeNamespace (/*i/o*/ register req * r)

    {                 
    GV *    gv;

    dTHR ;

	/* The following is borrowed from Opcode.xs */

    if (r -> bOptions & optOpcodeMask)
        opmask_addlocal(r -> pConf -> pOpcodeMask, r -> op_mask_buf);

        
    if (r -> bOptions & optSafeNamespace)
        {
        save_aptr(&endav);
        endav = (AV*)sv_2mortal((SV*)newAV()); /* ignore END blocks for now	*/

        save_hptr(&defstash);		/* save current default stack	*/
        /* the assignment to global defstash changes our sense of 'main'	*/
        defstash = gv_stashpv(r -> Buf.pFile -> sCurrPackage, GV_ADDWARN); /* should exist already	*/

        if (r -> bDebug)
            lprintf (r, "[%d]REQ:  switch to safe namespace %s\n", r -> nPid, r -> Buf.pFile -> sCurrPackage) ;


        /* defstash must itself contain a main:: so we'll add that now	*/
        /* take care with the ref counts (was cause of long standing bug)	*/
        /* XXX I'm still not sure if this is right, GV_ADDWARN should warn!	*/
        gv = gv_fetchpv("main::", GV_ADDWARN, SVt_PVHV);
        sv_free((SV*)GvHV(gv));
        GvHV(gv) = (HV*)SvREFCNT_inc(defstash);
        }
    }

    
/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* Start the output stream                                                      */
/*                                                                              */
/* ---------------------------------------------------------------------------- */

    
static int StartOutput (/*i/o*/ register req * r)

    {
    int rc ;
    SV * pOutData  = r -> pOutData ;
    int  bOutToMem = SvROK (pOutData) ;
    
    if (!bOutToMem)
        {
        if (!r -> bAppendToMainReq)
            {
            if ((rc = OpenOutput (r, r -> sOutputfile)) != ok)
                return rc ;
            }
        else
            OutputToStd (r) ;
        }
    else
        { /* only reset output buffers */
        r -> ofd = NULL ;
        OpenOutput (r, NULL) ;
        }


#ifdef APACHE
    if (r -> pApacheReq && r -> pApacheReq -> main)
    	r -> bOptions |= optEarlyHttpHeader ; /* do not direct output to memory on internal redirect */
#endif
    if (bOutToMem)
    	r -> bOptions &= ~optEarlyHttpHeader ;

    if (r -> bSubReq)
    	r -> bOptions &= ~optSendHttpHeader ;


    if (r -> bOptions & optEarlyHttpHeader)
        {
#ifdef APACHE
        if (r -> pApacheReq == NULL)
            {
#endif
            if (r -> nIOType != epIOPerl && (r -> bOptions & optSendHttpHeader))
                oputs (r, "Content-type: text/html\n\n") ;

#ifdef APACHE
            }
        else
            {
            if (r -> pApacheReq -> main == NULL && (r -> bOptions & optSendHttpHeader))
            	send_http_header (r -> pApacheReq) ;
#ifndef WIN32
            mod_perl_sent_header(r -> pApacheReq, 1) ;
#endif
            if (r -> pApacheReq -> header_only)
            	{
	        if (!r -> bAppendToMainReq)
                    CloseOutput (r) ;
            	return ok ;
    	        }
            }
#endif
        }
    else
        {
        if (r -> nIOType == epIOCGI && (r -> bOptions & optSendHttpHeader))
            oputs (r, "Content-type: text/html\n\n") ;
            
        oBegin (r) ;
        }

    return ok ;
    }

/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* End the output stream                                                        */
/*                                                                              */
/* ---------------------------------------------------------------------------- */


static int EndOutput (/*i/o*/ register req * r,
		      /*in*/ int    rc,
                      /*in*/ SV *   pOutData) 
                      

    {
    SV * pOut ;
    int  bOutToMem = SvROK (pOutData) ;

    
    if (rc != ok ||  r -> bError)
        { /* --- generate error page if necessary --- */
#ifdef APACHE
        if (r -> pApacheReq)
            r -> pApacheReq -> status = 500 ;
#endif
        if (!(r -> bOptions & optDisableEmbperlErrorPage) && !r -> bAppendToMainReq)
            {
            dSP;                            /* initialize stack pointer      */

            oRollbackOutput (r, NULL) ; /* forget everything outputed so far */
            oBegin (r) ;

            PUSHMARK(sp);                   /* remember the stack pointer    */
            XPUSHs(r -> pReqSV) ;            /* push pointer to obeject */
            PUTBACK;
            perl_call_method ("SendErrorDoc", G_DISCARD) ; /* call the function             */
            }
        }
    

    if (!(r -> bOptions & optEarlyHttpHeader) && (r -> bOptions & optSendHttpHeader) && !bOutToMem)
        {  /* --- send http headers if not alreay done --- */
#ifdef APACHE
        if (r -> pApacheReq)
            {
            if (!r -> bAppendToMainReq)
                {                    
				SV ** ppSVID ;
                
				set_content_length (r -> pApacheReq, GetContentLength (r) + 2) ;

                ppSVID = hv_fetch (r -> pUserHash, sUIDName, sizeof (sUIDName) - 1, 0) ;
				if (ppSVID && *ppSVID)
					{
					STRLEN ulen ;
					char * pUID = SvPV (*ppSVID, ulen) ;
				    if (ulen > 0)
						{
						table_set(r -> pApacheReq->headers_out, sSetCookie, 
							pstrcat (r -> pApacheReq->pool, sCookieName, "=", pUID, NULL));
							
						}
					}
				
				
				send_http_header (r -> pApacheReq) ;
#ifndef WIN32
                mod_perl_sent_header(r -> pApacheReq, 1) ;
#endif
                if (r -> bDebug & dbgHeadersIn)
        	    {
        	    int i;
        	    array_header *hdrs_arr;
        	    table_entry  *hdrs;

        	    hdrs_arr = table_elts (r -> pApacheReq->headers_out);
	            hdrs = (table_entry *)hdrs_arr->elts;

        	    lprintf (r,  "[%d]HDR:  %d\n", r -> nPid, hdrs_arr->nelts) ; 
	            for (i = 0; i < hdrs_arr->nelts; ++i)
		        if (hdrs[i].key)
                	    lprintf (r,  "[%d]HDR:  %s=%s\n", r -> nPid, hdrs[i].key, hdrs[i].val) ; 
        	    }
                }
            }
        else
#endif
           { 
            /*
           if (r -> nIOType == epIOCGI)
                {            
                char txt[100] ;

                oputs ("Content-type: text/html\n") ;
                sprintf (txt, "Content-Length: %d\n", GetContentLength () + 2) ;
                oputs (txt) ;
                oputs ("\n") ;
                }
            */
           }
       }

    /* --- output the content if not alreay done --- */

    if (bOutToMem)
        pOut = SvRV (pOutData) ;

#ifdef APACHE
    if ((r -> pApacheReq == NULL || !r -> pApacheReq -> header_only) && !(r -> bOptions & optEarlyHttpHeader))
#else
    if (!(r -> bOptions & optEarlyHttpHeader))
#endif
        {
        oputs (r, "\r\n") ;
        if (bOutToMem)
            {
            char * pData ;
            int    l = GetContentLength (r) + 1 ;
            
            sv_setpv (pOut, "") ;
            SvGROW (pOut, l) ;
            pData = SvPVX (pOut) ;
            oCommitToMem (r, NULL, pData) ;
            SvCUR_set (pOut, l - 1) ;
            }
        else
            {
            if (r -> bAppendToMainReq)
                {
                tReq * l = r -> pLastReq ;

                l -> pFirstBuf   = r -> pFirstBuf  ;
                l -> pLastBuf    = r -> pLastBuf   ;
                l -> pFreeBuf    = r -> pFreeBuf   ;
                l -> pLastFreeBuf= r -> pLastFreeBuf ;
                }
            else
                oCommit (r, NULL) ;
            }
        }
    else
        {
        oRollbackOutput (r, NULL) ;
        if (bOutToMem)
            sv_setsv (pOut, &sv_undef) ;
        }    

    if (!r -> bAppendToMainReq)
        CloseOutput (r) ;

    return ok ;
    }
    
/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* Reset Request                                                                */
/*                                                                              */
/* ---------------------------------------------------------------------------- */


static int ResetRequest (/*i/o*/ register req * r,
			/*in*/ char *  sInputfile)

    {
    if (r -> bDebug)
        {
        clock_t cl = clock () ;
        time_t t ;
        struct tm * tm ;
        
        time (&t) ;        
        tm =localtime (&t) ;
        
        lprintf (r, "[%d]PERF: input = %s\n", r -> nPid, sInputfile) ;
#ifdef CLOCKS_PER_SEC
        lprintf (r, "[%d]PERF: Time: %d ms ", r -> nPid, ((cl - r -> startclock) * 1000 / CLOCKS_PER_SEC)) ;
#else
        lprintf (r, "[%d]PERF: ", r -> nPid) ;
#endif        
        lprintf (r, "Evals: %d ", r -> numEvals) ;
        if (r -> bDebug & dbgCacheDisable)
            lprintf (r, "Cache disabled") ;
        else
            if (r -> numEvals == 0)
                lprintf (r, "No Evals to cache") ;
            else
                lprintf (r, "Cache Hits: %d (%d%%)", r -> numCacheHits, r -> numCacheHits * 100 / r -> numEvals) ;

        lprintf (r, "\n") ;    
        lprintf (r, "[%d]Request finished. %s. Entry-SVs: %d -OBJs: %d Exit-SVs: %d -OBJs: %d\n", r -> nPid, asctime(tm), r -> stsv_count, r -> stsv_objcount, sv_count, sv_objcount) ;
        }

    r -> Buf.pCurrPos = NULL ;


    FlushLog (r) ;

    r -> Buf.nSourceline = 1 ;
    r -> Buf.pSourcelinePos = NULL ;    
    r -> Buf.pLineNoCurrPos = NULL ;    

    r -> bReqRunning = 0 ;

    av_clear (r -> pErrFill) ;
    av_clear (r -> pErrState) ;

#ifdef APACHE
    /* This must be the very very very last !!!!! */
    r -> pApacheReq = NULL ;
#endif
    return ok ;
    }


/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* Process the file                                                             */
/*                                                                              */
/* ---------------------------------------------------------------------------- */

    

static int ProcessFile (/*i/o*/ register req * r,
			/*in*/ int     nFileSize)

    {
    int     rc ;
    char *  p ;
    int     n ;


    r -> Buf.pSourcelinePos = r -> Buf.pCurrPos = r -> Buf.pBuf ;
    r -> Buf.pEndPos  = r -> Buf.pBuf + nFileSize ;

    rc = ok ;
    while (r -> Buf.pCurrPos < r -> Buf.pEndPos && rc == ok)
        {
        if ((r -> bDebug & dbgMem) && (sv_count != r -> lstsv_count || sv_objcount != r -> lstsv_objcount))
            {
            lprintf (r, "[%d]SVs:  Entry-SVs: %d -OBJs: %d Curr-SVs: %d -OBJs: %d\n", r -> nPid, r -> stsv_count, r -> stsv_objcount, sv_count, sv_objcount) ;
            r -> lstsv_count = sv_count ;
            r -> lstsv_objcount = sv_objcount ;
            }
        
        /* */
        /* execute [x ... x] and special html tags and replace them if nessecary */
        /* */

        if (r -> CmdStack.State.bProcessCmds == cmdAll && !(r -> bOptions & optDisableHtmlScan))
            {
            n = strcspn (r -> Buf.pCurrPos, "[<") ;
            p = r -> Buf.pCurrPos + n ;
            }
        else
            p = strchr (r -> Buf.pCurrPos, '[') ;
            
            
        if (p == NULL || *p == '\0')
            { /* output the rest of html */
            owrite (r, r -> Buf.pCurrPos, r -> Buf.pEndPos - r -> Buf.pCurrPos) ;
            break ;
            }
        
        if (r -> CmdStack.State.bProcessCmds == cmdAll)
            /* output until next cmd */
            owrite (r, r -> Buf.pCurrPos, p - r -> Buf.pCurrPos) ;
        
        if (r -> bDebug & dbgSource)
            {
            char * s = p ;
            char * n ;

            while (*s && isspace (*s))
                s++ ;
            
            if (*s)
                {
                GetLineNo (r) ;    
                n = strchr (s, '\n') ;
                if (n)
                    lprintf (r, "[%d]SRC: Line %d: %*.*s\n", r -> nPid, r -> Buf.nSourceline, n-s, n-s, s) ;
                else
                    lprintf (r, "[%d]SRC: Line %d: %60.60s\n", r -> Buf.nSourceline, r -> nPid, s) ;

                }
            }        

        
        r -> Buf.pCurrStart = p ;
        if (*p == '<')
            { /* HTML Tag */
            rc = ScanHtmlTag (r, p) ;
            }
         else
            { /* [x ... x] sequenz */
            rc = ScanCmdEvals (r, p) ;
            }
        }

    return rc ;
    }

/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* Request handler                                                              */
/*                                                                              */
/* ---------------------------------------------------------------------------- */



int ExecuteReq (/*i/o*/ register req * r,
                /*in*/  SV *           pReqSV) 

    {
    int     rc = ok ;
    SV *    pBufSV = NULL ;
    char    olddir[PATH_MAX];
    char *  sInputfile = r -> Buf.pFile -> sSourcefile ;
#ifdef WIN32
    int		olddrive ;
#endif

	dTHR ;

    EPENTRY (ExecuteReq) ;

    
    r -> pReqSV = pReqSV ;
    
    ENTER;
    SAVETMPS ;
	
    SetupSafeNamespace (r) ;

    /* --- read form data from browser if not already read by perl part --- */
    if (rc == ok && !(r -> bOptions & optDisableFormData) && av_len (r -> pFormArray) == -1) 
        rc = GetInputData_CGIScript (r) ;
    
    /* --- open output and send http header if EarlyHttpHeaders --- */
    if (rc == ok)
        rc = StartOutput (r) ;

    if (SvROK(r -> pInData))
        { /* --- get input from memory --- */
        STRLEN n ;
        r -> Buf.pBuf = SvPV (SvRV(r -> pInData), n) ;
        r -> Buf.pFile -> nFilesize = n ; 
        }

    else
        {
        /* --- read input file --- */
        if (rc == ok)
            rc = ReadHTML (r, sInputfile, &r -> Buf.pFile -> nFilesize, &pBufSV) ;
        if (rc == ok)
            r -> Buf.pBuf = SvPVX (pBufSV) ;
        }

    if (rc == ok && r -> Buf.pBuf == NULL)
        rc = rcMissingInput ;
    
    /* --- ok so far? if not exit ---- */
#ifdef APACHE
    if (rc != ok || (r -> pApacheReq && r -> pApacheReq -> header_only && (r -> bOptions & optEarlyHttpHeader)))
#else
    if (rc != ok)
#endif
        {
        if (rc != ok)
            LogError (r, rc);
#ifdef APACHE
        r -> pApacheReq = NULL ;
#endif
        r -> bReqRunning = 0 ;
        FREETMPS ;
        LEAVE;
        return rc ;
        }

    /* --- change working directory --- */
    
    if ((r -> bOptions & optDisableChdir) == 0 && sInputfile != NULL && sInputfile != '\0' && !SvROK(r -> pInData))
	{
	char dir[PATH_MAX];
#ifdef WIN32
	char drive[_MAX_DRIVE];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];

	olddrive = _getdrive () ;
	_splitpath(sInputfile, drive, dir, fname, ext );
   	_chdrive (drive[0] - 'A') ;
#else
        Dirname (sInputfile, dir, sizeof (dir) - 1) ;
#endif
	getcwd (olddir, sizeof (olddir) - 1) ;
	
	chdir (dir) ;
	}
    else
	r -> bOptions |= optDisableChdir ;
    
    if ((rc = ProcessFile (r, r -> Buf.pFile -> nFilesize)) != ok)
        if (rc == rcExit)
            rc = ok ;
        else
            LogError (r, rc) ;

    /* --- restore working directory --- */
    if ((r -> bOptions & optDisableChdir) == 0)
	{
#ifdef WIN32
   	_chdrive (olddrive) ;
#endif
	chdir (olddir) ;
	}

    /* --- Restore Operatormask and Package, destroy temp perl sv's --- */
    FREETMPS ;
    LEAVE;
    r -> bReqRunning = 0 ;

    /* --- send http header and data to the browser if not already done --- */
    if ((rc = EndOutput (r, rc, r -> pOutData)) != ok)
        LogError (r, rc) ;

    /* --- reset variables and log end of request --- */
    if ((rc = ResetRequest (r, sInputfile)) != ok)
        LogError (r, rc) ;

    return ok ;
    }
