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



/*
    Errors and Return Codes
*/

enum tRc
    {
    ok = 0,
    rcStackOverflow,
    rcStackUnderflow,
    rcEndifWithoutIf,
    rcElseWithoutIf,
    rcEndwhileWithoutWhile,
    rcEndtableWithoutTable,
    rcCmdNotFound,
    rcOutOfMemory,
    rcPerlVarError,
    rcHashError,
    rcArrayError,
    rcFileOpenErr,    
    rcMissingRight,
    rcNoRetFifo,
    rcMagicError,
    rcWriteErr,
    rcUnknownNameSpace,
    rcInputNotSupported,
    rcCannotUsedRecursive,
    rcEndtableWithoutTablerow,
    rcTablerowOutsideOfTable, 
    rcEndtextareaWithoutTextarea,
    rcArgStackOverflow,
    rcEvalErr,
    rcNotCompiledForModPerl,
    rcLogFileOpenErr,
    rcExecCGIMissing,
    rcIsDir,
    rcXNotSet,
    rcNotFound,
    rcUnknownVarType,
    rcPerlWarn,
    rcVirtLogNotSet,
    rcMissingInput,
    } ;


/*
    Debug Flags
*/

enum dbg
    {
    dbgStd          = 1,
    dbgMem          = 2,
    dbgEval         = 4,
    dbgCmd          = 8,
    dbgEnv          = 16,
    dbgForm         = 32,
    dbgTab          = 64,
    dbgInput        = 128,
    dbgFlushOutput  = 256,
    dbgFlushLog     = 512,
    dbgAllCmds      = 1024,
    dbgSource       = 2048,
    dbgFunc         = 4096,
    dbgLogLink      = 8192,
    dbgDefEval      = 16384,
    dbgCacheDisable     = 0x08000,
    dbgEarlyHttpHeader  = 0x10000,
    dbgWatchScalar      = 0x20000,
    dbgHeadersIn        = 0x40000,
    dbgShowCleanup      = 0x80000,
    
    dbgAll  = -1
    } ;

/*
    Option Flags
*/

enum opt
    {
    optDisableVarCleanup       = 1,
    optDisableEmbperlErrorPage = 2,
    optSafeNamespace           = 4,
    optOpcodeMask              = 8,
    optRawInput                = 16,
    optSendHttpHeader          = 32,
    } ;

/*
    I/O modes
*/

enum epIO
    {
    epIOCGI      = 1,
    epIOProcess  = 2,
    epIOMod_Perl = 3,
    epIOPerl     = 4
    } ;
    
/*
    Table modes
*/

#define epTabRow        0x0f   /* Row Mask */
#define epTabRowDef     0x01   /* Last row where last defined expression */
#define epTabRowUndef   0x02   /* Last row where first undefined expression */
#define epTabRowMax     0x03   /* maxrow determinates number of rows */

#define epTabCol        0xf0   /* Column Mask */
#define epTabColDef     0x10   /* Last column where last defined expression */
#define epTabColUndef   0x20   /* Last column where first undefined expression */
#define epTabColMax     0x30   /* maxcol determinates number of columns */



#define ERRDATLEN 1024

extern char errdat1 [ERRDATLEN]  ;
extern char errdat2 [ERRDATLEN]  ;

/*
    Escape modes
*/


enum tEscMode
    {
    escNone = 0,
    escHtml = 1,
    escUrl  = 2,
    escStd  = 3
    } ;


#if !defined (pid_t) && defined (WIN32)
#define pid_t int
#endif

extern pid_t nPid ;

