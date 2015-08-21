#ifndef __SCREENSHOT_H__
#define __SCREENSHOT_H__

#define MAX_THREAD	64

int nkLoad();
int nkThreadSuspend(SceUID thId);
int nkThreadResume(SceUID thId);
void screenshot_BMP(const char* filename, char imageSize);
void screenshot(void);

char file[64];

#endif

