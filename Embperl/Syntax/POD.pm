
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
#   $Id: POD.pm,v 1.1.2.4 2001/09/20 13:27:31 richter Exp $
#
###################################################################################
 


package HTML::Embperl::Syntax::POD ;

use HTML::Embperl::Syntax (':types') ;
use HTML::Embperl::Syntax::EmbperlBlocks ;

use strict ;
use vars qw{@ISA %Tags %Format %Escape} ;



@ISA = qw(HTML::Embperl::Syntax::EmbperlBlocks) ;


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
    my $self = shift ;

    #$self = HTML::Embperl::Syntax::EmbperlBlocks::new ($self, 1) ;
    $self = HTML::Embperl::Syntax::new ($self, 1) ;

    if (!$self -> {-PODTags})
        {
	$self -> {-PODTags}	  = $self -> CloneHash (\%Tags) ;

	$self -> AddToRoot ($self -> {-PODTags}) ;
	$self -> AddToRoot ({'-defnodetype' => ntypText,}) ;
    

	$self -> {-PODCmds} = $self -> {-PODTags}{'POD Command'}{'follow'} ;
	Init ($self) ;
        }

    return $self ;
    }

# ---------------------------------------------------------------------------------
#
#   Add new POD command
#
# ---------------------------------------------------------------------------------


sub AddPODCmd

    {
    my ($self, $cmdname, $name) = @_ ;

    my $ttfollow = $self -> {-PODCmds} ;

    my $tag = $ttfollow -> {$cmdname} = { 
                                'text'      => $cmdname,
                                'nodetype'  => ntypStartEndTag,
                                'cdatatype' => ntypText,
                                'removespaces'  => 72,
                                'inside'  => \%Format,
                              } ;
    $tag -> {nodename} = $name if ($name) ;

    return $tag ;
    }


sub AddPODStartEnd

    {
    my ($self, $start, $end, $name) = @_ ;

    my $ttfollow = $self -> {-PODCmds} ;

    my $stag = $ttfollow -> {$start} = { 
                                'text'      => $start,
                                'nodetype'  => ntypStartTag,
                                'cdatatype' => 0,
                                'removespaces'  => 72,
                              } ;
    $stag -> {nodename} = $name if ($name) ;

    my $etag = $ttfollow -> {$end} = { 
                                'text'      => $end,
                                'nodetype'  => ntypEndTag,
                                'cdatatype' => 0,
                                'starttag'  => $start,
                                'removespaces'  => 72,
                              } ;
    return $stag ;
    }




###################################################################################
#
#   Definitions for POD
#
###################################################################################

sub Init

    {
    my ($self) = @_ ;

    $self -> AddPODCmd ('head1') ;
    $self -> AddPODCmd ('head2') ;
    $self -> AddPODCmd ('head3') ;
    $self -> AddPODStartEnd ('over', 'back', 'list') ;
    $self -> AddPODStartEnd ('pod', 'cut') ;
    $self -> AddPODCmd ('item') ;
    $self -> AddPODCmd ('item *', 'bulletitem') ;
    } 


%Escape = (
    '-lsearch' => 1,
    'POD Escape' => {
	'text' => '<',
	'end'  => '>',
       'nodename' => ':::&gt;:&lt;',
        'nodetype'  => ntypStartEndTag,
        },
   'POD Escape &' => {
	'text' => '&',
        'nodename' => ':::&amp;',
        'nodetype'  => ntypTag,
        },
) ;

my %Escape2 = (

    'POD Escape <' => {
	'text' => '<',
        'nodename' => ':::&lt;',
        'nodetype'  => ntypTag,
        },
    'POD Escape >' => {
	'text' => '>',
        'nodename' => ':::&gt;',
        'nodetype'  => ntypTag,
        },
   'POD Escape &' => {
	'text' => '&',
        'nodename' => ':::&amp;',
        'nodetype'  => ntypTag,
        },

    ) ;


%Format = (
    '-lsearch' => 1,
    '-defnodetype' => ntypText,
    'POD Format B' => {
	'text' => 'B<',
	'end'  => '>',
        'nodename' => 'B',
        'nodetype'  => ntypStartEndTag,
        },
    'POD Format C' => {
	'text' => 'C<',
	'end'  => '>',
        'nodename' => 'CODE',
        'nodetype'  => ntypStartEndTag,
        },
    'POD Format I' => {
	'text' => 'I<',
	'end'  => '>',
        'nodename' => 'I',
        'nodetype'  => ntypStartEndTag,
        },
    'POD Format U' => {
	'text' => 'U<',
	'end'  => '>',
        'nodename' => 'U',
        'nodetype'  => ntypStartEndTag,
        },
    'POD Format L' => {
	'text' => 'L<',
	'end'  => '>',
        'nodename' => 'L',
        'nodetype'  => ntypStartEndTag,
        },
    ) ;

%Tags = (
    '-lsearch' => 1,
    '-defnodetype' => ntypText,
    'POD Command' => {
	'text' => '=',
	'end' => "\n",
	'cdatatype' => ntypAttrValue,
	'follow' => {'-lsearch' => 1},
        },
    'POD Emptyline' => {
	'text' => "\n\n",
        'nodename' => 'BR/',
        'nodetype'  => ntypTag,
        },
#    'POD Emptyline' => {
#	'text' => "\r\n\r\n",
#        'nodename' => 'BR/',
#        'nodetype'  => ntypTag,
#        },
    'POD Code' => {
	'text' => "\n ",
	'end' => "\n",
        'nodename' => 'PRE',
        'cdatatype' => ntypText,
        'nodetype'  => ntypStartEndTag,
        'inside'    => \%Format,
        },
    %Format,
    ) ;




1;


__END__

=pod

=head1 NAME

HTML::Embperl::Syntax::POD

=head1 SYNOPSIS


=head1 DESCRIPTION

Documenation is still not written!!!!!!!    


=head1 Methods

I<HTML::Embperl::Syntax::POD> defines the following methods:

=head2 HTML::Embperl::Syntax::POD -> new  /  $self -> new

Create a new syntax class. This method should only be called inside a constructor
of a derived class.


=head2 AddPODCmd ($cmdname, $procinfo)

Add a new POD command with name C<$cmdname> and use processor info from
C<$procinfo>. See I<HTML::Embperl::Syntax> for a definition of procinfo.

=head2 AddPODCmdStartEnd ($cmdname, $endname, $procinfo)

Add a new POD command with name C<$cmdname> and use processor info from
C<$procinfo>. Addtionaly specify that a matching C<$endname> POD command
must be found to end the block, that is started by this POD command.
See I<HTML::Embperl::Syntax> for a definition of procinfo.



=head1 Author

G. Richter (richter@dev.ecos.de)

=head1 See Also

HTML::Embperl::Syntax
