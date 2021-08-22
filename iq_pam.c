/*
iq-pam is small example programs to show IQ data with libdddvb and 
DigitalDevices MAX SX8

Copyright (C) 2020-2021  Marcus Metzler

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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
    int type = BIT8_IQ;

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
    if ( init_pamdata(&iq,color,type,255,255) < 0 ) exit(1);
    iq.col = color;

    while (1){
	pam_read_data(fd, &iq);
	pam_write(1, &iq);
   }
    
    return 0;
}
