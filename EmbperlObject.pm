
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

use HTML::Embperl ;

if (defined ($ENV{MOD_PERL}))
    { 
    eval 'use Apache::Constants qw(&OPT_EXECCGI &DECLINED &OK &FORBIDDEN &NOT_FOUND) ;' ;
    die "use Apache::Constants failed: $@" if ($@); 
    }
else
    {
    eval 'sub OK        { 0 ;   }' ;
    eval 'sub NOT_FOUND { 404 ; }' ;
    eval 'sub FORBIDDEN { 403 ; }' ;
    eval 'sub DECLINED  { 403 ; }' ; # in non mod_perl environement, same as FORBIDDEN
    }


use File::Spec ;
use File::Basename ;

use strict ;
use vars qw(
    @ISA
    $VERSION
    ) ;


@ISA = qw(Exporter DynaLoader);


$VERSION = '1.3b4';


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

##########################################################################################

sub ScanEnvironment

    {
    my ($req, $req_rec) = @_ ; 

    HTML::Embperl::ScanEnvironment ($req, $req_rec) ;

    $req -> {object_base}           = $ENV{EMBPERL_OBJECT_BASE} || '_base.html' ;
    $req -> {object_addpath}        = $ENV{EMBPERL_OBJECT_ADDPATH} if (exists ($ENV{EMBPERL_OBJECT_ADDPATH})) ;
    $req -> {object_stopdir}        = $ENV{EMBPERL_OBJECT_STOPDIR} if (exists ($ENV{EMBPERL_OBJECT_STOPDIR})) ;
    $req -> {object_fallback}       = $ENV{EMBPERL_OBJECT_FALLBACK} if (exists ($ENV{EMBPERL_OBJECT_FALLBACK})) ;
    $req -> {object_handler_class}  = $ENV{EMBPERL_OBJECT_HANDLER_CLASS} if (exists ($ENV{EMBPERL_OBJECT_HANDLER_CLASS})) ;
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


    my %req ;

    ScanEnvironment (\%req, $r) ;
    
    $req{'inputfile'} = $filename ;
    $req{'object_base_modifier'}  = $mod ;
    $req{'uri'}       = $r -> Apache::uri ;
    $req{'cleanup'} = -1 if (($req{'options'} & HTML::Embperl::optDisableVarCleanup)) ;
    $req{'options'} |= HTML::Embperl::optSendHttpHeader ;
    $req{'req_rec'} = $r ;
    
    Execute (\%req) ;
    }
        
    
    
#############################################################################

