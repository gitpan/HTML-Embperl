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
#   $Id: epcomp.c,v 1.4.2.80 2001/11/16 11:29:02 richter Exp $
#
###################################################################################*/


#include "ep.h"
#include "epmacro.h"

struct tEmbperlCmd
    {
    int             bValid ;
    const char * *  sPerlCode ;             /* perl code that should be inserted (maybe an array) */
    const char * *  sCompileTimePerlCode ;  /* perl code that should be directly executed (maybe an array) */
    const char *    sCompileTimePerlCodeEnd ;  /* perl code that should be directly executed at the end tag */
    const char *    sPerlCodeEnd ;          /* perl code that should be inserted at the end tag  */
    const char *    sStackName ;
    const char *    sPushStack ;
    const char *    sPopStack ;
    const char *    sMatchStack ;
    const char *    sStackName2 ;
    const char *    sPushStack2 ;
    const char *    sPopStack2 ;
    int		    numPerlCode ;
    int		    numCompileTimePerlCode ;
    int		    bRemoveNode ;
    int		    bPerlCodeRemove ;
    int		    bCompileChilds ;
    int		    nNodeType ;
    int		    nSwitchCodeType ;
    int		    bCallReturn ;
    const char *    sMayJump ;
    struct tEmbperlCmd * pNext ;
    } ;

typedef struct tEmbperlCmd tEmbperlCmd ;


struct tEmbperlCompilerInfo
    {
    tStringIndex nMaxEmbperlCmd ;
    tEmbperlCmd * pEmbperlCmds ;
    } ;

typedef struct tEmbperlCompilerInfo tEmbperlCompilerInfo ;

/* ------------------------------------------------------------------------ */
/*                                                                          */
/* embperl_CompileInit                                                      */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/* ------------------------------------------------------------------------ */


static int embperl_CompileInit (/*out*/ tEmbperlCompilerInfo * * ppInfo)

    {
    tEmbperlCompilerInfo * pInfo = malloc (sizeof (tEmbperlCompilerInfo)) ;

    if (!pInfo)
	return rcOutOfMemory ;



    ArrayNew (&pInfo -> pEmbperlCmds, 256, sizeof (struct tEmbperlCmd)) ;
    ArraySet (&pInfo -> pEmbperlCmds, 0) ;
    pInfo -> nMaxEmbperlCmd = 1 ;
    *ppInfo = pInfo ;

    return ok ;
    }


/* ------------------------------------------------------------------------ */
/*                                                                          */
/* embperl_CompileInitItem                                                  */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/* ------------------------------------------------------------------------ */

int embperl_CompileInitItem      (/*i/o*/ register req * r,
				  /*in*/  HV *           pHash,
				  /*in*/  int            nNodeName,
				  /*in*/  int            nNodeType,
				  /*in*/  int		 nTagSet,
				  /*in*/  void * *	 ppInfo)

    {
    SV * * ppSV ;
    AV * pAV ;
    tEmbperlCmd *  pCmd ;
    tEmbperlCompilerInfo * pInfo = (tEmbperlCompilerInfo *)*ppInfo ;

    if (!pInfo)
	embperl_CompileInit ((tEmbperlCompilerInfo * *)ppInfo) ;
    pInfo = (tEmbperlCompilerInfo *)*ppInfo ;
    



    ArraySet (&pInfo -> pEmbperlCmds, nNodeName) ;
    if (pInfo -> nMaxEmbperlCmd < nNodeName)
	pInfo -> nMaxEmbperlCmd = nNodeName ;
    pCmd = &pInfo -> pEmbperlCmds[nNodeName] ;

    if (pCmd -> bValid)
        {
        tEmbperlCmd * pNewCmd ;
        if (pCmd -> bValid == nTagSet)
            return ok ;
        
        while (pCmd -> pNext)
            {
            if (pCmd -> bValid == nTagSet)
                return ok ;
            pCmd = pCmd -> pNext ;
            }

        if (pCmd -> bValid == nTagSet)
            return ok ;

        pNewCmd = malloc (sizeof (*pNewCmd)) ;
        pCmd -> pNext = pNewCmd ;
        pCmd = pNewCmd ;
        }
    pCmd -> bValid = nTagSet ;

    ppSV = hv_fetch(pHash, "perlcode", 8, 0) ;  
    if (ppSV != NULL && *ppSV != NULL && 
        SvTYPE(*ppSV) == SVt_RV && SvTYPE((pAV = (AV *)SvRV(*ppSV))) == SVt_PVAV)
	{ /* Array reference  */
	int f = AvFILL(pAV) + 1 ;
        int i ;
        STRLEN l ;
        char * s ;

        pCmd -> sPerlCode = malloc (f * sizeof (char *)) ;
        pCmd -> numPerlCode = f ;

        for (i = 0; i < f; i++)
	    {
	    ppSV = av_fetch (pAV, i, 0) ;
	    if (ppSV && *ppSV)
		pCmd -> sPerlCode[i] = strdup (SvPV (*ppSV, l)) ;
            else
		pCmd -> sPerlCode[i] = NULL ;
            }
        }
    else
        {
        if (ppSV)
            {
            STRLEN  l ; 
            
            pCmd -> sPerlCode = malloc (sizeof (char *)) ;
            pCmd -> numPerlCode = 1 ;
            pCmd -> sPerlCode[0] = sstrdup (SvPV (*ppSV, l)) ;
            }
        }
    

    ppSV = hv_fetch(pHash, "compiletimeperlcode", 19, 0) ;  
    if (ppSV != NULL && *ppSV != NULL && 
        SvTYPE(*ppSV) == SVt_RV && SvTYPE((pAV = (AV *)SvRV(*ppSV))) == SVt_PVAV)
	{ /* Array reference  */
	int f = AvFILL(pAV) + 1 ;
        int i ;
        STRLEN l ;
        char * s ;

        pCmd -> sCompileTimePerlCode = malloc (f * sizeof (char *)) ;
        pCmd -> numCompileTimePerlCode = f ;

        for (i = 0; i < f; i++)
	    {
	    ppSV = av_fetch (pAV, i, 0) ;
	    if (ppSV && *ppSV)
		pCmd -> sCompileTimePerlCode[i] = strdup (SvPV (*ppSV, l)) ;
            else
		pCmd -> sCompileTimePerlCode[i] = NULL ;
            }
        }
    else
        {
        if (ppSV)
            {
            STRLEN  l ; 
            
            pCmd -> sCompileTimePerlCode = malloc (sizeof (char *)) ;
            pCmd -> numCompileTimePerlCode = 1 ;
            pCmd -> sCompileTimePerlCode[0] = sstrdup (SvPV (*ppSV, l)) ;
            }
        }
    
    
    
    
    pCmd -> sPerlCodeEnd    = GetHashValueStrDup (pHash, "perlcodeend", NULL) ;
    pCmd -> sCompileTimePerlCodeEnd    = GetHashValueStrDup (pHash, "compiletimeperlcodeend", NULL) ;
    pCmd -> sStackName	    = GetHashValueStrDup (pHash, "stackname", NULL) ;
    pCmd -> sPushStack	    = GetHashValueStrDup (pHash, "push", NULL) ;
    pCmd -> sPopStack	    = GetHashValueStrDup (pHash, "pop", NULL) ;
    pCmd -> sMatchStack	    = GetHashValueStrDup (pHash, "stackmatch", NULL) ;
    pCmd -> sStackName2	    = GetHashValueStrDup (pHash, "stackname2", NULL) ;
    pCmd -> sPushStack2	    = GetHashValueStrDup (pHash, "push2", NULL) ;
    pCmd -> sPopStack2	    = GetHashValueStrDup (pHash, "pop2", NULL) ;
    pCmd -> bRemoveNode	    = GetHashValueInt    (pHash, "removenode", 0) ;
    pCmd -> sMayJump	    = GetHashValueStrDup (pHash, "mayjump", NULL) ;
    pCmd -> bPerlCodeRemove = GetHashValueInt	 (pHash, "perlcoderemove", 0) ;
    pCmd -> bCompileChilds  = GetHashValueInt	 (pHash, "compilechilds", 1) ;
    pCmd -> nSwitchCodeType = GetHashValueInt	 (pHash, "switchcodetype", 0) ;
    pCmd -> bCallReturn     = GetHashValueInt	 (pHash, "callreturn", 0) ;
    pCmd -> nNodeType	    = nNodeType == ntypStartEndTag?ntypStartTag:nNodeType ;
    pCmd -> pNext  = NULL ;

    pInfo -> pEmbperlCmds[nNodeName].bRemoveNode |= pCmd -> bRemoveNode ;
    pInfo -> pEmbperlCmds[nNodeName].bPerlCodeRemove |= pCmd -> bPerlCodeRemove ;
    if (pCmd -> nSwitchCodeType)
	pInfo -> pEmbperlCmds[nNodeName].nSwitchCodeType = pCmd -> nSwitchCodeType ;
    if (pCmd -> sMayJump && !pInfo -> pEmbperlCmds[nNodeName].sMayJump)
	pInfo -> pEmbperlCmds[nNodeName].sMayJump = pCmd -> sMayJump ;

    if (r -> bDebug & dbgBuildToken) 
        lprintf (r, "[%d]EPCOMP: InitItem %s (#%d) perlcode=%s (num=%d) perlcodeend=%s compilechilds=%d removenode=%d\n", 
	                  r -> nPid, Ndx2String(nNodeName), nNodeName, 
			  pCmd -> sPerlCode?pCmd -> sPerlCode[0]:"", 
			  pCmd -> numPerlCode, 
			  pCmd -> sPerlCodeEnd?pCmd -> sPerlCodeEnd:"<undef>",
			  pCmd -> bCompileChilds, 
			  pCmd -> bRemoveNode) ; 

    return ok ;
    }

/* ------------------------------------------------------------------------ */
/*                                                                          */
/* strstrn			                                            */
/*                                                                          */
/* find substring of length n                                               */
/*                                                                          */
/* ------------------------------------------------------------------------ */

static const char * strstrn (const char * s1, const char * s2, int l)

    {
    while (*s1)
	{
	if ((s1 = strchr (s1, *s2)) == NULL)
	    return NULL ;
	if (strncmp (s1, s2, l) == 0)
	    return s1 ;
	s1++ ;
	}

    return NULL ;
    }


/* ------------------------------------------------------------------------ */
/*                                                                          */
/* embperl_CompileAddValue                                                  */
/*                                                                          */
/* Add value to perl code                                                   */
/*                                                                          */
/* ------------------------------------------------------------------------ */


static int embperl_CompileAddValue    (/*in*/  const char * sText,
				        const char * p,
					const char * q,
                                        const char * eq, 
					char op,
					char out,
                                       /*i/o*/  char * *      ppCode )




    {
    const char * or ;
    const char * e ;

    
    if (sText)
	{
	int l = strlen (sText) ;
	if (out == 3)
	    {
	    out = 2 ;
	    while (isspace (*sText))
		sText++, l-- ;
	    while (l > 0 && isspace (sText[l-1]))
		l-- ;
	    }

	if (op == '=' && eq)
	    {
	    eq++ ;
	    do
		{
		or = strchr (eq + 1, '|') ;
		e = or?or:q ;
		if (strnicmp (sText, eq, e - eq) == 0)
		    break ;
		if (or == NULL)
		    return 0 ;
		eq = or + 1 ;
		}
	    while (or) ;
	    }
	else if (op == '~' && eq)
	    {
	    eq++ ;
	    do 
		{
		char * f ;
		
		or = strchr (eq + 1, '|') ;
		e = or?or:q ;
		if (f = (char *)strstrn (sText, eq, e - eq))
		    if (!isalnum (f[e - eq]) && f[e - eq] != '_')
			break ;
		if (or == NULL)
		    return 0 ;
		eq = or + 1 ;
		}
	    while (or) ;
	    }
	else if (op == '!' && sText)
	    {
	    return 0 ;
	    }

        if (out)
	    {
	    if (out == 2)
		{
		StringAdd (ppCode, "'", 1) ;
		StringAdd (ppCode, sText, l) ;
		StringAdd (ppCode, "'", 1) ;
		}
	    else if (out)            
		StringAdd (ppCode, sText, 0) ;
	    }
        }
    else
        {
        if (op != '!' && op != 0)
	    return 0 ;
    	
	/*
        if (out == 2)
	    StringAdd (ppCode, "''", 2) ;
	else */ if (out)           
	    StringAdd (ppCode, "undef", 5) ;
	}

    return 1 ;
    }


/* ------------------------------------------------------------------------ */
/*                                                                          */
/* embperl_CompilePushStack                                                 */
/*                                                                          */
/* Push valuie on named stack                                               */
/*                                                                          */
/* ------------------------------------------------------------------------ */


