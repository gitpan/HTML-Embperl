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
#   $Id: epcache.c,v 1.1.2.16 2001/11/27 09:35:58 richter Exp $
#
###################################################################################*/

#include "ep.h"
#include "epmacro.h"




HV * pProviders ;       /**< global hash that holds all known providers classes */
HV * pCacheItems ;      /**< hash which contains all CacheItems by Key */
tCacheItem * * pCachesToRelease = NULL ;





/* ------------------------------------------------------------------------ */
/*                                                                          */
/* Cache_AddProviderClass				                    */
/*                                                                          */
/*! 
*   \_en
*   Add a provider class to list of known providers
*   @param  sName           Name of the Providerclass
*   @param  pProviderClass  Provider class record
*   @return                 error code
*   
*   \endif                                                                       
*
*   \_de									   
*   Fügt eine Providerklasse den der Liste der bekannten Providern hinzu
*   @param  sName           Name der Providerklasse
*   @param  pProviderClass  Provider class record
*   @return                 Fehlercode
*   
*   \endif                                                                       
*                                                                          
* ------------------------------------------------------------------------ */

int Cache_AddProviderClass (/*in*/ const char *     sName,
                            /*in*/ tProviderClass * pClass)

    {
    SetHashValueInt (pProviders, sName, (IV)pClass) ;
    return ok ;
    }

/* ------------------------------------------------------------------------ */
/*                                                                          */
/* Cache_Init      					                    */
/*                                                                          */
/*! 
*   \_en
*   Do global initialization of cache system
*   @return                 error code
*   
*   \endif                                                                       
*
*   \_de									   
*   Führt die globale Initialisierung des Cachesystems durch
*   @return                 Fehlercode
*   
*   \endif                                                                       
*                                                                          
* ------------------------------------------------------------------------ */

int Cache_Init (void)

    {
    pProviders  = newHV () ;
    pCacheItems = newHV () ;

    ArrayNew (&pCachesToRelease, 16, sizeof (tCacheItem *)) ;
    return ok ;
    }


/* ------------------------------------------------------------------------ */
/*                                                                          */
/* Cache_CleanupRequest  				                    */
/*                                                                          */
/*! 
*   \_en
*   Do cleanup at end of request
*   @param  r               Embperl request record
*   @return                 error code
*   
*   \endif                                                                       
*
*   \_de									   
*   Führt die Aufräumarbeiten am Requestende aus
*   @param  r               Embperl request record
*   @return                 Fehlercode
*   
*   \endif                                                                       
*                                                                          
* ------------------------------------------------------------------------ */

int Cache_CleanupRequest (req * r)

    {
    int n = ArrayGetSize (pCachesToRelease) ;
    int i ;

    for (i = 0; i < n; i++)
        Cache_FreeContent (r, pCachesToRelease[i]) ;

    ArraySetSize(&pCachesToRelease, 0) ;

    return ok ;
    }


/* ------------------------------------------------------------------------ */
/*                                                                          */
/* Cache_New      						            */
/*                                                                          */
/*! 
*   \_en
*   Checks if a CacheItem which matches the parameters already exists, if
*   not it creates a new CacheItem and fills it with data from the hash 
*   pParam
*   
*   @param  r               Embperl request record
*   @param  pParam          Parameter
*                               expires_in  number of seconds when the item 
*                                           expires, 0 = expires never
*                               expires_func    Perl Function (coderef) that
*                                               is called and item is expired
*                                               if it returns true
*                               expires_filename    item expires when modification
*                                                   time of file changes
*                               cache               set to zero to not cache the content
*                               provider            parameter for the provider 
*   @param  pItem           Return of the new Items
*   @return                 error code
*   \endif                                                                       
*
*   \_de									   
*   Prüft ob ein passendes CacheItem bereits vorhanden ist, wenn nicht 
*   erzeugt die Funktion ein neues CacheItem und füllte es mit den Daten aus 
*   pParam
*   
*   @param  r               Embperl request record
*   @param  pParam          Parameter
*                               expires_in  anzahl der Sekunden wenn Item
*                                           abläuft; 0 = nie
*                               expires_func    Perl Funktion (coderef) die
*                                               aufgerufen wird. Wenn sie wahr
*                                               zurückgibt ist das Item abgelaufen
*                               expires_filename    Item ist abgelaufen wenn 
*                                                   Dateidatum sich ändert
*                               cache               Auf Null setzen damit Inhalt
*                                                   nicht gecacht wird
*                               provider            parameter für Provider
*   @param  pItem           Rückgabe des neuen Items
*   @return                 Fehlercode
*   \endif                                                                       
*                                                                          
* ------------------------------------------------------------------------ */

