#include <pspkernel.h>
#include <string.h>
#include <stdio.h>
#include <psppower.h>
#include <pspdisplay_kernel.h>
#include <pspusb.h>

#include "guicommon.h"
#include "graphic.h"
#include "ctrl.h"
#include "conf.h"
#include "common.h"
#ifndef LIGHT
#include "filer.h"
#endif
#include "utils.h"

extern void write_config();
extern int translateButtons();
int button_n;

const char CFG_BUTTON[17][16] =
{
	"L",
	"R",
	"SQUARE",
	"CIRCLE",
	"TRIANGLE",
	"CROSS",
	"UP",	
	"DOWN",
	"RIGHT",	
	"LEFT",	
	"HOME",	
	"NOTE",	
	"SELECT",
	"START",
	"VOL_UP",
	"VOL_DOWN",
	"SCREEN"
};

#define MENU_CFG_CPU 0
#define MENU_CFG_BRIGHTNESS 1
#define MENU_CFG_BT_COMBO 2
#define MENU_CFG_BT_MENU 3
#define MENU_CFG_BT_SCREENSHOT 4
#define MENU_CFG_BT_CPU_PLUS 5
#define MENU_CFG_BT_CPU_MINUS 6
#define MENU_CFG_BT_BRIGHTNESS_PLUS 7
#define MENU_CFG_BT_BRIGHTNESS_MINUS 8
#define MENU_CFG_BT_MUSIC_MENU 9
#define MENU_CFG_MUSIC_FOLDER 10
#define MENU_CFG_CAPTURE_FOLDER 11
#define MENU_CFG_SAVE 12

#define MENU_CFG_CPU_X 29
#define MENU_CFG_BRIGHTNESS_X 44
#define MENU_CFG_BT_COMBO_X 59
#define MENU_CFG_BT_MENU_X 74
#define MENU_CFG_BT_SCREENSHOT_X 89
#define MENU_CFG_BT_CPU_PLUS_X 104
#define MENU_CFG_BT_CPU_MINUS_X 119
#define MENU_CFG_BT_BRIGHTNESS_PLUS_X 134
#define MENU_CFG_BT_BRIGHTNESS_MINUS_X 149
#define MENU_CFG_BT_MUSIC_MENU_X 164
#define MENU_CFG_MUSIC_FOLDER_X 179
#define MENU_CFG_CAPTURE_FOLDER_X 194
#define MENU_CFG_SAVE_X 219


int menu_cfg_items_x[13] =
{
	MENU_CFG_CPU_X,
	MENU_CFG_BRIGHTNESS_X,
	MENU_CFG_BT_COMBO_X,
	MENU_CFG_BT_MENU_X,
	MENU_CFG_BT_SCREENSHOT_X,
	MENU_CFG_BT_CPU_PLUS_X,
	MENU_CFG_BT_CPU_MINUS_X,
	MENU_CFG_BT_BRIGHTNESS_PLUS_X,
	MENU_CFG_BT_BRIGHTNESS_MINUS_X,
	MENU_CFG_BT_MUSIC_MENU_X,
	MENU_CFG_MUSIC_FOLDER_X,
	MENU_CFG_CAPTURE_FOLDER_X,
	MENU_CFG_SAVE_X
};

