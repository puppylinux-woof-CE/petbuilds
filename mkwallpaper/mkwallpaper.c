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
 * (c)2015 Michael Amadio. Gold Coast QLD, Australia 01micko@gmail.com
 */
 
/** This program is designed to be used in Puppy Linux.
 * It is designed to be used in Woof-CE or in a running Puppy as root or
 * an ordinary user.
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <limits.h>

#include <cairo-svg.h>
#include <pango/pangocairo.h>

#define PROG "mkwallpaper"
#define THIS_VERSION "0.6"

void usage(){
	printf("%s-%s\n\n", PROG , THIS_VERSION);
	printf("\tGenerate SVG or PNG Puppy Linux wallpapers. SVG is default.\n\n");
	printf("Usage :\n");
	printf("\t%s [-l, -n, -f, -p, -s, -x, -y, -r, -w, -h]\n", PROG);
	printf("\t-n\t: image name\n");
	printf("\t-l\t: label for an image, up to 36 chars\n");
	printf("\t-f\t: a TTF font family\n");
	printf("\t-p\t: \"png\" or \"svg\" format\n");
	printf("\t-x\t: width of image in pixels\n");
	printf("\t-y\t: height of image in pixels\n");
	printf("\t-s\t: font size in pixels\n");
	printf("\t-k\t: \"yes\" : embossed effect on font\n");
	printf("\t-z\t: floating point RGB, quoted, "
					"space delimited values for colour\n"
					"\t(mandatory arg!) eg: -z \"0.1 0.2 0.3\"\n");
	printf("\t-o [offset] floating point value from 0.0 to 1.0 for the gradient"
								" offset\n");
	printf("\t-a [angle] integer value from 0 to 20 for the gradient angle\n");
	printf("\t-w\t: \"woof\" : FOR WOOF USE ONLY!!!\n");
	printf("\t-h\t: show this help and exit.\n");
	exit (EXIT_SUCCESS);
}

static const char *get_user_out_file(const char *ww){
	static char out_file[PATH_MAX];
	if (strncmp(ww, "woof", 4) == 0) {
		strcpy(out_file, "usr/share/backgrounds"); /*woof only */
	} else {
		char *my_home = getenv("HOME");
		if ((my_home != NULL) && (strcmp(my_home, "/root") == 0)) {
			strcpy(out_file, "/usr/share/backgrounds");
		} else {
			snprintf(out_file, sizeof(out_file), "%s/%s", my_home, ".local/share/backgrounds");
		}
	}
	mkdir(out_file, 0755);
	if (access(out_file, W_OK) == -1) {
		fprintf(stderr, "Failed to access directory %s\n", out_file);
		exit (EXIT_FAILURE);
	}
	return out_file;
}

