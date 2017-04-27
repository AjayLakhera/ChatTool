#include"TCPHeader.h"
#include<stdio.h>
#include<conio.h>
#include<windows.h>
//#include <winsock2.h>
//#include <ws2tcpip.h>



SERVICE_STATUS							MyServiceStatus; 
SERVICE_STATUS_HANDLE					MyServiceStatusHandle; 
void WINAPI 							AjayServiceStart (DWORD argc, LPTSTR *argv);
void WINAPI 	   						AjayServiceCtrlHandler (DWORD Opcode); 

TCPThread gTcpObj;
void CreateServ();
void DeleteServ();
int main(int argc, char *argv[])
{

	if (strcmp (argv[1], (const char*)"i")==0)
	{

		CreateServ();

		SERVICE_TABLE_ENTRY   DispatchTable[] = { 
	   { "AjayService", AjayServiceStart }, 
	   { NULL, NULL } };

	    StartServiceCtrlDispatcher( DispatchTable);
	} else if (strcmp (argv[1], (const char*)"u")==0)
	{

		DeleteServ();
	}
   	return 0;
} 


//VOID SvcDebugOut(LPSTR String, DWORD Status);
void WINAPI AjayServiceStart (DWORD argc, LPTSTR *argv) 
{ 
   
    MyServiceStatus.dwServiceType        = SERVICE_WIN32_OWN_PROCESS; 
    MyServiceStatus.dwCurrentState       = SERVICE_START_PENDING; 
    MyServiceStatus.dwControlsAccepted   = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE; 
    MyServiceStatus.dwWin32ExitCode      = 0; 
    MyServiceStatus.dwServiceSpecificExitCode = 0; 
    MyServiceStatus.dwCheckPoint         = 0; 
    MyServiceStatus.dwWaitHint           = 0; 
 
    MyServiceStatusHandle = RegisterServiceCtrlHandler( "AjayService",  AjayServiceCtrlHandler); 
 
    if (MyServiceStatusHandle == (SERVICE_STATUS_HANDLE)0) 
    { 
         return; 
    } 
 
    // Initialization complete - report running status. 
   // MyServiceStatus.dwCurrentState       = SERVICE_RUNNING; 
    
    SetServiceStatus (MyServiceStatusHandle, &MyServiceStatus); 
    return; 
} 
 
void WINAPI AjayServiceCtrlHandler (DWORD Opcode) 
{ 
   switch(Opcode) 
   { 
      case SERVICE_CONTROL_PAUSE:
		 //service wants paused
		 //MyServiceStatus.dwCurrentState = SERVICE_PAUSED; 
         break; 
 
      case SERVICE_CONTROL_CONTINUE:
		 gTcpObj.Connect ();
          //  service wants continue
          MyServiceStatus.dwCurrentState = SERVICE_RUNNING; 
         break; 
 
      case SERVICE_CONTROL_STOP: 
		 //service wants stop
		gTcpObj.Disconnect();
        MyServiceStatus.dwCurrentState  = SERVICE_STOPPED; 
       	break;
      
   }
 SetServiceStatus (MyServiceStatusHandle,  &MyServiceStatus);
}


void CreateServ()
{
    SC_HANDLE schSCManager;
    SC_HANDLE schService;
	TCHAR szPath[MAX_PATH]={0};
	char tempb[250]={0};
        
 /*   if( !GetModuleFileName( "D:\Hakathon\TCPThread\TCPThread\Debug\TCPThread.exe", szPath, MAX_PATH ) )
    {
		sprintf(tempb,"Cannot install service (%d)\n", GetLastError());
		MessageBox(NULL,tempb,"Error",0);
        return;
    }*/

    // Get a handle to the SCM database. 
 
    schSCManager = OpenSCManager( 
        NULL,                    // local computer
        NULL,                    // ServicesActive database 
        SC_MANAGER_ALL_ACCESS);  // full access rights 
 
    if (NULL == schSCManager) 
    {
		sprintf(tempb,"OpenSCManager failed (%d)\n", GetLastError());
		MessageBox(NULL,tempb,"Error",0);
        return;
    }

    // Create the service
    schService = CreateService( 
        schSCManager,              // SCM database 
        "AjayService",             // name of service 
        "TestTCPService",                   // service name to display 
        SERVICE_ALL_ACCESS,        // desired access 
        SERVICE_WIN32_OWN_PROCESS, // service type 
        SERVICE_DEMAND_START,      // start type 
        SERVICE_ERROR_NORMAL,      // error control type 
        "D:\\Hakathon\\TCPThread\\TCPThread\\Debug\\TCPThread.exe",                    // path to service's binary 
        NULL,                      // no load ordering group 
        NULL,                      // no tag identifier 
        NULL,                      // no dependencies 
        NULL,                      // LocalSystem account 
        NULL);                     // no password 
 
    if (schService == NULL) 
    {
        sprintf(tempb,"CreateService failed (%d)\n");
		MessageBox(NULL,tempb,"Message",0);   	
		CloseServiceHandle(schSCManager);
        return;
    }
    else
	{
		sprintf(tempb,"Service installed successfully\n");
		MessageBox(NULL,tempb,"Message",0);   	
	}

    CloseServiceHandle(schService); 
    CloseServiceHandle(schSCManager);
}


void DeleteServ()
{
    SC_HANDLE schSCManager;
    SC_HANDLE schService;
    SERVICE_STATUS ssStatus; 

	char tempb[250]={0};
    
    // Get a handle to the SCM database. 
 
    schSCManager = OpenSCManager( 
        NULL,                    // local computer
        NULL,                    // ServicesActive database 
        SC_MANAGER_ALL_ACCESS);  // full access rights 
 
    if (NULL == schSCManager) 
    {
		sprintf(tempb,"OpenSCManager failed (%d)\n", GetLastError());
		MessageBox(NULL,tempb,"Error",0);   	
	
        return;
    }

    // Get a handle to the service.

    schService = OpenService( 
        schSCManager,       // SCM database 
        "AjayService",          // name of service 
        DELETE);            // need delete access 
 
    if (schService == NULL)
    { 
		sprintf(tempb,"OpenService failed (%d)\n", GetLastError());
		MessageBox(NULL,tempb,"Error",0);   	
	    CloseServiceHandle(schSCManager);
        return;
    }

    // Delete the service.
 
    if (! DeleteService(schService) ) 
    {
		sprintf(tempb,"DeleteService failed (%d)\n", GetLastError());
		MessageBox(NULL,tempb,"Error",0);   	
	}
    else
	{
		sprintf(tempb,"Service deleted successfully\n");
		MessageBox(NULL,tempb,"Message",0);   	
	}
 
    CloseServiceHandle(schService); 
    CloseServiceHandle(schSCManager);
}