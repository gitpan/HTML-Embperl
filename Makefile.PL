#
# Building Makefile for Embperl
#
# (C) 1997-1999 G.Richter (richter@dev.ecos.de) / ECOS
#
#


use ExtUtils::MakeMaker;
use Cwd qw {abs_path cwd} ;
use Config ;
use File::Basename ;


$win32 = ($Config{osname} =~ /win32/i) ;
$aix = ($Config{osname} =~ /aix/i);
$dynlib = {};
print "\nRunning on Win 32\n" if ($win32) ;

## ----------------------------------------------------------------------------



%neededmodules =
    ( 
      'mod_perl.c' => { name	    => 'perl_module', 
			path	    => ['$apache_src/modules/perl/libperl.so', '$EPBINDIR/modules/libperl.so'],
			win32path   => ['$EPBINDIR/modules/apachemoduleperl.dll'],
			win32path   => ['$mpdll'],
			file	    => 'libperl.so',
			win32file   => 'apachemoduleperl.dll',
		      },
      'mod_dir.c'  => { name	    => 'dir_module', 
			path	    => ['$apache_src/modules/standard/mod_dir.so', '$EPBINDIR/modules/mod_dir.so'],
			win32path   => ['$apache_src/modules/standard/apachemoduledir.dll', '$EPBINDIR/modules/apachemoduledir.dll'],
			file	    => 'mod_dir.so',
			win32file   => 'apachemoduledir.dll',
		      },

      'mod_env.c'  => { name	    => 'env_module', 
			path	    => ['$apache_src/modules/standard/mod_env.so', '$EPBINDIR/modules/mod_env.so'],
			win32path   => ['$apache_src/modules/standard/apachemoduleenv.dll', '$EPBINDIR/modules/apachemoduleenv.dll'],
			file	    => 'mod_env.so',
			win32file   => 'apachemoduleenv.dll',
		      },

      'mod_mime.c'  => { name	    => 'mime_module', 
			path	    => ['$apache_src/modules/standard/mod_mime.so', '$EPBINDIR/modules/mod_mime.so'],
			win32path   => ['$apache_src/modules/standard/apachemodulemime.dll', '$EPBINDIR/modules/apachemodulemime.dll'],
			file	    => 'mod_mime.so',
			win32file   => 'apachemodulemime.dll',
		      },

      'mod_alias.c'  => { name	    => 'alias_module', 
			path	    => ['$apache_src/modules/standard/mod_alias.so', '$EPBINDIR/modules/mod_alias.so'],
			win32path   => ['$apache_src/modules/standard/apachemodulealias.dll', '$EPBINDIR/modules/apachemodulealias.dll'],
			file	    => 'mod_alias.so',
			win32file   => 'apachemodulealias.dll',
		      },

      'mod_cgi.c'  => { name	    => 'cgi_module', 
			path	    => ['$apache_src/modules/standard/mod_cgi.so', '$EPBINDIR/modules/mod_cgi.so'],
			win32path   => ['$apache_src/modules/standard/apachemodulecgi.dll', '$EPBINDIR/modules/apachemodulecgi.dll'],
			file	    => 'mod_cgi.so',
			win32file   => 'apachemodulecgi.dll',
		      },

      'mod_actions.c'  => { name	    => 'action_module', 
			path	    => ['$apache_src/modules/standard/mod_actions.so', '$EPBINDIR/modules/mod_actions.so'],
			win32path   => ['$apache_src/modules/standard/apachemoduleactions.dll', '$EPBINDIR/modules/apachemoduleactions.dll'],
			file	    => 'mod_actions.so',
			win32file   => 'apachemoduleactions.dll',
		      },


    ) ;




## ----------------------------------------------------------------------------


sub MY::test_via_harness 
	{
	my ($txt) = shift -> MM::test_via_harness (@_) ;
	$txt =~ s/PERL_DL_NONLAZY=1/PERL_DL_NONLAZY=0/ ;
	#$txt =~ s/\$\(FULLPERL\)/\$\(FULLPERL\) \-T / ;
	$txt =~ s/\$\(FULLPERL\)/SET PATH=\$\(PATH\)\;$EPHTTPDDLL\n\t\$\(FULLPERL\)/ if ($win32) ;
	return $txt ;
	}

sub MY::test_via_script 
	{
	my ($txt) = shift -> MM::test_via_script (@_) ;
	$txt =~ s/PERL_DL_NONLAZY=1/PERL_DL_NONLAZY=0/ ;
	#$txt =~ s/\$\(FULLPERL\)/\$\(FULLPERL\) \-T / ;
	$txt =~ s/\$\(FULLPERL\)/SET PATH=\$\(PATH\)\;$EPHTTPDDLL\n\t\$\(FULLPERL\)/ if ($win32) ;
	return $txt ;
	}
	
