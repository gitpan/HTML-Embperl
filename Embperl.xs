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

#ifdef __cplusplus
extern "C" {
#endif
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"
#ifdef __cplusplus
}
#endif

#include "ep.h"

static int
not_here(s)
char *s;
{
    croak("%s not implemented on this architecture", s);
    return -1;
}
#define PERLCONST(NAME) \
        pSV = newSViv (NAME) ; \
        if (hv_store (pHash, #NAME, sizeof (#NAME) - 1, pSV, 0) == NULL) \
            return rcHashError ;

static int constants()
	{

	HV * pHash ;
	SV * pSV ;

        if ((pHash = perl_get_hv ("CONSTANT", TRUE)) == NULL)
            return rcHashError ;


	
        PERLCONST(epIOCGI)
        PERLCONST(epIOMod_Perl)
        PERLCONST(epIOPerl)
        PERLCONST(epIOProcess)

        PERLCONST(ok)
        PERLCONST(rcStackOverflow)
        PERLCONST(rcStackUnderflow)
        PERLCONST(rcEndifWithoutIf)
        PERLCONST(rcElseWithoutIf)
        PERLCONST(rcEndwhileWithoutWhile)
        PERLCONST(rcEndtableWithoutTable)
        PERLCONST(rcCmdNotFound)
        PERLCONST(rcOutOfMemory)
        PERLCONST(rcPerlVarError)
        PERLCONST(rcHashError)
        PERLCONST(rcArrayError)
        PERLCONST(rcFileOpenErr)
        PERLCONST(rcMissingRight)
        PERLCONST(rcNoRetFifo)
        PERLCONST(rcMagicError)
        PERLCONST(rcWriteErr)
        PERLCONST(rcUnknownNameSpace)
        PERLCONST(rcInputNotSupported)
        PERLCONST(rcCannotUsedRecursive)
        PERLCONST(rcEndtableWithoutTablerow)
        PERLCONST(rcEndtextareaWithoutTextarea)
        PERLCONST(rcArgStackOverflow)
        PERLCONST(rcEvalErr)
        PERLCONST(rcNotCompiledForModPerl)
        PERLCONST(rcLogFileOpenErr)
        PERLCONST(rcExecCGIMissing)
        PERLCONST(rcIsDir)
        PERLCONST(rcXNotSet)
        PERLCONST(rcNotFound)
        PERLCONST(rcUnknownVarType)
        PERLCONST(rcPerlWarn)
        PERLCONST(rcVirtLogNotSet)
        
        PERLCONST(optDisableVarCleanup)
        PERLCONST(optDisableEmbperlErrorPage)
        PERLCONST(optSafeNamespace)
        PERLCONST(optOpcodeMask)
        PERLCONST(optRawInput)
        PERLCONST(optSendHttpHeader)

        PERLCONST(dbgStd)
        PERLCONST(dbgMem)
        PERLCONST(dbgEval)
        PERLCONST(dbgCmd)
        PERLCONST(dbgEnv)
        PERLCONST(dbgForm)
        PERLCONST(dbgTab)
        PERLCONST(dbgInput)
        PERLCONST(dbgFlushOutput)
        PERLCONST(dbgFlushLog)
        PERLCONST(dbgAllCmds)
        PERLCONST(dbgSource)
        PERLCONST(dbgFunc)
        PERLCONST(dbgLogLink)
        PERLCONST(dbgDefEval)
        PERLCONST(dbgCacheDisable)
        PERLCONST(dbgEarlyHttpHeader)
        PERLCONST(dbgWatchScalar)
        PERLCONST(dbgHeadersIn)
        PERLCONST(dbgShowCleanup)
        PERLCONST(dbgAll)
     
        PERLCONST(escNone)
        PERLCONST(escHtml)
        PERLCONST(escUrl)
        PERLCONST(escStd)

    return ok;
}

# /* ############################################################################### */

MODULE = HTML::Embperl      PACKAGE = HTML::Embperl     PREFIX = embperl_


int
embperl_constants()
CODE:
	RETVAL = constants () ;
OUTPUT:
    RETVAL


int
embperl_XS_Init(nIOType, sLogFile, nDebugDefault)
    int nIOType
    char * sLogFile
    int    nDebugDefault
CODE:
    RETVAL = Init(nIOType, sLogFile, nDebugDefault) ;
OUTPUT:
    RETVAL




int
embperl_XS_Term()
CODE:
    RETVAL = Term() ;
OUTPUT:
    RETVAL



int
embperl_ResetHandler(pReqSV)
    SV * pReqSV
CODE:
    RETVAL = ResetHandler(pReqSV) ;
OUTPUT:
    RETVAL



#if defined (__GNUC__) && defined (__i386__)

int
embperl_dbgbreak()
CODE:
    __asm__ ("int   $0x03\n") ;

#endif


# /* ---- Configuration data ----- */

tConf *
embperl_SetupConfData(req,opcodemask) 
    HV *    req = NO_INIT
    SV *    opcodemask 
INIT:
    req = (HV *)SvRV(ST(0));
CODE:
    RETVAL = SetupConfData(req, opcodemask) ;
OUTPUT:
    RETVAL


# /* ----- Request data ----- */

tReq *
embperl_SetupRequest(req_rec,sInputfile,mtime,filesize,sOutputfile,pConf,nIOtype,pIn,pOut) 
    SV *    req_rec
    char *  sInputfile
    long    mtime
    long    filesize
    char *  sOutputfile = NO_INIT
    tConf * pConf
    int     nIOtype
    SV *    pIn
    SV *    pOut
INIT:
    if (SvOK(ST(4)))
        sOutputfile = SvPV(ST(4), na);
    else
        sOutputfile = "\1" ; 
CODE:        
    RETVAL = SetupRequest(req_rec,sInputfile,mtime,filesize,sOutputfile,pConf,nIOtype,pIn,pOut) ;
OUTPUT:
    RETVAL





double
Clock()
CODE:
#ifdef CLOCKS_PER_SEC
        RETVAL = clock () * 1000 / CLOCKS_PER_SEC / 1000.0 ;
#else
        RETVAL = clock () ;
#endif        
OUTPUT:
    RETVAL


int
embperl_logerror(code, sText)
    int    code
    char * sText
INIT:
    tReq * r = pCurrReq ;
CODE:
     strncpy (r->errdat1, sText, sizeof (r->errdat1) - 1) ;
     LogError (r,code) ;


void
embperl_log(sText)
    char * sText
INIT:
    tReq * r = pCurrReq ;
CODE:
    OpenLog (r,"", 2) ;
    lwrite (r,sText, strlen (sText)) ;


void
embperl_output(sText)
    char * sText
INIT:
    tReq * r = pCurrReq ;
CODE:
    OutputToHtml (r,sText) ;


int
embperl_logevalerr(r,sText)
    char * sText
PREINIT:
    int l ;
INIT:
    tReq * r = pCurrReq ;
CODE:
     l = strlen (sText) ;
     while (l > 0 && isspace(sText[l-1]))
        sText[--l] = '\0' ;

     strncpy (r -> errdat1, sText, sizeof (r -> errdat1) - 1) ;
     LogError (r, rcEvalErr) ;


int
embperl_getlineno()
INIT:
    tReq * r = pCurrReq ;
CODE:
    RETVAL = GetLineNo (r) ;
OUTPUT:
    RETVAL


void
embperl_flushlog()
INIT:
    tReq * r = pCurrReq ;
CODE:
    FlushLog (r) ;


char *
embperl_Sourcefile()
INIT:
    tReq * r = pCurrReq ;
CODE:
    if (r -> Buf.pFile)
        RETVAL = r -> Buf.pFile -> sSourcefile ;
    else
        RETVAL = NULL ;
OUTPUT:
    RETVAL



################################################################################

MODULE = HTML::Embperl      PACKAGE = HTML::Embperl::Req     PREFIX = embperl_


char *
embperl_CurrPackage(r)
    tReq * r
CODE:
    if (r -> Buf.pFile)
        RETVAL = r -> Buf.pFile -> sCurrPackage ;
    else
        RETVAL = NULL ;
OUTPUT:
    RETVAL


char *
embperl_Sourcefile(r)
    tReq * r
CODE:
    if (r -> Buf.pFile)
        RETVAL = r -> Buf.pFile -> sSourcefile ;
    else
        RETVAL = NULL ;
OUTPUT:
    RETVAL


SV *
embperl_ApacheReq(r)
    tReq * r
CODE:
#ifdef APACHE
    ST(0) = r -> pApacheReqSV ;
    SvREFCNT_inc(ST(0)) ;
    sv_2mortal(ST(0));
#else
    ST(0) = &sv_undef ;
#endif



SV *
embperl_ErrArray(r)
    tReq * r
CODE:
    RETVAL = newRV_inc((SV *)r -> pErrArray) ;
OUTPUT:
    RETVAL





char *
embperl_VirtLogURI(r)
    tReq * r
CODE:
    if (r -> pConf)
        RETVAL = r -> pConf -> sVirtLogURI ;
    else
        RETVAL = NULL ;
OUTPUT:
    RETVAL


int
embperl_SubReq(r)
    tReq * r
CODE:
    RETVAL = r -> bSubReq ;
OUTPUT:
    RETVAL




int
embperl_logevalerr(r,sText)
    tReq * r
    char * sText
PREINIT:
    int l ;
CODE:
     l = strlen (sText) ;
     while (l > 0 && isspace(sText[l-1]))
        sText[--l] = '\0' ;

     strncpy (r -> errdat1, sText, sizeof (r -> errdat1) - 1) ;
     LogError (r, rcEvalErr) ;

int
embperl_logerror(r,code, sText)
    tReq * r
    int    code
    char * sText
CODE:
     strncpy (r->errdat1, sText, sizeof (r->errdat1) - 1) ;
     LogError (r,code) ;

int
embperl_getloghandle(r)
    tReq * r
CODE:
    RETVAL = GetLogHandle(r) ;
OUTPUT:
    RETVAL


long
embperl_getlogfilepos(r)
    tReq * r
CODE:
    OpenLog (r, "", 2) ;
    RETVAL = GetLogFilePos(r) ;
OUTPUT:
    RETVAL



void
embperl_output(r,sText)
    tReq * r
    char * sText
CODE:
    OutputToHtml (r,sText) ;


void
embperl_log(r,sText)
    tReq * r
    char * sText
CODE:
    OpenLog (r,"", 2) ;
    lwrite (r, sText, strlen (sText)) ;

void
embperl_flushlog(r)
    tReq * r
CODE:
    FlushLog (r) ;



int
embperl_getlineno(r)
    tReq * r
CODE:
    RETVAL = GetLineNo (r) ;
OUTPUT:
    RETVAL


void
log_svs(r,sText)
    tReq * r
    char * sText
CODE:
    lprintf (r,"[%d]MEM:  %s: SVs: %d OBJs: %d\n", r->nPid, sText, sv_count, sv_objcount) ;



int
embperl_ExecuteReq(r, param)
    tReq * r
    AV *   param = NO_INIT 
CODE:
    RETVAL = ExecuteReq(r, ST(0)) ; 
OUTPUT:
    RETVAL




int
embperl_Abort(r)
    tReq * r
CODE:
    FreeRequest(r) ;
    RETVAL = 0 ;
OUTPUT:
    RETVAL




void
embperl_FreeRequest(r)
    tReq * r
CODE:
    FreeRequest(r) ; 