static void embperl_CompilePushStack    (/*in*/ tDomTree *   pDomTree,
				              const char * sStackName,
				              const char * sStackValue)
                                              
    {
    SV **   ppSV ;
    SV *    pSV ;
    AV *    pAV ;

    ppSV = hv_fetch((HV *)(pDomTree -> pSV), (char *)sStackName, strlen (sStackName), 1) ;  
    if (ppSV == NULL)
        return  ;

    if (*ppSV == NULL || SvTYPE (*ppSV) != SVt_RV)
	{
	if (*ppSV)
	    SvREFCNT_dec (*ppSV) ;
        *ppSV = newRV_noinc ((SV *)(pAV = newAV ())) ;
        }
    else
        pAV = (AV *)SvRV (*ppSV) ;
        
        
    pSV = newSVpv ((char *)sStackValue, strlen (sStackValue)) ;
    SvUPGRADE (pSV, SVt_PVIV) ;
    SvIVX (pSV) = 0 ;
    av_push (pAV, pSV) ;
    }


/* ------------------------------------------------------------------------ */
/*                                                                          */
/* embperl_CompilePopStack                                                  */
/*                                                                          */
/* pop value from named stack                                               */
/*                                                                          */
/* ------------------------------------------------------------------------ */


static void embperl_CompilePopStack    (/*in*/ tDomTree *   pDomTree,
				              const char * sStackName)
                                              
    {
    SV **   ppSV ;
    SV *    pSV ;

    ppSV = hv_fetch((HV *)(pDomTree -> pSV), (char *)sStackName, strlen (sStackName), 0) ;  
    if (ppSV == NULL || *ppSV == NULL || SvTYPE (*ppSV) != SVt_RV)
        return  ;

    pSV = av_pop ((AV *)SvRV (*ppSV)) ;
    SvREFCNT_dec (pSV) ;
    }

/* ------------------------------------------------------------------------ */
/*                                                                          */
/* embperl_CompileMatchStack                                                */
/*                                                                          */
/* check if top of stack value matches given value                          */
/*                                                                          */
/* ------------------------------------------------------------------------ */


static int embperl_CompileMatchStack (/*in*/ tDomTree *   pDomTree,
					     tNodeData *  pNode,
					     const char * sStackName,
				             const char * sStackValue)
                                              
    {
    SV **   ppSV ;
    SV *    pSV ;
    AV *    pAV ;
    STRLEN  l ;
    char *  s ;

    ppSV = hv_fetch((HV *)(pDomTree -> pSV), (char *)sStackName, strlen (sStackName), 0) ;  
    if (ppSV == NULL || *ppSV == NULL || SvTYPE (*ppSV) != SVt_RV)
        return rcHashError ;

    pSV = av_pop ((AV *)SvRV (*ppSV)) ;

    s = SvPV (pSV, l) ;
    if (strcmp (s, sStackValue) == 0)
	{
        SvREFCNT_dec (pSV) ;
	return ok ;
	}
	
    strncpy (pCurrReq -> errdat1, Node_selfNodeName (pNode), sizeof (pCurrReq -> errdat1)) ;
    sprintf (pCurrReq -> errdat2, "'%s', starttag should be '%s' or there is a 'end%s' missing", s, sStackValue, s) ;
    pCurrReq -> Buf.pCurrPos	 = NULL ;
    pCurrReq -> Buf.nSourceline = pNode -> nLinenumber ;

    SvREFCNT_dec (pSV) ;

    return rcTagMismatch ;
    }


/* ------------------------------------------------------------------------ */
/*                                                                          */
/* embperl_CompileAddStack                                                  */
/*                                                                          */
/* Add value of child node to perl code                                     */
/*                                                                          */
/* ------------------------------------------------------------------------ */


static int embperl_CompileAddStack    (/*in*/ tDomTree *   pDomTree,
				                const char * p,
					        const char * q,
					        char op,
					        char out,
                                                char str,
                                       /*i/o*/  char * *      ppCode )
    {
    const char * eq = strchr (p, ':') ;
    const char * e = eq && eq < q?eq:q;
    STRLEN           l ;
    const char * sText = NULL ;
    SV **   ppSV ;
    AV *    pAV ;


    ppSV = hv_fetch((HV *)(pDomTree -> pSV), (char *)p, e - p, 0) ;  
    if (ppSV == NULL || *ppSV == NULL || SvTYPE (*ppSV) != SVt_RV)
        return  op == '!'?1:0 ;

    pAV = (AV *)SvRV (*ppSV) ;

    if (SvTYPE (pAV) != SVt_PVAV)
        return  op == '!'?1:0 ;
    
    ppSV = av_fetch (pAV, av_len (pAV), 0) ;
    if (ppSV == NULL || *ppSV == NULL)
        return  op == '!'?1:0 ;

    if (str)
        {
        sText = SvPV (*ppSV, l) ;    
        (SvIVX (*ppSV))++ ;
        }
    else
        sText = SvIVX (*ppSV)?"1":NULL ;


    return embperl_CompileAddValue (sText, p, q, eq, op, out, ppCode) ;
    }

/* ------------------------------------------------------------------------ */
/*                                                                          */
/* embperl_CompileAddChildNode                                              */
/*                                                                          */
/* Add value of child node to perl code                                     */
/*                                                                          */
/* ------------------------------------------------------------------------ */


static int embperl_CompileAddChildNode (/*in*/ tDomTree *   pDomTree,
    		                 /*in*/ tNodeData *	 pNode,
				        const char * p,
					const char * q,
					char op,
					char out,
                                       /*i/o*/  char * *      ppCode )



    {
    const char * eq = strchr (p, ':') ;
    int nChildNo = atoi (p) ;
    struct tNodeData * pChildNode = Node_selfNthChild (pDomTree, pNode, 0, nChildNo) ;
    const char * sText = NULL ;
    
    if (pChildNode)
	sText = Node_selfNodeName(pChildNode) ;

    return embperl_CompileAddValue (sText, p, q, eq, op, out, ppCode) ;
    }

/* ------------------------------------------------------------------------ */
/*                                                                          */
/* embperl_CompileAddSiblingNode                                            */
/*                                                                          */
/* Add value of sibling node to perl code                                   */
/*                                                                          */
/* ------------------------------------------------------------------------ */


static int embperl_CompileAddSiblingNode (/*in*/ tDomTree *   pDomTree,
    		                 /*in*/ tNodeData *	 pNode,
				        const char * p,
					const char * q,
					char op,
					char out,
                                       /*i/o*/  char * *      ppCode )



    {
    const char * eq = strchr (p, ':') ;
    int nChildNo = atoi (p) ;
    struct tNodeData * pChildNode  ;
    const char * sText = NULL ;
    
    if (nChildNo == 0)
	pChildNode = pNode ; 
    else if (nChildNo > 0)
	{
	nChildNo-- ;
	pChildNode = Node_selfNextSibling (pDomTree, pNode, 0) ;
	while (pChildNode && nChildNo-- > 0)
	    pChildNode = Node_selfNextSibling (pDomTree, pChildNode, 0) ;
	    
	}
    else
	{
	nChildNo++ ;
	pChildNode = Node_selfPreviousSibling (pDomTree, pNode, 0) ;
	while (pChildNode && nChildNo++ < 0)
	    pChildNode = Node_selfPreviousSibling (pDomTree, pChildNode, 0) ;
	    
	}
    
    if (pChildNode)
	sText = Node_selfNodeName(pChildNode) ;

    return embperl_CompileAddValue (sText, p, q, eq, op, out, ppCode) ;
    }

/* ------------------------------------------------------------------------ */
/*                                                                          */
/* embperl_CompileAddAttribut                                               */
/*                                                                          */
/* Add value of child node to perl code                                     */
/*                                                                          */
/* ------------------------------------------------------------------------ */


static int embperl_CompileAddAttribut (/*in*/   tDomTree *   pDomTree,
    		                       /*in*/   tNodeData *	 pNode,
				                const char * p,
					        const char * q,
					        char op,
					        char out,
                                       /*i/o*/  char * *      ppCode )



    {
    const char * eq = strchr (p, ':') ;
    const char * e = eq && eq < q?eq:q;
    tAttrData * pChildNode = Element_selfGetAttribut (pDomTree, pNode, p, e - p) ;
    const char * sText = NULL ;
    char buf [128] ;

    
    if (pChildNode)
	{
	if (pChildNode -> bFlags & aflgAttrChilds)
	    {
	    int l = sprintf (buf, "XML::Embperl::DOM::Attr::iValue ($_ep_DomTree,%d)", pChildNode -> xNdx) ;
	    sText = buf ;
	    if (out == 2)
		out = 1 ;
	    }
	else
	    {
	    sText = Ndx2String (pChildNode -> xValue) ;
	    }

        }
    
    return embperl_CompileAddValue (sText, p, q, eq, op, out, ppCode) ;
    }

/* ------------------------------------------------------------------------ */
/*                                                                          */
/* embperl_CompileToPerlCode                                                */
/*                                                                          */
/* Compile one command inside a node                                        */
/*                                                                          */
/* ------------------------------------------------------------------------ */


static int embperl_CompileToPerlCode  (/*in*/ tDomTree *    pDomTree,
    		                       /*in*/ tNodeData *   pNode,
                                       /*in*/ const char *  sPerlCode,
                                       /*out*/ char * *     ppCode )


    {
    const char * p ;
    const char * q ;
    int    valid = 1 ;
    tNode   xCurrNode = 0 ;

    StringNew (ppCode, 512) ;
    if (sPerlCode)
	{
	p = strchr (sPerlCode, '%') ;	
	while (p)
	    {
	    int n = p - sPerlCode ;
	    if (n)
		StringAdd (ppCode, sPerlCode, n) ;
	    q = strchr (p+1, '%') ;	
	    if (q)
		{
		char  type  ;
		char  op  ;
		char  out = 1 ;

		p++ ;
		type = *p ;
		p++ ;
		op = *p ;
		if (op != '=' && op != '*' && op != '!' && op != '~')
		    op = 0 ;
		else
		    p++ ;

		if (*p == '-')
		    out = 0, p++ ;
		else if (*p == '\'')
		    out = 2, p++ ;
		else if (*p == '"')
		    out = 3, p++ ;

		
		if (type == '#')
		    {
		    if (!embperl_CompileAddChildNode (pDomTree, pNode ,p, q, op, out, ppCode))
			{
			valid = 0 ;
			break ;
			}
		    }
		else if (type == '>')
		    {
		    if (!embperl_CompileAddSiblingNode (pDomTree, pNode ,p, q, op, out, ppCode))
			{
			valid = 0 ;
			break ;
			}
		    }
		else if (type == '&')
		    {
		    if (!embperl_CompileAddAttribut (pDomTree, pNode ,p, q, op, out, ppCode))
			{
			valid = 0 ;
			break ;
			}
		    }
		else if (type == '^')
		    {
		    if (!embperl_CompileAddStack (pDomTree, p, q, op, out, 1, ppCode))
			{
			valid = 0 ;
			break ;
			}
		    }
		else if (type == '?')
		    {
		    if (!embperl_CompileAddStack (pDomTree, p, q, op, out, 0, ppCode))
			{
			valid = 0 ;
			break ;
			}
		    }
		else if (type == '%')
		    {
		    StringAdd (ppCode, "%", 1) ; 
		    }
		else if (type == '$')
		    {
		    if (*p == 'n')
			{
			char s [20] ;
			int  l = sprintf (s, "$_ep_DomTree,%u", pNode -> xNdx) ;
			StringAdd (ppCode, s, l) ; 
			}
		    else if (*p == 't')
			{
			StringAdd (ppCode, "$_ep_DomTree", 0) ; 
			}
		    else if (*p == 'x')
			{
			char s [20] ;
			int  l = sprintf (s, "%u", pNode -> xNdx) ;
			StringAdd (ppCode, s, l) ; 
			}
		    else if (*p == 'l')
			{
			char s [20] ;
			int  l = sprintf (s, "%u", pDomTree -> xLastNode) ;
			StringAdd (ppCode, s, l) ; 
			}
		    else if (*p == 'c')
			{
			char s [20] ;
			if (pDomTree -> xLastNode != pDomTree -> xCurrNode)
			    {
			    int  l = sprintf (s, "$_ep_node=%u;", pDomTree -> xLastNode) ;
			    StringAdd (ppCode, s, l) ; 
			    xCurrNode = pDomTree -> xLastNode ;
			    }
			}
		    else if (*p == 'q')
			{
			char s [20] ;
			int  l = sprintf (s, "%u", pDomTree -> xNdx) ;
			StringAdd (ppCode, s, l) ; 
			}
		    else if (*p == 'p')
			{
			char s [20] ;
			int  l = sprintf (s, "%u", ArrayGetSize (pDomTree -> pCheckpoints)) ;
			StringAdd (ppCode, s, l) ; 
			}
		    else if (*p == 'k')
			{
			char s [40] ;
                        int  l ;
	                tIndex nCheckpointArrayOffset = ArrayAdd (&pDomTree -> pCheckpoints, 1) ;
	                pDomTree -> pCheckpoints[nCheckpointArrayOffset].xNode = pNode -> xNdx ;
	                l = sprintf (s, " _ep_cp(%d) ;\n", nCheckpointArrayOffset) ;
			StringAdd (ppCode, s, l) ; 

	                if (pCurrReq -> bDebug & dbgCompile)
	                    lprintf (pCurrReq, "[%d]EPCOMP: #%d L%d Checkpoint\n", pCurrReq -> nPid, pNode -> xNdx, pNode -> nLinenumber) ;
                        }
                    
                    
                    }

		sPerlCode = q + 1 ;
		p = strchr (sPerlCode, '%') ;	
		}
	    else
		{
		sPerlCode = p ;
		p = NULL ; 
		}
	    }
	if (valid)
	    {
	    StringAdd (ppCode, sPerlCode,  0) ; 
	    if (xCurrNode)
    		pDomTree -> xCurrNode = xCurrNode ;
	    }
				    
	}
    return valid ;
    }


