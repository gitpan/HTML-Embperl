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


/* -------------------------------------------------------------------------------
*
* Eval PERL Statements 
* 
* in  sArg   Statement to eval
* out pRet   pointer to SV contains an CV to the evaled code
*
------------------------------------------------------------------------------- */

int EvalDirect (/*i/o*/ register req * r,
			/*in*/    SV * pArg) 
    {
    dSP;
    SV *  pSVErr  ;

    EPENTRY (EvalDirect) ;

    tainted = 0 ;
    pCurrReq = r ;

    PUSHMARK(sp);
    perl_eval_sv(pArg, G_SCALAR | G_KEEPERR);


    pSVErr = ERRSV ;
    if (SvTRUE (pSVErr))
	{
        STRLEN l ;
        char * p = SvPV (pSVErr, l) ;
        if (l > sizeof (r -> errdat1) - 1)
            l = sizeof (r -> errdat1) - 1 ;
        strncpy (r -> errdat1, p, l) ;
        if (l > 0 && r -> errdat1[l-1] == '\n')
            l-- ;
        r -> errdat1[l] = '\0' ;
         
	LogError (r, rcEvalErr) ;

	sv_setpv(pSVErr,"");
        return rcEvalErr ;
        }

    return ok ;
    }



/* -------------------------------------------------------------------------------
*
* Eval PERL Statements into a sub 
* 
* in  sArg   Statement to eval
* out pRet   pointer to SV contains an CV to the evaled code
*
------------------------------------------------------------------------------- */

static int EvalAll (/*i/o*/ register req * r,
			/*in*/  const char *  sArg,
                    /*in*/  int           flags,
                    /*out*/ SV **         pRet)             
    {
    static char sFormat []       = "package %s ; sub { \n#line %d %s\n%s\n}" ;
    static char sFormatStrict [] = "package %s ; use strict ; sub {\n#line %d %s\n%s\n}" ; 
    static char sFormatArray []       = "package %s ; sub { \n#line %d %s\n[%s]\n}" ;
    static char sFormatStrictArray [] = "package %s ; use strict ; sub {\n#line %d %s\n[%s]\n}" ; 
    SV *   pSVCmd ;
    SV *   pSVErr ;

    dSP;
    
    EPENTRY (EvalAll) ;

    GetLineNo (r) ;

    if (r -> bDebug & dbgDefEval)
        lprintf (r, "[%d]DEF:  Line %d: %s\n", r -> nPid, r -> Buf.nSourceline, sArg) ;

    tainted = 0 ;
    pCurrReq = r ;

    if (r -> bStrict)
        if (flags & G_ARRAY)
            pSVCmd = newSVpvf(sFormatStrictArray, r -> Buf.sEvalPackage, r -> Buf.nSourceline, r -> Buf.pFile -> sSourcefile, sArg) ;
        else
            pSVCmd = newSVpvf(sFormatStrict, r -> Buf.sEvalPackage, r -> Buf.nSourceline, r -> Buf.pFile -> sSourcefile, sArg) ;
    else
        if (flags & G_ARRAY)
            pSVCmd = newSVpvf(sFormatArray, r -> Buf.sEvalPackage, r -> Buf.nSourceline, r -> Buf.pFile -> sSourcefile, sArg) ;
        else
            pSVCmd = newSVpvf(sFormat, r -> Buf.sEvalPackage, r -> Buf.nSourceline, r -> Buf.pFile -> sSourcefile, sArg) ;

    PUSHMARK(sp);
    perl_eval_sv(pSVCmd, G_SCALAR | G_KEEPERR);
    SvREFCNT_dec(pSVCmd);

    SPAGAIN;
    *pRet = POPs;
    PUTBACK;

    if (r -> bDebug & dbgMem)
        lprintf (r, "[%d]SVs:  %d\n", r -> nPid, sv_count) ;
    
    pSVErr = ERRSV ;
    if (SvTRUE (pSVErr))
        {
        STRLEN l ;
        char * p = SvPV (pSVErr, l) ;
        if (l > sizeof (r -> errdat1) - 1)
            l = sizeof (r -> errdat1) - 1 ;
        strncpy (r -> errdat1, p, l) ;
        if (l > 0 && r -> errdat1[l-1] == '\n')
            l-- ;
        r -> errdat1[l] = '\0' ;
         
	*pRet = newSVpv (r -> errdat1, 0) ;
         
        LogError (r, rcEvalErr) ;
	sv_setpv(pSVErr, "");
        return rcEvalErr ;
        }

    return ok ;
    }


