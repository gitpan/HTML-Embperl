/*###################################################################################
#
#   Embperl - Copyright (c) 1997-1999 Gerald Richter / ECOS
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


# /* ############################################################################### */

MODULE = HTML::Embperl      PACKAGE = HTML::Embperl     PREFIX = embperl_



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


# /* ---- Helper ----- */

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



char *
embperl_GVFile(gv)
    SV * gv
CODE:
    if (!gv)
	RETVAL = "" ;
    else
	{
	GV * fgv = GvFILEGV(gv) ;
	if (!fgv)
	    RETVAL = "" ;
	else
	    {
	    char * name = GvNAME (fgv) ;
	    if (name && SvTYPE(fgv) == SVt_PVGV)
		RETVAL = name ;
	    else
		RETVAL = "" ;
	    }
	}
OUTPUT:
    RETVAL


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


int
embperl_FreeConfData(pConf) 
    tConf *   pConf
CODE:
    FreeConfData(pConf) ;
    RETVAL = 1 ;
OUTPUT:
    RETVAL



# /* ----- Request data ----- */

tReq *
embperl_SetupRequest(req_rec,sInputfile,mtime,filesize,nFirstLine,sOutputfile,pConf,nIOtype,pIn,pOut,sSubName,sImport,nSessionMgnt) 
    SV *    req_rec
    char *  sInputfile
    double  mtime
    long    filesize
    int     nFirstLine
    char *  sOutputfile = NO_INIT
    tConf * pConf
    int     nIOtype
    SV *    pIn
    SV *    pOut
    char *  sSubName 
    char *  sImport
    int     nSessionMgnt
INIT:
    if (SvOK(ST(5)))
        sOutputfile = SvPV(ST(5), na);
    else
        sOutputfile = "\1" ; 
CODE:        
    RETVAL = SetupRequest(req_rec,sInputfile,mtime,filesize,nFirstLine,sOutputfile,pConf,nIOtype,pIn,pOut,sSubName,sImport,nSessionMgnt) ;
OUTPUT:
    RETVAL


tReq *
embperl_CurrReq(dummy)
    int dummy
CODE:        
    RETVAL = pCurrReq ;
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


int
embperl_ProcessSub(pFile, nBlockStart, nBlockNo)
    int     pFile
    int     nBlockStart
    int     nBlockNo
INIT:
    tReq * r = pCurrReq ;
CODE:
    RETVAL = ProcessSub(r,(tFile *)pFile, nBlockStart, nBlockNo) ;
OUTPUT:
    RETVAL

void
embperl_exit()
CODE:
    /* from mod_perl's perl_util.c */
    struct ufuncs umg;
	sv_magic(ERRSV, Nullsv, 'U', (char*) &umg, sizeof(umg));

	ENTER;
	SAVESPTR(diehook);
	diehook = Nullsv; 
	croak("");
	LEAVE; /* we don't get this far, but croak() will rewind */

	sv_unmagic(ERRSV, 'U');


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

SV *
embperl_ExportHash(r)
    tReq * r
CODE:
    if (r -> Buf.pFile && r -> Buf.pFile -> pExportHash)
	{
        ST(0) = newRV_inc((SV *)r -> Buf.pFile -> pExportHash) ;
	if (SvREFCNT(ST(0))) sv_2mortal(ST(0));
	}
    else
        ST(0) = &sv_undef ;


char *
embperl_Sourcefile(r)
    tReq * r
CODE:
    if (r -> Buf.pFile)
        RETVAL = r -> Buf.pFile -> sSourcefile ;
    else
        RETVAL = NULL;
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



SV *
embperl_FormArray(r)
    tReq * r
CODE:
    RETVAL = newRV_inc((SV *)r -> pFormArray) ;
OUTPUT:
    RETVAL


SV *
embperl_FormHash(r)
    tReq * r
CODE:
    RETVAL = newRV_inc((SV *)r -> pFormHash) ;
OUTPUT:
    RETVAL



SV *
embperl_EnvHash(r)
    tReq * r
CODE:
    RETVAL = newRV_inc((SV *)r -> pEnvHash) ;
OUTPUT:
    RETVAL




long
embperl_LogFileStartPos(r)
    tReq * r
CODE:
    RETVAL = r -> nLogFileStartPos ;
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




char *
embperl_CookieName(r)
    tReq * r
CODE:
    if (r -> pConf)
        RETVAL = r -> pConf -> sCookieName ;
    else
        RETVAL = NULL ;
OUTPUT:
    RETVAL


int
embperl_SessionMgnt(r,...)
    tReq * r
CODE:
    RETVAL = r -> nSessionMgnt ;
    if (items > 1)
        r -> nSessionMgnt = (int)SvIV(ST(1)) ;
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
embperl_Error(r)
    tReq * r
CODE:
    RETVAL = r -> bError ;
OUTPUT:
    RETVAL


int
embperl_ProcessBlock(r,nBlockStart,nBlockSize,nBlockNo)
    tReq * r
    int     nBlockStart
    int     nBlockSize
    int     nBlockNo
CODE:
    RETVAL = ProcessBlock(r,nBlockStart,nBlockSize,nBlockNo) ;
OUTPUT:
    RETVAL


int
embperl_ProcessSub(r,pFile,nBlockStart,nBlockNo)
    tReq * r
    int     pFile
    int     nBlockStart
    int     nBlockNo
CODE:
    RETVAL = ProcessSub(r,(tFile *)pFile, nBlockStart, nBlockNo) ;
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
