#include"TCPHeader.h"
SOCKET  ListenSocket=INVALID_SOCKET;
HANDLE thread1 = NULL;

/**CreateConnection - bind and listening  connection
*/
void TCPThread::Connect()
{
	sockaddr_in 			service;  							///< service object					
	WSADATA					wsaData;							///< Create a WSADATA object
	char*					temparray;

	WSAStartup(MAKEWORD(2,2), &wsaData);								///< initilize WS2_32.dll
	///socket on serverside
	ListenSocket = WSASocket(AF_INET,SOCK_STREAM,IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);			///< socket object
	///case: soket valid or not
	if (ListenSocket == INVALID_SOCKET){
		WSACleanup();
		return ;
	}
	///fill the sockaddr_in structure
	service.sin_family 		= AF_INET;
	service.sin_addr.s_addr = inet_addr(IP_ADDRESS);
	service.sin_port 		= htons(DEFAULT_PORT);

	///case: bind the address to socket
	if (bind( ListenSocket, (SOCKADDR*) &service, sizeof(service)) == SOCKET_ERROR) {
		int i = GetLastError();
		closesocket(ListenSocket);
		WSACleanup();
		return ;
	}
	///case: listening for an incoming connection
	if (listen( ListenSocket, 10) == SOCKET_ERROR){
		closesocket(ListenSocket);
		WSACleanup();
		return ;
	}

	temparray =(char*) calloc( MAX_SIZE , sizeof(int));
	sprintf(temparray, "Server is ready to respond\r\n IPAddress :  %s\r\nPort : %d\r\n", IP_ADDRESS, DEFAULT_PORT);
	printf (temparray);
	free (temparray);

	while (1){
		SendRecData();
	}
}

void TCPThread::SendRecData ()
{	
	int ret ;

	SOCKET clientsocks = accept(ListenSocket, NULL, NULL);

	if (ClientRegis (clientsocks)){

		if (thread1 == NULL){

			eevent = CreateEvent (NULL,FALSE,FALSE,NULL);
			// Create a thread
			thread1 = CreateThread (NULL, 0, ThreadProc, 0, 0, 0);
		}
	}
}

DWORD WINAPI ThreadProc(LPVOID lpParam)
{
	WSABUF DataBuf;
	char buffer[255];
	WSAOVERLAPPED RecvOverlapped = {0};
	DWORD RecvBytes;
	DWORD Flags;
	struct tSsnNode * ssnode;

	while(1){
		memset (buffer, 0, 255);
		DataBuf.len = 255;
		DataBuf.buf = buffer;
		Flags = RecvBytes = 0;

		WaitForSingleObject(eevent,1000);

		// Reset the str
		ssnode = First;

		while(ssnode){

			//select(NULL, ssnode->uSock, NULL,NULL,20);
			WSARecv(ssnode->uSock, &DataBuf, 1, &RecvBytes, &Flags, &RecvOverlapped, NULL);
			if (strlen(buffer) != 0){
				//printf("%s",buffer);
				TCPThread tTcpObj;
				if(tTcpObj.ParseMessage(buffer)==0){
					break;
				}
			}
			memset (buffer, 0, 255);
			if (ssnode){
				ssnode = ssnode->uLeft;
			}
		}	

	}

	return 1;
}


int TCPThread::ClientRegis (SOCKET pSocket)
{
	WSABUF DataBuf;
	char buffer[255]={0};
	WSAOVERLAPPED SendOverlapped = {0};
	DWORD SendBytes;
	int session;

	if((session = Insert (pSocket)) != 0){

		memset (buffer, 0, 255);
		sprintf (buffer, "Trying to connect...\r\nSuccess..\r\nSession:%d\r\n", session);
		DataBuf.len = strlen(buffer);
		DataBuf.buf = buffer;

		WSASend(pSocket, &DataBuf, 1, &SendBytes, 0, &SendOverlapped, NULL);

		return 1;
	} 

	return 0;
}


int TCPThread::ParseMessage(char * pMessage)
{
	char command[10]={0};

	memcpy (command, &pMessage[11], 4);

	if (strcmp(command, "NICK") == 0){

		ProcessNick(pMessage);		
	} else if ((strcmp(command, "USER") == 0) || 
		(strcmp(command, "user") == 0)){

			ProcessUser (pMessage);
	} else if (strcmp(command, "MSSG") == 0){

		ProcessMSG(pMessage);
	} else if (strcmp(command, "QUIT") == 0){

		return ProcessQuit(pMessage);
	} 
	return 1;
}

