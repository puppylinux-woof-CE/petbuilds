/*========================================================================
Gtk Theme Switch, a fast and small Gtk theme switching utility that has a 
GUI and console interface.

Copyright (C) 2009 	Maher Awamy <muhri@muhri.net>
 	     			Aaron Lehmann <aaronl@vitelus.com>
 	     			Joshua Kwan <joshk@triplehelix.org> 
 	     			Pedro Villavicencio Garrido <pvillavi@gnome.cl>
					Denis Briand <denis@narcan.fr>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
=========================================================================*/

#include <gtk/gtk.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

static GList *get_dirs(void);
static void preview_clicked(GtkWidget *button, gpointer data);
static void update_newfont (void);
static void apply_clicked(GtkWidget *button, gpointer data);
static void usage(void);
static void backup_gtkrc(gchar *path_to_gtkrc);
static void ok_clicked(gchar *rc);
static void preview_ok_clicked(gchar *rc);
static GtkTreeModel *create_model (void);
void clist_insert (GtkTreeView *clist);
static void dock(void);
static void preview(gchar *rc_file);
static void preview_window(gchar *rc_file);
static void send_refresh_signal(void);
static short is_themedir (gchar *path, gchar **rc_file);
static short is_installed_theme (gchar *path, gchar **rc_file);
static short install_tarball (gchar *path, gchar **rc_file);
static int switcheroo (gchar *actual);
static void install_clicked (GtkWidget *w, gpointer data);
static void install_ok_clicked (GtkWidget *w, gpointer data);
static void search_for_theme_or_die_trying (gchar *actual, gchar **rc_file);
static void set_font (GtkWidget *w, GtkWidget *dialog);
static void font_browse_clicked (GtkWidget *w, gpointer data);
static short fgrep_gtkrc_for (gchar *needle);
static void get_theme_name (gchar *the_theme_name);
static GList *compare_glists (GList *t1, GList *t2, GCompareFunc cmpfunc);
void quit_preview();
void quit();
void hide_stuff();
void show_stuff();
void on_eventbox_click();

#define INIT_GTK if (!using_gtk) { gtk_init (&argc, &argv); using_gtk = 1; }


/* globals */
GHashTable *hash;
GList *glist=NULL;
GSList *kids=NULL;
gint preview_counter = 0;

gchar	*homedir, /* we use it a lot, so keep it handy */
	*execname, /* == argv[0] */
	*newfont; /* The name of a new font to use as the default if the user has
				 selected one. Otherwise NULL. */
gchar *themename; /*new var to get theme name */

GtkWidget *dockwin, *combo=NULL, *font_entry, *use_font_button, *box, *install_button, *browse;
gint hidden = 1;
/*end globals */

static short fgrep_gtkrc_for (gchar *needle)
{
	gchar *path_to_gtkrc = g_strdup_printf("%s/.gtkrc-2.0", homedir);
	FILE *gtkrc = fopen(path_to_gtkrc, "r");
	char tempbuf[16384], *commentsearch;
	g_free (path_to_gtkrc);

        if (!gtkrc)
          return 0;

	while (!feof (gtkrc))
	{
		fgets (tempbuf, 16383, gtkrc);
		/* Strip comments */
		for (commentsearch = tempbuf; *commentsearch != '\0'; ++commentsearch)
			if (*commentsearch == '#') { *commentsearch = '\0'; break; }
		if (strstr(tempbuf, needle))
		{
			fclose (gtkrc);
			return 1;
		}
	}
	fclose (gtkrc);
	return 0;
}

static GList*
get_dirs (void) 
{
	gchar *dirname, *localthemedir, *globalthemedir, *dname;
	DIR *dir; 
	struct dirent *dent;
	struct stat stats;
	gchar *origdir=g_get_current_dir(); /* back up cwd */
	GList *list=0;
	
	dirname = g_strconcat(homedir,"/.themes",NULL);
	chdir (dirname);
	dir = opendir (dirname);
	if (dir)
	{
		/* ONE copy of the local theme directory for putting in the hash */
		/* NB: Don't g_free() it!! */
		localthemedir = g_strdup(dirname);

		while ((dent = readdir (dir)))
		{
			gchar* gtkrc_path = g_strconcat(dent->d_name, "/gtk-2.0/gtkrc", NULL);
			stat(dent->d_name,&stats);
			if (!S_ISDIR(stats.st_mode)) continue;
			if (strcmp(dent->d_name,"." ) == 0) continue;
			if (strcmp(dent->d_name,"..") == 0) continue;
			if (access(gtkrc_path, R_OK) == -1) continue;

			dname = g_strdup(dent->d_name);
			g_hash_table_insert (hash, dname, localthemedir);
			list = g_list_insert_sorted(list, dname, (GCompareFunc)strcmp);
			g_free(gtkrc_path);
		}

		closedir(dir);
	}

	g_free(dirname);

	dirname = gtk_rc_get_theme_dir();
	chdir (dirname);
	dir = opendir (dirname);
	if (dir)
	{
		/* ONE copy of the global theme directory for putting in the hash */
		/* NB: Don't g_free() it!! */
		globalthemedir = g_strdup(dirname);

		while ((dent = readdir (dir)))
		{
			gchar* gtkrc_path = g_strconcat(dent->d_name,"/gtk-2.0/gtkrc",NULL);
			stat(dent->d_name,&stats);
			if (!S_ISDIR(stats.st_mode)) continue;
			if (strcmp(dent->d_name, "." ) == 0) continue;
			if (strcmp(dent->d_name, "..") == 0) continue;
			if (access(gtkrc_path, R_OK) == -1) continue;

			dname = g_strdup(dent->d_name);
			g_hash_table_insert (hash, dname, globalthemedir);
			list =  g_list_insert_sorted(list, dname, (GCompareFunc)strcmp);
			g_free(gtkrc_path);
		}

		closedir(dir);
	}

	g_free(dirname);

	chdir(origdir); /* Go back to where we were */
	g_free(origdir); /* Now go play like a good little program */

	return list;
}

