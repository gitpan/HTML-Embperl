#
# Building Makefile for Embperl
#
# (C) 1997-1998 G.Richter (richter@dev.ecos.de) / ECOS
#
#


use ExtUtils::MakeMaker;
use Cwd ;
use Config ;

$win32 = ($Config{osname} =~ /win32/i) ;
print "\nRunning on Win 32\n" if ($win32) ;


sub MY::test_via_harness 
	{
	my ($txt) = shift -> MM::test_via_harness (@_) ;
	$txt =~ s/PERL_DL_NONLAZY=1/PERL_DL_NONLAZY=0/ ;
	#$txt =~ s/\$\(FULLPERL\)/\$\(FULLPERL\) \-T / ;
	return $txt ;
	}

sub MY::test_via_script 
	{
	my ($txt) = shift -> MM::test_via_script (@_) ;
	$txt =~ s/PERL_DL_NONLAZY=1/PERL_DL_NONLAZY=0/ ;
	#$txt =~ s/\$\(FULLPERL\)/\$\(FULLPERL\) \-T / ;
	return $txt ;
	}
	
sub MY::xs_c 
	{
	my ($txt) = shift -> MM::xs_c (@_) ;
	$txt =~ s/\&\&/\n\t/ if ($win32) ;
	return $txt ;
	}
	

## ----------------------------------------------------------------------------

sub GetString
	{
	my ($prompt, $default) = @_ ;

	printf ("%s [%s]", $prompt, $default) ;
	chop ($_ = <STDIN>) ;
	if (!/^\s*$/)
	    {return $_ ;}
	else
    	{
        if ($_ eq "")
	        {return $default ;}
	    else
            { return "" ; }
    
        }
    }

## ----------------------------------------------------------------------------

sub GetYesNo
	{
	my ($prompt, $default) = @_ ;
	my ($value) ;

	do
	    {
	    $value = lc (GetString ($prompt . "(y/n)", ($default?"y":"n"))) ;
	    }
	until (($value cmp "j") == 0 || ($value cmp "y") == 0 || ($value cmp "n" ) == 0) ;

	return ($value cmp "n") != 0 ;
	}

## ----------------------------------------------------------------------------

#
# Check if known config
#


$apache = 0 ;
$b = 0 ;

if (defined ($ARGV[0]))
    {
    $apache = 2 ;
    $b = 1 ;
    $apache_src = $ARGV[0] ;
    }
elsif (defined ($ENV{APACHE_SRC}))
    {
    $apache = 2 ;
    $b = 1 ;
    $apache_src = $ENV{APACHE_SRC} ;
    }

if (!$apache)
    {
    eval 'use Apache::MyConfig' ;

    if ($@ eq '')
        { 
        $apache_src = $Apache::MyConfig::Setup{Apache_Src} ; 
        }
    else
        {
        $apache_src = '' ;
        }
    }

$base = '..' ;



$apache = GetYesNo ("Build with support for Apache mod_perl?", 'y') if (!$apache) ;

if ($apache && $apache_src ne '')
    {
    if ($apache_src =~ /^(.*?)\/$/)
   	{ $apache_src = $1 ; } 

    if ($apache_src =~ /^(.*?)\/main$/)
   	{ $apache_src = $1 ; } 

    if ($apache_src =~ /^(.*?)\/include$/)
   	{ $apache_src = $1 ; } 

    if (-e "$apache_src/httpd.h" || -e "$apache_src/main/httpd.h" || -e "$apache_src/include/httpd.h")
	    {
	    $b = GetYesNo ("Use $apache_src as Apache source", 'y') if (!$b) ;
	    }
    }

while ($apache && !$b)
    {
    print "Searching for Apache sources...\n" ;
    foreach $src_dir ($base,
		      "$base/src",
		      <$base/apache*/src>,
		      <./src>)
       {
	print "Look at $src_dir\n" ;
	
	if (-e "$src_dir/httpd.h" || -e "$src_dir/main/httpd.h" || -e "$src_dir/include/httpd.h")
		{
		$b = GetYesNo ("Use $src_dir as Apache source", 'y') ;
		if ($b)
			{
			$apache_src = $src_dir ;
			last ;
			}
		}
	}		

    if ($apache_src =~ /^(.*?)\/$/)
   	{ $apache_src = $1 ; } 

    if ($apache_src =~ /^(.*?)\/main$/)
   	{ $apache_src = $1 ; } 

    if ($apache_src =~ /^(.*?)\/include$/)
   	{ $apache_src = $1 ; } 


    if (!$b)
    	{
    	$base = GetString ("Apache source not found, enter path name or q to quit", '') ;
    	if ($base eq 'q')
    		{
    		$apache = 0 ;
    		}
    	
    	}
    }

