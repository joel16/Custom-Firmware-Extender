#include <pspkernel.h>
#include <pspiofilemgr.h>
#include <pspaudio.h>
#include <pspmodulemgr.h>
#include <psploadcore.h>
#include <psppower.h>
#include <pspctrl.h>
#include <pspmscm.h>
#include <pspdisplay.h>
#include <systemctrl.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <time.h>

#include "playback.h"
#include "music.h"
#include "init.h"
#include "type.h"
#include "graphic.h"
#include "guicommon.h"
#include "common.h"
#include "ctrl.h"
#include "utils.h"

//#include "conf.h"

#include "hw.h"
#include "hw_aa3.h"
#include "hw_at3.h"
#include "hw_mp3.h"

char music_root[64];
int game=0;

int do_music_action = 0;

int in_game_mute = 0;
SceUID main_thid;
//MusicConf config;
//for on screen display
char str_buf[MAXPATH+1];
int blit_mode_timer = 0;
int blit_ready_timer = DISPLAY_TIMER_AMT;
int blit_debug_timer = 0;
int hw_init = 0;

SceUID display_thid;

//Playlist kmusic;
Playlist *music;

int menu_music_done=0;
int showMusicMenu=0;
SceCtrlData pad;
unsigned int pad_old;

SceUID memid, musicmem;

SceUID header_memid;
void draw_menu_music_in_game_song();

char file_playing[256];

void TerminatePlaylist()
{
    sceKernelTerminateDeleteThread(music->pl_thid); 
    memset(music->file, 0, sizeof(music->file));
    sceKernelFreePartitionMemory(music->memid);
}

int module_reboot_before()
{
//	sceKernelTerminateDeleteThread(main_thid);
	sceKernelTerminateDeleteThread(music->mus_thid);

	TerminatePlaylist();

	sceKernelFreePartitionMemory(memid);
	sceKernelFreePartitionMemory(data_memid);
	sceKernelFreePartitionMemory(header_memid);
	sceKernelFreePartitionMemory(musicmem);

	return 0;
}


//returns number of music files in MUSIC_DIR and all subdirs 
int CountMusicFiles(char* dir)
{
	int fd;
	int music_cnt = 0;
	SceIoDirent d_dir;
	char tmp[MAXPATH];
	char ext[5];

	memset(&d_dir,0,sizeof(SceIoDirent));//prevents a crash
	fd = sceIoDopen(dir);

	if(fd >= 0)
	{
		while((sceIoDread(fd, &d_dir) > 0))
		{
			if(FIO_S_ISDIR(d_dir.d_stat.st_mode) == 0) //not a directory
			{
                		memcpy(ext, d_dir.d_name + strlen(d_dir.d_name) - 4, 5);

                		if(!stricmp(ext, ".mp3") || !stricmp(ext, ".at3")
                 		||!stricmp(ext, ".aa3") || !stricmp(ext, ".oma")
                 		||!stricmp(ext, ".omg") ) //if it is an music file
				{
                    			music_cnt++;
					printf("%i -> %s\n", music_cnt, d_dir.d_name);
				}
			} 
			else if(FIO_S_ISDIR(d_dir.d_stat.st_mode)) //handle sub dirs
			{
				if (d_dir.d_name[0] != '.')
				{
					sprintf(tmp, "%s/%s", dir, d_dir.d_name);
					printf("subdir : %s\n",  tmp);
					music_cnt += CountMusicFiles(tmp);
				}
			}
		}
		sceIoDclose(fd);
	}
	return music_cnt;
}

