/*------------------------------------------------------
	Project:		PSP-CFW*
	Module:			CFW_CTRL
	Maintainer:		
------------------------------------------------------*/

#include "ctrl.h"
#include <psppower.h>

#define CTRL_REPEAT_TIME 0x40000
#define CTRL_REPEAT_INTERVAL 0x12000

static u32 last_btn = 0;
static u32 last_tick = 0;
static u32 repeat_flag = 0;

static u32 ctrlRead()
{
	SceCtrlData ctl;
	sceCtrlPeekBufferPositive(&ctl, 1);

	if (ctl.Buttons == last_btn)
	{
		if (ctl.TimeStamp - last_tick < (repeat_flag ? CTRL_REPEAT_INTERVAL : CTRL_REPEAT_TIME)) return 0;
		repeat_flag = 1;
		last_tick = ctl.TimeStamp;
		return last_btn;
	}
	repeat_flag = 0;
	last_tick = ctl.TimeStamp;
	last_btn  = ctl.Buttons;
	return last_btn;
}

extern u32 ctrlWaitMask(u32 keymask)
{
	u32 key;
	while((key = (ctrlRead() & keymask)) == 0)
	{
		//scePowerTick(0);
		sceKernelDelayThread(20000);
	}
	return key;
}
