/*
 *  Copyright (C) 2006 Zx / Ludovic Jacomme (ludovic.jacomme@gmail.com)
 */

#include "std.h"
//#include <pspumd.h>
# include "psp_init.h"
# include "nlh.h"

#include <stdarg.h>

  char *psp_home_dir = (char *)0;

# ifdef DEBUG
  static FILE *pspDebugFd = (FILE *)0;

void 
pspDebugPrintf(const char *Format, ...)
{
  va_list  Args;
  va_start( Args, Format );
  if (pspDebugFd == (FILE *)0) {
    pspDebugFd = fopen("ms0:/psp/game/debug.log", "w");
    if (! pspDebugFd) {
      pspDebugScreenPrintf("can\'t open debug file !\n");
      sceKernelExitGame();
    }
  }

  vfprintf(pspDebugFd, Format, Args);
  va_end( Args );
}
# endif

char *
my_dirname(char *filename)
{
  char *dirname = (char *)0;

  int index = strlen(filename);
  while (index >= 0) {
    if (filename[index] == '/') break;
    index--;
  }
  if (index >= 0) {
    dirname = (char *)malloc(sizeof(char) * (index + 1));
    strncpy(dirname, filename, index);
    dirname[index] = '\0';

  } else {
    dirname = (char *)malloc(sizeof(char) * 2);
    dirname[0] = '.';
    dirname[1] = '\0'; 
  }

  return dirname;
}

void
kbd_wait_start(void)
{
  while (1)
  {
    SceCtrlData c;
    sceCtrlReadBufferPositive(&c, 1);
    if (c.Buttons & PSP_CTRL_START) break;
  }
}

int
psp_exit(int status)
{
  if (status) {
    pspDebugScreenPrintf("exit with status %d!\n Press Start to contine\n", status);
    kbd_wait_start();
  } else {
  }
# ifdef DEBUG
  if (pspDebugFd) {
    fclose(pspDebugFd);
    pspDebugFd = (FILE *)0;
  }
# endif
  sceKernelExitGame();
  return 0;
}

int
psp_exit_callback(int arg1, int arg2, void *arg3)
{
  pspDebugScreenPrintf("exit call back call !\n");
  psp_exit(0);

  return 0;
}

int
psp_power_callback(int arg1, int arg2, void *arg3)
{
  pspDebugScreenPrintf("power call back call !\n");
  psp_exit(0);

  return 0;
}

int 
psp_callback_thread(SceSize args, void *argp)
{
  int cid;

  cid = sceKernelCreateCallback("Exit Call Back", psp_exit_callback, NULL);
  sceKernelRegisterExitCallback(cid);
  cid = sceKernelCreateCallback("Power Call Back", psp_power_callback, NULL);
  //scePowerRegisterCallback(0,cid);

	sceKernelSleepThreadCB();
  
  return 0;
}

int
psp_setup_callbacks()
{
	int thid = 0;

	thid = sceKernelCreateThread("update_thread", psp_callback_thread, 0x11, 0xFA0, 0, 0);
	if(thid >= 0) {
		sceKernelStartThread(thid, 0, 0);
	}
	return thid;
}

/*void
psp_init_stuff(int argc, char **argv)
{
  int check_umd;

  psp_setup_callbacks();

  psp_home_dir = my_dirname(argv[0]);

	sceCtrlSetSamplingCycle(0);
	sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);

  sceIoUnassign("disc0:");
  sceIoAssign("disc0:", "umd0:0,0", "umdfat0:", 0, NULL, 0);

  check_umd = sceUmdCheckMedium(0);
  if (check_umd) {
    sceUmdActivate(1,"disc0:");
    sceUmdWaitDriveStat(UMD_WAITFORINIT);
  }
}*/