//gets the name of an music file as a offset from the first music file in a directory and all subdirs
//filename is 262 (MAXPATH) bytes
int GetMusicFileName(char* dir,char* filename,int num,int basenum)
{
	int fd;
	int music_cnt;
	int music_dir_cnt;
	SceIoDirent d_dir;
	char tmp[MAXPATH];
	char ext[5];

	music_cnt = basenum;

	memset(&d_dir,0,sizeof(SceIoDirent));//prevents a crash
	fd = sceIoDopen(dir);

	if(fd >= 0)
	{
		while((sceIoDread(fd, &d_dir) > 0))
		{
			//if((d_dir.d_stat.st_attr & FIO_SO_IFDIR) == 0)//not a directory
			if(FIO_S_ISDIR(d_dir.d_stat.st_mode) == 0)
			{
				memcpy(ext, d_dir.d_name + strlen(d_dir.d_name) - 4, 5);
				if(!stricmp(ext, ".mp3") || !stricmp(ext, ".at3")
				 ||!stricmp(ext, ".aa3") || !stricmp(ext, ".oma")
				 ||!stricmp(ext, ".omg") ) //if it is an music file
				{
					if(music_cnt == num)
					{
                        			sprintf(filename, "%s/%s", dir, d_dir.d_name);
						printf("file found : %s/%s\n\n", dir, d_dir.d_name);
						memset(file_playing, 0, 256);
						strcpy(file_playing, d_dir.d_name);
                        			music_cnt = -1;
                        			break;
                   			}
                   			 music_cnt++;
                		}
			} 
			//else if(d_dir.d_stat.st_attr & FIO_SO_IFDIR)//handle sub dirs
			else if(FIO_S_ISDIR(d_dir.d_stat.st_mode))
			{
				sprintf(tmp, "%s/%s", dir, d_dir.d_name);
				if (d_dir.d_name[0] != '.') 
				{
					music_dir_cnt = GetMusicFileName(tmp, filename, num, music_cnt);
					if(music_dir_cnt == -1) //found file
 					break;
					music_cnt = music_dir_cnt;
				}
			}
		}

		sceIoDclose(fd);
	}
 	return music_cnt;
}

void InitPlaylist()
{
	printf("Start => InitPlaylist();\n\n");

	printf("Start => CountMusicFiles();\n\n");
    music->omg_count = CountMusicFiles(OMG_AUDIO_DIR);
    music->count = CountMusicFiles(music_root) + music->omg_count;
	printf("Done => CountMusicFiles();\n\n");
	
#ifdef GAME
    music->memid = sceKernelAllocPartitionMemory(8, "PLYLST_MEM", PSP_SMEM_Low, music->count*sizeof(int), NULL);
#else
    music->memid = sceKernelAllocPartitionMemory(2, "PLYLST_MEM", PSP_SMEM_Low, music->count*sizeof(int), NULL);
#endif
    music->random_played = sceKernelGetBlockHeadAddr(music->memid);

    music->offset = 0;
	music->resume = 0;

    music->pause = PLAYBACK_PAUSE_SONG;
	music->change_track = 0;

    music->loop = LOOP_NONE;

	printf("Start => ResetPlaylist();\n");
    ResetPlaylist();
	printf("Done => ResetPlaylist();\n");
    //create playlist thread

#ifdef GAME
	SceKernelThreadOptParam option;
	memset(&option, 0, sizeof(option));
	option.size = sizeof(option);

	option.stackMpid = 8;
	music->pl_thid = sceKernelCreateThread("playlist_thread", playlist_thread, THREAD_PRIORITY+2, 0x4000, 0, &option);
#else
	music->pl_thid = sceKernelCreateThread("playlist_thread", playlist_thread, THREAD_PRIORITY+2, 0x4000, 0, NULL);
#endif
   	
   	if(music->pl_thid >= 0)
   		sceKernelStartThread(music->pl_thid, 0, NULL);
	printf("Done => ResetPlaylist();\n");
}


void ResetPlaylist()
{
	int i;
	music->index = 0;
	music->random_index = 0;

	//music->random = 0;

	for(i = 0; i < music->count; i++)
		music->random_played[i] = -1; //played file not = to -1
}

