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
#define CONST(NAME) \
        pSV = newSViv (NAME) ; \
        if (hv_store (pHash, #NAME, sizeof (#NAME) - 1, pSV, 0) == NULL) \
            return rcHashError ;

static int constants()
	{

	HV * pHash ;
	SV * pSV ;

        if ((pHash = perl_get_hv ("CONSTANT", TRUE)) == NULL)
            return rcHashError ;


	
	CONST(epIOCGI)
	CONST(epIOMod_Perl)
	CONST(epIOPerl)
	CONST(epIOProcess)

	CONST(ok)
	CONST(rcStackOverflow) 
	CONST(rcStackUnderflow)
	CONST(rcEndifWithoutIf)
	CONST(rcElseWithoutIf)
	CONST(rcEndwhileWithoutWhile)
	CONST(rcEndtableWithoutTable)
	CONST(rcCmdNotFound) 
	CONST(rcOutOfMemory) 
	CONST(rcPerlVarError)
	CONST(rcHashError) 
	CONST(rcArrayError) 
	CONST(rcFileOpenErr) 
	CONST(rcMissingRight)
	CONST(rcNoRetFifo) 
	CONST(rcMagicError)
	CONST(rcWriteErr) 
	CONST(rcUnknownNameSpace) 
	CONST(rcInputNotSupported) 
	CONST(rcCannotUsedRecursive) 
	CONST(rcEndtableWithoutTablerow) 
	CONST(rcEndtextareaWithoutTextarea)
	CONST(rcArgStackOverflow) 
	CONST(rcEvalErr) 
	CONST(rcNotCompiledForModPerl)
	CONST(rcLogFileOpenErr)
	CONST(rcExecCGIMissing)
	CONST(rcIsDir)
	CONST(rcXNotSet) 
	CONST(rcNotFound) 
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


int
embperl_req(sInputfile, sOutputfile, bDebugFlags, pNameSpace, nFileSize)
    char * sInputfile
    char * sOutputfile
    int bDebugFlags
    char * pNameSpace 
    int    nFileSize
CODE:
    RETVAL = iembperl_req(sInputfile, sOutputfile, bDebugFlags, pNameSpace, nFileSize) ;
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

int
embperl_output(sText)
    char * sText
CODE:
    OutputToHtml (sText) ;

