
###################################################################################
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
###################################################################################


package HTML::Embperl;



require Cwd ;

require Exporter;
require DynaLoader;

@ISA = qw(Exporter DynaLoader);


$VERSION = '1.1.1';


bootstrap HTML::Embperl $VERSION;



# Default logfilename

$DefaultLog = '/tmp/embperl.log' ;
$Outputfile = '' ;  # Default to stdout


%cache    = () ;    # cache for evaled code
%filepack = () ;    # translate filename to packagename
$package  = '' ;    # package name for current document
$packno   = 1 ;     # for assigning unique packagenames
@cleanups = () ;    # packages which need a cleanup
@errfill  = () ;    # predefine -> avoid waring
@errstate = () ;    # predefine -> avoid waring
$LogOutputFileno = 0 ;

# setup constans

use constant dbgAll                 => -1 ;
use constant dbgAllCmds             => 1024 ;
use constant dbgCacheDisable        => 32768 ;
use constant dbgCmd                 => 8 ;
use constant dbgDefEval             => 16384 ;
use constant dbgEarlyHttpHeader     => 65536 ;
use constant dbgEnv                 => 16 ;
use constant dbgEval                => 4 ;
use constant dbgFlushLog            => 512 ;
use constant dbgFlushOutput         => 256 ;
use constant dbgForm                => 32 ;
use constant dbgFunc                => 4096 ;
use constant dbgHeadersIn           => 262144 ;
use constant dbgInput               => 128 ;
use constant dbgLogLink             => 8192 ;
use constant dbgMem                 => 2 ;
use constant dbgShowCleanup         => 524288 ;
use constant dbgSource              => 2048 ;
use constant dbgStd                 => 1 ;
use constant dbgTab                 => 64 ;
use constant dbgWatchScalar         => 131072 ;

use constant epIOCGI                => 1 ;
use constant epIOMod_Perl           => 3 ;
use constant epIOPerl               => 4 ;
use constant epIOProcess            => 2 ;

use constant escHtml                => 1 ;
use constant escNone                => 0 ;
use constant escStd                 => 3 ;
use constant escUrl                 => 2 ;


use constant optDisableChdir            => 128 ;
use constant optDisableEmbperlErrorPage => 2 ;
use constant optDisableFormData         => 256 ;
use constant optDisableHtmlScan         => 512 ;
use constant optDisableInputScan        => 1024 ;
use constant optDisableMetaScan         => 4096 ;
use constant optDisableTableScan        => 2048 ;
use constant optDisableVarCleanup       => 1 ;
use constant optEarlyHttpHeader         => 64 ;
use constant optOpcodeMask              => 8 ;
use constant optRawInput                => 16 ;
use constant optSafeNamespace           => 4 ;
use constant optSendHttpHeader          => 32 ;
use constant optAllFormData             => 8192 ;
use constant optRedirectStdout          => 16384 ;
use constant optUndefToEmptyValue       => 32768 ;


use constant ok                     => 0 ;
use constant rcArgStackOverflow => 23 ;
use constant rcArrayError => 11 ;
use constant rcCannotUsedRecursive => 19 ;
use constant rcCmdNotFound => 7 ;
use constant rcElseWithoutIf => 4 ;
use constant rcEndifWithoutIf => 3 ;
use constant rcEndtableWithoutTable => 6 ;
use constant rcEndtableWithoutTablerow => 20 ;
use constant rcEndtextareaWithoutTextarea => 22 ;
use constant rcEndwhileWithoutWhile => 5 ;
use constant rcEvalErr => 24 ;
use constant rcExecCGIMissing => 27 ;
use constant rcFileOpenErr => 12 ;
use constant rcHashError => 10 ;
use constant rcInputNotSupported => 18 ;
use constant rcIsDir => 28 ;
use constant rcLogFileOpenErr => 26 ;
use constant rcMagicError => 15 ;
use constant rcMissingRight => 13 ;
use constant rcNoRetFifo => 14 ;
use constant rcNotCompiledForModPerl => 25 ;
use constant rcNotFound => 30 ;
use constant rcOutOfMemory => 8 ;
use constant rcPerlVarError => 9 ;
use constant rcPerlWarn => 32 ;
use constant rcStackOverflow => 1 ;
use constant rcStackUnderflow => 2 ;
use constant rcUnknownNameSpace => 17 ;
use constant rcUnknownVarType => 31 ;
use constant rcVirtLogNotSet => 33 ;
use constant rcWriteErr => 16 ;
use constant rcXNotSet => 29 ;


$DebugDefault = dbgStd ;

# Color definition for logfile output

