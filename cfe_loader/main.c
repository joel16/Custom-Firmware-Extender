/*
 * PSP Software Development Kit - http://www.pspdev.org
 * -----------------------------------------------------------------------
 * Licensed under the BSD license, see LICENSE in PSPSDK root for details.
 *
 * main.c - Basic Kernel PRX template
 *
 * Copyright (c) 2005 Marcus R. Brown <mrbrown@ocgnet.org>
 * Copyright (c) 2005 James Forshaw <tyranid@gmail.com>
 * Copyright (c) 2005 John Kelley <ps2dev@kelley.ca>
 *
 * $Id$
 * $HeadURL$
 */
#include <pspkernel.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pspctrl.h>
#include "pspinit.h"
#include "pspsysmem_kernel.h"

PSP_MODULE_INFO("cfe_loader", PSP_MODULE_KERNEL, 1, 1);

#define MAX_ARGS 512

SceUID loadStartModuleWithArgs(const char *filename, int mpid, int argc, char * const argv[])
{
	SceKernelLMOption option;
	SceUID modid = 0;
	int retVal = 0, mresult;
	char args[MAX_ARGS];
	int  argpos = 0;
	int  i;

	memset(args, 0, MAX_ARGS);
	strcpy(args, filename);
	argpos += strlen(args) + 1;
	for(i = 0; (i < argc) && (argpos < MAX_ARGS); i++)
	{
		int len;

		snprintf(&args[argpos], MAX_ARGS-argpos, "%s", argv[i]);
		len = strlen(&args[argpos]);
		argpos += len + 1;
	}

	memset(&option, 0, sizeof(option));
	option.size = sizeof(option);
	option.mpidtext = mpid;
	option.mpiddata = mpid;
	option.position = 0;
	option.access = 1;

	retVal = sceKernelLoadModule(filename, 0, &option);
	if(retVal < 0){
		return retVal;
	}

	modid = retVal;

	retVal = sceKernelStartModule(modid, argpos, args, &mresult, NULL);
	if(retVal < 0){
		return retVal;
	}

	return modid;
}

int main_thread(SceSize args, void *argp)
{
	int exit = 0, init_key = 0, model = 0, light = 0;
	SceCtrlData pad;
	sceCtrlPeekBufferPositive(&pad, 1);

	if(pad.Buttons & PSP_CTRL_LTRIGGER)
	{
		exit = 1;
	}
	else if(pad.Buttons & PSP_CTRL_RTRIGGER)
	{
		light = 1;
	}

	if(!exit)
	{
		init_key = sceKernelInitKeyConfig();

		if(init_key == PSP_INIT_KEYCONFIG_VSH) // VSH MODE SPOTTED
		{
			loadStartModuleWithArgs("ms0:/seplugins/cfe/cfe_vsh.prx", 1, 0, NULL);
		}
		else
		{
			model = sceKernelGetModel();

			if(model == PSP_MODEL_SLIM_AND_LITE) // PSP SLIM SPOTTED
			{
				if(sceKernelInitApitype() == PSP_INIT_APITYPE_DISC) 
				{
					if(!light) loadStartModuleWithArgs("ms0:/seplugins/cfe/cfe_game.prx", 8, 0, NULL);
						else loadStartModuleWithArgs("ms0:/seplugins/cfe/cfe_light.prx", 1, 0, NULL);
				}
				else
				{
					loadStartModuleWithArgs("ms0:/seplugins/cfe/cfe_light.prx", 1, 0, NULL);
				}
			}
			else // PSP FAT SPOTTED
			{
				loadStartModuleWithArgs("ms0:/seplugins/cfe/cfe_light.prx", 1, 0, NULL);
			}
		}
	}

	sceKernelStopUnloadSelfModule(0, NULL, NULL, NULL);
	return 0;
}

/* Entry point */
int module_start(SceSize args, void *argp)
{
	int thid;

	thid = sceKernelCreateThread("cfe_loaderth", main_thread, 7, 0x800, 0, NULL);
	if(thid >= 0)
	{
		sceKernelStartThread(thid, args, argp);
	}
	return 0;
}

/* Module stop entry */
int module_stop(SceSize args, void *argp)
{
	return 0;
}
