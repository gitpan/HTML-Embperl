BEGIN { 
    use lib qw{ . } ;
    use ExtUtils::testlib ;
    
    if ($EPSESSIONCLASS = $ENV{EMBPERL_SESSION_CLASS})
        {
        eval " use Apache\:\:Session; " ;
        die $@ if ($@) ;
        eval " use Apache\:\:Session\:\:$EPSESSIONCLASS; " ;
        }
    } ;

sub main::trans { return -1 } ;

use Apache ;
use Apache::Registry ;
use HTML::Embperl ;

$testshare = "Shared Data" ; 

$cp = HTML::Embperl::AddCompartment ('TEST') ;

$cp -> deny (':base_loop') ;

$cp -> share ('$testshare') ;



1 ;
