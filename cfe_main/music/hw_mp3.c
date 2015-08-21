#include <pspiofilemgr.h>
#include <pspaudio.h>
#include <pspaudiocodec.h>
#include <pspthreadman.h>

#include <malloc.h>
#include <string.h>
#include <stdlib.h>

#include "playback.h"
#include "hw.h"
#include "hw_mp3.h"

//based on example from cooleyes http://forums.ps2dev.org/viewtopic.php?t=8469
static int bitrates[] = {0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320 };
static int bitrates_v2[] = {0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160 };

static int samplerates[4][3] = 
{
    {11025, 12000, 8000,},//mpeg 2.5
    {0, 0, 0,}, //reserved
    {22050, 24000, 16000,},//mpeg 2
    {44100, 48000, 32000}//mpeg 1
};

unsigned char	OutputBuffer[2][OUTPUT_BUFFER_SIZE],
				*OutputPtr=OutputBuffer[0];

char output2init; //set to one to indicate we are using sceAudioOutput2, since it doesn't have channels

int HW_MP3PlayFile(SceSize args, void *argp)
{

    int ret;

    HW_LoadModules();

    pspOpenAudio(OUTPUT_BUFFER_SIZE/4);
    ret = hw_mp3_play();

    if (ret == PLAYBACK_DONE)
        music->offset = 0;
    else if ((ret == PLAYBACK_RESET) || (ret == PLAYBACK_ERR))//invalid file after suspend/usb
        music->resume = 1;

    if (getEDRAM)
        sceAudiocodecReleaseEDRAM(codec->codec_buffer);

    if(fd >= 0)
        sceIoClose(fd);

    if (output2init == 0)
        pspCloseAudio();
    else
        output2init = 0;//don't call Output2Release so it doesn't mess up vsh video/audio players


    while(1)
    {

        music->flags = PLAYBACK_CLEANED_UP;
        sceKernelDelayThreadCB(DELAY_THREAD_AMT);
    }

    music->flags = PLAYBACK_ERR;
    sceKernelSleepThreadCB();
    return 0;
}

