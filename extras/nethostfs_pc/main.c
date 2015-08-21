/*
 * NetHostFS by Dark_AleX
 *
 * Inspired by psplink
 *
 * Added reconnection handling		(by AhMan)
 * Added multi-PSP support		(by AhMan)
 * Added multi-threading PSP support	(by AhMan)
 *	 Note: The current multi-threading implementation doesn't handle
 *	       simultaneous updating to global data.  This may not be a
 *	       problem.
 *
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <dirent.h>
#include <utime.h>
#include <time.h>		// AHMAN Added for Linux platform
#include <signal.h>		// AHMAN
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>	// AHMAN Added for Linux platform
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/errno.h>
#include <pthread.h>

#ifdef __CYGWIN__
#include "pspkeyctrl.h"
#include <w32api/windows.h>
#include <w32api/mmsystem.h>
#endif

#if defined _MAXOSX_
#include <netdb.h> 
#endif

#if defined BUILD_BIGENDIAN || defined _BIG_ENDIAN	//Poison Add for OSX
#include "swap.h"
#endif

#include "nethostfs.h"

#define DEBUG_PRINTF(fmt, ...)
//#define DEBUG_PRINTF		printf

#ifndef SOL_TCP			//Poion add
#define SOL_TCP IPPROTO_TCP
#endif

#define MAX_OPENED_FILES		1024
#define MAX_OPENED_DIRS			 256
#define	MAXI_PATH				1024	

int spawn, spawn_idx;

int blk_size = 1492;
int session_delay = 10;
int g_login = 1;
int g_port = 7513;
int verbose = 0;
int rdonly = 0;
#ifdef __CYGWIN__
int keypad_redirect = 0;
int g_keypad = 1;
int g_p2joy = 0;
#endif
char password[12] = "";

typedef struct FileData
{
	char	name[256];
	int		fd;
	int		mode;			
} FileData;

typedef struct DirData
{
	char		name[256];
	SceIoDirent *entries;
	int			pos;
	int			nentries;
	int			opened;
} DirData;

char	rootdir[512];

FileData	files_table[MAX_OPENED_FILES];
DirData		dirs_table[MAX_OPENED_DIRS];

//#define MAX_CLIENTS		20
#define TOTAL_NETSESSIONS	4
typedef struct _clientstruct{
	struct in_addr ip_addr;
	int cid[TOTAL_NETSESSIONS];
	int cidcnt;
	pid_t pid;
	time_t inittime;
} clientstruct;
//} client[MAX_CLIENTS];
//int client_cnt = 0;
int max_clients = 20;
clientstruct *client;

int recv_rcode;
int send_rcode;

#define SEND_INTEGER(x) if ((send_rcode = send(sock, &x, 4, 0)) != 4) { \
				printf("SEND_INTEGER problem not 4 bytes: %d\n", send_rcode); \
				return -1; \
			}
#define RECV_INTEGER(x) if ((recv_rcode = recv(sock, x, 4, 0)) != 4) { \
				if (recv_rcode != 0) \
					printf("RECV_INTEGER problem not 4 bytes: %d\n", recv_rcode); \
				return -1; \
			}
#define RECV_PARAMS() if ((recv_rcode = recv(sock, &params, sizeof(params), 0)) != sizeof(params)) { \
				printf("RECV_PARAMS problem not sizeof(%d): %d\n", (int)sizeof(params), recv_rcode); \
				return -1; \
			}
#define SEND_RETURN(x) res = x; if ((send_rcode = send(sock, &res, 4, 0)) != 4) { \
				printf("SEND_RETURN problem not 4 bytes: %d\n", send_rcode); \
				return -1; \
			}
#define SEND_RESULT() if ((send_rcode = send(sock, &result, sizeof(result), 0)) != sizeof(result)) { \
				printf("RECV_RESULT problem not sizeof(%d): %d\n", (int)sizeof(result), send_rcode); \
				return -1; \
			}
#define MAKE_PATH(s) snprintf(path, MAXI_PATH, "%s%s", rootdir, s)

/* Converts native time to psp time */
void psptime(time_t *t, ScePspDateTime *pspt)
{
	struct tm *filetime;

	memset(pspt, 0, sizeof(ScePspDateTime));
	filetime = localtime(t);
	
	#if defined BUILD_BIGENDIAN || defined _BIG_ENDIAN //OSX
	pspt->year = SWAPL(filetime->tm_year + 1900);
	pspt->month = SWAPL(filetime->tm_mon + 1);
	pspt->day = SWAPL(filetime->tm_mday);
	pspt->hour = SWAPL(filetime->tm_hour);
	pspt->minute = SWAPL(filetime->tm_min);
	pspt->second = SWAPL(filetime->tm_sec);
	#else
	pspt->year = filetime->tm_year + 1900;
	pspt->month = filetime->tm_mon + 1;
	pspt->day = filetime->tm_mday;
	pspt->hour = filetime->tm_hour;
	pspt->minute = filetime->tm_min;
	pspt->second = filetime->tm_sec;
	#endif
}

/* Converts psp time to native time */
void nativetime(ScePspDateTime *pspt, time_t *t)
{
	struct tm stime;
	
	#if defined BUILD_BIGENDIAN || defined _BIG_ENDIAN //OSX
	stime.tm_year = SWAPL(pspt->year) - 1900;
	stime.tm_mon = SWAPL(pspt->month) - 1;
	stime.tm_mday = SWAPL(pspt->day);
	stime.tm_hour = SWAPL(pspt->hour);
	stime.tm_min = SWAPL(pspt->minute);
	stime.tm_sec = SWAPL(pspt->second);
	#else
	stime.tm_year = pspt->year - 1900;
	stime.tm_mon = pspt->month - 1;
	stime.tm_mday = pspt->day;
	stime.tm_hour = pspt->hour;
	stime.tm_min = pspt->minute;
	stime.tm_sec = pspt->second;
	#endif
	
	*t = mktime(&stime);
}

