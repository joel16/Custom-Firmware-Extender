#ifndef __CONF_H__
#define __CONF_H__

typedef struct
{
	int authEnable;
	char password[64];

} CONFIGFILE;

void read_config(const char *file, CONFIGFILE *config);

#endif 