/* -------------------------------------------------------------------------------
*
* Eval PERL Statements without any caching of p-code
* 
* in  sArg   Statement to eval
* out pRet   pointer to SV contains the eval return
*
------------------------------------------------------------------------------- */

#define EVAL_SUB

static int EvalAllNoCache (/*i/o*/ register req * r,
			/*in*/  const char *  sArg,
                           /*out*/ SV **         pRet)             
    {
    int   num ;         
    int   nCountUsed = r -> TableStack.State.nCountUsed ;
    int   nRowUsed   = r -> TableStack.State.nRowUsed ;
    int   nColUsed   = r -> TableStack.State.nColUsed ;
#ifndef EVAL_SUB    
    SV *  pSVArg ;
#endif
    SV *  pSVErr ;
    dSP;                            /* initialize stack pointer      */

    EPENTRY (EvalAll) ;

    if (r -> bDebug & dbgEval)
        lprintf (r, "[%d]EVAL< %s\n", r -> nPid, sArg) ;

    tainted = 0 ;
    pCurrReq = r ;

#ifdef EVAL_SUB    

    ENTER;                          /* everything created after here */
    SAVETMPS;                       /* ...is a temporary variable.   */
    PUSHMARK(sp);                   /* remember the stack pointer    */
    XPUSHs(sv_2mortal(newSVpv((char *)sArg, strlen (sArg)))); /* push the base onto the stack  */
    PUTBACK;                        /* make local stack pointer global */
    num = perl_call_pv ("_eval_", G_SCALAR /*| G_EVAL*/) ; /* call the function             */
#else
    
    pSVArg = sv_2mortal(newSVpv((char *)sArg, strlen (sArg))) ;

    /*num = perl_eval_sv (pSVArg, G_SCALAR) ; /* call the function             */ */
    num = perl_eval_sv (pSVArg, G_DISCARD) ; /* call the function             */
    num = 0 ;
#endif    
    SPAGAIN;                        /* refresh stack pointer         */
    
    if (r -> bDebug & dbgMem)
        lprintf (r, "[%d]SVs:  %d\n", r -> nPid, sv_count) ;
    /* pop the return value from stack */
    if (num == 1)   
        {
        *pRet = POPs ;
        SvREFCNT_inc (*pRet) ;

        if (r -> bDebug & dbgEval)
            if (SvOK (*pRet))
                lprintf (r, "[%d]EVAL> %s\n", r -> nPid, SvPV (*pRet, na)) ;
            else
                lprintf (r, "[%d]EVAL> <undefined>\n", r -> nPid) ;
        
        if ((nCountUsed != r -> TableStack.State.nCountUsed ||
             nColUsed != r -> TableStack.State.nColUsed ||
             nRowUsed != r -> TableStack.State.nRowUsed) &&
              !SvOK (*pRet))
            {
            r -> TableStack.State.nResult = 0 ;
            SvREFCNT_dec (*pRet) ;
            *pRet = newSVpv("", 0) ;
            } 

        if ((r -> bDebug & dbgTab) &&
            (r -> TableStack.State.nCountUsed ||
             r -> TableStack.State.nColUsed ||
             r -> TableStack.State.nRowUsed))
            lprintf (r, "[%d]TAB:  nResult = %d\n", r -> nPid, r -> TableStack.State.nResult) ;
        }
    else
        {
        *pRet = NULL ;
        if (r -> bDebug & dbgEval)
            lprintf (r, "[%d]EVAL> <NULL>\n", r -> nPid) ;
        }

	PUTBACK;

    pSVErr = ERRSV ;
    if (SvTRUE (pSVErr))
        {
        strncpy (r -> errdat1, SvPV (pSVErr, na), sizeof (r -> errdat1) - 1) ;
        LogError (r, rcEvalErr) ;
	num = rcEvalErr ;
        }
    else
        num = ok ;



#ifdef EVAL_SUB    
    FREETMPS;                       /* free that return value        */
    LEAVE;                       /* ...and the XPUSHed "mortal" args.*/
#endif
    
    return num ;
    }