/* Converts native stat to psp stat */
void pspstat(struct stat *st, SceIoStat *pspst)
{
	#if defined BUILD_BIGENDIAN || defined _BIG_ENDIAN //OSX
	pspst->st_size = SWAPLL(st->st_size);
	#else
	pspst->st_size = st->st_size;
	#endif
	pspst->st_mode = 0;
	pspst->st_attr = 0;
	
	if(S_ISLNK(pspst->st_mode))
	{
		#if defined BUILD_BIGENDIAN || defined _BIG_ENDIAN //OSX
		pspst->st_attr = SWAPL(FIO_SO_IFLNK);
		pspst->st_mode = SWAPL(FIO_S_IFLNK);
		#else
		pspst->st_attr = FIO_SO_IFLNK;
		pspst->st_mode = FIO_S_IFLNK;
		#endif
	}
	else if(S_ISDIR(st->st_mode))
	{
		#if defined BUILD_BIGENDIAN || defined _BIG_ENDIAN //OSX
		pspst->st_attr = SWAPL(FIO_SO_IFDIR);
		pspst->st_mode = SWAPL(FIO_S_IFDIR);
		#else
		pspst->st_attr = FIO_SO_IFDIR;
		pspst->st_mode = FIO_S_IFDIR;
		#endif
	}
	else
	{
		#if defined BUILD_BIGENDIAN || defined _BIG_ENDIAN //OSX
		pspst->st_attr = SWAPL(FIO_SO_IFREG);
		pspst->st_mode = SWAPL(FIO_S_IFREG);
		#else
		pspst->st_attr = FIO_SO_IFREG;
		pspst->st_mode = FIO_S_IFREG;
		#endif
	}
	
	#if defined BUILD_BIGENDIAN || defined _BIG_ENDIAN //OSX
	pspst->st_mode |= (st->st_mode & (SWAPL(S_IRWXU) | SWAPL(S_IRWXG) | SWAPL(S_IRWXO)));
	#else
	pspst->st_mode |= (st->st_mode & (S_IRWXU | S_IRWXG | S_IRWXO));
	#endif
	psptime(&st->st_ctime, &pspst->stctime);
	psptime(&st->st_atime, &pspst->statime);
	psptime(&st->st_mtime, &pspst->stmtime);
}

void gentxt(unsigned char *text, int len)
{
	int i;
	
	srandom(time(0));
	for (i = 0; i < len; i++) {
		text[i] = (random() & 0xFF) ^ (random() & 0xFF);
		//printf("%02X", text[i]);
	}
}
		
void psw_encrypt(unsigned char *text, int textlen, unsigned char *key)
{
	int keylen;
	int i, j, b;
	unsigned char a;
	unsigned char lastkey;

	keylen = strlen((char *)key);
	lastkey = key[0];
	j = 0;
	for (i = 0; i < textlen; i++) {
		a = key[j];
		text[i] = (text[i] ^ a) - j;
		b = lastkey % 8;
		text[i] = (text[i] >> b) + ((text[i] << (8-b)) & 0xFF);
		text[i] ^= lastkey + key[keylen - j - 1];
		lastkey = key[j];
		j++;
		if (j == keylen)
			j = 0;
	}
}

int process_hello(int sock)
{
	int res;
	#if defined BUILD_BIGENDIAN || defined _BIG_ENDIAN //OSX
	SEND_RETURN(SWAPL(NET_HOSTFS_CMD_HELLO));
	#else
	SEND_RETURN(NET_HOSTFS_CMD_HELLO);
	#endif
	
	return 0;
}

int process_ioinit(int sock)
{
	int i, res;
	IO_LOGIN_RESPONSE_PARAMS params;
	IO_LOGIN_CHALLENGE_PARAMS result;

	gentxt(result.challenge, CHALLENGE_TEXT_LEN);
	SEND_RESULT();
	RECV_PARAMS();
	psw_encrypt(result.challenge, CHALLENGE_TEXT_LEN, (unsigned char *)password);

	if (g_login == 0) {
		if (memcmp(params.response, result.challenge, CHALLENGE_TEXT_LEN)) {
			printf("Incorrect password\n");
			#if defined BUILD_BIGENDIAN || defined _BIG_ENDIAN //OSX
			SEND_RETURN(SWAPL(-2));
			#else
			SEND_RETURN(-2);
			#endif
			return 0;
		}
	}

	g_login = 1;

	memset(files_table, 0, sizeof(files_table));
	memset(dirs_table, 0, sizeof(dirs_table));

	for (i = 0; i < MAX_OPENED_FILES; i++)
		files_table[i].fd = -1;
	
	//getcwd(rootdir, 512);	

	SEND_RETURN(0);

	return 0;
}

int process_ioexit(int sock)
{
	return -1; // End
}

int process_ioopen(int sock)
{
	IO_OPEN_PARAMS params;
	char path[MAXI_PATH];
	int i, flags, res;
	struct stat filestat;
	int dir;
	
	RECV_PARAMS();

	for (i = 0; i < MAX_OPENED_FILES; i++)
	{
		if (files_table[i].fd < 0)
			break;
	}

	if (i == MAX_OPENED_FILES)
	{
		SEND_RETURN(-1);
		return 0;
	}

	strncpy(files_table[i].name, params.file, 256);	
	MAKE_PATH(files_table[i].name);
	
	flags = 0;
	#if defined BUILD_BIGENDIAN || defined _BIG_ENDIAN //OSX
	params.flags = SWAPL(params.flags);
	#endif

	if (params.flags & PSP_O_RDONLY)
		flags |= O_RDONLY;

	if (params.flags & PSP_O_WRONLY)
		flags |= O_WRONLY;

	if (params.flags & PSP_O_APPEND)
		flags |= O_APPEND;

	if (params.flags & PSP_O_CREAT)
		flags |= O_CREAT;

	if (params.flags & PSP_O_TRUNC)
		flags |= O_TRUNC;

	if (params.flags & PSP_O_EXCL)
		flags |= O_EXCL;	

	dir = 0;
	if (stat(path, &filestat) == 0) {
		if (S_ISDIR(filestat.st_mode))
			dir = 1;
	}
	if (!dir) {
		if (rdonly && (params.flags & (PSP_O_WRONLY|PSP_O_CREAT|PSP_O_TRUNC))) {
			files_table[i].fd = 65535;
		} else {
			#if defined BUILD_BIGENDIAN || defined _BIG_ENDIAN //OSX
			files_table[i].fd = open(path, flags, SWAPL(params.mode));
			#else
			files_table[i].fd = open(path, flags, params.mode);
			#endif
		}
	}
	if (verbose)
		printf("open [%d] sock [%d]: file %s, fd=%d\n", spawn_idx, sock, path, files_table[i].fd);

	if (files_table[i].fd < 0)
	{
		#if defined BUILD_BIGENDIAN || defined _BIG_ENDIAN //OSX
		SEND_RETURN(SWAPL(files_table[i].fd));
		#else
		SEND_RETURN(files_table[i].fd);
		#endif
	}	
	else 
	{
		#if defined BUILD_BIGENDIAN || defined _BIG_ENDIAN //OSX
		SEND_RETURN(SWAPL(i));
		#else
		SEND_RETURN(i); 
		#endif 
	}

	return 0;
}