int playlist_thread(SceSize args, void *argp)
{
	int i;
    char ext[5];
    char file_type;
    int ms_in;
    SceKernelThreadEntry music_thread = NULL;

    srand(time(NULL));

    while(1)
    {
        if (music->count)
        {
            if (music->random && (music->random_index >= music->count))
                ResetPlaylist(); 

            if ((!music->resume) && (music->loop != LOOP_SONG)) //not resuming music playback, load a new file
            {
                 //random file from dir
                if (music->random)
                {
                    music->index = rand()%music->count;

                    //check to see if this file was played before
                    for(i = music->random_index; i>=0; i--)
                    {
                        if (music->index == music->random_played[i])//if rand already played, pick a new one and recheck
                        {
                            music->index = rand()%music->count;
                            i = music->random_index;
                        }
                    }

                    music->random_played[music->random_index] = music->index;
                    music->random_index++;

                    if (music->index >= music->omg_count)
                        GetMusicFileName(music_root, music->file, music->index - music->omg_count, 0);
                    else
                        GetMusicFileName(OMG_AUDIO_DIR, music->file, music->index, 0);
                }
                else
                {
                    if (music->index >= music->omg_count)
                        GetMusicFileName(music_root, music->file, music->index - music->omg_count, 0);
                    else
                        GetMusicFileName(OMG_AUDIO_DIR, music->file, music->index, 0);

                    music->index++;

                    if (music->index >= music->count)
                       ResetPlaylist();
                }//if (music->random)
            }//if (!music->resume && music->loop == LOOP_NONE)

            music->title[0] = '\0'; //reset so the right title/filename is displayed

            if (music->loop == LOOP_IGNORE) //finished changing the track, reset the loop
                music->loop = LOOP_SONG;

        }//if (music->count)
        else
        {   /* if no files are in the playlist, sleep, 
            since the only way this can be fixed is by reloading */
            sceKernelSleepThreadCB();
        }

        memcpy(ext, music->file + strlen(music->file) - 4, 5);//get file extension

        //get the id3 title so the osd will have a real name to display 
        if (!stricmp(ext, ".aa3") || !stricmp(ext, ".oma") || !stricmp(ext, ".omg"))
            GetOMGTitle(music->file, music->title);
        //delay init'ing the hw decoder until play is pressed
        if (hw_init == 0) 
        {                     
            if (music->pause == PLAYBACK_PAUSE_SONG)
                music->pause = PLAYBACK_PAUSED;

            while( music->pause == PLAYBACK_PAUSED)
            {   //we must handle next/prev here too
                if ((music->change_track == PREV_SONG) || (music->change_track == NEXT_SONG)) 
                {
                    if (music->change_track == PREV_SONG)
                    { 
                        music->index -= 2;

                        if (music->index < 0)
                            music->index += music->count; //allows moving back in a loop
                    }

                    break;
                }
                sceKernelDelayThreadCB(DELAY_THREAD_AMT);
            }

            if ( music->change_track != 0)
            {
                music->change_track = 0;
                music->offset = 0;
                music->resume = 0;

                if (music->loop == LOOP_SONG)
                    music->loop = LOOP_IGNORE;

                continue;
            }
            hw_init = 1;
            HWInit();
        }
        
        //check to see if the music library is already loaded (keeps games from locking up)
        if ( FindProc("sceAvcodec_wrapper","sceAudiocodec",0x5B37EB1D)
          || FindProc("sceAudiocodec_Driver","sceAudiocodec",0x5B37EB1D) )
            music->init = 1;

        if (!stricmp(ext, ".mp3")) //if it is an mp3 file
            music_thread = HW_MP3PlayFile;
        else if (!stricmp(ext, ".at3")) //at3/at3+ file
            music_thread = HW_AT3PlayFile;
        else if (!stricmp(ext, ".aa3") || !stricmp(ext, ".oma") || !stricmp(ext, ".omg") )
        {
            file_type = GetOMGFileType(music->file);
            if (file_type == TYPE_AT3)
              music_thread = HW_AA3PlayFile;
            else if (file_type == TYPE_MP3)
                music_thread = HW_MP3PlayFile;
            else //invalid format
            {
                WaitMSReady();//wait until system is ready, this can happen due to suspend
                continue;
            }
        }
        music->flags = PLAYBACK_PLAYING;

#ifdef GAME
	SceKernelThreadOptParam option;
	memset(&option, 0, sizeof(option));
	option.size = sizeof(option);

	option.stackMpid = 8;

	music->mus_thid = sceKernelCreateThread("music_thread", music_thread, THREAD_PRIORITY, 0x4000, 0, &option);
#else
	music->mus_thid = sceKernelCreateThread("music_thread", music_thread, THREAD_PRIORITY, 0x4000, 0, NULL);
#endif

       
        if (music->mus_thid >= 0)
	{
		printf("Starting thread\n");
		sceKernelStartThread(music->mus_thid, 0, NULL);
	}
	else printf("error : %x\n", music->mus_thid);
	
	draw_menu_music_in_game_song();

        //handle next, previous, pause resume
        while(1)//playing
        {
            if (music->pause == PLAYBACK_PAUSE_SONG)
            {
                sceKernelSuspendThread(music->mus_thid);
                music->pause = PLAYBACK_PAUSED;
            }
            else if (music->pause == PLAYBACK_RESUME_SONG)
            {
                sceKernelResumeThread(music->mus_thid);
                music->pause = PLAYBACK_PLAYING;
            }

            if ((music->change_track == PREV_SONG) || (music->change_track == NEXT_SONG)) 
            {
                if (music->change_track == PREV_SONG)
                { 
                    music->index -= 2;

                    if (music->index < 0)
                        music->index += music->count; //allows moving back in a loop
                }

                music->change_track = 0;
                music->offset = 0;
                music->resume = 0;

                if (music->loop == LOOP_SONG)
                    music->loop = LOOP_IGNORE;

                music->flags = PLAYBACK_DONE;//signal to the music thread that we are exiting

                if (music->pause == PLAYBACK_PAUSED)//resume thread if paused and then repause it 
                {
                    sceKernelResumeThread(music->mus_thid);
                    music->pause = PLAYBACK_PAUSE_SONG;
                }

                while(music->flags != PLAYBACK_CLEANED_UP)
                    sceKernelDelayThreadCB(DELAY_THREAD_AMT);//let it clean up

                sceKernelTerminateDeleteThread(music->mus_thid);
                break;
            }

            if (music->flags == PLAYBACK_CLEANED_UP)
            {
                sceKernelTerminateDeleteThread(music->mus_thid);
                do //check to see if the ms is ready (for suspend/resume)
                {
                    ms_in = MScmIsMediumInserted();
                    sceKernelDelayThreadCB(DELAY_THREAD_AMT);
                } while (ms_in <= 0);
                break;
            }

            sceKernelDelayThreadCB(DELAY_THREAD_AMT*5);
        } //while (1) //playing

    } //while (1) 

    return 0;
}

