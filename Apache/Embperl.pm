###################################################################################
#
#   Embperl - Copyright (c) 1997 Gerald Richter / ECOS
#
#   You may distribute under the terms of either the GNU General Public
#   License or the Artistic License, as specified in the Perl README file.
#   For use with Apache httpd and mod_perl, see also Apache copyright.
#
#   THIS IS BETA SOFTWARE!
#
#   THIS PACKAGE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR
#   IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
#   WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
#
###################################################################################


#
# Is module is only here that HTML::Embperl also shows up under module/by-module/Apache/ .
#
package Apache::Embperl; 

use HTML::Embperl ;



$VERSION = '0.01';


*handler = \&HTML::Embperl::handler ;