/* -------------------------------------------------------------------------------
*
* Watch if there are any variables changed
* 
------------------------------------------------------------------------------- */


static int Watch (/*i/o*/ register req * r)

    {
    dSP;                            /* initialize stack pointer      */

    EPENTRY (Watch) ;

    PUSHMARK(sp);                   /* remember the stack pointer    */

    perl_call_pv ("HTML::Embperl::watch", G_DISCARD | G_NOARGS) ; /* call the function             */
    
    return ok ;
    }

/* -------------------------------------------------------------------------------
*
* Call an already evaled PERL Statement
* 
* in  sArg   Statement to eval (only used for logging)
* in  pSub   CV which should be called
* out pRet   pointer to SV contains the eval return
*
------------------------------------------------------------------------------- */


static int CallCV (/*i/o*/ register req * r,
			/*in*/  const char *  sArg,
                    /*in*/  CV *          pSub,
                    /*in*/  int           flags,
                    /*out*/ SV **         pRet)             
    {
    int   num ;         
    int   nCountUsed = r -> TableStack.State.nCountUsed ;
    int   nRowUsed   = r -> TableStack.State.nRowUsed ;
    int   nColUsed   = r -> TableStack.State.nColUsed ;
    int   bDynTab    = 0 ;
    SV *  pSVErr ;

    SV *  pSVArg ;
    dSP;                            /* initialize stack pointer      */

    EPENTRY (CallCV) ;

    if (r -> bDebug & dbgEval)
        lprintf (r, "[%d]EVAL< %s\n", r -> nPid, sArg) ;

    tainted = 0 ;
    pCurrReq = r ;

    ENTER ;
    SAVETMPS ;
    PUSHMARK(sp);                   /* remember the stack pointer    */

    num = perl_call_sv ((SV *)pSub, flags | G_EVAL | G_NOARGS) ; /* call the function             */
    
    SPAGAIN;                        /* refresh stack pointer         */
    
    if (r -> bDebug & dbgMem)
        lprintf (r, "[%d]SVs:  %d\n", r -> nPid, sv_count) ;
    /* pop the return value from stack */
    if (num == 1)   
        {
        *pRet = POPs ;
        if (SvTYPE (*pRet) == SVt_PVMG)
            { /* variable is magicaly -> fetch value now */
            SV * pSV = newSVsv (*pRet) ;
            *pRet = pSV ;
            }
        else        
            SvREFCNT_inc (*pRet) ;

        if (r -> bDebug & dbgEval)
            {
            if (SvOK (*pRet))
                lprintf (r, "[%d]EVAL> %s\n", r -> nPid, SvPV (*pRet, na)) ;
            else
                lprintf (r, "[%d]EVAL> <undefined>\n", r -> nPid) ;
            }                
            
        if ((nCountUsed != r -> TableStack.State.nCountUsed ||
             nColUsed != r -> TableStack.State.nColUsed ||
             nRowUsed != r -> TableStack.State.nRowUsed) &&
              !SvOK (*pRet))
            {
            r -> TableStack.State.nResult = 0 ;
            SvREFCNT_dec (*pRet) ;
            *pRet = newSVpv("", 0) ;
            } 

        if ((r -> bDebug & dbgTab) &&
            (r -> TableStack.State.nCountUsed ||
             r -> TableStack.State.nColUsed ||
             r -> TableStack.State.nRowUsed))
            lprintf (r, "[%d]TAB:  nResult = %d\n", r -> nPid, r -> TableStack.State.nResult) ;

        }
     else if (num == 0)
        {
        *pRet = NULL ;
        if (r -> bDebug & dbgEval)
            lprintf (r, "[%d]EVAL> <NULL>\n", r -> nPid) ;
        }
     else
        {
        *pRet = &sv_undef ;
        if (r -> bDebug & dbgEval)
            lprintf (r, "[%d]EVAL> returns %d args instead of one\n", r -> nPid, num) ;
        }

     /*if (SvREFCNT(*pRet) != 2)
            lprintf (r, "[%d]EVAL refcnt != 2 !!= %d !!!!!\n", r -> nPid, SvREFCNT(*pRet)) ;*/

     PUTBACK;
     FREETMPS ;
     LEAVE ;


     pSVErr = ERRSV ;
     if (SvTRUE (pSVErr))
        {
        STRLEN l ;
        char * p ;

        if (SvMAGICAL (pSVErr) && mg_find (pSVErr, 'U'))
            {
 	    /* On an Apache::exit call, the function croaks with error having 'U' magic.
 	     * When we get this return, we'll just give up and quit this file completely,
 	     * without error. */
             
            struct magic * m = SvMAGIC (pSVErr) ;

            sv_unmagic(pSVErr,'U');
	    sv_setpv(pSVErr,"");

            return rcExit ;
            }

        p = SvPV (pSVErr, l) ;
        if (l > sizeof (r -> errdat1) - 1)
            l = sizeof (r -> errdat1) - 1 ;
        strncpy (r -> errdat1, p, l) ;
        if (l > 0 && r -> errdat1[l-1] == '\n')
             l-- ;
        r -> errdat1[l] = '\0' ;
         
	LogError (r, rcEvalErr) ;

	sv_setpv(pSVErr,"");

	return rcEvalErr ;
        }

     if (r -> bDebug & dbgWatchScalar)
         Watch (r) ;

     
    return ok ;
    }

