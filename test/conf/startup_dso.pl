BEGIN { 
    use lib qw{ . } ;
    use ExtUtils::testlib ;
    
    if ($EPSESSIONCLASS = $ENV{EMBPERL_SESSION_CLASS})
        {
        eval " use Apache\:\:Session\:\:$EPSESSIONCLASS; " ;
        die $@ if ($@) ;
        }
    }



1 ;
