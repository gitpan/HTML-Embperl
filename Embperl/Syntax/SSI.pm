
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
#   $Id: SSI.pm,v 1.1.2.12 2001/11/20 15:15:42 richter Exp $
#
###################################################################################
 
package HTML::Embperl::Syntax::SSI ;

use HTML::Embperl::Syntax qw{:types} ;
use HTML::Embperl::Syntax::HTML ;
use File::Basename;

BEGIN { 
    local $^W = 0 ;
    require POSIX ;

    eval "use Apache::Constants qw(:common OPT_INCNOEXEC);" ;
    } ;

use strict ;
use vars qw{@ISA $apreq $epreq} ;

@ISA = qw(HTML::Embperl::Syntax::HTML) ;


###################################################################################
#
#   Methods
#
###################################################################################

# ---------------------------------------------------------------------------------
#
#   Create new Syntax Object
#
# ---------------------------------------------------------------------------------


sub new

    {
    my $class = shift ;

    my $self = HTML::Embperl::Syntax::HTML::new ($class) ;

    if (!$self -> {-ssiInit})
        {
        $self -> {-ssiInit} = 1 ;    
        Init ($self) ;
        }

    return $self ;
    }



###################################################################################
#
#   Definitions for SSI HTML tags
#
###################################################################################

sub Init

    {
    my ($self) = @_ ;

    $self -> AddInitCode (undef, 'HTML::Embperl::Syntax::SSI::InitSSI($_[0], $req_rec);', undef) ;

    $self -> AddComment ('#echo', ['var', 'encoding'], undef, undef, { perlcode => '_ep_rp(%$x%, $ENV{%&*\'var%}) ;' } ) ;
    $self -> AddComment ('#printenv', undef, undef, undef, { perlcode => '_ep_rp(%$x%, join ("\\\\<br\\\\>\n", map { "$_ = $ENV{$_}" } keys %ENV)) ;' } ) ;
    $self -> AddComment ('#config', ['errmsg', 'sizefmt', 'timefmt'], undef, undef,  
                            {   perlcode => [
                                            '$_ep_ssi_errmsg  = %&*\'errmsg% ;',
                                            '$_ep_ssi_sizefmt = %&*\'sizefmt% ;',
                                            '$_ep_ssi_timefmt = %&*\'timefmt% ;',
                                            ],
                              removenode => 1 
                                             } ) ;

    $self -> AddComment ('#exec', ['cgi', 'cmd'], undef, undef, 
                            { perlcode => [
                                        '_ep_rp(%$x%, HTML::Embperl::Syntax::SSI::exec (%&\'cmd%, %&\'cgi%)) ;',
                                        ] } ) ;

    $self -> AddComment ('#fsize', ['file', 'virtual'], undef, undef, 
                            { perlcode => [
                                        '_ep_rp(%$x%, HTML::Embperl::Syntax::SSI::fsize ($_ep_ssi_sizefmt, %&\'file%, %&\'virtual%)) ;',
                                        ] } ) ;
    $self -> AddComment ('#flastmod', ['file', 'virtual'], undef, undef, 
                            { perlcode => [
                                        '_ep_rp(%$x%, HTML::Embperl::Syntax::SSI::flastmod ($_ep_ssi_timefmt, %&\'file%, %&\'virtual%)) ;',
                                        ] } ) ;

    $self -> AddComment ('#include', ['file', 'virtual'], undef, undef, 
                            { perlcode => [
                                        '_ep_rp(%$x%, HTML::Embperl::Syntax::SSI::include (%&\'file%, %&\'virtual%)) ;',
                                        ] } ) ;
    $self -> AddComment ('#set', ['var', 'value'], undef, undef, 
                            { perlcode   => '%&value%',
                              compiletimeperlcode => '$_[0] -> Code (q{$ENV{%&*\'var%} = "} . HTML::Embperl::Syntax::SSI::InterpretVars (%&\'value%) . \'";\') ;',
                              removenode => 1 
                                         } ) ;
    $self -> AddComment ('#if', ['expr'], undef, undef, 
                            { perlcode   => '%&\'expr%',
                              compiletimeperlcode => '$_[0] -> Code (q{if (} . HTML::Embperl::Syntax::SSI::InterpretVars (%&\'expr%) . \') {\') ;',
                                removenode  => 10,
                                mayjump     => 1,
                                stackname   => 'ssicmd',
                                'push'      => 'if',
                            } ) ;
    $self -> AddComment ('#elif', ['expr'], undef, undef, 
                            { perlcode   => '%&\'expr%',
                              compiletimeperlcode => '$_[0] -> Code (\'} elsif (\' . HTML::Embperl::Syntax::SSI::InterpretVars (%&\'expr%) . \') {\') ;',
                            removenode => 10,
                            mayjump     => 1,
                            stackname   => 'ssicmd',
                            stackmatch  => 'if',
                            'push'      => 'if',
                            } ) ;
    $self -> AddComment ('#else', undef, undef, undef, 
                            { perlcode   => '} else {',
                            removenode => 10,
                            mayjump     => 1,
                            stackname   => 'ssicmd',
                            stackmatch  => 'if',
                            'push'      => 'if',
                            } ) ;
    $self -> AddComment ('#endif', undef, undef, undef, 
                            { perlcode   => '} ;',
                            removenode => 10,
                            mayjump     => 1,
                            stackname   => 'ssicmd',
                            stackmatch  => 'if',
                            } ) ;
    my $tag = $self -> AddComment ('#syntax', ['type'], undef, undef, 
                { 
                compiletimeperlcode => '$_[0] -> Syntax (HTML::Embperl::Syntax::GetSyntax(%&\'type%, $_[0] -> SyntaxName));', 
                removenode => 3,
                },
                 ) ;
    my $ptcode = '$_[0] -> Syntax (HTML::Embperl::Syntax::GetSyntax(\'%%\', $_[0] -> SyntaxName)) ;' ;
    
    if (!$self -> {-ssiAssignAttrType})
        {
        $self -> {-ssiAssignAttrType}     = $self -> CloneHash ($self -> {-htmlAssignAttr}) ;
        }
    $tag -> {inside}{type}{'follow'} = $self -> {-ssiAssignAttrType} ;
    $self -> {-ssiAssignAttrType}{Assign}{follow}{'Attribut ""'}{parsetimeperlcode} = $ptcode ;
    $self -> {-ssiAssignAttrType}{Assign}{follow}{'Attribut \'\''}{parsetimeperlcode} = $ptcode ;
    $self -> {-ssiAssignAttrType}{Assign}{follow}{'Attribut alphanum'}{parsetimeperlcode} = $ptcode ;
 

    }