/* -------------------------------------------------------------------------------
*
* Eval PERL Statements and execute the evaled code
* 
* in  sArg   Statement to eval
* out ppSV   pointer to an SV with should be set to CV of the evaled code
* out pRet   pointer to SV contains the eval return
*
------------------------------------------------------------------------------- */


static int EvalAndCall (/*i/o*/ register req * r,
			/*in*/  const char *  sArg,
                        /*in*/  SV **         ppSV,
                        /*in*/  int           flags,
                        /*out*/ SV **         pRet)             


    {
    int     rc ;
    SV *   pSub ;
    
    
    EPENTRY (EvalAndCall) ;

    r -> lastwarn[0] = '\0' ;
    
    rc = EvalAll (r, sArg, flags, &pSub) ;

    if (rc == ok && pSub != NULL && SvTYPE (pSub) == SVt_RV)
        {
        /*sv_setsv (*ppSV, pSub) ;*/
        SvREFCNT_dec (*ppSV) ;  
        *ppSV = SvRV(pSub) ;
        SvREFCNT_inc (*ppSV) ;  
        }
    else
        {
        if (pSub != NULL && SvTYPE (pSub) == SVt_PV)
            *ppSV = pSub ; /* save error message */
        else if (r -> lastwarn[0] != '\0')
    	    *ppSV = newSVpv (r -> lastwarn, 0) ;
        else
    	    *ppSV = newSVpv ("Compile Error", 0) ;
        
        *pRet = NULL ;
        r -> bError = 1 ;
        return rc ;
        }

    if (*ppSV && SvTYPE (*ppSV) == SVt_PVCV)
        { /* Call the compiled eval */
        return CallCV (r, sArg, (CV *)*ppSV, flags, pRet) ;
        }
    
    *pRet = NULL ;
    r -> bError = 1 ;
    
    if (r -> lastwarn[0] != '\0')
    	*ppSV = newSVpv (r -> lastwarn, 0) ;
    else
    	*ppSV = newSVpv ("Compile Error", 0) ;

    return rcEvalErr ;
    }


