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
#define LogError               EMBPERL_LogError          
#define OutputToHtml           EMBPERL_OutputToHtml      
#define Eval                   EMBPERL_Eval              
#define EvalNum                EMBPERL_EvalNum           
#define EvalBool               EMBPERL_EvalBool           
#define stristr                EMBPERL_stristr           
#define strlower               EMBPERL_strlower          
#define TransHtml              EMBPERL_TransHtml         
#define TransHtmlSV            EMBPERL_TransHtmlSV
#define GetHtmlArg             EMBPERL_GetHtmlArg        
#define GetHashValueLen        EMBPERL_GetHashValueLen   
#define GetHashValue           EMBPERL_GetHashValue      
#define Char2Html              EMBPERL_Char2Html         
#define Html2Char              EMBPERL_Html2Char         
#define sizeHtml2Char          EMBPERL_sizeHtml2Char     
#define OutputToMemBuf         EMBPERL_OutputToMemBuf
#define OutputToStd            EMBPERL_OutputToStd
#define GetLogHandle           EMBPERL_GetLogHandle
#define SearchCmd              EMBPERL_SearchCmd     
#define ProcessCmd             EMBPERL_ProcessCmd    
#define ProcessSub             EMBPERL_ProcessSub
#define Char2Url               EMBPERL_Char2Url            
#define CmdTab                 EMBPERL_CmdTab              
#define EvalTrans              EMBPERL_EvalTrans           
#define EvalMain               EMBPERL_EvalMain
#define EvalTransFlags         EMBPERL_EvalTransFlags
#define EvalTransOnFirstCall   EMBPERL_EvalTransOnFirstCall           
#define EvalSub                EMBPERL_EvalSub
#define GetContentLength       EMBPERL_GetContentLength    
#define GetLogFilePos          EMBPERL_GetLogFilePos       
#define ReadHTML               EMBPERL_ReadHTML            
#define ScanCmdEvalsInString   EMBPERL_ScanCmdEvalsInString
#define EvalDirect             EMBPERL_EvalDirect
#define GetLineNo              EMBPERL_GetLineNo
#define Dirname                EMBPERL_Dirname
#define CommitError            EMBPERL_CommitError
#define RollbackError          EMBPERL_RollbackError
#define _memstrcat             EMBPERL__memstrcat
#define _ep_strdup             EMBPERL__ep_strdup
#define _ep_strndup            EMBPERL__ep_strndup
#define _realloc               EMBPERL__realloc
#define ExecuteReq             EMBPERL_ExecuteReq     
#define FreeConfData           EMBPERL_FreeConfData   
#define FreeRequest            EMBPERL_FreeRequest    
#define GetHashValueInt        EMBPERL_GetHashValueInt
#define GetHashValueStr        EMBPERL_GetHashValueStr
#define Init                   EMBPERL_Init           
#define ResetHandler           EMBPERL_ResetHandler   
#define SetupConfData          EMBPERL_SetupConfData  
#define SetupFileData          EMBPERL_SetupFileData  
#define SetupRequest           EMBPERL_SetupRequest   
#define Term                   EMBPERL_Term           
#define sstrdup                EMBPERL_sstrdup        
#define ProcessBlock           EMBPERL_ProcessBlock
#define NewEscMode             EMBPERL_NewEscMode
#define GetSubTextPos          EMBPERL_GetSubTextPos
#define SetSubTextPos          EMBPERL_SetSubTextPos
#define SetupDebugger          EMBPERL_SetupDebugger


#define InitialReq             EMBPERL_InitialReq
#define pCurrReq               EMBPERL_pCurrReq

#ifndef PERL_VERSION
#include <patchlevel.h>
#define PERL_VERSION PATCHLEVEL
#define PERL_SUBVERSION SUBVERSION
#endif


#if PERL_VERSION >= 5

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

#if PERL_SUBVERSION >= 50 || PERL_VERSION >= 6

#ifndef na
#define na PL_na
#endif
#ifndef sv_undef
#define sv_undef PL_sv_undef
#endif
#ifndef tainted
#define tainted PL_tainted
#endif

#endif


#else  /* PERL_VERSION > 5 */

#ifndef ERRSV
#define ERRSV GvSV(errgv)
#endif

#ifndef dTHR
#define dTHR
#endif


#endif /* PERL_VERSION > 5 */
