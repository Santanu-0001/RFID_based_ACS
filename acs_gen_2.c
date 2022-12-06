#include "acs_gen_2.h"

void printf_tagdb_record(tagdb_record_v3_t *tr, char *tag);

short cnt_valid_cards, cnt_invalid_cards;
int s_create;
extern int iris;

//Returns the host address to be connected to
char * getMachAddr()
{
	char *retval;
	retval = (char *) malloc(24);

	#ifdef FOR_READER
		retval= "127.0.0.1";	//RFU
	#else
		retval="192.168.1.81";	//RFU
	#endif
	printf("Connecting reader at ");
	log_action(_SCREEN,retval);
	return retval;
}
//Opens a reader and returns the handle
reader_handle_t connect_reader()
{
	return (open_reader(getMachAddr()));
}
//Initiates a search operation
search_uid_t st_search(reader_handle_t *h_reader, bool resetdb)
{
	char p_value[PARAM_SIZE];
	
	memset(p_value,0,PARAM_SIZE);
	if (SearchParamList("READ_TIME_OUT",p_value)==_FAIL)
	{
		log_action(_SCREEN,"Failed retrieving ACK");
		Fail_Action();
	}
	search_params_t s;
	s.protocol_list_length = 0;
	s.protocol_list[s.protocol_list_length++]=PROTOCOL_ID_GEN2;
	s.antenna_list_length=0; //RFU: replace with reading from system parameter
	s.timeout_ms = atoi(p_value)*100;//RFU: replace with reading from system parameter
	//Clear tagdb if requested
	if (resetdb)
		tagdb_reset_db(*h_reader);  // initialize TAG DATABASE in reader
	return search_start(*h_reader,&s); // get tag handle to get tag id from db
}
//Returns tags from the given reader handle
tagdb_record_v3_t getTags(reader_handle_t *h_reader)
{
	int i;
	tagdb_record_v3_t retval;
	tagdb_consume(*h_reader,&retval);
	return retval;
}


//The Business Rule to be implemented, enters when a search is active
_retflag bRule(reader_handle_t *h_reader, search_uid_t *sh, int fd)
{
_retflag retval=_SUCCESS;  //Return value
int status, found, len = 0;
long raw_tag_count, cnt_to_pop, tag_count;	
FILE *fp, *fs, *fm;
char tagid[TAG_ID_SIZE], buffer[1024], mst[29], slv[29], tag[TAG_ID_SIZE + 1], p_value[PARAM_SIZE];
tagdb_record_v3_t tr1;

printf("In bRule()..\n");	
//The loop where all of the validations has to occur for a given business rule
while (1)
{
	status = operation_status(*h_reader,*sh);
	if (status == OPERATION_DONE || status == OPERATION_TIMED_OUT)
	{
		log_action(_SCREEN,"Operation Done/Timed Out");
		break;
	}

	if (boConnected == 1)
	{
		len = send(boConnect, "SEARCH", 6, 0);
		if (len < 0)
		{
			close(boConnect);
			boConnected = 0;
			s_create = 1;
			break;
		}
		memset(buffer, '\0', 1024);
		recv(boConnect, buffer, 1024, 0);
		fs = fopen("tag_slave.txt", "w");
		fp = fopen("tag.txt", "w");
		if (strcmp(buffer, "FAIL") !=0)
		{
			fprintf(fs, "%s", buffer);
   			fprintf(fp, "%s", buffer);
		}
		fclose(fp);
		fclose(fs);
		
		//Enter the branch only when a card has been read; then process it
		fp = fopen("tag_master.txt", "w");
		fclose(fp);
		while (tagdb_consume(*h_reader, &tr1) == ERROR_SUCCESS)
			{
                if (tr1.status == 0)
                        break;
                else
				{
                    printf_tagdb_record(&tr1, tag);
					fp = fopen("tag_master.txt", "a");
					fprintf(fp, "%s\n", tag);
					fclose(fp);
				}
    	    }
        fm = fopen("tag_master.txt", "r");
		fs = fopen("tag_slave.txt", "r");
        fp = fopen("tag.txt", "a");
        while(fscanf(fm, "%s", mst) != EOF)
        {
            found = 0;
            while(fscanf(fs, "%s", slv) != EOF)
            {
                    if(strcasecmp(mst, slv) == 0)
                    {
                        found = 1;
                        break;
                    }
            }
            if (found == 0)
                    fprintf(fp, "%s\n", mst);
        }
		fclose(fp);
		fclose(fs);
		fclose(fm);

		tag_count = 0;
		fp = fopen("tag.txt", "r");
		while(fscanf(fp, "%s", mst) != EOF)
		{
			tag_count++;
		}
		fclose(fp);
		if (tag_count > 0)
		{
			retval = validateTagsAndControlHW(h_reader, fd, sh, tag_count);
			return retval;
		}
	}
	else
	{
	fp = fopen("tag.txt", "w");
	fclose(fp);
	fp = fopen("tag_valid.txt", "w");
	fclose(fp);
	fp = fopen("tag_invalid.txt", "w");
	fclose(fp);
	raw_tag_count = tags_available(*h_reader,TA_TAGS_TO_CONSUME);
	if (SearchParamList("TAILGATE",p_value)==_FAIL)
	{
		log_action(_SCREEN,"Failed retrieving TAILGATE");
		Fail_Action();
	}
	if (strcmp("YES",p_value)==0)
	{
		if (SearchParamList("END_NO_CARDS",p_value)==_FAIL)
		{
			log_action(_SCREEN,"Failed retrieving END_NO_CARDS");
			Fail_Action();
		}
		if (raw_tag_count == atoi(p_value))
		{
			//Start a transaction
			retval = validateTagsAndControlHW(h_reader, fd, sh,
							  raw_tag_count);
			return retval;
		}
	}
	else
	if (raw_tag_count >= 1)
	{
		//Start a transaction
		retval = validateTagsAndControlHW(h_reader, fd, sh, raw_tag_count);
		
		return retval;
	}
	}
	//Break if MI On
	if(_SUCCESS==GetMI(fd))
	{
		if(detailed_read_stop(*h_reader,*sh)==ERR_NO)
		{
			log_action(_SCREEN,"Failed to stop Read");
			tagdb_reset_db(*h_reader);
			Fail_Action();
		}
		else
		{
			log_action(_SCREEN,"Stopped Read");
		}
		//Break: The main while(1) loop		
		break;
	}
}//End: The loop where all of the validations has to occur for a given business rule

return retval;
}


