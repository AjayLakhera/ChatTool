
/*openWindow.cpp 
* 
* Implementation of IRC Chat client 
*
*@author ajay kumar
*/
#include <winsock2.h>
#include <ws2tcpip.h>
#include <strsafe.h>
#include <conio.h>


#define			MAX_THREADS				3
#define			MAX_SIZE				255
#define			DEFAULT_PORT			6555				///< port number
#define			IP_ADDRESS				"192.168.0.103"			///< ip address
#define			GET_DATA				100
#define			STOP					102
#define         START					104
#define         RESPONSE				106

extern SOCKET ListenSocket;		///< socket object

struct tSsnNode {
	int uSession;
	char * uNick;
	SOCKET uSock;
	struct tSsnNode * uClient;
	struct tSsnNode * uLeft;
};
extern struct tSsnNode * First;
extern HANDLE thread1;
extern HANDLE eevent;

DWORD WINAPI ThreadProc  (LPVOID lpParam) ;

class TCPThread
{
public:
	void Connect		(void);
	void Disconnect	(void);

	
	void SendRecData ();
	int ClientRegis (SOCKET pSocket);
	void ProcessNick(char * pMessage);
	void ProcessUser(char * pMessage);
	void ProcessMSG (char * pMessage);
	void SendReq (SOCKET pSocket, char * buffer);
	int  ParseMessage(char * pMessage);
	int  ProcessQuit(char * pMessage);

	void ReceReq ();
};

int InsertSession(struct tSsnNode * pSessionNode, int pSession, SOCKET pSocket);
int Insert (SOCKET pSocket);
void SetNick(struct tSsnNode * pSessionNode, char* pNick);
int DeleteSession (SOCKET & pSock, int pSessionid, char *& pNick);