%LogFileColors = (
    'EVAL<' => '#FF0000',
    'EVAL>' => '#FF0000',
    'FORM:' => '#0000FF',
    'CMD:'  => '#FF8000',
    'SRC:'  => '#808080',
    'TAB:'  => '#FF0080',
    'INPU:' => '#008040',
                ) ;


#######################################################################################
#
# tie for logfile output
#

    {
    package HTML::Embperl::Log ;


    sub TIEHANDLE 

        {
        my $class ;
        
        return bless \$class, shift ;
        }


    sub PRINT

        {
        shift ;
        HTML::Embperl::embperl_log(join ('', @_)) ;
        }

    sub PRINTF

        {
        shift ;
        my $fmt = shift ;
        HTML::Embperl::embperl_log(sprintf ($fmt, @_)) ;
        }
    }



#######################################################################################
#
# tie for output
#

    {
    package HTML::Embperl::Out ;


    sub TIEHANDLE 

        {
        my $class ;
        
        return bless \$class, shift ;
        }


    sub PRINT

        {
        shift ;
        HTML::Embperl::embperl_output(join ('', @_)) ;
        }

    sub PRINTF

        {
        shift ;
        my $fmt = shift ;
        HTML::Embperl::embperl_output(sprintf ($fmt, @_)) ;
        }
    }




#######################################################################################
#
# init on startup
#

$DefaultLog = $ENV{EMBPERL_LOG} || $DefaultLog ;
if (defined ($ENV{MOD_PERL}))
    { 
    eval 'use Apache' ; # make sure Apache.pm is loaded (is not at server startup in mod_perl < 1.11)
    die "use Apache failed: $@" if ($@); 
    eval 'use Apache::Constants qw(:common &OPT_EXECCGI)' ;
    die "use Apache::Constants failed: $@" if ($@); 
    embperl_init (epIOMod_Perl, $DefaultLog) ;
    }
else
    {
    eval 'sub OK        { 0 ;   }' ;
    eval 'sub NOT_FOUND { 404 ; }' ;
    eval 'sub FORBIDDEN { 401 ; }' ;
    embperl_init (epIOPerl, $DefaultLog) ;
    }

$cwd       = Cwd::fastcwd();

tie *LOG, 'HTML::Embperl::Log' ;
tie *OUT, 'HTML::Embperl::Out' ;


#######################################################################################

#no strict ;

sub _eval_ ($)
    {
    my $result = eval "package $evalpackage ; $_[0] " ;
    if ($@ ne '')
        { embperl_logevalerr ($@) ; }
    return $result ;
    }

#use strict ;

#######################################################################################

sub _evalsub_ ($)
    {
    my $result = eval "package $evalpackage ; sub { $_[0] } " ;
    if ($@ ne '')
        { embperl_logevalerr ($@) ; }
    return $result ;
    }

#######################################################################################

sub Warn 
    {
    local $^W = 0 ;
    my $msg = $_[0] ;
    chop ($msg) ;
    
    my $lineno = embperl_getlineno () ;
    if ($msg =~ /HTML\/Embperl/)
        {
        $msg =~ s/at (.*?) line (\d*)/at $Inputfile in block starting at line $lineno/ ;
        }
    embperl_logerror (rcPerlWarn, $msg);
    }

#######################################################################################

sub AddCompartment ($)

    {
    my ($sName) = @_ ;
    my $cp ;
    
    return $cp if (defined ($cp = $NameSpace{$sName})) ;

    eval 'require Safe' ;
    die "require Safe failed: $@" if ($@); 

    $cp = new Safe ($sName) ;
    
    $NameSpace{$sName} = $cp ;

    $cp -> share ('&_evalsub_', '&_eval_') ;

    return $cp ;
    }


#######################################################################################

