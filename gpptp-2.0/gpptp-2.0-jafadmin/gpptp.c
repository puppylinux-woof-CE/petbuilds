/*
 * GTK interface for pptp-client
 * 
 * Copyright 2006 Andrew Kozin <stanson@btv.ru> under the terms of the
 * GNU GPL.
 *
 * Very simple frontend for VPN connections
 * 
 *  	- Further fixing up by jafadmin (jafa) 2009	- 2014 - jafadmin@gmail.com  
 * 		
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <gtk/gtk.h>
#include <math.h>
#include <signal.h>
#include <unistd.h>

/* jafa globals ...*/
int ppp_pts;
char cache_path[256];
char pid_cache[256];
char dns_cache[256];
#define 	NOPID				-1
#define	NOFILE				0
#define  	MATCH				0
#define  	DNS_COPY			1
#define 	DNS_RESTORE	0

GtkWidget *radio_button_1, *radio_button_2, *radio_button_3;
/* end jafa globals  */


GPid	pptp_pid = NOPID;
GtkTextTag	*tag_error;
GtkTextTag	*tag_info;
GtkTextMark	*end_mark;


typedef struct _widgets {
	GtkWidget *server;
	GtkWidget *login;
	GtkWidget *passwd;
	GtkWidget *start;
	GtkWidget *stop;
	GtkWidget *log;
} GpptpWidgets;

void SetRouting( void )
{
	char FileName[256], str[256];
	FILE *InFile, *OutFile;
	
	
   	if( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( radio_button_1 ) ) )
      		strcpy(FileName, "/etc/ppp/gpptp/ip-up.default" );
    else  if( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( radio_button_2 ) ) )
      		strcpy(FileName, "/etc/ppp/gpptp/ip-up.1918" );
    else if ( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( radio_button_3 ) ) )  
     			strcpy(FileName, "/etc/ppp/gpptp/ip-up.custom" );
     	
	if((InFile = fopen(FileName, "r"))) 
	{
		if(access("/etc/ppp/ip-up", F_OK) != -1)  // ip-up file already exists, rename it.
				rename("/etc/ppp/ip-up", "/etc/ppp/.ip-up.keeper");
		OutFile=fopen("/etc/ppp/ip-up", "w");
		while( fgets( str, 256, InFile) ) fputs(str, OutFile);
		fclose(InFile); fclose(OutFile);
		chmod("/etc/ppp/ip-up", 0755);
	}
}

void stretch(char *array, int array_size) 	/* XOR Cypher */
{
    int i;
    char str[] = "t|!p(`$v6pP&+^x~$T'2{#~?P)4.=l"; //30 character key
    for(i = 0; i < array_size; i++)
        array[i] ^= str[i];
}

void log_add(GtkWidget *w, GtkTextTag *tag, gchar *str)
{
  GtkTextBuffer	*buf;
  GtkTextIter	end_iter;

	buf = gtk_text_view_get_buffer (GTK_TEXT_VIEW (w));
	gtk_text_buffer_get_end_iter (buf, &end_iter);
	gtk_text_buffer_insert_with_tags (buf, &end_iter, str, -1, tag, NULL);
	gtk_text_buffer_get_end_iter (buf, &end_iter);
	gtk_text_buffer_move_mark_by_name (buf, "end", &end_iter);
	gtk_text_view_scroll_mark_onscreen (GTK_TEXT_VIEW (w),
				gtk_text_buffer_get_mark (buf, "end"));
}

void add_entries_from_file(GtkWidget *combo_box, char * filename, char * stops)
{
  FILE		*f;
  char		buf[256], *s, *e;
	f = fopen(filename,"r");
	if(f)
	{
		while(fgets(buf,sizeof(buf),f))
		{
			s = buf;
			while((*s == ' ') || (*s == '\t')) s++;
			if((!*s) || (*s == '#') || (*s == '\n')) continue;
			e = s;
			while((*e) && !strchr(stops, *e)) e++;
			*e = 0;
			if(e != s)
			{
				gtk_combo_box_append_text(GTK_COMBO_BOX(combo_box), s);
			}
		}
		fclose(f);
	}
}

