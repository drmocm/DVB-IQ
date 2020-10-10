#ifndef _PAM_H_
#define _PAM_H_

#include <stdint.h>
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

#define TS_SIZE 188

enum { IQ_RED=1, IQ_GREE, IQ_BLUE , IQ_EVIL, IQ_LOG_RED, IQ_LOG_GREEN, IQ_LOG_BLUE , IQ_LOG_EVIL , IQ_TEST, };

enum { BIT8_IQ, BIT16_IQ,};

typedef struct pamdata_
{
    unsigned char *data_points;
    uint64_t *data;
    int col;
    int type;
} pamdata;

int init_pamdata(pamdata *iq,int col, int type);
void pam_read_data (int fdin, pamdata *iq);
void pam_write (int fdout, pamdata *iq);
void pam_coordinate_axes(pamdata *iq, unsigned char r,
			 unsigned char g, unsigned char b);
void pam_data_convert(pamdata *iq ,uint64_t maxd);


#endif /* _PAM_H_*/
