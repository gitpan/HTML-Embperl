#!/usr/bin/perl --
# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl test.pl'

# avoid some warnings:

use vars qw ($httpconfsrc $httpconf $EPPORT *SAVEERR *ERR $EPHTTPDDLL) ;

    {
    local $^W = 0 ;
    eval " use Win32::Process; " ;
    $win32loaderr = $@ ;
    eval " use Win32; " ;
    $win32loaderr ||= $@ ;
    }

BEGIN { $fatal = 1 ; $^W = 1 ; }
END   { print "\nTest terminated with fatal error\n" if ($fatal) ; }

@tests = (
    'ascii',
    'pure.htm',
    'plain.htm',
    'plain.htm',
    'plain.htm',
    'error.htm???7',
    'error.htm???7',
    'error.htm???7',
    'noerr/noerrpage.htm???7?2',
    'rawinput/rawinput.htm????16',
    'var.htm',
    'varerr.htm???-1',
    'varerr.htm???2',
    'escape.htm',
    'tagscan.htm',
    'tagscan.htm??1',
    'if.htm',
    'while.htm?erstes=Hallo&zweites=Leer+zeichen&drittes=%21%22%23&erstes=Wert2',
    'table.htm',
    'table.htm??1',
    'table.htm??32769',
#   'table.htm??131085',
    'lists.htm?sel=2&SEL1=B&SEL3=D&SEL4=cc',
    'object.htm???2',
    'discard.htm???12',
    'input.htm?feld5=Wert5&feld6=Wert6&feld7=Wert7&feld8=Wert8&cb5=cbv5&cb6=cbv6&cb7=cbv7&cb8=cbv8&cb9=ncbv9&cb10=ncbv10&cb11=ncbv11&mult=Wert3&mult=Wert6',
    'hidden.htm?feld1=Wert1&feld2=Wert2&feld3=Wert3&feld4=Wert4',
    'java.htm',
    'inputjava.htm',
    'reqrec.htm',
    'reqrec.htm',
    'registry/Execute.htm',
    'registry/errpage.htm???14',
    'nph/div.htm????64',
    'nph/npherr.htm???7?64',
    'sub.htm',
    'chdir.htm?a=1&b=2&c=&d=&f=5&g&h=7&=8&=',
    'allform/allform.htm?a=1&b=2&c=&d=&f=5&g&h=7&=8&=???8192',
    'stdout/stdout.htm????16384',
    'nochdir/nochdir.htm?a=1&b=2???384',
    'http.htm',
    'div.htm',
    'taint.htm???1',
    'safe/safe.htm???-1?4',
    'safe/safe.htm???-1?4',
    'safe/safe.htm???-1?4',
    'opmask/opmask.htm???-1?12?TEST',
    'opmask/opmasktrap.htm???2?12?TEST',
    ) ;



$confpath = './test/conf' ;

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


die "You must install libwin32 first" if ($EPWIN32 && $win32loaderr) ;

#### setup paths #####

$inpath  = './test/html' ;
$tmppath = './test/tmp' ;
$cmppath = './test/cmp' ;


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
$defaultdebug = 0x85ffd ;