void printf_tagdb_record(tagdb_record_v3_t *tr, char *tag)
{
        char *temp = NULL;
        int i, j, diff, num;
        char tmp[5];
        FILE *fp;

        memset(tag, '\0', 29);
        temp = tag;
        num = tr->num_bits / 16;
        for (i = 0; i < num; i++)
        {
                if (tr->tag_id[i] != 0)
                {
                        memset(tmp, '\0', 5);
                        sprintf(tmp,"%X", tr->tag_id[i]);
                        if (strlen(tmp) < 4)
                        {
                                diff = 4 - strlen(tmp);
                                for (j = 0; j < diff; j++)
                                {
                                        sprintf(temp,"0");
                                        ++temp; 
                                }
                        }
                        sprintf(temp,"%X", tr->tag_id[i]);
                        temp += strlen(temp);
                }
                else
                {
                        sprintf(temp, "0000");
                        temp += strlen(temp);
                }
        }
}

//Returns max number of cards for a given gate
_retflag getMaxCards()
{
	char p_value[PARAM_SIZE];//Area to store value of a parameter
	memset(p_value,0,PARAM_SIZE);
	if ((SearchParamList("END_NO_CARDS",p_value)==_SUCCESS))
	{
		return atoi(p_value);
	}
	else
	{
		return _FAIL;
	}
}

//Returns min number of cards for a given gate 
_retflag getMinCards()
{
	char p_value[PARAM_SIZE];		//Area to store value of a parameter
	memset(p_value,0,PARAM_SIZE);
	if ((SearchParamList("ST_NO_CARDS",p_value)==_SUCCESS))
	{
		return atoi(p_value);
	}
	else
	{
		return _FAIL;
	}
}

// For Fence Gate: Check whether all read tag are from same antenna else return false
/*_retflag chk_antenna_id(TagArray *lstCards, int len)
{
	_retflag retval;
	int i;
	u_short ant_id = lstCards->antenna_id;
	
	for (i=0; i<len; i++)
	{
		if( ant_id != lstCards->antenna_id )
		{
			retval=_FAIL;
			return retval;
		}
		else
		{
			lstCards++;
		}
	}
	retval=_SUCCESS;
	return retval;
}

// Business rule applicable for manual intervention 
// Manual Intervention: Allocate memory for valid cards
_retflag create_list_cards(TagArray **lstCards, char* tagid,tagdb_record_v3_t tag_read)
{
	_retflag retval;
	TagArray *tmp_valid_cards;	//Ptr of list containing tag ids of valid cards and the traversal list
	TagArray *trvCards;
	trvCards = (TagArray *)*lstCards;
	tmp_valid_cards = (TagArray *) malloc(sizeof(TagArray));
	if (SearchTagList(tagid)==_SUCCESS)
		strcpy(tmp_valid_cards->auth,"A");
	else
		strcpy(tmp_valid_cards->auth,"U");
	
	memcpy(tmp_valid_cards->TagId,tagid,TAG_ID_SIZE-1);
	tmp_valid_cards->TagId[TAG_ID_SIZE-1]='\0';
	
	memcpy(&(tmp_valid_cards->txnTimeSec),&tag_read.tv_sec,sizeof(u_int));
	memcpy(&(tmp_valid_cards->txnTimeMicSec),&tag_read.tv_usec,sizeof(u_int));
			
	tmp_valid_cards->next = NULL;
	if(*lstCards==NULL)
	{
		*lstCards = tmp_valid_cards;
//		printf(" %u lstCards\n", *lstCards);		
	}	
	else
	{
//		printf(" %u trv\n", trvCards);//getc(stdin);
		while(trvCards->next!=NULL)
			trvCards = trvCards->next;
		trvCards->next = tmp_valid_cards;
	}

	retval=_SUCCESS;
	return retval;
}
// Manual Intervention: Check if same card is read more than once if so return false
_retflag chk_dup_cards(TagArray *lstCards, char * tagid, u_int tsec)
{
	_retflag retval;
	//TagList * curr;
	//curr = g_TagList_head;
	char buffer[PARAM_SIZE];
	bzero(buffer, PARAM_SIZE);
	u_int intval=0;
	while( lstCards != NULL )
	{
	   if(strcasecmp(lstCards->TagId, tagid)==0)
      {//printf("Id: %s Matched\n", readId);
           	if( SearchParamList( "MI_INTVAL", buffer ) != _SUCCESS )       //   fscanf( fp,"%s", buffer);
       			return ERR_NO;
		sscanf(buffer, "%u", &intval);
		if(tsec-lstCards->txnTimeSec<intval)	
		{
			retval=_YES;
			return retval;
		}          
	   }
	   else
	      lstCards = lstCards->next;
	}
	//printf("Id: %s Not Matched\n", readId);  
	retval=_NO;
	return retval;
}
// Manual Intervention: Free memory allocated for valid cards
_retflag delete_valid_cards_list(TagArray *lstCards)
{
	_retflag retval;
	TagArray * prev;//, * curr;
	//curr = g_TagList_head;
	while( lstCards != NULL )
	{
		prev = lstCards;
		lstCards = lstCards->next;
		free(prev);
	}
	
	retval=_SUCCESS;
	return retval;
}
//Saves list of authenticated cards
_retflag save_valid_cards(TagArray *lstCards, int len)
{
	int i;
	char txnstr[TXN_STR_SIZE];

	memset(txnstr, 0, TXN_STR_SIZE);
	log_action(_SCREEN,"Waiting to save in file...");
	
	//Iterate once for each card data. len denotes no. of valid cards
	for (i=0; i<len; i++)
	{	
	    	if(_SUCCESS != make_txn_str(lstCards,txnstr, LOG_VALID_ACS_TRANSACTION))
		        return _FAIL;
      	sendBytesThroughSocket(sConnect, (void*)txnstr, TXN_STR_SIZE-1);
		lstCards++;
	}
	log_action(_SCREEN, "Saved...");
	return _SUCCESS;
}

// Manual Intervention: Save txn for valid cards
_retflag save_valid_cards_list(TagArray *lstCards)
{
	int i;
	char txnstr[TXN_STR_SIZE];

	memset(txnstr, 0, TXN_STR_SIZE);
	while (lstCards!=NULL)
	{
		if(_SUCCESS != make_txn_str(lstCards,txnstr, LOG_VALID_ACS_TRANSACTION))
			return _FAIL;
		sendBytesThroughSocket(sConnect, (void*)txnstr, TXN_STR_SIZE-1);
		lstCards=lstCards->next;
	}
	log_action(_SCREEN, "Saved...");
	return _SUCCESS;
}
// Make the txn string to save
_retflag make_txn_str(TagArray *lstCards, char* _txnstr, unsigned char tagType)
{
	_retflag retval;
	time_t tmt;
	struct tm* tmm;
	char buffer[PARAM_SIZE];
	char* RFU = "XXXX";
	bzero(buffer, PARAM_SIZE);
	
	_txnstr[0] = tagType; //denotes valid/invalid transaction
	//btolend(lstCards->txnTimeSec);
	tmt = lstCards->txnTimeSec;
	tmm = localtime(&tmt);
	if( SearchParamList( "READER_ID", buffer ) != _SUCCESS )
       		return ERR_NO;
//	strcpy(_txnstr, buffer);
	strcat(_txnstr, buffer);
	strcat(_txnstr, lstCards->TagId);
	
	bzero(buffer, PARAM_SIZE);
	sprintf(buffer, "%02d%02d%d%02d%02d%02d", tmm->tm_mday,tmm->tm_mon + 1,tmm->tm_year + 1900,tmm->tm_hour,tmm->tm_min,tmm->tm_sec);		
	strcat(_txnstr, buffer);

	bzero(buffer, PARAM_SIZE);
	
	sprintf(buffer, "%06u", lstCards->txnTimeMicSec);
	strcat(_txnstr, buffer);
	
	bzero(buffer, PARAM_SIZE);
	if( SearchParamList( "TXN_TYPE", buffer ) != _SUCCESS )
       		return ERR_NO;
	strcat(_txnstr, buffer);
	
	strcat(_txnstr, lstCards->auth);
	
	strcat(_txnstr, RFU);
	
	retval=_SUCCESS;
	return retval;
}
// Convert time stamp from big to little endian
void btolend(u_int  stamp)
{
	typedef union
	{
		u_int ui;
		unsigned char uc[4];
	}x;
	u_int i, j, k, l, _k, _l, stat;
	x z;i=j=k=l=_k=_l=0;
	z.ui=stamp;j=stamp;
	//unsigned char *s, str[10];
	//s = &stamp;
	time_t t;t=stamp;
	//printf("%s", ctime(&t));
	//printf("stamp=%u\n",stamp);
	//printf("sizeof=%d\n",sizeof(u_int));
	//for(i=0;i<4;i++)
	//{
	  //printf("stat..%d..%x\n",i,z.uc[i]);
	//}
	t=stat;
	//printf("%s", ctime(&t));
}*/

