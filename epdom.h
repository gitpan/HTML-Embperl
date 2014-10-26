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
#   $Id: epdom.h,v 1.4.2.45 2001/11/16 11:29:02 richter Exp $
#
###################################################################################*/



struct tArrayCtrl
    {
#ifdef DMALLOC
    char    sSig [6] ;
    char *  sFile ;
    int     nLine ;
#endif
    int	    nFill ;	    /* index of last element */
    int	    nMax ;	    /* number of last element for which is space */
    int	    nAdd ;	    /* number of elements to add on grow */
    int	    nElementSize ;  /* number of bytes per element */
    } ;

typedef void  tArray  ;

typedef unsigned char	tUInt8 ;
typedef signed char	tSInt8 ;
typedef unsigned short	tUInt16 ;
typedef signed short	tSInt16 ;
typedef unsigned long	tUInt32 ;
typedef signed long	tSInt32 ;

typedef tSInt32		tIndex ;
typedef tSInt16		tIndexShort ;
typedef tIndex		tStringIndex ;

typedef tUInt8          tNodeType ;
typedef tIndex		tNode	 ;
typedef tIndex		tAttr	 ;
typedef tUInt16         tRepeatLevel ;


struct tNodeData
    {
    tNodeType		nType ;
    tUInt8		bFlags ;
    tIndexShort		xDomTree ;
    tIndex		xNdx ;
    tStringIndex        nText ;
    tNode		xChilds ;
    tUInt16		numAttr ;
    tUInt16             nLinenumber ;
    tNode		xPrev ;
    tNode		xNext ;
    tNode		xParent ;
    tRepeatLevel	nRepeatLevel ;
    } ;

typedef struct tNodeData tNodeData ;

struct tAttrData
    {
    tNodeType		nType ;     /* must be at the same offset as tNodeData.nType ! */
    tUInt8		bFlags ;
    tUInt16		nNodeOffset ;
    tIndex		xNdx ;      /* must be at the same offset as tNodeData.xNdx ! */
    tIndex              xName ;
    tIndex              xValue ;  /* must be at the same offset as tNodeData.xChilds ! */
    } ;

typedef struct tAttrData tAttrData ;


struct tDomNode
    {
    tIndex		xDomTree ;
    tNode		xNode ;
    SV *		pDomNodeSV ;
    } ;

typedef struct tDomNode tDomNode ;


/*
  Node Types
*/

/* ------------------------------------------------------------------------ 

interface Node {
  # NodeType 
  const unsigned short      ELEMENT_NODE                   = 1;
  const unsigned short      ATTRIBUTE_NODE                 = 2;
  const unsigned short      TEXT_NODE                      = 3;
  const unsigned short      CDATA_SECTION_NODE             = 4;
  const unsigned short      ENTITY_REFERENCE_NODE          = 5;
  const unsigned short      ENTITY_NODE                    = 6;
  const unsigned short      PROCESSING_INSTRUCTION_NODE    = 7;
  const unsigned short      COMMENT_NODE                   = 8;
  const unsigned short      DOCUMENT_NODE                  = 9;
  const unsigned short      DOCUMENT_TYPE_NODE             = 10;
  const unsigned short      DOCUMENT_FRAGMENT_NODE         = 11;
  const unsigned short      NOTATION_NODE                  = 12;

*/

enum tNodeTypeValues
    {
    ntypTag	        = 1,
    ntypStartTag        = 1 + 0x20,
    ntypStartEndTag     = 1 + 0x80,
    ntypEndTag	        = 1 + 0x40,
    ntypEndStartTag     = 1 + 0x60,
    ntypAttr	        = 2,
    ntypAttrValue       = 2 + 0x20,
    ntypText	        = 3,
    ntypCDATA	        = 4,
    ntypEntityRef       = 5,
    ntypEntity          = 6,
    ntypProcessingInstr = 7,
    ntypComment         = 8,
    ntypDocument        = 9,
    ntypDocumentType    = 10,
    ntypDocumentFraq    = 11,
    ntypNotation        = 12
    } ;

#define ntypMask 0x1f 

