#
# Building Makefile for Embperl
#
# (C) 1997 G.Richter (richter@dev.ecos.de) / ECOS
#
# Note: This is a quick hack, hope it works...
#       Still beta no warrenty!
#


use ExtUtils::MakeMaker;


WriteMakefile(
    'NAME'	   => 'Apache::Embperl',
    'VERSION_FROM' => 'Embperl.pm', 		# finds $VERSION
    'LIBS'	   => [''],   			 
    'dist'         => { COMPRESS => 'gzip', SUFFIX => 'gz'},
);
