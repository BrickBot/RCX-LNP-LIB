/****************************************************************
/   file:   RCXLIB.cpp... 
/****************************************************************/
#define STRICT

#define USECOMM

#include <stdio.h>
#include <windows.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include "RCXLIB.h"

// constant definitions

#define BAUDRATE	2400
#define STOPBITS	0
#define PARITY		0
#define BYTESIZE	8

// ascii definitions

#define ASCII_XON       0x11
#define ASCII_XOFF      0x13


/*****************************************************************************
*  Global variables
*******************************************************************************/
HINSTANCE		hInst ;					//dll's instance handle


static char *GlobMsg[1000];

// state of the LNP protocol when receiving a packet
typedef enum { LNPwaitHeader, LNPwaitLength, LNPwaitData, LNPwaitCRC } lnp_integrity_state_t;

// LNP packet is copied here after it has been received
static char packet[1000];

static char RCXString;



/////////////////////////////////////////////////////////////
// lnp_checksum
/////////////////////////////////////////////////////////////

//unsigned char lnp_checksum( const char *data, unsigned length )
unsigned char lnp_checksum( const unsigned char *data, unsigned length )
{
  unsigned char a = 0xff;
//  unsigned char b = 0xff;
//  This variable was in the original code but it is ignored by the cast done
//  when the validation

  while (length > 0) {
    a = a + *data;
//    b = b + a;
    data++;
    length--;
  }

  return a;// + (b << 8);
}



/////////////////////////////////////////////////////////////
// lnp_integrity_byte
/////////////////////////////////////////////////////////////

int lnp_integrity_byte(unsigned char b)
{
	static unsigned char buffer[256];
  	static int bytesRead,endOfData;
	static int lnp_integrity_state = LNPwaitHeader;

	if(lnp_integrity_state==LNPwaitHeader)
    	bytesRead=0;

  	buffer[bytesRead++]=b;

	switch(lnp_integrity_state)
	{
    case LNPwaitHeader:
    	// valid headers are 0xf0 .. 0xf7		
		if((b & (unsigned char) 0xf8) == (unsigned char) 0xf0)
       		lnp_integrity_state++;
       	break;

   	case LNPwaitLength:
	   	endOfData=b+2;		
      	lnp_integrity_state++;
      	break;

    case LNPwaitData:
      	if(bytesRead==endOfData)
		lnp_integrity_state++;
	    break;

    case LNPwaitCRC:
      	if(b == (unsigned char)lnp_checksum(buffer,endOfData))
      	{
	        buffer[buffer[1]+2]=0;
			
			//strcpy(packet,buffer+2);
strcpy(packet,reinterpret_cast<CONST CHAR*>(buffer+2));
  			lnp_integrity_state=LNPwaitHeader;
			return 1;
	    }
		else MessageBox(0,"bad checksum\n","error",MB_OK);//printf("bad checksum\n");
	}
	return 0;
}



/*****************************************************************************
* Public (Exportable) Functions
*
*
******************************************************************************/

/////////////////////////////////////////////////////////////
//Main DLL function
/////////////////////////////////////////////////////////////

//This is a window's DLL's equivalent to WinMain()

BOOL WINAPI DllMain(HINSTANCE hInstA, DWORD dwReason, LPVOID lpvReserved)
{
	switch (dwReason) {
	case DLL_PROCESS_ATTACH:
		// The DLL is being mapped into the process's address space
		hInst = hInstA;
		
		// Initialize here if necessary*******
		//************************************
	   	//
		//
		break;

	case DLL_THREAD_ATTACH:
		// A thread is being created
	    break;

	case DLL_THREAD_DETACH:
		// A thread is exiting cleanly
	    break;

	case DLL_PROCESS_DETACH:
		// The DLL is being unmapped from the process's address space
		// Cleanup here if necessary*************
		//***************************************
		hInst = 0;
	    break;
	}

	return TRUE;
}





/////////////////////////////////////////////////////////////
// Send func
/////////////////////////////////////////////////////////////

