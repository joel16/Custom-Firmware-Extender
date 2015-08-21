/*
 *  swap.h
 *  
 *
 *  Created by Komlósi Zoltán on 2006.09.09..
 *  Copyright 2006 WATTks All rights reserved.
 *
 */

#define SWAPLL(x) \
( \
	(((x) & 0xff00000000000000LL) >> 8 * 7) \
	| (((x) & 0xff000000000000LL) >> 8 * 5) \
	| (((x) & 0xff0000000000LL) >> 8 * 3) \
	| (((x) & 0xff00000000LL) >> 8 * 1) \
	| (((x) & 0xff000000LL) << 8 * 1) \
	| (((x) & 0xff0000LL) << 8 * 3) \
	| (((x) & 0xff00LL) << 8 * 5) \
	| (((x) & 0xffLL) << 8 * 7) \
)
#define SWAPL(x) (((((unsigned long) x)) >> 24) | (((x) & 0xff0000) >> 8) | (((x) & 0xff00) << 8) | (((x)) << 24))
#define SWAPW(x) ((((x) & 0xff00) >> 8) | (((x) & 0xff) << 8))


