/** A program written in C to replace 'gtkdialog-splash'.
 *  It hasn't got all the fancy features but does have the 
 *  most useful ones.
 *  (c) 2015 Michael Amadio, Gold Coast, QLD, Au
 *  01micko@gmail.com
 *  GPL v2 */ 

#include <glib.h>
#include <glib/gprintf.h>
#include <string.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <unistd.h>
#include <limits.h>
#include <signal.h>

#define _VERSION "0.9"
#define _PROGNAME "gtksplash" 

/* declarations */
static void display_window(gchar *mystring, gchar *mycolor, gchar *fcolor, 
					gchar *position, gchar *font, gchar *deco, gchar *icon);
static void usage();
static gint get_width();
static gint get_height();
static gshort get_font_size(gchar *f_size);
static void font_err(gint font_size, gint too_big);
static gboolean button_quit(GtkWidget *widget, 
					GdkEventButton *event, gpointer gdata);
static gboolean timeout_quit(gpointer gdata);
static void i_quit(GtkWidget *widget, gpointer gdata);
static void sig_handler(gint signo);

/* build the window */
static void display_window(gchar *mystring, gchar *mycolor, gchar *fcolor, 
					gchar *position, gchar *font, gchar *deco, gchar *icon) {
	GtkWidget *window;
	GtkWidget *label;
	GtkWidget *ibox; /* vbox is needed to get some vertical padding */
	GtkWidget *box;
	GdkColor color;
	GdkColor font_color;
	gdk_color_parse (fcolor, &font_color);
	gdk_color_parse (mycolor, &color);
	gint w = get_width();
	gint h = get_height();
	
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_widget_set_events (window, 
					GDK_BUTTON_PRESS_MASK); /* allows close on click */
	g_signal_connect(G_OBJECT(window), "button_press_event", 
					G_CALLBACK(button_quit), NULL);
	gtk_widget_modify_bg (window, GTK_STATE_NORMAL, &color);
	if (g_strcmp0(position, "c") == 0) {
		gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	} else if (g_strcmp0(position, "m") == 0) {
		gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_MOUSE);
	} else { /* this should get window size from pango, needs work */
		gint x = 0;
		gint y = 0;
		if (position[0] == 't') y = 60;
		if (position[0] == 'b') y = h - 120;	
		if (position[1] == 'l') x = 60;
		if (position[1] == 'c') x = w / 2 - 150;
		if (position[1] == 'r') x = w - 300;
		gtk_widget_set_uposition(window, x, y);
	}
	guint font_size = (guint)get_font_size(font);
	if (font_size > 200)
		font_err(font_size, 200);
	gint padding = 10;
	gint icon_size, img_size;
	if (strlen(mystring) <= 65) {
		icon_size = 4;
		img_size = 32;
		if (font_size > 24) {
			icon_size = 5;
			if (font_size > 36)
				icon_size = 6;
			img_size = font_size;
		}
		padding = 6;
	} else if ((65 < strlen(mystring)) &&  (strlen(mystring) < 130)) {
		icon_size = 5;
		img_size = 48;
		if (font_size > 30)
			font_err(font_size, 30);
	} else {
		icon_size = 6;
		img_size = 64;
		if (font_size > 20)
			font_err(font_size, 20);
	}
	ibox = gtk_vbox_new (FALSE, 0);;
	box = gtk_hbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (window), ibox);
	gtk_widget_show(ibox);
	gtk_box_pack_start(GTK_BOX(ibox), box, FALSE, FALSE, padding);
	if (g_strcmp0(icon, "none") != 0) { /* use an icon, stock or otherwise */
		GtkWidget *image;
		if (g_str_has_prefix(icon, "gtk-") == TRUE) { /* stock icon */
			GtkStockItem item; /* wow the docs are crap! */
			/** You see you are automatically expected to know what goes
			 *  where in the structure. Finally, I figured out that member
			 *  gchar *stock_id is the "gtk-whatever" or "gtk-dialog-whatever"
			 *  Ok.. I'm not a real programmer :-)  */
			if (gtk_stock_lookup(icon, &item)) {
				image = gtk_image_new_from_stock(item.stock_id, icon_size);
			} else {
				g_printf("Unable to access %s icon\n", icon);
				exit (EXIT_FAILURE);
			}
		} else { /* /path/to/icon */
			GdkPixbuf *pixbuf;
			GdkPixbuf *new_pixbuf = NULL;
			GError *gerror = NULL;
			gchar icon_path[PATH_MAX];
			if (access(icon, R_OK) == 0) {
				g_stpcpy(icon_path, icon); /* we need to scale it */
				pixbuf = gdk_pixbuf_new_from_file(icon_path, &gerror);
				new_pixbuf = gdk_pixbuf_scale_simple((const GdkPixbuf *)pixbuf,
                                         img_size, img_size, GDK_INTERP_HYPER);
				image = gtk_image_new_from_pixbuf(new_pixbuf);
				gtk_image_set_from_pixbuf((GtkImage *)image, new_pixbuf);
			} else {
				g_printf("Unable to access %s icon\n", icon);
				exit (EXIT_FAILURE);
			}
		}
		gtk_widget_show(box);
		gtk_box_pack_start(GTK_BOX(box), image, FALSE, FALSE, padding);
		gtk_widget_show(image);
		label = gtk_label_new(mystring);
		gtk_widget_modify_font (label,
					pango_font_description_from_string (font));
		gtk_widget_show(box);
		gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_CENTER);
		gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
		gtk_box_pack_end(GTK_BOX(box), label, FALSE, FALSE, padding);
		gtk_widget_show(label);
	} else { /* no icon */
		label = gtk_label_new(mystring);
		
		gtk_widget_modify_font (label,
					pango_font_description_from_string (font));
		gtk_widget_show(box);
		gtk_label_set_justify (GTK_LABEL(label), GTK_JUSTIFY_CENTER);
		gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
		gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, padding);
		gtk_widget_show(label);
	}
	gtk_widget_modify_fg (label, GTK_STATE_NORMAL, &font_color);
	if (g_strcmp0(deco, "yes") == 0) {
		g_signal_connect(G_OBJECT(window), "destroy", 
					G_CALLBACK(i_quit), NULL);
		gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
	} else
		gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
	gtk_window_set_keep_above(GTK_WINDOW(window), TRUE);
 	gtk_window_set_skip_taskbar_hint(GTK_WINDOW(window), TRUE);
	gtk_widget_show(window);
}
/* usage */
static void usage() {
	g_printf("%s-%s\n"
			"\n\t %s -s \"Some string here\"\nOptions :\n"
			"\t -s 'string' : the string to be displayed\n"
			"\t -c 'color' : either an X color or quoted hex notation\n"
			"\t -k 'font color' : either an X color or quoted hex notation\n"
			"\t -t 'timeout' : in seconds\n"
			"\t -p 'position' : 'tl' (top left), 'tc' (top centre), "
			"'tr' (top right),\n\t\t'bl' (bottom left), 'bc' (bottom centre), "
			"'br' (bottom right),\n\t\t'm' (near mouse), "
			"'c' (centre of display - default)\n"
			"\t -f 'font' : a ttf font with size, default is \"Sans 16\"\n"
			"\t -d 'yes' : have the window decorated\n"
			"\t -i '/path/to/icon' : an svg|png|gif icon. No default.\n"
			"\t\t OR a stock GTK icon - eg: \"gtk-apply\"\n\n"
			"\tThe splash window can be dismissed by clicking on it.\n",
			_PROGNAME, _VERSION, _PROGNAME);
}
/* screen size */
static gint get_width() {
	GdkScreen *myscreen;
	myscreen = gdk_screen_get_default();
	static gint width;
	width = gdk_screen_get_width(myscreen);
	return width;
}
static gint get_height() {
	GdkScreen *myscreen;
	myscreen = gdk_screen_get_default();
	static gint height;
	height = gdk_screen_get_height(myscreen);
	return height;
}
/* get font size to scale icons accordingly */
static gshort get_font_size(gchar *f_size) {
	gint i, c;
	gint f = 1;
	c = f;
	gshort new_font_size, old_font_size;
	for (i = strlen(f_size); i > strlen(f_size) - 5; --i) { /* 4 digits */
		if (f_size[i] == '\0') {
			continue;
		} else if (g_ascii_isdigit (f_size[i])) {
			old_font_size = f_size[i] - '0';
			if (c == 1) {
				new_font_size = old_font_size;
			} else {
				old_font_size = old_font_size * f;
				new_font_size = new_font_size + old_font_size;
			}
			f = f * 10;
			if (c >= 4) {
				if (new_font_size == 0)
					new_font_size = 9999; /* bit of a hack */
				font_err((gint)new_font_size, 200); /* stops stupid sizes */
			}
			c++;
		} else {
			break;
		}
	}
	if (new_font_size > 200)
		font_err(new_font_size, 200);
	return new_font_size;
}
static void font_err(gint font_size, gint too_big) {
	g_printf("%d font size is greater than %d. Its too big!\n", 
													font_size, too_big);
	exit (EXIT_FAILURE);
}
/* event handler */
static gboolean button_quit(GtkWidget *widget, GdkEventButton *event, 
					gpointer gdata) {
	if ((event->button == 1) || (event->button == 3)) {
		i_quit(widget, gdata);
	}
	return TRUE;
}
/* time out */
static gboolean timeout_quit(gpointer gdata) {
	gtk_main_quit();
	return TRUE;
}
/* kill it */
static void i_quit(GtkWidget *widget, gpointer gdata) {
	gtk_main_quit();
}
/* signal handler */
static void sig_handler(gint signo) {
	if (signo == SIGINT) /* catch ^C and try to exit gracefully */
		g_printf("\nreceived SIGINT\n");
	timeout_quit(NULL);
}
/* main */
int main(int argc, char **argv) {
	if (argc < 2)
	{
		usage();
		return 1;
	}
	gchar sval[512] = "Hello World!"; /* default */
	gchar cval[32] = "slate gray"; /* default colour */
	gchar kval[32] = "black"; /* default font colour */
	gint tval = 1000; /* default timeout */
	gchar pval[6] = "c"; /* default position */
	gchar fval[32] = "Sans 16"; /* default font */
	gchar dval[32] = "no"; /* default deco */
	gchar ival[PATH_MAX] = "none";
	gint c;
	while ((c = getopt (argc, argv, "c:k:s:t:p:f:d:i:")) != -1) {
		switch (c)
		{
			case 'c':
				g_stpcpy(cval, optarg);
				break;
			case 'k':
				g_stpcpy(kval, optarg);
				break;
			case 's': /* no glib strlen? g_string? Nah! */
				if (strlen(optarg) > 512) { 
					g_printf("Error: Your string is too long\n");
					return 1;
				}
				g_stpcpy(sval, optarg);
				break;
			case 't':
				tval = atoi(optarg);
				break;
			case 'p':
				if (( optarg[0] == 't' ) || ( optarg[0] == 'b' )) {
					if (( optarg[1] == 'l' ) || ( optarg[1] == 'r' ) ||
							( optarg[1] == 'c' )) {
						g_stpcpy(pval, optarg);
					} 
				} else if ( optarg[0] == 'm' ) {
					g_stpcpy(pval, "m"); /* mouse pos */
				} else {
					g_stpcpy(pval, "c"); /* default to centre */
				}
				break;
			case 'f': /* if junk, defaults to system pango alias */
				g_stpcpy(fval, optarg);
				break;
			case 'd':
				g_stpcpy(dval, optarg);
				break;	
			case 'i':
				g_stpcpy(ival, optarg);
				break;	
			default:
				usage();
		}
	}
	if (signal(SIGINT, sig_handler) == SIG_ERR)
		g_printf("\ncan't catch SIGINT\n");
	gtk_init(&argc, &argv);
	display_window(sval, cval, kval, pval, fval, dval, ival);
	if (tval != 1000){
		g_timeout_add_seconds(tval, timeout_quit, NULL);
    }
	gtk_main();
	return 0;
}