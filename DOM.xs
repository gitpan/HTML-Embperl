###################################################################################
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
#   $Id: DOM.xs,v 1.1.2.20 2001/11/08 13:14:02 richter Exp $
#
###################################################################################



MODULE = XML::Embperl::DOM      PACKAGE = XML::Embperl::DOM     PREFIX = embperl_


################################################################################

MODULE = XML::Embperl::DOM      PACKAGE = XML::Embperl::DOM::Node     PREFIX = embperl_Node_

void
embperl_Node_attach (pRV,xDomTree,xNode)
    SV * pRV ;
    int  xDomTree
    int  xNode
CODE:
    tDomNode * pDomNode ;
    MAGIC * mg ;
    SV *    pSV = SvRV(pRV) ;
    if (mg = mg_find (pSV, '~'))
        {
        pDomNode = (tDomNode *)(mg -> mg_len) ;
        if (xDomTree)
            pDomNode -> xDomTree = xDomTree ;
        if (xNode)    
            pDomNode -> xNode = xNode ;
        }    
    else
        {
        Newc (0, pDomNode, 1, sizeof (tDomNode), tDomNode) ;
        pDomNode -> xDomTree = xDomTree ;
        pDomNode -> xNode = xNode ;
        pDomNode -> pDomNodeSV = pRV ;
        /* sv_unmagic ((SV *)pSV, '~') ; */
        sv_magic ((SV *)pSV, NULL, '~', (char *)&pDomNode, sizeof (pDomNode)) ;
        /* sv_bless (pRV, gv_stashpv ("XML::Embperl::DOM::Node", 0)) ; */
        }
    


SV *
embperl_Node_replaceChildWithCDATA (pDomNode,sText)
    tDomNode * pDomNode
    SV *     sText
PREINIT:
    STRLEN l ;
    char * s  ;
PPCODE:
    SvGETMAGIC_P4(sText) ;
    s = SV2String (sText, l) ;
    Node_replaceChildWithCDATA (DomTree_self(pDomNode -> xDomTree), pDomNode -> xNode, pCurrReq -> nCurrRepeatLevel, s, l, (pCurrReq -> nCurrEscMode & 11)== 3?1 + (pCurrReq -> nCurrEscMode & 4):pCurrReq -> nCurrEscMode, 0) ;
    pCurrReq -> nCurrEscMode = pCurrReq -> nEscMode ;
    pCurrReq -> bEscModeSet = -1 ;
    /*SvREFCNT_inc (sText) ;*/
    ST(0) = sText ;
    XSRETURN(1) ;


SV *
embperl_Node_XXiReplaceChildWithCDATA (xDomTree, xOldChild,sText)
    int xDomTree
    int xOldChild
    SV * sText
PREINIT:
    STRLEN l ;
    char * s  ;
PPCODE:
    SvGETMAGIC_P4(sText) ;
    s = SV2String (sText, l) ;
    Node_replaceChildWithCDATA (DomTree_self(xDomTree), xOldChild, pCurrReq -> nCurrRepeatLevel, s, l, (pCurrReq -> nCurrEscMode & 11)== 3?1 + (pCurrReq -> nCurrEscMode & 4):pCurrReq -> nCurrEscMode, 0) ;
    pCurrReq -> nCurrEscMode = pCurrReq -> nEscMode ;
    pCurrReq -> bEscModeSet = -1 ;
    /*SvREFCNT_inc (sText) ;*/
    ST(0) = sText ;
    XSRETURN(1) ;


SV *
embperl_Node_iReplaceChildWithCDATA (xOldChild,sText)
    int xOldChild
    SV * sText
PREINIT:
    STRLEN l ;
    char * s  ;
PPCODE:
    SvGETMAGIC_P4(sText) ;
    s = SV2String (sText, l) ;
    Node_replaceChildWithCDATA (DomTree_self(pCurrReq -> xCurrDomTree), xOldChild, pCurrReq -> nCurrRepeatLevel, s, l, (pCurrReq -> nCurrEscMode & 11)== 3?1 + (pCurrReq -> nCurrEscMode & 4):pCurrReq -> nCurrEscMode, 0) ;
    pCurrReq -> nCurrEscMode = pCurrReq -> nEscMode ;
    pCurrReq -> bEscModeSet = -1 ;
    /*SvREFCNT_inc (sText) ;*/
    ST(0) = sText ;
    XSRETURN(1) ;



