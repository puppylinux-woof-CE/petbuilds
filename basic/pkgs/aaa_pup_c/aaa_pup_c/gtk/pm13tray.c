#include <stdlib.h>
#include <gtk/gtk.h>

GtkStatusIcon *tray_icon;
GdkPixbuf *pixbuf;

/* XPM */
static char *floppy_xpm[] = {
"26 26 62 1",
" 	c None",
"A	c #2B2B2B",
"B	c #424242",
"C	c #454545",
"D	c #434B43",
"E	c #404040",
"F	c #202020",
"G	c #787878",
"H	c #6C6C6C",
"I	c #389F40",
"J	c #33A33C",
"K	c #767676",
"L	c #3D3D3D",
"M	c #434343",
"N	c #676767",
"O	c #383838",
"P	c #4A4A4A",
"Q	c #309738",
"R	c #2C9C35",
"S	c #555555",
"T	c #6B6B6B",
"U	c #686868",
"V	c #414141",
"W	c #4E4E4E",
"X	c #AAB9AB",
"Y	c #B2C0B3",
"Z	c #E2E2E2",
"a	c #EEEEEE",
"b	c #BFBFBF",
"c	c #BDBDBD",
"d	c #939393",
"e	c #989898",
"f	c #565656",
"g	c #595959",
"h	c #585858",
"i	c #A3A3A3",
"j	c #ABABAB",
"k	c #858585",
"l	c #7B7B7B",
"m	c #9F9F9F",
"n	c #9D9D9D",
"o	c #575757",
"p	c #5A5A5A",
"q	c #BBBBBB",
"r	c #969696",
"s	c #515151",
"t	c #535353",
"u	c #636363",
"v	c #909090",
"w	c #525252",
"x	c #656565",
"y	c #646464",
"z	c #393939",
"0	c #626262",
"1	c #5E5E5E",
"2	c #ADADAD",
"3	c #848484",
"4	c #000000",
"5	c #3A3A3A",
"6	c #6D6D6D",
"7	c #4D4D4D",
"8	c #2D2D2D",
"                          ",
"  BCCDDDDDDDDDDDDDDDDCCE  ",
" BGHHIJJJJJJJJJJJJJJIHHKL ",
" MNOPQRRRRRRRRRRRRRRQSSTM ",
" MUVWXYYYYYYYYYYYYYYXSSTM ",
" MTSSZaaaaaaaaaaaaaaZSSTM ",
" MTSSZbccccccccccccbZSSTM ",
" MTSSZaaaaaaaaaaaaaaZSSTM ",
" MTSSZaaaaaaaaaaaaaaZSSTM ",
" MTSSZbccccccccccccbZSSTM ",
" MTSSZaaaaaaaaaaaaaaZSSTM ",
" MTSSZaaaaaaaaaaaaaaZSSTM ",
" MTSSZbccccccccccccbZSSTM ",
" MTSSZaaaaaaaaaaaaaaZSSTM ",
" MTSSdeeeeeeeeeeeeeedSSTM ",
" MTSSSSfggggggggggSSSSSTM ",
" MTSSShijklmjjjjjjnoSSSTM ",
" MTSSSpqrstuqqqqqqqpSSSTM ",
" MTSSSpqvwSxqqqqqqqpSSSTM ",
" MTSSSpqvwSxqqqqqqqpSSSTM ",
" MTSSSpqvwSxqqqqqqqpSSSTM ",
" MHSSSpqvwSyqqqqqqqpSSSTM ",
" z01SSpq2Nx3qqqqqqqpSSSTM ",
"  5u6HuqqqqqqqqqqqquHHHGV ",
"   zCC7pppppppppppp7CCCV  ",
"                          "
};

void tray_icon_on_click(GtkStatusIcon *status_icon, gpointer user_data) {
		system("eventmanager pm13 & ");
}


/* -create icon and 'click' properties */
static GtkStatusIcon *create_tray_icon() {

	tray_icon = gtk_status_icon_new();

	g_signal_connect(G_OBJECT(tray_icon), "activate", G_CALLBACK(tray_icon_on_click), NULL);
	pixbuf = gdk_pixbuf_new_from_xpm_data ((const char **)floppy_xpm);
	//gtk_status_icon_set_from_file(tray_icon,"/root/raspi-icons/floppy.xpm" );
	gtk_status_icon_set_from_pixbuf (tray_icon, pixbuf);
	gtk_status_icon_set_tooltip(tray_icon, "Pupmode 13 - press for settings");
	
	gtk_status_icon_set_visible(tray_icon, TRUE);

	return tray_icon;
}

int main(int argc, char **argv) {

	gtk_init(&argc, &argv);
		
	tray_icon = create_tray_icon();
                        
	gtk_main();

	return 0;
}

