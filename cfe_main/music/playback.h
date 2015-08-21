#ifndef __PLAYBACK_H
#define __PLAYBACK_H
//this header is for anything shared between the hw and sw decoder libs

//is 261 (not 256) with ms0:/ +1 for '\0'
#define MAXPATH 262

//main thread delay amt
//too high and the on screen display will flicker
//too low and the system will slow down
#define DELAY_THREAD_AMT 10000
//delay for one sec
#define DELAY_THREAD_SEC 1000000

//music->flags
#define PLAYBACK_ERR -1
#define PLAYBACK_DONE 0
#define PLAYBACK_RESET 1
#define PLAYBACK_PLAYING 2
//music thread has finished cleaning up and is ready to be terminated
#define PLAYBACK_CLEANED_UP 3

//music->pause;
#define PLAYBACK_PAUSE_SONG 4
#define PLAYBACK_PAUSED 5
#define PLAYBACK_RESUME_SONG 6

#define NEXT_SONG 1
#define PREV_SONG -1

//later I might add other types of loops
//like random shuffle in curr folder or something
#define LOOP_NONE 0
#define LOOP_SONG 1
//so we can still use prev/next while looping is on
#define LOOP_IGNORE 2

//for the playlist
typedef struct Playlist
{
    int count;
    int omg_count;//count of files in OMGAUDIO and subdirs
    int index;
    int random_index;
    int *random_played;
    SceUID memid;//for random_played
    int random;
    int loop;
    int pause;
    int change_track;
    SceUID pl_thid;
    SceUID mus_thid;

    //these vars are used by the music modules
    int flags;
    int volume;
    int resume;//resume after error reading a file
    int offset;
    int audio_id;
    int init;//set to one to indicate the audiocodec prx is loaded
    int is_vsh;//set to one to fix vsh no sound bug
    int deb[10];
    char file[MAXPATH];
    char title[MAXPATH];//only used for aa3(sonicstage) encoded files
} Playlist;

extern Playlist *music;
#endif
