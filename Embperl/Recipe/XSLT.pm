
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
#   $Id: XSLT.pm,v 1.1.2.1 2001/11/16 11:55:32 richter Exp $
#
###################################################################################
 


package HTML::Embperl::Recipe::XSLT ;

use strict ;
use vars qw{@ISA} ;

use HTML::Embperl::Recipe ;

@ISA = ('HTML::Embperl::Recipe') ;

# ---------------------------------------------------------------------------------
#
#   Create a new recipe by converting request parameter
#
# ---------------------------------------------------------------------------------


sub new

    {
    my ($class, $r, $recipe, $param) = @_ ;

    my $self ;
    my $src ;
    my $file ;
    my $xsltproc = $param -> {xsltproc} ;

    if (!$param -> {inputfile} && $param->{sub}) 
        {
        ($file) = $r -> Sourcefile =~ /.*\/(.*?)$/ ;
        }
    else
        {
        $file = $r -> Sourcefile ;
        }

    if (ref $param -> {input})
        {
        $src = 
            {
            'type'   =>  'memory',
            'name'   => $param -> {inputfile},
            'source' => $param -> {input},
            'mtime'  => $param -> {mtime}, 
            } ;
        }
    else
        {
        $src = 
            {
            'type'      =>  'file',
            'filename'  => $file,
            'cache'     => 0,
            } ;
        }


    my $self =
        {
        'provider' => 
            {
            'type' => $xsltproc,
            'source' => 
                {
                'cache'    => 0,
                provider =>
                    {
                    'type'      =>  $xsltproc . '-parse-xml',
                    'source' =>
                        {
                       'cache' => 0,
                        provider => $src,
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