void draw_cfg_menu()
{
	if(menu_item == MENU_CFG_CPU)
	{
		setP(195, 30, 195 + 4*8 , 43);
		drawRect(0, true, 0);
		setP0(200, 40);
		gprintf("%i", config->default_cpu_speed);
	}
	else if(menu_item == MENU_CFG_BRIGHTNESS)
	{
		setP(195, 45, 195 + 3*8 , 58);
		drawRect(0, true, 0);
		setP0(200, 55);
		gprintf("%i", config->default_brightness);
	}
	else if(menu_item == MENU_CFG_BT_COMBO)
	{
		setP(195, 60, 195 + 17*8 , 73);
		drawRect(0, true, 0);
		setP0(200, 70);
		gprintf("%s", config->button_combo);
	}
	else if(menu_item == MENU_CFG_BT_MENU)
	{
		setP(195, 75, 195 + 17*8 , 88);
		drawRect(0, true, 0);
		setP0(200, 85);
		gprintf("%s + %s", config->button_combo, config->button_menu);
	}
	else if(menu_item == MENU_CFG_BT_SCREENSHOT)
	{
		setP(195, 90, 195 + 20*8 , 103);
		drawRect(0, true, 0);
		setP0(200, 100);
		gprintf("%s + %s", config->button_combo, config->button_screenshot);
	}
	else if(menu_item == MENU_CFG_BT_CPU_PLUS)
	{
		setP(195, 105, 195 + 20*8 , 118);
		drawRect(0, true, 0);
		setP0(200, 115);
		gprintf("%s + %s", config->button_combo, config->button_cpu_plus);
	}
	else if(menu_item == MENU_CFG_BT_CPU_MINUS)
	{
		setP(195, 120, 195 + 20*8 , 133);
		drawRect(0, true, 0);
		setP0(200, 130);
		gprintf("%s + %s", config->button_combo, config->button_cpu_minus);
	}
	else if(menu_item == MENU_CFG_BT_BRIGHTNESS_PLUS)
	{
		setP(195, 135, 195 + 20*8 , 148);
		drawRect(0, true, 0);
		setP0(200, 145);
		gprintf("%s + %s", config->button_combo, config->button_brightness_plus);
	}
	else if(menu_item == MENU_CFG_BT_BRIGHTNESS_MINUS)
	{
		setP(195, 150, 195 + 20*8 , 163);
		drawRect(0, true, 0);
		setP0(200, 160);
		gprintf("%s + %s", config->button_combo, config->button_brightness_minus);
	}
	else if(menu_item == MENU_CFG_BT_MUSIC_MENU)
	{
		setP(195, 165, 195 + 20*8 , 178);
		drawRect(0, true, 0);
		setP0(200, 175);
		gprintf("%s + %s", config->button_combo, config->button_music_menu);
	}
	else if(menu_item == MENU_CFG_MUSIC_FOLDER)
	{
		setP(195, 180, 195 + 17*8 , 193);
		drawRect(0, true, 0);
		setP0(200, 190);
#ifdef LIGHT
		gprintf("Not Available");
#else
		gprintf("%s", config->music_folder);
#endif
	}
	else if(menu_item == MENU_CFG_CAPTURE_FOLDER)
	{
		setP(195, 195, 195 + 17*8 , 208);
		drawRect(0, true, 0);
		setP0(200, 205);
#ifdef LIGHT
		gprintf("Not Available");
#else
		gprintf("%s", config->capture_folder);
#endif
	}
}

void update_all_button()
{
		setP(195, 75, 195 + 17*8 , 88);
		drawRect(0, true, 0);
		setP0(200, 85);
		gprintf("%s + %s", config->button_combo, config->button_menu);

		setP(195, 90, 195 + 20*8 , 103);
		drawRect(0, true, 0);
		setP0(200, 100);
		gprintf("%s + %s", config->button_combo, config->button_screenshot);

		setP(195, 105, 195 + 20*8 , 118);
		drawRect(0, true, 0);
		setP0(200, 115);
		gprintf("%s + %s", config->button_combo, config->button_cpu_plus);

		setP(195, 120, 195 + 20*8 , 133);
		drawRect(0, true, 0);
		setP0(200, 130);
		gprintf("%s + %s", config->button_combo, config->button_cpu_minus);

		setP(195, 135, 195 + 20*8 , 148);
		drawRect(0, true, 0);
		setP0(200, 145);
		gprintf("%s + %s", config->button_combo, config->button_brightness_plus);

		setP(195, 150, 195 + 20*8 , 163);
		drawRect(0, true, 0);
		setP0(200, 160);
		gprintf("%s + %s", config->button_combo, config->button_brightness_minus);

		setP(195, 165, 195 + 20*8 , 178);
		drawRect(0, true, 0);
		setP0(200, 175);
		gprintf("%s + %s", config->button_combo, config->button_music_menu);
}

