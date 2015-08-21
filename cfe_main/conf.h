#ifndef __CONF_H__
#define __CONF_H__

int defaultBrightnessEnable;
int defaultCpuSpeedEnable;

typedef struct
{
	int default_cpu_speed;
	int default_brightness;
	char button_combo[10];
	char button_menu[10];
	char button_screenshot[10];
	char button_cpu_plus[10];
	char button_cpu_minus[10];
	char button_brightness_plus[10];
	char button_brightness_minus[10];
	char button_music_menu[10];
	char capture_folder[64];
	char music_folder[256];
	u32 color_rectangle;
	u32 color_rectangle_shadow;
	u32 color_text;
	u32 color_text_shadow;	

} CONFIGFILE;

void read_config(const char *file, CONFIGFILE *config);

CONFIGFILE *config;

#endif 