//Checks and increments/resets the LED counter, if applicable
_retflag ctrlLEDCounter(int fd, int count)
{
	char p_value[PARAM_SIZE];
	int isok;
	memset(p_value,0,PARAM_SIZE);
	isok=SearchParamList("LED",p_value);
		//Check the function went OK
		if (_SUCCESS==isok)
		{
			//Check if LED is applicable
			if (0==strcmp("YES", p_value))
			{
				if (count==-1)
				{
					ResCounter(fd, '2');
					return ResCounter(fd, '4');
				}
				else
				{
					return IncCounter(fd,count);
				}
			}
			else
			{
				return _SUCCESS;
			}
		}
		else
		{
			return _FAIL;
		}
}

//Checks and open, close the Barrier if applicable
_retflag ctrlBarrier(int fd, int count)
{
	char p_value[PARAM_SIZE];
	int isok;
	memset(p_value,0,PARAM_SIZE);
	isok=SearchParamList("BARRIER",p_value);
		//Check the function went OK
		if (_SUCCESS==isok)
		{
			//Check if LED is applicable
			if (0==strcmp("YES", p_value))
			{
				if (count==-1)
					return CloseBarrier(fd);
				else
				{
					return OpenBarrier(fd);
				}
			}
			else
			{
				return _SUCCESS;
			}
		}
		else
		{
			return _FAIL;
		}
}
//Controller for setting hardware before the program enters a running state
_retflag ctrlHWSet(int *fd)
{
	int isOK;
	char* dtTime;
        
	//Open Port for RS232 communication
	if ((isOK = openPort(fd))!=_SUCCESS)
	{
		log_action(_SCREEN, "Error in opening Serial Port, aborting...");
		return _FAIL;
	}

	//Send Power Up sync commands to the Board Module
	if(PwrSync(*fd)!=_SUCCESS)
	{
		log_action(_SCREEN, "Error in Power Sync, aborting...");
		return _FAIL;
	}

	//GET BM TIME **********



	//Send Power Up sync commands to the Board Module
	if(ctrlLEDCounter(*fd,-1)!=_SUCCESS)
	{
		log_action(_SCREEN, "Error in Resetting counter, aborting...");
		return _FAIL;	
	}
        
        /***********Get date-time from Board Module***************************/
        dtTime = GetBMTime(*fd);
        if(dtTime == NULL)
        {
            log_action(_SCREEN, "Failed fetching BM time !");
            return _FAIL;
        }
        else
        {
            //print current date-time as sent by board module
            puts(dtTime);
        }        

	return _SUCCESS;
}

