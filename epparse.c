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
#   $Id: epparse.c,v 1.4.2.44 2001/11/14 15:01:41 richter Exp $
#
###################################################################################*/


#include "ep.h"
#include "epmacro.h"



struct tTokenCmp
    {
    char *          pStart ;
    char *          pCurr ;
    int		nLen ;
    } ;



struct tTokenTable DefaultTokenTable ;


#define parse_malloc(a,b) malloc(b)

/* ------------------------------------------------------------------------ */
/* compare tokens                                                           */
/* ------------------------------------------------------------------------ */

static int CmpToken (/*in*/ const void *  p1,
                     /*in*/ const void *  p2)

    {
    return strcmp (*((const char * *)p1), *((const char * *)p2)) ;
    }
    
/* ------------------------------------------------------------------------ */
/* compare tokens for descending order                                      */
/* ------------------------------------------------------------------------ */

static int CmpTokenDesc (/*in*/ const void *  p1,
                     /*in*/ const void *  p2)

    {
    return strcmp (*((const char * *)p2), *((const char * *)p1)) ;
    }


	    
/* ------------------------------------------------------------------------ */
/*                                                                          */
/* CheckProcInfo                                                            */
/*                                                                          */
/* Check for processor informations                                         */
/*                                                                          */
/* ------------------------------------------------------------------------ */

static int CheckProcInfo      (/*i/o*/ register req * r,
			/*in*/  HV *           pHash,
			/*in*/	struct tToken * pToken,
			/*i/o*/ void * *       ppCompilerInfo)
			

    {	
    int		    rc ;
    HE *	    pEntry ;
    char *	    pKey ;
    SV * *	    ppSV ;
    SV * 	    pSVValue ;
    IV		    l	 ;
    HV *            pHVProcInfo ;
    int             n ;
    
    ppSV = hv_fetch(pHash, "procinfo", sizeof ("procinfo") - 1, 0) ;  
    if (ppSV != NULL)
	{		
	if (*ppSV == NULL || !SvROK (*ppSV) || SvTYPE (SvRV (*ppSV)) != SVt_PVHV)
	    {
	    strncpy (r -> errdat1, "BuildTokenHash", sizeof (r -> errdat1)) ;
	    sprintf (r -> errdat2, "%s => procinfo", pToken -> sText) ;
	    return rcNotHashRef ;
	    }

	pHVProcInfo = (HV *)SvRV (*ppSV) ;

	hv_iterinit (pHVProcInfo ) ;
	while (pEntry = hv_iternext (pHVProcInfo))
	    {
	    HV *   pProcInfoHash ;
        
	    pKey     = hv_iterkey (pEntry, &l) ;
	    pSVValue = hv_iterval (pHVProcInfo , pEntry) ;
        
	    if (pSVValue == NULL || !SvROK (pSVValue) || SvTYPE (SvRV (pSVValue)) != SVt_PVHV)
		{
		strncpy (r -> errdat1, "BuildTokenHash", sizeof (r -> errdat1)) ;
		sprintf (r -> errdat2, "%s => procinfo", pToken -> sText) ;
		return rcNotHashRef ;
		}
	    if (strcmp (pKey, "embperl") == 0)
		embperl_CompileInitItem (r, (HV *)(SvRV (pSVValue)), pToken -> nNodeName, pToken -> nNodeType, 1, ppCompilerInfo) ;
	    else if (strncmp (pKey, "embperl#", 8) == 0 && (n = atoi (pKey+8)) > 0)
		embperl_CompileInitItem (r, (HV *)(SvRV (pSVValue)), pToken -> nNodeName, pToken -> nNodeType, n, ppCompilerInfo) ;
	    }
	}	    


    return ok ;
    }


	    
	    


    
/* ------------------------------------------------------------------------ */
/*                                                                          */
/* BuildSubTokenTable                                                       */
/*                                                                          */
/* Build the C token table out of a Perl Hash                               */
/*                                                                          */
/* ------------------------------------------------------------------------ */

static int BuildSubTokenTable (/*i/o*/ register req * r,
				/*in*/ int            nLevel,
			       /*in*/  HV *           pHash,
			/*in*/  const char *   pKey,
			/*in*/  const char *   pAttr,
			/*in*/  const char *   pDefEnd,
			/*i/o*/ void * *       ppCompilerInfo,
			/*out*/ struct tTokenTable * * pTokenTable)

                     
    {
    SV * *  ppSV ;
    int	    rc ;
    
    nLevel++ ;

    ppSV = hv_fetch(pHash, (char *)pAttr, strlen (pAttr), 0) ;  
    if (ppSV != NULL)
	{		
	struct tTokenTable * pNewTokenTable ;
	HV *                 pSubHash ;

	if (*ppSV == NULL || !SvROK (*ppSV) || SvTYPE (SvRV (*ppSV)) != SVt_PVHV)
	    {
	    strncpy (r -> errdat1, "BuildTokenHash", sizeof (r -> errdat1)) ;
	    sprintf (r -> errdat2, "%s => %s", pKey, pAttr) ;
	    return rcNotHashRef ;
	    }
	
	pSubHash = (HV *)SvRV (*ppSV) ;
	if ((pNewTokenTable = (struct tTokenTable *)GetHashValueInt (pSubHash, "--cptr", 0)) == NULL)
	    {
	    if ((pNewTokenTable = parse_malloc (r, sizeof (struct tTokenTable))) == NULL)
		 return rcOutOfMemory ;

	    if (r -> bDebug & dbgBuildToken)
		lprintf (r, "[%d]TOKEN: %*c-> %s\n", r -> nPid, nLevel*2, ' ', pAttr) ; 
	    if ((rc = BuildTokenTable (r, nLevel, NULL, pSubHash, pDefEnd, ppCompilerInfo, pNewTokenTable)))
		return rc ;    
	    if (r -> bDebug & dbgBuildToken)
		lprintf (r, "[%d]TOKEN: %*c<- %s\n", r -> nPid, nLevel*2, ' ', pAttr) ; 
	    
	    if (pNewTokenTable -> numTokens == 0)
		{
		strncpy (r -> errdat1, "BuildTokenHash", sizeof (r -> errdat1)) ;
		sprintf (r -> errdat2, "%s => %s does not contain any tokens", pKey, pAttr) ;
		return rcNotFound ;
		}

	    hv_store(pSubHash, "--cptr", sizeof ("--cptr") - 1, newSViv ((IV)pNewTokenTable), 0) ;
	    }
	else
	    if (r -> bDebug & dbgBuildToken)
	        lprintf (r, "[%d]TOKEN: %*c-> %s already build; numTokens=%d\n", r -> nPid, nLevel*2, ' ', pAttr, pNewTokenTable->numTokens) ; 
	

	*pTokenTable = pNewTokenTable ;
	return  ok  ;
	}

    *pTokenTable = NULL ;
    return ok ;
    }
    