int process_ioclose(int sock)
{
	int fd, res;

	RECV_INTEGER(&fd);
	
	#if defined BUILD_BIGENDIAN || defined _BIG_ENDIAN //OSX
	fd = SWAPL(fd);
	#endif
	
	if (fd < 0 || fd >= MAX_OPENED_FILES) {
		SEND_RETURN(-1);
	} else if (rdonly && (files_table[fd].fd == 65535)) {
		files_table[fd].fd = -1;
		SEND_RETURN(0);
	} else if (close(files_table[fd].fd) < 0) {
		SEND_RETURN(-1);
	} else {
		files_table[fd].fd = -1;
		SEND_RETURN(0);
	}

	return 0;
}

#ifdef __CYGWIN__
void remove_wsp(char *buf)
{
        int len = strlen(buf);
        int i = 0;

        while(isspace(buf[i]))
        {
                i++;
        }

        if(i > 0)
        {
                len -= i;
                memmove(buf, &buf[i], len + 1);
        }

        if(len <= 0)
        {
                return;
        }

        i = len-1;
        while(isspace(buf[i]))
        {
                buf[i--] = 0;
        }
}

int parse_joymap(int joyno, char *joymap)
{
	FILE *fpt;
	int i;
	int line;
	int butt;
	char *tok, *val;
	char buffer[512];

	if ((fpt = fopen(joymap, "r")) == NULL) {
		fprintf(stderr, "Error: Can't open joystick mapping file <%s>\n", joymap);
		exit(1);
	}

	line = 0;
	while (fgets(buffer, sizeof(buffer), fpt) != NULL) {
		line++;
		remove_wsp(buffer);

		if ((buffer[0] == '#') || (buffer[0] == '\0'))
			continue;
		tok = strtok(buffer, "=");
		val = strtok(NULL, "");
		if ((tok == NULL) || (val == NULL)) {
			fprintf(stderr, "Error: Invalid joystick mapping <%s> at line %d\n", joymap, line);
			exit(1);
		}
		butt = atoi(val);
		for (i = 0; i < TOTAL_PSP_BUTTONS; i++) {
			if (strcasecmp(map_names[i], tok) == 0) {
				jbut[joyno][i] = butt;
				break;
			}
		}
		if (i == TOTAL_PSP_BUTTONS) {
			if (strcasecmp("analog_x", tok) == 0)
				jxanalog[joyno] = butt;
			else if (strcasecmp("analog_y", tok) == 0)
				jyanalog[joyno] = butt;
			else if (strcasecmp("digital_x", tok) == 0)
				jxdigital[joyno] = butt;
			else if (strcasecmp("digital_y", tok) == 0)
				jydigital[joyno] = butt;
			else if (strcasecmp("digital_tol", tok) == 0)
				jtol[joyno] = butt;
			else if (strcasecmp("analog_deadzone", tok) == 0)
				jdeadzone[joyno] = butt;
			else {
				fprintf(stderr, "Error: Unknown token in joystick mapping <%s> at line %d\n", joymap, line);
				exit(1);
			}
		}
	}
	fclose(fpt);
	return(0);
}

void getJoyAxisValue(int axisno, JOYINFOEX *joy, unsigned char *value)
{
	*value = 0x7F;
	switch (axisno) {
		case 1:	*value = joy->dwXpos / 256;
			break;
		case 2: *value = joy->dwYpos / 256;
			break;
		case 3: *value = joy->dwZpos / 256;
			break;
		case 4: *value = joy->dwRpos / 256;
			break;
		case 5: *value = joy->dwUpos / 256;
			break;
		case 6: *value = joy->dwVpos / 256;
			break;
	}
}

