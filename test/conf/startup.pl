BEGIN { use ExtUtils::testlib ; }

use Apache ;
use Apache::Registry ;
#use Apache::Session::Win32 ;
use HTML::Embperl ;

$cp = HTML::Embperl::AddCompartment ('TEST') ;

$cp -> deny (':base_loop') ;

1 ;
