
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
#   $Id: EmbperlLibXSLT.pm,v 1.1.2.3 2001/11/27 08:37:56 richter Exp $
#
###################################################################################
 


package HTML::Embperl::Recipe::EmbperlLibXSLT ;

use strict ;
use vars qw{@ISA} ;

use HTML::Embperl::Recipe::EmbperlXSLT ;

@ISA = ('HTML::Embperl::Recipe::EmbperlXSLT') ;

# ---------------------------------------------------------------------------------
#
#   Create a new recipe by converting request parameter
#
# ---------------------------------------------------------------------------------


sub new

    {
    my ($class, $r, $recipe, $param) = @_ ;

    $param -> {xsltproc} = 'libxslt' ;
    return  HTML::Embperl::Recipe::EmbperlXSLT -> new ($r, $recipe, $param) ;
    }


1 ;