////////////////////////////
/* cfe translateButtons.c */
////////////////////////////

#include <pspkernel.h>
#include <pspctrl.h>
#include <string.h>
#include "translateButtons.h"
#include "conf.h"
#include "mem.h"


int vshTranslateButtonsByName(char* button)
{
	if(strcmp(button, "L")==0) return 0x000100;
	else if(strcmp(button, "R")==0) return 0x000200;
	else if(strcmp(button, "SQUARE")==0) return 0x008000;
	else if(strcmp(button, "CIRCLE")==0) return 0x002000;
	else if(strcmp(button, "TRIANGLE")==0) return 0x001000;
	else if(strcmp(button, "CROSS")==0) return 0x004000;	
	else if(strcmp(button, "UP")==0) return 0x000010;	
	else if(strcmp(button, "DOWN")==0) return 0x000040;	
	else if(strcmp(button, "RIGHT")==0) return 0x000020;	
	else if(strcmp(button, "LEFT")==0) return 0x000080;	
	else if(strcmp(button, "HOME")==0) return 0x010000;	
	else if(strcmp(button, "NOTE")==0) return 0x800000;	
	else if(strcmp(button, "SELECT")==0) return 0x000001;
	else if(strcmp(button, "START")==0) return 0x000008;
	else if(strcmp(button, "VOL_UP")==0) return 0x100000;
	else if(strcmp(button, "VOL_DOWN")==0) return 0x200000;
	else if(strcmp(button, "SCREEN")==0) return 0x400000;

	return 1;
}

int translateButtons()
{

	button = (BUTTONCONFIG *) Kmalloc(1, sizeof(BUTTONCONFIG), &button_memid);

	button->combo = vshTranslateButtonsByName(config->button_combo);
	button->menu = vshTranslateButtonsByName(config->button_menu);
	button->screenshot = vshTranslateButtonsByName(config->button_screenshot);
	button->cpuPlus = vshTranslateButtonsByName(config->button_cpu_plus);
	button->cpuMinus = vshTranslateButtonsByName(config->button_cpu_minus);
	button->brightnessPlus = vshTranslateButtonsByName(config->button_brightness_plus);
	button->brightnessMinus = vshTranslateButtonsByName(config->button_brightness_minus);
	button->music = vshTranslateButtonsByName(config->button_music_menu);

	return 1;
}
	
