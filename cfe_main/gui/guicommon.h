#ifndef __GUICOMMON_H__
#define __GUICOMMON_H__

#define MENU_MUSIC_DIR 0
#define MENU_MUSIC_START 1

#ifdef VSH
#define MENU_SECTION_MAIN 0
#define MENU_SECTION_MUSIC 1
#define MENU_SECTION_TM 2
#define MENU_SECTION_INFO 3
#define MENU_SECTION_CFG 4
#elif GAME
#ifndef LIGHT
#define MENU_SECTION_MAIN 0
#define MENU_SECTION_MUSIC 1
#define MENU_SECTION_INFO 2
#define MENU_SECTION_CFG 3
#else
#define MENU_SECTION_MAIN 0
#define MENU_SECTION_INFO 1
#define MENU_SECTION_CFG 2
#endif
#endif

int menu_info_done, menu_music_done, screenshot_done, do_reboot, do_poweroff, do_music, menu_count;

int music_prx_active, menu_item, brightness_level, music_prx_load, music_active;

int all_done, menu_section, menu_main_done, menu_cfg_done;

char menu_str_cpu[64];
char menu_str_brightness[64];

void change_menu_section();
void draw_menu_section();
void gui_print(char *msg);

char display_msg[128];
int display_show_message_one_time;

#endif
