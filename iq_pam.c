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
#include <time.h>

#include "ddzap.h"


#define WIDTH 640
#define HEIGHT 720
#define TS_SIZE 188
#define MAXPACKS 200000

enum { IQ_RED=1, IQ_GREE, IQ_BLUE , IQ_EVIL, IQ_LOG_RED, IQ_LOG_GREEN, IQ_LOG_BLUE , IQ_LOG_EVIL ,};

typedef struct iqdata_
{
    char *data_points;
    uint64_t *data;
    int col;
} iqdata;

int init_iqdata(iqdata *iq, int npacks)
{
    iq->col = 0;
    if (npacks < 1 || npacks >MAXPACKS) return -1;
    if (!( iq->data=(uint64_t *) malloc(sizeof(uint64_t) *256*256)))
    {
        fprintf(stderr,"not enough memory\n");
        return -1;
    }
    memset(iq->data,0,256*256*sizeof(uint64_t));
    if (!( iq->data_points=(char *) malloc(sizeof(char) *
					     256*256*3)))
    {
        fprintf(stderr,"not enough memory\n");
        return -1;
    }
    memset(iq->data_points,0,256*256*3*sizeof(char));
    return 0;
}


static long getutime(){
        struct timespec t0;
        clock_gettime(CLOCK_MONOTONIC_RAW,&t0);
        return t0.tv_sec * (int)1e9 + t0.tv_nsec;
}

#define BSIZE 100*TS_SIZE
#define DTIME 40000000ULL
void read_data (int fdin, iqdata *iq)
{
    int8_t buf[BSIZE];
    int i,j;
    long t0;
    long t1;
    uint64_t maxd = 0;
    
    t0 = getutime();
    t1 = t0;

    while ((t1 - t0) < DTIME){
	int re =0;
	if ((re=read(fdin,(char *)buf, BSIZE)) < 0){
	    fprintf(stderr,"Error reading data\n");
	}
	for (i=0; i < re; i+=TS_SIZE){
	    for (j=4; j<TS_SIZE; j+=2){
		uint8_t ix = buf[i+j]+128;
		uint8_t qy = 128-buf[i+j+1];
		iq->data[ix+256*qy] += 1;
		uint64_t c = iq->data[ix+256*qy];
		if ( c > maxd) maxd = c;
	    }
	}
	t1 = getutime();
    }

    uint64_t m = 255*maxd;
    double lm = log((double)m);
    for (i = 0; i < 256*256*3; i+=3){
	// IQ data plot
	int r = i; 
	int g = i+1;
	int b = i+2;
	uint64_t odata = iq->data[i/3];
	uint64_t data = 255*iq->data[i/3];
	double lod = log((double)odata); 

	if (data){
	    switch (iq->col){
	    case IQ_LOG_EVIL:
		if (data < m/4) iq->data_points[r] = (int)((512.0*lod)/lm)&0xff;
		else  if (data >m/2) iq->data_points[g] = (int)(255.0*lod/lm)&0xff;
		break;
	    case IQ_LOG_RED:
		iq->data_points[r] = (int)(255.0*lod/lm)&0xff;
		break;
	    case IQ_LOG_GREEN:
		iq->data_points[g] = (int)(255.0*lod/lm)&0xff;
		break;
	    case IQ_LOG_BLUE:
		iq->data_points[b] = (int)(255.0*lod/lm)&0xff;
		break;
	    case IQ_EVIL:
		if (data < m/4) iq->data_points[r] = ((2*data)/maxd)&0xff;
		else  if (data >m/2) iq->data_points[g] = (data/maxd)&0xff;
		break;
	    case IQ_RED:
		iq->data_points[r] = (data/maxd)&0xff;
		break;
	    case IQ_BLUE:
		iq->data_points[b] = (data/maxd)&0xff;
		break;
		
	    default:
	    case IQ_GREE:
		iq->data_points[g] = (data/maxd)&0xff;
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
    memset(iq->data,0,256*256*sizeof(uint64_t));
	
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