/* Find the first element in t2 that does not exist in t1.
 * Uses the supplied cmpfunc() for determining equality of list->data
 * strcmp() is the recommended compare function */
static GList*
compare_glists (GList *t1, GList *t2, GCompareFunc cmpfunc)
{
	GList *tmp1;
	for (; t2; t2=t2->next)
	{ 
		short matched = 0;
		for (tmp1=t1; tmp1; tmp1=tmp1->next)
			if ((*cmpfunc)(tmp1->data, t2->data) == 0) { matched = 1; break; }
		if (!matched) return t2;
	}
	return 0;
}

static void
preview_clicked(GtkWidget *button, gpointer data)
{
	G_CONST_RETURN gchar *entry; 
	gchar *dir; 
	gchar *rc;
	gchar *actual;

	entry = gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(combo)->entry));
	dir = g_hash_table_lookup(hash,entry);
	dir = g_strconcat(dir,"/",NULL);
	rc = g_strconcat(dir,entry,NULL);
	actual = g_strconcat(dir,entry,NULL);
	rc = g_strconcat(rc,"/gtk-2.0/gtkrc",NULL);
	update_newfont ();
	preview(rc);
	g_free(rc);
	g_free(actual);
}

/* Update 'newfont' */
static void update_newfont (void)
{
	if (newfont) g_free (newfont);

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(use_font_button)))
	{
		G_CONST_RETURN gchar *newerfont = gtk_entry_get_text (GTK_ENTRY(font_entry));
		if (newerfont && newerfont[0])
		{
			newfont = g_strdup(newerfont);
		}
	}
	else newfont = NULL;
}

static void 
apply_clicked(GtkWidget *button, gpointer data)
{
	G_CONST_RETURN gchar *entry = gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(combo)->entry));
	gchar *dir = g_hash_table_lookup(hash,entry);
	gchar *name = g_strdup_printf ("%s/%s/gtk-2.0/gtkrc", dir, entry);
/*	GtkStyle *style;*/

	update_newfont ();

	ok_clicked(name);
	g_free(name);

	/* make sure we get the ClientEvent ourselves */
	while (gtk_events_pending())
		gtk_main_iteration();

	/* sync the font field with the GTK font */
/*	style = gtk_rc_get_style (font_entry);
	if (style && style->rc_style)
		gtk_entry_set_text (GTK_ENTRY(font_entry), pango_font_description_to_string(style->rc_style->font_desc));*/
}

static void
usage (void)
{
	printf("\
%s command line options:\n\
-h[elp]\t\t\t(display this help message)\n\
-d[ock]\t\t\t(open dock)\n\
file\t\t\t(switch to theme (install if theme is a tarball))\n\
-p[review] file\t\t(preview a theme (installs if file is a tarball))\n\
-i[nstall] theme.tar.gz\t(install a .tar.gz)\n\
-f[ont] fontstring\t(set GTK's main font to fontstring)\n\n\
\
Passing no command line arguments will cause %s to start in dock-mode)\n\n\
\
\"file\" represents any one of (looked for in the listed order):\n\
1) An absolute or relative path to a GTK theme directory.\n\
   A directory is considered a theme directory if it contains a\n\
   gtk/gtkrc file.\n\
2) A gzipped tar file which expands to a GTK theme directory as explained in 1).\n\
3) A GTK theme directory with the name passed located in ~/.themes.\n\
4) A GTK theme directory with the name passed located in the global\n\
   \"gtk_themedir\"\n\
If none of these files are located and found to be correct, the program will\n\
exit with an error.\n",
execname, execname);
}

