#ifndef __UTILS_H
#define __UTILS_H

#define MAX_ARGS 512
#define RJLITE_DRIVERNAME "RJLiteDriver"
#define RJLITE_DRIVERPID  (0x1C9)

#define PSP_USBBUS_DRIVERNAME "USBBusDriver"
#define PSP_USBSTOR_DRIVERNAME "USBStor_Driver"
#define PSP_USB_ACTIVATED              0x200

#define HOSTFSDRIVER_NAME "USBHostFSDriver"
#define HOSTFSDRIVER_PID  (0x1C9)

u32 state;

void copy_file(const char *read_loc, const char *write_loc);
int file_exist(char *filename);
void unloadUsbhost();
int usbmodloaded;
void unloadUsb();
int loadUsb(void);
int startRemotejoy();
int stopRemotejoy();
int loadUsbHost();
void patch_audio();
int unloadStopModule(char *modname);
int loadStartModule(const char *name, int argc, char **argv);
SceUID loadStartModuleWithArgs(const char *filename, int mpid, int argc, char * const argv[]);
int build_args(char *args, const char *execfile, int argc, char **argv);

int channel_volume[8];

#endif
