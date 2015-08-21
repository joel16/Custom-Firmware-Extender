#include <pspkernel.h>
#include <string.h>
#include <stdio.h>
#include <pspdisplay_kernel.h>

#include "guicommon.h"
#include "graphic.h"
#include "ctrl.h"
#ifndef LIGHT
#include "filer.h"
#endif
#include "utils.h"

int tm_done;
int tm_up;
int tm_down;

void get_fw_list() 
{

/*	if(sceKernelDevkitVersion() == 0x01050001)
	{
		
	}
	else
	{
*/
		int ret,fd;

		fd = sceIoDopen("ms0:/TM/");
		dlist_num = 0;
		ret = 1;

/*	#ifdef GAME
		if(dlist_mem) sceKernelFreePartitionMemory(dlist_mem);
		dlist_mem = sceKernelAllocPartitionMemory(8, "block", 0, sizeof(SceIoDirent), NULL);
		dlist = (SceIoDirent*)sceKernelGetBlockHeadAddr(dlist_mem);
	#else
		//dlist_mem = sceKernelAllocPartitionMemory(2, "block", 0, sizeof(SceIoDirent), NULL);
		memset(&dlist, 0, sizeof(SceIoDirent));
	#endif
*/
		memset(&dlist, 0, sizeof(SceIoDirent));
		int first_pass = 1;

		while((ret>0) && (dlist_num<MAXDIRNUM)) 
		{
			sceKernelDelayThread(10000);

			ret = sceIoDread(fd, &dlist[dlist_num]);
			// replace one of the kernel "dir up" points by the NAND reboot option
			if(first_pass)
			{
				strcpy(dlist[dlist_num].d_name, "NAND");
				dlist_num++;
				first_pass = 0;
				continue;
			}

			// if it's one of the kernel "dir up" points, do not include it in the reboot option
			if (dlist[dlist_num].d_name[0] == '.') continue;

			// if it's a DC dir do not include it too
			if (((dlist[dlist_num].d_name[0]) == 'd') &&
			((dlist[dlist_num].d_name[1]) == 'c')) continue;
			if (((dlist[dlist_num].d_name[0]) == 'D') &&
			((dlist[dlist_num].d_name[1]) == 'C')) continue;

			// if there is no ipl.bin file in the dir do not include it too
			char tmp[256];
			memset(tmp, 0, 256);
			sprintf(tmp, "ms0:/TM/%s/ipl.bin", dlist[dlist_num].d_name);
			if(!file_exist(tmp))
			{
				printf("ipl.bin not found in %s\n", tmp);
				continue;
			}
				
			if (ret>0) dlist_num++;
		}
		sceIoDclose(fd);
		if (dlist_start  >= dlist_num) { dlist_start  = dlist_num-1; }
		if (dlist_start  <  0)         { dlist_start  = 0;           }
		if (dlist_curpos >= dlist_num) { dlist_curpos = dlist_num-1; }
		if (dlist_curpos <  0)         { dlist_curpos = 0;           }

		if(dlist_num < 1) noDir=1; else noDir=0;

//	}
}

