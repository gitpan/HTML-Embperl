
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


package HTML::EmbperlObject ;

require Cwd ;
require File::Basename ;

require Exporter;
require DynaLoader;

require HTML::Embperl ;

use Apache::Constants qw(&OPT_EXECCGI &DECLINED &OK &FORBIDDEN &NOT_FOUND) ;

use File::Spec ;
use File::Basename ;

use strict ;
use vars qw(
    @ISA
    $VERSION
    ) ;


@ISA = qw(Exporter DynaLoader);


$VERSION = '0.02_dev-1';


#############################################################################
#
# Normalize path into filesystem
#
#   in	$path	path to normalize
#   ret		normalized path
#


sub norm_path

    {
    return '' if (!$_[0]) ;

    my $path = File::Spec -> canonpath (shift) ;
    $path =~ s/\\/\//g ;
    $path = $1 if ($path =~ /^\s*(.*?)\s*$/) ;
    
    return $path ;
    }


#############################################################################

sub handler
    {
    my $r = shift ;
    my $filename  = $r -> filename ;
    my $mod ;
    if ($filename =~ /^(.*)__(.*?)$/)
	{
        $filename  = norm_path ($1) ;
	$mod	   = $2 ;
	$mod 	   =~ s/[^a-zA-Z0-9]/_/g ;
	}
    else
	{	
        $filename  = norm_path ($filename) ;
	$mod = '' ;
	}

    if (exists $ENV{EMBPERL_FILESMATCH} && 
                         !($filename =~ m{$ENV{EMBPERL_FILESMATCH}})) 
        {
        return &DECLINED ;
        }

    if (exists $ENV{EMBPERL_DECLINE} && 
                         ($filename =~ m{$ENV{EMBPERL_DECLINE}})) 
        {
        return &DECLINED ;
        }


    my $basename  = $ENV{EMBPERL_OBJECT_BASE} ;
    $basename     =~ s/%modifier%/$mod/ ;
    my $addpath   = $ENV{EMBPERL_OBJECT_ADDPATH}  ;
    my @addpath   = $addpath?split (/:/, $addpath):() ;
    my $directory ;
    my $rootdir   = norm_path ($r -> document_root) ;
    my $stopdir   = norm_path ($ENV{EMBPERL_OBJECT_STOPDIR}) ;
    
    if (-d $filename)
        {
        $directory = $filename ;
        }
    else
        {
        $directory = dirname ($filename) ;
        }
    
    my $searchpath  ;
    	 
    $r -> notes ('EMBPERL_orgfilename',  $filename) ;
 
    #warn "EmbperlObject Filename: $filename\n" ;
    #warn "EmbperlObject basename: $basename\n" ;
    
    my $fn ;
    my $ap ;
    my $ldir ;
	
    do
        {
        $fn = "$directory/$basename" ;
        $searchpath .= ";$directory" ; 
        #warn "EmbperlObject Check: $fn\n" ;
        if (-e $fn)
            {
            $r -> filename ($fn) ;
            $r -> notes ('EMBPERL_searchpath',  $searchpath) ;
            #warn "EmbperlObject Found: $fn\n" ;
            #warn "EmbperlObject path: $searchpath\n" ;
            return HTML::Embperl::handler ($r) ;
            }

	$ldir      = $directory ;
        $directory = dirname ($directory) ;
        }
    while ($ldir ne $rootdir && $ldir ne $stopdir && $directory ne '/' && $directory ne '.' && $directory ne $ldir) ;

    foreach $ap (@addpath)
        {
        next if (!$ap) ;
        $fn = "$ap/$basename" ;
        $searchpath .= ";$ap" ; 
        #warn "EmbperlObject Check: $fn\n" ;
        if (-e $fn)
            {
            $r -> filename ($fn) ;
            $r -> notes ('EMBPERL_searchpath',  $searchpath) ;
            #warn "EmbperlObject Found: $fn\n" ;
            #warn "EmbperlObject path: $searchpath\n" ;
            return HTML::Embperl::handler ($r) ;
            }

        }

    $r -> log_error ("EmbperlObject searched '$searchpath'" . ($addpath?" and '$addpath' ":'')) ;

    return &NOT_FOUND ;
    }


__END__


=head1 NAME