/* ------------------------------------------------------------------------ */
/*                                                                          */
/* BuildTokenTable                                                          */
/*                                                                          */
/* Build the C token table out of a Perl Hash                               */
/*                                                                          */
/* ------------------------------------------------------------------------ */

int BuildTokenTable (/*i/o*/ register req *	  r,
 		     /*in*/ int            nLevel,
                     /*in*/  const char *         sName,
		     /*in*/  HV *		  pTokenHash,
		     /*in*/  const char *         pDefEnd,
		     /*i/o*/ void * *		  ppCompilerInfo,
                     /*out*/ struct tTokenTable * pTokenTable)

                     
    {
    int		    rc ;
    SV *	    pToken ;
    HE *	    pEntry ;
    char *	    pKey ;
    const char *    c ;
    int		    numTokens ;
    struct tToken * pTable ;
    struct tToken * p ;
    IV		    l	 ;
    int		    n ;
    int		    i ;
    unsigned char * pStartChars = pTokenTable -> cStartChars ;
    unsigned char * pAllChars	= pTokenTable -> cAllChars ;

    tainted = 0 ;

    /* r -> bDebug |= dbgBuildToken ; */

    memset (pStartChars, 0, sizeof (pTokenTable -> cStartChars)) ;
    memset (pAllChars,   0, sizeof (pTokenTable -> cAllChars)) ;
    pTokenTable -> bLSearch = 0 ;    
    pTokenTable -> nDefNodeType = ntypCDATA ;
    pTokenTable -> pContainsToken = NULL ;
    pTokenTable -> pCompilerInfo = NULL ;
    pTokenTable -> sName = sName ;
    if (ppCompilerInfo == NULL)
	ppCompilerInfo = &pTokenTable -> pCompilerInfo ;

    hv_store(pTokenHash, "--cptr", sizeof ("--cptr") - 1, newSViv ((IV)pTokenTable), 0) ;

    numTokens = 1 ;
    hv_iterinit (pTokenHash) ;
    while ((pEntry = hv_iternext (pTokenHash)))
	{
	pKey     = hv_iterkey (pEntry, &l) ;
	pToken   = hv_iterval (pTokenHash, pEntry) ;
        if (*pKey != '-') 
	    numTokens++ ;
        }
            
    if ((pTable = parse_malloc (r, sizeof (struct tToken) * numTokens)) == NULL)
         return rcOutOfMemory ;

    n = 0 ;
    hv_iterinit (pTokenHash) ;
    while ((pEntry = hv_iternext (pTokenHash)))
        {
        HV *   pHash ;
	struct tTokenTable * pNewTokenTable ;
	char *  sContains ;
	char *  sC ;
        
        pKey     = hv_iterkey (pEntry, &l) ;
        pToken   = hv_iterval (pTokenHash, pEntry) ;
        
	if (*pKey == '-')
	    { /* special key */
	    if (strcmp (pKey, "-defnodetype") == 0)
		{
		pTokenTable -> nDefNodeType = SvIV ((SV *)pToken) ;
		}
	    else if (strcmp (pKey, "-lsearch") == 0)
		{
		pTokenTable -> bLSearch = SvIV ((SV *)pToken) ;
		}
	    else if (strcmp (pKey, "-contains") == 0)
		{
		STRLEN l ;
		char * c = SvPV (pToken, l) ;
		while (*c)
		    {
		    pAllChars [tolower(*c) >> 3] |= 1 << (tolower(*c) & 7) ;
		    pAllChars [toupper(*c) >> 3] |= 1 << (toupper(*c) & 7) ;
		    c++ ;
		    }
		}
	    }
        }

    n = 0 ;
    hv_iterinit (pTokenHash) ;
    while ((pEntry = hv_iternext (pTokenHash)))
        {
        HV *   pHash ;
	struct tTokenTable * pNewTokenTable ;
	char *  sContains ;
	char *  sC ;
        
        pKey     = hv_iterkey (pEntry, &l) ;
        pToken   = hv_iterval (pTokenHash, pEntry) ;
	if (*pKey != '-')
	    {
	    if (!SvROK (pToken) || SvTYPE (SvRV (pToken)) != SVt_PVHV)
		{
		strncpy (r -> errdat1, "BuildTokenHash", sizeof (r -> errdat1)) ;
		sprintf (r -> errdat2, "%s", pKey) ;
		return rcNotHashRef ;
		}
	    pHash = (HV *)SvRV (pToken) ;
        
	    p = &pTable[n] ;
	    p -> sName     = pKey ;
	    p -> sText     = GetHashValueStrDup (pHash, "text", "") ;
	    p -> nTextLen  = strlen (p -> sText) ;
	    p -> sEndText  = GetHashValueStrDup (pHash, "end", (char *)pDefEnd) ;
	    p -> sNodeName = GetHashValueStrDup (pHash, "nodename", NULL) ;
	    p -> nNodeType = GetHashValueInt (pHash, "nodetype", ntypTag) ;
	    p -> bUnescape = GetHashValueInt (pHash, "unescape", 0) ;
	    p -> bAddFlags = GetHashValueInt (pHash, "addflags", 0) ;
	    p -> nCDataType = GetHashValueInt (pHash, "cdatatype", pTokenTable -> nDefNodeType) ;
	    p -> nForceType = GetHashValueInt (pHash, "forcetype", 0) ;
	    p -> bRemoveSpaces = GetHashValueInt (pHash, "removespaces", p -> nNodeType != ntypCDATA?2:0) ;
	    p -> bInsideMustExist = GetHashValueInt (pHash, "insidemustexist", 0) ;
	    p -> pStartTag  = (struct tToken *)GetHashValueStrDup (pHash, "starttag", NULL) ;
	    p -> pEndTag    = (struct tToken *)GetHashValueStrDup (pHash, "endtag", NULL) ;
	    p -> sParseTimePerlCode =  GetHashValueStrDup (pHash, "parsetimeperlcode", NULL) ;
	    if (sC = sContains  = GetHashValueStrDup (pHash, "contains", NULL))
		{
		unsigned char * pC ;
		if ((p -> pContains = parse_malloc (r, sizeof (tCharMap))) == NULL)
		    return rcOutOfMemory ;

                pC = p -> pContains ;
		memset (pC, 0, sizeof (tCharMap)) ;
		while (*sContains)
		    {
		    pC[*sContains >> 3] |= 1 << (*sContains & 7) ;
	            pStartChars [*sContains >> 3] |= 1 << (*sContains & 7) ;
	            pStartChars [*sContains >> 3] |= 1 << (*sContains & 7) ;
		    sContains++ ;
		    }
		}
	    else
		p -> pContains = NULL ;

	    c = p -> sText ;
	    pStartChars [toupper(*c) >> 3] |= 1 << (toupper(*c) & 7) ;
	    pStartChars [tolower(*c) >> 3] |= 1 << (tolower(*c) & 7) ;
        
	    while (*c)
		{
		pAllChars [tolower(*c) >> 3] |= 1 << (tolower(*c) & 7) ;
		pAllChars [toupper(*c) >> 3] |= 1 << (toupper(*c) & 7) ;
		c++ ;
		}
	    

	    if (r -> bDebug & dbgBuildToken)
                lprintf (r, "[%d]TOKEN: %*c%s ... %s  unesc=%d nodetype=%d, cdatatype=%d, nodename=%s contains='%s'\n", r -> nPid, nLevel*2, ' ', p -> sText, p -> sEndText, p -> bUnescape, p -> nNodeType, p -> nCDataType, p -> sNodeName?p -> sNodeName:"<null>", sC?sC:"") ; 
        
	    if (p -> sNodeName)
		{
		if (p -> sNodeName[0] != '!')
		    p -> nNodeName = String2Ndx (p -> sNodeName, strlen (p -> sNodeName)) ;
		else
		    p -> nNodeName = String2UniqueNdx (p -> sNodeName + 1, strlen (p -> sNodeName + 1)) ;
		}
	    else
		p -> nNodeName = String2Ndx (p -> sText, strlen (p -> sText)) ;


	    if ((rc = CheckProcInfo (r, pHash, p, ppCompilerInfo)) != ok)
		return rc ;

	    
	    if ((rc = BuildSubTokenTable (r, nLevel, pHash, pKey, "follow", p -> sEndText, ppCompilerInfo, &pNewTokenTable)))
		return rc ;
	    p -> pFollowedBy = pNewTokenTable ;

	    if ((rc = BuildSubTokenTable (r, nLevel, pHash, pKey, "inside", "", ppCompilerInfo, &pNewTokenTable)))
		return rc ;
	    p -> pInside     = pNewTokenTable ;

	    n++ ;
	    }
	}

    qsort (pTable, numTokens - 1, sizeof (struct tToken), pTokenTable -> bLSearch?CmpTokenDesc:CmpToken) ;


    for (i = 0; i < n; i++)
	{
	if (pTable[i].pContains && !pTable[i].sText[0])
	    pTokenTable -> pContainsToken = &pTable[i] ;
        if (pTable[i].pEndTag)
	    {
	    char * s = (char *)pTable[i].pEndTag ;
	    int    j ;

	    pTable[i].pEndTag = NULL ;
	    for (j = 0; j < n; j++)
		{
		if (strcmp (pTable[j].sName, s) == 0)
		    pTable[i].pEndTag = &pTable[j] ;
		}
	    if (pTable[i].pEndTag == NULL)
		{
		strncpy (r -> errdat1, "BuildTokenHash", sizeof (r -> errdat1)) ;
		sprintf (r -> errdat2, " EndTag %s for %s not found", pTable[i].sText, s) ;
		return rcNotFound ;
		}
	    
	    }
        if (pTable[i].pStartTag)
	    {
	    char * s = (char *)pTable[i].pStartTag ;
	    int    j ;

	    pTable[i].pStartTag = NULL ;
	    for (j = 0; j < n; j++)
		{
		if (strcmp (pTable[j].sName, s) == 0)
		    pTable[i].pStartTag = &pTable[j] ;
		}
	    if (pTable[i].pStartTag == NULL)
		{
		strncpy (r -> errdat1, "BuildTokenHash", sizeof (r -> errdat1)) ;
		sprintf (r -> errdat2, " StartTag %s for %s not found", pTable[i].sText, s) ;
		return rcNotFound ;
		}
	    
	    }
        }

    
    p = &pTable[n] ;
    p -> sText = "" ;
    p -> nTextLen = 0 ;
    p -> sEndText = "" ;
    p -> pFollowedBy = NULL ;
    p -> pInside     = NULL ;
    
        
    pTokenTable -> pTokens   = pTable ;
    pTokenTable -> numTokens = numTokens - 1 ;
            
    return ok ;
    }
    
