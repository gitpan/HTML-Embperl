###################################################################################
#
#   Embperl - Copyright (c) 1997-2000 Gerald Richter / ECOS
#
#   You may distribute under the terms of either the GNU General Public
#   License or the Artistic License, as specified in the Perl README file.
#
#   THIS PACKAGE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR
#   IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
#   WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
#
#   $Id$
#
###################################################################################


###################################################################################
#
#   Embperl - Copyright (c) 1997-2000 Gerald Richter / ECOS
#
#   You may distribute under the terms of either the GNU General Public
#   License or the Artistic License, as specified in the Perl README file.
#
#   THIS PACKAGE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR
#   IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
#   WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
#
#   $Id$
#
###################################################################################


package HTML::Embperl::Module ;

use HTML::Embperl ;


$VERSION = '0.01_dev-1';


# define subs

sub init
    {
    local $/ = undef ;
    my $hdl  = shift ;
    my $data = <$hdl> ;

    # compile page

    HTML::Embperl::Execute ({'inputfile' => __FILE__, 
			    'input' => \$data,
			    'mtime' => -M __FILE__ ,
			    'import' => 0,
			    'options' => HTML::Embperl::optKeepSrcInMemory,
			    'package' => __PACKAGE__}) ;


    }

# import subs

sub import

    {
    HTML::Embperl::Execute ({'inputfile' => __FILE__, 
			    'import' => 2,
			    'package' => __PACKAGE__}) ;


    1 ;
    }



1 ;
