/*###################################################################################
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
#   $Id: eplibxslt.c,v 1.1.2.10 2001/11/27 08:37:56 richter Exp $
#
###################################################################################*/


#include "../ep.h"
#include "../epmacro.h"

#include <libxml/xmlmemory.h>
#include <libxml/debugXML.h>
#include <libxml/HTMLtree.h>
#include <libxml/xmlIO.h>
#include <libxml/DOCBparser.h>
#include <libxml/xinclude.h>
#include <libxml/catalog.h>
#include <libxslt/xsltconfig.h>
#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>

extern int xmlLoadExtDtdDefaultValue;


/* ------------------------------------------------------------------------ */
/*                                                                          */
/* iowrite                                                                  */
/*                                                                          */
/* output callback                                                          */
/*                                                                          */
/* ------------------------------------------------------------------------ */

static int  iowrite   (void *context,
                const char *buffer,
                int len)

    {
    return owrite ((tReq *)context, buffer, len) ;
    }


/* ------------------------------------------------------------------------ */
/*                                                                          */
/* embperl_LibXSLT_Text2Text                                                */
/*                                                                          */
/* Do an XSL transformation using LibXSLT. Input and Output is Text.        */
/* The stylesheet is directly read from disk                                */
/*                                                                          */
/* in   pReqParameter   Parameter for request                               */
/*          xsltparameter   Hash which is passed as parameters to libxslt   */
/*          xsltstylesheet  filename of stylsheet                           */
/*      pSource         XML source in memory                                */
/*                                                                          */
/* ------------------------------------------------------------------------ */



int embperl_LibXSLT_Text2Text   (/*in*/  tReq *	  r,
                                 /*in*/  HV *     pReqParameter,
                                 /*in*/  SV *     pSource)

    {
    xsltStylesheetPtr cur = NULL;
    xmlDocPtr	    doc ;
    xmlDocPtr	    res;
    HE *	    pEntry ;
    HV *            pParam ;
    SV * *          ppSV ;
    char *	    pKey ;
    SV *            pValue ;
    STRLEN          len ;
    IV              l ;
    int		    n ;
    const char * *  pParamArray ;
    const char *    sStylesheet ;
    char *          p ;
    xmlOutputBufferPtr obuf ;

    sStylesheet = GetHashValueStr (pReqParameter, "xsltstylesheet", NULL) ;
    if (!sStylesheet)
	{
	strncpy (r -> errdat1, "XSLT", sizeof (r -> errdat1)) ;
	strncpy (r -> errdat2, "No stylesheet given", sizeof (r -> errdat2)) ;
	return 9999 ;
	}

    ppSV = hv_fetch (pReqParameter, "xsltparameter", sizeof("xsltparameter") - 1, 0) ;
    if (ppSV && *ppSV)
	{
	if (!SvROK (*ppSV) || SvTYPE ((SV *)(pParam = (HV *)SvRV (*ppSV))) != SVt_PVHV)
	    {
	    strncpy (r -> errdat1, "XSLT", sizeof (r -> errdat1)) ;
	    sprintf (r -> errdat2, "%s", pKey) ;
	    return rcNotHashRef ;
	    }

	n = 0 ;
	hv_iterinit (pParam) ;
	while ((pEntry = hv_iternext (pParam)))
	    {
	    n++ ;
	    }
        
	if (!(pParamArray = _malloc(r, sizeof (const char *) * (n + 1) * 2)))
	    return rcOutOfMemory ;

	n = 0 ;
	hv_iterinit (pParam) ;
	while ((pEntry = hv_iternext (pParam)))
	    {
	    pKey     = hv_iterkey (pEntry, &l) ;
	    pValue   = hv_iterval (pParam, pEntry) ;
	    pParamArray[n++] = pKey ;
	    pParamArray[n++] = SvPV (pValue, len) ;
	    }
	pParamArray[n++] = NULL ;
	}
    else
	{
	pParamArray = NULL ;
	}

    xmlSubstituteEntitiesDefault(1);
    xmlLoadExtDtdDefaultValue = 1;
    xmlSetGenericErrorFunc (stderr, NULL) ;
    
    cur = xsltParseStylesheetFile(sStylesheet);
    p   = SvPV (pSource, len) ;
    doc = xmlParseMemory(p, len);
    res = xsltApplyStylesheet(cur, doc, pParamArray);

    
    obuf = xmlOutputBufferCreateIO (iowrite, NULL, r, NULL) ;
    
    xsltSaveResultTo(obuf, res, cur);

    xsltFreeStylesheet(cur);
    xmlFreeDoc(res);
    xmlFreeDoc(doc);

    xsltCleanupGlobals();
    xmlCleanupParser();

    return(0);
    }