int getJoy(int joyno, unsigned int *key, unsigned char *xaxis, unsigned char *yaxis)
{
	int rc;
	JOYINFOEX joy;
	unsigned char val;

	joy.dwSize = sizeof(joy);
	joy.dwFlags = JOY_RETURNALL;
	rc = joyGetPosEx(joyno, &joy);

	if (rc == 0) {
		if (joy.dwButtons & jbut[joyno][CROSS])
			*key |= PSP_CTRL_CROSS;
		if (joy.dwButtons & jbut[joyno][CIRCLE])
			*key |= PSP_CTRL_CIRCLE;
		if (joy.dwButtons & jbut[joyno][SQUARE])
			*key |= PSP_CTRL_SQUARE;
		if (joy.dwButtons & jbut[joyno][TRIANGLE])
			*key |= PSP_CTRL_TRIANGLE;
		if (joy.dwButtons & jbut[joyno][RTRIGGER])
			*key |= PSP_CTRL_RTRIGGER;
		if (joy.dwButtons & jbut[joyno][LTRIGGER])
			*key |= PSP_CTRL_LTRIGGER;
		if (joy.dwButtons & jbut[joyno][START])
			*key |= PSP_CTRL_START;
		if (joy.dwButtons & jbut[joyno][SELECT])
			*key |= PSP_CTRL_SELECT;
		if (joy.dwButtons & jbut[joyno][EXIT])
			*key |= 0x40000000;
		if (joy.dwButtons & jbut[joyno][HOME])
			*key |= PSP_CTRL_HOME;
/*
		if (joy.dwButtons & jbut[joyno][RIGHT])
			*key |= PSP_CTRL_RIGHT;
		if (joy.dwButtons & jbut[joyno][LEFT])
			*key |= PSP_CTRL_LEFT;
		if (joy.dwButtons & jbut[joyno][DOWN])
			*key |= PSP_CTRL_DOWN;
		if (joy.dwButtons & jbut[joyno][UP])
			*key |= PSP_CTRL_UP;
*/

		getJoyAxisValue(jxdigital[joyno], &joy, &val);
		if (val > 0x7F + jtol[joyno])
			*key |= PSP_CTRL_RIGHT;
		else if (val < jtol[joyno])
			*key |= PSP_CTRL_LEFT;
		getJoyAxisValue(jydigital[joyno], &joy, &val);
		if (val > 0x7F + jtol[joyno])
			*key |= PSP_CTRL_DOWN;
		else if (val < jtol[joyno])
			*key |= PSP_CTRL_UP;

		getJoyAxisValue(jxanalog[joyno], &joy, &val);
		if ((val < (unsigned char) 0x7F - jdeadzone[joyno]) || (val > (unsigned char) 0x7F + jdeadzone[joyno]))
			*xaxis = val;
		getJoyAxisValue(jyanalog[joyno], &joy, &val);
		if ((val < (unsigned char) 0x7F - jdeadzone[joyno]) || (val > (unsigned char) 0x7F + jdeadzone[joyno]))
			*yaxis = val;

	}
	return(rc);
}

int getKey(unsigned int *key, unsigned char *xaxis, unsigned char *yaxis)
{
	HWND win;
	BOOL infocus;
	char title[80];

	infocus = FALSE;
	if (g_keypad) {
		win = GetForegroundWindow();
		GetWindowText(win, title, sizeof(title));
		strupr(title);
		if (strstr(title, "NETHOSTFS") || strstr(title, "REMOTEJOYSDL") || strstr(title, "REMOTEJOY4IRS"))
			infocus = TRUE;
	}

	if (infocus) {
		if (GetAsyncKeyState(VK_END) & 0x8000)
			*key |= PSP_CTRL_CROSS;
		if (GetAsyncKeyState(VK_NEXT) & 0x8000)
			*key |= PSP_CTRL_CIRCLE;
		if (GetAsyncKeyState(VK_DELETE) & 0x8000)
			*key |= PSP_CTRL_SQUARE;
		if (GetAsyncKeyState(VK_HOME) & 0x8000)
			*key |= PSP_CTRL_TRIANGLE;
		if (GetAsyncKeyState(VK_PRIOR) & 0x8000)
			*key |= PSP_CTRL_RTRIGGER;
		if (GetAsyncKeyState('Q') & 0x8000)
			*key |= PSP_CTRL_LTRIGGER;
		if (GetAsyncKeyState(VK_RETURN) & 0x8000)
			*key |= PSP_CTRL_START;
		if (GetAsyncKeyState(VK_RSHIFT) & 0x8000)
			*key |= PSP_CTRL_SELECT;
		if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
			*key |= 0x40000000;
		if (GetAsyncKeyState('S') & 0x8000)
			*key |= PSP_CTRL_RIGHT;
		if (GetAsyncKeyState('A') & 0x8000)
			*key |= PSP_CTRL_LEFT;
		if (GetAsyncKeyState('Z') & 0x8000)
			*key |= PSP_CTRL_DOWN;
		if (GetAsyncKeyState('W') & 0x8000)
			*key |= PSP_CTRL_UP;
		if (GetAsyncKeyState('H') & 0x8000)
			*key |= PSP_CTRL_HOME;

		if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
			*xaxis = 255;
		else if (GetAsyncKeyState(VK_LEFT) & 0x8000)
			*xaxis = 0;
		if (GetAsyncKeyState(VK_DOWN) & 0x8000)
			*yaxis = 255;
		else if (GetAsyncKeyState(VK_UP) & 0x8000)
			*yaxis = 0;
	}
	return(0);
}

int process_getkey(int sock)
{
	IO_GETKEY_INPUTPARAMS params;
	IO_GETKEY_PARAMS result;
	int p2type;
	unsigned int key;

	RECV_PARAMS();
	key = 0;
	result.xaxis = 127;
	result.yaxis = 127;
	result.but2 = 0;
	result.xaxis2 = 127;
	result.yaxis2 = 127;
	if (keypad_redirect) {
		p2type = params.p2type;

		key = 0;
		result.xaxis = 127;
		result.yaxis = 127;
		result.but2 = 0;
		result.xaxis2 = 127;
		result.yaxis2 = 127;
		if (p2type == 2) {
			if (g_p2joy) {
				getJoy(0, &key, &result.xaxis, &result.yaxis);
				getKey(&key, &result.xaxis, &result.yaxis);
				getJoy(1, &result.but2, &result.xaxis2, &result.yaxis2);
			} else {
				getJoy(0, &key, &result.xaxis, &result.yaxis);
				getKey(&result.but2, &result.xaxis2, &result.yaxis2);
			}
		} else {
			getJoy(0, &key, &result.xaxis, &result.yaxis);
			getKey(&key, &result.xaxis, &result.yaxis);
		}
	}

	result.res = key;
	//printf("key: %X %d %d\n", result.res, (int) result.xaxis, (int) result.yaxis);
	SEND_RESULT();
	return(0);
}
#else
int process_getkey(int sock)
{
	IO_GETKEY_PARAMS result;

	
	result.res = 0;
	result.xaxis = 127;
	result.yaxis = 127;

	SEND_RESULT();
	return(0);
}
#endif

