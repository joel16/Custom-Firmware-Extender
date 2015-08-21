
#include "std.h"
#include "ftp.h"
#include "ftpd.h"
#include "sutils.h"
#include "conf.h"

#include <pspsysmem.h>

    SceUID memid;
    SceUID memid2;
    SceUID memid3;
    SceUID memid4;

  typedef struct thread_list {
    struct thread_list *next;
    int                 thread_id; 
  } thread_list;

  thread_list *mftp_thread_head = NULL;

  SOCKET sockListen = 0;

static void
mftpAddThread(int thread_id) 
{
	//char *memid;

	memid = sceKernelAllocPartitionMemory(2,"TH_CACHE",0,sizeof(thread_list),NULL);

 	thread_list *new_thread = (thread_list *)sceKernelGetBlockHeadAddr(memid);

	//thread_list *new_thread = (thread_list *)sceKernelAllocPartitionMemory(1, "myCurFile", 0, sizeof(thread_list), NULL);

  new_thread->next      = mftp_thread_head;
  new_thread->thread_id = thread_id;

  mftp_thread_head = new_thread;
}

static void
mftpDelThread(int thread_id) 
{

  thread_list **prev_thread = &mftp_thread_head;
  thread_list  *del_thread;

  del_thread = mftp_thread_head; 
  while (del_thread != (thread_list *)0) {
    if (del_thread->thread_id == thread_id) break;
    prev_thread = &del_thread->next;
    del_thread  = del_thread->next;
  }
  if (del_thread) {
    *prev_thread = del_thread->next;
    //free(del_thread);
	sceKernelFreePartitionMemory(memid);

  }
}

SceCtrlData c;

int
mftpExitHandler(SceSize argc, void *argv) 
{
  int err = 0;

  while (1) {
    sceCtrlReadBufferPositive(&c, 1);
    if (c.Buttons & PSP_CTRL_CIRCLE) break;
  }
  if (sockListen) {
	  err = sceNetInetClose(sockListen);
  }

  thread_list  *scan_thread = mftp_thread_head; 
  while (scan_thread != (thread_list *)0) {
    //sceKernelTerminateThread(scan_thread->thread_id);
	mftpDelThread(scan_thread->thread_id);
	sceKernelTerminateDeleteThread(scan_thread->thread_id);
    scan_thread = scan_thread->next;
  }
	sceKernelFreePartitionMemory(memid);
	sceKernelFreePartitionMemory(memid2);
	//sceKernelGetBlockHeadAddr(memid3);
	sceKernelFreePartitionMemory(memid3);
	//sceKernelGetBlockHeadAddr(memid4);
	sceKernelFreePartitionMemory(memid4);
	sceKernelExitDeleteThread(0);
  return 0;
}


