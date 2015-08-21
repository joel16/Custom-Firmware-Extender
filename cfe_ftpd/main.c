#include <pspkernel.h>
#include <pspdebug.h>
#include <pspsdk.h>
#include <pspuser.h>
#include <pspctrl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <psputility_netmodules.h>
#include <psputility_netparam.h>
#include <pspwlan.h>
#include <pspnet.h>
#include <pspnet_apctl.h>

#include "std.h"
#include "ftp.h"
#include "ftpd.h"
#include "psp_init.h"
#include "blit.h"
#include "conf.h"

#define THREAD_PRIORITY 8

PSP_MODULE_INFO("cfe_ftpd", 0, 1, 1);
PSP_MAIN_THREAD_ATTR(0);PSP_MAIN_THREAD_STACK_SIZE_KB(32);

int pspBrowserLoaded=0;

char szMyIPAddr[32];
int display_thid;
int kill_display= 0;

char msg_ftp_on[40];
int ip_ok = 0;
int msg_load_network =0;
int msg_connecting =0;

int msg_error_common = 0;
int msg_error_inet = 0;
int msg_error_sdkinit = 0;

CONFIGFILE config;

int display_thread(SceSize args, void *argp)
{
    while(1) {
	if(kill_display) {
		sceKernelExitDeleteThread(0);
	}

	if(msg_load_network) {
		blit_string(1,32,"Loading network modules...",0xffffff,0x000000);
	}

	if(msg_connecting) {
		blit_string(1,32,"Connecting to acces point...",0xffffff,0x000000);
	}

	if(msg_error_common) {
		blit_string(1,32,"common",0xffffff,0x000000);
	}
	if(msg_error_inet) {
		blit_string(1,32,"inet",0xffffff,0x000000);
	}
	if(msg_error_sdkinit) {
		blit_string(1,32,"sdkinit",0xffffff,0x000000);
	}

	if(ip_ok) {
        blit_string(1,32,msg_ftp_on,0xffffff,0x000000);
		blit_string(1,33,"Press circle to unload",0xffffff,0x000000);
	}

       sceKernelDelayThread(2000);
    }
}

typedef struct sDevCtl
{	s32 max_clusters;
	s32 free_clusters;
	s32 max_sectors; 
	s32 sector_size;
	s32 sector_count;
} sDevCtl;

typedef struct sDevCommand
{
	sDevCtl * p_dev_inf;
} sDevCommand;


int connect_to_apctl(int config) {

	int err;
	int stateLast = -1;

	msg_load_network=0;
	msg_connecting=1;

	if (sceWlanGetSwitchState() != 1) blit_string(1,32,"Turn on Wlan switch",0xffffff,0x000000);

	while (sceWlanGetSwitchState() != 1) {
		sceKernelDelayThread(1000 * 1000);
	}

	err = sceNetApctlConnect(config);
	if (err != 0) {
		blit_string(1,32,"sceNetApctlConnect error",0xffffff,0x000000);
    		return 0;
	}

	while (1) {
		int state;
		err = sceNetApctlGetState(&state);

		if (err != 0) {
			blit_string(1,32,"sceNetApctlGetState error",0xffffff,0x000000);
      			break;
    		}

    		if (state != stateLast) {
			blit_string(1,32,"Connecting to Ap",0xffffff,0x000000);
      			stateLast = state;
    		}

    		if (state == 4) {
      			break;
    		}
    		sceKernelDelayThread(50 * 1000);
  	}

	blit_string(1,32,"Connected",0xffffff,0x000000);
  	sceKernelDelayThread(3000 * 1000);

	if (err != 0) {
		return 0;
	}

	msg_connecting=0;
  	return 1;
}

char *getconfname(int confnum) {
	static char confname[128];

	sceUtilityGetNetParam(confnum, PSP_NETPARAM_NAME, (netData *)confname);
	return confname;
}

static void
net_thread(void) {

	int selComponent = 1;

	//do {
	if(connect_to_apctl(selComponent)) {
		if (sceNetApctlGetInfo(8, szMyIPAddr) != 0)
			strcpy(szMyIPAddr, "unknown IP address");
			strcpy(msg_ftp_on,"Ftp server iP => ");
			strcat(msg_ftp_on,szMyIPAddr);
			ip_ok=1;
			ftpdLoop(szMyIPAddr);
		}
	//} 
//while(0);
	sceKernelExitDeleteThread(0);
	//sceKernelSleepThread();
}

