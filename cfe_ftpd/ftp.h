# ifndef PSP_FTP_H
# define PSP_FTP_H

#include "nlh.h"

#define MAX_USER_LENGTH 50
#define MAX_PASS_LENGTH 50

# if 0 //200kb
#define MAX_PATH_LENGTH 256
#define TRANSFER_WRITE_BUFFER_SIZE (16*1024)
#define TRANSFER_RECV_BUFFER_SIZE       512
#define TRANSFER_SEND_BUFFER_SIZE      4096
#define MAX_COMMAND_LENGTH             1024
# endif
# if 0 //300kb
#define MAX_PATH_LENGTH 256
#define TRANSFER_WRITE_BUFFER_SIZE (32*1024)
#define TRANSFER_RECV_BUFFER_SIZE       512
#define TRANSFER_SEND_BUFFER_SIZE      4096
#define MAX_COMMAND_LENGTH             1024
# endif
# if 1 //400kb !!!!!!!!!
#define MAX_PATH_LENGTH                 256
#define TRANSFER_WRITE_BUFFER_SIZE (48*1024)
#define TRANSFER_RECV_BUFFER_SIZE       512
#define TRANSFER_SEND_BUFFER_SIZE      4096
#define MAX_COMMAND_LENGTH             1024
# endif


typedef struct MftpConnection {
  SOCKET sockCommand;
  SOCKET sockPASV;
  SOCKET sockData;
  char root[MAX_PATH_LENGTH];
  char curDir[MAX_PATH_LENGTH];
  int userLoggedIn;
  int usePassiveMode;
  char user[MAX_USER_LENGTH];
  char pass[MAX_PASS_LENGTH];
  unsigned char port_addr[4];
  unsigned short port_port;
  char transfertType;
  char serverIp[32];
  char clientIp[32];

  char renameFromFileName[MAX_PATH_LENGTH];
  char renameFrom;
  
  char sockCommandBuffer[1024];
  char sockDataBuffer[1024];

} MftpConnection;

extern int mftpServerHello(MftpConnection *con);
extern int mftpDispatch(MftpConnection *con, char* command);
extern int mftpRestrictedCommand(MftpConnection *con, char* command) ;

# endif