/*! Provider that reads compiles LibXSLT stylesheet */

typedef struct tProviderLibXSLTXSL
    {
    tProvider           Provider ;
    } tProviderLibXSLTXSL ;


/* ------------------------------------------------------------------------ */
/*                                                                          */
/* ProviderLibXSLTXSL_New      					            */
/*                                                                          */
/*! 
*   \_en
*   Creates a new LibXSLT stylesheet provider and fills it with data from the hash pParam
*   The resulting provider is put into the cache structure
*   
*   @param  r               Embperl request record
*   @param  pItem           CacheItem which holds the output of the provider
*   @param  pProviderClass  Provider class record
*   @param  pParam          Parameter Hash
*                               stylesheet  filename or provider for the
*                                           stylesheet 
*   @return                 error code
*   \endif                                                                       
*
*   \_de									   
*   Erzeugt einen neue Provider f�r LibXSLT Stylesheets. Der ein Zeiger
*   auf den resultierenden Provider wird in die Cachestrutr eingef�gt
*   
*   @param  r               Embperl request record
*   @param  pItem           CacheItem welches die Ausgabe des Providers 
*                           speichert
*   @param  pProviderClass  Provider class record
*   @param  pParam          Parameter Hash
*                               stylesheet  dateiname oder provider f�r das
*                                           stylesheet 
*   @return                 Fehlercode
*   \endif                                                                       
*                                                                          
* ------------------------------------------------------------------------ */

static int ProviderLibXSLTXSL_New (/*in*/ req *              r,
                          /*in*/ tCacheItem *       pItem,
                          /*in*/ tProviderClass *   pProviderClass,
                          /*in*/ HV *               pParam)


    {
    int                 rc ;
    
    if ((rc = Provider_NewDependOne (r, sizeof(tProviderLibXSLTXSL), "stylesheet", pItem, pProviderClass, pParam)) != ok)
        return rc ;

    return ok ;
    }

/* ------------------------------------------------------------------------ */
/*                                                                          */
/* ProviderFile_AppendKey    					            */
/*                                                                          */
/*! 
*   \_en
*   Append it's key to the keystring. If it depends on anything it must 
*   call Cache_AppendKey for any dependency.
*   
*   @param  r               Embperl request record
*   @param  pProviderClass  Provider class record
*   @param  pParam          Parameter Hash
*                               filename        name of file
*   @param  pKey            Key to which string should be appended
*   @return                 error code
*   \endif                                                                       
*
*   \_de									   
*   H�ngt ein eigenen Schl�ssel an den Schl�sselstring an. Wenn irgednwelche
*   Abh�nigkeiten bestehen, mu� Cache_AppendKey f�r alle Abh�nigkeiten aufgerufen 
*   werden.
*   
*   @param  r               Embperl request record
*   @param  pProviderClass  Provider class record
*   @param  pParam          Parameter Hash
*                               filename        Dateiname
*   @param  pKey            Schl�ssel zu welchem hinzugef�gt wird
*   @return                 Fehlercode
*   \endif                                                                       
*                                                                          
* ------------------------------------------------------------------------ */

static int ProviderLibXSLTXSL_AppendKey (/*in*/ req *              r,
                                   /*in*/ tProviderClass *   pProviderClass,
                                   /*in*/ HV *               pParam,
                                   /*i/o*/ SV *              pKey)
    {
    int          rc ;

    if ((rc = Cache_AppendKey (r, pParam, "stylesheet", pKey)) != ok)
        return rc;

    sv_catpv (pKey, "*libxslt-compile-xsl") ;
    return ok ;
    }



/* ------------------------------------------------------------------------ */
/*                                                                          */
/* ProviderLibXSLTXSL_GetContentPtr  				            */
/*                                                                          */
/*! 
*   \_en
*   Get the whole content from the provider. 
*   This gets the stylesheet and compiles it
*   
*   @param  r               Embperl request record
*   @param  pProvider       The provider record
*   @param  pData           Returns the content
*   @return                 error code
*   \endif                                                                       
*
*   \_de									   
*   Holt den gesamt Inhalt vom Provider.
*   Die Funktion holt sich das Stylesheet und kompiliert es
*   
*   @param  r               Embperl request record
*   @param  pProvider       The provider record
*   @param  pData           Liefert den Inhalt
*   @return                 Fehlercode
*   \endif                                                                       
*                                                                          
* ------------------------------------------------------------------------ */



