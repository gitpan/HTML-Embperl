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


#define INTMG(name,var,used,sub) \
    \
int EMBPERL_mgGet##name (SV * pSV, MAGIC * mg) \
\
    { \
    sv_setiv (pSV, var) ; \
    used++ ; \
    if ((bDebug & dbgTab) && bReqRunning) \
        lprintf ("[%d]TAB:  get %s = %d, Used = %d\n", nPid, #name, var, used) ; \
    return 0 ; \
    } \
\
    int EMBPERL_mgSet##name (SV * pSV, MAGIC * mg) \
\
    { \
    var = SvIV (pSV) ; \
    if ((bDebug & dbgTab) && bReqRunning) \
        lprintf ("[%d]TAB:  set %s = %d, Used = %d\n", nPid, #name, var, used) ; \
    sub () ; \
    return 0 ; \
    } \
    \
    MGVTBL EMBPERL_mvtTab##name = { EMBPERL_mgGet##name, EMBPERL_mgSet##name, NULL, NULL, NULL } ;



#ifdef EPDEBUGALL
#define EPENTRY(func) if (bDebug & dbgFunc) { lprintf ("[%d]DBG:  %s\n", nPid, #func) ; FlushLog () ; }
#define EPENTRY1N(func,arg1) if (bDebug & dbgFunc) { lprintf ("[%d]DBG:  %s %d\n", nPid, #func, arg1) ; FlushLog () ; }
#define EPENTRY1S(func,arg1) if (bDebug & dbgFunc) { lprintf ("[%d]DBG:  %s %s\n", nPid, #func, arg1) ; FlushLog () ; }
#else
#define EPENTRY(func)
#define EPENTRY1N(func,arg1)
#define EPENTRY1S(func,arg1)
#endif