void TCPThread::ProcessNick(char * pMessage)
{
	char sssin[10]={0};
	char buffer[255];
	int  ssnid, i=0;
	struct tSsnNode * tempnode;
	SOCKET tempsock=INVALID_SOCKET;
	char* ptr;

	ptr = pMessage;

	while (((*ptr != 0x00) && (*ptr != 0x0D))){
		sssin[i++] = *ptr++;
	}

	ssnid = atoi(sssin);

	// 
	tempnode = First;

	while (tempnode != NULL){
		if(tempnode->uSession == ssnid){
			tempsock = tempnode->uSock;
			SetNick(tempnode, &pMessage[16]);	
			break;
		}
		tempnode=tempnode->uLeft;
	}

	memset(buffer, 0, 255);

	if (tempsock != INVALID_SOCKET){
		sprintf (buffer, "NICK %s Accepted..\r\n", tempnode->uNick);
		printf("\n%s Connected..", tempnode->uNick);
	} else{
		strcpy(buffer, "Invalid connectio\r\n");
	}

	SendReq (tempsock, buffer);
}


void TCPThread::ProcessUser(char * pMessage)
{

	char sssin[10]={0};
	char buffer[255];
	int  ssnid, i=0;
	struct tSsnNode * ssnode;
	struct tSsnNode * tempnode;
	char* ptr;
	ptr = pMessage;
	SOCKET tempsock=INVALID_SOCKET;

	while(((*ptr != 0x0D) && (*ptr != 0x00))){
		sssin[i++] = *ptr++;
	}


	ssnid = atoi(sssin);

	// 
	tempnode = First;

	while (tempnode != NULL){
		if(tempnode->uSession == ssnid){
			tempsock = tempnode->uSock;
			break;
		}
		tempnode = tempnode->uLeft;
	}

	ssnode = First;

	memset(buffer, 0, 255);

	strcat(buffer, "USER ");

	while(tempnode && tempnode->uNick &&  ssnode){
		// Compare nick names
		if(ssnode->uNick && (strcmp(tempnode->uNick, ssnode->uNick) != 0)){
			strcat(buffer, ssnode->uNick);
			strcat(buffer, ",");
		}
		ssnode = ssnode->uLeft;
	}

	strcat(buffer, "\r\n");

	if(tempsock != INVALID_SOCKET){
		SendReq (tempsock, buffer);
	}
}

void TCPThread::ProcessMSG(char * pMessage)
{

	char sssin[10]={0};
	char nckname[11]={0};
	int  ssnid, i=0;
	char buffer[255]={0};
	char* ptr, *ptr1;
	SOCKET tempsock=INVALID_SOCKET;
	int len;
	struct tSsnNode * tempnode;
	struct tSsnNode * tempnode1;

	ptr1 = pMessage;

	while(((*ptr1 != 0x0D) && (*ptr1 != 0x00))){
		sssin[i++] = *ptr1++;
	}

	ssnid = atoi(sssin);

	i =0;

	while(*ptr1 == 0x0D || *ptr1 == 0x0A || *ptr1 == 0x00){
		ptr1++;
	}

	ptr = &ptr1[5];

	while(*ptr != 0x00){
		nckname[i++] = *ptr++;
	}

	// 
	tempnode = First;

	while (tempnode != NULL){
		if(tempnode->uSession == ssnid){
			// 
			tempnode1 = First;

			while (tempnode1 != NULL){
				if(strcmp(tempnode1->uNick, nckname) == 0){
					tempsock = tempnode1->uSock;
					break;
				}
				tempnode1 = tempnode1->uLeft;
			}
			break;
		}
		tempnode = tempnode->uLeft;
	}

	memset(buffer, 0, 255);

	if (tempsock != INVALID_SOCKET){
		strcat(buffer, ++ptr);
		strcat(buffer, "\r\n");
	} else{
		strcpy(buffer, "Nick not exists\r\n");
		tempsock = tempnode->uSock;
	}

	SendReq (tempsock, buffer);
}


int TCPThread::ProcessQuit(char * pMessage)
{

	char sssin[10]={0};
	char buffer[255]={0};
	int  ssnid, i=0;
	char * nick = NULL;
	char* ptr;
	SOCKET tempsock;

	// Default
	nick     = NULL;
	tempsock = INVALID_SOCKET;

	ptr = pMessage;

	while(((*ptr != 0x0D) && (*ptr != 0x00))){
		sssin[i++] = *ptr++;
	}

	ssnid = atoi(sssin);

	memset(buffer, 0, 255);

	if(DeleteSession (tempsock, ssnid, nick)){
		sprintf(buffer, "\n%s got Disconnect", nick);
		//printf (buffer);
		SendReq (tempsock, buffer);
	} else {
		sprintf(buffer, "\n%s not a valid connection", nick);
		printf (buffer);
	}

	closesocket(tempsock);
	free(nick);

	return 0;
}

void TCPThread::SendReq (SOCKET pSocket, char * buffer)
{
	WSABUF DataBuf;
	WSAOVERLAPPED SendOverlapped = {0};
	DWORD SendBytes;

	DataBuf.len = 235;
	DataBuf.buf = buffer;

	puts(buffer);
	WSASend(pSocket, &DataBuf, 1, &SendBytes, 0, &SendOverlapped, NULL);
	memset (buffer, 0, 235);		
}


void TCPThread::Disconnect(){
	printf("\n server is shutting down...");
	shutdown(ListenSocket,SD_BOTH);
	closesocket(ListenSocket);
	ListenSocket=INVALID_SOCKET;
}
