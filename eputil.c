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



/* ---------------------------------------------------------------------------- */
/* Output a string and escape html special character to html special            */
/* representation (&xxx;)                                                       */
/*                                                                              */
/* i/o sData     = input:  perl string                                          */
/*                                                                              */
/* ---------------------------------------------------------------------------- */

void OutputToHtml (/*i/o*/ register req * r,
 		   /*i/o*/ const char *   sData)

    {
    char * pHtml  ;
    const char * p = sData ;
    
    EPENTRY (OutputToHtml) ;

    if (r -> pCurrEscape == NULL)
        {
        oputs (r, sData) ;
        return ;
        }

    
    while (*sData)
        {
        if (*sData == '\\')
            {
            if (p != sData)
                owrite (r, p, sData - p) ;
            sData++ ;
            p = sData ;
            }
        else
            {
            pHtml = r -> pCurrEscape[(unsigned char)(*sData)].sHtml ;
            if (*pHtml)
                {
                if (p != sData)
                    owrite (r, p, sData - p) ;
                oputs (r, pHtml) ;
                p = sData + 1;
                }
            }
        sData++ ;
        }
    if (p != sData)
        owrite (r, p, sData - p) ;
    }



/* ---------------------------------------------------------------------------- */
/* find substring ignore case                                                   */
/*                                                                              */
/* in  pSring  = string to search in (any case)                                 */
/* in  pSubStr = string to search for (must be upper case)                      */
/*                                                                              */
/* out ret  = pointer to pSubStr in pStringvalue or NULL if not found           */
/*                                                                              */
/* ---------------------------------------------------------------------------- */



static const char * stristr (/*in*/ const char *   pString,
                             /*in*/ const char *   pSubString)

    {
    char c = *pSubString ;
    int  l = strlen (pSubString) ;
    
    do
        {
        while (*pString && toupper (*pString) != c)
            pString++ ;

        if (*pString == '\0')
            return NULL ;

        if (strnicmp (pString, pSubString, l) == 0)
            return pString ;
        pString++ ;
        }
    
    while (TRUE) ;
    }

/* ---------------------------------------------------------------------------- */
/* make string lower case */
/* */
/* i/o  pSring  = string to search in (any case) */
/* */
/* ---------------------------------------------------------------------------- */


static char * strlower (/*in*/ char *   pString)

    {
    char * p = pString ;
    
    while (*p)
        {
        *p = tolower (*p) ;
        p++ ;
        }

    return pString ;
    }


/* ---------------------------------------------------------------------------- */
/* save strdup                                                                  */
/*                                                                              */
/* in  pSring  = string to save on memory heap                                  */
/*                                                                              */
/* ---------------------------------------------------------------------------- */


char * sstrdup (/*in*/ char *   pString)

    {
    char * p ;
    
    if (pString == NULL)
        return NULL ;

    p = malloc (strlen (pString) + 1) ;

    strcpy (p, pString) ;

    return p ;
    }



/* */
/* compare html escapes */
/* */

static int CmpCharTrans (/*in*/ const void *  pKey,
                         /*in*/ const void *  pEntry)

    {
    return strcmp ((const char *)pKey, ((struct tCharTrans *)pEntry) -> sHtml) ;
    }



/* ------------------------------------------------------------------------- */
/*                                                                           */
/* replace html special character representation (&xxx;) with correct chars  */
/* and delete all html tags                                                  */
/* The Replacement is done in place, the whole string will become shorter    */
/* and is padded with spaces                                                 */
/* tags and special charcters which are preceeded by a \ are not translated  */
/* carrige returns are replaced by spaces                                    */
/* if optRawInput is set the functions do just nothing                       */
/* 								             */
/* i/o sData     = input:  html string                                       */
/*                 output: perl string                                       */
/* in  nLen      = length of sData on input (or 0 for null terminated)       */
/*                                                                           */
/* ------------------------------------------------------------------------- */

