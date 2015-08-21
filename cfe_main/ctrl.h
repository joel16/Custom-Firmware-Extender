/*------------------------------------------------------
	Project:		PSP-CFW*
	Module:			CFW_CTRL
	Maintainer:		
------------------------------------------------------*/

#ifndef _CFW_CTRL_
#define _CFW_CTRL_

#include <pspctrl.h>

#include "type.h"

/*	wait a key mask
	@param keymask - The keymask to be waited.
	@returns a key mask pressed*/
extern u32 ctrlWaitMask(u32 keymask);

#endif /*_CFW_CTRL_*/