$b = 0 if ($apache_src eq '') ;

if ($b && $apache)
    {
    print "Will use $apache_src for Apache Headers\n" ;
    if ($win32)
	{ 
        $i = "-I. -I$apache_src -I$apache_src/regex -I$apache_src/main -I$apache_src/include -I$apache_src/os/win32" ;
        $o = " $apache_src/CoreR/ApacheCore.lib" ;
        }
    else
	{
        $i = "-I$apache_src -I$apache_src/regex -I$apache_src/main -I$apache_src/include -I$apache_src/os/unix" ; 
        $o = '' ;
        }
    $d = "-DAPACHE" ;
    }
else
    {
    $apache = 0 ;
    print "Will build without mod_perl support\n" ;
    $i = '' ;
    $d = '' ;
    $o = '' ;
    }
		
if ($win32 && $apache)
    {	# borrowed from mod_perl
    local *FH;
    open FH, ">dirent.h" || die "can't write dirent.h $!";
    print FH <<EOF;
/* major kludge to workaround conflict(s) between perl's dirent.h and apache's readdir.h */

#ifdef WIN32

#define _INC_DIRENT
#define DIR void

#endif

EOF
    close FH;
    }

#
# Check to see which user to use for httpd tests
#

$EPPATH  = cwd ;

if ($b && $apache && !$win32)
    {
    $EPPORT  = 8529 ;
    $EPUSER  = getpwuid($>) ||  $> ;
    $EPGROUP = getgrgid($)) || $) ;
    $EPHTTPD = "$apache_src/httpd" ;
    if ($EPUSER eq 'root')
        {
        my $nobody = (getpwnam('nobody'))[0] ;
        $EPUSER = $nobody if $nobody ;
        }

    if ($EPUSER eq 'root')
        {
        print "Cannot run test httpd as User $EPUSER\n" ;
        $EPUSER = GetString ("User to run httpd", 'nobody') ;
        $EPGROUP = GetString ("Group to run httpd", $EPGROUP) ;
        }

    if (!-e $EPHTTPD)
        {
        $EPHTTPD = GetString ("Enter file to start as httpd", "$EPHTTPD") ;
        }

    print "Test start $EPHTTPD\n" ;
    print "Test httpd will run as user $EPUSER and group $EPGROUP\n" ;
    print "Test httpd will listen on port $EPPORT\n" ;
    open FH, ">$EPPATH/test/conf/config.pl" or die "Cannot open $EPPATH/test/conf/config.pl" ;
    print FH "# This file is automaticly generated by Makefile.PL, do not edit\n" ;
    print FH "\$EPPATH='$EPPATH' ;\n" ;
    print FH "\$EPUSER='$EPUSER' ;\n" ;
    print FH "\$EPGROUP='$EPGROUP' ;\n" ;
    print FH "\$EPPORT='$EPPORT' ;\n" ;
    print FH "\$EPHTTPD='$EPHTTPD' ;\n" ;
    close FH ;
    }
else
    {
    open FH, ">$EPPATH/test/conf/config.pl" or die "Cannot open $EPPATH/test/conf/config.pl" ;
    print FH "# This file is automaticly generated by Makefile.PL, do not edit\n" ;
    print FH "\$EPPATH='$EPPATH' ; \n" ;
    print FH "\$EPHTTPD='' ;\n" ;
    close FH ;
    }

#
# Change path to perl binary
#

@bins = ('embpexec.pl', 'embpexec.test.pl') ;

foreach $f (@bins)
	{
	unlink "$f.org" ;
	rename $f, "$f.org" or die "Cannot rename $f to $f.org" ;
	open IN, "<$f.org" or die "Cannot open $f.org" ;
	open OUT, ">$f" or die "Cannot open $f" ;
	while (<IN>)
		{
		s/\#\!\/.*?\/perl.*?\s(.*?)/#!$Config{'perlpath'} $1/ ;
		print OUT $_ ;
		}
	close IN ;
	close OUT ;
	chmod 0755, $f or die "Cannot set executable $f" ;
	}

WriteMakefile(
    'NAME'	   => 'HTML::Embperl',
    'VERSION_FROM' => 'Embperl.pm', 		# finds $VERSION
    'OBJECT'     => 'Embperl$(OBJ_EXT) epmain$(OBJ_EXT) epio$(OBJ_EXT) epchar$(OBJ_EXT) epcmd$(OBJ_EXT) eputil$(OBJ_EXT) epeval$(OBJ_EXT)' . $o,
    'LIBS'	   => [''],   			 
    'DEFINE'	   => "$d \$(DEFS)", 			 
    'INC'	   => $i,     			 
    'clean'        => { FILES => 'dirent.h' },
    'dist'         => { COMPRESS => 'gzip', SUFFIX => 'gz'},
);