int process_ioread(int sock)
{
	IO_READ_PARAMS params;
	int bytesread, res;
	char *buf;

	RECV_PARAMS();
	
	#if defined BUILD_BIGENDIAN || defined _BIG_ENDIAN //OSX
	params.fd = SWAPL(params.fd);
	params.len = SWAPL(params.len);
	#endif
	
	// Limit of 32 MB (which anyways it has no sense in the psp)
	if (params.len >= (32 * 1048576))
	{
		SEND_RETURN(-1);
		return 0;
	}
	else if (params.len <= 0)
	{
		SEND_RETURN(0);
		return 0;
	}
	else if (params.fd < 0 || params.fd >= MAX_OPENED_FILES)
	{
		SEND_RETURN(-1);
		return 0;
	}
	else if (files_table[params.fd].fd < 0)
	{
		SEND_RETURN(-1);
		return 0;
	}	

	buf = (char *)malloc(params.len);
	if (!buf)
	{
		SEND_RETURN(-1);
		return 0;
	}

	bytesread = read(files_table[params.fd].fd, buf, params.len);
	if (verbose)
		printf("read [%d] sock [%d]: file size %d\n", spawn_idx, sock, bytesread);
	
	#if defined BUILD_BIGENDIAN || defined _BIG_ENDIAN //OSX
	SEND_RETURN(SWAPL(bytesread));
	#else
	SEND_RETURN(bytesread);
	#endif

	if (bytesread > 0)
	{
		int x = 0, sent = 0;
		char *pbuf = buf;

		while (sent < bytesread)
		{
			int blocksize;

			if ((bytesread - sent) >= blk_size)
				blocksize = blk_size;
			else
				blocksize = bytesread - sent;

			x = send(sock, pbuf, blocksize, 0);
			if (verbose)
				printf("read [%d] sock [%d]: Try to send %d, actual sent %d\n", spawn_idx, sock, blocksize, x);
			
			if (x <= 0)
				return -1;

			sent += x;
			pbuf += x;
		}
	}

	free(buf);
	return 0;
}

int process_iowrite(int sock)
{
	IO_WRITE_PARAMS params;
	int error, x, received, res;
	char *buf, *pbuf;

	error = 0;	//AHMAN
	RECV_PARAMS();
	
	#if defined BUILD_BIGENDIAN || defined _BIG_ENDIAN //OSX
	params.fd = SWAPL(params.fd);
	params.len = SWAPL(params.len);
	#endif
	
	if (params.len >= (32 * 1048576))
	{
		error = 1;
	}
	else if (params.len <= 0)
	{
		SEND_RETURN(0);
		return 0;
	}
	else if (params.fd < 0 || params.fd >= MAX_OPENED_FILES)
	{
		error = 1;
	}
	else if (files_table[params.fd].fd < 0)
	{
		error = 1;
	}

	x = 0;
	received = 0;
	
	if (error)
	{
		buf = (char *)malloc(blk_size);
	}
	else
	{
		buf = (char *)malloc(params.len);
	}

	if (!buf)
		return -1;
			
	pbuf = buf;

	while (received < params.len)
	{
		int blocksize;

		if ((params.len - received) >= blk_size)
			blocksize = blk_size;
		else
			blocksize = params.len - received;
		
		x = recv(sock, pbuf, blocksize, 0);
		if (verbose)
			printf("write [%d] sock [%d]: Try to recv %d, actual recv %d\n", spawn_idx, sock, blocksize, x);
		
		if (x <= 0)
			return -1;

		received += x;

		if (!error)
			pbuf += x;
	}

	if (error)
	{
		SEND_RETURN(-1);
	}

	else
	{
		if (rdonly) {
			#if defined BUILD_BIGENDIAN || defined _BIG_ENDIAN //OSX
			SEND_RETURN(SWAPL(params.len));
			#else
			SEND_RETURN(params.len);
			#endif
		} else {
			#if defined BUILD_BIGENDIAN || defined _BIG_ENDIAN //OSX
			SEND_RETURN(SWAPL(write(files_table[params.fd].fd, buf, params.len)));
			#else
			SEND_RETURN(write(files_table[params.fd].fd, buf, params.len));
			#endif
		}
	}

	free(buf);
	return 0;
}

int process_iolseek(int sock)
{
	IO_LSEEK_PARAMS params;
	int res;

	RECV_PARAMS();
	
	#if defined BUILD_BIGENDIAN || defined _BIG_ENDIAN //OSX
	params.fd = SWAPL(params.fd); 
    params.offset = SWAPLL(params.offset);
	params.whence = SWAPL(params.whence);
	#endif
	
	if (params.fd < 0 || params.fd >= MAX_OPENED_FILES || files_table[params.fd].fd < 0)
	{
		SEND_RETURN(-1);
	}
	else
	{
		#if defined BUILD_BIGENDIAN || defined _BIG_ENDIAN //OSX
		SEND_RETURN(SWAPL(lseek(files_table[params.fd].fd, params.offset, params.whence)));
		#else
		SEND_RETURN(lseek(files_table[params.fd].fd, params.offset, params.whence));
		#endif
	}

	return 0;
}

int process_ioremove(int sock)
{
	IO_REMOVE_PARAMS params;
	char path[MAXI_PATH];
	int res;

	RECV_PARAMS();
	params.file[255] = 0;
	MAKE_PATH(params.file);
	
	if (rdonly) {
		SEND_RETURN(0);
	} else {
		#if defined BUILD_BIGENDIAN || defined _BIG_ENDIAN //OSX
		SEND_RETURN(SWAPL(remove(path)));
		#else
		SEND_RETURN(remove(path));
		#endif
	}
	return 0;
}

int process_iomkdir(int sock)
{
	IO_MKDIR_PARAMS params;
	char path[MAXI_PATH];
	int res;

	RECV_PARAMS();
	params.dir[255] = 0;
	MAKE_PATH(params.dir);
	
	if (rdonly) {
		SEND_RETURN(0);
	} else {
		#if defined BUILD_BIGENDIAN || defined _BIG_ENDIAN //OSX
		SEND_RETURN(SWAPL(mkdir(path, SWAPL(params.mode))));
		#else
		SEND_RETURN(mkdir(path, params.mode));
		#endif
	}
	return 0;
}

int process_iormdir(int sock)
{
	IO_RMDIR_PARAMS params;
	char path[MAXI_PATH];
	int res;

	RECV_PARAMS();
	params.dir[255] = 0;
	MAKE_PATH(params.dir);
	
	if (rdonly) {
		SEND_RETURN(0);
	} else {
		SEND_RETURN(rmdir(path));
	}
	return 0;
}

