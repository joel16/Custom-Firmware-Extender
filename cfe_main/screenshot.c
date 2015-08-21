/********************************************************************/
/********************************************************************/
/***********************  SCREENSHOT  *******************************/ 
/********************************************************************/
/********************************************************************/
#include <pspkernel.h>
#include <pspdebug.h>
#include <pspkdebug.h>
#include <pspctrl_kernel.h>
#include <pspdisplay.h>
#include <pspdisplay_kernel.h>
#include <pspsdk.h>
#include <pspctrl.h>
#include <psppower.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pspthreadman.h>
#include <pspsysmem.h>
#include <pspusb.h>
#include <pspusbstor.h>
#include <pspsyscon.h>
#include <pspiofilemgr.h>

#include "screenshot.h"
#include "conf.h"

CONFIGFILE *config;

typedef struct THREAD{
	int count;
	SceUID pThread;
} THREAD;

int count_Start;
SceUID Thread_Start[MAX_THREAD];
THREAD bufNow;


int nkLoad(){

	count_Start = 0;

	sceKernelGetThreadmanIdList(SCE_KERNEL_TMID_Thread, Thread_Start, MAX_THREAD, &count_Start);

	bufNow.count = 0;
	bufNow.pThread = 0;

	return 0;
}



int nkThreadSuspend(SceUID thId){

	int i, j;
	SceUID myThread, *Thread_Now;

	if(bufNow.pThread != 0) return 1;

	bufNow.pThread = sceKernelAllocPartitionMemory(1, "th", 0, MAX_THREAD*sizeof(SceUID), NULL);
	if(bufNow.pThread < 0){
		bufNow.count = 0;
		bufNow.pThread = 0;
		return 1;
	}
	Thread_Now = (SceUID*)sceKernelGetBlockHeadAddr(bufNow.pThread);

	sceKernelGetThreadmanIdList(SCE_KERNEL_TMID_Thread, Thread_Now, MAX_THREAD, &(bufNow.count));
	myThread = sceKernelGetThreadId();

	for(i = 0; i < bufNow.count; i++){
		unsigned char match = 0;
		SceUID tmp_thid = Thread_Now[i];
		
		for(j = 0; j < count_Start; j++){
			if((tmp_thid == Thread_Start[j]) || (tmp_thid == thId) || (tmp_thid == myThread)){
				match = 1;
				j = count_Start;
			}
		}
		if(match == 0){
			sceKernelSuspendThread(tmp_thid);
		}
	}

	return 0;
}

int nkThreadResume(SceUID thId){

	int i, j;
	SceUID myThread, *Thread_Now;

	if(bufNow.pThread == 0) return 1;

	Thread_Now = (SceUID*)sceKernelGetBlockHeadAddr(bufNow.pThread);
	myThread = sceKernelGetThreadId();
	

	for(i = 0; i < bufNow.count; i++){
		unsigned char match = 0;
		SceUID tmp_thid = Thread_Now[i];
		
		for(j = 0; j < count_Start; j++){
			if((tmp_thid == Thread_Start[j]) || (tmp_thid == thId) || (tmp_thid == myThread)){
				match = 1;
				j = count_Start;
			}
		}

		if(match == 0){
			sceKernelResumeThread(tmp_thid);
		}
	}
	
	sceKernelFreePartitionMemory(bufNow.pThread);
	bufNow.count = 0;
	bufNow.pThread = 0;

	return 0;
}

typedef unsigned char   BYTE;
typedef unsigned short  WORD;
typedef unsigned long   DWORD;
typedef long            LONG;

typedef struct tagBITMAPFILEHEADER {
   //    WORD    bfType;
        DWORD   bfSize;
        DWORD   bfReserved;
        DWORD   bfOffBits;
} BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER{
	DWORD	biSize;
	LONG	biWidth;
	LONG	biHeight;
	WORD	biPlanes;
	WORD	biBitCount;
	DWORD	biCompression;
	DWORD	biSizeImage;
	LONG	biXPelsPerMeter;
	LONG	biYPelsPerMeter;
	DWORD	biClrUsed;
	DWORD	biClrImportant;
} BITMAPINFOHEADER;

