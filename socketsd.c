
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

#define REQUIRED_ARGC 3
#define PORT_POS 1
#define MSG_POS 2
#define ERROR 1
#define QLEN 1
#define PROTOCOL "tcp"
#define BUFLEN 1024

int usage (char *progname)
{
    fprintf (stderr,"usage: %s port msg\n", progname);
    exit (ERROR);
}

int errexit (char *format, char *arg)
{
    fprintf (stderr,format,arg);
    fprintf (stderr,"\n");
    exit (ERROR);
}

int main (int argc, char *argv [])
{
    struct sockaddr_in sin;
    struct sockaddr addr;
    struct protoent *protoinfo;
    unsigned int addrlen;
    int sd, sd2, ret;
    char buffer [BUFLEN];
    FILE* filetowrite;
    if (argc != REQUIRED_ARGC)
        usage (argv [0]);

    /* determine protocol */
    if ((protoinfo = getprotobyname (PROTOCOL)) == NULL)
        errexit ("cannot find protocol information for %s", PROTOCOL);

    /* setup endpoint info */
    memset ((char *)&sin,0x0,sizeof (sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons ((u_short) atoi (argv [PORT_POS]));

    /* allocate a socket */
    /*   would be SOCK_DGRAM for UDP */
    sd = socket(PF_INET, SOCK_STREAM, protoinfo->p_proto);
    if (sd < 0)
        errexit("cannot create socket", NULL);

    /* bind the socket */
    if (bind (sd, (struct sockaddr *)&sin, sizeof(sin)) < 0)
        errexit ("cannot bind to port %s", argv [PORT_POS]);

    /* listen for incoming connections */
    if (listen (sd, QLEN) < 0)
        errexit ("cannot listen on port %s\n", argv [PORT_POS]);
    printf("%u\n", addrlen);
    /* accept a connection */
    while (1) {
    sd2 = accept (sd,&addr,&addrlen);
    if (sd2 < 0)
        errexit ("error accepting connection%s", strerror(errno));
    printf("%u\n", addrlen);
    /* write message to the connection */

    /* snarf whatever server provides and print it */
    char option[2];
    char filenamelength[1];
    ret = read(sd2,option,1);
    if (option[0]=='C') {
      memset (buffer,0x0,BUFLEN);
      ret = read (sd2,buffer,BUFLEN - 1);
      if (ret < 0)
          errexit ("reading error",NULL);
      fprintf (stdout,"%s\n",buffer);
    }
    else if ((option[0]== 'G')||(option[0]== 'V')) {
        memset (filenamelength, 0x0, 1);
        ret = read(sd2,filenamelength,1);
        printf("%u\n", (unsigned int)filenamelength[0]);
        char filename[(unsigned int)filenamelength[0]+1];
        memset(filename,0x0,filenamelength[0]+1);
        ret = read(sd2,filename,((unsigned int)filenamelength[0]));
        printf("%s\n",filename );
        filetowrite = fopen(filename,"rb");
        if(!filetowrite){
            errexit("Could not open file", NULL);
        }
        fseek(filetowrite, 0, SEEK_END);
        long fsize = ftell(filetowrite);
        fseek(filetowrite, 0, SEEK_SET);  //same as rewind(f);
        char *string = malloc(fsize + 1);
        fread(string, fsize, 1, filetowrite);
        fclose(filetowrite);
        string[fsize] = 0;
        printf("ready to write message%s\n",string );
        int writevalue = write (sd2,string,strlen (string));
        printf("wrote\n");
        if (writevalue < 0){
            printf("write failed\n");
            errexit ("error writing message%s", string);
        }
        printf("wrote message:%s\n",string );
    }
    printf("option:%c\n",option[0] );
    /* close connections and exit */

    close (sd2);
    }
    close (sd);
    exit (0);
}
