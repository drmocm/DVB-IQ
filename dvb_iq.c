#include <glib.h>
#include <glib/gprintf.h>
#include <gio/gio.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <getopt.h>
#include <math.h>
#include "ddzap.h"


#define WIDTH 640
#define HEIGHT 720
#define NCOL 8
/* window */
GtkWidget *window = NULL;
GtkWidget *image;
GtkWidget *shot_button;
GtkWidget *cscale;
GtkWidget *rbutton;

char *colors[]={"r","g","b","rgb","lr","lg","lb","lrgb"};


int get_col(char *c){
    int i=1;
    while (i<NCOL+1 && strcmp(c,colors[i-1])){
	i++;
    }
    return i;
}

typedef struct iqdata_
{
    pamdata pam;
    int fd;
    int save;
    int shot;
} iqdata;

typedef struct radb_
{
    iqdata *iq;
    char *col;
} radb;

int init_iqdata(iqdata *iq)
{
    init_pamdata(&iq->pam,1,BIT8_IQ,255,255);
    return 0;
}


static void
close_window (void)
{
    gtk_main_quit();
}

/* our usual callback function */
void shot_callback (GtkWidget *widget, gpointer *data)
{
    iqdata *iq = (iqdata *)data;

    if (!iq->save) iq->save=1;
    
}

static gboolean key_function (GtkWidget *widget, GdkEventKey *event, gpointer data) {
    switch (event->keyval){
    case GDK_KEY_q: 
    case GDK_KEY_Escape:
	gtk_main_quit();
	exit(0);
	return TRUE;
	break;
    default:
	return FALSE; 
    }
}

void destroy_pixdata (guchar *pixels, gpointer data){
    iqdata *iq = (iqdata *) data;
}

/*
static gboolean got_data (gpointer data)
{
    gtk_widget_queue_draw(image);

    return G_SOURCE_REMOVE;
}
*/	

static gboolean get_pam_data(void *args) {
    iqdata *iq = (iqdata *)args;
    GError *error = NULL;

    pam_read_data(iq->fd, &iq->pam);
    gtk_widget_queue_draw(image);
    
    return FALSE;
}

static void realize_cb (GtkWidget *widget, gpointer data) {
    iqdata *iq = (iqdata *)data;
    g_idle_add (get_pam_data, data);
}

gboolean
draw_callback (GtkWidget *widget, cairo_t *cr, gpointer data)
{
    gint x, y, width, height;
    GdkPixbuf *pixbuf;

    iqdata *iq = (iqdata *)data;

    width = gtk_widget_get_allocated_width (image);
    height = gtk_widget_get_allocated_height (image);
    pixbuf = gdk_pixbuf_new_from_data (iq->pam.data_points,
				       GDK_COLORSPACE_RGB,
				       FALSE, //has_alpha
				       8,256,256,3*256,destroy_pixdata,iq);
    
    pixbuf = gdk_pixbuf_scale_simple(pixbuf,
				     width, height, GDK_INTERP_BILINEAR);
    
    gdk_cairo_set_source_pixbuf(cr, pixbuf, 0, 0);
    cairo_paint(cr);
    g_idle_add (get_pam_data, data);

    if (iq->save){
	char filename[25];
	sprintf(filename,"IQ-Screenshot%03d.png",iq->shot);
	gdk_pixbuf_save (pixbuf, filename,"png",NULL,NULL);
	g_print ("Screenshot save as %s\n",filename);
	iq->shot++;
	if (iq->shot >= 1000) iq->shot=0;
	iq->save = 0;
    }
    g_object_unref(pixbuf);
    
    return FALSE;
}

void RadioEvent (GtkWidget *widget, gpointer *data)
{
    radb *r = (radb *)data;
    r->iq->pam.col = get_col(r->col);
//    g_print ("radio event: %s %d\n",r->col,get_col(r->col));
}

