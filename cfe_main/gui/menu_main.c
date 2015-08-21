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

#define MENU_CPU 0
#define MENU_BRIGHTNESS 1
#define MENU_SCREENSHOT 2
#define MENU_LOAD_PRX 3
#define MENU_USB 4
#define MENU_REMOTEJOY 5
#define MENU_USBHOST 6
#define MENU_RESTORE_MS 7
#define MENU_REBOOT 8
#define MENU_POWEROFF 9

#define MENU_CPU_X 29
#define MENU_BRIGHTNESS_X 49
#define MENU_SCREENSHOT_X 69
#define MENU_LOAD_PRX_X 89
#define MENU_USB_X 109
#define MENU_REMOTEJOY_X 129
#define MENU_USBHOST_X 149
#define MENU_RESTORE_MS_X 169
#define MENU_REBOOT_X 189
#define MENU_POWEROFF_X 209

int remotejoy_module;

int menu_main_items_x[10] =
{
	MENU_CPU_X,
	MENU_BRIGHTNESS_X,
	MENU_SCREENSHOT_X,
	MENU_LOAD_PRX_X,
	MENU_USB_X,
	MENU_REMOTEJOY_X,
	MENU_USBHOST_X,
	MENU_RESTORE_MS_X,
	MENU_REBOOT_X,
	MENU_POWEROFF_X
};

void draw_main_menu()
{
	if(menu_item == MENU_CPU)
	{
		setP(195, 30, 195 + (strlen(menu_str_cpu)+1)*8 , 42);
		drawRect(0, true, 0);
		setP0(200, 40);
		drawString(menu_str_cpu , FSHADOW);
	}
	else if(menu_item == MENU_BRIGHTNESS)
	{
		setP(195, 50, 195 + (strlen(menu_str_brightness)+1)*8 , 64);
		drawRect(0, true, 0);
		setP0(200, 60);
		drawString(menu_str_brightness , FSHADOW);
	}
}

void draw_main_menu_items()
{
	if(menu_item == MENU_CPU)
	{
		setP0(25, 40);
		drawString("Cpu Speed :", FSHADOW);	
	}	
	
	if(menu_item == MENU_BRIGHTNESS)
	{	
		setP0(25, 60);
		drawString("Brightness :", FSHADOW);
	}

	if(menu_item == MENU_SCREENSHOT)	
	{
		setP0(25, 80);
		drawString("Take Screenshot", FSHADOW);
	}

	if(menu_item == MENU_LOAD_PRX)	
	{
		setP0(25, 100);
		drawString("Load Prx Plugin", FSHADOW);	
	}

	if(menu_item == MENU_USB)	
	{
		setP0(25, 120);
		drawString("Start/Stop USB", FSHADOW);
	}

	if(menu_item == MENU_REMOTEJOY)	
	{
		setP0(25, 140);
		drawString("Start Remotejoy", FSHADOW);
	}

	if(menu_item == MENU_USBHOST)
	{
		setP0(25, 160);
		drawString("Remap usb to ms", FSHADOW);
	}

	if(menu_item == MENU_RESTORE_MS)
	{
		setP0(25, 180);
		drawString("Stop remap (reboot)", FSHADOW);
	}

	if(menu_item == MENU_REBOOT)
	{
		setP0(25, 200);
		drawString("Reboot", FSHADOW);
	}

	if(menu_item == MENU_POWEROFF)
	{
		setP0(25, 220);
		drawString("Power Off", FSHADOW);
	}
}

void menu_main_update_items()
{
	memset(menu_str_cpu, 0, 64);
	sprintf(menu_str_cpu, "%03d/%03d",scePowerGetCpuClockFrequency(),scePowerGetBusClockFrequency());

	memset(menu_str_brightness, 0, 64);
	sprintf(menu_str_brightness, "%i", brightness_level);
}

void main_menu_update_all_down()
{
	if(menu_item==0)
	{
		setP(20, menu_main_items_x[MENU_POWEROFF], 20 + 20*8 , menu_main_items_x[MENU_POWEROFF]+15);
		drawRect(0, true, 0);

		menu_main_update_items();

		menu_item=9;
		draw_main_menu_items();

		menu_item=0;
		setP(20, menu_main_items_x[menu_item], 20 + 20*8 , menu_main_items_x[menu_item]+15);
		drawRect(0, false, 0);
		
	}
	else
	{
		setP(20, menu_main_items_x[menu_item-1], 20 + 20*8 , menu_main_items_x[menu_item-1]+15);
		drawRect(0, true, 0);

		menu_main_update_items();
				
		menu_item-=1;
		draw_main_menu_items();

		menu_item+=1;
		setP(20, menu_main_items_x[menu_item], 20 + 20*8 , menu_main_items_x[menu_item]+15);
		drawRect(0, false, 0);
	}
	draw_main_menu_items();
	draw_main_menu();
}

