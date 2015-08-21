#include <pspkernel.h>
#include <string.h>
#include <stdio.h>
#include <pspdisplay_kernel.h>

#include "guicommon.h"
#include "graphic.h"
#include "ctrl.h"
#include "conf.h"
#include "common.h"
#ifndef LIGHT
#include "filer.h"
#endif

void draw_main_menu();
int module_reboot_before();

void menu_music_draw()
{
	setP(195, 30, 195 + (strlen(config->music_folder)+1)*8 , 42);
	drawRect(0, true, 0);
	setP0(200, 40);
	drawString(config->music_folder , FSHADOW);
}

void menu_music_update()
{
	setP(20, 30, 20 + 20*8 , 44);
	drawRect(0, true, 0);
	if(menu_item == MENU_MUSIC_DIR)
	{
		setP(20, 30, 20 + 20*8 , 44);
		drawRect(0, false, 0);	
	}	
	setP0(25, 40);
	drawString("Music Directory :", FSHADOW);

	setP(20, 50, 20 + 20*8 , 64);
	drawRect(0, true, 0);	
	if(menu_item == MENU_MUSIC_START)
	{
		setP(20, 50, 20 + 20*8 , 64);
		drawRect(0, false, 0);	
	}
	setP0(25, 60);
	drawString("Start Music", FSHADOW);
}

void menu_music()
{
	all_done = 0;
	menu_music_done = 0;
	menu_item = 0;

	draw_menu_section();

	setP(15, 25, 480, 249);
	clearRect(0);
	setP(16, 25, 459, 240);
	drawRect(15, true, 0x0A0000FF);

	setP(20, 30, 20 + 20*8 , 44);
	drawRect(0, false, 0);	
	setP0(25, 40);
	drawString("Music Directory :", FSHADOW);

	setP0(25, 60);
	drawString("Start Music", FSHADOW);

	menu_music_draw();

	while(!menu_music_done)
	{
		printf("%s\n", config->music_folder);
		sceDisplaySetBrightness(brightness_level, 0);

		u32 key = ctrlWaitMask(PSP_CTRL_RTRIGGER | PSP_CTRL_LTRIGGER | PSP_CTRL_UP | PSP_CTRL_DOWN | PSP_CTRL_CROSS | PSP_CTRL_CIRCLE);
		switch(key)
		{
			case PSP_CTRL_RTRIGGER:
#ifndef GAME
				if(menu_section < 4) menu_section++;
#else
				if(menu_section < 3) menu_section++;
#endif
					else menu_section = 0;

				menu_music_done = 1;
			break;

			case PSP_CTRL_LTRIGGER:

				if(menu_section > 0) menu_section--;
#ifndef GAME
					else menu_section = 4;
#else
					else menu_section = 3;
#endif
					menu_music_done = 1;
			break;

			case  PSP_CTRL_DOWN:
				if(menu_item < 1) menu_item++;
					else menu_item=0;
				menu_music_update();
				menu_music_draw();
				break;

			case  PSP_CTRL_UP:
				if(menu_item > 0) menu_item--;
					else menu_item=1;
				menu_music_update();
				menu_music_draw();
				break;

			case PSP_CTRL_CROSS:
				if(menu_item == MENU_MUSIC_DIR)
				{
				/*	if(sceKernelDevkitVersion() == 0x01050001)
					{
						gui_print("Filer not available under 1.50fw");
					}
					else
					{*/
						configure_music = 1;
						filer_on();
						configure_music = 0;

						setP(15, 25, 480, 249);
						clearRect(0);
						setP(16, 25, 459, 240);
						drawRect(15, true, 0x0A0000FF);
						setP(20, 30, 20 + 20*8 , 44);
						drawRect(0, false, 0);	
						setP0(25, 40);
						drawString("Music Directory :", FSHADOW);
						setP0(25, 60);
						drawString("Start Music", FSHADOW);
						menu_music_update();
						menu_music_draw();
				//	}
				}

				if(menu_item == MENU_MUSIC_START)
				{
					if(!music_prx_active) 
					{
						music_prx_load = 1;//music_start(music_dir);
						music_prx_active = 1;
						all_done = 1;
						menu_music_done = 1;
					}
					else
					{
						module_reboot_before();
						music_prx_load = 1;//music_start(music_dir);
						music_prx_active = 1;
						all_done = 1;
						menu_music_done = 1;
						//gui_print("Music already started");
					}
				}
			break;

			case  PSP_CTRL_CIRCLE:
				all_done = 1;
				menu_music_done=1;
			break;
		}
	}
	if(!all_done)
	{
		change_menu_section();
	}
}

