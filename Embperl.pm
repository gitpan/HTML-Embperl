
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

use strict ;
use vars qw(
    $DefaultLog 
    $DebugDefault
    
    %cache
    %mtime
    %filepack
    $packno

    @cleanups
    
    $LogOutputFileno
    %LogFileColors
    %NameSpace
    @ISA
    $VERSION
    
    %watchval

    $cwd
    
    $escmode
    %fdat
    %udat
    %mdat
    @ffld

    $evalpackage

    $optRedirectStdout
    $optDisableFormData
    $optDisableVarCleanup
    $optAllowZeroFilesize

    $dbgShowCleanup
    $dbgLogLink

    $escmode

    $SessionMgnt
    ) ;


@ISA = qw(Exporter DynaLoader);


$VERSION = '1.2b3';


bootstrap HTML::Embperl $VERSION;



# Default logfilename

$DefaultLog = '/tmp/embperl.log' ;

%cache    = () ;    # cache for evaled code
%filepack = () ;    # translate filename to packagename
$packno   = 1 ;     # for assigning unique packagenames

@cleanups = () ;    # packages which need a cleanup
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
use constant optNoHiddenEmptyValue      => 0x10000 ;
use constant optAllowZeroFilesize       => 0x20000 ;


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
use constant rcCallInputFuncFailed => 40 ;
use constant rcCallOutputFuncFailed => 41 ;


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
        HTML::Embperl::log(join ('', @_)) ;
        }

    sub PRINTF

        {
        shift ;
        my $fmt = shift ;
        HTML::Embperl::log(sprintf ($fmt, @_)) ;
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
        HTML::Embperl::output(join ('', @_)) ;
        }

    sub PRINTF

        {
        shift ;
        my $fmt = shift ;
        HTML::Embperl::output(sprintf ($fmt, @_)) ;
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
    #eval 'use Apache::Constants qw(:common &OPT_EXECCGI)' ;
    eval 'use Apache::Constants qw(&OPT_EXECCGI &DECLINED &OK &FORBIDDEN)' ;
    die "use Apache::Constants failed: $@" if ($@); 
    XS_Init (epIOMod_Perl, $DefaultLog, $DebugDefault) ;
#    eval 'sub OK        { 0 ;   }' if (!defined (&OK)) ;
    eval 'sub NOT_FOUND { 404 ; }' if (!defined (&NOT_FOUND)) ;
#    eval 'sub FORBIDDEN { 401 ; }' if (!defined (&FORBIDDEN)) ;

    }
else
    {
    eval 'sub OK        { 0 ;   }' ;
    eval 'sub NOT_FOUND { 404 ; }' ;
    eval 'sub FORBIDDEN { 401 ; }' ;
no strict ;
    XS_Init (epIOPerl, $DefaultLog, $DebugDefault) ;
use strict ;
    }

$cwd       = Cwd::fastcwd();

tie *LOG, 'HTML::Embperl::Log' ;
tie *OUT, 'HTML::Embperl::Out' ;

#use Apache::Session ;
#use Apache::Session::Win32 ;

$SessionMgnt = 0 ;
if (exists $INC{'Apache/Session.pm'})
    {
    $SessionMgnt = 1 ;
    tie %udat, 'Apache::Session', $ENV{EMBPERL_SESSION_CLASS} || 'Win32',
                  undef, {not_lazy=>0, autocommit=>0, lifetime=>&Apache::Session::LIFETIME} ;
    tie %mdat, 'Apache::Session', $ENV{EMBPERL_SESSION_CLASS} || 'Win32', 
                  undef, {not_lazy=>0, autocommit=>0, lifetime=>&Apache::Session::LIFETIME} ;
    warn "[$$]SES:  Embperl Session management enabled\n" ;
    }

#######################################################################################

#no strict ;

sub _eval_ ($)
    {
    my $result = eval "package $evalpackage ; $_[0] " ;
    if ($@ ne '')
        { logevalerr ($@) ; }
    return $result ;
    }

#use strict ;

#######################################################################################

sub _evalsub_ ($)
    {
    my $result = eval "package $evalpackage ; sub { $_[0] } " ;
    if ($@ ne '')
        { logevalerr ($@) ; }
    return $result ;
    }

#######################################################################################

