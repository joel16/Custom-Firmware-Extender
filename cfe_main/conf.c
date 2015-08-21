#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pspiofilemgr.h>
#include <pspctrl.h>

#include "conf.h"
#include "mem.h"

unsigned short RGB(unsigned char r,unsigned char g,unsigned char b)
{
	return ((((b>>3) & 0x1F)<<10)+(((g>>3) & 0x1F)<<5)+(((r>>3) & 0x1F)<<0)+0x8000);
}
//#define isvalidvarchar(c) (isalnum(c) || c == '_' || c == '=') 

#define isvalidvarchar(c) (c == 'a' || c == 'b' || c == 'c' || c == 'd' || c == 'e' || c == 'f' \
 || c == 'g' || c == 'h' || c == 'i' || c == 'j' || c == 'k' || c == 'l' || c == 'm' || c == 'n' || c == 'o' \
 || c == 'p' || c == 'q' || c == 'r' || c == 's' || c == 't' || c == 'u' || c == 'v' || c == 'w' || c == 'x' \
 || c == 'y' || c == 'z' || c == 'A' || c == 'B' || c == 'C' || c == 'D' || c == 'E' || c == 'F' \
 || c == 'G' || c == 'H' || c == 'I' || c == 'J' || c == 'K' || c == 'L' || c == 'M' || c == 'N' || c == 'O' \
 || c == 'P' || c == 'Q' || c == 'R' || c == 'S' || c == 'T' || c == 'U' || c == 'V' || c == 'W' || c == 'X' \
 || c == 'Y' || c == 'Z' || c == '0' || c == '1' || c == '2' || c == '3' || c == '4' || c == '5' || c == '6' \
 || c == '7' || c == '8' || c == '9' || c == '_' || c == '=')


static int read_line(SceUID fd, char *line, int max)
{
	int i, read;
	char ch;

	i = 0;

	if (max == 0)
		return 1;

	do
	{
		read = sceIoRead(fd, &ch, 1);
		
		if (read && ch != '\n' && ch != '\r')
			line[i++] = ch;		

	} while (ch != '\n' && read == 1 && i < max);

	line[i] = 0;

	return !read;
}

static int get_tokens(char tokens[][128], int maxtokens, char *line)
{
	int iline = 0;
	int itoken = 0;
	int jtoken = 0;
	int intoken = 0;
	int instring = 0;
	char ch;

	while (itoken < maxtokens)
	{
		ch = line[iline++];

		if (ch == 0)
		{
			if (instring)
				return 0; // Error: not terminated string

			if (intoken)			
				tokens[itoken++][jtoken] = 0;

			break;
		}

		if (!instring && (ch == '#' || ch == ';'))
		{
			if (intoken)			
				tokens[itoken++][jtoken] = 0;

			break;
		}

		//if (isspace(ch) || ch == '=')
		if(ch == ' ' || ch == '=')
		{
			if (intoken)
			{
				if (!instring)
				{
					intoken = 0;
					tokens[itoken++][jtoken] = 0;
					jtoken = 0;
				}

				else
				{
					tokens[itoken][jtoken++] = ch;
				}
			}
		}

		else if (ch == '"')
		{
			if (intoken)
			{
				if ((!instring && jtoken != 0) || 
					(instring && isvalidvarchar(line[iline])))
				{
					// Error: Mixing string token with something else 
					return 0; 
				}

				tokens[itoken][jtoken++] = ch;				
				instring = !instring;
			}
			
			else
			{
				intoken = 1;
				instring = 1;
				tokens[itoken][jtoken++] = ch;
			}
		}

		else if (isvalidvarchar(ch))
		{
			if (!intoken)
				intoken = 1;

			tokens[itoken][jtoken++] = ch;
		}

		else
		{
			if (instring)
				tokens[itoken][jtoken++] = ch;
		}
	}

	return itoken;
}

static char *get_string(char *out, int max, char *in)
{
	char *p;
	int len;
	
	memset(out, 0, max);

	if (in[0] != '"')
		return NULL;

	p = strchr(in+1, '"');

	if (!p)
		return NULL;

	if (p-(in+1) > max)
		len = max;
	else
		len = p-(in+1);

	strncpy(out, in+1, len);	

	return out;
}