__declspec(dllexport) WINAPI SendToRCX(LPCTSTR RCXString)
{
char szPort[6];		// com1 or com2 - processed from command line
    DCB        dcb ;	// device control block to setup communication port 	
	BOOL       fRetVal, fWriteStat;  // some return values from system calls
	DWORD       dwBytesWritten;
	long filelen;	// length of file containing message
	HANDLE comHandle;	// communication port file handle
	COMMTIMEOUTS CommTimeOuts;	// timoeout settings for communication port
	int wakeup, pu;
unsigned char msg[1000];	// send/receive buffer
	clock_t t1,t;	// what time is it?



pu = 0; wakeup = 0;
strcpy(szPort, "COM1");


   // open COMM device


    if ((comHandle =
       CreateFile( szPort, GENERIC_READ | GENERIC_WRITE,
                  0,                    // exclusive access
                  NULL,                 // no security attrs
                  OPEN_EXISTING,
                  FILE_ATTRIBUTE_NORMAL, // |
                  NULL )) == (HANDLE) -1 )
    {
		//perror("cannot open communication port");
		MessageBox(0,"cannot open communication port","error",MB_OK);
		return(1);
	}

    // setup device buffers
    SetupComm(comHandle, 4096, 4096) ;

    // purge any information in the buffer
    PurgeComm(comHandle, PURGE_TXABORT | PURGE_RXABORT |
                         PURGE_TXCLEAR | PURGE_RXCLEAR ) ;

    // set up timeouts for serial port
    CommTimeOuts.ReadIntervalTimeout = MAXDWORD;  
    CommTimeOuts.ReadTotalTimeoutMultiplier = 0 ;
    CommTimeOuts.ReadTotalTimeoutConstant = 0; 
    CommTimeOuts.WriteTotalTimeoutMultiplier = 1 + 8 * 1000 / BAUDRATE;  // this is pure guess
    CommTimeOuts.WriteTotalTimeoutConstant = 1000 ;
    SetCommTimeouts(comHandle, &CommTimeOuts ) ;
   
	// communication parameters
    dcb.DCBlength = sizeof( DCB ) ;
    GetCommState( comHandle, &dcb ) ;
    dcb.BaudRate = BAUDRATE;
    dcb.ByteSize = BYTESIZE;
    dcb.Parity = PARITY;
    dcb.StopBits = STOPBITS;

   // setup hardware flow control (I don't know anything about this...)
   dcb.fOutxDsrFlow = 0;
   dcb.fDtrControl = DTR_CONTROL_ENABLE;
   dcb.fOutxCtsFlow = 0;
   dcb.fRtsControl = RTS_CONTROL_ENABLE;
   // setup software flow control
   dcb.fInX = dcb.fOutX = 1;
   dcb.XonChar = ASCII_XON;
   dcb.XoffChar = ASCII_XOFF;
   dcb.XonLim = 100;
   dcb.XoffLim = 100;
   // other various settings
   dcb.fBinary = TRUE;
   dcb.fParity = TRUE;
   fRetVal = SetCommState(comHandle, &dcb) ;
   EscapeCommFunction(comHandle, SETDTR);

   if (!fRetVal)
   {
         CloseHandle( comHandle ) ;
		// printf("SetCommState()\n");
         MessageBox(0,"SetCommState()","error",MB_OK);
		 return(1);
   }



/////////////////////////////////
filelen=strlen(RCXString);


long cnt=0;

do
{
msg[cnt+2]=RCXString[cnt];
cnt++;
} while (cnt < filelen);

			msg[0] = (char)0xf0;
			msg[1] = (char)filelen + 1;
			msg[filelen + 2] = (char)0x00;
			msg[filelen + 3] = (char)lnp_checksum(msg,filelen + 3);
	fWriteStat = WriteFile(comHandle,msg, filelen + 4,
                           &dwBytesWritten, NULL); 


	// this was in Manuel's code, closing the serial port...
	SetCommMask(comHandle, 0);
	EscapeCommFunction(comHandle, CLRDTR);

	// purge any outstanding reads/writes and close device handle

	PurgeComm(comHandle, PURGE_TXABORT | PURGE_RXABORT |
                        PURGE_TXCLEAR | PURGE_RXCLEAR ) ;
	CloseHandle(comHandle) ;

return 0;

}




//////////////////////////////////////////////////////////
// Get func
//////////////////////////////////////////////////////////


