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
// Errors and Return Codes
//

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
    rcEndtextareaWithoutTextarea,
    rcArgStackOverflow,
    } ;


extern char errdat1 [1024]  ;


//
// Debug Flags
//

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
    
    dbgAll  = 0xffff,
    } ;

#define epDbgStd dbgStd
#define epDbgMem dbgMem
#define epDbgEval dbgEval
#define epDbgCmd dbgCmd
#define epDbgEnv dbgEnv
#define epDbgForm  dbgForm
#define epDbgTab   dbgTab
#define epDbgInput dbgInput
#define epDbgFlushOutput dbgFlushOutput
#define epDbgFlushLog dbgFlushLog
#define epDbgAllCmds  dbgAllCmds
#define epDbgSource   dbgSource


#define epDbgAll   dbgAll


//
// I/O modes
//


#define epIOCGI      1
#define epIOProcess  2
#define epIOMod_Perl 3
#define epIOPerl     4

//
// Table modes
//

#define epTabRow        0x0f   // Row Mask
#define epTabRowDef     0x01   // Last row where last defined expression
#define epTabRowUndef   0x02   // Last row where first undefined expression
#define epTabRowMax     0x03   // maxrow determinates number of rows

#define epTabCol        0xf0   // Column Mask
#define epTabColDef     0x10   // Last column where last defined expression
#define epTabColUndef   0x20   // Last column where first undefined expression
#define epTabColMax     0x30   // maxcol determinates number of columns


//
// exported functions
//


int embperl_init (int  nIOType,
                  const char * sLogFile) ;
int embperl_term (void) ;
int embperl_req  (/*in*/ int    bDebugFlags,
                  /*in*/ SV *   pSVNameSpace) ;