static void 
get_theme_name (gchar *the_theme_name)
{
	/* hack to get the theme name */
	short slashes=0;
	gint i;
	/* ok, we're like evil or something, but that won't stop us */
	for (i=strlen(the_theme_name); i != 0; --i)
	{
		if (the_theme_name[i] == '/') ++slashes;
		if (slashes == 2) { the_theme_name[i] = '\0'; break; }
	}
	themename = the_theme_name;
	for (i=strlen(the_theme_name) /*different*/; i != 0; --i)
	{
		if (the_theme_name[i] == '/') { themename = &the_theme_name[i+1]; break; }
	}
}
static void
write_rc_file (gchar *include_file, gchar *path)
{
	FILE *gtkrc = fopen(path, "w");
	/*XXX XXX*/	
	if (gtkrc == NULL) {
		GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW(dockwin),
                                  GTK_DIALOG_DESTROY_WITH_PARENT,
                                  GTK_MESSAGE_ERROR,
                                  GTK_BUTTONS_CLOSE,
                                  "Unable to save your preferences to %s: %s.",
								  path,strerror(errno) );
		gtk_window_set_title(GTK_WINDOW(dialog), "Error");
 		gtk_dialog_run (GTK_DIALOG (dialog));
 		gtk_widget_destroy (dialog);
		quit();
	}
	/* the caps stuff is bullshit for gnome */
	fprintf(gtkrc, "# -- THEME AUTO-WRITTEN BY gtk-theme-switch2 DO NOT EDIT\ninclude \"%s\"\n\n", include_file);
	if (newfont)
		/* User set a custom font to overrride defaults. */
		fprintf (gtkrc, "style \"user-font\"\n{\n  font_name=\"%s\"\n}\nwidget_class \"*\" style \"user-font\"\n\n", newfont);
	fprintf(gtkrc, "include \"%s/.gtkrc-2.0.mine\"\n\n# -- THEME AUTO-WRITTEN BY gtk-theme-switch2 DO NOT EDIT\n", homedir);
	/* added get_theme_name (gchar *the_theme_name)
	 * to get themes working for QT */
	get_theme_name(include_file);
	fprintf(gtkrc, "gtk-theme-name = \"%s\"\n", themename);
	fclose (gtkrc);
}

static void
backup_gtkrc(gchar *path_to_gtkrc)
{
	FILE *gtkrc;
	if (!(gtkrc = fopen(path_to_gtkrc, "r")))
		return;
	int c;
	FILE *gtkrc_backup = fopen(g_strdup_printf("%s/.gtkrc-2.0.bak", homedir),"w");
	
	while((c = fgetc(gtkrc)) != EOF){
		fputc(c, gtkrc_backup);
	}
	
	fclose(gtkrc);
	fclose(gtkrc_backup);
}	

static void
ok_clicked (gchar *rc_file)
{
	/* Write the config file to disk */
	gchar *path_to_gtkrc = g_strdup_printf("%s/.gtkrc-2.0", homedir);
	backup_gtkrc(path_to_gtkrc);
	write_rc_file (rc_file, path_to_gtkrc);
	send_refresh_signal();
	g_free(path_to_gtkrc);
}

static void
preview_ok_clicked (gchar *rc_file)
{
	/* Write the config file to disk */
	gchar *path_to_gtkrc = g_strdup_printf("%s/.gtkrc-2.0", homedir);
	rename (rc_file, path_to_gtkrc);
	send_refresh_signal();
	g_free(path_to_gtkrc);
}

static void
install_clicked (GtkWidget *w, gpointer data)
{
	GtkWidget *checkbutton = gtk_check_button_new_with_label ("Switch to theme after installation");
	GtkWidget *fs = gtk_file_selection_new ("Select a GTK theme tarball");
	g_signal_connect(G_OBJECT(GTK_FILE_SELECTION(fs)->ok_button), "clicked", G_CALLBACK(install_ok_clicked), fs);
	g_signal_connect_swapped(G_OBJECT(GTK_FILE_SELECTION(fs)->cancel_button), "clicked", G_CALLBACK(gtk_widget_destroy), (gpointer)fs);
	gtk_box_pack_start (GTK_BOX(GTK_FILE_SELECTION(fs)->main_vbox), checkbutton, FALSE, FALSE, 0);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(checkbutton), FALSE);
	g_object_set_data (G_OBJECT(fs), "checkbutton", checkbutton);
	gtk_widget_show(checkbutton);
	gtk_widget_show(fs);
}

static void
install_ok_clicked (GtkWidget *w, gpointer data)
{
	gchar *rc_file, *beginning_of_theme_name, *thn;
     	G_CONST_RETURN gchar *filename;
	gint pos;
	gboolean cbstate=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(g_object_get_data(G_OBJECT(data), "checkbutton")));
	filename = gtk_file_selection_get_filename(GTK_FILE_SELECTION(data));
     	thn = g_strdup(filename);
	search_for_theme_or_die_trying(thn, &rc_file);
     	g_free(thn);
	gtk_widget_destroy(GTK_WIDGET(data));
	/* this block moved to get_theme_name (gchar *the_theme_name) */
	
	/* get the list item that contains this */
	get_theme_name(rc_file);
	beginning_of_theme_name = themename;
	pos = g_list_position (glist, g_list_find_custom (glist, beginning_of_theme_name, (GCompareFunc) strcmp));
	if (pos != -1)
		/* set combo's item to the newly-installed theme */
