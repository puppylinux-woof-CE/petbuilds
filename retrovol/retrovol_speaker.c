#include <stdio.h>
#include <string.h>
#include <stdlib.h>


char *colour;
const char head1[45] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
const char head2[53] = "<svg height=\"100\" width=\"100\" version=\"1.0\">\n";
const char string1[83] = "\t<polygon points=\"10 60,  10 40,  20 40,  40 25, 40 75, 20 60, 10 60\" style=\"fill:"; //colour;
const char string2[28] = ";fill-opacity: 0.5; stroke:"; //colour;
const char string3[24] = ";stroke-width: 2;\"/>\n";
const char string4[70] = "\t<path d=\"M 55,27 c 24,23 0,48 0,48\" style=\"fill:none;stroke:"; //colour;
const char string5[22] = ";stroke-width:7\"/>\n";
const char string6[73] = "\t<path d=\"M 63,15 C 100,53 62,88 62,88\" style=\"fill:none;stroke:"; //colour;
const char string7[22] = ";stroke-width:7\"/>\n";
const char string8[72] = "\t<path d=\"M 72,5 C 120,54 73,97 73,97\" style=\"fill:none;stroke:"; //colour;
const char string9[22] = ";stroke-width:7\"/>\n";
const char string10[9] = "</svg>\n";
const char stringX[101] = "\t<line x1=\"5\" y1=\"75\" x2=\"60\" y2=\"20\" style=\"fill:none;stroke:red;stroke-width:7\"/>\n";


FILE *fp0;
FILE *fp1;
FILE *fp2;
FILE *fp3;
FILE *fp4;
	

static void usage();
static void parse_input(char *input);
static void print_output(char *argument);

void usage()
{
	printf("Takes 1 color argument\n");
	printf("\teg:\tred, blue, green (in English)\n");
	printf("\tor hex (escaped #):\t\\#ff0000,\\#0000ff,\\#00ff00\n");
	printf("If input is garbage then output is garbage!\n");
}

void parse_input(char *input)
{
	const char hex_input[] = "#ABCDEFabcdef0123456789";
	const char alpha_input[] = "abcdefghijklmnopqrstuvwxyz";
	if (input[0] == '#')
	{
		if ((strspn(input,hex_input) != 7)||(strlen(input) != 7))
		{
			usage();
			printf("Illegal entry\n");
			exit(1);
		}
	}
	else if (strspn(input,alpha_input) != strlen(input))
	{
		usage();
		printf("Illegal entry\n");
		exit(1);
	}
}

static void print_output(char *argument)
{
	fp0 = fopen("/usr/share/retrovol/images/audio-volume-high.svg", "w");
	fp1 = fopen("/usr/share/retrovol/images/audio-volume-medium.svg", "w");
	fp2 = fopen("/usr/share/retrovol/images/audio-volume-low.svg", "w");
	fp3 = fopen("/usr/share/retrovol/images/audio-volume-none.svg", "w");
	fp4 = fopen("/usr/share/retrovol/images/audio-volume-muted.svg", "w");		
	
	colour = argument;
	int j;
	for (j = 1; j <= 5; j++)
	{
		if (j == 1)
		{
			fprintf(fp0,"%s%s%s%s%s%s%s",head1,head2,string1,colour,string2,colour,string3); //common to all
			fprintf(fp0,"%s%s%s",string4,colour,string5);
			fprintf(fp0,"%s%s%s",string6,colour,string7);
			fprintf(fp0,"%s%s%s",string8,colour,string9);
			fprintf(fp0,"%s",string10); //common to all
			fclose(fp0);
		}
		else if (j == 2)
		{
			fprintf(fp1,"%s%s%s%s%s%s%s",head1,head2,string1,colour,string2,colour,string3); //common to all
			fprintf(fp1,"%s%s%s",string4,colour,string5);
			fprintf(fp1,"%s%s%s",string6,colour,string7);
			fprintf(fp1,"%s",string10); //common to all
			fclose(fp1);
		}
		else if (j == 3)
		{
			fprintf(fp2,"%s%s%s%s%s%s%s",head1,head2,string1,colour,string2,colour,string3); //common to all
			fprintf(fp2,"%s%s%s",string4,colour,string5);
			fprintf(fp2,"%s",string10); //common to all
			fclose(fp2);
		}	
		else if (j == 4)
		{
			fprintf(fp3,"%s%s%s%s%s%s%s",head1,head2,string1,colour,string2,colour,string3); //common to all
			fprintf(fp3,"%s",string10); //common to all
			fclose(fp3);
		}
		else if (j == 5)
		{
			fprintf(fp4,"%s%s%s%s%s%s%s",head1,head2,string1,colour,string2,colour,string3); //common to all
			fprintf(fp4,"%s",stringX);
			fprintf(fp4,"%s",string10); //common to all
			fclose(fp4);
		}
	}
}

int main(int argc, char **argv)
{
	if ((argc < 2)||(strcmp (argv[1], "-h") == 0)||(argc > 2))
	{
		usage();
		return 0;
	}
	if (strlen(argv[1]) > 7)
	{
		usage();
		printf("Only 7 chars allowed excluding \"\\\"\n");
		return 1;
	}
	parse_input(argv[1]);
	print_output(argv[1]);
	return 0;
}