/* ------------------------------------------------------------------------ */
/*                                                                          */
/* embperl_CompileCleanupSpaces                                             */
/*                                                                          */
/* remove any following spaces                                              */
/*                                                                          */
/* ------------------------------------------------------------------------ */


static int embperl_CompileCleanupSpaces  (/*in*/  tReq *	r,
					  /*in*/  tDomTree *	pDomTree,
					  /*in*/  tNodeData *	pNode,
					  /*i/o*/ tEmbperlCmd *	pCmd)


    {
    if ((pCmd -> bRemoveNode & 6) && (r -> bOptions & optKeepSpaces) == 0)
	{
	tNodeData *  pNextNode   = Node_selfFirstChild (pDomTree, pNode, 0) ;
	if ((pCmd -> bRemoveNode & 1) || !pCmd -> bCompileChilds || pNextNode == NULL || (pNextNode -> nType != ntypText && pNextNode -> nType != ntypCDATA))
	    pNextNode    = Node_selfNextSibling (pDomTree, pNode, 0) ;
	if (pNextNode)
	    {
	    const char * sText        = Node_selfNodeName (pNextNode) ;
	    const char * p            = sText ;

	    while (*p && isspace (*p))
		p++;
	    if (p > sText && (pCmd -> bRemoveNode & 4))
		p-- ;

	    if (p > sText)
		{ /* remove spaces */
		if (*p)
		    Node_replaceChildWithCDATA(pDomTree, pNextNode -> xNdx, 0, p, strlen (p), -1, 0) ;
		else
		    Node_selfRemoveChild(pDomTree, -1, pNextNode) ;
		}

	    }
	}
    return ok ;
    }



/* ------------------------------------------------------------------------ */
/*                                                                          */
/* embperl_CompileCmd							    */
/*                                                                          */
/* Compile one cmd of one node						    */
/*                                                                          */
/* ------------------------------------------------------------------------ */


static int embperl_CompileCmd  (/*in*/	tReq *	       r,
			 /*in*/  tDomTree *	pDomTree,
			 /*in*/  tNodeData *	pNode,
			 /*in*/  tEmbperlCmd *	pCmd,
			 /*out*/ int *		nStartCodeOffset)


    {
    int rc ;
    char *          pCode = NULL ; 
    char *          pCTCode = NULL ; 
    char *          sSourcefile ;
    int             nSourcefile ;
    int i ;
    SV *        args[4] ;
    int nCodeLen  ;
    int found = 0 ;

    r -> pCodeSV = NULL ;

    Ndx2StringLen (pDomTree -> xFilename, sSourcefile, nSourcefile) ;

    if (pCmd -> nNodeType != pNode -> nType)
	return ok ;

    for (i = 0; i < pCmd -> numPerlCode; i++)
	if (embperl_CompileToPerlCode (pDomTree, pNode, pCmd -> sPerlCode[i], &pCode))
	    {
	    found = 1 ;
	    break ;
	    }

    if (found && pCode)
	{
	nCodeLen = ArrayGetSize (pCode) ;

	if (nCodeLen)
	    {
	    char buf [32] ;

	    if (pNode ->  nLinenumber && pNode ->  nLinenumber != pDomTree -> nLastLinenumber )
		{
		int l2 = sprintf (buf, "#line %d \"", pDomTree -> nLastLinenumber = pNode ->	nLinenumber) ;

		StringAdd (r -> pProg, buf, l2) ;
		StringAdd (r -> pProg, sSourcefile, nSourcefile) ;
		StringAdd (r -> pProg, "\"\n", 2) ;
		}

	    if (pCmd -> bPerlCodeRemove)
		*nStartCodeOffset = StringAdd (r -> pProg, " ", 1) ;
	    }
	else
	    {
	    StringFree (&pCode) ;
	    pCode = NULL ;
	    }
	}
    else
	{
	StringFree (&pCode) ;
	pCode = NULL ;
	}

    for (i = 0; i < pCmd -> numCompileTimePerlCode; i++)
	{
	if (embperl_CompileToPerlCode (pDomTree, pNode, pCmd -> sCompileTimePerlCode[i], &pCTCode))
	    {
	    SV * pSV ;
	    int   rc ;

	    if (pCTCode)
		{
		int l = ArrayGetSize (pCTCode) ;
		int i = l ;
		char *p = pCTCode ;

		if (r -> bDebug & dbgCompile)
		    lprintf (r, "[%d]EPCOMP: #%d L%d CompileTimeCode:    %*.*s\n", r -> nPid, pNode -> xNdx, pNode -> nLinenumber, l, l, pCTCode) ;

		while (i--)
		    { /* keep everything on one line, to make linenumbers correct */
		    if (*p == '\r' || *p == '\n')
			*p = ' ' ;
		    p++ ;
		    }
		
		
		pSV = newSVpvf("package %s ;\n#line %d \"%s\"\n%*.*s",
			r -> Buf.sEvalPackage, pNode ->	nLinenumber, sSourcefile, l,l, pCTCode) ;
		newSVpvf2(pSV) ;
		args[0] = r -> pReqSV ;
		if (pCode)
		    {			
		    r -> pCodeSV = newSVpv (pCode, nCodeLen) ;
		    }
		else
		    r -> pCodeSV = &sv_undef ; 
                SvTAINTED_off (pSV) ;
                if ((rc = EvalDirect (r, pSV, 1, args)) != ok)
		    LogError (r, rc) ;
		SvREFCNT_dec(pSV);
		}
	    break ;
	    }
	}

    if (r -> pCodeSV)
	{
	STRLEN l ;
	char * p = SvPV (r -> pCodeSV, l) ;
	StringAdd (r -> pProg, p, l ) ;
	StringAdd (r -> pProg, "\n",  1) ;
	if (r -> bDebug & dbgCompile)
	    lprintf (r, "[%d]EPCOMP: #%d L%d Code:    %s\n", r -> nPid, pNode -> xNdx, pNode -> nLinenumber, p) ;
	}
    else if (pCode)
	{
	StringAdd (r -> pProg, pCode, nCodeLen ) ;
	StringAdd (r -> pProg, "\n",  1) ;
	if (r -> bDebug & dbgCompile)
	    lprintf (r, "[%d]EPCOMP: #%d L%d Code:    %*.*s\n", r -> nPid, pNode -> xNdx, pNode -> nLinenumber, nCodeLen, nCodeLen, pCode) ;
	}    
    
    StringFree (&pCode) ;
    StringFree (&pCTCode) ;

    if (r -> pCodeSV)
	{
	SvREFCNT_dec(r -> pCodeSV);
	r -> pCodeSV = NULL ;
	}
    return ok ;
    }



/* ------------------------------------------------------------------------ */
/*                                                                          */
/* embperl_CompilePostProcess						    */
/*                                                                          */
/* Do some postprocessing after compiling				    */
/*                                                                          */
/* ------------------------------------------------------------------------ */


static int embperl_CompilePostProcess  (/*in*/	tReq *	       r,
			 /*in*/  tDomTree *	pDomTree,
			 /*in*/  tNodeData *	pNode,
			 /*in*/  tEmbperlCmd *	pCmd,
			 /*in*/  int		nCheckpointCodeOffset,
			 /*in*/  int		nCheckpointArrayOffset,
			 /*i/o*/ int *		bCheckpointPending)


    {
    int rc ;
    char *          sStackValue = NULL ;
    int i ;


    embperl_CompileCleanupSpaces (r, pDomTree, pNode, pCmd) ;

    if (pCmd -> sMayJump)
	if (embperl_CompileToPerlCode (pDomTree, pNode, pCmd -> sMayJump, &sStackValue))
	    {
	    *bCheckpointPending = -1 ;
	    if (r -> bDebug & dbgCompile)
		lprintf (r, "[%d]EPCOMP: #%d L%d Set Checkpoint pending\n", r -> nPid, pNode -> xNdx, pNode -> nLinenumber) ;
	    }

    if (pCmd -> bRemoveNode & 1)
	pNode -> bFlags = 0 ; 
    else if (pCmd -> bRemoveNode & 8)
	pNode -> bFlags |= nflgIgnore ;
    if (pCmd -> bRemoveNode & 16)
	{
	tNodeData * pChild ;
	while (pChild = Node_selfFirstChild (pDomTree, pNode, 0))
	    {
	    Node_selfRemoveChild (pDomTree, pNode -> xNdx, pChild) ;
	    }
	}
    else if (pCmd -> bRemoveNode & 32)
	{
	tNodeData * pChild = Node_selfFirstChild (pDomTree, pNode, 0) ;
	while (pChild)
	    {
	    pChild -> bFlags |= nflgIgnore ;
            pChild = Node_selfNextSibling (pDomTree, pChild, 0) ;

	    }
	}


    if (nCheckpointCodeOffset && (pNode -> bFlags == 0 || (pNode -> bFlags & nflgIgnore)))
	{
	(*r -> pProg)[nCheckpointCodeOffset] = '#' ;
	nCheckpointArrayOffset = ArraySub (&pDomTree -> pCheckpoints, 1) ;
        if (r -> bDebug & dbgCompile)
	    lprintf (r, "[%d]EPCOMP: #%d L%d Remove Checkpoint\n", r -> nPid, pNode -> xNdx, pNode -> nLinenumber) ;
	nCheckpointCodeOffset = 0 ;
	*bCheckpointPending = -1 ; /* set checkpoint on next possibility */
        }

    if (*bCheckpointPending && (pNode -> bFlags & nflgIgnore))
	{
	int l ;
	char buf [80] ;

	nCheckpointArrayOffset = ArrayAdd (&pDomTree -> pCheckpoints, 1) ;
	pDomTree -> pCheckpoints[nCheckpointArrayOffset].xNode = pNode -> xNdx ;
	*bCheckpointPending = 0 ;
	l = sprintf (buf, " _ep_cp(%d) ;\n", nCheckpointArrayOffset) ;
	nCheckpointCodeOffset = StringAdd (r -> pProg, buf,	l) ;

	if (r -> bDebug & dbgCompile)
	    lprintf (r, "[%d]EPCOMP: #%d L%d Checkpoint\n", r -> nPid, pNode -> xNdx, pNode -> nLinenumber) ;

	}

    if (pCmd -> sPopStack)
	embperl_CompilePopStack (pDomTree, pCmd -> sPopStack) ;
    if (pCmd -> sPopStack2)
	embperl_CompilePopStack (pDomTree, pCmd -> sPopStack2) ;

    if (pCmd -> sStackName)
	{
	if (pCmd -> sMatchStack && pNode -> nType != ntypStartTag && pNode -> nType != ntypDocument && pNode -> nType != ntypDocumentFraq)
	    {
	    if (embperl_CompileToPerlCode (pDomTree, pNode, pCmd -> sMatchStack, &sStackValue))
		if ((rc = embperl_CompileMatchStack (pDomTree, pNode, pCmd -> sStackName, sStackValue)) != ok)
		    {
		    StringFree (&sStackValue) ;
		    return rc ;
		    }
	    }
	if (pCmd -> sPushStack)
	    {
	    if (embperl_CompileToPerlCode (pDomTree, pNode, pCmd -> sPushStack, &sStackValue))
		embperl_CompilePushStack (pDomTree, pCmd -> sStackName, sStackValue) ;
	    }
	}
    if (pCmd -> sStackName2 && pCmd -> sPushStack2)
	{
	if (embperl_CompileToPerlCode (pDomTree, pNode, pCmd -> sPushStack2, &sStackValue))
	    {
	    embperl_CompilePushStack (pDomTree, pCmd -> sStackName2, sStackValue) ;
	    }
	}

    StringFree (&sStackValue) ;

    return ok ;
    }
    
    



/* ------------------------------------------------------------------------ */
/*                                                                          */
/* embperl_CompileCmdEnd						   */
/*                                                                          */
/* Compile the end of the node						    */
/*                                                                          */
/* ------------------------------------------------------------------------ */


