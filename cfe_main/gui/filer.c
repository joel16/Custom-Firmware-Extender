#include <pspsdk.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <pspctrl.h>

#include "ctrl.h"
#include "filer.h"
#include "graphic.h"
#include "common.h"
#include "conf.h"
#include "guicommon.h"
#include "utils.h"

CONFIGFILE *config;

int filer_done;
int filer_up;
int filer_down;

char prx_path[512];

void Get_DirList(char *path) 
{
	int ret,fd;

	fd = sceIoDopen(path);
	dlist_num = 0;
	ret = 1;

	memset(&dlist, 0, sizeof(SceIoDirent));

/*#ifdef GAME
	if(dlist_mem) sceKernelFreePartitionMemory(dlist_mem);
	dlist_mem = sceKernelAllocPartitionMemory(8, "block", 0, sizeof(SceIoDirent), NULL);
	dlist_mem = sceKernelAllocPartitionMemory(2, "block", 0, sizeof(SceIoDirent), NULL);
#else

#endif
	dlist = (SceIoDirent*)sceKernelGetBlockHeadAddr(dlist_mem);
*/

	while((ret>0) && (dlist_num<MAXDIRNUM)) 
	{
		ret = sceIoDread(fd, &dlist[dlist_num]);

		if (dlist[dlist_num].d_name[0] == '.') continue;
				
		if (ret>0) dlist_num++;
	}
	sceIoDclose(fd);
	if (dlist_start  >= dlist_num) { dlist_start  = dlist_num-1; }
	if (dlist_start  <  0)         { dlist_start  = 0;           }
	if (dlist_curpos >= dlist_num) { dlist_curpos = dlist_num-1; }
	if (dlist_curpos <  0)         { dlist_curpos = 0;           }

	if(dlist_num < 1) noDir=1; else noDir=0;
}

int filer_Keys(void)
{
	int i;

	u32 key = ctrlWaitMask(PSP_CTRL_UP | PSP_CTRL_DOWN | PSP_CTRL_LEFT | PSP_CTRL_RIGHT | PSP_CTRL_CROSS | PSP_CTRL_CIRCLE | PSP_CTRL_SQUARE);
	switch(key)
	{
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
			if(!noDir)
			{
				if (FIO_S_ISDIR(dlist[dlist_curpos].d_stat.st_mode)) 
				{
					if (now_depth<MAXDEPTH)
					{
						strcat(now_path, dlist[dlist_curpos].d_name);
						strcat(now_path,"/");
						cbuf_start[now_depth] = dlist_start;
						cbuf_curpos[now_depth] = dlist_curpos;
						dlist_start  = 0;
						dlist_curpos = 0;
						now_depth++;
						return 1;
					}
				} 
				else
				{
					for(i=0;i<MAXPATH;i++) if (dlist[dlist_curpos].d_name[i]==0) break;
					if (i>3)
					{
						if (((dlist[dlist_curpos].d_name[i-4]) == '.') &&
						    ((dlist[dlist_curpos].d_name[i-3]) == 'p') &&
						    ((dlist[dlist_curpos].d_name[i-2]) == 'r') &&
						    ((dlist[dlist_curpos].d_name[i-1]) == 'x'))
						{
							if(load_prx) return 7;
						}
						if (((dlist[dlist_curpos].d_name[i-4]) == '.') &&
						    ((dlist[dlist_curpos].d_name[i-3]) == 'P') &&
						    ((dlist[dlist_curpos].d_name[i-2]) == 'R') &&
						    ((dlist[dlist_curpos].d_name[i-1]) == 'X'))
						{
							if(load_prx) return 7;
						}
						i--;
					}
					return 2;
				}
			}
		break;

		case PSP_CTRL_CIRCLE:
			if (now_depth > 0) 
			{
			
				for(i=0;i<MAXPATH;i++) if (now_path[i]==0) break;

				i--;

				while(i>4)
				{
					if (now_path[i-1]=='/')
					{
						now_path[i]=0;
						break;
					}
					i--;
				}

				now_depth--;
				dlist_start  = cbuf_start[now_depth];
				dlist_curpos = cbuf_curpos[now_depth];
				return 1;
			}
			else
			{
				filer_done = 1;
			}
		break;

		case PSP_CTRL_SQUARE:
			if(configure_music | configure_capture) return 6;
		break;
	}
	return 0;
}