int process_iodopen(int sock)
{
	IO_DOPEN_PARAMS params;
	char path[MAXI_PATH];
	struct dirent **entries;
	int i, j, n, res;
	int idx;

	RECV_PARAMS();
	
	for (i = 0; i < MAX_OPENED_DIRS; i++)
	{
		if (dirs_table[i].entries == NULL)
			break;
	}

	if (i == MAX_OPENED_DIRS)
	{
		SEND_RETURN(-1);
		return 0;
	}

	MAKE_PATH(params.dir);
	n = scandir(path, &entries, NULL, alphasort);

	if (n < 0)
	{
		#if BUILD_BIGENDIAN //OSX
		SEND_RETURN(SWAPL(n));
		#else
		SEND_RETURN(n);
		#endif
	}
	else
	{
		dirs_table[i].opened = 1;

		if (n != 0)
		{
			dirs_table[i].entries = (SceIoDirent *)malloc(n * sizeof(SceIoDirent));

			if (!dirs_table[i].entries)
			{
				SEND_RETURN(-1);
				return 0;
			}

			dirs_table[i].pos = 0;
			//dirs_table[i].nentries = n;

			for (j = 0, idx = 0; j < n; j++)
			{
				char filepath[MAXI_PATH];
				struct stat st;

				snprintf(filepath, MAXI_PATH, "%s/%s", path, entries[j]->d_name);

				if (stat(filepath, &st) < 0)
				{
					printf("Can't stat file %s\n", filepath);
					//free(dirs_table[i].entries);
					//dirs_table[i].entries = NULL;
					
					//SEND_RETURN(-1);
					//return 0;
				} else {
					memset(&dirs_table[i].entries[idx], 0, sizeof(SceIoDirent));
					strncpy(dirs_table[i].entries[idx].d_name, entries[j]->d_name, 256);
					pspstat(&st, &dirs_table[i].entries[idx].d_stat);
					idx++;
				}
			}
			dirs_table[i].nentries = idx;
		}
		if (verbose)
			printf("dopen [%d] sock [%d]: Open %s\n", spawn_idx, sock, path);
		#if defined BUILD_BIGENDIAN || defined _BIG_ENDIAN //OSX
		SEND_RETURN(SWAPL(i));
		#else
		SEND_RETURN(i);
		#endif
	}

	return 0;
}

int process_iodclose(int sock)
{
	int fd, res;

	RECV_INTEGER(&fd);
	
	#if defined BUILD_BIGENDIAN || defined _BIG_ENDIAN //OSX
	fd = SWAPL(fd);
	#endif

	if (fd < 0 || fd >= MAX_OPENED_DIRS)
	{
		SEND_RETURN(-1);
	}
	else if (!dirs_table[fd].opened)
	{
		SEND_RETURN(-1);
	}
	else if (!dirs_table[fd].entries)
	{
		dirs_table[fd].opened = 0;
		SEND_RETURN(0);
	}
	else
	{
		free(dirs_table[fd].entries);
		dirs_table[fd].entries = NULL;
		dirs_table[fd].opened = 0;
		SEND_RETURN(0);
	}	
	
	return 0;
}

int process_iodread(int sock)
{
	IO_DREAD_RESULT result;
	int fd;

	RECV_INTEGER(&fd);
	
	#if defined BUILD_BIGENDIAN || defined _BIG_ENDIAN //OSX
	fd = SWAPL(fd);
	#endif

	memset(&result, 0, sizeof(result));

	if (fd < 0 || fd >= MAX_OPENED_DIRS)
	{
		result.res = -1;
	}
	else if (!dirs_table[fd].opened)
	{
		result.res = -1;
	}
	else if (!dirs_table[fd].entries)
	{
		result.res = 0;
	}
	else
	{
		int pos = dirs_table[fd].pos;
		
		if (pos >= dirs_table[fd].nentries)
		{
			result.res = 0; // No more entries
		}
		else
		{
			#if defined BUILD_BIGENDIAN || defined _BIG_ENDIAN //OSX
			result.res = SWAPL(pos+1);
			#else
			result.res = pos+1;
			#endif
			memcpy(&result.entry, &dirs_table[fd].entries[pos], sizeof(SceIoDirent));
			dirs_table[fd].pos++;
		}
	}

	SEND_RESULT();
	return 0;
}

int	process_iogetstat(int sock)
{
	IO_GETSTAT_PARAMS params;
	IO_GETSTAT_RESULT result;
	char path[MAXI_PATH];
	struct stat st;

	RECV_PARAMS();
	MAKE_PATH(params.file);

	memset(&result, 0, sizeof(result));
	memset(&st, 0, sizeof(st));

	if (stat(path, &st) < 0)
	{
		result.res = -1;
	}
	else
	{
		result.res = 0;
		pspstat(&st, &result.stat);		
	}

	SEND_RESULT();
	return 0;
}

int process_iochstat(int sock)
{
	IO_CHSTAT_PARAMS params;
	char path[MAXI_PATH];
	int res;

	RECV_PARAMS();
	
	#if defined BUILD_BIGENDIAN || defined _BIG_ENDIAN //OSX
	params.bits = SWAPL(params.bits);
	params.stat.st_mode = SWAPL(params.stat.st_mode);
	#endif
	
	MAKE_PATH(params.file);

	if (!rdonly) {
		if (params.bits & PSP_CHSTAT_MODE)
		{
			if (chmod(path, params.stat.st_mode & (FIO_S_IRWXU | FIO_S_IRWXG | FIO_S_IRWXO)) < 0)
				SEND_RETURN(-1);
		}
	}

	SEND_RETURN(0);
	return 0;
}