int   TransHtml (/*i/o*/ register req * r,
		/*i/o*/ char *         sData,
		/*in*/   int           nLen)

    {
    char * p = sData ;
    char * s ;
    char * e ;
    struct tCharTrans * pChar ;
    int  bInUrl = r -> bEscInUrl ;

    EPENTRY (TransHtml) ;
	
    if (r -> bOptions & optRawInput)
	{ /* Just remove CR for raw input */
	while (*p != '\0')
	    {
	    if (*p == '\r')
	    	*p = ' ' ;
	    p++ ;
	    }	

	return nLen ; 	
        }
        
    s = NULL ;
    if (nLen == 0)
        nLen = strlen (sData) ;
    e = sData + nLen ;

    while (*p && p < e)
	{
	if (*p == '\\')
	    {
        
	    if (p[1] == '<')
		{ /*  Quote next HTML tag */
		memmove (p, p + 1, e - p - 1) ;
		e[-1] = ' ' ;
		p++ ;
		while (*p && *p != '>')
		    p++ ;
		}
	    else if (p[1] == '&')
		{ /*  Quote next HTML char */
		memmove (p, p + 1, e - p - 1) ;
		e[-1] = ' ' ;
		p++ ;
		while (*p && *p != ';')
		    p++ ;
		}
	    else if (bInUrl && p[1] == '%')
		{ /*  Quote next URL escape */
		memmove (p, p + 1, e - p - 1) ;
		e[-1] = ' ' ;
		p += 3 ;
		}
	    else
		p++ ; /* Nothing to quote */
	    }
	else if (*p == '\r')
	    {
	    *p++ = ' ' ;
	    }
	else
	    {
	    if (p[0] == '<' && (isalpha (p[1]) || p[1] == '/'))
		{ /*  count HTML tag length */
		s = p ;
		p++ ;
		while (*p && *p != '>')
		    p++ ;
		if (*p)
		    p++ ;
		else
		    { /* missing left '>' -> no html tag  */
		    p = s ;
		    s = NULL ;
		    }
		}
	    else if (p[0] == '&')
		{ /*  count HTML char length */
		s = p ;
		p++ ;
		while (*p && *p != ';')
		    p++ ;

		if (*p)
		    {
		    *p = '\0' ;
		    p++ ;
		    pChar = (struct tCharTrans *)bsearch (s, Html2Char, sizeHtml2Char, sizeof (struct tCharTrans), CmpCharTrans) ;
		    if (pChar)
			*s++ = pChar -> c ;
		    else
			{
			*(p-1)=';' ;
			p = s ;
			s = NULL ;
			}
		    }
		else
		    {
		    p = s ;
		    s = NULL ;
		    }
		}
	    else if (bInUrl && p[0] == '%' && isdigit (p[1]) && isxdigit (p[2]))
		{ 

		s = p ;
		p += 3 ;
                *s++ = ((toupper (p[-2]) - (isdigit (p[-2])?'0':('A' - 10))) << 4) 
                      + (toupper (p[-1]) - (isdigit (p[-1])?'0':('A' - 10)))  ;
		}
	    if (s && (p - s) > 0)
		{ /* copy rest of string, pad with spaces */
		memmove (s, p, e - p + 1) ;
		memset (e - (p - s), ' ', (p - s)) ;
		nLen -= p - s ;
		p = s ;
		s = NULL ;
		}
	    else
		if (*p)
		    p++ ;
	    }
	}

    return nLen ;
    }



void TransHtmlSV (/*i/o*/ register req * r,
		  /*i/o*/ SV *           pSV) 

    {
    STRLEN vlen ;
    STRLEN nlen ;
    char * pVal = SvPV (pSV, vlen) ;

    nlen = TransHtml (r, pVal, vlen) ;

    pVal[nlen] = '\0' ;
    SvCUR_set(pSV, nlen) ;
    }		  


/* ---------------------------------------------------------------------------- */
/* get argument from html tag  */
/* */
/* in  pTag = html tag args  (eg. arg=val arg=val .... >) */
/* in  pArg = name of argument (must be upper case) */
/* */
/* out pLen = length of value */
/* out ret  = pointer to value or NULL if not found */
/* */
/* ---------------------------------------------------------------------------- */

