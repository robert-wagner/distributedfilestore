
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define ERROR 1
#define REQUIRED_ARGC 3
#define HOST_POS 1
#define PORT_POS 2
#define PROTOCOL "tcp"
#define BUFLEN 1024

int usage (char *progname)
{
    fprintf (stderr,"usage: %s host port\n", progname);
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
    struct hostent *hinfo;
    struct protoent *protoinfo;
    char buffer [BUFLEN];
    int sd, ret;
    int hostnameflag = 0;
    int portflag = 0;
    int localfileflag = 0;
    int remotefileflag = 0;
    int createflag = 0;
    int getflag = 0;
    int viewflag = 0;
    char* hostname;
    char* portno;
    char* localfilename;
    char* remotefilename;
    FILE* filetowrite;
    char currentarg = getopt(argc, argv, "h:p:cgdvF:f:");
    while (currentarg!=-1) {
      switch (currentarg ) {
        case 'h':
          if (hostnameflag) {
            errexit("Too many hostnames", NULL);
          }
          hostname  = optarg;
          hostnameflag = 1;
          break;
        case 'p':
          if (portflag) {
            errexit("Too many ports", NULL);
          }
          portno = optarg;
          portflag = 1;
          break;
        case 'f':
          if (localfileflag) {
            errexit("Too many local files", NULL);
          }
          localfilename  = optarg;
          localfileflag = 1;
          break;
        case 'F':
          if (remotefileflag) {
            errexit("Too many remote files", NULL);
          }
          remotefilename  = optarg;
          remotefileflag = 1;
          break;
        case 'c':
          if(getflag||viewflag){
            errexit("Cannot get and push at the same time", NULL);
          }
          createflag = 1;
          break;
        case 'g':
          if (createflag) {
            errexit("Cannot get and push at the same time", NULL);
          }
          getflag = 1;
          break;
        case 'v':
          if (createflag) {
            errexit("Cannot get and push at the same time", NULL);
          }
          viewflag = 1;
          break;
      }
      currentarg = getopt(argc, argv, "h:p:cgdvF:f:");
    }
    if (!hostnameflag) {
      errexit("No hostname specified", NULL);
    }
    if (!portflag) {
      errexit("No port specified", NULL);
    }
    if ((createflag||getflag||viewflag)&&!remotefileflag) {
      errexit("No remote file specified", NULL);
    }
    if ((createflag||getflag)&&!localfileflag) {
      errexit("No local file specified", NULL);
    }
    printf("%s\n",remotefilename );

    /* lookup the hostname */
    hinfo = gethostbyname (hostname);
    if (hinfo == NULL)
        errexit ("cannot find name: %s", hostname);

    /* set endpoint information */
    memset ((char *)&sin, 0x0, sizeof (sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons (atoi (portno));
    memcpy ((char *)&sin.sin_addr,hinfo->h_addr,hinfo->h_length);

    if ((protoinfo = getprotobyname (PROTOCOL)) == NULL)
        errexit ("cannot find protocol information for %s", PROTOCOL);

    /* allocate a socket */
    /*   would be SOCK_DGRAM for UDP */
    sd = socket(PF_INET, SOCK_STREAM, protoinfo->p_proto);
    if (sd < 0)
        errexit("cannot create socket",NULL);

    /* connect the socket */
    if (connect (sd, (struct sockaddr *)&sin, sizeof(sin)) < 0)
        errexit ("cannot connect", NULL);
    if (createflag) {
      filetowrite = fopen(localfilename,"rb");
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
      if (write (sd,"C",1) < 0)
          errexit ("error writing message: %s", localfilename);
      if (write (sd,string,strlen (string)) < 0)
          errexit ("error writing message: %s", localfilename);
    } else if (getflag||viewflag) {
          memset (buffer,0x0,BUFLEN);
          ret = read (sd,buffer,BUFLEN - 1);
          while (ret>=BUFLEN-1) {
              if (viewflag) {
                fprintf (stdout,"%s",buffer);
              }
              memset (buffer,0x0,BUFLEN);
              ret = read (sd,buffer,BUFLEN - 1);
          }

          if (ret < 0)
              errexit ("reading error",NULL);
          if (viewflag) {
            fprintf (stdout,"%s\n",buffer);
          }
    }



    /* close & exit */
    close (sd);
    exit (0);
}
