
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



use Safe;
use IO::Handle ;
use CGI;
use File::Basename ();
use Cwd ();
#eval ' require Apache::Symbol ; ' ;

require Exporter;
require DynaLoader;

@ISA = qw(Exporter DynaLoader);


$VERSION = '1.0.0';


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
    eval 'use Apache::Constants qw(:common &OPT_EXECCGI)' ;
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


    embperl_output ("<HTML><HEAD><TITLE>Embperl Error</TITLE></HEAD><BODY bgcolor=\"#FFFFFF\">\r\n$url") ;
    embperl_output ("<H1>Internal Server Error</H1>\r\n") ;
    embperl_output ("The server encountered an internal error or misconfiguration and was unable to complete your request.<P>\r\n") ;
    embperl_output ("Please contact the server administrator, $mail and inform them of the time the error occurred, and anything you might have done that may have caused the error.<P><P>\r\n") ;

    if (defined ($LogfileURL) && $virtlog ne '')
        {
        foreach $err (@errors)
            {
            embperl_output ("<A HREF=\"$virtlog?$logfilepos&$$#E$cnt\">") ;
            $escmode = 3 ;
            embperl_output ($err) ;
            $escmode = 0 ;
            embperl_output ("</a><P>\r\n") ;
            $cnt++ ;
            }
        }
    else
        {
	$escmode = 3 ;
        foreach $err (@errors)
            {
            embperl_output ("$err\\<P\\>\r\n") ;
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
    my $cgi ;
    my $filesize ;
    my $mtime ;
    
    
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
	$cgi = new CGI;
	    
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
    $DebugDefault = shift || dbgStd ;
        
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

    eval 'use Net::SMTP' ;

    $smtp = Net::SMTP->new('localhost');
    $smtp->mail('WWW-Server');
    $smtp->to($to);
    $ok = $smtp->data();
    $ok and $ok = $smtp->datasend("Return-Path: $ret\n") if ($ret) ;
    $ok and $ok = $smtp->datasend("To: $to\n");
    $ok and $ok = $smtp->datasend("Subject: $subject\n");
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
__END__


=head1 NAME

HTML::Embperl - Perl extension for embedding Perl code in HTML documents


=head1 SYNOPSIS

Embperl is a Perl extension module which gives you the ability to
embed Perl code in HTML documents, like server-side includes for shell
commands.


=head1 DESCRIPTION

Embperl can operate in one of four modes:

=over 4

=item B<Offline>

Converts an HTML file with embedded Perl statements into a standard
HTML file.

B<embpexec.pl [-o outputfile] [-l logfile] [-d debugflags] htmlfile
[query_string]>

B<embpexec.bat [-o outputfile] [-l logfile] [-d debugflags] htmlfile
[query_string]>


Use embpexec.pl on unix systems and embpexec.bat on win32 systems.

=over 4

=item B<htmlfile>

The full pathname of the HTML file which should be processed by
Embperl.

=item B<query_string>

Optional.  Has the same meaning as the environment variable
QUERY_STRING when invoked as a CGI script.  That is, QUERY_STRING
contains everything following the first "?" in a URL.  <query_string>
should be URL-encoded.  The default is no query string.

=item B<-o outputfile>

Optional.  Gives the filename to which the output is written.  The
default is stdout.

=item B<-l logfile>

Optional.  Gives the filename of the logfile.  The default is
/tmp/embperl.log.

=item B<-d debugflags>

Optional.  Specifies the level of debugging (what is written to the
log file).  The default is nothing.  See below for exact values.

=back


=item B<As a CGI script>

Instead of a file being sent directly by the web server, the document
is processed by the CGI script and the result is sent to the client.

B<embpexec.pl>

B<embpexec.bat>

Use embpexec.pl on unix systems and embpexec.bat on win32 systems.

If C<embpexec.pl/embpexec.bat> is invoked without any parameters and the
environment variable PATH_TRANSLATED is set, it runs itself as a CGI
script.  This means that form data is taken either from the
environment variable QUERY_STRING or from stdin, depending on whether
or not CONTENT_LENGTH is set.  (This will be set by the web server
depending on whether the request method is GET or POST).  Input is
taken from the file pointed to by PATH_TRANSLATED and the output is
send to stdout.  The logfile is generated at its default location,
which is configurable via the environment variable EMBPERL_LOG.

To use this mode you must copy B<embpexec.pl> to your cgi-bin
directory.  You can invoke it with the URL
http://www.domain.xyz/cgi-bin/embpexec.pl/url/of/your/document.

The /url/of/your/document will be passed to Embperl by the web server.
Normal processing (aliasing, etc.) takes place before the URI makes it
to PATH_TRANSLATED.

If you are running the Apache httpd, you can also define
B<embpexec.pl> as a handler for a specific file extention or
directory.

Example of Apache C<srm.conf>:

    <Directory /path/to/your/html/docs>
    Action text/html /cgi-bin/embperl/embpexec.pl
    </Directory>


=item B<From mod_perl> (Apache httpd)

This works like the CGI-Script, but with the advantage that the script
is compiled only once at server startup, where other one-time actions
(such as opening files and databases) can take place.  This will
drastically reduce response times for the request.  To use this you
must compile C<Apache httpd> with C<mod_perl> and add C<HTML::Embperl>
as the C<PerlHandler>.

Example of Apache C<srm.conf>:

    SetEnv EMBPERL_DEBUG 2285

    Alias /embperl /path/to/embperl/eg

    <Location /embperl/x>
    SetHandler  perl-script
    PerlHandler HTML::Embperl
    Options     ExecCGI
    </Location>

Another possible setup (for Apache 1.3bX see below) is

    SetEnv EMBPERL_DEBUG 2285

    <Files *.epl>
    SetHandler  perl-script
    PerlHandler HTML::Embperl
    Options     ExecCGI
    </files>

    AddType text/html .epl

Don't forget the B<AddType>.  In this setup, all files ending with
.epl are processed by Embperl.

C<NOTE:> Since <Files> does not work the same in Apache 1.3bX as it
does in Apache 1.2.x, you need to use <FilesMatch> instead.

    <FilesMatch ".*\.epl$">
    SetHandler  perl-script
    PerlHandler HTML::Embperl
    Options     ExecCGI
    </FilesMatch>


See the section on debugging (dbgLogLink and EMBPERL_VIRTLOG) to find
out how you can configure Embperl so you can view the log file with
your browser!


=item B<By calling HTML::Embperl::Execute (\%param)>

Execute takes a hash reference as argument. This gives the chance to
vary the parameters according to the job that should be done.

(See B<eg/x/Excute.pl> for more detailed examples)

Possible items are:

=over 4

=item B<inputfile>

File which should be used as source. If B<input> is also specified
this parameter should give a unique name to identify the source.
Everytime Embperl sees the same text in B<inputfile> it assumes that
you compile the same source, that means Embperl uses the same package
name as in your last call and only recompiles the code if B<mtime> has
changed or is undefined.

=item B<input>

Reference to a string which contains the source. B<inputfile> must also
be specified to give a name for the source. The name can be any text.

=item B<mtime>

Last modification time of member B<input>. If undef the code passed
by input is always recompiled, else the code is only recompiled if
mtime changes.


=item B<outputfile>

File to which the output should be written. If neither outputfile
nor output is specified ouput is written to stdout.

=item B<output>

Reference to a scalar where the output should be written to.

=item B<cleanup>

This value specifies if and when the cleanup of the package should be
executed. (See variables scopes below for more information on cleanup)

=over 4

=item B<cleanup = -1>

Never cleanup the variables

=item B<cleanup = 0> or not specified

If running under mod_perl cleanup is delayed until the connection to the
client is closed, so it does not enlarge the response time to the client.
If the Execute function is called more the once before the end of the request
all cleanups take place after the end of the request and not between calls
of Execute.

If running as cgi or offline cleanup takes immediately place.

=item B<cleanup = 1>

Immediate cleanup

=back

=item B<param>

Can be used to pass parameters to the Embperl document and back. Must contain
an reference to an array.


 Example:

    HTML::Embperl::Execute(..., param => [1, 2, 3]) ;
    HTML::Embperl::Execute(..., param => \@parameters) ;

The array @param in the Embperl document is setup as an alias to the array.
See eg/x/Excute.pl for more detailed example.

=item B<ffld and fdat>

Could be used to setup the two Embperl predefined variables.

=item B<options>

Same as EMBPERL_OPTIONS (see below), except for cleanup.

=item B<debug>

Same as EMBPERL_DEBUG (see below).

=item B<escmode>

Same as EMBPERL_ESCMODE (see below).

=item B<package>

Same as EMBPERL_PACKAGE (see below).

=item B<virtlog>

Same as EMBPERL_VIRTLOG (see below). If B<virtlog> is equal to B<uri> the
logfile is send.

=item B<uri>

The URI of the request. Only needed for the virtlog feature.

=item B<compartment>

Same as EMBPERL_COMPARTMENT (see below).


B<NOTE:> You should set the B<optDisableFormData> if you have already
read the form data from stdin, while in a POST request, otherwise
Execute will hang and try to read the data a second time.

=back


=head2 Helper functions for Execute

=over 4

=item B<HTML::Embperl::Init ($Logfile, $DebugDefault)>

This function can be used to setup the logfile path and (optional) 
a default value for the debugflags, which will be used in further calls
to Execute. There will be always only one logfile, but you can use B<Init>
to change it at any time.

NOTE: You do not need to call Init in version >= 0.27. The initialisation
of Embperl takes place automaticly when it is loaded.


=item B<HTML::Embperl::ScanEnvironement (\%params)>

Scans the B<%ENV> and setups B<%params> for use by B<Execute>. All
Embperl runtime configuration options are recognised, except EMBPERL_LOG.

=back


=head2 EXAMPLES for Execute:

 # Get source form /path/to/your.html and
 # write output to /path/to/output'

 HTML::Embperl::Execute ({ inputfile  => '/path/to/your.html',
                           outputfile => '/path/to/output'}) ;


 # Get source from scalar and write output to stdout
 # Don't forget to modify mtime if $src changes

 $src = '<html><head><title>Page [+ $no +]</title></head>' ;

 HTML::Embperl::Execute ({ inputfile  => 'some name',
                           input      => \$src,
                           mtime      => 1 }) ;

 # Get source from scalar and write output another scalar

 my $src = '<html><head><title>Page [+ $no +]</title></head>' ;
 my $out ;

 HTML::Embperl::Execute ({ inputfile  => 'another name',
                           input      => \$src,
                           mtime      => 1,
                           output     => \$out }) ;

 print $out ;






=head1 B<Runtime configuration>

The runtime configuration is done by setting environment variables,
either on the command line (when working offline) or in your web
server's configuration file.  Most HTTP servers understand

SetEnv <var> <value>

If you are using Apache and mod_perl you can use

PerlSetEnv <var> <value>

The advantage of PerlSetEnv over SetEnv is that it can be used on a
per directory/virtual host basis.

=over 4

=item B<EMBPERL_COMPARTMENT>

Gives the name of the compartment from which to take the opcode mask.
(See the chapter about Safe namespaces for more details.)


=item B<EMBPERL_ESCMODE>

Specifies the initial value for $escmode (see below).


=item B<EMBPERL_LOG>

Gives the location of the log file.  This will contain information
about what Embperl is doing.  How much information depends on the
debug settings (see below).  The log output is intended to show what
your embedded Perl code is doing and to help debug it.  Default is
B</tmp/embperl.log>.

NOTE: When running under mod_perl you need to use B<PerlSetEnv> for
setting the logfile path, and mod_perl >= 1.07_03 if you load Embperl
at server startup (with PerlScript or PerlModule).


=item B<EMBPERL_PACKAGE>

The name of the package where your code will be executed.  By default
Embperl generates a unique package name for every file.  This ensures
that variables and functions from one file can not affect those from
another file.  (Any package's variables will still be accessible with
explict package names.)


=item B<EMBPERL_VIRTLOG>

Gives a virtual location where you can access the Embperl logfile with
a browser.  This feature is disabled (default) if EMBPERL_VIRTLOG is
not specified.  See also dbgLogLink for an Example how to set it up in
your srm.conf.


=item B<EMBPERL_OPTIONS>

This bitmask specifies some options for the execution of Embperl:

=over 4

=item optDisableVarCleanup = 1

Disables the automatic cleanup of variables at the end of each
request.

=item optDisableEmbperlErrorPage = 2

Tells Embperl to not send its own errorpage in case of failure,
instead giving the error back to the web server and let the web server
handle it the standard way.  Without this option, Embperl sends its
own error page, showing all the errors which have occured.  If you
have dbgLogLink enabled, every error will be a link to the
corresponding location in the log file.

=item optSafeNamespace = 4

Tells Embperl to execute the embbeded code in a safe namespace so the
code cannot access data or code in any other package.  (See the
chapter about Safe namespaces below for more details.)

=item optOpcodeMask = 8

Tells Embperl to aply an operator mask.  This gives you the chance to
disallow special (unsafe) opcodes.  (See the Chapter about Safe
namespaces below for more details.)

=item optRawInput = 16

Causes Embperl not to preprocess the source for a Perl expression.
(The only exception is that carriage returns will be removed, as Perl
does not like them.)  This option should be set when you writing your
code with an ASCII editor.

If you using a WYSIWYG editor which inserts unwanted HTML tags in your
Perl expressions and escapes special charcaters automatically (e.g.,
`<' appears as `&lt;' in the source), you should not set this option.
Embperl will automaticly convert the HTML input back to the Perl
expressions as you wrote them.

=item optEarlyHttpHeader = 64

Normally, HTTP headers are sent after a request is finished without
error.  This gives you the chance to set arbitrary HTTP headers within
the page, and Embperl the chance to calculate the content length. Also
Embperl watchs out for errors and sends an errorpage instead of the
document if something wents wrong.
To do this, all the output is kept in memory until the whole request is
processed, then the HTTP headers are
sent, and then the document.  This flag will cause the HTTP
headers to be sent before the script is processed, and the script's
output will be sent directly. 

=item optDisableChdir = 128

Without this option Embperl changes the currect directory to the one where
the script resides. This gives you the chance to use relative pathnames. 
Since directory-changeing takes up some millisecs, you can disable it with 
this option if you don't need it.

=item optDisableFormData = 256

This option disables the setup of %fdat and @ffld. Embperl will not do anything
with the posted form data.

=item optDisableHtmlScan = 512

When set, this option disables the scanning of B<all> html-tags. Embperl will only
look for [+/-/!/$ ... $/!/-/+]. This will disable dynamic tables, processing of the
input data and so on.

=item optDisableInputScan = 1024

Disables processing of all input related tags. (<INPUT><TEXTAREA><OPTION>)

=item optDisableTableScan = 2048

Disables processing of all table related tags. (<TABLE><TH><TR><TD><MENU><OL><SELECT><UL>)

=item optDisableMetaScan = 4096

Disables processing of all meta tags. (<META HTTP-EQUIV>)

=item optAllFormData = 8192

This option will cause Embperl to insert all formfields in %fdat and @ffld, even if they
are empty. Empty formfields will be insert with an empty string. Without this option
empty formfields will not be insert in %fdat and @ffld.


=item optRedirectStdout = 16384

Redirects STDOUT to the Embperl output stream before every request and reset it afterwards.
If set, you can use a normal perl B<print> inside any perl block, to output data. 
Without this option you can only output data by using the [+ ... +] block, or printing
to the filehandle B<OUT>.



=back


=item B<EMBPERL_DEBUG>

This is a bitmask which specifies what should be written to the log.
The following values are defined:

=over 4

=item dbgStd = 1

Show minimum information.

=item dbgMem = 2

Show memory and scalar value allocation.

=item dbgEval = 4

Show arguments to and results of evals.

=item dbgCmd = 8

Show metacommands and HTML tags which are processed.

=item dbgEnv = 16,

List every request's environment variables.

=item dbgForm = 32

Lists posted form data.

=item dbgTab = 64

Show processing of dynamic tables.

=item dbgInput = 128

Show processing of HTML input tags.

=item dbgFlushOutput = 256

Flush Embperl's output after every write.  This should only be set to
help debug Embperl crashes, as it drastically slows down Embperl's
operation.

=item dbgFlushLog = 512

Flush Embperl's logfile output after every write.  This should only be
set to help debug Embperl crashes, as it drastically slows down
Embperl's operation.

=item dbgAllCmds  = 1024

Logs all commands and HTML tags, whether or not they are really
excuted or not.  (It logs a `+' or `-' to tell you if they are
executed.)

=item dbgSource = 2048

Logs the next piece of the HTML source to be processed. (NOTE: This
generates a lot of output!)

=item dbgFunc = 4096

This is only anvailable when Embperl is compiled with -DEPDEBUGALL,
and it normally only used for debugging Embperl itself.  Records all
function entries to the logfile.

=item dbgLogLink = 8192

Inserts a link at the top of each page which can be used to view the
log for the current HTML file.  See also EMBEPRL_VIRTLOG.

Example:

    SetEnv EMBPERL_DEBUG 10477
    SetEnv EMBPERL_VIRTLOG /embperl/log

    <Location /embperl/log>
    SetHandler perl-script
    PerlHandler HTML::Embperl
    Options ExecCGI
    </Location>

=item dbgDefEval = 16384

Shows every time new Perl code is compiled.

=item dbgCacheDisable = 32768

Disables the use of the p-code cache.  All Perl code is recompiled
every time.  (This should not be used in normal operation as it slows
down Embperl dramatically.) This option is only here for debugging
Embperls cache handling. There is no guarantee, that Embperl behaves
the same with and without cache (actually is does not!)


=item dbgHeadersIn = 262144

Log all HTTP headers which are sent from the browser.

=item dbgShowCleanup = 524288

Show every variable which is undef'd at the end of the request.  For
scalar variables, the value before undef'ing is logged.


=back

A good value to start is C<2285> or C<10477> if you want to view the
logfile with your browser.  (Don't forget to set EMBPERL_VIRTLOG.)  If
Embperl crashes, add C<512> so the logfile is flushed after every line
is written and you can see where Embperl is when it crashes.


=head1 B<SYNTAX>

Embperl understands four categories of commands. The first three are
special Embperl commands, and the last category are some HTML tags
that can trigger special processing.  Embperl commands can span
multiple lines and need not start or end at a line boundary.

Before the special Embperl commands are processed, and for the VALUE
attribute of the INPUT tag (see below), all HTML tags are removed and
special HTML characters are translated to their ASCII values (e.g.,
`&lt;' is translated to `<').  You can avoid from this behavior by
preceding the special character or HTML tag with a backslash.  This is
done in case your favorite (WYSIWYG) HTML editor inserts tags like
line breaks or formatting into your Embperl commands where you don't
want them.

B<NOTE:> If you do use an ASCII editor to write your HTML documents,
you should set the option B<optRawInput> so Embperl does not
preprocess your source.  You can also HTML-escape your code
(i.e. write `&lt;' instead of `<'), to avoid ambiguity.  In most cases
it will also work without the optRawInput and HTML-escaping, but in
some cases Embperl will deteced an HTML tag were there isn't one.

All Embperl commands start with a `[' and end with a `]'.  To get a
real `[' you must enter `[['.

Embperl does not use SGML comments (i.e., <! ... !> or similar things)
because some HTML editors can't create them, or it's much more
complicated.  Sinces every HTML editor takes (or B<should> take) `['
and `]' as normal text, there should be no problem.

=over 4

=item B<[+ Perl code +]>

Replace the command with the result of evaluating the Perl code.  The
Perl code can be anything which can be used as an argument to Perl
eval statement.  (See Security below for restrictions.)  Examples:

 [+ $a +]        Replaces the [+ $a +] with the content of
                 the variable $a

 [+ $a+1 +]      (Any expression can be used)

 [+ $x[$i] +]    (Arrays, hashes, and more complex
                  expressions work)

C<NOTE:> Whitespace is ignored.  The output will be automatically
HTML-escaped (e.g., `<' is translated to `&lt;') depending on the
value of the variables C<$escmode>.  You do not have to worry about
it.


=item B<[- Perl code -]>

Executes the Perl code, but delete the whole command from the HTML
output.

Examples:

 [- $a=1 -]            Set the variable $a to one.
 		       No output will be generated.

 [- use SomeModule -]  You can use other modules.

 [- $i=0; while ($i<5) {$i++} -]  Even more complex
                                  statements or multiple
                                  statements are possible.

C<NOTE:> Statements like if, while, for, etc., must be contained in a
single Embperl command.  You cannot have the if in one command block
and the terminating `}' or else in another.

C<NOTE:> To define subroutines, use [! ... !] (see below) instead of
[- ... -] to avoid recompilation of the subroutine on every request.


=item B<[! Perl Code !]>

Same as [- Perl Code -] with the exception that the code is only
executed at the first request.  This could be used to define
subroutines, or do one-time initialization.


=item B<[$ Cmd Arg $]>

Execute an Embperl metacommand.  B<Cmd> can be one of the following.
(B<Arg> varies depending on <Cmd>).

=over 4

=item B<if>, B<elsif>, B<else>, B<endif>

Everything following the B<if> metacommand until the B<else>,
B<elsif>, or B<endif> is only output if the Perl expression given in
B<Arg> is true.  B<else> and B<elsif> work similarly.

Example:

 [$ if $ENV{REQUEST_METHOD} eq 'GET' $]
 Method was GET<BR>
 [$ else $]
 Method other than GET used<BR>
 [$ endif $]

This will send one of the two sentences to the client, depending on the
request method used to retrieve the document.

=item B<while>, B<endwhile>

Executes a loop until the B<Arg> given to B<while> is false.

Example: (see eg/x/while.htm)

 [- $i = 0; @k = keys %ENV -]
 [$ while ($i &lt; $#k) $]
 [+ $k[$i] +] = [+ $ENV{$k[$i]} +]<BR>
 [- $i++ -]
 [$ endwhile $]

This will send a list of all environment variables to the client.

NOTE: The `&lt;' is translated to `<' before call the Perl eval,
unless optRawInput is set.

=item B<hidden>

B<Arg> consists of zero, one or two names of hashes (with or without
the leading %) and an optional array as third parameter.  The
B<hidden> metacommand will generated hidden fields for all data
contained in first hash and not in second hash.  The default used for
the first hash is C<%fdat>, C<%idat> for the second.

If the third parameter is specified, the fields are written in the
order they appear in this array.  That is, all keys of the first hash
must be in this array properly sorted.  This is intended for
situations where you want to pass data from one form to the next, for
example, two forms which should be filled one after the other.
(Examples might be an input form and a second form to review and
accept the input, or a Windows-style "wizard").  Here you can pass
along data from previous forms in hidden fields.  (See eg/x/neu.htm
for an example.)  If you use just the hidden command without
parameters, it simply generates hidden fields for all form fields
submitted to this document which aren't already contained in another
input field.

Example:

    <FORM ACTION="inhalt.htm" METHOD="GET">
	<INPUT TYPE="TEXT" NAME="field1">
    [$ hidden $]
    </FORM>

If you request this with 
    
    http://host/doc.htm?field1=A&field2=B&field3=C

the output will be

    <FORM ACTION="inhalt.htm" METHOD="GET">
	<INPUT TYPE="TEXT" NAME="feld1" VALUE="A">
	
    <INPUT TYPE="HIDDEN" NAME="field2" VALUE="B">
    <INPUT TYPE="HIDDEN" NAME="field3" VALUE="C">
    </FORM>


C<NOTE:> This should only be used for small amount of data, since the
hidden fields are sent to the browser, which sends it back with the
next request.  If you have a large amount of data, store it in a file
with a unique name and send only the filename in a hidden field.  Be
aware of the fact that the data could be change by the browser if the
user didn't behave exactly as you expect. Your program should handle
such situations properly.


=item B<var>

The var command declares one or more variables for use within this
Embperl document and sets the B<strict> pragma. The variable names
must be supplied as space separated list.

Example:
	
	[$var $a %b @c $]

This is the same as writing the following in normal Perl code:

	use strict ;
	use vars qw($a %b @c) ;

NOTE 1: `use strict' within an Embperl document will only aply to the
block where it occurs.


=back

=item B<HTML Tags>

Embperl recognizes the following HTML tags specially.  All other are
simply passed through, as long as they are not part of a Embperl
command.

=over 4

=item B<TABLE>, B</TABLE>, B<TR>, B</TR>

Embperl can generate dynamic tables (one or two dimensional).  You
only need to specify one row or column.

Embperl generates as many rows or columns as necessary. This is done
by using the magic variables $row, $col, and $cnt.  If you don't use
$row/$col/$cnt within a table, Embperl does nothing and simply passes
the table through.

Embperl checks if any of $row, $col, or $cnt is used.  Embperl repeats
all text between <table> and </table>, as long the expressions in
which $row or $cnt occurs is/are defined.

Embperl repeats all text between <tr> and </tr>, as long the
expressions in which $col or $cnt occurs is/are defined.

See also $tabmode (below) for end-of-table criteria.

Examples: (see eg/x/table.htm for more examples)

 [- @k = keys %ENV -]
 <TABLE>
     <TR>
         <TD>[+ $i=$row +]</TD>
         <TD>[+ $k[$row] +]</TD>
         <TD>[+ $ENV{$k[$i]} +]</TD>
     </TR> 
 </TABLE>

This will show all entries in array @k (which contains the keys from
%ENV), so the whole environment is displayed (as in the B<while>
example), with the first column containing the zero-based index, the
second containing the content of the variable name, and the third the
environment variable's value.

This could be used to display the result of database query if you have
the result in an array.  You may provide as many columns as you need.
It is also possible to call a fetch subroutine in each table row.

=item B<TH>, B</TH>

The TH tag is interpreted as table heading.  If the whole row is made
of <TH> </TH> instead of <TD> </TD> it's treated as column heading.
Everything else will be treated as row headings in the future, but is
ignored in the current version.

=item B<DIR>, B<MENU>, B<OL>, B<UL>, B<DL>, B<SELECT>, B</DIR>, B</MENU>,
B</OL>, B</UL>, B</DL>, B</SELECT>

Lists and dropdowns or list boxes are treated exactly as one-
dimensional tables.  Only $row, $maxrow, $col, $maxcol and $tabmode
are honored.  $col and $maxcol are ignored.  See eg/x/lists.htm for an
example.

=item B<OPTION>

Embperl checks if there is a value from the form data for a specific
option in a menu.  If so, this option will be preselected.

Example:

<FORM METHOD="POST">
  <P>Select Tag</P>

  If you request this document with list.htm?SEL1=x
  you can specify that the element which has a value
  of x is initialy selected

  <P><SELECT NAME="SEL1">
     <OPTION VALUE="[+ $v[$row] +]">
        [+ $k[$row] +]
     </OPTION>
  </SELECT></P>
</FORM>


=item B<INPUT>

The INPUT tag interacts with the hashes C<%idat> und C<%fdat>.  If the
input tag has no value, and a key exists with the same text as the
NAME attribute of the input tag, Embperl will generate a VALUE
attribute with the corresponding value of the hash key.

All values of <INPUT> tags are stored in the hash C<%idat>, with NAME
as the hash key and VALUE as the hash value.  Special processing is
done for TYPE=RADIO and TYPE=CHECKBOX.  If the VALUE attribute
contains the same text as the value of the hash the CHECKED attribute
is inserted, else it is removed.  So if you specify as the ACTION URL
the file which contains the form itself, the form will be redisplayed
with same values as entered the first time. (See eg/x/neu.htm for an
example.)

=item B<TEXTAREA>, B</TEXTAREA>

The TEXTAREA tag is treated excatly like other input fields.

=back

=item B<META HTTP-EQUIV= ... >

<meta http-equiv= ... > will override the correspondig http header
this avoids netscape from asking the user to reload the document
when the content-type differs between the http header and the
meta http-equiv
This can also be used to set http headers. When running under mod_perl
http-headers can also be set by the function B<header_out>

    Example to set a http header:

    <META HTTP-EQUIV="Language" CONTENT="DE">

    same using a Apache function

    [- $req_rec ->  header_out("Language" => "DE"); -]



=head1 B<Variable scope and cleanup>

The scope of a variable declared with B<my> or B<local> ends at the
end of the enclosing [+/- ... -/+] block; the [+/- ... -/+] blocks act
much like Perl's { ... } in that regard.

Global variables (everything not declared with B<my> or B<local>) will
be undef'ed at the end of each request, so you don't need to worry
about any old variables laying around and causing suspicous results.
This is only done for variables in the package the code is evaled in--
every variable that does not have an explicit package name.  All
variables with an explicit package name (i.e., in modules you use)
will stay valid until the httpd child process dies.  Embperl will
change the current package to unique name for every document, so the
influence between different documents is kept to a minimum.  You can
set the name of the package with B<EMBPERL_PACKAGE>. (See also Safe
namespaces.)

Since a CGI scripts is always a process of its own, you don't need to
worry about that when you use Embperl as a CGI script.

If you need to declare variables which live more the one HTTP request
(for example, a database handle), you must declare then in another
package (i.e., $Persistent::handle instead of $handle).

If you use the strict pragma, you can use the B<var> metacommand to
declare your variables.


C<NOTE:> Bacause Apache::DBI has its own namespace this module will
work together with Embperl to maintain your persistent database
connection.

You can disable the automatic cleanup of global variables with
B<EMBPERL_OPTIONS> or the B<cleanup> parameter of the B<Execute>
function.

If you like to do your own cleanup you can define a subroutine B<CLEANUP>
in your document. This will called right before the variables will be
cleaned up, but after the connection to the client is closed.

 EXAMPLE:

  [! sub CLEANUP { close FH ; } !]




=head1 B<Predefined variables>

Embperl has some special variables which have a predefined meaning.

=over 4

=item B<%ENV>

Contains the environment as seen from a CGI script.

=item B<%fdat>

Contains all the form data sent to the script by the calling form.
The NAME attribute builds the key and the VALUE attribute is used as
the hash value.  Embperl doesn't care if it's called with the GET or
POST method, but there may be restrictions on the length of parameters
using GET--not from Embperl, but perhaps from the web server,
especially if you're using Embperl's CGI mode--do it's safer to use
POST.

Embperl also supports ENCTYPE multipart/form-data, which is used for
file uploads.  The entry in %fdat corresponding to the file field will
be a filehandle, as with CGI.pm.  (Embperl uses CGI.pm internally to
process forms encoded with multipart/form-data.)

File upload example:

  HTML page:

    <FORM METHOD="POST" ENCTYPE="multipart/form-data">
      <INPUT TYPE="FILE" NAME="ImageName">
    </FORM>

  Embperl ACTION:

    [- if (defined $fdat{ImageName}) {
         open FILE, "> /tmp/file.$$";
	 print FILE $buffer
           while read($fdat{ImageName}, $buffer, 32768);
         close FILE;
       }
    -]
	

=item B<@ffld>

Contains all the field names in the order in which they were sent by
the browser.  This is normally--but not necessarily--the order in
which they appear in your form.

=item B<%idat>

Contains all the values from all input tags processed so far.

=item B<$row>, B<$col>

Row and column counts for use in dynamic tables.  (See HTML tag
table.)

=item B<$maxrow>, B<$maxcol>

Maxium number of rows or columns to display in a table. To prevent
endless loops, $maxrow defaults to 100 and $maxcol to 10.  (See HTML
tag table.)

=item B<$cnt>

Contains the number of table cells displayed so far.  (See HTML tag
table.)

=item B<$tabmode>

Determines how the end of a dynamic table is detected:

B<end of table>

=over 4

=item B<1>

End when an expression with $row becomes undefined.  The row
containing the undefined expression is B<not> displayed.

=item B<2>

End when an expression with $row becomes undefined.  The row
containing the undefined expression B<is> displayed.

=item B<4>

End when $maxrow rows have been displayed.

=back

B<end of row>

=over 4

=item B<16>

End when an expression with $col becomes undefined.  The column
containing the undefined expression is B<not> displayed.

=item B<32>

End when an expression with $col becomes undefined.  The column
containing the undefined B<is> displayed.

=item B<64>

End when $maxcol columns have been displayed.

=back

The default is B<17>, which is correct for all sort of arrays.  You
should rarely need to change it.  The two values can be added
together.

=item B<$escmode>

Turn HTML and URL escaping on and off.  The default is on ($escmode =
3).

=over 4

=item B<$escmode = 3>

The result of a Perl expression is HTML-escaped (e.g., `>' becomes
`&gt;') in normal text and URL-escaped (e.g., `&' becomes `%26')
within an <A> tag.

=item B<$escmode = 2>

The result of a Perl expression is always URL-escaped (e.g., `&'
becomes `%26').

=item B<$escmode = 1>

The result of a Perl expression is always HTML-escaped (e.g., `>'
becomes `&gt;').

=item B<$escmode = 0>

No escaping takes place.

=back

=item B<$req_rec>

This variable is only available when running under control of
mod_perl.  It contains the request record needed to access the Apache
server API.  See B<perldoc Apache> for more information.

=item B<LOG>

This is the filehandle of the Embperl logfile.  By writing `print LOG
"something"' you can add lines to the logfile.  NOTE: The logfile line
should always start with the pid of the current process and continue
with a four character signature delimited by a ':', which specifies
the log reason.

Example: print LOG "[$$]ABCD: your text\n" ;

If you are writing a module for use under Embperl you can say

    tie *LOG, 'HTML::Embperl::Log';

to get a handle by which you can write to the Embperl logfile.

=item B<OUT>

This filehandle is tied to Embperls output stream. Printing to it has the same effect
as using the [+ ... +] block. (See also B<optRedirectStdout>)

=item B<@param>

Will be setup by the B<'param'> parameter of the B<Execute> function. Could be used
to pass parameters to an Embperl document and back. (see B<Execute> for further docs)

=item B<$optXXX> B<$dbgXXX>

All options (see B<EMBPERL_OPTIONS>) and all debugging flags (see B<EMBPERL_DEBUG>) can
be read and set by the corresponding variables.

  Example:

    [- $optRawInput = 1 -] # Turn the RawInput option on
    
    Now write something here

    [- $optRawInput = 0 -] # Turn the RawInput option off again


    [+ $dbgCmd +] # Output the state of the dbgCmd flag


=back


=head1 B<Namespaces and opcode restrictions>

Since most web servers will contain more than one document, it is
necessary to protect the documents against each other.  Embperl does
this by using Perl namespaces.  By default, Embperl executes every
document in its own namespace (package).  This will prevent documents
from accidentally overriding the others' data.  You can change this
behavior (or simply the package name) with the configuration directive
B<EMBPERL_PACKAGE>.  NOTE: By explicitly specifing a package name, you
can access data that is used by another document.

If Embperl is used by more then one person, it may be neccessary to
really protect one document from each other.  To do this, Embperl
gives you the option to use safe namespaces.  Each document runs in
its own package and can't access anything outside of this package.
(See the documentation of Safe.pm for a more detailed discusion about
safe namespaces.)

To make a document run in a safe namespace, simply add
B<optSafeNamespace> to B<EMBPERL_OPTIONS>.  The default package name
used is the same as in normal operation and can be changed with
B<EMBPERL_PACKAGE>.  NOTE: From the perspective of the document being
executed, the code is running in the package B<main>!

A second option to make Embperl more secure is the use of the opcode
restriction mask.  Before you can use the opcode mask, you must set up
a safe compartement.

 B<$cp = HTML::Embperl::AddCompartment($name);>

This will create a new compartment with a default opcode mask and the
name $name.  (The name is used later to tell Embperl which compartment
to use.)  Now you can change the operator mask.  For example:

 B<$cp->deny(':base_loop');>

In your configuration you must set the option B<optOpcodeMask> in
B<EMBPERL_OPTIONS> and specify from which compartment the opcode mask
should be taken by setting B<EMBPERL_COMPARTMENT>.

 Example (for use with mod_perl):

    B<srm.conf:>

    PerlScript startup.pl

    SetEnv EMBPERL_DEBUG 2285

    Alias /embperl /path/to/embperl/eg

    <Location /embperl/x>
    SetHandler perl-script
    PerlHandler HTML::Embperl
    Options ExecCGI
    PerlSetEnv EMBPERL_OPTIONS 12
    PerlSetEnv EMBPERL_COMPARTMENT test
    </Location>

    B<startup.pl:>

    $cp = HTML::Embperl::AddCompartment('test');
    $cp->deny(':base_loop');


This will execute the file startup.pl on server startup.  startup.pl
sets up a compartment named `test', which will have a default opcode
mask and additionaly, will have loops disabled.  Code will be executed
in a safe namespace.

NOTE: The package name from the compartment is B<NOT> used!

Look at the documentation of Safe.pm and Opcode.pm for more detailed
information on how to set opcode masks.


=head1 UTILITY FUNCTIONS

=over 4

=item B<AddCompartment($Name)>

Adds a compartment for use with Embperl.  Embperl only uses the opcode
mask from it, not the package name.  AddCompartement return the newly-
created compartment so you can allow or deny certain opcodes.  See the
Safe.pm documentation for details of setting up a compartment.  See
chapter about Safe namepsaces for details of how Embperl uses it.

Example:

	$cp = HTML::Embperl::AddCompartment('TEST');
	$cp->deny(':base_loop');


=item B<MailFormTo($MailTo, $Subject, $ReturnField)>

Sends the content of the hash %fdat in the order specified by @ffld to
the given B<$MailTo> addressee, with a subject of B<$Subject>.
If you specify $ReturnField the value of that formfield will be used
as B<Return-Path>. Usualy this should be the field where the user enters his
e-mail adress in the form.

If you specifiy the following example code as the action in your form

  <FORM ACTION="x/feedback.htm" METHOD="POST"
        ENCTYPE="application/x-www-form-urlencoded">

The content of the form will be mailed to the given email address.


Example:

 <HTML>
 <HEAD>
 <TITLE>Feedback</TITLE>
 </HEAD>
 <BODY>
        [- MailFormTo('webmaster@domain.xy',
                      'Mail from WWW Form', 'email') -]
        Your data has been sccesfully sent!
 </BODY>
 </HTML>

This will send a mail with all fields of the form to webmaster@domain.xy, with the
Subject 'Mail form WWW Form' and will set the Return-Path of the mail to the
adress which was entered in the field with the name 'email'.

B<NOTE:> You must have Net::SMTP (from the libnet package) installed
to use this function.

=back


=head1 Inside Embperl - How the embedded perl code is actually processed


If Embperl encounters a pieces of perl code
  B<([+/-/!/$ .... $/!/-/+])>
it takes the following steps.


=item 1.
Remove anything which looks like an HTML tag

=item 2.
Translate HTML escapes to their corresponding ASCII characters

=item 3.
Remove all carriage returns

=item 4.
Eval the perl code into a subroutine

=item 5.
Call the subroutine

=item 6.
Escape special characters in the return value

=item 7.
Send the return value as output to the destination (browser or file)


Steps 1-4 take place only the first time the perl code is encountered.
Embperl stores the evaled subroutine, so all subsequent requests only
need to execute steps 5-7.

Steps 6 and 7 take place only for code surrounded by [+ ... +].


What does this mean?

Lets take a piece of code like the following:

 [+ <BR>
 $a = "This '&gt;' is a greater-than sign"
 <BR> +]

=head2 1. Remove the HTML tags.  Now it looks like

 [+
 $a = "This '&gt;' is a greater-than sign"
 +]

The <BR>s were inserted by some WYSIWYG HTML editor (e.g., by hitting
return to make the source more readable.  Also, such editors often
generate "random" tags like <FONT>, etc.).  Embperl removes them so
they don't cause syntax errors.

There are cases where you actually want the HTML tag to be there.  For
example, suppose you want to output something like

 [+ "<FONT COLOR=$col>" +]

If you write it this way Embperl will just remove everything, leaving
only

 [+ "" +]


There are several ways to handle this correctly.

 a. <FONT COLOR=[+$col+]>
    Move the HTML tag out of the perl code. This is the best way, but
    not possible everytime.

 b. [+ "\<FONT COLOR=$col>" +]
    You can escape the opening angle bracket of the tag with `\'.

 c. [+ "&lt;FONT COLOR=$col&gt;" +]

    You can use the HTML escapes instead of the ASCII characters.
    Most HTML editors will automatically do this.  (In this case,
    you don't have to worry about it at all.)

 d. Set optRawInput (see below).
    This will completely disable the removal of HTML tags.

NOTE: In cases b-d, you must also be aware of output escaping (see
below).

You should also be aware that Embperl will interpret the perl
spaceship operator (<>) as an HTML tag and will remove it.  So instead
of

  [- $line = <STDIN>; -]

you need to write either

 a. [- $line = \<STDIN>; -]
 b. [- $line = &lt;STDIN&gt;; -]

Again, if you use a high-level HTML editor, it will probably write
version (b) for you automatically.


=head2 2. Translate HTML escapes to ASCII characters

Since perl doesn't understand things like $a &lt; $b, Embperl will
translate it to $a < $b.  If we take the example from earlier, it will
now look like

 [+
 $a = "This '>' is a greater sign"
 +]

This step is done to make it easy to write perl code in a high level
HTML editor.  You do not have to care that your editor writes &gt;
instead of > in the source.

Again, sometimes you need to have such escapes in your code.  You can
write them

 a. \&gt;
    Escape them with a `\' and Embperl will not translate them.

 b. &amp;gt;
    Write the first `&' as its HTML escape (&amp;).  A normal HTML
    editor will do this on its own if you enter &gt; as text.

 c. Set optRawInput (see below)
    This will completely disable the input translation.

Since not all people writing in a high level or WYSIWYG HTML editor,
there is an option to disable steps 1 and 2.  You can use the
B<optRawInput> in EMBPERL_OPTIONS to tell Embperl to leave the perl
code as it is.  It is highly recommended to set this options if you
are writing your HTML in an ASCII editor. You normally don't want to
set it if you use some sort of high level HTML editor.


=head2 3. Remove all carriage returns

All carriage returns (B<\r>) are removed from the perl code, so you
can write source on a DOS/Windows platform and execute it on a UNIX
server.  (perl doesn't like getting carriage returns in the code it
parses.)


=head2 4. Eval perl code into a subroutine

The next step generates a subroutine out of your perl code.  In the
above example it looks like:

sub foo
    {
    $a = "This '>' is a greater sign"
    }

The subroutine is now stored in the perl interpreter in its internal
precompiled format and can be called later as often as necessary
without doing steps 1-4 again.  Embperl recognizes if you request the
same document a second time and will just call the compiled
subroutine.  This will also speed up the execution of dynamic tables
and loops, because the code inside must be compiled only on the first
iteration.


=head2 5. Call the subroutine

Now the subroutine can be called to actually execute the code.

If Embperl isn't executing a [+ ... +] block we are done.  If it is a
[+ ... +] block, Embperl needs to generate output, so it continues.


=head2 6. Escape special characters in the return value

Our example returns the string:

"This '>' is a greater sign"

The greater sign is literal text (and not a closing html tag), so
according to the HTML specification it must be send as &gt; to the
browser.  In most cases this won't be a problem, because the browser
will display the correct text if we send a literal '>'.  Also we could
have directly written &gt; in our perl string.  But when the string
is, for example, the result of a database query and/or includes
characters from national character sets, it's absolutely necessary to
send them correctly escaped to the browser to get the desired result.

A special case is the <A> HTML tag.  Since it includes a URL the text
must be URL-escaped instead of HTML-escaped.  This means special
characters like `&' must be sent by there hexadecimal ASCII code and
blanks must be translated to a `+' sign.  If you do not do this your
browser may not be able to interpret the URL correctly.

Example:

   <A HREF='http://host/script?name="[+$n+]"'>

When $n is "My name" the requested URL, when you click on the
hyperlink, will be

   http://host/script?name=My+name


In some cases it is useful to disable escaping.  This can be done by
the variable B<$escmode>.

Example: (For better readability, we assume that optRawInput is set.
Without it, you need to cover the Embperl pre-processing described in
steps 1-3.)

    [+ "<FONT COLOR=5>" +]

    This will be sent to the browser as &lt;FONT COLOR=5&gt;, so you
    will see the tag on the browser screen instead of the browser
    switching the color.

    [+ local $escmode=0 ; "<FONT COLOR=5>" +]

    This will (locally) turn off escaping and send the text as a plain
    HTML tag to the browser, so the color of the output will change.


=head2 7. Send the return value as output to the destination
(browser/file)

Now everything is done and the output can be send to the browser.  If
you haven't set dbgEarlyHttpHeaders, the output is buffered until the
successful completion of document execution of the document and sent
to the browser along with the HTTP headers.  If an error occurs an
error document is sent instead.

The content length and every <META HTTP-EQUIV=...> is added to the
HTTP header before sending it.  If Embperl is executed as a subrequest
or the output is going to a file no http header is sent.


=head1 PERFORMANCE

To get the best performace from Embperl, it is necessary to restrict
logging to a minimum.  You can drastically slow down Embperl if you
enable all logging options.  (This is why `make test' takes a while to
run.)  You should B<never> enable B<dbgFlushOutput>, B<dbgFlushLog> or
B<dbgCacheDisable> in a production environment.  More debugging
options are useful for developement where it doesn't matter if the
request takes a little bit longer, but on a heavily-loaded server they
should be disabled.

Also take a look at B<mod_perl_tuning.pod> for general ideas about
performance.


=head1 BUGS

None known.

Under perl5.004 there are memory leaks.  This is not an Embperl bug,
but can cause your httpd to grow endlessly when running under
mod_perl.  Please upgrade to perl5.004_04 to fix this.  You should
also upgrade to a mod_perl version higher than 1.07_01 as soon as
possible, because until 1.07_01 there is a memory leak in
Apache->push_handler.


=head1 COMPATIBILITY

I have tested Embperl succesfully

on B<Linux 2.x> with

perl5.004_04
apache_1.2.5
apache_1.2.6
apache_1.3b3
apache_1.3b5
apache_1.3b6
apache_ssl (Ben SSL)
Stringhold 2.2

on B<Windows NT 4.0> with

perl5.004_04
apache_1.3b5

on B<Windows 95> with
perl5.004_02 (binary distribution)
Offline mode

I know from other people that it works on many other UNIX systems


=head1 FEEDBACK and BUG REPORTS

Please let me know if you use or test this module.  Bugs, questions,
suggestions for things you would find useful, etc., are discussed on
the mod_perl mailling list.

>From the mod_perl README:
 
The Apache/Perl mailing list (modperl@apache.org) is available for mod_perl
users and developers to share ideas, solve problems and discuss things related
to mod_perl and the Apache::* (and Embperl) modules. To subscribe to this list, send mail to
majordomo@apache.org with the string "subscribe modperl" in the body. 


There is a hypermail archive for this list available from:
http://outside.organic.com/mail-archives/modperl/

There is an Epigone archive for the mod_perl mailing list at
http://forum.swarthmore.edu/epigone/modperl


=head1 SUPPORT

You can get free support on the mod_perl mailing list (see above).  If
you need commercial support (with a guarantee for response time or a
solution) for Embperl or want a web site where you can run your
Embperl/mod_perl scripts without setting up your own web server,
please send email to info@ecos.de.


=head1 REFERENCES

 mod_perl               http://perl.apache.org/
 mod_perl FAQ           http://perl.apache.org/faq
 Embperl                http://perl.apache.org/embperl.html
 DBIx::Recordset	http://ftp.dev.ecos.de/pub/perl/dbi
 apache web server      http://www.apache.org/
 ben-ssl (free httpsd)  http://www.apache-ssl.org/
 stronghold (commerical httpsd) http://www.c2.net/
    europe              http://www.eu.c2.net/
 see also               http://www.perl.com/CPAN/modules/by-module/Apache/apache-modlist.html


=head1 AUTHOR

G. Richter (richter@dev.ecos.de)


=head1 SEE ALSO

perl(1), mod_perl, Apache httpd