void filer_draw()
{
	if(!(filer_up | filer_down))
	{
		setP(16, 25, 225, 240);
		drawRect(0, true, 0);

		memset(path_tmp, 0, MAXPATH);
		strncpy(path_tmp,now_path,21);

		setP0(30, 53);
		drawString(path_tmp, FTHICK | FSHADOW);
	}

	//Display directory listing
	int filer_i = dlist_start;
	while (filer_i<(dlist_start+12)) 
	{
		if (filer_i<dlist_num) 
		{
			if (filer_i==dlist_curpos) 
			{
				if(!(filer_up | filer_down))
				{
					setP(25, (((filer_i-dlist_start)+5)*13)+2, 25 + 20*10 , ((filer_i-dlist_start)+6)*13+2);
					drawRect(0, false, 0);	
				}
				else if(filer_up)
				{
					setP(25, (((filer_i-dlist_start+1)+5)*13)+2, 25 + 20*10 , ((filer_i-dlist_start+1)+6)*13+2);
					drawRect(0, true, 0);	
					setP(25, (((filer_i-dlist_start)+5)*13)+2, 25 + 20*10 , ((filer_i-dlist_start)+6)*13+2);
					drawRect(0, false, 0);	
					memset(path_tmp, 0, MAXPATH);
					strncpy(path_tmp, dlist[filer_i+1].d_name, 21);
					setP0(35, ((filer_i-dlist_start+1)+6)*13);
					drawString(path_tmp, FSHADOW);
					memset(path_tmp, 0, MAXPATH);
					strncpy(path_tmp, dlist[filer_i].d_name, 21);
					setP0(35, ((filer_i-dlist_start)+6)*13);
					drawString(path_tmp, FSHADOW);
				}
				else if(filer_down)
				{
					setP(25, (((filer_i-dlist_start-1)+5)*13)+2, 25 + 20*10 , ((filer_i-dlist_start-1)+6)*13+2);
					drawRect(0, true, 0);
					setP(25, (((filer_i-dlist_start)+5)*13)+2, 25 + 20*10 , ((filer_i-dlist_start)+6)*13+2);
					drawRect(0, false, 0);	
					memset(path_tmp, 0, MAXPATH);
					strncpy(path_tmp, dlist[filer_i-1].d_name, 21);
					setP0(35, ((filer_i-dlist_start-1)+6)*13);
					drawString(path_tmp, FSHADOW);	
					memset(path_tmp, 0, MAXPATH);
					strncpy(path_tmp, dlist[filer_i].d_name, 21);
					setP0(35, ((filer_i-dlist_start)+6)*13);
					drawString(path_tmp, FSHADOW);	
				}
			}
			
			if(!(filer_up | filer_down))
			{
				if(FIO_SO_ISDIR(dlist[filer_i].d_stat.st_attr) || FIO_S_ISDIR(dlist[filer_i].d_stat.st_mode)) 
				{
					//if(!flashing_font) oslIntraFontSetStyle(ltn13, 0.5f,LITEGRAY,BLACK,INTRAFONT_ALIGN_LEFT);		
				}

				memset(path_tmp, 0, MAXPATH);
				strncpy(path_tmp, dlist[filer_i].d_name, 21);
				setP0(35, ((filer_i-dlist_start)+6)*13);
				drawString(path_tmp, FSHADOW);
			}
		}
		filer_i++;
	}
	if((filer_up | filer_down)) {filer_up = 0; filer_down = 0; }
}

int filer_on() 
{

	if(configure_music) gui_print("Tips : Press Square To Select");
		else if(configure_capture) gui_print("Tips : Press Square To Select");
		else load_prx = 1;

	strncpy(now_path,"ms0:/",MAXPATH);

	Get_DirList(now_path);
	dlist_start  = 0;
	dlist_curpos = 0;
	now_depth    = 0;

	setP(15, 25, 480, 249);
	clearRect(0);
	setP(16, 25, 250, 240);
	drawRect(15, true, 0x0A0000FF);

	filer_done = 0;
	filer_draw();

	while(!filer_done)
	{
		switch(filer_Keys()) 
		{

			case 1:
				Get_DirList(now_path);
				filer_draw();
				break;

			case 3:
				filer_up = 1;
				filer_draw();
				break;

			case 4:
				filer_down = 1;
				filer_draw();
				break;

			case 5:
				filer_draw();
				break;

			case 6:
				memset(prx_path, 0, 512);
				strcpy(prx_path, now_path);
				strcat(prx_path, dlist[dlist_curpos].d_name);
				if(configure_music) strcpy(config->music_folder, prx_path);
				else if(configure_capture) strcpy(config->capture_folder, prx_path);
				filer_done = 1;

				break;

			case 7:
				memset(prx_path, 0, 512);
				strcpy(prx_path, now_path);
				strcat(prx_path, dlist[dlist_curpos].d_name);

#ifdef GAME
				int ret = loadStartModuleWithArgs(prx_path, 8, 0, NULL);
#else
				int ret = loadStartModuleWithArgs(prx_path, 1, 0, NULL);
#endif
				char tmpStr[256];
				if(ret != 1) { sprintf(tmpStr, "NOK : Module error %x", ret); gui_print(tmpStr); }
					else gui_print("OK : Module Loaded Successfully");
				filer_done = 1;
				break;

		}
	}
	gui_print("");
	//if(dlist_mem) sceKernelFreePartitionMemory(dlist_mem);
	return 0;
}

