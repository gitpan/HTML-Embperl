#!/usr/bin/perl --
# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl test.pl'


$inpath  = './test/html' ;
$tmppath = './test/tmp' ;
$cmppath = './test/cmp' ;


@tests = ('plain.htm',
	  'tagscan.htm',
	  'tagscan.htm??0',
          'if.htm',
          'while.htm?erstes=Hallo&zweites=Leer+zeichen&drittes=%21%22%23&erstes=Wert2',
 	  'table.htm',
	  'lists.htm?sel=2',
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

    open F1, $f1 || die "Cannot open $f1" ; 
    open F2, $f2 || die "Cannot open $f2" ; 

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




######################### We start with some black magic to print on failure.


use Config qw (myconfig);
#print myconfig () ;



BEGIN { $| = 1; print "\nloading...\t"; }
###END {print "not ok 1\n" unless $loaded;}
use HTML::Embperl;
$loaded = 1;
print "ok\n";


mkdir $tmppath, 0755 ;

unlink ("$tmppath/test.log") ;
unlink ("$tmppath/out.htm") ;

-w $tmppath or die "Cannot write to $tmppath" ;

foreach $url (@tests)
    {
    ($file, $query_info, $debug) = split (/\?/, $url) ;
    #print "f=$file q=$query_info d=$debug\n" ;
    $debug = 65533 if ($debug eq '') ;	

    $page = "$inpath/$file" ;
    @testargs = ( '-o', "$tmppath/out.htm" ,
                  '-l', "$tmppath/test.log",
                  '-d', $debug,
                   $page, $query_info) ;
    
    #print "$testargs[6] -> $testargs[1], Log = $testargs[3]\n" ;
    print "$file", $debug != 65533 ?"-d $debug ":"", "...\t" ;
    $err = HTML::Embperl::run (@testargs) ;

    if ($err == 0)
        {
        $page =~ /.*\/(.*)$/ ;
        $org = "$cmppath/$1" ;

        #print "Compare $page with $org\n" ;
        $err = CmpFiles ("$tmppath/out.htm", $org) ;
        }

	print "ok\n" unless ($err) ;
	#print "ERROR ($err)\n" if ($err) ;
	last if $err ;

	}
	
	
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