/* ------------------------------------------------------------------------ */
/*                                                                          */
/* ExecParseTimeCode                                                        */
/*                                                                          */
/* executes Perl code at parse time                                         */
/*                                                                          */
/* ------------------------------------------------------------------------ */

static int ExecParseTimeCode (/*i/o*/	register req *	    r,
			      /*in */	struct tToken *	    pToken, 
		  	      /*in */	char * 		    pCurr, 
					int		    nLen,
					int                 nLinenumber)

    {
    SV * pSV ;
    int  rc ;
    const char * sPCode = pToken -> sParseTimePerlCode ;
    int          plen = strlen (sPCode) ;
    char *       sCode ;
    const char *  p ;
    int          n ;
    SV *        args[2] ;


    if (p = strnstr (sPCode, "%%", nLen))
	{
	sCode = parse_malloc (r, nLen + plen + 1) ;
	n = p - sPCode ;
	memcpy (sCode, sPCode, n) ;
	memcpy (sCode + n, pCurr, nLen) ;
	memcpy (sCode + n + nLen, sPCode + n + 2, plen - n - 2) ;
	nLen = nLen + plen - 2 ;
	sCode[nLen] = '\0' ;
	}
    else
	{
	sCode = (char *)sPCode ;
	nLen = plen ;
	}
    
    if (nLen && pCurrReq -> bDebug & dbgParse)
	lprintf (r, "[%d]PARSE: ParseTimeCode:    %*.*s\n", r -> nPid, nLen, nLen, sCode) ; 

    pSV = newSVpvf("package %s ;\nmy ($_ep_req) = @_;\n#line %d \"%s\"\n%*.*s",
	    pCurrReq -> Buf.sEvalPackage, nLinenumber, r -> Buf.pFile -> sSourcefile, nLen, nLen, sCode) ;
    newSVpvf2(pSV) ;
    args[0] = r -> pReqSV ;
    if ((rc = EvalDirect (r, pSV, 1, args)) != ok)
	LogError (r, rc) ;
    SvREFCNT_dec(pSV);

    return rc ;
    }





    