void draw_cfg_menu_items()
{
	if(menu_item == MENU_CFG_CPU)
	{
		setP0(25, 40);
		drawString("Default Cpu Speed :", FSHADOW);	
	}	
	
	if(menu_item == MENU_CFG_BRIGHTNESS)
	{	
		setP0(25, 55);
		drawString("Default Brightness :", FSHADOW);
	}

	if(menu_item == MENU_CFG_BT_COMBO)	
	{
		setP0(25, 70);
		drawString("Combo Button :", FSHADOW);
	}

	if(menu_item == MENU_CFG_BT_MENU)	
	{
		setP0(25, 85);
		drawString("Menu Button :", FSHADOW);	
	}

	if(menu_item == MENU_CFG_BT_SCREENSHOT)	
	{
		setP0(25, 100);
		drawString("Screenshot Button :", FSHADOW);
	}

	if(menu_item == MENU_CFG_BT_CPU_PLUS)	
	{
		setP0(25, 115);
		drawString("Cpu Speed+ Button :", FSHADOW);
	}

	if(menu_item == MENU_CFG_BT_CPU_MINUS)
	{
		setP0(25, 130);
		drawString("Cpu Speed- Button :", FSHADOW);
	}

	if(menu_item == MENU_CFG_BT_BRIGHTNESS_PLUS)
	{
		setP0(25, 145);
		drawString("Brightness+ Button :", FSHADOW);
	}

	if(menu_item == MENU_CFG_BT_BRIGHTNESS_MINUS)
	{
		setP0(25, 160);
		drawString("Brightness- Button :", FSHADOW);
	}

	if(menu_item == MENU_CFG_BT_MUSIC_MENU)
	{
		setP0(25, 175);
		drawString("Music Menu Button :", FSHADOW);
	}

	if(menu_item == MENU_CFG_SAVE)
	{
		setP0(25, 230);
		drawString("Save Configuration", FSHADOW);
	}
	if(menu_item == MENU_CFG_MUSIC_FOLDER)
	{
		setP0(25, 190);
		drawString("Music Folder :", FSHADOW);
	}
	if(menu_item == MENU_CFG_CAPTURE_FOLDER)
	{
		setP0(25, 205);
		drawString("Capture Folder :", FSHADOW);
	}
}

void cfg_menu_update_all_down()
{
	if(menu_item==0)
	{
		setP(20, menu_cfg_items_x[MENU_CFG_SAVE], 25 + 20*8 , menu_cfg_items_x[MENU_CFG_SAVE]+15);
		drawRect(0, true, 0);

		//menu_cfg_update_items();

		menu_item=12;
		draw_cfg_menu_items();

		menu_item=0;
		setP(20, menu_cfg_items_x[menu_item], 25 + 20*8 , menu_cfg_items_x[menu_item]+15);
		drawRect(0, false, 0);
		
	}
	else
	{
		setP(20, menu_cfg_items_x[menu_item-1], 25 + 20*8 , menu_cfg_items_x[menu_item-1]+15);
		drawRect(0, true, 0);

		//menu_cfg_update_items();
				
		menu_item-=1;
		draw_cfg_menu_items();

		menu_item+=1;
		setP(20, menu_cfg_items_x[menu_item], 25 + 20*8 , menu_cfg_items_x[menu_item]+15);
		drawRect(0, false, 0);
	}
	draw_cfg_menu_items();
	draw_cfg_menu();
}

void cfg_menu_update_all_up()
{
	if(menu_item==12)
	{
		setP(20, menu_cfg_items_x[MENU_CFG_CPU], 25 + 20*8 , menu_cfg_items_x[MENU_CFG_CPU]+15);
		drawRect(0, true, 0);

		//menu_cfg_update_items();

		menu_item=0;
		draw_cfg_menu_items();

		menu_item=12;
		setP(20, menu_cfg_items_x[menu_item], 25 + 20*8 , menu_cfg_items_x[menu_item]+15);
		drawRect(0, false, 0);
		
	}
	else
	{
		setP(20, menu_cfg_items_x[menu_item+1], 25 + 20*8 , menu_cfg_items_x[menu_item+1]+15);
		drawRect(0, true, 0);

		//menu_cfg_update_items();
				
		menu_item+=1;
		draw_cfg_menu_items();

		menu_item-=1;
		setP(20, menu_cfg_items_x[menu_item], 25 + 20*8 , menu_cfg_items_x[menu_item]+15);
		drawRect(0, false, 0);
	}
	draw_cfg_menu_items();
	draw_cfg_menu();
}