#define MENU_PLAY 0
#define MENU_PREVIOUS 1
#define MENU_NEXT 2
#define MENU_RAND 3
#define MENU_STOP 4

int menu_channel_items;

int menu_item;
int music_is_paused;

char volume_char[3];

void draw_menu_music_in_game_song()
{
	if(!menu_music_done)
	{
		setP(0, 253, 480, 272);
		drawRect(0, true, 0);
		setP0(5, 265);
		drawString(file_playing, FSHADOW);
	}
	else
	{
		memset(display_msg, 0, 128);
		strcpy(display_msg, file_playing);

		display_show_message_one_time = 1;
	}
}

void menu_draw()
{
	setP(10, 0, 469, 16);
	drawRect(0, true, 0);

	if (music_is_paused)
	{
		if(menu_channel_items == 0)
		{
			setP0(15, 12);
			if(menu_item == MENU_PLAY) drawString("Play", FTHICK | FSHADOW);
				else drawString("Play", FSHADOW); 
		}
		else { setP0(15, 12); drawString("Play", FSHADOW); }
	}
	else
	{
		if(menu_channel_items == 0)
		{
			setP0(15, 12);
			if(menu_item == MENU_PLAY) drawString("Pause", FTHICK | FSHADOW);
				else drawString("Pause", FSHADOW); 
		}
		else { setP0(15, 12); drawString("Pause", FSHADOW); }
	}

	setP0(65, 12);
	if(menu_item == MENU_PREVIOUS)
	{
		if(menu_channel_items == 0) drawString("Prev", FTHICK | FSHADOW);
			else drawString("Prev", FSHADOW);
	}
	else drawString("Prev", FSHADOW);

	setP0(110, 12);
	if(menu_item == MENU_NEXT)
	{
		if(menu_channel_items == 0) drawString("Next", FTHICK | FSHADOW);
			else drawString("Next", FSHADOW);
	}
	else drawString("Next", FSHADOW);

	if(music->random)
	{
		setP0(155, 12);
		if(menu_item == MENU_RAND)
		{
			if(menu_channel_items == 0) drawString("Random (Y)", FTHICK | FSHADOW);
				else drawString("Random (Y)", FSHADOW);
		}
		else drawString("Random (Y)", FSHADOW);
	}
	else
	{
		setP0(155, 12);
		if(menu_item == MENU_RAND)
		{
			if(menu_channel_items == 0) drawString("Random (N)", FTHICK | FSHADOW);
				else drawString("Random (N)", FSHADOW);
		}
		else drawString("Random (N)", FSHADOW);
	}

	setP0(255, 12);
	if(menu_item == MENU_STOP)
	{
		if(menu_channel_items == 0) drawString("Stop", FTHICK | FSHADOW);
			else drawString("Stop", FSHADOW);
	}
	else drawString("Stop", FSHADOW);
}

