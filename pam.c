#include "pam.h"
#include "ddzap.h"

static int stop_pam(pamdata *iq){
    if (iq->stop){
	if(iq->stopped) return 1;
	free(iq->data);
	free(iq->data_points);
	iq->stopped=1;
	return 1;
    }
    return 0;
}

int init_pamdata(pamdata *iq, int color, int type, int width, int height)
{
    int numiq = width;
    iq->stop = 0;
    iq->stopped = 0;
    width = 256;
    height = 256;	
    if (!( iq->data=(uint64_t *) malloc(sizeof(uint64_t) *256*256)))
    {
	fprintf(stderr,"not enough memory\n");
	return -1;
    }
    memset(iq->data,0,256*256*sizeof(uint64_t));
    
    if (!( iq->data_points=(unsigned char *) malloc(sizeof(unsigned char) *
						    width*height*3)))
    {
	fprintf(stderr,"not enough memory\n");
	return -1;
    }
    memset(iq->data_points,0,width*height*3*sizeof(char));

    iq->width = width;
    iq->height = height;
    iq->col = color;
    iq->type = type;
    return 0;
}


static long getutime(){
        struct timespec t0;
        clock_gettime(CLOCK_MONOTONIC_RAW,&t0);
        return t0.tv_sec * (int)1e9 + t0.tv_nsec;
}

void pam_coordinate_axes(pamdata *iq, unsigned char r,
			 unsigned char g, unsigned char b){
    int i;
    for (i = 0; i < iq->width*3; i+=3){
	// coordinate axes
	int xr = i    + iq->height/2*iq->width*3;
	iq->data_points[xr] = r;
	iq->data_points[xr+1] = g;
	iq->data_points[xr+2] = b;
    }
    for (i = 0; i < iq->height; i++){
	int yr = iq->width/2*3 + i*3*iq->width;
	iq->data_points[yr] = r;
	iq->data_points[yr+1] = g;
	iq->data_points[yr+2] = b;
    }
}

void pam_data_convert(pamdata *iq ,uint64_t maxd)
{
    int i;
    uint64_t m = 255*maxd;
    double lm = log((double)m);
    memset(iq->data_points,0,256*256*3*sizeof(char));
    for (i = 0; i < 256*256*3; i+=3){
	// IQ data plot
	int r = i; 
	int g = i+1;
	int b = i+2;
	uint64_t odata = iq->data[i/3];
	uint64_t data = 255*iq->data[i/3];
	double lod = log((double)odata); 
	double q = lod/lm;

	if (data){
	    switch (iq->col){
	    case IQ_LOG_EVIL:
		if ( q < 0.25){
		    iq->data_points[b] = (int)(1024.0*q);
		} else {
			if (q >0.5) iq->data_points[g] = (int)(255.0*q);
		    else
			iq->data_points[r] = (int)(512.0*q);
		}
		break;
	    case IQ_LOG_RED:
		iq->data_points[r] = (int)(255.0*q)&0xff;
		break;
	    case IQ_LOG_GREEN:
		iq->data_points[g] = (int)(255.0*q)&0xff;
		break;
	    case IQ_LOG_BLUE:
		iq->data_points[b] = (int)(255.0*q)&0xff;
		break;
	    case IQ_EVIL:
		if  (data < m/4){
		    iq->data_points[b] = ((4*data)/maxd)&0xff;
		} else {
		    if (data >m/2) iq->data_points[g] = (data/maxd)&0xff;
		    else iq->data_points[r] = (2*data/maxd)&0xff;
		}
		break;
	    case IQ_TEST:
		if  (data < m/4){
		    iq->data_points[b] = ((4*data)/maxd)&0xff;
		    iq->data_points[g] = ((4*data)/maxd)&0xff;
		} else {
		    if (data >m/2) {
			iq->data_points[g] = (data/maxd)&0xff;
		    } else {
			iq->data_points[g] = (2*data/maxd)&0xff;
			iq->data_points[r] = (2*data/maxd)&0xff;
		    }
		}
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
}

#define RSIZE 100
int read_iq_data(int fdin, int8_t *bufx, int8_t *bufy, int size)
{
    int i,j;
    
    char ibuf[RSIZE*TS_SIZE+1];
    int c=0;
    int re = 0;
    while (c<size){
	int s = RSIZE;
	int d = 2*(size-c)/(TS_SIZE-4);
	fsync(fdin);
	if ( d < s ) s = d;
	if ((re=read(fdin, ibuf, s*TS_SIZE)) < 0){
	    //fprintf(stdout,"Could not read data (%d) %s\n",re,strerror(errno));
	    return -1;
	}
               
       for (i=0; i < re; i+=TS_SIZE){
           for (j=4; j<TS_SIZE; j+=2){
               bufx[c] = ibuf[i+j];
               bufy[c] = ibuf[i+j+1];
               c++;
           }
       }
    }
    return c;
}

#define BSIZE 100*(TS_SIZE-4)

uint64_t read_averaged_data (int fdin, pamdata *iq, long dtime)
{
    int8_t bufx[BSIZE];
    int8_t bufy[BSIZE];
    int i;
    long t0, t1;
    uint64_t maxd = 0;
    int c;

    dtime = dtime * 1000000;
    t0 = getutime();
    t1 = t0;
    
    while ((t1 - t0) < dtime){
	
	if ((c=read_iq_data(fdin, bufx, bufy, BSIZE))<0)
	    return 0;
	
	for (i=0; i < BSIZE; i++){
	    uint8_t ix = bufx[i]+128;
	    uint8_t qy = 128-bufy[i];
	    iq->data[ix|(qy<<8)] += 1;
	    uint64_t c = iq->data[ix|(qy<<8)];
	    if ( c > maxd) maxd = c;
        }
        t1 = getutime();
    }
    return maxd;
}


#define DTIME 40 // msec
void pam_read_data (int fdin, pamdata *iq)
{
    if(stop_pam(iq)){
	close(fdin);
	return;
    }
    if (iq->type == BIT8_IQ){
	uint64_t maxd = 0;
	maxd = read_averaged_data(fdin, iq, DTIME);
	if (!maxd) return;
	pam_data_convert(iq, maxd);
	pam_coordinate_axes(iq, 255,255 ,0);
	memset(iq->data,0,256*256*sizeof(uint64_t));
    }
}    


void pam_write (int fd, pamdata *iq){
    char *HEAD="P7\nWIDTH 256\nHEIGHT 256\nDEPTH 3\nMAXVAL 255\nTUPLTYPE RGB\nENDHDR\n";
    int headlen = strlen(HEAD);
    int we=0;
    we=write(fd,HEAD,headlen);
    we=write(fd,iq->data_points,256*256*3);
    memset(iq->data_points,0,256*256*3*sizeof(char));
}


