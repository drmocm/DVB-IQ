
#include "pam.h"
#include "ddzap.h"

int init_pamdata(pamdata *iq,int color)
{
    iq->col = 0;
    if (!( iq->data=(uint64_t *) malloc(sizeof(uint64_t) *256*256)))
    {
        fprintf(stderr,"not enough memory\n");
        return -1;
    }
    memset(iq->data,0,256*256*sizeof(uint64_t));
    if (!( iq->data_points=(unsigned char *) malloc(sizeof(unsigned char) *
					     256*256*3)))
    {
        fprintf(stderr,"not enough memory\n");
        return -1;
    }
    memset(iq->data_points,0,256*256*3*sizeof(char));
    iq->col = color;
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
    for (i = 0; i < 256*3; i+=3){
	// coordinate axes
	int xr = i    + 256*128*3;
	int yr =128*3 + i*256;
	iq->data_points[xr] = r;
	iq->data_points[yr] = r;
	iq->data_points[xr+1] = g;
	iq->data_points[yr+1] = g;
	iq->data_points[xr+2] = b;
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

#define BSIZE 100*TS_SIZE
#define DTIME 40000000ULL
void pam_read_data (int fdin, pamdata *iq)
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
		iq->data[ix|(qy<<8)] += 1;
		uint64_t c = iq->data[ix|(qy<<8)];
		if ( c > maxd) maxd = c;
	    }
	}
	t1 = getutime();
    }

    pam_data_convert(iq, maxd);
    pam_coordinate_axes(iq, 255,255 ,0);
    memset(iq->data,0,256*256*sizeof(uint64_t));	
}


void pam_write (int fd, pamdata *iq){
    char *HEAD="P7\nWIDTH 256\nHEIGHT 256\nDEPTH 3\nMAXVAL 255\nTUPLTYPE RGB\nENDHDR\n";
    int headlen = strlen(HEAD);
    int we=0;
    we=write(fd,HEAD,headlen);
    we=write(fd,iq->data_points,256*256*3);
    memset(iq->data_points,0,256*256*3*sizeof(char));
}


