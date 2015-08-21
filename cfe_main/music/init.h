#ifndef __INIT_H__
#define __INIT_H__

void InitPatches();

#define AUDIO_FUNC_NUM 5
#define RESERVED_MEM (500*1024)

//from devhook 041d
#define MIPS_LUI(R,IMM) 0x3c000000|(R<<16)|((unsigned int)(IMM)&0xffff)
#define MIPS_ORI(RT,RS,IMM)   (0x34000000|(RS<<21)|(RT<<16)|((unsigned int)(IMM)&0xffff))
#define MIPS_NOP              0x00000000
#define MIPS_J(ADDR)          (0x08000000 + ((((unsigned int)(ADDR))&0x0ffffffc)>>2))
#define MIPS_JR(R)            (0x00000008 + ((R)<<21))
#define MIPS_JAL(ADDR)        (0x0c000000 + (((unsigned int)(ADDR)>>2)&0x03ffffff))
#define MIPS_LW(RT,BASE,OFFSET) (0x8c000000|(BASE<<21)|(RT<<16)|(OFFSET&0xFFFF))
#define MIPS_SW(RT,BASE,OFFSET) (0xac000000|(BASE<<21)|(RT<<16)|(OFFSET&0xFFFF))
#define MIPS_ADDIU(RT,RS,IMM) (0x24000000|(RS<<21)|(RT<<16)|((unsigned int)(IMM)&0xffff))

typedef int (*APRS_EVENT)(char *modname, u8 *modbuf);//for sctrlHENSetOnApplyPspRelSectionEvent
int OnPspRelSectionEvent(char *modname, u8 *modbuf);
extern APRS_EVENT previous_func;
APRS_EVENT sctrlHENSetOnApplyPspRelSectionEvent(APRS_EVENT func);
u32 FindProc(const char* szMod, const char* szLib, u32 nid);

void AudioOutput2RestoreSettings();
extern u32 music_text_addr;
extern u32 music_text_end;
#endif