// Stop Search and Reset tag database in Reader
/*_retflag stop_search(reader_handle_t *h_reader, search_uid_t *sh)
{
	if(detailed_read_stop(*h_reader,*sh)==ERR_NO)
	{
		log_action(_SCREEN,"Failed to stop Read");
		tagdb_reset_db(*h_reader);
		Fail_Action();
	}
	else
	{
		log_action(_SCREEN,"Stopped Read");
	}
	tagdb_reset_db(*h_reader);
	
	return _SUCCESS;
}*/

// Action for failing fundamental operation
_retflag Fail_Action()
{
	exit(1);
	return _SUCCESS;
}

//Feedback/timeout for First barrier (tailgating only)
int FeedFirstRecd(int fd)
{
	//In case Tailgating is applicable
	char p_value[PARAM_SIZE];
	time_t base_first_bar_time=0;	//Base time for first barrier timeout (tailgating only)
	memset(p_value,0,PARAM_SIZE);
	if(SearchParamList("TAILTIMEOUT",p_value)==_FAIL)
	{
		log_action(_SCREEN,"Error in retrieving TailTimeOut");
		Fail_Action();
	}
	else
	{
		base_first_bar_time=time(NULL);
		base_first_bar_time+=atoi(p_value);
		//Loop to wait for timeout of first barrier/feedback
		while(1)
		{
			if(time(NULL) >= base_first_bar_time)
			{
				log_action(_SCREEN, "First Barrier timed out");
				return _TIMEOUT;
			}
			if(_SUCCESS==GetFirstBar(fd))
			{
				log_action(_SCREEN, "First Barrier Feedback recd.");
				return _SUCCESS;
			}
			log_action(_SCREEN, "Waiting for First Barrier Feedback");
		}
	}
}

//Feedback/timeout barrier (Last in case of tailgating)
int FeedRecd(int fd)
{
	char p_value[PARAM_SIZE];
	time_t base_first_bar_time=0;	//Base time for barrier timeout
	memset(p_value,0,PARAM_SIZE);
	if(SearchParamList("BARTIMEOUT",p_value)==_FAIL)
	{
		log_action(_SCREEN,"Error in retrieving TailGate");
		Fail_Action();
	}
	else
	{
		puts("inside FEEDRECD");
		base_first_bar_time=time(NULL);
		base_first_bar_time+=atoi(p_value);
		//Loop to ignore first feedback from Boom  (ask for 0x0E)  //sourav
		while(1)
		{
			if(time(NULL) >= base_first_bar_time)
			{
				log_action(_SCREEN, "Ignore: Barrier timed out");
				return _TIMEOUT;
			}
			if(_SUCCESS==GetSwitch(fd, 0x0e))
			{
				log_action(_SCREEN, "Ignore: Barrier Feedback recd. Barrier already closed");
				break;
			}
			else
			{
				//break;
			}
			log_action(_SCREEN, "Ignore: Waiting for Barrier Feedback");
		}
		while(1)
		{
			if(time(NULL) >= base_first_bar_time)
			{
				log_action(_SCREEN, "Ignore: Barrier timed out");
				return _TIMEOUT;
			}
			if(_SUCCESS==GetSwitch(fd, 0x0f))
			{
				log_action(_SCREEN, "Ignore: Barrier Feedback recd. Barrier already closed");
				break;
			}
			else
			{
				//break;
			}
			log_action(_SCREEN, "Ignore: Waiting for Barrier Feedback");
		}
		//Loop to wait for timeout of barrier/feedback (ask for 0x0E)
		while(1)
		{
			if(time(NULL) >= base_first_bar_time)
			{
				log_action(_SCREEN, "Barrier timed out");
				return _TIMEOUT;
			}
			if(_SUCCESS==CloseBarrier(fd))
			{
				log_action(_SCREEN, "Barrier Feedback recd.");
				return _SUCCESS;
			}
			log_action(_SCREEN, "Waiting for Barrier Feedback. IN SECOND WHILE LOOP");
		}
		return _SUCCESS;
	}		
}

void initTransaction(int* cnt_max_cards, int* cnt_min_cards, short* option)
{
	char p_value[PARAM_SIZE];
	
	//Find out the max number of cards permitted in the gate and allocate memory for it
	if ((*cnt_max_cards=getMaxCards())==_FAIL)
	{
		log_action(_SCREEN,"Error in getting max number of cards");
		//Fail_Action();
	}
	
	//Find out the min number of cards permitted at the gate
	if ((*cnt_min_cards=getMinCards())==_FAIL)
	{
		log_action(_SCREEN,"Error in getting min. number of cards");
		//Fail_Action();
	}

	//Find out at which gate the Reader is installed
        *option = 0;

	//Find out the option applicable -  ACK
	memset(p_value,0,PARAM_SIZE);
	if (SearchParamList("ACK",p_value)==_FAIL)
	{
		log_action(_SCREEN,"Failed retrieving ACK");
		Fail_Action();
	}
	if (strcmp("YES",p_value)==0)
	{
		*option = ACK;
	}

	//Find out the option applicable -  TAIL
	memset(p_value,0,PARAM_SIZE);
	if (SearchParamList("TAILGATE",p_value)==_FAIL)
	{
		log_action(_SCREEN,"Failed retrieving TAILGATE");
		Fail_Action();
	}
	if (strcmp("YES",p_value)==0)
	{
            if(*option != ACK)
                *option = TAIL;
            else
            {
                log_action(_SCREEN,"ACK and TAIL both cannot be YES in business_params.txt !");
                Fail_Action();    
            }
	}
		
	//Find out the option applicable -  PED
        memset(p_value,0,PARAM_SIZE);
        if (SearchParamList("PEDESTRIAN",p_value)==_FAIL)
        {
            log_action(_SCREEN,"Failed retrieving PEDESTRIAN");
            Fail_Action();
        }
        if (strcmp("YES",p_value)==0)
        {
            if(*option != ACK || *option != TAIL)
                *option = PED;
            else
            {
                log_action(_SCREEN,"If PEDESTRIAN is YES, ACK or TAIL cannot be YES in business_params.txt !");
                Fail_Action();            
            }
	}
        
	if(*option == 0)
	{
            log_action(_SCREEN,"PEDESTRIAN, ACK or TAIL, either of those should be YES in business_params.txt !");
            Fail_Action();            
	}
}