static int ProviderLibXSLTXSL_GetContentPtr     (/*in*/ req *            r,
                                        /*in*/ tProvider *      pProvider,
                                        /*in*/ void * *         pData)

    {
    int    rc ;
    char * p ;
    STRLEN len ;
    SV *   pSource ;
    xsltStylesheetPtr cur ;
    xmlDocPtr	    doc ;

    tCacheItem * pFileCache = Cache_GetDependency(r, pProvider -> pCache, 0) ;
    if ((rc = Cache_GetContentSV (r, pFileCache, &pSource)) != ok)
        return rc ;
        
    p   = SvPV (pSource, len) ;

    if (p == NULL || len == 0)
	{
	strncpy (r -> errdat1, "LibXSLT XML stylesheet", sizeof (r -> errdat1)) ;
	return rcMissingInput ;
	}

    if ((doc = xmlParseMemory(p, len)) == NULL)
      	{
	Cache_ReleaseContent (r, pFileCache) ;
        strncpy (r -> errdat1, "XSL parse", sizeof (r -> errdat1)) ;
	return rcLibXSLTError ;
	}
    ;
	
    if ((cur = xsltParseStylesheetDoc(doc)) == NULL)
      	{
        xmlFreeDoc(doc) ;
	Cache_ReleaseContent (r, pFileCache) ;
        strncpy (r -> errdat1, "XSL compile", sizeof (r -> errdat1)) ;
	return rcLibXSLTError ;
	}
    
    *pData = (void *)cur ;

    return ok ;
    }

/* ------------------------------------------------------------------------ */
/*                                                                          */
/* ProviderLibXSLTXSL_FreeContent 		                            */
/*                                                                          */
/*! 
*   \_en
*   Free the cached data
*   
*   @param  r               Embperl request record
*   @param  pProvider       The provider record
*   @return                 error code
*   \endif                                                                       
*
*   \_de									   
*   Gibt die gecachten Daten frei
*   
*   @param  r               Embperl request record
*   @param  pProvider       The provider record
*   @return                 Fehlercode
*   \endif                                                                       
*                                                                          
* ------------------------------------------------------------------------ */



static int ProviderLibXSLTXSL_FreeContent(/*in*/ req *             r,
                                 /*in*/ tCacheItem * pItem)

    {
    xsltStylesheetPtr  pCompiledStylesheet ;
    
    pCompiledStylesheet = (xsltStylesheetPtr)pItem -> pData ;
    if (pCompiledStylesheet)
        xsltFreeStylesheet(pCompiledStylesheet) ;


    return ok ;
    }

/* ------------------------------------------------------------------------ */

static tProviderClass ProviderClassLibXSLTXSL = 
    {   
    "text/*", 
    &ProviderLibXSLTXSL_New, 
    &ProviderLibXSLTXSL_AppendKey, 
    NULL,
    NULL,
    &ProviderLibXSLTXSL_GetContentPtr,
    NULL,
    &ProviderLibXSLTXSL_FreeContent,
    NULL,
    } ;



/*! Provider that reads compiles LibXSLT xml source */

typedef struct tProviderLibXSLTXML
    {
    tProvider           Provider ;
    } tProviderLibXSLTXML ;


/* ------------------------------------------------------------------------ */
/*                                                                          */
/* ProviderLibXSLTXML_New      					            */
/*                                                                          */
/*! 
*   \_en
*   Creates a new LibXSLT xml source provider and fills it with data from the hash pParam
*   The resulting provider is put into the cache structure
*   
*   @param  r               Embperl request record
*   @param  pItem           CacheItem which holds the output of the provider
*   @param  pProviderClass  Provider class record
*   @param  pParam          Parameter Hash
*                               stylesheet  filename or provider for the
*                                           stylesheet 
*   @return                 error code
*   \endif                                                                       
*
*   \_de									   
*   Erzeugt einen neue Provider f�r LibXSLT XML Quellen. Der ein Zeiger
*   auf den resultierenden Provider wird in die Cachestrutr eingef�gt
*   
*   @param  r               Embperl request record
*   @param  pItem           CacheItem welches die Ausgabe des Providers 
*                           speichert
*   @param  pProviderClass  Provider class record
*   @param  pParam          Parameter Hash
*                               stylesheet  dateiname oder provider f�r das
*                                           stylesheet 
*   @return                 Fehlercode
*   \endif                                                                       
*                                                                          
* ------------------------------------------------------------------------ */