int Cache_New (/*in*/ req *             r,
               /*in*/ HV *              pParam,
               /*in*/ tCacheItem * *    pItem)


    {
    int          rc ;
    HV *         pProviderParam ;
    char *       sProvider ;
    tProviderClass *  pProviderClass ;
    tCacheItem * pNew = NULL ;
    SV *         pKey = NULL ;
    const char * sKey = "" ;


    if ((rc = GetHashValueHREF  (r, pParam, "provider", &pProviderParam)) != ok)
        {
        return rc ;
        }
    if (pProviderParam)
        {
        STRLEN       len ;

        sProvider      = GetHashValueStr  (pProviderParam, "type", "") ;
        pProviderClass = (tProviderClass *)GetHashValuePtr (pProviders, sProvider, NULL) ;
        if (!pProviderClass)
            {
            if (*sProvider)
		strncpy (r -> errdat1, sProvider, sizeof(r -> errdat1) - 1) ;
	    else
		strncpy (r -> errdat1, "<provider missing>", sizeof(r -> errdat1) - 1) ;

            return rcUnknownProvider ;
            }
        pKey = newSVpv ("", 0) ;
        if (pProviderClass -> fAppendKey)
            if ((rc = (*pProviderClass -> fAppendKey)(r, pProviderClass, pProviderParam, pKey)) != ok)
                return rc ;

        sKey = SvPV(pKey, len) ;
        if (pNew = Cache_GetByKey (r, sKey))
            {
	    char * exfn ;

            pNew -> nExpiresInTime      = GetHashValueInt (pParam, "expires_in", 0) ;
            if (pNew -> pExpiresCV)
                SvREFCNT_dec (pNew -> pExpiresCV) ;
            if ((rc = GetHashValueCREF  (r, pParam, "expires_func", &pNew -> pExpiresCV)) != ok)
                return rc ;
            exfn = GetHashValueStrDup  (pParam, "expires_filename", NULL) ;
	    if (pNew -> sExpiresFilename)
		{
		if (exfn)
		    {
		    lprintf (r, "exfn=%s\n", exfn) ;
		    free ((void *)pNew -> sExpiresFilename) ;
		    pNew -> sExpiresFilename    = exfn ;
		    }
		}
	    else
		pNew -> sExpiresFilename    = exfn ;

            pNew -> bCache              = GetHashValueInt   (pParam, "cache", 1) ;

            if (r -> bDebug & dbgCache)
                lprintf (r, "[%d]CACHE: Update CacheItem %s; expires_in=%d expires_func=%s expires_filename=%s cache=%s\n",
                                   r -> nPid, pNew -> sKey, pNew -> nExpiresInTime,
                                   pNew -> pExpiresCV?"yes":"no", pNew -> sExpiresFilename?pNew -> sExpiresFilename:"",
                                   pNew -> bCache?"yes":"no") ; 

            if (pProviderClass -> fUpdateParam)
                if ((rc = (*pProviderClass -> fUpdateParam)(r, pNew -> pProvider, pProviderParam)) != ok)
                    return rc ;
            }        

        }

    if (!pNew)
        {
        pNew = cache_malloc (r, sizeof(tCacheItem)) ;
        if (!pNew)
            {
            if (pKey)
                SvREFCNT_dec (pKey) ;
            return rcOutOfMemory ;
            }

        *pItem = NULL ;
        memset (pNew, 0, sizeof (tCacheItem)) ;

        pNew -> nExpiresInTime      = GetHashValueInt (pParam, "expires_in", 0) ;
        if ((rc = GetHashValueCREF  (r, pParam, "expires_func", &pNew -> pExpiresCV)) != ok)
            return rc ;
        pNew -> sExpiresFilename    = GetHashValueStrDup  (pParam, "expires_filename", NULL) ;
        pNew -> bCache              = GetHashValueInt   (pParam, "cache", 1) ;
        pNew -> sKey                = strdup (sKey) ;

        if (pProviderParam)
            {
            tCacheItem * pSubItem ;
            int          i ;
            int	     numItems  ;

        
            if ((rc = (*pProviderClass -> fNew)(r, pNew, pProviderClass, pProviderParam)) != ok)
                {
                if (pKey)
                    SvREFCNT_dec (pKey) ;
                cache_free (r, pNew) ;
                return rc ;
                }
            }
        if (r -> bDebug & dbgCache)
            lprintf (r, "[%d]CACHE: Created new CacheItem %s; expires_in=%d expires_func=%s expires_filename=%s cache=%s\n",
                               r -> nPid, sKey, pNew -> nExpiresInTime,
                               pNew -> pExpiresCV?"yes":"no", pNew -> sExpiresFilename?pNew -> sExpiresFilename:"",
                               pNew -> bCache?"yes":"no") ; 
        SetHashValueInt (pCacheItems, sKey, (IV)pNew) ;
        }

    if (pKey)
        SvREFCNT_dec (pKey) ;
    *pItem = pNew ;

    return ok ;
    }


