#include "init.h"

// Glabal variable to store the address of the header node of TagId linked list
TagList	* g_TagList_head;
// Glabal variable to store the address of the header node of param linked list
ParamList	* g_ParamList_head;
// Glabal variable to store the socket file descriptor 
int g_sockfd;

// Read all tag id from tag_id.txt and create a linked list containing all those ids.
int CreateTagList()
{
   TagList * curr, * tmp;
   char fstr[TAG_ID_SIZE + 1];
   FILE * fp;
   static int cnt = 0;
	int i;

   curr = NULL;
   g_rdr_state = LOAD_TAG;

   if( !cnt )
      cnt = 1;
   else
      DeleteTagList();

	#ifdef FOR_READER
		log_action(_SCREEN,"going to open /tm/bin/tag_id.txt");
   		fp = fopen("/tm/bin/tag_id.txt","r");
	#else
		log_action(_SCREEN,"going to open tag_id.txt");
		fp = fopen("tag_id.txt","r");
	#endif

   if( fp == NULL)
   {
      log_action(_SCREEN,"\"tag_id.txt\" Open Error\n");
      return _FAIL;
   }
   
   memset( fstr, '\0', TAG_ID_SIZE + 1);
   while(fscanf( fp, "%s", fstr) != EOF)
   {
      tmp = (TagList *)malloc(sizeof(TagList));
      memset( tmp->TagId, '\0', TAG_ID_SIZE + 1 );
      strcpy( tmp->TagId, fstr );
      tmp->next = NULL;
            
      if( curr == NULL )
	curr = g_TagList_head = tmp;
      else
      {
	curr->next = tmp;
	curr = tmp;
      }
   }
   fclose(fp);

	curr = g_TagList_head;
	while(curr != NULL)
	{
		printf("ID %s\n", curr->TagId);
		curr = curr->next;
	}

	
   return _SUCCESS;
}
/***********************************************************************************/
// Free the memory allocated by the linked list containing tag ids.
int DeleteTagList()
{
	TagList * prev, * curr;

	if(g_TagList_head == NULL)
		return _SUCCESS;

	curr = g_TagList_head;
	while( curr != NULL )
	{
		prev = curr;
		curr = curr->next;
		free(prev);
	}
	
	return _SUCCESS;
}
/***********************************************************************************/
/* Read all param name and value from system_param.txt and business_param.txt and create a linked list
	containing all those param and values.*/
int CreateParamList()
{
   ParamList * curr, * tmp;
   char fstr[PARAM_SIZE];
   FILE * fp;
   static int cnt = 0;

	//Initialize
	curr = NULL;
	g_rdr_state = LOAD_PAR;
	DeleteParamList();

	log_action(_SCREEN,"CreateParam");
	//'''' from 1st param file ''''''''''''

	#ifdef FOR_READER
		fp = fopen("/tm/bin/system_param.txt","r");
	#else
		fp = fopen("system_param.txt","r");
	#endif

   if( fp == NULL)
   {
      log_action(_SCREEN,"\"system_param.txt\" Open Error\n");
	getc(stdin);
      return _FAIL;
   }
   memset( fstr, 0, PARAM_SIZE );
   while(fscanf( fp, "%s", fstr) != EOF)
   {
      tmp = (ParamList *)malloc(sizeof(ParamList));
      strcpy( tmp->pname, fstr );			printf("pname: %s\n", fstr);

      memset( fstr, 0, PARAM_SIZE );
      fscanf( fp, "%s", fstr );
      strcpy( tmp->pvalue, fstr );			printf("pvalue: %s\n", fstr);      
      tmp->next = NULL;
            
      if( curr == NULL )
	curr = g_ParamList_head = tmp;
      else
      {
	curr->next = tmp;
	curr = tmp;
      }
   }
   log_action(_SCREEN,"End File");

   fclose(fp);

//''''' from 2nd param file ''''''''''''
#ifdef FOR_READER
	fp = fopen("/tm/bin/business_param.txt","r");
#else
	fp = fopen("business_param.txt","r");
#endif

   if( fp == NULL)
   {
     log_action(_SCREEN,"\"business_param.txt\" Open Error\n");
      return _FAIL;
   }
   memset( fstr, 0, PARAM_SIZE );
   while(fscanf( fp, "%s", fstr) != EOF)
   {
      tmp = (ParamList *)malloc(sizeof(ParamList));
      strcpy( tmp->pname, fstr );
      memset( fstr, 0, PARAM_SIZE );
      fscanf( fp, "%s", fstr );
      strcpy( tmp->pvalue, fstr );
      tmp->next = NULL;
            
      if( curr == NULL )
	curr = g_ParamList_head = tmp;
      else
      {
	curr->next = tmp;
	curr = tmp;
      }
   }
   fclose(fp);

   return _SUCCESS;
}