sub MY::xs_c 
	{
	my ($txt) = shift -> MM::xs_c (@_) ;
	$txt =~ s/\&\&/\n\t/ if ($win32) ;
	return $txt ;
	}

sub MY::dist_test 
	{
	my $self = shift ;
        
        my $txt = $self -> MM::dist_test (@_) ;
	$txt =~ s/-I\$\(PERL_ARCHLIB\) -I\$\(PERL_LIB\)// ;
	$txt =~ s/\$\(PERL\)/\$\(PERLDT\)/ ;
	
        $main::perlbinpath = $self -> {FULLPERL} ;
        
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

sub search_config

    {
    my ($key, $path) = @_ ;


    open FH, $path or return undef ;

    while (<FH>)
        {
        return $1 if (/^$key\s*=\s*(.*?)$/) ;
        }

    close FH ;
    return undef ;
    }

## ----------------------------------------------------------------------------


sub cnvpath

    {
    my $path = shift ;
    
    $path =~ s/\//\\/g if ($win32) ;

    return $path ;
    }

## ----------------------------------------------------------------------------


sub start

    {
    my ($cmd) = @_ ;


    $cmd =~ s/\//\\/g if ($win32) ;

    
    open FH, "$cmd|" or die "\nCannot start $cmd\nPlease make sure you have build Apache and mod_perl before makeing Embperl\n" ;

    my @x = <FH> or die "\nCannot start $cmd\nPlease make sure you have build Apache and mod_perl before makeing Embperl\n" ;

    close FH ;
    return @x ;
    }


## ----------------------------------------------------------------------------
#
# Check if required modules present
#


sub CheckModule

    {
    my ($mod, $text) = @_ ;

    eval "require $mod" ;
    if ($@)
        {
        print "$mod not installed on this system\n" ;
        print "$text\n" ;
        return undef ;
        }
    else
        {
        my $ver = ${"$mod\:\:VERSION"} ;
        print "Found $mod  Version $ver\n" ;
        return $ver ;
        }
    }
         


## ----------------------------------------------------------------------------
#
# Check if known config
#


$apache = 0 ;
$b = 0 ;

if (defined ($ARGV[0]) && ($ARGV[0] =~ /^\W/))
    {
    $apache = 2 ;
    $b = 1 ;
    $apache_src = shift @ARGV ;
    }
elsif (defined ($ENV{APACHE_SRC}))
    {
    $apache = 2 ;
    $b = 1 ;
    $apache_src = $ENV{APACHE_SRC} ;
    }

if (!$apache && $apache_src eq '')
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

if (!$apache && $apache_src eq '')
    {
    eval 'do "test/conf/config.pl"' ;

    $apache_src = $EPAPACHESRC ;
    $loadmodules = $EPMODPERL ;
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
        else
            {
            $base =~ s/\//\\/g if ($win32) ;
            }    	
    	}
    }