enum tNodeFlags
    {
    nflgDeleted	     = 0,
    nflgOK	     = 1,
    nflgEscUrl       = 2,
    nflgEscChar	     = 4,
    nflgEscXML	     = 8,
    nflgIgnore       = 16,	/**< Ignore this node */
    nflgNewLevelNext = 32,	/**< Next sibling has new RepeatLevel */
    nflgNewLevelPrev = 64	/**< Previous sibling has new RepeatLevel */
    } ;

enum tAttrFlags
    {
    aflgDeleted	    = 0,
    aflgOK	    = 1,
    aflgAttrValue   = 2,
    aflgAttrChilds  = 4,
    aflgSingleQuote = 8
    } ;

struct tDomTreeOrder
    {
    tNode  xFromNode ;	    
    tNode  xToNode ;
    } ;

typedef struct tDomTreeOrder tDomTreeOrder ;

struct tDomTreeCheckpoint
    {
    tNode  xNode ;	    
    } ;

typedef struct tDomTreeCheckpoint tDomTreeCheckpoint ;

struct tDomTreeCheckpointStatus
    {
    tRepeatLevel    nRepeatLevel ;	 /**< Repeatlevel when last passed this checkpoint */
    tIndex          nCompileCheckpoint ; /**< r -> nCurrCheckpoint when last passed this checkpoint */
    tNode	    xJumpFromNode ;	 /**< node from which backward jump started */
    tNode	    xJumpToNode ;	 /**< node to which backward jump was */
    } ;

typedef struct tDomTreeCheckpointStatus tDomTreeCheckpointStatus ;


struct tRepeatLevelLookupItem
    {
    tNodeData	*	    pNode ;	/* pointer to actual node data */
    struct tRepeatLevelLookupItem *  pNext ;	/* next node with same node index but different nRepeatLevel */
    } ;

typedef struct tRepeatLevelLookupItem tRepeatLevelLookupItem ;

struct tRepeatLevelLookup
    {
    tNode                   xNullNode ; /**< node index of node with RepeatLevel == 0 */
    tRepeatLevel	    numItems ;	/**< size of table (must be 2^n) */
    tRepeatLevel	    nMask ;	/**< mask (usualy numItems - 1) */
    tRepeatLevelLookupItem  items[1] ;	/**< array with numItems items */
    } ;

typedef struct tRepeatLevelLookup tRepeatLevelLookup ;

struct tLookupItem
    {
    void  *	    pLookup ;	/* table for converting tNode and tAttr to pointers */
    tRepeatLevelLookup  * pLookupLevel ; /* hash table used to index xNode/nRepeatLevel */
    } ;

typedef struct tLookupItem tLookupItem ;


struct tDomTree
    {
    tLookupItem *   pLookup ;		/**< \_en  table for converting tNode and tAttr to pointers \endif */
					/**< \_de  Tabelle um tNode und tAttr Indexe zu Zeigern umzuwandeln \endif */
    tDomTreeCheckpoint * pCheckpoints ; /**< \_en  checkpoints in the code, to check against execution order when running \endif*/
					/**< \_de  checkpoints im Code. Diese Liste wird wird der Reiehenfolge der 
                                                  tats�chlichen Ausf�hrung verglichen um den DomTree entsprechend zu modifizieren \endif*/
    tDomTreeCheckpointStatus * pCheckpointStatus ; /**< \_en  status of checkpoint while generating new DomTree \endif */
					/**< \_de  Status eines checkpoints w�hrend dem neu Erzeugen eines DomTree \endif */
    tIndex	    xNdx ;		/**< \_en  Index of Dom Tree \endif */
					/**< \_de  Index des Dom Tree \endif */    
    tIndex	    xSourceNdx ;	/**< \_en  When a cloned DomTree this is the index of the source Dom Tree \endif */
					/**< \_de  Wenn dieser Dom Tree gecloned ist, ist dies der Index des Quellen Dom Tree \endif */    
    tNode	    xDocument ;		/**< \_en  Index of the root document node \endif */
					/**< \_en  Index des Wurzelknotenes des DomTrees \endif */
    tNode           xLastNode ;		/**< last node that was compiled */
    tNode           xCurrNode ;		/**< curr node that is compiled */
    tIndex          xFilename ;		/**< name of source file */
    SV *	    pSV ;		/**< general purpose SV */
    SV *	    pDomTreeSV ;	/**< SV that's hold the Index */
					/**< Domtree will be deleted when this SV is delted */
    tUInt16         nLastLinenumber ;

    AV *            pDependsOn ;        /**< List of DomTree on which this one depends */
    } ;

