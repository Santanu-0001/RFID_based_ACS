#include<sys/stat.h>
#include "acs_gen_2.h"

int exists(const char* filePath);
extern int s_create;
int iris;

_retflag main()
{
	int fd;	//File Descriptor for Serial Port Communication
	search_uid_t sh;
	_retflag isOK;
	char p_value[PARAM_SIZE];
	int tagListPresent = 0, bParamsPresent = 0, sParamsPresent = 0;
	struct sockaddr_in dest;
	reader_handle_t h_reader;
	iris=FALSE;
	while(TRUE)
	{
		if(exists("tag_id.txt"))
			tagListPresent=1;
		if(exists("system_param.txt"))
			sParamsPresent=1;
		if(exists("business_param.txt"))
			bParamsPresent=1;
		
		//If all of those exist, break free from checking files
		if(tagListPresent && sParamsPresent && bParamsPresent)
		{
			break;
		}
		sleep(2);
	}
	printf("\ninit started\n");
	//Initialize ACS
	acsInit();
	printf("****** acsInit complete *****\n");
	
	//Test Serial Line and Board Module
	while (ctrlHWSet(&fd)!=_SUCCESS);	
	printf("****** ctrlHW complete *****\n");
	
	//Connect with Mercury4Reader
	h_reader=connect_reader();
	rh1 = h_reader;
	printf("***** reader connected ******\n");

	if(!h_reader)
	{
		log_action(_SCREEN,"Error in opening reader, aborting...");
		Fail_Action();
	}	

	//Show OS Version
	log_action(_SCREEN,get_os_version(h_reader));
	memset(p_value,0,PARAM_SIZE);

	/////////////////////////////////////////////////
	//****  setting antenna power  ****
	if (SearchParamList("ANT_POWER",p_value)==_FAIL)
	{
		log_action(_SCREEN,"Failed retrieving ANTENNA POWER");
		Fail_Action();
	}
	if(set_tx_power(h_reader, atoi(p_value)) == 1.0)
	{
		log_action(_SCREEN,"Failed to set antenna power");
		Fail_Action();
	}
	close_reader(h_reader);
	h_reader=connect_reader();
	/////////////////////////////////////////////////

	//Go on searching for tags. If found run bRule
	while (1)
	{
		g_rdr_state = _NULL;
		// Start a reader search operation
		memset(p_value,0,PARAM_SIZE);
		if (SearchParamList("IRIS",p_value)==_SUCCESS)
		{
			if(strcmp("YES",p_value)==0)
			{
				iris=TRUE;
				if(_SUCCESS!=GetAck(fd))
				{
					printf("**** Road Open *****\n");
					sh = st_search(&h_reader,_YES);
				}
				else
				{
					printf("**** Road Blocked *****\n");
					sleep(1);
					//break;
				}
			}
			else
			{
				iris=FALSE;
				printf("**** IRIS Option disabled *****\n");
				sh = st_search(&h_reader,_YES);
			}
		}
		else
		{
				iris=FALSE;
				printf("**** Without IRIS *****\n");
				sh = st_search(&h_reader,_YES);
		}		
		if(sh >= 0)
		{
				
				if(_SUCCESS==GetMI(fd))
				{
					g_rdr_state = InMI;
					isOK=mRule(&h_reader,&sh,fd);
				}
				else
				{
					g_rdr_state = In_bRule;
					isOK=bRule(&h_reader,&sh,fd);
				}
			//}
		}
		bzero(p_value, PARAM_SIZE);
		if (SearchParamList("OTHER_IP",p_value)==_FAIL)
		{
			log_action(_SCREEN,"Failed retrieving M4_READER_IP");
			Fail_Action();
		}
		bzero(&dest, sizeof(dest));
		dest.sin_family = AF_INET;
		dest.sin_port = htons(5020);
		if(inet_aton(p_value, &dest.sin_addr.s_addr) != 0 )
		if (boConnected == 0)
		{
			if (s_create == 1)
			{
				connectBO();
				s_create = 0;
			}
			if(connect(boConnect, (struct sockaddr*)&dest, sizeof(dest)) != 0)
			{
				boConnected = 0;
			}
			else
				boConnected = 1;
		}
	}
	
	//Let's call it a day
	if (h_reader)
	{
		log_action(_SCREEN,"Closing reader...");
		close_reader(h_reader);
	}

	return 0;
}

int exists(const char* filePath)
{
	int exists = 0;
	int retval;
	struct stat buff;
	
	retval = stat(filePath, &buff);
	if(retval == 0)
	{
		exists = 1;
	}
	return exists;
}