/*FIXME		gtk_list_select_item(GTK_LIST(GTK_COMBO(combo)->list), pos);*/
	
	if (cbstate) /* checkbutton pressed */
		apply_clicked(NULL, NULL);

	/* I guess we should free this... */
	g_free (rc_file);
}

static void
set_font (GtkWidget *w, GtkWidget *dialog)
{
	if (newfont) g_free (newfont);
	newfont = gtk_font_selection_dialog_get_font_name (GTK_FONT_SELECTION_DIALOG(dialog));
	gtk_entry_set_text (GTK_ENTRY(font_entry), newfont);
	gtk_widget_destroy (dialog);
}

static void
font_browse_clicked (GtkWidget *w, gpointer data)
{
	GtkWidget *font_dialog;
	font_dialog = gtk_font_selection_dialog_new ("Select Font");
	gtk_font_selection_dialog_set_preview_text (GTK_FONT_SELECTION_DIALOG(font_dialog), "Gtk Theme Switch");
/*	gtk_font_selection_dialog_set_font_name (GTK_FONT_SELECTION_DIALOG(font_dialog), gtk_entry_get_text(GTK_ENTRY(font_entry)));*/
	g_signal_connect (G_OBJECT(GTK_FONT_SELECTION_DIALOG(font_dialog)->ok_button), "clicked", G_CALLBACK(set_font), (gpointer)font_dialog);
	g_signal_connect_swapped (G_OBJECT(GTK_FONT_SELECTION_DIALOG(font_dialog)->cancel_button), "clicked", G_CALLBACK(gtk_widget_destroy), (gpointer)font_dialog);

	gtk_widget_show (font_dialog);
}

void quit()
{
	GSList *l;
     	for (l = kids; l ; l=l->next) {
	     kill(GPOINTER_TO_INT(l->data), 9);
	}
     	exit(0);
}

static void
dock (void)
{
	GtkWidget *label, *button, *pixmap, *evbox;
       	GtkTooltips *tips;
     	
	dockwin = gtk_dialog_new();
     	gtk_widget_realize(dockwin);
	gtk_window_set_title(GTK_WINDOW(dockwin),"Theme Dock");
/*     	gtk_window_set_policy(GTK_WINDOW(dockwin), TRUE, TRUE, FALSE);*/
     	gtk_window_set_resizable(GTK_WINDOW(dockwin), TRUE);
	g_signal_connect(G_OBJECT(dockwin),"destroy",G_CALLBACK(quit),NULL);
	box = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dockwin)->vbox), box, FALSE, FALSE, 0);
	tips = gtk_tooltips_new();
	label = gtk_label_new("Theme: ");
	gtk_box_pack_start(GTK_BOX(box),label,FALSE,FALSE,FALSE);

	combo = gtk_combo_new();
	glist = get_dirs();
	gtk_combo_set_popdown_strings(GTK_COMBO(combo), glist);
	gtk_box_pack_start(GTK_BOX(box),combo,TRUE,TRUE,FALSE);
     	
     	pixmap = gtk_image_new_from_stock(GTK_STOCK_ADD, GTK_ICON_SIZE_BUTTON);

     	evbox = gtk_event_box_new();

     	gtk_tooltips_set_tip(tips,evbox,"click here for more options","private");
     	gtk_widget_set_events(evbox, GDK_BUTTON_PRESS);
     	g_signal_connect(G_OBJECT(evbox), "button_press_event", G_CALLBACK(on_eventbox_click), NULL);
     
     	gtk_container_add(GTK_CONTAINER(evbox), pixmap);
     	gtk_box_pack_start(GTK_BOX(box),evbox,FALSE,FALSE,FALSE);
	gtk_widget_show_all(box);
     
     	box = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dockwin)->vbox), box, FALSE, FALSE, 0);

	use_font_button = gtk_check_button_new_with_label("Use font: ");
	gtk_box_pack_start(GTK_BOX(box), use_font_button, FALSE, FALSE, 0);
	gtk_widget_show(use_font_button);
     
	font_entry = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(box), font_entry, TRUE, TRUE, 0);
	gtk_widget_show(font_entry);
	if (newfont)
	{
		gtk_entry_set_text (GTK_ENTRY(font_entry), newfont);
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(use_font_button), TRUE);
		g_free (newfont);
	}
	else
	{
/*		GtkStyle *style = gtk_rc_get_style (font_entry);
		if (style && style->rc_style)
	       		gtk_entry_set_text (GTK_ENTRY(font_entry), pango_font_description_to_string(style->rc_style->font_desc));*/
	}
	
	newfont = g_strdup(gtk_entry_get_text(GTK_ENTRY(font_entry)));

	if (newfont != 0 && newfont[0])
	{
		/* Very dirty hack...
		   We want to only set the checkbutton to TRUE if the user specified
		   the font. If the name occurs in their ~/.gtkrc file, they probably did.
		   If it isn't, they probably didn't. So, "grep" the file for the font string. */
		if (fgrep_gtkrc_for(newfont))
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(use_font_button), TRUE);
	}

	browse = gtk_button_new_with_label("Browse...");
	g_signal_connect(G_OBJECT(browse), "clicked", G_CALLBACK(font_browse_clicked), NULL);
	gtk_box_pack_start(GTK_BOX(box), browse, FALSE, FALSE, 0);
	
	button = gtk_button_new_with_label("Apply");
	g_signal_connect(G_OBJECT(button),"clicked",G_CALLBACK(apply_clicked),NULL);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG (dockwin)->action_area),button,TRUE,TRUE,FALSE);
	gtk_widget_show(button);
     
	button = gtk_button_new_with_label("Preview");
	g_signal_connect(GTK_OBJECT(button),"clicked",G_CALLBACK(preview_clicked),NULL);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG (dockwin)->action_area),button,TRUE,TRUE,FALSE);
	gtk_widget_show(button);
     
	install_button = gtk_button_new_with_label("Install New Theme");
	g_signal_connect(G_OBJECT(install_button), "clicked", G_CALLBACK(install_clicked), NULL);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG (dockwin)->action_area),install_button,TRUE,TRUE,FALSE);

	gtk_widget_show(dockwin);

}

