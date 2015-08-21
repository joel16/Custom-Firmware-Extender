#include <pspkernel.h>
#include <string.h>
#include <stdio.h>
#include <pspusb.h>

#include "guicommon.h"
#include "graphic.h"

void menu_main();
#ifndef LIGHT
void menu_music();
#endif
void menu_info();
#ifndef GAME
void menu_tm();
#endif
void menu_cfg();
u32 state;

void gui_print(char *msg)
{
	setP(0, 253, 480, 272);
	drawRect(0, true, 0);
	setP0(5, 265);
	drawString(msg, FSHADOW);

	setP0(400, 265);
	if(state & PSP_USB_ACTIVATED) drawString("USB: ON", FSHADOW);
		else drawString("USB: OFF", FSHADOW);
}

void change_menu_section()
{
	if(menu_section == MENU_SECTION_MAIN)
	{
		menu_main();
	}
#ifndef LIGHT
	else if(menu_section == MENU_SECTION_MUSIC)
	{
		menu_music();
		if(music_prx_load) { all_done = 1; menu_main_done = 1; }
	}
#endif
#ifndef GAME
	else if(menu_section == MENU_SECTION_TM)
	{
		menu_tm();
	}
#endif
	else if(menu_section == MENU_SECTION_INFO)
	{
		menu_info();
	}
	else if(menu_section == MENU_SECTION_CFG)
	{
		menu_cfg();
	}
}

void draw_menu_section()
{
	setP(20, 2, 300, 18);
	drawRect(0, true, 0);

	setP0(25, 12);
	if(menu_section == MENU_SECTION_MAIN) drawString("Main", FTHICK | FSHADOW);
		else drawString("Main", FSHADOW);
#ifndef LIGHT
	setP0(75, 12);
	if(menu_section == MENU_SECTION_MUSIC) drawString("Music", FTHICK | FSHADOW);
		else drawString("Music", FSHADOW);
#endif
#ifdef VSH
	setP0(135, 12);
	if(menu_section == MENU_SECTION_TM) drawString("Tm", FTHICK | FSHADOW);
		else drawString("Tm", FSHADOW);

	setP0(175, 12);
	if(menu_section == MENU_SECTION_INFO) drawString("Infos", FTHICK | FSHADOW);
		else drawString("Infos", FSHADOW);

	setP0(235, 12);
	if(menu_section == MENU_SECTION_CFG) drawString("Config", FTHICK | FSHADOW);
		else drawString("Config", FSHADOW);
#elif GAME
#ifndef LIGHT
	setP0(130, 12);
	if(menu_section == MENU_SECTION_INFO) drawString("Infos", FTHICK | FSHADOW);
		else drawString("Infos", FSHADOW);

	setP0(185, 12);
	if(menu_section == MENU_SECTION_CFG) drawString("Config", FTHICK | FSHADOW);
		else drawString("Config", FSHADOW);
#else
	setP0(75, 12);
	if(menu_section == MENU_SECTION_INFO) drawString("Infos", FTHICK | FSHADOW);
		else drawString("Infos", FSHADOW);

	setP0(135, 12);
	if(menu_section == MENU_SECTION_CFG) drawString("Config", FTHICK | FSHADOW);
		else drawString("Config", FSHADOW);
#endif
#endif
}

