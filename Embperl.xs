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

#include "epnames.h"
#include "embperl.h"

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

MODULE = HTML::Embperl		PACKAGE = HTML::Embperl


int
embperl_constants()
CODE:
	RETVAL = constants () ;
OUTPUT:
    RETVAL


int
embperl_init(nIOType, sLogFile)
    int nIOType
    char * sLogFile
CODE:
    RETVAL = iembperl_init(nIOType, sLogFile) ;
OUTPUT:
    RETVAL

int
embperl_setreqrec(pReqSV)
    SV * pReqSV
CODE:
    RETVAL = iembperl_setreqrec(pReqSV) ;
OUTPUT:
    RETVAL

int
embperl_resetreqrec()
CODE:
    iembperl_resetreqrec() ;

#if defined (__GNUC__) && defined (__i386__)

int
embperl_dbgbreak()
CODE:
    __asm__ ("int   $0x03\n") ;

#endif


int
embperl_req(sInputfile, sOutputfile, bDebugFlags, bOptionFlags, nFileSize, pCache, pInData, pOutData)
    char * sInputfile
    char * sOutputfile
    int bDebugFlags
    int bOptionFlags
    int    nFileSize
    HV   * pCache = NO_INIT 
    SV   * pInData 
    SV   * pOutData 
INIT:
    pCache = (HV *)SvRV((SvRV(ST(5))));
CODE:
    RETVAL = iembperl_req(sInputfile, sOutputfile, bDebugFlags, bOptionFlags, nFileSize, pCache, pInData, pOutData) ; 
OUTPUT:
    RETVAL


int
embperl_term()
CODE:
    RETVAL = iembperl_term() ;
OUTPUT:
    RETVAL


int
embperl_logevalerr(sText)
    char * sText
CODE:
     int l = strlen (sText) ;
     while (l > 0 && isspace(sText[l-1]))
        sText[--l] = '\0' ;

     strncpy (errdat1, sText, sizeof (errdat1) - 1) ;
     LogError (rcEvalErr) ;

int
embperl_logerror(code, sText)
    int    code
    char * sText
CODE:
     strncpy (errdat1, sText, sizeof (errdat1) - 1) ;
     LogError (code) ;

int
embperl_getloghandle()
CODE:
    RETVAL = GetLogHandle() ;
OUTPUT:
    RETVAL


long
embperl_getlogfilepos()
CODE:
    RETVAL = GetLogFilePos() ;
OUTPUT:
    RETVAL



void
embperl_output(sText)
    char * sText
CODE:
    OutputToHtml (sText) ;


void
embperl_log(sText)
    char * sText
CODE:
    lwrite (sText, strlen (sText), 1) ;



int
embperl_getlineno()
CODE:
    RETVAL = GetLineNo () ;
OUTPUT:
    RETVAL


void
log_svs(sText)
    char * sText
CODE:
        lprintf ("[%d]MEM:  %s: SVs: %d OBJs: %d\n", nPid, sText, sv_count, sv_objcount) ;

