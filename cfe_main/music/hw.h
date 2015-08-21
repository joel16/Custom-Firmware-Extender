#ifndef __HW__H__
#define __HW__H__

#define MIPS_J(ADDR)          (0x08000000 + ((((unsigned int)(ADDR))&0x0ffffffc)>>2))
#define AT3_OUTPUT_BUFFER_SIZE	(2048*2*4) /* Must be an integer multiple of 4 + of 2048. */

//to play mp3's at nonstandard sample rates
//Output2 has no concept of channels, so only one app can use it at a time
//this causes the vsh music player to stop working when we use it
//int sceAudio_driver_669D93E4(int samplecount, int samplerate, int unk);//unk = 2 (only func called by sceAudioOutput2Reserve)
//int sceAudio_driver_43645E69(int vol, void *buf);//sceAudioOutput2OutputBlocking alias, present in 1.5
//int sceAudio_driver_138A70F1(void);//sceAudioOutput2Release alias, also present in 1.5

u32 FindProc(const char* szMod, const char* szLib, u32 nid);

void pspOpenAudio(int samplecount);
void pspCloseAudio();
void HW_LoadModules();
int HWInit();

//this struct represents everything which needs to be in user mem
typedef struct CodecData {
    unsigned long codec_buffer[65]__attribute__((aligned(64)));
    unsigned char input_buffer[2889]__attribute__((aligned(64)));//mp3 has the largest max frame, at3+ 352 is 2176
    unsigned char output_buffer[2048*4]__attribute__((aligned(64)));//at3+ sample_per_frame*4
} CodecData;

extern CodecData *codec;
extern SceUID fd;
extern u16 data_align;
extern u32 sample_per_frame;
extern u16 channel_mode;
extern long data_start;
extern long data_size;
extern u8 getEDRAM;
extern u32 samplerate;
extern SceUID data_memid;
extern volatile int OutputBuffer_flip;

extern u16 at3_type;
extern u8 at3_at3plus_flagdata[2];
extern unsigned char AT3_OutputBuffer[2][AT3_OUTPUT_BUFFER_SIZE];
extern unsigned char *AT3_OutputPtr;

#endif
