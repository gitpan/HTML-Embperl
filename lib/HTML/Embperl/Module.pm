
package HTML::Embperl::Module ;

use HTML::Embperl ;


# define subs

sub init
    {
    local $/ = undef ;
    my $hdl  = shift ;
    my $data = <$hdl> ;

    # compile page

    HTML::Embperl::Execute ({'inputfile' => __FILE__, 
			    'input' => \$data,
			    'mtime' => -M __FILE__ ,
			    'import' => 0,
			    'options' => HTML::Embperl::optKeepSrcInMemory,
			    'package' => __PACKAGE__}) ;


    }

# import subs

sub import

    {
    HTML::Embperl::Execute ({'inputfile' => __FILE__, 
			    'import' => 2,
			    'package' => __PACKAGE__}) ;


    1 ;
    }



1 ;