static void paint_img (const char *label,
						const char *name,
						const char *font,
						const char *form,
						const int wdth,
						int hght,
						const char *fp_color,
						int f_size,
						const char *woofy,
						double offset,
						int angle,
						const char *kfont) {
	char destimg[PATH_MAX];

	if (!fp_color) exit (EXIT_FAILURE);

	int msg_len = strlen(label);
	if (msg_len > 36) {
		fprintf(stderr,"\"%s\" is too long!\n", label);
		exit (EXIT_FAILURE);
	}
	int v_size;
	if (hght > wdth) v_size = 1; /* in case of vertical wallpapers */
	if (f_size == 20) {
		if(v_size != 1)
			f_size = hght / 10;
		else 
			f_size = wdth / 10;
	}
	if ((angle < 0) || (angle > 20)) {
		fprintf(stderr, "%d is out of range. Must be 0 - 20 inclusive\n", angle);
		exit(EXIT_FAILURE);
	}
	if ((offset < 0) || (offset > 1)) {
		fprintf(stderr, "%f is out of range. Must be 0.00 - 1.00 inclusive\n", offset);
		exit(EXIT_FAILURE);
	}

	float r, g , b;
	char red[8];
	char green[8];
	char blue[8];
	int len = strlen(fp_color);
	if (len > 27 ) {
		fprintf(stderr,"ERROR: colour argument too long\n");
		exit (EXIT_FAILURE);
	}
	int result = sscanf(fp_color, "%s %s %s", red, green, blue);
	if (result < 3) {
		fprintf(stderr,"ERROR: less than 3 colour aguments!\n");
		exit (EXIT_FAILURE);
	}
	r = atof(red);
	g = atof(green);
	b = atof(blue);
	if ((r > 1.0) || (g > 1.0) || (b > 1.0) || 
		(r < 0.0) || (g < 0.0) || (b <0.0))  {
		fprintf(stderr, "Color values are out of range\n");
		exit (EXIT_FAILURE);
	}
	/* gradient */
	float r1, g1, b1, r2, g2, b2;
	r1 = r; g1 = g; b1 = b;
	if ((r > 0.701) || (g > 0.701) || (b > 0.701)) {
		r2 = r + 0.3;
		g2 = g + 0.3;
		b2 = b + 0.3;
	} else {
		r2 = r - 0.3;
		g2 = g - 0.3;
		b2 = b - 0.3;		
	}
	
	/* font color */
	float rf;
	if ((r > 0.5) || (g > 0.5) || (b > 0.5))
		rf = 0.1;
	else
		rf = 0.9;
	
	/* offset font for effect option */	
	float or, og, ob;
	int fc = 0;
	if (strncmp(kfont, "yes", 3) == 0) {
		or = (r1 + r2) / 2;
		og = (g1 + g2) /2 ;
		ob = (b1 + b2) / 2;
		fc = 1;
	}

	cairo_surface_t *cs;

	if (strcmp(form, "svg") == 0) {
		snprintf(destimg, sizeof(destimg), "%s/%s.svg", get_user_out_file(woofy), name);
		cs = cairo_svg_surface_create(destimg, wdth, hght);

	} else {
		cs = cairo_image_surface_create
							(CAIRO_FORMAT_ARGB32, wdth, hght);
		snprintf(destimg, sizeof(destimg), "%s/%s.png", get_user_out_file(woofy), name);
	}
	cairo_t *c;
	c = cairo_create(cs);

	cairo_rectangle(c, 0.0, 0.0, wdth, hght);
	cairo_pattern_t *linear = cairo_pattern_create_linear(angle * wdth / 20, 0, wdth / 2, hght);
	cairo_pattern_add_color_stop_rgb(linear, 0.1, r1, g1, b1);
	cairo_pattern_add_color_stop_rgb(linear, offset, r2, g2, b2);
	cairo_pattern_add_color_stop_rgb(linear, 0.9, r1, g1, b1);
	cairo_set_source(c, linear);
	cairo_fill(c);
	
	PangoLayout *layout;
	PangoFontDescription *font_description;
	
	font_description = pango_font_description_new ();
	pango_font_description_set_family (font_description, font);
	pango_font_description_set_weight (font_description, PANGO_WEIGHT_BOLD);
	pango_font_description_set_style (font_description, PANGO_STYLE_OBLIQUE);
	pango_font_description_set_absolute_size (font_description, f_size * PANGO_SCALE);
	
	layout = pango_cairo_create_layout (c);
	pango_layout_set_font_description (layout, font_description);
	pango_layout_set_width (layout, wdth / 2 * PANGO_SCALE);
	pango_layout_set_wrap (layout, PANGO_WRAP_WORD);
	pango_layout_set_text (layout, label, -1);
	
	cairo_move_to(c, wdth / 2 , 4 * hght / 7);
	cairo_set_source_rgba(c, rf, rf, rf, 0.55);
	pango_cairo_show_layout (c, layout);
	
	if (fc == 1) {
		cairo_move_to(c, (wdth / 2) - 1.5 , (4 * hght / 7) - 1.2);
		cairo_set_source_rgba(c, or, og, ob, 0.65);
		pango_cairo_show_layout (c, layout);
	}
	
	cairo_status_t res = cairo_surface_status(cs);
	const char *ret;
	ret = cairo_status_to_string (res);
	if (res != CAIRO_STATUS_SUCCESS) {
		g_object_unref (layout);
		pango_font_description_free (font_description);
		cairo_surface_destroy(cs);
		cairo_destroy(c);
		fprintf(stderr, "Cairo : %s\n", ret);
		exit (EXIT_FAILURE);
	}
	
	g_object_unref (layout);
	pango_font_description_free (font_description);
	if (strcmp(form, "png") == 0) {
		cairo_surface_write_to_png (cs, destimg);
	}
	cairo_surface_destroy(cs);
	cairo_destroy(c);
	printf("image saved to %s\n", destimg);
}

int main(int argc, char **argv) {
	if (argc < 2) {
		usage();
		return 0;
	}
	if (strcmp(argv[1], "-h") == 0) {
		usage();
		return 0;
	}
	
	char *lvalue = "hello wallpaper!"; /* the cli string that appears in image */
	char *nvalue = "foo"; /* image name */
	char *fvalue = "Sans"; /* font */
	char *pvalue = "svg";
	char *wvalue = "notwoof"; /* used for woof */
	char *zvalue = NULL; /* fp colour */
	char *kvalue = "no";
	double ovalue = 0.65;
	int avalue = 10;
	int width = 200; int height = 60;
	int font_size = 20;
	int c;
	while ((c = getopt (argc, argv, "l:n:f:p:x:y:w:z:o:a:k:s:")) != -1) {
		switch (c)
		{
			case 'l':
				lvalue = optarg;
				break;
			case 'n':
				nvalue = optarg;
				break;
			case 'f':
				fvalue = optarg;
				break;
			case 'p':
				pvalue = optarg;
				break;
			case 'x':
				width = atoi(optarg);
				break;
			case 'y':
				height = atoi(optarg);
				break;
			case 'w':
				wvalue = optarg;
				break;
			case 'z':
				zvalue = optarg;
				break;
			case 'o':
				ovalue = atof(optarg);
				break;
			case 'a':
				avalue = atoi(optarg);
				break;
			case 'k':
				kvalue = optarg;
				break;
			case 's':
				font_size = atoi(optarg);
				break;
			default:
				usage();
		}
	}
	if (zvalue == NULL) {
		fprintf(stderr, "You must have 3 floating point value arguments to \"-z\"\n");
		usage();
		return 1;
	}
	paint_img(lvalue, nvalue, fvalue, pvalue,
						width, height, zvalue, font_size, wvalue, ovalue, avalue, kvalue);
	return 0;
}