typedef struct tDomTree tDomTree ;




extern tDomTree  *    pDomTrees ;	     /* Array with all Dom Trees */
extern HE * *	      pStringTableArray  ;   /* Array with pointers to strings */
extern tIndex	      xNoName ;		     /* String index for Attribut with noname */
extern tIndex	      xDomTreeAttr ;	     /* String index for Attribut which holds the DomTree index */
extern tIndex	      xDocument ;	     /* String index for Document */
extern tIndex	      xDocumentFraq ;	     /* String index for Document Fraquent */
extern tIndex	      xOrderIndexAttr ;	     /* String index for Attribute which holds the OrderIndex */


tStringIndex String2NdxInc (/*in*/ const char *	    sText,
			    /*in*/ int		    nLen,
			    /*in*/ int		    bInc) ;

tStringIndex String2UniqueNdx (/*in*/ const char *	    sText,
  			       /*in*/ int		    nLen) ;


#define SV2String(pSV,l)  (SvOK (pSV)?SvPV (pSV, l):(l=0,((char *)NULL)))

#define String2NdxNoInc(sText,nLen) String2NdxInc(sText,nLen,0)
#define String2Ndx(sText,nLen) String2NdxInc(sText,nLen,1)

#define Ndx2String(nNdx) (HeKEY (pStringTableArray[nNdx]))
#define Ndx2StringLen(nNdx,sVal,nLen) { HE * pHE = pStringTableArray[nNdx] ; nLen=HeKLEN(pHE) ; sVal = HeKEY (pHE) ; }

#define NdxStringRefcntInc(nNdx) (SvREFCNT_inc (HeVAL (pStringTableArray[nNdx])))


#ifdef DMALLOC
int ArrayNew_dbg (/*in*/ const tArray * pArray,
	      /*in*/ int	nAdd,
	      /*in*/ int	nElementSize,
	      /*in*/ char *     sFile,
	      /*in*/ int        nLine) ;
int ArrayNewZero_dbg (/*in*/ const tArray * pArray,
	      /*in*/ int	nAdd,
	      /*in*/ int	nElementSize,
	      /*in*/ char *     sFile,
	      /*in*/ int        nLine) ;
int ArrayClone_dbg (/*in*/  const tArray * pOrgArray,
	        /*out*/ const tArray * pNewArray,	      
		/*in*/ char *     sFile,
		/*in*/ int        nLine) ;

#undef ArrayNew
#define ArrayNew(a,n,s) ArrayNew_dbg(a,n,s,__FILE__,__LINE__)
#undef ArrayNewZero
#define ArrayNewZero(a,n,s) ArrayNewZero_dbg(a,n,s,__FILE__,__LINE__)
#undef ArrayClone
#define ArrayClone(o,n) ArrayClone_dbg(o,n,__FILE__,__LINE__)

#else

int ArrayNew (/*in*/ const tArray * pArray,
	      /*in*/ int	nAdd,
	      /*in*/ int	nElementSize) ;

int ArrayNewZero (/*in*/ const tArray * pArray,
	      /*in*/ int	nAdd,
	      /*in*/ int	nElementSize) ;

int ArrayClone (/*in*/  const tArray * pOrgArray,
	        /*out*/ const tArray * pNewArray) ;

#endif

int ArrayFree (/*in*/ const tArray * pArray) ;

int ArraySet (/*in*/ const tArray * pArray,
	      /*in*/ int	numElements) ;

int ArraySetSize (/*in*/ const tArray * pArray,
	          /*in*/ int	numElements) ;

int ArrayGetSize (/*in*/ const tArray * pArray) ;

int ArrayAdd (/*in*/ const tArray * pArray,
	      /*in*/ int	numElements) ;

int ArraySub (/*in*/ const tArray * pArray,
	      /*in*/ int	numElements) ;

#ifdef DMALLOC
void StringNew_dbg (/*in*/ char * * pArray,
	      /*in*/ int	nAdd,
	      /*in*/ char *     sFile,
	      /*in*/ int        nLine) ;

#undef StringNew
#define StringNew(a,n) StringNew_dbg(a,n,__FILE__,__LINE__)

