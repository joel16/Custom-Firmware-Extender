#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pspiofilemgr.h>
#include <pspctrl.h>

#include "conf.h"
#define isvalidvarchar(c) (isalnum(c) || c == '_' || c == '=')

int isspace(int c);
int isalnum(int c);

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

		if (isspace(ch) || ch == '=')
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

static int get_integer(char *str)
{
	return strtol(str, NULL, 0);
}

static int get_boolean(char *str)
{
	if (strcasecmp(str, "false") == 0)
		return 0;

	if (strcasecmp(str, "true") == 0)
		return 1;	

	if (strcasecmp(str, "off") == 0)
		return 0;

	if (strcasecmp(str, "on") == 0)
		return 1;

	return get_integer(str);
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

void read_config(const char *file, CONFIGFILE *config)
{
	SceUID fd = sceIoOpen(file, PSP_O_RDONLY, 0777);

	/* Default: all values to zero */
	memset(config, 0, sizeof(CONFIGFILE));

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
				if (strcasecmp(tokens[0], "authEnable") == 0)
				{
					config->authEnable = get_boolean(tokens[1]);
				}
				else if (strcasecmp(tokens[0], "password") == 0)
				{
					get_string(config->password, 64, tokens[1]);
				}
			}
		}

		sceIoClose(fd);
	}
}
