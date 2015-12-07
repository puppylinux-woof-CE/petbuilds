#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <glib/gstdio.h>
#define THIS_VERSION "0.1"
#include <libintl.h>
#include <locale.h>
#define _(STRING)    gettext(STRING)

GtkStatusIcon *tray_icon;

unsigned int interval = 2000; /*update interval in milliseconds */

int running();
int running() {
	int is_running = system("pidof xvkbd");
	return is_running;
}

gboolean keyboard_state(gpointer ptr);
gboolean keyboard_state(gpointer ptr) {    /* This is the constantly updated routine */
	if (running() == 0) {
		gtk_status_icon_set_from_file(tray_icon,"/usr/share/pixmaps/puppy/keyboard_shortcut.svg" );
		gtk_status_icon_set_tooltip(tray_icon, _("Click to close X Keyboard"));
	} else {
		gtk_status_icon_set_from_file(tray_icon,"/usr/share/pixmaps/puppy/keyboard.svg" );
		gtk_status_icon_set_tooltip(tray_icon, _("Click to open X Keyboard"));
	}
	return(1);
}

void start_keyboard();
void start_keyboard() {
	system("x-keyboard &");
}

void stop_keyboard();
void stop_keyboard() {
	system("r=$(pidof xvkbd); kill -9 $r");
}

/* Quit and remove from starting */
void quit(GtkWidget *w, gpointer dummy);
void quit(GtkWidget *w, gpointer dummy) {
    int r = 0;
    r = remove("/root/.config/autostart/x-key-state.desktop"); /*  removes applet from Startup */
    /* legacy, in case it is in /root/startup */
    if (r != 0) {
		remove("/root/Startup/x-key-state"); 
	}
    gtk_main_quit();
}


/* What right click does, calls gtk menu with "items" */
void tray_icon_on_menu(GtkStatusIcon *status_icon, 
		guint button, guint activate_time, gpointer user_data);
void tray_icon_on_menu(GtkStatusIcon *status_icon,  
		guint button, guint activate_time, gpointer user_data) {
	
	
	GtkWidget *menu, *label, *iconw;
	menu = gtk_menu_new();

	int run = running();
	if (run == 0) {
		label = gtk_image_menu_item_new_with_label(_("Stop X Keyboard"));
	    iconw = gtk_image_new_from_stock(GTK_STOCK_NO, GTK_ICON_SIZE_MENU);
	    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(label), iconw);
	    gtk_menu_shell_append(GTK_MENU_SHELL(menu), label);
		g_signal_connect(label, "activate", (GCallback) stop_keyboard, status_icon); 
	} else {	
		label = gtk_image_menu_item_new_with_label(_("Start X Keyboard"));
	    iconw = gtk_image_new_from_stock(GTK_STOCK_YES, GTK_ICON_SIZE_MENU);
	    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(label), iconw);
	    gtk_menu_shell_append(GTK_MENU_SHELL(menu), label);
		g_signal_connect(label, "activate", (GCallback) start_keyboard, status_icon); 
    }
	
    label = gtk_image_menu_item_new_with_label(_("Quit"));
    iconw = gtk_image_new_from_stock(GTK_STOCK_QUIT, GTK_ICON_SIZE_MENU);
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(label), iconw);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), label);
    g_signal_connect(label, "activate", (GCallback) quit, status_icon);	    


	gtk_widget_show_all(menu);
	gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL, button, gdk_event_get_time(NULL));
}

/* What left click does */
void tray_icon_on_click(GtkStatusIcon *status_icon, gpointer user_data);
void tray_icon_on_click(GtkStatusIcon *status_icon, gpointer user_data)  {  
	
	int run = running();
	if (run != 0) { /*0 means it IS running! */
		system("x-keyboard &");
	}else {
		system("r=$(pidof xvkbd); kill -9 $r");
	}
}
 
/* -create icon and 'click' properties */
static GtkStatusIcon *create_tray_icon();
static GtkStatusIcon *create_tray_icon() {

	tray_icon = gtk_status_icon_new();

	g_signal_connect(G_OBJECT(tray_icon), "activate", G_CALLBACK(tray_icon_on_click), NULL);

	g_signal_connect(G_OBJECT(tray_icon), "popup-menu", G_CALLBACK(tray_icon_on_menu), NULL);

	gtk_status_icon_set_from_file(tray_icon,"/usr/share/pixmaps/puppy/keyboard.svg" );
	gtk_status_icon_set_tooltip(tray_icon, _("Click to open X Keyboard") );
	
	gtk_status_icon_set_visible(tray_icon, TRUE);

	return tray_icon;
}

int main(int argc, char **argv) {
	
	setlocale( LC_ALL, "" );
	bindtextdomain( "x-key-state", "/usr/share/locale" );
	textdomain( "x-key-state" );

	gtk_init(&argc, &argv);
		
	tray_icon = create_tray_icon();
                        
	g_timeout_add(interval, keyboard_state, NULL);
	gtk_main();

	return 0;
}