void menu_channel_draw()
{
	if(menu_channel_items == 1)
	{
		setP(0, 70, 65, 85);
		drawRect(0, true, 0);
		setP0(5, 82);
		drawString("1:", FTHICK | FSHADOW);
		setP0(30, 82);
		gprintf("%i", channel_volume[menu_channel_items-1]);
	}
	else
	{
		setP(0, 70, 30, 85);
		drawRect(0, true, 0);
		setP0(5, 82);
		drawString("1:", FSHADOW);
	}

	if(menu_channel_items == 2)
	{
		setP(0, 90, 65, 105);
		drawRect(0, true, 0);
		setP0(5, 102);
		drawString("2:", FTHICK | FSHADOW);
		setP0(30, 102);
		gprintf("%i", channel_volume[menu_channel_items-1]);
	}
	else
	{
		setP(0, 90, 30, 105);
		drawRect(0, true, 0);
		setP0(5, 102);
		drawString("2:", FSHADOW);
	}

	if(menu_channel_items == 3)
	{
		setP(0, 110, 65, 125);
		drawRect(0, true, 0);
		setP0(5, 122);
		drawString("3:", FTHICK | FSHADOW);
		setP0(30, 122);
		gprintf("%i", channel_volume[menu_channel_items-1]);
	}
	else
	{
		setP(0, 110, 30, 125);
		drawRect(0, true, 0);
		setP0(5, 122);
		drawString("3:", FSHADOW);	
	}

	if(menu_channel_items == 4)
	{
		setP(0, 130, 65, 145);
		drawRect(0, true, 0);
		setP0(5, 142);
		drawString("4:", FTHICK | FSHADOW);
		setP0(30, 142);
		gprintf("%i", channel_volume[menu_channel_items-1]);
	}
	else
	{
		setP(0, 130, 30, 145);
		drawRect(0, true, 0);
		setP0(5, 142);
		drawString("4:",  FSHADOW);
	}

	if(menu_channel_items == 5)
	{
		setP(0, 150, 65, 165);
		drawRect(0, true, 0);
		setP0(5, 162);
		drawString("5:", FTHICK | FSHADOW);
		setP0(30, 162);
		gprintf("%i", channel_volume[menu_channel_items-1]);
	}
	else
	{
		setP(0, 150, 30, 165);
		drawRect(0, true, 0);
		setP0(5, 162);
		drawString("5:", FSHADOW);	
	}

	if(menu_channel_items == 6)
	{
		setP(0, 170, 65, 185);
		drawRect(0, true, 0);
		setP0(5, 182);
		drawString("6:", FTHICK | FSHADOW);
		setP0(30, 182);
		gprintf("%i", channel_volume[menu_channel_items-1]);
	}
	else
	{
		setP(0, 170, 30, 185);
		drawRect(0, true, 0);
		setP0(5, 182);
		drawString("6:", FSHADOW);		
	}

	if(menu_channel_items == 7)
	{
		setP(0, 190, 65, 205);
		drawRect(0, true, 0);
		setP0(5, 202);
		drawString("7:", FTHICK | FSHADOW);
		setP0(30, 202);
		gprintf("%i", channel_volume[menu_channel_items-1]);
	}
	else
	{
		setP(0, 190, 30, 205);
		drawRect(0, true, 0);
		setP0(5, 202);
		drawString("7:", FSHADOW);		
	}

	if(menu_channel_items == 8)
	{
		setP(0, 210, 65, 225);
		drawRect(0, true, 0);
		setP0(5, 222);
		drawString("8:", FTHICK | FSHADOW);
		setP0(30, 222);
		gprintf("%i", channel_volume[menu_channel_items-1]);
	}
	else
	{
		setP(0, 210, 30, 225);
		drawRect(0, true, 0);
		setP0(5, 222);
		drawString("8:", FSHADOW);
	}
}

