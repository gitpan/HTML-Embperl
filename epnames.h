/*###################################################################################
#
#   Embperl - Copyright (c) 1997-1998 Gerald Richter / ECOS
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

/*
    Avoid namespace conflict with other packages
*/


#define oBegin                 EMBPERL_oBegin            
#define oRollback              EMBPERL_oRollback         
#define oRollbackOutput        EMBPERL_oRollbackOutput
#define oCommit                EMBPERL_oCommit           
#define oCommitToMem           EMBPERL_oCommitToMem
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
#define errdat2                EMBPERL_errdat2          
#define Stack                  EMBPERL_Stack             
#define nStack                 EMBPERL_nStack            
#define State                  EMBPERL_State             
#define ArgStack               EMBPERL_ArgStack          
#define pArgStack              EMBPERL_pArgStack         
#define HtmlStack              EMBPERL_HtmlStack             
#define nHtmlStack             EMBPERL_nHtmlStack            
#define HtmlState              EMBPERL_HtmlState             
#define ArgHtmlStack           EMBPERL_ArgHtmlStack          
#define pArgHtmlStack          EMBPERL_pArgHtmlStack         
#define nTabMode               EMBPERL_nTabMode          
#define nTabMaxRow             EMBPERL_nTabMaxRow        
#define nTabMaxCol             EMBPERL_nTabMaxCol        
#define LogError               EMBPERL_LogError          
#define OutputToHtml           EMBPERL_OutputToHtml      
#define Eval                   EMBPERL_Eval              
#define EvalNum                EMBPERL_EvalNum           
#define EvalBool               EMBPERL_EvalBool           
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
#define OutputToMemBuf         EMBPERL_OutputToMemBuf
#define OutputToStd            EMBPERL_OutputToStd
#define GetLogHandle           EMBPERL_GetLogHandle
#define TableStack 	       EMBPERL_TableStack
#define nTableStack 	       EMBPERL_nTableStack
#define TableState 	       EMBPERL_TableState
#define pEvalErr               EMBPERL_pEvalErr   
#define pCacheHash             EMBPERL_pCacheHash 
#define pNameSpace             EMBPERL_pNameSpace
#define pBuf                   EMBPERL_pBuf          
#define pCurrPos               EMBPERL_pCurrPos      
#define pCurrStart             EMBPERL_pCurrStart    
#define pEndPos                EMBPERL_pEndPos       
#define pCurrTag               EMBPERL_pCurrTag      
#define pEnvHash               EMBPERL_pEnvHash      
#define pFormHash              EMBPERL_pFormHash     
#define pFormSplitHash         EMBPERL_pFormSplitHash     
#define pInputHash             EMBPERL_pInputHash    
#define pNameSpaceHash         EMBPERL_pNameSpaceHash
#define pFormArray             EMBPERL_pFormArray    
#define cMultFieldSep          EMBPERL_cMultFieldSep 
#define SearchCmd              EMBPERL_SearchCmd     
#define ProcessCmd             EMBPERL_ProcessCmd    
#define bSafeEval              EMBPERL_bSafeEval
#define sLogfileURLName        EMBPERL_sLogfileURLName
#define numEvals               EMBPERL_numEvals
#define numCacheHits           EMBPERL_numCacheHits 
#define Char2Url               EMBPERL_Char2Url            
#define CmdTab                 EMBPERL_CmdTab              
#define EvalTrans              EMBPERL_EvalTrans           
#define EvalTransFlags         EMBPERL_EvalTransFlags
#define EvalTransOnFirstCall   EMBPERL_EvalTransOnFirstCall           
#define GetContentLength       EMBPERL_GetContentLength    
#define GetLogFilePos          EMBPERL_GetLogFilePos       
#define ReadHTML               EMBPERL_ReadHTML            
#define ScanCmdEvalsInString   EMBPERL_ScanCmdEvalsInString
#define bEscMode               EMBPERL_bEscMode            
#define bEscModeSet            EMBPERL_bEscModeSet            
#define nAllocSize             EMBPERL_nAllocSize          
#define pCurrEscape            EMBPERL_pCurrEscape         
#define pNextEscape            EMBPERL_pNextEscape         
#define bStrict                EMBPERL_bStrict
#define EvalDirect             EMBPERL_EvalDirect
#define bOptions               EMBPERL_bOptions    
#define bReqRunning            EMBPERL_bReqRunning 
#define nEvalPackage           EMBPERL_nEvalPackage
#define pEvalPackage           EMBPERL_pEvalPackage
#define sEvalPackage           EMBPERL_sEvalPackage
#define pPackage               EMBPERL_pPackage    
#define pErrArray              EMBPERL_pErrArray   
#define pOpcodeMask            EMBPERL_pOpcodeMask 
#define sSourcefile            EMBPERL_sSourcefile
#define nSourceline            EMBPERL_nSourceline
#define pSourcelinePos         EMBPERL_nSourcelinePos
#define GetLineNo              EMBPERL_GetLineNo
#define pLineNoCurrPos         EMBPERL_pLineNoCurrPos
#define bError                 EMBPERL_bError
#define lastwarn               EMBPERL_lastwarn
#define lastwarn               EMBPERL_lastwarn
#define Dirname                EMBPERL_Dirname
#define nMarker                EMBPERL_nMarker
#define CommitError            EMBPERL_CommitError
#define RollbackError          EMBPERL_RollbackError
#define pErrFill	       EMBPERL_pErrFill
#define pErrState              EMBPERL_pErrState
#define _memstrcat             EMBPERL__memstrcat
#define _realloc               EMBPERL__realloc


/* --> from mod_perl.h
 * patchlevel.h causes a -Wall warning, 
 * plus chance that another patchlevel.h might be in -I paths
 * so try to avoid it if possible 
 */ 
#ifdef PERL_VERSION
#if PERL_VERSION >= 500476
#define PERL5_005
#endif
#else
#include "patchlevel.h"
#if ((PATCHLEVEL >= 4) && (SUBVERSION >= 76)) || (PATCHLEVEL >= 5)
#define PERL5_005
#endif
#endif /*PERL_VERSION*/

#ifdef PERL5_005
#ifndef rs
#define rs PL_rs
#endif
#ifndef beginav
#define beginav PL_beginav
#endif
#ifndef defoutgv
#define defoutgv PL_defoutgv
#endif
#ifndef defstash
#define defstash PL_defstash
#endif
#ifndef egid
#define egid PL_egid
#endif
#ifndef endav
#define endav PL_endav
#endif
#ifndef envgv
#define envgv PL_envgv
#endif
#ifndef euid
#define euid PL_euid
#endif
#ifndef gid
#define gid PL_gid
#endif
#ifndef hints
#define hints PL_hints
#endif
#ifndef incgv
#define incgv PL_incgv
#endif
#ifndef pidstatus
#define pidstatus PL_pidstatus
#endif
#ifndef scopestack_ix
#define scopestack_ix PL_scopestack_ix
#endif
#ifndef siggv
#define siggv PL_siggv
#endif
#ifndef uid
#define uid PL_uid
#endif
#ifndef warnhook
#define warnhook PL_warnhook
#endif
#ifndef diehook
#define diehook PL_diehook
#endif
#ifndef perl_destruct_level
#define perl_destruct_level PL_perl_destruct_level 
#endif
#ifndef sv_count
#define sv_count PL_sv_count
#endif
#ifndef sv_objcount
#define sv_objcount PL_sv_objcount
#endif
#ifndef op_mask
#define op_mask PL_op_mask
#endif
#ifndef maxo
#define maxo PL_maxo
#endif
#endif /* PERL5_005 */
