
###################################################################################
#
#   Embperl - Copyright (c) 1997-1999 Gerald Richter / ECOS
#
#   Apache::Session::epDBI - workround to make Apache::Session 0.17 work with Embperl
#
#   You may distribute under the terms of either the GNU General Public
#   License or the Artistic License, as specified in the Perl README file.
#   For use with Apache httpd and mod_perl, see also Apache copyright.
#
#   THIS PACKAGE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR
#   IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
#   WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
#
###################################################################################


package Apache::Session::epDBI;
use Apache::Session::DBI ();

@ISA = qw(Apache::Session::DBI);
$VERSION = '0.01';

use strict;
use vars qw{ $org_glock $warn } ;

use constant WAIT      => ($ENV{'EMBPERL_SESSION_LOCK_WAIT'} || 0.1) ;
use constant TIMEOUT   => (($ENV{'EMBPERL_SESSION_LOCK_TIMEOUT'} || 20) / WAIT) ;


BEGIN
    {
    #
    # since the glock of Apache::Session::DBI is not call as an object method
    # we need to replace it
    #
    # get a pointer to the old glock
    #

    $org_glock = \&Apache::Session::DBI::glock ;

    # turn of redefinition warings
    $warn = $^W ;
    $^W = 0 ; 
    }



sub Apache::Session::DBI::glock 
    {
    my $id = shift;
    return undef unless $id;
    my $rc ;
    my $timeout = TIMEOUT ;

    #warn "try lock $id" ;
    while (!($rc = &{$org_glock} ($id)))
        {
        return undef if (--$timeout == 0) ; # fail if timeout

        select (undef, undef, undef, WAIT) ; # wait a little bit
        }

    #warn "try lock $id returned $rc" ;
    return $rc ;
    }
    
BEGIN
    {
    # restore warings
    $^W = $warn ; 
    }




######################

sub fetch 
    {
    my $class = shift;
    my $id    = shift;

    #warn "fetch $id" ;
    my $self = $class -> SUPER::fetch ($id) ; # do real fetch

    if ($self) 
        {
        $self -> {'_ID'} = $id ;   # save id for later unlock
        }
    #else
    #    {
    #    #warn "fecth returns $self" ;
    #    }

    return $self ;
    }


######################

sub create
    {
    my $class = shift;
    my $id    = shift;

    #warn "create $id" ;
    my $self = $class -> SUPER::create ($id) ; # do real create

    $self -> {'_ID'} = $id  if ($self) ; # save id for later unlock

    return $self ;
    }


1;

__END__

=head1 NAME

Apache::Session::epDBI - Store client sessions in your DBMS (patch for Embperl)

=head1 SYNOPSIS

use Apache::Session::epDBI

=head1 DESCRIPTION

This is a patched DBI storage subclass for Apache::Session::DBI.  Client state is stored
in a database via the DBI module.  Look at C<perldoc Apache::Session::DBI> for
a description.

This module makes sure that Apache::Session::DBI works correctly together with 
HTML::Embperl. Additionaly it serializes all requests to the same ID. Look at
Apache::Session::DBI and HTML::Embperl for more documentation.

If you see warnings about 'Lock for session xxx failed...', this means that currently another
request is accessing the same session and Apache::Session::epDBI is waiting until either the 
first request releases the lock or the timeout valus is reached.


=head1 ADDTIONAL CONFIGURATION VARS

EMBPERL_SESSION_LOCK_WAIT (default 0.1)

Time in seconds to wait between two reties to lock the same ID


EMBPERL_SESSION_LOCK_TIMEOUT (default 20) 

Time in seconds afters which the lock fails.


=head1 NOTE

This is only a workaround for Apache::Session 0.17. The next release of
Apache::Session will (hopefully) work with Embperl and serializes all request
on it own.

=head1 AUTHORS

Gerald Richter <richter@dev.ecos.de> 

Redistribute under the Perl Artistic License.

