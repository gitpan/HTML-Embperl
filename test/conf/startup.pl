BEGIN { 
    use lib qw{ . } ;
    use ExtUtils::testlib ;
    
    if ($EPSESSIONCLASS = $ENV{EMBPERL_SESSION_CLASS})
        {
        eval " use Apache\:\:Session\:\:$EPSESSIONCLASS; " ;
        die $@ if ($@) ;
        }
    }

use Apache ;
use Apache::Registry ;
use HTML::Embperl ;

$cp = HTML::Embperl::AddCompartment ('TEST') ;

$cp -> deny (':base_loop') ;

1 ;
