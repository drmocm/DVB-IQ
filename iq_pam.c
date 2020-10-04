#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <getopt.h>
#include <math.h>
#include "ddzap.h"


#define WIDTH 640
#define HEIGHT 720
#define TS_SIZE 188
#define MAXPACKS 200000

enum { IQ_RED=1, IQ_GREE, IQ_BLUE , IQ_EVIL, IQ_LOG_RED, IQ_LOG_GREEN, IQ_LOG_BLUE , IQ_LOG_EVIL ,};

typedef struct iqdata_
{
    char *data_points;
    uint32_t *data;
    int npacks;
    int newn;
    int width;
    int height;
    int maxd;
    int col;
} iqdata;

int init_iqdata(iqdata *iq, int npacks)
{
    iq->npacks = 0;
    iq->col = 0;
    iq->maxd = 0;
    iq->width= 0;
    iq->height = 0;
    if (npacks < 1 || npacks >MAXPACKS) return -1;
    if (!( iq->data=(uint32_t *) malloc(sizeof(uint32_t) *256*256)))
    {
        fprintf(stderr,"not enough memory\n");
        return -1;
    }
    memset(iq->data,0,256*256*sizeof(uint32_t));
    if (!( iq->data_points=(char *) malloc(sizeof(char) *
					     256*256*3)))
    {
        fprintf(stderr,"not enough memory\n");
        return -1;
    }
    memset(iq->data_points,0,256*256*3*sizeof(char));
    iq->npacks = npacks;
    iq->newn = 0;
    return 0;
}



#define READPACK 1000
#define BSIZE (TS_SIZE * READPACK)
void read_data (int fdin, iqdata *iq)
{
    char buf[BSIZE];
    int i,j;
    if (iq->newn){
	iq->npacks = iq->newn;
	iq->newn = 0;
    }
    for (i = 0; i < iq->npacks; i+= READPACK){
	int re =0;
	if ((re=read(fdin,(char *)buf, BSIZE)) < 0){
	    fprintf(stderr,"Error reading data\n");
	}
	for (i=0; i < re; i+=TS_SIZE){
	    for (j=4; j<TS_SIZE; j+=2){
		int ix = buf[i+j]+128;
		int qy = 128-buf[i+j+1];
		iq->data[ix+256*qy] += 1;
		int c = iq->data[ix+256*qy];
		if ( c > iq->maxd) iq->maxd = c;
	    }
	}
    }

    int m = 255*iq->maxd;
    double lm = log((double)m);
    for (i = 0; i < 256*256*3; i+=3){
	// IQ data plot
	int r = i; 
	int g = i+1;
	int b = i+2;
	int odata = iq->data[i/3];
	int data = 255*iq->data[i/3];
	double lod = log((double)odata); 

	if (data){
	    switch (iq->col){
	    case IQ_LOG_EVIL:
		if (data < m/4) iq->data_points[r] = (int)((512.0*lod)/lm);
		else  if (data >m/2) iq->data_points[g] = (int)(255.0*lod/lm);
		break;
	    case IQ_LOG_RED:
		iq->data_points[r] = (int)(255.0*lod/lm);
		break;
	    case IQ_LOG_GREEN:
		iq->data_points[g] = (int)(255.0*lod/lm);
		break;
	    case IQ_LOG_BLUE:
		iq->data_points[b] = (int)(255.0*lod/lm);
		break;
	    case IQ_EVIL:
		if (data < m/4) iq->data_points[r] = (2*data)/iq->maxd;
		else  if (data >m/2) iq->data_points[g] = data/iq->maxd;
		break;
	    case IQ_RED:
		iq->data_points[r] = data/iq->maxd;
		break;
	    case IQ_BLUE:
		iq->data_points[b] = data/iq->maxd;
		break;
		
	    default:
	    case IQ_GREE:
		iq->data_points[g] = data/iq->maxd;
		break;
	    }
	}
	
    }
    for (i = 0; i < 256*3; i+=3){
	// coordinate axes
	int xr = i    + 256*128*3;
	int yr =128*3 + i*256;
	iq->data_points[xr] = 255;
	iq->data_points[yr] = 255;
	iq->data_points[xr+1] = 255;
	iq->data_points[yr+1] = 255;
    }
	
    memset(iq->data,0,256*256*sizeof(uint32_t));
    iq->maxd = 0;
}


void write_pam (int fd, iqdata *iq){
    char *HEAD="P7\nWIDTH 256\nHEIGHT 256\nDEPTH 3\nMAXVAL 255\nTUPLTYPE RGB\nENDHDR\n";
    int headlen = strlen(HEAD);
    int we=0;
    we=write(fd,HEAD,headlen);
    we=write(fd,iq->data_points,256*256*3);
    memset(iq->data_points,0,256*256*3*sizeof(char));
}

int main (int argc, char **argv)
{
    iqdata iq;
    char filename[25];
    int filedes[2];
    int fd;
    int color = 0;
    struct dddvb_fe *fe=NULL;
    pid_t pid=0;

    char *newargs[argc+1];
    int i=0;
    for(int j = 0; j<argc; j++)
    {
	if ( strncmp (argv[j],"-q",2)){
	    newargs[i] = argv[j];
	    i++;
	} else {
	    color = strtoul(argv[j]+2, NULL, 0);
	    if (!color) {
		color = strtoul(argv[j+1], NULL, 0);
		j++;
	    }
	}
    }
    argc = i;
    newargs[argc] = "-o";
    if ((fe = ddzap(argc+1, newargs))){
	snprintf(filename,25,
		 "/dev/dvb/adapter%d/dvr%d",fe->anum, fe->fnum);
	if (pipe(filedes) == -1) {
	    perror("pipe");
	    exit(1);
	}
	
	pid = fork();
	if (pid == -1) {
	    perror("fork");
	    exit(1);
	} else if (pid == 0) {
	    while ((dup2(filedes[1], STDOUT_FILENO) == -1) && (errno == EINTR)) {}
	    close(filedes[1]);
	    close(filedes[0]);
#define BUFFSIZE (1024*188)
	    uint8_t buf[BUFFSIZE];
	    fprintf(stderr,"opening %s\n", filename);
	    
	    if ((fd = open(filename ,O_RDONLY)) < 0){
		fprintf(stderr,"Error opening input file: %s\n",filename);
	    }
	    while(1){
		int r,w;
		r=read(fd,buf,BUFFSIZE);
		w=write(fileno(stdout),buf,r);
	    }
	    _exit(1);
	}
	close(filedes[1]);
	fd = filedes[0];
    } else fd = fileno(stdin);

    if ( init_iqdata(&iq,MAXPACKS/10) < 0 ) exit(1);
    iq.col = color;

    int fdout = 0;
    if ((fdout = open("test.pam", O_WRONLY | O_APPEND | O_CREAT, 0644)) < 0){
	fprintf(stderr,"Error opening input file: %s\n",filename);
    }
    while (1){
	read_data(fd, &iq);
	write_pam(1, &iq);
   }
    
    return 0;
}