void screenshot_BMP(const char* filename, char imageSize){
	
	int x, y;
	int fd = sceIoOpen(filename, PSP_O_CREAT|PSP_O_WRONLY|PSP_O_TRUNC, 0777);
	if(fd < 0) return;

	int pwidth, pheight, bufferwidth, pixelformat, unk;
	unsigned int* vram32;
	unsigned short* vram16;

	sceDisplayWaitVblankStart();
	sceDisplayGetMode(&unk, &pwidth, &pheight);
	sceDisplayGetFrameBuf((void*)&vram32, &bufferwidth, &pixelformat, unk);
	vram16 = (unsigned short*) vram32;
		
	unsigned char bm[2], padding;
	BITMAPFILEHEADER h1;
	BITMAPINFOHEADER h2;

	bm[0] = 0x42;
	bm[1] = 0x4D;
	sceIoWrite(fd, bm, 2);

	int pw = pwidth, ph = pheight;
	if(imageSize > 0) pw /=2, ph /= 2;
		
	padding = (3*(pw/2) % 4);
	h1.bfSize	= ((24*pw + padding)*ph)/8 + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + 2;
	h1.bfReserved	= 0;
	h1.bfOffBits	= 2 + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	h2.biSize	= sizeof(BITMAPINFOHEADER);
	h2.biPlanes	= 1;
	h2.biBitCount	= 24;
	h2.biCompression 	= 0;
	h2.biWidth	= pw;
	h2.biHeight	= ph;
	h2.biSizeImage	= ((24*pw + padding)*ph)/8;
	h2.biXPelsPerMeter	= 0xEC4;
	h2.biYPelsPerMeter	= 0xEC4;
	h2.biClrUsed		= 0;
	h2.biClrImportant	= 0;

	sceIoWrite(fd, &h1, sizeof(BITMAPFILEHEADER));
	sceIoWrite(fd, &h2, sizeof(BITMAPINFOHEADER));
	
	SceUID mem = sceKernelAllocPartitionMemory(1, "block", 0, (3*pw + padding)*sizeof(unsigned char), NULL);
	unsigned char *buf = (unsigned char*)sceKernelGetBlockHeadAddr(mem);

	for(x = 0; x < padding; x++) buf[3*pw + x] = 0;

	for(y = (ph-1); y >= 0; y--){
		int i;
		for(i = 0, x = 0; x < pw; x++){
			unsigned int color, offset = x + y*bufferwidth;
			unsigned char r = 0, g = 0, b = 0;
			if(imageSize) offset *= 2;

			switch (pixelformat) {
				case 0:	// 16-bit RGB 5:6:5
					color = vram16[offset];
					vram16[offset] ^= 0xFFFF;
					r = (color & 0x1f) << 3; 
					g = ((color >> 5) & 0x3f) << 2 ;
					b = ((color >> 11) & 0x1f) << 3 ;
					break;
				case 1:// 16-bit RGBA 5:5:5:1
					color = vram16[offset];
					vram16[offset] ^= 0x7FFF;	
					r = (color & 0x1f) << 3; 
					g = ((color >> 5) & 0x1f) << 3 ;
					b = ((color >> 10) & 0x1f) << 3 ;
					break;
				case 2:// 16-bit RGBA 4:4:4:4
					color = vram16[offset];
					vram16[offset] ^= 0x0FFF;	
					r = (color & 0xf) << 4; 
					g = ((color >> 4) & 0xf) << 4 ;
					b = ((color >> 8) & 0xf) << 4 ;
					break;
				case 3:// 32-bit RGBA 8:8:8:8
					color = vram32[offset];
					vram32[offset] ^= 0x00FFFFFF;	
					r = color & 0xff; 
					g = (color >> 8) & 0xff;
					b = (color >> 16) & 0xff;
					break;
			}
			buf[i++] = b;
			buf[i++] = g;
			buf[i++] = r;
		}
		sceIoWrite(fd, buf, 3*pw + padding);
	}
	sceKernelFreePartitionMemory(mem);
	sceIoClose(fd);

	for(y = 0; y < ph; y++){
		int i;
		for(i = 0, x = 0; x < pw; x++){
			unsigned int offset = x + y*bufferwidth;
			if(imageSize) offset *= 2;

			switch (pixelformat) {
				case 0:	// 16-bit RGB 5:6:5
					vram16[offset] ^= 0xFFFF;	
					break;
				case 1:// 16-bit RGBA 5:5:5:1
					vram16[offset] ^= 0x7FFF;	
					break;
				case 2:// 16-bit RGBA 4:4:4:4
					vram16[offset] ^= 0x0FFF;
					break;
				case 3:// 32-bit RGBA 8:8:8:8
					vram32[offset] ^= 0x00FFFFFF;
					break;
			}
		}
	}
}

void screenshot(void) {

	int fd, count = 0, size = 0;

	//nkThreadSuspend(sceKernelGetThreadId());

	while(1)
	{
		sprintf(file, "%s/snap%03d.bmp", config->capture_folder, count);
		fd = sceIoOpen(file, PSP_O_RDONLY, 0644);
		if(fd < 0){
			sceIoClose(fd);
			break;
		}
		sceIoClose(fd);
		count++;
	}
	screenshot_BMP(file, size);

	//nkThreadResume(sceKernelGetThreadId());
}
