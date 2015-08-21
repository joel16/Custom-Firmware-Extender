/////////////////////////////////////////////
//* Custom Firmware Extender 3.0 Firmware *//
/////////////////////////////////////////////

#include <psptypes.h>
#include <pspkernel.h>
#include <string.h>
#include <stdio.h>
#include <pspusb.h>
#include <psppower.h>
#include <pspdisplay.h>
#include <pspusbstor.h>
#include <pspdisplay_kernel.h>
#include <pspumd.h>

#include "../include/systemctrl_se.h"
#include "../include/pspmodulemgr_kernel.h"
#include "../include/systemctrl.h"
#include "../include/pspsysmem_kernel.h"

#include "main.h"
#include "screenshot.h"
#include "conf.h"
#include "translateButtons.h"
#include "graphic.h"
#include "ctrl.h"
//#include "filer.h"
#include "common.h"
#ifndef LIGHT
#include "music.h"
#endif
#include "guicommon.h"
#include "mem.h"
#include "utils.h"
#include "blit.h"

PSP_MODULE_INFO("cfe_mod", 0x1000, 1, 1);
PSP_MAIN_THREAD_ATTR(0);
PSP_MAIN_THREAD_STACK_SIZE_KB(0);

void menu_main();
#ifndef LIGHT
void menu_music();
void menu_music_in_game();
#endif
void menu_info();
void menu_tm();
void menu_cfg();

extern void scePower_driver_0442D852();
extern int filer_on();

int webBrowserLoaded = 0;
int usbMass = 0;
int usbModules = 0;
int firstBoot = 0;
int stopSleepMode = 0;
int usbhostLoaded = 0;

int remotejoy =0;

int num = 0;
u32 retVal;

SceUID main_thid;

int cleanExit;

#define MAX_THREAD 64

static int thread_count_start, thread_count_now;
static SceUID pauseuid = -1, thread_buf_start[MAX_THREAD], thread_buf_now[MAX_THREAD], thid1 = -1;

void pauseGame(SceUID thid)
{
	if(pauseuid >= 0) return;
	pauseuid = thid;

	SceKernelThreadInfo thinfo;

	sceKernelGetThreadmanIdList(SCE_KERNEL_TMID_Thread, thread_buf_now, MAX_THREAD, &thread_count_now);
	int x, y, match;
	for(x = 0; x < thread_count_now; x++)
	{
		match = 0;
		SceUID tmp_thid = thread_buf_now[x];
		for(y = 0; y < thread_count_start; y++)
		{
			if((tmp_thid == thread_buf_start[y]) || (tmp_thid == thid1))
			{
				match = 1;
				break;
			}
		}
		if(match == 0)
		{
			memset(&thinfo, 0, sizeof(SceKernelThreadInfo));
			thinfo.size = sizeof(SceKernelThreadInfo);
			sceKernelReferThreadStatus(tmp_thid, &thinfo);

			if(strcmp(&thinfo.name[0], "playlist_thread")==0)
			{
				printf("No Suspend : thread[%i] (%s)\n", x, &thinfo.name[0]);
			}
			else if(strcmp(&thinfo.name[0], "music_thread")==0) 
			{
				printf("No Suspend : thread[%i] (%s)\n", x, &thinfo.name[0]);
			}
			else if(strcmp(&thinfo.name[0], "SceUsbstor")==0)
			{
				printf("No Suspend : thread[%i] (%s)\n", x, &thinfo.name[0]);
			}
			else if(strcmp(&thinfo.name[0], "SceUsbstorMsMed")==0)
			{
				printf("No Suspend : thread[%i] (%s)\n", x, &thinfo.name[0]);
			}
			else if(strcmp(&thinfo.name[0], "SceUsbstorMsCmd")==0)
			{
				printf("No Suspend : thread[%i] (%s)\n", x, &thinfo.name[0]);
			}
			else if(strcmp(&thinfo.name[0], "SceUsbstorBoot")==0)
			{
				printf("No Suspend : thread[%i] (%s)\n", x, &thinfo.name[0]);
			}
			else 
			{
				printf("Suspending : thread[%i] (%s)\n", x, &thinfo.name[0]);
				sceKernelSuspendThread(tmp_thid);
			}
		}
	}
}

