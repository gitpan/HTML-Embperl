#!/usr/bin/perl --
# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl test.pl'


@tests = (
    'ascii',
    'pure.htm',
    'plain.htm',
    'plain.htm',
    'plain.htm',
    'plainblock.htm',
    'plainblock.htm',
    'error.htm???8',
    'error.htm???8',
    'error.htm???8',
    'unclosed.htm???1',
#    'errorright.htm???1',
    'notfound.htm???1',
    'noerr/noerrpage.htm???8?2',
    'errdoc/errdoc.htm???8?262144',
    'rawinput/rawinput.htm????16',
    'var.htm',
    'varerr.htm???-1',
    'varerr.htm???2',
    'escape.htm',
    'tagscan.htm',
    'tagscan.htm??1',
    'if.htm',
    'ifperl.htm',
    'loop.htm?erstes=Hallo&zweites=Leer+zeichen&drittes=%21%22%23%2a%2B&erstes=Wert2',
    'loopperl.htm?erstes=Hallo&zweites=Leer+zeichen&drittes=%21%22%23&erstes=Wert2',
    'table.htm',
    'table.htm??1',
    'lists.htm?sel=2&SEL1=B&SEL3=D&SEL4=cc',
    'mix.htm',
    'nesting.htm',
    'object.htm???2',
    'discard.htm???12',
    'input.htm?feld5=Wert5&feld6=Wert6&feld7=Wert7&feld8=Wert8&cb5=cbv5&cb6=cbv6&cb7=cbv7&cb8=cbv8&cb9=ncbv9&cb10=ncbv10&cb11=ncbv11&mult=Wert3&mult=Wert6&esc=a<b&escmult=a>b&escmult=Wert3',
    'hidden.htm?feld1=Wert1&feld2=Wert2&feld3=Wert3&feld4=Wert4',
    'java.htm',
    'inputjava.htm',
    'post.htm',
    'upload.htm?multval=A&multval=B&multval=C&single=S',
    'reqrec.htm',
    'reqrec.htm',
    'include.htm',
    'includeerr1.htm???1',
    'includeerr2.htm???4',
    'registry/Execute.htm',
    'registry/errpage.htm???16',
    'callsub.htm',
    'callsub.htm',
    'importsub.htm',
    'importsub.htm',
    'importsub2.htm',
    'importmodule.htm',
    'recursexec.htm',
    'nph/div.htm????64',
    'nph/npherr.htm???8?64',
    'nph/nphinc.htm????64',
    'sub.htm',
    'sub.htm',
    'exit.htm',
    'exit2.htm',
    'chdir.htm?a=1&b=2&c=&d=&f=5&g&h=7&=8&=',
    'chdir.htm?a=1&b=2&c=&d=&f=5&g&h=7&=8&=',
    'allform/allform.htm?a=1&b=2&c=&d=&f=5&g&h=7&=8&=???8192',
    'stdout/stdout.htm????16384',
    'nochdir/nochdir.htm?a=1&b=2???384',
    'match/div.htm',
    'match/div.asc',
    'http.htm',
    'div.htm',
    'taint.htm???1',
    'ofunc/div.htm',
    'safe/safe.htm???-1?4',
    'safe/safe.htm???-1?4',
    'safe/safe.htm???-1?4',
    'opmask/opmask.htm???-1?12?TEST',
    'opmask/opmasktrap.htm???2?12?TEST',
    'mdatsess.htm?cnt=0',
    'setsess.htm?a=1',
    'mdatsess.htm?cnt=1',
    'getnosess.htm?nocookie=2',
    'mdatsess.htm?cnt=2',
    'getsess.htm',
    'mdatsess.htm?cnt=3',
    'execgetsess.htm',
    ) ;


# avoid some warnings:

use vars qw ($httpconfsrc $httpconf $EPPORT $EPPORT2 *SAVEERR *ERR $EPHTTPDDLL $EPSTARTUP $EPDEBUG
             $EPSESSIONDS $EPSESSIONCLASS $EPSESSIONVERSION) ;

    {
    local $^W = 0 ;
    eval " use Win32::Process; " ;
    $win32loaderr = $@ ;
    eval " use Win32; " ;
    $win32loaderr ||= $@ ;
    }

BEGIN 
    { 
    $fatal  = 1 ;
    $^W     = 1 ;
    $|      = 1;
    
    eval 'use ExtUtils::testlib' if (defined ($ARGV[0]) && $ARGV[0] =~ /b/) ;

    #### install handler which kill httpd when terminating ####

    $SIG{__DIE__} = sub { 
	return unless $_[0] =~ /^\*\*\*/ ;
	return unless $killhttpd ;
	if ($EPWIN32)
	    {
	    $HttpdObj->Kill(-1) if ($HttpdObj) ;
	    }
	else
	    {
	    system "kill `cat $tmppath/httpd.pid` 2> /dev/null" if ($EPHTTPD ne '') ;
	    }
	} ;

    print "\nloading...                    ";
    
    }