static int embperl_CompileCmdEnd (/*in*/  tReq *	 r,
			 /*in*/  tDomTree *	pDomTree,
			 /*in*/  tNodeData *	pNode,
			 /*in*/  tEmbperlCmd *	pCmd,
			 /*in*/  int		nStartCodeOffset,
			 /*i/o*/ int *		bCheckpointPending)


    {
    int rc ;
    char *          sStackValue = NULL ;
    int i ;
    char *          pCode = NULL ; 
    char *          pCTCode = NULL ; 
    SV *	    args[4] ;
    STRLEN	    nCodeLen  = 0 ;


    if (pCmd -> nNodeType != pNode -> nType)
	return ok ;


    if (pCmd)
	{
        if (pCmd -> sPerlCodeEnd && embperl_CompileToPerlCode (pDomTree, pNode, pCmd -> sPerlCodeEnd, &pCode))
            nCodeLen = ArrayGetSize (pCode) ;
	    
	if (embperl_CompileToPerlCode (pDomTree, pNode, pCmd -> sCompileTimePerlCodeEnd, &pCTCode))
	    {
	    SV * pSV ;
	    int   rc ;

	    if (pCTCode && *pCTCode)
		{
		int l = ArrayGetSize (pCTCode) ;
		char *          sSourcefile ;
		int             nSourcefile ;
		int i = l ;
		char * p = pCTCode ;
		
		Ndx2StringLen (pDomTree -> xFilename, sSourcefile, nSourcefile) ;

		if (r -> bDebug & dbgCompile)
		    lprintf (r, "[%d]EPCOMP: #%d L%d CompileTimeCodeEnd:    %*.*s\n", r -> nPid, pNode -> xNdx, pNode -> nLinenumber, l, l, pCTCode) ;

		while (i--)
		    { /* keep everything on one line, to make linenumbers correct */
		    if (*p == '\r' || *p == '\n')
			*p = ' ' ;
		    p++ ;
		    }
		

		
		pSV = newSVpvf("package %s ;\n#line %d \"%s\"\n%*.*s",
			r -> Buf.sEvalPackage, pNode ->	nLinenumber, sSourcefile, l,l, pCTCode) ;
		newSVpvf2(pSV) ;
		args[0] = r -> pReqSV ;
		if (pCode)
		    {			
		    r -> pCodeSV = newSVpv (pCode, nCodeLen) ;
		    }
		else
		    r -> pCodeSV = &sv_undef ; 
		if ((rc = EvalDirect (r, pSV, 1, args)) != ok)
		    LogError (r, rc) ;
		SvREFCNT_dec(pSV);
		}
	    }

	if (r -> pCodeSV)
	    {
	    if (SvOK (r -> pCodeSV))
		{
		char * p = SvPV (r -> pCodeSV, nCodeLen) ;
		if (nCodeLen)
		    {			
		    StringAdd (r -> pProg, p, nCodeLen ) ;
		    StringAdd (r -> pProg, "\n",  1) ;
		    if (r -> bDebug & dbgCompile)
			lprintf (r, "[%d]EPCOMP: #%d L%d CodeEnd:    %s\n", r -> nPid, pNode -> xNdx, pNode -> nLinenumber, p) ;
		    }
		}
	    }
	else if (pCode && nCodeLen)
	    {
	    StringAdd (r -> pProg, pCode, nCodeLen ) ;
	    StringAdd (r -> pProg, "\n",  1) ;
	    if (r -> bDebug & dbgCompile)
		lprintf (r, "[%d]EPCOMP: #%d L%d CodeEnd:    %*.*s\n", r -> nPid, pNode -> xNdx, pNode -> nLinenumber, nCodeLen, nCodeLen, pCode) ;
	    }    
	if (nCodeLen == 0)
	    {
	    if (pCmd -> bPerlCodeRemove && nStartCodeOffset)
		{
		(*r -> pProg)[nStartCodeOffset] = '#' ;
		if (r -> bDebug & dbgCompile)
		    lprintf (r, "[%d]EPCOMP: #%d L%d Remove Codeblock\n", r -> nPid, pNode -> xNdx, pNode -> nLinenumber) ; 
		}
	    }
        if (pCmd -> sPerlCodeEnd && pCmd -> sMayJump)
            if (embperl_CompileToPerlCode (pDomTree, pNode, pCmd -> sMayJump, &sStackValue))
	        {
		*bCheckpointPending = -1 ;
		if (r -> bDebug & dbgCompile)
		    lprintf (r, "[%d]EPCOMP: #%d L%d Set Checkpoint pending\n", r -> nPid, pNode -> xNdx, pNode -> nLinenumber) ; 
	        }
	if (pCmd -> sStackName  && (pNode -> nType == ntypStartTag || pNode -> nType == ntypDocument || pNode -> nType == ntypDocumentFraq))
	    {
	    if (pCmd -> sMatchStack)
		{
		if (embperl_CompileToPerlCode (pDomTree, pNode, pCmd -> sMatchStack, &sStackValue))
		    {
		    if ((rc = embperl_CompileMatchStack (pDomTree, pNode, pCmd -> sStackName, sStackValue)) != ok)
			{
			StringFree (&pCode) ;
			StringFree (&pCTCode) ;
			StringFree (&sStackValue) ;
			return rc ;
			}
		    }
		}
	    else if (pCmd -> sPushStack && pCmd -> sPerlCodeEnd)
		embperl_CompilePopStack (pDomTree, pCmd -> sStackName) ;
	    }

        if (pCmd -> sStackName2 && pCmd -> sPushStack2 && pCmd -> sPerlCodeEnd)
            embperl_CompilePopStack (pDomTree, pCmd -> sStackName2) ;
  
	if (pCmd -> nSwitchCodeType == 1)
            {
            r -> pProg = &r -> pProgRun ;
	    *bCheckpointPending = -1 ;
	    if (r -> bDebug & dbgCompile)
		lprintf (r, "[%d]EPCOMP: #%d L%d Set Checkpoint pending (switch to ProgRun)\n", r -> nPid, pNode -> xNdx, pNode -> nLinenumber) ;
            }
        }


    StringFree (&pCode) ;
    StringFree (&pCTCode) ;

    if (r -> pCodeSV)
	{
	SvREFCNT_dec(r -> pCodeSV);
	r -> pCodeSV = NULL ;
	}
	    
    StringFree (&sStackValue) ;

    return ok ;
    }


/* ------------------------------------------------------------------------ */
/*                                                                          */
/* embperl_CompileNode                                                      */
/*                                                                          */
/* Compile one node and his childs                                          */
/*                                                                          */
/* ------------------------------------------------------------------------ */


int embperl_CompileNode (/*in*/  tReq *         r,
			 /*in*/  tDomTree *	pDomTree,
			 /*in*/  tNode		xNode,
			 /*i/o*/ int *		bCheckpointPending)


    {
    int rc ;
    tNode           xChildNode  ;
    tStringIndex    nNdx  ;
    tEmbperlCmd *   pCmd  ;
    tEmbperlCmd *   pCmdHead  ;
    tNodeData *     pNode = Node_self (pDomTree, xNode) ;
    tAttrData *     pAttr ;
    int             nAttr = 0 ;
    int		    nStartCodeOffset = 0 ;               
    int		    nCheckpointCodeOffset = 0 ;               
    int		    nCheckpointArrayOffset = 0 ;               
    tEmbperlCompilerInfo * pInfo = (tEmbperlCompilerInfo *)(*(void * *)r -> pTokenTable) ;
    tIndex          xDomTree = pDomTree -> xNdx ;

    pCmd = NULL ;
    
    nNdx = Node_selfNodeNameNdx (pNode) ;

    if (nNdx <= pInfo -> nMaxEmbperlCmd)
	{
	pCmd = pCmdHead = &(pInfo -> pEmbperlCmds[nNdx]) ;
	/* ??if (pCmd -> nNodeType != pNode -> nType) */
	/*	 pCmd = NULL ; */
	}
    else
	pCmd = pCmdHead = NULL ;
    

    if (r -> bDebug & dbgCompile)
        {
        char buf[20] ;
        lprintf (r, "[%d]EPCOMP: #%d L%d -------> parent=%d node=%d type=%d text=%s (#%d,%s) %s\n", 
		     r -> nPid, pNode -> xNdx, pNode -> nLinenumber, 
		     Node_parentNode (pDomTree, pNode -> xNdx, 0), pNode -> xNdx,
                     pNode -> nType, Node_selfNodeName(pNode), nNdx, pCmd?"compile":"-", (pCmd && pCmd -> bRemoveNode)?(sprintf (buf, "removenode=%d", pCmd -> bRemoveNode), buf):"") ;
        }
    

    if (pCmd == NULL || (pCmd -> bRemoveNode & 1) == 0)
        pDomTree -> xLastNode = xNode ;

    /*    if (*bCheckpointPending && (pNode -> nType == ntypText || pNode -> nType == ntypCDATA) && pNode -> bFlags && (pNode -> bFlags & nflgIgnore) == 0) */
    /*    if (*bCheckpointPending &&	 pNode -> bFlags && (pNode -> bFlags & nflgIgnore) == 0) */
    if (*bCheckpointPending &&	!(pCmd && pCmd -> nSwitchCodeType == 2) && pNode -> bFlags && (pNode -> bFlags & nflgIgnore) == 0)
	{
	int l ;
	char buf [80] ;
	
	nCheckpointArrayOffset = ArrayAdd (&pDomTree -> pCheckpoints, 1) ;
	pDomTree -> pCheckpoints[nCheckpointArrayOffset].xNode = xNode ;
	*bCheckpointPending = 0 ;
	l = sprintf (buf, " _ep_cp(%d) ;\n", nCheckpointArrayOffset) ;
	nCheckpointCodeOffset = StringAdd (r -> pProg, buf,  l) ; 

	if (r -> bDebug & dbgCompile)
	    lprintf (r, "[%d]EPCOMP: #%d L%d Checkpoint\n", r -> nPid, pNode -> xNdx, pNode -> nLinenumber) ; 
	
	}

    if (pCmd && pCmd -> nSwitchCodeType == 2)
        {
        r -> pProg = &r -> pProgDef ;
        nCheckpointArrayOffset = 0 ;
        nCheckpointCodeOffset = 0 ;
        }
	
    if (pCmd == NULL || (pCmd -> bRemoveNode & 8) == 0)
        { /* calculate attributes before tag, but not when tag should be ignored in output stream */
        while (pAttr = Element_selfGetNthAttribut (pDomTree, pNode, nAttr++))
	    {
            if (pAttr -> bFlags & aflgAttrChilds)
                {
                tNodeData * pChild = Node_selfFirstChild (pDomTree, (tNodeData *)pAttr, 0) ;
                tNodeData * pNext ;

                while (pChild)
                    {
                    embperl_CompileNode (r, pDomTree, pChild -> xNdx, bCheckpointPending) ;
	            pDomTree = DomTree_self (xDomTree) ; /* addr may have changed */
                    pNext = Node_selfNextSibling (pDomTree, pChild, 0) ;
                    if (pChild -> bFlags == 0)
                        Node_selfRemoveChild(pDomTree, -1, pChild) ;
                    pChild = pNext ;
                    }
                }                

	    }
        }            
    

    while (pCmd)
	{
	if ((rc = embperl_CompileCmd (r, pDomTree, pNode, pCmd, &nStartCodeOffset)) != ok)
	    return rc ;
	pDomTree = DomTree_self (xDomTree) ; /* addr may have changed */
        pCmd = pCmd -> pNext ;
	}

    pCmd = pCmdHead ;
    if (pCmd)
        if ((rc = embperl_CompilePostProcess (r, pDomTree, pNode, pCmd, nCheckpointCodeOffset, nCheckpointArrayOffset, bCheckpointPending)) != ok)
	    return rc ;


    if (pCmd == NULL || pCmd -> bCompileChilds)
	{
	tNodeData * pChildNode ;

	xChildNode = pNode -> bFlags?Node_firstChild (pDomTree, xNode, 0):0 ;

	while (xChildNode)
	    {
	    if ((rc = embperl_CompileNode (r, pDomTree, xChildNode, bCheckpointPending)) != ok)
		return rc ;

	    pDomTree = DomTree_self (xDomTree) ; /* addr may have changed */
	    pChildNode = Node_self (pDomTree, xChildNode) ;
            xChildNode  = Node_nextSibling (pDomTree, xChildNode, 0) ;
            if (pChildNode -> bFlags == 0)
                Node_selfRemoveChild(pDomTree, -1, pChildNode) ;
            }
	}
	    

    while (pCmd)
	{
	if ((rc = embperl_CompileCmdEnd (r, pDomTree, pNode, pCmd, nStartCodeOffset, bCheckpointPending)) != ok)
	    return rc ;
	pCmd = pCmd -> pNext ;
	}

    if (pCmdHead && pCmdHead -> nSwitchCodeType == 2)
        {
        r -> pProg = &r -> pProgRun ;
        nCheckpointArrayOffset = 0 ;
        nCheckpointCodeOffset = 0 ;
        }


    return ok ;
    }



