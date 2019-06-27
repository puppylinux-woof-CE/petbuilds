/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * (c)2019 Michael Amadio. Gold Coast QLD, Australia 01micko@gmail.com
 */
 
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

#include <cairo.h>

#define PROG "pngoverlay-cairo"
#define THIS_VERSION "0.1"
#define D 48

void usage(){
	printf("%s-%s\n\n", PROG , THIS_VERSION);
	printf("\tGenerate a PNG file overlayed from a source and overlay\nA 48x48 PNG output icon is generated\n\n");
	printf("Usage :\n");
	printf("%s [source path] [overlay path] [output path]\n", PROG);
	printf("\tvalid PNG paths are required\n");
	exit (EXIT_SUCCESS);
}

struct { 
	/* allows a png icon */
	cairo_surface_t *overlay_image;
	/* allows a background - png only */
	cairo_surface_t *background_image;
}glob;

static const char *get_user_out_file(char *destination){
	static char out_file[PATH_MAX];
	if (destination != NULL) {
		snprintf(out_file, sizeof(out_file), "%s", destination);
	} else {
		fprintf(stderr, "Failed to recognise directory\n");
		exit (EXIT_FAILURE);
	}
	return out_file;
}

static void paint_img (char *sicon,
						char *oicon,
						char *dest) {
						
	/* input icon */
	if (sicon != NULL) {
		if (access(sicon, R_OK) == -1) {
			fprintf(stderr, "Failed to access image %s\n", sicon);
			exit (EXIT_FAILURE);
		}
	}	
	/* overlay icon */
	if (oicon != NULL) {
		if (access(oicon, R_OK) == -1) {
			fprintf(stderr, "Failed to access image %s\n", oicon);
			exit (EXIT_FAILURE);
		}
	}
	
	char destimg[PATH_MAX];
	cairo_surface_t *cd;
	cd = cairo_image_surface_create
							(CAIRO_FORMAT_ARGB32, D, D);
	snprintf(destimg, sizeof(destimg), "%s", get_user_out_file(dest));
	cairo_t *d;
	d = cairo_create(cd);
	
	/* background icon and position */
	cairo_rectangle(d, 0.0, 0.0, D, D);
	glob.background_image = cairo_image_surface_create_from_png(sicon);
	cairo_set_source_surface(d, glob.background_image, 0, 0);
	cairo_paint(d);
	
	/* overlay icon and position */
	glob.overlay_image = cairo_image_surface_create_from_png(oicon);
	cairo_set_source_surface(d, glob.overlay_image, 0, 0);
	cairo_paint(d);
	
	/* error check */
	cairo_status_t res = cairo_surface_status(cd);
	const char *ret;
	ret = cairo_status_to_string (res);
	if (res != CAIRO_STATUS_SUCCESS) {
		cairo_surface_destroy(cd);
		cairo_destroy(d);
		fprintf(stderr, "Cairo : %s\n", ret);
		exit (EXIT_FAILURE);
	}
	cairo_surface_write_to_png (cd, destimg);
	
	/* cleanup */
	cairo_surface_destroy(glob.overlay_image);
	cairo_surface_destroy(glob.background_image);
	cairo_surface_destroy(cd);
	cairo_destroy(d);
	fprintf(stdout, "image saved to %s\n", destimg);
}

int main(int argc, char **argv) {
	if (argc < 4) {
		fprintf(stderr, "3 arguments required\n\n");
		usage();
		return 1;
	}
	char svalue[PATH_MAX]; /* source png */
	char nvalue[PATH_MAX]; /* overlay png */
	char ovalue[PATH_MAX]; /* output path png */
	strcpy(svalue, argv[1]);
	strcpy(nvalue, argv[2]);
	strcpy(ovalue, argv[3]);
	
	paint_img(svalue, nvalue, ovalue);
	return 0;
}