static int ProviderLibXSLTXML_New (/*in*/ req *              r,
                          /*in*/ tCacheItem *       pItem,
                          /*in*/ tProviderClass *   pProviderClass,
                          /*in*/ HV *               pParam)


    {
    int                 rc ;
    
    if ((rc = Provider_NewDependOne (r, sizeof(tProviderLibXSLTXML), "source", pItem, pProviderClass, pParam)) != ok)
        return rc ;

    return ok ;
    }

/* ------------------------------------------------------------------------ */
/*                                                                          */
/* ProviderFile_AppendKey    					            */
/*                                                                          */
/*! 
*   \_en
*   Append it's key to the keystring. If it depends on anything it must 
*   call Cache_AppendKey for any dependency.
*   
*   @param  r               Embperl request record
*   @param  pProviderClass  Provider class record
*   @param  pParam          Parameter Hash
*                               filename        name of file
*   @param  pKey            Key to which string should be appended
*   @return                 error code
*   \endif                                                                       
*
*   \_de									   
*   H�ngt ein eigenen Schl�ssel an den Schl�sselstring an. Wenn irgednwelche
*   Abh�nigkeiten bestehen, mu� Cache_AppendKey f�r alle Abh�nigkeiten aufgerufen 
*   werden.
*   
*   @param  r               Embperl request record
*   @param  pProviderClass  Provider class record
*   @param  pParam          Parameter Hash
*                               filename        Dateiname
*   @param  pKey            Schl�ssel zu welchem hinzugef�gt wird
*   @return                 Fehlercode
*   \endif                                                                       
*                                                                          
* ------------------------------------------------------------------------ */

static int ProviderLibXSLTXML_AppendKey (/*in*/ req *              r,
                                   /*in*/ tProviderClass *   pProviderClass,
                                   /*in*/ HV *               pParam,
                                   /*i/o*/ SV *              pKey)
    {
    int          rc ;

    if ((rc = Cache_AppendKey (r, pParam, "source", pKey)) != ok)
        return rc;

    sv_catpv (pKey, "*libxslt-parse-xml") ;
    return ok ;
    }


/* ------------------------------------------------------------------------ */
/*                                                                          */
/* ProviderLibXSLTXML_GetContentPtr  				            */
/*                                                                          */
/*! 
*   \_en
*   Get the whole content from the provider. 
*   This gets the stylesheet and compiles it
*   
*   @param  r               Embperl request record
*   @param  pProvider       The provider record
*   @param  pData           Returns the content
*   @return                 error code
*   \endif                                                                       
*
*   \_de									   
*   Holt den gesamt Inhalt vom Provider.
*   Die Funktion holt sich das Stylesheet und kompiliert es
*   
*   @param  r               Embperl request record
*   @param  pProvider       The provider record
*   @param  pData           Liefert den Inhalt
*   @return                 Fehlercode
*   \endif                                                                       
*                                                                          
* ------------------------------------------------------------------------ */



static int ProviderLibXSLTXML_GetContentPtr     (/*in*/ req *            r,
                                        /*in*/ tProvider *      pProvider,
                                        /*in*/ void * *         pData)

    {
    int    rc ;
    char * p ;
    STRLEN len ;
    SV *   pSource ;
    xmlDocPtr	    doc ;

    tCacheItem * pFileCache = Cache_GetDependency(r, pProvider -> pCache, 0) ;
    if ((rc = Cache_GetContentSV (r, pFileCache, &pSource)) != ok)
        return rc ;
        
    p   = SvPV (pSource, len) ;

    if (p == NULL || len == 0)
	{
	strncpy (r -> errdat1, "LibXSLT XML source", sizeof (r -> errdat1)) ;
	return rcMissingInput ;
	}

    if ((doc = xmlParseMemory(p, len)) == NULL)
      	{
	Cache_ReleaseContent (r, pFileCache) ;
        strncpy (r -> errdat1, "XML parse", sizeof (r -> errdat1)) ;
	return rcLibXSLTError ;
	}

    *pData = (void *)doc ;

    return ok ;
    }