/* ------------------------------------------------------------------------ */
/*                                                                          */
/* Cache_AppendKey    					                    */
/*                                                                          */
/*! 
*   \_en
*   Append it's key to the keystring. If it depends on anything it must 
*   call Cache_AppendKey for any dependency.
*   The file provider appends the filename
*   
*   @param  r               Embperl request record
*   @param  pParam          Parameter Hash
*   @param  sSubProvider    sub provider parameter
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
*   @param  pParam          Parameter Hash
*   @param  sSubProvider    sub provider parameter
*   @param  pKey            Schlüssel zu welchem hinzugefügt wird
*   @return                 Fehlercode
*   \endif                                                                       
*                                                                          
* ------------------------------------------------------------------------ */

int Cache_AppendKey               (/*in*/ req *              r,
                                   /*in*/ HV *               pParam,
                                   /*in*/ const char *       sSubProvider, 
                                   /*i/o*/ SV *              pKey)
    {
    int  rc ;
    HV * pSubProvider ;
    HV * pProviderParam ;
    char *       sProvider ;
    tProviderClass *  pProviderClass ;

    
    if ((rc = GetHashValueHREF  (r, pParam, sSubProvider, &pSubProvider)) != ok)
        {
        STRLEN l ;
        if (!r->errdat1[0])
            strncpy (r -> errdat1, SvPV(pKey, l), sizeof (r -> errdat1) -1 ) ;
        return rc ;
        }
    if ((rc = GetHashValueHREF  (r, pSubProvider, "provider", &pProviderParam)) != ok)
        {
        STRLEN l ;
        if (!r->errdat1[0])
            strncpy (r -> errdat1, SvPV(pKey, l), sizeof (r -> errdat1) -1 ) ;
        return rc ;
        }
    if (pProviderParam)
        {
        STRLEN       len ;
        tCacheItem * pItem ;

        sProvider      = GetHashValueStr  (pProviderParam, "type", "") ;
        pProviderClass = (tProviderClass *)GetHashValuePtr (pProviders, sProvider, NULL) ;
        if (!pProviderClass)
            {
            if (*sProvider)
		strncpy (r -> errdat1, sProvider, sizeof(r -> errdat1) - 1) ;
	    else
		strncpy (r -> errdat1, "<provider missing>", sizeof(r -> errdat1) - 1) ;
            return rcUnknownProvider ;
            }
        if (pProviderClass -> fAppendKey)
            if ((rc = (*pProviderClass -> fAppendKey)(r, pProviderClass, pProviderParam, pKey)) != ok)
		{
		if (r -> bDebug & dbgCache)
		    lprintf (r, "[%d]CACHE: Error in Update CacheItem provider=%s\n",
		    r -> nPid,  sProvider) ;
                return rc ;
		}
        if (pItem = Cache_GetByKey (r, SvPV(pKey, len)))
            {
            int bCache = pItem -> bCache ;
	    char * exfn ;

            pItem -> nExpiresInTime      = GetHashValueInt (pSubProvider, "expires_in", 0) ;
            if (pItem -> pExpiresCV)
                SvREFCNT_dec (pItem -> pExpiresCV) ;
            if ((rc = GetHashValueCREF  (r, pSubProvider, "expires_func", &pItem -> pExpiresCV)) != ok)
                return rc ;
            exfn = GetHashValueStrDup  (pSubProvider, "expires_filename", NULL) ;
	    if (pItem -> sExpiresFilename)
		{
		if (exfn)
		    {
		    free ((void *)pItem -> sExpiresFilename) ;
		    pItem -> sExpiresFilename    = exfn ;
		    }
		}
	    else
		pItem -> sExpiresFilename    = exfn ;

            pItem -> bCache              = GetHashValueInt   (pSubProvider, "cache", 1) ;
            if (!pItem -> bCache && bCache)
                Cache_FreeContent (r, pItem) ;

            if (r -> bDebug & dbgCache)
                lprintf (r, "[%d]CACHE: Update CacheItem %s; expires_in=%d expires_func=%s expires_filename=%s cache=%s\n",
                                   r -> nPid, pItem -> sKey, pItem -> nExpiresInTime,
                                   pItem -> pExpiresCV?"yes":"no", pItem -> sExpiresFilename?pItem -> sExpiresFilename:"",
                                   pItem -> bCache?"yes":"no") ; 

            if (pProviderClass -> fUpdateParam)
                if ((rc = (*pProviderClass -> fUpdateParam)(r, pItem -> pProvider, pProviderParam)) != ok)
                    return rc ;
            }        
        }
    else
        sv_catpv (pKey, "-?") ;

    return ok ;
    }