/* ------------------------------------------------------------------------ */
/* compare tokens                                                           */
/* ------------------------------------------------------------------------ */

static int CmpTokenN (/*in*/ const void *  p1,
                     /*in*/ const void *  p2)

    {
    struct tTokenCmp * c = (struct tTokenCmp *)p1 ;
    int                i ;
    int	p1Len = c -> nLen ;
    int p2Len = ((struct tToken *)p2) -> nTextLen ;

    if ((i = strnicmp (c -> pStart, *((const char * *)p2), p1Len)) == 0)
	{
	if (p1Len == p2Len)
	    return 0 ;
	else if (p1Len > p2Len)
	    return 1 ;
	return -1 ;
	}
    return i ;
    }


/* ------------------------------------------------------------------------ */
/*                                                                          */
/* ParseTokens                                                              */
/*                                                                          */
/* Parse a text for tokens                                                  */
/*                                                                          */
/* ------------------------------------------------------------------------ */

static int ParseTokens (/*i/o*/ register req *		r,
			/*in */ char * *		ppCurr, 
				char *			pEnd, 
				struct tTokenTable *	pTokenTable, 
				const char *		sEndText,
				const unsigned char *	pParentContains,
				tNodeType		nCDataType,
				tNodeType		nForceType,
				int			bUnescape,
				int			bInsideMustExist, 
				int			bRemoveSpaces, 
				tStringIndex 		nParentNodeName,
				tNode			xParentNode,
				int			level, 
				char *		        pCDATAStart, 
				const char *		sStopText) 

    {
    unsigned char * pStartChars = pTokenTable -> cStartChars ;
    struct tTokenCmp c ;    
    int nEndText = sEndText?strlen (sEndText):0 ;    
    char * pCurr = *ppCurr  ;
    char * pCurrStart = pCDATAStart?pCDATAStart:pCurr ;
    tNode xNewNode ;
    int	    rc ;
    tDomTree * pDomTree = DomTree_self (r -> xCurrDomTree) ;
    
    if (nEndText == 0 && sStopText)
	{
	sEndText = sStopText ;
	nEndText = sEndText?strlen (sEndText):0 ;    
	}
    else
	sStopText = NULL ;


    while (pCurr < pEnd)
        {
	struct tToken *	    pToken	    = NULL ;
        int                 bFollow         = 0 ;

	if (level == 0 && pTokenTable != r -> pTokenTable)
	    { /* syntax has changed */
	    pTokenTable = r -> pTokenTable ;
	    pStartChars = pTokenTable -> cStartChars ;
	    }
	
        if (pStartChars [*pCurr >> 3] & 1 << (*pCurr & 7))
            { /* valid token start char found */
	    struct tTokenTable *    pNextTokenTab   = pTokenTable ;
	    tStringIndex 	    nNodeName	    = 0 ;
	    tNodeType		    nNodeType	    = 0 ;
	    char *	            pCurrTokenStart = pCurr ;

	    
	    do
                {
		struct tToken * pTokenTab = pNextTokenTab -> pTokens ;
		int             numTokens = pNextTokenTab -> numTokens ;
		unsigned char * pAllChars = pNextTokenTab -> cAllChars ;

	        bFollow++ ;

                if (pNextTokenTab -> bLSearch)
		    { /* search linear thru the tokens */
		    int r = 1 ;
		    int i ;

		    for (i = 0, pToken = pTokenTab; i < numTokens; i++, pToken++)
			{
			if (pToken -> nTextLen == 0)
			    continue ;
			r = strnicmp (pCurr, pToken -> sText, pToken -> nTextLen)  ;
			/* if ((r == 0 && !(pAllChars [pCurr[pToken -> nTextLen] >> 3] & (1 << (pCurr[pToken -> nTextLen] & 7)))) || */
			if (r == 0  ||
			        *pCurr > *(pToken -> sText))
			    break ;
			}
		    if (r != 0)
		    	pToken = NULL ;
		    else
			pCurr += pToken -> nTextLen ;
		    }
		else
		    { /* do a binary search for tokens */
		    c.pStart = pCurr ;

		    while (pAllChars [*pCurr >> 3] & (1 << (*pCurr & 7)))
			pCurr++ ;
                
		    c.nLen = pCurr - c.pStart ;
		    pToken = (struct tToken *)bsearch (&c, pTokenTab, numTokens, sizeof (struct tToken), CmpTokenN) ;
		    if (!pToken)
		    	{
		    	pCurr = c.pStart ;
		    /*	bFollow = 0 ; */
		    	}
		    }

		if (pToken)
                    {
		    if (pToken -> bRemoveSpaces & 2)
			while (isspace (*pCurr))
			    pCurr++ ;
		    else if (pToken -> bRemoveSpaces & 8)
			while ((*pCurr == ' ' || *pCurr == '\t'  || *pCurr == '\r'))
			    pCurr++ ;
		    
		    if (pToken -> sNodeName)
			nNodeName = pToken -> nNodeName ;
		    }
                else
		    {
		    pToken = pNextTokenTab -> pContainsToken ;
		    /*
		    if (pToken = pNextTokenTab -> pContainsToken)
		    	{
		    	unsigned char * pContains ;
		    	if (!(pToken -> pInside) && (!(pContains = pToken -> pContains) || !(pContains [*pCurr >> 3] & (1 << (*pCurr & 7)))))
	    	    	    pToken = NULL ;
		    	}
		    */		    
    		    if (pToken && pToken -> sNodeName)
			nNodeName = pToken -> nNodeName ;
		    
		    break ;
		    }

	
		}
            while (pNextTokenTab = pToken -> pFollowedBy) ;
            
            if (pToken)
                { /* matching token found */       
                struct tTokenTable * pInside ;                

		if (pCurrStart < pCurrTokenStart)
		    {
		    if (nCDataType)
			{ /* add text before current token as node */
			const char * pEnd = pCurrTokenStart - 1;
			if (pToken -> bRemoveSpaces & 1)
			    while (pEnd >= pCurrStart && isspace (*pEnd))
				pEnd-- ;
			else if (pToken -> bRemoveSpaces & 4)
			    while (pEnd >= pCurrStart && (*pEnd == ' ' || *pEnd == '\t'  || *pEnd == '\r'))
				pEnd-- ;
			else if (pToken -> bRemoveSpaces & 16)
			    {
			    while (pEnd >= pCurrStart && isspace (*pEnd))
				pEnd-- ;
			    if (pEnd >= pCurrStart && pEnd < pCurrTokenStart - 1)
				pEnd++ ;
			    }

			if (bUnescape)
                            {
                            r -> bEscInUrl = bUnescape - 1 ;
                            TransHtml (r, pCurrStart, pEnd - pCurrStart + 1) ;
                            r -> bEscInUrl = 0 ;
                            }

			
			if (pEnd - pCurrStart + 1)
			    if (!(xNewNode = Node_appendChild (pDomTree, xParentNode, 0, nCDataType, 0, pCurrStart, pEnd - pCurrStart + 1, level, GetLineNoOf (r, pCurrStart), NULL)))
				return 1 ;
			}
		    pCurrStart = pCurrTokenStart ;
		    }
            
		if (nNodeName == 0)
		    nNodeName = pToken -> nNodeName ;
		
		if (pToken -> nNodeType == ntypEndTag && level > 0)
		    { /* end token found */
		    tNodeData * pStartTag ;
		    char * pEndCurr = strstr (pCurr, pToken -> sEndText) ;
                    if (pEndCurr)
			{ 
			tNode xNewAttrNode ;
                        if (pEndCurr - pCurr && pToken -> nCDataType && pToken -> nCDataType != ntypCDATA)
			    { /* add text before end of token as node */
                            char * pEnd = pEndCurr ;

                            if (pToken -> bRemoveSpaces & 32)
			        while (pEnd >= pCurrStart && isspace (*pEnd))
				    pEnd-- ;
			    else if (pToken -> bRemoveSpaces & 64)
			        while (pEnd >= pCurrStart && (*pEnd == ' ' || *pEnd == '\t'  || *pEnd == '\r'))
				    pEnd-- ;
				
			    if (pToken -> bUnescape)
                                {
                                r -> bEscInUrl = pToken -> bUnescape - 1 ;
				TransHtml (r, pCurr, pEnd - pCurr) ;
                                r -> bEscInUrl = 0 ;
                                }

			    if (!(xNewAttrNode = Node_appendChild (pDomTree, xParentNode, 0, pToken -> nCDataType, 0, pCurr, pEnd - pCurr, level+1, GetLineNoOf (r, pCurr), NULL)))
				return 1 ;
			    if (pToken -> bAddFlags)
                                Node_self (pDomTree, xNewAttrNode) -> bFlags |= pToken -> bAddFlags ;
                            
                            }
			pCurr = pEndCurr + strlen (pToken -> sEndText) ;
			}
		    level-- ;
		    xParentNode = Node_parentNode  (pDomTree, xParentNode, 0) ;
		    pStartTag = Node_selfLastChild (pDomTree, Node_self (pDomTree, xParentNode), 0) ;
		    if (pToken -> pStartTag == NULL || pStartTag -> nText != pToken -> pStartTag -> nNodeName)
			{
			strncpy (r -> errdat2, Ndx2String (pStartTag -> nText), sizeof (r -> errdat2)) ;	
			strncpy (r -> errdat1, Ndx2String (pToken -> nNodeName), sizeof (r -> errdat1)) ;	
			r -> Buf.pCurrPos = pCurrTokenStart ;
			return rcTagMismatch ;
			}
		    }
		else
		    {
		    if (pToken -> nNodeType == ntypEndStartTag && level > 0)
			{
			xParentNode = Node_parentNode  (pDomTree, xParentNode, 0) ;
			level-- ;
			}
		    if (pToken -> nNodeType != ntypCDATA || pToken -> sNodeName)
			{
			/* add token as node if not cdata*/
			tNodeType nType = pToken -> nNodeType ;
			if (nType == ntypStartEndTag)
			    nType = ntypStartTag ;

			if (!(xNewNode = Node_appendChild (pDomTree, xParentNode, 0, nType, (nCDataType == ntypAttrValue && pToken -> nNodeType != ntypAttr)?(pToken -> nForceType?2:1):0, NULL, nNodeName, level, GetLineNoOf (r, pCurrTokenStart), pToken -> sText)))
			    {
			    r -> Buf.pCurrPos = pCurrTokenStart ;

			    return rc ;
			    }
			if (pToken -> bAddFlags)
                            Node_self (pDomTree, xNewNode) -> bFlags |= pToken -> bAddFlags ;
			if (!pToken -> pInside)
			    bInsideMustExist = 0 ;
			}
		    else
			{
			xNewNode = xParentNode ;
			}

		    if (pInside = pToken -> pInside)
			{ /* parse for further tokens inside of this token */
                        rc = ParseTokens (r, &pCurr, pEnd, pInside, 
					    pToken -> sEndText,
					    pToken -> pContains,
					    pToken -> nCDataType == ntypCDATA && !pToken -> sNodeName?ntypAttrValue:pToken -> nCDataType,
					    0,
					    pToken -> bUnescape, 
					    pToken -> bInsideMustExist + bInsideMustExist, 
					    pToken -> bRemoveSpaces, 
					    nNodeName,
					    xNewNode,
					    level+1,
					    pToken -> nNodeType == ntypCDATA?pCurrTokenStart:NULL,
					    sEndText && *sEndText?sEndText:NULL) ;
			if (rc == ok)
			    bInsideMustExist = 0 ;
			else if (pToken -> bInsideMustExist && rc == rcNotFound)
			    {
			    rc = ok ;
			    /*
			    pToken = NULL ;
			    bFollow = 0 ;
			    sEndText = NULL ;
			    nEndText = 0 ;
	    		    pCurr  = pCurrTokenStart  ;
			    */
			    if (xNewNode != xParentNode)
				{
				Node_removeChild (pDomTree, xParentNode, xNewNode, 0) ; 
				if (r -> bDebug & dbgParse)
				    lprintf (pCurrReq, "[%d]PARSE: DelNode: +%02d %*s parent=%d node=%d\n", 
	                             pCurrReq -> nPid, level, level * 2, "", xParentNode, xNewNode) ; 
				}
				
			    /* add as cdata*/
			    if (!(xNewNode = Node_appendChild (pDomTree, xParentNode, 0, pTokenTable -> nDefNodeType, 0, pCurrStart, pCurr - pCurrStart, level, GetLineNoOf (r, pCurrStart), NULL)))
				return 1 ;
			    }
			else if (rc != rcNotFound)
			    return rc ;
			 if (pToken -> nNodeType == ntypStartEndTag)
			    {
			    xParentNode = Node_parentNode  (pDomTree, xNewNode, 0) ;
			    pToken = NULL ;
			    bFollow = 2 ;
			    }
			}    
		    else
			{ /* nothing more inside of this token allowed, so search for the end of the token */
			char * pEndCurr ;
			unsigned char * pContains ;
			int nSkip ;
			if ((pContains = pToken -> pContains))
			    {
			    pEndCurr = pCurr ;
			    while (pContains [*pEndCurr >> 3] & (1 << (*pEndCurr & 7)))
                                pEndCurr++ ;
			    nSkip = 0 ;
			    /*
			    if (pEndCurr == pCurr)
				{
				pEndCurr = NULL ;
				pToken   = NULL ;
				}
			    */	
			    }
			else
			    {
			    pEndCurr = strstr (pCurr, pToken -> sEndText) ;
			    nSkip = strlen (pToken -> sEndText) ;
			    if (pToken -> nNodeType == ntypCDATA && pEndCurr && !pToken -> sNodeName)
				{
				pEndCurr += nSkip ;
				nSkip = 0 ;
				pCurr = pCurrTokenStart ;
				}
			    }

			if (pEndCurr)
			    {
			    tNode xNewAttrNode ;
                            if (pEndCurr - pCurr && pToken -> nCDataType)
				{
				int nLine ;
                                char * pEnd = pEndCurr ;

                                if (pToken -> bRemoveSpaces & 32)
			            while (pEnd >= pCurrStart && isspace (*pEnd))
				        pEnd-- ;
			        else if (pToken -> bRemoveSpaces & 64)
			            while (pEnd >= pCurrStart && (*pEnd == ' ' || *pEnd == '\t'  || *pEnd == '\r'))
				        pEnd-- ;
				
                                if (pToken -> bUnescape)
                                    {
                                    r -> bEscInUrl = pToken -> bUnescape - 1 ;
				    TransHtml (r, pCurr, pEnd - pCurr) ;
                                    r -> bEscInUrl = 0 ;
                                    }


				if (!(xNewAttrNode = Node_appendChild (pDomTree, xNewNode, 0, pToken -> nCDataType, 
									0, pCurr, pEnd - pCurr, level+1, 
									nLine = GetLineNoOf (r, pCurr), pToken -> sText)))
				    return 1 ;
			        if (pToken -> bAddFlags)
                                    Node_self (pDomTree, xNewAttrNode) -> bFlags |= pToken -> bAddFlags ;
				if (pToken -> sParseTimePerlCode)
				    if ((rc = ExecParseTimeCode (r, pToken, pCurr, pEnd - pCurr, nLine)) != ok)
					return rc ;
				}

			     if (pToken -> nNodeType == ntypStartEndTag)
				{
				xParentNode = Node_parentNode  (pDomTree, xNewNode, 0) ;
				pToken = NULL ;
				}

			    
			    pCurr = pEndCurr + nSkip ;
			    }
			}

		    if (pToken && (pToken -> nNodeType == ntypStartTag || 
			           pToken -> nNodeType == ntypEndStartTag ||
				   pToken -> nNodeType == ntypStartEndTag))
			{
			level++ ;
			xParentNode = xNewNode ;
		        nCDataType = pTokenTable -> nDefNodeType ;
			}
		    }
		pCurrStart = pCurr ;
                }
	    }
        if (pParentContains && ((pParentContains [*pCurr >> 3] & 1 << (*pCurr & 7)) == 0) )
            {
	    if (pCurr - pCurrStart && nCDataType)
		{
		if (!(xNewNode = Node_appendChild (pDomTree, xParentNode, 0, nCDataType, 0, 
		                                   pCurrStart, pCurr - pCurrStart, level, 
						   GetLineNoOf (r, pCurrStart), NULL)))
		    return 1 ;
		}
            *ppCurr = pCurr ;
            return bInsideMustExist?rcNotFound:ok ;
            }
        else if (sEndText == NULL ||
	    (*pCurr == *sEndText && strncmp (pCurr, sEndText, nEndText) == 0))
            {
            char * pEnd  ;
	    if (pCDATAStart)
		pCurr += nEndText ;
            pEnd = pCurr - 1 ;

            if (bRemoveSpaces & 32)
		while (pEnd >= pCurrStart && isspace (*pEnd))
		    pEnd-- ;
	    else if (bRemoveSpaces & 64)
		while (pEnd >= pCurrStart && (*pEnd == ' ' || *pEnd == '\t'  || *pEnd == '\r'))
		    pEnd-- ;

            if ((pEnd - pCurrStart + 1 != 0 || nCDataType == ntypAttrValue) && nCDataType)
		if (!(xNewNode = Node_appendChild (pDomTree,  xParentNode, 0, nCDataType, 0, 
						    pCurrStart, pEnd - pCurrStart + 1, level, 
						    GetLineNoOf (r, pCurr), NULL)))
		    return 1 ;

            if (!pCDATAStart && !sStopText)
		pCurr += nEndText ;
            *ppCurr = pCurr ;
            return bInsideMustExist?rcNotFound:ok ;
            }
        else if (!pToken && bFollow < 2)
	    pCurr++ ;
        }
        
    if (nCDataType && pCurr - pCurrStart)
	if (!(xNewNode = Node_appendChild (pDomTree, xParentNode, 0, nCDataType, 0,
					    pCurrStart, pCurr - pCurrStart, level, 
					    GetLineNoOf (r, pCurrStart), NULL)))
	    return 1 ;

    *ppCurr = pCurr ;
    return bInsideMustExist?rcNotFound:ok ;
    }