int hw_mp3_play()
{  
    int res;
    unsigned char mp3_header_buf[4];
    int mp3_header;
    int version;
    int bitrate;
    int padding;
    int frame_size;
    char eof;
    int size;

    OutputBuffer_flip = 0;
    OutputPtr = OutputBuffer[0];

    fd = sceIoOpen(music->file, PSP_O_RDONLY, 0777);
    if (fd < 0)
        return PLAYBACK_ERR;

    if (music->resume)
    {
        music->resume = 0;
        size = sceIoLseek32(fd, 0, PSP_SEEK_END); 
        sceIoLseek32(fd, music->offset, PSP_SEEK_SET);
        data_start = music->offset;
    }
    else
    {
        //now search for the first sync byte, tells us where the mp3 stream starts
        size = sceIoLseek32(fd, 0, PSP_SEEK_END);
        sceIoLseek32(fd, 0, PSP_SEEK_SET);
        data_start = SeekNextFrame(fd);
        if (data_start < 0)
            goto wait;
    }
    size -= data_start;

    memset(codec->codec_buffer, 0, sizeof(codec->codec_buffer));

    if ( sceAudiocodecCheckNeedMem(codec->codec_buffer, 0x1002) < 0 )
        goto wait; 

    if ( sceAudiocodecGetEDRAM(codec->codec_buffer, 0x1002) < 0 )
        goto wait;

    getEDRAM = 1;

    if ( sceAudiocodecInit(codec->codec_buffer, 0x1002) < 0 )
        goto wait;

    eof = 0;   
    while( !eof && (music->flags == PLAYBACK_PLAYING) )
    {
        if ( sceIoRead( fd, mp3_header_buf, 4 ) != 4 )
            return PLAYBACK_RESET;

        mp3_header = mp3_header_buf[0];
        mp3_header = (mp3_header<<8) | mp3_header_buf[1];
        mp3_header = (mp3_header<<8) | mp3_header_buf[2];
        mp3_header = (mp3_header<<8) | mp3_header_buf[3];

        bitrate = (mp3_header & 0xf000) >> 12;
        padding = (mp3_header & 0x200) >> 9;
        version = (mp3_header & 0x180000) >> 19;
        samplerate = samplerates[version][ (mp3_header & 0xC00) >> 10 ];

        if ((bitrate > 14) || (version == 1) || (samplerate == 0) || (bitrate == 0))//invalid frame, look for the next one
        {
            data_start = SeekNextFrame(fd);
            if(data_start < 0)
            {
                eof = 1;
                continue;
            }
            size -= (data_start - music->offset); 
            music->offset = data_start;
            continue;
        }

//      FIXME, CAUSES PROBLEMS IN GTA:VCS, possibly others 
        if ((!output2init) && (samplerate != 44100))
        {
            sceAudioChRelease(music->audio_id);

// Cpasjuste 
           res = sceAudioSRCChReserve(OUTPUT_BUFFER_SIZE/4, samplerate, 2);

            if (res >= 0)//sucessfully alloc'd a channel, returns < 0 on resume
            {
                output2init = 1;
                music->audio_id = 9;
            }

        }

        if (output2init && (samplerate == 44100))//bad mp3,changes samplerate, fix, really should re-encode this song...
        {
            pspOpenAudio(OUTPUT_BUFFER_SIZE/4);
            output2init = 0;        
        }

        if (version == 3) //mpeg-1
        {
            sample_per_frame = 1152;
            frame_size = 144000*bitrates[bitrate]/samplerate + padding;
        }
        else
        {
            sample_per_frame = 576;
            frame_size = 72000*bitrates_v2[bitrate]/samplerate + padding;
        }

        sceIoLseek32(fd, data_start, PSP_SEEK_SET); //seek back
                   
        size -= frame_size;
        if ( size <= 0)
        {
           eof = 1;
           continue;
        }
        //since we check for eof above, this can only happen when the file
        // handle has been invalidated by syspend/resume/usb
        if ( sceIoRead( fd, codec->input_buffer, frame_size ) != frame_size )
           return PLAYBACK_RESET;

        data_start += frame_size;
        music->offset = data_start;

        codec->codec_buffer[6] = (unsigned long)codec->input_buffer;
        codec->codec_buffer[8] = (unsigned long)codec->output_buffer;
            
        codec->codec_buffer[7] = codec->codec_buffer[10] = frame_size;
        codec->codec_buffer[9] = sample_per_frame * 4;

        res = sceAudiocodecDecode(codec->codec_buffer, 0x1002);

        if ( res < 0 )
        {
            //instead of quitting see if the next frame can be decoded
            //helps play files with an invalid frame
            //we must look for a valid frame, the offset above may be wrong
            data_start = SeekNextFrame(fd);
            if(data_start < 0)
            {
                eof = 1;
                continue;
            }
            size -= (data_start - music->offset); 
            music->offset = data_start;
            continue;
        }

        memcpy( OutputPtr, codec->output_buffer, sample_per_frame*4);
        OutputPtr += (sample_per_frame * 4);
        if( OutputPtr + (sample_per_frame * 4) > &OutputBuffer[OutputBuffer_flip][OUTPUT_BUFFER_SIZE])
        {
            if (!output2init)
                sceAudioOutputBlocking(music->audio_id, PSP_AUDIO_VOLUME_MAX*music->volume/100, OutputBuffer[OutputBuffer_flip] );
            else
            {
// Cpasjuste

                 res = sceAudioOutput2OutputBlocking(PSP_AUDIO_VOLUME_MAX*music->volume/100, OutputBuffer[OutputBuffer_flip]);//sceAudioOutput2OutputBlocking
                 if (res < 0) //error, re-alloc output2
                     output2init = 0;

            }

            OutputBuffer_flip ^= 1;
            OutputPtr = OutputBuffer[OutputBuffer_flip];
        }
    }
wait:

    if (getEDRAM)
        sceAudiocodecReleaseEDRAM(codec->codec_buffer);

    if (music->flags == PLAYBACK_RESET)
        return PLAYBACK_RESET;

    return PLAYBACK_DONE;

}

int SeekNextFrame(SceUID fd)
{
    int offset = 0;
    unsigned char buf[1024];
    unsigned char *pBuffer;
    int i;
    int size = 0;

    offset = sceIoLseek32(fd, 0, PSP_SEEK_CUR);
    sceIoRead(fd, buf, sizeof(buf));
    if (!strncmp((char*)buf, "ID3", 3) || !strncmp((char*)buf, "ea3", 3)) //skip past id3v2 header, which can cause a false sync to be found
    {
        //get the real size from the syncsafe int
        size = buf[6];
        size = (size<<7) | buf[7];
        size = (size<<7) | buf[8];
        size = (size<<7) | buf[9];

        size += 10;

        if (buf[5] & 0x10) //has footer
            size += 10;
    }

    sceIoLseek32(fd, offset + size, PSP_SEEK_SET); //now seek for a sync
    while(1) 
    {
        offset = sceIoLseek32(fd, 0, PSP_SEEK_CUR);
        size = sceIoRead(fd, buf, sizeof(buf));

        if (size <= 2)//at end of file
            return -1;
    
        if (!strncmp((char*)buf, "EA3", 3))//oma mp3 files have non-safe ints in the EA3 header
        {
            sceIoLseek32(fd, (buf[4]<<8)+buf[5], PSP_SEEK_CUR);
            continue;
        }

        pBuffer = buf;
        for( i = 0; i < size; i++)
        {
            //if this is a valid frame sync (0xe0 is for mpeg version 2.5,2+1)
            if ( (pBuffer[i] == 0xff) && ((pBuffer[i+1] & 0xE0) == 0xE0))
            {
                offset += i;
                sceIoLseek32(fd, offset, PSP_SEEK_SET);
                return offset;
            }
        }
       //go back two bytes to catch any syncs that on the boundary
        sceIoLseek32(fd, -2, PSP_SEEK_CUR);
    } 
}