END 
    { 
    print "\nTest terminated with fatal error\n" if ($fatal) ; 
    system "kill `cat $tmppath/httpd.pid` 2> /dev/null" if ($EPHTTPD ne '' && $killhttpd && !$EPWIN32) ;
    exit ($fatal || $err) ;	
    }




$confpath = 'test/conf' ;

$cmdarg   = $ARGV[0] || '' ;
shift @ARGV ;
$dbgbreak = 0 ;
if ($cmdarg eq 'dbgbreak')
	{
	$dbgbreak = 1 ;
	$cmdarg = shift @ARGV || '' ;
	}

#### read config ####

if ($cmdarg =~ /f/)
    { do $ARGV[0] ; shift @ARGV ; }
else
    { do "$confpath/config.pl" ; }


$EPPORT2 = ($EPPORT || 0) + 1 ;
$EPSESSIONCLASS = $ENV{EMBPERL_SESSION_CLASS} || ($EPSESSIONVERSION?'Win32':'0') ;
$EPSESSIONDS    = $ENV{EMBPERL_SESSION_DS} || 'dbi:mysql:session' ;

die "You must install libwin32 first" if ($EPWIN32 && $win32loaderr && $EPHTTPD) ;

#### setup paths #####

$inpath  = 'test/html' ;
$tmppath = 'test/tmp' ;
$cmppath = 'test/cmp' ;


#### setup files ####

$httpdconfsrc = "$confpath/httpd.conf.src" ;
$httpdconf = "$confpath/httpd.conf" ;
$httpderr   = "$tmppath/httpd.err.log" ;
$offlineerr = "$tmppath/test.err.log" ;
$outfile    = "$tmppath/out.htm" ;
$logfile    = "$tmppath/test.log" ;

#### setup path in URL ####

$embploc = 'embperl/' ;
if ($EPWIN32)
    {
    $cgiloc  = 'cgi-bin-32/' ;
    }
else
    {
    $cgiloc  = 'cgi-bin/' ;
    }


$port    = $EPPORT ;
$host    = 'localhost' ;
$httpdpid = 0 ;
$defaultdebug = 0x785ffd ;


if ($cmdarg =~ /\?/)
    {
    print "\n\n" ;
    print "test.pl [options] [files]\n" ;
    print "files: <filename>|<testnumber>|-<testnumber>\n\n" ;
    print "options:\n" ;
    print "o	test offline\n" ;
    print "c	test cgi\n" ;
    print "h	test mod_perl\n" ;
    print "e	test execute\n" ;
    print "r	don't kill httpd at end of test\n" ;
    print "l	loop forever\n" ;
    print "m	start httpd with mulitple childs\n" ;
    print "v    memory check\n" ;
    print "g    exit if httpd grows after 2 loop\n" ;   
    print "f    file to use for config.pl\n" ;
    print "x    do not start httpd\n" ;
    print "u    use unique filenames\n" ;
    print "n    do not check httpd errorlog\n" ;
    print "q    set debug to 0\n" ;
    print "i    ignore errors\n" ;
    print "t    list tests\n" ;
    print "b    use uninstalled version (from blib/..)\n" ;
    print "\n\n" ;
    print "path\t$EPPATH\n" ;
    print "httpd\t$EPHTTPD\n" ;
    print "port\t$port\n" ;
    exit (1) ;
    }

if ($cmdarg =~ /t/)
    {
    $i = 0 ;
    foreach $t (@tests)
	{
	print "$i = $t\n" ;
	$i++ ;
	}
    exit (1) ;
    }


	
$killhttpd = 1 ; # kill httpd at end of test
$multhttpd = 0 ; # start httpd with child fork
$looptest  = 0 ; # endless loop tests

$vmmaxsize = 0 ;
$vminitsize = 0 ;
$vmhttpdsize = 0 ;
$vmhttpdinitsize = 0 ;


#####################################################

sub chompcr

    {
    local $^W = 0 ;

    chomp ($_[0]) ;
    if ($_[0] =~ /(.*?)\s*\r$/) 
	{
	$_[0] = $1
	}
    elsif ($_[0] =~ /(.*?)\s*$/) 
	{
	$_[0] = $1
	}
    }

#####################################################