void draw_cfg_menu_init()
{
	setP(15, 25, 480, 249);
	clearRect(0);
	setP(16, 25, 459, 240);
	drawRect(15, true, 0x0A0000FF);

	setP(20, MENU_CFG_CPU_X, 25 + 20*8 , MENU_CFG_CPU_X+15);
	drawRect(0, false, 0);

	draw_cfg_menu_items();
	
	setP0(25, 55);
	drawString("Default Brightness :", FSHADOW);
	setP0(25, 70);
	drawString("Combo Button :", FSHADOW);
	setP0(25, 85);
	drawString("Menu Button :", FSHADOW);		
	setP0(25, 100);
	drawString("Screenshot Button :", FSHADOW);
	setP0(25, 115);
	drawString("Cpu Speed+ Button :", FSHADOW);
	setP0(25, 130);
	drawString("Cpu Speed- Button :", FSHADOW);
	setP0(25, 145);
	drawString("Brightness+ Button :", FSHADOW);
	setP0(25, 160);
	drawString("Brightness- Button :", FSHADOW);
	setP0(25, 175);
	drawString("Music Menu Button :", FSHADOW);

	setP0(25, 190);
	drawString("Music Folder :", FSHADOW);
	setP0(25, 205);
	drawString("Capture Folder :", FSHADOW);

	setP0(25, 230);
	drawString("Save Configuration", FSHADOW);

	setP0(200, 40);
	gprintf("%i", config->default_cpu_speed);
	setP0(200, 55);
	gprintf("%i", config->default_brightness);
	setP0(200, 70);
	gprintf("%s", config->button_combo);
	setP0(200, 85);
	gprintf("%s + %s", config->button_combo, config->button_menu);
	setP0(200, 100);
	gprintf("%s + %s", config->button_combo, config->button_screenshot);
	setP0(200, 115);
	gprintf("%s + %s", config->button_combo, config->button_cpu_plus);
	setP0(200, 130);
	gprintf("%s + %s", config->button_combo, config->button_cpu_minus);
	setP0(200, 145);
	gprintf("%s + %s", config->button_combo, config->button_brightness_plus);
	setP0(200, 160);
	gprintf("%s + %s", config->button_combo, config->button_brightness_minus);
	setP0(200, 175);
	gprintf("%s + %s", config->button_combo, config->button_music_menu);
	setP0(200, 190);
#ifdef LIGHT
	gprintf("Not Available");
#else
	gprintf("%s", config->music_folder);
#endif
	setP0(200, 205);
#ifdef LIGHT
	gprintf("Not Available");
#else
	gprintf("%s", config->capture_folder);
#endif
}