void
user_thread(SceSize args, void *argp) {
	net_thread();
}

SceUID thids[50];
int thid_count = 0;
int i;
SceKernelThreadInfo thinfo;


int unloadNetworkModules(void) {
	int err;

	sceKernelDelayThread(50000); 

	pspSdkInetTerm();
   	sceNetApctlDisconnect();
   	sceNetApctlTerm(); 

	sceKernelDelayThread(1000);
 
	err = sceUtilityUnloadNetModule(PSP_NET_MODULE_INET);
	if (err != 0) {
		blit_string(1,32,"Could not unload PSP_NET_MODULE_INET",0xffffff,0x000000);
    		return 1;
	}
	sceKernelDelayThread(1000); 

	err = sceUtilityUnloadNetModule(PSP_NET_MODULE_COMMON);
	if (err != 0) {
		blit_string(1,32,"Could not unload PSP_NET_MODULE_COMMON",0xffffff,0x000000);
		return 1;
	}

	return 0;
}


int InitialiseNetwork(void) {

	int err;
	msg_load_network=1;

			err = sceUtilityLoadNetModule(PSP_NET_MODULE_COMMON);
			if (err != 0) {
				blit_string(1,32,"Could not load PSP_NET_MODULE_COMMON",0xffffff,0x000000);
				msg_load_network=0;
				msg_error_common=1;
				return 0;
			}

			err = sceUtilityLoadNetModule(PSP_NET_MODULE_INET);
			if (err != 0) {
				blit_string(1,32,"Could not load PSP_NET_MODULE_INET",0xffffff,0x000000);
				msg_load_network=0;
				msg_error_inet=1;
				return 0;
			}

			err = pspSdkInetInit();
			if (err != 0) {
				blit_string(1,32,"Could not initialise the network",0xffffff,0x000000);
				msg_load_network=0;
				msg_error_sdkinit=1;
				return 0;
			}
  	return 0;
}

int main(int argc, char* argv[])
{

	if(!sceWlanGetSwitchState()) {
		int i=0;
		while(1)
		{
			blit_string(1,32,"Wlan switch is OFF",0xffffff,0x000000);
			sceKernelDelayThread(1000);
			i++;
			if(i==400) break;
		}
		sceKernelSelfStopUnloadModule(1, 0, NULL);
	}
	
	int user_thid;
	unsigned int paddata_old = 0;
	SceCtrlData paddata;

	display_thid = sceKernelCreateThread("cfe_ftpd_display", display_thread, THREAD_PRIORITY, 0x04000, 0, NULL);
	if(display_thid >= 0) sceKernelStartThread(display_thid, NULL, NULL);

	if (InitialiseNetwork() != 0) {
		sceKernelSleepThread();
	}
	sceKernelDelayThread(1000); 

	read_config("ms0:/seplugins/cfe/cfe.config", &config);
	//psp_read_config();

	sceKernelDelayThread(1000); 

	user_thid = sceKernelCreateThread( "cfe_ftpd_main", 
		(SceKernelThreadEntry)user_thread, 0x8, 256*1024, PSP_THREAD_ATTR_USER, 0 );
	if(user_thid >= 0) {
		sceKernelStartThread(user_thid, 0, 0);
		//sceKernelWaitThreadEnd(user_thid, NULL);  
	}

	while(1){

		sceKernelDelayThread(5*2000);//needs less running time

		sceCtrlPeekBufferPositive(&paddata, 1);
		
		if(paddata.Buttons != paddata_old){
			if(paddata.Buttons & PSP_CTRL_CIRCLE){
				kill_display=1;
				// Dont kill network modules if psp browser was loaded
				//if(!pspBrowserLoaded) 
				unloadNetworkModules();
	
				//sceKernelExitDeleteThread(0);
				sceKernelSelfStopUnloadModule(0, 0, NULL);
			}
		}

		paddata_old = paddata.Buttons;
		sceKernelDelayThread(10000);
	}
	sceKernelExitDeleteThread(0);
	return 0;
}
