
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
#   $Id: EmbperlHTML.pm,v 1.1.2.8 2001/10/25 08:26:04 richter Exp $
#
###################################################################################
 


package HTML::Embperl::Syntax::EmbperlHTML ;

use HTML::Embperl::Syntax qw{:types} ;
use HTML::Embperl::Syntax::HTML ;

use strict ;
use vars qw{@ISA} ;

@ISA = qw(HTML::Embperl::Syntax::HTML) ;


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

    my $self = HTML::Embperl::Syntax::HTML::new ($class) ;

    Init ($self) ;

    return $self ;
    }



###################################################################################
#
#   Definitions for Embperl HTML tags
#
###################################################################################

sub Init

    {
    my ($self) = @_ ;

    $self -> AddTag ('input', ['type', 'name', 'value'], ['src'], ['checked'], 
                { 
                perlcode =>
                    [ 
                    'HTML::Embperl::Cmd::InputCheck (%$n%, %&*\'name%, %&*\'value%, %&\'checked%) ;  %&=-type:radio|checkbox% ',
                    '$idat{%&*\'name%}=$fdat{%&*\'name%} ; _ep_sa(%$n%, \'value\', $fdat{%&*\'name%} || \'\') ;   %&!-value%',
                    '$idat{%&*\'name%}=%&*\'value% ; ',
                    ]
                }) ;
    $self -> AddTagBlock ('textarea', ['name'], undef, undef,
                { 
                perlcode =>
                    [ 
                    '$idat{%&*\'name%}=$fdat{%&*\'name%};_ep_ac(%$n%, HTML::Embperl::Syntax::ntypText, $fdat{%&*\'name%}) ;   %#!-0%',
                    '$idat{%&*\'name%}=\'%#*0%\' ; ',
                    ]
                }) ;

    $self -> AddTagBlock ('tr', undef, undef, undef, 
                { 
                perlcode    => 'l%$p%: for (my $col = 0; $col < $maxcol; $col++) {' ,
                perlcodeend => '} %?*-htmlrow%' ,
                perlcoderemove => 1,
                stackname   => 'htmlrow',
                'push'        => '%$p%',
                mayjump     => 1,
                }) ;

    my %ProcInfoTable = (
                perlcode    => 'l%$p%: for (my $row = 0; $row < $maxrow; $row++) {' ,
                perlcodeend =>  '} %?*-htmltable%' ,
                perlcoderemove => 1,
                stackname   => 'htmltable',
                'push'        => '%$p%',
                mayjump     => 1,
            ) ;

    $self -> AddTagBlock ('table',  undef, undef, undef, \%ProcInfoTable) ;
    $self -> AddTagBlock ('ol',     undef, undef, undef, \%ProcInfoTable) ;
    $self -> AddTagBlock ('ul',     undef, undef, undef, \%ProcInfoTable) ;
    $self -> AddTagBlock ('dl',     undef, undef, undef, \%ProcInfoTable) ;
    $self -> AddTagBlock ('menu',   undef, undef, undef, \%ProcInfoTable) ;
    $self -> AddTagBlock ('dir',    undef, undef, undef, \%ProcInfoTable) ;


    $self -> AddTagBlock ('select', ['name'], undef, undef,
                { 
                stackname2   => 'htmlselect',
                push2        => '%&*\'name%',

                perlcode    => 'l%$p%: for (my $row = 0; $row < $maxrow; $row++) {' ,
                perlcodeend =>  '} %?*-htmltable%' ,
                perlcoderemove => 1,
                stackname   => 'htmltable',
                'push'        => '%$p%',
                mayjump     => 1,
                }) ;
    # option tag are _not_ added as block, to allow <option> without </option>
    # which are interpreted correct by most browsers
    $self -> AddTag ('option', ['value'], undef, ['selected'],
                { 
                perlcode =>
                    [ 
                    '_ep_opt (%$n%, %^*htmlselect%, %&*\'value%, %&\'selected%);',
                    '_ep_opt (%$n%, %^*htmlselect%, %>*\'1%, %&\'selected%);',
                    ]
                }) ;
    $self -> AddTagWithStart ('/option', 'option') ;

    $self -> AddTag ('a', undef, ['href'], undef, undef) ;
    $self -> AddTag ('frame', undef, ['src'], undef, undef) ; 
    $self -> AddTag ('iframe', undef, ['src'], undef, undef) ; 
    $self -> AddTag ('embed', undef, ['src'], undef, undef) ; 
    $self -> AddTag ('layer', undef, ['src'], undef, undef) ; 
    $self -> AddTag ('img', undef, ['src'], undef, undef) ; 
    $self -> AddTag ('form', undef, ['action'], undef, undef) ; 
    
    }






1;



__END__

=pod

=head1 NAME

HTML::Embperl::Syntax::EmbperlHTML

=head1 SYNOPSIS


=head1 DESCRIPTION

Class derived from HTML::Embperl::Syntax::HTML to define the syntax for 
HTML tags that Embperl processes.

=head1 Methods

I<HTML::Embperl::Syntax::EmbperlHTML> does not defines any methods.

=head1 Author

G. Richter (richter@dev.ecos.de)

=head1 See Also

HTML::Embperl::Syntax, HTML::Embperl::Syntax::HTML


