/*###################################################################################
#
#   Embperl - Copyright (c) 1997-2001 Gerald Richter / ECOS
#
#   You may distribute under the terms of either the GNU General Public
#   License or the Artistic License, as specified in the Perl README file.
#
#   THIS PACKAGE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR
#   IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
#   WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
#
#   $Id: epcmd2.c,v 1.4.2.14 2001/10/29 20:07:12 richter Exp $
#
###################################################################################*/


#include "ep.h"
#include "epmacro.h"

/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* SetRemove Attribute on html tag ...                                          */
/*                                                                              */
/* ---------------------------------------------------------------------------- */



static embperlCmd_SetRemove (/*i/o*/ register req * r,
			     /*in*/ tDomTree *	    pDomTree,
			     /*in*/ tNode	    xNode,
			     /*in*/ tRepeatLevel    nRepeatLevel,
			     /*in*/ const char *    pName,
			     /*in*/ int             nNameLen,
			     /*in*/ const char *    pVal,
			     /*in*/ int             nValLen,
			     /*in*/ const char *    sAttrName, 
			     /*in*/ int             nAttrLen,
                             /*in*/ int             bSetInSource) 

    {
    int	    bEqual = 0 ;
    SV **   ppSV = hv_fetch(pCurrReq -> pFormHash, (char *)pName, nNameLen, 0) ;  
    tNodeData * pNode = Node_selfLevel (pDomTree, xNode, nRepeatLevel) ;

    if (ppSV)
	{
	SV **   ppSVerg = hv_fetch(pCurrReq -> pFormSplitHash, (char *)pName, nNameLen, 0) ;  
	SV *    pSV = SplitFdat (pCurrReq, ppSV, ppSVerg, (char *)pName, nNameLen) ;

	if (SvTYPE (pSV) == SVt_PVHV)
	    { /* -> Hash -> check if key exists */
	    if (hv_exists ((HV *)pSV, (char *)pVal, nValLen))
		{
		bEqual = 1 ;
		hv_store (pCurrReq -> pInputHash, (char *)pName, nNameLen, newSVpv ((nValLen?((char *)pVal):""), nValLen), 0) ;
		}
	    }
	else
	    {
	    STRLEN   dlen ;
	    char * pData = SvPV (pSV, dlen) ;
	    if (dlen == nValLen && strncmp (pVal, pData, dlen) == 0)
		{
		bEqual = 1 ;
		hv_store (pCurrReq -> pInputHash, (char *)pName, nNameLen, newSVsv(pSV), 0) ; 
		}
	    }

	if (bEqual)
	    {
	    Element_selfSetAttribut (pDomTree, pNode, nRepeatLevel, sAttrName, nAttrLen, NULL, 0) ;
	    if (r -> bDebug & dbgInput)
		lprintf (r, "[%d]INPU: Set Attribut: Name: '%*.*s' Value: '%*.*s' Attribute: '%*.*s'\n", r -> nPid, nNameLen, nNameLen, pName, nValLen, nValLen, pVal, nAttrLen, nAttrLen, sAttrName) ; 
            }
	else
	    {
	    Element_selfRemoveAttribut (pDomTree, pNode, nRepeatLevel, sAttrName, nAttrLen) ;
	    if (r -> bDebug & dbgInput)
		lprintf (r, "[%d]INPU: Remove Attribut: Name: '%*.*s' Value: '%*.*s' Attribute: '%*.*s'\n", r -> nPid, nNameLen, nNameLen, pName, nValLen, nValLen, pVal, nAttrLen, nAttrLen, sAttrName ) ; 
	    }
	}
    else
	{
	if (Element_selfGetAttribut (pDomTree, pNode, sAttrName, nAttrLen))
	    {
	    hv_store (pCurrReq -> pInputHash, (char *)pName, nNameLen, newSVpv ((nValLen?((char *)pVal):""), nValLen), 0) ;
	    if (r -> bDebug & dbgInput)
		lprintf (r, "[%d]INPU: Has already Attribut: Name: '%*.*s' Value: '%*.*s' Attribute: '%*.*s'\n", r -> nPid, nNameLen, nNameLen, pName, nValLen, nValLen, pVal, nAttrLen, nAttrLen, sAttrName ) ; 
	    }
	else
	    {
	    if (r -> bDebug & dbgInput)
		lprintf (r, "[%d]INPU: No value in %%fdat for Attribut: Name: '%*.*s' Value: '%*.*s' Attribute: '%*.*s'\n", r -> nPid, nNameLen, nNameLen, pName, nValLen, nValLen, pVal, nAttrLen, nAttrLen, sAttrName ) ; 
            }

	}
    }

