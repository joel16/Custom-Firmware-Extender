#include <pspsdk.h>
#include <pspkernel.h>
#include <pspdebug.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <psploadexec_kernel.h>
#include <kubridge.h>
#include <systemctrl.h>
#include <systemctrl_se.h>
#include <pspusb.h>
#include <pspaudio.h>

#include "utils.h"
#include "common.h"
#include "guicommon.h"

u32 orig_funcs[6];

int file_exist(char *filename)
{
	SceUID file = sceIoOpen(filename, PSP_O_RDONLY, 0511);

	if(file < 0) // file don't exist
	{
		printf("File do not exist : %s\n", filename);
		sceIoClose(file);
		return 0;
	}

	printf("File exist : %s\n", filename);
	sceIoClose(file);
	return 1;
}

char write_buffer[64*1024];

void copy_file(const char *read_loc, const char *write_loc)
{
	int fdin;
	int fdout;

	fdin = sceIoOpen(read_loc, PSP_O_RDONLY, 0511);
	if(fdin >= 0)
	{
		int bytesRead = 1;
		fdout = sceIoOpen(write_loc, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
		if(fdout < 0)
		{
			printf("Couldn't open %s\n", write_loc);
		}

		bytesRead = sceIoRead(fdin, write_buffer, sizeof(write_buffer));
		while((bytesRead > 0) && (fdout >= 0))
		{
			sceIoWrite(fdout, write_buffer, bytesRead);
			bytesRead = sceIoRead(fdin, write_buffer, sizeof(write_buffer));
		}

		if(fdout >= 0)
		{
			sceIoClose(fdout);
		}

		if(fdin >= 0)
		{
			sceIoClose(fdin);
		}
	}
	else
	{
		printf("Couldn't open %s\n", read_loc);
	}
}

int build_args(char *args, const char *execfile, int argc, char **argv)
{
	int loc = 0;
	int i;

	strcpy(args, execfile);
	loc += strlen(execfile) + 1;
	for(i = 0; i < argc; i++)
	{
		strcpy(&args[loc], argv[i]);
		printf("Arg %i => %s\n", i, argv[i]);
		loc += strlen(argv[i]) + 1;
	}

	return loc;
}

SceUID loadStartModuleWithArgs(const char *filename, int mpid, int argc, char * const argv[])
{
	SceKernelLMOption option;
	SceUID modid = 0;
	int retVal = 0, mresult;
	char args[MAX_ARGS];
	int  argpos = 0;
	int  i;

	memset(args, 0, MAX_ARGS);
	strcpy(args, filename);
	argpos += strlen(args) + 1;
	for(i = 0; (i < argc) && (argpos < MAX_ARGS); i++)
	{
		int len;

		snprintf(&args[argpos], MAX_ARGS-argpos, "%s", argv[i]);
		len = strlen(&args[argpos]);
		argpos += len + 1;
	}

	memset(&option, 0, sizeof(option));
	option.size = sizeof(option);
	option.mpidtext = mpid;
	option.mpiddata = mpid;
	option.position = 0;
	option.access = 1;

	retVal = sceKernelLoadModule(filename, 0, &option);
	if(retVal < 0){
		printf("sceKernelLoadModule : %x\n", retVal);
		return retVal;
	}

	modid = retVal;

	retVal = sceKernelStartModule(modid, argpos, args, &mresult, NULL);
	if(retVal < 0){
		printf("sceKernelStartModule : %x\n", retVal);
		return retVal;
	}

	return 1;
}

int loadStartModule(const char *name, int argc, char **argv)
{
	SceUID modid;
	int status;
	char args[128];
	int len;

	modid = sceKernelLoadModule(name, 0, NULL);
	if(modid >= 0)
	{
		len = build_args(args, name, argc, argv);
		modid = sceKernelStartModule(modid, len, (void *) args, &status, NULL);
	}
	else printf("lsm: Error loading module %s\n", name);

	return modid;
}

int unloadStopModule(char* modname)
{
	SceModule * module = ( SceModule * )sceKernelFindModuleByName(modname);

	if (module != NULL)
	{
		sceKernelStopModule(module->modid, 0, NULL, NULL, NULL);
		sceKernelUnloadModule(module->modid);
		sceKernelDelayThread(10000);
	}
	return 0;

}

int startRemotejoy()
{
	int ret = 0;
	ret = sceUsbStart( PSP_USBBUS_DRIVERNAME, 0, 0 );
	if ( ret != 0 ){ gui_print("Remotejoy Error : sceUsbStart"); return( -1 ); }
	ret = sceUsbStart( RJLITE_DRIVERNAME, 0, 0 );
	if ( ret != 0 ){ gui_print("Remotejoy Error : sceUsbStart"); return( -1 ); }
	ret = sceUsbActivate( RJLITE_DRIVERPID );
	if ( ret != 0 ){ gui_print("Remotejoy Error : sceUsbActivate"); return( -1 ); }

	remotejoy_active = 1;
	
	return 1;
}

int stopRemotejoy()
{
	sceUsbDeactivate(RJLITE_DRIVERPID);
	sceUsbStop(RJLITE_DRIVERNAME, 0, 0); 
  	sceUsbStop(PSP_USBBUS_DRIVERNAME, 0, 0);

	remotejoy_active = 0;

	return 1;
}

void unloadUsbhost()
{
	sceUsbDeactivate(HOSTFSDRIVER_PID);
	sceUsbStop(HOSTFSDRIVER_NAME, 0, 0); //sceUsbStop
  	sceUsbStop(PSP_USBBUS_DRIVERNAME, 0, 0);
	unloadStopModule("USBHostFS");
}

int loadUsbHost() 
{
	loadStartModule("ms0:/seplugins/cfe/usbhostfs.prx", 0, NULL);

	int retVal = 0;
	retVal =  sceUsbStart(PSP_USBBUS_DRIVERNAME, 0, 0); //SceUsbStart
	if (retVal != 0) {
		printf("Error starting USB Bus driver\n");
		return 0;
	}
	retVal =  sceUsbStart(HOSTFSDRIVER_NAME, 0, 0);
	if (retVal != 0) {
		printf("Error starting USB Host driver\n");
		return 0;
	}
	retVal = sceUsbActivate(HOSTFSDRIVER_PID); //SceUsbActivate
			
	sceKernelDelayThread(30*100000);

	return 0;
}

void unloadUsb()
{
	sceUsbDeactivate(0x1c8);
	sceUsbStop(PSP_USBSTOR_DRIVERNAME, 0, 0); //sceUsbStop
  	sceUsbStop(PSP_USBBUS_DRIVERNAME, 0, 0);
}

int loadUsb(void)
{

	if(!usbmodloaded) 
	{
		loadStartModule("flash0:/kd/semawm.prx", 0, NULL);
		loadStartModule("flash0:/kd/usbstor.prx", 0, NULL);
		loadStartModule("flash0:/kd/usbstormgr.prx", 0, NULL);
		loadStartModule("flash0:/kd/usbstorms.prx", 0, NULL);
		loadStartModule("flash0:/kd/usbstorboot.prx", 0, NULL);
		loadStartModule("flash0:/kd/usbdevice.prx", 0, NULL); 

		usbmodloaded=1;
	}

	if (state & PSP_USB_ACTIVATED)
	{
         	sceUsbDeactivate(0x1c8);
		sceUsbStop(PSP_USBSTOR_DRIVERNAME, 0, 0);
         	sceUsbStop(PSP_USBBUS_DRIVERNAME, 0, 0); 
	}
	else
	{
		sceUsbStart(PSP_USBBUS_DRIVERNAME, 0, 0);
		sceUsbStart(PSP_USBSTOR_DRIVERNAME, 0, 0);
		sceUsbActivate(0x1c8); 
	}
	return 0;
}


typedef int (*sceAudioOutput_orig_t)(int channel, int vol, void *buf);
static sceAudioOutput_orig_t sceAudioOutput_orig;

typedef int (*sceAudioOutputBlocking_orig_t)(int channel, int vol, void *buf);
static sceAudioOutputBlocking_orig_t sceAudioOutputBlocking_orig;

typedef int (*sceAudioOutputPanned_orig_t)(int channel, int leftvol, int rightvol, void *buf);
static sceAudioOutputPanned_orig_t sceAudioOutputPanned_orig;

typedef int (*sceAudioOutputPannedBlocking_orig_t)(int channel, int leftvol, int rightvol, void *buf);
static sceAudioOutputPannedBlocking_orig_t sceAudioOutputPannedBlocking_orig;

typedef int (*sceAudioOutput2OutputBlocking_orig_t)(int vol, void *buf);
static sceAudioOutput2OutputBlocking_orig_t sceAudioOutput2OutputBlocking_orig;

typedef int (*sceAudioSRCOutputBlocking_orig_t)(int vol, void *buf);
static sceAudioSRCOutputBlocking_orig_t sceAudioSRCOutputBlocking_orig;


int sceAudioOutput_patched(int channel, int vol, void *buf)
{
	return sceAudioOutput_orig(channel, PSP_AUDIO_VOLUME_MAX*channel_volume[channel]/100, buf);
}

int sceAudioOutputBlocking_patched(int channel, int vol, void *buf)
{
	return sceAudioOutputBlocking_orig(channel, PSP_AUDIO_VOLUME_MAX*channel_volume[channel]/100, buf);
}

int sceAudioOutputPanned_patched(int channel, int leftvol, int rightvol, void *buf)
{
	return sceAudioOutputPanned_orig(channel, PSP_AUDIO_VOLUME_MAX*channel_volume[channel]/100, PSP_AUDIO_VOLUME_MAX*channel_volume[channel]/100, buf);
}

int sceAudioOutputPannedBlocking_patched(int channel, int leftvol, int rightvol, void *buf)
{
	return sceAudioOutputPannedBlocking_orig(channel, PSP_AUDIO_VOLUME_MAX*channel_volume[channel]/100, PSP_AUDIO_VOLUME_MAX*channel_volume[channel]/100, buf);
}

int sceAudioOutput2OutputBlocking_patched(int vol, void *buf)
{
	return sceAudioOutput2OutputBlocking_orig(PSP_AUDIO_VOLUME_MAX*channel_volume[0]/100, buf);
}

int sceAudioSRCOutputBlocking_patched(int vol, void *buf)
{
	return sceAudioSRCOutputBlocking_orig(PSP_AUDIO_VOLUME_MAX*channel_volume[0]/100, buf);
}

void patch_audio()
{
	printf("patching sceAudio nids\n");
	//int sceAudioOutput(int channel, int vol, void *buf);
	orig_funcs[0] = sctrlHENFindFunction("sceAudio_Driver", "sceAudio", 0x8C1009B2);

	//int sceAudioOutputBlocking(int channel, int vol, void *buf);
	orig_funcs[1] = sctrlHENFindFunction("sceAudio_Driver", "sceAudio", 0x136CAF51);

	//int sceAudioOutputPanned(int channel, int leftvol, int rightvol, void *buf);
	orig_funcs[2] = sctrlHENFindFunction("sceAudio_Driver", "sceAudio", 0xE2D56B2D);

	//int sceAudioOutputPannedBlocking(int channel, int leftvol, int rightvol, void *buf);
	orig_funcs[3] = sctrlHENFindFunction("sceAudio_Driver", "sceAudio", 0x13F592BC);

	//int sceAudioOutput2OutputBlocking(int vol, void *buf);
	orig_funcs[4] = sctrlHENFindFunction("sceAudio_Driver", "sceAudio", 0x2D53F36E);

	//int sceAudioSRCOutputBlocking(int vol, void *buf);
	orig_funcs[5] = sctrlHENFindFunction("sceAudio_Driver", "sceAudio", 0xE0727056);

	sctrlHENPatchSyscall(orig_funcs[0], sceAudioOutput_patched);
	sctrlHENPatchSyscall(orig_funcs[1], sceAudioOutputBlocking_patched);
	sctrlHENPatchSyscall(orig_funcs[2], sceAudioOutputPanned_patched);
	sctrlHENPatchSyscall(orig_funcs[3], sceAudioOutputPannedBlocking_patched);
	sctrlHENPatchSyscall(orig_funcs[4], sceAudioOutput2OutputBlocking_patched);
	sctrlHENPatchSyscall(orig_funcs[5], sceAudioSRCOutputBlocking_patched);

	sceAudioOutput_orig = (sceAudioOutput_orig_t)sctrlHENFindFunction("sceAudio_Driver", "sceAudio", 0x8C1009B2);
	sctrlHENPatchSyscall((u32)sceAudioOutput_orig, sceAudioOutput_patched); 

	sceAudioOutputBlocking_orig = (sceAudioOutputBlocking_orig_t)sctrlHENFindFunction("sceAudio_Driver", "sceAudio", 0x136CAF51);
	sctrlHENPatchSyscall((u32)sceAudioOutputBlocking_orig, sceAudioOutputBlocking_patched); 

	sceAudioOutputPanned_orig = (sceAudioOutputPanned_orig_t)sctrlHENFindFunction("sceAudio_Driver", "sceAudio", 0xE2D56B2D);
	sctrlHENPatchSyscall((u32)sceAudioOutputPanned_orig, sceAudioOutputPanned_patched); 

	sceAudioOutputPannedBlocking_orig = (sceAudioOutputPannedBlocking_orig_t)sctrlHENFindFunction("sceAudio_Driver", "sceAudio", 0x13F592BC);
	sctrlHENPatchSyscall((u32)sceAudioOutputPannedBlocking_orig, sceAudioOutputPannedBlocking_patched);

	sceAudioOutput2OutputBlocking_orig = (sceAudioOutput2OutputBlocking_orig_t)sctrlHENFindFunction("sceAudio_Driver", "sceAudio", 0x2D53F36E);
	sctrlHENPatchSyscall((u32)sceAudioOutput2OutputBlocking_orig, sceAudioOutput2OutputBlocking_patched); 

	sceAudioSRCOutputBlocking_orig = (sceAudioSRCOutputBlocking_orig_t)sctrlHENFindFunction("sceAudio_Driver", "sceAudio", 0xE0727056);
	sctrlHENPatchSyscall((u32)sceAudioSRCOutputBlocking_orig, sceAudioSRCOutputBlocking_patched);  
}