if ($b && $apache && $apache_src ne '')
    {
    $apache_src = abs_path ($apache_src) ;

    print "Will use $apache_src for Apache Headers\n" ;

    #### look in which subdir the include files resides ####
    
    if (-e "$apache_src/httpd.h")
         {
         $inc_dir = $apache_src ;
         }
    elsif (-e "$apache_src/main/httpd.h")
         {
         $inc_dir = "$apache_src/main" ;
         }
    elsif (-e "$apache_src/include/httpd.h")
         {
         $inc_dir = "$apache_src/include" ;
         }
        

    if ($win32)
	{ 
        $i = "-I. -I$inc_dir -I$apache_src/regex -I$apache_src/os/win32" ;
	if (!-e "$apache_src/CoreR/ApacheCore.lib")
            {
	    $o = " $apache_src/CoreD/ApacheCore.lib" ;
	    }
	else
	    {        
	    $o = " $apache_src/CoreR/ApacheCore.lib" ;
	    }
        }
    else
	{
        $i = "-I$inc_dir -I$apache_src/regex -I$apache_src/os/unix" ; 
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

=pod
    if ($ENV{APACHE_PERL_DLL})
	{
	$mpdll = $ENV{APACHE_PERL_DLL} ;
	}
    elsif ($EPMODPERL =~ /^LoadModule perl_module (.*?)$/)
	{
	$mpdll = $1 ;
	}
    else
	{
	SEARCH:
	    {
	    for my $drive ('c'..'g') 
		{
		for my $p ("program files\\apache\\modules", "apache\\modules")
		    { 
		    last SEARCH if -e ($mpdll = "$drive:\\$p\\apachemoduleperl.dll");
		    }
		}
	    }
	}

    $mpdll .= "\\apachemoduleperl.dll" if (!($mpdll =~ /apachemoduleperl\.dll/i)) ;
    if (!(-f $mpdll))
        {
        require ExtUtils::MakeMaker;
        ExtUtils::MakeMaker->import('prompt');
        $mpdll = prompt("Where is your ApacheModulePerl.dll located ?", $mpdll);
        $mpdll .= "\\apachemoduleperl.dll" if (!($mpdll =~ /apachemoduleperl\.dll/i)) ;
	if (!(-f $mpdll))
	    {
	    print "$mpdll not found, please make sure Apache and mod_perl are installed before installing Embperl\n" ;
	    exit (1) ;
	    }
	}

    $mpdll = cnvpath ($mpdll) ;
    $mpdll =~ s/\\\\/\\/g ;

    die "Can't find ApacheModulePerl.dll at $mpdll!" if (!(-e $mpdll));
=cut

    }

#
# Check to see which user to use for httpd tests
#

$EPPATH    = cwd ;
$EPMODPERL = '' ;
$EPSTARTUP ='startup.pl' ;

if ($b && $apache)
    {
    $EPPORT  = 8529 ;
    if (!$win32)
        {
        $EPUSER  = getpwuid($>) ||  $> ;
        $EPGROUP = getgrgid($)) || $) ;
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
        
        $EPHTTPD = "$apache_src/httpd" ;
        $EPHTTPD = "$apache_src/httpsd" if (!-e $EPHTTPD && -e "$apache_src/httpsd") ;
        
        if (!-e $EPHTTPD)
            {
            $EPHTTPD = GetString ("Enter path and file to start as httpd", "$EPHTTPD") ;
            }
	
	$EPMODPERL="" ;
        }
    else
        {
        $EPHTTPD = "$apache_src/ApacheR/Apache.exe" ;
        $EPHTTPDDLL = "$apache_src/CoreR" ;
        if (!-e $EPHTTPD) 
            {
            $EPHTTPD = "$apache_src/ApacheD/Apache.exe" ;
            $EPHTTPDDLL = "$apache_src/CoreD" ;
            }
        #$EPMODPERL="LoadModule perl_module $mpdll" ;
        $EPUSER  = 'www' ; # dummy value
        $EPGROUP = 'www' ;
        if (!-e $EPHTTPD)
            {
            $EPHTTPD = GetString ("Enter path and file to start as Apache.exe", "$EPHTTPD") ;
            }
        }
    

    ### check the apache version ###
                                
    $ENV{PATH} .= ";$EPHTTPDDLL" if ($win32) ;

    
    @EPAPACHEVERSION = start ("$EPHTTPD -v") ;
    @modules         = start ("$EPHTTPD -l") ;

    print "Apache Version $EPAPACHEVERSION[0]" ;

    $EPSTRONGHOLD = ($EPAPACHEVERSION[0] =~ /stronghold/i) ;
    $EPAPACHE_SSL = grep (/apache_ssl.c/, @modules) ;
    $EPMOD_SSL    = !$EPSTRONGHOLD && grep (/mod_ssl.c/, @modules) ;

    $EPBINDIR	  = dirname ($EPHTTPD) ;    
    $EPMODPERL	  = '' ;
    
    $addmodpath = '' ;
    while (($mod, $opt) = each %neededmodules)
	{
	if (!grep (/$mod/, @modules))
	    { # module not linked staticly
	    $paths = $win32?$opt->{win32path}:$opt->{path} ;
	    $modfile = $win32?$opt->{win32file}:$opt->{file} ;
	    $found = 2 ;
	    while ($found == 2)
		{
		$found = 0 ;
		if ($addmodpath)
		    {
		    $path = cnvpath ("$addmodpath/$modfile") ;
		    #print "path = $_ -> $path\n" ;
		    if (-e $path)
			{ ## module fould
	    		$EPMODPERL .= "\r\nLoadModule $opt->{name} $path" ;
			print " + Load dynamic module $mod\n" ;
			$found = 1 ;
			last ;
			}
		    }
		foreach (@$paths)
		    {
		    $path = cnvpath (eval "\"$_\"") ;
		    #print "path = $_ -> $path\n" ;
		    if (-e $path)
			{ ## module fould
	    		$EPMODPERL .= "\r\nLoadModule $opt->{name} $path" ;
			print " + Load dynamic module $mod\n" ;
			$found = 1 ;
			last ;
			}
		    }
		if (!$found)
		    {
		    my $w32msg = '' ;
		    $w32msg = "\nPlease enter full path including the drive letter!! " if ($win32) ;
		    $addmodpath = GetString ("Library for $mod not found, please enter path to $modfile $w32msg", "") ;
		    $found = 2 ;
		    }
		}			    
	    }
	}


		    

    #if (!$win32 && -e "$apache_src/modules/perl/libperl.so" && !grep (/mod_perl.c/, @modules)) 
    if (!$win32 && ($EPMODPERL =~ /perl_module/))
	{
	#$EPMODPERL="LoadModule perl_module $apache_src/modules/perl/libperl.so" ;
        $EPSTARTUP ='startup_dso.pl' ;
        print " + mod_perl was build with USE_DSO\n" ;
	$dynlib->{'OTHERLDFLAGS'} = "-bI:mod_perl.exp -bI:$apache_src/support/httpd.exp" if ($aix);
	}

    
    if ($EPSTRONGHOLD) 
        {
        $i .= " -I$apache_src/../ssl/include" ;
        $d .= " -DSTRONGHOLD" ;
        print " + found Stronghold\n" ;
        }
    elsif ($EPAPACHE_SSL)
        {
        $d .= " -DEPAPACHE_SSL" ;
        print " + found Apache SSL\n" ;
        }
    elsif ($EPMOD_SSL)
        {
        my $sslbase = search_config ('SSL_BASE', "$apache_src/Configuration.apaci") ;
        $sslbase = search_config ('SSL_BASE', "$apache_src/Configuration") if (!$sslbase) ;
        $i .= " -I$sslbase/include" ;
        print " + found mod_ssl\n" ;
        }



    if ($EPSTRONGHOLD)
        {
        my $conf = "$apache_src/../conf/httpd.conf";
        if(-e $conf) 
            {
	    open FH, $conf;
	    while(<FH>)
                {
	        if(/^StrongholdKey/)
                    {
                    chomp ;
		    $EPSTRONGHOLDKEY = $_;
		    last;
	            }
	        }
	    close FH;
            }
        }
    else
        {
        $EPSTRONGHOLDKEY = '' ;
        }


    print "Test start $EPHTTPD\n" ;
    print "Test httpd will run as user $EPUSER and group $EPGROUP\n" if (!$win32) ;
    print "Test httpd will listen on port $EPPORT\n" ;
    print "Test will use $EPSTRONGHOLDKEY\n" if($EPSTRONGHOLDKEY) ;


    ### check for required modules ###
    
    if (($MPVer = CheckModule ("mod_perl", "-> Cannot build for mod_perl without mod_perl installed!!")))
        {
        if ($win32 && $MPVer lt "1.12")
            {
            print "-> Please upgrade to an higher version of mod_perl on Win32\n" ;
            }
        }

    if (CheckModule ("LWP::UserAgent", "-> Cannot test mod_perl and CGI mode"))
        {
        CheckModule ("HTML::HeadParser", "-> Is required by LWP::UserAgent");
        }

    $SessVer = CheckModule ("Apache::Session", "-> Disable tests for persistent data storage") || '' ;

    if ($SessVer && !($SessVer >= 1.01) && $SessVer != 1.03)
        {
        if ($SessVer && !($SessVer =~ /0\.17/))
            {
            print "-> Embperl works only with Apache::Session 0.17.x or 1.03 and higher\n" ;
            print "-> Disable tests for persistent data storage\n" ;
            $SessVer = '0' ;
            }
        }

    $SessVer ||= 0 ;

    CheckModule ("CGI", "-> File Upload will not work without CGI.pm installed") ;

    ### write out test configuration file ###

    open FH, ">$EPPATH/test/conf/config.pl" or die "Cannot open $EPPATH/test/conf/config.pl" ;
    print FH "# This file is automaticly generated by Makefile.PL, do not edit\n" ;
    print FH "\$EPPATH='" . cnvpath($EPPATH) . "' ;\n" ;
    print FH "\$EPUSER='$EPUSER' ;\n" ;
    print FH "\$EPGROUP='$EPGROUP' ;\n" ;
    print FH "\$EPPORT=$EPPORT ;\n" ;
    print FH "\$EPHTTPD='" . cnvpath($EPHTTPD) . "' ;\n" ;
    print FH "\$EPHTTPDDLL='" . cnvpath($EPHTTPDDLL) . "' ;\n" ;
    print FH "\$EPWIN32='$win32' ;\n" ;
    print FH "\$EPAPACHESRC='" . cnvpath($apache_src) . "' ;\n" ;
    print FH "\$EPAPACHE_SSL='" . $EPAPACHE_SSL . "' ;\n" ;
    print FH "\$EPSTRONGHOLD='$EPSTRONGHOLD' ;\n" ;
    print FH "\$EPSTRONGHOLDKEY='$EPSTRONGHOLDKEY' ;\n" ;
    print FH "\$EPMODPERL='" . cnvpath($EPMODPERL) . "';\n" ;
    print FH "\$EPSTARTUP='" . cnvpath($EPSTARTUP) . "';\n" ;
    print FH "\$EPAPACHEVERSION='$EPAPACHEVERSION[0]';\n" ;
    print FH "\$EPSESSIONVERSION='$SessVer';\n" ;
    if ($win32)
        {
        print FH "\$EPNULL='nul';\n" ;
        }
    else
        {
        print FH "\$EPNULL='/dev/null';\n" ;
        }



    close FH ;
    }