sub CmpFiles 
    {
    my ($f1, $f2, $errin) = @_ ;
    my $line = 1 ;
    my $err  = 0 ;

    open F1, $f1 || die "***Cannot open $f1" ; 
    if (!$errin)
	{
	open F2, $f2 || die "***Cannot open $f2" ; 
	}

    while (defined ($l1 = <F1>))
	{
	chompcr ($l1) ;
	if (!$errin) 
	    {
	    $l2 = <F2> ;
	    chompcr ($l2) ;
	    }
	if (!defined ($l2))
	    {
	    print "\nError in Line $line\nIs:\t$l1\nShould:\t<EOF>\n" ;
	    return $line ;
	    }

	
	$eq = 0 ;
	while (((!$notseen && ($l2 =~ /^\^\^(.*?)$/)) || ($l2 =~ /^\^\-(.*?)$/)) && !$eq)
	    {
	    $l2 = $1 ;
	    if (($l1 =~ /^\s*$/) && ($l2 =~ /^\s*$/))
                { 
                $eq = 1 ;
                }
            else
                {
                $eq = $l1 =~ /$l2/ ;
                }
            $l2 = <F2> if (!$eq) ;
	    chompcr ($l2) ;
	    }

	if (!$eq)
	    {
	    if ($l2 =~ /^\^(.*?)$/)
		{
		$l2 = $1 ;
		$eq = $l1 =~ /$l2/ ;
		}
	    else
		{
		$eq = $l1 eq $l2 ;
		}
	    }

	if (!$eq)
	    {
	    print "\nError in Line $line\nIs:\t>$l1<\nShould:\t>$l2<\n" ;
	    return $line ;
	    }
	$line++ ;
	}

    if (!$errin)
	{
	while (defined ($l2 = <F2>))
	   {
	   chompcr ($l2) ;
	   if (!($l2 =~ /^\s*$/))
		{
		print "\nError in Line $line\nIs:\t\nShould:\t$l2\n" ;
		return $line ;
		}
	    $line++ ;
	    }
	}

    close F1 ;
    close F2 ;

    return $err ; 
    }

#########################
#
# GET/POST via HTTP.
#

sub REQ

    {
    my ($loc, $file, $query, $ofile, $content, $upload) = @_ ;
	
    eval 'require LWP::UserAgent' ;
    

    if ($@)
	{
	return "LWP not installed\n" ;
	}
    
    eval 'use HTTP::Request::Common' ;
    if ($@)
	{
	return "HTTP::Request::Common not installed\n" ;
	}
    
    
    $query ||= '' ;     
	
    my $ua = new LWP::UserAgent;    # create a useragent to test

    my($request,$response,$url);


    if (!$upload)
	{
	$url = new URI::URL("http://$host:$port/$loc$file?$query");

	$request = new HTTP::Request($content?'POST':'GET', $url);
        $request -> header ('Cookie' => $cookie) if ($cookie && !($query =~ /nocookie/)) ;
        
	$request -> content ($content) if ($content) ;
	}
    else
	{
	my @q = split (/\&|=/, $query) ;
        
        $request = POST ("http://$host:$port/$loc$file",
					Content_Type => 'form-data',
					Content      => [ upload => [undef, 'upload-filename', 
								    'Content-type' => 'test/plain',
								    Content => $upload],
							  content => $content,
                                                          @q ]) ;
	}
	    
    #print "Request: " . $request -> as_string () ;
	    

    $response = $ua->request($request, undef, undef);

    open FH, ">$ofile" ;
    print FH $response -> content ;
    close FH ;

    my $c = $response -> header ('Set-Cookie') || '' ;
    $cookie = $c if (!$cookie && ($c =~ /EMBPERL_UID/)) ;  
    #print "Got Cookie $cookie\n" ;

    #print $response -> headers -> as_string () ;

    return $response -> message if (!$response->is_success) ;
    
    return "ok" ;
    }

###########################################################################
#
# Get Memory from /proc filesystem
#

sub GetMem
    {
    my ($pid) = @_ ;
    
    my @status ;
    
    open FH, "/proc/$pid/status" or die "Cannot open /proc/$pid/status" ;
    @status = <FH> ;
    close FH ;

    my @line = grep (/VmSize/, @status) ;
    $line[0] =~ /^VmSize\:\s+(\d+)\s+/ ;
    my $vmsize = $1 ;
    
    return $vmsize ;
    }           

###########################################################################
#
# Get output in error log
#

