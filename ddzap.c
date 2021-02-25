#include "ddzap.h"


static uint32_t root2gold(uint32_t root)
{
	uint32_t x, g;
	
	for (g = 0, x = 1; g < 0x3ffff; g++)  {
		if (root == x)
			return g;
		x = (((x ^ (x >> 7)) & 1) << 17) | (x >> 1);
	}
	return 0xffffffff;
}

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
				 enum fe_delivery_system delsys)
{
    struct dddvb_fe *fe;
    struct dddvb_params p;

    switch (delsys) {
    case SYS_DVBC_ANNEX_A:
	if (!symbol_rate)
	    symbol_rate = 6900000;
	break;
    default:
	break;
    }

    if (num != DDDVB_UNDEF)
	fe = dddvb_fe_alloc_num(dd, delsys, num);
    else
	fe = dddvb_fe_alloc(dd, delsys);
    if (!fe) {
	return NULL;
    }
    dddvb_param_init(&p);
    dddvb_set_mtype(&p, mtype);
    dddvb_set_frequency(&p, frequency);
    dddvb_set_src(&p, source);
    dddvb_set_bandwidth(&p, bandwidth);
    dddvb_set_symbol_rate(&p, symbol_rate);
    dddvb_set_polarization(&p, pol);
    dddvb_set_delsys(&p, delsys);
    dddvb_set_id(&p, id);
    dddvb_set_ssi(&p, ssi);
    dddvb_dvb_tune(fe, &p);

    return fe;    
}


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
	       int *color)

{
    int outt = 0;
    while (1) {
	int cur_optind = optind ? optind : 1;
	int option_index = 0;
	int c;
	static struct option long_options[] = {
	    {"config", required_argument, 0, 'c'},
	    {"frequency", required_argument, 0, 'f'},
	    {"bandwidth", required_argument, 0, 'b'},
	    {"symbolrate", required_argument, 0, 's'},
	    {"source", required_argument, 0, 'l'},
	    {"delsys", required_argument, 0, 'd'},
	    {"id", required_argument, 0, 'i'},
	    {"ssi", required_argument, 0, 'g'},
	    {"gold", required_argument, 0, 'g'},
	    {"root", required_argument, 0, 'r'},
	    {"num", required_argument, 0, 'n'},
	    {"mtype", required_argument, 0, 'm'},
	    {"verbosity", required_argument, 0, 'v'},
	    {"open_dvr", no_argument, 0, 'o'},
	    {"color_scheme", required_argument , 0, 'q'},
	    {"help", no_argument , 0, 'h'},
	    {0, 0, 0, 0}
	};
	c = getopt_long(argc, argv, 
			"c:i:f:s:d:p:hg:r:n:b:l:v:m:o:q:",
			long_options, &option_index);
	if (c==-1)
	    break;
	
	switch (c) {
	case 'q':
	    *color = strtoul(optarg, NULL, 0);
	    break;
	case 'o':
	    outt = strtoul(optarg, NULL, 0);
	    outt++;
	    printf("outt %d\n",outt);
	    break;
	case 'c':
	    *config = strdup(optarg);
	    break;
	case 'f':
	    *frequency = strtoul(optarg, NULL, 0);
	    break;
	case 'b':
	    *bandwidth = strtoul(optarg, NULL, 0);
	    break;
	case 's':
	    *symbol_rate = strtoul(optarg, NULL, 0);
	    break;
	case 'l':
	    *source = strtoul(optarg, NULL, 0);
	    break;
	case 'v':
	    *verbosity = strtoul(optarg, NULL, 0);
	    break;
	case 'g':
	    *ssi = strtoul(optarg, NULL, 0);
	    break;
	case 'r':
	    *ssi = root2gold(strtoul(optarg, NULL, 0));
	    break;
	case 'i':
	    *id = strtoul(optarg, NULL, 0);
	    break;
	case 'n':
	    *num = strtoul(optarg, NULL, 0);
	    break;
	case 'm':
	    if (!strcmp(optarg, "16APSK"))
		*mtype = APSK_16;
	    if (!strcmp(optarg, "32APSK"))
		*mtype = APSK_32;
	    if (!strcmp(optarg, "64APSK"))
		*mtype = APSK_64;
	    if (!strcmp(optarg, "128APSK"))
		*mtype = APSK_128;
	    if (!strcmp(optarg, "256APSK"))
		*mtype = APSK_256;
	    if (*mtype == DDDVB_UNDEF)
		printf("unknown mtype %s\n", optarg);
	    break;
	case 'd':
	    if (!strcmp(optarg, "C"))
		*delsys = SYS_DVBC_ANNEX_A;
	    if (!strcmp(optarg, "DVBC"))
		*delsys = SYS_DVBC_ANNEX_A;
	    if (!strcmp(optarg, "S"))
		*delsys = SYS_DVBS;
	    if (!strcmp(optarg, "DVBS"))
		*delsys = SYS_DVBS;
	    if (!strcmp(optarg, "S2"))
		*delsys = SYS_DVBS2;
	    if (!strcmp(optarg, "DVBS2"))
		*delsys = SYS_DVBS2;
	    if (!strcmp(optarg, "T"))
		*delsys = SYS_DVBT;
	    if (!strcmp(optarg, "DVBT"))
		*delsys = SYS_DVBT;
	    if (!strcmp(optarg, "T2"))
		*delsys = SYS_DVBT2;
	    if (!strcmp(optarg, "DVBT2"))
		*delsys = SYS_DVBT2;
	    if (!strcmp(optarg, "J83B"))
		*delsys = SYS_DVBC_ANNEX_B;
	    if (!strcmp(optarg, "ISDBC"))
		*delsys = SYS_ISDBC;
	    if (!strcmp(optarg, "ISDBT"))
		*delsys = SYS_ISDBT;
	    break;
	case 'p':
	    if (!strcmp(optarg, "h"))
		*pol = 1;
	    if (!strcmp(optarg, "v"))
		*pol = 0;
	    break;
	case 'h':
	    return -1;
	default:
	    break;
	    
	}
    }
    return outt;
}


