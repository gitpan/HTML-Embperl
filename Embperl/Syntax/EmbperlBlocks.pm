
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
#   $Id: EmbperlBlocks.pm,v 1.1.2.18 2001/10/30 14:40:16 richter Exp $
#
###################################################################################
 


package HTML::Embperl::Syntax::EmbperlBlocks ;

use HTML::Embperl::Syntax (':types') ;

use strict ;
use vars qw{@ISA %Blocks %BlocksOutput %BlocksOutputLink} ;



@ISA = qw(HTML::Embperl::Syntax) ;


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

    $self = HTML::Embperl::Syntax::new ($self) ;

    if (!$self -> {-epbBlocks})
        {
        $self -> {-epbBlocks}     = $self -> CloneHash ({ %Blocks, %BlocksOutput }) ;
        $self -> {-epbBlocksLink} = $self -> CloneHash ({ %Blocks, %BlocksOutputLink }, { 'unescape' => 2 }) ;

        $self -> AddToRoot ($self -> {-epbBlocks}) ;

        Init ($self) ;
        }

    return $self ;
    }

# ---------------------------------------------------------------------------------
#
#   Add new meta command
#
# ---------------------------------------------------------------------------------


sub AddMetaCmd

    {
    my ($self, $cmdname, $procinfo, $taginfo) = @_ ;

    my $tagtype = 'Embperl meta command' ;
    my $ttref ;
    die "'$tagtype' unknown" if (!($ttref = $self -> {-epbBlocks}{$tagtype})) ;
    my $ttfollow = ($ttref -> {'follow'} ||= {}) ;

    my $tag = $ttfollow -> {$cmdname} = { 
                                'text'      => $cmdname,
                                'nodetype'  => ntypTag,
                                'cdatatype' => ntypAttrValue,
                                'forcetype' => 1,
                                'unescape'  => 1,
                                (ref($taginfo) eq 'HASH'?%$taginfo:()),
                              } ;
    $tag -> {'procinfo'} = { $self -> {-procinfotype} => $procinfo } if ($procinfo) ;

    die "'$tagtype' unknown" if (!($ttref = $self -> {-epbBlocksLink}{$tagtype})) ;
    $ttfollow = ($ttref -> {'follow'} ||= {}) ;

    my $tag2 = $ttfollow -> {$cmdname} = { 
                                'text'      => $cmdname,
                                'nodetype'  => ntypTag,
                                'cdatatype' => ntypAttrValue,
                                'forcetype' => 1,
                                'unescape'  => 2,
                                (ref($taginfo) eq 'HASH'?%$taginfo:()),
                              } ;
    $tag2 -> {'procinfo'} = { $self -> {-procinfotype} => $procinfo } if ($procinfo) ;

    return $tag ;
    }


# ---------------------------------------------------------------------------------
#
#   Add new meta command that has an corresponding end meta command
#
# ---------------------------------------------------------------------------------


sub AddMetaCmdWithEnd

    {
    my ($self, $cmdname, $endname, $procinfo) = @_ ;

    my $tag = $self -> AddMetaCmd ($cmdname, $procinfo) ;

    $tag -> {'endtag'} = $endname ;

    return $tag ;
    }

# ---------------------------------------------------------------------------------
#
#   Add new meta command with start and end
#
# ---------------------------------------------------------------------------------


sub AddMetaCmdBlock

    {
    my ($self, $cmdname, $endname, $procinfostart, $procinfoend) = @_ ;

    my $tag ;
    my $pinfo ;

    $tag = $self -> AddMetaCmd ($cmdname, $procinfostart) ;
    $tag -> {'endtag'} = $endname ;
    $pinfo = $tag -> {'procinfo'} -> {$self -> {-procinfotype}} ;
    $pinfo -> {'stackname'} = 'metacmd' ;
    $pinfo -> {'push'} = $cmdname ;

    $tag = $self -> AddMetaCmd ($endname, $procinfoend) ;
    $pinfo = $tag -> {'procinfo'} -> {$self -> {-procinfotype}} ;
    $pinfo -> {'stackname'} = 'metacmd' ;
    $pinfo -> {'stackmatch'} = $cmdname ;
    

    return $tag ;
    }

# ---------------------------------------------------------------------------------
#
#   Add new block 
#
# ---------------------------------------------------------------------------------


sub AddMetaStartEnd 

    {
    my ($self, $cmdname, $endname, $procinfostart) = @_ ;

    my $tag ;
    my $pinfo ;

    $tag = $self -> AddMetaCmd ($cmdname, $procinfostart) ;
    $tag -> {'nodetype'} = &ntypStartTag ;


    $tag = $self -> AddMetaCmd ($endname) ;
    $tag -> {'nodetype'} = &ntypEndTag ;
    $tag -> {'starttag'} = $cmdname ;
    

    return $tag ;
    }


    