sub SendLogFile ($$)

    {
    my ($fn, $info) = @_ ;
    
    my $lastpid = 0 ;
    my ($filepos, $pid, $src) = split (/&/, $info) ;
    my $cnt = 0 ;
    my $ecnt = 0 ;
    my $tag ;
    my $fontcol ;

    $escmode = 3 ;
        
    open (LOGFILE, $fn) || return 500 ;

    seek (LOGFILE, $filepos, 0) || return 500 ;

    print "<HTML><HEAD><TITLE>Embperl Logfile</TITLE></HEAD><BODY bgcolor=\"#FFFFFF\">\r\n" ;
    print "<font color=0>" ;
    print "Logfile = $fn, Position = $filepos, Pid = $pid<BR><CODE>\r\n" ;
    $fontcol = 0 ;

    while (<LOGFILE>)
        {
        /^\[(\d+)\](.*?)\s/ ;
        $cnt++ ;
        $tag = $2 ;
        if ($1 == $pid && (!defined($src) || $tag eq $src))
            {
            if (defined ($LogFileColors{$tag}))
                {
                if ($fontcol ne $LogFileColors{$tag})
                    {
                    $fontcol = $LogFileColors{$tag} ;
                    print "</font><font color=\"$fontcol\">" ;
                    }
                }
            else
                {
                if ($fontcol ne '0')
                    {
                    $fontcol = '0' ;
                    print "</font><font color=0>" ;
                    }
                }
            s/\n/\\<BR\\>\r\n/ ;
            if (defined($src) && ($tag eq $src || $tag eq 'ERR:'))
                {
                if ($tag eq 'ERR:')
                    { print "<A HREF=\"$ENV{EMBPERL_VIRTLOG}?$filepos&$pid#E$ecnt\">" ; $ecnt++ ; }
                else
                    { print "<A HREF=\"$ENV{EMBPERL_VIRTLOG}?$filepos&$pid#N$cnt\">" ;  }
                }
            else
                {
                if ($tag eq 'ERR:')
                    { print "<A NAME=\"E$ecnt\">" ; $ecnt++; }
                else
                    { print "<A NAME=\"N$cnt\">" ; }
                }

            embperl_output ("$_") ;
            print '</A>' ;
            last if (/Request finished/) ;
            }
        }

    print "</CODE></BODY></HTML>\r\n\r\n" ;

    close (LOGFILE) ;

    return 200 ;
    }

#######################################################################################

sub SendErrorDoc ()

    {
    local $SIG{__WARN__} = 'Default' ;
    my $err ;
    my $cnt = 0 ;
    local $escmode = 0 ;
    my $time = localtime ;
    my $mail = $req_rec -> server -> server_admin if (defined ($req_rec)) ;
    $mail ||= '' ;
    my $virtlog = $ENV{EMBPERL_VIRTLOG} || '' ;
    my $url     = $LogfileURL || '' ;

    $req_rec -> content_type('text/html') if (defined ($req_rec)) ;

    embperl_output ("<HTML><HEAD><TITLE>Embperl Error</TITLE></HEAD><BODY bgcolor=\"#FFFFFF\">\r\n$url") ;
    embperl_output ("<H1>Internal Server Error</H1>\r\n") ;
    embperl_output ("The server encountered an internal error or misconfiguration and was unable to complete your request.<P>\r\n") ;
    embperl_output ("Please contact the server administrator, $mail and inform them of the time the error occurred, and anything you might have done that may have caused the error.<P><P>\r\n") ;

    if (defined ($LogfileURL) && $virtlog ne '')
        {
        foreach $err (@errors)
            {
            embperl_output ("<A HREF=\"$virtlog?$logfilepos&$$#E$cnt\">") ; #<tt>") ;
            $escmode = 3 ;
            $err =~ s|\n|\n\\<br\\>\\&nbsp;\\&nbsp;\\&nbsp;\\&nbsp;|g;
            $err =~ s|(Line [0-9]*:)|$1\\</a\\>|;
            embperl_output ($err) ;
            $escmode = 0 ;
            embperl_output ("<p>\r\n") ;
            #embperl_output ("</tt><p>\r\n") ;
            $cnt++ ;
            }
        }
    else
        {
	$escmode = 3 ;
        foreach $err (@errors)
            {
            $err =~ s|\n|\n\\<br\\>\\&nbsp;\\&nbsp;\\&nbsp;\\&nbsp;|g;
            embperl_output ("$err\\<p\\>\r\n") ;
            #embperl_output ("\\<tt\\>$err\\</tt\\>\\<p\\>\r\n") ;
            $cnt++ ;
            }
	$escmode = 0 ;
        }
         
    my $server = $ENV{SERVER_SOFTWARE} || 'Offline' ;

    embperl_output ("$server HTML::Embperl $VERSION [$time]<P>\r\n") ;
    embperl_output ("</BODY></HTML>\r\n\r\n") ;

    }


#######################################################################################


sub CheckFile

    {
    my ($filename) = @_ ;


    unless (-r $filename && -s _)
        {
	embperl_logerror (rcNotFound, $filename);
	return &NOT_FOUND ;
        }

    if (defined ($req_rec) && !($req_rec->allow_options & &OPT_EXECCGI))
        {
	embperl_logerror (rcExecCGIMissing, $filename);
	return &FORBIDDEN ;
 	}
	
    if (-d _)
        {
	embperl_logerror (rcIsDir, $filename);
	return &FORBIDDEN ;
	}                 
    
    return ok ;
    }

#######################################################################################

