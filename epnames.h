/*###################################################################################
#
#   Embperl - Copyright (c) 1997 Gerald Richter / ECOS
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

//
// Avoid namespace conflict with other packages
//


#define oBegin                 EMBPERL_oBegin            
#define oRollback              EMBPERL_oRollback         
#define oCommit                EMBPERL_oCommit           
#define OpenInput              EMBPERL_OpenInput         
#define CloseInput             EMBPERL_CloseInput        
#define iread                  EMBPERL_iread             
#define igets                  EMBPERL_igets             
#define OpenOutput             EMBPERL_OpenOutput        
#define CloseOutput            EMBPERL_CloseOutput       
#define oputs                  EMBPERL_oputs             
#define owrite                 EMBPERL_owrite            
#define oputc                  EMBPERL_oputc             
#define OpenLog                EMBPERL_OpenLog           
#define CloseLog               EMBPERL_CloseLog          
#define FlushLog               EMBPERL_FlushLog          
#define lprintf                EMBPERL_lprintf           
#define lwrite                 EMBPERL_lwrite            
#define _free                  EMBPERL__free             
#define _malloc                EMBPERL__malloc           
#define pReq                   EMBPERL_pReq              
#define bDebug                 EMBPERL_bDebug            
#define nIOType                EMBPERL_nIOType           
#define errdat1                EMBPERL_errdat1           
#define Stack                  EMBPERL_Stack             
#define nStack                 EMBPERL_nStack            
#define State                  EMBPERL_State             
#define ArgStack               EMBPERL_ArgStack          
#define pArgStack              EMBPERL_pArgStack         
#define nTabMode               EMBPERL_nTabMode          
#define nTabMaxRow             EMBPERL_nTabMaxRow        
#define nTabMaxCol             EMBPERL_nTabMaxCol        
#define CmdTab                 EMBPERL_CmdTab 
#define CmdIf                  EMBPERL_CmdIf             
#define CmdElse                EMBPERL_CmdElse           
#define CmdElsif               EMBPERL_CmdElsif          
#define CmdEndif               EMBPERL_CmdEndif          
#define CmdWhile               EMBPERL_CmdWhile          
#define CmdEndwhile            EMBPERL_CmdEndwhile       
#define CmdHidden              EMBPERL_CmdHidden         
#define HtmlTable              EMBPERL_HtmlTable         
#define HtmlList               EMBPERL_HtmlList          
#define HtmlEndtable           EMBPERL_HtmlEndtable      
#define HtmlRow                EMBPERL_HtmlRow           
#define HtmlEndrow             EMBPERL_HtmlEndrow        
#define HtmlInput              EMBPERL_HtmlInput         
#define HtmlTextarea           EMBPERL_HtmlTextarea      
#define HtmlEndtextarea        EMBPERL_HtmlEndtextarea   
#define LogError               EMBPERL_LogError          
#define OutputToHtml           EMBPERL_OutputToHtml      
#define EvalAll                EMBPERL_EvalAll           
#define EvalSafe               EMBPERL_EvalSafe          
#define Eval                   EMBPERL_Eval              
#define EvalNum                EMBPERL_EvalNum           
#define stristr                EMBPERL_stristr           
#define strlower               EMBPERL_strlower          
#define TransHtml              EMBPERL_TransHtml         
#define GetHtmlArg             EMBPERL_GetHtmlArg        
#define GetHashValueLen        EMBPERL_GetHashValueLen   
#define GetHashValue           EMBPERL_GetHashValue      
#define Char2Html              EMBPERL_Char2Html         
#define Html2Char              EMBPERL_Html2Char         
#define sizeHtml2Char          EMBPERL_sizeHtml2Char     
#define nPid                   EMBPERL_nPid 
