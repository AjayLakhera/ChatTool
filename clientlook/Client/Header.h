//WINDOWS.H is a master header file that includes other Windows header files
//#include <windows.h>
//#include<Commctrl.h>
#include <stdio.h>
#include <conio.h>
#include <winsock2.h>
#include<Richedit.h>
#include<commctrl.h>


//Global Variables
HINSTANCE	ghInstance;
HWND	hWnd;			// window handle
HWND	editbox;
HWND	chatbox;
HWND	listview;
HWND	submitbutton;

HWND	nickstat;
HWND	nickeditbox;
HWND	nickchatbutton;



HWND	usercaption;
// thread and event object
HANDLE	RecThread=NULL;
HANDLE	RecEvent;

SOCKET	ConnectSocket = INVALID_SOCKET;		///< socket object

#define			MSG_ERROR							-1
#define			MAX_SIZE								255				///< maximum buffer size
#define			IP_ADDRESS							"192.168.0.103"    ///< ip address
#define			DEFAULT_PORT					6555				///< port number
#define			SIZE										10
#define			ID_LIST_BOX						100
#define			ID_RICH_EDIT_BOX			101
#define			ID_EDIT_BOX						102
#define			ID_CONNECT_BUTTON		103
#define			ID_DISCONNECT_BUTTON	104
#define			ID_CHAT_BUTTON				105
#define			ID_USER_CAPTION				106

#define         ID_NICK_EDIT						107
#define         ID_NICK_REG						108
#define         ID_NICK_STATIC					109


LRESULT WINAPI	MainWndProc			(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
DWORD WINAPI	RecThreadProc		(LPVOID lpParam) ;
void			ProcessSubmit							(void);
void			DisplayText								(char* pBuffer, int pNxtLine = FALSE);
void			SendRecReq							(void);
int				ReceiveReq								(void);
int				SendNick									(void);
int				SendRequest							(char * pBuffer, int pPosition, HWND pHwnd=NULL);
int				ReceiveRequest						(void);
int				SendMessage							(void);
int				SendDisconnect						(HWND pHwnd);
void			ParseUserList							(char * pBuffer);
void			ProcessRefreshUser					(HWND pHWnd);
void			CreateConnection						(void);				
void			ParseSessionID						(char* pMessage);
int				SendNickMsg							(void);
int  			ParseResponse							(char * buffer);

//Change your nickname. 
#define	NICK				1
//Send a message. The second function allows usage like printf. 
#define	PRIVMSG		2
//Join a channel. 
#define	JOIN				3
//Disconnect from the IRC server. 
#define	QUIT				4
//Send a notice. The second function allows usage like printf. 
#define	NOTICE			4
//Kick somebody from a channel. 
#define	KICK				6
//Change channel or user modes. 
#define	MODE				7

#define DLG_MSG					WM_USER+1 
#define CLIENT_DLG_MSG  WM_USER+2

extern char Session[124];
extern char sNick[10];


//int CALLBACK DlgProc( HWND pWnd, UINT msg, WPARAM wParam, LPARAM lParam );
void ParsePrivateMsg				(char * pBuffer);
int		ProcessPrivateMessage	(char * pBuffer);

void SetWindowPosition			(HWND pHWnd);
void SetNickControlPositions	(HWND pDlgWnd);

void CreateControls					(HWND pHWnd);

void SetWindowPosition			(HWND pHWnd);

LRESULT CALLBACK ChatDialogProc (HWND pHWnd, UINT pMessage, WPARAM pWParam, LPARAM pLParam);
//int CALLBACK ChatDialogProc (HWND pHWnd, UINT pMessage, WPARAM pWParam, LPARAM pLParam);

void ProcessClickEvent			(WPARAM pWParam, LPARAM pLParam);
void CreateChatWindow			(char * pBuffer);
void OpenChatWindow			();
void ResetControls					();
void SendChatMessage			(HWND pHWnd);
void RegisterNickName			();
BOOL CALLBACK GoToProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);

BOOL CALLBACK GoToNickProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam) ;
void CreateNickGlgControls			(HWND pNickDLG);
void SetNickControlPositions		(HWND pDlgWnd);
//HWND Handle[5]={NULL};
//HWND Handle	 = NULL;
HWND NickDLG = NULL;

static int count = 0;

struct ChatHandle{
	HWND Handle;
	struct ChatHandle * Next;
};

struct ChatHandle * First = NULL;

int SetClientWinHandle(HWND pHandle, char * pTitle){
	
		struct ChatHandle * temp;
		struct ChatHandle * node;

	if(count == 5){
		return 0;
	}

	node = (struct ChatHandle *)calloc(1, sizeof(struct ChatHandle));
	node->Handle = pHandle;
	node->Next   = NULL;
		
	if(First == NULL){
		First = node;
	} else{

		temp = First;
	
		while(temp->Next != NULL){
			temp = temp->Next;
		}

		temp->Next = node;
	}
	count++;
	return 1;
}

int DeleteHandle(HWND pHandle){
	
	struct ChatHandle * temp;
	struct ChatHandle * node;
	struct ChatHandle * next;

	temp = First;

	if(First == NULL){
		return 1;

	} else if(First->Handle == pHandle){
		First = First->Next;
		free (temp);
		count--;
	} else{
		
		next = First->Next;
	
		while(next != NULL){
			
			if(next->Handle == pHandle){
				
				temp->Next = next->Next;
				free (temp);
				count--;
				break;
			}
			temp = temp->Next;
			next = next->Next;
		}
	}
	return 1;
}

HWND CheckDialogExists (char * pTitle)
{
		struct ChatHandle * node;
		char  title[10]={0};

	node = First;
	while(node != NULL){
		 GetWindowText(node->Handle, title, 10);
		 if(strcmp(title, pTitle) ==0){
			return node->Handle;
		}
		 node = node->Next;
	}
	return NULL;
}