void on_eventbox_click()
{
     	if (hidden) {
	     	show_stuff();
	} else {
	     	hide_stuff();
	}
}

void hide_stuff()
{
     	gtk_widget_hide(box);
     	gtk_widget_hide(install_button);
     	gtk_widget_hide(browse);
	hidden = 1;
}

void show_stuff()
{
     	gtk_widget_show(box);
     	gtk_widget_show(install_button);
     	gtk_widget_show(browse);
     	hidden = 0;
}

	
static GtkTreeModel *
create_model (void)
{

  GtkListStore *store;
  GtkTreeIter iter;
  gint i;
  gchar *stuff[4][2] = { { "blah1", "blah2" },
	 		{ "blah3", "blah4" },
	 		{ "blah5", "blah6" },
	 		{ "blah7", "blah8" } };

  /* create list store */
  store = gtk_list_store_new (2,
			      G_TYPE_STRING,
			      G_TYPE_STRING);

  /* add data to the list store */
  for (i = 0; i < 4; i++)
    {
      gtk_list_store_append (store, &iter);
      gtk_list_store_set (store, &iter,
			  0, stuff[i][0],
			  1, stuff[i][1],
				-1);
    }

  return GTK_TREE_MODEL (store);
}


static void
preview (gchar *rc_file)
{
	FILE *pipe;
     	gint got_it = 0, it_died = 0;
     	gchar *line;
     	gchar *path = g_strdup_printf ("%s/.gtkrc.tmp-%i", homedir, preview_counter);
	gchar *command = g_strdup_printf ("%s -_dont-use-this %s &", execname, path);
	write_rc_file (rc_file, path);
	
	pipe = popen(command,"r");
     	
     	if (pipe == NULL) {
	     	g_print("gts: error previewing\n");
	     	return;
	}

     	fcntl(fileno(pipe), F_SETFL, O_NONBLOCK);
     	
     	line = (gchar *)g_malloc(1024);     
     	while(!feof(pipe)) {
      		fgets(line,1024,pipe);
	     	line[strlen(line)-1] = '\0';	       
       	     	if (!got_it && !g_ascii_strncasecmp(line,"pid=",4)) {
		     	kids = g_slist_append(kids,GINT_TO_POINTER(atoi(line+4)));
		     	got_it = 1;
		} else if (!it_died && !g_ascii_strncasecmp(line,"die=",4)) {
		     	kids = g_slist_remove(kids,GINT_TO_POINTER(atoi(line+4)));
		     	it_died = 1;
		     	break;
		}
	     
		while (gtk_events_pending())
	     		gtk_main_iteration();
		usleep(50000);
	}
            
     	pclose(pipe);
	g_free (line);
	g_free (path);
	g_free (command);
	++preview_counter;
}

static void
preview_window (gchar *rc_file)
{

	GtkWidget *label;
	GtkWidget *win;
	GtkWidget *button;
	GtkWidget *toggle_button;
	GtkWidget *check_button;
	GtkWidget *sw;
	GtkWidget *clist;
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *radio;
	GtkWidget *radio2;
	GtkWidget *radio3;
	GtkWidget *ok;
	GtkWidget *cancel;
	GtkWidget *hbox2;
	GtkWidget *notebook;
	GtkWidget *text;
     	GtkTreeModel *model;
/*	GtkWidget *popup;
	GtkWidget *item;*/
     	GtkTextBuffer *buff;
	gint argc=1;
	gchar **argv = &execname;
	gchar *default_files[] = { rc_file, NULL};
	gchar *bb  = "Type some text here\nto check if your\ntheme has a problem\nwith the text widget.\nAlso right-click here\nfor a popup-menu sample.\n";
	GSList *group;

	gtk_rc_set_default_files (default_files);
	gtk_init (&argc, &argv);
	
	win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(win), "Gtk Theme Switch theme preview");