void incrementLedCount(int fd)
{
	//Check whether LED is applicable and increase the counter
	if (_SUCCESS!=ctrlLEDCounter(fd,1))
	{
		log_action(_SCREEN, "Failed in incrementing LED Counter");
		Fail_Action();
	}
}
/*
void storeTagInfo(char *infoStr, char *tagid, short *cardCount)
{
	//char *timestamp;
	//timestamp = getCurrentTimestamp();
	
	//Store tagid
	memcpy(infoStr+((*cardCount) * TXN_STR_SIZE), tagid, TAG_ID_SIZE);
	
	//Store current date-time
	memcpy(infoStr+((*cardCount) * TXN_STR_SIZE)+TAG_ID_SIZE, getCurrentTimestamp(), TIMESTAMP_SIZE);
	
//	printf("IFSTR: %s\n",infoStr);
	//Increment the count of cards
	(*cardCount)++;

	#ifdef __DEBUG
	printf("bRule:Count of valid/suspicious cards %d\n",*cardCount);
	#endif
}
*/

_retflag validateTagsAndControlHW(reader_handle_t *h_reader, int fd, search_uid_t *sh, long raw_tag_count)
{
	int 	isTailOk = _FAIL;	//Tail gating time out
	int 	cnt_to_pop, i;		//Counter used when popping records from tag db
	tagdb_record_v3_t tag_read;	//Stores tag record after popping it
	char 	tagid[TAG_ID_SIZE + 1];	//A place to store tag id
	char 	p_value[PARAM_SIZE];	//Area to store value of a parameter
	_retflag retval=_SUCCESS;	//Return value
	_retflag transaction=_FAIL;   	//Return value after barrier feedback, marks endpoint of a transaction
	int 	cnt_max_cards=0;	//Count of max cards allowed per transaction
	int 	cnt_min_cards=0;	//Count of min cards allowed per transaction
      short 	option;                 //Whether TAIL/ACK/PED
	bool 	TRIGGER_HW=_NO;		//Whether h/w peripherals can be triggered
	FILE	*fp, *fm, *fv, *fiv;

	//Get max and min number of cards allowed at the gate and allocate
	//space for holding informations for max cards and get the option 
	//whether ACK, TAILGATE, or PED is applicable
	cnt_valid_cards=0;
	cnt_invalid_cards=0;
	initTransaction(&cnt_max_cards, &cnt_min_cards, &option);
	
	//Block taglist updation
	if(pthread_sigmask(SIG_BLOCK, &usr1_mask1, &usr1_mask2) != 0)
	{
		printf("signal blocking failed\n");
		exit(0);
	}

	if (boConnected == 1)
		fp = fopen("tag.txt", "r");
	//Pop each record from tagdb and validate
	for (cnt_to_pop=0; cnt_to_pop < raw_tag_count; cnt_to_pop++)
	{
		if(boConnected == 0)
		{
			tag_read=getTags(h_reader);
			memset(tagid,'\0',TAG_ID_SIZE + 1);
			tostr(tag_read, tagid);
			fm = fopen("tag.txt", "a");
			fprintf(fm, "%s\n", tagid);
			fclose(fm);
		}
		else
		{
			fscanf(fp, "%s", tagid);
		}
		//Process further, only if the tag is found valid
		if (SearchTagList(tagid)==_SUCCESS)
		{
			fv = fopen("tag_valid.txt", "a");
		       	fprintf(fv, "%s\n", tagid);
			fclose(fv);
			cnt_valid_cards++;
			if(opMode == ACS_MASTER || opMode == ACS_SLAVE)
			{
				//No functioning of hardware/BM if the operational
				//mode is Attendance Monitoring(AMS)
				TRIGGER_HW = _YES;
				if(option != PED)
				{
				     incrementLedCount(fd);
				}
			}
		}//single tag validation ends
		else
		{
			fiv = fopen("tag_invalid.txt", "a");
		       	fprintf(fiv, "%s\n", tagid);
			fclose(fiv);
			cnt_invalid_cards++;
		}
	}//validate ALL tags
	if (boConnected == 1)
		fclose(fp);
	//If one or more tags found valid , TRIGGER_HW will be _YES.
	if(TRIGGER_HW == _YES)
	{
		//Barrier will be controlled in different ways
		//for different type of Gates.
		switch(option)
		{
		case ACK:
			printf("\n*********Car Gate Mode***********\n");
			break;
		case TAIL:
			printf("\n*********Cycle/Scooter Gate Mode***********\n");
			break;
		case PED:
			printf("\n*********Pedestrian Gate Mode***********\n");
			break;
		}
		switch(option)
		{
			case ACK:
			/*
			 * It's applicable for CAR GATE - vrboom.
			 * Wait for ACK switch to be pressed. If not timed
			 * out,open the barrier and wait for barrier feedback.
			 */
				retval = waitForAckSwitch(h_reader,fd,option);
 	      		if(retval == _SUCCESS)
 	      		{
					ctrlBarrier(fd,1);//Open the barrier
					g_rdr_state = BARRIER_WAIT;
					log_action(_SCREEN, "Waiting for barrier to close");
					//ctrlBarrier(fd,-1);//close trigger
					transaction = FeedRecd(fd);//Wait for feedback
 	      		}
 	      		else
 	      		if(retval == _FAIL)
 	      		{
 	      			//ACK switch has timed out - break free 
 	      			//from processing, reset all and return
 	      			tagdb_reset_db(*h_reader);
 	      			ctrlLEDCounter(fd,-1);
 	      			return retval;
 	      		}
 	      		g_rdr_state = _NULL;
				break;
			
			case TAIL:
			/* 
     	                 *  It's applicable for SCOOTER/CYCLE GATE - TAILGAITING.
     	            	 *  Open the barrier, wait for First barrier feedback.
     	            	 *  If successfull, wait for Second barrier feedback.
     	            	 */
				ctrlBarrier(fd,1);//Open the barrier
				g_rdr_state = BARRIER_WAIT;
				log_action(_SCREEN, "Waiting for first barrier to close");
				if((isTailOk=FeedFirstRecd(fd))==_SUCCESS)//waiting for First barrier feedback
				{
					ctrlLEDCounter(fd,-1);//new change for light stack
					transaction=FeedRecd(fd);//waiting for Second barrier feedback
				}
				g_rdr_state = _NULL;
				break;
	            	
			case PED:
			/*
			 *  It is applicable for pedestrian's gate - VRIS.
			 *  Open the barrier and wait for barrier feedback.
			 */
				ctrlBarrier(fd,1);//Open the barrier
				g_rdr_state = BARRIER_WAIT;
				transaction = FeedRecdForPedestrian(fd);//Wait for feedback
				g_rdr_state = _NULL;
				break;
			
			default:
				//Execution control will never come to this case !
				Fail_Action();
				break;
		}//switch case ends
            
     	      //Check if transaction has completed successfully,
     	      //save transaction details in txn.txt.
		if(transaction == _SUCCESS)
		{
			printf("Valid transactions: \n");	   
			logvalidTransaction(cnt_valid_cards, LOG_VALID_ACS_TRANSACTION);
			printf("\n");
		}
		else
		{
			log_action(_SCREEN,"Transaction timed out\n");
		}

		if (iris)
		{
			if(OpenRoad(fd)==_SUCCESS)
			{
				printf("****** Road Released ******\n");
			}
		}
/*		if(OpenRoad(fd)==_SUCCESS)
		{
			printf("****** Road Released ******\n");
		}
*/	}//end TRIGGER_HW == _YES
	
	//Log transactions if txn is other than valid in ACS mode
	if(opMode == AMS)
	{
		if(cnt_valid_cards>0)
		{
			printf("Valid transactions: \n");	
			logvalidTransaction(cnt_valid_cards, LOG_VALID_AMS_TRANSACTION);
			printf("\n");			
		}
		if(cnt_invalid_cards>0)
		{
			printf("Suspicious transactions: \n");
			loginvalidTransaction(cnt_invalid_cards, LOG_INVALID_AMS_TRANSACTION);
			printf("\n");			
		}
	}
	else//ACS_MASTER, ACS_SLAVE
	{
		//Log suspicious tag information
		if(cnt_invalid_cards>0)
		{
			printf("Suspicious transactions: ");
			loginvalidTransaction(cnt_invalid_cards, LOG_INVALID_ACS_TRANSACTION);
			printf("\n");		
		}
	}


	/**********Now reset all******************
	Stop reading, reset tagdb, reset LedCount,
	free allocated memory for storing card info.
	******************************************/

	//Stop reading
	if(detailed_read_stop(*h_reader,*sh)==ERR_NO)
	{
		log_action(_SCREEN,"Failed to stop Read");
		tagdb_reset_db(*h_reader);
		Fail_Action();
	}				
      	tagdb_reset_db(*h_reader);
	if(TRIGGER_HW==_YES )
	{
        	ctrlLEDCounter(fd,-1);
	}
	
	//Unblock taglist updation
	if(pthread_sigmask(SIG_SETMASK, &usr1_mask2, NULL) != 0)
	{
		printf("signal unblocking failed\n");
		exit(0);
	}
	return _SUCCESS;
}

