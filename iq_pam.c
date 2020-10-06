
#include "pam.h"
#include "ddzap.h"


int main (int argc, char **argv)
{
    pamdata iq;
    char filename[25];
    int filedes[2];
    int fd;
    int color = 0;
    struct dddvb_fe *fe=NULL;
    pid_t pid=0;

    if (argc > 3 ){
	char *newargs[argc+2];
	for(int j = 0; j<argc; j++) newargs[j] = argv[j];

	newargs[argc] = "-o\0";
	newargs[argc+1] ="2\0";
	if ((fe = ddzap(argc+2, newargs))){
	    fprintf(stderr,"Error\n");
	}
	exit(1);
    } else {
	if ( !strncmp (argv[1],"-q",2)){
	    color = strtoul(argv[1]+2, NULL, 0);
	    if (!color) {
		color = strtoul(argv[2], NULL, 0);
	    }
	}

	fd = fileno(stdin);
    }
    if ( init_pamdata(&iq) < 0 ) exit(1);
    iq.col = color;

    while (1){
	pam_read_data(fd, &iq);
	pam_write(1, &iq);
   }
    
    return 0;
}
