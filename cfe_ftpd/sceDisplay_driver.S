	.set noreorder

#include "pspimport.s"

	IMPORT_START	"sceDisplay_driver",0x00010000
	IMPORT_FUNC	"sceDisplay_driver",0x820C6038,sceDisplay_driver_820C6038 // sceDisplayGetFrameBufferInternal
	IMPORT_FUNC	"sceDisplay_driver",0x643F3841,sceDisplay_driver_643F3841 // sceDisplaySetFrameBufferInternal
	IMPORT_FUNC	"sceDisplay_driver",0xE56B11BA,sceDisplay_driver_E56B11BA //sceDisplayGetFrameBuf
	IMPORT_FUNC	"sceDisplay_driver",0x4AB7497F,sceDisplay_driver_4AB7497F // sceDisplaySetFrameBuf
	IMPORT_FUNC	"sceDisplay_driver",0xDEA197D4,sceDisplay_driver_DEA197D4 //sceDisplayGetMode
	IMPORT_FUNC	"sceDisplay_driver",0xB685BA36,sceDisplay_driver_B685BA36 // sceDisplayWaitVblankStart
	IMPORT_FUNC	"sceDisplay_driver",0x1CB8CB47,sceDisplay_driver_1CB8CB47 //sceDisplayGetBrightness
 	IMPORT_FUNC	"sceDisplay_driver",0x776ADFDB,sceDisplay_driver_776ADFDB //sceDisplayGetBrightness
 	IMPORT_FUNC	"sceDisplay_driver",0xE8466BC2,sceDisplay_driver_E8466BC2 // sceDisplayGetVcount
