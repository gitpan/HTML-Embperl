@rem = '--*-Perl-*--
@echo off
/usr/bin/perl5.00404 -x %0 %1 %2 %3 %4 %5 %6 %7 %8 %9
goto endofperl
@rem ';
#!/usr/bin/perl5.00404 --
#line 8
###################################################################################
#
#   Embperl - Copyright (c) 1997-1998 Gerald Richter / ECOS
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

HTML::Embperl::run (@ARGV) ;


__END__
:endofperl
