
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
#   $Id: Perl.pm,v 1.1.2.5 2001/10/31 12:04:26 richter Exp $
#
###################################################################################
 
package HTML::Embperl::Syntax::Perl ;

use HTML::Embperl::Syntax qw{:types} ;
use HTML::Embperl::Syntax ;

use strict ;
use vars qw{@ISA} ;

@ISA = qw(HTML::Embperl::Syntax) ;


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

    my $self = HTML::Embperl::Syntax::new ($class) ;

    if (!$self -> {-perlInit})
        {
        $self -> {-perlInit} = 1 ;    
        
        $self -> AddInitCode (undef, '$_ep_node=%$x%+2; %#1% ;', undef,
                            {
                            removenode  => 32,
                            compilechilds => 0,
                            }) ;

        }


    return $self ;
    }




1; 

__END__

=pod

=head1 NAME

Perl syntax module for Embperl 

=head1 SYNOPSIS

Execute ({inputfile => 'code.pl', syntax => 'Perl'}) ;

=head1 DESCRIPTION

This syntax cause Embperl to interpret the whole file as Perl script
without any markup.

=head1 Author

Gerald Richter <richter@dev.ecos.de>


=cut