/* ------------------------------------------------------------------------ */
/*                                                                          */
/* embperl_CompileDomTree						    */
/*                                                                          */
/* Compile root node and his childs					    */
/*                                                                          */
/* ------------------------------------------------------------------------ */


static int embperl_CompileDomTree (/*in*/  tReq *	  r,
				   /*in*/  tDomTree *	  pDomTree)



    {
    int rc ;
    int         bCheckpointPending = 0 ;
    tIndex      xDomTree = pDomTree -> xNdx ;


    pDomTree -> xCurrNode = 0 ;

    if ((rc = embperl_CompileNode (r, pDomTree, pDomTree -> xDocument, &bCheckpointPending)) != ok)
	return rc ;

    pDomTree = DomTree_self (xDomTree) ; /* addr may have changed */

    if (bCheckpointPending)
	{
	int l ;
	char buf [80] ;

	int nCheckpointArrayOffset = ArrayAdd (&pDomTree -> pCheckpoints, 1) ;
	pDomTree -> pCheckpoints[nCheckpointArrayOffset].xNode = -1 ;
	l = sprintf (buf, " _ep_cp(%d) ;\n", nCheckpointArrayOffset) ;
	StringAdd (r -> pProg, buf,	l) ;

	if (r -> bDebug & dbgCompile)
	    lprintf (r, "[%d]EPCOMP: #%d  Checkpoint\n", r -> nPid, -1) ;

	}
    
    return ok ;
    }



/* ------------------------------------------------------------------------ */
/*                                                                          */
/* embperl_Compile                                                          */
/*                                                                          */
/* Compile the whole document                                               */
/*                                                                          */
/* ------------------------------------------------------------------------ */

int embperl_Compile                 (/*in*/  tReq *	  r,
				     /*in*/  tIndex       xDomTree,
				     /*out*/ tIndex *     pxResultDomTree,
                                     /*out*/ SV * *       pProg) 


    {
    int rc ;
    tDomTree * pDomTree = DomTree_self (*pxResultDomTree = xDomTree) ;
    char *      sSourcefile = DomTree_selfFilename (pDomTree)  ;
    clock_t	cl1 = clock () ;
    clock_t	cl2  ;
    clock_t	cl3  ;
    clock_t	cl4  ;
    STRLEN      l ;
    SV *        pSV ;
    SV *        args[2] ;
    int         nStep = r -> Buf.pFile -> nFilesize / 4 ;
    if (nStep < 1024)
        nStep = 1024 ;
    else if (nStep > 4096)
        nStep = 4096 ;

    if (r -> bDebug & dbgCompile)
	lprintf (r, "[%d]EPCOMP: Start compiling %s DomTree = %d\n", r -> nPid, sSourcefile, xDomTree) ; 

    ChdirToSource (r, sSourcefile) ;    

    r -> nPhase  = phCompile ;

    r -> pProgRun = NULL ;
    r -> pProgDef = NULL ;

    StringNew (&r -> pProgRun, nStep ) ;
    StringNew (&r -> pProgDef, nStep ) ;
    r -> pProg = &r -> pProgRun ;

    pDomTree -> pSV = (SV *)newHV () ;
    if (pDomTree -> pCheckpoints)
	ArraySetSize (&pDomTree -> pCheckpoints, 0) ;
    else
	ArrayNew (&pDomTree -> pCheckpoints, 256, sizeof (tDomTreeCheckpoint)) ;
    ArrayAdd (&pDomTree -> pCheckpoints, 1) ;
    pDomTree -> pCheckpoints[0].xNode = 0 ;

    if ((rc = embperl_CompileDomTree (r, pDomTree)) != ok)
	{
        /*
        *ppSV = newSVpvf ("%s\t%s", r -> errdat1, r -> errdat2) ;
	SvUPGRADE (*ppSV, SVt_PVIV) ;
	SvIVX (*ppSV) = rc ;
	if (r -> xCurrDomTree)
	    {
	    DomTree_delete(DomTree_self(r -> xCurrDomTree)) ;
	    r -> xCurrDomTree = 0 ;
	    }
	*/
        StringFree (&r -> pProgRun) ;
	StringFree (&r -> pProgDef) ;
	ArrayFree (&pDomTree -> pCheckpoints) ;
	pDomTree -> pCheckpoints = NULL ;

	pDomTree = DomTree_self (xDomTree) ;
	DomTree_delete (pDomTree) ;
	*pxResultDomTree = 0 ;

	return rc ;
	}

    SvREFCNT_dec (pDomTree -> pSV) ;
    pDomTree -> pSV = NULL ;

    StringAdd (&r -> pProgRun, "", 1) ;
    StringAdd (&r -> pProgDef, "", 1) ;

    cl2 = clock () ;

    r -> nPhase  = phRunAfterCompile ;
    
    l = ArrayGetSize (r -> pProgDef) ;
    if (l > 1 && r -> bDebug & dbgCompile)
	lprintf (r, "[%d]EPCOMP: AfterCompileTimeCode:    %*.*s\n", r -> nPid, l, l, r -> pProgDef) ; 

    if (l > 1)
	{
	pSV = newSVpvf("package %s ; \n%*.*s", r -> Buf.sEvalPackage, l,l, r -> pProgDef) ;
	newSVpvf2(pSV) ;
	args[0] = r -> pReqSV ;
	args[1] = pDomTree -> pDomTreeSV ;
	if ((rc = EvalDirect (r, pSV, 0, args)) != ok)
	    LogError (r, rc) ;
	SvREFCNT_dec(pSV);
	}

    cl3 = clock () ;
    
    r -> nPhase  = phPerlCompile ;

    if (PERLDB_LINE)
	{ /* feed source to file gv (@/%_<filename) if we are running under the debugger */
	AV * pAV ;
	GV * pGVFile = gv_fetchfile (sSourcefile) ;
	AV * pDebugArray = GvAV (pGVFile) ;

	
	char * p = r -> Buf.pBuf ;
	char * end ;
	I32    i = 1 ;
	while (*p)
	    {
	    end = strchr (p, '\n') ;
	    if (end)
		{		
		SV * pLine  ;
		pLine = newSVpv (p, end - p + 1) ;
		SvUPGRADE (pLine, SVt_PVMG) ;
		av_store (pDebugArray, i++, pLine) ;
		p = end + 1 ;
		}
	    else if (p < r -> Buf.pEndPos)
		{
		SV * pLine  ;
		pLine = newSVpv (p, r -> Buf.pEndPos - p + 1) ;
		SvUPGRADE (pLine, SVt_PVMG) ;
		av_store (pDebugArray, i++, pLine) ;
		break ;
		}
	    }
	if (r -> bDebug)
	    lprintf (r, "Setup source code for interactive debugger\n") ;
	}    
    
    rc = EvalOnly (r, r -> pProgRun, pProg, G_SCALAR, EPMAINSUB) ;
	    
    StringFree (&r -> pProgRun) ;
    StringFree (&r -> pProgDef) ;

    if (rc != ok && xDomTree)
	{
	pDomTree = DomTree_self (xDomTree) ;
	if (pDomTree)
	    DomTree_delete (pDomTree) ;
	*pxResultDomTree = 0 ;
	}

    cl4 = clock () ;

#ifdef CLOCKS_PER_SEC
    if (r -> bDebug)
	{
	lprintf (r, "[%d]PERF: Compile Start Time:	    %d ms \n", r -> nPid, ((cl1 - r -> startclock) * 1000 / CLOCKS_PER_SEC)) ;
	lprintf (r, "[%d]PERF: Compile End Time:	    %d ms \n", r -> nPid, ((cl2 - r -> startclock) * 1000 / CLOCKS_PER_SEC)) ;
	lprintf (r, "[%d]PERF: After Compile Exec End Time: %d ms \n", r -> nPid, ((cl3 - r -> startclock) * 1000 / CLOCKS_PER_SEC)) ;
	lprintf (r, "[%d]PERF: Perl Compile End Time:	    %d ms \n", r -> nPid, ((cl4 - r -> startclock) * 1000 / CLOCKS_PER_SEC)) ;
	lprintf (r, "[%d]PERF: Compile Time:		    %d ms \n", r -> nPid, ((cl4 - cl1) * 1000 / CLOCKS_PER_SEC)) ;
	DomStats () ;
	}
#endif        

    return rc ;
    }


/* ------------------------------------------------------------------------ */
/*                                                                          */
/* embperl_Executer                                                         */
/*                                                                          */
/* ------------------------------------------------------------------------ */

static int embperl_Execute2         (/*in*/  tReq *	  r,
				     /*in*/  tIndex       xSrcDomTree,
                                     /*in*/  CV *         pCV,
				     /*in*/  tIndex  *    pResultDomTree)


    {
    int rc ;
    tDomTree * pCurrDomTree ;
    clock_t	cl1 = clock () ;
    clock_t	cl2 ;
    SV *        pSV ;
    char * sSubName  ;

    tainted         = 0 ;
    r -> xCurrDomTree = xSrcDomTree ;

    sSubName = r -> sSubName ;

    if (sSubName && !*sSubName)
	sSubName = NULL ;
    rc = ok ;
    cl1 = clock () ;
    

    r -> nPhase  = phRun ;

	
    r -> nCurrCheckpoint = 1 ;
    r -> nCurrRepeatLevel = 0 ;
    r -> xSourceDomTree = r -> xCurrDomTree ;
    if (!(r -> xCurrDomTree  = DomTree_clone (DomTree_self (xSrcDomTree), &pCurrDomTree, sSubName?1:0)))
	return 1 ;

    *pResultDomTree = r -> xCurrDomTree ;
    /* -> is done by cache management -> av_push (r -> pDomTreeAV, pCurrDomTree -> pDomTreeSV) ; */
    pCurrDomTree = DomTree_self (r -> xCurrDomTree) ; 
    ArrayNewZero (&pCurrDomTree -> pCheckpointStatus, ArrayGetSize (pCurrDomTree -> pCheckpoints), sizeof(tDomTreeCheckpointStatus)) ;

    if (pCV)
	{
	SV * args[2] ;
	STRLEN l ;
	SV * sDomTreeSV = newSVpvf ("%s::%s", r -> Buf.sEvalPackage, "_ep_DomTree") ;
	SV * pDomTreeSV = perl_get_sv (SvPV (sDomTreeSV, l), TRUE) ;
	IV xOldDomTree = 0 ;
	newSVpvf2(sDomTreeSV) ;
	
	if (SvIOK (pDomTreeSV))
	    xOldDomTree = SvIVX (pDomTreeSV) ;

	SvREFCNT_dec (sDomTreeSV) ;
	sv_setiv (pDomTreeSV, r -> xCurrDomTree) ;

    	av_push (r -> pCleanupAV, newRV_inc (pDomTreeSV)) ;
	
	args[0] = r -> pReqSV ;
	if (sSubName)
	    {
	    SV * pSVName = newSVpvf ("%s::_ep_sub_%s", r -> Buf.sEvalPackage, sSubName) ;
	    newSVpvf2(pSVName) ;
            pCurrDomTree -> xDocument = 0 ; /* set by first checkpoint */
	    rc = CallStoredCV (r, r -> pProgRun, (CV *)pSVName, 1, args, 0, &pSV) ;
	    if (pSVName)
		SvREFCNT_dec (pSVName) ;
	    if (pSV)
		SvREFCNT_dec (pSV) ;
	    }
	else
	    {
	    rc = CallStoredCV (r, r -> pProgRun, (CV *)pCV, 1, args, 0, &pSV) ;
	    if (pSV)
		SvREFCNT_dec (pSV) ;
	    }

	pCurrDomTree = DomTree_self (r -> xCurrDomTree) ; /* relookup DomTree in case it has moved */
	cl2 = clock () ;
#ifdef CLOCKS_PER_SEC
	if (r -> bDebug)
	    {
	    lprintf (r, "[%d]PERF: Run Start Time: %d ms \n", r -> nPid, ((cl1 - r -> startclock) * 1000 / CLOCKS_PER_SEC)) ;
	    lprintf (r, "[%d]PERF: Run End Time:   %d ms \n", r -> nPid, ((cl2 - r -> startclock) * 1000 / CLOCKS_PER_SEC)) ;
	    lprintf (r, "[%d]PERF: Run Time:       %d ms \n", r -> nPid, ((cl2 - cl1) * 1000 / CLOCKS_PER_SEC)) ;
	    DomStats () ;
	    }
#endif    

	sv_setiv (pDomTreeSV, xOldDomTree) ;
	}

    ArrayFree (&pCurrDomTree -> pCheckpointStatus) ;

    if (rc != ok && rc != rcEvalErr)
        return rc ;

    r -> nPhase  = phTerm ;
    
    return ok ;
    }