sub CheckError

    {
    my ($cnt) = @_ ;
    my $err = 0 ;
    my $ic ;

    $cnt ||= 0 ;
    $ic    = $cnt ;

    while (<ERR>)
	{
	chomp ;
	if (!($_ =~ /^\s*$/) &&
	    !($_ =~ /\-e /) &&
	    !($_ =~ /Warning/) &&
	    !($_ =~ /mod_ssl\:/) &&
	    !($_ =~ /SES\:/) &&
	    $_ ne 'Use of uninitialized value.')
	    {
	    $cnt-- ;
	    if ($cnt < 0)
		{ 
		print "\n\n" if ($cnt == -1) ;
		print "[$cnt]$_\n" ;
		$err = 1 ;
		}
	    }
	}
    
    if ($cnt > 0)
	{
	$err = 1 ;
	print "\n\nExpected $cnt more error(s) in logfile\n" ;
	}

    print "\n" if $err ;

    return $err ;
    }

#########################


sub CheckSVs

    {
    my ($loopcnt, $n) = @_ ;
    
    open SVLOG, $logfile or die "Cannot open $logfile ($!)" ;

    seek SVLOG, -3000, 2 ;

    while (<SVLOG>)
	{
	if (/Exit-SVs: (\d+)/)
	    {
	    $num_sv = $1 || 0;
	    $last_sv[$n] ||= 0 ;
	    print "SVs=$num_sv/$last_sv[$n]/$max_sv " ;
	    if ($num_sv > $max_sv) 
		{
		print "GROWN " ;
		$max_sv = $num_sv ;
		
		}
	    die "\n\nMemory problem (SVs)" if ($exitonmem && $loopcnt > 2 && $last_sv[$n] < $num_sv) ;
	    $last_sv[$n] = $num_sv  ;
	    last ;
	    }
	 }

     close SVLOG ;
     }



######################### We start with some black magic to print on failure.


#use Config qw (myconfig);
#print myconfig () ;


##################


use HTML::Embperl;
require HTML::Embperl::Module ;

print "ok\n";

#### check commandline options #####

if ($EPHTTPD ne '')
    { $testtype = $cmdarg || 'ohce' ; }
else
    { $testtype = $cmdarg || 'oe' ; }

$checkerr = 1 ;
$checkerr = 0 if ($cmdarg =~/n/) ;
$starthttpd = 1 ;
$starthttpd = 0 if ($cmdarg =~/x/) ;
$killhttpd = 0 if (!$starthttpd) ;
$killhttpd = 0 if ($cmdarg =~/r/) ;
$multhttpd = 1 if ($cmdarg =~/m/) ;
$looptest  = 1 if ($cmdarg =~/l/) ;
$memcheck  = 1 if ($cmdarg =~/v/) ;
$exitonmem = 1 if ($cmdarg =~/g/) ;
$outfile .= ".$$" if ($cmdarg =~/u/) ;
$defaultdebug = 0 if ($cmdarg =~/q/) ;
$ignoreerror = 1 if ($cmdarg =~/i/) ;


