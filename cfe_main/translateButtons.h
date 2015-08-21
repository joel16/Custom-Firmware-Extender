#ifndef __TRANSLATEBUTTONS_H__
#define __TRANSLATEBUTTONS_H__

/*
u32 comboButton_i, menuButton_i, screenshotButton_i, cpuPlusButton_i, cpuMinusButton_i,\
	brightnessPlusButton_i, brightnessMinusButton_i, musicButton_i;
*/

SceUID button_memid;

typedef struct
{
	u32 combo;
	u32 menu;
	u32 screenshot;
	u32 cpuPlus;
	u32 cpuMinus;
	u32 brightnessPlus;
	u32 brightnessMinus;
	u32 music;

} BUTTONCONFIG;

BUTTONCONFIG *button;

int translateButtons();

#endif 

