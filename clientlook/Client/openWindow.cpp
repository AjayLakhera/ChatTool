
/*openWindow.cpp 
* 
* Implementation of IRC Chat client 
*
*@author ajay kumar
*/

#include "Header.h"
#include "resource.h"
char	 Session [124];
char	 sNick   [10];
HMENU    MainMenu;


/**/
void CreateControl ()
{
	
	RECT rect;
	//for rich edit control we have to call LoadLibraray function
	LoadLibrary( "Riched32.dll" );
	
	//InitCommonControls();
	GetClientRect(hWnd, &rect);

	usercaption = CreateWindow( "button", "Refresh Buddy List" , WS_VISIBLE | WS_CHILD |BS_PUSHBUTTON,
				   rect.left+10, rect.top+10, rect.right-20, 20, hWnd,(HMENU)ID_USER_CAPTION, NULL, NULL );

	listview = CreateWindow( "ListBox", "Buddy List.", WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_STANDARD | LBS_SORT  | WS_BORDER| LVS_REPORT , 
				rect.left+10, rect.top+40, rect.right-20, rect.bottom-50, hWnd,(HMENU)ID_LIST_BOX, NULL, NULL );

	
}



int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow )
{
			WNDCLASS wc;		// object of window class
			MSG      msg;			// msg structure
			BOOL     bRet;			// boolean value
            
	//fill the window class structure
	if( !hPrevInstance ){
		wc.lpszClassName	= "Chat_Client";
		wc.lpfnWndProc		= MainWndProc;			
		wc.style			= CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
		wc.hInstance		= hInstance;
		//wc.hIcon			= LoadIcon( NULL, IDI_APPLICATION );
		wc.hIcon			= (HICON)LoadImage(hInstance,  MAKEINTRESOURCE (IDI_ICON1),IMAGE_ICON, 
							  GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
		wc.hCursor			= LoadCursor( NULL, IDC_ARROW );
		wc.hbrBackground	= (HBRUSH)( COLOR_WINDOW+1 );
		wc.lpszMenuName		= NULL;
		wc.cbClsExtra		= 0;
		wc.cbWndExtra		= 0;
		RegisterClass( &wc );
	}

    //load main menu
    MainMenu = LoadMenu (hInstance, MAKEINTRESOURCE(ID_MENU));

	ghInstance = hInstance;
	hWnd = CreateWindow( "Chat_Client",							//register class name
					"Chat_Client",								//window caption
					WS_OVERLAPPEDWINDOW|WS_THICKFRAME ,			//window style
					300,										//x coordinate
					30,										    //y coordinate
					300,										//default width
					500,										//default height
					NULL,										//parent handle
					MainMenu,									//menu handle
					hInstance,									//current instance
					NULL										//lParam
					);
	//display window
	ShowWindow( hWnd, nCmdShow );

	// create the control
	CreateControl ();
	
	while( (bRet = GetMessage( &msg, NULL, 0, 0 )) != 0 ){
		if (bRet == MSG_ERROR){
			// handle the error and possibly exit
		}else{
			//This function translates key messages into character messages
			TranslateMessage( &msg );
			// dispatches a message to  window procedure
			DispatchMessage( &msg );
		}
	}
	return (int)msg.wParam;
}


/**
*@brief Window procedure
*/

LRESULT CALLBACK MainWndProc( HWND pWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
		LPNMITEMACTIVATE lpnmitem;
		int ctrlid;
		
	ctrlid = LOWORD(wParam);

	switch( msg )
	{
	
	case WM_COMMAND:
		switch(ctrlid){
        case ID_CONNECT_MSG:
            CreateConnection ();
            break;

        case ID_DISCONNECT_MSG:
			SetWindowText(pWnd, "");
            SendDisconnect(pWnd);
			break;

		case ID_USER_CAPTION:
			ProcessRefreshUser (hWnd);
			break;
		case ID_LIST_BOX:
			if(HIWORD(wParam) == LBN_DBLCLK){
				OpenChatWindow();
			}
		}
		break;
	
	case WM_SIZE:
			MoveWindow(hWnd, 10, 20, 300, 500, TRUE);
			break;
	case WM_SIZING:
			MoveWindow(hWnd, 10, 20, 300, 500, TRUE);
		
		break;
	case DLG_MSG:
		ProcessPrivateMessage ((char*)wParam);
		
		break;
	case WM_DESTROY:
		SendDisconnect(pWnd);
		PostQuitMessage( 0 );
		break;
	default:
		return( DefWindowProc( pWnd, msg, wParam, lParam ));
	}
	return false;
}

void  CreateConnection()
{
			int				Result;
			void*			temp;
			WSADATA			wsaData;	
			sockaddr_in		clientService; 	

	//initialize  Windows sockets .
	WSAStartup(MAKEWORD(2,2), &wsaData);                                         
	
	//argments in socket( family of protocol,type and protocol)
	ConnectSocket = WSASocket(AF_INET,SOCK_STREAM,IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);

	clientService.sin_family		=   AF_INET;
	clientService.sin_addr.s_addr   =   inet_addr(IP_ADDRESS);
	clientService.sin_port		    =   htons(DEFAULT_PORT) ;
	
	// Connect to server.
	///connect return 0 if successful other wise SOCKET_ERROR error
	Result = connect(ConnectSocket, (sockaddr*) &clientService, sizeof(clientService) ); 
	
	if ( Result == SOCKET_ERROR) {
		DisplayText("\nerror in socket creation..\n");
		
		closesocket(ConnectSocket);
		WSACleanup();									
		return ;
	} else{  	
		Sleep(100);
		RegisterNickName ();
        EnableMenuItem (MainMenu,  ID_DISCONNECT_MSG, MF_ENABLED);
        EnableMenuItem (MainMenu,  ID_CONNECT_MSG, MF_GRAYED);
		EnableWindow(chatbox , true );
        ReceiveRequest ();
	}	
}


int SendNickMsg ()
{
	if (SendNick ()){
		// Create a thread and event
		ResetControls ();
		
		if (RecThread == NULL)
		RecThread  = CreateThread (NULL, 0, RecThreadProc, 0, 0, 0);	
       
		return 1;
	}
	return 0;
}



DWORD WINAPI RecThreadProc(LPVOID lpParam)
{
	fd_set          fds;
    timeval         tv;
	
	 // setup the timeout
    tv.tv_sec   = 1000;
    tv.tv_usec  = 0; 

	FD_ZERO(&fds);
    FD_SET(ConnectSocket, &fds);

	while(1){
		
		select (NULL,&fds, NULL, NULL, &tv);
		ReceiveRequest ();
	}
	return 1;
}


int ReceiveRequest ()
{
	WSABUF DataBuf;
	char buffer[255];
    WSAOVERLAPPED RecvOverlapped = {0};
    DWORD RecvBytes;
    DWORD Flags;

	memset (buffer, 0, 255);
	
	DataBuf.len = 255;
	DataBuf.buf = buffer;
	Flags = RecvBytes = 0;
	
	WSARecv(ConnectSocket, &DataBuf, 1, &RecvBytes, &Flags, &RecvOverlapped, NULL);
	
	DisplayText( buffer);
	
	if (strlen(buffer) != 0){
		ParseResponse (buffer);
	}
	
	return 0;
}

int SendRequest (char * pBuffer, int pPosition, HWND pHwnd)
{
	int ret ;
	WSABUF DataBuf;
	char buffer[255];
	char msgbuf[1024]={0};
    WSAOVERLAPPED SendOverlapped = {0};
    DWORD SendBytes;
	
	if (pHwnd){
		GetDlgItemText(pHwnd, ID_EDIT_BOX, buffer, 255);
		sprintf(msgbuf, "<%s>%s", sNick, buffer);
		DisplayText (msgbuf, 1);
	} else {
		GetDlgItemText(NickDLG, ID_EDIT_BOX, buffer, 255);
		if(strlen(buffer)==0){
			return 1;
		}
	}

	//DisplayText (buffer, 1);
	SetWindowText( chatbox, "" );
	if(strlen(pBuffer) != 0) {
		strcat(&pBuffer[pPosition], buffer);
				
		DataBuf.len = 255;
		DataBuf.buf = pBuffer;
		strcat(&pBuffer[251],"\r\n"); 
		
		WSASend(ConnectSocket, &DataBuf, 1, &SendBytes, 0, &SendOverlapped, NULL);
	}
	memset (buffer, 0, 255);
	memset (msgbuf, 0, 1024);
	return 1;
}

void ParseSessionID (char* pMessage)
{
	char *p = strchr(pMessage, ':');
	strcpy(Session, ++p);	
}

int SendNick ()
{
	char buffer[255];
	memset (buffer, 0, 255);

	strcat(buffer, Session);
	strcat(&buffer[11], "NICK");

	SendRequest (buffer, 16);
	return 1;
}

void DisplayText(char* pBuffer, int pNxtLine)
{
	if (strlen(pBuffer) == 0)
		return;
	if(pNxtLine){
		strcat(pBuffer, "\n");
	}
	SETTEXTEX setstruct;
	setstruct.flags  = ST_SELECTION;
	setstruct.codepage = 0;
	SendMessage(editbox, EM_SETTEXTEX, (WPARAM)&setstruct, (LPARAM)pBuffer); 

}

int SendMessage ()
{
	char buffer[255];
	char user[10]={0};

	memset (user, 0, 10);
	
	int sel = SendMessage(listview, LB_GETCURSEL, (WPARAM)0, (LPARAM)0); 
	SendMessage(listview, LB_GETTEXT, (WPARAM)sel, (LPARAM)user); 

	memset (buffer, 0, 255);

	strcat(buffer, Session);
	strcat(&buffer[11], "MSSG");
	strcat(&buffer[16], user);
	sprintf(&buffer[17+strlen(user)],"<%s>", sNick);
	
	SendRequest (buffer, (19 + strlen (user) + strlen(sNick)));
	return 1;

}

int SendDisconnect (HWND pHwnd)
{
	char buffer[255];
	memset (buffer, 0, 255);

	strcat(buffer, Session);
	strcat(&buffer[11], "QUIT");

	SendRequest (buffer, 16, pHwnd);

	EnableMenuItem (MainMenu,  ID_DISCONNECT_MSG, MF_GRAYED);
	EnableMenuItem (MainMenu,  ID_CONNECT_MSG, MF_ENABLED);

	EnableWindow( submitbutton , false);
	

	return 1;
}

void ProcessRefreshUser (HWND pHWnd)
{
	char buffer[255];
	int indexcount;
	memset (buffer, 0, 255);

	strcat(buffer, Session);
	strcat(&buffer[11], "USER");

	SendRequest (buffer, 16, pHWnd);
	SendMessage(listview, LB_RESETCONTENT, 0, 0);
}


void ParseUserList(char * pBuffer)
{
	char *userlist;
	char usr[10]={0};
	int pos=0;
	userlist = &pBuffer[5];
	
	while(*userlist != '\r'){
		int i =0;
		while(*userlist != ','){
			usr[i]=*userlist;
			i++,userlist++;
		}
		userlist++;

		SendMessage(listview,  LB_ADDSTRING, pos++, (LPARAM)usr);
		
		memset(usr, 0, 10);
	}
}

int ParseResponse (char * buffer)
{
	if (strstr(buffer, "Session:")){
		ParseSessionID(buffer);
		return 1;
	} else if (strstr(buffer, "USER")){
		ParseUserList(buffer);
		return 1;
	} else {
		ParsePrivateMsg (buffer);
	}

	return 0;
}

void ParsePrivateMsg (char * pBuffer)
{
			char * ptr;
			char * msg;
			int i;
	
	ptr = strchr(pBuffer, '<');
	
	if (ptr != NULL){
		int len = strlen(ptr);
		msg =(char*)calloc(len+1, sizeof(char));
		memcpy(msg, ptr, len);
		PostMessage(hWnd, DLG_MSG, (WPARAM)msg, (LPARAM)0); 
		
	} else if (strstr(pBuffer, "NICK")){
		
		ptr = &pBuffer[5];
		i =0;
		while(*ptr != 0x20 && i < 10){
			sNick[i++] = *ptr++;
		}
		SetWindowText(hWnd, sNick);
		ProcessRefreshUser (hWnd);
	}
}


int ProcessPrivateMessage (char * pBuffer)
{
		char   buffer[10]={0};
		char   title[10]={0};
		int    i=0;
		char * ptr;
		HWND   handle;
		
	ptr = &pBuffer[1];
	
	while(*ptr != '>'){
		buffer[i++] = *ptr++;
	}
	
	// Check handle already exists
	handle = CheckDialogExists(buffer);

	if( handle ==  NULL){
		handle = CreateDialog(ghInstance, MAKEINTRESOURCE(CLIENT_DIALOG), NULL, (DLGPROC)GoToProc); 


		HICON hIcon=(HICON)LoadImage(ghInstance, MAKEINTRESOURCE(IDI_ICON1),IMAGE_ICON, 
			GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR); 
			SendMessage(handle, WM_SETICON, ICON_BIG  , (LPARAM)hIcon);
			
		if (SetClientWinHandle(handle, buffer) != 0){
			ShowWindow(handle, SW_SHOW);
			SetWindowText(handle, buffer);
			DisplayText(pBuffer);
		} else {
			CloseHandle(handle);
		}
	} else{
		ptr = (char*)calloc(strlen(pBuffer)+1, sizeof(char));
		strcpy(ptr, pBuffer);
		PostMessage(handle, CLIENT_DLG_MSG, (WPARAM)ptr, (LPARAM)0);
	}

	free (pBuffer);

	return 1;
}


LRESULT CALLBACK ChatDialogProc (HWND pHWnd, UINT pMessage, WPARAM pWParam, LPARAM pLParam)
{
       
	switch (pMessage){
		
	case WM_COMMAND:
	switch(LOWORD(pWParam)){
	
	case ID_CHAT_BUTTON:
			SendChatMessage (pHWnd);
		}
		break;

	case WM_CLOSE:
		DestroyWindow(pHWnd);
		break;
    case WM_DESTROY:
		PostQuitMessage(0);
		
		break;
	default:
		return( DefWindowProc( pHWnd, pMessage, pWParam, pLParam ));
	 }

	 return 0;
}


void SetWindowPosition (HWND pHWnd)
{   
		RECT rect;

    MoveWindow (pHWnd, 300,200, 400, 700, TRUE);

	GetClientRect(pHWnd, &rect);

	MoveWindow (editbox, rect.left+10, rect.top, rect.right-20, rect.bottom-120, TRUE);
	MoveWindow (chatbox, rect.left+10, rect.bottom-100, rect.right-20, 30 , TRUE);
	MoveWindow (submitbutton, rect.right/2+80, rect.bottom-50, 100, 30, TRUE);

}


void CreateControls (HWND pHWnd)
{
		RECT rect;

	GetClientRect(pHWnd, &rect);
	//creating Rich Edit Control window
	editbox = CreateWindow( "richedit", "" , WS_VISIBLE | WS_CHILD |ES_MULTILINE |WS_VSCROLL, 
					rect.left+10, rect.top+50, rect.right-20, rect.bottom-120 , pHWnd, (HMENU)ID_RICH_EDIT_BOX, NULL, NULL );
    
	//creating Rich Edit Control window
	chatbox = CreateWindow( "edit", "" , WS_VISIBLE | WS_CHILD  |WS_BORDER, 
					rect.left+10, rect.bottom-100, rect.right-20, 30 , pHWnd, (HMENU)ID_EDIT_BOX, NULL, NULL );
   
	//creating Rich Edit Control window
	submitbutton = CreateWindow( "button", "Message" , WS_VISIBLE | WS_CHILD |BS_PUSHBUTTON , 
							rect.right/2+80, rect.bottom-50, 100, 30, pHWnd, (HMENU)ID_CHAT_BUTTON, NULL, NULL );  
}

void OpenChatWindow()
{
	int		index;
	char 	buffer[10];
	HWND    handle;
	
	index = SendMessage (listview, LB_GETCURSEL, 0, 0);
	
	if (index != LB_ERR){
		memset(buffer, 0, 10);
		if(SendMessage (listview, LB_GETTEXT, (WPARAM)index, (LPARAM)buffer) != LB_ERR){
		
			handle = CheckDialogExists(buffer);

			if(handle == 0){
				handle = CreateDialog(ghInstance, MAKEINTRESOURCE(CLIENT_DIALOG), NULL, (DLGPROC)GoToProc); 
			
				if (SetClientWinHandle(handle, buffer) != 0){
					ShowWindow(handle, SW_SHOW);
					SetWindowText(handle, buffer);
				} else {
					CloseHandle(handle);
				}
			}
		}
	}
}


void ResetControls ()
{
		RECT rect;

	GetClientRect(hWnd, &rect);

	MoveWindow(usercaption, rect.left+10, rect.top+10, rect.right-20, 20, TRUE);
	MoveWindow(listview, rect.left+10, rect.top+40, rect.right-20, rect.bottom-50, TRUE);
}

void SendChatMessage (HWND pHWnd)
{
	
		char user[10];
		char buffer[255];
	
	memset(buffer, 0, 255);
	memset(user, 0, 10);
	GetWindowText (pHWnd, user, 10);

	if(user){
		strcat(buffer, Session);
		strcat(&buffer[11], "MSSG");
		strcat(&buffer[16], user);
		sprintf(&buffer[17+strlen(user)],"<%s>", sNick);
	
		SendRequest (buffer, (19 + strlen (user) + strlen(sNick)), pHWnd);
	}
}

BOOL CALLBACK GoToProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam) 
{ 
    BOOL fError; 
 
    switch (message) 
    { 
        case WM_INITDIALOG: 
			CreateControls(hwndDlg);
            SetWindowPosition (hwndDlg);
			
            return TRUE; 
 
        case WM_COMMAND: 
            switch (LOWORD(wParam)) 
            case ID_CHAT_BUTTON:
				SendChatMessage (hwndDlg);
				return TRUE;
			
		case WM_CLOSE:
			DeleteHandle(hwndDlg);
			EndDialog(hwndDlg, 0);
			return TRUE;               
		case CLIENT_DLG_MSG:
			{
				char * ptr = (char*)wParam;
				DisplayText (ptr, 1);
				//free(ptr);
			}
			return TRUE;
	} 
    return FALSE; 
} 