if ($#ARGV >= 0)
    {
    if ($ARGV[0] =~ /^-/)
	{
	$#tests = - $ARGV[0] ;
	}
    elsif ($ARGV[0] =~ /^\d/)
	{
	@savetests = @tests ;
	@tests = () ;
	while ($t = shift @ARGV)
	    {
	    push @tests, $savetests[$t] ;
	    }
	}
    else
	{
	@tests = @ARGV ;
	}
    }
    


#### preparefile systems stuff ####

$um = umask 0 ;
mkdir $tmppath, 0777 ;
chmod 0777, $tmppath ;
umask $um ;

unlink ($logfile) ;
unlink ($outfile) ;
unlink ($httpderr) ;
unlink ($offlineerr) ;

-w $tmppath or die "***Cannot write to $tmppath" ;

#### some more init #####
	
$DProf = $INC{'Devel/DProf.pm'}?1:0 ;    
$err = 0 ;
$loopcnt = 0 ;
$notseen = 1 ;
%seen = () ;
$max_sv = 0 ;
	
$cp = HTML::Embperl::AddCompartment ('TEST') ;

$cp -> deny (':base_loop') ;


do  
    {
    #############
    #
    #  OFFLINE
    #
    #############

    if ($testtype =~ /o/)
	{
	print "\nTesting offline mode...\n\n" ;

	if ($loopcnt == 0)
	    {   
	    open (SAVEERR, ">&STDERR")  || die "Cannot save stderr" ;  
	    open (STDERR, ">$offlineerr") || die "Cannot redirect stderr" ;  
	    open (ERR, "$offlineerr")  || die "Cannot open redirected stderr ($offlineerr)" ;  ;  
	    }

	$n = 0 ;
	$t_offline = 0 ;
	$n_offline = 0 ;
	foreach $url (@tests)
	    {
	    ($file, $query_info, $debug, $errcnt, $option, $ns) = split (/\?/, $url) ;
	    next if ($file eq 'http.htm') ;
	    next if ($file eq 'taint.htm') ;
	    next if ($file eq 'reqrec.htm') ;
	    next if ($file eq 'http.htm') ;
	    next if ($file eq 'post.htm') ;
	    next if ($file eq 'upload.htm') ;
	    next if ($file =~ /^exit/) ;
	    next if ($file =~ /registry/) ;
	    next if ($file =~ /match/) ;
	    next if ($file =~ /sess\.htm/) ;
	    next if ($DProf && ($file =~ /safe/)) ;
	    next if ($DProf && ($file =~ /opmask/)) ;

	    $debug ||= $defaultdebug ;  
	    $page = "$inpath/$file" ;
	    $errcnt ||= 0 ;
    
	    $notseen = $seen{"o:$page"}?0:1 ;
	    $seen{"o:$page"} = 1 ;
    
	    delete $ENV{EMBPERL_OPTIONS} if (defined ($ENV{EMBPERL_OPTIONS})) ;
	    $ENV{EMBPERL_OPTIONS} = $option if (defined ($option)) ;
	    $ENV{EMBPERL_COMPARTMENT} = $ns if (defined ($ns)) ;
	    @testargs = ( '-o', $outfile ,
			  '-l', $logfile,
			  '-d', $debug,
			   $page, $query_info || '') ;
	    unshift (@testargs, 'dbgbreak') if ($dbgbreak) ;
    
	    $txt = $file . ($debug != $defaultdebug ?"-d $debug ":"") . '...' ;
	    $txt .= ' ' x (30 - length ($txt)) ;
	    print $txt ; 
    
    
	    unlink ($outfile) ;

	    $n_offline++ ;
	    $t1 = HTML::Embperl::Clock () ;
	    $err = HTML::Embperl::run (@testargs) ;
	    $t_offline += HTML::Embperl::Clock () - $t1 ;

	    if ($memcheck)
		{
		my $vmsize = GetMem ($$) ;
		$vminitsize = $vmsize if $loopcnt == 2 ;
		print "\#$loopcnt size=$vmsize init=$vminitsize " ;
		print "GROWN! at iteration = $loopcnt  " if ($vmsize > $vmmaxsize) ;
		$vmmaxsize = $vmsize if ($vmsize > $vmmaxsize) ;
		CheckSVs ($loopcnt, $n) ;
		}
		
	    $errin = $err ;
	    $err = CheckError ($errcnt) if ($err == 0 || ($errcnt > 0 && $err == 500) || $file eq 'notfound.htm') ;
    
	    
	    if ($err == 0 && $errin != 500 && $file ne 'notfound.htm')
		{
		$page =~ /.*\/(.*)$/ ;
		$org = "$cmppath/$1" ;

		$err = CmpFiles ($outfile, $org, $errin) ;
		}

	    print "ok\n" unless ($err) ;
	    $err = 0 if ($ignoreerror) ;
	    last if $err ;
	    $n++ ;
	    }
	}
    
    if ($testtype =~ /e/)
	{
	#############
	#
	#  Execute
	#
	#############

	if ($err == 0)
	    {
	    print "\nTesting Execute function...\n\n" ;

    
	    HTML::Embperl::Init ($logfile) ;
    
	    $notseen = 1 ;        
	    $txt = 'div.htm' ;
	    $org = "$cmppath/$txt" ;
	    $src = "$inpath/$txt" ;
	    $errcnt = 0 ;

		{
		local $/ = undef ;
		open FH, $src or die "Cannot open $src ($!)" ;
		binmode FH ;
		$indata = <FH> ;
		close FH ;
		}


	    $txt2 = "$txt from file...";
	    $txt2 .= ' ' x (30 - length ($txt2)) ;
	    print $txt2 ; 

	    unlink ($outfile) ;
	    $t1 = HTML::Embperl::Clock () ;
	    $err = HTML::Embperl::Execute ({'inputfile'  => $src,
					    'mtime'      => 1,
					    'outputfile' => $outfile,
					    'debug'      => $defaultdebug,
					    }) ;
		
	    $t_exec += HTML::Embperl::Clock () - $t1 ; 

	    $err = CheckError ($errcnt) if ($err == 0) ;
	    $err = CmpFiles ($outfile, $org)  if ($err == 0) ;
	    print "ok\n" unless ($err) ;

	    if ($err == 0)
		{
		$txt2 = "$txt from memory...";
		$txt2 .= ' ' x (30 - length ($txt2)) ;
		print $txt2 ; 

		unlink ($outfile) ;
		$t1 = HTML::Embperl::Clock () ;
		$err = HTML::Embperl::Execute ({'input'      => \$indata,
						'inputfile'  => 'i1',
						'mtime'      => 1,
						'outputfile' => $outfile,
						'debug'      => $defaultdebug,
						}) ;
		$t_exec += HTML::Embperl::Clock () - $t1 ; 
		    
		$err = CheckError ($errcnt) if ($err == 0) ;
		$err = CmpFiles ($outfile, $org)  if ($err == 0) ;
		print "ok\n" unless ($err) ;
		}

	    if ($err == 0)
		{
		$txt2 = "$txt to memory...";
		$txt2 .= ' ' x (30 - length ($txt2)) ;
		print $txt2 ; 

		my $outdata ;
		unlink ($outfile) ;
		$t1 = HTML::Embperl::Clock () ;
		$err = HTML::Embperl::Execute ({'inputfile'  => $src,
						'mtime'      => 1,
						'output'     => \$outdata,
						'debug'      => $defaultdebug,
						}) ;
		$t_exec += HTML::Embperl::Clock () - $t1 ; 
		    
		$err = CheckError ($errcnt) if ($err == 0) ;
	
		open FH, ">$outfile" or die "Cannot open $outfile ($!)" ;
		print FH $outdata ;
		close FH ;
		$err = CmpFiles ($outfile, $org)  if ($err == 0) ;
		print "ok\n" unless ($err) ;
		}

	    if ($err == 0)
		{
		$txt2 = "$txt from/to memory...";
		$txt2 .= ' ' x (30 - length ($txt2)) ;
		print $txt2 ; 

		my $outdata ;
		unlink ($outfile) ;
		$t1 = HTML::Embperl::Clock () ;
		$err = HTML::Embperl::Execute ({'input'      => \$indata,
						'inputfile'  => $src,
						'mtime'      => 1,
						'output'     => \$outdata,
						'debug'      => $defaultdebug,
						}) ;
		$t_exec += HTML::Embperl::Clock () - $t1 ; 
		    
		$err = CheckError ($errcnt) if ($err == 0) ;
	
		open FH, ">$outfile" or die "Cannot open $outfile ($!)" ;
		print FH $outdata ;
		close FH ;
		$err = CmpFiles ($outfile, $org)  if ($err == 0) ;
		print "ok\n" unless ($err) ;
		}

	    $txt = 'error.htm' ;
	    $org = "$cmppath/$txt" ;
	    $src = "$inpath/$txt" ;

	    $notseen = $seen{"o:$src"}?0:1 ;
	    $seen{"o:$src"} = 1 ;


	    if ($err == 0)
		{
		$txt2 = "$txt to memory...";
		$txt2 .= ' ' x (30 - length ($txt2)) ;
		print $txt2 ; 

		my $outdata ;
		unlink ($outfile) ;
		$t1 = HTML::Embperl::Clock () ;
		$err = HTML::Embperl::Execute ({'inputfile'  => $src,
						'mtime'      => 1,
						'output'     => \$outdata,
						'debug'      => $defaultdebug,
						}) ;
		$t_exec += HTML::Embperl::Clock () - $t1 ; 
		    
		$err = CheckError (8) if ($err == 0) ;
	
		open FH, ">$outfile" or die "Cannot open $outfile ($!)" ;
		print FH $outdata ;
		close FH ;
		$err = CmpFiles ($outfile, $org)  if ($err == 0) ;
		print "ok\n" unless ($err) ;
		}

	    HTML::Embperl::Term () ;
	    }
	}

    if ((($testtype =~ /e/) || ($testtype =~ /o/)) && $looptest == 0)
	{
	close STDERR ;
	open (STDERR, ">&SAVEERR") ;
	}

    #############
    #
    #  mod_perl & cgi
    #
    #############

    if ($testtype =~ /h/)
	{ $loc = $embploc ; }
    elsif ($testtype =~ /c/)   
	{ $loc = $cgiloc ; }
    else
	{ $loc = '' ; }


    if ($loc ne '' && $err == 0 && $loopcnt == 0 && $starthttpd)
	{
	#### Configure httpd conf file
	$EPDEBUG = $defaultdebug ;

	my $cf ;
	my $rs = $/ ;
	undef $/ ;

	$ENV{EMBPERL_LOG} = $logfile ;
	open IFH, $httpdconfsrc or die "***Cannot open $httpconfsrc" ;
	$cf = <IFH> ;
	close IFH ;
	open OFH, ">$httpdconf" or die "***Cannot open $httpconf" ;
	eval $cf ;
	die "***Cannot eval $httpconf ($@)" if ($@) ;
	close OFH ;
	$/ = $rs ;
    
	#### Start httpd
	print "\n\nStarting httpd...       " ;
	unlink "$tmppath/httpd.pid" ;
	chmod 0666, $logfile ;
	$XX = $multhttpd?'':'-X' ;


	if ($EPWIN32)
	    {
	    $ENV{PATH} .= ";$EPHTTPDDLL" if ($EPWIN32) ;
	    $ENV{PERL_STARTUP_DONE} = 1 ;

	    Win32::Process::Create($HttpdObj, $EPHTTPD,
				   "Apache -s $XX -f $EPPATH/$httpdconf ", 0,
				   # NORMAL_PRIORITY_CLASS,
				   0,
				    ".") or die "***Cannot start $EPHTTPD" ;
	    }
	else
	    {
	    system ("$EPHTTPD $XX -f $EPPATH/$httpdconf &") and die "***Cannot start $EPHTTPD" ;
	    }
	sleep (3) ;
	if (!open FH, "$tmppath/httpd.pid")
	    {
	    sleep (7) ;
	    if (!open FH, "$tmppath/httpd.pid")
		{
		sleep (7) ;
		if (!open FH, "$tmppath/httpd.pid")
                    {
            	    open (FERR, "$httpderr") ;  
                    print $_ while (<FERR>) ;
                    close FERR ;
                    die "Cannot open $tmppath/httpd.pid" ;
		    }
                }

	    }
	$httpdpid = <FH> ;
	chop($httpdpid) ;       
	close FH ;
	print "pid = $httpdpid  ok\n" ;

	close ERR ;
	open (ERR, "$httpderr") ;  
	<ERR> ; # skip first line
	}
    elsif ($err == 0 && $EPHTTPD eq '')
	{
	print "\n\nSkiping tests for mod_perl, because Embperl is not build for it.\n" ;
	print "Embperl can still be used as CGI-script, but 'make test' cannot test it\n" ;
	print "without apache httpd installed.\n" ;
	}

    
    while ($loc ne '' && $err == 0)
	{
	if ($loc eq $embploc)
	    { print "\nTesting mod_perl mode...\n\n" ; }
	else
	    { print "\nTesting cgi mode...\n\n" ; }

	$cookie = undef ;
        $t_req = 0 ;
	$n_req = 0 ;
	$n = 0 ;
	foreach $url (@tests)
	    {
	    ($file, $query_info, $debug, $errcnt) = split (/\?/, $url) ;

	    next if ($file =~ /\// && $loc eq $cgiloc) ;        
	    #next if ($file eq 'taint.htm' && $loc eq $cgiloc) ;
	    next if ($file eq 'reqrec.htm' && $loc eq $cgiloc) ;
	    next if (($file =~ /^exit/) && $loc eq $cgiloc) ;
	    #next if ($file eq 'error.htm' && $loc eq $cgiloc && $errcnt < 16) ;
	    next if ($file eq 'varerr.htm' && $loc eq $cgiloc && $errcnt > 0) ;
	    next if ($file eq 'varerr.htm' && $looptest) ;
	    next if (($file =~ /registry/) && $loc eq $cgiloc) ;
	    next if (($file =~ /match/) && $loc eq $cgiloc) ;
	    next if ($file eq 'http.htm' && $loc eq $cgiloc) ;
	    next if ($file eq 'chdir.htm' && $EPWIN32) ;
	    next if ($file eq 'notfound.htm' && $loc eq $cgiloc && $EPWIN32) ;
	    next if ($file =~ /opmask/ && $EPSTARTUP =~ /_dso/) ;
	    if ($file =~ /sess\.htm/)
                { 
                next if ($loc eq $cgiloc) ;
                if (!$EPSESSIONVERSION)
                    {
		    $txt2 = "$file...";
		    $txt2 .= ' ' x (29 - length ($txt2)) ;
		    print "$txt2 skip on this plattform\n" ; 
                    next ;
                    }
                }
     
	    $debug ||= $defaultdebug ;  
	    $errcnt ||= 0 ;
	    $errcnt = -1 if ($EPWIN32 && $loc eq $cgiloc) ;
	    $page = "$inpath/$file" ;
	    if (!$starthttpd)
		{
		$notseen = 0 ;
		}
	    elsif ($loc eq $embploc)
		{
		$notseen = $seen{"$loc:$page"}?0:1 ;
		$seen{"$loc:$page"} = 1 ;
		$notseen = 0 if ($file eq 'registry/errpage.htm') ;
		}
	    else
		{
		$notseen = 1 ;
		}
    
	    $txt = "$file" . ($debug != $defaultdebug ?"-d $debug ":"") . '...' ;
	    $txt .= ' ' x (30 - length ($txt)) ;
	    print $txt ; 
	    unlink ($outfile) ;
	    
	    $content = undef ;
	    $content = "f1=abc1&f2=1234567890&f3=" . 'X' x 8192 if ($file eq 'post.htm') ;
	    $upload = undef ;
	    if ($file eq 'upload.htm') 
		{
		$upload = "f1=abc1\r\n&f2=1234567890&f3=" . 'X' x 8192 ;
		$content = "Hi there!" ;
		}

	    $n_req++ ;
	    $t1 = HTML::Embperl::Clock () ;
	    $m = REQ ($loc, $file, $query_info, $outfile, $content, $upload) ;
	    $t_req += HTML::Embperl::Clock () - $t1 ; 

	    if ($memcheck)
		{
		my $vmsize = GetMem ($httpdpid) ;
		$vmhttpdinitsize = $vmsize if $loopcnt == 2 ;
		print "\#$loopcnt size=$vmsize init=$vmhttpdinitsize " ;
		print "GROWN! at iteration = $loopcnt  " if ($vmsize > $vmhttpdsize) ;
		die "\n\nMemory problem (Total memory)" if ($exitonmem && $loopcnt > 2 && $vmsize > $vmhttpdsize) ;
		$vmhttpdsize = $vmsize if ($vmsize > $vmhttpdsize) ;
		CheckSVs ($loopcnt, $n) ;
		
		}
	    if (($m || '') ne 'ok' && $errcnt == 0)
		{
		$err = 1 ;
		print "ERR:$m\n" ;
		last ;
		}

	    $err = CheckError ($errcnt) if (($err == 0 || $file eq 'notfound.htm') && $checkerr ) ;
	    if ($err == 0 && $file ne 'notfound.htm')
		{
		$page =~ /.*\/(.*)$/ ;
		$org = "$cmppath/$1" ;

		#print "Compare $page with $org\n" ;
		$err = CmpFiles ($outfile, $org) ;
		}

	    print "ok\n" unless ($err) ;
	    $err = 0 if ($ignoreerror) ;
	    last if ($err) ;
	    $n++ ;
	    }

	if ($loc ne $cgiloc)   
	    { 
	    $t_mp = $t_req ;
	    $n_mp = $n_req ;
	    }
	else
	    {
	    $t_cgi = $t_req ;
	    $n_cgi = $n_req ;
	    }

	if ($testtype =~ /c/ && $err == 0 && $loc ne $cgiloc && $loopcnt == 0)   
	    { 
	    $loc = $cgiloc ;
	    }
	else
	    {
	    $loc = '' ;
	    }
	}

    if ($defaultdebug == 0)
	{
	print "\n" ;
	print "Offline:  $n_offline tests takes $t_offline sec = ", int($t_offline / $n_offline * 1000) / 1000.0, " sec per test\n" if ($t_offline) ;
	print "mod_perl: $n_mp tests takes $t_mp sec = ", int($t_mp / $n_mp * 1000) / 1000.0 , " sec per test\n"  if ($t_mp) ;
	print "CGI:      $n_cgi tests takes $t_cgi sec = ", int($t_cgi / $n_cgi * 1000) / 1000.0 , " sec per test\n"  if ($t_cgi) ;
	}

    $loopcnt++ ;
    }
until ($looptest == 0 || $err != 0)     ;


if ($err)
    {
    $page ||= '???' ;
    $org  ||= '???' ;
    print "Input:\t\t$page\n" ;
    print "Output:\t\t$outfile\n" ;
    print "Compared to:\t$org\n" ;
    print "Log:\t\t$logfile\n" ;
    print "\n ERRORS detected! NOT all test have been passed successfully\n\n" ;
    }
else
    {
    print "\nAll test have been passed successfully!\n\n" ;
    }

if (defined ($line = <ERR>))
	{
	print "\nFound unexpected output in httpd errorlog:\n" ;
	print $line ;
	}
while (defined ($line = <ERR>))
	{ print $line ; }
close ERR ;
		
$fatal = 0 ;


if ($EPWIN32)
    {
    $HttpdObj->Kill(-1) if ($HttpdObj) ;
    }
else
    {
    system "kill `cat $tmppath/httpd.pid`  2> /dev/null" if ($EPHTTPD ne '' && $killhttpd) ;
    }

exit ($err) ;