sub CheckCache

    {
    my ($filename, $mtime, $pack) = @_ ;



    # Escape everything into valid perl identifiers
    if (!defined ($pack))
        {
        $package = $filepack{$filename} ;
        if (!defined($package))
            {
            $package = "HTML::Embperl::DOC::_$packno" ;
            $packno++ ;
            $filepack{$filename} = $package ;
            }
        }
    else
        {
        $filepack{$filename} = $package = $pack ;
        }



    if (!defined($mtime{$filename}) || !defined($mtime) || $mtime{$filename} != $mtime) 
        {
        # clear out any entries in the cache
        delete $cache{$filename} ;
 	$mtime{$filename} = $mtime ;
        # This will remove the functions to avoid redefinition warnings
        # but the symbol table entrys still remain
        #Apache::Symbol::undef_functions ($package) if (defined(&Apache::Symbol::undef_functions)) ;
        # setup one function, so we can check later if it was undef'd by another request
        #eval "sub $package\:\:__info__ { my %info = ('filename' => '$filename', 'mtime' => $mtime) ; } " ;
        print LOG  "[$$]MEM: Reload $filename in $package\n" if ($Debugflags);
        }
    
    if (!defined(${"$package\:\:row"}))
        { # create new aliases for Embperl magic vars
        *{"$package\:\:fdat"}    = \%fdat ;
        *{"$package\:\:fsplitdat"}  = \%fsplitdat ;
        *{"$package\:\:ffld"}    = \@ffld ;
        *{"$package\:\:idat"}    = \%idat ;
        *{"$package\:\:cnt"}     = \$cnt ;
        *{"$package\:\:row"}     = \$row ;
        *{"$package\:\:col"}     = \$col ;
        *{"$package\:\:maxrow"}  = \$maxrow ;
        *{"$package\:\:maxcol"}  = \$maxcol ;
        *{"$package\:\:tabmode"} = \$tabmode ;
        *{"$package\:\:escmode"} = \$escmode ;
    	*{"$package\:\:req_rec"} = \$req_rec if defined ($req_rec) ;
    	*{"$package\:\:exit"}    = \&Apache::exit if defined (&Apache::exit) ;
        
        *{"$package\:\:MailFormTo"} = \&MailFormTo ;

        tie *{"$package\:\:LOG"}, 'HTML::Embperl::Log' ;
        tie *{"$package\:\:OUT"}, 'HTML::Embperl::Out' ;

        *{"$package\:\:optDisableChdir"}                = \$optDisableChdir                   ;
        *{"$package\:\:optDisableEmbperlErrorPage"}     = \$optDisableEmbperlErrorPage ;
        *{"$package\:\:optDisableFormData"}             = \$optDisableFormData         ;
        *{"$package\:\:optDisableHtmlScan"}             = \$optDisableHtmlScan         ;
        *{"$package\:\:optDisableInputScan"}            = \$optDisableInputScan        ;
        *{"$package\:\:optDisableMetaScan"}             = \$optDisableMetaScan         ;
        *{"$package\:\:optDisableTableScan"}            = \$optDisableTableScan        ;
        *{"$package\:\:optDisableVarCleanup"}           = \$optDisableVarCleanup       ;
        *{"$package\:\:optEarlyHttpHeader"}             = \$optEarlyHttpHeader         ;
        *{"$package\:\:optOpcodeMask"}                  = \$optOpcodeMask              ;
        *{"$package\:\:optRawInput"}                    = \$optRawInput                ;
        *{"$package\:\:optSafeNamespace"}               = \$optSafeNamespace           ;
        *{"$package\:\:optSendHttpHeader"}              = \$optSendHttpHeader          ;
        *{"$package\:\:optAllFormData"}                 = \$optAllFormData ;
        *{"$package\:\:optRedirectStdout"}              = \$optRedirectStdout ;
        *{"$package\:\:optUndefToEmptyValue"}           = \$optUndefToEmptyValue ;
        

        *{"$package\:\:dbgAllCmds"}               = \$dbgAllCmds           ;
        *{"$package\:\:dbgCacheDisable"}          = \$dbgCacheDisable      ;
        *{"$package\:\:dbgCmd"}                   = \$dbgCmd               ;
        *{"$package\:\:dbgDefEval"}               = \$dbgDefEval           ;
        *{"$package\:\:dbgEarlyHttpHeader"}       = \$dbgEarlyHttpHeader   ;
        *{"$package\:\:dbgEnv"}                   = \$dbgEnv               ;
        *{"$package\:\:dbgEval"}                  = \$dbgEval              ;
        *{"$package\:\:dbgFlushLog"}              = \$dbgFlushLog          ;
        *{"$package\:\:dbgFlushOutput"}           = \$dbgFlushOutput       ;
        *{"$package\:\:dbgForm"}                  = \$dbgForm              ;
        *{"$package\:\:dbgFunc"}                  = \$dbgFunc              ;
        *{"$package\:\:dbgHeadersIn"}             = \$dbgHeadersIn         ;
        *{"$package\:\:dbgInput"}                 = \$dbgInput             ;
        *{"$package\:\:dbgLogLink"}               = \$dbgLogLink           ;
        *{"$package\:\:dbgMem"}                   = \$dbgMem               ;
        *{"$package\:\:dbgShowCleanup"}           = \$dbgShowCleanup       ;
        *{"$package\:\:dbgSource"}                = \$dbgSource            ;
        *{"$package\:\:dbgStd"}                   = \$dbgStd               ;
        *{"$package\:\:dbgTab"}                   = \$dbgTab               ;
        *{"$package\:\:dbgWatchScalar"}           = \$dbgWatchScalar       ;

        #print LOG  "[$$]MEM: Created Aliases for $package\n" ;
        }


    $cache{$filename}{'~-1'} = 1 ; # make sure hash is defined 
    
    return \$cache{$filename} ;
    }


