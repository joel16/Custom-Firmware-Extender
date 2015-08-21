	.set noreorder

#include "pspimport.s"

	IMPORT_START	"sceAudio_driver",0x00010000
	IMPORT_FUNC "sceAudio_driver",0xB282F4B2,sceAudio_driver_B282F4B2
	IMPORT_FUNC "sceAudio_driver",0x669D93E4,sceAudio_driver_669D93E4
	IMPORT_FUNC "sceAudio_driver",0x138A70F1,sceAudio_driver_138A70F1
	IMPORT_FUNC "sceAudio_driver",0x43645E69,sceAudio_driver_43645E69