static int get_integer(char *str)
{
	return strtol(str, NULL, 0);
}
/*
static int get_boolean(char *str)
{
	if (strcmp(str, "false") == 0)
		return 0;

	if (strcmp(str, "true") == 0)
		return 1;	

	if (strcmp(str, "off") == 0)
		return 0;

	if (strcmp(str, "on") == 0)
		return 1;

	return get_integer(str);
}
*/

void read_config(const char *file, CONFIGFILE *config)
{
	SceUID fd = sceIoOpen(file, PSP_O_RDONLY, 0777);

	/* Default: all values to zero */
	Kmemset(config, 0, sizeof(CONFIGFILE));

	if (fd > 0)
	{
		int eof = 0, ntokens;
		char line[128];
		char tokens[2][128];
		
		while (!eof)
		{
			eof = read_line(fd, line, 127);
			ntokens = get_tokens(tokens, 2, line);	

			if (ntokens == 2)
			{
				if (strcmp(tokens[0], "default_cpu_speed") == 0)
				{
					config->default_cpu_speed = get_integer(tokens[1]);
					defaultCpuSpeedEnable = 1;
					printf("config->default_cpu_speed = %i\n", config->default_cpu_speed);
				}
				else if (strcmp(tokens[0], "default_brightness") == 0)
				{
					config->default_brightness = get_integer(tokens[1]);
					defaultBrightnessEnable=1;
					printf("config->default_brightness = %i\n", config->default_brightness);
				}
				else if (strcmp(tokens[0], "button_combo") == 0)
				{
					get_string(config->button_combo, 10, tokens[1]);
					printf("config->button_combo = %s\n", config->button_combo);
				}
				else if (strcmp(tokens[0], "button_menu") == 0)
				{
					get_string(config->button_menu, 10, tokens[1]);
					printf("config->button_menu = %s\n", config->button_menu);
				}
				else if (strcmp(tokens[0], "button_screenshot") == 0)
				{
					get_string(config->button_screenshot, 10, tokens[1]);
					printf("config->button_screenshot = %s\n", config->button_screenshot);
				}
				else if (strcmp(tokens[0], "button_cpu_plus") == 0)
				{
					get_string(config->button_cpu_plus, 10, tokens[1]);
					printf("config->button_cpu_plus = %s\n", config->button_cpu_plus);
				}
				else if (strcmp(tokens[0], "button_cpu_minus") == 0)
				{
					get_string(config->button_cpu_minus, 10, tokens[1]);
					printf("config->button_cpu_minus = %s\n", config->button_cpu_minus);
				}
				else if (strcmp(tokens[0], "button_brightness_plus") == 0)
				{
					get_string(config->button_brightness_plus, 10, tokens[1]);
					printf("config->button_brightness_plus = %s\n", config->button_brightness_plus);
				}
				else if (strcmp(tokens[0], "button_brightness_minus") == 0)
				{
					get_string(config->button_brightness_minus, 10, tokens[1]);
					printf("config->button_brightness_minus = %s\n", config->button_brightness_minus);
				}
				else if (strcmp(tokens[0], "button_music_menu") == 0)
				{
					get_string(config->button_music_menu, 10, tokens[1]);
					printf("config->button_music_menu = %s\n", config->button_music_menu);
				}
				else if (strcmp(tokens[0], "capture_folder") == 0)
				{
					get_string(config->capture_folder, 64, tokens[1]);
					printf("config->capture_folder = %s\n", config->capture_folder);
				}
				else if (strcmp(tokens[0], "music_folder") == 0)
				{
					get_string(config->music_folder, 256, tokens[1]);
					printf("config->music_folder = %s\n", config->music_folder);
				}
				else if (strcmp(tokens[0], "color_rectangle") == 0)
				{
					config->color_rectangle = get_integer(tokens[1]);
				}
				else if (strcmp(tokens[0], "color_rectangle_shadow") == 0)
				{
					config->color_rectangle_shadow = get_integer(tokens[1]);
				}
				else if (strcmp(tokens[0], "color_text") == 0)
				{
					config->color_text = get_integer(tokens[1]);
				}
				else if (strcmp(tokens[0], "color_text_shadow") == 0)
				{
					config->color_text_shadow = get_integer(tokens[1]);
				}
			}
		}

		sceIoClose(fd);
	}
}

