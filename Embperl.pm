
###################################################################################
#
#   Embperl - Copyright (c) 1997 Gerald Richter / ECOS
#
#   You may distribute under the terms of either the GNU General Public
#   License or the Artistic License, as specified in the Perl README file.
#   For use with Apache httpd and mod_perl, see also Apache copyright.
#
#   THIS IS BETA SOFTWARE!
#
#   THIS PACKAGE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR
#   IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
#   WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
#
###################################################################################


package HTML::Embperl;

use Safe;
use IO::Handle ;
use CGI;

require Exporter;
require DynaLoader;

@ISA = qw(Exporter DynaLoader);


$VERSION = '0.19-beta';


bootstrap HTML::Embperl $VERSION;


# Default logfilename

$DefaultLog = '/tmp/embperl.log' ;
$Outputfile = '' ;  # Default to stdout
$LogfileURL = '' ;


%cache = () ;   # cache for evaled code
$package = '' ; # package name for current document




#$^W = TRUE ;




embperl_constants () ;

while (($k, $v) = each (%CONSTANT))
	{
	eval "sub $k { $v ; }" ;
	die "Cannot define constant $k" if ($@) ;
	}


# Color definition for logfile output

%LogFileColors = (
    'EVAL<' => '#FF0000',
    'EVAL>' => '#FF0000',
    'FORM:' => '#0000FF',
    'CMD:'  => '#FF8000',
    'SRC:'  => '#808080',
    'TAB:'  => '#FF0080',
    'INPU:' => '#008040',
                ) ;

#######################################################################################
#
# tie for logfile output
#

    {
    package HTML::Embperl::Log ;


    sub TIEHANDLE 

        {
        my $class ;
        
        return bless \$class, shift ;
        }


    sub PRINT

        {
        shift ;
        HTML::Embperl::embperl_log(join ('', @_)) ;
        }

    sub PRINTF

        {
        shift ;
        my $fmt = shift ;
        HTML::Embperl::embperl_log(sprintf ($fmt, @_)) ;
        }
    }





#######################################################################################
#
# init on httpd startup
#

if (defined ($INC{'Apache.pm'}))
    { 
    my $log ;
    
    eval 'use Apache::Constants' ;

    $DefaultLog = $ENV{EMBPERL_LOG} if defined ($ENV{EMBPERL_LOG}) ;
    embperl_init (&epIOMod_Perl, $DefaultLog) ;
    tie *LOG, 'HTML::Embperl::Log' ;
    #$log = embperl_getloghandle () ;
    #open LOG, ">>&=$log" || print STDERR "Embperl: Cannot open LOG (fh=$log)" ;
    #autoflush LOG 1 ;
    }

#######################################################################################

sub _eval_ ($)
    {
    my $result = eval "$_[0]" ;
    if ($@ ne '')
        { embperl_logevalerr ($@) ; }
    return $result ;
    }

#######################################################################################

sub _evalsub_ ($)
    {
    my $result = eval "sub { $_[0] } " ;
    if ($@ ne '')
        { embperl_logevalerr ($@) ; }
    return $result ;
    }

#######################################################################################


sub addNameSpace ($)

    {
    my ($sName) = @_ ;
    
    my $cp ;


    
    $cp = $NameSpace{$sName} ;
    if (defined ($cp))
        {
        undef $NameSpace{$sName} ;
        undef $cp ;
        }


    $cp = new Safe ($sName, Safe::emptymask ()) ;
    
    $NameSpace{$sName} = $cp ;

    $cp -> share ('%fdat', '@ffld', '%idat', '$row', '$cnt', '$col', 
                  '$tabmode', '$maxrow', '$maxcnt') ;

    eval "package $sName ; sub $sName::_eval_  { eval $_[0] } ;" ;
    
    return 0 ;
    }


#######################################################################################