SV *
embperl_Node_replaceChildWithUrlDATA (pDomNode,sText)
    tDomNode * pDomNode
    SV * sText
PREINIT:
    SV * sRet  ;
PPCODE:
    SvGETMAGIC_P4(sText) ;
    sRet = Node_replaceChildWithUrlDATA (pDomNode -> xDomTree, pDomNode -> xNode, pCurrReq -> nCurrRepeatLevel, sText) ;

    ST(0) = sRet ;
    XSRETURN(1) ;

SV *
embperl_Node_iReplaceChildWithUrlDATA (xOldChild,sText)
    int xOldChild
    SV * sText
PREINIT:
    SV * sRet  ;
PPCODE:
    SvGETMAGIC_P4(sText) ;
    sRet = Node_replaceChildWithUrlDATA (pCurrReq -> xCurrDomTree, xOldChild, pCurrReq -> nCurrRepeatLevel, sText) ;

    ST(0) = sRet ;
    XSRETURN(1) ;


void
embperl_Node_removeChild (pDomNode)
    tDomNode * pDomNode
CODE:
    Node_removeChild (DomTree_self (pDomNode -> xDomTree), -1, pDomNode -> xNode, 0) ;


void
embperl_Node_iRemoveChild (xDomTree, xChild)
    int xDomTree
    int xChild
CODE:
    Node_removeChild (DomTree_self (xDomTree), -1, xChild, 0) ;


void
embperl_Node_appendChild (pParentNode, nType, sText)
    tDomNode * pParentNode
    int nType
    SV * sText
PREINIT:
    int xNewParent ;
    STRLEN nText ;
    char * sT  ;
    tDomTree * pDomTree  ;
CODE:
    sT = SV2String (sText, nText) ;
    pDomTree = DomTree_self(pParentNode -> xDomTree) ;
    Node_appendChild (pDomTree, pParentNode -> xNode, pCurrReq -> nCurrRepeatLevel, nType, 0, sT, nText, 0, 0, NULL) ;


void
embperl_Node_iAppendChild (xDomTree, xParent, nType, sText)
    int xDomTree
    int xParent
    int nType
    SV * sText
CODE:
    int xNewParent ;
    STRLEN nText ;
    char * sT = SV2String (sText, nText) ;
    tDomTree * pDomTree = DomTree_self(xDomTree) ;
    Node_appendChild (pDomTree, xParent, pCurrReq -> nCurrRepeatLevel, nType, 0, sT, nText, 0, 0, NULL) ;


char *
embperl_Node_iChildsText (xDomTree, xChild, bDeep=0)
    int xDomTree
    int xChild
    int bDeep
PREINIT:
    char * sText ;
CODE:
    sText = Node_childsText (DomTree_self (xDomTree), xChild, pCurrReq -> nCurrRepeatLevel, 0, bDeep) ;
    RETVAL = sText?sText:"" ;
OUTPUT:
    RETVAL
CLEANUP:
    StringFree (&sText) ;


################################################################################

MODULE = XML::Embperl::DOM      PACKAGE = XML::Embperl::DOM::Tree     PREFIX = embperl_DomTree_

void
embperl_DomTree_iCheckpoint (nCheckpoint)
    int nCheckpoint
CODE:
    pCurrReq -> nCurrEscMode = pCurrReq -> nEscMode ;
    pCurrReq -> bEscModeSet = -1 ;
    DomTree_checkpoint (pCurrReq, nCheckpoint) ;

void
embperl_DomTree_iDiscardAfterCheckpoint (nCheckpoint)
    int nCheckpoint
CODE:
    DomTree_discardAfterCheckpoint (pCurrReq, nCheckpoint) ;

#void
#Node_parentNode (xChild)
#    int xChild
#
#void
#Node_firstChild (xChild)
#    int xChild