void main_menu_update_all_up()
{
	if(menu_item==9)
	{
		setP(20, menu_main_items_x[MENU_CPU], 20 + 20*8 , menu_main_items_x[MENU_CPU]+15);
		drawRect(0, true, 0);

		menu_main_update_items();

		menu_item=0;
		draw_main_menu_items();

		menu_item=9;
		setP(20, menu_main_items_x[menu_item], 20 + 20*8 , menu_main_items_x[menu_item]+15);
		drawRect(0, false, 0);
		
	}
	else
	{
		setP(20, menu_main_items_x[menu_item+1], 20 + 20*8 , menu_main_items_x[menu_item+1]+15);
		drawRect(0, true, 0);

		menu_main_update_items();
				
		menu_item+=1;
		draw_main_menu_items();

		menu_item-=1;
		setP(20, menu_main_items_x[menu_item], 20 + 20*8 , menu_main_items_x[menu_item]+15);
		drawRect(0, false, 0);
	}
	draw_main_menu_items();
	draw_main_menu();
}

void draw_main_menu_init()
{
	setP(15, 25, 480, 249);
	clearRect(0);
	setP(16, 25, 459, 240);
	drawRect(15, true, 0x0A0000FF);

	setP(20, MENU_CPU_X, 20 + 20*8 , MENU_CPU_X+15);
	drawRect(0, false, 0);

	menu_main_update_items();
	draw_main_menu_items();
	
	setP0(25, 60);
	drawString("Brightness :", FSHADOW);
	setP0(25, 80);
	drawString("Take Screenshot", FSHADOW);
	setP0(25, 100);
	drawString("Load Prx Plugin", FSHADOW);		
	setP0(25, 120);
	drawString("Start/Stop USB", FSHADOW);
	setP0(25, 140);
	drawString("Start Remotejoy", FSHADOW);
	setP0(25, 160);
	drawString("Remap usb to ms", FSHADOW);
	setP0(25, 180);
	drawString("Stop remap (reboot)", FSHADOW);
	setP0(25, 200);
	drawString("Reboot", FSHADOW);
	setP0(25, 220);
	drawString("Power Off", FSHADOW);

	setP(195, 30, 195 + (strlen(menu_str_cpu)+1)*8 , 42);
	drawRect(0, true, 0);
	setP0(200, 40);
	drawString(menu_str_cpu , FSHADOW);
	
	setP(195, 50, 195 + (strlen(menu_str_brightness)+1)*8 , 64);
	drawRect(0, true, 0);
	setP0(200, 60);
	drawString(menu_str_brightness , FSHADOW);
}