/* ------------------------------------------------------------------------ */
/*                                                                          */
/* embperl_Execute                                                          */
/*                                                                          */
/* ------------------------------------------------------------------------ */

int embperl_Execute	            (/*in*/  tReq *	  r,
				     /*in*/  tIndex       xSrcDomTree,
                                     /*in*/  CV *         pCV,
				     /*in*/  tIndex  *    pResultDomTree)


    {
    int	    rc  = ok ;
    /* char *  sSourcefile = DomTree_filename (xSrcDomTree)  ;*/
    char *  sSourcefile = r -> Buf.pFile -> sSourcefile  ;
    
    tainted         = 0 ;

    if (!r -> bError)
	{
	/* --- change working directory --- */
        ChdirToSource (r, sSourcefile) ;    

        rc = embperl_Execute2 (r, xSrcDomTree, pCV, pResultDomTree) ;


	/* --- restore working directory --- */
	if (r -> sResetDir[0])
	    {
#ifdef WIN32
   	    _chdrive (r -> nResetDrive) ;
#endif
	    chdir (r -> sResetDir) ;
	    strcpy (r->sCWD,r -> sResetDir) ;
	    r -> sResetDir[0] = '\0' ;
            }

        }
    else
        *pResultDomTree = 0 ;

    r -> nPhase  = phTerm ;
    
    return rc ;
    }

/* ------------------------------------------------------------------------ */
/*                                                                          */
/* embperl_CompileSetupVar                                                  */
/*                                                                          */
/* Looks for vars/subs inside compiled document                             */
/*                                                                          */
/* ------------------------------------------------------------------------ */

int embperl_PreExecute	            (/*in*/  tReq *	  r,
				     /*in*/  tCacheItem * pCache)


    {
    STRLEN      l ;
    SV *        pSV ;
    CV *        pCV ;
    SV *        pSVVar ;
    
    pSV = newSVpvf("%s::EXPIRES", r -> Buf.sEvalPackage) ;
    newSVpvf2(pSV) ;
    pCV = perl_get_cv (SvPV(pSV, l), 0) ;
    if (pCV)
	{
	SvREFCNT_dec (pCache -> pExpiresCV) ;
	pCache -> pExpiresCV = pCV ;
	SvREFCNT_inc (pCV) ;
	}    
    SvREFCNT_dec(pSV);
    
    pSV = newSVpvf("%s::EXPIRES", r -> Buf.sEvalPackage) ;
    newSVpvf2(pSV) ;
    pSVVar = perl_get_sv (SvPV(pSV, l), 0) ;
    if (pSVVar)
	{
	pCache -> nExpiresInTime = SvUV (pSVVar) ;
	}    
    SvREFCNT_dec(pSV);
    
    /*
    pSV = newSVpvf("%s::CACHE_KEY", r -> Buf.sEvalPackage) ;
    newSVpvf2(pSV) ;
    pCV = perl_get_cv (SvPV(pSV, l), 0) ;
    if (pCV)
	{
	SvREFCNT_dec (pProcessor -> pCacheKeyCV) ;
	pProcessor -> pCacheKeyCV = pCV ;
	SvREFCNT_inc (pCV) ;
	}    
    SvREFCNT_dec(pSV);
    
    pSV = newSVpvf("%s::CACHE_KEY", r -> Buf.sEvalPackage) ;
    newSVpvf2(pSV) ;
    pSVVar = perl_get_sv (SvPV(pSV, l), 0) ;
    if (pSVVar)
	{
	pProcessor -> sCacheKey = SvPV (pSVVar, l) ;
	}    
    SvREFCNT_dec(pSV);

    pSV = newSVpvf("%s::CACHE_KEY_OPTIONS", r -> Buf.sEvalPackage) ;
    newSVpvf2(pSV) ;
    pSVVar = perl_get_sv (SvPV(pSV, l), 0) ;
    if (pSVVar)
	{
	pProcessor -> bCacheKeyOptions = SvIV (pSVVar) ;
	}    
    SvREFCNT_dec(pSV);
    */

    return ok ;
    }

/* ------------------------------------------------------------------------ */
/*                                                                          */
/* embperl_CompileProcessorSetupVar                                         */
/*                                                                          */
/* Looks for vars/subs inside compiled document                             */
/*                                                                          */
/* in   pProcessor  Processor data                                          */
/*                                                                          */
/* ------------------------------------------------------------------------ */

int embperl_PreExecuteProcessor	    (/*in*/  tReq *	  r,
				     /*in*/  tProcessor * pProcessor,
				     /*in*/  tDomTree **  pDomTree,
				     /*in*/  SV **        ppPreCompResult,
				     /*in*/  SV **        ppCompResult)


    {
    STRLEN      l ;
    SV *        pSV ;
    CV *        pCV ;
    SV *        pSVVar ;
    
    pSV = newSVpvf("%s::EXPIRES", r -> Buf.sEvalPackage) ;
    newSVpvf2(pSV) ;
    pCV = perl_get_cv (SvPV(pSV, l), 0) ;
    if (pCV)
	{
	SvREFCNT_dec (pProcessor -> pOutputExpiresCV) ;
	pProcessor -> pOutputExpiresCV = pCV ;
	SvREFCNT_inc (pCV) ;
	}    
    SvREFCNT_dec(pSV);
    
    pSV = newSVpvf("%s::EXPIRES", r -> Buf.sEvalPackage) ;
    newSVpvf2(pSV) ;
    pSVVar = perl_get_sv (SvPV(pSV, l), 0) ;
    if (pSVVar)
	{
	pProcessor -> nOutputExpiresIn = SvNV (pSVVar) ;
	}    
    SvREFCNT_dec(pSV);
    
    pSV = newSVpvf("%s::CACHE_KEY", r -> Buf.sEvalPackage) ;
    newSVpvf2(pSV) ;
    pCV = perl_get_cv (SvPV(pSV, l), 0) ;
    if (pCV)
	{
	SvREFCNT_dec (pProcessor -> pCacheKeyCV) ;
	pProcessor -> pCacheKeyCV = pCV ;
	SvREFCNT_inc (pCV) ;
	}    
    SvREFCNT_dec(pSV);
    
    pSV = newSVpvf("%s::CACHE_KEY", r -> Buf.sEvalPackage) ;
    newSVpvf2(pSV) ;
    pSVVar = perl_get_sv (SvPV(pSV, l), 0) ;
    if (pSVVar)
	{
	pProcessor -> sCacheKey = SvPV (pSVVar, l) ;
	}    
    SvREFCNT_dec(pSV);

    pSV = newSVpvf("%s::CACHE_KEY_OPTIONS", r -> Buf.sEvalPackage) ;
    newSVpvf2(pSV) ;
    pSVVar = perl_get_sv (SvPV(pSV, l), 0) ;
    if (pSVVar)
	{
	pProcessor -> bCacheKeyOptions = SvIV (pSVVar) ;
	}    
    SvREFCNT_dec(pSV);

    return ok ;
    }


/* ------------------------------------------------------------------------ */
/*                                                                          */
/* embperl_CompileProcessor                                                 */
/*                                                                          */
/* Compile the whole document                                               */
/*                                                                          */
/* in   pProcessor  Processor data                                          */
/*                                                                          */
/* ------------------------------------------------------------------------ */

int embperl_CompileProcessor        (/*in*/  tReq *	  r,
				     /*in*/  tProcessor * pProcessor,
				     /*in*/  tDomTree **  ppDomTree,
				     /*in*/  SV **        ppPreCompResult,
				     /*out*/ SV **        ppCompResult)


    {
    int rc ;
    tDomTree * pDomTree = *ppDomTree ;
    clock_t	cl1 = clock () ;
    clock_t	cl2  ;
    clock_t	cl3  ;
    clock_t	cl4  ;
    STRLEN      l ;
    SV *        pSV ;
    SV *        args[2] ;
    SV **       ppLastResult = (ppCompResult && *ppCompResult?ppCompResult:ppPreCompResult) ;
    
    r -> nPhase  = phCompile ;

    if (ppLastResult && *ppLastResult && SvTYPE (*ppLastResult) == SVt_PVCV)
	{ /* Get already parsed Dom Tree */
        if (ppCompResult != ppLastResult)
            {
	    AssignSVPtr (ppCompResult,*ppLastResult) ;
	    SvREFCNT_inc (*ppCompResult) ; 
	    }

        return ok ;
        }

    r -> pProgRun = NULL ;
    r -> pProgDef = NULL ;

    StringNew (&r -> pProgRun, r -> Buf.pFile -> nFilesize / 4) ;
    StringNew (&r -> pProgDef, r -> Buf.pFile -> nFilesize / 4) ;
    r -> pProg = &r -> pProgRun ;

    pDomTree -> pSV = (SV *)newHV () ;
    ArrayNew (&pDomTree -> pCheckpoints, 256, sizeof (tDomTreeCheckpoint)) ;
    ArrayAdd (&pDomTree -> pCheckpoints, 1) ;
    pDomTree -> pCheckpoints[0].xNode = 0 ;

    if ((rc = embperl_CompileDomTree (r, pDomTree)) != ok)
	{
        /*
        *ppSV = newSVpvf ("%s\t%s", r -> errdat1, r -> errdat2) ;
	newSVpvf2(*ppSV) ;
	SvUPGRADE (*ppSV, SVt_PVIV) ;
	SvIVX (*ppSV) = rc ;
	if (r -> xCurrDomTree)
	    {
	    DomTree_delete(DomTree_self(r -> xCurrDomTree)) ;
	    r -> xCurrDomTree = 0 ;
	    }
	*/
        StringFree (&r -> pProgRun) ;
	StringFree (&r -> pProgDef) ;
	
	return rc ;
	}

    SvREFCNT_dec (pDomTree -> pSV) ;
    pDomTree -> pSV = NULL ;

    StringAdd (&r -> pProgRun, "", 1) ;
    StringAdd (&r -> pProgDef, "", 1) ;

    cl2 = clock () ;

    r -> nPhase  = phRunAfterCompile ;
    
    l = ArrayGetSize (r -> pProgDef) ;
    if (l && r -> bDebug & dbgCompile)
	lprintf (r, "[%d]EPCOMP: AfterCompileTimeCode:    %*.*s\n", r -> nPid, l, l, r -> pProgDef) ; 

    /* pSV = newSVpvf("package %s ; \nmy ($_ep_req, $_ep_DomTree) = @_;\n%*.*s", r -> Buf.sEvalPackage, l,l, r -> pProgDef) ; */
    pSV = newSVpvf("package %s ; \n%*.*s", r -> Buf.sEvalPackage, l,l, r -> pProgDef) ;
    newSVpvf2(pSV) ;
    args[0] = r -> pReqSV ;
    args[1] = pDomTree -> pDomTreeSV ;
    if ((rc = EvalDirect (r, pSV, 0, args)) != ok)
	LogError (r, rc) ;
    SvREFCNT_dec(pSV);

    cl3 = clock () ;
    
    r -> nPhase  = phPerlCompile ;

    if (PERLDB_LINE)
	{ /* feed source to file gv (@/%_<filename) if we are running under the debugger */
	AV * pAV ;
	GV * pGVFile = gv_fetchfile (r -> Buf.pFile -> sSourcefile) ;
	AV * pDebugArray = GvAV (pGVFile) ;

	
	char * p = r -> Buf.pBuf ;
	char * end ;
	I32    i = 1 ;
	while (*p)
	    {
	    end = strchr (p, '\n') ;
	    if (end)
		{		
		SV * pLine  ;
		pLine = newSVpv (p, end - p + 1) ;
		SvUPGRADE (pLine, SVt_PVMG) ;
		av_store (pDebugArray, i++, pLine) ;
		p = end + 1 ;
		}
	    else if (p < r -> Buf.pEndPos)
		{
		SV * pLine  ;
		pLine = newSVpv (p, r -> Buf.pEndPos - p + 1) ;
		SvUPGRADE (pLine, SVt_PVMG) ;
		av_store (pDebugArray, i++, pLine) ;
		break ;
		}
	    }
	if (r -> bDebug)
	    lprintf (r, "Setup source code for interactive debugger\n") ;
	}    
    
    rc = EvalOnly (r, r -> pProgRun, ppCompResult, G_SCALAR, EPMAINSUB) ;
	    
    StringFree (&r -> pProgRun) ;
    StringFree (&r -> pProgDef) ;

    cl4 = clock () ;

#ifdef CLOCKS_PER_SEC
    if (r -> bDebug)
	{
	lprintf (r, "[%d]PERF: Compile Start Time:	    %d ms \n", r -> nPid, ((cl1 - r -> startclock) * 1000 / CLOCKS_PER_SEC)) ;
	lprintf (r, "[%d]PERF: Compile End Time:	    %d ms \n", r -> nPid, ((cl2 - r -> startclock) * 1000 / CLOCKS_PER_SEC)) ;
	lprintf (r, "[%d]PERF: After Compile Exec End Time: %d ms \n", r -> nPid, ((cl3 - r -> startclock) * 1000 / CLOCKS_PER_SEC)) ;
	lprintf (r, "[%d]PERF: Perl Compile End Time:	    %d ms \n", r -> nPid, ((cl4 - r -> startclock) * 1000 / CLOCKS_PER_SEC)) ;
	lprintf (r, "[%d]PERF: Compile Time:		    %d ms \n", r -> nPid, ((cl4 - cl1) * 1000 / CLOCKS_PER_SEC)) ;
	DomStats () ;
	}
#endif        

    return rc ;
    }