HTML::EmbperlObject - Extents HTML::Embperl for building webpages out of small objects


=head1 SYNOPSIS


    <Location /foo>
        PerlSetEnv EMBPERL_OBJECT_BASE base.htm
        PerlSetEnv EMBPERL_FILESMATCH "\.htm.?|\.epl$"
        SetHandler perl-script
        PerlHandler HTML::EmbperlObject 
        Options ExecCGI
    </Location>


=head1 DESCRIPTION

I<HTML::EmbperlObject> is basicly a I<mod_perl> handler that helps you to
build a whole page out of smaller parts. Basicly it does the following:

When a request comes in a page which name is specified by L<EMBPERL_OBJECT_BASE>, is
searched in the same directory as the requested page. If the pages isn't found, 
I<EmbperlObject> walking up the directory tree until it finds the page, or it
reaches C<DocumentRoot> or the directory specified by L<EMBPERL_OBJECT_STOPDIR>.

This page is then called as frame for building the real page. Addtionaly I<EmbperlObject>
sets the search path to contain all directories it had to walk before finding that page.

This frame page can now include other pages, using the C<HTML::Embperl::Execute> method.
Because the search path is set by I<EmbperlObject> the included files are searched in
the directories starting at the directory of the original request walking up thru the directory
which contains the base page. This means that you can have common files, like header, footer etc.
in the base directory and override them as necessary in the subdirectory.

To include the original requested file, you need to call C<Execute> with a C<'*'> as filename.


=head1 Runtime configuration

The runtime configuration is done by setting environment variables,
in your web
server's configuration file. 

=head2 EMBPERL_DECLINE

Perl regex which files should be ignored by I<EmbperlObject>

=head2 EMBPERL_FILESMATCH

Perl regex which files should be processed by I<EmbperlObject>

=head2 EMBPERL_OBJECT_BASE

Name of the base page to search for

=head2 EMBPERL_OBJECT_STOPDIR

Directory where to stop searching for the base page

=head2 EMBPERL_OBJECT_ADDPATH

Additional directories where to search for pages. Directories are
separated by C<;> (on Unix C<:> works also)


=head1 Example


With the following setup:


 <Location /foo>
    PerlSetEnv EMBPERL_OBJECT_BASE base.htm
    PerlSetEnv EMBPERL_FILESMATCH "\.htm.?|\.epl$"
    SetHandler perl-script
    PerlHandler HTML::EmbperlObject 
    Options ExecCGI
 </Location>


B<Directory Layout:>

 /foo/base.htm
 /foo/head.htm
 /foo/foot.htm
 /foo/page1.htm
 /foo/sub/head.htm
 /foo/sub/page2.htm

B</foo/base.htm:>

 <html>
 <head>
 <title>Example</title>
 </head>
 <body>
 [- Execute ('head.htm') -]
 [- Execute ('*') -]
 [- Execute ('foot.htm') -]
 </body>
 </html>

B</foo/head.htm:>

 <h1>head from foo</h1>

B</foo/sub/head.htm:>

 <h1>another head from sub</h1>

B</foo/foot.htm:>

 <hr> Footer <hr>


B</foo/page1.htm:>

 PAGE 1

B</foo/sub/page2.htm:>

 PAGE 2

B</foo/sub/index.htm:>

 Index of /foo/sub



If you now request B<http://host/foo/page1.htm> you will get the following page

  
 <html>
 <head>
 <title>Example</title>
 </head>
 <body>
 <h1>head from foo</h1>
 PAGE 1
 <hr> Footer <hr>
 </body>
 </html>


If you now request B<http://host/foo/sub/page2.htm> you will get the following page

  
 <html>
 <head>
 <title>Example</title>
 </head>
 <body>
 <h1>another head from sub</h1>
 PAGE 2
 <hr> Footer <hr>
 </body>
 </html>


If you now request B<http://host/foo/sub/> you will get the following page

  
 <html>
 <head>
 <title>Example</title>
 </head>
 <body>
 <h1>another head from sub</h1>
 Index of /foo/sub
 <hr> Footer <hr>
 </body>
 </html>

 
  

=head1 Author

G. Richter (richter@dev.ecos.de)

=head1 See Also

perl(1), HTML::Embperl, mod_perl, Apache httpd
