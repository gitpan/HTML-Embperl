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
#   $Id: Cmd.xs,v 1.1.2.9 2001/10/29 20:07:12 richter Exp $
#
###################################################################################



MODULE = HTML::Embperl::Cmd      PACKAGE = HTML::Embperl::Cmd     PREFIX = embperl_


#void
#embperl_InputText (xDomTree, xNode, sName)
#    int xDomTree
#    int xOldChild
#    char * sName
#CODE:
    


void
embperl_InputCheck (xDomTree, xNode, sName, sValue, bSetInSource)
    int xDomTree
    int xNode
    SV * sName
    SV * sValue
    SV * bSetInSource 
CODE:
    STRLEN nName ;
    STRLEN nValue ;
    char * sN = SV2String (sName, nName) ;
    char * sV = SV2String (sValue, nValue) ;
    embperlCmd_InputCheck (pCurrReq, DomTree_self (xDomTree), xNode, pCurrReq -> nCurrRepeatLevel, sN, nName, sV, nValue, SvOK (bSetInSource)?1:0) ;
    

void
embperl_Option (xDomTree, xNode, sName, sValue, bSetInSource)
    int xDomTree
    int xNode
    SV * sName
    SV * sValue
    SV * bSetInSource 
CODE:
    STRLEN nName ;
    STRLEN nValue ;
    char * sN = SV2String (sName, nName) ;
    char * sV = SV2String (sValue, nValue) ;
    embperlCmd_Option (pCurrReq, DomTree_self (xDomTree), xNode, pCurrReq -> nCurrRepeatLevel, sN, nName, sV, nValue,  SvOK (bSetInSource)?1:0) ;
    

void
embperl_Hidden (xDomTree, xNode, sArg)
    int xDomTree
    int xNode
    char * sArg
CODE:
    embperlCmd_Hidden (pCurrReq, DomTree_self (xDomTree), xNode, pCurrReq -> nCurrRepeatLevel, sArg) ;
    


void
embperl_SubStart (pDomTreeSV, xDomTree, pSaveAV)
    SV * pDomTreeSV 
    int  xDomTree
    AV * pSaveAV
CODE:
    embperl_ExecuteSubStart (pCurrReq, pDomTreeSV, xDomTree, pSaveAV) ;


void
embperl_SubEnd (pDomTreeSV, pSaveAV)
    SV * pDomTreeSV 
    AV * pSaveAV
CODE:
    embperl_ExecuteSubEnd (pCurrReq, pDomTreeSV, pSaveAV) ;

