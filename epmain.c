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


/* pid */

pid_t nPid ;

/* Apache Request Record */

#ifdef APACHE
request_rec * pReq = NULL ;
#endif




int  bDebug = 0 ;       /* Debugging options */
int  bOptions ;         /* Options */
int  bReqRunning = 0 ;  /* we are inside of a request */
int  bError = 0 ;       /* Error has occured somewhere */
static I32  nLastErrFill  ;
static int  bLastErrState ;

static int  bInitDone = 0 ; /* c part is already initialized */

int  nIOType   = epIOPerl ; /* what mode we are run as */

static char sCmdFifo [] = "/local/www/cgi-bin/embperl/cmd.fifo" ;
static char sRetFifo [1024] ;

static char sRetFifoName [] = "__RETFIFO" ;

char cMultFieldSep = '\t' ;  /* Separator if a form filed is multiplie defined */

static char sEnvHashName   [] = "ENV" ;
static char sFormHashName  [] = "HTML::Embperl::fdat" ;
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


HV *    pEnvHash ;   /* environement from CGI Script */
HV *    pFormHash ;  /* Formular data */
HV *    pFormSplitHash ;  /* Formular data split up at \t */
HV *    pInputHash ; /* Data of input fields */
AV *    pFormArray ; /* Fieldnames */
AV *    pErrArray ;  /* Errors to show on Error response */
AV *    pErrFill ;   /* AvFILL of pErrArray, index is nMarker */
AV *    pErrState ;  /* bError, index is nMarker  */
/*AV *    pHeaderArray ;*/ /* contains http headers in cgi mode */

HV *    pCacheHash ; /* Hash containing CVs to precompiled subs */

SV *    pPackage ;   /* Currently active Package */
SV *    pEvalPackage ; /* Currently active Package */
char *  sEvalPackage ; /* Currently active Package */
STRLEN  nEvalPackage ; /* Currently active Package (length) */
SV *    pOpcodeMask ;/* Currently active Opcode mask (if any) */


static char *  sCurrPackage ; /* Name of package to eval everything */

char * pBuf ;        /* Buffer which holds the html source file */
char * pCurrPos ;    /* Current position in html file */
char * pCurrStart ;  /* Current start position of html tag / eval expression */
char * pEndPos ;     /* end of html file */
char * pCurrTag ;    /* Current start position of html tag */

char * sSourcefile ; /* Name of sourcefile */
int    nSourceline ; /* Currentline in sourcefile */
char * pSourcelinePos ; /* Positon of nSourceline in sourcefile */
char * pLineNoCurrPos ; /* save pCurrPos for line no calculation */                     
                     
/*
   Additional Error info   
*/


char errdat1 [ERRDATLEN]  ;
char errdat2 [ERRDATLEN]  ;

char lastwarn [ERRDATLEN]  ;


/* for statistics */

static    clock_t startclock ;
static    I32     stsv_count ;
static    I32     stsv_objcount ;
static    I32     lstsv_count ;
static    I32     lstsv_objcount  ;


/* */
/* print error */
/* */

char * LogError (/*in*/ int   rc)

    {
    const char * msg ;
    char * sText ;
    SV *   pSV ;
    SV **  ppSV ;
    int    n ;

    
    EPENTRY (LogError) ;
    
    errdat1 [sizeof (errdat1) - 1] = '\0' ;
    errdat2 [sizeof (errdat2) - 1] = '\0' ;

    GetLineNo () ;
    
    if (rc != rcPerlWarn)
        bError = 1 ;

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
        default:                        msg ="[%d]ERR:  %d: Line %d: Error %s%s" ; break ; 
        }

    pSV = newSVpvf (msg, nPid , rc, nSourceline, errdat1, errdat2) ;

    sText = SvPV (pSV, na) ;    
    
    lprintf ("%s\n", sText) ;

#ifdef APACHE
    if (pReq)
#ifdef APLOG_ERR
        if (rc != rcPerlWarn)
            aplog_error (APLOG_MARK, APLOG_ERR | APLOG_NOERRNO, pReq -> server, sText) ;
        else
            aplog_error (APLOG_MARK, APLOG_WARNING | APLOG_NOERRNO, pReq -> server, sText) ;
#else
        log_error (sText, pReq -> server) ;
#endif
    else
