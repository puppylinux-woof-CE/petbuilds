/*BK based on a simple systray applet example by Rodrigo De Castro, 2007*/
/*sys tray applet to monitor free personal storage space
  GPL license /usr/share/doc/legal/gpl-2.0.txt.
  freememapplet_tray is started in /usr/sbin/delayedrun and pfbpanel.
  2.1: changed from png to included xpm images, passed param not needed.
  2.2: 100517: always read free space in save file, not RAM space.
  2.2: pet pkg has moved executable to /root/Startup.
  2.3: 100820: 01micko: PUPMODE==2 fix.
  2.3.1: BK 110805 testing with PUPMODE=2, needs fix, there is no /dev/root.
   (puppy has a fixed df that returns actual partition instead of /dev/root)
  2.4 (20120519): rodin.s: added gettext.
  2.8.6: gyro See http://www.murga-linux.com/puppy/viewtopic.php?t=97351

  requires icons from /usr/share/pixmaps/puppy
  */

#include <libintl.h>
#include <locale.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <glib/gstdio.h>
#include <sys/statfs.h>
#include <string.h>

#define MAXCHARNUM 16
#define MAXCHARMSG 256

#define _(STRING)    gettext(STRING)

#define PFILE "/etc/rc.d/PUPSTATE"

#define QUIT _("Quit")
#define RESIZE _("Resize personal storage")
#define STORAGE_MSG _("personal storage, free space")
#define RIGHT_MENU _("Resize personal storage, right click for menu.")

GtkStatusIcon *tray_icon;
guint interval = 10; /*update interval in seconds*/
unsigned long long sizetotal;
unsigned long long sizefree;
unsigned long long sizefreeprev = 0;
gchar memdisplayfree[MAXCHARNUM];
gchar memdisplaytotal[MAXCHARNUM];
gchar memdisplaylong[MAXCHARMSG];
gchar* save_layer_dir;
unsigned int ptoffset;
int percentfree;

GdkPixbuf *grey_pixbuf;
GdkPixbuf *critical_pixbuf;
GdkPixbuf *ok_pixbuf;
GdkPixbuf *good_pixbuf;
GdkPixbuf *excellent_pixbuf;

GError *gerror = NULL;

gboolean pupSavefile = FALSE;

gboolean Update(gpointer ptr);

void getFileSystemData(const char* fln)
{
	struct statfs vfs;
	int retval;
	retval = statfs(fln, &vfs);
	if (retval == 0)
	{
		sizetotal = vfs.f_blocks;
		sizetotal = (sizetotal * vfs.f_bsize) / 1048576;
		sizefree = vfs.f_bavail;
		sizefree = (sizefree * vfs.f_bsize) / 1048576;
	}
	else
	{
		sizetotal = sizefree = 0;
	}
}

gboolean Update(gpointer ptr) {
	
    //read free personal storage...
	getFileSystemData(save_layer_dir);

	if (sizefreeprev == sizefree)
		return TRUE; //unchanged.
    sizefreeprev=sizefree;
    
    //format display...
    if ( sizefree < 1000 ) g_snprintf(memdisplayfree, MAXCHARNUM, "%lldMiB", sizefree);
    else if (sizefree >= 10240) g_snprintf(memdisplayfree, MAXCHARNUM, "%lldGiB", (sizefree/1024)); //>=10G
    else g_snprintf(memdisplayfree, MAXCHARNUM, "%.1fGiB", (float)(sizefree/1024.0));
    if ( sizetotal < 1000 ) g_snprintf(memdisplaytotal, MAXCHARNUM, "%lldMiB", sizetotal);
    else if (sizetotal >= 10240) g_snprintf(memdisplaytotal, MAXCHARNUM, "%lldGiB", (sizetotal/1024)); //>=10G
    else g_snprintf(memdisplaytotal, MAXCHARNUM, "%.1fGiB", (float)(sizetotal/1024.0));
    
    //update tooltip...
    if (pupSavefile)
		g_snprintf(memdisplaylong, MAXCHARMSG, "%s %s %s\n%s", memdisplaytotal, STORAGE_MSG, memdisplayfree, RIGHT_MENU);
    else
		g_snprintf(memdisplaylong, MAXCHARMSG, "%s %s %s", memdisplaytotal, STORAGE_MSG, memdisplayfree);
    gtk_status_icon_set_tooltip(tray_icon, memdisplaylong);

    //update icon... (sizefree,sizetotal are in MB)
    if (gtk_status_icon_get_blinking(tray_icon)==TRUE) gtk_status_icon_set_blinking(tray_icon,FALSE);
	if (sizetotal == 0)
		percentfree = 0;
	else
		percentfree=(sizefree*100)/sizetotal;
    if (sizefree < 20) {
        gtk_status_icon_set_from_pixbuf(tray_icon,critical_pixbuf);
        gtk_status_icon_set_blinking(tray_icon,TRUE);
    }
    else if (sizefree < 50) {
        gtk_status_icon_set_from_pixbuf(tray_icon,critical_pixbuf);
    }
    else if (percentfree < 20) {
        gtk_status_icon_set_from_pixbuf(tray_icon,critical_pixbuf);
    }
    else if (percentfree < 45) {
        gtk_status_icon_set_from_pixbuf(tray_icon,ok_pixbuf);
	}
    else if (percentfree < 70) {
        gtk_status_icon_set_from_pixbuf(tray_icon,good_pixbuf);
	}
    else {
        gtk_status_icon_set_from_pixbuf(tray_icon,excellent_pixbuf);
	}
	return TRUE;
}

