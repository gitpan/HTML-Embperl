@rem = '--*-Perl-*--
@echo off
/usr/bin/perl -x -T %0 %1 %2 %3 %4 %5 %6 %7 %8 %9
goto endofperl
@rem ';
#!/usr/bin/perl --
#line 8
###################################################################################
#
#   Embperl - Copyright (c) 1997-1999 Gerald Richter / ECOS
#
#   You may distribute under the terms of either the GNU General Public
#   License or the Artistic License, as specified in the Perl README file.
#   For use with Apache httpd and mod_perl, see also Apache copyright.
#
#   THIS PACKAGE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR
#   IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
#   WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
#
###################################################################################

BEGIN 
    {
    use ExtUtils::testlib ;
    }	

use HTML::Embperl;

$^W = 1;

my $rc = HTML::Embperl::runcgi ;

print "Status: $rc\n\n" if ($rc) ;


__END__
:endofperl