// Free the memory allocated by the linked list containing param and values.
int DeleteParamList()
{
	ParamList * prev, * curr;

	if(g_ParamList_head == NULL)
		return _SUCCESS;

	curr = g_ParamList_head;
	while( curr != NULL )
	{
		prev = curr;
		curr = curr->next;
		free(prev);
	}
	
	return 0;
}

// Search the tag id specified by input parameter "readId" in the linked list containing tag ids.
int SearchTagList(char * readId)
{
	TagList * curr;
	curr = g_TagList_head;
	int i;

	while( curr != NULL )
	{
		if(strcasecmp(curr->TagId, readId)==0)
		{
			#ifdef __DEBUG
				printf("Id: %s ......Matched\n", readId);
			#endif
			return _SUCCESS;
		}
		else
	      		curr = curr->next;
	}
	#ifdef __DEBUG
	printf("Id: %s ......NOT Matched\n", readId);  
	#endif
	return _FAIL;
}

/* Search the param specified by input parameter "param" in the linked list containing param and values and if the param found store the value in output paramater "value".*/
int SearchParamList(char * param, char * value)
{
	ParamList * curr;
	curr = g_ParamList_head;
	while( curr != NULL )
	{
	   if(strcasecmp(curr->pname, param)==0)
           {
   	     strcpy( value, curr->pvalue );
	     
             return _SUCCESS;
	  }
	  else
	     curr = curr->next;
	}
	#ifdef __DEBUG
	printf("%s Not Found\n", param);  
	#endif

	return ERR_NO;
}