void RegisterNickName ()
{
	char title[10]={0};

	if(NickDLG != NULL){
		GetWindowText(NickDLG, title, 10);
		if(strcmp(title,"Register Nick") ==0)
			return;
	}

	NickDLG = CreateDialog(ghInstance, MAKEINTRESOURCE(CLIENT_DIALOG), hWnd, (DLGPROC)GoToNickProc); 
	ShowWindow(NickDLG, SW_SHOW);

	SetWindowText(NickDLG, "Register Nick");
}

BOOL CALLBACK GoToNickProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam) 
{ 
    BOOL fError; 
 
    switch (message) 
    { 
        case WM_INITDIALOG: 
			CreateNickGlgControls(hwndDlg);
			SetNickControlPositions(hwndDlg);
            return TRUE; 
 
        case WM_COMMAND: 
            switch (LOWORD(wParam)) 
			{
            case ID_NICK_REG:
				SendNickMsg ();
				NickDLG = NULL;
				EndDialog(hwndDlg, 0);
			}
			return TRUE;
			
		case WM_CLOSE:
			EndDialog(hwndDlg, 0);
			return TRUE;               
    } 
    return FALSE; 
} 

void CreateNickGlgControls(HWND pHwnd)
{
		RECT rect;
	GetClientRect(pHwnd, &rect);

	
	nickstat = CreateWindow( "static", "Enter nick name" , WS_VISIBLE | WS_CHILD, 
					10, 0, 100, 20 , pHwnd, (HMENU)ID_NICK_STATIC, NULL, NULL );
    
	nickeditbox = CreateWindow( "edit", "" , WS_VISIBLE | WS_CHILD | WS_BORDER  ,
					10, 30, 200, 30 , pHwnd, (HMENU)ID_EDIT_BOX, NULL, NULL );
    
	nickchatbutton = CreateWindow( "button", "Register" , WS_VISIBLE | WS_CHILD  |WS_BORDER | BS_PUSHBUTTON, 
						40, 80, 100, 30 , pHwnd, (HMENU)ID_NICK_REG, NULL, NULL );
   
}

void SetNickControlPositions(HWND pDlgWnd)
{
	RECT rect;

	GetClientRect(hWnd, &rect);

	MoveWindow (pDlgWnd, rect.left+40,rect.top+40, 250, 140, TRUE);

	GetClientRect(pDlgWnd, &rect);

	MoveWindow(nickstat, rect.left+10, rect.top+10, 150, 20, TRUE);
	MoveWindow(nickeditbox, rect.left+10, rect.top+40, rect.right-20, 20, TRUE);
	MoveWindow(nickchatbutton, rect.right/2+10, rect.top+70, 100, 30, TRUE);
}
