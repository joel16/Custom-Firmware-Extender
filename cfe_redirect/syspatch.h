/*
 *	syspatch.h is part of HostCore
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
 *	Date Created:	2008-05-15
 */

#pragma once

extern int fw_version;

extern int initPatches( void );

extern unsigned int getFindDriverAddr( void );

extern void getCtrlNids( unsigned int * nid );

extern void getUtilsNids( unsigned int * nid );

extern unsigned int getKillMutexNid();

extern void getDisplayNids( unsigned int * nid );

extern void patchMemPartitionInfo();

extern void restoreLoadExecVSHCommon();

extern void * patchLoadExecVSHCommon( void * func );

extern void wifiModulesPatch1();

extern void wifiModulesPatch2();

extern void wifiModulesPatch3();