###################################################################################
#
#   SSI Implementation
#
###################################################################################

# ---------------------------------------------------------------------------------
#
#   Init SSI
#
# ---------------------------------------------------------------------------------

sub InitSSI
    {
    $epreq = shift ;
    $apreq = shift ;

    my $fn ;

    $ENV{DATE_GMT}      = gmtime ;
    $ENV{DATE_LOCAL}    = localtime ;
    $ENV{DOCUMENT_NAME} = $fn = basename ($epreq -> Sourcefile) ;
    $ENV{DOCUMENT_URI}  = $apreq?$apreq -> uri:'' ;
    $ENV{LAST_MODIFIED} = format_time('', (stat ($fn))[9])
    }
     

# ---------------------------------------------------------------------------------
#
#   Interpolate vars inside string
#
# ---------------------------------------------------------------------------------


sub InterpretVars

    {
    my $val = shift ;
    my $esc = shift ;
    $val =~ s/\$(\w)([a-zA-Z0-9_]*)/\$ENV{'$1$2'}/g ;
    $val =~ s/\$\{(\w)([a-zA-Z0-9_]*?)\}/\$ENV{'$1$2'}/g ;
    $val =~ s/\'/\\\'/g if ($esc) ;
    return $val ;
    }

# ---------------------------------------------------------------------------------
#
#   Find a file
#
# ---------------------------------------------------------------------------------