int process_iorename(int sock)
{
	IO_RENAME_PARAMS params;
	char oldpath[MAXI_PATH], newpath[MAXI_PATH];
	char destpath[MAXI_PATH];
	int res;

	RECV_PARAMS();
	
	if (rdonly) {
		SEND_RETURN(0);
	} else {
		// AHMAN  Added rename fix as in usbhost0:
		/* If the old path is absolute and the new path is relative then rebase newpath */
		if((params.oldfile[0] == '/') && (params.newfile[0] != '/'))
		{
			char *slash;
			
			strcpy(destpath, params.oldfile);
			/* No need to check, should at least stop on the first slash */
			slash = strrchr(destpath, '/');
			/* Nul terminate after slash */
			*(slash+1) = '\0';
			strcat(destpath, params.newfile);
		}
		else
		{
			/* Just copy in oldpath */
			strcpy(destpath, params.oldfile);
		}
	
		sprintf(oldpath, "%s%s", rootdir, params.oldfile);
		sprintf(newpath, "%s%s", rootdir, destpath);
		
		#if defined BUILD_BIGENDIAN || defined _BIG_ENDIAN //OSX
		SEND_RETURN(SWAPL(rename(oldpath, newpath)));
		#else
		SEND_RETURN(rename(oldpath, newpath));
		#endif
	}
	return 0;
}

int process_cmd(int sock)
{
	int cmd, res;
	int (* process_func)(int);

	RECV_INTEGER(&cmd);	
	//printf("cmd = %08X\n", cmd);
	
	#if defined BUILD_BIGENDIAN || defined _BIG_ENDIAN //OSX
	cmd = SWAPL(cmd);
	#endif
	
	if (verbose)
		printf("cmd [%d] sock [%d]: %08X\n", spawn_idx, sock, cmd);

	process_func = NULL;

	if (!g_login) {
		switch (cmd)
		{
			case NET_HOSTFS_CMD_HELLO:	
				process_func = process_hello;
			break;
	
			case NET_HOSTFS_CMD_IOINIT:
				process_func = process_ioinit;	
			break;
			default:
				//printf("Unknown command: 0x%08X\n", cmd);
				printf("PSP haven't login, improper command [%d] sock [%d]: 0x%08X\n", spawn_idx, sock, cmd);
		}
	} else {
		switch (cmd)
		{
			case NET_HOSTFS_CMD_HELLO:	
				process_func = process_hello;
			break;
	
			case NET_HOSTFS_CMD_IOINIT:
				process_func = process_ioinit;	
			break;
			
			case NET_HOSTFS_CMD_IOEXIT:
				process_func = process_ioexit;
			break;
	
			case NET_HOSTFS_CMD_IOOPEN:
				process_func = process_ioopen;
			break;
	
			case NET_HOSTFS_CMD_IOCLOSE:
				process_func = process_ioclose;
			break;
	
			case NET_HOSTFS_CMD_IOREAD:
				process_func = process_ioread;
			break;
	
			case NET_HOSTFS_CMD_IOWRITE:
				process_func = process_iowrite;
			break;
	
			case NET_HOSTFS_CMD_IOLSEEK:
				process_func = process_iolseek;
			break;
	
			case NET_HOSTFS_CMD_IOREMOVE:
				process_func = process_ioremove;
			break;
	
			case NET_HOSTFS_CMD_IOMKDIR:
				process_func = process_iomkdir;
			break;
	
			case NET_HOSTFS_CMD_IORMDIR:
				process_func = process_iormdir;
			break;
	
			case NET_HOSTFS_CMD_IODOPEN:
				process_func = process_iodopen;
			break;
	
			case NET_HOSTFS_CMD_IODCLOSE:
				process_func = process_iodclose;
			break;
	
			case NET_HOSTFS_CMD_IODREAD:
				process_func = process_iodread;
			break;
	
			case NET_HOSTFS_CMD_IOGETSTAT:
				process_func = process_iogetstat;
			break;
	
			case NET_HOSTFS_CMD_IOCHSTAT:
				process_func = process_iochstat;
			break;
	
			case NET_HOSTFS_CMD_IORENAME:
				process_func = process_iorename;
			break;

			case NET_HOSTFS_CMD_GETKEY:
				process_func = process_getkey;
			break;
	
			default:
				//printf("Unknown command: 0x%08X\n", cmd);
				printf("Unknown command [%d] sock [%d]: 0x%08X\n", spawn_idx, sock, cmd);
		}
	}
	
	if (process_func)
		//res = process_func(sock);
		res = (*process_func)(sock);
	else
		res = 0;

	if (res >= 0)
	{
		if (cmd >= 0)
			res = cmd;
		else
			res = 0;
	}

	return res;
}

void *process_thread(void *conn_id)
{
	int cid;
	int flag;

	cid = *((int *) conn_id);

	flag = 1;
	setsockopt(cid, SOL_TCP, TCP_NODELAY, &flag, sizeof(flag));

	while (process_cmd(cid) >= 0);
		
	close(cid);
	DEBUG_PRINTF("Connection closed.\n");

	return NULL;
}

int parse_args(int argc, char **argv)
{
	int ch;

	while(1)
	{

		ch = getopt(argc, argv, "hvrskjp:l:c:d:b:1:2:");
		if(ch == -1)
		{
			break;
		}

		switch(ch)
		{
			case 'p': g_port = atoi(optarg);
					  break;
			case 'l': strncpy(password, optarg, 8);
				  password[8] = '\0';
				  g_login = 0;
				  break;
			case 'c': max_clients = atoi(optarg);
				  if (max_clients < 4)
					max_clients = 4;
				  else if (max_clients > 1000)
					max_clients = 1000;
				  break;
			case 'd': session_delay = atoi(optarg);
				  break;
			case 'b': blk_size = atoi(optarg);
				  break;
			case 'v': verbose = 1;
				  break;
			case 'r': rdonly = 1;
				  break;
			#ifdef __CYGWIN__
			case 's': keypad_redirect = 1;
				  break;
			case 'k': g_keypad = 0;
				  break;
			case 'j': g_p2joy = 1;
				  break;
			case '1': joymap1 = optarg;
				  break;
			case '2': joymap2 = optarg;
				  break;
			#endif
			case 'h': return 0;
			default:  printf("Unknown option\n");
					  return 0;
		};
	}

	argc -= optind;
	argv += optind;

	if(argc > 0)
	{
		strcpy(rootdir, argv[0]);
	}
	else
	{
		getcwd(rootdir, 512);
	}
	
	#ifdef __CYGWIN__
	if (joymap1)
		parse_joymap(0, joymap1);
	if (joymap2)
		parse_joymap(1, joymap2);
	#endif
	return 1;
}


