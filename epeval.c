/*###################################################################################
#
#   Embperl - Copyright (c) 1997-1998 Gerald Richter / ECOS
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


int numEvals ;
int numCacheHits ;

int  bStrict ; /* aply use strict in each eval */

/* -------------------------------------------------------------------------------
*
* Eval PERL Statements 
* 
* in  sArg   Statement to eval
* out pRet   pointer to SV contains an CV to the evaled code
*
------------------------------------------------------------------------------- */

int EvalDirect (/*in*/    SV * pArg) 
    {
    dSP;
    
    EPENTRY (EvalDirect) ;

    tainted = 0 ;

    PUSHMARK(sp);
    perl_eval_sv(pArg, G_SCALAR | G_KEEPERR);


    if (SvTRUE (GvSV(errgv)))
         {
         STRLEN l ;
         char * p = SvPV (GvSV(errgv), l) ;
         if (l > sizeof (errdat1) - 1)
             l = sizeof (errdat1) - 1 ;
         strncpy (errdat1, p, l) ;
         if (l > 0 && errdat1[l-1] == '\n')
             l-- ;
         errdat1[l] = '\0' ;
         
	 LogError (rcEvalErr) ;

	 sv_setpv(GvSV(errgv),"");
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

static int EvalAll (/*in*/  const char *  sArg,
                    /*out*/ SV **         pRet)             
    {
    static char sFormat []       = "package %s ; sub { \n#line %d %s\n%s\n}" ;
    static char sFormatStrict [] = "package %s ; use strict ; sub {\n#line %d %s\n%s\n}" ; 
    SV *   pSVCmd ;
    dSP;
    
    EPENTRY (EvalAll) ;

    GetLineNo () ;

    if (bDebug & dbgDefEval)
        lprintf ("[%d]DEF:  Line %d: %s\n", nPid, nSourceline, sArg) ;

    tainted = 0 ;

    if (bStrict)
        pSVCmd = newSVpvf(sFormatStrict, sEvalPackage, nSourceline, sSourcefile, sArg) ;
    else
        pSVCmd = newSVpvf(sFormat, sEvalPackage, nSourceline, sSourcefile, sArg) ;

    PUSHMARK(sp);
    perl_eval_sv(pSVCmd, G_SCALAR | G_KEEPERR);
    SvREFCNT_dec(pSVCmd);

    SPAGAIN;
    *pRet = POPs;
    PUTBACK;

    if (bDebug & dbgMem)
        lprintf ("[%d]SVs:  %d\n", nPid, sv_count) ;
    
    if (SvTRUE (GvSV(errgv)))
         {
         STRLEN l ;
         char * p = SvPV (GvSV(errgv), l) ;
         if (l > sizeof (errdat1) - 1)
             l = sizeof (errdat1) - 1 ;
         strncpy (errdat1, p, l) ;
         if (l > 0 && errdat1[l-1] == '\n')
             l-- ;
         errdat1[l] = '\0' ;
         
	 *pRet = newSVpv (errdat1, 0) ;
         
         LogError (rcEvalErr) ;
	 sv_setpv(GvSV(errgv),"");
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

static int EvalAllNoCache (/*in*/  const char *  sArg,
                           /*out*/ SV **         pRet)             
    {
    int   num ;         
    int   nCountUsed = TableState.nCountUsed ;
    int   nRowUsed   = TableState.nRowUsed ;
    int   nColUsed   = TableState.nColUsed ;
#ifndef EVAL_SUB    
    SV *  pSVArg ;
#endif
    dSP;                            /* initialize stack pointer      */

    EPENTRY (EvalAll) ;

    if (bDebug & dbgEval)
        lprintf ("[%d]EVAL< %s\n", nPid, sArg) ;

    tainted = 0 ;

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
    
    if (bDebug & dbgMem)
        lprintf ("[%d]SVs:  %d\n", nPid, sv_count) ;
    /* pop the return value from stack */
    if (num == 1)   
        {
        *pRet = POPs ;
        SvREFCNT_inc (*pRet) ;

        if (bDebug & dbgEval)
            if (SvOK (*pRet))
                lprintf ("[%d]EVAL> %s\n", nPid, SvPV (*pRet, na)) ;
            else
                lprintf ("[%d]EVAL> <undefined>\n", nPid) ;
        
        if ((nCountUsed != TableState.nCountUsed ||
             nColUsed != TableState.nColUsed ||
             nRowUsed != TableState.nRowUsed) &&
              !SvOK (*pRet))
            {
            TableState.nResult = 0 ;
            SvREFCNT_dec (*pRet) ;
            *pRet = newSVpv("", 0) ;
            } 

        if ((bDebug & dbgTab) &&
            (TableState.nCountUsed ||
             TableState.nColUsed ||
             TableState.nRowUsed))
            lprintf ("[%d]TAB:  nResult = %d\n", nPid, TableState.nResult) ;
        }
     else
        {
        *pRet = NULL ;
        if (bDebug & dbgEval)
            lprintf ("[%d]EVAL> <NULL>\n", nPid) ;
        }

     PUTBACK;

     if (SvTRUE (GvSV(errgv)))
         {
         strncpy (errdat1, SvPV (GvSV(errgv), na), sizeof (errdat1) - 1) ;
         LogError (rcEvalErr) ;
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


static int Watch  ()

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


static int CallCV  (/*in*/  const char *  sArg,
                    /*in*/  CV *          pSub,
                    /*out*/ SV **         pRet)             
    {
    int   num ;         
    int   nCountUsed = TableState.nCountUsed ;
    int   nRowUsed   = TableState.nRowUsed ;
    int   nColUsed   = TableState.nColUsed ;
    int   bDynTab    = 0 ;

    SV *  pSVArg ;
    dSP;                            /* initialize stack pointer      */

    EPENTRY (CallCV) ;

    if (bDebug & dbgEval)
        lprintf ("[%d]EVAL< %s\n", nPid, sArg) ;

    tainted = 0 ;


    ENTER ;
    SAVETMPS ;
    PUSHMARK(sp);                   /* remember the stack pointer    */

    num = perl_call_sv ((SV *)pSub, G_SCALAR | G_EVAL | G_NOARGS) ; /* call the function             */
    
    SPAGAIN;                        /* refresh stack pointer         */
    
    if (bDebug & dbgMem)
        lprintf ("[%d]SVs:  %d\n", nPid, sv_count) ;
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

        if (bDebug & dbgEval)
            {
            if (SvOK (*pRet))
                lprintf ("[%d]EVAL> %s\n", nPid, SvPV (*pRet, na)) ;
            else
                lprintf ("[%d]EVAL> <undefined>\n", nPid) ;
            }                
            
        if ((nCountUsed != TableState.nCountUsed ||
             nColUsed != TableState.nColUsed ||
             nRowUsed != TableState.nRowUsed) &&
              !SvOK (*pRet))
            {
            TableState.nResult = 0 ;
            SvREFCNT_dec (*pRet) ;
            *pRet = newSVpv("", 0) ;
            } 

        if ((bDebug & dbgTab) &&
            (TableState.nCountUsed ||
             TableState.nColUsed ||
             TableState.nRowUsed))
            lprintf ("[%d]TAB:  nResult = %d\n", nPid, TableState.nResult) ;

        }
     else if (num == 0)
        {
        *pRet = NULL ;
        if (bDebug & dbgEval)
            lprintf ("[%d]EVAL> <NULL>\n", nPid) ;
        }
     else
        {
        *pRet = &sv_undef ;
        if (bDebug & dbgEval)
            lprintf ("[%d]EVAL> returns %d args instead of one\n", nPid, num) ;
        }

     /*if (SvREFCNT(*pRet) != 2)
            lprintf ("[%d]EVAL refcnt != 2 !!= %d !!!!!\n", nPid, SvREFCNT(*pRet)) ;*/


     PUTBACK;
     FREETMPS ;
     LEAVE ;

     if (SvTRUE (GvSV(errgv)))
         {
         STRLEN l ;
         char * p = SvPV (GvSV(errgv), l) ;
         if (l > sizeof (errdat1) - 1)
             l = sizeof (errdat1) - 1 ;
         strncpy (errdat1, p, l) ;
         if (l > 0 && errdat1[l-1] == '\n')
             l-- ;
         errdat1[l] = '\0' ;
         
	 LogError (rcEvalErr) ;

	 sv_setpv(GvSV(errgv),"");
         return rcEvalErr ;
         }

     if (bDebug & dbgWatchScalar)
         Watch () ;

    return num ;
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


static int EvalAndCall (/*in*/  const char *  sArg,
                        /*in*/  SV **         ppSV,
                        /*out*/ SV **         pRet)             


    {
    int     rc ;
    SV *   pSub ;
    
    
    EPENTRY (EvalAndCall) ;

    lastwarn[0] = '\0' ;
    
    rc = EvalAll (sArg, &pSub) ;

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
        else if (lastwarn[0] != '\0')
    	    *ppSV = newSVpv (lastwarn, 0) ;
        else
    	    *ppSV = newSVpv ("Compile Error", 0) ;
        
        *pRet = NULL ;
        bError = 1 ;
        return rc ;
        }

    if (*ppSV && SvTYPE (*ppSV) == SVt_PVCV)
        { /* Call the compiled eval */
        return CallCV (sArg, (CV *)*ppSV, pRet) ;
        }
    
    *pRet = NULL ;
    bError = 1 ;
    
    if (lastwarn[0] != '\0')
    	*ppSV = newSVpv (lastwarn, 0) ;
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

int Eval (/*in*/  const char *  sArg,
          /*in*/  int           nFilepos,
          /*out*/ SV **         pRet)             


    {
    int     rc ;
    SV **   ppSV ;
    
    
    EPENTRY (Eval) ;

    numEvals++ ;
    *pRet = NULL ;

    if (bDebug & dbgCacheDisable)
        return EvalAllNoCache (sArg, pRet) ;

    /* Already compiled ? */

    ppSV = hv_fetch(pCacheHash, (char *)&nFilepos, sizeof (nFilepos), 1) ;  
    if (ppSV == NULL)
        return rcHashError ;

    if (*ppSV != NULL && SvTYPE (*ppSV) == SVt_PV)
        {
        strncpy (errdat1, SvPV(*ppSV, na), sizeof (errdat1) - 1) ; 
        LogError (rcEvalErr) ;
        return rcEvalErr ;
        }

    if (*ppSV == NULL || SvTYPE (*ppSV) != SVt_PVCV)
        return EvalAndCall (sArg, ppSV, pRet) ;

    numCacheHits++ ;
    return CallCV (sArg, (CV *)*ppSV, pRet) ;
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


int EvalTrans (/*in*/  char *   sArg,
               /*in*/  int      nFilepos,
               /*out*/ SV **    pRet)             


    {
    int     rc ;
    SV **   ppSV ;
    
    EPENTRY (EvalTrans) ;


    numEvals++ ;
    *pRet = NULL ;

    if (bDebug & dbgCacheDisable)
        {
        /* strip off all <HTML> Tags */
        TransHtml (sArg) ;
        
        return EvalAllNoCache (sArg, pRet) ;
        }

    /* Already compiled ? */

    ppSV = hv_fetch(pCacheHash, (char *)&nFilepos, sizeof (nFilepos), 1) ;  
    if (ppSV == NULL)
        return rcHashError ;

    if (*ppSV != NULL && SvTYPE (*ppSV) == SVt_PV)
        {
        strncpy (errdat1, SvPV(*ppSV, na), sizeof (errdat1) - 1) ; 
        LogError (rcEvalErr) ;
        return rcEvalErr ;
        }

    if (*ppSV == NULL || SvTYPE (*ppSV) != SVt_PVCV)
        {
        /* strip off all <HTML> Tags */
        TransHtml (sArg) ;

        return EvalAndCall (sArg, ppSV, pRet) ;
        }

    numCacheHits++ ;
    
    return CallCV (sArg, (CV *)*ppSV, pRet) ;
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


int EvalTransOnFirstCall (/*in*/  char *   sArg,
                          /*in*/  int      nFilepos,
                          /*out*/ SV **    pRet)             


    {
    int     rc ;
    SV **   ppSV ;
    
    EPENTRY (EvalTrans) ;


    numEvals++ ;
    *pRet = NULL ;

    /* Already compiled ? */

    ppSV = hv_fetch(pCacheHash, (char *)&nFilepos, sizeof (nFilepos), 1) ;  
    if (ppSV == NULL)
        return rcHashError ;

    if (*ppSV != NULL && SvTYPE (*ppSV) == SVt_PV)
        {
        strncpy (errdat1, SvPV(*ppSV, na), sizeof (errdat1) - 1) ; 
        LogError (rcEvalErr) ;
        return rcEvalErr ;
        }

    if (*ppSV == NULL || SvTYPE (*ppSV) != SVt_PVCV)
        {
        /* strip off all <HTML> Tags */
        TransHtml (sArg) ;

        return EvalAndCall (sArg, ppSV, pRet) ;
        }

    numCacheHits++ ;
    
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



int EvalNum (/*in*/  char *        sArg,
             /*in*/  int           nFilepos,
             /*out*/ int *         pNum)             
    {
    SV * pRet ;
    int  n ;

    EPENTRY (EvalNum) ;


    n = Eval (sArg, nFilepos, &pRet) ;
    
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



int EvalBool (/*in*/  char *        sArg,
              /*in*/  int           nFilepos,
              /*out*/ int *         pTrue)             
    {
    SV * pRet ;
    int  n ;

    EPENTRY (EvalNum) ;


    n = Eval (sArg, nFilepos, &pRet) ;
    
    if (pRet)
        {
        *pTrue = SvTRUE (pRet) ;
        SvREFCNT_dec (pRet) ;
        }
    else
        *pTrue = 0 ;

    return ok ;
    }
    