#else
void StringNew (/*in*/ char * * pArray,
	      /*in*/ int	nAdd) ;
#endif

void StringFree (/*in*/ char * * pArray) ;

int StringAdd (/*in*/ char * *	   pArray,
	       /*in*/ const char * sAdd,
	       /*in*/ int 	   nLen) ;

int DomInit (void) ;

void DomStats (void) ;

int DomTree_clone (/*in*/ tDomTree *	pOrgDomTree,
		   /*out*/tDomTree * *  pNewDomTree,
		   /*in*/ int           bForceDocFraq) ;

int DomTree_new (tDomTree * * pNewLookup) ;

int DomTree_delete (tDomTree * pDomTree) ;

void DomTree_checkpoint (tReq * r, tIndex nRunCheckpoint) ;
void DomTree_discardAfterCheckpoint (tReq * r, tIndex nRunCheckpoint) ;

#define DomTree_SV(pSV) SvIVX(pSV)
#define DomTree_selfSV(pSV) (DomTree_self(SvIVX(pSV)))
#define SV_DomTree(xDomTree) (DomTree_self (xDomTree) -> pDomTreeSV)
#define SV_DomTree_self(pDomTree) (pDomTree -> pDomTreeSV)
#define DomTree_filename(xDomTree)	    (Ndx2String (DomTree_self(xDomTree) -> xFilename))
#define DomTree_selfFilename(pDomTree)	    (Ndx2String ((pDomTree) -> xFilename))

tNodeData * Node_selfLevelItem (/*in*/ tDomTree *    pDomTree,
				/*in*/ tNode	 xNode,
				/*in*/ tRepeatLevel  nLevel) ;

tNode Node_appendChild (/*in*/  tDomTree *	 pDomTree,
			/*in*/	tNode 		 xParent,
                        /*in*/  tRepeatLevel     nRepeatLevel,
			/*in*/	tNodeType	 nType,
			/*in*/	int              bForceAttrValue,
			/*in*/	const char *	 sText,
			/*in*/	int		 nTextLen,
			/*in*/	int		 nLevel,
			/*in*/	int		 nLinenumber,
			/*in*/	const char *	 sLogMsg) ;



struct tNodeData * Node_selfNthChild (/*in*/  tDomTree *        pDomTree,
				      /*in*/ struct tNodeData * pNode,
                                      /*in*/  tRepeatLevel      nRepeatLevel,
				      /*in*/ int		nChildNo) ;


tNodeData * Node_selfLastChild   (/*in*/  tDomTree *   pDomTree,
				  /*in*/  tNodeData *  pNode,
                                  /*in*/  tRepeatLevel nRepeatLevel) ;

tNodeData * Node_selfNextSibling (/*in*/ tDomTree *  pDomTree,
			          /*in*/ tNodeData * pNode,
                                  /*in*/  tRepeatLevel nRepeatLevel) ;
				  
tNode Node_nextSibling (/*in*/ tDomTree *  pDomTree,
			/*in*/ tNode       xNode,
                        /*in*/  tRepeatLevel nRepeatLevel) ;

tNodeData * Node_selfPreviousSibling (/*in*/ tDomTree *	    pDomTree,
				      /*in*/ tNodeData *    pNode,
				      /*in*/ tRepeatLevel   nRepeatLevel) ;

tNode Node_previousSibling (/*in*/ tDomTree *	pDomTree,
			    /*in*/ tNode	xNode,
                            /*in*/ tRepeatLevel	nRepeatLevel) ;


#define DomTree_self(xDomTree)		    (&pDomTrees[xDomTree]) 

#define Node_self(pDomTree,xNode)	    ((struct tNodeData *)(pDomTree -> pLookup[xNode].pLookup))
#define Node_selfLevel(pDomTree,xNode,nLevel)  \
			    (pDomTree -> pLookup[xNode].pLookup == NULL?NULL:    \
				(((struct tNodeData *)(pDomTree -> pLookup[xNode].pLookup)) -> nRepeatLevel == nLevel? \
				((struct tNodeData *)(pDomTree -> pLookup[xNode].pLookup)): \
				Node_selfLevelItem(pDomTree,xNode,nLevel)))

