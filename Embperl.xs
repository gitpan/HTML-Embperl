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

static double
constant(name, arg)
char *name;
int arg;
{
    errno = 0;
    switch (*name) {
    case 'a':
	break;
    case 'b':
	break;
    case 'c':
	break;
    case 'd':
	break;
    case 'e':
	if (strEQ(name, "epDbgFlushOutput"))
#ifdef epDbgAll
	    return epDbgFlushOutput;
#else
	    goto not_there;
#endif
	if (strEQ(name, "epDbgFlushLog"))
#ifdef epDbgAll
	    return epDbgFlushLog;
#else
	    goto not_there;
#endif
	if (strEQ(name, "epDbgAll"))
#ifdef epDbgAll
	    return epDbgAll;
#else
	    goto not_there;
#endif
	if (strEQ(name, "epDbgCmd"))
#ifdef epDbgCmd
	    return epDbgCmd;
#else
	    goto not_there;
#endif
	if (strEQ(name, "epDbgEnv"))
#ifdef epDbgEnv
	    return epDbgEnv;
#else
	    goto not_there;
#endif
	if (strEQ(name, "epDbgEval"))
#ifdef epDbgEval
	    return epDbgEval;
#else
	    goto not_there;
#endif
	if (strEQ(name, "epDbgForm"))
#ifdef epDbgForm
	    return epDbgForm;
#else
	    goto not_there;
#endif
	if (strEQ(name, "epDbgInput"))
#ifdef epDbgInput
	    return epDbgInput;
#else
	    goto not_there;
#endif
	if (strEQ(name, "epDbgMem"))
#ifdef epDbgMem
	    return epDbgMem;
#else
	    goto not_there;
#endif
	if (strEQ(name, "epDbgStd"))
#ifdef epDbgStd
	    return epDbgStd;
#else
	    goto not_there;
#endif
	if (strEQ(name, "epDbgTab"))
#ifdef epDbgTab
	    return epDbgTab;
#else
	    goto not_there;
#endif
	if (strEQ(name, "epIOCGI"))
#ifdef epIOCGI
	    return epIOCGI;
#else
	    goto not_there;
#endif
	if (strEQ(name, "epIOMod_Perl"))
#ifdef epIOMod_Perl
	    return epIOMod_Perl;
#else
	    goto not_there;
#endif
	if (strEQ(name, "epIOPerl"))
#ifdef epIOPerl
	    return epIOPerl;
#else
	    goto not_there;
#endif
	if (strEQ(name, "epIOProcess"))
#ifdef epIOProcess
	    return epIOProcess;
#else
	    goto not_there;
#endif
	break;
    case 'f':
	break;
    case 'g':
	break;
    case 'h':
	break;
    case 'i':
	break;
    case 'j':
	break;
    case 'k':
	break;
    case 'l':
	break;
    case 'm':
	break;
    case 'n':
	break;
    case 'o':
	break;
    case 'p':
	break;
    case 'q':
	break;
    case 'r':
	break;
    case 's':
	break;
    case 't':
	break;
    case 'u':
	break;
    case 'v':
	break;
    case 'w':
	break;
    case 'x':
	break;
    case 'y':
	break;
    case 'z':
	break;
    }
    errno = EINVAL;
    return 0;

not_there:
    errno = ENOENT;
    return 0;
}


MODULE = HTML::Embperl		PACKAGE = HTML::Embperl


double
constant(name,arg)
	char *		name
	int		arg

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
embperl_req(sInputfile, sOutputfile, bDebugFlags, pNameSpace)
    char * sInputfile
    char * sOutputfile
    int bDebugFlags
    char * pNameSpace 
CODE:
    RETVAL = iembperl_req(sInputfile, sOutputfile, bDebugFlags, pNameSpace) ;
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