/*  If the user envokes Gpptp from within a script "wrapper" in order to perform 
 * 	additional network configuration like setting routes, gateways, or DNS after 
 * 	connecting, we have made it possible for them to click the "Close window" 
 * 	(but stay connected) button so the app will exit normally  (0) and the script can 
 * 	continue with it's mischief.  In so doing, we need to create a record of the pid 
 * 	of the pppd process before exiting so they can re-fire the app later and still 
 * 	kill the  running  zombie process using the "Disconnect" button.
 * 
 * 	This  function handles the caching logic...   (sorry about the name  :D  )
 * 		- jafa
 */
int Cache_or_Check(int pid)
{  		
	FILE *fptr;  char buf1[25], buf2[25] ; int foo;			
	
	/* We have no pid and need to "Check"  if we have one cached from a previous session. */
	if((fptr = fopen(pid_cache, "r")) &&  pid == NOPID) 
	{
		fscanf(fptr, "%d", &pid);				
		fclose(fptr);
		sprintf(buf1, "/proc/%d/stat", pid);
		if((fptr = fopen(buf1, "r")))   /* see if there is actually a process running with that pid */
		{
			fscanf(fptr, "%d (%[^)]", &foo, buf2);   /* read the process name (2nd item) to see if it is "pppd" */
			if(strcmp("pppd", buf2) != MATCH)
			{
				 remove(pid_cache);
				 pid = NOPID;
			}											
		}	
		else 
		{
			remove(pid_cache);
			pid = NOPID;
		}
	}
	else if(pid != NOPID)	/*  We have a real pid and need to create the "Cache" file for it.  */
	{				
		if((fptr = fopen(pid_cache, "w" )))
		{				
			fprintf(fptr, "%d", pid);					
			fclose(fptr);		
		}		
	}		
	return(pid);	
}	


/*  This function checks the system for "our" pppd pid because the call  to
 * 	"g_spawn_async_with_pipes()" doesn't return the pid of the pppd (grandchild) process, 
 * 	instead it returns the pid of the pptp (child) process. So, to find the pid of the pppd 
 *  process we search the "/var/run" folder for the most recently created (highest numbered)  
 * 	"ppp#.pid" file.  That file will contain the pid number we really need. We'll cache the info 
 * 	in a file in the folder pointed to in the "$HOME"   environment variable .
 * 
 * 	  Parent          		child                 		grandchild
 * 	"gpptp" calls -> "pptp"  which calls  -> "pppd"
 * 
 * 		- jafa
 * */
int get_pppd_pid(void) 
{		
	FILE *fptr;  int our_pid = NOPID; char ppp_path[25] , buf[25];  
			
	strcpy(ppp_path,  "/var/run/ppp0.pid"); 
	
	 /* Pause 5 seconds waiting for the ppp# to be created by pppd  -  (this will be system dependant..),
	  *  	otherwise a false connect often happens ... */
	sleep(5);
	
	if((fptr=fopen(ppp_path, "r")))
	{	 	
		fclose(fptr);
		++ppp_path[12]; 
   		while( (fptr=fopen(ppp_path, "r")))
		{
			fclose(fptr);						
			++ppp_path[12];  /* This will harmlessly fail after 10 times, but who 
												the heck  has 10 ppp tunnels running at once?!? 	*/															
		}
		--ppp_path[12] ;
		if((fptr=fopen(ppp_path, "r")))
		{
			our_pid = atoi(fgets(buf, 25, fptr));
			fclose(fptr);		
		}
	}
	ppp_pts = ppp_path[12] - 48;
	return(our_pid );	
}

/*  Simple function to copy,  then restore the "/etc/resolv.conf" DNS
 *  file when we properly disconnect our VPN session.  - jafa
 * */
void fix_dns(int save_mode)
{
	FILE *fptr1, *fptr2; 	char str[256];
	
	if( save_mode ==DNS_COPY)
	{							
		if((fptr1= fopen("/etc/resolv.conf", "r"))) //DNS
		{
			fptr2=fopen(dns_cache, "w");
			while( fgets( str, 256, fptr1) ) fputs(str, fptr2);
			fclose(fptr1); fclose(fptr2);
		}
	}			
			
	if(save_mode == DNS_RESTORE)
	{
		if((fptr1=fopen(dns_cache, "r")))  // DNS
		{
			fptr2= fopen("/etc/resolv.conf", "w");
			while( fgets(str, 256,  fptr1)) fputs(str, fptr2);
			fclose(fptr1); fclose(fptr2);
		}
	}			
}