#define Node_selfNotNullLevel(pDomTree,xNode,nLevel)	\
			    (pDomTree -> pLookup[xNode].pLookup == NULL?NULL:    \
				(((struct tNodeData *)(pDomTree -> pLookup[xNode].pLookup)) -> xDomTree == pDomTree -> xNdx? \
				((struct tNodeData *)(pDomTree -> pLookup[xNode].pLookup)): \
				Node_selfLevelItem(pDomTree,xNode,nLevel)))
#if 0
			    (pDomTree -> pLookup[xNode].pLookup == ?  \
				((((struct tNodeData *)(pDomTree -> pLookup[xNode].pLookup)) -> nRepeatLevel == nLevel || \
				((struct tNodeData *)(pDomTree -> pLookup[xNode].pLookup)) -> nRepeatLevel !=  0 || \
				pDomTree -> pLookup[xNode].pLookupLevel == NULL)?   \
				    ((struct tNodeData *)(pDomTree -> pLookup[xNode].pLookup)):	\
				    Node_selfLevelItem(pDomTree,xNode,nLevel)):	\
			    NULL)
#endif

#define xNode_selfLevelNull(pDomTree,pNode)   ((pDomTree) -> pLookup[(pNode)->xNdx].pLookupLevel?(pDomTree) -> pLookup[(pNode)->xNdx].pLookupLevel -> xNullNode:(pNode) -> xNdx)


#define Node_parentNode(pDomTree,xNode,nLevel)	    (Node_selfLevel(pDomTree,xNode,nLevel)->xParent)
#define Node_selfParentNode(pDomTree,pNode,nLevel) (Node_selfLevel(pDomTree,(pNode)->xParent,nLevel))
#define xNode_selfParentNode(pDomTree,pNode) ((pNode)->xParent)


#define Node_firstChild(pDomTree,xNode,nLevel)	    (Node_selfLevel(pDomTree,xNode,nLevel)->xChilds)
#define Node_selfFirstChild(pDomTree,pNode,nLevel)  (Node_selfLevel(pDomTree,(pNode)->xChilds,nLevel))

#define Node_selfNodeNameNdx(pNode)	    ((pNode) -> nText) ;
#define Node_selfNodeName(pNode)	    (Ndx2String ((pNode) -> nText))
#define Node_nodeName(pDomTree,xNode,nLevel)	    (Ndx2String (Node_selfLevel (pDomTree,xNode,nLevel) -> nText))
#define Node_selfFirstAttr(pNode)	    ((tAttrData *)((pNode) + 1))

tNodeData * Node_selfCloneNode (/*in*/ tDomTree *      pDomTree,
				/*in*/ tNodeData *     pNode,
                                /*in*/ tRepeatLevel    nRepeatLevel,
				/*in*/  int	       bDeep) ;

tNodeData * Node_selfCondCloneNode (/*in*/ tDomTree *      pDomTree,
				    /*in*/ tNodeData *     pNode,
                                    /*in*/ tRepeatLevel    nRepeatLevel) ;

tNodeData * Node_selfForceLevel(/*in*/ tDomTree *      pDomTree,
				/*in*/ tNode           xNode,
                                /*in*/ tRepeatLevel    nRepeatLevel) ;

tNode Node_cloneNode (/*in*/ tDomTree *      pDomTree,
		      /*in*/ tNode 	     xNode,
                      /*in*/ tRepeatLevel    nRepeatLevel,
		      /*in*/  int	     bDeep) ;


void Node_toString (/*i/o*/ register req *  r,
		    /*in*/ tDomTree *	    pDomTree,
		    /*in*/ tNode	    xNode,
                    /*in*/ tRepeatLevel     nRepeatLevel) ;

struct tNodeData *  Node_selfRemoveChild (/*in*/ tDomTree *  pDomTree,
					  /*in*/ tNode		    xNode,
					  /*in*/ struct tNodeData *	    pChild) ;

tNode Node_removeChild (/*in*/ tDomTree *   pDomTree,
			/*in*/ tNode	    xNode,
			/*in*/ tNode	    xChild,
                        /*in*/ tRepeatLevel nRepeatLevel) ;


tNode Node_insertAfter          (/*in*/ tDomTree *      pNewNodeDomTree,
				 /*in*/ tNode		xNewNode,
                                 /*in*/ tRepeatLevel    nNewRepeatLevel,
                                 /*in*/ tDomTree *      pRefNodeDomTree,
				 /*in*/ tNode		xRefNode,
                                 /*in*/ tRepeatLevel    nRefRepeatLevel) ;