/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* input checkbox/radio html tag ...                                            */
/*                                                                              */
/* ---------------------------------------------------------------------------- */



void embperlCmd_InputCheck (/*i/o*/ register req *     r,
			    /*in*/ tDomTree *	    pDomTree,
			    /*in*/ tNode	    xNode,
			    /*in*/ tRepeatLevel     nRepeatLevel,
			    /*in*/ const char *     pName,
			    /*in*/ int              nNameLen,
			    /*in*/ const char *     pVal,
			    /*in*/ int              nValLen, 
                            /*in*/ int              bSetInSource) 

    {
    embperlCmd_SetRemove (r, pDomTree, xNode, nRepeatLevel, pName, nNameLen, pVal, nValLen, "checked", 7, bSetInSource) ;
    }


/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* option html tag ...                                                          */
/*                                                                              */
/* ---------------------------------------------------------------------------- */


void embperlCmd_Option (/*i/o*/ register req *  r,
			/*in*/ tDomTree *	pDomTree,
			/*in*/ tNode	        xNode,
		        /*in*/ tRepeatLevel     nRepeatLevel,
			/*in*/ const char *     pName,
			/*in*/ int              nNameLen,
			/*in*/ const char *     pVal,
			/*in*/ int              nValLen,
                        /*in*/ int              bSetInSource) 
                         

    {
    embperlCmd_SetRemove (r, pDomTree, xNode, nRepeatLevel, pName, nNameLen, pVal, nValLen, "selected", 8, bSetInSource) ;
    }





/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* hidden command ...                                                           */
/*                                                                              */
/* ---------------------------------------------------------------------------- */


