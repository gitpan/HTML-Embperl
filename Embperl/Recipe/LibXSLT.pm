
###################################################################################
#
#   Embperl - Copyright (c) 1997-2001 Gerald Richter / ECOS
#
#   You may distribute under the terms of either the GNU General Public
#   License or the Artistic License, as specified in the Perl README file.
#
#   THIS PACKAGE IS PROVIDED 'AS IS' AND WITHOUT ANY EXPRESS OR
#   IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
#   WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
#
#   $Id: LibXSLT.pm,v 1.1.2.1 2001/11/16 11:55:32 richter Exp $
#
###################################################################################
 


package HTML::Embperl::Recipe::LibXSLT ;

use strict ;
use vars qw{@ISA} ;

use HTML::Embperl::Recipe::XSLT ;

@ISA = ('HTML::Embperl::Recipe::XSLT') ;

# ---------------------------------------------------------------------------------
#
#   Create a new recipe by converting request parameter
#
# ---------------------------------------------------------------------------------


sub new

    {
    my ($class, $r, $recipe, $param) = @_ ;

    $param -> {xsltproc} = 'libxslt' ;
    return  HTML::Embperl::Recipe::XSLT -> new ($r, $recipe, $param) ;
    }


