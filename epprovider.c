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
#   $Id: epprovider.c,v 1.1.2.13 2001/11/16 11:29:02 richter Exp $
#
###################################################################################*/


#include "ep.h"
#include "epmacro.h"



/* ------------------------------------------------------------------------ */
/*                                                                          */
/* Provider_New      					                    */
/*                                                                          */
/*! 
*   \_en
*   Creates a provider. 
*
*   @note   This function should not be called directly, but from another
*           ProviderXXX_New function
*   
*   @param  r               Embperl request record
*   @param  nSize           Size of provider struct
*   @param  pItem           CacheItem which holds the output of the provider
*   @param  pProviderClass  Provider class record
*                               source      Sourcetext provider
*   @param  pParam          Parameter Hash
*   @return                 error code
*   \endif                                                                       
*
*   \_de									   
*   Erzeugt einen neuen Provider.
*
*   @note   Diese Funktion sollte nicht direkt aufgerufen werden, sondern
*           von einer anderen ProviderXXX_New Funktion aus
*  
*   @param  r               Embperl request record
*   @param  nSize           Größer der provider struct
*   @param  pItem           CacheItem welches die Ausgabe des Providers 
*                           speichert
*   @param  pProviderClass  Provider class record
*   @param  pParam          Parameter Hash
*                               source      Quellentext provider
*   @return                 Fehlercode
*   \endif                                                                       
*                                                                          
* ------------------------------------------------------------------------ */

int Provider_New            (/*in*/ req *              r,
                             /*in*/ size_t             nSize,
                             /*in*/ tCacheItem *       pItem,
                             /*in*/ tProviderClass *   pProviderClass,
                             /*in*/ HV *               pParam)


    {
    tProvider *     pNew = (tProvider *)cache_malloc (r, nSize) ;
    
    if (!pNew)
        return rcOutOfMemory ;

    memset (pNew, 0, nSize) ;

    pNew -> pCache             = pItem ;
    pNew -> pProviderClass     = pProviderClass ;
    pNew -> sOutputType        = pProviderClass -> sOutputType ;

    pItem -> pProvider = (tProvider *)pNew ;

    return ok ;
    }

/* ------------------------------------------------------------------------ */
/*                                                                          */
/* Provider_AddDependOne				                    */
/*                                                                          */
/*! 
*   \_en
*   Adds another dependency provider to a new provider. If only a string
*   is given for the dependend provider the fMatch functions of the
*   provider classes are called until a provider class is found.  
*
*   @note   This function should not be called directly, but from another
*           ProviderXXX_New function
*   
*   @param  r               Embperl request record
*   @param  nSize           Size of provider struct
*   @param  sSourceName     Name of the element in pParam that holds the source
*   @param  pItem           CacheItem which holds the output of the provider
*   @param  pProviderClass  Provider class record
*                               source      Sourcetext provider
*   @param  pParam          Parameter Hash
*   @return                 error code
*   \endif                                                                       
*
*   \_de									   
*   Fügt einen neuen Abhänigkeit für einen neuen Provider.
*   Wird nur eine Zeichenkette als Abhänigkeit übergeben, werden der Reihe
*   nach die fMatch Funktionen der Providerklassen aufgerufen, bis eine
*   passende Klasse gefunden wurde.
*
*   @note   Diese Funktion sollte nicht direkt aufgerufen werden, sondern
*           von einer anderen ProviderXXX_New Funktion aus
*  
*   @param  r               Embperl request record
*   @param  nSize           Größer der provider struct
*   @param  sSourceName     Name des Elements in pParam welches die Quelle enthält
*   @param  pItem           CacheItem welches die Ausgabe des Providers 
*                           speichert
*   @param  pProviderClass  Provider class record
*   @param  pParam          Parameter Hash
*                               source      Quellentext provider
*   @return                 Fehlercode
*   \endif                                                                       
*                                                                          
* ------------------------------------------------------------------------ */

int Provider_AddDependOne   (/*in*/ req *              r,
                             /*in*/ tProvider *        pProvider,
                             /*in*/ const char *       sSourceName,
                             /*in*/ tCacheItem *       pItem,
                             /*in*/ tProviderClass *   pProviderClass,
                             /*in*/ HV *               pParam)


    {
    int          rc ;
    HV *         pSourceParam ;
    char *       sSource ;
    
    tCacheItem * pSubCache  ;
    
    GetHashValueStrOrHash (pParam, sSourceName, &sSource, &pSourceParam) ;

    if (sSource)
        {
        pSourceParam = (HV *)SvRV(sv_2mortal (CreateHashRef (
            "provider", hashtsv, CreateHashRef (
                "type", hashtstr, "file",
                "filename", hashtstr, sSource,
                NULL),
            "cache", hashtint, 0,
            NULL))) ;
        }

    if (pSourceParam)
        {
        if ((rc = Cache_New (r, pSourceParam, &pSubCache)) != ok)
            return rc ;

        if ((rc = Cache_AddDependency (r, pItem, pSubCache)) != ok)
            return rc ;
    
        }
    else
        {
        strcpy (r -> errdat1, pItem -> sKey) ;
        strcpy (r -> errdat2, sSourceName) ;
        return rcMissingParam ;
        }

    return ok ;
    }

/* ------------------------------------------------------------------------ */
/*                                                                          */
/* Provider_NewDependOne				                    */
/*                                                                          */
/*! 
*   \_en
*   Creates a provider which depends on another provider. If only a string
*   is given for the dependend provider the fMatch functions of the
*   provider classes are called until a provider class is found.  
*
*   @note   This function should not be called directly, but from another
*           ProviderXXX_New function
*   
*   @param  r               Embperl request record
*   @param  nSize           Size of provider struct
*   @param  sSourceName     Name of the element in pParam that holds the source
*   @param  pItem           CacheItem which holds the output of the provider
*   @param  pProviderClass  Provider class record
*                               source      Sourcetext provider
*   @param  pParam          Parameter Hash
*   @return                 error code
*   \endif                                                                       
*
*   \_de									   
*   Erzeugt einen neuen Provider der von einem anderem Provider abhängt.
*   Wird nur eine Zeichenkette als Abhänigkeit übergeben, werden der Reihe
*   nach die fMatch Funktionen der Providerklassen aufgerufen, bis eine
*   passende Klasse gefunden wurde.
*
*   @note   Diese Funktion sollte nicht direkt aufgerufen werden, sondern
*           von einer anderen ProviderXXX_New Funktion aus
*  
*   @param  r               Embperl request record
*   @param  nSize           Größer der provider struct
*   @param  sSourceName     Name des Elements in pParam welches die Quelle enthält
*   @param  pItem           CacheItem welches die Ausgabe des Providers 
*                           speichert
*   @param  pProviderClass  Provider class record
*   @param  pParam          Parameter Hash
*                               source      Quellentext provider
*   @return                 Fehlercode
*   \endif                                                                       
*                                                                          
* ------------------------------------------------------------------------ */