sub find_file 
    {
    my ($fn, $virt) = @_;
    my $req;

    if (!defined ($apreq))
        {
        return $fn if ($fn);
        return $ENV{DOCUMENT_NAME} if (!$virt) ;

	my $filename = $virt;
        
	#die "Cannot use 'virtual' without mod_perl" if ($virt) ;

	if ($filename =~ /^\//) {
		$filename = $ENV{DOCUMENT_ROOT} . $filename;
	}
	return $filename;
        }

    if ($fn) 
        {
        my $req = $apreq -> lookup_file (InterpretVars ($fn)) ;
        return $req -> filename ;
        }
    if ($virt) 
        {
        my $req = $apreq -> lookup_uri (InterpretVars ($virt)) ;
        return $req -> filename ;
        }
    else
        {
        return $ENV{DOCUMENT_NAME} || $ENV{SCRIPT_FILENAME};
        }
    }


# ---------------------------------------------------------------------------------
#
#   Format time
#
# ---------------------------------------------------------------------------------

sub time_args 

    {
    # This routine must respect the caller's wantarray() context.
    my ($time, $zone) = @_;
    return $zone =~ /GMT/ ? gmtime($time) : localtime($time);
    }


sub format_time 
  {
  my ($format, $time, $tzone) = @_;
  return ($format ? 
	  POSIX::strftime($format, time_args($time, $tzone)) :
	  scalar time_args($time, $tzone));
  }




# ---------------------------------------------------------------------------------
#
#   Output fsize
#
# ---------------------------------------------------------------------------------



sub fsize
   
    { 
    my ($fmt, $fn, $virt) = @_;
    
    my $size = -s find_file($fn, $virt) ;
    
    $fmt ||= 'abbrev' ;

    if ($fmt eq 'bytes')
         {
         return $size;
         }
    elsif ($fmt eq 'abbrev') 
        {
        return "   0k" unless $size;
        return "   1k" if $size < 1024;
        return sprintf("%4dk", ($size + 512)/1024) if $size < 1048576;
        return sprintf("%4.1fM", $size/1048576.0)  if $size < 103809024;
        return sprintf("%4dM", ($size + 524288)/1048576);
        } 
    else 
        {
        die "Unrecognized size format '$fmt'" ;
        }
    }

# ---------------------------------------------------------------------------------
#
#   Output flastmod
#
# ---------------------------------------------------------------------------------

sub flastmod 
    {
    my($fmt, $fn, $virt) = @_;
    
    return format_time($fmt, (stat (find_file($fn, $virt)))[9])
    }

# ---------------------------------------------------------------------------------
#
#   Include
#
# ---------------------------------------------------------------------------------

sub include {
	my($fn, $virt) = @_;
				
	local $/ = undef ;

	my $type = "SSI"; # adding Embperl to syntax results in errors I can't figure out...
	my $filename = $virt;

	if ($fn) {
		$type = "Text";
		$filename = $fn;
	}
	elsif ($virt) {
		if ($filename =~ /^\//) {
			$filename = $ENV{DOCUMENT_ROOT} . "/$filename";
		}
	}
	else {
		warn "Nothing to #include... need file or virtual";
		return "";
	}

	my $output = "";
	HTML::Embperl::Execute({inputfile=>$filename, output=>\$output, syntax=>$type});

        local $HTML::Embperl::escmode = 0 ;
	return $output;
}

# ---------------------------------------------------------------------------------
#
#   Exec
#
# ---------------------------------------------------------------------------------


sub exec 
    {
    my($cmd, $cgi) = @_;

    if (!defined (&Apache::request))
        {
        return scalar `$cmd` if ($cmd) ;
        die "Cannot use 'cgi' without mod_perl" ;
        }

    my $r = $apreq ;
    my $filename = $r->filename;
    
    die ("httpd: exec used but not allowed in $filename") if ($r->allow_options & &OPT_INCNOEXEC) ;
    
    return scalar `$cmd` if ($cmd) ;
    
    die ("No 'cmd' or 'cgi' argument given to #exec") if (!$cgi) ;

    die ("'cgi' as argument to #exec not implemented yet") ;

    # Okay, we're doing <!--#exec cgi=...>
    my $rr = $r->lookup_uri($cgi);
    die("Error including cgi: subrequest returned status '" . $rr->status . "', not 200") if ($rr->status != 200);
    
    # Pass through our own path_info and query_string (does this work?)
    $rr->path_info( $r->path_info );
    $rr->args( scalar $r->args );
    $rr->content_type("application/x-httpd-cgi");
    &_set_VAR($rr, 'DOCUMENT_URI', $r->uri);
    
    my $status = $rr->run;
    return '';
    }