int embperlCmd_Hidden	(/*i/o*/ register req *     r,
			 /*in*/ tDomTree *	    pDomTree,
			 /*in*/ tNode		    xNode,
		         /*in*/ tRepeatLevel	    nRepeatLevel,
			 /*in*/ const char *	    sArg)

    {
    char *  pKey ;
    SV *    psv ;
    SV * *  ppsv ;
    HV *    pAddHash = r -> pFormHash ;
    HV *    pSubHash = r -> pInputHash ;
    AV *    pSort    = NULL ;
    HE *    pEntry ;
    I32     l ;
    char *  sArgs ;
    char *  sVarName ;
    char    sVar[512] ;
    int     nMax ;
    STRLEN  nKey ;
    tNodeData * pNode ;
    tNodeData * pNewNode ;


    EPENTRY (CmdHidden) ;

    pNode = Node_selfCondCloneNode (pDomTree, Node_selfLevel (pDomTree, xNode, nRepeatLevel), nRepeatLevel) ;

    pNewNode = pNode ;

    sArgs = _ep_strdup (r, sArg) ;
    if (sArgs && *sArgs != '\0')
        {            
        strncpy (sVar, r -> Buf.sEvalPackage, sizeof (sVar) - 5) ;
        sVar[r -> Buf.nEvalPackage] = ':' ;
        sVar[r -> Buf.nEvalPackage+1] = ':' ;
        sVar[sizeof(sVar) - 1] = '\0' ;
        nMax = sizeof(sVar) - r -> Buf.nEvalPackage - 3 ;
        
        if ((sVarName = strtok (sArgs, ", \t\n")))
            {
            if (*sVarName == '%')
                sVarName++ ;
        
            strncpy (sVar + r -> Buf.nEvalPackage + 2, sVarName, nMax) ;
            
            if ((pAddHash = perl_get_hv ((char *)sVar, FALSE)) == NULL)
                {
                strncpy (r -> errdat1, sVar, sizeof (r -> errdat1) - 1) ;
                _free (r, sArgs) ;
                return rcHashError ;
                }

            if ((sVarName = strtok (NULL, ", \t\n")))
                {
                if (*sVarName == '%')
                    sVarName++ ;
        
                strncpy (sVar + r -> Buf.nEvalPackage + 2, sVarName, nMax) ;
        
                if ((pSubHash = perl_get_hv ((char *)sVar, FALSE)) == NULL)
                    {
                    strncpy (r -> errdat1, sVar, sizeof (r -> errdat1) - 1) ;
                    _free (r, sArgs) ;
                    return rcHashError ;
                    }

                if ((sVarName = strtok (NULL, ", \t\n")))
                    {
                    if (*sVarName == '@')
                        sVarName++ ;
        
                    strncpy (sVar + r -> Buf.nEvalPackage + 2, sVarName, nMax) ;
        
                    if ((pSort = perl_get_av ((char *)sVar, FALSE)) == NULL)
                        {
                        strncpy (r -> errdat1, sVar, sizeof (r -> errdat1) - 1) ;
                        _free (r, sArgs) ;
                        return rcArrayError ;
                        }
                    }
                }
            }
        }
    else
        pSort = r -> pFormArray ;


    /* oputc (r, '\n') ; */
    if (pSort)
        {
        int n = AvFILL (pSort) + 1 ;
        int i ;

        for (i = 0; i < n; i++)
            {
            ppsv = av_fetch (pSort, i, 0) ;
            if (ppsv && (pKey = SvPV(*ppsv, nKey)) && !hv_exists (pSubHash, pKey, nKey))
                {
                STRLEN lppsv ;
		ppsv = hv_fetch (pAddHash, pKey, nKey, 0) ;
                
               if (ppsv && (!(r -> bOptions & optNoHiddenEmptyValue) || *SvPV (*ppsv, lppsv)))
                    {
                    char * s ;
		    STRLEN     l ;
		    tNode xInputNode = Node_appendChild (pDomTree, pNewNode -> xNdx, nRepeatLevel, ntypTag, 0, "input", 5, 0, 0, NULL) ;
                    tNode xAttr      = Node_appendChild (pDomTree, xInputNode, nRepeatLevel, ntypAttr, 0, "type", 4, 0, 0, NULL) ;
                                       Node_appendChild (pDomTree, xAttr, nRepeatLevel, ntypAttrValue, 0, "hidden", 6, 0, 0, NULL) ;
		    
                          xAttr      = Node_appendChild (pDomTree, xInputNode, nRepeatLevel, ntypAttr, 0, "name", 4, 0, 0, NULL) ;
                                       Node_appendChild (pDomTree, xAttr, nRepeatLevel, ntypAttrValue, 0, pKey, nKey, 0, 0, NULL) ;
                          xAttr      = Node_appendChild (pDomTree, xInputNode, nRepeatLevel, ntypAttr, 0, "value", 5, 0, 0, NULL) ;

		    s = SvPV (*ppsv, l) ;			  
			  
			  Node_appendChild (pDomTree, xAttr, nRepeatLevel, ntypAttrValue, 0, s, l, 0, 0, NULL) ;
                    }
                }
            }
        }
    else
        {
        hv_iterinit (pAddHash) ;
        while ((pEntry = hv_iternext (pAddHash)))
            {
            STRLEN nKey ;
	    pKey = hv_iterkey (pEntry, &l) ;
	    nKey = strlen (pKey) ;
            if (!hv_exists (pSubHash, pKey, nKey))
                {
                STRLEN lpsv ;
		psv = hv_iterval (pAddHash, pEntry) ;

                if (!(r -> bOptions & optNoHiddenEmptyValue) || *SvPV (psv, lpsv)) 
                    {
                    char * s ;
		    STRLEN     l ;
		    tNode xInputNode = Node_appendChild (pDomTree, pNewNode -> xNdx, nRepeatLevel, ntypTag, 0, "input", 5, 0, 0, NULL) ;
                    tNode xAttr      = Node_appendChild (pDomTree, xInputNode, nRepeatLevel, ntypAttr, 0, "type", 4, 0, 0, NULL) ;
                                       Node_appendChild (pDomTree, xAttr, nRepeatLevel, ntypAttrValue, 0, "hidden", 6, 0, 0, NULL) ;
		    
                          xAttr      = Node_appendChild (pDomTree, xInputNode, nRepeatLevel, ntypAttr, 0, "name", 4, 0, 0, NULL) ;
                                       Node_appendChild (pDomTree, xAttr, nRepeatLevel, ntypAttrValue, 0, pKey, nKey, 0, 0, NULL) ;
                          xAttr      = Node_appendChild (pDomTree, xInputNode, nRepeatLevel, ntypAttr, 0, "value", 5, 0, 0, NULL) ;

		    s = SvPV (psv, l) ;			  
			  
			  Node_appendChild (pDomTree, xAttr, nRepeatLevel, ntypAttrValue, 0, s, l, 0, 0, NULL) ;
                    }
                }
            }
        }

    if (sArgs)
        _free (r, sArgs) ;

    return ok ;
    }