sub SendLogFile ($$)

    {
    my ($fn, $info) = @_ ;
    
    my $lastpid = 0 ;
    my ($filepos, $pid) = split (/&/, $info) ;


    
    open (LOGFILE, $fn) || return 500 ;

    seek (LOGFILE, $filepos, 0) || return 500 ;

    print "<HTML><HEAD><TITLE>Embperl Logfile</TITLE></HEAD><BODY bgcolor=\"#FFFFFF\">\r\n" ;
    print "Logfile = $fn, Position = $filepos, Pid = $pid<BR><CODE>\r\n" ;

    while (<LOGFILE>)
        {
        /^\[(\d+)\](.*?)\s/ ;
        if ($1 == $pid)
            {
            if (defined ($LogFileColors{$2}))
                {
                print "<font color=\"$LogFileColors{$2}\">" ;
                }
            else
                {
                print "<font color=0>" ;
                }
            s/\n/\\<BR\\>\r\n/ ;
            embperl_output ("$_") ;
            last if (/Request finished/) ;
            }
        }

    print "</CODE></BODY></HTML>\r\n\r\n" ;

    close (LOGFILE) ;

    return 200 ;
    }

#######################################################################################


sub CheckFile

    {
    my ($filename) = @_ ;


    unless (-r $filename && -s _)
        {
	    embperl_logerror (&rcNotFound, $filename);
	    return (&NOT_FOUND, 0, 0) ;
        }

	if (defined ($req_rec) && !($req_rec->allow_options & &OPT_EXECCGI))
        {
	    embperl_logerror (&rcExecCGIMissing, $filename);
	    return (&FORBIDDEN, 0, 0) ;
 	    }
	
    if (-d _)
        {
	    embperl_logerror (&rcIsDir, $filename);
	    return (&FORBIDDEN, 0, 0);
	    }
    
   	my $mtime = -M _ ;

	# Escape everything into valid perl identifiers
        my @p = unpack ('H*', $filename) ;
	my $package = 'HTML::Embperl::ROOT' . join ('x', @p ) ;


	if (!defined($mtime{$filename}) || $mtime{$filename} != $mtime)
        {
        # clear out any entries in the cache
        delete $cache{$filename} ;
 	    $mtime{$filename} = $mtime ;
        }
    
    $cache{$filename}{'~-1'} = 1 ; # make sure hash is defined 
    
    return (0, -s _, \$cache{$filename}) ;
    }

#######################################################################################


