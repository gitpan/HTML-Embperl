###################################################################################
#
#   Embperl - Copyright (c) 1997-2001 Gerald Richter / ECOS
#
#   You may distribute under the terms of either the GNU General Public
#   License or the Artistic License, as specified in the Perl README file.
#   For use with Apache httpd and mod_perl, see also Apache copyright.
#
#   THIS PACKAGE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR
#   IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
#   WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
#
#   $Id: Syntax.xs,v 1.1.2.6 2001/03/23 09:28:10 richter Exp $
#
###################################################################################



MODULE = HTML::Embperl      PACKAGE = HTML::Embperl::Syntax     PREFIX = embperl_


void
embperl_BuildTokenTable (pSyntaxObj)
    SV * pSyntaxObj ;
CODE:
    MAGIC * mg ;
    tTokenTable * pTab ;
    HV *          pHV ;
    SV **         ppSV ;
    int           rc ;
    STRLEN        l ;
    char *        sName ;
	
    if (!SvROK (pSyntaxObj) || SvTYPE(pHV = (HV *)SvRV(pSyntaxObj)) != SVt_PVHV || (mg = mg_find ((SV *)pHV, '~')))
	{        
	/* tTokenTable * pTab = *((tTokenTable **)(mg -> mg_ptr)) ; */
	croak ("Internal Error: pSyntaxObj has already a TokenTable") ;
	}

    pTab = malloc (sizeof (tTokenTable)) ;	

    sv_unmagic ((SV *)pHV, '~') ;
    sv_magic ((SV *)pHV, NULL, '~', (char *)&pTab, sizeof (pTab)) ;

    ppSV = hv_fetch (pHV, "-name", 5, 0) ;
    if (ppSV == NULL || *ppSV == NULL || !SvPOK (*ppSV))
	croak ("Internal Error: pSyntaxObj has no -name") ;

    sName = strdup (SvPV(*ppSV, l)) ;

    ppSV = hv_fetch (pHV, "-root", 5, 0) ;
    if (ppSV == NULL || *ppSV == NULL || !SvROK (*ppSV))
	croak ("Internal Error: pSyntaxObj has no -root") ;
    else	
    	if ((rc = BuildTokenTable (pCurrReq, 0, sName, (HV *)(SvRV(*ppSV)), "", NULL, pTab)) != ok)
            LogError (pCurrReq, rc) ;
	
