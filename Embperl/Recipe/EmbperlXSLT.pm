
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
#   $Id: EmbperlXSLT.pm,v 1.1.2.4 2001/11/27 08:37:56 richter Exp $
#
###################################################################################
 


package HTML::Embperl::Recipe::EmbperlXSLT ;

use strict ;
use vars qw{@ISA} ;

use HTML::Embperl::Recipe::Embperl ;

@ISA = ('HTML::Embperl::Recipe::Embperl') ;

# ---------------------------------------------------------------------------------
#
#   Create a new recipe by converting request parameter
#
# ---------------------------------------------------------------------------------


sub new

    {
    my ($class, $r, $recipe, $param) = @_ ;

    my $ep = HTML::Embperl::Recipe::Embperl -> new ($r, $recipe, $param) ;
    my $xsltproc = $param -> {xsltproc} ;
    my $xsltparam= $param -> {xsltparam} || \%HTML::Embperl::fdat ;

    my $self =
        {
        'provider' => 
            {
            'type' => $xsltproc,
            'param'  => $xsltparam,
            'source' => 
                {
                'cache'    => 0,
                provider =>
                    {
                    'type'      =>  $xsltproc . '-parse-xml',
                    'source' =>
                        {
                       'cache' => 0,
                        provider => 
                            {
                            type => 'eptostring',
                            source =>
                                {
                                %$ep,
                                'cache' => 0,
                                },
                            }
                        },
                    },
                },
            'stylesheet' => 
                {
                'cache'    => 1,
                provider =>
                    {
                    'type'      =>  $xsltproc . '-compile-xsl',
                    'stylesheet' =>
                        {
                        'cache'    => 0,
                        provider =>
                            {
                            'type'      =>  'file',
                            'filename'  => $param -> {xsltstylesheet},
                            }
                        },
                    },
                }
            }
        } ;

    foreach (qw{expires_in expires_func expires_filename cache_key cache_key_options cache_key_func})
        {
        $self -> {$_} = $param -> {$_} if (exists $param -> {$_}) ;
        }
    $self -> {'cache'} = $self -> {expires_in} || $self -> {expires_func} || $self -> {expires_filename}?1:0 ;


    return $self ;
    }

1 ;
