@rem = '--*-Perl-*--
@echo off
/usr/bin/perl -x %0 %1 %2 %3 %4 %5 %6 %7 %8 %9
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

use HTML::Embperl;


my $rc = HTML::Embperl::runcgi ;


if ($rc)
    {
    $time = localtime ;

    print <<EOT;
Status: $rc
Content-Type: text/html

<HTML><HEAD><TITLE>Embperl Error</TITLE></HEAD>
<BODY bgcolor=\"#FFFFFF\">
<H1>embpcgi Server Error: $rc</H1>
Please contact the server administrator, $ENV{SERVER_ADMIN} and inform them of the time the error occurred, and anything you might have done that may have caused the error.<P><P>
$ENV{SERVER_SOFTWARE} HTML::Embperl $HTML::Embperl::VERSION [$time]<P>
</BODY></HTML>

EOT
    }


__END__
:endofperl