# ---------------------------------------------------------------------------------
#
#   Add new simple html tag (override to add meta commands inside html tags)
#
# ---------------------------------------------------------------------------------


sub AddTag

    {
    my $self = shift ;

    my $tag = $self -> HTML::Embperl::Syntax::HTML::AddTag (@_) ;

    #### add the Embperl Block inside the new HTML Tag ####

    $tag -> {inside} ||= {} ;
    my $inside = $tag -> {inside} ;    

    while (my ($k, $v) = each (%{$self -> {-epbBlocks}}))
        {
        $inside -> {$k} = $v ;
        }

    if (!$self -> {-epbHTMLInit})
        {
        #### if not already done add the Embperl Block inside the HTML Attributes ####

        $self -> {-epbHTMLInit} = 1 ;

        my $unescape = 0 ;
        foreach ('', 'Link')
            {
            my $attr   = $self -> {"-htmlAssignAttr$_"} ;
            my $blocks = $self -> {"-epbBlocks$_"} ;
            while (my ($k1, $v1) = each %$attr)
                {
                if (!($k1 =~ /^-/) && ref ($v1) eq 'HASH')
                    {
                    my $follow = $v1 -> {follow} ;
                    if (ref($follow) eq 'HASH')
                        {
                        while (my ($k2, $v2) = each %$follow)
                            {
                            if (ref($v2) eq 'HASH')
				{	  
				$v2 -> {inside} ||= {} ;
                            	my $inside = $v2 -> {inside} ;

	                        while (my ($k, $v) = each (%$blocks))
                                    {
                                    $inside -> {$k} = $v ;
                                    }
				}
                            }
                        }
                    }
                }
            }

        my $quotes = $self -> {"-htmlQuotes"} ;
        my $blocks = $self -> {"-epbBlocks"} ;
        while (my ($k2, $v2) = each %$quotes)
            {
            if (ref($v2) eq 'HASH')
		{	  
		$v2 -> {inside} ||= {} ;
                my $inside = $v2 -> {inside} ;

	        while (my ($k, $v) = each (%$blocks))
                    {
                    $inside -> {$k} = $v ;
                    }
		}
            }
        }
    return $tag ;
    }



###################################################################################
#
#   Definitions for Embperl Blocks
#
###################################################################################