else
    {
    ### check for required modules ###

    $SessVer = CheckModule ("Apache::Session", "-> Disable tests for persistent data storage") || '' ;

    if ($SessVer && $SessVer lt "0.17")
        {
        print "-> Please upgrade to Apache::Session 0.17 or higher\n" ;
        print "-> Disable tests for persistent data storage\n" ;
        $SessVer = 0 ;
        }

    $SessVer ||= 0 ;

    ### write out test configuration file ###

    open FH, ">$EPPATH/test/conf/config.pl" or die "Cannot open $EPPATH/test/conf/config.pl" ;
    print FH "# This file is automaticly generated by Makefile.PL, do not edit\n" ;
    print FH "\$EPPATH='$EPPATH' ; \n" ;
    print FH "\$EPHTTPD='' ;\n" ;
    print FH "\$EPWIN32='$win32' ;\n" ;
    print FH "\$EPSESSIONVERSION='$SessVer';\n" ;
    close FH ;
    }


if ($win32)
    { # Check for winsock2api

    if (open FH, "$inc_dir/ap_config.h")
        {
        my @conf = <FH> ;
        close FH ;

        if (grep (/winsock2\.h/, @conf))
            {
            $d .= ' -D_WINSOCK2API_ ' ;
            }
        }
    }


WriteMakefile(
    'NAME'	   => 'HTML::Embperl',
    'VERSION_FROM' => 'Embperl.pm', 		# finds $VERSION
    'OBJECT'     => 'Embperl$(OBJ_EXT) epmain$(OBJ_EXT) epio$(OBJ_EXT) epchar$(OBJ_EXT) epcmd$(OBJ_EXT) eputil$(OBJ_EXT) epeval$(OBJ_EXT) epdbg$(OBJ_EXT)' . $o,
    'LIBS'	   => [''],   			 
    'DEFINE'	   => "$d \$(DEFS)", 			 
    'INC'	   => $i,     			 
    'MAN3PODS'     => { 'Embperl.pod' => 'Embperl' }, 
    'clean'        => { FILES => 'dirent.h' },
    'dist'         => { COMPRESS => 'gzip', SUFFIX => 'gz'},
    'dynamic_lib'  => $dynlib,
);



#
# Change path to perl binary
#

@bins = ('embpexec.pl','embpexec.bat', 
         'embpcgi.pl', 'embpcgi.test.pl', 'embpcgi.bat', 'embpcgi.test.bat',) ;


die "Missing path to perl binary" if (!$perlbinpath) ;


foreach $f (@bins)
	{
	unlink "$f.org" ;
	rename $f, "$f.org" or die "Cannot rename $f to $f.org" ;
	open IN, "<$f.org" or die "Cannot open $f.org" ;
	open OUT, ">$f" or die "Cannot open $f" ;
	my $l = 1 ;
        while (<IN>)
		{
		if ($l++ < 10)
                    {
		    #if ($win32)
		    #	{
                    #	s/^\#\!.*?perl.*?\s(.*?)/#!perl $1/ ;
		    #	}
		    #else
		    #	{
                    	s/^\#\!.*?perl.*?\s(.*?)/#!$perlbinpath $1/ ;
		    #	}
		    s/^.*?perl.*?\s-x/$perlbinpath -x/ ;
                    }
		print OUT $_ ;
		}
	close IN ;
	close OUT ;
	chmod 0755, $f or die "Cannot set executable $f" ;
	}