GtkWidget *CreateRadio (GtkWidget *box, GSList **group, char *szLabel)
{
    GtkWidget *radio;
  
    /* --- Get the radio button --- */
    radio = gtk_radio_button_new_with_label (*group, szLabel);

    *group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (radio));

    /* --- Pack the radio button into the vertical box (box).  --- */
    gtk_box_pack_start (GTK_BOX (box), radio, FALSE, FALSE, 10);

    /* --- Show the widget --- */
    gtk_widget_show (radio);

    return (radio);
}

GtkWidget *make_radio_buttons(iqdata *iq, GtkWidget *hbox)
{
    GtkWidget *radio = NULL;
    GSList *group = NULL;
    radb *r;

    for (int i=1; i<NCOL+1; i++){
	
	radio = CreateRadio (hbox, &group, colors[i-1] );
	r = (radb *) malloc(sizeof(radb));
	r->iq = iq;
	r->col = colors[i-1];
	g_signal_connect (G_OBJECT (radio), "clicked",
			  G_CALLBACK (RadioEvent), (void *)r);
	if (i == iq->pam.col)
	    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio),TRUE);
    }
    
    return radio;
}

int main (int argc, char **argv)
{
    GtkWidget *npackr;
    GtkWidget *vbox;
    GtkWidget *hbox;
    GError *error=NULL;
    iqdata iq;
    char filename[25];
    int fd;
    int color = 0;
    struct dddvb_fe *fe=NULL;
    pid_t pid=0;

    gtk_init (&argc, &argv);

    char *newargs[argc+2];
    for(int j = 0; j<argc; j++)
    {
	newargs[j] = argv[j];
	if ( !strncmp (argv[j],"-q",2)){
	    color = strtoul(argv[j]+2, NULL, 0);
	    if (!color && j+1 <argc) {
		color = strtoul(argv[j+1], NULL, 0);
	    }
	}
    }

    newargs[argc] = "-o";
    newargs[argc+1] = " 0";
    if ((fe = ddzap(argc+2, newargs))){
	snprintf(filename,25,
		 "/dev/dvb/adapter%d/dvr%d",fe->anum, fe->fnum);
	fprintf(stderr,"opening %s\n", filename);
	
	if ((fd = open(filename ,O_RDONLY)) < 0){
	    fprintf(stderr,"Error opening input file: %s\n",filename);
	}
    } else fd = fileno(stdin);

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);

    if ( init_iqdata(&iq) < 0 ) exit(1);
    iq.pam.col = color;
    iq.fd = fd;
    iq.save = 0;
    iq.shot = 0;
    
    gtk_window_set_default_size (GTK_WINDOW (window), WIDTH, HEIGHT);
    gtk_window_set_title (GTK_WINDOW (window), "DVB IQ");

    g_signal_connect (window, "destroy", G_CALLBACK (close_window), (void*)&iq);
    g_signal_connect (window, "key_press_event", G_CALLBACK (key_function),(void*)&iq);
    g_signal_connect (window, "realize", G_CALLBACK (realize_cb), (void *)&iq);   
    rbutton = make_radio_buttons(&iq,hbox);

    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

    
    gtk_box_set_homogeneous (GTK_BOX(vbox),FALSE);

    image = gtk_drawing_area_new ();
    g_signal_connect (G_OBJECT (image), "draw",
		      G_CALLBACK (draw_callback), &iq);
    gtk_box_pack_start(GTK_BOX(vbox), image, TRUE, TRUE, 0);

    shot_button = gtk_button_new_with_label ("Screenshot");
    g_signal_connect (G_OBJECT (shot_button), "clicked",
                        G_CALLBACK(shot_callback), (gpointer) &iq);
    gtk_box_pack_start(GTK_BOX(vbox), shot_button, FALSE, FALSE, 0);

    gtk_container_add (GTK_CONTAINER (window), vbox);
    gtk_widget_show_all (window);
    
    gtk_main ();

    return 0;
}