int menu_main()
{
	all_done = 0;
	menu_main_done = 0;
	screenshot_done = 0;
	menu_item = MENU_CPU;

	draw_menu_section();

	draw_main_menu_init();

	while(!menu_main_done)
	{
		sceDisplaySetBrightness(brightness_level, 0);

		u32 key = ctrlWaitMask(PSP_CTRL_LTRIGGER | PSP_CTRL_RTRIGGER | PSP_CTRL_UP | PSP_CTRL_DOWN | PSP_CTRL_LEFT | PSP_CTRL_RIGHT | PSP_CTRL_CROSS | PSP_CTRL_CIRCLE);
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

					menu_main_done = 1;
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
					menu_main_done = 1;
			break;

			case  PSP_CTRL_DOWN:
				if(menu_item < 9) menu_item++;
					else menu_item = 0;

					main_menu_update_all_down();
			break;

			case  PSP_CTRL_UP:
				if(menu_item > 0) menu_item--;
					else menu_item = 9;

					main_menu_update_all_up();
			break;


			case  PSP_CTRL_RIGHT:
				if(menu_item == MENU_CPU)
				{
					if(cpuSpeed<333) cpuSpeed++;
					scePowerSetClockFrequency(cpuSpeed, cpuSpeed, cpuSpeed/2);
					menu_main_update_items();
					draw_main_menu();
				}
				if(menu_item == MENU_BRIGHTNESS)
				{
					if(brightness_level<99) brightness_level++;
					sceDisplaySetBrightness(brightness_level, 0);
					menu_main_update_items();
					draw_main_menu();
				}
			break;

			case  PSP_CTRL_LEFT:
				if(menu_item == MENU_CPU)
				{
					if(cpuSpeed > 20) cpuSpeed--;
					scePowerSetClockFrequency(cpuSpeed, cpuSpeed, cpuSpeed/2);
					menu_main_update_items();
					draw_main_menu();
				}
				if(menu_item == MENU_BRIGHTNESS)
				{
					if(brightness_level > 0) brightness_level--;
					sceDisplaySetBrightness(brightness_level, 0);
					menu_main_update_items();
					draw_main_menu();
				}
			break;

			case PSP_CTRL_CROSS:
				if(menu_item == MENU_SCREENSHOT)
				{
					int plop = sceIoDopen(config->capture_folder);
					if (plop >= 0)
					{
						printf("Directory already exist\n");					
					} 
					else 
					{
						sceIoMkdir(config->capture_folder, 0777);
					}
					freeGraphic();
					sceKernelDeleteHeap(heapid);
					screenshot_done = 1;
					all_done = 1;
					menu_main_done=1;
					//sprintf(strBuf0,"Screenshot saved to %s", file);
				}

				if(menu_item == MENU_LOAD_PRX)
				{
					load_prx = 1;
#ifndef LIGHT
					filer_on();
#endif
					load_prx = 0;
					menu_item = MENU_CPU;
					draw_main_menu_init();	
				}

				if(menu_item == MENU_USB)
				{
					if(remotejoy_active) stopRemotejoy();
					if(usbhost_active) unloadUsbhost();

					state = sceUsbGetState();
					loadUsb();
					state = sceUsbGetState();
					gui_print("");
				}

				if(menu_item == MENU_REMOTEJOY)
				{
					unloadUsb();
					if(usbhost_active) unloadUsbhost();

					if(!remotejoy_module)
					{
#ifndef GAME
						int ret = loadStartModuleWithArgs("ms0:/seplugins/cfe/RemoteJoyLite.prx", 1, 0, NULL);
#else
						int ret = loadStartModuleWithArgs("ms0:/seplugins/cfe/RemoteJoyLite.prx", 8, 0, NULL);
#endif					
						char tmpStr[256];
						if(ret != 1)
						{
							state = sceUsbGetState();
							sprintf(tmpStr, "NOK : Remotejoy error %x", ret);
							gui_print(tmpStr);
						}
						else
						{
							remotejoy_active = 1;
							remotejoy_module = 1;
							state = sceUsbGetState();
							gui_print("OK : Remotejoy Loaded Successfully");
						}
				
					}
					else
					{
						if(remotejoy_active)
						{
							stopRemotejoy();
						}
						else
						{
							startRemotejoy();
						}
					}
				}

				if(menu_item == MENU_USBHOST)
				{
#ifndef GAME
					unloadUsb();
					if(remotejoy_active) stopRemotejoy();

					if(!cfe_redirect_loaded)
					{
						usbhost_active = 1;
						load_cfe_redirect = 1;
					}
					else
					{
						gui_print("Usb already remaped to ms0");
/*
						if(!usbhost_active) 
						{
							usbhost_active = 1;
							redirect(1);
						}
*/
					}
					
					all_done = 1;
					menu_main_done = 1;

					//filer_on();
					//menu_item = MENU_CPU;
					//draw_main_menu_init();
#else
					gui_print("Remaping not available under games");
#endif
				}

				if(menu_item == MENU_RESTORE_MS)
				{
#ifndef GAME
					usbhost_active = 0;
					stop_redirect = 1;
					all_done = 1;
					menu_main_done = 1;
#else
					gui_print("Remaping not available under games");
#endif
				}
				
				if(menu_item == MENU_REBOOT)
				{
					do_reboot = 1;
					all_done = 1;
					menu_main_done = 1;
				}

				if(menu_item == MENU_POWEROFF)
				{
					do_poweroff = 1;
					all_done = 1;
					menu_main_done = 1;
				}
			break;

			case  PSP_CTRL_CIRCLE:
				all_done = 1;
				menu_main_done = 1;
			break;
		}
	}
	if(!all_done)
	{
	//	draw_menu_section();
		change_menu_section();
	}

	return 0;	

}

