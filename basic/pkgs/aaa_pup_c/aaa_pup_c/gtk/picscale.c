/*

picscale.c (based on vovchik's picscale.bac)

location:
	/usr/sbin/picscale

compile:
	gcc -no-pie `pkg-config gtk+-2.0 --cflags --libs` -o picscale picscale.c
	strip picscale

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <gtk/gtk.h>
//#include <gdk-pixbuf/gdk-pixbuf.h>

// allowed output formats
char *my_output_formats[] = {
	"png", "bmp", "ico", "tif", "tiff", "jpg", "jpeg", "jpe", NULL,
};

void usage(char *my_msg) {
	if (my_msg) {
		printf("\n%s\n\n", my_msg);
	}
	printf("picscale image resizer/converter - 0.1\n");
	printf("Input formats supported: pnm, pbm, pgm, ppm, tga, xpm, tiff, pcx, gif,\n");
	printf("xbm, wmf, icns, bmp, png, jpg and ico.\n");
	printf("Output formats supported: png, jpg, bmp, tiff and ico.\n");
	printf("Input parameters: -i oldfilename -o newfilename height width quality/compression/depth\n");
	printf("Example: picscale -i old.png -o new.png 128 128 9\n");
	printf("Quality/compression/depth settings:\n");
	printf("	bmp (N/A):               0 - 100\n");
	printf("	jpeg (quality):          0 - 100\n");
	printf("	png (compression):       0 - 9\n");
	printf("	tiff (compression type): 1 - 8\n");
	printf("	ico (depth):             16, 24 or 32\n");
}


int main(int argc, char **argv) {

	unsigned long t_height, t_width, w, h, r_x, r_y, new_w, new_h;

	//
	// initialize variables
	//
	char *org_file = NULL;
	char *new_file =  NULL;
	char *new_width = NULL;
	char *new_height = NULL;
	char *quality = NULL;
	char *new_file_type = NULL;
	w = 0;
	h = 0;
	r_x = 0;
	r_y = 0;
	new_w = 0;
	new_h = 0;

	//
	// parse arguments
	//
	if (argc < 2) {
		usage(NULL);
		return 1;
	}
	if (argc < 8) {
		usage("\nERROR: Check input arguments!");
		return 1;
	}

	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-i") == 0)
			org_file = argv[i+1];
		else if (strcmp(argv[i], "-o") == 0)
			new_file = argv[i+1];
	}
	new_width = argv[argc - 3];
	new_height = argv[argc - 2];
	quality = argv[argc - 1];
	//printf("new_width: %s\n", new_width); //debug
	//printf("new_height: %s\n", new_height); //debug
	//printf("quality: %s\n", quality); //debug

	if (access(org_file,0) != 0) {
		usage("ERROR: Input image does not exist!");
		return 1;
	}
	GdkPixbuf *im2;
	gtk_init(0, 0);
	im2 = gdk_pixbuf_new_from_file(org_file, NULL);
	if (!im2) {
		printf("ERROR: could not open %s\n", org_file);
		return 1;
	}

	char *new_extension = strrchr(new_file, '.');
	if (new_extension) {
		if (strlen(new_extension) < 2) {
			//printf("WARNING: no actual extension\n");
			new_extension = NULL;
		} else
			new_extension++;
	}
	//printf("new_extension: %s\n", new_extension); //debug
	unsigned ext_is_valid = 0;
	if (new_extension) {
		for (int i = 0; my_output_formats[i]; i++) {
			if (strcasecmp(my_output_formats[i], new_extension) == 0) {
				//printf("found out: %s\n", my_output_formats[i]); //debug
				new_file_type = my_output_formats[i];
				ext_is_valid = 1;
				break;
			}
		}
	}
	if (!ext_is_valid) {
		printf("\nERROR: Cannot handle the '.%s' output format..\n\n", new_extension);
		usage(NULL);
		return 1;
	}

	//
	// process data
	//
	t_height = strtoul(new_height, NULL, 0);
	t_width = strtoul(new_width, NULL, 0);
	w = gdk_pixbuf_get_width(im2);
	h = gdk_pixbuf_get_height(im2);
	r_x = t_width / w;
	r_y = t_height / h;
	if (t_height == t_width) {
		new_w = t_width;
		new_h = t_height;
	} else {
		if (r_x <= r_y) {
			new_w = t_width;
			new_h = h * r_x + 1;
		} else {
			new_w = w * r_y + 1;
			new_h = t_height;
		}
	}

	GdkPixbuf *im;
	im = gdk_pixbuf_scale_simple(im2, new_w, new_h, GDK_INTERP_HYPER); // GDK_INTERP_BILINEAR
	if (strcmp(new_file_type, "png") == 0) {
		gdk_pixbuf_save(im, new_file, new_file_type, NULL, "compression", quality, NULL);
	}
	else if (strncmp(new_file_type, "jp", 2) == 0) {
		gdk_pixbuf_save(im, new_file, "jpeg", NULL, "quality", quality, NULL);
	}
	else if (strncmp(new_file_type, "tif", 3) == 0) {
		gdk_pixbuf_save(im, new_file, "tiff", NULL, "compression", quality, NULL);
	}
	else if (strcmp(new_file_type, "ico") == 0) {
		gdk_pixbuf_save(im, new_file, new_file_type, NULL, "depth", quality, NULL);
	}
	else {
		gdk_pixbuf_save(im, new_file, new_file_type, NULL, NULL, NULL, NULL);
	}

	g_object_unref(im);
	g_object_unref(im2);

	//
	// report results
	//
	if (access(new_file,0) == 0) {
		printf("File %s created.\n", new_file);
		printf("Original image: %s\n", org_file);
		printf("New dimensions (w x h): %s x %s\n", new_width, new_height);
		printf("Quality of new image: %s\n", quality);
	} else {
		printf("OOPS.. something terrible happened and %s was not created.\n", new_file);
	}
	gtk_exit(0);

}