int main(int argc, char *argv[])
{
	int sock;
	struct sockaddr_in addr, claddr;	
	pid_t pid;
	int i, found;
	int flag;
	int rc;
	pthread_t threadid;

	signal(SIGCHLD, SIG_IGN);

	if (!parse_args(argc, argv)) {
		printf("Usage: nethostfs [options] rootdir\n");
		printf("Options:\n");
		printf("  -p port		: TCP port number, default 7513\n");
		printf("  -l login_password	: Password for client PSP to login\n");
		printf("  -c max_clients	: Maximum number of client PSPs, valid range 4-1000, default 20\n");
		printf("  -d max_delay		: Max delay time between two session, default 10\n");
		printf("  -b block_size		: Max block size of data to send, default 1492\n");
		printf("  -v			: Print verbose message\n");
		printf("  -r			: Restrict to Read-Only access\n");
#ifdef __CYGWIN
		printf("  -s			: Enable PSP keypad redirection\n");
		printf("  -k			: Disable keyboard redirection\n");
		printf("  -j			: Player 2 uses Joystick 2, default keyboard\n");
		printf("  -1 mapfile		: Joystick 1 Mapping file\n");
		printf("  -2 mapfile		: Joystick 2 Mapping file\n");
#endif
		printf("  -h			: Print this help messages\n");
		exit(0);
	}

	client = malloc(sizeof(clientstruct) * max_clients);
	if (client == NULL) {
		printf("Error in allocating memory for %d clients\n", max_clients);
		exit(1);
	}

	printf("nethostfs v1.7 starting up%s, maximum of %d PSPs allowed\n", rdonly ? " under read-only mode":"", max_clients);
#ifdef __CYGWIN__
	if (keypad_redirect) {
		printf("PSP keypad redirection to PC %s active.\n", g_keypad ? "Joypad & Keyboard" : "Joypad");
		if (g_p2joy)
			printf("2 Player Host Mode: P1 - Joypad 1/Keyboard, P2 - Joypad 2\n");
		else
			printf("2 Player Host Mode: P1 - Joypad 1, P2 - Keyboard\n");
	}
#endif

	sock = socket(PF_INET, SOCK_STREAM, 0);

	if (sock < 0)
	{
		printf("Error in socket.\n");
		return -1;
	}

	flag = 1;
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) < 0)
	{
		printf("Error in setsockopt for REUSEADDR\n");
		return -1;
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(g_port);
	addr.sin_addr.s_addr = INADDR_ANY;
	
	if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
	{
		printf("Error in bind\n");
		return -1;
	}

	if (listen(sock, 1) < 0)
	{
		printf("Error in listen\n");
		return -1;
	}
	printf("Listening for incoming connections...\n");

	while (1)
	{
		int consock;
		unsigned int size;

		size = sizeof(claddr);
		consock = accept(sock, (struct sockaddr *)&claddr, &size);
		//printf("New connection arriving...\n");
		if (consock < 0)
		{
			printf("Error in accept\n");
			return 0;
		}

		found = 0;
		spawn = 0;
		for (i = 0; i < max_clients; i++) {
			if (claddr.sin_addr.s_addr == client[i].ip_addr.s_addr) {
				found = 1;
				if ((client[i].cidcnt == TOTAL_NETSESSIONS) || (time(NULL) - client[i].inittime > session_delay)) {
					if (client[i].pid != 0) {
						DEBUG_PRINTF("Killing previous session, idx = %d, pid = %d\n", i, client[i].pid);
						kill(client[i].pid, SIGHUP);
					}
					printf("Accepted reconnection from IP %s\n", inet_ntoa(claddr.sin_addr));
					client[i].cid[0] = consock;
					client[i].cidcnt = 1;
					client[i].inittime = time(NULL);
				} else if (client[i].cidcnt < TOTAL_NETSESSIONS) {
					printf("Adding multi-session for IP %s\n", inet_ntoa(claddr.sin_addr));
					client[i].cid[client[i].cidcnt] = consock;
					client[i].cidcnt++;
				}
				if (client[i].cidcnt == TOTAL_NETSESSIONS) {
					spawn = 1;
					spawn_idx = i;
				}
				break;
			}
		}
		if (!found) {
			for (i = 0; i < max_clients; i++) {
				if ((client[i].cidcnt != TOTAL_NETSESSIONS) && (time(NULL) - client[i].inittime > 5)) {

					printf("Accepted new connection from IP %s\n", inet_ntoa(claddr.sin_addr));
					client[i].ip_addr = claddr.sin_addr;
					client[i].cid[0] = consock;
					client[i].cidcnt = 1;
					client[i].inittime = time(NULL);
					break;
				}
			}
			if (i == max_clients) {
				printf("Too many clients connected (max %d), request from IP %s rejected\n", max_clients, inet_ntoa(claddr.sin_addr));
				close(consock);
			}
		}
					

		// AHMAN Add support for multiple client connections
		//       and cleanup of reconnection from same client
		if (spawn) {
			DEBUG_PRINTF("Creating child for index #%d...\n", spawn_idx);
			pid = fork();
			if (pid) {
				// Parent handling
				client[spawn_idx].pid = pid;
				DEBUG_PRINTF("Assigning pid (%d) to index #%d\n", pid, spawn_idx);
				for (i = 0; i < TOTAL_NETSESSIONS; i++) {
					close(client[spawn_idx].cid[i]);
				}
			} else {
				// Child handling
				for (i = 0; i < TOTAL_NETSESSIONS; i++) {
					rc = pthread_create(&threadid, NULL, process_thread, (void *) &(client[spawn_idx].cid[i]));
					if (rc != 0) {
						printf("Failed to create thread, abort!\n");
						exit(1);
					}
				}
				pthread_join(threadid, NULL);
				DEBUG_PRINTF("Self terminating thread for index #%d\n", spawn_idx);
				printf("Shutting down connections for IP %s\n", inet_ntoa(client[spawn_idx].ip_addr));
				exit(0);
			}
		}
	}
}

