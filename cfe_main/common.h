#ifndef __COMMON_H__
#define __COMMON_H__

char music_dir[512];
int menu_music_in_game_active;
int usbhost_active;

int start_launcher;
int cpuSpeed;

int mute_channel[8];

SceUID heapid;

int audio_is_patched;
void patch_audio();
int remotejoy_active;
int load_cfe_redirect, cfe_redirect_loaded;
int stop_redirect;
int load_prx;
int configure_music, configure_capture;

extern void redirect(int mode);

#endif