/* ------------------------------------------------------------------------ */
/*                                                                          */
/* Cache_GetByKey  						            */
/*                                                                          */
/*! 
*   \_en
*   Gets an CacheItem by it's key.
*   
*   @param  r               Embperl request record
*   @param  sKey            Key
*   @return                 Returns the cache item specified by the key if found
*   \endif                                                                       
*
*   \_de									   
*   Liefert das durch den Schlüssel angegeben CacheItem zurück. 
*   
*   @param  r               Embperl request record
*   @param  sKey            Key
*   @return                 Liefert das CacheItem welches durch den Schlüssel
*                           angegeben wird, soweit gefunden.
*   \endif                                                                       
*                                                                          
* ------------------------------------------------------------------------ */

tCacheItem * Cache_GetByKey    (/*in*/ req *       r,
                                /*in*/ const char * sKey)

    {
    tCacheItem * pItem ;
    
    pItem = (tCacheItem *)GetHashValuePtr (pCacheItems, sKey, NULL) ;

    return pItem ;
    }



/* ------------------------------------------------------------------------ */
/*                                                                          */
/* Cache_AddDependency  						    */
/*                                                                          */
/*! 
*   \_en
*   Adds a CacheItem on which this cache items depends
*   
*   @param  r               Embperl request record
*   @param  pItem           CacheItem which depends on pDependsOn
*   @param  pDependsOn      CacheItem on which pItem depends
*   @return                 0 on success
*   \endif                                                                       
*
*   \_de									   
*   Fügt ein CacheItem von welches Adds a CacheItem on which this cache items depends
*   
*   @param  r               Embperl request record
*   @param  pItem           CacheItem welches von pDependsOn anhängt
*   @param  pDependsOn      CacheItem von welchem pItem abhängt
*   @return                 0 wenn fehlerfrei ausgeführt
*   \endif                                                                       
*                                                                          
* ------------------------------------------------------------------------ */




int Cache_AddDependency (/*in*/ req *       r,
                         /*in*/ tCacheItem *    pItem,
                         /*in*/ tCacheItem *    pDependsOn)

    {
    int n ;
    
    if (!pItem -> pDependsOn)
        ArrayNew (&pItem -> pDependsOn, 2, sizeof (tCacheItem *)) ;

    n = ArrayAdd (&pItem -> pDependsOn, 1) ;
    pItem -> pDependsOn[n] = pDependsOn ;


    if (!pDependsOn -> pNeededFor)
        ArrayNew (&pDependsOn -> pNeededFor, 2, sizeof (tCacheItem *)) ;

    n = ArrayAdd (&pDependsOn -> pNeededFor, 1) ;
    pDependsOn -> pNeededFor[n] = pItem ;

    return ok ;
    }


/* ------------------------------------------------------------------------ */
/*                                                                          */
/* Cache_GetDependency  						    */
/*                                                                          */
/*! 
*   \_en
*   Get the Nth CacheItem on which this cache depends
*   
*   @param  r               Embperl request record
*   @param  pItem           CacheItem 
*   @param  n               Dependency number
*   @return                 Nth Dependency CacheItem
*   \endif                                                                       
*
*   \_de									   
*   Gibt das Nte CacheItem von dem pItem abhängt zurück
*   
*   @param  r               Embperl request record
*   @param  pItem           CacheItem
*   @param  n               Number der Abhänigkeit
*   @return                 Ntes CacheItem von welchem pItem abhängt
*   \endif                                                                       
*                                                                          
* ------------------------------------------------------------------------ */




tCacheItem * Cache_GetDependency (/*in*/ req *           r,
                                  /*in*/ tCacheItem *    pItem,
                                  /*in*/ int             n)

    {
    if (!pItem -> pDependsOn || ArrayGetSize (pItem -> pDependsOn) < n || n < 0)
        return NULL ;

    return pItem -> pDependsOn[n] ;
    }



/* ------------------------------------------------------------------------ */
/*                                                                          */
/* Cache_IsExpired  							    */
/*                                                                          */
/*! 
*   \_en
*   Checks if the cache item or a cache item on which this one depends is
*   expired
*   
*   @param  r               Embperl request record
*   @param  pItem           CacheItem which should be checked
*   @param  nLastUpdated    When a item on which this one depends, was 
*                           updated after the given request count, then
*                           this item is expired
*   @return                 TRUE if expired, otherwise false
*   \endif                                                                       
*
*   \_de									   
*   Prüft ob das CacheItem oder eines von welchem dieses abhängt nihct
*   mehr gültig ist
*   
*   @param  r               Embperl request record
*   @param  pItem           CacheItem welches überprüft werden soll
*   @param  nLastUpdated    Wenn ein Item von welchem dieses Item abhängt
*                           nach dem angegebenen Request Count geändert 
*                           wurde ist diese Item nicht mehr gültig
*   @return                 wahr wenn ungültig, ansonsten falsch
*   \endif                                                                       
*                                                                          
* ------------------------------------------------------------------------ */




