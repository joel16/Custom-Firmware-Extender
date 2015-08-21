#ifndef MAIN_H
#define MAIN_H

#include <pspctrl.h>
#include <pspkernel.h>

int scePower_driver_9B1A9C5F(void);
int scePower_driver_E65F00BD(void);
int sceCtrl_driver_C4AAD55F(SceCtrlData *pad_data, int count);

int sceDisplay_driver_820C6038(int pri, void **topaddr, int *bufferwidth, int *pixelformat, int sync);
int sceDisplay_driver_E56B11BA(void **topaddr, int *bufferwidth, int *pixelformat, int sync);
int sceDisplay_driver_4AB7497F(void *topaddr, int bufferwidth, int pixelformat, int sync);
int sceDisplay_driver_DEA197D4(int *pmode, int *pwidth, int *pheight);
int sceDisplay_driver_B685BA36(void); // sceDisplayWaitVblankStart

int sceUsb_driver_AE5DE6AF(const char* driverName, int size, void *args);
int sceUsb_driver_586DB82C(u32 pid);

int scePower_driver_FDB5BFE9(void);
int scePower_driver_1688935C(void);

#endif