#endif
        {
#ifdef WIN32
        if (nIOType != epIOCGI)
#endif
            {
            fprintf (stderr, "%s\n", sText) ;
            fflush (stderr) ;
            }
        }
    
    if (rc == rcPerlWarn)
        strncpy (lastwarn, errdat1, sizeof (lastwarn) - 1) ;

    /*lprintf ("DIS: AvFILL (pErrArray) = %d, nMarker = %d,  nLastErrFill= %d , bLastErrState = %d\n" , AvFILL (pErrArray), nMarker, nLastErrFill, bLastErrState) ;*/
    av_push (pErrArray, pSV) ;
    
    av_store (pErrFill, nMarker, newSViv (AvFILL(pErrArray))) ;
    av_store (pErrState, nMarker, newSViv (bError)) ;
    n = nMarker ;
    while (n-- > 0)
        {
        ppSV = av_fetch (pErrFill, n, 0) ;
        if (ppSV && SvOK (*ppSV))
            break ;
        av_store (pErrFill, n, newSViv (nLastErrFill)) ;
        av_store (pErrState, n, newSViv (bLastErrState)) ;
        }

    nLastErrFill  = AvFILL(pErrArray) ;
    bLastErrState = bError ;

    errdat1[0] = '\0' ;
    errdat2[0] = '\0' ;

    return sText ;
    }


/* */
/* begin for error rollback */
/* */

void CommitError ()
    
    {
    int f = AvFILL(pErrArray)  ;
    
    if (f == -1)
        return ; /* no errors -> nothing to do */

    /*lprintf ("DIS: Commit AvFILL (pErrArray) = %d, nMarker = %d,  nLastErrFill= %d , bLastErrState = %d\n" , AvFILL (pErrArray), nMarker, nLastErrFill, bLastErrState) ;*/

    av_store (pErrFill, nMarker, newSViv (f)) ;
    av_store (pErrState, nMarker, newSViv (bError)) ;
    }

    
    
/* */
/* rollback error */
/* */

void RollbackError ()

    {
    SV *  pFill ;
    SV *  pState ;
    SV ** ppSV ;
    I32   f = AvFILL (pErrFill) ;
    int   n ;
    int   i ;

    if (f < nMarker)
        return ;
    
    /*lprintf ("DIS: AvFILL (pErrFill) = %d, nMarker = %d\n" , f, nMarker) ;*/
    
    for (i = f; i > nMarker; i--)
        {
        pFill  = av_pop(pErrFill) ;
        pState = av_pop(pErrState) ;
        SvREFCNT_dec (pFill) ;
        SvREFCNT_dec (pState) ;
        }
    ppSV   = av_fetch(pErrFill, nMarker, 0) ;
    if (ppSV)
        n = SvIV (*ppSV) ;
    else
        n = 0 ;
    ppSV = av_fetch(pErrState, nMarker, 0) ;
    if (ppSV)
        bError = SvIV (*ppSV) ;
    else
        bError = 1 ;
    f = AvFILL (pErrArray) ;
    /*lprintf ("DIS: AvFILL (pErrArray) = %d, n = %d\n" , f, n) ;*/
    if (f > n)
        lprintf ("[%d]ERR:  Discard the last %d errormessages, because they occured after the end of a table\n", nPid, f - n) ;
    for (i = f; i > n; i--)
        {
        SvREFCNT_dec (av_pop(pErrArray)) ;
        }

    nLastErrFill  = AvFILL(pErrArray) ;
    bLastErrState = bError ;
    }


    
/* */
/* Magic */
/* */

static void NewEscMode (SV * pSV)

    {
    if (bEscMode & escHtml)
        pNextEscape = Char2Html ;
    else if (bEscMode & escUrl)
        pNextEscape = Char2Url ;
    else 
        pNextEscape = NULL ;

    if (bEscModeSet < 1)
        pCurrEscape = pNextEscape ;

    if (bEscModeSet < 0 && SvOK (pSV))
        bEscModeSet = 1 ;
    }



static int notused ;

INTMG (TabCount, TableState.nCount, TableState.nCountUsed, ;) 
INTMG (TabRow, TableState.nRow, TableState.nRowUsed, ;) 
INTMG (TabCol, TableState.nCol, TableState.nColUsed, ;) 
INTMG (TabMaxRow, nTabMaxRow, notused,  ;) 
INTMG (TabMaxCol, nTabMaxCol, notused, ;) 
INTMG (TabMode, nTabMode, notused, ;) 
INTMG (EscMode, bEscMode, notused, NewEscMode (pSV)) 

