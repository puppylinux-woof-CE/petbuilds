
/************************************************************************
 *  
 *  setvol.c  -- version 1.1
 *  
 *  Command line soundcard level setting utility.
 *  Written by Derek Quinn Wyatt (derek@scar.utoronto.ca)
 *  
 *  Compile with:     gcc -o setvol setvol.c -Wall
 *  Install with:     cp setvol /usr/local/bin
 *  
 *  Don't forget to set the permissions on /dev/mixer to 666.
 *  
 *  
 *  There's a bit of repeated code in this, but it's a small program
 *  and it didn't seem like such a bad thing :)
 *  
 *
 ************************************************************************/

#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/soundcard.h>

int fd;

void open_mixer(void);
void set_value(int channel, int level1, int level2);
void run_query(int channel);
int is_stereo(int channel);
int is_supported(int channel);
int conv_int(char *arg);
void usage(void);

int main(int argc, char **argv)
{
  int channel;
  int level1, level2;

  open_mixer();
  if (argc == 2)
    {
      channel = conv_int(argv[1]);
      if (!is_supported(channel))
	{
	  fprintf(stderr, "Device %d is not supported by this system\n", channel);
	  close(fd);
	  exit(3);
	}
      run_query(channel);
    }
  else
    {
      if (argc < 3 || argc > 4)
	usage();
      channel = conv_int(argv[1]);
      if (!is_supported(channel))
	{
	  fprintf(stderr, "Device %d is not supported by this system\n", channel);
	  close(fd);
	  exit(3);
	}
      level1 = conv_int(argv[2]);
      if (level1 < 0) level1 = 0;
      if (level1 > 100) level1 = 100;
      if (argc == 4)
	{
	  level2 = conv_int(argv[3]);
	  if (level2 < 0) level2 = 0;
	  if (level2 > 100) level2 = 100;
	}
      else
	level2 = level1;
      set_value(channel, level1, level2);
    }
  close(fd);

    return 0;
}

void open_mixer(void)
{
  if ((fd = open("/dev/mixer", 0)) == -1)
    {
      fprintf(stderr, "\nEither /dev/mixer isn't there, or I can't access it.\n");
      fprintf(stderr, "If the latter is the case, chmod 666 /dev/mixer\n\n");
      exit(2);
    }
}

void set_value(int channel, int level1, int level2)
{
  int level;

  if (is_stereo(channel))
    {
      level2 = level2 << 8;
      level = level1 | level2;
    }
  else
    level = level1;
  ioctl(fd, MIXER_WRITE(channel), &level);
}

void run_query(int channel)
{
  int level, level1, level2;

  ioctl(fd, MIXER_READ(channel), &level);
  if (is_stereo(channel))
    {
      level1 = level&255; 
      level2 = level >> 8;
      printf("%02d %02d\n", level1, level2);
    }
  else
    printf("%02d\n", level&255);
}

int is_stereo(int channel)
{
  int devmask;

  ioctl(fd, MIXER_READ(SOUND_MIXER_STEREODEVS), &devmask);
  if (!(devmask & (1 << channel)))
    return 0;
  else
    return 1;
}

int is_supported(int channel)
{
  int devmask;

  ioctl(fd, MIXER_READ(SOUND_MIXER_DEVMASK), &devmask);
  if (!(devmask & (1 << channel)))
    return 0;
  else
    return 1;
}

int conv_int(char *arg)
{
  int converted;
  char *tmp;

  tmp = arg;
  while (*tmp != '\0' && isdigit(*tmp)) tmp++;
  if (*tmp != '\0')
    {
      fprintf(stderr, "\"%s\" is not a number.  I can't do anything with that.\n", arg);
      close(fd);
      exit(1);
    }
  converted = strtol(arg, (char **)NULL, 10);

  return converted;
}

void usage(void)
{
  fprintf(stderr, "\nSetVol 1.1 -- Derek Quinn Wyatt (derek@scar.utoronto.ca)\n");
  fprintf(stderr, "\nusage: setvol <device #> [level1 [level2]]\n");
  fprintf(stderr, "\nexamples:\n\n");
  fprintf(stderr, "setvol 0           - reports status of device 0\n");
  fprintf(stderr, "setvol 0 50        - sets device 0 to half of its maximum level\n");
  fprintf(stderr, "setvol 0 20 60     - sets the left channel of device 0 to 20/100\n");
  fprintf(stderr, "                     and the right channel of device 0 to 60/100\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Level has to be between 0 and 100\n"
	  "The following devices are supported\n\n"
	  "SOUND_MIXER_VOLUME      0\n"
	  "SOUND_MIXER_BASS        1\n"
	  "SOUND_MIXER_TREBLE      2\n"
	  "SOUND_MIXER_SYNTH       3\n"
	  "SOUND_MIXER_PCM         4\n"
	  "SOUND_MIXER_SPEAKER     5\n"
	  "SOUND_MIXER_LINE        6\n"
	  "SOUND_MIXER_MIC         7\n"
	  "SOUND_MIXER_CD          8\n"
	  "SOUND_MIXER_IMIX        9\n"
	  "SOUND_MIXER_ALTPCM      10\n"
	  "SOUND_MIXER_RECLEV      11\n"
	  "SOUND_MIXER_IGAIN       12\n"
	  "SOUND_MIXER_OGAIN       13\n\n");
  close(fd);
  exit(1);
}