struct dddvb_fe *ddzap(int argc, char **argv)
{
        char filename[25];
        int color = 0;
	int outt  = 0;
	pamdata iq;
	struct dddvb *dd;
	struct dddvb_fe *fe;
	struct dddvb_params p;
	uint32_t bandwidth = 8000000, frequency = 0, symbol_rate = 0, pol = DDDVB_UNDEF;
	uint32_t id = DDDVB_UNDEF, ssi = DDDVB_UNDEF, num = DDDVB_UNDEF, source = 0;
	uint32_t mtype= DDDVB_UNDEF;
	uint32_t verbosity = 0;
	enum fe_code_rate fec = FEC_AUTO;
	enum fe_delivery_system delsys = ~0;
	char *config = "config/";
	int fd = 0;
	int odvr = 0;
	FILE *fout = stdout;
	if (argc < 2) return 0;

	outt = parse_args(argc, argv,
			  &bandwidth,
			  &frequency,
			  &symbol_rate,
			  &pol,
			  &id,
			  &ssi,
			  &num,
			  &source,
			  &mtype,
			  &fec,
			  &delsys,
			  &config,
			  &verbosity,
			  &color
	    );
	    
	if (delsys == ~0) {
	    fprintf(fout,"You have to choose a delivery system: -d (C|S|S2|T|T2)\n");
	    return NULL;
	}

	
	if (outt < 0){
	    fprintf(fout,"ddzap [-d delivery_system] [-p polarity] [-c config_dir] [-f frequency(Hz)]\n"
		    "      [-b bandwidth(Hz)] [-s symbol_rate(Hz)]\n"
		    "      [-g gold_code] [-r root_code] [-i id] [-n device_num]\n"
		    "      [-o (write dvr to stdout)]\n"
		    "\n"
		    "      delivery_system = C,S,S2,T,T2,J83B,ISDBC,ISDBT\n"
		    "      polarity        = h,v\n"
		    "\n");
	    return NULL;
	}

	if (outt){
	    outt--;
	    fout = stderr;
	    fprintf(fout,"Reading from dvr %d\n", outt);
	    odvr = 1;
	}

	if (optind < argc) {
	    fprintf(fout,"Warning: unused arguments\n");
	}

	dd = dddvb_init(config, verbosity);
	if (!dd) {
	    fprintf(fout,"dddvb_init failed\n");
		return NULL;
	}
	fprintf(fout,"dvbnum = %u\n", dd->dvbfe_num);

	fe = set_tune_dddvb( dd,
			     bandwidth,
			     frequency,
			     symbol_rate,
			     pol,
			     id,
			     ssi,
			     num,
			     source,
			     mtype,
			     fec,
			     delsys);
	if (!fe) {
	    fprintf(fout,"dddvb_fe_alloc failed\n");
		return NULL;
	}

	if (!odvr){
		while (1) {
			fe_status_t stat;
			int64_t str, cnr;
			
			stat = dddvb_get_stat(fe);
			str = dddvb_get_strength(fe);
			cnr = dddvb_get_cnr(fe);
			
			printf("stat=%02x, str=%lld.%03llddB, snr=%lld.%03llddB \n",
			       stat, str/1000, abs(str%1000), cnr/1000, abs(cnr%1000));
		sleep(1);
		}
	} else {
#define BUFFSIZE (1024*188)
	        fe_status_t stat;
		uint8_t buf[BUFFSIZE];
		
		stat = 0;
		stat = dddvb_get_stat(fe);
		while (!(stat == 0x1f)&& id < 0x20000000) {
		        int64_t str, cnr;
		
			stat = dddvb_get_stat(fe);
			str = dddvb_get_strength(fe);
			cnr = dddvb_get_cnr(fe);
			
			fprintf(stderr,"stat=%02x, str=%lld.%03llddB, snr=%lld.%03llddB \n",
			       stat, str/1000, abs(str%1000), cnr/1000, abs(cnr%1000));
			sleep(1);
		}
		fprintf(stderr,"got lock on %s\n", fe->name);

		switch (outt){
		case 0:
		        fprintf(stderr,"returning fe\n");
		        return fe;
		        break;
		case 1:
		case 2:
		        snprintf(filename,25,
				 "/dev/dvb/adapter%d/dvr%d",fe->anum, fe->fnum);

		        fprintf(stderr,"opening %s\n", filename);
		        if ((fd = open(filename ,O_RDONLY)) < 0){
		            fprintf(stderr,"Error opening input file:%s\n",filename);
		        }
			while(outt == 1){
			    int re=0;
			    re=read(fd,buf,BUFFSIZE);
			    re=write(fileno(stdout),buf,BUFFSIZE);
			}
		        fprintf(stderr,"writing pamdata\n");
			if (outt ==2 ) init_pamdata(&iq,color,BIT8_IQ,255,255);
			while(outt == 2){
			    pam_read_data(fd, &iq);
			    pam_write(STDOUT_FILENO, &iq);
			}
		        return NULL;
			break;
		}

	}
	return NULL;
}
