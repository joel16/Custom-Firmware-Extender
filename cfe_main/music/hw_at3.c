#include <pspiofilemgr.h>
#include <pspaudio.h>
#include <pspaudiocodec.h>
#include <pspsysmem.h>
#include <pspthreadman.h>

#include <malloc.h>
#include <string.h>
#include <stdlib.h>

#include "playback.h"
#include "hw.h"
#include "hw_at3.h"

//based on example by cooleyes http://forums.ps2dev.org/viewtopic.php?t=8357
u32 at3_channels;

int HW_AT3PlayFile(SceSize args, void *argp)
{
    int ret;

    HW_LoadModules();

    pspOpenAudio(AT3_OUTPUT_BUFFER_SIZE/4);

    ret = hw_at3_play();

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

int hw_at3_play()
{
    u32 riff_header[2];
    u32 wavefmt_header[3];
    u32 data_header[2];
    u8* wavefmt_data;
    int eof = 0;
    int res;
    unsigned long decode_type;
    int temp_size;
    int mod_64;
  SceUID header_memid;

    long size;

    OutputBuffer_flip = 0;
    AT3_OutputPtr = AT3_OutputBuffer[0];

    fd = sceIoOpen(music->file, PSP_O_RDONLY, 0777);
    if (fd < 0)
        return PLAYBACK_ERR;
   
    if ( sceIoRead( fd, riff_header, 8 ) != 8 )
        goto wait;
    if ( riff_header[0] != 0x46464952 )
        goto wait;
    if ( sceIoRead( fd, wavefmt_header, 12 ) != 12 )
        goto wait;
    if ( wavefmt_header[0] != 0x45564157 || wavefmt_header[1] != 0x20746D66 )
        goto wait;

    header_memid = sceKernelAllocPartitionMemory(8, "mem", PSP_SMEM_Low, wavefmt_header[2], NULL);
    wavefmt_data = (u8*)sceKernelGetBlockHeadAddr(header_memid);

    if ( wavefmt_data == NULL )
        goto wait;

    if ( sceIoRead( fd, wavefmt_data, wavefmt_header[2] ) != wavefmt_header[2] )
    {
        sceKernelFreePartitionMemory(header_memid);
        goto wait;
    }

    at3_type = *((u16*)wavefmt_data);
    at3_channels = *((u16*)(wavefmt_data+2));
    samplerate = *((u32*)(wavefmt_data+4));
    data_align = *((u16*)(wavefmt_data+12));
   
    if ( at3_type == TYPE_ATRAC3PLUS)
    {
       at3_at3plus_flagdata[0] = wavefmt_data[42];
       at3_at3plus_flagdata[1] = wavefmt_data[43];
    }
   
    sceKernelFreePartitionMemory(header_memid);
   
    if ( sceIoRead( fd, data_header, 8 ) != 8 )
        goto wait;

    while(data_header[0] != 0x61746164 )
    {
        sceIoLseek32(fd, data_header[1], PSP_SEEK_CUR);
        if ( sceIoRead( fd, data_header, 8 ) != 8 )
            goto wait;
    }
   
    data_start = sceIoLseek32(fd, 0, PSP_SEEK_CUR);
    size = sceIoLseek32(fd, 0, PSP_SEEK_END);
    sceIoLseek32(fd, data_start, PSP_SEEK_SET);

    data_size = data_header[1];
   
    if ( data_size % data_align != 0 )
        goto wait;

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
        size -= music->offset;
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
            res = sceIoRead(fd, codec->input_buffer, data_align);

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

        size -= data_align;
        if (size <= 0)
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
        AT3_OutputPtr += (sample_per_frame*4);
        if( AT3_OutputPtr + (sample_per_frame*4) > &AT3_OutputBuffer[OutputBuffer_flip][AT3_OUTPUT_BUFFER_SIZE])
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
