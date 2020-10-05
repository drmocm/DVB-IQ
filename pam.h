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
#define MAXPACKS 200000

enum { IQ_RED=1, IQ_GREE, IQ_BLUE , IQ_EVIL, IQ_LOG_RED, IQ_LOG_GREEN, IQ_LOG_BLUE , IQ_LOG_EVIL , IQ_TEST, };

typedef struct pamdata_
{
    char *data_points;
    uint64_t *data;
    int col;
} pamdata;

int init_pamdata(pamdata *iq, int npacks);
void pam_read_data (int fdin, pamdata *iq);
void pam_write (int fdout, pamdata *iq);

#endif /* _PAM_H_*/