int 
mftpClientHandler(SceSize argc, void *argv) 
{

	//mftp_config.auth_required = 1;

  int thid = sceKernelGetThreadId();
  mftpAddThread(thid);
	MftpConnection *con = *(MftpConnection **)argv;

	con->sockData =0;
	con->sockPASV =0;

  //if (mftp_config.head_user) {
   // strcpy(con->root,mftp_config.head_user->root);
  //} else {
	  strcpy(con->root,"ms0:");
  //}

	//memset(con->sockCommandBuffer, 0, 1024);
	//memset(con->sockDataBuffer, 0, 1024);
	strcpy(con->curDir,"/");
	//strcpy(con->user,"login");
	//strcpy(con->pass,"pass");
	memset(con->user, 0, MAX_USER_LENGTH);
	memset(con->pass, 0, MAX_PASS_LENGTH);
  	//strcpy(con->renameFromFileName,"");
  	con->renameFrom = 0;
	con->usePassiveMode=0;
	con->userLoggedIn=0;
	con->port_port=0;
	con->port_addr[0] = 0;
	con->port_addr[1] = 0;
	con->port_addr[2] = 0;
	con->port_addr[3] = 0;
	con->transfertType='A';

	int err;

	mftpServerHello(con);

  char messBuffer[64];
	char readBuffer[1024];
	char lineBuffer[1024];
	int lineLen=0;
	int errLoop=0;
	while (errLoop>=0)
  {
  	int nb = sceNetInetRecv(con->sockCommand, (u8*)readBuffer, 1024, 0);
  	if (nb <= 0) break;

  	int i=0; 
  	while (i<nb) {
  		if (readBuffer[i]!='\r') {
  			lineBuffer[lineLen++]=readBuffer[i];
  			if (readBuffer[i]=='\n' || lineLen==1024) {
  				lineBuffer[--lineLen]=0;
  				char* command=skipWS(lineBuffer);
  				trimEndingWS(command);

          snprintf(messBuffer, 64, "> %s from %s", command, con->clientIp);

				  if ((errLoop=mftpDispatch(con,command))<0) break;
				  lineLen=0;
			  }
		  }
		  i++;

	  }
  }

	err = sceNetInetClose(con->sockCommand);
	//free(con);
	//sceKernelGetBlockHeadAddr(memid2);
	sceKernelFreePartitionMemory(memid2);

  mftpDelThread(thid);
	sceKernelExitDeleteThread(0);
	
	return 0;
}

int 
ftpdLoop(const char* szMyIPAddr)
{
  char buffer[64];
  u32 err;
  SOCKET sockClient;

	char url[128];
	strcpy(url, "ftp://");
	strcat(url, szMyIPAddr);
	strcat(url, "/");


	int exit_id = sceKernelCreateThread("ftpd_client_exit",
                                      mftpExitHandler, 0x8, 256*1024, PSP_THREAD_ATTR_USER, 0);
	if(exit_id >= 0) {
		sceKernelStartThread(exit_id, 0, 0);
	}

	struct sockaddr_in addrListen;
	struct sockaddr_in addrAccept;
	u32 cbAddrAccept;
	sockListen = sceNetInetSocket(AF_INET, SOCK_STREAM, 0);
	if (sockListen & 0x80000000) goto done;
	addrListen.sin_family = AF_INET;
	addrListen.sin_port = htons(21);
	addrListen.sin_addr[0] = 0;
	addrListen.sin_addr[1] = 0;
	addrListen.sin_addr[2] = 0;
	addrListen.sin_addr[3] = 0;

	// any
	err = sceNetInetBind(sockListen, &addrListen, sizeof(addrListen));
	if (err) goto done;
	err = sceNetInetListen(sockListen, 1);
	if (err) goto done;

  while (1) {
	  cbAddrAccept = sizeof(addrAccept);

	  sockClient = sceNetInetAccept(sockListen, &addrAccept, &cbAddrAccept);
	  if (sockClient & 0x80000000) goto done;


	memid2 = sceKernelAllocPartitionMemory(2,"TH_CACHE2",0,sizeof(MftpConnection),NULL);
 	MftpConnection* con = (MftpConnection*)sceKernelGetBlockHeadAddr(memid2);


    //MftpConnection* con=(MftpConnection*)malloc(sizeof(MftpConnection));

    if (sceNetApctlGetInfo(8, con->serverIp) != 0) {
      goto done;
    }

    snprintf(con->clientIp, 32, "%d.%d.%d.%d",
            addrAccept.sin_addr[0], addrAccept.sin_addr[1],
            addrAccept.sin_addr[2], addrAccept.sin_addr[3]);
    snprintf(buffer, 64, "Connection from %s", con->clientIp);

    con->sockCommand = sockClient;
	  int client_id = sceKernelCreateThread("ftpd_client_loop", mftpClientHandler, 0x18, 0x10000, 0, 0);
	  if(client_id >= 0) {
		  sceKernelStartThread(client_id, 4, &con);
	  }
  }

done:
	err = sceNetInetClose(sockListen);

  return 0;
}