/* ------------------------------------------------------------------------ */
/*                                                                          */
/* ProviderLibXSLTXML_FreeContent 		                            */
/*                                                                          */
/*! 
*   \_en
*   Free the cached data
*   
*   @param  r               Embperl request record
*   @param  pProvider       The provider record
*   @return                 error code
*   \endif                                                                       
*
*   \_de									   
*   Gibt die gecachten Daten frei
*   
*   @param  r               Embperl request record
*   @param  pProvider       The provider record
*   @return                 Fehlercode
*   \endif                                                                       
*                                                                          
* ------------------------------------------------------------------------ */



static int ProviderLibXSLTXML_FreeContent(/*in*/ req *             r,
                                 /*in*/ tCacheItem * pItem)

    {
    if (pItem -> pData)
	{
	xmlFreeDoc((xmlDocPtr)pItem -> pData) ;
	}
    return ok ;
    }

/* ------------------------------------------------------------------------ */

static tProviderClass ProviderClassLibXSLTXML = 
    {   
    "text/*", 
    &ProviderLibXSLTXML_New, 
    &ProviderLibXSLTXML_AppendKey, 
    NULL,
    NULL,
    &ProviderLibXSLTXML_GetContentPtr,
    NULL,
    &ProviderLibXSLTXML_FreeContent,
    NULL,
    } ;




/*! Provider that reads compiles LibXSLT stylesheet */

typedef struct tProviderLibXSLT
    {
    tProvider           Provider ;
    SV *                pOutputSV ;
    const char * *      pParamArray ;
    } tProviderLibXSLT ;

/* ------------------------------------------------------------------------ */
/*                                                                          */
/* ProviderLibXSLT_iowrite                                                  */
/*                                                                          */
/* output callback                                                          */
/*                                                                          */
/* ------------------------------------------------------------------------ */

static  int  ProviderLibXSLT_iowrite   (void *context,
						     const char *buffer,
						     int len)

    {
    sv_catpvn (((tProviderLibXSLT *)context) -> pOutputSV, (char *)buffer, len) ;
    return len ;
    }


/* ------------------------------------------------------------------------ */
/*                                                                          */
/* ProviderLibXSLT_New      					            */
/*                                                                          */
/*! 
*   \_en
*   Creates a new LibXSLT provider and fills it with data from the hash pParam
*   The resulting provider is put into the cache structure
*   
*   @param  r               Embperl request record
*   @param  pItem           CacheItem which holds the output of the provider
*   @param  pProviderClass  Provider class record
*   @param  pParam          Parameter Hash
*                               stylesheet  filename or provider for the
*                                           stylesheet 
*   @return                 error code
*   \endif                                                                       
*
*   \_de									   
*   Erzeugt einen neue Provider f�r LibXSLT.  Der ein Zeiger
*   auf den resultierenden Provider wird in die Cachestrutr eingef�gt
*   
*   @param  r               Embperl request record
*   @param  pItem           CacheItem welches die Ausgabe des Providers 
*                           speichert
*   @param  pProviderClass  Provider class record
*   @param  pParam          Parameter Hash
*                               stylesheet  dateiname oder provider f�r das
*                                           stylesheet 
*   @return                 Fehlercode
*   \endif                                                                       
*                                                                          
* ------------------------------------------------------------------------ */

int ProviderLibXSLT_New (/*in*/ req *              r,
                          /*in*/ tCacheItem *       pItem,
                          /*in*/ tProviderClass *   pProviderClass,
                          /*in*/ HV *               pParam)


    {
    int                 rc ;
    
    if ((rc = Provider_NewDependOne (r, sizeof(tProviderLibXSLT), "source", pItem, pProviderClass, pParam)) != ok)
        return rc ;

    if ((rc = Provider_AddDependOne (r, pItem -> pProvider, "stylesheet", pItem, pProviderClass, pParam)) != ok)
        return rc ;

    return ok ;
    }

