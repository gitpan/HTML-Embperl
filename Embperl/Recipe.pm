
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
#   $Id: Recipe.pm,v 1.1.2.7 2001/11/20 15:15:42 richter Exp $
#
###################################################################################
 


package HTML::Embperl::Recipe ;

use strict ;
use vars qw{@ISA @EXPORT_OK %EXPORT_TAGS %Recipes} ;


# ---------------------------------------------------------------------------------
#
#   Get/create named recipe
#
# ---------------------------------------------------------------------------------


sub GetRecipe

    {
    my ($r, $param, $name) = @_ ;

    my @names = split (/\s/, $name) ;
    @names = ('Embperl') if (!@names) ;

    foreach my $recipe (@names)
        {
        my $mod ;
        $recipe =~ /([a-zA-Z0-9_:]*)/ ;
        $recipe = $1 ;
        if (!($mod = $Recipes{$recipe})) 
            {
            $mod = ($name =~ /::/)?$recipe:'HTML::Embperl::Recipe::'. $recipe ;
            if (!defined (&{$mod . '::new'}))
                {
                eval "require $mod" ;
                if ($@) 
                    {
                    warn $@ ;
                    return undef ;
                    }
                }
            $Recipes{$recipe} = $mod ;
            }
        my $obj = $mod -> new ($r, $recipe, $param) ;
        return $obj if ($obj) ;
        }
        
    return undef ;                
    }


# ---------------------------------------------------------------------------------
#
#   Execute
#
# ---------------------------------------------------------------------------------



sub Execute

    {
    my ($self) = @_ ;

    return HTML::Embperl::Execute ({recipe => $self}) ;
    }


1;


__END__        


=pod

=head1 NAME

Embperl base class for defining custom recipes

=head1 SYNOPSIS

PerlSetEnv EMBPERL_RECIPE "XSLT Embperl"

=head1 DESCRIPTION

HTML::Embperl::Recipe provides basic features that are necessary for createing 
your own recipes.
To do so you have to create a class that provides a C<new> method which returns
a hash that contains the description what to do.

=head2 new ($class, $r, $recipe, $param)

=over 4

=item $class

The class name

=item $r

The Embperl request record object (HTML::Embperl::Req), maybe a derived
object when running under EmbperlObject.

=item $recipe

The name of the recipe

=item $param

The parameters the user passed to Execute.

=back

The function must return a hash that describes the desired action.
The hash contains a tree structure of providers. 

=head2 Providers

=over 4


=item file

read file data

Parameter:

=over 4

=item filename

Gives the file to read

=back


=item memory

get data from a scalar

Parameter:

=over 4

=item source

Gives the source as a scalar reference

=item name

Gives the name under which this item should be cache

=back


=item epparse

parse file into a Embperl tree structure

Parameter:

=over 4

=item source

Gives the source 

=item syntax

Syntax to use

=back


=item epcompile

compile Embperl tree structure

Parameter:

=over 4

=item source

Gives the source 

=back


=item eprun

execute Embperl tree structure

Parameter:

=over 4

=item source

Gives the source 

=item cache_key

See description of cacheing

=item cache_key_options

See description of cacheing

=item cache_key_func

See description of cacheing

=back


=item eptostring

convert Embperl tree structure to string

Parameter:

=over 4

=item source

Gives the source 

=back


=item libxslt-parse-xml

parse xml source for libxslt

Parameter:

=over 4

=item source

Gives the xml source 

=back


=item libxslt-compile-xsl   

parse and compile stylesheet for libxslt

Parameter:

=over 4

=item stylesheet

Gives the stylesheet source 

=back


=item libxslt

do a xsl transformation via libxslt

Parameter:

=over 4

=item source

Gives the parsed xml source 

=item stylesheet

Gives the compiled stylesheet source 

=back


=item xalan-parse-xml

parse xml source for xalan

Parameter:

=over 4

=item source

Gives the xml source 

=back



=item xalan-compile-xsl

parse and compile stylesheet for xalan

Parameter:

=over 4

=item stylesheet

Gives the stylesheet source 

=back


=item xalan

do a xsl transformation via xalan

Parameter:

=over 4

=item source

Gives the parsed xml source 

=item stylesheet

Gives the compiled stylesheet source 

=back


=back

=head2 Cache parameter

=over 4

=item expires_in

=item expires_func

=item expires_filename

=item cache

=back


=head2 Format

Heres an example that show how the hash must be build:

  sub new
    {
    my ($class, $r, $recipe, $param) = @_ ;

    my $self =
        {
        'provider' => 
            {
            'type' => 'xalan',
            'source' => 
                {
                'cache'    => 0,
                provider =>
                    {
                    'type'      =>  'xalan-parse-xml',
                    'source' =>
                        {
                       'cache' => 0,
                        provider => 
                            {
                            'type'      =>  'file',
                            'filename'  => $param -> {inputfile},
                            }
                        },
                    },
                },
            'stylesheet' => 
                {
                'cache'    => 1,
                provider =>
                    {
                    'type'      =>  'xalan-compile-xsl',
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

    return $self ;
    }

This corresponds to the following diagramm:



    +-------------------+   +--------------------+           
    + file {inputfile}  +   +file{xsltstylesheet}+           
    +-------------------+   +--------------------+           
          |                         |                         
          v                         v                         
    +-------------------+   +-------------------+           
    + xalan-parse-xml   +   + xalan-compile-xsl +           
    +-------------------+   +-------------------+           
          |                         | 
          |                         |
          |         +-----------+   |
          +-------> + xalan     + <-+
                    +-----------+

Take a look at the recipes that comes with Embperl to get more
ideas what can be done.