void write_config()
{
	char cfgLine[256];

#ifdef GAME
	sceIoRemove("ms0:/seplugins/cfe/game.cfg");
	SceUID fd = sceIoOpen( "ms0:/seplugins/cfe/game.cfg", PSP_O_RDWR | PSP_O_CREAT | PSP_O_APPEND, 0777 );
#else
	sceIoRemove("ms0:/seplugins/cfe/vsh.cfg");
	SceUID fd = sceIoOpen( "ms0:/seplugins/cfe/vsh.cfg", PSP_O_RDWR | PSP_O_CREAT | PSP_O_APPEND, 0777 );
#endif

	memset(cfgLine, 0, 256);
	sprintf(cfgLine, "default_cpu_speed = %i;\n", config->default_cpu_speed);
	sceIoWrite(fd, cfgLine, strlen(cfgLine));

	memset(cfgLine, 0, 256);
	sprintf(cfgLine, "default_brightness = %i;\n", config->default_brightness);
	sceIoWrite(fd, cfgLine, strlen(cfgLine));

	memset(cfgLine, 0, 256);
	sprintf(cfgLine, "button_combo = \"%s\";\n", config->button_combo);
	sceIoWrite(fd, cfgLine, strlen(cfgLine));

	memset(cfgLine, 0, 256);
	sprintf(cfgLine, "button_menu = \"%s\";\n", config->button_menu);
	sceIoWrite(fd, cfgLine, strlen(cfgLine));

	memset(cfgLine, 0, 256);
	sprintf(cfgLine, "button_screenshot = \"%s\";\n", config->button_screenshot);
	sceIoWrite(fd, cfgLine, strlen(cfgLine));

	memset(cfgLine, 0, 256);
	sprintf(cfgLine, "button_cpu_plus = \"%s\";\n", config->button_cpu_plus);
	sceIoWrite(fd, cfgLine, strlen(cfgLine));

	memset(cfgLine, 0, 256);
	sprintf(cfgLine, "button_cpu_minus = \"%s\";\n", config->button_cpu_minus);
	sceIoWrite(fd, cfgLine, strlen(cfgLine));

	memset(cfgLine, 0, 256);
	sprintf(cfgLine, "button_brightness_plus = \"%s\";\n", config->button_brightness_plus);
	sceIoWrite(fd, cfgLine, strlen(cfgLine));

	memset(cfgLine, 0, 256);
	sprintf(cfgLine, "button_brightness_minus = \"%s\";\n", config->button_brightness_minus);
	sceIoWrite(fd, cfgLine, strlen(cfgLine));

	memset(cfgLine, 0, 256);
	sprintf(cfgLine, "button_music_menu = \"%s\";\n", config->button_music_menu);
	sceIoWrite(fd, cfgLine, strlen(cfgLine));

	memset(cfgLine, 0, 256);
	sprintf(cfgLine, "capture_folder = \"%s\";\n", config->capture_folder);
	sceIoWrite(fd, cfgLine, strlen(cfgLine));

	memset(cfgLine, 0, 256);
	sprintf(cfgLine, "music_folder = \"%s\";\n", config->music_folder);
	sceIoWrite(fd, cfgLine, strlen(cfgLine));

	memset(cfgLine, 0, 256);
	sprintf(cfgLine, "# Colors in ABGR hex format\n");
	sceIoWrite(fd, cfgLine, strlen(cfgLine));

	memset(cfgLine, 0, 256);
	sprintf(cfgLine, "color_rectangle = 0x%08x;\n", config->color_rectangle);
	sceIoWrite(fd, cfgLine, strlen(cfgLine));

	memset(cfgLine, 0, 256);
	sprintf(cfgLine, "color_rectangle_shadow = 0x%08x;\n", config->color_rectangle_shadow);
	sceIoWrite(fd, cfgLine, strlen(cfgLine));

	memset(cfgLine, 0, 256);
	sprintf(cfgLine, "color_text = 0x%08x;\n", config->color_text);
	sceIoWrite(fd, cfgLine, strlen(cfgLine));

	memset(cfgLine, 0, 256);
	sprintf(cfgLine, "color_text_shadow = 0x%08x;\n", config->color_text_shadow);
	sceIoWrite(fd, cfgLine, strlen(cfgLine));

	sceIoClose(fd);
}