tNode Node_insertAfter_CDATA    (/*in*/ const char *    sText,
				 /*in*/ int             nTextLen,
			         /*in*/ int		nEscMode,
                                 /*in*/ tDomTree *      pRefNodeDomTree,
				 /*in*/ tNode		xRefNode,
                                 /*in*/ tRepeatLevel    nRefRepeatLevel) ;

tNode Node_replaceChildWithNode (/*in*/ tDomTree *      pDomTree,
				 /*in*/ tNode		xNode,
                                 /*in*/ tRepeatLevel    nRepeatLevel,
                                 /*in*/ tDomTree *      pOldChildDomTree,
				 /*in*/ tNode		xOldChild,
                                 /*in*/ tRepeatLevel    nOldRepeatLevel) ;


tNode Node_replaceChildWithCDATA (/*in*/ tDomTree *	 pDomTree,
				  /*in*/ tNode		 xOldChild,
                                  /*in*/ tRepeatLevel    nRepeatLevel,
                                  /*in*/ const char *	 sText,
				  /*in*/ int		 nTextLen,
				  /*in*/ int		 nEscMode,
				  /*in*/ int		 bFlags) ;

char * Node_childsText (/*in*/ tDomTree *  pDomTree,
		       /*in*/  tNode       xNode,
                       /*in*/ tRepeatLevel nRepeatLevel,
		       /*i/o*/ char * *    ppText,
		       /*in*/  int         bDeep) ;


tAttrData *  Element_selfGetAttribut (/*in*/ tDomTree *     pDomTree,
				      /*in*/ struct tNodeData * pNode,
				      /*in*/ const char *	sAttrName,
				      /*in*/ int		nAttrNameLen) ;



tAttrData *  Element_selfGetNthAttribut (/*in*/ tDomTree *	   pDomTree,
			  	         /*in*/ struct tNodeData * pNode,
				         /*in*/ int                n) ;


tAttrData *  Element_selfSetAttribut (/*in*/ tDomTree *	        pDomTree,
				      /*in*/ struct tNodeData * pNode,
				      /*in*/ tRepeatLevel       nRepeatLevel,
				      /*in*/ const char *	sAttrName,
				      /*in*/ int		nAttrNameLen,
				      /*in*/ const char *       sNewValue, 
				      /*in*/ int		nNewValueLen) ;

tAttrData *  Element_selfRemoveAttributPtr (/*in*/ tDomTree *	        pDomTree,
				      /*in*/ struct tNodeData * pNode,
				      /*in*/ tRepeatLevel       nRepeatLevel,
				      /*in*/ tAttrData *	pAttr) ;

tAttrData *  Element_selfRemoveNthAttribut (/*in*/ tDomTree *	        pDomTree,
				      /*in*/ struct tNodeData * pNode,
				      /*in*/ tRepeatLevel       nRepeatLevel,
				      /*in*/ int                n) ;


tAttrData *  Element_selfRemoveAttribut (/*in*/ tDomTree *	        pDomTree,
				      /*in*/ struct tNodeData * pNode,
				      /*in*/ tRepeatLevel       nRepeatLevel,
				      /*in*/ const char *	sAttrName,
				      /*in*/ int		nAttrNameLen) ;

#define Attr_self(pDomTree,xAttr)	    ((struct tAttrData *)(pDomTree -> pLookup[xAttr].pLookup))
#define Attr_selfNode(pAttr)        ((struct tNodeData * )(((tUInt8 *)pAttr) - pAttr -> nNodeOffset))
#define Attr_selfAttrNum(pAttr)    (pAttr - Node_selfFirstAttr (Attr_selfNode(pAttr)))


char *       Attr_selfValue (/*in*/  tDomTree *	        pDomTree,
			     /*in*/  struct tAttrData * pAttr,
			     /*in*/  tRepeatLevel       nRepeatLevel,
			     /*out*/ char * *		ppAttr) ;


#define NodeAttr_selfParentNode(pDomTree,pNode,nLevel) (pNode->nType == ntypAttr?Attr_selfNode(((tAttrData *)pNode)):(Node_self(pDomTree,(pNode)->xParent)))

