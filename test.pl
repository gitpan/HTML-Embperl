#!/usr/bin/perl --
# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl test.pl'


$confpath = './test/conf' ;

do "$confpath/config.pl" ;


$httpdconfsrc = "$confpath/httpd.conf.src" ;
$httpdconf = "$confpath/httpd.conf" ;

$inpath  = './test/html' ;
$tmppath = './test/tmp' ;
$cmppath = './test/cmp' ;
$embploc = '/embperl/' ;
$cgiloc  = '/cgi-bin/' ;

$port    = $EPPORT ;
$host    = 'localhost' ;
$httpdpid = 0 ;

if ($ARGV[0] =~ /\?/)
    {
    print "\n\n" ;
    print "o	test offline\n" ;
    print "c	test cgi\n" ;
    print "h	test mod_perl\n" ;
    print "r	don't kill httpd at end of test\n" ;
    print "l	loop forever\n" ;
    print "m	start httpd with mulitple childs\n" ;
    print "v    memory check\n" ;
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

@tests = (
	'ascii',
	'pure.htm',
	  'plain.htm',
    	  'tagscan.htm',
	  'tagscan.htm??1',
          'if.htm',
          'while.htm?erstes=Hallo&zweites=Leer+zeichen&drittes=%21%22%23&erstes=Wert2',
 	      'table.htm',
 	      'table.htm??1',
 	      'table.htm??32769',
	      'lists.htm?sel=2&SEL1=B&SEL3=D&SEL4=cc',
 	      'input.htm?feld5=Wert5&feld6=Wert6&feld7=Wert7&feld8=Wert8&cb5=cbv5&cb6=cbv6&cb7=cbv7&cb8=cbv8&cb9=ncbv9&cb10=ncbv10&cb11=ncbv11',
	      'java.htm',
	      'inputjava.htm',
         ) ;
# 	  'taint.htm' ) ;

sub CmpFiles 
    {
    my ($f1, $f2) = @_ ;
    my $line = 1 ;
    my $err  = 0 ;

    open F1, $f1 || die "***Cannot open $f1" ; 
    open F2, $f2 || die "***Cannot open $f2" ; 

    while ($l1 = <F1>)
        {
        $l2 = <F2> ;
        if ($l1 ne $l2)
            {
            print "\nError in Line $line\nIs:\t$l1\Should:\t$l2\n" ;
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
    	

    my $ua = new LWP::UserAgent;    # create a useragent to test

    my($request,$response,$url);

    $url = new URI::URL("http://$host:$port/$loc$file?$query");

    $request = new HTTP::Request('GET', $url);

    #print "GET $url\n\n";

    $response = $ua->request($request, undef, undef);

    return $response -> message if (!$response->is_success) ;

    open FH, ">$ofile" ;
    print FH $response -> content ;
    close FH ;
    
    return "ok" ;
    }

###########################################################################
#
# Get Memory from /proc filesystem

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


######################### We start with some black magic to print on failure.


#use Config qw (myconfig);
#print myconfig () ;



BEGIN { $| = 1;
        $SIG{__DIE__} = sub { 
            return unless $_[0] =~ /^\*\*\*/ ;
            return unless $killhttpd ;
            system "kill `cat $tmppath/httpd.pid`" if ($EPHTTPD ne '') ;
            } ;

        print "\nloading...              ";
      }


use HTML::Embperl;

$loaded = 1;

print "ok\n";


$um = umask 0 ;
mkdir $tmppath, 0777 ;
chmod $tmppath, 0777 ;
umask $um ;

unlink ("$tmppath/test.log") ;
unlink ("$tmppath/out.htm") ;
unlink ("$tmppath/httpd.err.log") ;

-w $tmppath or die "***Cannot write to $tmppath" ;

$dbgbreak = 0 ;
if ($ARGV[0] eq 'dbgbreak')
	{
	$dbgbreak = 1 ;
	shift @ARGV ;
	}
	
if ($EPHTTPD ne '')
    { $testtype = $ARGV[0] || 'ohc' ; }
else
    { $testtype = $ARGV[0] || 'o' ; }

$killhttpd = 0 if ($ARGV[0] =~/r/) ;
$multhttpd = 1 if ($ARGV[0] =~/m/) ;
$looptest  = 1 if ($ARGV[0] =~/l/) ;
$memcheck  = 1 if ($ARGV[0] =~/v/) ;

shift @ARGV ;

if ($#ARGV >= 0)
    {
    @tests = @ARGV ;
    }
    
    
$err = 0 ;
$loopcnt = 0 ;
	
do  {
#############
#
#  OFFLINE
#
#############

if ($testtype =~ /o/)
  {
  print "\nTesting offline mode...\n\n" ;
  foreach $url (@tests)
    {
    ($file, $query_info, $debug) = split (/\?/, $url) ;
    $debug = 32765 if ($debug eq '') ;	
    $page = "$inpath/$file" ;
    @testargs = ( '-o', "$tmppath/out.htm" ,
                  '-l', "$tmppath/test.log",
                  '-d', $debug,
                   $page, $query_info) ;
    unshift (@testargs, 'dbgbreak') if ($dbgbreak) ;
    
    $txt = $file . ($debug != 32765 ?"-d $debug ":"") ;
    formline ('@<<<<<<<<<<<<<<<<<<<<... ', $txt) ;
    print $^A ;	
    $^A = '' ;
    
    
    $err = HTML::Embperl::run (@testargs) ;
    if ($memcheck)
    	{
    	my $vmsize = GetMem ($$) ;
    	$vminitsize = $vmsize if $loopcnt == 2 ;
    	print "\#$loopcnt size=$vmsize init=$vminitsize " ;
    	print "GROWN! at iteration = $loopcnt  " if ($vmsize > $vmmaxsize) ;
    	$vmmaxsize = $vmsize if ($vmsize > $vmmaxsize) ;
    	}
    	

    if ($err == 0)
        {
        $page =~ /.*\/(.*)$/ ;
        $org = "$cmppath/$1" ;

        $err = CmpFiles ("$tmppath/out.htm", $org) ;
        }

	print "ok\n" unless ($err) ;
	last if $err ;
	}
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


if ($loc ne '' && $err == 0 && $loopcnt == 0)
    {
    #### Configure httpd conf file
    my $cf ;
    my $rs = $/ ;
    undef $/ ;

    open IFH, "$httpdconfsrc" or die "***Cannot open $httpconfsrc" ;
    $cf = <IFH> ;
    close IFH ;
    open OFH, ">$httpdconf" or die "***Cannot open $httpconf" ;
    eval "print OFH \"$cf\"" ;
    close OFH ;
    $/ = $rs ;
    
    #### Start httpd
    print "\n\nStarting httpd...       " ;
    chmod 0666, "$tmppath/test.log" ;
    $XX = $multhttpd?'':'-X' ;
    system ("$EPHTTPD $XX -f $EPPATH/$httpdconf &") and die "***Cannot start $EPHTTPD" ;
    sleep (3) ;
    open FH, "$tmppath/httpd.pid" or die "Cannot open $tmppath/httpd.pid" ;
    $httpdpid = <FH> ;
    chop($httpdpid) ;	
    close FH ;
    print "pid = $httpdpid  ok\n" ;
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
    ($file, $query_info, $debug) = split (/\?/, $url) ;
	
    $debug = 32765 if ($debug eq '') ;	
    $page = "$inpath/$file" ;
    $txt = "$file" . ($debug != 32765 ?"-d $debug ":"") ;
    formline ('@<<<<<<<<<<<<<<<<<<<<... ', $txt) ;
    print $^A ;	
    $^A = '' ;
    $m = GET ($loc, $file, $query_info, "$tmppath/out.htm") ;
    if ($memcheck)
    	{
    	my $vmsize = GetMem ($httpdpid) ;
    	$vmhttpdinitsize = $vmsize if $loopcnt == 2 ;
    	print "\#$loopcnt size=$vmsize init=$vmhttpdinitsize " ;
    	print "GROWN! at iteration = $loopcnt  " if ($vmsize > $vmhttpdsize) ;
    	$vmhttpdsize = $vmsize if ($vmsize > $vmhttpdsize) ;
    	}
    if ($m ne 'ok')
    	{
    	$err = 1 ;
    	print "ERR:$m\n" ;
    	last ;
    	}
    
    if ($err == 0)
        {
        $page =~ /.*\/(.*)$/ ;
        $org = "$cmppath/$1" ;

        #print "Compare $page with $org\n" ;
        $err = CmpFiles ("$tmppath/out.htm", $org) ;
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
until ($looptest == 0 || $err != 0)	;

if ($err)
    {
    print "Input:\t\t$page\n" ;
    print "Output:\t\t$tmppath/out.htm\n" ;
    print "Compared to:\t$org\n" ;
    print "Log:\t\t$tmppath/test.log\n" ;
    print "\n ERRORS detected! NOT all test have been passed successfully\n\n" ;
    }
else
    {
    print "\nAll test have been passed successfully!\n\n" ;
    }

system "kill `cat $tmppath/httpd.pid`" if ($EPHTTPD ne '' && $killhttpd) ;