void resumeGame(SceUID thid)
{
	SceKernelThreadInfo thinfo;

	if(pauseuid != thid)
		return;
	pauseuid = -1;
	int x, y, match;
	for(x = 0; x < thread_count_now; x++)
	{
		match = 0;
		SceUID tmp_thid = thread_buf_now[x];
		for(y = 0; y < thread_count_start; y++)
		{
			if((tmp_thid == thread_buf_start[y]) || (tmp_thid == thid1))
			{
				match = 1;
				break;
			}
		}
		if(match == 0)
		{
			memset(&thinfo,0,sizeof(SceKernelThreadInfo));
			thinfo.size = sizeof(SceKernelThreadInfo);
			sceKernelReferThreadStatus(tmp_thid, &thinfo);

			if(strcmp(&thinfo.name[0], "playlist_thread")==0) 
			{
				printf("No Resume : thread[%i] (%s)\n", x, &thinfo.name[0]);
			}
			else if(strcmp(&thinfo.name[0], "music_thread")==0) 
			{
				printf("No Resume : thread[%i] (%s)\n", x, &thinfo.name[0]);
			}
			else if(strcmp(&thinfo.name[0], "SceUsbstor")==0)
			{
				printf("No Resume : thread[%i] (%s)\n", x, &thinfo.name[0]);
			}
			else if(strcmp(&thinfo.name[0], "SceUsbstorMsMed")==0)
			{
				printf("No Resume : thread[%i] (%s)\n", x, &thinfo.name[0]);
			}
			else if(strcmp(&thinfo.name[0], "SceUsbstorMsCmd")==0)
			{
				printf("No Resume : thread[%i] (%s)\n", x, &thinfo.name[0]);
			}
			else if(strcmp(&thinfo.name[0], "SceUsbstorBoot")==0)
			{
				printf("No Resume : thread[%i] (%s)\n", x, &thinfo.name[0]);
			}
			else
			{
				printf("Resuming : thread[%i] (%s)\n", x, &thinfo.name[0]);
				sceKernelResumeThread(tmp_thid);
			}
		}
	}
}

void execEboot(char *target)
{
	cleanExit = 0;
			
	sceKernelDelayThread(1000);

	struct SceKernelLoadExecVSHParam param;
			
	memset(&param, 0, sizeof(param));
			
	param.key = "game";
	param.size = sizeof(param);
	param.args = strlen(target)+1;
	param.argp = target;
	
	
	sctrlKernelLoadExecVSHMs2(target, &param);	
	
	sceKernelExitDeleteThread(0);
}

#ifndef GAME
void checkUnloadBrowser() {

	SceUID thids[50];
	int thid_count = 0;
	int i;
	SceKernelThreadInfo thinfo;

	sceKernelGetThreadmanIdList(SCE_KERNEL_TMID_Thread,thids,50,&thid_count);
	for(i=0;i<thid_count;i++)
        {
		memset(&thinfo,0,sizeof(SceKernelThreadInfo));
		thinfo.size = sizeof(SceKernelThreadInfo);

		sceKernelReferThreadStatus(thids[i],&thinfo);

		if(strcmp(&thinfo.name[0],"SceHtmlViewer")==0)
		{
			webBrowserLoaded=1;

			//sceKernelTerminateDeleteThread(thids[i]);
		}
	}
}
#endif