/*	gtk_window_set_policy(GTK_WINDOW(win), TRUE, TRUE, FALSE);*/
	g_signal_connect(G_OBJECT(win), "destroy", G_CALLBACK(quit_preview), NULL);
	
	vbox = gtk_vbox_new(FALSE,0);
	notebook = gtk_notebook_new();
	gtk_container_add (GTK_CONTAINER(win), notebook);
	label = gtk_label_new("Page 1");
	gtk_notebook_append_page (GTK_NOTEBOOK(notebook), vbox, label);
	label = gtk_label_new("Label");
	button = gtk_button_new_with_label("Button");
	toggle_button = gtk_toggle_button_new_with_label("Toggle Button");
	check_button = gtk_check_button_new_with_label("Check Button");
	hbox = gtk_hbox_new(FALSE,0);
	radio = gtk_radio_button_new_with_label(NULL,"Radio 1");
	group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (radio));
	radio2 = gtk_radio_button_new_with_label(group,"Radio 2");
	group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (radio2));
	radio3 = gtk_radio_button_new_with_label(group,"Radio 3");
	gtk_box_pack_start((GtkBox*)hbox,radio,FALSE,FALSE,FALSE);
	gtk_box_pack_start((GtkBox*)hbox,radio2,FALSE,FALSE,FALSE);
	gtk_box_pack_start((GtkBox*)hbox,radio3,FALSE,FALSE,FALSE);
	sw = gtk_scrolled_window_new(NULL,NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
     	model = create_model();
	clist = gtk_tree_view_new_with_model(model);
     	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (clist), TRUE);
     	gtk_tree_view_set_search_column (GTK_TREE_VIEW (clist),0);     
     	g_object_unref (G_OBJECT (model));     

	ok = gtk_button_new_with_label("Ok");
	g_signal_connect_swapped(G_OBJECT(ok),"clicked",G_CALLBACK(preview_ok_clicked), (gpointer) rc_file);
	cancel = gtk_button_new_with_label("Cancel");
	g_signal_connect_swapped(G_OBJECT(cancel),"clicked",G_CALLBACK(quit_preview),NULL);
	hbox2 = gtk_hbox_new(FALSE,0);
	gtk_box_pack_start((GtkBox*)hbox2,ok,TRUE,TRUE,FALSE);
	gtk_box_pack_start((GtkBox*)hbox2,cancel,TRUE,TRUE,FALSE);
	gtk_container_add(GTK_CONTAINER(sw),clist);
	gtk_box_pack_start((GtkBox*)vbox,label,FALSE,FALSE,FALSE);
	gtk_box_pack_start((GtkBox*)vbox,button,FALSE,FALSE,FALSE);
	gtk_box_pack_start((GtkBox*)vbox,toggle_button,FALSE,FALSE,FALSE);
	gtk_box_pack_start((GtkBox*)vbox,check_button,FALSE,FALSE,FALSE);
	gtk_box_pack_start((GtkBox*)vbox,hbox,FALSE,FALSE,FALSE);
	gtk_box_pack_start((GtkBox*)vbox,sw,TRUE,TRUE,FALSE);
	gtk_box_pack_start((GtkBox*)vbox,hbox2,FALSE,FALSE,FALSE);

	vbox = gtk_vbox_new (FALSE, 0);
	label = gtk_label_new ("Page 2");
	gtk_notebook_append_page (GTK_NOTEBOOK(notebook), vbox, label);
	label = gtk_label_new ("Insensitive Label");
	gtk_widget_set_sensitive (label, 0);
	gtk_box_pack_start (GTK_BOX(vbox), label, FALSE, FALSE, 0);
	button = gtk_button_new_with_label ("Insensitive Button");
	gtk_widget_set_sensitive (button, 0);
	gtk_box_pack_start (GTK_BOX(vbox), button, FALSE, FALSE, 0);
	sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
     	buff = gtk_text_buffer_new(NULL);
     	gtk_text_buffer_set_text(buff,bb, strlen(bb));
	text = gtk_text_view_new();
     	gtk_text_view_set_buffer(GTK_TEXT_VIEW(text), buff);
				 
	gtk_container_add (GTK_CONTAINER(sw), text);
