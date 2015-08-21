#include <pspiofilemgr.h>
#include <pspaudio.h>
#include <pspaudiocodec.h>
#include <pspsysmem.h>
#include <pspthreadman.h>

#include <malloc.h>
#include <string.h>
#include <stdlib.h>

#include "main.h"
#include "playback.h"
#include "hw.h"
#include "hw_aa3.h"

//based on example by cooleyes http://forums.ps2dev.org/viewtopic.php?t=8357

int GetID3TagSize(char *fname);

int HW_AA3PlayFile(SceSize args, void *argp)
{
    int ret;

    HW_LoadModules();

    pspOpenAudio(AT3_OUTPUT_BUFFER_SIZE/4);

    ret = hw_aa3_play();

    if (ret == PLAYBACK_DONE)
        music->offset = 0;
    else if ((ret == PLAYBACK_RESET) || (ret == PLAYBACK_ERR))//invalid file after suspend/usb
        music->resume = 1;

    if (getEDRAM)
       sceAudiocodecReleaseEDRAM(codec->codec_buffer);

    if(fd >= 0)
        sceIoClose(fd);

    pspCloseAudio();

    while(1)
    {
        music->flags = PLAYBACK_CLEANED_UP;
        sceKernelDelayThreadCB(DELAY_THREAD_AMT);
    }
    return 0;
}