sub Warn 
    {
    local $^W = 0 ;
    my $msg = $_[0] ;
    chop ($msg) ;
    
    my $lineno = getlineno () ;
    my $Inputfile = Sourcefile () ;
    if ($msg =~ /HTML\/Embperl/)
        {
        $msg =~ s/at (.*?) line (\d*)/at $Inputfile in block starting at line $lineno/ ;
        }
    logerror (rcPerlWarn, $msg);
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

sub SendLogFile ($$$)

    {
    my ($fn, $info, $req_rec) = @_ ;
    
    my $lastpid = 0 ;
    my ($filepos, $pid, $src) = split (/&/, $info) ;
    my $cnt = 0 ;
    my $ecnt = 0 ;
    my $tag ;
    my $fontcol ;

    if (defined ($req_rec))
        {
        $req_rec -> content_type ('text/html') ;
        $req_rec -> send_http_header ;
        }
        
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
            #s/\n/\\<BR\\>\r\n/ ;
            s/&/&amp;/g;
            s/\"/&quot;/g;
            s/>/&gt;/g;
            s/</&lt;/g;
            s/\n/\<BR\>\r\n/ ;
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

            
            print $_ ;
            print '</A>' ;
            last if (/Request finished/) ;
            }
        }

    print "</CODE></BODY></HTML>\r\n\r\n" ;

    close (LOGFILE) ;

    return 200 ;
    }



##########################################################################################

sub CheckFile

    {
    my ($filename, $req_rec, $AllowZeroFilesize) = @_ ;


    unless (-r $filename && (-s _ || $AllowZeroFilesize))
        {
	logerror (rcNotFound, $filename);
	return &NOT_FOUND ;
        }

    if (defined ($req_rec) && !($req_rec->allow_options & &OPT_EXECCGI))
        {
	logerror (rcExecCGIMissing, $filename);
	return &FORBIDDEN ;
 	}
	
    if (-d _)
        {
	logerror (rcIsDir, $filename);
	return &FORBIDDEN ;
	}                 
    
    return ok ;
    }

##########################################################################################

