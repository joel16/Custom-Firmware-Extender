/*
 *	conf.c is part of HostCore
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
#include <pspsdk.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "conf.h"
#include "utils.h"
#include "log.h"

#define CONF_FILE "ms0:/HostCore/conf.txt"

HostCoreConf * readConf( HostCoreConf * hc_conf )
{
	const char * labels[] =
	{
		"Quick Key",
		"IP",
		"Port",
		"Password",
		"Connection",
		"Block Size",
	};
	const int items_count = 6;
	char buf[128];
	int fd = sceIoOpen( CONF_FILE, PSP_O_RDONLY, 0644 );
	memset( hc_conf, 0, sizeof( HostCoreConf ) );
	if ( fd >= 0 )
	{
		while( readLine( fd, buf, 128 ) >= 0 )
		{
			stripSpace( buf );
			if ( buf[0] == '#' || buf[0] == 0 )
				continue;
			int i;
			for ( i = 0; i < items_count; i ++ )
			{
				if ( !strncmp( labels[i], buf, strlen( labels[i] ) ) )
				{
					char * start = strchr( buf, '"' );
					if ( !start )
						continue;
					char * end = strchr( start + 1, '"' );
					if ( !end )
						continue;
					*end = 0;
					if ( strlen( start + 1 ) < 16 )
						strcpy( ( char * )hc_conf + ( 16 * i ), start + 1 );
				}
			}
		}
		sceIoClose( fd );
	}
	else
	{
		log( "Error opening conf\n" );
	}
	if ( hc_conf->key[0] == 0 )
		strcpy( hc_conf->key, "800000" );
	if ( hc_conf->ip[0] == 0 )
		strcpy( hc_conf->ip, "192.168.0.1" );
	if ( hc_conf->port[0] == 0 )
		strcpy( hc_conf->port, "7513" );
	if ( hc_conf->entry[0] == 0 )
		strcpy( hc_conf->entry, "1" );
	if ( hc_conf->entry[0] == 0 )
		strcpy( hc_conf->blocksize, "2048" );
	return hc_conf;
}