void menu_music_in_game() 
{
	menu_music_done = 0;
	menu_item = 0;
	menu_channel_items = 0;
	int do_menu_draw = 0;

	setP(5, 0, 474, 19);
	drawRect(15, true, 0x0A0000FF);
	setP0(15, 12);
	if(music_is_paused) drawString("Play", FTHICK | FSHADOW);
		else drawString("Pause", FTHICK | FSHADOW);
	setP0(65, 12);
	drawString("Prev", FSHADOW);
	setP0(110, 12);
	drawString("Next", FSHADOW);

	setP0(155, 12);
	drawString("Random (N)", FSHADOW);

	setP0(255, 12);
	drawString("Stop", FSHADOW);

	setP(0, 50, 150, 65);
	drawRect(0, true, 0x0A0000FF);
	setP0(10, 62);
	drawString("Audio Channel :", FTHICK | FSHADOW);

	setP(0, 70, 65, 85);
	drawRect(0, true, 0x0A0000FF);
	setP0(5, 82);
	drawString("1:", FSHADOW);
	setP0(30, 82);
	gprintf("%i", channel_volume[0]);

	setP(0, 90, 65, 105);
	drawRect(0, true, 0x0A0000FF);
	setP0(5, 102);
	drawString("2:", FSHADOW);
	setP0(30, 102);
	gprintf("%i", channel_volume[1]);

	setP(0, 110, 65, 125);
	drawRect(0, true, 0x0A0000FF);
	setP0(5, 122);
	drawString("3:", FSHADOW);
	setP0(30, 122);
	gprintf("%i", channel_volume[2]);

	setP(0, 130, 65, 145);
	drawRect(0, true, 0x0A0000FF);
	setP0(5, 142);
	drawString("4:", FSHADOW);
	setP0(30, 142);
	gprintf("%i", channel_volume[3]);

	setP(0, 150, 65, 165);
	drawRect(0, true, 0x0A0000FF);
	setP0(5, 162);
	drawString("5:", FSHADOW);
	setP0(30, 162);
	gprintf("%i", channel_volume[4]);

	setP(0, 170, 65, 185);
	drawRect(0, true, 0x0A0000FF);
	setP0(5, 182);
	drawString("6:", FSHADOW);
	setP0(30, 182);
	gprintf("%i", channel_volume[5]);

	setP(0, 190, 65, 205);
	drawRect(0, true, 0x0A0000FF);
	setP0(5, 202);
	drawString("7:", FSHADOW);
	setP0(30, 202);
	gprintf("%i", channel_volume[6]);

	setP(0, 210, 65, 225);
	drawRect(0, true, 0x0A0000FF);
	setP0(5, 222);
	drawString("8:", FSHADOW);
	setP0(30, 222);
	gprintf("%i", channel_volume[7]);

	draw_menu_music_in_game_song();

	while(!menu_music_done)
	{
		if(do_menu_draw)
		{
			menu_draw();
			do_menu_draw = 0;
		}

		u32 key = ctrlWaitMask(PSP_CTRL_UP | PSP_CTRL_DOWN | PSP_CTRL_LEFT | PSP_CTRL_RIGHT | PSP_CTRL_CROSS | PSP_CTRL_CIRCLE | \
			PSP_CTRL_LTRIGGER | PSP_CTRL_RTRIGGER);

		switch(key)
		{
			case PSP_CTRL_DOWN:
				if(menu_channel_items < 8) menu_channel_items++;
				if(menu_channel_items == 0) menu_draw();
				if(menu_channel_items == 1) menu_draw();
				menu_channel_draw();
			break;

			case PSP_CTRL_UP:
				if(menu_channel_items > 0) menu_channel_items--;
				if(menu_channel_items == 0) menu_draw();
				menu_channel_draw();
			break;

			case PSP_CTRL_RTRIGGER:
				if(menu_channel_items != 0)
				{
					if(channel_volume[menu_channel_items-1] <= 96) channel_volume[menu_channel_items-1]+=4;
					if(!audio_is_patched)
					{
						patch_audio();
						audio_is_patched = 1;
					}

					menu_channel_draw();
				}
				else
				{
					if(menu_item < 4) menu_item++;
						else menu_item=0;
					do_menu_draw = 1;
				}
			break;

			case PSP_CTRL_LTRIGGER:
				if(menu_channel_items != 0)
				{
					if(channel_volume[menu_channel_items-1] >= 4) channel_volume[menu_channel_items-1]-=4;
					if(!audio_is_patched)
					{
						patch_audio();
						audio_is_patched = 1;
					}

					menu_channel_draw();
				}
				else
				{
					if(menu_item > 0) menu_item--;
						else menu_item=4;
					do_menu_draw = 1;
				}
			break;

			case PSP_CTRL_CROSS:
				if(menu_channel_items == 0)
				{
					if(menu_item == MENU_PLAY)
					{
						if (music->pause == PLAYBACK_PAUSED)
						{
							music->pause = PLAYBACK_RESUME_SONG;
							music_is_paused = 0;
						}
						else if (music->pause == PLAYBACK_PLAYING)
						{
							music->pause = PLAYBACK_PAUSE_SONG;	
							music_is_paused = 1;
						}
					}
					else if(menu_item == MENU_PREVIOUS)
					{
						if(!music->random)
						{
							music->change_track = PREV_SONG;
						}
					}

					else if(menu_item == MENU_NEXT)
					{
						music->change_track = NEXT_SONG;
					}

					else if(menu_item == MENU_RAND)
					{
						music->random = !music->random;
						ResetPlaylist();
						music->change_track = NEXT_SONG;
					}
					else if(menu_item == MENU_STOP)
					{
						module_reboot_before();
						music_prx_active = 0;
						menu_music_done=1;
					}

					do_menu_draw = 1;
				}
			break;

			case PSP_CTRL_CIRCLE:
				menu_music_done=1;
			break;
		}
	}
	menu_music_in_game_active = 0;
}