int Provider_NewDependOne   (/*in*/ req *              r,
                             /*in*/ size_t             nSize,
                             /*in*/ const char *       sSourceName,
                             /*in*/ tCacheItem *       pItem,
                             /*in*/ tProviderClass *   pProviderClass,
                             /*in*/ HV *               pParam)


    {
    int          rc ;
    
    if ((rc = Provider_New (r, nSize, pItem, pProviderClass, pParam)) != ok)
        return rc ;

    if ((rc = Provider_AddDependOne (r, pItem -> pProvider, sSourceName, pItem, pProviderClass, pParam)) != ok)
        return rc ;

    return ok ;
    }




/* ------------------------------------------------------------------------ */
/*                                                                          */
/*!         Provider that reads input from file                             */
/*                                                                          */

typedef struct tProviderFile
    {
    tProvider           Provider ;
    const char *        sFilename ;         /**< Filename */
    } tProviderFile ;


/* ------------------------------------------------------------------------ */
/*                                                                          */
/* ProviderFile_New      					            */
/*                                                                          */
/*! 
*   \_en
*   Creates a new file provider and fills it with data from the hash pParam
*   The resulting provider is put into the cache structure
*   
*   @param  r               Embperl request record
*   @param  pItem           CacheItem which holds the output of the provider
*   @param  pProviderClass  Provider class record
*   @param  pParam          Parameter Hash
*                               filename        name of file
*   @return                 error code
*   \endif                                                                       
*
*   \_de									   
*   Erzeugt einen neue Provider der daten aus Dateien ließt. Der ein Zeiger
*   auf den resultierenden Provider wird in die Cachestrutr eingefügt
*   
*   @param  r               Embperl request record
*   @param  pItem           CacheItem welches die Ausgabe des Providers 
*                           speichert
*   @param  pProviderClass  Provider class record
*   @param  pParam          Parameter Hash
*                               filename        Dateiname
*   @return                 Fehlercode
*   \endif                                                                       
*                                                                          
* ------------------------------------------------------------------------ */