gboolean on_pptp_out (GIOChannel *src, GIOCondition cond, gpointer data)
{
  GpptpWidgets	*w = (GpptpWidgets *)data;
  gchar		*line;
  GIOStatus	status;

	status = g_io_channel_read_line (src, &line, NULL, NULL, NULL);
	
	if (status != G_IO_STATUS_NORMAL)
		return FALSE;
	
	log_add(w->log, NULL, line);
	
	g_free (line);
	
	return TRUE;
}

gboolean on_pptp_err (GIOChannel *src, GIOCondition cond, gpointer data)
{
  GpptpWidgets	*w = (GpptpWidgets *)data;
  gchar		*line;
  GIOStatus	status;

	status = g_io_channel_read_line (src, &line, NULL, NULL, NULL);
	
	if (status != G_IO_STATUS_NORMAL)
		return FALSE;
	
	log_add(w->log, tag_error, line);
	
	g_free (line);
	
	return TRUE;
}

void on_pptp_exit (GPid pid, gint status, gpointer data)
{
  	GpptpWidgets	*w = (GpptpWidgets *)data;
  	gchar		str[256];  

/* pptp_pid  actually needs to be set to the pid of the 
 * 	grandchild pppd process. - jafa
*/ pptp_pid =get_pppd_pid(); 
	
  	snprintf(str, sizeof(str), "pptp process (%d) exited with %d\n", pid, status);
	log_add(w->log, tag_info, str);	
	if(pptp_pid == NOPID)
	{
		snprintf(str, sizeof(str), "ppp process was NOT created!\n"); 
		log_add(w->log, tag_error, str);	
		gtk_widget_set_sensitive (GTK_WIDGET (w->start), TRUE);
		gtk_widget_set_sensitive (GTK_WIDGET (w->stop), FALSE);
		
		// Restore ip-up file
		if(access("/etc/ppp/.ip-up.keeper", F_OK) != -1) 
				rename("/etc/ppp/.ip-up.keeper", "/etc/ppp/ip-up");
	}
	else
	{
		snprintf(str, sizeof(str), "\n\tVPN ppp%d - (pid  %d) created ...\n", ppp_pts, pptp_pid); 
		log_add(w->log, tag_info, str);	
		gtk_widget_set_sensitive (GTK_WIDGET (w->start), FALSE);
		gtk_widget_set_sensitive (GTK_WIDGET (w->stop), TRUE);		
		Cache_or_Check(pptp_pid);		
	}		
}