OPTMGRD (optDisableVarCleanup      , bOptions) ;
OPTMG   (optDisableEmbperlErrorPage, bOptions) ;
OPTMGRD (optSafeNamespace          , bOptions) ;
OPTMGRD (optOpcodeMask             , bOptions) ;
OPTMG   (optRawInput               , bOptions) ;
OPTMG   (optSendHttpHeader         , bOptions) ;
OPTMGRD (optDisableChdir           , bOptions) ;
OPTMG   (optDisableHtmlScan        , bOptions) ;
OPTMGRD (optEarlyHttpHeader        , bOptions) ;
OPTMGRD (optDisableFormData        , bOptions) ;
OPTMG   (optDisableInputScan       , bOptions) ;
OPTMG   (optDisableTableScan       , bOptions) ;
OPTMG   (optDisableMetaScan        , bOptions) ;
OPTMGRD (optAllFormData            , bOptions) ;
OPTMGRD (optRedirectStdout         , bOptions) ;
OPTMG   (optUndefToEmptyValue      , bOptions) ;



OPTMG   (dbgStd          , bDebug) ;
OPTMG   (dbgMem          , bDebug) ;
OPTMG   (dbgEval         , bDebug) ;
OPTMG   (dbgCmd          , bDebug) ;
OPTMG   (dbgEnv          , bDebug) ;
OPTMG   (dbgForm         , bDebug) ;
OPTMG   (dbgTab          , bDebug) ;
OPTMG   (dbgInput        , bDebug) ;
OPTMG   (dbgFlushOutput  , bDebug) ;
OPTMG   (dbgFlushLog     , bDebug) ;
OPTMG   (dbgAllCmds      , bDebug) ;
OPTMG   (dbgSource       , bDebug) ;
OPTMG   (dbgFunc         , bDebug) ;
OPTMG   (dbgLogLink      , bDebug) ;
OPTMG   (dbgDefEval      , bDebug) ;
OPTMG   (dbgCacheDisable , bDebug) ;
OPTMG   (dbgWatchScalar  , bDebug) ;
OPTMG   (dbgHeadersIn    , bDebug) ;
OPTMG   (dbgShowCleanup  , bDebug) ;

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
    hv_clear (pFormSplitHash) ;
    

    if (nLen == 0)
        return ok ;
    
    if ((pMem = _malloc (nLen + 4)) == NULL)
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
                *p++ = cMultFieldSep ;
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
            
                if (nKey > 0 && (nVal > 0 || (bOptions & optAllFormData)))
                    {
                    if (pVal > pKey)
                        pVal[-1] = '\0' ;
                    
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
                nLen-- ;
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
            iread (p, len) ;
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
    STRLEN  len   = 0 ;
    char    sLen [20] ;
    

    EPENTRY (GetInputData_CGIScript) ;

#ifdef APACHE
    if (pReq && (bDebug & dbgHeadersIn))
        {
        int i;
        array_header *hdrs_arr;
        table_entry  *hdrs;

        hdrs_arr = table_elts (pReq->headers_in);
        hdrs = (table_entry *)hdrs_arr->elts;

        lprintf ( "[%d]HDR:  %d\n", nPid, hdrs_arr->nelts) ; 
        for (i = 0; i < hdrs_arr->nelts; ++i)
	    if (hdrs[i].key)
                lprintf ( "[%d]HDR:  %s=%s\n", nPid, hdrs[i].key, hdrs[i].val) ; 
        }
#endif

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
        SV * * ppSV = hv_fetch(pEnvHash, "QUERY_STRING", sizeof ("QUERY_STRING") - 1, 0) ;  
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
        if ((p = _malloc (len + 1)) == NULL)
            return rcOutOfMemory ;

        if ((rc = OpenInput (NULL)) != ok)
            {
            _free (p) ;
            return rc ;
            }
        iread (p, len) ;
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
    char    nType ;
    SV *    pRet ;
    struct tCmd * pCmd ;


    EPENTRY (ScanCmdEvals) ;
    
    p++ ;

    pCurrPos = p ;

    if ((nType = *p++) == '\0')
        return ok ;

    pCurrPos = p ;

    if (nType != '+' && nType != '-' && nType != '$' && nType != '!')
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
                bEscModeSet = -1 ;
                pNextEscape = pCurrEscape ;
                rc = EvalTrans (pCurrPos, (pCurrPos - pBuf), &pRet) ;
                if (rc != ok && rc != rcEvalErr)
                    return rc ;

                if (pRet)
                    {
                    OutputToHtml (SvPV (pRet, na)) ;
                    SvREFCNT_dec (pRet) ;
                    }
                pCurrEscape = pNextEscape ;
                bEscModeSet = 0 ;
                }

            p [-2] = nType ;
            pCurrPos = p ;

        
            break ;
        case '-':
            if (State.bProcessCmds == cmdAll)
                {
                rc = EvalTrans (pCurrPos, (pCurrPos - pBuf), &pRet) ;
                if (rc != ok && rc != rcEvalErr)
                    return rc ;
                if (pRet)
                    SvREFCNT_dec (pRet) ;
                }

            p [-2] = nType ;
            pCurrPos = p ;

            break ;
        case '!':
            if (State.bProcessCmds == cmdAll)
                {
                rc = EvalTransOnFirstCall (pCurrPos, (pCurrPos - pBuf), &pRet) ;
                if (rc != ok && rc != rcEvalErr)
                    return rc ;
                if (pRet)
                    SvREFCNT_dec (pRet) ;
                }

            p [-2] = nType ;
            pCurrPos = p ;

            break ;
        case '$':
            TransHtml (pCurrPos) ;

            /* skip spaces before command */
            while (*pCurrPos != '\0' && isspace (*pCurrPos))
                    pCurrPos++ ;

            /* c holds the start of the command */
            a = c = pCurrPos ;
            while (*a != '\0' && isalpha (*a))
                a++ ;

            /* a points to first char after command */

            pCurrPos = p ;

            if ((rc = SearchCmd (c, a-c, a, FALSE, &pCmd)) != ok)
                return rc ;
        
        
            if ((rc = ProcessCmd (pCmd, a)) != ok)
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


    
int ScanCmdEvalsInString (/*in*/  char *   pIn,
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
        /* lprintf ("SCEV nothing sArg = %s\n", pIn) ; */
        *pOut = pIn ; /* Nothing to do */
        return ok ;
        }
    /* lprintf ("SCEV sArg = %s, p = %s\n", pIn, p) ; */

    /* save global vars */
    pSaveCurrPos   = pCurrPos ;
    pSaveCurrStart = pCurrStart ;
    pSaveEndPos    = pEndPos ;
    pSaveLineNo    = pLineNoCurrPos ;
    if (pLineNoCurrPos == NULL)
        pLineNoCurrPos = pCurrPos ; /* save it for line no calculation */
    

    pCurrPos = pIn ;
    pEndPos  = pIn + strlen (pIn) ;

    *pOut = _malloc (nSize) ;
    if (*pOut == NULL)
        return rcOutOfMemory ;

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
    
    *pFree = *pOut = OutputToStd () ;

    pCurrPos   = pSaveCurrPos ;
    pCurrStart = pSaveCurrStart ;
    pEndPos    = pSaveEndPos ;
    pLineNoCurrPos = pSaveLineNo ;
    
    return rc ;
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
    char * pArgBuf  = NULL ;
    char * pFreeBuf = NULL ;
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

    if ((rc = SearchCmd (pCmd, pec - pCmd, "", TRUE, &pCmdInfo)) != ok)
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

    pCurrPos = p ;    /* pCurrPos = first char after whole tag */

    
    if (*pArg != '\0' && pCmdInfo -> bScanArg)
    	{
        if ((rc = ScanCmdEvalsInString ((char *)pArg, &pArgBuf, nInitialScanOutputSize, &pFreeBuf)) != ok)
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
            {
            if (pFreeBuf)
                _free (pFreeBuf) ;
            
            return rc ;
            }
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

    if (pFreeBuf)
        _free (pFreeBuf) ;

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

int iembperl_init (/*in*/ int           _nIOType,
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
    bReqRunning = 0 ;
    
    nPid = getpid () ;

    sSourcefile = "???" ;
    nSourceline = 1 ;
    pSourcelinePos = NULL ;    
    pLineNoCurrPos = NULL ;    

    if ((rc = OpenLog (sLogFile, (bDebug & dbgFunc)?1:0)) != ok)
        { 
        bDebug = 0 ; /* Turn debbuging off, only errors will go to stderr */
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
        
        /* lprintf ("[%d]INIT: Embperl %s starting... mode = %s (%d)\n", nPid, sVersion, p, nIOType) ; */
        }


#ifndef APACHE
    if (nIOType == epIOMod_Perl)
        {
        LogError (rcNotCompiledForModPerl) ;
        return 1 ;
        }
#endif

    if (bInitDone)
        return ok ; /* the rest was alreay done */

    if ((pFormHash = perl_get_hv (sFormHashName, TRUE)) == NULL)
        {
        LogError (rcHashError) ;
        return 1 ;
        }


    if ((pFormSplitHash = perl_get_hv (sFormSplitHashName, TRUE)) == NULL)
        {
        LogError (rcHashError) ;
        return 1 ;
        }


    if ((pFormArray = perl_get_av (sFormArrayName, TRUE)) == NULL)
        {
        LogError (rcArrayError) ;
        return 1 ;
        }

    if ((pErrArray = perl_get_av (sErrArrayName, TRUE)) == NULL)
        {
        LogError (rcArrayError) ;
        return 1 ;
        }

    if ((pErrFill = perl_get_av (sErrFillName, TRUE)) == NULL)
        {
        LogError (rcArrayError) ;
        return 1 ;
        }

    if ((pErrState = perl_get_av (sErrStateName, TRUE)) == NULL)
        {
        LogError (rcArrayError) ;
        return 1 ;
        }

    /*
    if ((pHeaderArray = perl_get_av (sHeaderArrayName, TRUE)) == NULL)
        {
        LogError (rcArrayError) ;
        return 1 ;
        }
    */

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

    if ((pPackage = perl_get_sv (sPackageName, TRUE)) == NULL)
        {
        LogError ( rcPerlVarError) ;
        return 1 ;
        }

    if ((pEvalPackage = perl_get_sv (sEvalPackageName, TRUE)) == NULL)
        {
        LogError ( rcPerlVarError) ;
        return 1 ;
        }

    if ((pOpcodeMask = perl_get_sv (sOpcodeMaskName, TRUE)) == NULL)
        {
        LogError ( rcPerlVarError) ;
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
    bDebug &= ~(dbgFlushLog) ; /* set Debugflags to no output to logfiles */
#endif

    return ok ;
    }



int iembperl_resetreqrec  (/*in*/ int bResetHandler)
    {
#ifdef APACHE
    if (pReq && bResetHandler)
        pReq -> handler = NULL ;
    
    pReq = NULL ;
#endif
    bReqRunning = 0 ;

    sSourcefile = "???" ;
    nSourceline = 1 ;
    pSourcelinePos = NULL ;    
    pLineNoCurrPos = NULL ;    

    FlushLog () ;

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
/* Setup Request                                                                */
/*                                                                              */
/* ---------------------------------------------------------------------------- */


static int SetupRequest   (/*in*/ char *  sInputfile,
                           /*in*/ int     bDebugFlags,
                           /*in*/ int     bOptionFlags,
                           /*in*/ HV *    pCache,
                           /*in*/ char *  op_mask_buf, 
                           /*in*/ SV *    pOutData) 

    {
    int     rc ;
    GV *    gv;
    char *  sMode ;

	dTHR ;

    EPENTRY (SetupRequest) ;
	
    startclock      = clock () ;
    stsv_count      = sv_count ;
    stsv_objcount   = sv_objcount ;
    lstsv_count     = sv_count ;
    lstsv_objcount  = sv_objcount ;

    nPid            = getpid () ; /* reget pid, because it could be chaned when loaded with PerlModule */
    bDebug          = bDebugFlags ;
    bOptions        = bOptionFlags ;
    pCacheHash      = pCache ;
    bReqRunning     = 1 ;
    sSourcefile     = sInputfile ;
    nSourceline     = 1 ;
    pSourcelinePos  = NULL ;    
    pLineNoCurrPos  = NULL ;    
    bError          = 0 ;    
    tainted         = 0 ;
    sEvalPackage    = SvPV (pEvalPackage, nEvalPackage) ;
    bStrict         = FALSE ;

    nStack          = 0 ;
    nTableStack     = 0 ;
    nHtmlStack      = 0 ;
    pArgStack       = ArgStack ;
    pArgHtmlStack   = ArgHtmlStack ;

    memset (&State, 0, sizeof (State)) ;
    memset (&HtmlState, 0, sizeof (HtmlState)) ;
    memset (&TableState, 0, sizeof (TableState)) ;
    
    State.nCmdType      = cmdNorm ;
    State.bProcessCmds  = cmdAll ;
    State.sArg          = strcpy (pArgStack, "") ;
    pArgStack       += 1 ;
    HtmlState.nCmdType      = cmdNorm ;
    HtmlState.bProcessCmds  = cmdAll ;
    HtmlState.sArg          = strcpy (pArgHtmlStack, "") ;
    pArgHtmlStack       += 1 ;
    nTabMode        = epTabRowDef | epTabColDef ;
    nTabMaxRow      = 100 ;
    nTabMaxCol      = 10 ;
    pCurrTag        = NULL ;

    av_clear (pErrFill) ;
    av_clear (pErrState) ;
    av_clear (pErrArray) ;
    nLastErrFill  = AvFILL(pErrArray) ;
    bLastErrState = bError ;

    if ((rc = OpenLog (NULL, 2)) != ok)
        { 
        bDebug = 0 ; /* Turn debbuging off, only errors will go to stderr */
        LogError (rc) ;
        }

    if (bDebug)
        {
        time_t t ;
        struct tm * tm ;
        time (&t) ;        
        tm =localtime (&t) ;
        lprintf ("[%d]REQ:  Embperl %s starting... %s\n", nPid,  sVersion, asctime(tm)) ;
        numEvals = 0  ;
        numCacheHits = 0 ;
        }
    

    /* The following is borrowed from Opcode.xs */

    if (bOptions & optOpcodeMask)
        opmask_addlocal(pOpcodeMask, op_mask_buf);

        
    if (bOptions & optSafeNamespace)
        {
        save_aptr(&endav);
        endav = (AV*)sv_2mortal((SV*)newAV()); /* ignore END blocks for now	*/

        save_hptr(&defstash);		/* save current default stack	*/
        /* the assignment to global defstash changes our sense of 'main'	*/
        defstash = gv_stashpv(SvPV (pPackage, na), GV_ADDWARN); /* should exist already	*/

        if (bDebug)
            lprintf ("[%d]REQ:  switch to safe namespace %s\n", nPid, SvPV (pPackage, na)) ;


        /* defstash must itself contain a main:: so we'll add that now	*/
        /* take care with the ref counts (was cause of long standing bug)	*/
        /* XXX I'm still not sure if this is right, GV_ADDWARN should warn!	*/
        gv = gv_fetchpv("main::", GV_ADDWARN, SVt_PVHV);
        sv_free((SV*)GvHV(gv));
        GvHV(gv) = (HV*)SvREFCNT_inc(defstash);
        }

    /* for backwards compability */
    if (bDebug & dbgEarlyHttpHeader)
        bOptions |= optEarlyHttpHeader ;
        
   
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
        
        lprintf ("[%d]REQ:  %s  %s  ", nPid, (bOptions & optSafeNamespace)?"SafeNamespace":"No Safe Eval", (bOptions & optOpcodeMask)?"OpcodeMask":"All Opcode allowed") ;
        lprintf (" mode = %s (%d)\n", sMode, nIOType) ;
        lprintf ("[%d]REQ:  Package = %s\n", nPid, SvPV (pPackage, na)) ;
        }

    return ok ;
    }

    
/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* Start the output stream                                                      */
/*                                                                              */
/* ---------------------------------------------------------------------------- */

    
static int StartOutput (/*in*/ char *  sOutputfile,
                        /*in*/ SV *    pOutData) 

    {
    int rc ;
    int  bOutToMem = SvROK (pOutData) ;

    
    if ((rc = OpenOutput (sOutputfile)) != ok)
        return rc ;

#ifdef APACHE
    if (pReq && pReq -> main)
    	bOptions |= optEarlyHttpHeader ; /* do not direct output to memory on internal redirect */
#endif
    if (bOutToMem)
    	bOptions &= ~optEarlyHttpHeader ;


    if (bOptions & optEarlyHttpHeader)
        {
#ifdef APACHE
        if (pReq == NULL)
            {
#endif
            if (nIOType != epIOPerl && (bOptions & optSendHttpHeader))
                oputs ("Content-type: text/html\n\n") ;

#ifdef APACHE
            }
        else
            {
            if (pReq -> main == NULL && (bOptions & optSendHttpHeader))
            	send_http_header (pReq) ;
#ifndef WIN32
            mod_perl_sent_header(pReq, 1) ;
#endif
            if (pReq -> header_only)
            	{
            	CloseOutput () ;
            	return ok ;
    	        }
            }
#endif
        }
    else
        {
        if (nIOType == epIOCGI && (bOptions & optSendHttpHeader))
            oputs ("Content-type: text/html\n\n") ;
            
        oBegin () ;
        }

    return ok ;
    }

/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* End the output stream                                                        */
/*                                                                              */
/* ---------------------------------------------------------------------------- */


static int EndOutput (/*in*/ int    rc,
                      /*in*/ SV *   pOutData) 
                      

    {
    SV * pOut ;
    int  bOutToMem = SvROK (pOutData) ;

    
    if (rc != ok ||  bError)
        { /* --- generate error page if necessary --- */
#ifdef APACHE
        if (pReq)
            pReq -> status = 500 ;
#endif
        if (!(bOptions & optDisableEmbperlErrorPage))
            {
            dSP;                            /* initialize stack pointer      */

            oRollbackOutput (NULL) ; /* forget everything outputed so far */
            oBegin () ;

            PUSHMARK(sp);                   /* remember the stack pointer    */
            perl_call_pv ("HTML::Embperl::SendErrorDoc", G_DISCARD | G_NOARGS) ; /* call the function             */
            }
        }
    

    if (!(bOptions & optEarlyHttpHeader) && (bOptions & optSendHttpHeader) && !bOutToMem)
        {  /* --- send http headers if not alreay done --- */
#ifdef APACHE
        if (pReq)
            {
            set_content_length (pReq, GetContentLength () + 2) ;
            send_http_header (pReq) ;
#ifndef WIN32
            mod_perl_sent_header(pReq, 1) ;
#endif
            if (bDebug & dbgHeadersIn)
        	{
        	int i;
        	array_header *hdrs_arr;
        	table_entry  *hdrs;

        	hdrs_arr = table_elts (pReq->headers_out);
	        hdrs = (table_entry *)hdrs_arr->elts;

        	lprintf ( "[%d]HDR:  %d\n", nPid, hdrs_arr->nelts) ; 
	        for (i = 0; i < hdrs_arr->nelts; ++i)
		    if (hdrs[i].key)
                	lprintf ( "[%d]HDR:  %s=%s\n", nPid, hdrs[i].key, hdrs[i].val) ; 
        	}
            }
       else
#endif
           { 
            /*
           if (nIOType == epIOCGI)
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
    if ((pReq == NULL || !pReq -> header_only) && !(bOptions & optEarlyHttpHeader))
#else
    if (!(bOptions & optEarlyHttpHeader))
#endif
        {
        oputs ("\r\n") ;
        if (bOutToMem)
            {
            char * pData ;
            int    l = GetContentLength () + 1 ;
            
            sv_setpv (pOut, "") ;
            SvGROW (pOut, l) ;
            pData = SvPVX (pOut) ;
            oCommitToMem (NULL, pData) ;
            SvCUR_set (pOut, l - 1) ;
            }
        else
            {
            oCommit (NULL) ;
            }
        }
    else
        {
        oRollbackOutput (NULL) ;
        if (bOutToMem)
            sv_setsv (pOut, &sv_undef) ;
        }    

    CloseOutput () ;

    return ok ;
    }
    
/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* Reset Request                                                                */
/*                                                                              */
/* ---------------------------------------------------------------------------- */


static int ResetRequest (/*in*/ char *  sInputfile)

    {
    if (bDebug)
        {
        clock_t cl = clock () ;
        time_t t ;
        struct tm * tm ;
        
        time (&t) ;        
        tm =localtime (&t) ;
        
        lprintf ("[%d]PERF: input = %s\n", nPid, sInputfile) ;
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

    pCurrPos = NULL ;


    FlushLog () ;

    sSourcefile = "???" ;
    nSourceline = 1 ;
    pSourcelinePos = NULL ;    
    pLineNoCurrPos = NULL ;    

    bReqRunning = 0 ;

    av_clear (pErrFill) ;
    av_clear (pErrState) ;

#ifdef APACHE
    /* This must be the very very very last !!!!! */
    pReq = NULL ;
#endif
    return ok ;
    }


/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* Process the file                                                             */
/*                                                                              */
/* ---------------------------------------------------------------------------- */

    

static int ProcessFile (/*in*/ int     nFileSize)

    {
    int     rc ;
    char *  p ;
    int     n ;


    pSourcelinePos = pCurrPos = pBuf ;
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

        if (State.bProcessCmds == cmdAll && !(bOptions & optDisableHtmlScan))
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
                GetLineNo () ;    
                n = strchr (s, '\n') ;
                if (n)
                    lprintf ("[%d]SRC: Line %d: %*.*s\n", nPid, nSourceline, n-s, n-s, s) ;
                else
                    lprintf ("[%d]SRC: Line %d: %60.60s\n", nSourceline, nPid, s) ;

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

    return rc ;
    }

/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* Request handler                                                              */
/*                                                                              */
/* ---------------------------------------------------------------------------- */



int iembperl_req  (/*in*/ char *  sInputfile,
                   /*in*/ char *  sOutputfile,
                   /*in*/ int     bDebugFlags,
                   /*in*/ int     bOptionFlags,
                   /*in*/ STRLEN  nFileSize,
                   /*in*/ HV *    pCache, 
                   /*in*/ SV *    pInData,
                   /*in*/ SV *    pOutData) 




    {
    int     rc ;
    char    op_mask_buf[MAXO + 100]; /* maxo shouldn't differ from MAXO but leave room anyway (see BOOT:)	*/
    SV *    pBufSV = NULL ;
    char    olddir[PATH_MAX];
#ifdef WIN32
    int		olddrive ;
#endif


    EPENTRY (iembperl_req) ;

    ENTER;

    /* --- initialize variables and setup perl namespace --- */
    rc = SetupRequest (sInputfile, bDebugFlags, bOptionFlags, pCache, op_mask_buf, pOutData); 

    /* --- read form data from browser if not already read by perl part --- */
    if (rc == ok && !(bOptions & optDisableFormData) && av_len (pFormArray) == -1) 
        rc = GetInputData_CGIScript () ;
    
    /* --- open output and send http header if EarlyHttpHeaders --- */
    if (rc == ok)
        rc = StartOutput (sOutputfile, pOutData) ;

    if (SvROK(pInData))
        { /* --- get input from memory --- */
        pBuf = SvPV (SvRV(pInData), nFileSize) ;
        }

    else
        {
        /* --- read input file --- */
        if (rc == ok)
            rc = ReadHTML (sInputfile, &nFileSize, &pBufSV) ;
        if (rc == ok)
            pBuf = SvPVX (pBufSV) ;
        }

    if (rc == ok && pBuf == NULL)
        rc = rcMissingInput ;
    
    /* --- ok so far? if not exit ---- */
#ifdef APACHE
    if (rc != ok || (pReq && pReq -> header_only && (bOptions & optEarlyHttpHeader)))
#else
    if (rc != ok)
#endif
        {
        if (rc != ok)
            LogError (rc);
#ifdef APACHE
        pReq = NULL ;
#endif
        bReqRunning = 0 ;
        LEAVE;
        return rc ;
        }

	/* --- change working directory --- */
	if ((bOptions & optDisableChdir) == 0 && sInputfile != NULL && sInputfile[0] != '\0' && !SvROK(pInData))
	    {
	    char dir[PATH_MAX];
#ifdef WIN32
	    char drive[_MAX_DRIVE];
	    char fname[_MAX_FNAME];
	    char ext[_MAX_EXT];
    
	    olddrive = _getdrive () ;
	    _splitpath( sInputfile, drive, dir, fname, ext );
   	    _chdrive (drive[0] - 'A') ;
#else
            Dirname (sInputfile, dir, sizeof (dir) - 1) ;
#endif
	    getcwd (olddir, sizeof (olddir) - 1) ;
	    
	    chdir (dir) ;
	    }
        else
	    bOptions |= optDisableChdir ;
	
	if ((rc = ProcessFile (nFileSize)) != ok)
            if (rc == rcExit)
                rc = ok ;
            else
                LogError (rc) ;

	/* --- restore working directory --- */
	if ((bOptions & optDisableChdir) == 0)
	    {
#ifdef WIN32
   	    _chdrive (olddrive) ;
#endif
	    chdir (olddir) ;
	    }

    /* --- Restore Operatormask and Package, destroy temp perl sv's --- */
    LEAVE;
    bReqRunning = 0 ;

    /* --- send http header and data to the browser if not already done --- */
    if ((rc = EndOutput (rc, pOutData)) != ok)
        LogError (rc) ;

    /* --- reset variables and log end of request --- */
    if ((rc = ResetRequest (sInputfile)) != ok)
        LogError (rc) ;

    return ok ;
    }
