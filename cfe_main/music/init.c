#include <pspsdk.h>
#include <psppower.h>
#include <pspsysmem.h>
#include <pspsysmem_kernel.h>
#include <pspkernel.h>
#include <string.h>
#include <pspaudio.h>
#include <pspkerror.h>
#include "playback.h"

#include "init.h"
#include "hw.h"

/* New FindProc based on tyranid's psplink code. PspPet one doesn't work
   well with 2.7X+ sysmem.prx */
u32 FindProc(const char* szMod, const char* szLib, u32 nid)
{
	struct SceLibraryEntryTable *entry;
	void *entTab;
	int entLen;
	SceModule * pMod = ( SceModule * )sceKernelFindModuleByName(szMod);

	if (!pMod)
        return 0;
	
	int i = 0;

	entTab = pMod->ent_top;
	entLen = pMod->ent_size;

	while(i < entLen)
	{
		int count;
		int total;
		unsigned int *vars;

		entry = (struct SceLibraryEntryTable *) (entTab + i);
        	if(entry->libname && !strcmp(entry->libname, szLib))
		{
			total = entry->stubcount + entry->vstubcount;
			vars = entry->entrytable;
			if(entry->stubcount > 0)
			{
				for(count = 0; count < entry->stubcount; count++)
				{
					if (vars[count] == nid)
						return vars[count+total];					
				}
			}
		}

		i += (entry->len * 4);
	}

	return 0;
}

