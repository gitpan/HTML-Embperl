BEGIN { use ExtUtils::testlib ; }

use Apache ;
use HTML::Embperl ;

$cp = HTML::Embperl::AddCompartment ('TEST') ;

$cp -> deny (':base_loop') ;