/* ------------------------------------------------------------------------ */
/*                                                                          */
/* ProviderFile_AppendKey    					            */
/*                                                                          */
/*! 
*   \_en
*   Append it's key to the keystring. If it depends on anything it must 
*   call Cache_AppendKey for any dependency.
*   
*   @param  r               Embperl request record
*   @param  pProviderClass  Provider class record
*   @param  pParam          Parameter Hash
*                               filename        name of file
*   @param  pKey            Key to which string should be appended
*   @return                 error code
*   \endif                                                                       
*
*   \_de									   
*   H�ngt ein eigenen Schl�ssel an den Schl�sselstring an. Wenn irgednwelche
*   Abh�nigkeiten bestehen, mu� Cache_AppendKey f�r alle Abh�nigkeiten aufgerufen 
*   werden.
*   
*   @param  r               Embperl request record
*   @param  pProviderClass  Provider class record
*   @param  pParam          Parameter Hash
*                               filename        Dateiname
*   @param  pKey            Schl�ssel zu welchem hinzugef�gt wird
*   @return                 Fehlercode
*   \endif                                                                       
*                                                                          
* ------------------------------------------------------------------------ */

static int ProviderLibXSLT_AppendKey (/*in*/ req *              r,
                                   /*in*/ tProviderClass *   pProviderClass,
                                   /*in*/ HV *               pParam,
                                   /*i/o*/ SV *              pKey)
    {
    int          rc ;

    if ((rc = Cache_AppendKey (r, pParam, "source", pKey)) != ok)
        return rc;

    if ((rc = Cache_AppendKey (r, pParam, "stylesheet", pKey)) != ok)
        return rc;

    sv_catpv (pKey, "*libxslt") ;
    return ok ;
    }


/* ------------------------------------------------------------------------ */
/*                                                                          */
/* ProviderLibXSLT_UpdateParam   				            */
/*                                                                          */
/*! 
*   \_en
*   Update the parameter of the provider
*   
*   @param  r               Embperl request record
*   @param  pProvider       Provider record
*   @param  pParam          Parameter Hash
*                               param        hash with parameter 
*   @param  pKey            Key to which string should be appended
*   @return                 error code
*   \endif                                                                       
*
*   \_de									   
*   Aktualisiert die Parameter des Providers
*   
*   @param  r               Embperl request record
*   @param  pProvider       Provider record
*   @param  pParam          Parameter Hash
*                               param        hash mit parametern 
*   @param  pKey            Schl�ssel zu welchem hinzugef�gt wird
*   @return                 Fehlercode
*   \endif                                                                       
*                                                                          
* ------------------------------------------------------------------------ */

static int ProviderLibXSLT_UpdateParam(/*in*/ req *              r,
                                   /*in*/ tProvider *        pProvider,
                                   /*in*/ HV *               pParam)
    {
    int		    rc ;
    SV           *  pSrc ;
    HV *	    pParamHV ;
    HE *	    pEntry ;
    char *	    pKey ;
    SV *            pValue ;
    STRLEN          len ;
    IV		    l ;
    int		    n ;
    const char * *  pParamArray ;
    
    if ((rc = GetHashValueHREF  (r, pParam, "param", &pParamHV)) != ok)
        {
        if (!r->errdat1[0])
            strncpy (r -> errdat1, pProvider -> pCache -> sKey, sizeof (r -> errdat1) -1 ) ;
        return rc ;
        }

    if (((tProviderLibXSLT *)pProvider) -> pParamArray)
	{
	free (((tProviderLibXSLT *)pProvider) -> pParamArray) ;
	((tProviderLibXSLT *)pProvider) -> pParamArray = NULL ;
	}

    if (pParamHV)
	{
	n = 0 ;
	hv_iterinit (pParamHV) ;
	while ((pEntry = hv_iternext (pParamHV)))
	    {
	    n++ ;
	    }
        
	if (!(pParamArray = malloc(sizeof (const char *) * (n + 1) * 2)))
	    return rcOutOfMemory ;

	n = 0 ;
	hv_iterinit (pParamHV) ;
	while ((pEntry = hv_iternext (pParamHV)))
	    {
	    pKey     = hv_iterkey (pEntry, &l) ;
	    pValue   = hv_iterval (pParamHV, pEntry) ;
	    pParamArray[n++] = pKey ;
	    pParamArray[n++] = SvPV (pValue, len) ;
	    }
	pParamArray[n++] = NULL ;
	((tProviderLibXSLT *)pProvider) -> pParamArray = pParamArray ;
	}
    return ok ;
    }


