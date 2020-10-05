#ifndef _DDZAP_H_
#define _DDZAP_H_
//#include <linux/dvb/frontend.h>
#include "frontend.h"
#include <libdddvb.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>
#include "ddzap.h"
#include "pam.h"

struct dddvb_fe *ddzap(int argc, char **argv);

#endif /* _DDZAP_H_*/
 