int main_music(char *dir)
{
	printf("WaitMSReady();\n");
	//wait until memory stick is readable
	WaitMSReady();

#ifdef GAME
    	musicmem = sceKernelAllocPartitionMemory(8, "musicmem", PSP_SMEM_Low, sizeof(Playlist), NULL);
#else
    	musicmem = sceKernelAllocPartitionMemory(1, "musicmem", PSP_SMEM_Low, sizeof(Playlist), NULL);
#endif
   	music = (Playlist*)sceKernelGetBlockHeadAddr(musicmem);

	char *path;

	strcpy(music_root, dir);
	printf("music_dir = %s\n", music_root);

	path = strrchr(music_root, '/');
	if(path == NULL)
	{
		strcat(music_root, "/");
	}

	channel_volume[0] = 100;
	channel_volume[1] = 100;
	channel_volume[2] = 100;
	channel_volume[3] = 100;
	channel_volume[4] = 100;
	channel_volume[5] = 100;
	channel_volume[6] = 100;
	channel_volume[7] = 100;
	
	music->mus_thid = -1;
	music->init = 0;

#ifdef GAME
	music->is_vsh = 0;
#else
	music->is_vsh = 1;
#endif
	sceKernelDelayThread(10000);
	music->random = 0;

	//InitPatches();
	InitPlaylist();

	music->volume=100;
	music->random = 0;

	//music_is_paused = 1;
	music->pause = PLAYBACK_RESUME_SONG;
	//music->pause = PLAYBACK_PAUSE_SONG;

	music->random = 0;

	return 0;
}

void WaitMSReady()//wait until ms is ready
{
	int ret;
	ret = MScmIsMediumInserted();
	while(ret <= 0)
	{
		sceKernelDelayThreadCB(DELAY_THREAD_AMT);
		ret = MScmIsMediumInserted();
	}
	sceKernelDelayThreadCB(DELAY_THREAD_SEC);
}