/* ------------------------------------------------------------------------ */
/*                                                                          */
/* embperl_ParseProcessor                                                   */
/*                                                                          */
/* Parse the whole document                                                 */
/*                                                                          */
/* in   pProcessor  Processor data                                          */
/*                                                                          */
/* ------------------------------------------------------------------------ */

int embperl_ParseProcessor          (/*in*/  tReq *	  r,
				     /*in*/  tProcessor * pProcessor,
				     /*in*/  tDomTree **  ppDomTree,
				     /*in*/  SV **        ppPreCompResult,
				     /*out*/ SV **        ppCompResult)


    {
    int rc ;
    tDomTree * pDomTree ;
    SV **       ppLastResult = (ppCompResult && *ppCompResult?ppCompResult:ppPreCompResult) ;
    
    r -> nPhase  = phParse ;

    if (ppLastResult && *ppLastResult && SvIOKp (*ppLastResult))
        { /* Get already parsed Dom Tree */
        pDomTree = DomTree_selfSV(*ppLastResult) ;
        if (ppCompResult != ppLastResult)
            {
	    AssignSVPtr (ppCompResult,*ppLastResult) ;
    	    SvREFCNT_inc (*ppCompResult) ; 
	    }
        }
    else
        {
	/* if ((rc = ReadInputFile (r)) != ok) */
	/*	return rc ; */
        if ((rc = ParseFile (r)) != ok)
	    {
	    return rc ;
	    }
    
        pDomTree      = DomTree_self (r -> xCurrDomTree) ;
        AssignSVPtr (ppCompResult,SV_DomTree_self (pDomTree)) ;
	/* SvREFCNT_inc (*ppCompResult) ; */
        /* add timestamp */
	SvUPGRADE (*ppCompResult, SVt_PVNV) ;
	SvNVX (*ppCompResult) = time (NULL) ;
	SvNOK_on (*ppCompResult)  ;
        

        }

    *ppDomTree    = pDomTree ;
    
    return ok ;
    }


/* ------------------------------------------------------------------------ */
/*                                                                          */
/* embperl_ExecuteSubStart                                                  */
/*                                                                          */
/* Setup the start of a sub                                                 */
/*                                                                          */
/* in   pDomTreeSV	SV which holds the DomTree in the current package   */
/* in   xDomTree	Source DomTree                                      */
/* in   pSaveAV		Array to save some values                           */
/*                                                                          */
/* ------------------------------------------------------------------------ */


int embperl_ExecuteSubStart         (/*in*/  tReq *	  r,
				     /*in*/  SV *         pDomTreeSV,
				     /*in*/  tIndex       xDomTree,
				     /*in*/  AV *         pSaveAV)

    {
    tIndex xOrgDomTree = -1  ;
    tIndex xOldDomTree  ;
    tDomTree * pDomTree ;
    tDomTree * pCurrDomTree ;

 #if 0
    if (SvIOK (pDomTreeSV))
	if (xOrgDomTree = SvIVX (pDomTreeSV))
	    {
	    if (r -> xCurrDomTree == xOrgDomTree)
		return xOrgDomTree ;

	    /*
	    av_push (pSaveAV, newSViv (r -> xCurrDomTree)) ;
	    av_push (pSaveAV, newSViv (r -> xCurrNode)) ;
	    av_push (pSaveAV, newSViv (ArrayGetSize (DomTree_self (xOrgDomTree) -> pOrder))) ;

	    if (r -> bDebug & dbgCompile)
		lprintf (r, "[%d]SUB: Enter from DomTree=%d into DomTree=%d, Source DomTree=%d \n", r -> nPid, r -> xCurrDomTree, xOrgDomTree, xDomTree) ; 
	    return r -> xCurrDomTree = xOrgDomTree ;*/ /* DomTree already cloned */
	    }
#endif

    av_push (pSaveAV, newSViv (r -> xCurrDomTree)) ;
    av_push (pSaveAV, newSViv (r -> xCurrNode)) ;
    av_push (pSaveAV, newSViv (r -> nCurrRepeatLevel)) ;
    av_push (pSaveAV, newSViv (r -> nCurrCheckpoint)) ;

    pDomTree = DomTree_self (xDomTree) ;

    xOldDomTree = r -> xCurrDomTree ;

    if (!(r -> xCurrDomTree  = DomTree_clone (pDomTree, &pCurrDomTree, 1)))
	    return 0 ;
    ArrayNewZero (&pCurrDomTree -> pCheckpointStatus, ArrayGetSize (pCurrDomTree -> pCheckpoints), sizeof(tDomTreeCheckpointStatus)) ;
    r -> nCurrCheckpoint = 1 ;
    r -> nCurrRepeatLevel = 0 ;
    pCurrDomTree -> xDocument = 0 ; /* set by first checkpoint */
    
    av_push (r -> pDomTreeAV, pCurrDomTree -> pDomTreeSV) ;
    av_push (r -> pCleanupAV, newRV_inc (pDomTreeSV)) ;

    sv_setiv (pDomTreeSV, r -> xCurrDomTree) ;

    if (r -> bDebug & dbgRun)
	lprintf (r, "[%d]SUB: Enter from DomTree=%d into new DomTree=%d, Source DomTree=%d (org=%d)\n", r -> nPid, xOldDomTree, r -> xCurrDomTree, xDomTree, xOrgDomTree) ; 

    return r -> xCurrDomTree ;
    }

/* ------------------------------------------------------------------------ */
/*                                                                          */
/* embperl_ExecuteSubEnd                                                    */
/*                                                                          */
/* End a sub                                                                */
/*                                                                          */
/* in   pSaveAV		Array to save some values                           */
/*                                                                          */
/* ------------------------------------------------------------------------ */


int embperl_ExecuteSubEnd           (/*in*/  tReq *	  r,
				     /*in*/  SV *         pDomTreeSV,
				     /*in*/  AV *         pSaveAV)

    {
    tIndex xSubDomTree = r -> xCurrDomTree ;
    tIndex xDocFraq ;
    tDomTree * pCallerDomTree  ;
    tDomTree * pSubDomTree = DomTree_self (xSubDomTree) ;

    if (AvFILL (pSaveAV) < 1)
	return ok ;
    
    ArrayFree (&pSubDomTree -> pCheckpointStatus) ;
    /* DomTree_checkpoint (r, -1) ; */

    r -> xCurrDomTree = SvIV (* av_fetch (pSaveAV, 0, 0)) ;
    r -> xCurrNode    = SvIV (* av_fetch (pSaveAV, 1, 0)) ;
    r -> nCurrRepeatLevel = SvIV (* av_fetch (pSaveAV, 2, 0)) ;
    r -> nCurrCheckpoint = SvIV (* av_fetch (pSaveAV, 3, 0)) ;

    pCallerDomTree = DomTree_self (r -> xCurrDomTree) ;
    /* xDocFraq = Node_replaceChildWithNode (pSubDomTree, pSubDomTree -> xDocument, pCallerDomTree, r -> xCurrNode) ; */
    r -> xCurrNode = xDocFraq = Node_insertAfter (pSubDomTree, pSubDomTree -> xDocument, 0, pCallerDomTree, r -> xCurrNode, r -> nCurrRepeatLevel) ;

    /* Element_selfSetAttribut (pCallerDomTree, Node_self (pCallerDomTree, xDocFraq), NULL, xOrderIndexAttr, NULL, nOrderNdx, 0) ; */

    if (r -> bDebug & dbgRun)
	lprintf (r, "[%d]SUB: Leave from DomTree=%d back to DomTree=%d\n", r -> nPid, xSubDomTree, r -> xCurrDomTree) ; 

    return ok ;
    }


/* ------------------------------------------------------------------------ */
/*                                                                          */
/* embperl_Executer                                                         */
/*                                                                          */
/* Parse the whole document                                                 */
/*                                                                          */
/* in   pProcessor  Processor data                                          */
/*                                                                          */
/* ------------------------------------------------------------------------ */

int embperl_ExecuteProcessor	    (/*in*/  tReq *	  r,
				     /*in*/  tProcessor * pProcessor,
				     /*in*/  tDomTree **  pDomTree,
				     /*in*/  SV **        ppPreCompResult,
				     /*in*/  SV **        ppCompResult,
				     /*out*/ SV **        ppExecResult)


    {
    int rc ;
    CV *  pCV = (CV *)(ppCompResult && *ppCompResult?*ppCompResult:*ppPreCompResult) ;
    tDomTree * pCurrDomTree ;
    clock_t	cl1 = clock () ;
    clock_t	cl2 ;
    SV *        pSV ;
    
    if (ppExecResult && *ppExecResult && SvIOKp (*ppExecResult))
	{
	if (r -> bDebug & dbgCache)
	    lprintf (r, "[%d]CACHE: Result for file %s, processor %s taken form cache, not executed (DomTree #%d)\n", r -> nPid, r -> Buf.pFile -> sSourcefile, pProcessor -> sName, DomTree_SV (*ppExecResult)) ; 
        return ok ;
	}

    if (!r -> bError)
	{
	char * sSubName = r -> sSubName ;

	if (sSubName && !*sSubName)
	    sSubName = NULL ;
	rc = ok ;
	cl1 = clock () ;
	

    	r -> nPhase  = phRun ;

	    
        r -> nCurrCheckpoint = 1 ;
        r -> nCurrRepeatLevel = 0 ;
	r -> xSourceDomTree = r -> xCurrDomTree ;
	if (!(r -> xCurrDomTree  = DomTree_clone (*pDomTree, &pCurrDomTree, sSubName?1:0)))
	    return 1 ;

	av_push (r -> pDomTreeAV, pCurrDomTree -> pDomTreeSV) ;
	pCurrDomTree = DomTree_self (r -> xCurrDomTree) ; 
	ArrayNewZero (&pCurrDomTree -> pCheckpointStatus, ArrayGetSize (pCurrDomTree -> pCheckpoints), sizeof(tDomTreeCheckpointStatus)) ;

	if (pCV)
	    {
	    SV * args[2] ;
	    STRLEN l ;
	    SV * sDomTreeSV = newSVpvf ("%s::%s", r -> Buf.sEvalPackage, "_ep_DomTree") ;
	    SV * pDomTreeSV = perl_get_sv (SvPV (sDomTreeSV, l), TRUE) ;
	    IV xOldDomTree = 0 ;
	    newSVpvf2(sDomTreeSV) ;
	    
	    if (SvIOK (pDomTreeSV))
		xOldDomTree = SvIVX (pDomTreeSV) ;

	    SvREFCNT_dec (sDomTreeSV) ;
	    sv_setiv (pDomTreeSV, r -> xCurrDomTree) ;

    	    av_push (r -> pCleanupAV, newRV_inc (pDomTreeSV)) ;
	    
	    args[0] = r -> pReqSV ;
	    if (sSubName)
		{
		SV * pSVName = newSVpvf ("%s::_ep_sub_%s", r -> Buf.sEvalPackage, sSubName) ;
		newSVpvf2(pSVName) ;
                pCurrDomTree -> xDocument = 0 ; /* set by first checkpoint */
		rc = CallStoredCV (r, r -> pProgRun, (CV *)pSVName, 1, args, 0, &pSV) ;
		if (pSVName)
		    SvREFCNT_dec (pSVName) ;
		if (pSV)
		    SvREFCNT_dec (pSV) ;
		}
	    else
		{
		rc = CallStoredCV (r, r -> pProgRun, (CV *)pCV, 1, args, 0, &pSV) ;
		if (pSV)
		    SvREFCNT_dec (pSV) ;
		}

	    pCurrDomTree = DomTree_self (r -> xCurrDomTree) ; /* relookup DomTree in case it has moved */
	    cl2 = clock () ;
    #ifdef CLOCKS_PER_SEC
	    if (r -> bDebug)
	        {
	        lprintf (r, "[%d]PERF: Run Start Time: %d ms \n", r -> nPid, ((cl1 - r -> startclock) * 1000 / CLOCKS_PER_SEC)) ;
	        lprintf (r, "[%d]PERF: Run End Time:   %d ms \n", r -> nPid, ((cl2 - r -> startclock) * 1000 / CLOCKS_PER_SEC)) ;
	        lprintf (r, "[%d]PERF: Run Time:       %d ms \n", r -> nPid, ((cl2 - cl1) * 1000 / CLOCKS_PER_SEC)) ;
	        DomStats () ;
		}
    #endif    

	    sv_setiv (pDomTreeSV, xOldDomTree) ;
	    }

	ArrayFree (&pCurrDomTree -> pCheckpointStatus) ;

        if (rc != ok && rc != rcEvalErr)
            return rc ;

	*pDomTree = pCurrDomTree ;
	AssignSVPtr (ppExecResult, SV_DomTree_self (pCurrDomTree)) ;
        SvREFCNT_inc (*ppExecResult) ;
        /* add timestamp */
	SvUPGRADE (*ppExecResult, SVt_PVNV) ;
	SvNVX (*ppExecResult) = time (NULL) ;
	SvNOK_on (*ppExecResult)  ;
        }

    r -> nPhase  = phTerm ;
    
    return ok ;
    }

