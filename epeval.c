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


int numEvals ;
int numCacheHits ;

/* -------------------------------------------------------------------------------
*
* Eval PERL Statements 
* 
* in  sArg   Statement to eval
* out pRet   pointer to SV contains an CV to the evaled code
*
------------------------------------------------------------------------------- */

#define EVAL_SUB

static int EvalAll (/*in*/  const char *  sArg,
                    /*out*/ SV **         pRet)             
    {
    int   num ;         
#ifndef EVAL_SUB    
    SV *  pSVArg ;
#endif
    dSP;                            /* initialize stack pointer      */

    EPENTRY (EvalAll) ;

    if (bDebug & dbgDefEval)
        lprintf ("[%d]DEF:  %s\n", nPid, sArg) ;

    tainted = 0 ;

#ifdef EVAL_SUB    

    ENTER;                          /* everything created after here */
    SAVETMPS;                       /* ...is a temporary variable.   */
    PUSHMARK(sp);                   /* remember the stack pointer    */
    XPUSHs(sv_2mortal(newSVpv((char *)sArg, strlen (sArg)))); /* push the base onto the stack  */
    PUTBACK;                        /* make local stack pointer global */
    num = perl_call_pv ("HTML::Embperl::_evalsub_", G_SCALAR | G_EVAL) ; /* call the function             */
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
        }
     else
        {
        *pRet = NULL ;
        }

     PUTBACK;

     num = ok ;

#ifdef EVAL_SUB    
    FREETMPS;                       /* free that return value        */
    LEAVE;                       /* ...and the XPUSHed "mortal" args.*/
#endif
    
     if (SvTRUE (GvSV(errgv)))
         {
         STRLEN l ;
         char * p = SvPV (GvSV(errgv), l) ;
         l = min (l, sizeof (errdat1) - 1) ;
         strncpy (errdat1, p, l) ;
         errdat1[l] = '\0' ;
         LogError (rcEvalErr) ;
	 num = rcEvalErr ;
         }

    return num ;
    }


/* -------------------------------------------------------------------------------
*
* Eval PERL Statements in safe namespace 
* 
* in  sArg   Statement to eval
* out pRet   pointer to SV contains an CV to the evaled code
*
------------------------------------------------------------------------------- */


static int EvalSafe (/*in*/  const char *  sArg,
                     /*out*/ SV **         pRet)             
    {
    int   num ;         
    int   nCountUsed = TableState.nCountUsed ;
    int   nRowUsed   = TableState.nRowUsed ;
    int   nColUsed   = TableState.nColUsed ;
    dSP;                            /* initialize stack pointer      */

    EPENTRY (EvalSafe) ;

    if (bDebug & dbgEval)
        lprintf ("[%d]EVAL< %s\n", nPid, sArg) ;


    ENTER;                          /* everything created after here */
    SAVETMPS;                       /* ...is a temporary variable.   */
    PUSHMARK(sp);                   /* remember the stack pointer    */
    XPUSHs(pNameSpace) ;
    XPUSHs(sv_2mortal(newSVpv((char *)sArg, strlen (sArg)))); 
    PUTBACK;                        /* make local stack pointer global */
    num = perl_call_method ("reval", G_SCALAR | G_EVAL) ;  /* call the function             */
    SPAGAIN;                        /* refresh stack pointer         */
    
    if (bDebug & dbgMem)
        lprintf ("[%d]SVs:  %d\n", nPid, sv_count) ;
    /* pop the return value from stack */
    if (num == 1)   
        {
        *pRet = POPs ;
        SvREFCNT_inc (*pRet) ;
        if ((nCountUsed != TableState.nCountUsed ||
             nColUsed != TableState.nColUsed ||
             nRowUsed != TableState.nRowUsed) &&
              !SvOK (*pRet))
            TableState.nResult = 0 ;

        if ((bDebug & dbgTab) &&
            (TableState.nCountUsed ||
             TableState.nColUsed ||
             TableState.nRowUsed))
            lprintf ("[%d]TAB:  nResult = %d\n", nPid, TableState.nResult) ;

        if (bDebug & dbgEval)
            if (SvOK (*pRet))
                lprintf ("[%d]EVAL> %s\n", nPid, SvPV (*pRet, na)) ;
            else
                lprintf ("[%d]EVAL> <undefined>\n",nPid) ;
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
         }

    
    
    FREETMPS;                       /* free that return value        */
    LEAVE;                       /* ...and the XPUSHed "mortal" args.*/
    
    return num ;
    }

/* -------------------------------------------------------------------------------
*
* Eval PERL Statements without any caching of p-code
* 
* in  sArg   Statement to eval
* out pRet   pointer to SV contains the eval return
*
------------------------------------------------------------------------------- */

