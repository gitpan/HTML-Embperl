
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
#   $Id: Embperl.pm,v 1.1.2.8 2001/11/27 08:37:56 richter Exp $
#
###################################################################################
 


package HTML::Embperl::Recipe::Embperl ;

use strict ;
use vars qw{@ISA} ;

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
    if (!$param -> {inputfile} && $param->{sub}) 
        {
        ($file) = $r -> Sourcefile =~ /.*(?:\/|\\)(.*?)$/ ;
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

    my $import =
        {
        'provider' => 
            {
            'type' => 'epcompile',
            ($param -> {'package'}?('package' => $param -> {'package'}):()),
            'source' => 
                {
                'cache', => 0,
                'provider' => 
                    {
                    'type' => 'epparse',
                    'syntax' => $param -> {'syntax'} || 'Embperl',
                    'source' => 
                        {
                        'cache' => 0,
                        'provider' =>  $src,
                        }
                    }
                }
            }
        } ;

    if (!$r -> IsImport)
        {
        my $run =
            {
            'provider' =>
                {
                'type' => 'eprun',
                'source' => $import 
                }
            } ;                


        foreach (qw{expires_in expires_func expires_filename cache_key cache_key_options cache_key_func})
            {
            $run -> {$_} = $param -> {$_} if (exists $param -> {$_}) ;
            }
        $run -> {'cache'} = $run -> {expires_in} || $run -> {expires_func} || exists ($run -> {expires_filename})?1:0 ;
        $self = $run ;
        }
    else
        {
        $self = $import ;
        }

    return $self ;
    }

1;