################################################################################

MODULE = XML::Embperl::DOM      PACKAGE = XML::Embperl::DOM::Element     PREFIX = embperl_Element_


void
embperl_Element_setAttribut (pDomNode, sAttr, sText)
    tDomNode * pDomNode
    SV * sAttr
    SV * sText
PREINIT:
    STRLEN nAttr ;
    STRLEN nText ;
    char * sT  ;
    char * sA  ;
    tDomTree * pDomTree ;
CODE:
    sT = SV2String (sText, nText) ;
    sA = SV2String (sAttr, nAttr) ;

    pDomTree = DomTree_self (pDomNode -> xDomTree) ;

    Element_selfSetAttribut (pDomTree, Node_self (pDomTree, pDomNode -> xNode), pCurrReq -> nCurrRepeatLevel, sA, nAttr, sT, nText) ;


void
embperl_Element_iSetAttribut (xDomTree, xNode, sAttr, sText)
    int xDomTree
    int xNode
    SV * sAttr
    SV * sText
CODE:
    STRLEN nAttr ;
    STRLEN nText ;
    char * sT = SV2String (sText, nText) ;
    char * sA = SV2String (sAttr, nAttr) ;
    tDomTree * pDomTree = DomTree_self (xDomTree) ;

    Element_selfSetAttribut (pDomTree, Node_self (pDomTree, xNode), pCurrReq -> nCurrRepeatLevel, sA, nAttr, sT, nText) ;




void
embperl_Element_removeAttribut (pDomNode, xNode, sAttr)
    tDomNode * pDomNode
    SV * sAttr
PREINIT:
    STRLEN nAttr ;
    char * sA  ;
    tDomTree * pDomTree ;
CODE:
    sA = SV2String (sAttr, nAttr) ;
    pDomTree = DomTree_self (pDomNode -> xDomTree) ;

    Element_selfRemoveAttribut (pDomTree, Node_self (pDomTree, pDomNode -> xNode), pCurrReq -> nCurrRepeatLevel, sA, nAttr) ;


void
embperl_Element_iRemoveAttribut (xDomTree, xNode, sAttr)
    int xDomTree
    int xNode
    SV * sAttr
CODE:
    STRLEN nAttr ;
    char * sA = SV2String (sAttr, nAttr) ;
    tDomTree * pDomTree = DomTree_self (xDomTree) ;

    Element_selfRemoveAttribut (pDomTree, Node_self (pDomTree, xNode), pCurrReq -> nCurrRepeatLevel, sA, nAttr) ;


################################################################################

MODULE = XML::Embperl::DOM      PACKAGE = XML::Embperl::DOM::Attr     PREFIX = embperl_Attr_



SV *
embperl_Attr_value (pAttr)
    tDomNode * pAttr
PREINIT:
    tDomTree * pDomTree  ;
    char * sAttrText = NULL ;
CODE:
    pDomTree = DomTree_self (pAttr -> xDomTree) ;

    Attr_selfValue (pDomTree, Attr_self(pDomTree, pAttr -> xNode), pCurrReq -> nCurrRepeatLevel, &sAttrText) ;
    RETVAL = newSVpv (sAttrText, ArrayGetSize (sAttrText)) ;
    StringFree (&sAttrText) ;
OUTPUT:
    RETVAL


SV *
embperl_Attr_iValue (xDomTree, xAttr)
    int xDomTree
    int xAttr
CODE:
    tDomTree * pDomTree = DomTree_self (xDomTree) ;
    char * sAttrText = NULL ;
    tAttrData * pAttr  ;
    
    lprintf (pCurrReq, "xDomTree=%d, xAttr=%d pDomTree=%x\n", xDomTree, xAttr, pDomTree) ;
    
    pAttr = Attr_self(pDomTree, xAttr) ;
    Attr_selfValue (pDomTree, pAttr , pCurrReq -> nCurrRepeatLevel, &sAttrText) ;
    RETVAL = newSVpv (sAttrText, ArrayGetSize (sAttrText)) ;
    StringFree (&sAttrText) ;
OUTPUT:
    RETVAL