/* The function to initialize client socket and connect for date time request */
int initializesocket()
{
   struct sockaddr_in dest;
   char buffer[PARAM_SIZE];
   unsigned short port;
   FILE * fp;
   

   bzero(buffer, PARAM_SIZE);
   if( SearchParamList( "TIME_SET_PORT", buffer ) != _SUCCESS ) 
       return ERR_NO;
   sscanf( buffer, "%u", &port );

   bzero(buffer, PARAM_SIZE);
   if( SearchParamList( "TIME_SET_IP", buffer ) != _SUCCESS)	
       return ERR_NO;	

   /*---Open socket for streaming---*/
	
   //Set to blocking
   if ( ( g_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
   {
       perror("Socket");
       return ERR_NO;
   }						
   /*---Initialize server address/port struct---*/
   bzero(&dest, sizeof(dest));
   dest.sin_family = AF_INET;
   dest.sin_port = htons(port);
   if ( inet_aton(buffer, &dest.sin_addr.s_addr) == 0 )
   {
       perror(buffer);
       return ERR_NO;
   }
   /*---Connect to server---*/
	//If connection is requested once and the program quits if it doesn't get one use if
	//else use while to get a successful connection first
 
   while(1)
	{
	log_action(_SCREEN,"Waiting for time sync");
	if (connect( g_sockfd, (struct sockaddr*)&dest, sizeof(dest)) == 0)
		break;
	else
		sleep(50);
	}
   return 0;
}
/***********************************************************************************/
/* The function to set date time and exit from the system if not succeed */
int m4SetDateTime()
{
   fd_set readfds;
   fd_set writefds;
   fd_set exceptfds;
   char buffer[MAXBUF]; 
   char tmpbuf[5];
   int day, mon, year, hr, min, sec;
   time_t tmt, tmt2;
   struct tm tmm;
   char * p; 
   char ch=0x01;
   int i, timeout, stat;

   bzero(buffer, MAXBUF);
   day = mon = year = hr = min = sec = 0;
   g_rdr_state = SET_DTTM;

   if( SearchParamList( "SERVER_TIMEOUT", buffer ) != _SUCCESS )
       return ERR_NO; 
   sscanf( buffer, "%d", &timeout );

   if( initializesocket() != 0 )
	 return ERR_NO;//break;

   if( send( g_sockfd,&ch,1,0) != 1 )
       return ERR_NO;

   FD_ZERO(&readfds);
   FD_ZERO(&writefds);
   FD_ZERO(&exceptfds);
   FD_SET(g_sockfd, &readfds);
   struct timeval tmout;
   tmout.tv_sec = timeout;
   tmout.tv_usec = 0;

   stat = select(g_sockfd+1, &readfds, &writefds, &exceptfds, &tmout);	
   if( !FD_ISSET( g_sockfd, &readfds ) )
       return ERR_NO;
   if( recv( g_sockfd, buffer, sizeof(buffer), 0) != 15 )
       return ERR_NO;

   buffer[14]='\0';		//	printf("Recvd: %s\n",buffer);

   close( g_sockfd );

   p = buffer;
   memcpy((char*)tmpbuf, (char*)p, 2);tmpbuf[2] = '\0';		//printf("%s\n",tmpbuf);
   sscanf(tmpbuf, "%d", &day);   		//printf("%02d",day);
   memset(tmpbuf, 0, 5);
   p += 2;
   memcpy(tmpbuf, p, 2);tmpbuf[2] = '\0';
   sscanf(tmpbuf, "%d", &mon);			//printf("%02d",mon);
   memset(tmpbuf, 0, 5);
   p += 2;
   						//printf("%s\n",p);
   memcpy(tmpbuf, p, 4);tmpbuf[4] = '\0';
   sscanf(tmpbuf, "%d", &year);			//printf("%04d",year);
   memset(tmpbuf, 0, 5);
   p += 4;
   memcpy(tmpbuf, p, 2);tmpbuf[4] = '\0';
   sscanf(tmpbuf, "%d", &hr);			//printf("%02d",hr);
   memset(tmpbuf, 0, 5);
   p += 2;
   memcpy(tmpbuf, p, 2);tmpbuf[2] = '\0';
   sscanf(tmpbuf, "%d", &min);			//printf("%02d",min);
   memset(tmpbuf, 0, 5);
   p += 2;
   memcpy(tmpbuf, p, 2);tmpbuf[2] = '\0';
   sscanf(tmpbuf, "%d", &sec);			//printf("%02d\n",sec);
   
   tmm.tm_mday = day;
   tmm.tm_mon = mon-1;
   tmm.tm_year= year-1900;
   tmm.tm_hour = hr;
   tmm.tm_min = min;
   tmm.tm_sec = sec;
   tmt = mktime(&tmm);  
   				//	printf("Time to set: %s", ctime(&tmt));
   stime(&tmt);
   tmt2 = time(NULL);
   #ifdef __DEBUG
   printf("Current Time: %s", ctime(&tmt2));
   #endif

   return 0;
}

//ACS_Monitor has notified that tag_id.txt has been updated.
//So update taglist
int handleSigusr1(int i)
{
	char p_value[PARAM_SIZE];

	CreateTagList();
	CreateParamList();
	if (SearchParamList("ANT_POWER",p_value)==_FAIL)
	{
		log_action(_SCREEN,"Failed retrieving ANTENNA POWER");
		Fail_Action();
	}
	if(set_tx_power(rh1, atoi(p_value)) == 1.0)
	{
		log_action(_SCREEN,"Failed to set antenna power");
		Fail_Action();
	}
	signal(SIGUSR1, handleSigusr1);
}

//ACS_Monitor has notified that BackOffice wants current ACS status.
//So send it
int handleSigusr2(int i)
{
	if(sendStatusToMonitor() == _FAIL)
		printf("ACS event could not be sent\n");
}
/***********************************************************************************/
int 	handleSigterm(int i)
{
	exit(0);
}
/***********************************************************************************/
int 	handleSigtstp(int i)
{
	exit(0);
}
/***********************************************************************************/
int 	handleSigquit(int i)
{
	exit(0);
}
/***********************************************************************************/
int	handleSigabrt(int i)
{
	exit(0);
}
/***********************************************************************************/
int 	handleSigint(int i)
{
	exit(0);
}
/***********************************************************************************/
int	handleSigsegv(int i)
{
	exit(0);
}

int handleSigpipe(int i)
{
	signal(SIGPIPE, handleSigpipe);	
}
/***********************************************************************************/
void acsInit()
{
	char p_value[PARAM_SIZE];
	
	log_action(_SCREEN,"V1.0.0");
	#ifdef FOR_READER
		log_action(_SCREEN,"#define FOR_READER uncommented");
	#endif
        
	//Iniitialize tag lists
	g_TagList_head = NULL;
	CreateTagList();
	
	//Initialize parameter lists
	g_ParamList_head = NULL;
	if(CreateParamList()==_FAIL)
	{
		log_action(_SCREEN,"Error in creating params");
	}

	//Initialize the operational mode of the reader
	memset(p_value,0,PARAM_SIZE);
	if(SearchParamList("READER_MODE",p_value)==_FAIL)
	{
		log_action(_SCREEN,"Error in retrieving operational mode");
		Fail_Action();
	}
	if(strcmp(p_value, "ACS_MASTER")==0)
		opMode = ACS_MASTER;
	else
	if(strcmp(p_value, "ACS_SLAVE")==0)
		opMode = ACS_SLAVE;
	else
	if(strcmp(p_value, "AMS")==0)		
		opMode = AMS;
	
	//Initialize reader id
	bzero(readerId, 3);
	memset(p_value,0,PARAM_SIZE);
	if(SearchParamList("READER_ID",p_value)==_FAIL)
	{
		log_action(_SCREEN,"Error in retrieving READER_ID");
		Fail_Action();
	}
	memcpy(readerId, p_value, 2);
	
	//Initialize mask
	if(sigemptyset(&usr1_mask1) == -1)
		exit(0);
	if(sigemptyset(&usr1_mask2) == -1)
		exit(0);
	
	//Prepare signal management
	if(sigaddset(&usr1_mask1, SIGUSR1) != 0)
	{
		printf("SIGUSR1 cannot be added to mask1\n");
	}
	if(sigaddset(&usr1_mask1, SIGTERM) != 0)
	{
		printf("SIGTERM cannot be added to mask1\n");
	}
	if(sigaddset(&usr1_mask1, SIGQUIT) != 0)
	{
		printf("SIGQUIT cannot be added to mask1\n");
	}
	if(sigaddset(&usr1_mask1, SIGABRT) != 0)
	{
		printf("SIGABRT cannot be added to mask1\n");
	}
	if(sigaddset(&usr1_mask1, SIGINT) != 0)
	{
		printf("SIGINT cannot be added to mask1\n");
	}
	if(sigaddset(&usr1_mask1, SIGSEGV) != 0)
	{
		printf("SIGSEGV cannot be added to mask1\n");
	}
	if(sigaddset(&usr1_mask1, SIGTSTP) != 0)
	{
		printf("SIGTSTP cannot be added to mask1\n");
	}
	if(sigaddset(&usr1_mask1, SIGPIPE) != 0)
	{
		printf("SIGPIPE cannot be added to mask1\n");
	}
	
	//Global flag
	statusPendingToBeSent=FALSE;
	
	//Install signal handlers
	signal(SIGUSR1, handleSigusr1);
	signal(SIGUSR2, handleSigusr2);
	signal(SIGTERM, handleSigterm);
	signal(SIGTSTP, handleSigtstp);
	signal(SIGQUIT, handleSigquit);
	signal(SIGABRT, handleSigabrt);
	signal(SIGINT, handleSigint);
	signal(SIGSEGV, handleSigsegv);	
	signal(SIGPIPE, handleSigpipe);	
	//Connect ACS Monitor
	connectMonitor();
	connectBO();
	sendPIDToMonitor();
}
