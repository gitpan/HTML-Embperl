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



/* ---------------------------------------------------------------------------- */
/* Output a string and escapte html special character to html special representation (&xxx;) */
/* */
/* i/o sData     = input:  perl string */
/* */
/* ---------------------------------------------------------------------------- */

void OutputToHtml (/*i/o*/ const char *  sData)

    {
    char * pHtml  ;
    const char * p = sData ;
    
    EPENTRY (OutputToHtml) ;

    if (pCurrEscape == NULL)
        {
        oputs (sData) ;
        return ;
        }

    
    while (*sData)
        {
        if (*sData == '\\')
            {
            if (p != sData)
                owrite (p, 1, sData - p) ;
            sData++ ;
            p = sData ;
            }
        else
            {
            pHtml = pCurrEscape[(unsigned char)(*sData)].sHtml ;
            if (*pHtml)
                {
                if (p != sData)
                    owrite (p, 1, sData - p) ;
                oputs (pHtml) ;
                p = sData + 1;
                }
            }
        sData++ ;
        }
    if (p != sData)
        owrite (p, 1, sData - p) ;
    }




/* ---------------------------------------------------------------------------- */
/* find substring ignore case */
/* */
/* in  pSring  = string to search in (any case) */
/* in  pSubStr = string to search for (must be upper case) */
/* */
/* out ret  = pointer to pSubStr in pStringvalue or NULL if not found */
/* */
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
/*                                                                           */
/* ------------------------------------------------------------------------- */

void TransHtml (/*i/o*/ char *  sData)

    {
    char * p = sData ;
    char * s ;
    char * e ;
    struct tCharTrans * pChar ;

    EPENTRY (TransHtml) ;
	
    if (bOptions & optRawInput)
	{ /* Just remove CR for raw input */
	while (*p != '\0')
	    {
	    if (*p == '\r')
	    	*p = ' ' ;
	    p++ ;
	    }	

    	return ; 	
        }
        
    s = NULL ;
    e = sData + strlen (sData) ;

    while (*p)
        {
        if (*p == '\\')
            {
            if (p[1] == '<')
                { /*  Quote next HTML tag */
                p += 2 ;
                while (*p && *p != '>')
                    p++ ;
                }
            else if (p[1] == '&')
                { /*  Quote next HTML char */
                p += 2 ;
                while (*p && *p != ';')
                    p++ ;
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
            if (p[0] == '<' && isalpha (p[1]))
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

            if (s && (p - s) > 0)
                { /* copy rest of string, pad with spaces */
                memmove (s, p, e - p + 1) ;
                memset (e - (p - s), ' ', (p - s)) ;
                p = s ;
                s = NULL ;
                }
            else
                if (*p)
                    p++ ;
            
            }
        }
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

    EPENTRY (GetHtmlArg) ;

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
                char q = *pVal++ ;
                pEnd++ ;
                while (*pEnd && *pEnd != q)
                    pEnd++ ;
                }
            else
                {
                while (*pEnd && !isspace (*pEnd) && *pEnd != '>')
                    pEnd++ ;
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

    EPENTRY (GetHashValueLen) ;

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


int GetLineNo ()

    {
    char * pPos = pCurrPos ;
    
    if (pSourcelinePos == NULL)
        return nSourceline = 1 ;

    if (pLineNoCurrPos)
        pPos = pLineNoCurrPos ;

    if (pPos == NULL || pPos == pSourcelinePos || pPos < pBuf || pPos > pEndPos)
        return nSourceline ;


    if (pPos > pSourcelinePos)
        {
        char * p = pSourcelinePos ;

        while (p < pPos && p < pEndPos)
            {
            if (*p++ == '\n')
                nSourceline++ ;
            }
        }
    else
        {
        char * p = pSourcelinePos ;

        while (p > pPos && p > pBuf)
            {
            if (*--p == '\n')
                nSourceline-- ;
            }
        }

    pSourcelinePos = pPos ;
    return nSourceline ;
    }