SceUID LoadStartModule(char *modname, int partition)
{
    SceKernelLMOption option;
    SceUID modid;

    memset(&option, 0, sizeof(option));
    option.size = sizeof(option);
    option.mpidtext = partition;
    option.mpiddata = partition;
    option.position = 0;
    option.access = 1;

    modid = sceKernelLoadModule(modname, 0, &option);
    if (modid < 0)
        return modid;

    return sceKernelStartModule(modid, 0, NULL, NULL, NULL);
}

void PowerCallback(int unknown, int powerInfo)
{
    if ( (powerInfo & PSP_POWER_CB_POWER_SWITCH)
       ||(powerInfo & PSP_POWER_CB_SUSPENDING)
       ||(powerInfo & PSP_POWER_CB_STANDBY) )
    {
        music->flags = PLAYBACK_RESET;
    }
}

int GetID3TagSize(char *fname)
{
    SceUID fd;
    char header[10];
    int size = 0;
    fd = sceIoOpen(fname, PSP_O_RDONLY, 0777);
    if (fd < 0)
        return 0;

    sceIoRead(fd, header, sizeof(header));
    sceIoClose(fd);

    if (!strncmp((char*)header, "ea3", 3) || !strncmp((char*)header, "EA3", 3)
      ||!strncmp((char*)header, "ID3", 3))
    {
        //get the real size from the syncsafe int
        size = header[6];
        size = (size<<7) | header[7];
        size = (size<<7) | header[8];
        size = (size<<7) | header[9];

        size += 10;

        if (header[5] & 0x10) //has footer
            size += 10;
         return size;
    }
    return 0;
}

// uses ID3(ea3) TIT2 to get the song's name (this is only for sonicstage copied
// audio, so we don't display 1XXXXXXX.OMA as the name)
void GetOMGTitle(char *fname, char *title)
{
   
    unsigned char *pBuffer;
    int i,x;
    int current_char = 0;
    int size = 0;
    int maxsize = 0;
    unsigned char character;
    SceUID fd;

    maxsize = GetID3TagSize(fname);

    fd = sceIoOpen(fname, PSP_O_RDONLY, 0777);
    if (fd < 0)
        return;
#ifdef GAME
    memid = sceKernelAllocPartitionMemory(8, "id3", PSP_SMEM_Low, maxsize, NULL);
#else
    memid = sceKernelAllocPartitionMemory(1, "id3", PSP_SMEM_Low, maxsize, NULL);
#endif
    pBuffer = (u8*)sceKernelGetBlockHeadAddr(memid);

    size = sceIoRead(fd, pBuffer, maxsize);
    sceIoClose(fd);
    if (size <= 0)
    {
        sceKernelFreePartitionMemory(memid);
        return;
    }
    
    for (i = 0; i < size; i++)
    {
        //if this is a TIT2 frame
        if ( !strncmp((char*)pBuffer+i, "TIT2", 4))
        {
            size = pBuffer[i+4]; //size of title
            size = (size<<7) | pBuffer[i+5];
            size = (size<<7) | pBuffer[i+6];
            size = (size<<7) | pBuffer[i+7];

            if (size > MAXPATH)
                size = MAXPATH;

            memset(title, 0, MAXPATH);

            for (x = 1; x < size; x++) //this allows us to parse latin-encoded 
            {                          //unicode strings, though it could be better
                character = pBuffer[i+x+10]; //buf offset + str offset + frame header size
                if ((character >= 0x20) && (character <= 0x7f))
                {  
                    title[current_char] = character;
                    current_char++;
                }
            }
            break;
        }
    }

    sceKernelFreePartitionMemory(memid);
}

char GetOMGFileType(char *fname)
{
    SceUID fd;
    int size;
    char ea3_header[0x60];

    size = GetID3TagSize(fname);

    fd = sceIoOpen(fname, PSP_O_RDONLY, 0777);
    if (fd < 0)
        return TYPE_UNK;

    sceIoLseek32(fd, size, PSP_SEEK_SET);

    if (sceIoRead(fd, ea3_header, 0x60) != 0x60)
        return TYPE_UNK;

    sceIoClose(fd);

    if (strncmp(ea3_header, "EA3", 3) != 0)
        return TYPE_UNK;

    switch (ea3_header[3])
    {
        case 1:
        case 3:
            return TYPE_AT3;
        case 2:
            return TYPE_MP3;
        default:
            return TYPE_UNK;
    }
}