void read_tag(reader_handle_t *h_reader, int fd, short option)
{
	tagdb_record_v3_t 	tr1;
	search_uid_t		u1;
	search_params_t		s1;
	char 			tag[TAG_ID_SIZE + 1], mst[29], slv[29], buffer[1024];
	FILE 			*fp, *fs, *fv, *fiv, *fm;
	int 			status, found, tag_count, i;

	if (boConnected == 1)
	{
		send(boConnect, "SEARCH", 6, 0);
		memset(buffer, '\0', 1024);
		recv(boConnect, buffer, 1024, 0);
		fs = fopen("tag_slave.txt", "w");
		if (strcmp(buffer, "FAIL") !=0)
		{
			fprintf(fs, "%s", buffer);
		}
		fclose(fs);

		fs = fopen("tag_slave.txt", "r");
	        fm = fopen("tag_master.txt", "w");
	        while(fscanf(fs, "%s", slv) != EOF)
	        {
	                found = 0;
			fp = fopen("tag.txt", "r");
	                while(fscanf(fp, "%s", mst) != EOF)
	                {
	                        if(strcasecmp(mst, slv) == 0)
	                        {
	                                found = 1;
	                                break;
 	                        }
	                }
			fclose(fp);
	                if (found == 0)
			{
	                        fprintf(fm, "%s\n", slv);
				fp = fopen("tag.txt", "a");
				fprintf(fp, "%s\n", slv);
				fclose(fp);
			}
	        }
		fclose(fm);
        	fclose(fs);

		tag_count = 0;
		fp = fopen("tag_master.txt", "r");
		while(fscanf(fp, "%s", mst) != EOF)
		{
			tag_count++;
		}
		fclose(fp);
		if (tag_count > 0)
		printf("TAG COUNT %d\n", tag_count);
		fp = fopen("tag_master.txt", "r");
		for(i=0; i<tag_count; i++)
		{
				fscanf(fp, "%s", tag);
				if (SearchTagList(tag)==_SUCCESS)
				{
					if(option != PED)
					{
						incrementLedCount(fd);
					}
					fv = fopen("tag_valid.txt", "a");
			       		fprintf(fv, "%s\n", tag);
					fclose(fv);
					cnt_valid_cards++;
					printf("COUNT %d\n", cnt_valid_cards);
				}
				else
				{
					fiv = fopen("tag_invalid.txt", "a");
			        	fprintf(fiv, "%s\n", tag);
					fclose(fiv);
					cnt_invalid_cards++;
				}
		}
		fclose(fp);
	}

	if (tagdb_reset_db(*h_reader) < 0)
		reader_perror("tagdb_reset-db()");

	/* Do search operation */
	/* Search using EPC0 and GEN2 tag protocols */
	s1.protocol_list_length = 0;
	s1.protocol_list[s1.protocol_list_length++] = PROTOCOL_ID_GEN2;
	/* Search on all available antennas */
	s1.antenna_list_length = 0;
	/* Search times out after 1000 ms */
	s1.timeout_ms = 100;

	u1 = search_start(*h_reader, &s1);


	while (tagdb_consume(*h_reader, &tr1) == ERROR_SUCCESS)
	{
		if (tr1.status == 0)
			break;
		else
		{
			printf_tagdb_record(&tr1, tag);
	                found = 0;
			fp = fopen("tag.txt", "r");
	                while(fscanf(fp, "%s", mst) != EOF)
	                {
	                        if(strcasecmp(tag, mst) == 0)
	                        {
	                                found = 1;
	                                break;
 	                        }
	                }
			fclose(fp);
	                if (found == 0)
			{
			        fp = fopen("tag.txt", "a");
			        fprintf(fp, "%s\n", tag);
				fclose(fp);
				printf("Tag ID %s\n", tag);
				if (SearchTagList(tag)==_SUCCESS)
				{
					if(option != PED)
					{
						incrementLedCount(fd);
					}
					fp = fopen("tag_valid.txt", "a");
			       		fprintf(fp, "%s\n", tag);
					fclose(fp);
					cnt_valid_cards++;
				}
				else
				{
					fp = fopen("tag_invalid.txt", "a");
			        	fprintf(fp, "%s\n", tag);
					fclose(fp);
					cnt_invalid_cards++;
				}
			}
		}
	}
}