sub ScanEnvironement

    {
    my ($req, $req_rec) = @_ ; 

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
    
    if (!ref ($req)) 
        {    
        my @parameter = @_ ;
        $req = { inputfile => $req, param => \@parameter }
        } 
    
    my $req_rec ;
    if (defined ($$req{req_rec})) 
        { $req_rec = $$req{'req_rec'} }
    elsif (exists $INC{'Apache.pm'})
        { $req_rec = Apache->request }


    if (defined ($$req{'virtlog'}) && $$req{'virtlog'} eq $$req{'uri'})
        {
        return SendLogFile ($DefaultLog, $ENV{QUERY_STRING}, $$req{'req_rec'}) ;
        }

    my $ns ;
    my $opcodemask ;

    if (defined ($ns = $$req{'compartment'}))
        {
        my $cp = AddCompartment ($ns) ;
        $opcodemask = $cp -> mask ;
        }

    my $conf = SetupConfData ($req, $opcodemask) ;

    
    my $Outputfile = $$req{'outputfile'} ;
    my $In         = $$req{'input'}   ;
    my $Out        = $$req{'output'}  ;
    my $filesize ;
    my $mtime ;
    my $OutData ;
    my $InData ;

    if (exists $$req{'input_func'})  
        {
        my @p ;
        $In = \$InData ;
        $$req{mtime} = 0 ;
        @p = split (/\s*\,\s*/, $$req{'input_func'}) ;
        my $InFunc = shift @p ;
no strict ;
        eval {$rc = &{$InFunc} ($req_rec, $In, \$$req{mtime}, @p)} ;
use strict ;
        if ($rc || $@)
            {
            if ($@) 
                {
                $rc = 500 ;
                logerror (rcCallInputFuncFailed, $@) ;
                }

            return $rc ;
            }
        }

    $Out = \$OutData if (exists $$req{'output_func'}) ;

    my $Inputfile    = $$req{'inputfile'} || '<none>' ;
    
    if (defined ($In))
        {
        $filesize = -1 ;
        $mtime    = $$req{'mtime'} || 0 ;
        }
   else
        {
        if ($rc = CheckFile ($Inputfile, $req_rec, (($$req{options} || 0) & optAllowZeroFilesize))) 
            {
            return $rc ;
            }
        $filesize = -s _ ;
        $mtime = -M _ ;
        }

    my $r = SetupRequest ($req_rec, $Inputfile, $mtime, $filesize, $Outputfile, $conf, &epIOMod_Perl, $In, $Out) ;
    
    my $package = $r -> CurrPackage ;
    $evalpackage = $package ;   
    
    $r -> CreateAliases () ;
   

    if (!($optDisableFormData) &&
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
        @ffld = @{$$req{'ffld'}} if (defined ($$req{'ffld'})) ;
        %fdat = %{$$req{'fdat'}} if (defined ($$req{'fdat'})) ;
        }

no strict ;
    # pass parameters via @param
    *{"$package\:\:param"}   = $$req{'param'} if (exists $$req{'param'}) ;
use strict ;
    
    my $udat ;
    my $mdat ;

    if ($SessionMgnt)
        {
        $udat = tied(%udat) ;

        if (defined ($ENV{HTTP_COOKIE}) && ($ENV{HTTP_COOKIE} =~ /EMBPERL_UID=(.*?)(\s|$)/))
	    {
	    $udat -> {ID} = $1 ;
            print LOG "[$$]SES:  Received session cookie $1\n" ;
	    }
        else
	    {
	    $udat -> {ID} = undef ;
	    }
    
        $udat -> {DIRTY} = 0 ;

        $mdat = tied(%mdat) ;
	$mdat -> {ID} = MD5 -> hexhash ($Inputfile) ;
        $mdat -> {DIRTY} = 0 ;
        }

        {
        local $SIG{__WARN__} = \&Warn ;
        local *0 = \$Inputfile;
        my $oldfh = select (OUT) if ($optRedirectStdout) ;
        my $saver = $r ;
        
        $rc = $r -> ExecuteReq ($$req{'param'}) ;
        
        $r = $saver ;
        select ($oldfh) if ($optRedirectStdout) ;
        
        if (exists $$req{'output_func'}) 
            {
            my @p ;
            my $OutFunc ;
            ($OutFunc, @p) = split (/\s*,\s*/, $$req{'output_func'}) ;
no strict ;
            eval { &$OutFunc ($req_rec, $Out,@p) } ;
use strict ;
            $r -> logerror (rcCallOutputFuncFailed, $@) if ($@) ;
            }
        }


    no strict ;
    undef *{"$package\:\:param"} ;
    use strict ;

    
    if ($SessionMgnt)
        {
        if ($udat->{'DIRTY'})
	    {
            print LOG "[$$]SES:  Store session data of \%udat id=$udat->{ID}\n" ;
            $udat->{'DATA'}->store ;
	    }

        $udat->{'DATA'} = undef ;
        $udat -> {ID}   = undef ;
        if ($mdat->{'DIRTY'})
	    {
            print LOG "[$$]SES:  Store session data of \%mdat id=$mdat->{ID}\n" ;
            $mdat->{'DATA'}->store ;
	    }

        $mdat->{'DATA'} = undef ;
        $mdat -> {ID}   = undef ;
        }

    my $cleanup    = $$req{'cleanup'}    || ($optDisableVarCleanup?-1:0) ;

    if ($cleanup == -1)
        { ; } 
    elsif ($cleanup == 0)
        {
        if ($#cleanups == -1) 
            {
            push @cleanups, 'dbgShowCleanup' if ($dbgShowCleanup) ;
            $req_rec -> register_cleanup(\&HTML::Embperl::cleanup) if (defined ($req_rec)) ;
            }
        push @cleanups, $package ;
        
        cleanup () if (!$r -> SubReq () && !$req_rec) ;
        }
    else
        {
        push @cleanups, 'dbgShowCleanup' if ($dbgShowCleanup) ;
        push @cleanups, $package ;
        cleanup () ;
        }

    $r -> FreeRequest () ;
    
    return 0 ;
    }

#######################################################################################


sub Init

    {
    my $Logfile   = shift ;
    $DebugDefault = shift ;
    $DebugDefault = dbgStd if (!defined ($DebugDefault)) ;
        
    XS_Init (epIOPerl, $Logfile || $DefaultLog, $DebugDefault) ;
    
    tie *LOG, 'HTML::Embperl::Log' ;
    }

#######################################################################################


sub Term

    {
    cleanup () ;
    XS_Term () ;
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

    ScanEnvironement (\%req) ;
    

    if (defined ($$args[0]) && $$args[0] eq 'dbgbreak') 
    	{
    	shift @$args ;
    	dbgbreak () ;
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


    XS_Init ($ioType, $Logfile, $DebugDefault) ;

    
    tie *LOG, 'HTML::Embperl::Log' ;

    $req{'uri'} = $ENV{SCRIPT_NAME} ;

    $req{'cleanup'} = 0 ;
    $req{'cleanup'} = -1 if (($req{'options'} & optDisableVarCleanup)) ;
    $req{'options'} |= optSendHttpHeader ;

    $rc = Execute (\%req) ;

    #close LOG ;
    XS_Term () ;

    return $rc ;
    }


#######################################################################################



sub handler 
    
    {
    #log_svs ("handler entry") ;

    my $req_rec = shift ;

    my %req ;

    ScanEnvironement (\%req, $req_rec) ;
    
    $req{'uri'}       = $req_rec -> Apache::uri ;

    if (exists $ENV{EMBPERL_FILESMATCH} && 
                         !($req{'uri'} =~ m{$ENV{EMBPERL_FILESMATCH}})) 
        {
        # Reset the perl-handler to work with older mod_perl versions
        ResetHandler ($req_rec) ;
        return &DECLINED ;
        }


    $req{'inputfile'} = $ENV{PATH_TRANSLATED} = $req_rec -> filename ;

    #print LOG "i = $req{'inputfile'}\n" ;

    $req{'cleanup'} = -1 if (($req{'options'} & optDisableVarCleanup)) ;
    $req{'options'} |= optSendHttpHeader ;
    $req{'req_rec'} = $req_rec ;

    my $rc = Execute (\%req) ;

    #log_svs ("handler exit") ;
    return $rc ;
    }

#######################################################################################

no strict ;


sub cleanup 
    {
    #log_svs ("cleanup entry") ;
    my $glob ;
    my $val ;
    my $key ;
    local $^W = 0 ;
    my $package ;
    my %seen ;
    my $Debugflags ;


    $seen{''}      = 1 ;
    $seen{'dbgShowCleanup'} = 1 ;
    foreach $package (@cleanups)
        {
        $Debugflags = dbgShowCleanup if ($package eq 'dbgShowCleanup') ;
        next if ($seen{$package}) ;

	$seen{$package} = 1 ;
        
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
		if ($key ne 'udat')
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
            }
        else
            {
            while (($key,$val) = each(%{*{"$package\::"}}))
                {
		if ($key ne 'udat')
		    {
		    local(*ENTRY) = $val;
		    $glob = $package.'::'.$key ;
		    undef ${$glob} if (defined (*ENTRY{SCALAR})) ;
		    undef %{$glob} if (defined (*ENTRY{HASH}) && !($key =~ /\:\:$/)) ;
		    undef @{$glob} if (defined (*ENTRY{ARRAY})) ;
		    }
		}
            }
        }

    @cleanups = () ;

    flushlog () ;

    #log_svs ("cleanup exit") ;
    return &OK ;
    }

use strict ;

#######################################################################################

sub watch 
    {
    my ($package) = @_ ;

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

    $smtp = Net::SMTP->new($ENV{'EMBPERL_MAILHOST'} || 'localhost') or die "Cannot connect to mailhost" ;
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

    my $url ;

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
    my $key ;
    my $val ;
    while (($key,$val) = each %headers_in)
        {
 	$request->header($key,$val) if (lc ($key) ne 'connection') ;
        }

    $response = $ua->request($request);

    my $code = $response -> code ;
    my $mod  = $response -> last_modified || '???' ;

    #if ($Debugflags) 
    #    { 
    #    print LOG "[$$]PXY: uri=" . $r->uri . "\n" ;
    #    print LOG "[$$]PXY: src=$src, dest=$dest\n" ;
    #    print LOG "[$$]PXY: -> url=$url\n" ;
    #    print LOG "[$$]PXY: code=$code,  last modified = $mod\n" ;
    #    print LOG "[$$]PXY: msg =". $response -> message . "\n" ;
    #    }
            
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

    #if ($Debugflags) 
    #    { 
    #    print LOG "[$$]OUT:  Logged output to $basepath.$$.$LogOutputFileno\n" ;
    #    }

    return 0 ;
    }

#######################################################################################

package HTML::Embperl::Req ; 

#######################################################################################

use strict ;

#######################################################################################

sub CreateAliases

    {
    my ($self) = @_ ;
    
    my $package = $self -> CurrPackage ;
    
    no strict ;

    if (!defined(${"$package\:\:row"}))
        { # create new aliases for Embperl magic vars

        *{"$package\:\:fdat"}    = \%HTML::Embperl::fdat ;
        *{"$package\:\:udat"}    = \%HTML::Embperl::udat ;
        *{"$package\:\:mdat"}    = \%HTML::Embperl::mdat ;
        *{"$package\:\:fsplitdat"}    = \%HTML::Embperl::fsplitdat ;
        *{"$package\:\:ffld"}    = \@HTML::Embperl::ffld ;
        *{"$package\:\:idat"}    = \%HTML::Embperl::idat ;
        *{"$package\:\:cnt"}     = \$HTML::Embperl::cnt ;
        *{"$package\:\:row"}     = \$HTML::Embperl::row ;
        *{"$package\:\:col"}     = \$HTML::Embperl::col ;
        *{"$package\:\:maxrow"}  = \$HTML::Embperl::maxrow ;
        *{"$package\:\:maxcol"}  = \$HTML::Embperl::maxcol ;
        *{"$package\:\:tabmode"} = \$HTML::Embperl::tabmode ;
        *{"$package\:\:escmode"} = \$HTML::Embperl::escmode ;
    	*{"$package\:\:req_rec"} = \$HTML::Embperl::req_rec if defined ($req_rec) ;
    	*{"$package\:\:exit"}    = \&Apache::exit if defined (&Apache::exit) ;
        
        *{"$package\:\:MailFormTo"} = \&HTML::Embperl::MailFormTo ;
        *{"$package\:\:Execute"} = \&HTML::Embperl::Execute ;

        tie *{"$package\:\:LOG"}, 'HTML::Embperl::Log' ;
        tie *{"$package\:\:OUT"}, 'HTML::Embperl::Out' ;

        *{"$package\:\:optDisableChdir"}                = \$HTML::Embperl::optDisableChdir                   ;
        *{"$package\:\:optDisableEmbperlErrorPage"}     = \$HTML::Embperl::optDisableEmbperlErrorPage ;
        *{"$package\:\:optDisableFormData"}             = \$HTML::Embperl::optDisableFormData         ;
        *{"$package\:\:optDisableHtmlScan"}             = \$HTML::Embperl::optDisableHtmlScan         ;
        *{"$package\:\:optDisableInputScan"}            = \$HTML::Embperl::optDisableInputScan        ;
        *{"$package\:\:optDisableMetaScan"}             = \$HTML::Embperl::optDisableMetaScan         ;
        *{"$package\:\:optDisableTableScan"}            = \$HTML::Embperl::optDisableTableScan        ;
        *{"$package\:\:optDisableVarCleanup"}           = \$HTML::Embperl::optDisableVarCleanup       ;
        *{"$package\:\:optEarlyHttpHeader"}             = \$HTML::Embperl::optEarlyHttpHeader         ;
        *{"$package\:\:optOpcodeMask"}                  = \$HTML::Embperl::optOpcodeMask              ;
        *{"$package\:\:optRawInput"}                    = \$HTML::Embperl::optRawInput                ;
        *{"$package\:\:optSafeNamespace"}               = \$HTML::Embperl::optSafeNamespace           ;
        *{"$package\:\:optSendHttpHeader"}              = \$HTML::Embperl::optSendHttpHeader          ;
        *{"$package\:\:optAllFormData"}                 = \$HTML::Embperl::optAllFormData ;
        *{"$package\:\:optRedirectStdout"}              = \$HTML::Embperl::optRedirectStdout ;
        *{"$package\:\:optUndefToEmptyValue"}           = \$HTML::Embperl::optUndefToEmptyValue ;
        *{"$package\:\:optNoHiddenEmptyValue"}          = \$HTML::Embperl::optNoHiddenEmptyValue ;
        *{"$package\:\:optAllowZeroFilesize"}           = \$HTML::Embperl::optAllowZeroFilesize ;
 

        *{"$package\:\:dbgAllCmds"}               = \$HTML::Embperl::dbgAllCmds           ;
        *{"$package\:\:dbgCacheDisable"}          = \$HTML::Embperl::dbgCacheDisable      ;
        *{"$package\:\:dbgCmd"}                   = \$HTML::Embperl::dbgCmd               ;
        *{"$package\:\:dbgDefEval"}               = \$HTML::Embperl::dbgDefEval           ;
        *{"$package\:\:dbgEarlyHttpHeader"}       = \$HTML::Embperl::dbgEarlyHttpHeader   ;
        *{"$package\:\:dbgEnv"}                   = \$HTML::Embperl::dbgEnv               ;
        *{"$package\:\:dbgEval"}                  = \$HTML::Embperl::dbgEval              ;
        *{"$package\:\:dbgFlushLog"}              = \$HTML::Embperl::dbgFlushLog          ;
        *{"$package\:\:dbgFlushOutput"}           = \$HTML::Embperl::dbgFlushOutput       ;
        *{"$package\:\:dbgForm"}                  = \$HTML::Embperl::dbgForm              ;
        *{"$package\:\:dbgFunc"}                  = \$HTML::Embperl::dbgFunc              ;
        *{"$package\:\:dbgHeadersIn"}             = \$HTML::Embperl::dbgHeadersIn         ;
        *{"$package\:\:dbgInput"}                 = \$HTML::Embperl::dbgInput             ;
        *{"$package\:\:dbgLogLink"}               = \$HTML::Embperl::dbgLogLink           ;
        *{"$package\:\:dbgMem"}                   = \$HTML::Embperl::dbgMem               ;
        *{"$package\:\:dbgShowCleanup"}           = \$HTML::Embperl::dbgShowCleanup       ;
        *{"$package\:\:dbgSource"}                = \$HTML::Embperl::dbgSource            ;
        *{"$package\:\:dbgStd"}                   = \$HTML::Embperl::dbgStd               ;
        *{"$package\:\:dbgTab"}                   = \$HTML::Embperl::dbgTab               ;
        *{"$package\:\:dbgWatchScalar"}           = \$HTML::Embperl::dbgWatchScalar       ;

        #print LOG  "[$$]MEM:  Created Aliases for $package\n" ;

        }

    use strict ;
    }

#######################################################################################

sub SendErrorDoc ()

    {
    my ($self) = @_ ;
    local $SIG{__WARN__} = 'Default' ;
    
    my $virtlog = $self -> VirtLogURI || '' ;
    my $logfilepos = $self -> LogFileStartPos () ;
    my $url     = $HTML::Embperl::dbgLogLink?"<A HREF=\"$virtlog\?$logfilepos\&$$\">Logfile</A>":'' ;    
    my $req_rec = $self -> ApacheReq ;
    my $err ;
    my $cnt = 0 ;
    local $HTML::Embperl::escmode = 0 ;
    my $time = localtime ;
    my $mail = $req_rec -> server -> server_admin if (defined ($req_rec)) ;
    $mail ||= '' ;

    $req_rec -> content_type('text/html') if (defined ($req_rec)) ;

    $self -> output ("<HTML><HEAD><TITLE>Embperl Error</TITLE></HEAD><BODY bgcolor=\"#FFFFFF\">\r\n$url") ;
    $self -> output ("<H1>Internal Server Error</H1>\r\n") ;
    $self -> output ("The server encountered an internal error or misconfiguration and was unable to complete your request.<P>\r\n") ;
    $self -> output ("Please contact the server administrator, $mail and inform them of the time the error occurred, and anything you might have done that may have caused the error.<P><P>\r\n") ;

    my $errors = $self -> ErrArray() ;
    if ($virtlog ne '' && $HTML::Embperl::dbgLogLink)
        {
        foreach $err (@$errors)
            {
            $self -> output ("<A HREF=\"$virtlog?$logfilepos&$$#E$cnt\">") ; #<tt>") ;
            $HTML::Embperl::escmode = 3 ;
            $err =~ s|\n|\n\\<br\\>\\&nbsp;\\&nbsp;\\&nbsp;\\&nbsp;|g;
            $err =~ s|(Line [0-9]*:)|$1\\</a\\>|;
            $self -> output ($err) ;
            $HTML::Embperl::escmode = 0 ;
            $self -> output ("<p>\r\n") ;
            #$self -> output ("</tt><p>\r\n") ;
            $cnt++ ;
            }
        }
    else
        {
        $HTML::Embperl::escmode = 3 ;
        foreach $err (@$errors)
            {
            $err =~ s|\n|\n\\<br\\>\\&nbsp;\\&nbsp;\\&nbsp;\\&nbsp;|g;
            $self -> output ("$err\\<p\\>\r\n") ;
            #$self -> output ("\\<tt\\>$err\\</tt\\>\\<p\\>\r\n") ;
            $cnt++ ;
            }
        $HTML::Embperl::escmode = 0 ;
        }
         
    my $server = $ENV{SERVER_SOFTWARE} || 'Offline' ;

    $self -> output ("$server HTML::Embperl $HTML::Embperl::VERSION [$time]<P>\r\n") ;
    $self -> output ("</BODY></HTML>\r\n\r\n") ;

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
