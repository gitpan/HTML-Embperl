package HTML::Embperl::Syntax::Test;

use strict;
use HTML::Embperl::Syntax qw{:types} ;
use HTML::Embperl::Syntax::HTML ;
use base qw(HTML::Embperl::Syntax::HTML);


sub new 
    {
    my $class = shift;
    my $self = HTML::Embperl::Syntax::HTML::new($class);

    if (!$self->{-testInit}) 
        {
        $self->{-testInit} = 1;
        init($self);
        }
    return $self;
    }

sub init {
  my $self = shift;

  $self->AddTagInside('testname', ['type'], undef, undef, {
                perlcode => q[{
                                _ep_rp(%$x%, "test syntax %&'type%");
                }]
  });

}


1;