int tm_keys(void)
{
/*
	if(sceKernelDevkitVersion() == 0x01050001)
	{
		u32 key = ctrlWaitMask(PSP_CTRL_UP | PSP_CTRL_DOWN | PSP_CTRL_RTRIGGER | PSP_CTRL_CROSS | PSP_CTRL_CIRCLE);
		switch(key)
		{
			case PSP_CTRL_RTRIGGER:
				return 2;
			break;

			case PSP_CTRL_CROSS:
				if(!file_exist("ms0:/TM/config.txt.back")) copy_file("ms0:/TM/config.txt", "ms0:/TM/config.txt.back");
				if(file_exist("ms0:/TM/config.txt.back")) sceIoRemove("ms0:/TM/config.txt");
		
				char temp[128];
				sprintf(temp, "NOTHING = \"NAND\";");
				SceUID tm_cfg = sceIoOpen("ms0:/TM/config.txt", PSP_O_CREAT | PSP_O_RDWR, 0777);
				sceIoWrite(tm_cfg, temp, strlen(temp));
				sceIoClose(tm_cfg);

				tm_cfg = sceIoOpen("ms0:/seplugins/cfe/tm_reboot", PSP_O_CREAT | PSP_O_RDWR, 0777);
				sceIoWrite(tm_cfg, "1", 1);
				sceIoClose(tm_cfg);

				sceKernelDelayThread(10000);

				tm_done = 1;
				all_done = 1;
				do_reboot = 1;
			break;

			case PSP_CTRL_CIRCLE:
				tm_done = 1;
				all_done = 1;			
			break;
		}
	}
	else
	{
*/
		u32 key = ctrlWaitMask(PSP_CTRL_UP | PSP_CTRL_DOWN | PSP_CTRL_RTRIGGER | PSP_CTRL_LTRIGGER | PSP_CTRL_CROSS | PSP_CTRL_CIRCLE);
		switch(key)
		{
			case PSP_CTRL_RTRIGGER:
				return 2;
			break;

			case PSP_CTRL_LTRIGGER:
				return 7;
			break;

			case PSP_CTRL_UP:
				if (dlist_curpos > 0) 
				{
					dlist_curpos--;
					if (dlist_curpos < dlist_start) { dlist_start = dlist_curpos; return 5;}
					return 3;
				}
			break;

			case PSP_CTRL_DOWN:
				if (dlist_curpos < (dlist_num-1)) 
				{
					dlist_curpos++;
					if (dlist_curpos >= (dlist_start+12)) { dlist_start++;  return 5;}
					return 4;
				}
			break;

			case PSP_CTRL_CROSS:
				return 6;
			break;

			case PSP_CTRL_CIRCLE:
				tm_done = 1;
				all_done = 1;			
			break;

			case PSP_CTRL_SQUARE:
			break;
		}
//	}
	return 0;
}

void tm_draw()
{
/*
	if(sceKernelDevkitVersion() == 0x01050001)
	{
		setP0(35, 50);
		drawString("Reboot to NAND", FTHICK | FSHADOW);
	}
	else
	{
*/
		//Display directory listing
		int tm_i = dlist_start;
		while (tm_i<(dlist_start+12)) 
		{
			if (tm_i<dlist_num) 
			{
				if (tm_i==dlist_curpos) 
				{
					if(!(tm_up | tm_down))
					{
						setP(25, (((tm_i-dlist_start)+5)*13)+2, 25 + 20*10 , ((tm_i-dlist_start)+6)*13+2);
						drawRect(0, false, 0);	
					}
					else if(tm_up)
					{
						setP(25, (((tm_i-dlist_start+1)+5)*13)+2, 25 + 20*10 , ((tm_i-dlist_start+1)+6)*13+2);
						drawRect(0, true, 0);	
						setP(25, (((tm_i-dlist_start)+5)*13)+2, 25 + 20*10 , ((tm_i-dlist_start)+6)*13+2);
						drawRect(0, false, 0);	
						memset(path_tmp, 0, MAXPATH);
						strncpy(path_tmp, dlist[tm_i+1].d_name, 21);
						setP0(35, ((tm_i-dlist_start+1)+6)*13);
						drawString(path_tmp, FSHADOW);
						memset(path_tmp, 0, MAXPATH);
						strncpy(path_tmp, dlist[tm_i].d_name, 21);
						setP0(35, ((tm_i-dlist_start)+6)*13);
						drawString(path_tmp, FSHADOW);
					}
					else if(tm_down)
					{
						setP(25, (((tm_i-dlist_start-1)+5)*13)+2, 25 + 20*10 , ((tm_i-dlist_start-1)+6)*13+2);
						drawRect(0, true, 0);
						setP(25, (((tm_i-dlist_start)+5)*13)+2, 25 + 20*10 , ((tm_i-dlist_start)+6)*13+2);
						drawRect(0, false, 0);	
						memset(path_tmp, 0, MAXPATH);
						strncpy(path_tmp, dlist[tm_i-1].d_name, 21);
						setP0(35, ((tm_i-dlist_start-1)+6)*13);
						drawString(path_tmp, FSHADOW);	
						memset(path_tmp, 0, MAXPATH);
						strncpy(path_tmp, dlist[tm_i].d_name, 21);
						setP0(35, ((tm_i-dlist_start)+6)*13);
						drawString(path_tmp, FSHADOW);	
					}
				}
			
				if(!(tm_up | tm_down))
				{
					if(FIO_SO_ISDIR(dlist[tm_i].d_stat.st_attr) || FIO_S_ISDIR(dlist[tm_i].d_stat.st_mode)) 
					{
						//if(!flashing_font) oslIntraFontSetStyle(ltn13, 0.5f,LITEGRAY,BLACK,INTRAFONT_ALIGN_LEFT);		
					}

					memset(path_tmp, 0, MAXPATH);
					strncpy(path_tmp, dlist[tm_i].d_name, 21);
					setP0(35, ((tm_i-dlist_start)+6)*13);
					drawString(path_tmp, FSHADOW);
				}
			}
			tm_i++;
		}
		if((tm_up | tm_down)) {tm_up = 0; tm_down = 0; }
//	}
}