int menu_cfg()
{
	all_done = 0;
	menu_cfg_done = 0;
	screenshot_done = 0;
	menu_item = MENU_CFG_CPU;

	draw_menu_section();
	draw_cfg_menu_init();

	while(!menu_cfg_done)
	{
		sceDisplaySetBrightness(brightness_level, 0);

		u32 key = ctrlWaitMask(PSP_CTRL_LTRIGGER |PSP_CTRL_RTRIGGER | PSP_CTRL_UP | PSP_CTRL_DOWN | PSP_CTRL_LEFT | PSP_CTRL_RIGHT | PSP_CTRL_CROSS | PSP_CTRL_CIRCLE);
		switch(key)
		{
			case PSP_CTRL_RTRIGGER:
#ifdef VSH
				if(menu_section < 4) menu_section++;
#elif GAME
#ifndef LIGHT
				if(menu_section < 3) menu_section++;
#else
				if(menu_section < 2) menu_section++;
#endif
#endif
					else menu_section = 0;

					menu_cfg_done = 1;
			break;

			case PSP_CTRL_LTRIGGER:

				if(menu_section > 0) menu_section--;
#ifdef VSH
					else menu_section = 4;
#elif GAME
#ifndef LIGHT
					else menu_section = 3;
#else
					else menu_section = 2;
#endif
#endif
					menu_cfg_done = 1;
			break;

			case  PSP_CTRL_DOWN:
				if(menu_item < 12) menu_item++;
					else menu_item = 0;

					cfg_menu_update_all_down();
			break;

			case  PSP_CTRL_UP:
				if(menu_item > 0) menu_item--;
					else menu_item = 12;

					cfg_menu_update_all_up();
			break;


			case  PSP_CTRL_RIGHT:
				if(menu_item == MENU_CFG_CPU)
				{
					if(config->default_cpu_speed < 333) config->default_cpu_speed++;
				}
				if(menu_item == MENU_CFG_BRIGHTNESS)
				{
					if(config->default_brightness < 99) config->default_brightness++;
				}
				if(menu_item == MENU_CFG_BT_COMBO)
				{
					if(button_n < 16) button_n++;
					strcpy(config->button_combo, CFG_BUTTON[button_n]);
					update_all_button();
				}
				if(menu_item == MENU_CFG_BT_MENU)
				{
					if(button_n < 16) button_n++;
					strcpy(config->button_menu, CFG_BUTTON[button_n]);
				}
				if(menu_item == MENU_CFG_BT_SCREENSHOT)
				{
					if(button_n < 16) button_n++;
					strcpy(config->button_screenshot, CFG_BUTTON[button_n]);
				}
				if(menu_item == MENU_CFG_BT_CPU_PLUS)
				{
					if(button_n < 16) button_n++;
					strcpy(config->button_cpu_plus, CFG_BUTTON[button_n]);
				}
				if(menu_item == MENU_CFG_BT_CPU_MINUS)
				{
					if(button_n < 16) button_n++;
					strcpy(config->button_cpu_minus, CFG_BUTTON[button_n]);
				}
				if(menu_item == MENU_CFG_BT_BRIGHTNESS_PLUS)
				{
					if(button_n < 16) button_n++;
					strcpy(config->button_brightness_plus, CFG_BUTTON[button_n]);
				}
				if(menu_item == MENU_CFG_BT_BRIGHTNESS_MINUS)
				{
					if(button_n < 16) button_n++;
					strcpy(config->button_brightness_minus, CFG_BUTTON[button_n]);
				}
				if(menu_item == MENU_CFG_BT_MUSIC_MENU)
				{
					if(button_n < 16) button_n++;
					strcpy(config->button_music_menu, CFG_BUTTON[button_n]);
				}
				
				draw_cfg_menu();
			break;

			case  PSP_CTRL_LEFT:
				if(menu_item == MENU_CFG_CPU)
				{
					if(config->default_cpu_speed > 20) config->default_cpu_speed--;
				}
				if(menu_item == MENU_CFG_BRIGHTNESS)
				{
					if(config->default_brightness > 0) config->default_brightness--;
				}
				if(menu_item == MENU_CFG_BT_COMBO)
				{
					if(button_n > 0) button_n--;
					strcpy(config->button_combo, CFG_BUTTON[button_n]);
					update_all_button();
				}
				if(menu_item == MENU_CFG_BT_MENU)
				{
					if(button_n > 0) button_n--;
					strcpy(config->button_menu, CFG_BUTTON[button_n]);
				}
				if(menu_item == MENU_CFG_BT_SCREENSHOT)
				{
					if(button_n > 0) button_n--;
					strcpy(config->button_screenshot, CFG_BUTTON[button_n]);
				}
				if(menu_item == MENU_CFG_BT_CPU_PLUS)
				{
					if(button_n > 0) button_n--;
					strcpy(config->button_cpu_plus, CFG_BUTTON[button_n]);
				}
				if(menu_item == MENU_CFG_BT_CPU_MINUS)
				{
					if(button_n > 0) button_n--;
					strcpy(config->button_cpu_minus, CFG_BUTTON[button_n]);
				}
				if(menu_item == MENU_CFG_BT_BRIGHTNESS_PLUS)
				{
					if(button_n > 0) button_n--;
					strcpy(config->button_brightness_plus, CFG_BUTTON[button_n]);
				}
				if(menu_item == MENU_CFG_BT_BRIGHTNESS_MINUS)
				{
					if(button_n > 0) button_n--;
					strcpy(config->button_brightness_minus, CFG_BUTTON[button_n]);
				}
				if(menu_item == MENU_CFG_BT_MUSIC_MENU)
				{
					if(button_n > 0) button_n--;
					strcpy(config->button_music_menu, CFG_BUTTON[button_n]);
				}
			
				draw_cfg_menu();
			break;

			case  PSP_CTRL_CROSS:
				if(menu_item == MENU_CFG_MUSIC_FOLDER)
				{
					configure_music = 1;
#ifndef LIGHT
					filer_on();
					menu_item = MENU_CFG_CPU;
					draw_menu_section();
					draw_cfg_menu_init();
					configure_music = 0;
#endif
				}
				if(menu_item == MENU_CFG_CAPTURE_FOLDER)
				{
					configure_capture = 1;
#ifndef LIGHT
					filer_on();
					menu_item = MENU_CFG_CPU;
					draw_menu_section();
					draw_cfg_menu_init();
					configure_capture = 0;
#endif
				}
				if(menu_item == MENU_CFG_SAVE)
				{
					write_config();
					translateButtons();
				}
			break;

			case  PSP_CTRL_CIRCLE:
				all_done = 1;
				menu_cfg_done = 1;
			break;
		}
	}
	if(!all_done)
	{
		change_menu_section();
	}

	return 0;	

}