static int ProviderFile_New (/*in*/ req *              r,
                             /*in*/ tCacheItem *       pItem,
                             /*in*/ tProviderClass *   pProviderClass,
                             /*in*/ HV *               pParam)


    {
    int          rc ;
    tProviderFile * pNew  ;
    const char * sFilename  ;
    const char * sDir = NULL ;
    
    if ((rc = Provider_New (r, sizeof(tProviderFile), pItem, pProviderClass, pParam)) != ok)
        return rc ;

    pNew = (tProviderFile *)pItem -> pProvider ;

    sFilename = GetHashValueStr  (pParam, "filename", "") ;
    /* is it a relative filename? -> append path */
    if (!(sFilename[0] == '/'  
#ifdef WIN32
        ||
        sFilename[0] == '\\' || 
            (isalpha(sFilename[0]) && sFilename[1] == ':' && 
	      (sFilename[2] == '\\' || sFilename[2] == '/')) 
#endif                  
        ))            
        {
        int l = strlen (sFilename) + strlen (r -> sCWD) + 2 ;
        
        pNew -> sFilename                   = malloc (l) ;
        strcpy ((char *)pNew -> sFilename, r -> sCWD) ;
        strcat ((char *)pNew -> sFilename, "/") ;
        strcat ((char *)pNew -> sFilename, sFilename) ;
        }
    else
        pNew -> sFilename                   = strdup (sFilename) ;
    pItem -> sExpiresFilename           = strdup (pNew -> sFilename) ;

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
*   The file provider appends the filename
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
*   Hängt ein eigenen Schlüssel an den Schlüsselstring an. Wenn irgednwelche
*   Abhänigkeiten bestehen, muß Cache_AppendKey für alle Abhänigkeiten aufgerufen 
*   werden.
*   Der File Provider hängt den Dateinamen an.
*   
*   @param  r               Embperl request record
*   @param  pProviderClass  Provider class record
*   @param  pParam          Parameter Hash
*                               filename        Dateiname
*   @param  pKey            Schlüssel zu welchem hinzugefügt wird
*   @return                 Fehlercode
*   \endif                                                                       
*                                                                          
* ------------------------------------------------------------------------ */

static int ProviderFile_AppendKey (/*in*/ req *              r,
                                   /*in*/ tProviderClass *   pProviderClass,
                                   /*in*/ HV *               pParam,
                                   /*i/o*/ SV *              pKey)
    {
    const char * sFilename = GetHashValueStr  (pParam, "filename", "") ;
    const char * sDir = "" ;

    /* is it a relative filename? -> append path */
    if (!(sFilename[0] == '/'  
#ifdef WIN32
        ||
        sFilename[0] == '\\' || 
            (isalpha(sFilename[0]) && sFilename[1] == ':' && 
	      (sFilename[2] == '\\' || sFilename[2] == '/')) 
#endif                  
        ))            
        {
        sDir = r -> sCWD ;
        }
    
    sv_catpvf (pKey, "*file:%s%s%s", sDir, *sDir?"/":"", sFilename) ;
    return ok ;
    }


/* ------------------------------------------------------------------------ */
/*                                                                          */
/* ProviderFile_GetContentSV   					            */
/*                                                                          */
/*! 
*   \_en
*   Get the whole content from the provider. 
*   The file provider reads the whole file into memory
*   
*   @param  r               Embperl request record
*   @param  pProvider       The provider record
*   @param  pData           Returns the content
*   @return                 error code
*   \endif                                                                       
*
*   \_de									   
*   Holt den gesamt Inhalt vom Provider.
*   Der File Provider ließt die komplette Datei.
*   
*   @param  r               Embperl request record
*   @param  pProvider       The provider record
*   @param  pData           Liefert den Inhalt
*   @return                 Fehlercode
*   \endif                                                                       
*                                                                          
* ------------------------------------------------------------------------ */



static int ProviderFile_GetContentSV (/*in*/ req *             r,
                             /*in*/ tProvider *     pProvider,
                             /*in*/ SV * *              pData)

    {
    size_t nSize = pProvider -> pCache -> FileStat.st_size ;
    int rc = ReadHTML(r, (char *)((tProviderFile *)pProvider) -> sFilename, &nSize, pData) ;
    if (rc == ok)
        SvREFCNT_inc (*pData) ;
    return rc ;
    }


/* ------------------------------------------------------------------------ */


tProviderClass ProviderClassFile = 
    {   
    "text/*", 
    &ProviderFile_New, 
    &ProviderFile_AppendKey, 
    NULL,
    &ProviderFile_GetContentSV,
    NULL,
    NULL,
    NULL,
    } ;


/* ------------------------------------------------------------------------ */
/*                                                                          */
/*!         Provider that reads input from file                             */
/*                                                                          */

typedef struct tProviderMem
    {
    tProvider           Provider ;
    SV *                pSource ;           /**< Source */
    const char *        sName ;             /**< Name of memory provider */
    time_t              nLastModified ;     /**< Last modified */
    time_t              nLastModifiedWhileGet ;     /**< Last modified during last get */
    } tProviderMem ;


/* ------------------------------------------------------------------------ */
/*                                                                          */
/* ProviderMem_New      					            */
/*                                                                          */
/*! 
*   \_en
*   Creates a new file provider and fills it with data from the hash pParam
*   The resulting provider is put into the cache structure
*   
*   @param  r               Embperl request record
*   @param  pItem           CacheItem which holds the output of the provider
*   @param  pProviderClass  Provider class record
*   @param  pParam          Parameter Hash
*                               name          name (used to compare mtime)
*                               source        source text
*                               mtime         last modification time
*   @return                 error code
*   \endif                                                                       
*
*   \_de									   
*   Erzeugt einen neue Provider der daten aus Dateien ließt. Der ein Zeiger
*   auf den resultierenden Provider wird in die Cachestrutr eingefügt
*   
*   @param  r               Embperl request record
*   @param  pItem           CacheItem welches die Ausgabe des Providers 
*                           speichert
*   @param  pProviderClass  Provider class record
*   @param  pParam          Parameter Hash
*                               name        Name (wird benutzt um mtime zu vergelichen)
*                               source      Quellentext
*                               mtime       Zeitpunkt der letzten Änderung
*   @return                 Fehlercode
*   \endif                                                                       
*                                                                          
* ------------------------------------------------------------------------ */

static int ProviderMem_New (/*in*/ req *              r,
                             /*in*/ tCacheItem *       pItem,
                             /*in*/ tProviderClass *   pProviderClass,
                             /*in*/ HV *               pParam)


    {
    int          rc ;
    tProviderMem * pNew  ;
    SV           * pSrc ;
    
    if ((rc = Provider_New (r, sizeof(tProviderMem), pItem, pProviderClass, pParam)) != ok)
        return rc ;

    pNew = (tProviderMem *)pItem -> pProvider ;

    pNew -> sName                   = GetHashValueStrDup    (pParam, "name", NULL) ;
    pNew -> nLastModified           = GetHashValueUInt       (pParam, "mtime", 0) ;

    pSrc = GetHashValueSV     (pParam, "source") ;
    if (!pSrc)
        pNew -> pSource = NULL ;
    else if (SvROK(pSrc))
        pNew -> pSource = SvREFCNT_inc (SvRV(pSrc)) ;
    else
        pNew -> pSource = SvREFCNT_inc (pSrc) ;

    return ok ;
    }


/* ------------------------------------------------------------------------ */
/*                                                                          */
/* ProviderMem_AppendKey    					            */
/*                                                                          */
/*! 
*   \_en
*   Append it's key to the keystring. If it depends on anything it must 
*   call Cache_AppendKey for any dependency.
*   The file provider appends the filename
*   
*   @param  r               Embperl request record
*   @param  pProviderClass  Provider class record
*   @param  pParam          Parameter Hash
*                               name        name 
*   @param  pKey            Key to which string should be appended
*   @return                 error code
*   \endif                                                                       
*
*   \_de									   
*   Hängt ein eigenen Schlüssel an den Schlüsselstring an. Wenn irgednwelche
*   Abhänigkeiten bestehen, muß Cache_AppendKey für alle Abhänigkeiten aufgerufen 
*   werden.
*   Der File Provider hängt den Dateinamen an.
*   
*   @param  r               Embperl request record
*   @param  pProviderClass  Provider class record
*   @param  pParam          Parameter Hash
*                               name        name
*   @param  pKey            Schlüssel zu welchem hinzugefügt wird
*   @return                 Fehlercode
*   \endif                                                                       
*                                                                          
* ------------------------------------------------------------------------ */

static int ProviderMem_AppendKey (/*in*/ req *              r,
                                   /*in*/ tProviderClass *   pProviderClass,
                                   /*in*/ HV *               pParam,
                                   /*i/o*/ SV *              pKey)
    {
    STRLEN  l ;
    tCacheItem * pItem ;
    const char * sName = GetHashValueStr  (pParam, "name", "") ;
    sv_catpvf (pKey, "*memory:%s", sName) ;

    return ok ;
    }

/* ------------------------------------------------------------------------ */
/*                                                                          */
/* ProviderMem_UpdateParam    					            */
/*                                                                          */
/*! 
*   \_en
*   Update the parameter of the provider
*   
*   @param  r               Embperl request record
*   @param  pProvider       Provider record
*   @param  pParam          Parameter Hash
*                               name        name 
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
*                               name        name
*   @param  pKey            Schlüssel zu welchem hinzugefügt wird
*   @return                 Fehlercode
*   \endif                                                                       
*                                                                          
* ------------------------------------------------------------------------ */

static int ProviderMem_UpdateParam(/*in*/ req *              r,
                                   /*in*/ tProvider *        pProvider,
                                   /*in*/ HV *               pParam)
    {
    SV           * pSrc ;

    if (((tProviderMem *)pProvider) -> pSource)
        SvREFCNT_dec (((tProviderMem *)pProvider) -> pSource) ;

    ((tProviderMem *)pProvider) -> nLastModified           = GetHashValueUInt    (pParam, "mtime", 0) ;
    
    pSrc = GetHashValueSV     (pParam, "source") ;
    if (!pSrc)
        ((tProviderMem *)pProvider) -> pSource = NULL ;
    else if (SvROK(pSrc))
        ((tProviderMem *)pProvider) -> pSource = SvREFCNT_inc (SvRV(pSrc)) ;
    else
        ((tProviderMem *)pProvider) -> pSource = SvREFCNT_inc (pSrc) ;

    return ok ;
    }

/* ------------------------------------------------------------------------ */
/*                                                                          */
/* ProviderMem_GetContentSV   					            */
/*                                                                          */
/*! 
*   \_en
*   Get the whole content from the provider. 
*   The file provider reads the whole file into memory
*   
*   @param  r               Embperl request record
*   @param  pProvider       The provider record
*   @param  pData           Returns the content
*   @return                 error code
*   \endif                                                                       
*
*   \_de									   
*   Holt den gesamt Inhalt vom Provider.
*   Der File Provider ließt die komplette Datei.
*   
*   @param  r               Embperl request record
*   @param  pProvider       The provider record
*   @param  pData           Liefert den Inhalt
*   @return                 Fehlercode
*   \endif                                                                       
*                                                                          
* ------------------------------------------------------------------------ */



static int ProviderMem_GetContentSV (/*in*/ req *             r,
                             /*in*/ tProvider *     pProvider,
                             /*in*/ SV * *              pData)

    {
    ((tProviderMem *)pProvider) -> nLastModifiedWhileGet = ((tProviderMem *)pProvider) -> nLastModified ; 
    *pData = SvREFCNT_inc(((tProviderMem *)pProvider) -> pSource) ;
    return ok ;
    }

/* ------------------------------------------------------------------------ */
/*                                                                          */
/* ProviderMem_FreeContent 	                                    */
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



int ProviderMem_FreeContent(/*in*/ req *             r,
                                /*in*/ tCacheItem * pItem)

    {
    tProviderMem * pProvider = (tProviderMem *)(pItem -> pProvider) ;
    if (pItem -> pSVData && pProvider -> pSource)
        {
        SvREFCNT_dec (pProvider -> pSource) ;
        pProvider ->  pSource = NULL ;
        }

    return ok ;
    }

/* ------------------------------------------------------------------------ */
/*                                                                          */
/* ProviderMem_IsExpired    					            */
/*                                                                          */
/*! 
*   \_en
*   Check if content of provider is expired
*   
*   @param  r               Embperl request record
*   @param  pProvider       Provider 
*   @return                 TRUE if expired
*   \endif                                                                       
*
*   \_de									   
*   Prüft ob der Inhalt des Providers noch gültig ist.
*   
*   @param  r               Embperl request record
*   @param  pProvider       Provider 
*   @return                 WAHR wenn abgelaufen
*   \endif                                                                       
*                                                                          
* ------------------------------------------------------------------------ */

static int ProviderMem_IsExpired  (/*in*/ req *              r,
                                   /*in*/ tProvider *        pProvider)

    {
    return ((tProviderMem *)pProvider) -> nLastModified == 0 || ((tProviderMem *)pProvider) -> nLastModified != ((tProviderMem *)pProvider) -> nLastModifiedWhileGet ; 
    }


/* ------------------------------------------------------------------------ */


tProviderClass ProviderClassMem = 
    {   
    "text/*", 
    &ProviderMem_New, 
    &ProviderMem_AppendKey, 
    &ProviderMem_UpdateParam, 
    &ProviderMem_GetContentSV,
    NULL,
    NULL,
    &ProviderMem_FreeContent,
    &ProviderMem_IsExpired, 
    } ;


/* ------------------------------------------------------------------------ */
/*                                                                          */
/*!         Provider for Embperl parser                                     */
/*                                                                          */

typedef struct tProviderEpParse
    {
    tProvider           Provider ;
    } tProviderEpParse ;


/* ------------------------------------------------------------------------ */
/*                                                                          */
/* ProviderEpParse_New      					            */
/*                                                                          */
/*! 
*   \_en
*   Creates a new Embperl parser provider and fills it with data from the 
*   hash pParam. The resulting provider is put into the cache structure
*   
*   @param  r               Embperl request record
*   @param  pItem           CacheItem which holds the output of the provider
*   @param  pProviderClass  Provider class record
*                               source      Sourcetext provider
*   @param  pParam          Parameter Hash
*   @return                 error code
*   \endif                                                                       
*
*   \_de									   
*   Erzeugt einen neue Provider für den Embperl Parser. Der ein Zeiger
*   auf den resultierenden Provider wird in die Cachestrutr eingefügt
*   
*   @param  r               Embperl request record
*   @param  pItem           CacheItem welches die Ausgabe des Providers 
*                           speichert
*   @param  pProviderClass  Provider class record
*   @param  pParam          Parameter Hash
*                               source      Quellentext provider
*   @return                 Fehlercode
*   \endif                                                                       
*                                                                          
* ------------------------------------------------------------------------ */

static int ProviderEpParse_New (/*in*/ req *              r,
                             /*in*/ tCacheItem *       pItem,
                             /*in*/ tProviderClass *   pProviderClass,
                             /*in*/ HV *               pParam)


    {
    int          rc ;
    
    if ((rc = Provider_NewDependOne (r, sizeof(tProviderEpParse), "source", pItem, pProviderClass, pParam)) != ok)
        return rc ;

    pItem -> bCache = FALSE ; /* do not cache, because it's cached by the compiler */
    
    return ok ;
    }


/* ------------------------------------------------------------------------ */
/*                                                                          */
/* ProviderEpParse_AppendKey    					    */
/*                                                                          */
/*! 
*   \_en
*   Append it's key to the keystring. If it depends on anything it must 
*   call Cache_AppendKey for any dependency.
*   
*   @param  r               Embperl request record
*   @param  pProviderClass  Provider class record
*   @param  pParam          Parameter Hash
*                               source      Sourcetext
*   @param  pKey            Key to which string should be appended
*   @return                 error code
*   \endif                                                                       
*
*   \_de									   
*   Hängt ein eigenen Schlüssel an den Schlüsselstring an. Wenn irgednwelche
*   Abhänigkeiten bestehen, muß Cache_AppendKey für alle Abhänigkeiten aufgerufen 
*   werden.
*   
*   @param  r               Embperl request record
*   @param  pProviderClass  Provider class record
*   @param  pParam          Parameter Hash
*                               source      Quellentext
*   @param  pKey            Schlüssel zu welchem hinzugefügt wird
*   @return                 Fehlercode
*   \endif                                                                       
*                                                                          
* ------------------------------------------------------------------------ */

static int ProviderEpParse_AppendKey (/*in*/ req *              r,
                                   /*in*/ tProviderClass *   pProviderClass,
                                   /*in*/ HV *               pParam,
                                   /*i/o*/ SV *              pKey)
    {
    int          rc ;
    const char * sSyntax = GetHashValueStr  (pParam, "syntax", "") ;
    
    if ((rc = Cache_AppendKey (r, pParam, "source", pKey)) != ok)
        return rc;

    sv_catpvf (pKey, "*epparse:%s", sSyntax) ;
    return ok ;
    }


/* ------------------------------------------------------------------------ */
/*                                                                          */
/* ProviderEpParse_GetContentIndex				            */
/*                                                                          */
/*! 
*   \_en
*   Get the whole content from the provider. 
*   The Embperl parser provider parsers the source and generates a DomTree
*   
*   @param  r               Embperl request record
*   @param  pProvider       The provider record
*   @param  pData           Returns the content
*   @return                 error code
*   \endif                                                                       
*
*   \_de									   
*   Holt den gesamt Inhalt vom Provider.
*   Der Embperl Parser Provider parsest die Quelle und erzeugt einen DomTree
*   
*   @param  r               Embperl request record
*   @param  pProvider       The provider record
*   @param  pData           Liefert den Inhalt
*   @return                 Fehlercode
*   \endif                                                                       
*                                                                          
* ------------------------------------------------------------------------ */



static int ProviderEpParse_GetContentIndex (/*in*/ req *             r,
                                            /*in*/ tProvider *       pProvider,
                                            /*in*/ tIndex *          pData)

    {
    int    rc ;
    char * p ;
    STRLEN len ;
    SV *   pSource ;
    tCacheItem * pFileCache = Cache_GetDependency(r, pProvider -> pCache, 0) ;
    if ((rc = Cache_GetContentSV (r, pFileCache, &pSource)) != ok)
        return rc ;
        
    p   = SvPV (pSource, len) ;
    if ((rc =  embperl_Parse (r, p, len, pData)) != ok)
        return rc ;

    return ok ;
    }


/* ------------------------------------------------------------------------ */
/*                                                                          */
/* ProviderEpParse_FreeContent 		                                    */
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



int ProviderEpParse_FreeContent(/*in*/ req *             r,
                                /*in*/ tCacheItem * pItem)

    {
    tDomTree * pDomTree ;

    /*
    Do not delete, because it's same dom tree as compiler 
    if (pItem -> xData)
        return DomTree_delete (DomTree_self(pItem -> xData)) ;
    */
    return ok ;
    }

/* ------------------------------------------------------------------------ */


tProviderClass ProviderClassEpParse = 
    {   
    "X-Embperl/DomTree", 
    &ProviderEpParse_New, 
    &ProviderEpParse_AppendKey, 
    NULL,
    NULL,
    NULL,
    &ProviderEpParse_GetContentIndex,
    &ProviderEpParse_FreeContent,
    NULL,
    } ;




/* ------------------------------------------------------------------------ */
/*                                                                          */
/*!         Provider for Embperl compiler                                   */
/*                                                                          */

typedef struct tProviderEpCompile
    {
    tProvider           Provider ;
    SV *                pSVData ;
    } tProviderEpCompile ;


/* ------------------------------------------------------------------------ */
/*                                                                          */
/* ProviderEpCompile_New      					            */
/*                                                                          */
/*! 
*   \_en
*   Creates a new Embperl compile provider and fills it with data from the 
*   hash pParam. The resulting provider is put into the cache structure
*   
*   @param  r               Embperl request record
*   @param  pItem           CacheItem which holds the output of the provider
*   @param  pProviderClass  Provider class record
*                               source      Sourcetext provider
*   @param  pParam          Parameter Hash
*   @return                 error code
*   \endif                                                                       
*
*   \_de									   
*   Erzeugt einen neue Provider für den Embperl Compiler. Der ein Zeiger
*   auf den resultierenden Provider wird in die Cachestrutr eingefügt
*   
*   @param  r               Embperl request record
*   @param  pItem           CacheItem welches die Ausgabe des Providers 
*                           speichert
*   @param  pProviderClass  Provider class record
*   @param  pParam          Parameter Hash
*                               source      Quellentext provider
*   @return                 Fehlercode
*   \endif                                                                       
*                                                                          
* ------------------------------------------------------------------------ */

static int ProviderEpCompile_New (/*in*/ req *              r,
                             /*in*/ tCacheItem *       pItem,
                             /*in*/ tProviderClass *   pProviderClass,
                             /*in*/ HV *               pParam)


    {
    int          rc ;
    
    if ((rc = Provider_NewDependOne (r, sizeof(tProviderEpCompile), "source", pItem, pProviderClass, pParam)) != ok)
        return rc ;

    return ok ;
    }


/* ------------------------------------------------------------------------ */
/*                                                                          */
/* ProviderEpCompile_AppendKey    					    */
/*                                                                          */
/*! 
*   \_en
*   Append it's key to the keystring. If it depends on anything it must 
*   call Cache_AppendKey for any dependency.
*   
*   @param  r               Embperl request record
*   @param  pProviderClass  Provider class record
*   @param  pParam          Parameter Hash
*                               source      Sourcetext
*   @param  pKey            Key to which string should be appended
*   @return                 error code
*   \endif                                                                       
*
*   \_de									   
*   Hängt ein eigenen Schlüssel an den Schlüsselstring an. Wenn irgednwelche
*   Abhänigkeiten bestehen, muß Cache_AppendKey für alle Abhänigkeiten aufgerufen 
*   werden.
*   
*   @param  r               Embperl request record
*   @param  pProviderClass  Provider class record
*   @param  pParam          Parameter Hash
*                               source      Quellentext
*   @param  pKey            Schlüssel zu welchem hinzugefügt wird
*   @return                 Fehlercode
*   \endif                                                                       
*                                                                          
* ------------------------------------------------------------------------ */

static int ProviderEpCompile_AppendKey (/*in*/ req *              r,
                                   /*in*/ tProviderClass *   pProviderClass,
                                   /*in*/ HV *               pParam,
                                   /*i/o*/ SV *              pKey)
    {
    int          rc ;

    if ((rc = Cache_AppendKey (r, pParam, "source", pKey)) != ok)
        return rc;

    sv_catpv (pKey, "*epcompile") ;
    return ok ;
    }


/* ------------------------------------------------------------------------ */
/*                                                                          */
/* ProviderEpCompile_GetContentIndex				            */
/*                                                                          */
/*! 
*   \_en
*   Get the whole content from the provider. 
*   The Embperl compile provider compiles the source DomTRee and generates
*   a Perl programm and a compiled DomTRee
*   
*   @param  r               Embperl request record
*   @param  pProvider       The provider record
*   @param  pData           Returns the content
*   @return                 error code
*   \endif                                                                       
*
*   \_de									   
*   Holt den gesamt Inhalt vom Provider.
*   Der Embperl Compile Provider überstzes den Quellen DOmTree und erzeugt
*   ein Perlprogramm und einen übersetzten DomTree
*   
*   @param  r               Embperl request record
*   @param  pProvider       The provider record
*   @param  pData           Liefert den Inhalt
*   @return                 Fehlercode
*   \endif                                                                       
*                                                                          
* ------------------------------------------------------------------------ */



static int ProviderEpCompile_GetContentIndex (/*in*/ req *             r,
                                            /*in*/ tProvider *       pProvider,
                                            /*in*/ tIndex *          pData)

    {
    int     rc ;
    char *  p ;
    STRLEN  len ;
    tIndex  xSrcDomTree ;
    tCacheItem * pSrcCache ;
    SV *         pProg = NULL ;

    pSrcCache = Cache_GetDependency(r, pProvider -> pCache, 0) ;
    if ((rc = Cache_GetContentIndex (r, pSrcCache, &xSrcDomTree)) != ok)
        return rc ;
        
    if ((rc =  embperl_Compile (r, xSrcDomTree, pData, &pProg )) != ok)
        {
	((tProviderEpCompile *)pProvider) -> pSVData = NULL ;
	if (pProg)
	    SvREFCNT_dec (pProg) ;

	return rc ;
	}

    ((tProviderEpCompile *)pProvider) -> pSVData = pProg ;

    return ok ;
    }

/* ------------------------------------------------------------------------ */
/*                                                                          */
/* ProviderEpCompile_GetContentIndex				            */
/*                                                                          */
/*! 
*   \_en
*   Get the whole content from the provider. 
*   The Embperl compile provider compiles the source DomTRee and generates
*   a Perl programm and a compiled DomTRee
*   
*   @param  r               Embperl request record
*   @param  pProvider       The provider record
*   @param  pData           Returns the content
*   @return                 error code
*   \endif                                                                       
*
*   \_de									   
*   Holt den gesamt Inhalt vom Provider.
*   Der Embperl Compile Provider überstzes den Quellen DOmTree und erzeugt
*   ein Perlprogramm und einen übersetzten DomTree
*   
*   @param  r               Embperl request record
*   @param  pProvider       The provider record
*   @param  pData           Liefert den Inhalt
*   @return                 Fehlercode
*   \endif                                                                       
*                                                                          
* ------------------------------------------------------------------------ */



static int ProviderEpCompile_GetContentSV  (/*in*/ req *             r,
                                            /*in*/ tProvider *       pProvider,
                                            /*in*/ SV * *            pData)

    {
    *pData = SvREFCNT_inc (((tProviderEpCompile *)pProvider) -> pSVData) ;

    return ok ;
    }

/* ------------------------------------------------------------------------ */
/*                                                                          */
/* ProviderEpCompile_FreeContent 	                                    */
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



int ProviderEpCompile_FreeContent(/*in*/ req *             r,
                                /*in*/ tCacheItem * pItem)

    {
    tDomTree * pDomTree ;

    if (pItem -> xData)
        return DomTree_delete (DomTree_self(pItem -> xData)) ;

    return ok ;
    }

/* ------------------------------------------------------------------------ */


tProviderClass ProviderClassEpCompile = 
    {   
    "X-Embperl/DomTree", 
    &ProviderEpCompile_New, 
    &ProviderEpCompile_AppendKey, 
    NULL,
    &ProviderEpCompile_GetContentSV,
    NULL,
    &ProviderEpCompile_GetContentIndex,
    &ProviderEpCompile_FreeContent,
    NULL,
    } ;



/* ------------------------------------------------------------------------ */
/*                                                                          */
/*!         Provider for Embperl Executer                                   */
/*                                                                          */

typedef struct tProviderEpRun
    {
    tProvider           Provider ;
    } tProviderEpRun ;


/* ------------------------------------------------------------------------ */
/*                                                                          */
/* ProviderEpRun_New      					            */
/*                                                                          */
/*! 
*   \_en
*   Creates a new Embperl run provider and fills it with data from the 
*   hash pParam. The resulting provider is put into the cache structure
*   
*   @param  r               Embperl request record
*   @param  pItem           CacheItem which holds the output of the provider
*   @param  pProviderClass  Provider class record
*                               source      Sourcetext provider
*   @param  pParam          Parameter Hash
*   @return                 error code
*   \endif                                                                       
*
*   \_de									   
*   Erzeugt einen neue Provider für den Embperl Executer. Der ein Zeiger
*   auf den resultierenden Provider wird in die Cachestrutr eingefügt
*   
*   @param  r               Embperl request record
*   @param  pItem           CacheItem welches die Ausgabe des Providers 
*                           speichert
*   @param  pProviderClass  Provider class record
*   @param  pParam          Parameter Hash
*                               source      Quellentext provider
*   @return                 Fehlercode
*   \endif                                                                       
*                                                                          
* ------------------------------------------------------------------------ */

static int ProviderEpRun_New (/*in*/ req *              r,
                             /*in*/ tCacheItem *       pItem,
                             /*in*/ tProviderClass *   pProviderClass,
                             /*in*/ HV *               pParam)


    {
    int          rc ;
    
    if ((rc = Provider_NewDependOne (r, sizeof(tProviderEpRun), "source", pItem, pProviderClass, pParam)) != ok)
        return rc ;

    return ok ;
    }


/* ------------------------------------------------------------------------ */
/*                                                                          */
/* ProviderEpRun_AppendKey    					    */
/*                                                                          */
/*! 
*   \_en
*   Append it's key to the keystring. If it depends on anything it must 
*   call Cache_AppendKey for any dependency.
*   
*   @param  r               Embperl request record
*   @param  pProviderClass  Provider class record
*   @param  pParam          Parameter Hash
*                               source      Sourcetext
*                               cache_key   
*                               cache_key_options   
*                               cache_key_func   
*   @param  pKey            Key to which string should be appended
*   @return                 error code
*   \endif                                                                       
*
*   \_de									   
*   Hängt ein eigenen Schlüssel an den Schlüsselstring an. Wenn irgednwelche
*   Abhänigkeiten bestehen, muß Cache_AppendKey für alle Abhänigkeiten aufgerufen 
*   werden.
*   
*   @param  r               Embperl request record
*   @param  pProviderClass  Provider class record
*   @param  pParam          Parameter Hash
*                               source      Quellentext
*                               cache_key   
*                               cache_key_options   
*                               cache_key_func   
*   @param  pKey            Schlüssel zu welchem hinzugefügt wird
*   @return                 Fehlercode
*   \endif                                                                       
*                                                                          
* ------------------------------------------------------------------------ */

static int ProviderEpRun_AppendKey (/*in*/ req *              r,
                                   /*in*/ tProviderClass *   pProviderClass,
                                   /*in*/ HV *               pParam,
                                   /*i/o*/ SV *              pKey)
    {
    int          rc ;
    const char * sKey        = GetHashValueStr  (pParam, "cache_key", "") ;
    int          bKeyOptions = GetHashValueInt  (pParam, "cache_key_options", 15) ;
    CV *         pKeyCV ; 

    if ((rc = Cache_AppendKey (r, pParam, "source", pKey)) != ok)
        return rc;

    sv_catpv (pKey, "*eprun:") ;

    if ((rc = GetHashValueCREF   (r, pParam, "cache_key_func", &pKeyCV)) != ok)
        return rc ;
    
    if (pKeyCV)
	{
	SV * pRet ;

	if ((rc = CallCV (r, "CacheKey", pKeyCV, 0, &pRet)) != ok)
	    return rc ;

	if (pRet && SvOK(pRet))
	    sv_catsv (pKey, pRet) ;
	}
    
    if ((bKeyOptions & ckoptPathInfo) && r -> sPathInfo)
	sv_catpv (pKey, r -> sPathInfo) ;

    if ((bKeyOptions & ckoptQueryInfo) && r -> sQueryInfo)
	sv_catpv (pKey, r -> sQueryInfo) ;
    
    return ok ;
    }


/* ------------------------------------------------------------------------ */
/*                                                                          */
/* ProviderEpRun_IsExpired    					            */
/*                                                                          */
/*! 
*   \_en
*   Check if content of provider is expired
*   
*   @param  r               Embperl request record
*   @param  pProvider       Provider 
*   @return                 TRUE if expired
*   \endif                                                                       
*
*   \_de									   
*   Prüft ob der Inhalt des Providers noch gültig ist.
*   
*   @param  r               Embperl request record
*   @param  pProvider       Provider 
*   @return                 WAHR wenn abgelaufen
*   \endif                                                                       
*                                                                          
* ------------------------------------------------------------------------ */

static int ProviderEpRun_IsExpired  (/*in*/ req *              r,
                                   /*in*/ tProvider *        pProvider)

    {
    int rc ;
    bool bCache ;

    /* update cache parameters */
    if ((rc =  embperl_PreExecute (r, pProvider -> pCache)) != ok)
        {
        LogError (r, rc) ;
        }
    
    if (pProvider -> pCache -> nExpiresInTime || pProvider -> pCache -> pExpiresCV)
        pProvider -> pCache -> bCache = 1 ;
    else
        {
        pProvider -> pCache -> bCache = 0 ;
        if (bCache)
            Cache_FreeContent (r, pProvider -> pCache) ;
        }
        

    return FALSE ; 
    }

/* ------------------------------------------------------------------------ */
/*                                                                          */
/* ProviderEpRun_GetContentIndex				            */
/*                                                                          */
/*! 
*   \_en
*   Get the whole content from the provider. 
*   The Embperl Run provider executes the compiled DomTree & Perl programm
*   
*   @param  r               Embperl request record
*   @param  pProvider       The provider record
*   @param  pData           Returns the content
*   @return                 error code
*   \endif                                                                       
*
*   \_de									   
*   Holt den gesamt Inhalt vom Provider.
*   Der Embperl Run Provider führt den übersetzen DomTree und das Perlprogramm aus
*   
*   @param  r               Embperl request record
*   @param  pProvider       The provider record
*   @param  pData           Liefert den Inhalt
*   @return                 Fehlercode
*   \endif                                                                       
*                                                                          
* ------------------------------------------------------------------------ */



static int ProviderEpRun_GetContentIndex   (/*in*/ req *             r,
                                            /*in*/ tProvider *       pProvider,
                                            /*in*/ tIndex *          pData)

    {
    int         rc ;
    char *      p ;
    STRLEN      len ;
    tIndex      xSrcDomTree ;
    CV *        pCV ;

    tCacheItem * pSrcCache = Cache_GetDependency(r, pProvider -> pCache, 0) ;

    if ((rc = Cache_GetContentSvIndex (r, pSrcCache, (SV **)&pCV, &xSrcDomTree)) != ok)
        return rc ;
        
    if ((rc =  embperl_Execute (r, xSrcDomTree, pCV, pData)) != ok)
        return rc ;

    /* update cache parameter from source */
    ProviderEpRun_IsExpired  (r, pProvider) ;

    return ok ;
    }


/* ------------------------------------------------------------------------ */
/*                                                                          */
/* ProviderEpRun_FreeContent 	                                    */
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



int ProviderEpRun_FreeContent(/*in*/ req *             r,
                                /*in*/ tCacheItem * pItem)

    {
    tDomTree * pDomTree ;

    if (pItem -> xData)
        return DomTree_delete (DomTree_self(pItem -> xData)) ;

    return ok ;
    }

/* ------------------------------------------------------------------------ */


tProviderClass ProviderClassEpRun = 
    {   
    "X-Embperl/DomTree", 
    &ProviderEpRun_New, 
    &ProviderEpRun_AppendKey, 
    NULL,
    NULL,
    NULL,
    &ProviderEpRun_GetContentIndex,
    &ProviderEpRun_FreeContent,
    &ProviderEpRun_IsExpired,
    } ;



/* ------------------------------------------------------------------------ */
/*                                                                          */
/*!         Provider for Embperl DomTree to String converter                */
/*                                                                          */

typedef struct tProviderEpToString
    {
    tProvider           Provider ;
    } tProviderEpToString ;


/* ------------------------------------------------------------------------ */
/*                                                                          */
/* ProviderEpToString_New      					            */
/*                                                                          */
/*! 
*   \_en
*   Creates a new DomTree to String provider and fills it with data from the 
*   hash pParam. The resulting provider is put into the cache structure
*   
*   @param  r               Embperl request record
*   @param  pItem           CacheItem which holds the output of the provider
*   @param  pProviderClass  Provider class record
*                               source      Sourcetext provider
*   @param  pParam          Parameter Hash
*   @return                 error code
*   \endif                                                                       
*
*   \_de									   
*   Erzeugt einen neue Provider für den Embperl zu Textkonverter. Der ein Zeiger
*   auf den resultierenden Provider wird in die Cachestrutr eingefügt
*   
*   @param  r               Embperl request record
*   @param  pItem           CacheItem welches die Ausgabe des Providers 
*                           speichert
*   @param  pProviderClass  Provider class record
*   @param  pParam          Parameter Hash
*                               source      Quellentext provider
*   @return                 Fehlercode
*   \endif                                                                       
*                                                                          
* ------------------------------------------------------------------------ */

static int ProviderEpToString_New (/*in*/ req *              r,
                             /*in*/ tCacheItem *       pItem,
                             /*in*/ tProviderClass *   pProviderClass,
                             /*in*/ HV *               pParam)


    {
    int          rc ;
    
    if ((rc = Provider_NewDependOne (r, sizeof(tProviderEpToString), "source", pItem, pProviderClass, pParam)) != ok)
        return rc ;

    return ok ;
    }


/* ------------------------------------------------------------------------ */
/*                                                                          */
/* ProviderEpToString_AppendKey    					    */
/*                                                                          */
/*! 
*   \_en
*   Append it's key to the keystring. If it depends on anything it must 
*   call Cache_AppendKey for any dependency.
*   
*   @param  r               Embperl request record
*   @param  pProviderClass  Provider class record
*   @param  pParam          Parameter Hash
*                               source      Sourcetext
*                               cache_key   
*                               cache_key_options   
*                               cache_key_func   
*   @param  pKey            Key to which string should be appended
*   @return                 error code
*   \endif                                                                       
*
*   \_de									   
*   Hängt ein eigenen Schlüssel an den Schlüsselstring an. Wenn irgednwelche
*   Abhänigkeiten bestehen, muß Cache_AppendKey für alle Abhänigkeiten aufgerufen 
*   werden.
*   
*   @param  r               Embperl request record
*   @param  pProviderClass  Provider class record
*   @param  pParam          Parameter Hash
*                               source      Quellentext
*                               cache_key   
*                               cache_key_options   
*                               cache_key_func   
*   @param  pKey            Schlüssel zu welchem hinzugefügt wird
*   @return                 Fehlercode
*   \endif                                                                       
*                                                                          
* ------------------------------------------------------------------------ */

static int ProviderEpToString_AppendKey (/*in*/ req *              r,
                                   /*in*/ tProviderClass *   pProviderClass,
                                   /*in*/ HV *               pParam,
                                   /*i/o*/ SV *              pKey)
    {
    int          rc ;

    if ((rc = Cache_AppendKey (r, pParam, "source", pKey)) != ok)
        return rc;

    sv_catpv (pKey, "*eptostring") ;
    return ok ;
    }


/* ------------------------------------------------------------------------ */
/*                                                                          */
/* ProviderEpToString_GetContentIndex				            */
/*                                                                          */
/*! 
*   \_en
*   Get the whole content from the provider. 
*   The Embperl parser provider parsers the source and generates a DomTree
*   
*   @param  r               Embperl request record
*   @param  pProvider       The provider record
*   @param  pData           Returns the content
*   @return                 error code
*   \endif                                                                       
*
*   \_de									   
*   Holt den gesamt Inhalt vom Provider.
*   Der Embperl Parser Provider parsest die Quelle und erzeugt einen DomTree
*   
*   @param  r               Embperl request record
*   @param  pProvider       The provider record
*   @param  pData           Liefert den Inhalt
*   @return                 Fehlercode
*   \endif                                                                       
*                                                                          
* ------------------------------------------------------------------------ */



static int ProviderEpToString_GetContentSV (/*in*/ req *             r,
                                            /*in*/ tProvider *       pProvider,
                                            /*in*/ SV * *            pData)

    {
    int     rc ;
    char *  p ;
    STRLEN  len ;
    tIndex  xSrcDomTree ;
    tCacheItem * pSrcCache ;
    SV * pOut ;
    char * pBuf ;
    tDomTree * pDomTree ;


    pSrcCache = Cache_GetDependency(r, pProvider -> pCache, 0) ;
    if ((rc = Cache_GetContentIndex (r, pSrcCache, &xSrcDomTree)) != ok)
        return rc ;

    if (xSrcDomTree == 0)
	{
	strncpy (r -> errdat1, "EpToString source", sizeof (r -> errdat1)) ;
	return rcMissingInput ;
	}

        
    oRollbackOutput (r, NULL) ;
    oBegin (r) ;
    pDomTree = DomTree_self (xSrcDomTree) ;
    Node_toString (r, pDomTree, pDomTree -> xDocument, 0) ;

    pOut = newSV (1) ;
    len = GetContentLength (r) + 1 ;
    
    SvGROW (pOut, len) ;
    pBuf = SvPVX (pOut) ;
    oCommitToMem (r, NULL, pBuf) ;
    oRollbackOutput (r, NULL) ;
    SvCUR_set (pOut, len - 1) ;
    SvPOK_on (pOut) ;

    *pData = pOut ;

    return ok ;
    }


/* ------------------------------------------------------------------------ */
/*                                                                          */
/* ProviderEpToString_FreeContent 	                                    */
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



int ProviderEpToString_FreeContent(/*in*/ req *             r,
                                /*in*/ tCacheItem * pItem)

    {
    tDomTree * pDomTree ;

    if (pItem -> xData)
        return DomTree_delete (DomTree_self(pItem -> xData)) ;

    return ok ;
    }

/* ------------------------------------------------------------------------ */


tProviderClass ProviderClassEpToString = 
    {   
    "text/*", 
    &ProviderEpToString_New, 
    &ProviderEpToString_AppendKey, 
    NULL,
    &ProviderEpToString_GetContentSV,
    NULL,
    NULL,
    &ProviderEpToString_FreeContent,
    NULL,
    } ;



/* ------------------------------------------------------------------------ */
/*                                                                          */
/* Provider_Init      					                    */
/*                                                                          */
/*! 
*   \_en
*   Register all the providers
*   @return                 error code
*   
*   \endif                                                                       
*
*   \_de									   
*   Provider registrieren
*   @return                 Fehlercode
*   
*   \endif                                                                       
*                                                                          
* ------------------------------------------------------------------------ */

int Provider_Init (void)

    {
    Cache_AddProviderClass ("file",      &ProviderClassFile) ;
    Cache_AddProviderClass ("memory",    &ProviderClassMem) ;
    Cache_AddProviderClass ("epparse",   &ProviderClassEpParse) ;
    Cache_AddProviderClass ("epcompile", &ProviderClassEpCompile) ;
    Cache_AddProviderClass ("eprun",     &ProviderClassEpRun) ;
    Cache_AddProviderClass ("eptostring",&ProviderClassEpToString) ;

    return ok ;
    }