_retflag waitForAckSwitch(reader_handle_t *h_reader, int fd, short option)
{
    //This function will be called only in case of Car gate, where the security
    //personnel will have to press the ACK Switch within ACK_TIMEOUT.
    printf("waitForAckSwitch\n");
    char p_value[PARAM_SIZE];
    time_t base_ack_time=0; //Base time for ACK timeout
    bool AckTimeout = _NO;
            
    //Get the base time for ACK_TIMEOUT and add current time
    if (SearchParamList("ACK_TIMEOUT",p_value)==_FAIL)
    {
        log_action(_SCREEN,"Failed retrieving ACK_TIMEOUT");
        Fail_Action();
    }
    base_ack_time=time(NULL);
    base_ack_time+=atoi(p_value);
    g_rdr_state = ACK_WAIT;

    //Wait till switch is pressed or timed out
    while(1)
    {
        if(GetAck(fd) == _SUCCESS)
        {
            break;
        }
	read_tag(h_reader, fd, option);
	if (time(NULL) >= base_ack_time)
        {
            AckTimeout=_YES;
	    printf("ACK Timeout\n");
            break;							
        }
    }

    //If ACK switch has timed out return failure message
    if(_YES==AckTimeout)
    {
        return _FAIL;
    }
    else
    {
        return _SUCCESS;    
    }
}

int FeedRecdForPedestrian(int fd)
{
	char p_value[PARAM_SIZE];		// temp buffer
	time_t base_first_bar_time=0;	// Base time for barrier timeout
	memset(p_value,0,PARAM_SIZE);	// init buffer
	printf("\n********** Feed for Pedestrian entered *************\n\n");
	if(SearchParamList("BARTIMEOUT",p_value)==_FAIL)// get Barrier timeout
	{
		log_action(_SCREEN,"Error in retrieving TailGate");
		Fail_Action();
	}
	else
	{
		base_first_bar_time=time(NULL);				// get current time
		base_first_bar_time+=atoi(p_value);			// calculate end time
		//Loop to wait for timeout of barrier/feedback
		while(1)
		{
			if(time(NULL) >= base_first_bar_time)	// if current time exceeded
			{	// end time then trigger timeout
				log_action(_SCREEN, "Barrier timed out");
				return _TIMEOUT;		// return timeout
			}
			//if(_SUCCESS==GetSwitch(fd, 0x0f))	// if close barrier succeeded
			if(_SUCCESS==GetBarStatus(fd,BARRIER_OFF))
			{
			//	log_action(_SCREEN, "Barrier Feedback recd.");
				break;				// return succcess
			}
		}
		while(1)
		{
			if(time(NULL) >= base_first_bar_time)	// if current time exceeded
			{	// end time then trigge:r timeout
				log_action(_SCREEN, "Barrier timed out");
				return _TIMEOUT;		// return timeout
			}
			if(_SUCCESS==GetSwitch(fd, 0x0f))	// if close barrier succeeded
			{
				log_action(_SCREEN, "Barrier Feedback recd.");
				return _SUCCESS;		// return succcess
			}
		}
		return _SUCCESS;
	}
}

_retflag logvalidTransaction(short cardCount, logTarget_t tagType)
{
	FILE *fp;
	char tag[TAG_ID_SIZE +1], *strLog, type[2];
	int i;

	printf("Card Count %d\n", cardCount);
	strLog = (char*)malloc(TXN_STR_SIZE+3);	
	fp = fopen("tag_valid.txt", "r");
	//Pick up card-info one by one and send to Monitor
	for(i=0; i<cardCount; i++)
	{
		fscanf(fp, "%s", tag);
		bzero(strLog, TXN_STR_SIZE+3);
		*(strLog+0) = tagType;
		sendBytesThroughSocket(type, 1);
		memcpy(strLog+1, readerId, 2);
		memcpy(strLog + 3, tag, 28);
		memcpy(strLog+3+TAG_ID_SIZE, getCurrentTimestamp(), TIMESTAMP_SIZE);
		printf("STRLOG %s\n",strLog);
		if(sendBytesThroughSocket(strLog, TXN_STR_SIZE+3) == 0)
			printf("Not-saved... ");
		else
			printf("Saved... ");
		usleep(10*1000);
		
	}
	fclose(fp);
	free(strLog);
	printf("\n");
	return _SUCCESS;
}

