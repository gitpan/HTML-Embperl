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


#define INTMG(name,var,used) \
    \
int mgGet##name (SV * pSV, MAGIC * mg) \
\
    { \
    sv_setiv (pSV, var) ; \
    used++ ; \
    if (bDebug & dbgTab) \
        lprintf ("TAB:  get %s = %d, Used = %d\n", #name, var, used) ; \
    return 0 ; \
    } \
\
    int mgSet##name (SV * pSV, MAGIC * mg) \
\
    { \
    var = SvIV (pSV) ; \
    if (bDebug & dbgTab) \
        lprintf ("TAB:  set %s = %d, Used = %d\n", #name, var, used) ; \
    return 0 ; \
    } \
    \
    MGVTBL mvtTab##name = { mgGet##name, mgSet##name, NULL, NULL, NULL } ;