int Cache_IsExpired     (/*in*/ req *           r,
                         /*in*/ tCacheItem *    pItem,
                         /*in*/ int             nLastUpdated)


    {
    int          rc ;
    tCacheItem * pSubItem ;
    int          i ;
    int		 numItems = pItem -> pDependsOn?ArrayGetSize (pItem -> pDependsOn):0 ;

    if (nLastUpdated < pItem -> nLastUpdated)
        return TRUE ;

    if (pItem -> bExpired || pItem -> nLastChecked == r -> nRequestCount)
	return pItem -> bExpired ; /* we already have checked this or know that is it expired */

    pItem -> nLastChecked = r -> nRequestCount ;

    /* first check dependency */
    for (i = 0; i < numItems; i++)
	{
	pSubItem = pItem -> pDependsOn[i] ;
	if (Cache_IsExpired (r, pSubItem, pItem -> nLastUpdated))
            {
            if (r -> bDebug & dbgCache)
                lprintf (r, "[%d]CACHE: %s expired because dependencies is expired or newer\n", r -> nPid, pItem -> sKey) ; 
            Cache_FreeContent (r, pItem) ;
            return pItem -> bExpired = TRUE ;
            }
	}

    if (pItem -> pProvider -> pProviderClass -> fExpires)
        {
        if ((*pItem ->  pProvider -> pProviderClass -> fExpires)(r, pItem ->  pProvider))
            { 
            if (r -> bDebug & dbgCache)
                lprintf (r, "[%d]CACHE: %s expired because provider C sub returned TRUE\n", r -> nPid,  pItem -> sKey) ; 
            Cache_FreeContent (r, pItem) ;
	    return pItem -> bExpired = TRUE ;
            }
        }

    if (pItem -> nExpiresInTime && pItem -> nLastModified + pItem -> nExpiresInTime < r -> nRequestTime)
        {
        if (r -> bDebug & dbgCache)
            lprintf (r, "[%d]CACHE: %s expired because of timeout (%d sec)\n", r -> nPid, pItem -> sKey, pItem -> nExpiresInTime) ; 
        Cache_FreeContent (r, pItem) ;
        return pItem -> bExpired = TRUE ;
        }

    if (pItem -> sExpiresFilename)
        {
        if (stat (pItem -> sExpiresFilename, &pItem -> FileStat))
            {
            if (r -> bDebug & dbgCache)
                lprintf (r, "[%d]CACHE: %s expired because cannot stat file %s\n", r -> nPid,  pItem -> sKey, pItem -> sExpiresFilename) ; 
            Cache_FreeContent (r, pItem) ;
	    return pItem -> bExpired = TRUE ;
            }

        if (r -> bDebug & dbgCache)
            lprintf (r, "[%d]CACHE: %s stat file %s mtime=%d size=%d\n", r -> nPid, pItem -> sKey, pItem -> sExpiresFilename, pItem -> FileStat.st_mtime, pItem -> FileStat.st_size) ; 
        if (pItem -> nFileModified != pItem -> FileStat.st_mtime)
            {
            if (r -> bDebug & dbgCache)
                lprintf (r, "[%d]CACHE: %s expired because file %s changed\n", r -> nPid, pItem -> sKey, pItem -> sExpiresFilename) ; 
	    pItem -> nFileModified = pItem -> FileStat.st_mtime ;
            Cache_FreeContent (r, pItem) ;
            return pItem -> bExpired = TRUE ;
            }
        }
    
    
    if (pItem -> pExpiresCV)
        {
        SV * pRet ;

        if ((rc = CallCV (r, "Expired?", pItem -> pExpiresCV, 0, &pRet)) != ok)
            {
            LogError (r, rc) ;
            Cache_FreeContent (r, pItem) ;
	    return pItem -> bExpired = TRUE ;
            }
    
        if (pRet && SvTRUE(pRet))
            { /* Expire the entry */
            if (r -> bDebug & dbgCache)
                lprintf (r, "[%d]CACHE: %s expired because expirey Perl sub returned TRUE\n", r -> nPid,  pItem -> sKey) ; 
            Cache_FreeContent (r, pItem) ;
	    return pItem -> bExpired = TRUE ;
            }
        }

    if (pItem -> fExpires)
        {
        if ((*pItem -> fExpires)(pItem))
            { 
            if (r -> bDebug & dbgCache)
                lprintf (r, "[%d]CACHE: %s expired because expirey C sub returned TRUE\n", r -> nPid,  pItem -> sKey) ; 
            Cache_FreeContent (r, pItem) ;
	    return pItem -> bExpired = TRUE ;
            }
        }

    if (r -> bDebug & dbgCache)
        lprintf (r, "[%d]CACHE: %s NOT expired\n", r -> nPid,  pItem -> sKey) ; 

    return FALSE ;
    }