/* -------------------------------------------------------------------------------
*
* Eval PERL Statements and execute the evaled code, check if it's already compiled
* 
* in  sArg      Statement to eval
* in  nFilepos  position von eval in file (is used to build an unique key)
* out pRet      pointer to SV contains the eval return
*
------------------------------------------------------------------------------- */

int Eval (/*i/o*/ register req * r,
			/*in*/  const char *  sArg,
          /*in*/  int           nFilepos,
          /*out*/ SV **         pRet)             


    {
    int     rc ;
    SV **   ppSV ;
    
    
    EPENTRY (Eval) ;

    r -> numEvals++ ;
    *pRet = NULL ;

    if (r -> bDebug & dbgCacheDisable)
        return EvalAllNoCache (r, sArg, pRet) ;

    /* Already compiled ? */

    ppSV = hv_fetch(r -> Buf.pFile -> pCacheHash, (char *)&nFilepos, sizeof (nFilepos), 1) ;  
    if (ppSV == NULL)
        return rcHashError ;

    if (*ppSV != NULL && SvTYPE (*ppSV) == SVt_PV)
        {
        strncpy (r -> errdat1, SvPV(*ppSV, na), sizeof (r -> errdat1) - 1) ; 
        LogError (r, rcEvalErr) ;
        return rcEvalErr ;
        }

    if (*ppSV == NULL || SvTYPE (*ppSV) != SVt_PVCV)
        return EvalAndCall (r, sArg, ppSV, G_SCALAR, pRet) ;

    r -> numCacheHits++ ;
    return CallCV (r, sArg, (CV *)*ppSV, G_SCALAR, pRet) ;
    }


/* -------------------------------------------------------------------------------
*
* Eval PERL Statements and execute the evaled code, check if it's already compiled
* strip off all <HTML> Tags before 
* 
* in  sArg      Statement to eval
* in  nFilepos  position von eval in file (is used to build an unique key)
* out pRet      pointer to SV contains the eval return value
*
------------------------------------------------------------------------------- */


int EvalTransFlags (/*i/o*/ register req * r,
			/*in*/  char *   sArg,
                    /*in*/  int      nFilepos,
                    /*in*/  int      flags,
                    /*out*/ SV **    pRet)             


    {
    int     rc ;
    SV **   ppSV ;
    
    EPENTRY (EvalTrans) ;


    r -> numEvals++ ;
    *pRet = NULL ;

    if (r -> bDebug & dbgCacheDisable)
        {
        /* strip off all <HTML> Tags */
        TransHtml (r, sArg) ;
        
        return EvalAllNoCache (r, sArg, pRet) ;
        }

    /* Already compiled ? */

    ppSV = hv_fetch(r -> Buf.pFile -> pCacheHash, (char *)&nFilepos, sizeof (nFilepos), 1) ;  
    if (ppSV == NULL)
        return rcHashError ;

    if (*ppSV != NULL && SvTYPE (*ppSV) == SVt_PV)
        {
        strncpy (r -> errdat1, SvPV(*ppSV, na), sizeof (r -> errdat1) - 1) ; 
        LogError (r, rcEvalErr) ;
        return rcEvalErr ;
        }

    if (*ppSV == NULL || SvTYPE (*ppSV) != SVt_PVCV)
        {
        /* strip off all <HTML> Tags */
        TransHtml (r, sArg) ;

        return EvalAndCall (r, sArg, ppSV, flags, pRet) ;
        }

    r -> numCacheHits++ ;
    
    return CallCV (r, sArg, (CV *)*ppSV, flags, pRet) ;
    }


