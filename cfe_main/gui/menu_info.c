#include <pspkernel.h>
#include <string.h>
#include <stdio.h>
#include <psppower.h>
#include <pspdisplay_kernel.h>

#include "guicommon.h"
#include "graphic.h"
#include "ctrl.h"

void menu_info_update()
{
	SceSize free;

	PspSysmemPartitionInfo info;
	char tmp[256];

	setP(25, 25, 450, 239);
	drawRect(0, true, 0);

	setP0(25, 40);
	drawString("Refresh", FTHICK | FSHADOW);

	// Kernel Memory Information
	memset(&info, 0, sizeof(info));
	info.size = sizeof(info);
	if(sceKernelQueryMemoryPartitionInfo(1, &info) == 0)
	{
		free = sceKernelPartitionMaxFreeMemSize(1);
		memset(tmp, 0, 256);
		sprintf(tmp, "Kernel memory :     %9d / %8d bytes", free, info.memsize);
	} else sprintf(tmp, "Kernel memory :     NA");
	setP0(25, 80);
	drawString(tmp, FSHADOW);

	// User Memory Information
	memset(&info, 0, sizeof(info));
	info.size = sizeof(info);
	if(sceKernelQueryMemoryPartitionInfo(2, &info) == 0)
	{
		free = sceKernelPartitionMaxFreeMemSize(2);
		memset(tmp, 0, 256);
		sprintf(tmp, "User memory :       %9d / %8d bytes", free, info.memsize);
	} else sprintf(tmp, "User memory :       NA");
	setP0(25, 90);
	drawString(tmp, FSHADOW);

	// Slim Memory Information
	memset(&info, 0, sizeof(info));
	info.size = sizeof(info);
	if(sceKernelQueryMemoryPartitionInfo(8, &info) == 0)
	{
		free = sceKernelPartitionMaxFreeMemSize(8);
		memset(tmp, 0, 256);
		sprintf(tmp, "Slim memory :       %9d / %8d bytes", free, info.memsize);
	} else sprintf(tmp, "Slim memory :       NA");
	setP0(25, 100);
	drawString(tmp, FSHADOW);

	setP0(25, 140);
	gprintf("External Power :    %s\n", scePowerIsPowerOnline()? "yes" : "no ");
	setP0(25, 150);
	gprintf("Battery :           %s\n", scePowerIsBatteryExist()? "present" : "absent ");

	int batteryLifeTime = 0;

	if (scePowerIsBatteryExist())
	{
		setP0(25, 160);
		gprintf("Low Charge :        %s\n", scePowerIsLowBattery()? "yes" : "no ");
		setP0(25, 170);
		gprintf("Charging :          %s\n", scePowerIsBatteryCharging()? "yes" : "no ");
		batteryLifeTime = scePowerGetBatteryLifeTime();
		setP0(25, 180);
		gprintf("Charge :            %d%% (%02dh%02dm)     \n",
			scePowerGetBatteryLifePercent(), batteryLifeTime/60, batteryLifeTime-(batteryLifeTime/60*60));
		setP0(25, 190);
		gprintf("Battery Temp :      %d deg C\n", scePowerGetBatteryTemp());
	}
	else
	{
		setP0(25, 160);
		gprintf("Battery stats unavailable\n");
	}
}

void menu_info()
{
	all_done = 0;
	menu_info_done = 0;

	draw_menu_section();

	setP(15, 25, 480, 249);
	clearRect(0);
	setP(16, 25, 459, 240);
	drawRect(15, true, 0x0A0000FF);

	menu_info_update();

	while(!menu_info_done)
	{
		sceDisplaySetBrightness(brightness_level, 0);

		u32 key = ctrlWaitMask(PSP_CTRL_LTRIGGER | PSP_CTRL_RTRIGGER | PSP_CTRL_LTRIGGER | PSP_CTRL_CROSS | PSP_CTRL_CIRCLE);
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
				menu_info_done = 1;
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
					menu_info_done = 1;
			break;

			case PSP_CTRL_CROSS:
				menu_info_update();
			break;

			case  PSP_CTRL_CIRCLE:
				all_done = 1;
				menu_info_done = 1;
			break;
		}
	}
	if(!all_done)
	{
	//	draw_menu_section();
		change_menu_section();
	}
}