/* ------------------------------------------------------------------------ */
/*                                                                          */
/* ProviderLibXSLT_GetContentSV	  				            */
/*                                                                          */
/*! 
*   \_en
*   Get the whole content from the provider. 
*   This gets the stylesheet and compiles it
*   
*   @param  r               Embperl request record
*   @param  pProvider       The provider record
*   @param  pData           Returns the content
*   @return                 error code
*   \endif                                                                       
*
*   \_de									   
*   Holt den gesamt Inhalt vom Provider.
*   Die Funktion holt sich das Stylesheet und kompiliert es
*   
*   @param  r               Embperl request record
*   @param  pProvider       The provider record
*   @param  pData           Liefert den Inhalt
*   @return                 Fehlercode
*   \endif                                                                       
*                                                                          
* ------------------------------------------------------------------------ */



static int ProviderLibXSLT_GetContentSV         (/*in*/ req *            r,
                                        /*in*/ tProvider *      pProvider,
                                        /*in*/ SV * *           pData)

    {
    int    rc ;
    xsltStylesheetPtr cur ;
    xmlDocPtr	    doc ;
    xmlDocPtr	    res;
    xmlOutputBufferPtr obuf ;
    
    
    tCacheItem * pSrcCache = Cache_GetDependency(r, pProvider -> pCache, 0) ;
    tCacheItem * pXSLCache = Cache_GetDependency(r, pProvider -> pCache, 1) ;

    if ((rc = Cache_GetContentPtr  (r, pSrcCache, (void * *)&doc)) != ok)
        return rc ;

    if ((rc = Cache_GetContentPtr (r, pXSLCache, (void * *)&cur)) != ok)
        return rc ;

    if (((tProviderLibXSLT *)pProvider) -> pOutputSV)
        SvREFCNT_dec (((tProviderLibXSLT *)pProvider) -> pOutputSV) ;

    ((tProviderLibXSLT *)pProvider) -> pOutputSV = newSVpv("", 0) ;

    res = xsltApplyStylesheet(cur, doc, ((tProviderLibXSLT *)pProvider) -> pParamArray);
    if(res == NULL)
	{
	strncpy (r -> errdat1, "XSLT", sizeof (r -> errdat1)) ;
	return rcLibXSLTError ;
	}
    
    obuf = xmlOutputBufferCreateIO (ProviderLibXSLT_iowrite, NULL, pProvider, NULL) ;
    
    xsltSaveResultTo(obuf, res, cur);

    xmlFreeDoc(res);
    xmlOutputBufferClose (obuf) ;

    *pData = ((tProviderLibXSLT *)pProvider) -> pOutputSV ;
    SvREFCNT_inc(*pData) ;

    return ok ;
    }

/* ------------------------------------------------------------------------ */
/*                                                                          */
/* ProviderLibXSLT_FreeContent 		                            */
/*                                                                          */
/*! 
*   \_en
*   Free the cached data
*   
*   @param  r               Embperl request record
*   @param  pProvider       The provider record
*   @return                 error code
*   \endif                                                                       
*
*   \_de									   
*   Gibt die gecachten Daten frei
*   
*   @param  r               Embperl request record
*   @param  pProvider       The provider record
*   @return                 Fehlercode
*   \endif                                                                       
*                                                                          
* ------------------------------------------------------------------------ */



static int ProviderLibXSLT_FreeContent(/*in*/ req *             r,
                                 /*in*/ tCacheItem * pItem)

    {
    tProviderLibXSLT * pProvider = ((tProviderLibXSLT *)pItem -> pProvider) ;
    
    if (pProvider -> pOutputSV)
	{
	SvREFCNT_dec (pProvider -> pOutputSV) ;
	pProvider -> pOutputSV = NULL ;
	}
    
    if (pProvider -> pParamArray)
	{
	free (pProvider -> pParamArray) ;
	pProvider -> pParamArray = NULL ;
	}

    return ok ;
    }

/* ------------------------------------------------------------------------ */

static tProviderClass ProviderClassLibXSLT = 
    {   
    "text/*", 
    &ProviderLibXSLT_New, 
    &ProviderLibXSLT_AppendKey, 
    &ProviderLibXSLT_UpdateParam, 
    &ProviderLibXSLT_GetContentSV,
    NULL,
    NULL,
    &ProviderLibXSLT_FreeContent,
    NULL,
    } ;



/* ------------------------------------------------------------------------ */

int embperl_LibXSLT_Init ()
    {
    Cache_AddProviderClass ("libxslt-compile-xsl", &ProviderClassLibXSLTXSL) ;
    Cache_AddProviderClass ("libxslt-parse-xml", &ProviderClassLibXSLTXML) ;
    Cache_AddProviderClass ("libxslt", &ProviderClassLibXSLT) ;
    }