/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* ouput data inside a url                                                      */
/*                                                                              */
/* ---------------------------------------------------------------------------- */


SV * Node_replaceChildWithUrlDATA (/*in*/ tIndex	xDomTree, 
					  tIndex	xOldChild, 
				   /*in*/ tRepeatLevel  nRepeatLevel,
					  SV *		sText)
    
    {
    STRLEN l ;
    char * s ;
    AV *   pAV ;    
    HV *   pHV ;    
    tDomTree * pDomTree = DomTree_self(xDomTree) ;

    if (SvTYPE(sText) == SVt_RV && SvTYPE((pAV = (AV *)SvRV(sText))) == SVt_PVAV)
	{ /* Array reference inside URL */
	SV ** ppSV ;
	int i ;
	int f = AvFILL(pAV)  ;
        tNode xNode ;

        xOldChild = Node_replaceChildWithCDATA (DomTree_self(xDomTree), xOldChild, nRepeatLevel, "", 0, 4, 0) ;

	for (i = 0; i <= f; i++)
	    {
	    ppSV = av_fetch (pAV, i, 0) ;
	    if (ppSV && *ppSV)
		{
		s = SV2String (*ppSV, l) ;
                xNode = Node_appendChild (pDomTree, xOldChild, nRepeatLevel, ntypText, 0, s, l, 0, 0, NULL) ;
		if (pCurrReq -> nCurrEscMode & 2) 
                    Node_selfLevel (pDomTree, xNode, nRepeatLevel) -> bFlags |= nflgEscUrl ;
                }
	    if ((i & 1) == 0)
                Node_appendChild (pDomTree, xOldChild, nRepeatLevel, ntypCDATA, 0, "=", 1, 0, 0, NULL) ;
	    else if (i < f)
                Node_appendChild (pDomTree, xOldChild, nRepeatLevel, ntypCDATA, 0, "&amp;", 5, 0, 0, NULL) ;
	    }
    
	}

    else if (SvTYPE(sText) == SVt_RV && SvTYPE((pHV = (HV *)SvRV(sText))) == SVt_PVHV)
	{ /* Hash reference inside URL */
	SV ** ppSV ;
        HE *	    pEntry ;
        char *	    pKey ;
        SV * 	    pSVValue ;
        tNode       xNode ;
        int         i = 0 ;
	I32	    l32 ;

        xOldChild = Node_replaceChildWithCDATA (DomTree_self(xDomTree), xOldChild, nRepeatLevel, "", 0, 4, 0) ;

	hv_iterinit (pHV) ;
	while (pEntry = hv_iternext (pHV))
	    {
            if (i++ > 0)
                Node_appendChild (pDomTree, xOldChild, nRepeatLevel, ntypCDATA, 0, "&amp;", 5, 0, 0, NULL) ;
	    pKey     = hv_iterkey (pEntry, &l32) ;
            xNode = Node_appendChild (pDomTree, xOldChild, nRepeatLevel, ntypText, 0, pKey, l32, 0, 0, NULL) ;
	    if (pCurrReq -> nCurrEscMode & 2) 
                Node_self (pDomTree, xNode) -> bFlags |= nflgEscUrl ;

            Node_appendChild (pDomTree, xOldChild, nRepeatLevel, ntypCDATA, 0, "=", 1, 0, 0, NULL) ;

	    pSVValue = hv_iterval (pHV , pEntry) ;
	    if (pSVValue)
		{
		s = SV2String (pSVValue, l) ;
                xNode = Node_appendChild (pDomTree, xOldChild, nRepeatLevel, ntypText, 0, s, l, 0, 0, NULL) ;
		if (pCurrReq -> nCurrEscMode & 2) 
                    Node_selfLevel (pDomTree, xNode, nRepeatLevel) -> bFlags |= nflgEscUrl ;
                }
            }
        }
    else
        {
        char * s = SV2String (sText, l) ;
        Node_replaceChildWithCDATA (DomTree_self(xDomTree), xOldChild, nRepeatLevel, s, l, (pCurrReq -> nCurrEscMode & 3) == 3?2 + (pCurrReq -> nCurrEscMode & 4):pCurrReq -> nCurrEscMode, 0) ;
        }

    pCurrReq -> nCurrEscMode = pCurrReq -> nEscMode ;
    pCurrReq -> bEscModeSet = -1 ;
    /* SvREFCNT_inc (sText) ; */
    /* ST(0) = sText ;*/
    /* XSRETURN(1) ; */
    return sText ;
    }

