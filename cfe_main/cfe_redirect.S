	.set noreorder

#include "pspimport.s"

// Build files
// cfe_redirect_0000.o cfe_redirect_0001.o 

#ifdef F_cfe_redirect_0000
	IMPORT_START "cfe_redirect",0x40090000
#endif
#ifdef F_cfe_redirect_0001
	IMPORT_FUNC  "cfe_redirect",0xE8D15EFB,redirect
#endif
