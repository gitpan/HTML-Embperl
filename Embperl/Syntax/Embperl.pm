
###################################################################################
#
#   Embperl - Copyright (c) 1997-2001 Gerald Richter / ECOS
#
#   You may distribute under the terms of either the GNU General Public
#   License or the Artistic License, as specified in the Perl README file.
#
#   THIS PACKAGE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR
#   IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
#   WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
#
#   $Id: Embperl.pm,v 1.1.2.4 2001/03/28 19:15:32 richter Exp $
#
###################################################################################
 


package HTML::Embperl::Syntax::Embperl ;

use HTML::Embperl::Syntax qw{:types} ;
use HTML::Embperl::Syntax::EmbperlHTML ;
use HTML::Embperl::Syntax::EmbperlBlocks ;

@ISA = qw(HTML::Embperl::Syntax::EmbperlBlocks HTML::Embperl::Syntax::EmbperlHTML) ;


###################################################################################
#
#   Methods
#
###################################################################################

# ---------------------------------------------------------------------------------
#
#   Create new Syntax Object
#
# ---------------------------------------------------------------------------------

sub new

    {
    my $class = shift ;
    
    my $self = HTML::Embperl::Syntax::EmbperlBlocks::new ($class) ;
    HTML::Embperl::Syntax::EmbperlHTML::new ($self) ;

    return $self ;
    }


1 ;
__END__

=pod

=head1 NAME

Embperl syntax module for Embperl. 

=head1 SYNOPSIS

[$ syntax Embperl $]

=head1 DESCRIPTION

This module provides the default syntax for Embperl and include all defintions
from EmbperlHTML and EmbperlBlocks.

=head1 Author

Gerald Richter <richter@dev.ecos.de>

=head1 See Also

HTML::Embperl::Syntax, HTML::Embperl::Syntax::EmbperlHTML, HTML::Embperl::Syntax::EmbperlBlocks

