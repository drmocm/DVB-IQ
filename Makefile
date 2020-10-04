GTK_FLAGS = `pkg-config --cflags gtk+-3.0`
GTK_LIBS = `pkg-config --libs gtk+-3.0`
CFLAGS =  -g  -Wno-unused -Wall -Wno-format -O2 -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE  $(GTK_FLAGS)
LIBS = $(GTK_LIBS) -l dddvb -l pthread -l dvben50221 -l dvbapi -l ucsi -lm
OBJ = dvb_iq.o ddzap.o
OBJPAM = iq_pam.o ddzap.o

CC = gcc
CPP = g++
TARGETS = dvb_iq iq_pam

all: $(TARGETS)

dvb_iq: $(OBJ)
	$(CC) $(CFLAGS) -o dvb_iq $(OBJ) $(LIBS)

iq_pam: $(OBJPAM)
	$(CC) $(CFLAGS) -o iq_pam $(OBJPAM) $(LIBS)

%.o:    %.c %.h
	$(CC) -c $(CFLAGS) $(INCS) $(DEFINES) $<

clean:
	for f in $(TARGETS) *.o .depend *~ ; do \
		if [ -e "$$f" ]; then \
			rm "$$f" || exit 1; \
		fi \
	done