_retflag loginvalidTransaction(short cardCount, logTarget_t tagType)
{
	FILE *fp;
	char tag[TAG_ID_SIZE +1], *strLog;
	int i;

	printf("Card Count %d\n", cardCount);
	strLog = (char*)malloc(TXN_STR_SIZE+3);	
	fp = fopen("tag_invalid.txt", "r");
	//Pick up card-info one by one and send to Monitor
	for(i=0; i<cardCount; i++)
	{
		fscanf(fp, "%s", tag);
		bzero(strLog, TXN_STR_SIZE+3);
		*(strLog+0) = tagType;
		memcpy(strLog+1, readerId, 2);
		memcpy(strLog + 3, tag, 28);
		memcpy(strLog+3+TAG_ID_SIZE, getCurrentTimestamp(), TIMESTAMP_SIZE);
		printf("STRLOG %s\n",strLog);
		if(sendBytesThroughSocket(strLog, TXN_STR_SIZE+3) == 0)
			printf("Not-saved... ");
		else
			printf("Saved... ");
		usleep(10*1000);
		
	}
	fclose(fp);
	free(strLog);
	printf("\n");
	return _SUCCESS;
}
/*_retflag logTransactionDetails(char* txnInfo, short cardCount, logTarget_t tagType)
{
	char* strLog;
	int i;

	//Return if there are no card info stored
	printf("in logTransactionDetails...  ");
	if(cardCount==0) 
	{ 
		printf("No cards to be saved\n");
		return _SUCCESS;
	}
	
	//Allocate buffer. Size of the buffer will be:
	//1+ 2 + TXN_STR_SIZE
	//tagType + readerId + TXN_STR
	strLog = (char*)malloc(TXN_STR_SIZE+3);	
	
	//Pick up card-info one by one and send to Monitor
	for(i=0; i<cardCount; i++)
	{
		bzero(strLog, TXN_STR_SIZE+3);
		*(strLog+0) = tagType;
		memcpy(strLog+1, readerId, 2);
		memcpy(strLog+3, txnInfo+(TXN_STR_SIZE*i),TXN_STR_SIZE);
		printf("STRLOG %s\n",strLog);
		if(sendBytesThroughSocket(strLog, TXN_STR_SIZE+3) == 0)
		//if(sendBytesThroughSocket(strLog, 12) == 0)
			printf("Not-saved... ");
		else
			printf("Saved... ");
		usleep(10*1000);
		
	}
	free(strLog);
	printf("\n");
	return _SUCCESS;
}*/

_retflag mRule(reader_handle_t *h_reader, search_uid_t *sh, int fd)
{
	int status;                	//Search operation status
	long raw_tag_count;		//Value returned when tag db is queried for number of cards read	
	int cnt_to_pop, i;		//Counter used when popping records from tag db
	tagdb_record_v3_t tag_read;	//Stores tag record after popping it
	char tagid[TAG_ID_SIZE];	//A place to store tag id
	char p_value[PARAM_SIZE];	//Area to store value of a parameter
	_retflag retval=_SUCCESS;	//Return value
	int cnt_max_cards=0;		//Count of max cards allowed per transaction
	int cnt_min_cards=0;		//Count of min cards allowed per transaction
	short option;                 	//Whether TAIL/ACK/PED
	FILE *fp;

	cnt_valid_cards=0;
	cnt_invalid_cards=0;
	//The loop where all of the validations has to occur for a given business rule
	while (1)
	{
		status = operation_status(*h_reader,*sh);
		if (status == OPERATION_DONE || status == OPERATION_TIMED_OUT)
		{
			log_action(_SCREEN,"Operation Done/Timed Out");
			break;
		}
		fp = fopen("tag_valid.txt", "w");
		fclose(fp);
		fp = fopen("tag_invalid.txt", "w");
		fclose(fp);
		//Enter the branch only when a card has been read; then process it	
		raw_tag_count = tags_available(*h_reader,TA_TAGS_TO_CONSUME);	
	
		if (raw_tag_count >= 1)
		{
			initTransaction(&cnt_max_cards, &cnt_min_cards, &option);
	
			//Pop each record from tagdb and validate
			for (cnt_to_pop=0; cnt_to_pop < raw_tag_count; cnt_to_pop++)
			{
				tag_read=getTags(h_reader);
				memset(tagid,0,TAG_ID_SIZE);
				tostr(tag_read, tagid);

				//Process further, only if the tag is found valid
				if (SearchTagList(tagid)==_SUCCESS)
				{
					if(opMode == ACS_MASTER || opMode == ACS_SLAVE)
					{
						fp = fopen("tag_valid.txt", "a");
		       				fprintf(fp, "%s\n", tagid);
						fclose(fp);
						cnt_valid_cards++;
					}
				}//single tag validation ends
				else
				{
					//Details of a suspicious tag will be logged in a separate log file
					fp = fopen("tag_invalid.txt", "a");
		       			fprintf(fp, "%s\n", tagid);
					fclose(fp);
					cnt_invalid_cards++;
				}
			}//end: validate ALL tags

			//Log transactions
			if(opMode == ACS_MASTER || opMode == ACS_SLAVE)
			{
				if(cnt_valid_cards>0)
				{
					printf("Valid transactions: ");
					logvalidTransaction(cnt_valid_cards, LOG_VALID_ACS_TRANSACTION);
				}
				if(cnt_invalid_cards>0)
				{
					printf("Suspicious transactions: ");
					loginvalidTransaction(cnt_invalid_cards, LOG_INVALID_ACS_TRANSACTION);
				}
			}
			else
			if(opMode == AMS)
			{
				if(cnt_valid_cards>0)
				{
					printf("Valid transactions: ");
					logvalidTransaction(cnt_valid_cards, LOG_VALID_AMS_TRANSACTION);
				}
				if(cnt_invalid_cards>0)
				{
					printf("Suspicious transactions: ");
					loginvalidTransaction(cnt_invalid_cards, LOG_INVALID_AMS_TRANSACTION);
				}
			}
		}//end:if (raw_tag_count >= 1)					
            
		if(_FAIL==GetMIwithinMI(fd))
		{
			break;
		}
	}//End: while()
	return retval;
}
