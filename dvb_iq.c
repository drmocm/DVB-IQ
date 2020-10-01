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
#include "ddzap.h"


#define WIDTH 640
#define HEIGHT 480
#define TS_SIZE 188
#define MAXPACKS 200
#define BSIZE (TS_SIZE * MAXPACKS)
#define MINDATA ((TS_SIZE-4)/2)
#define MAXDATA (MAXPACKS*MINDATA)
#define N_IMAGES 3

/* window */
static GtkWidget *window = NULL;

/* Current frame */
static GdkPixbuf *frame;

/* Images */
static GdkPixbuf *image;

/* Widgets */
static GtkWidget *da;

typedef struct iqdata_
{
    guchar *data_points;
    int8_t *data;
    int npacks;
    int newn;
    int width;
    int height;
} iqdata;

int init_iqdata(iqdata *iq, int npacks)
{
    iq->npacks = 0;
    iq->width= 0;
    iq->height = 0;
    if (npacks < 1 || npacks >MAXPACKS) return -1;
    if (!( iq->data=(int8_t *) malloc(sizeof(int8_t) *
				      MAXPACKS*TS_SIZE)))
    {
        fprintf(stderr,"not enough memory\n");
        return -1;
    }
    memset(iq->data,0,MAXPACKS*TS_SIZE);
    if (!( iq->data_points=(guchar *) malloc(sizeof(guchar) *
				      256*256*3)))
    {
        fprintf(stderr,"not enough memory\n");
        return -1;
    }
    memset(iq->data_points,0,256*256*3);
    iq->npacks = npacks;
    iq->newn = 0;
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
    memset(iq->data_points,0,256*256*3);
}



gboolean read_data (GIOChannel *source, GIOCondition condition, gpointer data)
{
    GError *error = NULL;	
    gchar buf[BSIZE];
    gsize sr=0;
    int i,j;
    iqdata *iq = (iqdata *) data;
    if ( !GTK_IS_WIDGET (da)) return TRUE;
    if (iq->newn){
	iq->npacks = iq->newn;
	iq->newn = 0;
    }
//    if (iq->block) return TRUE;
    g_io_channel_read_chars (source,(char *)iq->data,
			     iq->npacks*TS_SIZE,
			     &sr, &error);
    memset(iq->data_points,0,256*256*3);
    for (i=0; i < iq->npacks; i++){
	for (j=0; j<TS_SIZE-4; j+=2){
	    int ix = iq->data[i*TS_SIZE+j+4]+128;
	    int qy = iq->data[i*TS_SIZE+j+4+1]+128;
	    iq->data_points[ix*3  +(256*3)*qy]=0;
	    iq->data_points[ix*3+1+(256*3)*qy]=255;
	    iq->data_points[ix*3+2+(256*3)*qy]=0;
	}
    }
    for (i = 0; i < 256; i++){
	iq->data_points[i*3+256*128*3] = 255;
	iq->data_points[i*3+1+256*128*3] = 255;
	iq->data_points[128*3+i*256*3] = 255;
	iq->data_points[128*3+1+i*256*3] = 255;
    }
    gtk_widget_queue_draw (da);

    return TRUE;
}

guint start_read_watch(int fd, GIOFunc func, gpointer data) {
  GError *error = NULL;

  GIOChannel *channel = g_io_channel_unix_new(fd);

  g_io_channel_set_encoding(channel, NULL, &error); // read binary
  guint id = g_io_add_watch(channel,
                            G_IO_IN | G_IO_HUP | G_IO_ERR,
                            func,
                            data);
  if(error != NULL){		/* handle potential errors */
    fprintf(stderr, "g_io_channel_set_encoding failed %s\n",
	    error->message);
    exit(1);
  }
  g_io_channel_unref(channel);
  return id;
}