/* ------------------------------------------------------------------------ */
/*                                                                          */
/* embperl_Parse                                                            */
/*                                                                          */
/*! 
*   \_en
*   Parse source into given DomTree
*   
*   @param  r               Embperl request record
*   @param  pSource         Sourcetext
*   @param  nLen            Length of Sourcetext
*   @param  pDomTree	    Destination DomTree
*   @return                 error code
*   \endif                                                                       
*
*   \_de									   
*   Parst die Quelle in den gegebenen  DomTree
*   
*   @param  r               Embperl request record
*   @param  pSource         Quellentext
*   @param  nLen            Länge des Quellentext
*   @param  pDomTree	    Ziel DomTree
*   @return                 Fehlercode
*   \endif                                                                       
*                                                                          
* ------------------------------------------------------------------------ */


static int embperl_ParseSource (/*i/o*/ register req * r,
                   /*in*/  char *   pSource,
                   /*in*/  size_t         nLen,
		   /*in*/  tDomTree * pDomTree)

    {
    char * pStart = pSource ;
    char * pEnd   = pSource + nLen ;
    int	    rc ;
    tNode   xDocNode ;
    tNode   xNode ;
    tTokenTable * pTokenTableSave ;
    clock_t	cl1 = clock () ;
    clock_t	cl2 ;

    r -> Buf.pBuf    = (char *)pStart ;
    r -> Buf.pEndPos = (char *)pEnd ;
    r -> Buf.pSourcelinePos = r -> Buf.pCurrPos = r -> Buf.pBuf ;

    if (r -> bDebug & dbgParse)
	lprintf (r, "[%d]PARSE: Start parsing %s DomTree = %d\n", r -> nPid, r -> Buf.pFile -> sSourcefile, r -> xCurrDomTree) ; 
    
    pDomTree -> xFilename = String2Ndx (r -> Buf.pFile -> sSourcefile, strlen (r -> Buf.pFile -> sSourcefile)) ;
    
    if (!(xDocNode = Node_appendChild (pDomTree, 0, 0, ntypTag, 0, "attr", 3, 0, 0, NULL)))
	return rcOutOfMemory ;

    if (!(xDocNode = Node_appendChild (pDomTree,  0, 0, r -> bSubReq?ntypDocumentFraq:ntypDocument, 0, 
					NULL, r -> bSubReq?xDocumentFraq:xDocument, 0, 0, NULL)))
	return rcOutOfMemory ;
    
    if (!(xNode = Node_appendChild (pDomTree, xDocNode, 0, ntypAttr, 0, NULL, xDomTreeAttr, 0, 0, NULL)))
	return rcOutOfMemory ;

    if (!(xNode = Node_appendChild (pDomTree, xNode, 0, ntypAttrValue, 0, NULL, r -> xCurrDomTree, 0, 0, NULL)))
	return rcOutOfMemory ;

    /* Add at least one child node to document to make insertafter at the beginning of the document work */
    if (!(xNode = Node_appendChild (pDomTree, xDocNode, 0, ntypCDATA, 0, "", 0, 0, 0, NULL)))
	return rcOutOfMemory ;

    pDomTree -> xDocument = xDocNode ;

    pTokenTableSave = r -> pTokenTable ;
    
    if ((rc = ParseTokens (r, &pStart, pEnd, r -> pTokenTable, "", NULL, r -> pTokenTable -> nDefNodeType, 0, 0, 0, 0, String2Ndx("root", 4), xDocNode, 0, NULL, NULL)) != ok)
	return rc ; 
    
    /* Add one child node end the end to catch loops that end at the very last node */
    if (!(xNode = Node_appendChild (pDomTree, xDocNode, 0, ntypCDATA, 0, "", 0, 0, 0, NULL)))
	return rcOutOfMemory ;

    r -> pTokenTable = pTokenTableSave ;


#ifdef CLOCKS_PER_SEC
    if (r -> bDebug)
	{
        cl2 = clock () ;
	lprintf (r, "[%d]PERF: Parse Start Time:	    %d ms \n", r -> nPid, ((cl1 - r -> startclock) * 1000 / CLOCKS_PER_SEC)) ;
	lprintf (r, "[%d]PERF: Parse End Time:		    %d ms \n", r -> nPid, ((cl2 - r -> startclock) * 1000 / CLOCKS_PER_SEC)) ;
	lprintf (r, "[%d]PERF: Parse Time:		    %d ms \n", r -> nPid, ((cl2 - cl1) * 1000 / CLOCKS_PER_SEC)) ;
	DomStats () ;
	}
#endif        

    return ok ;
    }