/*
	popup = gtk_menu_new ();
	gtk_widget_show (popup);
	item = gtk_menu_item_new_with_label ("Menu Entry 1");
	gtk_widget_show (item);
	gtk_menu_append(GTK_MENU(popup), item);
	item = gtk_menu_item_new_with_label ("Menu Entry 2");
	gtk_widget_show (item);
	gtk_menu_append(GTK_MENU(popup), item);
	item = gtk_menu_item_new_with_label ("Menu Entry 3");
	gtk_widget_show (item);
	gtk_menu_append(GTK_MENU(popup), item);
	gtk_signal_connect(GTK_OBJECT(text), "button_press_event", GTK_SIGNAL_FUNC(rightclick), popup);
 */
	gtk_box_pack_start(GTK_BOX(vbox), sw, TRUE, TRUE, 0);

	label = gtk_label_new ("About");
	vbox = gtk_vbox_new (FALSE, 0);
	gtk_notebook_append_page (GTK_NOTEBOOK(notebook), vbox, label);
	label = gtk_label_new ("Gtk Theme Switch\nby Maher Awamy <muhri@muhri.net>\nand Aaron Lehmann <aaronl@vitelus.com>\nhttp://www.muhri.net/nav.php3?node=gts");
	gtk_box_pack_start (GTK_BOX(vbox), label, TRUE, TRUE, 0);

     	clist_insert(GTK_TREE_VIEW(clist));

	gtk_widget_show_all(win);


	g_print("pid=%d\n",getpid());

	gtk_main ();

	unlink (rc_file);
	
	_exit (1); /* no change */ 
}

void quit_preview()
{
	g_print("die=%d\n",getpid());
     	gtk_main_quit();
}

static void
send_refresh_signal(void)
{
	GdkEventClient event;
    	event.type = GDK_CLIENT_EVENT;
	event.send_event = TRUE;
	event.window = NULL;
	event.message_type = gdk_atom_intern("_GTK_READ_RCFILES", FALSE);
	event.data_format = 8;
	gdk_event_send_clientmessage_toall((GdkEvent *)&event);
}

/* Sets rc_file to the rc_file of the theme if the result is true.
 * It is the caller's repsonsibility to free rc_file */
static short is_themedir (gchar *path, gchar **rc_file)
{
	gchar *test_rc_file;
	struct stat info;
	test_rc_file = g_strdup_printf ("%s/gtk-2.0/gtkrc",path);
	if (stat(test_rc_file, &info) == 0 && (S_ISREG(info.st_mode) || S_ISLNK(info.st_mode)))
	{
		/* $path/gtk/gtkrc exists, and is a regular file */
		*rc_file = test_rc_file;
		return 1;
	}
	else
	{
		g_free (test_rc_file);
		return 0;
	}
}

static short install_tarball (gchar *path, gchar **rc_file)
{
	gchar *command, *themedir;
	gint result;
	GList *new_list, *new_theme;

	themedir = g_strdup_printf ("%s/.themes", homedir);

	if (path[0] != '/')
	{
		gchar *cwd = g_get_current_dir();
		command = g_strdup_printf ("tar --directory %s -xzf %s/%s 2>/dev/null", themedir, cwd, path);
		g_free (cwd);
	}
	else
		command = g_strdup_printf ("tar --directory %s -xzf %s 2>/dev/null", themedir, path);

	/* Ensure that ~/.themes exists */
	mkdir (themedir, S_IRUSR | S_IWUSR | S_IXUSR);

	result = system(command);
	g_free (command);
	g_free (themedir);
	if (result != EXIT_SUCCESS)
		return 0;
	/* We don't do that anymore. Now we find the first new directory
	 * in either themedir, and presume that to be the name of the new theme.
	 * NOT FOOLPROOF, but good as long as you don't mess with stuff while the
	 * program is running. At least better than deriving the theme name from
	 * the tarball name. Ugh. */
	
	new_list = get_dirs();
	new_theme = compare_glists(glist, new_list, (GCompareFunc)strcmp);
	if (new_theme)
	{
		result = is_installed_theme(new_theme->data, rc_file);
		g_list_foreach (glist, (GFunc)g_free, NULL);
		g_list_free (glist);
		glist = new_list;
		/* Update combo, but only in dock mode */
		if (combo)	gtk_combo_set_popdown_strings(GTK_COMBO(combo), glist);
	}
	else
	{
		gchar *interestingpath, basename[1024];
		g_list_foreach (new_list, (GFunc)g_free, NULL);
		g_list_free (new_list);
		/* fall back to our old hack if no new dir was created, say if the theme
		 * was already installed... */
	
		/* Set rc_file to our best darn guess of the resultant gtk theme
		 * dir/gtk/gtkrc. This is very tricky. The hack that is used to is
		 * to return the segment in path between the last slash and the
		 * first dot or dash after that. */
		interestingpath = &path[strlen(path)-1];
		while (interestingpath != path && *(interestingpath-1) != '/') interestingpath--;
		strcpy (basename, interestingpath);
		for (interestingpath = &basename[0]; *interestingpath != '\0'; ++interestingpath)
		{
			if (*interestingpath == '-' || *interestingpath == '.')
			{
				*interestingpath = '\0';
				break;
			}
		}
		result = is_installed_theme(basename, rc_file);
	}
	
	return result;
}