void menu_tm() 
{
	draw_menu_section();

	setP(15, 25, 480, 249);
	clearRect(0);
	setP(16, 25, 250, 240);
	drawRect(15, true, 0x0A0000FF);

	tm_done = 0;
	all_done = 0;

	get_fw_list();
	dlist_start  = 0;
	dlist_curpos = 0;
	now_depth    = 0;
	tm_draw();

	while(!tm_done)
	{
		sceDisplaySetBrightness(brightness_level, 0);

		switch(tm_keys()) 
		{
			case 2:
#ifndef GAME
				if(menu_section < 4) menu_section++;
#else
				if(menu_section < 3) menu_section++;
#endif
					else menu_section = 0;

				tm_done = 1;
			break;

			case 7:
				if(menu_section > 0) menu_section--;
#ifndef GAME
					else menu_section = 4;
#else
					else menu_section = 3;
#endif
				tm_done = 1;
			break;

			case 3:
				tm_up = 1;
				tm_draw();
			break;

			case 4:
				tm_down = 1;
				tm_draw();
				break;

			case 5:
				tm_draw();
				break;

			case 6:
				if(!file_exist("ms0:/TM/config.txt.back")) copy_file("ms0:/TM/config.txt", "ms0:/TM/config.txt.back");
				if(file_exist("ms0:/TM/config.txt.back")) sceIoRemove("ms0:/TM/config.txt");
		
				char temp[128];

				if(strcmp(dlist[dlist_curpos].d_name, "NAND") == 0)
					sprintf(temp, "NOTHING = \"NAND\";");
				else
					sprintf(temp, "NOTHING = \"/TM/%s/ipl.bin\";", dlist[dlist_curpos].d_name);

				SceUID tm_cfg = sceIoOpen("ms0:/TM/config.txt", PSP_O_CREAT | PSP_O_RDWR, 0777);
				sceIoWrite(tm_cfg, temp, strlen(temp));
				sceIoClose(tm_cfg);

				tm_cfg = sceIoOpen("ms0:/seplugins/cfe/tm_reboot", PSP_O_CREAT | PSP_O_RDWR, 0777);
				sceIoWrite(tm_cfg, "1", 1);
				sceIoClose(tm_cfg);

				sceKernelDelayThread(10000);

				tm_done = 1;
				all_done = 1;
				do_reboot = 1;
			break;				
		}
	}

	if(!all_done)
	{
		change_menu_section();
	}

	if(dlist_mem) sceKernelFreePartitionMemory(dlist_mem);
}