const char * GetHtmlArg (/*in*/  const char *    pTag,
                         /*in*/  const char *    pArg,
                         /*out*/ int *           pLen)

    {
    const char * pVal ;
    const char * pEnd ;
    const char * pName ;
    int l ;

    /*EPENTRY (GetHtmlArg) ;*/

    l = strlen (pArg) ;
    while (*pTag)
        {
        *pLen = 0 ;

        while (*pTag && !isalpha (*pTag))
            pTag++ ; 

        pVal = pTag ;
        while (*pVal && !isspace (*pVal) && *pVal != '=' && *pVal != '>')
            pVal++ ;

        while (*pVal && isspace (*pVal))
            pVal++ ;

        if (*pVal == '=')
            {
            pVal++ ;
            while (*pVal && isspace (*pVal))
                pVal++ ;
        
            pEnd = pVal ;
            if (*pVal == '"' || *pVal == '\'')
                {
		char nType = '\0';
                char q = *pVal++ ;
                pEnd++ ;

		while ((*pEnd != q || nType) && *pEnd != '\0')
		    {
		    if (nType == '\0' && *pEnd == '[' && (pEnd[1] == '+' || pEnd[1] == '-' || pEnd[1] == '$' || pEnd[1] == '!' || pEnd[1] == '#'))
			nType = *++pEnd ;
		    else if (nType && *pEnd == nType && pEnd[1] == ']')
			{
			nType = '\0';
			pEnd++ ;
			}
                    pEnd++ ;
		    }
		}
            else
                {
		char nType = '\0';
		while ((!isspace (*pEnd) || nType) && *pEnd != '\0'  && *pEnd != '>')
		    {
		    if (nType == '\0' && *pEnd == '[' && (pEnd[1] == '+' || pEnd[1] == '-' || pEnd[1] == '$' || pEnd[1] == '!' || pEnd[1] == '#'))
			nType = *++pEnd ;
		    else if (nType && *pEnd == nType && pEnd[1] == ']')
			{
			nType = '\0';
			pEnd++ ;
			}
                    pEnd++ ;
		    }
                }

            *pLen = pEnd - pVal ;
            }
        else
            pEnd = pVal ;

        
        if (strnicmp (pTag, pArg, l) == 0 && (pTag[l] == '=' || isspace (pTag[l]) || pTag[l] == '>' || pTag[l] == '\0'))
            if (*pLen > 0)
                return pVal ;
            else
                return pTag ;

        pTag = pEnd ;
        }

    *pLen = 0 ;
    return NULL ;
    }


    
    
/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* Get a Value out of a perl hash                                               */
/*                                                                              */
/* ---------------------------------------------------------------------------- */


char * GetHashValueLen (/*in*/  HV *           pHash,
                        /*in*/  const char *   sKey,
                        /*in*/  int            nLen,
                        /*in*/  int            nMaxLen,
                        /*out*/ char *         sValue)

    {
    SV **   ppSV ;
    char *  p ;
    STRLEN  len ;        

    /*EPENTRY (GetHashValueLen) ;*/

    ppSV = hv_fetch(pHash, (char *)sKey, nLen, 0) ;  
    if (ppSV != NULL)
        {
        p = SvPV (*ppSV ,len) ;
        if (len >= nMaxLen)
            len = nMaxLen - 1 ;        
        strncpy (sValue, p, len) ;
        }
    else
        len = 0 ;

    sValue[len] = '\0' ;
        
    return sValue ;
    }


char * GetHashValue (/*in*/  HV *           pHash,
                     /*in*/  const char *   sKey,
                     /*in*/  int            nMaxLen,
                     /*out*/ char *         sValue)
    {
    return GetHashValueLen (pHash, sKey, strlen (sKey), nMaxLen, sValue) ;
    }




int    GetHashValueInt (/*in*/  HV *           pHash,
                        /*in*/  const char *   sKey,
                        /*in*/  int            nDefault)

    {
    SV **   ppSV ;
    char *  p ;
    STRLEN  len ;        

    /*EPENTRY (GetHashValueInt) ;*/

    ppSV = hv_fetch(pHash, (char *)sKey, strlen (sKey), 0) ;  
    if (ppSV != NULL)
        return SvIV (*ppSV) ;
        
    return nDefault ;
    }


char * GetHashValueStr (/*in*/  HV *           pHash,
                        /*in*/  const char *   sKey,
                        /*in*/  char *         sDefault)

    {
    SV **   ppSV ;
    char *  p ;
    STRLEN  len ;        

    /*EPENTRY (GetHashValueInt) ;*/

    ppSV = hv_fetch(pHash, (char *)sKey, strlen (sKey), 0) ;  
    if (ppSV != NULL)
        return SvPV (*ppSV, na) ;
        
    return sDefault ;
    }


/* ------------------------------------------------------------------------- */
/*                                                                           */
/* GetLineNo								     */
/*                                                                           */
/* Counts the \n between pCurrPos and pSourcelinePos and in-/decrements      */
/* nSourceline accordingly                                                   */
/*                                                                           */
/* return Linenumber of pCurrPos                                             */
/*                                                                           */
/* ------------------------------------------------------------------------- */