sub Execute

    {
    my $req = shift ;
    
    my $filename = $req -> {inputfile} ;
    my $r        ;
    $r = $req -> {req_rec} if ($req -> {req_rec}) ;

    if (exists $req -> {filesmatch} && 
                         !($filename =~ m{$req->{filesmatch}})) 
        {
        return &DECLINED ;
        }

    if (exists $req -> {decline} && 
                         ($filename =~ m{$req->{decline}})) 
        {
        return &DECLINED ;
        }


    my $basename  = $req -> {object_base} ;
    $basename     =~ s/%modifier%/$req->{object_base_modifier}/ ;
    my $addpath   =  $req -> {object_addpath}  ;
    my @addpath   = $addpath?split (/:/, $addpath):() ;
    my $directory ;
    my $rootdir   = $r?norm_path ($r -> document_root):'/' ;
    my $stopdir   = norm_path ($req -> {object_stopdir}) ;
    my $debug     = $req -> {debug} & HTML::Embperl::dbgObjectSearch ;
    
    if (-d $filename)
        {
        $directory = $filename ;
        }
    else
        {
        $directory = dirname ($filename) ;
        }
    
    my $searchpath  ;
    	 
    $r -> notes ('EMBPERL_orgfilename',  $filename) if ($r) ;
 
    print HTML::Embperl::LOG "[$$]EmbperlObject Request Filename: $filename\n" if ($debug);
    print HTML::Embperl::LOG "[$$]EmbperlObject basename: $basename\n"  if ($debug);
    
    my $fn ;
    my $ap ;
    my $ldir ;
    my $found = 0 ;
    my $fallback = 0 ;
    	
    do
        {
        $fn = "$directory/$basename" ;
        $searchpath .= ";$directory" ; 
        print HTML::Embperl::LOG "[$$]EmbperlObject Check for base: $fn\n"  if ($debug);
        if (-e $fn)
            {
            $r -> filename ($fn) if ($r) ;
            $r -> notes ('EMBPERL_searchpath',  $searchpath) if ($r) ;
            $found = 1 ;
            }
        else
            {
	    $ldir      = $directory ;
            $directory = dirname ($directory) ;
            }
        }
    while (!$found && $ldir ne $rootdir && $ldir ne $stopdir && $directory ne '/' && $directory ne '.' && $directory ne $ldir) ;

    if (!$found)
        {
        foreach $ap (@addpath)
            {
            next if (!$ap) ;
            $fn = "$ap/$basename" ;
            $searchpath .= ";$ap" ; 
            print HTML::Embperl::LOG "[$$]EmbperlObject Check for base: $fn\n"  if ($debug);
            if (-e $fn)
                {
                $r -> filename ($fn) if ($r) ;
                $r -> notes ('EMBPERL_searchpath',  $searchpath) if ($r) ;
                $found = 1 ;
                last ;
                }

            }
        }


    if ($found)
        {
        print HTML::Embperl::LOG "[$$]EmbperlObject Found Base: $fn\n"  if ($debug);
        print HTML::Embperl::LOG "[$$]EmbperlObject path: $searchpath\n"  if ($debug);
        my ($basenew, $basepackage) = HTML::Embperl::GetPackageOfFile ($fn, $req -> {'package'} || '', -M _) ;

        if (!-f $filename && exists $req -> {object_fallback})
            {
            $fallback = 1 ;
            $filename = $req -> {object_fallback} ;
            print HTML::Embperl::LOG "[$$]EmbperlObject use fallback: $filename\n"  if ($debug);
            $r -> notes ('EMBPERL_orgfilename',  $filename) if ($r) ;
            }


        my ($new, $package)  ;
        ($new, $package) = HTML::Embperl::GetPackageOfFile ($filename, $req -> {'package'} || '', -M _) if (!$fallback) ;

        if ($basenew)
            {
            print HTML::Embperl::LOG "[$$]EmbperlObject new Base: $fn, package = $basepackage\n"  if ($debug);
            
            HTML::Embperl::Execute ({%$req, inputfile => $fn, import => 0 }) ;

            no strict ;
            @{"$basepackage\:\:ISA"} = ($req -> {object_handler_class} || 'HTML::Embperl::Req') ;
            use strict ;
            }

        if ($new || $fallback)
            {
            print HTML::Embperl::LOG "[$$]EmbperlObject new file: $filename, package = $package\n"  if ($debug && !$fallback);
            
            HTML::Embperl::Execute ({%$req, inputfile => $filename, import => 0, 'path' => $searchpath }) ;

            if ($fallback)
                {
                $package = $HTML::Embperl::evalpackage ;
                print HTML::Embperl::LOG "[$$]EmbperlObject new file: $filename, package = $package\n"  if ($debug);
                }

            no strict ;
            @{"$package\:\:ISA"} = ($basepackage) ;
            use strict ;
            }

        $req -> {'inputfile'} = $ENV{PATH_TRANSLATED} = $fn ;
        $req -> {'bless'}     = $package ;
        $req -> {'path'}      = $searchpath ;
        $req -> {'reqfilename'} = $filename ;
        return HTML::Embperl::Execute ($req) ;
        }

   
    $r -> log_error ("EmbperlObject searched '$searchpath'" . ($addpath?" and '$addpath' ":''))  if ($r) ;

    return &NOT_FOUND ;
    }


__END__


=head1 NAME

HTML::EmbperlObject - Extents HTML::Embperl for building whole website with reusable components and objects


=head1 SYNOPSIS


    <Location /foo>
        PerlSetEnv EMBPERL_OBJECT_BASE base.htm
        PerlSetEnv EMBPERL_FILESMATCH "\.htm.?|\.epl$"
        SetHandler perl-script
        PerlHandler HTML::EmbperlObject 
        Options ExecCGI
    </Location>