/* JR-Mods - personal, not for distribution!
*/
static void pptp_connect(GtkWidget *widget, GpptpWidgets *w)
{
  char 		server[256], login[265], passwd[256], *argv[256], str[1024];  
  // JR Mods ..
  char 	KeyPath[256];
  FILE 		*InFile;
  // End JR Mods
  int			i;
  GError	*err;
  GIOChannel	*pptp_out, *pptp_err;
  gint		pptp_out_pipe, pptp_err_pipe;
  gint		pptp_out_tag, pptp_err_tag;
  GtkTextBuffer	*buf;
  //GtkTextIter	end_iter;

	SetRouting();

	buf = gtk_text_view_get_buffer (GTK_TEXT_VIEW (w->log));			
	gtk_text_buffer_set_text (buf, "", 0);
	log_add(w->log, tag_info, "Connecting to VPN server...\n");

	strncpy(server, gtk_entry_get_text (GTK_ENTRY (w->server)), sizeof(server));
	strncpy(login, gtk_entry_get_text (GTK_ENTRY (w->login)), sizeof(login));
	strncpy(passwd, gtk_entry_get_text (GTK_ENTRY (w->passwd)), sizeof(passwd));
	
	if(!strcmp(passwd, "mykey"))	// Get encrypted password from keyfile
	{		
		strcpy(KeyPath, cache_path);
		strcat(KeyPath, "/gpptpkey.0"); 
		
		if((InFile = fopen (KeyPath , "r")) != NOFILE)		
		{  			
			fgets(passwd, sizeof(passwd) - 1, InFile);	
			stretch(passwd, sizeof(passwd));	
			fclose(InFile);	
		}					
	}
	
	i = 0;
	argv[i++] = "/usr/sbin/pptp";
	argv[i++] = server;
	argv[i++] = "file";
	argv[i++] = "/etc/ppp/options.pptp";
	argv[i++] = "user";
	argv[i++] = login;
	if(passwd[0])
	{
	  argv[i++] = "password";
	  argv[i++] = passwd;
	}
	argv[i++] = NULL;
	
	fix_dns(DNS_COPY);

	if(! g_spawn_async_with_pipes ("/etc/ppp", argv, NULL,	G_SPAWN_DO_NOT_REAP_CHILD,
					NULL, NULL, &pptp_pid, NULL, &pptp_out_pipe, &pptp_err_pipe, &err))
	{
		snprintf(str, sizeof(str), "gpptp: Command failed: %s\n", err->message);
		log_add(w->log, tag_error, str);
		g_error_free (err);
		err = NULL;
		pptp_pid = NOPID;
	}
	else
	{
		g_child_watch_add(pptp_pid, on_pptp_exit, w);

		/* stdout */
		pptp_out = g_io_channel_unix_new (pptp_out_pipe);
		g_io_channel_set_encoding (pptp_out, NULL, NULL);
		pptp_out_tag = g_io_add_watch (pptp_out,
						(G_IO_IN | G_IO_HUP | G_IO_ERR),
						on_pptp_out,
						w);
		
		g_io_channel_unref (pptp_out);

		/* error */
		pptp_err = g_io_channel_unix_new (pptp_err_pipe);
		g_io_channel_set_encoding (pptp_err, NULL, NULL);
		pptp_err_tag = g_io_add_watch (pptp_err,
						(G_IO_IN | G_IO_HUP | G_IO_ERR),
						on_pptp_err,
						w);
		
		g_io_channel_unref (pptp_err);

		gtk_widget_set_sensitive (GTK_WIDGET (w->start), FALSE);
		gtk_widget_set_sensitive (GTK_WIDGET (w->stop), TRUE);
	}
}

static void pptp_disconnect(GtkWidget *widget, GpptpWidgets *w)
{
	log_add(w->log, tag_error, "\nDisconnecting from VPN server...");
	kill (pptp_pid, SIGTERM);
	remove(pid_cache); 				/*  delete our "pid cache" file.  - jafa */
	fix_dns(DNS_RESTORE);		/*  restore DNS to how it was before     */
	
	// Restore ip-up file
	if(access("/etc/ppp/.ip-up.keeper", F_OK) != -1) 
			rename("/etc/ppp/.ip-up.keeper", "/etc/ppp/ip-up");
	sleep(2);
	pptp_pid = NOPID; 
	log_add(w->log, tag_info, "   Done! \n");
	gtk_widget_set_sensitive (GTK_WIDGET (w->start), TRUE);
	gtk_widget_set_sensitive (GTK_WIDGET (w->stop),  FALSE);
}

static void destroy(GtkWidget *widget, gpointer *cb)
{
	gtk_main_quit();
	exit(0);
}

