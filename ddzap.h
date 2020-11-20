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
int parse_args(int argc, char **argv,
	       uint32_t *bandwidth,
	       uint32_t *frequency,
	       uint32_t *symbol_rate,
	       uint32_t *pol,
	       uint32_t *id,
	       uint32_t *ssi,
	       uint32_t *num,
	       uint32_t *source,
	       uint32_t *mtype,
	       enum fe_code_rate *fec,
	       enum fe_delivery_system *delsys,
	       char **config,
	       uint32_t *verbosity,
	       int *color);

struct dddvb_fe *set_tune_dddvb( struct dddvb *dd,
				 uint32_t bandwidth,
				 uint32_t frequency,
				 uint32_t symbol_rate,
				 uint32_t pol,
				 uint32_t id,
				 uint32_t ssi,
				 uint32_t num,
				 uint32_t source,
				 uint32_t mtype,
				 enum fe_code_rate fec,
				 enum fe_delivery_system delsys);

#endif /* _DDZAP_H_*/
 