=head1 DESCRIPTION

I<HTML::EmbperlObject> is basicly a I<mod_perl> handler or could be invoked
offline and helps you to
build a whole page out of smaller parts. Basicly it does the following:

When a request comes in a page, which name is specified by L<EMBPERL_OBJECT_BASE>, is
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

Additionaly I<EmbperlObject> sets up a inherence hierachie for you: The requested page
inherit from the base page and the base page inherit from a class which could be
specified by C<EMBPERL_OBJECT_HANDLER_CLASS>, or if C<EMBPERL_OBJECT_HANDLER_CLASS> is
not set, from C<HTML::Embperl::Req>. That allows you to define methods in base page and
overwrite them as neccessary in the original requested files. For this purpose a request
object, which is blessed into the package of the requested page, is given as first 
parameter to each page (in C<$_[0]>). Because this request object is a hashref, you can
also use it to store additional data, which should be available in all components. 
I<Embperl> does not use this hash itself, so you are free to store whatever you want.
Methods can be ordinary Perl sub's (defined with [! sub foo { ... } !] ) or Embperl sub's
(defined with [$sub foo $] .... [$endsub $]) .

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

=head2 EMBPERL_OBJECT_FALLBACK

If the requested file is not found the file given by C<EMBPERL_OBJECT_FALLBACK>
is displayed instead. If C<EMBPERL_OBJECT_FALLBACK> isn't set a
staus 404, NOT_FOUND is returned as usual. If the fileame given in 
C<EMBPERL_OBJECT_FALLBACK> doesn't contain a path, it is searched thru the same
directories as C<EMBPERL_OBJECT_BASE>.

=head2 EMBPERL_OBJECT_HANDLER_CLASS

If you specify this call the template base and the requested page inherit all
methods from this class. This class must contain C<HTML::Embperl::Req> in his
@ISA array.


=head1 Execute

You can use I<EmbperlObject> also offline. You can do this by calling the function
C<HTML::EmbperlObject::Execute>. C<Execute> takes a hashref as argument, which can
contains the same parameters as the C<HTML::Embperl::Execute> function. Additionaly
you may specify the following parameters:

=over 4

=item object_base

same as $ENV{EMBPERL_OBJECT_BASE} 

=item object_addpath

same as $ENV{EMBPERL_OBJECT_ADDPATH} 

=item object_stopdir

same as $ENV{EMBPERL_OBJECT_STOPDIR} 

=item object_fallback

same as $ENV{EMBPERL_OBJECT_FALLBACK} 

=item object_handler_class

same as $ENV{EMBPERL_OBJECT_HANDLER_CLASS}

=back


=head1 Basic Example


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

 

=head1 Example for using method calls

(Everything not given here is the same as in the example above)


B</foo/base.htm:>

 [!

 sub new
    {
    my $self = shift ; 

    # here we attach some data to the request object
    $self -> {fontsize} = 3 ;
    }

 # Here we give a default title
 sub title { 'Title not given' } ;

 !]

 [-
  
 # get the request object of the current request
 $req = shift ;

 # here we call the method new
 $req -> new ;

 -]

 <html>
 <head>
 <title>[+ $req -> title +]</title>
 </head>
 <body>
 [- Execute ('head.htm') -]
 [- Execute ('*') -]
 [- Execute ('foot.htm') -]
 </body>
 </html>


B</foo/head.htm:>


 [# 
    here we use the fontsize
    Note that 
      $foo = $_[0] 
    is the same as writing 
      $foo = shift  
 #]

 <font size=[+ $_[0] -> {fontsize} +]>header</font>

B</foo/sub/page2.htm:>

 [!

 sub new
    {
    my $self = shift ; 

    # here we overwrite the new method form base.htm
    $self -> {fontsize} = 5 ;
    }

 # Here we overwrite the default title
 sub title { 'Title form page 2' } ;

 !]

 PAGE 2


  

=head1 Author

G. Richter (richter@dev.ecos.de)

=head1 See Also

perl(1), HTML::Embperl, mod_perl, Apache httpd