int GetLineNo (/*i/o*/ register req * r)

    {
    char * pPos = r -> Buf.pCurrPos ;
    
    if (r -> Buf.pSourcelinePos == NULL)
	if (r -> Buf.pFile == NULL)
	    return 0 ;
	else
	    return r -> Buf.nSourceline = r -> Buf.pFile -> nFirstLine ;

    if (r -> Buf.pLineNoCurrPos)
        pPos = r -> Buf.pLineNoCurrPos ;

    if (pPos == NULL || pPos == r -> Buf.pSourcelinePos || pPos < r -> Buf.pBuf || pPos > r -> Buf.pEndPos)
        return r -> Buf.nSourceline ;


    if (pPos > r -> Buf.pSourcelinePos)
        {
        char * p = r -> Buf.pSourcelinePos ;

        while (p < pPos && p < r -> Buf.pEndPos)
            {
            if (*p++ == '\n')
                r -> Buf.nSourceline++ ;
            }
        }
    else
        {
        char * p = r -> Buf.pSourcelinePos ;

        while (p > pPos && p > r -> Buf.pBuf)
            {
            if (*--p == '\n')
                r -> Buf.nSourceline-- ;
            }
        }

    r -> Buf.pSourcelinePos = pPos ;
    return r -> Buf.nSourceline ;
    }



/* ------------------------------------------------------------------------- */
/*                                                                           */
/* Dirname								     */
/*                                                                           */
/* returns dir name of file                                                  */
/*                                                                           */
/* ------------------------------------------------------------------------- */



void Dirname (/*in*/ const char * filename,
              /*out*/ char *      dirname,
              /*in*/  int         size)

    {
    char * p = strrchr (filename, '/') ;

    if (p == NULL)
        {
        strncpy (dirname, ".", size) ;
        return ;
        }

    if (size - 1 > p - filename)
        size = p - filename ;

    strncpy (dirname, filename, size) ;
    dirname[size] = '\0' ;

    return ;
    }



/* ------------------------------------------------------------------------- */
/*                                                                           */
/* GetSubTextPos						             */
/*                                                                           */
/*                                                                           */
/* in	sName = name of sub                                                  */
/*                                                                           */
/* returns the position within the file for a given Embperl sub              */
/*                                                                           */
/* ------------------------------------------------------------------------- */


int GetSubTextPos (/*i/o*/ register req * r,
		   /*in*/  const char *   sName) 

    {
    SV **   ppSV ;
    const char *  sKey  ;
    char    sKeyBuf[sizeof (int) + 4] ;
    int	    l ;    
    
    EPENTRY (Eval) ;

    while (isspace(*sName))
	sName++ ;

    l = strlen (sName) ;
    while (l > 0 && isspace(sName[l-1]))
	l-- ;
    
    sKey = sName ;
    if (l < sizeof (int))
	{ /* right pad name with spaces to make sure name is longer then sizeof (int) */
	  /* distiguish it from filepos entrys */
	memset (sKeyBuf, ' ', sizeof (sKeyBuf) - 1) ;
	sKeyBuf[sizeof(sKeyBuf) - 1] = '\0' ;
	memcpy (sKeyBuf, sName, l) ;
	sKey = sKeyBuf ;
	l = sizeof(sKeyBuf) - 1 ;
	}
    
    
    ppSV = hv_fetch(r -> Buf.pFile -> pCacheHash, (char *)sKey, l, 0) ;  
    if (ppSV == NULL || *ppSV == NULL) /* || SvTYPE (*ppSV) != SVt_IV)*/
        return 0 ;

    return SvIV (*ppSV) ;    
    }


/* ------------------------------------------------------------------------- */
/*                                                                           */
/* SetSubTextPos						             */
/*                                                                           */
/*                                                                            */
/* in	sName = name of sub                                                  */
/* in   nPos  = position within the file for a given Embperl sub             */
/*                                                                           */
/* ------------------------------------------------------------------------- */


int SetSubTextPos (/*i/o*/ register req * r,
		   /*in*/  const char *   sName,
		   /*in*/  int		  nPos) 

    {
    SV **   ppSV ;
    const char *  sKey ;
    char    sKeyBuf[sizeof (int) + 4] ;
    int	    l ;    
    
    EPENTRY (Eval) ;

    while (isspace(*sName))
	sName++ ;

    l = strlen (sName) ;
    while (l > 0 && isspace(sName[l-1]))
	l-- ;
    
    sKey = sName ;
    if (l < sizeof (int))
	{ /* right pad name with spaces to make sure name is longer then sizeof (int) */
	  /* distiguish it from filepos entrys */
	memset (sKeyBuf, ' ', sizeof (sKeyBuf) - 1) ;
	sKeyBuf[sizeof(sKeyBuf) - 1] = '\0' ;
	memcpy (sKeyBuf, sName, l) ;
	sKey = sKeyBuf ;
	l = sizeof(sKeyBuf) - 1 ;
	}
    
    
    ppSV = hv_fetch(r -> Buf.pFile -> pCacheHash, (char *)sKey, l, 1) ;  
    if (ppSV == NULL)
        return rcHashError ;

    SvREFCNT_dec (*ppSV) ;  
    *ppSV = newSViv (nPos) ;
    
    return ok ;
    }