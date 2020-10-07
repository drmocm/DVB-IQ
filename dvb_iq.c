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

/* window */
GtkWidget *window = NULL;
GtkWidget *image;

/* Current frame */
static GdkPixbuf *frame;

/* Images */
static GdkPixbuf *gpix;

/* Widgets */
static GtkWidget *da;

typedef struct iqdata_
{
    pamdata pam;
    int fd;
    int width;
    int height;
} iqdata;

int init_iqdata(iqdata *iq)
{
    iq->width= 0;
    iq->height = 0;
    init_pamdata(&iq->pam,1);
    return 0;
}


static void
close_window (void)
{
    gtk_main_quit();
}

static gboolean key_function (GtkWidget *widget, GdkEventKey *event, gpointer data) {
    switch (event->keyval){
    case GDK_KEY_q: 
    case GDK_KEY_Escape: 
	gtk_widget_destroy(widget);
	gtk_main_quit();
	return TRUE;
	break;
    default:
	return FALSE; 
    }
}

void destroy_pixdata (guchar *pixels, gpointer data){
    iqdata *iq = (iqdata *) data;
//    memset(iq->data_points,0,256*256*3);
}


static gboolean
on_draw (GtkWidget *widget, cairo_t *cr, gpointer data)
{
    GdkRectangle da;            /* GtkDrawingArea size */
    GdkWindow *window = gtk_widget_get_window(widget);
    iqdata *iq = (iqdata *) data;

    /* Determine GtkDrawingArea dimensions */
    gdk_window_get_geometry (window, &da.x, &da.y, &da.width, &da.height);

    int w = da.width;
    int h = da.height;

    gpix = gdk_pixbuf_new_from_data (iq->pam.data_points,
				      GDK_COLORSPACE_RGB,
				      FALSE, //has_alpha
				      8,256,256,3*256,destroy_pixdata,iq);
    return FALSE;
}


void on_value_changed(GtkRange* widget, gpointer data)
{
    iqdata *iq = (iqdata *) data;
    // iq->newn =gtk_range_get_value (GTK_RANGE(widget));
}

static gboolean got_data (gpointer data)
{
    GdkPixbuf *pixbuf;
    gint x;
    gint y;
    gint width;
    gint height;

    iqdata *iq = (iqdata *)data;


    GdkWindow *win = gtk_widget_get_window(window);
    gdk_window_get_geometry (win, &x, &y, &width, &height);
    
    if (iq->width != width || iq->height != height){
	iq->width = width;
	iq->height = height;
	if ( frame && !GTK_IS_WIDGET (frame)) g_object_unref (frame);
	frame = gdk_pixbuf_new (GDK_COLORSPACE_RGB,
				FALSE, //has_alpha
				8,     //bits_per_sample
				width,   //width
				height);  //height
    }
    pixbuf = gdk_pixbuf_new_from_data (iq->pam.data_points,
				       GDK_COLORSPACE_RGB,
				       FALSE, //has_alpha
				       8,256,256,3*256,destroy_pixdata,iq);
	
    
    gdk_pixbuf_composite (pixbuf,
			  frame,
			  0,0,width,height,0,0,width/256.0,height/256.0,
			  GDK_INTERP_NEAREST,255); 
    
    gtk_image_set_from_pixbuf((GtkImage*) image, frame); 
    return G_SOURCE_REMOVE;
}
	

static void *get_pam_data(void *args) {
    iqdata *iq = (iqdata *)args;
    GError *error = NULL;

    while(1) {
	pam_read_data(iq->fd, &iq->pam);
	
	gdk_threads_add_idle (got_data, iq);
    }
    exit(0);
}

static void realize_cb (GtkWidget *widget, gpointer data) {
    /* start the video playing in its own thread */
    pthread_t tid;
    pthread_create(&tid, NULL, get_pam_data, (void *)data);
}

int main (int argc, char **argv)
{
    GtkWidget *npackr;
    GtkWidget *vbox;
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

    if ( init_iqdata(&iq) < 0 ) exit(1);
    iq.pam.col = color;
    iq.fd = fd;
    
    gtk_window_set_default_size (GTK_WINDOW (window), WIDTH, HEIGHT);
    gtk_window_set_title (GTK_WINDOW (window), "DVB IQ");
    g_signal_connect (window, "destroy", G_CALLBACK (close_window), NULL);
    g_signal_connect (window, "key_press_event", G_CALLBACK (key_function),NULL);
    g_signal_connect (window, "realize", G_CALLBACK (realize_cb), (void *)&iq);   

    /*
    g_signal_connect (G_OBJECT (da),"draw", G_CALLBACK (on_draw), (gpointer*)&iq);
    npackr = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL,
				      MAXPACKS/20,
				      MAXPACKS,
				      MAXPACKS/10);

    start_read_watch(fd, read_data, &iq);

    gtk_scale_set_draw_value(GTK_SCALE(npackr), TRUE);
    g_signal_connect( npackr, "value_changed",
		      G_CALLBACK(on_value_changed), &iq);
    gtk_range_set_value(GTK_RANGE(npackr),iq.npacks);
    gtk_box_pack_start(GTK_BOX(vbox), npackr, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(vbox), da, TRUE, TRUE, 0);
    gtk_container_add (GTK_CONTAINER (window), vbox);
    */
    image = gtk_image_new();
    gtk_widget_show (image);

    gtk_container_add (GTK_CONTAINER (window), image);
        
    
    gtk_widget_show_all (window);
    gtk_main ();
    if (fd != fileno(stdin)) kill( pid, SIGTERM );

    return 0;
}
