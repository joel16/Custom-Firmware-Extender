/*
 *	tinyui.c is part of HostCore
 *	Copyright (C) 2008  Poison
 *
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 *	Description:	
 *	Author:			Poison <hbpoison@gmail.com>
 *	Date Created:	2008-04-11
 */

#include <pspkernel.h>
#include <pspdisplay.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "utils.h"
#include "syspatch.h"
#include "tinyui.h"

#define A( color ) ( unsigned char )( color >> 24 )

static int buffer_width, pixel_format, end = 1;
static unsigned int * vram = NULL;

inline void blendColorPixel( unsigned int * pixel, unsigned int color )
{
	unsigned char alpha = A( color );
	if ( alpha == 0 )
		return;
	if ( alpha == 0xff )
	{
		*pixel = color;
		return;
	}
	unsigned int c1, c2;
	c1 = color & 0xff00ff;
	c2 = color & 0x00ff00;
	c1 = ( ( c1 * alpha ) >> 8 ) & 0xff00ff;
	c2 = ( ( c2 * alpha ) >> 8 ) & 0x00ff00;
	color = c1 + c2;
	alpha = 0xff - alpha;
	c1 = *pixel & 0xff00ff;
	c2 = *pixel & 0x00ff00;
	c1 = ( ( c1 * alpha ) >> 8 ) & 0xff00ff;
	c2 = ( ( c2 * alpha ) >> 8 ) & 0x00ff00;
	*pixel = c1 + c2 + color;
}

int ( * getFrameBuf )( void ** topaddr, int * bufferwidth, int * pixelformat, int * unk1 );
int ( * waitVblank )( void );

int initTinyUi()
{
	unsigned int nid[2];
	getDisplayNids( nid );
	getFrameBuf = ( void * )findProc( "sceDisplay_Service", "sceDisplay_driver", nid[0] );
	waitVblank = ( void * )findProc( "sceDisplay_Service", "sceDisplay_driver", nid[1] );
	getFrameBuf( ( void ** )&vram, &buffer_width, &pixel_format, NULL );
	if ( buffer_width == 0 || pixel_format != PSP_DISPLAY_PIXEL_FORMAT_8888 )
		return -1;
	return 0;
}

void updateFrameBuf()
{
	getFrameBuf( ( void ** )&vram, &buffer_width, &pixel_format, NULL );
	vram = ( unsigned int * )( ( unsigned int )vram | 0x40000000 );
}

RawImageBlitParams * opts;

int blit_thread( SceSize args, void *argp )
{
	while( opts->sema )
	{
		updateFrameBuf();
		int i, j;
		for( i = 0; i < opts->width; i ++ )
		{
			for( j = 0; j < opts->height; j ++ )
			{
				blendColorPixel( vram + ( opts->y + j ) * buffer_width + opts->x + i, opts->data[j * opts->width + i] );
			}
		}
		waitVblank();
	}
	end = 1;
	return sceKernelExitDeleteThread( 0 );
}

int blitRawImage( RawImageBlitParams * params )
{
	while ( !end )
	{
		waitVblank();
	}
	end = 0;
	params->sema = 1;
	opts = params;
	int bth_id = sceKernelCreateThread("blit_thread", blit_thread, 0xa, 0x800, 0, NULL);
	if( bth_id >= 0 )
		sceKernelStartThread( bth_id, 0, NULL );
	return 0;
}