1; 

__END__

=pod

=head1 NAME

SSI syntax module for Embperl 

=head1 SYNOPSIS

 [$ syntax SSI $]
 
 DATE_GMT:       <!-- #echo  var='DATE_GMT' -->
 DATE_LOCAL:	<!-- #echo  var='DATE_LOCAL' --> 
 DOCUMENT_NAME:	<!-- #echo  var='DOCUMENT_NAME' -->
 DOCUMENT_URI:	<!-- #echo  var='DOCUMENT_URI' -->
 LAST_MODIFIED:	<!-- #echo  var='LAST_MODIFIED' -->


=head1 DESCRIPTION

The module make Embperl understand the following SSI tags. See
Apaches mod_include (or Apache::SSI) for a description, what they
do.

=over 4

=item * config

=item * echo

=item * exec

=item * fsize

=item * flastmod

=item * include

=item * printenv

=item * set

=item * if

=item * elif

=item * else

=item * endif

=item * syntax

The syntax SSI is non standard and is used to change the syntax once you are
in SSI syntax. It looks like

  <!--#syntax type="Embperl" -->

=back


=head1 Author

Gerald Richter <richter@dev.ecos.de>

Some ideas and parts of the code are taken from Apache::SSI by Ken Williams. 

=head1 See Also

HTML::Embperl::Syntax, HTML::Embperl::Syntax::HTML

=cut



# ---------------------------------------------------------------------------------
#
#   Perl
#
# ---------------------------------------------------------------------------------



sub perl 
    {
    my($self, $args, $margs) = @_;

    my ($pass_r, @arg1, @arg2, $sub) = (1);
    {
        my @a;
        while (@a = splice(@$margs, 0, 2)) {
            $a[1] =~ s/\\(.)/$1/gs;
            if (lc $a[0] eq 'sub') {
                $sub = $a[1];
            } elsif (lc $a[0] eq 'arg') {
                push @arg1, $a[1];
            } elsif (lc $a[0] eq 'args') {
                push @arg1, split(/,/, $a[1]);
            } elsif (lc $a[0] eq 'pass_request') {
                $pass_r = 0 if lc $a[1] eq 'no';
            } elsif ($a[0] =~ s/^-//) {
                push @arg2, @a;
            } else { # Any unknown get passed as key-value pairs
                push @arg2, @a;
            }
        }
    }

    warn "sub is $sub, args are @arg1 & @arg2" if $debug;
    my $subref;
    if ( $sub =~ /^\s*sub\s/ ) {     # for <!--#perl sub="sub {print ++$Access::Cnt }" -->
        $subref = eval($sub);
        if ($@) {
            $self->error("Perl eval of '$sub' failed: $@") if $self->{_r};
            warn("Perl eval of '$sub' failed: $@") unless $self->{_r};  # For offline mode
        }
        return $self->error("sub=\"sub ...\" didn't return a reference") unless ref $subref;
    } else {             # for <!--#perl sub="package::subr" -->
        no strict('refs');
	$subref = (defined &{$sub} ? \&{$sub} :
		   defined &{"${sub}::handler"} ? \&{"${sub}::handler"} : 
		   \&{"main::$sub"});
    }
    
    $pass_r = 0 if $self->{_r} and lc $self->{_r}->dir_config('SSIPerlPass_Request') eq 'no';
    unshift @arg1, $self->{_r} if $pass_r;
    warn "sub is $subref, args are @arg1 & @arg2" if $debug;
    return scalar &{ $subref }(@arg1, @arg2);
}


1 ;



