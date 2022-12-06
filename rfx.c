#include "rfx.h"


rdr_stat 	g_rdr_state;
rdr_stat	tempState;
bool 		statusPendingToBeSent;

sigset_t 			usr1_mask1;
sigset_t 			usr1_mask2;
sigset_t 			usr2_mask1;
sigset_t 			usr2_mask2;
reader_handle_t rh1;
int	boConnect;
int     boConnected;
//The connecting socket fd
int 				sConnect;

//Operational mode: ACS / AMS
op_mode_t		opMode;
//Id of the reader. Will be initialized during init from system_params.txt.
char readerId[3];

_retflag log_action(int dest, char * string)
{
#ifdef __DEBUG
	switch (dest)
	{
		case _SCREEN:
			printf(string);
			printf("\n");
			break;
		default:
			break;
	}
#else

#endif
	return _SUCCESS;
}
char * btoa(int len, char * str)
{
	int i;
	char * retval = (char *) malloc(len);
	memset(retval,0,len);
	for (i=0; i<len; i++)
	{
		retval[i]=str[i];
		printf("%x",retval[i]);
	}
	printf("\n");
	return retval;
}

void tostr(tagdb_record_v3_t tr1, char * id)
{
   int i;
   char tmp[5];
   for (i=0; i<7; i++)
   {
      if (i == 0)
          sprintf(id, "%04X", tr1.tag_id[i]);
      else{
	  memset(tmp,0,5);
	  sprintf(tmp, "%04X", tr1.tag_id[i]);
	  strcat(id, tmp);
      }
   }

}

_retflag  sendBytesThroughSocket(void* sendMsg, short msgSize)
{
	int sentBytes;
	_retflag retval = _SUCCESS;
	char* msg;
	int i;
	
	sentBytes = send(sConnect, sendMsg, msgSize , 0) ;
	if(sentBytes == -1)
	{
		printf("Sending message failed ... \n");
		 retval = _FAIL;
	}
	printf("Send success ... bytes sent %d\n", sentBytes);
	return retval;
}


_retflag sendStatusToMonitor()
{
	unsigned char eventStr[3];
	_retflag retval = _FAIL;

	//Build the string to be sent
	bzero(eventStr, 3);
	
	eventStr[0] = LOG_EVENT;
	sprintf(eventStr+1, "%02x", g_rdr_state);
	retval = sendBytesThroughSocket((void*)eventStr, 3);
	return retval;
}

void sendPIDToMonitor()
{
	unsigned char pidStr[20];
	_retflag retval = _FAIL;

	//Build the string to be sent
	bzero(pidStr, 20);
	
	sprintf(pidStr, "PID%d", getpid());
	sendBytesThroughSocket((void*)pidStr, strlen(pidStr));
}

void connectMonitor()
{
	struct sockaddr_in dest;
	time_t tmt, tmt2;
	int cl, port_monitor=0;
	char p_value[PARAM_SIZE];
	
	//Create a connecting socket       
	if ((cl = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
	{
		printf("Error Socket creation\n");
		exit(0);
	}
	else
	{
		printf("Conn: Socket created\n");    
	}
   
	//Initialize client machine's connecting socket with server 
	//machine's details - where to connect to
	bzero(p_value, PARAM_SIZE);
	if(SearchParamList("M4_READER_PORT",p_value)==_FAIL)
	{
		log_action(_SCREEN,"Failed retrieving M4_READER_PORT");
		Fail_Action();
	}
	port_monitor = atoi(p_value);
	
	bzero(p_value, PARAM_SIZE);
	if (SearchParamList("M4_READER_IP",p_value)==_FAIL)
	{
		log_action(_SCREEN,"Failed retrieving M4_READER_IP");
		Fail_Action();
	}	
	bzero(&dest, sizeof(dest));
	dest.sin_family = AF_INET;
	dest.sin_port = htons(port_monitor);
	printf("IP %s \n", p_value);
	if(inet_aton(p_value, &dest.sin_addr.s_addr) == 0 )//"192.168.1.119"
	{
		printf("Conn: IP format conversion failed ...\n");
		exit(0);
	}      
   
	//Connect with server - try every 2 seconds if fails
	while(connect(cl, (struct sockaddr*)&dest, sizeof(dest)) != 0)
	{
		printf("Conn: Connecting with server ...  failed!\n");    
		sleep(2);
	}
	printf("Connect success\n");
	sConnect = cl;
}

void connectBO()
{
	struct sockaddr_in dest;
	time_t tmt, tmt2;
	int cl, port_monitor=0;
	char p_value[PARAM_SIZE];
	
	//Create a connecting socket       
	if ((cl = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
	{
		printf("Error Socket creation\n");
		exit(0);
	}
	else
	{
		printf("Conn: Socket created\n");    
	}

	boConnect = cl;

}

char*	getCurrentTimestamp()
{
	//Get current time and return 14 bytes timestamp in the fiollowing format:
	//yyyymmddhhmmss
	char *timeStamp;
	struct tm *local;
	time_t currTime = time(NULL);	
	local = localtime(&currTime);
	
	timeStamp = (char*)malloc(sizeof(char)*15);
	bzero(timeStamp, 15);	
	sprintf(timeStamp, "%04d%02d%02d%02d%02d%02d\n", local->tm_year+1900, local->tm_mon+1, \
		local->tm_mday, local->tm_hour, local->tm_min, local->tm_sec);
	return timeStamp;	

/* for BM
	int isOK,fd;

	if ((isOK = openPort(&fd))!=_SUCCESS)
	{
		printf("Error in opening Serial Port, aborting...");
		return _FAIL;
	}
	return GetBMTime(fd);
*/

}

