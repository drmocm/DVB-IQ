
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
	char *newargs[argc+1];

	newargs[argc] = "-o 2";
	if ((fe = ddzap(argc+1, newargs))){
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
    if ( init_pamdata(&iq,MAXPACKS/10) < 0 ) exit(1);
    iq.col = color;

    int fdout = 0;
    if ((fdout = open("test.pam", O_WRONLY | O_APPEND | O_CREAT, 0644)) < 0){
	fprintf(stderr,"Error opening input file: %s\n",filename);
    }
    while (1){
	pam_read_data(fd, &iq);
	pam_write(1, &iq);
   }
    
    return 0;
}