__declspec(dllexport) CHAR* WINAPI GetFromRCX()
{
char szPort[6];		// com1 or com2 - processed from command line
    DCB        dcb ;	// device control block to setup communication port 	
	BOOL       fReadStat, fRetVal, fWriteStat;  // some return values from system calls
	DWORD       dwBytesWritten, dwLength;
	long filelen;	// length of file containing message
	HANDLE comHandle;	// communication port file handle
	COMMTIMEOUTS CommTimeOuts;	// timoeout settings for communication port
	int wakeup, pu, end_of_message, timeout;	//wakeup IR tower? print usage?, message end?, how long to wait when nothing is comming?
	unsigned int i;	// i
char msg[1000];	// send/receive buffer
	clock_t t1,t;	// what time is it?



pu = 0; 
wakeup = 1;
timeout=2000;
strcpy(szPort, "COM1");
msg[0] = 0;
filelen = -3;

   // open COMM device


    if ((comHandle =
       CreateFile( szPort, GENERIC_READ | GENERIC_WRITE,
                  0,                    // exclusive access
                  NULL,                 // no security attrs
                  OPEN_EXISTING,
                  FILE_ATTRIBUTE_NORMAL, // |
                  NULL )) == (HANDLE) -1 )
    {
		//perror("cannot open communication port");
		MessageBox(0,"cannot open communication port","error",MB_OK);
		return "error";
	}

    // setup device buffers
    SetupComm(comHandle, 4096, 4096) ;

    // purge any information in the buffer
    PurgeComm(comHandle, PURGE_TXABORT | PURGE_RXABORT |
                         PURGE_TXCLEAR | PURGE_RXCLEAR ) ;

    // set up timeouts for serial port
    CommTimeOuts.ReadIntervalTimeout = MAXDWORD;  
    CommTimeOuts.ReadTotalTimeoutMultiplier = 0 ;
    CommTimeOuts.ReadTotalTimeoutConstant = 0; 
    CommTimeOuts.WriteTotalTimeoutMultiplier = 1 + 8 * 1000 / BAUDRATE;  // this is pure guess
    CommTimeOuts.WriteTotalTimeoutConstant = 1000 ;
    SetCommTimeouts(comHandle, &CommTimeOuts ) ;
   
	// communication parameters
    dcb.DCBlength = sizeof( DCB ) ;
    GetCommState( comHandle, &dcb ) ;
    dcb.BaudRate = BAUDRATE;
    dcb.ByteSize = BYTESIZE;
    dcb.Parity = PARITY;
    dcb.StopBits = STOPBITS;

   // setup hardware flow control (I don't know anything about this...)
   dcb.fOutxDsrFlow = 0;
   dcb.fDtrControl = DTR_CONTROL_ENABLE;
   dcb.fOutxCtsFlow = 0;
   dcb.fRtsControl = RTS_CONTROL_ENABLE;
   // setup software flow control
   dcb.fInX = dcb.fOutX = 1;
   dcb.XonChar = ASCII_XON;
   dcb.XoffChar = ASCII_XOFF;
   dcb.XonLim = 100;
   dcb.XoffLim = 100;
   // other various settings
   dcb.fBinary = TRUE;
   dcb.fParity = TRUE;
   fRetVal = SetCommState(comHandle, &dcb) ;
   EscapeCommFunction(comHandle, SETDTR);

   if (!fRetVal)
   {
         CloseHandle( comHandle ) ;
		 //printf("SetCommState()\n");
         MessageBox(0,"SetCommState()","error",MB_OK);
		 return "error";
   }




		fWriteStat = WriteFile(comHandle, msg, filelen + 4,
                           &dwBytesWritten, NULL); 

		if (!fWriteStat) {
			//printf("write error\n");
		MessageBox(0,"write error","error",MB_OK);
		}


end_of_message = 0;
		// when we reach time t, we will be waiting already too long
		t1 = clock(); t = t1 + CLOCKS_PER_SEC * timeout / 1000;

		// wait until LNP receives some message or timeout
		while (!end_of_message && (t1 < t))
		{			
			dwLength = 100;
			// see what's available at serial port, returns immediately
			fReadStat = ReadFile(comHandle, msg, dwLength, &dwLength, NULL); 
			if (!fReadStat) {
				//printf("read error");
				MessageBox(0,"read error","error",MB_OK);
				return "error";
			}

			// process all received bytes through LNP
			for (i = 0; i < dwLength && !end_of_message; i++)
				if (lnp_integrity_byte(msg[i])) end_of_message = 1;			

			t1 = clock();
		}

		// if the loop ended because of timeout...
		if (!end_of_message) packet[0]=0;


*GlobMsg = new char [strlen(packet)+1];
delete [] *GlobMsg;

strcpy(*GlobMsg,packet);

		
	// this was in Manuel's code, closing the serial port...
	SetCommMask(comHandle, 0);
	EscapeCommFunction(comHandle, CLRDTR);

	// purge any outstanding reads/writes and close device handle

	PurgeComm(comHandle, PURGE_TXABORT | PURGE_RXABORT |
                        PURGE_TXCLEAR | PURGE_RXCLEAR ) ;
	CloseHandle(comHandle) ;
		
return (*GlobMsg);

}