if ($cmdarg =~ /\?/)
    {
    print "\n\n" ;
    print "o	test offline\n" ;
    print "c	test cgi\n" ;
    print "h	test mod_perl\n" ;
    print "e	test execute\n" ;
    print "r	don't kill httpd at end of test\n" ;
    print "l	loop forever\n" ;
    print "m	start httpd with mulitple childs\n" ;
    print "v    memory check\n" ;
    print "f    file to use for config.pl\n" ;
    print "x    do not start httpd\n" ;
    print "u    use unique filenames\n" ;
    print "n    do not check httpd errorlog\n" ;
    print "q    set debug to 0\n" ;
    print "\n\n" ;
    print "path\t$EPPATH\n" ;
    print "httpd\t$EPHTTPD\n" ;
    print "port\t$port\n" ;
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
    my ($f1, $f2) = @_ ;
    my $line = 1 ;
    my $err  = 0 ;

    open F1, $f1 || die "***Cannot open $f1" ; 
    open F2, $f2 || die "***Cannot open $f2" ; 

    while (defined ($l1 = <F1>))
	{
	chompcr ($l1) ;
	$l2 = <F2> ;
	chompcr ($l2) ;
	if (!defined ($l2))
	    {
	    print "\nError in Line $line\nIs:\t$l1\nShould:\t<EOF>\n" ;
	    return $line ;
	    }

	
	$eq = 0 ;
	while (!$notseen && ($l2 =~ /^\^\^(.*?)$/) && !$eq)
	    {
	    $l2 = $1 ;
	    $eq = $l1 =~ /$l2/ ;
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

    close F1 ;
    close F2 ;

    return $err ; 
    }

#########################
#
# GET via HTTP.
#

sub GET

    {
    my ($loc, $file, $query, $ofile) = @_ ;
	
    eval 'require LWP::UserAgent' ;
    

    if ($@)
	{
	return "LWP not installed\n" ;
	}
    $query ||= '' ;     
	
    my $ua = new LWP::UserAgent;    # create a useragent to test

    my($request,$response,$url);

    $url = new URI::URL("http://$host:$port/$loc$file?$query");

    $request = new HTTP::Request('GET', $url);

    #print "GET $url\n\n";

    $response = $ua->request($request, undef, undef);

    open FH, ">$ofile" ;
    print FH $response -> content ;
    close FH ;

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



######################### We start with some black magic to print on failure.


#use Config qw (myconfig);
#print myconfig () ;

#### install handler which kill httpd when terminating ####

BEGIN { $| = 1;
	$SIG{__DIE__} = sub { 
	    return unless $_[0] =~ /^\*\*\*/ ;
	    return unless $killhttpd ;
	    if ($EPWIN32)
		{
		$HttpdObj->Kill(-1) if ($HttpdObj) ;
		}
	    else
		{
		system "kill `cat $tmppath/httpd.pid`" if ($EPHTTPD ne '') ;
		}
	    } ;

	print "\nloading...              ";
      }

##################


use HTML::Embperl;

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
$outfile .= ".$$" if ($cmdarg =~/u/) ;
$defaultdebug = 0 if ($cmdarg =~/q/) ;


if ($#ARGV >= 0)
    {
    @tests = @ARGV ;
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

	open (SAVEERR, ">&STDERR")  || die "Cannot save stderr" ;  
	open (STDERR, ">$offlineerr") || die "Cannot redirect stderr" ;  
	open (ERR, "$offlineerr")  || die "Cannot open redirected stderr ($offlineerr)" ;  ;  
	foreach $url (@tests)
	    {
	    ($file, $query_info, $debug, $errcnt, $option, $ns) = split (/\?/, $url) ;
	    next if ($file eq 'http.htm') ;
	    next if ($file eq 'taint.htm') ;
	    next if ($file eq 'reqrec.htm') ;
	    next if ($file eq 'http.htm') ;
	    next if ($file =~ /registry/) ;
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
    
	    $txt = $file . ($debug != $defaultdebug ?"-d $debug ":"") ;
	    formline ('@<<<<<<<<<<<<<<<<<<<<... ', $txt) ;
	    print $^A ; 
	    $^A = '' ;
    
    
	    unlink ($outfile) ;
	    $err = HTML::Embperl::run (@testargs) ;
	    if ($memcheck)
		{
		my $vmsize = GetMem ($$) ;
		$vminitsize = $vmsize if $loopcnt == 2 ;
		print "\#$loopcnt size=$vmsize init=$vminitsize " ;
		print "GROWN! at iteration = $loopcnt  " if ($vmsize > $vmmaxsize) ;
		$vmmaxsize = $vmsize if ($vmsize > $vmmaxsize) ;
		}
		
	    $err = CheckError ($errcnt) if ($err == 0) ;
    
	    if ($err == 0)
		{
		$page =~ /.*\/(.*)$/ ;
		$org = "$cmppath/$1" ;

		$err = CmpFiles ($outfile, $org) ;
		}

	    print "ok\n" unless ($err) ;
	    last if $err ;
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


	    formline ('@<<<<<<<<<<<<<<<<<<<<... ', "$txt from file") ;
	    print $^A ; 
	    $^A = '' ;

	    unlink ($outfile) ;
	    $err = HTML::Embperl::Execute ({'inputfile'  => $src,
					    'mtime'      => 1,
					    'outputfile' => $outfile,
					    'debug'      => $defaultdebug,
					    }) ;
		
	    $err = CheckError ($errcnt) if ($err == 0) ;
	    $err = CmpFiles ($outfile, $org)  if ($err == 0) ;
	    print "ok\n" unless ($err) ;

	    if ($err == 0)
		{
		formline ('@<<<<<<<<<<<<<<<<<<<<... ', "$txt from memory") ;
		print $^A ;     
		$^A = '' ;

		unlink ($outfile) ;
		$err = HTML::Embperl::Execute ({'input'      => \$indata,
						'inputfile'  => 'i1',
						'mtime'      => 1,
						'outputfile' => $outfile,
						'debug'      => $defaultdebug,
						}) ;
		    
		$err = CheckError ($errcnt) if ($err == 0) ;
		$err = CmpFiles ($outfile, $org)  if ($err == 0) ;
		print "ok\n" unless ($err) ;
		}

	    if ($err == 0)
		{
		formline ('@<<<<<<<<<<<<<<<<<<<<... ', "$txt to memory") ;
		print $^A ;     
		$^A = '' ;

		my $outdata ;
		unlink ($outfile) ;
		$err = HTML::Embperl::Execute ({'inputfile'  => $src,
						'mtime'      => 1,
						'output'     => \$outdata,
						'debug'      => $defaultdebug,
						}) ;
		    
		$err = CheckError ($errcnt) if ($err == 0) ;
	
		open FH, ">$outfile" or die "Cannot open $outfile ($!)" ;
		print FH $outdata ;
		close FH ;
		$err = CmpFiles ($outfile, $org)  if ($err == 0) ;
		print "ok\n" unless ($err) ;
		}

	    if ($err == 0)
		{
		formline ('@<<<<<<<<<<<<<<<<<<<<... ', "$txt from/to memory") ;
		print $^A ;     
		$^A = '' ;

		my $outdata ;
		unlink ($outfile) ;
		$err = HTML::Embperl::Execute ({'input'      => \$indata,
						'inputfile'  => $src,
						'mtime'      => 1,
						'output'     => \$outdata,
						'debug'      => $defaultdebug,
						}) ;
		    
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
		formline ('@<<<<<<<<<<<<<<<<<<<<... ', "$txt to memory") ;
		print $^A ;     
		$^A = '' ;

		my $outdata ;
		unlink ($outfile) ;
		$err = HTML::Embperl::Execute ({'inputfile'  => $src,
						'mtime'      => 1,
						'output'     => \$outdata,
						'debug'      => $defaultdebug,
						}) ;
		    
		$err = CheckError (7) if ($err == 0) ;
	
		open FH, ">$outfile" or die "Cannot open $outfile ($!)" ;
		print FH $outdata ;
		close FH ;
		$err = CmpFiles ($outfile, $org)  if ($err == 0) ;
		print "ok\n" unless ($err) ;
		}

	    HTML::Embperl::Term () ;
	    }
	}

    if (($testtype =~ /e/) || ($testtype =~ /o/))
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
	my $cf ;
	my $rs = $/ ;
	undef $/ ;

	$ENV{EMBPERL_LOG} = $logfile ;
	open IFH, $httpdconfsrc or die "***Cannot open $httpconfsrc" ;
	$cf = <IFH> ;
	close IFH ;
	open OFH, ">$httpdconf" or die "***Cannot open $httpconf" ;
	eval "print OFH \"$cf\"" ;
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

	    Win32::Process::Create($HttpdObj, $EPHTTPD,
				   "Apache  $XX -f $EPPATH/$httpdconf ", 0,
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
		open FH, "$tmppath/httpd.pid" or die "Cannot open $tmppath/httpd.pid" ;
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

	foreach $url (@tests)
	    {
	    ($file, $query_info, $debug, $errcnt) = split (/\?/, $url) ;

	    next if ($file =~ /\// && $loc eq $cgiloc) ;        
	    next if ($file eq 'taint.htm' && $loc eq $cgiloc) ;
	    next if ($file eq 'reqrec.htm' && $loc eq $cgiloc) ;
	    #next if ($file eq 'error.htm' && $loc eq $cgiloc && $errcnt < 16) ;
	    next if ($file eq 'varerr.htm' && $loc eq $cgiloc && $errcnt > 0) ;
	    next if ($file eq 'varerr.htm' && $looptest) ;
 	    next if (($file =~ /registry/) && $loc eq $cgiloc) ;
	    next if ($file eq 'http.htm' && $loc eq $cgiloc) ;
	    next if ($file eq 'chdir.htm' && $EPWIN32) ;
     
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
    
	    $txt = "$file" . ($debug != $defaultdebug ?"-d $debug ":"") ;
	    formline ('@<<<<<<<<<<<<<<<<<<<<... ', $txt) ;
	    print $^A ; 
	    $^A = '' ;
	    unlink ($outfile) ;
	    $m = GET ($loc, $file, $query_info, $outfile) ;
	    if ($memcheck)
		{
		my $vmsize = GetMem ($httpdpid) ;
		$vmhttpdinitsize = $vmsize if $loopcnt == 2 ;
		print "\#$loopcnt size=$vmsize init=$vmhttpdinitsize " ;
		print "GROWN! at iteration = $loopcnt  " if ($vmsize > $vmhttpdsize) ;
		$vmhttpdsize = $vmsize if ($vmsize > $vmhttpdsize) ;
		}
	    if (($m || '') ne 'ok' && $errcnt == 0)
		{
		$err = 1 ;
		print "ERR:$m\n" ;
		last ;
		}

	    $err = CheckError ($errcnt) if ($err == 0 && $checkerr) ;
	    if ($err == 0)
		{
		$page =~ /.*\/(.*)$/ ;
		$org = "$cmppath/$1" ;

		#print "Compare $page with $org\n" ;
		$err = CmpFiles ($outfile, $org) ;
		}

	    print "ok\n" unless ($err) ;
	    last if $err ;
	    }

	if ($testtype =~ /c/ && $err == 0 && $loc ne $cgiloc && $loopcnt == 0)   
	    { $loc = $cgiloc ; }
	else
	    { $loc = '' ; }
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
    system "kill `cat $tmppath/httpd.pid`" if ($EPHTTPD ne '' && $killhttpd) ;
    }

exit ($err) ;
