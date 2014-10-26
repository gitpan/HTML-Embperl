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
#include "epmacro.h"


static char sDebugGlobName [] = "main::_<%s" ;


/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* Setup Debug Informations                                                     */
/*                                                                              */
/* ---------------------------------------------------------------------------- */


int SetupDebugger (/*i/o*/ register req * r)

    {
    HV * pDebugHash ;
    AV * pDebugArray ;
    SV * sDebugNameSV = newSVpvf (sDebugGlobName, r -> Buf.pFile -> sSourcefile) ;
    char * p ;
    char * end ;
    int	 i ;
    STRLEN n ;
    GV * tmpgv ;
    /*
    if ((pDebugHash = perl_get_hv (SvPV (sDebugNameSV, n), TRUE)) == NULL)
        {
        LogError (r, rcHashError) ;
        return 1 ;
        }
    */
    
    
    
    if ((pDebugArray = perl_get_av (SvPV (sDebugNameSV, n), TRUE)) == NULL)
        {
        LogError (r, rcArrayError) ;
        return 1 ;
        }
    

    /*
    pDebugArray = GvAV(gv_AVadd(tmpgv = gv_fetchpv(SvPV (sDebugNameSV, n), TRUE, SVt_PVAV))) ;
    
    GvMULTI_on (tmpgv) ;
    AvREAL_off (pDebugArray) ;
    */

    p = r -> Buf.pBuf ;
    i = 100 ;
    while (*p)
	{
	end = strchr (p, '\n') ;
	if (end)
	    {		
	    SV * pLine = newSVpv (p, end - p + 1) ;
	    /* lprintf (r, "i = %d len = %d, str = %s", i, end - p + 1, SvPV (pLine, n)) ; */
	    av_store (pDebugArray, i++, pLine) ;
	    p = end + 1 ;
	    }
	else if (p < r -> Buf.pEndPos)
	    {
	    av_store (pDebugArray, i++, newSVpv (p, r -> Buf.pEndPos - p + 1)) ;
	    break ;
	    }
	}

    /*
    i = 100 ;
    while (i < AvFILL (pDebugArray))
	{
	SV ** pLine = av_fetch (pDebugArray, i++, 0) ;

	if (pLine && *pLine)
	    lprintf (r, "i = %d len = %d, str = %s", i, end - p + 1, SvPV (*pLine, n)) ;
	}
    */



    return ok ;
    }