#ifndef GAME
void restore_tm_config()
{
	if(file_exist("ms0:/seplugins/cfe/tm_reboot"))
	{
		sceIoRemove("ms0:/seplugins/cfe/tm_reboot");

		if(file_exist("ms0:/TM/config.txt.back"))
		{
			sceIoRemove("ms0:/TM/config.txt");
			copy_file("ms0:/TM/config.txt.back", "ms0:/TM/config.txt");
		}
	}
}
#endif

void menu_load()
{
	if(!screenshot_done)
	{
		thid1 = sceKernelGetThreadId();
		pauseGame(thid1);
	}
	else
	{
		screenshot();
	}

	void * vram;
	int bufferwidth = 512, pixelformat = PSP_DISPLAY_PIXEL_FORMAT_8888, unk = 1;
	vram = (void *) (0x40000000 | 0x04000000);
	if (sceKernelDevkitVersion() > 0x01050001)
	{
		sceDisplayGetFrameBuf(&vram, &bufferwidth, &pixelformat, unk);
		vram = (void *) ((u32)vram | 0x40000000);
	}
	sceDisplaySetMode(0, SCREEN_WIDTH, SCREEN_HEIGHT);
	sceDisplaySetFrameBuf((void *) vram, bufferwidth, pixelformat, unk);
#ifdef VSH	
	sceKernelSetDdrMemoryProtection((void *)0x88400000, 0x00400000, 0xF);
	heapid = sceKernelCreateHeap(5, 1024 * 1024, 1, "graphic_heap");
#elif GAME
#ifndef LIGHT
	//sceKernelSetDdrMemoryProtection((void *)0x0a000000, 0x00400000, 0xF);
	heapid = sceKernelCreateHeap(8, 1024 * 1024, 1, "graphic_heap");
#else
	sceKernelSetDdrMemoryProtection((void *)0x88400000, 0x00400000, 0xF);
	heapid = sceKernelCreateHeap(5, 1024 * 1024, 1, "graphic_heap");
#endif
#endif
	if(heapid >= 0 && initGraphic(heapid, vram))
	{
		sceDisplaySetBrightness(brightness_level, 0);

#ifndef LIGHT
		if(!menu_music_in_game_active)
		{
#endif
			setP(5, 0, 474, 19);
			drawRect(15, true, 0x0A0000FF);
			setP(0, 250, 480, 272);
			drawRect(0, true, 0x0A0000FF);
			gui_print("");

			if(menu_section == MENU_SECTION_MAIN) menu_main();
#ifndef LIGHT
			else if(menu_section == MENU_SECTION_MUSIC) menu_music();
#endif
#ifdef VSH
			else if(menu_section == MENU_SECTION_TM) menu_tm();
#endif
			else if(menu_section == MENU_SECTION_INFO) menu_info();
			else if(menu_section == MENU_SECTION_CFG) menu_cfg();
#ifndef LIGHT
		}
		else
		{
			menu_music_in_game();
		}
#endif
		freeGraphic();
		sceKernelDeleteHeap(heapid);

	}
	if(!screenshot_done) { sceKernelDelayThread(100000); resumeGame(thid1); }

#ifndef LIGHT
	if(music_prx_load)
	{ 
		main_music(config->music_folder); 
		music_prx_load = 0; 
	}
#endif
}

SceUID display_thid;

int display_thread(SceSize args, void *argp)
{
	while(1) 
	{
		if(display_show_message_one_time)
		{
			int i;
			for(i=0; i<480; i++)
			{
				blit_string(1, 32, display_msg, 0xffffff, 0x000000);		
				sceKernelDelayThread(2000);
			}
			display_show_message_one_time = 0;
		}
		sceKernelDelayThread(50000);
	}
	return 0;
}

