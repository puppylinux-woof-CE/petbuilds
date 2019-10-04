/*Simple pMusic tray icon by Iguleder 
 * Based on the tray icon skeleton by Barry Kauler*/
/* gcc `pkg-config --cflags --libs gtk+-2.0` -o pmusic_trayapp pmusic_trayapp.c */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <glib/gstdio.h>
#include <libintl.h>
#include <locale.h>

#define _(STRING)    gettext(STRING)

GdkPixbuf *pmusic_pixbuf;
GError *gerror;

//1 if pMusic GUI is running, otherwise 0
int running=1;
int stopval=1;
int cdval;

void play_next (GtkWidget *tray_icon, gpointer userdata)
{
   system("pmusic -s next &");
}

void play_previous (GtkWidget *tray_icon, gpointer userdata)
{
   system("pmusic -s prev &");
}

void quit_pmusic (GtkWidget *tray_icon, gpointer userdata)
{
   system("pmusic -s quit");
}

void pause_song (GtkWidget *tray_icon, gpointer userdata)
{
   system("pmusic -s pause");
}

void view_popup_menu (GtkWidget *tray_icon, GdkEventButton *event, gpointer userdata)
{

   //add menu icons
   GtkWidget *menu, *menuitem1, *menuitem2, *menuitem3, *menuitem4, *icon1, *icon2, *icon3, *icon4 ;

   menu = gtk_menu_new();

   menuitem1 = gtk_image_menu_item_new_with_label(_("Quit pMusic"));
   icon1 = gtk_image_new_from_stock(GTK_STOCK_QUIT, GTK_ICON_SIZE_MENU);
   gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuitem1), icon1);
   menuitem2 = gtk_image_menu_item_new_with_label(_("Next track"));
   icon2 = gtk_image_new_from_stock(GTK_STOCK_MEDIA_NEXT, GTK_ICON_SIZE_MENU);
   gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuitem2), icon2);
   menuitem3 = gtk_image_menu_item_new_with_label(_("Previous track"));
   icon3 = gtk_image_new_from_stock(GTK_STOCK_MEDIA_PREVIOUS, GTK_ICON_SIZE_MENU);
   gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuitem3), icon3);
   
   //test if we are paused or playing; if stopped we want play icon
   stopval=(system("ps -eo s,command|grep aplay|grep -v grep|head -c1|grep -q 'S'")); //return val changed T
   
   {
	   	{  
		if (stopval == 0) //playing, we want pause icon
			{
			menuitem4 = gtk_image_menu_item_new_with_label(_("Pause"));
			icon4 =  gtk_image_new_from_stock(GTK_STOCK_MEDIA_PAUSE, GTK_ICON_SIZE_MENU);
			gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuitem4), icon4);
			}
		else //not playing (1) or not there (1)
			{
		    menuitem4 = gtk_image_menu_item_new_with_label(_("Play"));
			icon4 =  gtk_image_new_from_stock(GTK_STOCK_MEDIA_PLAY, GTK_ICON_SIZE_MENU);
			gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuitem4), icon4);  
			}
		}
   }	
   
   g_signal_connect(menuitem1, "activate", (GCallback) quit_pmusic, tray_icon);
   g_signal_connect(menuitem2, "activate", (GCallback) play_next, tray_icon);                 
   g_signal_connect(menuitem3, "activate", (GCallback) play_previous, tray_icon);
   g_signal_connect(menuitem4, "activate", (GCallback) pause_song, tray_icon);
                       
   gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem1);
   gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem2);
   gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem3);
   gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem4);
   
   gtk_widget_show_all(menu);

   gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL, (event != NULL) ? event->button : 0, gdk_event_get_time((GdkEvent*)event));

}

gboolean view_onPopupMenu (GtkWidget *tray_icon, gpointer userdata)
{

   view_popup_menu(tray_icon, NULL, userdata);

   return TRUE; /* we handled this */
   
}

void tray_icon_on_click(GtkStatusIcon *tray_icon, gpointer user_data)
{
   // test if a CD is playing and resume appropriate GUI 
   //system("ps -A|grep -q 'cdda2wav';echo $? >/tmp/cdplaying");
   cdval=(system("ps -eo command|grep 'cdda2wav'|awk '{print $1}'|grep -q 'cdda2wav'"));
   
   if (running == 0)
    
   {
	  {  
	  if (cdval == 0)
		  {
			  system("pmusic -j -p '.CD' &");
		  }
		  else
		  {
		      system("pmusic &");
		  }
	  }
      //printf("%d", cdval);
	  running=1;
      gtk_status_icon_set_tooltip(tray_icon, _("Click to hide pMusic"));
   }
   else
   {
      system("pmusic -b &");
      running=0;
      gtk_status_icon_set_tooltip(tray_icon, _("Click to show pMusic"));
   }
   
}

static GtkStatusIcon *create_tray_icon()
{

   GtkStatusIcon *tray_icon;

   tray_icon = gtk_status_icon_new();
       
   g_signal_connect(G_OBJECT(tray_icon), "activate", G_CALLBACK(tray_icon_on_click), NULL);

   g_signal_connect(G_OBJECT(tray_icon), "popup-menu", (GCallback) view_onPopupMenu, NULL);

   pmusic_pixbuf=gdk_pixbuf_new_from_file("/usr/share/icons/hicolor/scalable/apps/pmusic.svg",&gerror);
   gtk_status_icon_set_from_pixbuf(tray_icon,pmusic_pixbuf);
     
   gtk_status_icon_set_tooltip(tray_icon, _("Click to hide pMusic"));   
                                             
   gtk_status_icon_set_visible(tray_icon, TRUE);
       
   return tray_icon;
}

int main(int argc, char **argv)
{
  
   GtkStatusIcon *tray_icon;


   setlocale( LC_ALL, "" ); 
   bindtextdomain( "pmusic_trayapp", "/usr/share/locale" ); 
   textdomain( "pmusic_trayapp" ); 

   gtk_init(&argc, &argv);
   tray_icon = create_tray_icon();
   gtk_main();

   return 0;
}