sub run (\@)
    
    {
    my ($args) = @_ ;
    my $Logfile    = $ENV{EMBPERL_LOG} || $DefaultLog ;
    my $Debugflags = $ENV{EMBPERL_DEBUG} || 0 ;
    my $Outputfile = '' ;
    my $Inputfile  = '' ;
    my $Daemon     = 0 ;
    my $Cgi        = $#$args >= 0?0:1 ;
    my $rc         = 0 ;
    my $log ;
    my $pcodecache ;
    my $cgi ;

    if ($$args[0] eq 'dbgbreak') 
    	{
    	shift @$args ;
    	embperl_dbgbreak () ;
    	}

    while ($#$args >= 0)
    	{
    	if ($$args[0] eq '-o')
    	    {
    	    shift @$args ;
    	    $Outputfile = shift @$args ;	
            }
    	elsif ($$args[0] eq '-l')
    	    {
    	    shift @$args ;
    	    $Logfile = shift @$args ;	
	        }
    	elsif ($$args[0] eq '-d')
    	    {
    	    shift @$args ;
    	    $Debugflags = shift @$args ;	
	        }
    	elsif ($$args[0] eq '-D')
    	    {
    	    shift @$args ;
    	    $Daemon = 1 ;	
	        }
	    else
	        {
	        last ;
	        }
	    }
    
    if ($#$args >= 0)
    	{
    	$Inputfile = shift @$args ;
    	}		
    if ($#$args >= 0)
    	{
        $ENV{QUERY_STRING} = shift @$args ;
        undef $ENV{CONTENT_LENGTH} ;
    	}		
	
    if ($Daemon)
        {
        $Logfile = '' || $ENV{EMBPERL_LOG};   # log to stdout
        $ioType = &epIOProcess ;
        $Outputfile = $ENV{__RETFIFO} ;
        }
    elsif ($Cgi)
        {
	    $Inputfile = $ENV{PATH_TRANSLATED} ;
        $ioType = &epIOCGI ;
        }
    else
        {
        $ioType = &epIOPerl ;
        }

    if (defined ($ENV{EMBPERL_VIRTLOG}) &&
        $ENV{EMBPERL_VIRTLOG} eq $ENV{SCRIPT_NAME})
        {
        return SendLogFile ($Logfile, $ENV{QUERY_STRING}) ;
        }
    

    if (defined ($ns = $ENV{EMBPERL_NAMESPACE}))
        {
        addNameSpace ($ns) if (!defined ($NameSpace {$ns})) ;
        }
    else
        {
        $ns = '' ;
        }

    
    my $filesize ;
    
    ($rc, $filesize, $pcodecache) = CheckFile ($Inputfile) ;

    if ($rc)
        {
        return $rc ;
        }


    embperl_init ($ioType, $Logfile) ;
    #$log = embperl_getloghandle () ;
    #open LOG, "<&=$log" ;
    tie *LOG, 'HTML::Embperl::Log' ;

    undef $cgi ;

    if (defined($ENV{'CONTENT_TYPE'}) && $ENV{'CONTENT_TYPE'}=~m|^multipart/form-data|)
        { # just let CGI.pm read the multipart form data, see cgi docu
	    $cgi = new CGI;
	    
        @ffld = $cgi->param;
    
    	foreach ( @ffld )
        	{
    	    $fdat{ $_ } = $cgi->param( $_ );
        	}
        }
    else
        {
        @ffld = () ;
        %fdat = () ;
        }


    do
	    {
	    $rc = embperl_req  ($Inputfile, $Outputfile, $Debugflags, $ns, $filesize,$pcodecache ) ;
	    }
    until ($ioType != &epIOProcess) ;

    #close LOG ;
    embperl_term () ;

    return $rc ;
    }


#######################################################################################



sub handler 
    
    {
    local ($req_rec) = @_ ;
    my $rc ;
    my $ns ;
    my $pcodecache ;
    my $cgi ;

    %ENV = $req_rec->Apache::cgi_env;

   
    if (defined ($ENV{EMBPERL_VIRTLOG}) &&
        $ENV{EMBPERL_VIRTLOG} eq $req_rec -> Apache::uri)
        {
        $req_rec -> content_type ('text/html') ;
        $req_rec -> Apache::send_http_header ;
        embperl_setreqrec ($req_rec) ;
        $rc = SendLogFile ($DefaultLog, $ENV{QUERY_STRING}) ;
        embperl_resetreqrec () ;
        return $rc ;
        }

        
    my $filesize ;
    ($rc, $filesize, $pcodecache) = CheckFile ($req_rec -> Apache::filename) ;

    if ($rc)
        {
        return $rc ;
        }

    
    
    $ENV{PATH_TRANSLATED} = $req_rec -> Apache::filename ;

    if (defined ($ns = $ENV{EMBPERL_NAMESPACE}))
        {
        addNameSpace ($ns) if (!defined ($NameSpace {$ns})) ;
        }
    else
        {
        $ns = '' ;
        }
   

    if (defined($ENV{'CONTENT_TYPE'}) && $ENV{'CONTENT_TYPE'}=~m|^multipart/form-data|)
        { # just let CGI.pm read the multipart form data, see cgi docu
	    $cgi = new CGI;
	    
        @ffld = $cgi->param;
    
    	foreach ( @ffld )
        	{
    	    $fdat{ $_ } = $cgi->param( $_ );
        	}

        }
    else
        {
        @ffld = () ;
        %fdat = () ;
        }


    $rc = embperl_setreqrec ($req_rec) ;
    
    if ($rc == 0)
        {
        $logfilepos = embperl_getlogfilepos () ;
        $LogfileURL = "<A HREF=\"$ENV{EMBPERL_VIRTLOG}?$logfilepos&$$\">Logfile</A><BR>" ;

        $rc = embperl_req ($ENV{PATH_TRANSLATED}, '', $ENV{EMBPERL_DEBUG}, $ns, $filesize, $pcodecache) ;
        
        
        }
    
    if ($rc != 0)
        {
        print STDERR "rc = $rc \n";
        return 500 ;
        }

    return 0 ;
    }


#######################################################################################

sub MailFormTo

    {
    my ($to, $subject) = @_ ;
    my $v ;
    my $k ;
    my $ok ;

    eval 'use Net::SMTP' ;

    $smtp = Net::SMTP->new('localhost');
    $smtp->mail('WWW-Server');
    $smtp->to($to);
    $ok = $smtp->data();
    $ok and $ok = $smtp->datasend("Subject: $subject\n");
    foreach $k (@ffld)
        { 
        $v = $fdat{$k} ;
        if (defined ($v) && $v ne '')
            {
            $ok and $ok = $smtp->datasend("$k\t= $v \n" );
            }
        }
    $ok and $ok = $smtp->datasend("\nClient\t= $ENV{REMOTE_HOST} ($ENV{REMOTE_ADDR})\n\n" );
    $ok and $ok = $smtp->dataend() ;
    $smtp->quit; 

    return $ok ;
    }    


###############################################################################    
#
# Is module is only here that HTML::Embperl also shows up under module/by-module/Apache/ .
#
package Apache::Embperl; 



*handler = \&HTML::Embperl::handler ;

            

1;
__END__


=head1 NAME

HTML::Embperl - Perl extension for embedding perl code in HTML documents

=head1 SYNOPSIS

Embperl is a perl extension module which gives you the ability to embed perl 
code in HTML documents (much like Server Side Includes for shell 
commands).

=head1 DESCRIPTION

Embperl can operate in one of four modes:

=over 4

=item B<Offline>

converts a HTML file with embedded perl statements into a standard HTML 
file.

B<embpexec.pl [-o outputfile][-l logfile][-d debugflags] htmlfile [query_string]>

=over 4

=item B<htmlfile>

is the full pathname of the html file which should be processed by Embperl

=item B<query_string>

is optional and has same meaning as the environment variable QUERY_STRING 
when invoked as CGI-Script, i.e. everything following the first "?" in an 
URL. <query_string> should be url-encoded. Default is no query_string.

=item B<-o outputfile>

gives the filename to which the output is written. Default is stdout. 

=item B<-l logfile>

is optional and give the filename of the logfile. Default is 
/tmp/embperl.log. 

=item B<-d debugflags>

specifies the level of debugging (What is written to the logfile). Default
is nothing. See below for exact values. 

=back


=item B<As CGI-Script>

instead of directly retrieving the document from the web-server, it is 
processed by the CGI-Script and the result is send to the client.

B<embpexec.pl>

If C<embpexec.pl> is invoked without any parameters and the environment variable 
PATH_TRANSLATED is set, it invoke it self as CGI-Script. That means form 
data is taken either from the environment variable QUERY_STRING or from 
stdin depending on CONTENT_LENGTH (this will be set by httpd depending on 
the method GET or POST). Input is taken from the file pointed to by 
PATH_TRANSLATED and output is send to stdout. The logfile is generated at 
it's default location (this is configurable via the environment variable 
EMBPERL_LOG).

To use this mode you have to copy B<embpexec.pl> to your cgi-bin 
directory. You can invoke it with the URL http://www.domain.xyz/cgi-
bin/embpexec.pl/url/of/your/document.

The /url/of/your/document will be passed to Embperl by the web server. 
Normal processing i.e. aliasing etc. takes place before it is made to the 
filename contained in PATH_TRANSLATED.

If you are running apache httpd you can also define B<embpexec.pl> as a 
handler for a specific file extention or directory.

Example of Apache C<srm.conf>:

    <Directory /path/to/your/html/docs>
    Action text/html /cgi-bin/embperl/embpexec.pl
    </Directory>


=item B<From mod_perl>

(Apache httpd), this works like the CGI-Script, but with the advantage 
that the script is compiled only once at server startup, where also other 
one time action (such as opening files and databases ) can take place. 
This will drastically reduce response times for the request.
To use this you have to compile C<Apache httpd> with C<mod_perl> and add C<HTML::Embperl> as
C<PerlHandler>.

Example of Apache C<srm.conf>:

    SetEnv EMBPERL_DEBUG 2285

    Alias /embperl /path/to/embperl/eg

    <Location /embperl/x>
    SetHandler perl-script
    PerlHandler HTML::Embperl
    Options ExecCGI
    </Location>

See also under debugging (dbgLogLink and EMBPERL_VIRTLOG) how you can setup Embperl
so you can view logfile with your browser!


=item B<As standalone process>

At the moment this is only for debugging because it can only handle one 
request at a time, no serialization takes place if more than one client is 
accessing an embperl-document. This mode has the same advantage as 
mod_perl, because the process is once started and running (hopefully) 
until the web server goes down. Data from the web-server is transferred 
via two named pipes. The first gives the data of the request and is feeded 
by a small c-program which is invoked as a CGI-Script and the second 
transfers the output back to the CGI-Program, which sends it to the 
client. This mode should work in conjunction with every web server, but to 
really use it a serialization (and maybe a management for multiple 
processes must be done)

B<embpexec.pl -D>

or

B<embpexec.pl> if PATH_TRANSLATED is not defined as a environment variable

This start Embperl as a Daemon. You have also to copy the file embcgi to 
your cgi-bin directory. This program is invoked as CGI-Script by the web 
server. The names of the named pipe which will be used must be changed in 
epmain.c and embpcgi.c before compiling it and the pipes must be created 
by hand (i.e. mkfifo) with read and write access for both processes, 
before starting the processes.
Input- and Formdata is the same as for the CGI-Script. Logfile outut is 
going to stdout.

B<WARNING:> Everybody who has write access to the named pipe can do things as 
user which runs Embperl daemon. So be carefully not to run Embperl as root 
unless you are sure nobody else can access it.

=back

=head1 B<Runtime configuration>

At the moment there are a few things which could be configured at runtime. 
This is done by setting environment variables, either on the command line 
(when working offline) or in your web servers configuration file. Most 
http daemons understand

SetEnv <var> <value>

=over 4

=item B<EMBPERL_LOG>

gives the pathname of the log file. This will contain more or less infos 
about what Embperl is doing depending on the debug settings (see below). 
The log-output is specially intended to see what your embedded perl code 
is doing and to debug it.
Default is B</tmp/embperl.log>

=item B<EMBEPRL_VIRTLOG>

gives a virtual location, where you can access the embperl logfile with a browser.
This feature is disabled (default) if EMBPERL_VIRTLOG is not specified. See also
dbgLogLink for an Example how to set it up in your srm.conf.

=item B<EMBPERL_DEBUG>

This is a bitmask which specifies what should be written to the log
The following values are defined:

=over 4

=item dbgStd = 1,

Minimum Infos

=item dbgMem = 2,

Memory and Scalar Value allocation

=item dbgEval = 4,

Arguments and result of evals

=item dbgCmd = 8,

Metacommands and HTML tags which are processed

=item dbgEnv = 16,

List environement variables

=item dbgForm = 32,

List form data 

=item dbgTab = 64,

Log processing of dynamic tables

=item dbgInput = 128,

Log processing of html input tags

=item dbgFlushOutput = 256,

Flush output after every write (Should normaly not set. Only for debugging 
when Embperl crashs, will drasticaly slow down operation)

=item dbgFlushLog = 512,

Flush logfile after every line (Should normaly not set. Only for debugging 
when Embperl crashs, log is automatily flushed after each html file, when set
will slow down operation)

=item dbgAllCmds  = 1024,

Logs all commands and HTML tags, regardless if they are really excuted or
not. (Showing a + or - to tell you if they are executed).

=item dbgSource = 2048,

Logs the next piece of the HTML-source which is processed. (Gives a lot
of output!)

=item dbgFunc = 4096,

Only anvailable when compiled with -DEPDEBUGALL. Normaly only used for debugging
Embperl itself. Logs all functionentrys to the logfile (lot of data!)

=item dbgLogLink = 8192,

Inserts a link on the top of each page, which can be used to view the log for current
html file. See also EMBEPRL_VIRTLOG under configuration.

Example:

    SetEnv EMBPERL_DEBUG 10477
    SetEnv EMBPERL_VIRTLOG /embperl/log

    <Location /embperl/log>
    SetHandler perl-script
    PerlHandler HTML::Embperl
    Options ExecCGI
    </Location>

=item dbgDefEval = 16386

Shows everytime a new perl-code is compiled

=item dbgCacheDisable = 32768

Disables the usage of the p-code cache. All the perl code is recompiled everytime.
(Should not be used in normal operation, slows down Embperl drasticaly)

=item dbgEarlyHttpHeader = 65536

Normaly http-headers are send after the request was finished without any errors.
This gives you the chance to set aribary http-header within the page, and Embperl
the chance to calculate the content length. To do this all the output the kept in
memory until the http-headers are send, and send afterwards. This flag will cause
the http-headers to be send before script-processing and then the output directly
without keeping it in memory (like versions before 0.18)


=back

A good value to start is C<2285> or C<10477> if you want to view the logfile
with your browser (Don't forget to set EMBPERL_VIRTLOG).
If Embperl crashs (you get a Segmentation
fault) add C<512> so the logfile is flushed and you can see where Embperl
crashs.


=item B<EMBPERL_NAMESPACE>

gives the name of the namesspace within the code should be executed. If 
not defined executes in namespace B<HTML::Embperl::> with no operator 
restrictions. (with a normal eval statement).

NOTE at the moment if you use a namespace it is added automatiliy with all 
operators allowed. This will change in furthrer releases.

=back

=head1 B<SYNTAX>

Embperl understands four categories of commands. The first three are 
special Embperl command, the last category are some HTML-Tags which can 
trigger a special processing. Before the special Embperl-commands are 
processed and for the VALUE attribute of the input tag (see below), all 
HTML-Tags are removed and special HTML characters are translated to their 
ascii values (e.g. &lt; is translated to "<" ). Embperl-commands can spawn 
multiple lines and not necessarily starts or ends at line boundary. You 
can escape from this behavior by preceding the special character or HTML 
tag with a backslash. This is done, so you can create your embperl-html-
file with your favorite (WYSIWYG) HTML-Editor, no matter if it inserts 
tags like line breaks or formatting into your Embperl-commands where you 
don't want them.
All Embperl-commands starts with a "[" and ends with a "]". To get a real 
"[" you must enter "[[".

I am not using sgml comments (i.e. <! ... !> or similar things) because 
some HTML-Editors can't create them or it's much more complicated. Sinces 
every HTML-Editor takes [ and ] as normal text, there should be no problem.

=over 4

=item B<[+ perl-code +]>

Replace the command with the result of the perl code (The value returned 
by eval "perl-code").
As C<perl-code> you can use everything which can be an argument to the perl 
eval statement (see Security below for restrictions). 
Examples:

 [+$a+]          replaces the [+$a+] with the content
                 of the variable $a
 [+$a+1+]        every expression can be used
 [+ $x[$i] +]    also array and hashes or more complex
                 expressions works

C<NOTE:> White space are ignored
The output will automaticly HTML escaped (e.g. "<" is translated to &lt;). 
You do have to care about it.


=item B<[- perl-code -]>

Executes the perl-code, but delete the whole command from the HTML output.

Examples:

 [-$a=1-]        set the variable $a to one, no output
                 will be generated
 [-use somemodule-] you can use other modules
 [-$i=0; while ($i<5) {$i++} -] even more complex statements
                                or multiple statements are possible.

NOTE: Statements like if, while, for, etc. must be included in one embperl 
command. You can not have the if in one command block and the terminating 
"}" or else in another.

=item B<[$cmd arg$]>

Execute a Embperl metacommand
<cmd> can be one of the following (<arg> varies depending on <cmd>):

=over 4

=item B<if, elsif, else, endif>

everything following the if metacommand until the else, elsif or endif is 
only outputted if the perl expression given in <arg> is true. else and 
elsif works analog.

Examples:

 [$if $ENV{REQUEST_METHOD} eq 'GET' $]
 Method was GET<BR>
 [$else$]
 Method other than GET used<BR>
 [$endif$]

This will send one of the two sentence to the client, depending on the 
request method used to call the document.

=item B<while, endwhile>

Executes a loop until the <arg> given to while is false

Example: (see eg/x/while.htm)

 [- $i = 0; @k = keys %ENV -]
 [$ while ($i &lt; $#k) $]
 [+ $k[$i] +] = [+ $ENV{$k[$i]} +]<BR>
 [- $i++ -]
 [$ endwhile $]

This will send a list of all environment variables to the client.

NOTE: The '&lt;' is translated to '<' before call the perl eval.

=item B<hidden>

<arg> consists of zero, one or two names of hashs (without the leading %). 
The hidden metacommand will generated hidden fields for all data contained 
in first hash and not in second hash. Default for first hash is C<%fdat> and 
for second hash C<%idat>. This is intended for situations where you want to 
pass data from one forms to the next, e.g. two forms which should be 
filled one after each other (e.g. an input form and a second form to 
review and accept the input). Here you can transport the data from 
previous forms within hidden fields. (See eg/x/neu.htm for an example).

NOTE: This should only be used for small amount of data, since the hidden 
fields are sent to the browser, which sends it back at next request. If 
you have large data, store it within a file with a unique name and send 
only the filename within the hidden field. But be aware of the fact, that 
the data could be change by the browser if the user didn't behave exactly 
as you except. Your program should handle such situations properly.

=back

=item B<HTML Tags>

Embperl recognizes the following HTML Tags (all other are simply passed 
through, as long as they not part of a Embperl command).

=over 4

=item B<table, /table, tr, /tr>

Embperl can generate dynamic tables (one or two dimensional). You only have to specify one row/column. 
Embperl generates as much rows/columns as nessecary. This is done by using the 
magic variables $row, $col and $cnt. If you don't use $row/$cnt/$cnt within a table,
Embperl does nothing and simply pass the table through.
Embperl checks if the varibale $row/$col/$cnt 
is used. 
Embperl repeats all text between <table> and </table>, as long 
the expressions in which $row or $cnt occurs is/are defined.
Embperl repeats all text between <tr> and </tr>, as long 
the expressions in which $col or $cnt occurs is/are defined.

See also $tabmode (below) for end of table criteria.


Examples: (see eg/x/table.htm for more examples)

 [- @k = keys %ENV -]
 <table>
     <tr>
         <td>[+ $i=$row +] </td>
         <td>[+ $k[$row] +] </td>
         <td>[+ $ENV{$k[$i]} +] </td>
     </tr> 
 </table>

This will show all entries in array @k (which contains the keys from %ENV), so 
the whole environment is displayed (like in the while example), with the 
first column containing the index (from 0) and the second containing the 
content of the array and the third the environment variable.

This could be used to display the result of database query if you have the 
result in an array. You make as much columns as you need. It is also 
possible to call a fetch subroutine in each table row.

=item B<th, /th>

th tags is interpreted as table heading. If the whole row is made of <th> </th>
instead of <td> </td> it's treated as column heading. Everythingelse will be 
treated as row heading in the future, but is ignored in the current version. 

=item B<dir, menu, ol, ul, dl, select, /dir, /menu, /ol, /ul, /dl, /select>

Lists and Dropdown/Listboxes are treated excatly as one dimensional tables.
Only $row, $maxrow, $cnt,
$maxcnt and $tabmode are honoured. $col and $maxcol are ignored.
see eg/x/lists.htm for an example.

=item B<option>

Embperl looks if theres a value from the form data for a specific option in 
a menu. If so this option will be preselected.

Example:

<form method="POST">
  <P>Select Tag</P>

    If you request this document with list.htm?SEL1=x
    you can specify that the element which has a value
    of x is initialy selected
    

    <p><select name="SEL1">
        <option value="[+ $v[$row] +]">[+ $k[$row] +]</option>
    </select></p>
</form>


=item B<input>

The input tag interacts with the hashs C<%idat> und C<%fdat>. If the input tag 
has no value and a key exists with the same text as the NAME attribute of 
the input tag, Embperl generates a VALUE attribute which the corresponding 
value to the hashkey.
All values of <input> tags are stored in the hash C<%idat>, which NAME as 
hashkey and VALUE as hashvalue.
Special processing is made for TYPE=RADIO and TYPE=CHECKBOX. If the VALUE 
attribute contains the same text as the value of the hash the CHECKED 
attribute is inserted else it is removed.
So if you specify as action url the file which contains the form itself, 
the form will be redisplayed with same values as entered in the first 
form. (See eg/x/neu.htm for an example)

=item B<textarea, /textarea>

The textarea tags treated excatly like other input fields (see above)


=back

=head1 Predefined variables

Embperl has some special variables which has a predefined meaning.

=over 4

=item B<%ENV>

contains the environment as seen from a CGI-Script.

=item B<%fdat>

contains all the formdata send to the script by the calling form. The NAME 
attribute build the key and the VALUE attribute is used as hashvalue. 
Embperl doesn't matter if it's called with GET or POST method. (but there 
may be restrictions on the length of parameters using GET, not from Embperl
but maybe from your Webserver, especialy if you using cgi-mode, so it's more 
save to use POST). Embperl also support multipart formdata, as it's been used
for fileupload. For normal fields there is no difference to normal formdata, 
see docs of CGI.pm how fileupload fields are handled. 

=item B<@ffld>

contains all the field names in the order they where send by the browser 
(normally as they appear in your form)

=item B<%idat>

contains all the values from all input tags processed so far.

=item B<$row>

row count for use in dynamic tables (see html tag table)

=item B<$maxrow>

maxium number of rows displayed in a table. This is set to 100 by default to prevent
endless loops. (see html tag table)

=item B<$col>

column count for use in dynamic tables (see html tag table)

=item B<$maxcol>

maxium number of columns displayed in a table. This is set to 10 by default to prevent
endless loops. (see html tag table)

=item B<$cnt>

contains the number of tables cells displayed so far (see html tag table)

=item B<$tabmode>

determintas how the end of a dynamic table is detected:

B<end of table>

=over 4

=item B<1>

end when a expression with $row gets undefined (The row containing the undefined is not displayed)

=item B<2>

end when a expression with $row gets undefined (The row containing the undefined is displayed)

=item B<3>

end when $maxrow rows displayed

=back

B<end of row>

=over 4

=item B<16>

end when a expression with $col gets undefined (The column containing the undefined is not displayed)

=item B<32>

end when a expression with $col gets undefined (The column containing the undefined is displayed)

=item B<63>

end when $maxcol column displayed

=back

default is B<17> which is correct for all sort of arrays. You rarely should have
to change it.
The two values can be added together

=item B<$req_rec>

This variable is only available when running under control of mod_perl. 
It contains the request record needed to access the apache server api.
See B<perldoc Apache> for more information.

=item B<LOG>

is the filehandle of the embperl logfile, by writing print LOG "something" you can add lines
to the logfile. NOTE: The logfileline should always start with the pid of the currect process
and continue with a four character signature delimited by a ':', which specifies the log reason.

Example: print LOG "[$$]ABCD: your text\n" ;

NOTE 2: You must set the dbgFlushLog (= 512) to get the right order in the logfile.
This will not be necessary any more in one of the next releases.

=back

=head1 Namespaces and operator restrictions

Since most web servers will contain more than one document, it is 
necessary to protected them against each other. Embperl does this by using 
perl-namespaces. You can assign different names spaces to different 
directories (or even files within apache 1.2). If an Embperl document is 
scaned in a namespace all execution takes place within this namesspace and 
the code is not allowed to access another namespace as his own. This is 
done by the perl module Safe.pm.
This gives also the ability to restrict the usage of perl operators, which 
can be defined with a operator mask.
At the moment these features are at experimental state and can't be fully 
configured. Also there seems to be a memory leak.

=head1 UTILITY FUNCTIONS

=over 4

=item B<MailFormTo ($MailTo, $Subject)>

Sends the content of the hash %fdat in the order specified by @ffld to the
given B<$MailTo> adresse, with the subject of B<$Subject>.

If you specifiy the following example code as action in your form

  <form action="x/feedback.htm" method="POST"
   enctype="application/x-www-form-urlencoded">

The content of the form will be mailed to the given email adress.


Example:

 <html>
 <head>
 <title>Feedback</title>
 </head>
 <body>
        Your data has been sccesfully send!
        [- MailFormTo ('webmaster@domain.xy',
                       'Mail from WWW Form') -]
 </body>
 </html>

B<NOTE:> You must have Net::SMTP (from libnet package) installed to use this function.

=back

=head1 BUGS

At the moment Namespaces are at experimental state and can't be fully 
configured. Also there seems to be a memory leak.

Tainting doesn't work correct under perl5.004.

Under perl5.004 there are memory leaks, this is not an Embperl Bug, but can
cause your httpd to endless grow when running under mod_perl. Please upgrade
to perl5.004_04 to fix this.

=head1 FEEDBACK and BUGREPORTS

Please give me a feedback if you use/test this module. Bugs, 
questions, thinks you would find useful etc. are discussed on the mod_perl
mailling list.

>From mod_perl README:
 
>For comments, questions, bug-reports, announcements, etc., join the
>Apache/Perl mailing list by sending mail to
>listserv@listproc.itribe.net with the string "subscribe modperl" in
>the body.   

>There is a hypermail archive for this list available from:
>http://outside.organic.com/mail-archives/modperl/


=head1 SUPPORT

You can get free support on the mod_perl mailing list (see above).
If you need commercial support (with a garantie for response time/a solution) for Embperl
or want a web site where you can run your Embperl/mod_perl scripts without setting
up your own internet www-server, please send an email to info@ecos.de.

=head1 WWW-LINKS

mod_perl            http://perl.apache.org/
mod_perl FAQ        http://www.ping.de/~fdc/mod_perl/
Embperl             http://perl.apache.org/embperl.html
apache web server   http://www.apache.org/
see also            http://www.perl.com/CPAN/modules/by-module/Apache/apache-modlist.html

=head1 AUTHOR

G.Richter
<richter@dev.ecos.de>

=head1 SEE ALSO

perl(1), mod_perl, apache httpd.