int threadMain(SceSize args, void *argp)
{	
	sceKernelDelayThread(40*100000);

	defaultBrightnessEnable = 0;
	defaultCpuSpeedEnable = 0;

	SceUID memid;

	config = (CONFIGFILE *) Kmalloc(1, sizeof(CONFIGFILE), &memid);

#ifdef GAME
	sceKernelDelayThread(20*100000);
	read_config("ms0:/seplugins/cfe/game.cfg", config);
#else
	read_config("ms0:/seplugins/cfe/vsh.cfg", config);
#endif
	while(!translateButtons())
	{
		printf("waiting\n");
	}

	if(defaultCpuSpeedEnable) cpuSpeed = config->default_cpu_speed;
		else cpuSpeed = 222;
	scePowerSetClockFrequency(cpuSpeed, cpuSpeed, cpuSpeed/2);

	if(defaultBrightnessEnable) sceDisplaySetBrightness(config->default_brightness, 0);

	sceDisplayGetBrightness(&brightness_level, 0);

	state = 0;
	cleanExit = 1;
	screenshot_done = 0;
#ifndef LIGHT
	music_prx_active = 0;
#endif

#ifndef GAME
	restore_tm_config();
	cfe_redirect_loaded = 0;
	load_cfe_redirect = 0;
#endif
	load_prx = 0;

	display_thid = sceKernelCreateThread("cfe_display", display_thread, 8, 0x200, 0, NULL);
	if(display_thid >= 0) sceKernelStartThread(display_thid, args, argp);

	while(1) 
	{
		sceKernelDelayThreadCB(5*10000);//needs less running time

		if(do_reboot) scePower_driver_0442D852();
		if(do_poweroff) scePowerRequestStandby();

#ifndef GAME
		if(load_cfe_redirect)
		{
			loadStartModuleWithArgs("ms0:/seplugins/cfe/cfe_redirect.prx", 1, 0, NULL);

			sceKernelDelayThread(100000);

			if(usbhost_active) redirect(1);

			load_cfe_redirect = 0;
			cfe_redirect_loaded = 1;
		}
		if(stop_redirect)
		{
			scePower_driver_0442D852();
			//redirect(0);
			stop_redirect = 0;
		}
#endif
		state = sceUsbGetState();

		if(stopSleepMode) scePowerTick(0);

		if(!screenshot_done)
		{
			u32 key = ctrlWaitMask(button->combo | button->menu | button->screenshot | button->cpuPlus | button->cpuMinus | \
				button->brightnessPlus | button->brightnessMinus | button->music);

			if((key & (button->menu | button->combo)) == (button->menu | button->combo))
			{
				menu_section = 0;
				sceKernelDelayThread(100000);
				menu_load();
			}
			else if((key & (button->screenshot | button->combo)) == (button->screenshot | button->combo))
			{
				thid1 = sceKernelGetThreadId();
				pauseGame(thid1);
				sceKernelDelayThread(100000);
				screenshot();
				resumeGame(thid1);
			}
			else if((key & (button->cpuPlus | button->combo)) == (button->cpuPlus | button->combo))
			{
				if((cpuSpeed<333) & (cpuSpeed >= 266)) cpuSpeed = 333;
					else if ((cpuSpeed<266) & (cpuSpeed >= 222)) cpuSpeed = 266;
					else if ((cpuSpeed<222) & (cpuSpeed >= 111)) cpuSpeed = 222;
					else if ((cpuSpeed<111) & (cpuSpeed >= 66)) cpuSpeed = 111;
				scePowerSetClockFrequency(cpuSpeed, cpuSpeed, cpuSpeed/2);

				memset(display_msg, 0, 128);
				sprintf(display_msg, "Cpu Speed : %i/%i", cpuSpeed, cpuSpeed/2);

				display_show_message_one_time = 1;
			}
			else if((key & (button->cpuMinus | button->combo)) == (button->cpuMinus | button->combo))
			{
				if((cpuSpeed>66) & (cpuSpeed <= 111)) cpuSpeed = 66;
					else if((cpuSpeed>111) & (cpuSpeed <= 222)) cpuSpeed = 111;
					else if((cpuSpeed>222) & (cpuSpeed <= 266)) cpuSpeed = 222;
					else if ((cpuSpeed>266) & (cpuSpeed <= 333)) cpuSpeed = 266;
				scePowerSetClockFrequency(cpuSpeed, cpuSpeed, cpuSpeed/2);

				memset(display_msg, 0, 128);
				sprintf(display_msg, "Cpu Speed : %i/%i", cpuSpeed, cpuSpeed/2);

				display_show_message_one_time = 1;
			}
			else if((key & (button->brightnessPlus | button->combo)) == (button->brightnessPlus | button->combo))
			{
				if((brightness_level<=100) & (brightness_level >= 90)) brightness_level = 99;
					else if ((brightness_level<=90) & (brightness_level >= 80)) brightness_level = 90;
					else if ((brightness_level<=80) & (brightness_level >= 70)) brightness_level = 80;
					else if ((brightness_level<=70) & (brightness_level >= 60)) brightness_level = 70;
					else if ((brightness_level<=60) & (brightness_level >= 50)) brightness_level = 60;
					else if ((brightness_level<=50) & (brightness_level >= 40)) brightness_level = 50;
					else if ((brightness_level<=40) & (brightness_level >= 30)) brightness_level = 40;
					else if ((brightness_level<=30) & (brightness_level >= 20)) brightness_level = 30;

				sceDisplaySetBrightness(brightness_level, 0);

				memset(display_msg, 0, 128);
				sprintf(display_msg, "Brightness : %i", brightness_level);

				display_show_message_one_time = 1;
			}
			else if((key & (button->brightnessMinus | button->combo)) == (button->brightnessMinus | button->combo))
			{
				if((brightness_level>=20) & (brightness_level <= 30)) brightness_level = 20;
					else if ((brightness_level>=30) & (brightness_level <= 40)) brightness_level = 30;
					else if ((brightness_level>=40) & (brightness_level <= 50)) brightness_level = 40;
					else if ((brightness_level>=50) & (brightness_level <= 60)) brightness_level = 50;
					else if ((brightness_level>=60) & (brightness_level <= 70)) brightness_level = 60;
					else if ((brightness_level>=70) & (brightness_level <= 80)) brightness_level = 70;
					else if ((brightness_level>=80) & (brightness_level <= 90)) brightness_level = 80;
					else if ((brightness_level>=90) & (brightness_level <= 100)) brightness_level = 90;

				sceDisplaySetBrightness(brightness_level, 0);

				memset(display_msg, 0, 128);
				sprintf(display_msg, "Brightness : %i", brightness_level);

				display_show_message_one_time = 1;
			}
#ifndef LIGHT
			else if((key & (button->music | button->combo)) == (button->music | button->combo))
			{
				if(music_prx_active)
				{
					menu_music_in_game_active = 1;
					menu_load();
				}
			}
#endif
		}
		else { menu_section = 0;  menu_load(); }
	}
	return 0;
}



int module_start(SceSize args, void *argp)
{
	sceKernelGetThreadmanIdList(SCE_KERNEL_TMID_Thread, thread_buf_start, MAX_THREAD, &thread_count_start);
#ifdef VSH
	SceUID main_thid = sceKernelCreateThread("cfe_vsh_main", threadMain, 47, 0x4000, 0, NULL);
#elif GAME
#ifndef LIGHT
	SceKernelThreadOptParam option;
	memset(&option, 0, sizeof(option));
	option.size = sizeof(option);
	option.stackMpid = 8;

	SceUID main_thid = sceKernelCreateThread("cfe_game_main", threadMain, 47, 0x2000, 0, &option);
#else
	SceUID main_thid = sceKernelCreateThread("cfe_light_main", threadMain, 47, 0x2000, 0, NULL);
#endif
#endif
	if(main_thid >= 0)
	{
		sceKernelStartThread(main_thid, 0, 0);
	}
	return 0;
}

int module_stop(SceSize args, void *argp)
{
	return 0;
}
