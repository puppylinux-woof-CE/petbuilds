/*

overlay png images from the command line

location:
	/usr/sbin/pngoverlay-gtk2

compile:
	gcc -no-pie `pkg-config gtk+-2.0 --cflags --libs` -o /usr/sbin/pngoverlay-gtk2 pngoverlay-gtk2.c
	strip pngoverlay

*/

#include <stdio.h>
#include <unistd.h>
#include <dlfcn.h>
#include <gtk/gtk.h>
//#include <gdk-pixbuf/gdk-pixbuf.h>

//#define GDK_INTERP_BILINEAR 2
//#define GDK_INTERP_HYPER 3
//#define GTK_WINDOW_TOPLEVEL 0
#define OVERALL_ALPHA 255

// ===========================================================================

int main(int argc, char **argv) {

	char *FrontImage, *BackImage, *NewImage;
	int hei_img1, wid_img1, hei_img2, wid_img2, dest_x;
	double offset_x, scale_x, scale_y;
	GdkPixbuf *img_dst;
	GdkPixbuf *img_scr;

	if (argc < 4) {
		printf("Usage: front-png back-png output-png\n");
		return 1;
	}

	FrontImage = argv[1];
	BackImage = argv[2];
	NewImage = argv[3];

	if (access(FrontImage,0) != 0) return 1;
	if (access(BackImage,0) != 0) return 1;
	if (access(NewImage,0) == 0) unlink(NewImage);

	// -------------------------------------------------------

	scale_x = 1;
	scale_y = 1;
	dest_x = 0;
	offset_x = 0;

	gtk_init(0, 0);

	img_dst = gdk_pixbuf_new_from_file(FrontImage, 0);
	img_scr = gdk_pixbuf_new_from_file(BackImage, 0);

	if (!img_dst || !img_scr) {
		printf("Error loading one of the images\n");
		if (img_dst) g_object_unref(img_dst);
		if (img_scr) g_object_unref(img_scr);
		return 1;
	}

	hei_img1 = gdk_pixbuf_get_height(img_dst);
	wid_img1 = gdk_pixbuf_get_width(img_dst);
	hei_img2 = gdk_pixbuf_get_height(img_scr);
	wid_img2 = gdk_pixbuf_get_width(img_scr);
	
	if (hei_img1 != 48 || wid_img1 != 48) {
		GdkPixbuf *fixed_img = gdk_pixbuf_scale_simple (img_dst, 48, 48, GDK_INTERP_BILINEAR);
		if (fixed_img) {
			g_object_unref(img_dst);
			img_dst = fixed_img;
		}
	}

	if (hei_img2 != 48 || wid_img2 != 48) {
		GdkPixbuf *fixed_img = gdk_pixbuf_scale_simple (img_scr, 48, 48, GDK_INTERP_BILINEAR);
		if (fixed_img) {
			g_object_unref(img_scr);
			img_scr = fixed_img;
		}
	}

	hei_img1 = 48;
	wid_img1 = 48;
	hei_img2 = 48;
	wid_img2 = 48;

	gdk_pixbuf_composite(img_scr,
		img_dst,
		dest_x,
		hei_img1 - hei_img2,
		wid_img2,
		hei_img2,
		offset_x,
		hei_img1 - hei_img2,
		scale_x,
		scale_y,
		GDK_INTERP_HYPER,
		OVERALL_ALPHA
	);

	gdk_pixbuf_save(img_dst, NewImage, "png" ,NULL, NULL);

	g_object_unref(img_dst);
	g_object_unref(img_scr);

	if (access(NewImage,0) == 0) {
		printf("File %s created.\n", NewImage);
		return 0;
	}

	printf("OOPS.. %s was not created.\n", NewImage);
	return 1;

}

/* END */