int main(int argc, char *argv[])
{
  GtkWidget	*window,
			*vbox1, vbox2,
				*hbox_server,
					*label_server,
					*entry_server,
				*hbox_login, 
					*label_login,
					*entry_login,
				*hbox_passwd,
					*label_passwd,
					*entry_passwd,
				*hbox_conn,
				//*ButtonTable,
					*button_start,
					*button_stop,
					*button_close,
				*scroll_log,
					*text_log;

  GpptpWidgets		w;

  GtkRequisition	req;
  GtkTextBuffer	*buffer;
  GtkTextIter		end_iter;
  
  	/* 	jafa mods 		*/  	
  	
  	strcpy(cache_path, getenv("HOME"));
	strcat(cache_path, "/.gpptp"); 				// Make a path to the user's home directory (for caching)  
  	if(access(cache_path , F_OK) != 0)  		// does "/root/.gpptp" exist?
  		mkdir (cache_path , 0755);            		 // if not, create it.
  		
	strcpy(dns_cache, cache_path);	
	strcat(dns_cache, "/gpptp-resolv.cache"); 
  	strcpy(pid_cache, cache_path);
	strcat(pid_cache, "/gpptp-pid.cache");  
	pptp_pid=Cache_or_Check(pptp_pid) ;  		/*  Check if we should have a zombie on the loose. 				*/
	
	// Restore ip-up file if backup exists 
	if(access("/etc/ppp/.ip-up.keeper", F_OK) != -1) 
			rename("/etc/ppp/.ip-up.keeper", "/etc/ppp/ip-up");
	/* 	end jafa mods 	*/

	gtk_init(&argc, &argv);

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), "Gpptp VPN v 2.0");
	gtk_container_set_border_width(GTK_CONTAINER(window), 5);

	vbox1 = gtk_vbox_new(FALSE, 2);
	gtk_container_add(GTK_CONTAINER(window), vbox1);

	/* VPN server field */
	hbox_server = gtk_hbox_new(FALSE,2);
	gtk_box_pack_start(GTK_BOX(vbox1), hbox_server, FALSE, FALSE, 0);

	label_server = gtk_label_new("VPN Server:"); 
	gtk_misc_set_alignment(GTK_MISC(label_server), 1, 0.5);
	gtk_box_pack_start(GTK_BOX(hbox_server), label_server, TRUE, TRUE, 0);

	entry_server = gtk_combo_box_entry_new_text();
	add_entries_from_file(entry_server, "/etc/ppp/gpptp/vpn_servers", "# \n");
	gtk_box_pack_start(GTK_BOX(hbox_server), entry_server, FALSE, FALSE, 0);

	/* Login field */
	hbox_login = gtk_hbox_new(FALSE, 2);
	gtk_box_pack_start(GTK_BOX(vbox1), hbox_login, FALSE, FALSE, 0);

	label_login = gtk_label_new("Login:");
	gtk_misc_set_alignment(GTK_MISC(label_login), 1, 0.5);
	gtk_box_pack_start(GTK_BOX(hbox_login), label_login, TRUE, TRUE, 0);

	entry_login = gtk_combo_box_entry_new_text();	
	add_entries_from_file(entry_login, "/etc/ppp/gpptp/vpn_userids", "# \n");
	gtk_box_pack_start(GTK_BOX(hbox_login), entry_login, FALSE, FALSE, 0);

	/* Password field */
	hbox_passwd = gtk_hbox_new(FALSE, 2);
	gtk_box_pack_start(GTK_BOX(vbox1), hbox_passwd, FALSE, FALSE, 0);

	label_passwd = gtk_label_new("Password:");
	gtk_misc_set_alignment(GTK_MISC(label_passwd), 1, 0.5);
	gtk_box_pack_start(GTK_BOX(hbox_passwd), label_passwd, TRUE, TRUE, 0);

	entry_passwd = gtk_entry_new();
	gtk_entry_set_visibility(GTK_ENTRY(entry_passwd), FALSE);
	gtk_box_pack_start(GTK_BOX(hbox_passwd), entry_passwd, FALSE, FALSE, 0);
	
	/*  Radio Buttons */
	GtkWidget *h_box = gtk_hbox_new( TRUE, 10 );	

	radio_button_1= gtk_radio_button_new_with_label( NULL, "Default Route" );
    radio_button_2 = gtk_radio_button_new_with_label_from_widget( GTK_RADIO_BUTTON( radio_button_1 ), "RFC 1918's" );
    radio_button_3 = gtk_radio_button_new_with_label_from_widget( GTK_RADIO_BUTTON( radio_button_1 ), "Custom" );
    
    gtk_box_pack_start_defaults( GTK_BOX( h_box ), radio_button_1 );
    gtk_box_pack_start_defaults( GTK_BOX( h_box ), radio_button_2 );
    gtk_box_pack_start_defaults( GTK_BOX( h_box ), radio_button_3 );    
    gtk_box_pack_start(GTK_BOX(vbox1), h_box, FALSE, FALSE, 0);
   /* End Radio Buttons */
   
	/* Start/Stop/Close buttons */
	hbox_conn = gtk_hbox_new(TRUE, 2);
	gtk_box_pack_start(GTK_BOX(vbox1), hbox_conn, FALSE, FALSE, 0);

	button_start = gtk_button_new_with_label("Connect");
	gtk_button_set_image (GTK_BUTTON (button_start),
				gtk_image_new_from_stock (GTK_STOCK_CONNECT,
				GTK_ICON_SIZE_BUTTON));
	if(pptp_pid == NOPID)	gtk_widget_set_sensitive (GTK_WIDGET (button_start), TRUE);		
	else  gtk_widget_set_sensitive (GTK_WIDGET (button_start), FALSE);		
	
	gtk_box_pack_start(GTK_BOX(hbox_conn), button_start, TRUE, TRUE, 0);
	
	button_stop = gtk_button_new_with_label("Disconnect");
	gtk_button_set_image (GTK_BUTTON (button_stop),
				gtk_image_new_from_stock (GTK_STOCK_DISCONNECT,
				GTK_ICON_SIZE_BUTTON));
	if(pptp_pid == NOPID)	gtk_widget_set_sensitive (GTK_WIDGET (button_stop), FALSE);
	else  gtk_widget_set_sensitive (GTK_WIDGET (button_stop), TRUE);
	gtk_box_pack_start(GTK_BOX(hbox_conn), button_stop, TRUE, TRUE, 0);
	
	button_close = gtk_button_new_with_label("Close  window");
	gtk_button_set_image (GTK_BUTTON (button_close),
				gtk_image_new_from_stock (GTK_STOCK_CLOSE,
				GTK_ICON_SIZE_BUTTON));	
	gtk_widget_set_sensitive (GTK_WIDGET (button_close), TRUE);
	gtk_box_pack_start(GTK_BOX(hbox_conn), button_close, TRUE, TRUE, 0);

	/* Log window */
	scroll_log = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll_log),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start(GTK_BOX(vbox1), scroll_log, TRUE, TRUE, 0);
									      
	text_log = gtk_text_view_new();
	gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (text_log), GTK_WRAP_WORD);
	gtk_text_view_set_editable (GTK_TEXT_VIEW (text_log), FALSE);
	gtk_container_add (GTK_CONTAINER (scroll_log), text_log);

	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_log));
	gtk_text_buffer_set_text (buffer, "", 0);
	gtk_text_buffer_get_end_iter (buffer, &end_iter);
	gtk_text_buffer_create_mark (buffer, "end", &end_iter, FALSE);

	tag_error = gtk_text_buffer_create_tag (buffer, NULL, 
						"foreground", "#C00000", NULL);
	tag_info  = gtk_text_buffer_create_tag (buffer, NULL,
						"foreground", "#00C000", NULL);	

	/* Adjust entry sizes */
	gtk_widget_size_request(entry_passwd, &req);
	req.width = 250;
	gtk_widget_set_size_request(entry_server, req.width, req.height);
	gtk_widget_set_size_request(entry_login, req.width, req.height);
	gtk_widget_set_size_request(entry_passwd, req.width, req.height);

	w.server = gtk_bin_get_child(GTK_BIN(entry_server));
	w.login  = gtk_bin_get_child(GTK_BIN(entry_login));
	w.passwd = entry_passwd;
	w.start  = button_start;
	w.stop   = button_stop;
	w.log    = text_log;

	gtk_signal_connect(GTK_OBJECT(button_start), "clicked", G_CALLBACK(pptp_connect), &w);
	gtk_signal_connect(GTK_OBJECT(button_stop), "clicked", G_CALLBACK(pptp_disconnect), &w);
	gtk_signal_connect(GTK_OBJECT(button_close), "clicked", G_CALLBACK(destroy), NULL);
	

	/* Show widgets */
	gtk_widget_show_all (vbox1);
	gtk_widget_show (window);
	
	if(pptp_pid != NOPID) log_add(w.log, tag_info, "\n\tVPN process already running ...\n");

	gtk_signal_connect(GTK_OBJECT(window), "destroy", G_CALLBACK(destroy), NULL);

	gtk_main();
	return 0;
}