void quit() 
{
	gtk_main_quit();
}

void resize_pupsave()
{
	system("resizepfile.sh &");
}

void tray_icon_on_click(GtkStatusIcon *status_icon, gpointer user_data)
{
    system("partview &");
}

void tray_icon_on_menu(GtkStatusIcon *status_icon, guint button, guint activate_time, gpointer user_data)
{
    GtkWidget *menu, *menuitem, *icon;
	menu = gtk_menu_new();
	if (pupSavefile) 
	{
		menuitem = gtk_image_menu_item_new_with_label(RESIZE);
		icon = gtk_image_new_from_stock(GTK_STOCK_EXECUTE, GTK_ICON_SIZE_MENU);
		gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuitem), icon);
		g_signal_connect(menuitem, "activate", (GCallback) resize_pupsave, status_icon);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	}
	
	menuitem = gtk_image_menu_item_new_with_label(QUIT);
	icon = gtk_image_new_from_stock(GTK_STOCK_QUIT, GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuitem), icon);
	g_signal_connect(menuitem, "activate", (GCallback) quit, status_icon);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	
	gtk_widget_show_all(menu);
	gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL, button, gdk_event_get_time(NULL));
}

static GtkStatusIcon *create_tray_icon() {

    tray_icon = gtk_status_icon_new();
    g_signal_connect(G_OBJECT(tray_icon), "activate", G_CALLBACK(tray_icon_on_click), NULL);
    g_signal_connect(G_OBJECT(tray_icon), "popup-menu", G_CALLBACK(tray_icon_on_menu), NULL);

	grey_pixbuf=gdk_pixbuf_new_from_file("/usr/share/pixmaps/puppy/container_0.svg",&gerror);
    critical_pixbuf=gdk_pixbuf_new_from_file("/usr/share/pixmaps/puppy/container_1.svg",&gerror);
    ok_pixbuf=gdk_pixbuf_new_from_file("/usr/share/pixmaps/puppy/container_2.svg",&gerror);
    good_pixbuf=gdk_pixbuf_new_from_file("/usr/share/pixmaps/puppy/container_3.svg",&gerror);
    excellent_pixbuf=gdk_pixbuf_new_from_file("/usr/share/pixmaps/puppy/container_4.svg",&gerror);

    gtk_status_icon_set_visible(tray_icon, TRUE);

    return tray_icon;
}

int main(int argc, char **argv) {
    
	gchar** linesArray = NULL;
	gchar* linesString = NULL;
	gchar* newLine = "\n";
	gchar* curLine;
	gchar* curVal;
	guint numLines;
	gchar** chunksArray = NULL;
	gchar* chunkSep = "'";
	guint psave;
	gboolean havePupstate = FALSE;
	gboolean haveSaveLayer = FALSE;

    setlocale( LC_ALL, "" );
    bindtextdomain( "freememapplet_tray", "/usr/share/locale" );
    textdomain( "freememapplet_tray" );

	save_layer_dir = NULL;
	if (g_file_get_contents(PFILE, &linesString, NULL, NULL))
	{
		havePupstate = TRUE;
		linesArray = g_strsplit(linesString, newLine, 0);
		numLines = g_strv_length(linesArray);
		for (psave = 0; psave < numLines; psave ++)
		{
			curLine = linesArray[psave];
			if (g_str_has_prefix(curLine, "PUPSAVE="))
			{
				chunksArray = g_strsplit(curLine, chunkSep, 0);
				if (g_strv_length(chunksArray) > 1)
				{
					curVal = chunksArray[1];
					if ((g_str_has_suffix(curVal, ".2fs")) ||
						(g_str_has_suffix(curVal, ".3fs")) ||
						(g_str_has_suffix(curVal, ".4fs")))
						pupSavefile = TRUE;
				}
				g_strfreev(chunksArray);
			}
			else if (g_str_has_prefix(curLine, "SAVE_LAYER="))
			{
				chunksArray = g_strsplit(curLine, chunkSep, 0);
				if (g_strv_length(chunksArray) > 1)
				{
					curVal = chunksArray[1];
					if (strlen(curVal) > 0)
						save_layer_dir = g_strconcat("/initrd", curVal, NULL);
				}
				g_strfreev(chunksArray);
			}
		}
		if (save_layer_dir == NULL)
		{
			save_layer_dir = g_strdup("/");
			haveSaveLayer = TRUE;
		}
		else
		{
			if (g_file_test(save_layer_dir, G_FILE_TEST_EXISTS))
			{
				haveSaveLayer = TRUE;
			}
			else
			{
				g_free(save_layer_dir);
				save_layer_dir = NULL;
			}
		}	
   	}

	g_free(linesString);
	g_strfreev(linesArray);

	if ((havePupstate) && (haveSaveLayer))
	{
		gtk_init(&argc, &argv);
		tray_icon = create_tray_icon();
			
		g_timeout_add_seconds(interval, Update, NULL);
		Update(NULL);

		gtk_main();

		g_free(save_layer_dir);

		return 0;
	}

	if (havePupstate)
		printf("SAVE_LAYER line is badly formatted.\n");
	else
		printf("PUPSTATE file is missing.\n");

	return 1;
}