##########################################################################################

sub ScanEnvironement

    {
    my $req = shift ; 
    
    %ENV = %{{$req_rec->cgi_env, %ENV}} if (defined ($req_rec)) ;
    
    $$req{'virtlog'}     = $ENV{EMBPERL_VIRTLOG}     if (exists ($ENV{EMBPERL_VIRTLOG})) ;
    $$req{'compartment'} = $ENV{EMBPERL_COMPARTMENT} if (exists ($ENV{EMBPERL_COMPARTMENT})) ;
    $$req{'package'}     = $ENV{EMBPERL_PACKAGE}     if (exists ($ENV{EMBPERL_PACKAGE})) ;
    $$req{'input_func'}  = $ENV{EMBPERL_INPUT_FUNC}  if (exists ($ENV{EMBPERL_INPUT_FUNC})) ;
    $$req{'output_func'} = $ENV{EMBPERL_OUTPUT_FUNC} if (exists ($ENV{EMBPERL_OUTPUT_FUNC})) ;
    $$req{'debug'}       = $ENV{EMBPERL_DEBUG}   || 0 ;
    $$req{'options'}     = $ENV{EMBPERL_OPTIONS} || 0 ;
    $$req{'log'}         = $ENV{EMBPERL_LOG}     || $DefaultLog ;

    if (defined($ENV{EMBPERL_ESCMODE}))
        { $$req{'escmode'}    = $ENV{EMBPERL_ESCMODE} }
    else
        { $$req{'escmode'}    = escStd ; }
    
    }



#######################################################################################