sub Init

    {
    my ($self) = @_ ;

    $self -> AddMetaCmdWithEnd ('if', 'endif', 
                            {
                            perlcode    => 'if (%&<noname>%) { ', 
                            removenode  => 10,
                            mayjump     => 1,
                            stackname   => 'metacmd',
                            'push'      => 'if',
                            }) ;

    $self -> AddMetaCmdWithEnd  ('else', 'endif', 
                            { 
                            perlcode => '} else {',
                            removenode => 10,
                            mayjump     => 1,
                            stackname   => 'metacmd',
                            stackmatch  => 'if',
                            'push'      => 'if',
                            }) ;
    $self -> AddMetaCmdWithEnd  ('elsif', 'endif',
                            { 
                            perlcode => '} elsif (%&<noname>%) { ', 
                            removenode => 10,
                            mayjump     => 1,
                            stackname   => 'metacmd',
                            stackmatch  => 'if',
                            'push'      => 'if',
                            }) ;
    $self -> AddMetaCmd ('endif',
                            { 
                            perlcode => '}', 
                            removenode => 10,
                            mayjump     => 1,
                            stackname   => 'metacmd',
                            stackmatch  => 'if',
                            }) ;
    $self -> AddMetaCmdBlock  ('while', 'endwhile', 
                { 
                perlcode => 'while (%&<noname>%) { ', 
                removenode => 10,
                mayjump     => 1,
                },
                { 
                perlcode => '};', 
                removenode => 10,
                mayjump     => 1,
                }) ;
    $self -> AddMetaCmdBlock  ('foreach', 'endforeach',
                { 
                perlcode => 'foreach %&<noname>% { ', 
                removenode => 10,
                mayjump     => 1,
                },
                { 
                perlcode => '};', 
                removenode => 10,
                mayjump     => 1,
                }) ;
    $self -> AddMetaCmdBlock  ('do', 'until',
                { 
                perlcode => 'do { ', 
                removenode => 10,
                mayjump     => 1,
                },
                { 
                perlcode => '} until (%&<noname>%) ; ',
                removenode => 10,
                mayjump     => 1,
                }) ;
    $self -> AddMetaCmd ('var',
                { 
                compiletimeperlcode => 'use strict ; use vars qw{%%CLEANUP %&<noname>%} ; map { $CLEANUP{substr($_,1)} = 1 } qw{%&<noname>%} ;', 
                perlcode => 'use strict ;', 
                removenode => 3,
                }) ;
    $self -> AddMetaCmd ('hidden',
                { 
                perlcode => '_ep_hid(%$n%,%&\'<noname>%);', 
                removenode => 8,
                }) ;
    $self -> AddMetaCmd ('syntax',
                { 
                compiletimeperlcode => '$_[0] -> Syntax (HTML::Embperl::Syntax::GetSyntax(%&\'<noname>%, $_[0] -> SyntaxName));', 
                removenode => 3,
                },
                { 
                parsetimeperlcode => '$_[0] -> Syntax (HTML::Embperl::Syntax::GetSyntax(\'%%\', $_[0] -> SyntaxName)) ;', 
                },
                ) ;
    $self -> AddMetaStartEnd ('sub', 'endsub',
                { 
                perlcode => 'sub _ep_sub_%&<noname>% { ', 
                perlcodeend => '};  sub %^subname% { my @_ep_save ; HTML::Embperl::Cmd::SubStart($_ep_DomTree,%$q%,\\@_ep_save); my $_ep_ret = _ep_sub_%^subname% (@_); HTML::Embperl::Cmd::SubEnd($_ep_DomTree,\\@_ep_save); return $_ep_ret } ; $_[0] -> ExportHash -> {%^"subname%} = \&%^subname% ; ', 
                removenode => 10,
                mayjump     => 1,
                stackname2   => 'subname',
                push2        => '%&<noname>%',
                switchcodetype => 2,
                callreturn => 1,
                },
                { 
                perlcode => '};  sub %^subname% { my @_ep_save ; HTML::Embperl::Cmd::SubStart($_ep_DomTree,%$q%,\\@_ep_save); my $_ep_ret = _ep_sub_%^subname% (@_); HTML::Embperl::Cmd::SubEnd($_ep_DomTree,\\@_ep_save); return $_ep_ret } ; $_[0] -> ExportHash -> {%^"subname%} = \&%^subname% ; ', 
                removenode => 10,
                mayjump     => 0,
                pop2        => 'subname',
                switchcodetype => 1,
                callreturn => 1,
                }) ;
=pod
    $self -> AddMetaStartEnd ('sub', 'endsub',
                { 
                perlcode => 'sub _ep_sub_%&<noname>% { ', 
                removenode => 10,
                mayjump     => 1,
                stackname2   => 'subname',
                push2        => '%&<noname>%',
                switchcodetype => 2,
                callreturn => 1,
                },
                { 
                perlcode => '};  sub %^subname% { my @_ep_save ; HTML::Embperl::Cmd::SubStart($_ep_DomTree,%$q%,\\@_ep_save); my $_ep_ret = _ep_sub_%^subname% (@_); HTML::Embperl::Cmd::SubEnd($_ep_DomTree,\\@_ep_save); return $_ep_ret } ; $_[0] -> ExportHash -> {%^"subname%} = \&%^subname% ; ', 
                removenode => 10,
                mayjump     => 0,
                pop2        => 'subname',
                switchcodetype => 1,
                callreturn => 1,
                }) ;
=cut
    } 



%Blocks = (
    '-lsearch' => 1,
    'Embperl command escape' => {
        'text' => '[[',
        'nodename' => '[',
        'nodetype' => ntypCDATA,
        },
    'Embperl meta command' => {
        'text' => '[$',
        'end'  => '$]',
        'unescape' => 1,
        },
     'Embperl code' => {
        'text' => '[-',
        'end'  => '-]',
        'unescape' => 1,
        'procinfo' => {
            embperl => { 
                        perlcode    => [
                                '%$c%if (!defined (scalar(do{' . "\n" . '%#~0:$col%' . "\n" . '}))) %#~-0:$row% { if ($col == 0) { _ep_dcp (%^*htmltable%) ; last l%^*htmltable% ; } else { _ep_dcp (%^*htmlrow%) ; last l%^*htmlrow% ; }}',
                                '%$c%if (!defined (scalar(do{' . "\n" . '%#~0:$col%' . "\n" . '}))) { _ep_dcp (%^*htmlrow%) ; last l%^*htmlrow% ; }',
                                '%$c%if (!defined (scalar(do{' . "\n" . '%#~0:$row%' . "\n" . '}))) { _ep_dcp (%^*htmltable%) ; last l%^*htmltable% ; }',
                                '%$c%{' . "\n" . '%#0%' . "\n" . ';}',
                                ],
                        removenode  => 3,
                        mayjump     => 1,
                        compilechilds => 0,
                        },
            },
        },
     'Embperl global code' => {
        'text' => '[*',
        'end'  => '*]',
        'unescape' => 1,
        'procinfo' => {
            embperl => { 
                        perlcode    => '%$c%' . "\n" . '%#0%',
                        removenode  => 3,
                        mayjump     => 1,
                        compilechilds => 0,
                        },
            },
        },
     'Embperl startup code' => {
        'text' => '[!',
        'end'  => '!]',
        'unescape' => 1,
        'procinfo' => {
            embperl =>  { 
                        compiletimeperlcode => '%#0%;',
                        removenode  => 3,
                        compilechilds => 0,
                        }
            },
        },
     'Embperl comment' => {
        'text' => '[#',
        'end'  => '#]',
#        'inside' => \%MetaComment,
        'procinfo' => {
            embperl => { 
                compilechilds => 0,
                removenode  => 3, 
                },
            },
        },
      ) ;  
   