/* ------------------------------------------------------------------------ */
/*                                                                          */
/* Cache_SetNotExpired  					            */
/*                                                                          */
/*! 
*   \_en
*   Reset expired flag and last modification time
*   
*   @param  r               Embperl request record
*   @param  pItem           CacheItem which should be checked
*   @return                 TRUE if expired, otherwise false
*   \endif                                                                       
*
*   \_de									   
*   Abgelaufen Flag zurücksetzen und Zeitpunkt der letzten Änderung setzen
*   
*   @param  r               Embperl request record
*   @param  pItem           CacheItem welches überprüft werden soll
*   @return                 wahr wenn ungültig, ansonsten falsch
*   \endif                                                                       
*                                                                          
* ------------------------------------------------------------------------ */


int Cache_SetNotExpired (/*in*/ req *       r,
                         /*in*/ tCacheItem *    pItem)

    {
    pItem -> nLastChecked   = r -> nRequestCount ;
    pItem -> nLastUpdated   = r -> nRequestCount ;
    pItem -> nLastModified  = r -> nRequestTime ;
    pItem -> bExpired       = FALSE ;

    if (!pItem -> bCache)
        pCachesToRelease[ArrayAdd(&pCachesToRelease, 1)] = pItem ;
    return ok ;
    }


/* ------------------------------------------------------------------------ */
/*                                                                          */
/* Cache_GetContentSV  					                    */
/*                                                                          */
/*! 
*   \_en
*   Get the whole content as SV, if not expired from the cache, otherwise ask
*   the provider to fetch it. This will also put a read lock on the
*   Cacheitem. When you are done with the content call ReleaseContent
*   
*   @param  r               Embperl request record
*   @param  pItem           CacheItem which should be checked
*   @param  pData           Returns the content
*   @param  pLen            Returns the length
*   @return                 error code
*   \endif                                                                       
*
*   \_de									   
*   Holt den gesamt Inhalt als SV soweit nich abgelaufen aus dem Cache, ansonsten
*   wird der Provider beauftragt ihn einzulesen. Zusätzlich wird ein
*   Read Lock gesetzt. Nach der Bearbeitetung des Inhalts sollte deshalb
*   ReleaseLock zum freigeben aufgerufen werden.
*   
*   @param  r               Embperl request record
*   @param  pItem           CacheItem welches überprüft werden soll
*   @param  pData           Liefert den Inhalt
*   @param  pLen            Liefert die Länge
*   @return                 Fehlercode
*   \endif                                                                       
*                                                                          
* ------------------------------------------------------------------------ */



int Cache_GetContentSV      (/*in*/ req *             r,
                             /*in*/ tCacheItem        *pItem,
                             /*in*/ SV * *            pData)

    {
    int rc ;

    if (Cache_IsExpired (r, pItem, pItem -> nLastUpdated) || !pItem -> pSVData)
        {
        if (pItem -> pProvider -> pProviderClass -> fGetContentSV)
            if ((rc = ((*pItem -> pProvider -> pProviderClass -> fGetContentSV) (r, pItem -> pProvider, pData))) != ok)
		{
                Cache_FreeContent (r, pItem)  ;
		return rc ;
		}
        Cache_SetNotExpired (r, pItem) ;
        pItem -> pSVData = *pData ;
        }
    else
        {
        if (r -> bDebug & dbgCache)
            lprintf (r, "[%d]CACHE: %s take from cache\n", r -> nPid,  pItem -> sKey) ; 
        *pData = pItem -> pSVData  ;
        }


    return ok ;
    }

/* ------------------------------------------------------------------------ */
/*                                                                          */
/* Cache_GetContentPtr					                    */
/*                                                                          */
/*! 
*   \_en
*   Get the whole content as pointer, if not expired from the cache, otherwise ask
*   the provider to fetch it. This will also put a read lock on the
*   Cacheitem. When you are done with the content call ReleaseContent
*   
*   @param  r               Embperl request record
*   @param  pItem           CacheItem which should be checked
*   @param  pData           Returns the content
*   @return                 error code
*   \endif                                                                       
*
*   \_de									   
*   Holt den gesamt Inhalt als Zeiger soweit nich abgelaufen aus dem Cache, ansonsten
*   wird der Provider beauftragt ihn einzulesen. Zusätzlich wird ein
*   Read Lock gesetzt. Nach der Bearbeitetung des Inhalts sollte deshalb
*   ReleaseLock zum freigeben aufgerufen werden.
*   
*   @param  r               Embperl request record
*   @param  pItem           CacheItem welches überprüft werden soll
*   @param  pData           Liefert den Inhalt
*   @return                 Fehlercode
*   \endif                                                                       
*                                                                          
* ------------------------------------------------------------------------ */



