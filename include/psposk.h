#ifndef SONYOSK__
#define SONYOSK__

/******************************************
  *	    	       psposk - Sony OSK - v1.4			*
  *		     PEB For Xtremlua.com				*
  *		     	http://bdpp.c.la				*
  *									*
  *		More information on ps2dev.org			*
  ******************************************
  *		Last UpDate :29/02/08				*
  ******************************************/
  
  
//#define	COMPOSK	// Don't define this!
#ifdef COMPOSK
	#include <pspdisplay.h>
	#include <pspkernel.h>
	#include <psputility.h>
	#include <psputility_osk.h>
	#include <pspgu.h>
	#include <pspgum.h>
	#include <png.h>
	#include "gu/graphics.h"

	#include <stdio.h>
	#include <string.h>
	#include <stdlib.h>
#endif
	
  
#define NONE		0

/**  Color  **/
#define GRIS		0xff918f8c
#define TURCOISE	0xffd0d95c
#define NOIR		0xff000000
#define BLANC		0xffffffff
#define BLEU		0x00ff0000
#define JAUNE		0xff5bb3e0
#define ROUGE		0x000000ff
#define VERT		0x0000ff00
#define VERT2		0xff8ed95c
#define VIOLET		0xffd143c9

/**  Define for InfoPsp() function  **/
#define PSEUDO				1
#define ADHOC_CHANNEL		2
#define WLAN_POWERSAVE		3
#define DATE_FORMAT			4
#define TIME_FORMAT			5
#define TIMEZONE			6
#define DAYLIGHTSAVINGS		7
#define LANGUAGE			8



/** SonyOskGU : You must call this function, and only this function! it's very easy!
  * @param : input : text returned by SonyOskGu
  * @param : color : color of background
  * @param : intext :  If you want add text in the box. If you don't want, just write NONE
  *			exemple : 	unsigned short intext[128] = {'E','n','t','r','e','z',' ','v','o','t','r','e',' ','t','e','x','t','e',0};
  *					SonyOskGu(input,BLEU,intext,NONE);
  * @param : desc :  If you want add description. If you don't want, just write NONE
  *			exemple : 	unsigned short desc[128] = {'D','e','s','c','r','i','p','t','i','o','n',0};
  *					SonyOskGu(input,BLEU,NONE,desc);
  */
int SonyOskGu(char *input,u32 color,char *intext,char *desc);


/** SonyOskPng : It's the same function to SonyOskGu but you can add background or transparency background. Don't require png's lib!
  * @param : input : text returned by SonyOskGu
  * @param : background : your image like that : "background.png"
  * @param : intext :  If you want add text in the box. If you don't want, just write NONE
  *			exemple : 	unsigned short intext[128] = {'E','n','t','r','e','z',' ','v','o','t','r','e',' ','t','e','x','t','e',0};
  *					SonyOskGu(input,BLEU,intext,NONE);
  * @param : desc :  If you want add description. If you don't want, just write NONE
  *			exemple : 	unsigned short desc[128] = {'D','e','s','c','r','i','p','t','i','o','n',0};
  *					SonyOskGu(input,BLEU,NONE,desc);
  */
int SonyOskPng(char *input,char *background,char *intext,char *desc);


/** SonyOskInfoPsp : This function return in input the information of the psp and uther.
 * @param : info :	 PSEUDO
			ADHOC_CHANNEL
			WLAN_POWERSAVE
			DATE_FORMAT
			TIME_FORMAT
			TIMEZONE
			DAYLIGHTSAVINGS
			LANGUAGE
 * @param : input : text returned by SonyOskInfoPsp
  */
int SonyOskInfoPsp(int info,char *input);


#endif