static short
is_installed_theme (gchar *path, gchar **rc_file)
{
	gchar *gtk_rc_theme_dir = gtk_rc_get_theme_dir();
	/* don't strlen things twice when computing the size to use for fullpath */
	gint path_len = strlen(path), homedir_len = strlen(homedir);
	/* ditto with get_theme_dir */
	gint gtk_rc_theme_dir_len = strlen(gtk_rc_theme_dir);
	gchar *fullpath = (gchar *) g_malloc (MAX(homedir_len+path_len+10,
										  gtk_rc_theme_dir_len+path_len+1));
	short result;

	/* use memcpy since we already have the lengths */
	memcpy (fullpath, homedir, homedir_len);
	memcpy (fullpath+homedir_len, "/.themes/", 9);
	/* add 1 to length so we get the null char too */
	memcpy (fullpath+homedir_len+9, path, path_len+1);

	if (is_themedir(fullpath, rc_file))
	{
		g_free(fullpath);
		return 1;
	}
	memcpy (fullpath, gtk_rc_theme_dir, gtk_rc_theme_dir_len);
	/* add 1 to length so we get the null char too */
	memcpy (fullpath+gtk_rc_theme_dir_len, path, path_len+1);
	
	result = is_themedir(fullpath, rc_file);

	g_free(fullpath);

	return result;
}

static void
search_for_theme_or_die_trying (gchar *actual, gchar **rc_file)
{
	if (!is_themedir(actual, rc_file) &&
		!install_tarball(actual, rc_file) &&
		!is_installed_theme(actual, rc_file))
	{
		fprintf (stderr, "\
%s: Sorry, \"%s\" does not appear to be a valid theme directory or tarball!\n", execname, actual);
		exit (EXIT_FAILURE);
	}
}

static gint
switcheroo (gchar *actual)
{
	gchar *rc_file;
	search_for_theme_or_die_trying (actual, &rc_file);
	/* If we get here, we actually found the theme */
	ok_clicked(rc_file);
	g_free (rc_file);
	return EXIT_SUCCESS;
}



int main (int argc, char *argv[])
{
	gchar *rc_file;
	gchar *actual;
	gint i;
	gint result = EXIT_SUCCESS;
     	GSList *l=NULL;
	short using_gtk = 0;

	newfont = 0;
	execname = argv[0];
	homedir = getenv("HOME");
	if(homedir == NULL)
	{
		fprintf(stderr, "No HOME variable in environment, bailing out\n");
		return 0;
	}
	hash = g_hash_table_new (g_str_hash, g_str_equal);

	if (argc == 1) /* start in dock mode auto if no args */
	{
		INIT_GTK
		dock();
	}

	else if (strcmp(argv[1], "-_dont-use-this") == 0)
	{
		preview_window (argv[2]); /* GARGARGAR */
	}				  /* hehe, aaronl is crazy */
	
	for (i=1; i != argc; ++i)
	{
		if (argv[i][0] == '-')
		{
			/* Single-arg commands/options */
			if (argv[i][1] == 'd')
			{
				INIT_GTK
				dock();
				continue;
			}
			
			/* Double-arg commands/options */
			if (i+1 != argc)
			{
				if (argv[i][1] == 'p')
				{
					glist = get_dirs ();
					actual = argv[++i];
					if (!is_themedir(actual, &rc_file) && !install_tarball(actual, &rc_file) && !is_installed_theme(actual, &rc_file))
					{
						fprintf (stderr, "\
%s: Sorry, \"%s\" does not appear to be a valid theme directory or tarball!\n", execname, actual);
						result = EXIT_FAILURE;
					}
					else
					{
						preview (rc_file);
						g_free (rc_file);
					}
					continue;
				}

				if (argv[i][1] == 'i')
				{
					actual = argv[++i];
					if (!install_tarball(actual, &rc_file))
					{
						fprintf (stderr, "\
%s: Sorry, \"%s\" does not appear to be a valid theme tarball!\n", execname, actual);
					}
					result = EXIT_FAILURE;
					continue;
				}

				if (argv[i][1] == 'f')
				{
					newfont = g_strdup (argv[++i]);
					continue;
				}
			}

			/* if this starts with a minus, it's either an unrecognized command
			 * or -help. Perfect */
			usage();
			result = EXIT_FAILURE;
			continue;
		}
		/* got here... fallback and assume it's a theme */
		glist = get_dirs ();
		gtk_init (&argc, &argv);
		result |= switcheroo(argv[i]);
	}

	if (using_gtk) {
		gtk_main();
		for (l=kids;l;l=l->next) {
		     kill(GPOINTER_TO_INT(l->data), 9);
			
		}
	}
     
     	return result;
}

void clist_insert(GtkTreeView *clist)
{
     GtkCellRenderer *renderer;
     GtkTreeViewColumn *column;


     renderer = gtk_cell_renderer_text_new();
     column = gtk_tree_view_column_new_with_attributes("Column 1",
						       renderer,
						       "text",
						       0,
						       NULL);


     gtk_tree_view_column_set_sort_column_id(column, 0);
     gtk_tree_view_append_column(clist, column);

     
     renderer = gtk_cell_renderer_text_new();

     column = gtk_tree_view_column_new_with_attributes("Column2",
						       renderer,
						       "text",
							1,
						       NULL);

     gtk_tree_view_column_set_sort_column_id(column, 1);
     gtk_tree_view_append_column(clist, column);

     return;
}