#%MetaComment = (
#    '-lsearch' => 1,
#     'Embperl comment' => {
#        'text' => '[#',
#        'end'  => '#]',
#        'inside' => \%MetaComment
#        },
#) ;


%BlocksOutput =
    (
     'Embperl output code' => {
        'text' => '[+',
        'end'  => '+]',
        'unescape' => 1,
        'procinfo' => {
            embperl => { 
                    perlcode => 
                        [
                        'if (!defined (_ep_rp(%$x%,scalar(%#~0:$col%)))) %#~-0:$row% { if ($col == 0) { _ep_dcp (%^*htmltable%) ; last l%^*htmltable% ; } else { _ep_dcp (%^*htmlrow%) ; last l%^*htmlrow% ; }}',
                        'if (!defined (_ep_rp(%$x%,scalar(%#~0:$col%)))) { _ep_dcp (%^*htmlrow%) ; last l%^*htmlrow% ; }',
                        'if (!defined (_ep_rp(%$x%,scalar(%#~0:$row%)))) { _ep_dcp (%^*htmltable%) ; last l%^*htmltable% ; }',
                        '_ep_rp(%$x%,scalar(%#0%));', 
			],
                    removenode  => 4,
                    mayjump => '%#~0:$col|$row|$cnt% %?*htmlrow% %?*htmltable%',
                    compilechilds => 0,
                    }
            },
        },
    ) ;

%BlocksOutputLink =
    (
     'Embperl output code URL' => {
        'text' => '[+',       
        'nodename' => '[+url',
        'end'  => '+]',
        'unescape' => 2,
        'procinfo' => {
            embperl => { 
                    perlcode => 
                        [
                        'if (!defined (_ep_rpurl(%$x%,scalar(do{%#~0:$col%})))) %#~-0:$row% { if ($col == 0) { _ep_dcp (%^*htmltable%) ; last l%^*htmltable% ; } else { _ep_dcp (%^*htmlrow%) ; last l%^*htmlrow% ; }}',
                        'if (!defined (_ep_rpurl(%$x%,scalar(do{%#~0:$col%})))) { _ep_dcp (%^*htmlrow%) ; last l%^*htmlrow% ; }',
                        'if (!defined (_ep_rpurl(%$x%,scalar($val3=do{%#~0:$row%;})))) {  _ep_dcp (%^*htmltable%) ; last l%^*htmltable% ; }',
                        '_ep_rpurl(%$x%,scalar(do{%#0%}));', 
                        ],
                    removenode  => 4,
                    mayjump => '%#~0:$col|$row|$cnt% %?*htmlrow% %?*htmltable%',
                    compilechilds => 0,
                    }
            },
        },
    ) ;


1;


__END__

=pod

=head1 NAME

HTML::Embperl::Syntax::EmbperlBlocks

=head1 SYNOPSIS


=head1 DESCRIPTION

Class derived from HTML::Embperl::Syntax to define the syntax for 
Embperl Blocks and metacommands.

=head1 Methods

I<HTML::Embperl::Syntax::EmbperlBlocks> defines the following methods:

=head2 HTML::Embperl::Syntax::EmbperlBlocks -> new  /  $self -> new

Create a new syntax class. This method should only be called inside a constructor
of a derived class.


=head2 AddMetaCmd ($cmdname, $procinfo)

Add a new metacommand with name C<$cmdname> and use processor info from
C<$procinfo>. See I<HTML::Embperl::Syntax> for a definition of procinfo.

=head2 AddMetaCmdWithEnd ($cmdname, $endname, $procinfo)

Add a new metacommand with name C<$cmdname> and use processor info from
C<$procinfo>. Addtionaly specify that a matching C<$endname> metacommand
must be found to end the block, that is started by this metacommand.
See I<HTML::Embperl::Syntax> for a definition of procinfo.

=head2 AddMetaCmdBlock ($cmdname, $endname, $procinfostart, $procinfoend)

Add a new metacommand with name C<$cmdname> and and a second metacommand
C<$endname> which ends the block that is started by C<$cmdname>.
Use processor info from C<$procinfo>.
See I<HTML::Embperl::Syntax> for a definition of procinfo.



=head1 Author

G. Richter (richter@dev.ecos.de)

=head1 See Also

HTML::Embperl::Syntax


