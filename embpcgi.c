
#include <stdlib.h>
#include <stdio.h>


#define sFifoPath "/local/www/cgi-bin/embperl/"
#define sCmdFifo "cmd.fifo"
#define sRetFifo "ret.fifo"


int main( int argc, char *argv[ ], char *envp[ ] )

    {
    int     i ;
    FILE *  fd ;

    int     len   = 0 ;
    char *  plen  = getenv ("CONTENT_LENGTH") ;
    char *  query = getenv ("QUERY_STRING") ;
    char *  buf ;
    char *  p ;

    char  sCmd [256] ;
    char  sRet [256] ;
    
    char  sLine [1024] ;


    sprintf (sCmd, "%s%s", sFifoPath, sCmdFifo) ;
    sprintf (sRet, "%s%s", sFifoPath, sRetFifo) ;
    
    if (plen)
        len = atoi (plen) ;

    if ((fd = fopen (sCmd, "w")) == NULL)
        {
        fprintf (stderr, "Cannot open %s\n", sCmd) ;
        return 1 ;
        }

    fprintf (fd, "----\n") ;

            
    i = 0 ;
    while (envp [i] != NULL)
        fprintf (fd, "%s\n", envp [i]), i++ ;

    fprintf (fd, "__RETFIFO=%s\n", sRet) ;


    buf = NULL ;
    if (len == 0 && query != NULL)
        {
        buf = strdup (query) ;
        len = strlen (query) ;
        }
    else           
        {
        if (len > 0)
            {
            buf = malloc (len + 1) ;
            if (buf != NULL)
                {
                read (0, buf, len) ;
                buf [len] = '\0' ;
                }
            }
        }

    if (buf)
        {
        fprintf (fd, "****\n") ;
        fprintf (fd, "%d\n", len) ;
        fprintf (fd, "%s\n", buf) ;
        }
    
    
    fclose (fd) ;


    if ((fd = fopen (sRet, "r")) == NULL)
        {
        fprintf (stderr, "Cannot open %s\n", sRet) ;
        return 1 ;
        }

    while (fgets (sLine, sizeof(sLine), fd))
        puts (sLine) ;

    fclose (fd) ;



    return 0 ;
    }