int hw_aa3_play()
{
    u8 ea3_header[0x60];
    int eof = 0;
    int temp_size;
    int mod_64;
    int res;
    unsigned long decode_type;
    int tag_size;

    OutputBuffer_flip = 0;
    AT3_OutputPtr = AT3_OutputBuffer[0];
    
    tag_size = GetID3TagSize(music->file);
    fd = sceIoOpen(music->file, PSP_O_RDONLY, 0777);
    if (fd < 0)
        return PLAYBACK_ERR;
    
    sceIoLseek32(fd, tag_size, PSP_SEEK_SET);//not all omg files have a fixed header
    
    if ( sceIoRead( fd, ea3_header, 0x60 ) != 0x60 )
        goto wait;
    
    if ( ea3_header[0] != 0x45 || ea3_header[1] != 0x41 || ea3_header[2] != 0x33 )
        goto wait;
    
    at3_at3plus_flagdata[0] = ea3_header[0x22];
    at3_at3plus_flagdata[1] = ea3_header[0x23];
       
    at3_type = (ea3_header[0x22] == 0x20) ? TYPE_ATRAC3 : ((ea3_header[0x22] == 0x28) ? TYPE_ATRAC3PLUS : 0x0);
    
    if ( at3_type != TYPE_ATRAC3 && at3_type != TYPE_ATRAC3PLUS )
        goto wait;

    if ( at3_type == TYPE_ATRAC3 )
        data_align = ea3_header[0x23]*8;
    else
        data_align = (ea3_header[0x23]+1)*8;

    data_start = tag_size+0x60;
    data_size = sceIoLseek32(fd, 0, PSP_SEEK_END) - data_start;

    sceIoLseek32(fd, data_start, PSP_SEEK_SET);

    memset(codec->codec_buffer, 0, sizeof(codec->codec_buffer));
   
    if ( at3_type == TYPE_ATRAC3 )
    {
        channel_mode = 0x0;

        if ( data_align == 0xC0 ) // atract3 have 3 bitrate, 132k,105k,66k, 132k align=0x180, 105k align = 0x130, 66k align = 0xc0
            channel_mode = 0x1;

        sample_per_frame = 1024;

        codec->codec_buffer[26] = 0x20;
        if ( sceAudiocodecCheckNeedMem(codec->codec_buffer, 0x1001) < 0 )
            goto wait;
        if ( sceAudiocodecGetEDRAM(codec->codec_buffer, 0x1001) < 0 )
            goto wait;

        getEDRAM = 1;

        codec->codec_buffer[10] = 4;
        codec->codec_buffer[44] = 2;

        if ( data_align == 0x130 )
            codec->codec_buffer[10] = 6;

        if ( sceAudiocodecInit(codec->codec_buffer, 0x1001) < 0 )
            goto wait;
    }
    else if ( at3_type == TYPE_ATRAC3PLUS )
    {
        sample_per_frame = 2048;
        temp_size = data_align+8;
        mod_64 = temp_size & 0x3f;
        if (mod_64 != 0) temp_size += 64 - mod_64;

        codec->codec_buffer[5] = 0x1;
        codec->codec_buffer[10] = at3_at3plus_flagdata[1];
        codec->codec_buffer[10] = (codec->codec_buffer[10] << 8 ) | at3_at3plus_flagdata[0];
        codec->codec_buffer[12] = 0x1;
        codec->codec_buffer[14] = 0x1;

        if ( sceAudiocodecCheckNeedMem(codec->codec_buffer, 0x1000) < 0 )
            goto wait;
        if ( sceAudiocodecGetEDRAM(codec->codec_buffer, 0x1000) < 0 )
            goto wait;

        getEDRAM = 1;

        if ( sceAudiocodecInit(codec->codec_buffer, 0x1000) < 0 )
            goto wait;
    }
    else
        goto wait;
   
    if (music->resume)
    {
        sceIoLseek32(fd, music->offset, PSP_SEEK_SET);
        data_size -= music->offset;
        music->resume = 0;
    }

    while( !eof && (music->flags == PLAYBACK_PLAYING) )
    {
        data_start = sceIoLseek32(fd, 0, PSP_SEEK_CUR);

        if (data_start < 0)
            return PLAYBACK_RESET;

        if ( at3_type == TYPE_ATRAC3 )
        {
            memset( codec->input_buffer, 0, 0x180);

            res = sceIoRead( fd, codec->input_buffer, data_align );

            if (res < 0)//error reading suspend/usb problem
                return PLAYBACK_RESET;
            else if (res != data_align)
            {
                eof = 1;
                continue;
            }

            if ( channel_mode )
                memcpy(codec->input_buffer+data_align, codec->input_buffer, data_align);

            decode_type = 0x1001;
        }
        else
        {
            memset( codec->input_buffer, 0, data_align+8);

            codec->input_buffer[0] = 0x0F;
            codec->input_buffer[1] = 0xD0;
            codec->input_buffer[2] = at3_at3plus_flagdata[0];
            codec->input_buffer[3] = at3_at3plus_flagdata[1];

            res = sceIoRead( fd, codec->input_buffer+8, data_align );

            if (res < 0)//error reading suspend/usb problem
                return PLAYBACK_RESET;
            else if (res != data_align)
            {
                eof = 1;
                continue;
            }
            decode_type = 0x1000;
        }

        music->offset = data_start;

        data_size -= data_align;
        if (data_size <= 0)
        {
            eof = 1;
            continue;
        }

        codec->codec_buffer[6] = (unsigned long)codec->input_buffer;
        codec->codec_buffer[8] = (unsigned long)codec->output_buffer;
   
        res = sceAudiocodecDecode(codec->codec_buffer, decode_type);
        if ( res < 0 )
        {
            eof = 1;
            continue;
        }

        memcpy( AT3_OutputPtr, codec->output_buffer, sample_per_frame*4);
        AT3_OutputPtr += (sample_per_frame * 4);
        if( AT3_OutputPtr + (sample_per_frame * 4) > &AT3_OutputBuffer[OutputBuffer_flip][AT3_OUTPUT_BUFFER_SIZE])
        {
            sceAudioOutputBlocking(music->audio_id, PSP_AUDIO_VOLUME_MAX*music->volume/100, AT3_OutputBuffer[OutputBuffer_flip] );

            OutputBuffer_flip ^= 1;
            AT3_OutputPtr = AT3_OutputBuffer[OutputBuffer_flip];
        }
    }

wait:
   if (music->flags == PLAYBACK_RESET)
       return PLAYBACK_RESET;

   return PLAYBACK_DONE;
}
