#
# Building Makefile for Embperl
#
# (C) 1997 G.Richter (richter@dev.ecos.de) / ECOS
#
# Note: This is a quick hack, hope it works...
#       Still beta no warrenty!
#


use ExtUtils::MakeMaker;


sub MY::test_via_harness 
	{
	my ($txt) = shift -> MM::test_via_harness (@_) ;
	$txt =~ s/PERL_DL_NONLAZY=1/PERL_DL_NONLAZY=0/ ;
	$txt =~ s/\$\(FULLPERL\)/\$\(FULLPERL\) \-T / ;
	return $txt ;
	}

sub MY::test_via_script 
	{
	my ($txt) = shift -> MM::test_via_script (@_) ;
	$txt =~ s/PERL_DL_NONLAZY=1/PERL_DL_NONLAZY=0/ ;
	$txt =~ s/\$\(FULLPERL\)/\$\(FULLPERL\) \-T / ;
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
# Check if knwon config
#

eval 'use Apache::MyConfig' ;

if ($@ eq '')
    { 
    $apache_src = $Apache::MyConfig::Setup{Apache_Src} ; 
    }
else
    {
    $apache_src = '' ;
    }

$base = '..' ;

$b = 0 ;


$apache = GetYesNo ("Build with support for Apache mod_perl?", 'y') ;

if ($apache && $apache_src ne '')
    {
	if (-e "$apache_src/httpd.h")
		{
		$b = GetYesNo ("Use $apache_src as Apache source", 'y') ;
		}
    }

while ($apache && !$b)
    {
    print "Searching for Apache sources...\n" ;
    foreach $src_dir (<$base/apache_1.1*/src>,
                      <$base/apache_1.2*/src>,
		      <$base/apache*/src>,
		      <$base/src>,
		      <./src>,
		      <$base>) 
       {
	print "Look at $src_dir\n" ;
	
	if (-e "$src_dir/httpd.h")
		{
		$b = GetYesNo ("Use $src_dir as Apache source", 'y') ;
		if ($b)
			{
			$apache_src = $src_dir ;
			last ;
			}
		}
	}		
    if (!$b)
    	{
    	$base = GetString ("Apache source not found, enter path name or q to quit", '') ;
    	if ($base eq 'q')
    		{
    		$apache = 0 ;
    		}
    	
    	}
    	

		
    }

if ($b && $apache)
	{
	print "Will use $apache_src for Apache Headers\n" ;
	$i = "-I$apache_src" ;
	$d = "-DAPACHE" ;
	}
else
	{
	$apache = 0 ;
	print "Will build without mod_perl support\n" ;
	$i = '' ;
	$d = '' ;

	}
		
WriteMakefile(
    'NAME'	   => 'HTML::Embperl',
    'VERSION_FROM' => 'Embperl.pm', 		# finds $VERSION
    'MYEXTLIB'     => 'epmain.o epio.o epchar.o',
    'LIBS'	   => [''],   			 
    'DEFINE'	   => "$d \$(DEFS)", 			 
    'INC'	   => $i,     			 
    'dist'         => { COMPRESS => 'gzip', SUFFIX => 'gz'},
);
