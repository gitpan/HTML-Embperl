#!/usr/bin/perl --
# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl test.pl'


$inpath  = './test/html' ;
$tmppath = './test/tmp' ;
$cmppath = './test/cmp' ;


@tests = ('plain.htm', 'if.htm', 'while.htm', 'table.htm', 'taint.htm' ) ;

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
            print "Error in Line $line\n Is:     $l1\nShould: $l2\n" ;
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



###BEGIN { $| = 1; print "1..1\n"; }
###END {print "not ok 1\n" unless $loaded;}
use HTML::Embperl;
$loaded = 1;
print "ok loaded\n";


mkdir $tmppath, 0755 ;

unlink ("$tmppath/test.log") ;
unlink ("$tmppath/out.htm") ;

-w $tmppath or die "Cannot write to $tmppath" ;

foreach $file (@tests)
	{
    $page = "$inpath/$file" ;
    @testargs = ( '-o', "$tmppath/out.htm" ,
                  '-l', "$tmppath/test.log",
                  '-d', '65535',
                   $page, 'erstes=Hallo&zweites=Leer+zeichen&drittes=%21%22%23') ;
    
    print "$testargs[6] -> $testargs[1], Log = $testargs[3]\n" ;
    $err = HTML::Embperl::run (@testargs) ;

    if ($err == 0)
        {
        $page =~ /.*\/(.*)$/ ;
        $org = "$cmppath/$1" ;

        print "Compare $page with $org\n" ;
        $err = CmpFiles ("$tmppath/out.htm", $org) ;
        }

	print "$page ok\n" unless ($err) ;
	print "$page err ($err)\n" if ($err) ;
	last if $err ;

	}
	
	
if ($err)
    {
    print "\n ERRORS detected! NOT all test have been passed successfully\n\n" ;
    }
else
    {
    print "\nAll test have been passed successfully!\n\n" ;
    }