int EvalAllNoCache (/*in*/  const char *  sArg,
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
    num = perl_call_pv ("HTML::Embperl::_eval_", G_SCALAR /*| G_EVAL*/) ; /* call the function             */
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
        if ((nCountUsed != TableState.nCountUsed ||
             nColUsed != TableState.nColUsed ||
             nRowUsed != TableState.nRowUsed) &&
              !SvOK (*pRet))
            TableState.nResult = 0 ;

        if ((bDebug & dbgTab) &&
            (TableState.nCountUsed ||
             TableState.nColUsed ||
             TableState.nRowUsed))
            lprintf ("[%d]TAB:  nResult = %d\n", nPid, TableState.nResult) ;

        if (bDebug & dbgEval)
            if (SvOK (*pRet))
                lprintf ("[%d]EVAL> %s\n", nPid, SvPV (*pRet, na)) ;
            else
                lprintf ("[%d]EVAL> <undefined>\n", nPid) ;
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
        SvREFCNT_inc (*pRet) ;
        if ((nCountUsed != TableState.nCountUsed ||
             nColUsed != TableState.nColUsed ||
             nRowUsed != TableState.nRowUsed) &&
              !SvOK (*pRet))
            TableState.nResult = 0 ;

        if ((bDebug & dbgTab) &&
            (TableState.nCountUsed ||
             TableState.nColUsed ||
             TableState.nRowUsed))
            lprintf ("[%d]TAB:  nResult = %d\n", nPid, TableState.nResult) ;

        if (bDebug & dbgEval)
            if (SvOK (*pRet))
                lprintf ("[%d]EVAL> %s\n", nPid, SvPV (*pRet, na)) ;
            else
                lprintf ("[%d]EVAL> <undefined>\n", nPid) ;
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
            lprintf ("[%d]EVAL> returns %d args\n", nPid, num) ;
        }

     if (SvREFCNT(*pRet) != 2)
            lprintf ("[%d]EVAL refcnt != 2 !!= %d !!!!!\n", nPid, SvREFCNT(*pRet)) ;


     PUTBACK;
     FREETMPS ;
     LEAVE ;

     if (SvTRUE (GvSV(errgv)))
         {
         strncpy (errdat1, SvPV (GvSV(errgv), na), sizeof (errdat1) - 1) ;
         LogError (rcEvalErr) ;
         }


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
    
    
    EPENTRY (EvalAndCall) ;

    rc = EvalAll (sArg, pRet) ;

    if (rc == ok && *pRet != NULL && SvTYPE (*pRet) == SVt_RV)
        *ppSV = SvRV(*pRet) ;
    else
        return rc ;

    if (SvTYPE (*ppSV) == SVt_PVCV)
        { /* Call the compiled eval */
        return CallCV (sArg, (CV *)*ppSV, pRet) ;
        }
    
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

    if (bDebug & dbgCacheDisable)
        return EvalAllNoCache (sArg, pRet) ;

    if (bSafeEval)
        return EvalSafe (sArg, pRet) ;

    /* Already compiled ? */

    ppSV = hv_fetch(pCacheHash, (char *)&nFilepos, sizeof (nFilepos), 1) ;  
    if (ppSV == NULL)
        return rcHashError ;


    if (SvTYPE (*ppSV) != SVt_PVCV)
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
* out pRet      pointer to SV contains the eval return
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

    if (bDebug & dbgCacheDisable)
        return EvalAllNoCache (sArg, pRet) ;

    if (bSafeEval)
        return EvalSafe (sArg, pRet) ;

    /* Already compiled ? */

    ppSV = hv_fetch(pCacheHash, (char *)&nFilepos, sizeof (nFilepos), 1) ;  
    if (ppSV == NULL)
        return rcHashError ;

    if (SvTYPE (*ppSV) != SVt_PVCV)
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
* strip off all <HTML> Tags before 
* 
* in  sArg      Statement to eval
* in  nFilepos  position von eval in file (is used to build an unique key)
* out pNum      pointer to int contains the eval return
*
------------------------------------------------------------------------------- */



int EvalNum (/*in*/  char *        sArg,
             /*in*/  int           nFilepos,
             /*out*/ int *         pNum)             
    {
    SV * pRet ;
    int  n ;

    EPENTRY (EvalNum) ;


    n = EvalTrans (sArg, nFilepos, &pRet) ;
    
    if (pRet)
        {
        *pNum = SvIV (pRet) ;
        SvREFCNT_dec (pRet) ;
        }
    else
        pNum = 0 ;

    return ok ;
    }
    