/* ------------------------------------------------------------------------ */
/*                                                                          */
/* embperl_Parse                                                            */
/*                                                                          */
/*! 
*   \_en
*   Parse source and create DomTree
*   
*   @param  r               Embperl request record
*   @param  pSource         Sourcetext
*   @param  nLen            Length of Sourcetext
*   @param  pxDomTree	    Returns DomTree index
*   @return                 error code
*   \endif                                                                       
*
*   \_de									   
*   Parst die Quelle und erzeugt einen DomTree
*   
*   @param  r               Embperl request record
*   @param  pSource         Quellentext
*   @param  nLen            Länge des Quellentext
*   @param  pxDomTree	    Gibt DomTree Index zurück
*   @return                 Fehlercode
*   \endif                                                                       
*                                                                          
* ------------------------------------------------------------------------ */


int embperl_Parse (/*i/o*/ register req * r,
                   /*in*/  char *	  pSource,
                   /*in*/  size_t         nLen,
                   /*out*/ tIndex *       pxDomTree)

    {
    int	    rc ;
    tDomTree * pDomTree  ;
    
    if (!(r -> xCurrDomTree  = DomTree_new (&pDomTree)))
	return rcOutOfMemory ;

    if ((rc = embperl_ParseSource (r, pSource, nLen, pDomTree)) != ok)
	{
	pDomTree = DomTree_self (r -> xCurrDomTree) ;
	*pxDomTree = r -> xCurrDomTree  = 0 ;
	DomTree_delete (pDomTree) ;
	return rc ;
	}

    *pxDomTree = r -> xCurrDomTree  ;
    return ok ;
    }


/* ------------------------------------------------------------------------ */
/*                                                                          */
/* ParseFile                                                                */
/*                                                                          */
/* Parse a source file                                                      */
/*                                                                          */
/* ------------------------------------------------------------------------ */


int ParseFile (/*i/o*/ register req * r)

    {
    char * pStart = r -> Buf.pBuf ;
    char * pEnd   = r -> Buf.pEndPos ;
    tIndex xDomTree ;

    return embperl_Parse (r, pStart, pEnd - pStart, &xDomTree) ;
    }