sub Execute
    
    {
    my $rc ;
    my $req = shift ;
    
    $Debugflags   = defined ($$req{'debug'})?$$req{'debug'}:$DebugDefault ;
    $req_rec      = $$req{'req_rec'} if (defined ($$req{'req_rec'})) ;
    $rc = embperl_setreqrec ($req_rec) if (defined ($req_rec)) ;
        
    undef $package if (defined ($package)) ; 
   
    if (defined ($$req{'virtlog'}) && $$req{'virtlog'} eq $$req{'uri'})
        {
        if (defined ($req_rec))
            {
            $req_rec -> content_type ('text/html') ;
            $req_rec -> send_http_header ;
            }
        $rc = SendLogFile ($DefaultLog, $ENV{QUERY_STRING}) ;
        embperl_resetreqrec () ;
        return $rc ;
        }


    if (defined($$req{'escmode'}))
        { $escmode    = $$req{'escmode'} }
    else
        { $escmode    = escStd ; }
    
    my $Outputfile = $$req{'outputfile'} || '' ;
    my $Options    = $$req{'options'}    || 0 ;
    my $cleanup    = $$req{'cleanup'}    || 0 ;
    my $In         = $$req{'input'}   ;
    my $Out        = $$req{'output'}  ;
    my $ns ;
    my $pcodecache ;
    my $filesize ;
    my $mtime ;
    my $InFunc ;
    my $OutFunc ;
    my $InData ;
    my $OutData ;

     if (exists $$req{'input_func'})  
        {
        my @p ;
        $In = \$InData ;
        $$req{mtime} = 0 ;
        @p = split (/\s*\,\s*/, $$req{'input_func'}) ;
        $InFunc = shift @p ;
        eval {$rc = &{$InFunc} ($req_rec, $In, \$$req{mtime}, @p)} ;
        if ($rc || $@)
            {
            if ($@) 
                {
                $rc = 500 ;
                print LOG "[$$]ERR:  $@\n" 
                }

            embperl_resetreqrec () ;
            return $rc ;
            }
        }

    $Out = \$OutData if (exists $$req{'output_func'}) ;

    $Inputfile    = $$req{'inputfile'} ;
    
    if (defined ($In))
        {
        $filesize = -1 ;
        $mtime    = $$req{'mtime'} ;
        }
   else
        {
        if ($rc = CheckFile ($Inputfile)) 
            {
            embperl_resetreqrec () ;
            return $rc ;
            }
        $filesize = -s _ ;
        $mtime = -M _ ;
        }

    $pcodecache = CheckCache ($Inputfile, $mtime, $$req{'package'}) ;
    if (defined ($ns = $$req{'compartment'}))
        {
        my $cp = AddCompartment ($ns) ;
        $opcodemask = $cp -> mask ;
        }
    else
        {
        undef $opcodemask if (defined ($opcodemask)) ;
        }
   
    if (($Options & optSafeNamespace))
	{ $evalpackage = 'main' ; }
    else
	{ $evalpackage = $package ; }


    if (!($Options & optDisableFormData) &&
           defined($ENV{'CONTENT_TYPE'}) &&
           $ENV{'CONTENT_TYPE'}=~m|^multipart/form-data|)
        { # just let CGI.pm read the multipart form data, see cgi docu
        eval 'require CGI' ;
        die "require CGI failed: $@" if ($@); 

	my $cgi = new CGI;
	    
        @ffld = $cgi->param;
    
    	foreach ( @ffld )
            {
    	    $fdat{ $_ } = $cgi->param( $_ );
            }

        }
    else
        {
        local $^W = 0 ;
        @ffld = @{$$req{'ffld'}} ;
        %fdat = %{$$req{'fdat'}} ;
        }

    # pass parameters via @param
    *{"$package\:\:param"}   = $$req{'param'} if (exists $$req{'param'}) ;
    
    @errors = () ;


    if ($Debugflags & dbgLogLink)
	{
        embperl_logerror (rcVirtLogNotSet, '') if (!defined($$req{'virtlog'})) ;
        $logfilepos = embperl_getlogfilepos () ;
        $LogfileURL = "<A HREF=\"$$req{'virtlog'}?$logfilepos&$$\">Logfile</A> / <A HREF=\"$ENV{EMBPERL_VIRTLOG}?$logfilepos&$$&SRC:\">Source only</A> / <A HREF=\"$ENV{EMBPERL_VIRTLOG}?$logfilepos&$$&EVAL\<\">Eval only</A><BR>" ;
	}
    else
	{ undef $LogfileURL if (defined ($LogfileURL)) ; }    

        {
        local $SIG{__WARN__} = \&Warn ;
        local *0 = \$Inputfile;
        my $oldfh = select (OUT) if ($Options & optRedirectStdout) ;

        $rc = embperl_req ($Inputfile, $Outputfile, $Debugflags, $Options, $filesize, $pcodecache, $In, $Out) ;
        
        select ($oldfh) if ($Options & optRedirectStdout) ;
        
        if (exists $$req{'output_func'}) 
            {
            my @p ;
            ($OutFunc, @p) = split (/\s*,\s*/, $$req{'output_func'}) ;
            eval { &$OutFunc ($req_rec, $Out,@p) } ;
            if ($@) 
                {
                print LOG "[$$]ERR:  $@\n" 
                }

            }
        }


    undef *{"$package\:\:param"} ;

    if ($cleanup == -1)
        { ; } 
    elsif ($cleanup == 0 && defined ($req_rec))
        {
        push @cleanups, $package ;
        $req_rec -> register_cleanup(\&HTML::Embperl::cleanup) if ($#cleanups == 0) ;
        }
    else
        {
        push @cleanups, $package ;
        cleanup () ;
        }

    return 0 ;
    }

#######################################################################################


sub Init

    {
    my $Logfile   = shift ;
    $DebugDefault = shift ;
    $DebugDefault = dbgStd if (!defined ($DebugDefault)) ;
        
    embperl_init (epIOPerl, $Logfile || $DefaultLog) ;
    
    tie *LOG, 'HTML::Embperl::Log' ;
    }

#######################################################################################


sub Term

    {
    cleanup () ;
    embperl_term () ;
    }


#######################################################################################


sub run (\@)
    
    {
    my ($args) = @_ ;
    my $Logfile    = $ENV{EMBPERL_LOG} || $DefaultLog ;
    my $Daemon     = 0 ;
    my $Cgi        = $#{$args} >= 0?0:1 ;
    my $rc         = 0 ;
    my $log ;
    my $cgi ;
    my $ioType ;
    my %req ;

    $Inputfile  = '' ;
    undef $req_rec ;

    ScanEnvironement (\%req) ;
    

    if (defined ($$args[0]) && $$args[0] eq 'dbgbreak') 
    	{
    	shift @$args ;
    	embperl_dbgbreak () ;
    	}

    while ($#{$args} >= 0)
    	{
    	if ($$args[0] eq '-o')
    	    {
    	    shift @$args ;
    	    $req{'outputfile'} = shift @$args ;	
            }
    	elsif ($$args[0] eq '-l')
    	    {
    	    shift @$args ;
    	    $Logfile = shift @$args ;	
            }
    	elsif ($$args[0] eq '-d')
    	    {
    	    shift @$args ;
    	    $req{'debug'} = shift @$args ;	
	    }
    	elsif ($$args[0] eq '-D')
    	    {
    	    shift @$args ;
    	    $Daemon = 1 ;	
	    }
	else
	    {
	    last ;
	    }
	}
    
    if ($#{$args} >= 0)
    	{
    	$req{'inputfile'} = shift @$args ;
    	}		
    if ($#{$args} >= 0)
    	{
        $ENV{QUERY_STRING} = shift @$args ;
        undef $ENV{CONTENT_LENGTH} if (defined ($ENV{CONTENT_LENGTH})) ;
    	}		
	
    if ($Daemon)
        {
        $Logfile = '' || $ENV{EMBPERL_LOG};   # log to stdout
        $ioType = epIOProcess ;
        $req{'outputfile'} = $ENV{__RETFIFO} ;
        }
    elsif ($Cgi)
        {
        $req{'inputfile'} = $ENV{PATH_TRANSLATED} ;
        $ioType = epIOCGI ;
        }
    else
        {
        $ioType = epIOPerl ;
        }


    embperl_init ($ioType, $Logfile) ;

    
    tie *LOG, 'HTML::Embperl::Log' ;

    $req{'uri'} = $ENV{SCRIPT_NAME} ;

    $req{'cleanup'} = 1 ;
    $req{'cleanup'} = -1 if (($req{'options'} & optDisableVarCleanup)) ;
    $req{'options'} |= optSendHttpHeader ;

    $rc = Execute (\%req) ;

    #close LOG ;
    embperl_term () ;

    return $rc ;
    }


#######################################################################################



sub handler 
    
    {
    #log_svs ("handler entry") ;

    $req_rec = shift ;

    my %req ;

    ScanEnvironement (\%req) ;
    
    undef $package if (defined ($package)) ; 
    
    $req{'uri'}       = $req_rec -> Apache::uri ;

    if (exists $ENV{EMBPERL_FILESMATCH} && 
                         !($req{'uri'} =~ m{$ENV{EMBPERL_FILESMATCH}})) 
        {
        # Reset the perl-handler to work with older mod_perl versions
        embperl_setreqrec ($req_rec) ;
        embperl_resetreqrec (1) ;
        return &DECLINED ;
        }


    $req{'inputfile'} = $ENV{PATH_TRANSLATED} = $req_rec -> filename ;

    #print LOG "i = $req{'inputfile'}\n" ;

    $req{'cleanup'} = -1 if (($req{'options'} & optDisableVarCleanup)) ;
    $req{'options'} |= optSendHttpHeader ;

    my $rc = Execute (\%req) ;

    #log_svs ("handler exit") ;
    return $rc ;
    }

#######################################################################################

sub cleanup 
    {
    #log_svs ("cleanup entry") ;
    my $glob ;
    my $val ;
    my $key ;
    local $^W = 0 ;
    my $package ;

    foreach $package (@cleanups)
        {
        next if ($package eq '') ;

	print LOG "[$$]CUP:  Cleanup package: $package\n" if ($Debugflags & dbgShowCleanup);
        if (defined (&{"$package\:\:CLEANUP"}))
	    {
    	    eval "\&$package\:\:CLEANUP;" ;
	    print LOG "[$$]CUP:  Call \&$package\:\:CLEANUP;\n" if ($Debugflags & dbgShowCleanup);
	    embperl_logevalerr ($@) if ($@) ;
	    }


        if ($Debugflags & dbgShowCleanup)
            {
	    while (($key,$val) = each(%{*{"$package\::"}}))
                {
	        local(*ENTRY) = $val;
                $glob = $package.'::'.$key ;
                if (defined (*ENTRY{SCALAR})) 
                    {
                    print LOG "[$$]CUP:  \$$key = ${$glob}\n" ;
                    undef ${$glob} ;
                    }
                if (defined (*ENTRY{HASH}) && !($key =~ /\:\:$/))
                    {
                    print LOG "[$$]CUP:  \%$key\n" ;
                    undef %{$glob} ;
                    }
                if (defined (*ENTRY{ARRAY}))
                    {
                    print LOG "[$$]CUP:  \@$key\n" ;
                    undef @{$glob} ;
                    }
                }
            }
        else
            {
            while (($key,$val) = each(%{*{"$package\::"}}))
                {
	        local(*ENTRY) = $val;
                $glob = $package.'::'.$key ;
                undef ${$glob} if (defined (*ENTRY{SCALAR})) ;
                undef %{$glob} if (defined (*ENTRY{HASH}) && !($key =~ /\:\:$/)) ;
                undef @{$glob} if (defined (*ENTRY{ARRAY})) ;
                }
            }
        }

    @cleanups = () ;

    embperl_flushlog () ;

    #log_svs ("cleanup exit") ;
    return &OK ;
    }

#######################################################################################

sub watch 
    {
    my $glob ;
    my $key  ;
    my $val  ;

    while (($key,$val) = each(%{*{"$package\::"}})) {
	    local(*ENTRY) = $val;
        $glob = $package.'::'.$key ;
        if (defined (*ENTRY{SCALAR}) && ${$glob} ne $watchval{$key}) 
            {
            print LOG "[$$]VAR:  \$$key = ${$glob}\n" ;
            $watchval{$key} = ${$glob} ;
            } 
        }
    }


#######################################################################################

sub MailFormTo

    {
    my ($to, $subject, $returnfield) = @_ ;
    my $v ;
    my $k ;
    my $ok ;
    my $smtp ;
    my $ret ;

    $ret = $fdat{$returnfield} ;

    eval 'require Net::SMTP' ;
    die "require Net::SMTP failed: $@" if ($@); 

    $smtp = Net::SMTP->new($ENV{EMBPERL_MAILHOST} || 'localhost') or die "Cannot connect to Mailhost" ;
    if ($ret)
        { $smtp->mail($ret); }
    else
        { $smtp->mail('WWW-Server');}
    $smtp->to($to);
    $ok = $smtp->data();
    $ok = $smtp->datasend("Return-Path: $ret\n") if ($ok && $ret) ;
    $ok and $ok = $smtp->datasend("To: $to\n");
    $ok and $ok = $smtp->datasend("Subject: $subject\n");
    $ok and $ok = $smtp->datasend("\n");
    foreach $k (@ffld)
        { 
        $v = $fdat{$k} ;
        if (defined ($v) && $v ne '')
            {
            $ok and $ok = $smtp->datasend("$k\t= $v \n" );
            }
        }
    $ok and $ok = $smtp->datasend("\nClient\t= $ENV{REMOTE_HOST} ($ENV{REMOTE_ADDR})\n\n" );
    $ok and $ok = $smtp->dataend() ;
    $smtp->quit; 

    return $ok ;
    }    


#######################################################################################


sub ProxyInput

    {
    my ($r, $in, $mtime, $src, $dest) = @_ ;



    if (defined ($src))
        {
        $url = $dest . $1 if ($r -> uri =~ m{^$src(.*?)$}) ;
        }
    else
        {
        return &NOT_FOUND ;
        }

    my $q = $r -> args ;
    $url .= "?$q" if ($q) ;
    
    my ($request, $response, $ua);

    eval 'require LWP::UserAgent' ;
    die "require LWP::UserAgent failed: $@" if ($@); 

    $ua = new LWP::UserAgent;  
    $ua -> use_eval (0) ;
    $request  = new HTTP::Request($r -> method, $url);

    my %headers_in = $r->headers_in;
    while (($key,$val) = each %headers_in)
        {
 	$request->header($key,$val) if (lc ($key) ne 'connection') ;
        }

    $response = $ua->request($request);

    my $code = $response -> code ;
    my $mod  = $response -> last_modified || '???' ;

    if ($Debugflags) 
        { 
        print LOG "[$$]PXY: uri=" . $r->uri . "\n" ;
        print LOG "[$$]PXY: src=$src, dest=$dest\n" ;
        print LOG "[$$]PXY: -> url=$url\n" ;
        print LOG "[$$]PXY: code=$code,  last modified = $mod\n" ;
        print LOG "[$$]PXY: msg =". $response -> message . "\n" ;
        }
            
    $$in    = $response -> content ;
    $$mtime = $mod if ($mod ne '???') ;

    return $code == 200?0:$code;
    }


#######################################################################################


sub LogOutput

    {
    my ($r, $out, $basepath) = @_ ;

    #$basepath =~ s*[^a-zA-Z0-9./-]*-* ;
    $basepath =~ /^(.*?)$/ ;

    $basepath = $1 ;

    $LogOutputFileno++ ;

    $r -> send_http_header ;

    $r -> print ($$out) ;
    
    open L, ">$basepath.$$.$LogOutputFileno" ;
    print L $$out ;
    close L ;

    if ($Debugflags) 
        { 
        print LOG "[$$]OUT:  Logged output to $basepath.$$.$LogOutputFileno\n" ;
        }

    return 0 ;
    }


###############################################################################    
#
# This package is only here that HTML::Embperl also shows up under module/by-module/Apache/ .
#

package Apache::Embperl; 

*handler = \&HTML::Embperl::handler ;

#
#
###############################################################################    
            

1;


# for documentation see Embperl.pod