/* ------------------------------------------------------------------------ */
/*                                                                          */
/* embperl_GetFromCache                                                     */
/*                                                                          */
/* See if this step is already in the cache and is valid                    */
/*                                                                          */
/* in   pProcessor      Processor to check for                              */
/*      cType           Type with should checked                            */
/*      nExpiresIn      0 imideately                                        */
/*                      -1 never (only if preceeding steps are expires)     */
/*                      <n> second                                          */
/*      pExiresCV       Exipres when CV return true                         */
/*      bForceExpire    When set to true entry is unconditional expired     */
/* out  ppSV            Cached value (or undef to store new value)          */
/*                                                                          */
/* ------------------------------------------------------------------------ */


int embperl_GetFromCache (/*in*/  tReq *	r,
                          /*in*/  tProcessor *  pProcessor,
                          /*in*/  char          cType,
                          /*in*/  const char *  sStepName,
                          /*in*/  double        nExpiresIn,
                          /*in*/  CV *          pExpiresCV,
                          /*in*/  int *         bForceExpire,  
                          /*i/o*/ char * *      ppCVKey,
			  /*out*/ SV * * *      pppSV)


    {
    int  rc ;
    char * sKey ;
    STRLEN  nKey ;
    char * pCVKey = "";
    char * pPathInfoKey ;
    char * pQueryInfoKey ;
    SV   * * ppSV ;
    SV   * pSVKey ;
    STRLEN      l ;
    int	 bOpt = cType == 'P'?0:pProcessor -> bCacheKeyOptions ;

    if (nExpiresIn == 0 && !pExpiresCV)
        {
        /* *pppSV = (SV**)_malloc (r, sizeof (SV *)) ; */
        **pppSV = NULL ;
        if (r -> bDebug & dbgCache)
	    lprintf (r, "[%d]CACHE: File: '%s'  Processor: '%s'  Step: '%s'  Type: '%c' not cached\n", r -> nPid, r -> Buf.pFile -> sSourcefile, pProcessor -> sName, sStepName, cType) ; 
        return ok ;
        }

    if ((bOpt & ckoptCarryOver) && *ppCVKey)
	{
	pCVKey = *ppCVKey ;
	}
    else
	{
	if (pProcessor -> pCacheKeyCV)
	    {
	    SV * pRet ;

	    if ((rc = CallCV (r, "CacheKey", pProcessor -> pCacheKeyCV, 0, &pRet)) != ok)
		return rc ;

	    if (pRet && SvOK(pRet))
		*ppCVKey = pCVKey = SvPV (pRet, l) ;
	    }
	}
    
    if ((bOpt & ckoptPathInfo) && r -> sPathInfo)
	pPathInfoKey = r -> sPathInfo ;
    else	
	pPathInfoKey = "" ;

    if ((bOpt & ckoptQueryInfo) && r -> sQueryInfo)
	pQueryInfoKey = r -> sQueryInfo ;
    else	
	pQueryInfoKey = "" ;

  
    pSVKey = newSVpvf("%c-%d-%s-%s-%s-%s", cType, pProcessor -> nProcessorNo, pProcessor -> sCacheKey, pCVKey, pPathInfoKey, pQueryInfoKey) ;
    newSVpvf2(pSVKey) ;
    sKey = SvPV (pSVKey, nKey);
    if (r -> bDebug & dbgCache)
        lprintf (r, "[%d]CACHE: File: '%s'  Processor: '%s'  Step: '%s' gives Key: '%s'\n", r -> nPid, r -> Buf.pFile -> sSourcefile, pProcessor -> sName,  sStepName, sKey) ; 
    *pppSV = ppSV = hv_fetch(r -> Buf.pFile -> pCacheHash, sKey, nKey, 1) ;  
    SvREFCNT_dec(pSVKey);
    if (ppSV == NULL || *ppSV == NULL)
        return rcHashError ;

    if (*bForceExpire)
        { /*  Expire the entry */
        sv_setsv (*ppSV, &sv_undef) ;
        if (r -> bDebug & dbgCache)
            lprintf (r, "[%d]CACHE: Expired because entry depends other expired entry\n", r -> nPid) ; 
        }
    else if (nExpiresIn > 0)
        {
        double nTimestamp = SvNOKp(*ppSV)?SvNVX(*ppSV):0 ;

        if (nTimestamp + nExpiresIn <= time (NULL))
            { /* Expire the entry */
            sv_setsv (*ppSV, &sv_undef) ;
            *bForceExpire = 1 ;
            if (r -> bDebug & dbgCache)
                lprintf (r, "[%d]CACHE: Expired because of timeout (%f sec)\n", r -> nPid, nExpiresIn) ; 
            }

        }

        if (*ppSV != NULL)
/* ###    lprintf (r, "[%d]CACHE: SvTYPE (*ppSV) = %d\n", r -> nPid, SvTYPE (*ppSV)) ; */ 
    if (*ppSV != NULL && SvTYPE (*ppSV) == SVt_PV)
        {
        strncpy (r -> errdat1, SvPV(*ppSV, l), sizeof (r -> errdat1) - 1) ; 
        LogError (r, rcEvalErr) ;
        return rcEvalErr ;
        }
    else if (*ppSV != NULL && SvTYPE (*ppSV) == SVt_PVIV)
        {
        char * s1 = SvPV(*ppSV, l) ;
	char * s2 = strchr (s1, '\t') ;
	int    l1 ;
	if (s2)
	    {
	    l1 = s2 - s1 ;
	    if (l1 > sizeof (r -> errdat1) - 1)
		l1 = sizeof (r -> errdat1) - 1 ;
	    
	    strncpy (r -> errdat1, s1, l1) ; 
	    r -> errdat1[l1] = '\0' ;
	    strncpy (r -> errdat2, s2+1, sizeof (r -> errdat2) - 1) ; 
	    }
	else
	    strncpy (r -> errdat1, s1, sizeof (r -> errdat1) - 1) ; 

	r -> Buf.pCurrPos = NULL ;
	r -> Buf.nSourceline = 0 ;
        return SvIVX(*ppSV) ;
        }
    else if (pExpiresCV)
        {
        SV * pRet ;

        if ((rc = CallCV (r, "Expired?", pExpiresCV, 0, &pRet)) != ok)
            return rc ;

        if (pRet && SvTRUE(pRet))
            { /* Expire the entry */
            sv_setsv (*ppSV, &sv_undef) ;
            *bForceExpire = 1 ;
            if (r -> bDebug & dbgCache)
                lprintf (r, "[%d]CACHE: Expired because Expirey sub returned TRUE\n", r -> nPid) ; 
            }
        }

    return ok ;
    }




/* ------------------------------------------------------------------------ */
/*                                                                          */
/* embperl_CompileDocument                                                  */
/*                                                                          */
/* Compile the whole document                                               */
/*                                                                          */
/* ------------------------------------------------------------------------ */


int embperl_CompileDocument (/*i/o*/ register req * r,
			     /*in*/  tProcessor   * pFirstProcessor)


    {
    int		rc ;
    SV *	pSV ;
    SV **	ppSV ;
    tNode	xNode ;
    tDomTree *	pDomTree ;
    tDomTree *	pCurrDomTree ;
    clock_t	cl1 = clock () ;
    clock_t	cl2  ;
    clock_t	cl3  ;
    clock_t	cl4  ;
    tProcessor  * pProcessor = NULL ;
    int         bForceExpire  ;
    int         bForceExpirePre  ;
    char *	pCVKey = NULL ;

    tainted = 0 ;
    cl2 = clock () ;
    
    pProcessor = pFirstProcessor ;
    bForceExpirePre = 0 ;
    pDomTree   = DomTree_self (r -> xCurrDomTree) ;
    while (pProcessor && !r -> bError)
        {
        if (pProcessor -> pPreCompiler)
	    {
	    SV *  pTempSV = NULL ;
	    ppSV = &pTempSV ;

	    if ((rc = embperl_GetFromCache (r, pProcessor, 'P', "Precompiler", -1, NULL, &bForceExpirePre, &pCVKey, &ppSV)) != ok)
		return rc ;

	    if ((rc = (*pProcessor -> pPreCompiler)(r, pProcessor, &pDomTree, NULL, ppSV)) != ok)
		{
		if (!r -> bError)
                    {
                    *ppSV = newSVpvf ("%s\t%s", r -> errdat1, r -> errdat2) ;
		    newSVpvf2(*ppSV) ;
		    SvUPGRADE (*ppSV, SVt_PVIV) ;
                    SvIVX (*ppSV) = rc ;
/* ###     lprintf (r, "[%d]temp: SvTYPE (*ppSV) = %d\n", r -> nPid, SvTYPE (*ppSV)) ; */
                    }
                /*
		if (r -> xCurrDomTree)
		    {
		    DomTree_delete(DomTree_self(r -> xCurrDomTree)) ;
		    r -> xCurrDomTree = 0 ;
		    }
		*/
		if (pTempSV)
		    {
/* ###     lprintf (r, "[%d]temp: SvTYPE (*ppSV) = %d\n", r -> nPid, SvTYPE (pTempSV)) ; */

		    SvREFCNT_dec (pTempSV) ;
		    }
		return rc ;
		}
	    if (pTempSV)
		SvREFCNT_dec (pTempSV) ;
	    }
	pProcessor = pProcessor -> pNext ;
        }

    if (!r -> bError && !r -> pImportStash)
        {

	pProcessor = pFirstProcessor ;
        bForceExpire = 0 ;
        bForceExpirePre = 0 ;
        while (pProcessor && !r -> bError)
            {
	    SV *  pPreCompResult    = NULL ;
	    SV *  pCompResult	    = NULL ;
	    SV *  pExecResult	    = NULL ;
	    SV ** ppPreCompResult   = &pPreCompResult ;
	    SV ** ppCompResult	    = &pCompResult ;
	    SV ** ppExecResult	    = &pExecResult ;

	    if (pProcessor -> pPreCompiler)
		{
		if ((rc = embperl_GetFromCache (r, pProcessor, 'P', "Precompiler", -1, NULL, &bForceExpirePre, &pCVKey, &ppPreCompResult)) != ok)
		    return rc ;
		}

	    if (pProcessor -> pCompiler)
		{
		if ((rc = embperl_GetFromCache (r, pProcessor, 'C', "Compiler", -1, NULL, &bForceExpire, &pCVKey, &ppCompResult)) != ok)
		    return rc ;

/* ###     lprintf (r, "[%d]comp1: SvTYPE (*ppSV) = %d\n", r -> nPid, SvTYPE (*ppCompResult)) ; */
		if ((rc = (*pProcessor -> pCompiler)(r, pProcessor, &pDomTree, ppPreCompResult, ppCompResult)) != ok)
		    return rc ;
/* ###     lprintf (r, "[%d]comp2: SvTYPE (*ppSV) = %d\n", r -> nPid, SvTYPE (*ppCompResult)) ; */
		}
	    
            if (!r -> bError && pProcessor -> pExecuter)
                {
		if (pProcessor -> pPreExecuter)
		    if ((rc = (*pProcessor -> pPreExecuter)(r, pProcessor, &pDomTree, ppPreCompResult, ppCompResult)) != ok)
			return rc ;

                if ((rc = embperl_GetFromCache (r, pProcessor, 'E', "Executer", pProcessor -> nOutputExpiresIn, pProcessor -> pOutputExpiresCV, &bForceExpire, &pCVKey, &ppExecResult)) != ok)
                    return rc ;

                if ((rc = (*pProcessor -> pExecuter)(r, pProcessor, &pDomTree, ppPreCompResult, ppCompResult, ppExecResult)) != ok)
                    return rc ;
                if (ppExecResult && *ppExecResult)
                    r -> xCurrDomTree = DomTree_SV (*ppExecResult) ;
                }
            pProcessor = pProcessor -> pNext ;
	    
	    if (pPreCompResult)
		SvREFCNT_dec (pPreCompResult) ;
	    if (pCompResult)
		SvREFCNT_dec (pCompResult) ;
	    if (pExecResult)
		SvREFCNT_dec (pExecResult) ;
	    }
	}        
        

    r -> nPhase  = phTerm ;


    return ok ;
    }
        
        