int Cache_GetContentPtr     (/*in*/ req *             r,
                             /*in*/ tCacheItem        *pItem,
                             /*in*/ void * *          pData)

    {
    int rc ;

    if (Cache_IsExpired (r, pItem, pItem -> nLastUpdated) || !pItem -> pData)
        {
        if (r -> bDebug & dbgCache)
            lprintf (r, "[%d]CACHE: %s get from provider\n", r -> nPid,  pItem -> sKey) ; 
        if (pItem -> pProvider -> pProviderClass -> fGetContentPtr)
            if ((rc = (*pItem -> pProvider -> pProviderClass -> fGetContentPtr) (r, pItem -> pProvider, pData)) != ok)
		{
                Cache_FreeContent (r, pItem)  ;
                return rc ;
		}
        pItem -> pData = *pData ;
        Cache_SetNotExpired (r, pItem) ;
        }
    else
        {
        if (r -> bDebug & dbgCache)
            lprintf (r, "[%d]CACHE: %s take from cache\n", r -> nPid,  pItem -> sKey) ; 
        *pData = pItem -> pData ;
        }
    return ok ;
    }

/* ------------------------------------------------------------------------ */
/*                                                                          */
/* Cache_GetContentIndex			                            */
/*                                                                          */
/*! 
*   \_en
*   Get the whole content as pointer, if not expired from the cache, otherwise ask
*   the provider to fetch it. This will also put a read lock on the
*   Cacheitem. When you are done with the content call ReleaseContent
*   
*   @param  r               Embperl request record
*   @param  pItem           CacheItem which should be checked
*   @param  pData           Returns the content
*   @return                 error code
*   \endif                                                                       
*
*   \_de									   
*   Holt den gesamt Inhalt als Zeiger soweit nich abgelaufen aus dem Cache, ansonsten
*   wird der Provider beauftragt ihn einzulesen. Zusätzlich wird ein
*   Read Lock gesetzt. Nach der Bearbeitetung des Inhalts sollte deshalb
*   ReleaseLock zum freigeben aufgerufen werden.
*   
*   @param  r               Embperl request record
*   @param  pItem           CacheItem welches überprüft werden soll
*   @param  pData           Liefert den Inhalt
*   @return                 Fehlercode
*   \endif                                                                       
*                                                                          
* ------------------------------------------------------------------------ */



int Cache_GetContentIndex   (/*in*/ req *             r,
                             /*in*/ tCacheItem        *pItem,
                             /*in*/ tIndex *          pData)

    {
    int rc ;

    if (Cache_IsExpired (r, pItem, pItem -> nLastUpdated) || !pItem -> xData)
        {
        if (r -> bDebug & dbgCache)
            lprintf (r, "[%d]CACHE: %s get from provider\n", r -> nPid,  pItem -> sKey) ; 
        if (pItem -> pProvider -> pProviderClass -> fGetContentIndex)
            if ((rc = (*pItem -> pProvider -> pProviderClass -> fGetContentIndex) (r, pItem -> pProvider, pData)) != ok)
		{
                Cache_FreeContent (r, pItem)  ;
                return rc ;
		}
        pItem -> xData = *pData ;
        Cache_SetNotExpired (r, pItem) ;
        }
    else
        {
        if (r -> bDebug & dbgCache)
            lprintf (r, "[%d]CACHE: %s take from cache\n", r -> nPid,  pItem -> sKey) ; 
        *pData = pItem -> xData ;
        }
    return ok ;
    }

/* ------------------------------------------------------------------------ */
/*                                                                          */
/* Cache_GetContentSvIndex			                            */
/*                                                                          */
/*! 
*   \_en
*   Get the whole content as pointer, if not expired from the cache, otherwise ask
*   the provider to fetch it. This will also put a read lock on the
*   Cacheitem. When you are done with the content call ReleaseContent
*   
*   @param  r               Embperl request record
*   @param  pItem           CacheItem which should be checked
*   @param  pData           Returns the content
*   @return                 error code
*   \endif                                                                       
*
*   \_de									   
*   Holt den gesamt Inhalt als Zeiger soweit nich abgelaufen aus dem Cache, ansonsten
*   wird der Provider beauftragt ihn einzulesen. Zusätzlich wird ein
*   Read Lock gesetzt. Nach der Bearbeitetung des Inhalts sollte deshalb
*   ReleaseLock zum freigeben aufgerufen werden.
*   
*   @param  r               Embperl request record
*   @param  pItem           CacheItem welches überprüft werden soll
*   @param  pData           Liefert den Inhalt
*   @return                 Fehlercode
*   \endif                                                                       
*                                                                          
* ------------------------------------------------------------------------ */