int EvalTrans (/*i/o*/ register req * r,
			/*in*/  char *   sArg,
                    /*in*/  int      nFilepos,
                    /*out*/ SV **    pRet)             
    
    {
    return EvalTransFlags (r, sArg, nFilepos, G_SCALAR, pRet) ;
    }
    
    
/* -------------------------------------------------------------------------------
*
* Eval PERL Statements and execute the evaled code, check if it's already compiled
* if yes do not call the code a second time
* strip off all <HTML> Tags before 
* 
* in  sArg      Statement to eval
* in  nFilepos  position von eval in file (is used to build an unique key)
* out pRet      pointer to SV contains the eval return value
*
------------------------------------------------------------------------------- */


int EvalTransOnFirstCall (/*i/o*/ register req * r,
			/*in*/  char *   sArg,
                          /*in*/  int      nFilepos,
                          /*out*/ SV **    pRet)             


    {
    int     rc ;
    SV **   ppSV ;
    
    EPENTRY (EvalTrans) ;


    r -> numEvals++ ;
    *pRet = NULL ;

    /* Already compiled ? */

    ppSV = hv_fetch(r -> Buf.pFile -> pCacheHash, (char *)&nFilepos, sizeof (nFilepos), 1) ;  
    if (ppSV == NULL)
        return rcHashError ;

    if (*ppSV != NULL && SvTYPE (*ppSV) == SVt_PV)
        {
        strncpy (r -> errdat1, SvPV(*ppSV, na), sizeof (r -> errdat1) - 1) ; 
        LogError (r, rcEvalErr) ;
        return rcEvalErr ;
        }

    if (*ppSV == NULL || SvTYPE (*ppSV) != SVt_PVCV)
        {
        /* strip off all <HTML> Tags */
        TransHtml (r, sArg) ;

        return EvalAndCall (r, sArg, ppSV, G_SCALAR, pRet) ;
        }

    r -> numCacheHits++ ;
    
    return ok ; /* Do not call this a second time */
    }



/* -------------------------------------------------------------------------------
*
* Eval PERL Statements and execute the evaled code, check if it's already compiled
* 
* in  sArg      Statement to eval
* in  nFilepos  position von eval in file (is used to build an unique key)
* out pNum      pointer to int, contains the eval return value
*
------------------------------------------------------------------------------- */



int EvalNum (/*i/o*/ register req * r,
			/*in*/  char *        sArg,
             /*in*/  int           nFilepos,
             /*out*/ int *         pNum)             
    {
    SV * pRet ;
    int  n ;

    EPENTRY (EvalNum) ;


    n = Eval (r, sArg, nFilepos, &pRet) ;
    
    if (pRet)
        {
        *pNum = SvIV (pRet) ;
        SvREFCNT_dec (pRet) ;
        }
    else
        *pNum = 0 ;

    return ok ;
    }
    

/* -------------------------------------------------------------------------------
*
* EvalBool PERL Statements and execute the evaled code, check if it's already compiled
* 
* in  sArg      Statement to eval
* in  nFilepos  position von eval in file (is used to build an unique key)
* out pTrue     return 1 if evaled expression is true
*
------------------------------------------------------------------------------- */



int EvalBool (/*i/o*/ register req * r,
	      /*in*/  char *        sArg,
              /*in*/  int           nFilepos,
              /*out*/ int *         pTrue)             
    {
    SV * pRet ;
    int  rc ;

    EPENTRY (EvalNum) ;


    rc = Eval (r, sArg, nFilepos, &pRet) ;
    
    if (pRet)
        {
        *pTrue = SvTRUE (pRet) ;
        SvREFCNT_dec (pRet) ;
        }
    else
        *pTrue = 0 ;

    return rc ;
    }
    
