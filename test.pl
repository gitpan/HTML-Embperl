#!/usr/bin/perl --
# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl test.pl'

######################### We start with some black magic to print on failure.

# Change 1..1 below to 1..last_test_to_print .
# (It may become useful if the test is moved to ./t subdirectory.)

###BEGIN { $| = 1; print "1..1\n"; }
###END {print "not ok 1\n" unless $loaded;}
use Embperl;
$loaded = 1;
print "ok loaded\n";

######################### End of black magic.

$Log = '/tmp/embperl.log' ;


#################################

###print "argc = $#ARGV, av0 = $ARGV[0] pt = <$ENV{PATH_TRANSLATED}>\n" ;

if (($#ARGV == 0 && $ARGV[0] eq '-D') || ($#ARGV == -1 && $ENV{PATH_TRANSLATED} eq ''))
    {
    $Log = '' ;
    $ioType = epIOProcess ;
    }
else
    {
    if ($#ARGV >= 2)
        {
        $Log = $ARGV[2] ;
        }

    if ($#ARGV >= 1)
        {
        $ENV{QUERY_STRING} = $ARGV[1] ;
        undef $ENV{CONTENT_LENGTH} ;
        }


    if ($#ARGV >= 0)
        {
        $ENV{PATH_TRANSLATED} = $ARGV[0] ;

        $ioType = epIOPerl ;
        }
    else
        {
        $ioType = epIOCGI ;
        }


    }

###printf "iotype = $ioType\n" ;
###print "argc = $#ARGV, av0 = $ARGV[0] pt = <$Embperl::env{PATH_TRANSLATED}>\n" ;

Embperl::embperl_init ($ioType, $Log) ;
Embperl::addNameSpace ('TEST') ;
do
	{
	Embperl::embperl_req  (epDbgAll, '') ;
	}
until ($ioType != epIOProcess) ;

Embperl::embperl_term () ;