int Cache_GetContentSvIndex   (/*in*/ req *             r,
                             /*in*/ tCacheItem        *pItem,
                             /*in*/ SV * *            pSVData,
                             /*in*/ tIndex *          pData)

    {
    int rc ;
    bool bUpdate = FALSE ;

    if (Cache_IsExpired (r, pItem, pItem -> nLastUpdated))
        {
        pItem -> xData = 0 ;
        pItem -> pSVData = NULL ;
        }
    if (!pItem -> xData)
        {
        if (r -> bDebug & dbgCache)
            lprintf (r, "[%d]CACHE: %s get from provider\n", r -> nPid,  pItem -> sKey) ; 
        if (pItem -> pProvider -> pProviderClass -> fGetContentIndex)
            if ((rc = (*pItem -> pProvider -> pProviderClass -> fGetContentIndex) (r, pItem -> pProvider, pData)) != ok)
		{
                Cache_FreeContent (r, pItem)  ;
                return rc ;
		}
        pItem -> xData = *pData ;
        bUpdate = TRUE ;
        }
    else
        *pData = pItem -> xData ;

    if (!pItem -> pSVData)
        {
        if ((r -> bDebug & dbgCache) && !bUpdate)
            lprintf (r, "[%d]CACHE: %s get from provider\n", r -> nPid,  pItem -> sKey) ; 
        if (pItem -> pProvider -> pProviderClass -> fGetContentSV)
            if ((rc = (*pItem -> pProvider -> pProviderClass -> fGetContentSV) (r, pItem -> pProvider, pSVData)) != ok)
		{
                Cache_FreeContent (r, pItem)  ;
                return rc ;
		}
        pItem -> pSVData = *pSVData ;
        bUpdate = TRUE ;
        }
    else
        *pSVData = pItem -> pSVData ;

    if (bUpdate)
        {
        Cache_SetNotExpired (r, pItem) ;
        }
    else
        {
        if (r -> bDebug & dbgCache)
            lprintf (r, "[%d]CACHE: %s take from cache\n", r -> nPid,  pItem -> sKey) ; 
        }
    return ok ;
    }

/* ------------------------------------------------------------------------ */
/*                                                                          */
/* Cache_ReleaseContent   				                    */
/*                                                                          */
/*! 
*   \_en
*   Removes the read and/or write lock from the content.
*   
*   @param  r               Embperl request record
*   @param  pItem           CacheItem which should be checked
*   @return                 error code
*   \endif                                                                       
*
*   \_de									   
*   Gibt den Read und/oder Write Lock wieder frei.
*   
*   @param  r               Embperl request record
*   @param  pItem           CacheItem welches überprüft werden soll
*   @return                 Fehlercode
*   \endif                                                                       
*                                                                          
* ------------------------------------------------------------------------ */



int Cache_ReleaseContent        (/*in*/ req *             r,
                                 /*in*/ tCacheItem        *pItem)

    {
    /* locking not yet implemented */
    tCacheItem * pSubItem ;
    int          i ;
    int		 numItems = pItem -> pDependsOn?ArrayGetSize (pItem -> pDependsOn):0 ;

    if (!pItem -> bCache)
        Cache_FreeContent (r, pItem) ;

    for (i = 0; i < numItems; i++)
	{
	pSubItem = pItem -> pDependsOn[i] ;
	Cache_ReleaseContent (r, pSubItem) ;
	}

    return ok ;
    }


/* ------------------------------------------------------------------------ */
/*                                                                          */
/* Cache_FreeContent   				                            */
/*                                                                          */
/*! 
*   \_en
*   Free the cached data
*   
*   @param  r               Embperl request record
*   @param  pItem           CacheItem
*   @return                 error code
*   \endif                                                                       
*
*   \_de									   
*   Gibt die gecachten Daten frei
*   
*   @param  r               Embperl request record
*   @param  pItem           CacheItem
*   @return                 Fehlercode
*   \endif                                                                       
*                                                                          
* ------------------------------------------------------------------------ */



int Cache_FreeContent           (/*in*/ req *             r,
                                 /*in*/ tCacheItem        *pItem)

    {
    int rc ;
    
    if ((r -> bDebug & dbgCache) && (pItem -> pSVData || pItem -> pData || pItem -> xData))
        lprintf (r, "[%d]CACHE: Free content for %s\n", r -> nPid, pItem -> sKey) ; 

    if (pItem -> pProvider -> pProviderClass -> fFreeContent)
        if ((rc = (*pItem -> pProvider -> pProviderClass -> fFreeContent) (r, pItem)) != ok)
            return rc ;
    
    if (pItem -> pSVData)
        {
        SvREFCNT_dec (pItem -> pSVData) ;
        pItem -> pSVData = NULL ;
        }
    pItem -> pData = NULL ;
    pItem -> xData = 0 ;

    return ok ;
    }


