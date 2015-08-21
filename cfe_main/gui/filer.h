#ifndef __FILER_H__
#define __FILER_H__

#define MAXDEPTH 16 // Nombre max de ..
#define MAXDIRNUM 256 //Nombre max de fichiers affichable dans un dir
#define MAXPATH 21

SceUID dlist_mem;

//#ifdef GAME
//SceIoDirent *dlist;
//#else
SceIoDirent		dlist[MAXDIRNUM];
//#endif
int			dlist_num;
char			now_path[MAXPATH];
char			target[MAXPATH];
char			path_tmp[MAXPATH];
int			dlist_start;
int			dlist_curpos;
int			cbuf_start[MAXDEPTH];
int			cbuf_curpos[MAXDEPTH];
int			now_depth;

int noDir;

int filer_on();

SceUID loadStartModuleWithArgs(const char *filename, int mpid, int argc, char * const argv[]);

#endif