static gboolean
on_draw (GtkWidget *widget, cairo_t *cr, gpointer data)
{
    GdkRectangle da;            /* GtkDrawingArea size */
    gdouble dx = 5.0, dy = 5.0; /* Pixels between each point */
    gdouble clip_x1 = 0.0, clip_y1 = 0.0, clip_x2 = 0.0, clip_y2 = 0.0;
    gint i,j;
    GdkWindow *window = gtk_widget_get_window(widget);
    iqdata *iq = (iqdata *) data;

    /* Determine GtkDrawingArea dimensions */
    gdk_window_get_geometry (window,
            &da.x,
            &da.y,
            &da.width,
            &da.height);


    /* Draw on a black background */
    cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
    cairo_paint (cr);


//    cairo_translate (cr, da.width / 2, da.height / 2);

    cairo_device_to_user_distance (cr, &dx, &dy);
    cairo_clip_extents (cr, &clip_x1, &clip_y1, &clip_x2, &clip_y2);
    int w = clip_x2-clip_x1;
    int h = clip_y2-clip_y1;

    if (iq->width != w || iq->height != h){
	iq->width = w;
	iq->width = h;
	g_object_unref (frame);
	frame = gdk_pixbuf_new (GDK_COLORSPACE_RGB,
				FALSE, //has_alpha
				8,     //bits_per_sample
				w,   //width
				h);  //height
    }
    image = gdk_pixbuf_new_from_data (iq->data_points,
				      GDK_COLORSPACE_RGB,
				      FALSE, //has_alpha
				      8,256,256,3*256,destroy_pixdata,iq);
    gdk_pixbuf_composite (image,
			  frame,
			  0,0,w,h,0,0,w/256.0,h/256.0,
			  GDK_INTERP_NEAREST,255); 
    
    gdk_cairo_set_source_pixbuf (cr, frame, 0, 0);
    cairo_paint (cr);
    g_object_unref(image);
    return FALSE;
}


void on_value_changed(GtkRange* widget, gpointer data)
{
    iqdata *iq = (iqdata *) data;
    iq->newn =gtk_range_get_value (GTK_RANGE(widget));
}


int main (int argc, char **argv)
{
    GtkWidget *npackr;
    GtkWidget *vbox;
    GError *error=NULL;
    iqdata iq;
    char filename[25];
    int filedes[2];
    int fd;
    struct dddvb_fe *fe=NULL;
    pid_t pid=0;

    gtk_init (&argc, &argv);

    char *newargs[argc+1];
    for(int j = 0; j<argc; j++)
    {
      newargs[j] = argv[j];
    }
    newargs[argc] = "-o";
    if ((fe = ddzap(argc+1, newargs))){
	snprintf(filename,25,
		 "/dev/dvb/adapter%d/dvr%d",fe->anum, fe->fnum);
	if (pipe(filedes) == -1) {
	    perror("pipe");
	    exit(1);
	}
	
	pid = fork();
	if (pid == -1) {
	    perror("fork");
	    exit(1);
	} else if (pid == 0) {
	    while ((dup2(filedes[1], STDOUT_FILENO) == -1) && (errno == EINTR)) {}
	    close(filedes[1]);
	    close(filedes[0]);
#define BUFFSIZE (1024*188)
	    uint8_t buf[BUFFSIZE];
	    fprintf(stderr,"opening %s\n", filename);
	    
	    if ((fd = open(filename ,O_RDONLY)) < 0){
		fprintf(stderr,"Error opening input file: %s\n",filename);
	    }
	    while(1){
		int r,w;
		r=read(fd,buf,BUFFSIZE);
		w=write(fileno(stdout),buf,r);
	    }
	    _exit(1);
	}
	close(filedes[1]);
	fd = filedes[0];
    } else fd = fileno(stdin);

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    da = gtk_drawing_area_new ();

    if ( init_iqdata(&iq,MAXPACKS/10) < 0 ) exit(1);

    gtk_window_set_default_size (GTK_WINDOW (window), WIDTH, HEIGHT);
    gtk_window_set_title (GTK_WINDOW (window), "DVB IQ");
    g_signal_connect (window, "destroy", G_CALLBACK (close_window), NULL);
    g_signal_connect (window, "key_press_event", G_CALLBACK (key_function),NULL);

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

    
    gtk_widget_show_all (window);
    gtk_main ();
    if (fd != fileno(stdin)) kill( pid, SIGTERM );

    return 0;
}
