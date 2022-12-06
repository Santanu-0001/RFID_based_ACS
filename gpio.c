#include "gpio.h"
extern int iris;
//Structural functions
//Opens port
int openPort(int *fd)
{
int isok;
*fd = open(COM_PORT,O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (*fd == _ERR)
	{
		perror("Error in opening port\n");
		return _ERR;
	}
	else
	{
		printf("Port opened successfully\n");
		fcntl(*fd,F_SETFL,0);
		//Set the port settings
		isok=initPort(*fd);
		return isok;
	}
}
//Initialize port settings
int initPort(int fd) 
{

	struct termios options;
	// Get the current options for the port...
	if (tcgetattr(fd, &options)!=0)
		return _ERR;
	// Set the baud rates to 19200...
//	cfsetispeed(&options, B9600);
//	cfsetospeed(&options, B9600);
        
	// Enable the receiver and set local mode...
	options.c_cflag = (B9600 | CLOCAL | CREAD | CS8);
//	options.c_cflag &= ~PARENB;
//	options.c_cflag &= ~CSTOPB;
//	options.c_cflag &= ~CSIZE;
	options.c_oflag=0;
	
	options.c_cflag |= CS8;
	//options.c_cflag &= ~CRTSCTS;
        options.c_iflag = IGNPAR;// | IXON | IXOFF | IXANY);

	//#ifndef FOR_READER
	options.c_lflag =0;//&= ~(ICANON | ECHO | ECHOE | ISIG);
	//#endif
	
	// Set the new options for the port...
	if (tcsetattr(fd, TCSANOW, &options)==0)
	{
		return _SUCCESS;	
	}
	else
	{
		return _ERR;
	}
}
//Returns baud rate of the given connection
int getBaudRate(int fd)
{
	struct termios ti;
	int inputSpeed=_ERR;
	speed_t baudRate;
	tcgetattr(fd,&ti);
	baudRate=cfgetispeed(&ti);
	switch (baudRate)
	{
		case B0:      inputSpeed = 0; break;
		case B50:     inputSpeed = 50; break;
		case B110:    inputSpeed = 110; break;
		case B134:    inputSpeed = 134; break;
		case B150:    inputSpeed = 150; break;
		case B200:    inputSpeed = 200; break;
		case B300:    inputSpeed = 300; break;
		case B600:    inputSpeed = 600; break;
		case B1200:   inputSpeed = 1200; break;
		case B1800:   inputSpeed = 1800; break;
		case B2400:   inputSpeed = 2400; break;
		case B4800:   inputSpeed = 4800; break;
		case B9600:   inputSpeed = 9600; break;
		case B19200:  inputSpeed = 19200; break;
		case B38400:  inputSpeed = 38400; break;
	}
	return (inputSpeed);
}
//End: Structural functions

//BM specific functions
//Syncs the BM after power up
int PwrSync(int fd)
{
	unsigned char strSend[16];// string to send bytes to serial comm. port
	unsigned char strRecv[16];// string to recv bytes from serial comm. port
	memset(strSend,0,16);// init send buffer
	memset(strRecv,0,16);// init receive buffer
	int i, br;	// counter, return value
	strSend[0]=0x16;// store value to send
	if(write(fd,strSend,1) < 0)// write to serial comm. port
	{	// if failed return fail.
		printf("PWR Sync: Error in writing\n");			
		return _FAIL;
	}	
	else// else print success msg
	   printf("PWR Sync: Written Successfully\n");
           
	fcntl(fd,F_SETFL,0);// iwait for serail comm. event	

	if ((br=read(fd,strRecv,16))==_ERR)
	{	
		printf("PWR Sync: Read: Err No. %d \n",br);
		return _FAIL;
	}
	else
	{
           //check received value
	   printf("PWR Sync: #Bytes read:%d byte:0x%02x\n", br, strRecv[0]);
           
	   if(strRecv[0]==0x15)// check received value
	   {
                printf("PWR Sync: Error\n");
                return _FAIL;
           }           
           
	   if (strRecv[0]==0x0F)
           {
                puts("Ack on pwr sync : 0x0F");
                if(chooseFunction(fd) == _FAIL)
                {
                    printf("Choose-function during pwr sync failed!");
                    return _FAIL;
                }
		return _SUCCESS;
	   }
           else
           if(strRecv[0]==0x06)
           {
                puts("Ack on pwr sync : 0x06");
                return _SUCCESS;
           }
        }
}	


//Get current date-time from Board Module
char* GetBMTime(int fd)
{
        char 		*buff;
	unsigned char 	strSend[25], strRecv[25];
        int 		i, br=0, recvCount=0;
        char *dtTime;

        buff = (char*)malloc(60 * sizeof(char));
        memset(buff, 0, 60);
        strcpy(buff, "Board Module: Current date-time: ");
        
	memset(strSend, 0, 25);
	memset(strRecv, 0, 25);
        dtTime = (char*)malloc(15 * sizeof(char));
        memset(dtTime, 0, 15);
        
	strSend[0] = 0x1B;
        strSend[1] = 'G';
        strSend[2] = 'T';

        printf("Sending 0x1B G T\n");
	if(write(fd, strSend, 3) < 0)
	{
		printf("Get time: Error in writing\n");			
		return;
	}
	usleep(100000);
        fcntl(fd, F_SETFL, 0);// wait for serail comm. event

		TCIOFLUSH;
		br = 0;
		memset(strRecv, 0, 25);
		buff=strRecv;
		br = read(fd, strRecv, 25);
			//do these after first read
			if(br == _ERR)
			{
				printf("Get time: Read: Err No. %d \n",br);
				return;
			}
			if(strRecv[0] == 0x15) 
			{
			printf("error1\n");
			//return NULL;
			}
			else if(strRecv[0] == 0x1d) 
			{
			
			buff++;buff++;buff++;buff++;
			memcpy(dtTime,buff+6,4);//yyyy
			memcpy(dtTime+4,buff+3,2);//mm
			memcpy(dtTime+6,buff,2);//dd
			memcpy(dtTime+8,buff+11,2);//hh
			memcpy(dtTime+10,buff+14,2);//mm
			memcpy(dtTime+12,buff+17,2);//ss
			}
			
        return dtTime;





/*
        char* buff;
	unsigned char strSend[25];// string to send bytes to serial comm. port
	unsigned char strRecv[25];// string to recv bytes from serial comm. port
        int i, br=0, recvCount=0;	// counter, return value
        
        buff = (char*)malloc(60 * sizeof(char));
        memset(buff,0,60);
        strcpy(buff, "Board Module: Current date-time: ");
        
        //store value to send
	memset(strSend,0,25);//init send buffer
	memset(strRecv,0,25);//init receive buffer      
        
	//Send the command to get date-time
	strSend[0]=0x1B;
        strSend[1]='G';
        strSend[2]='T';

        printf("Sending 0x1B G T\n");
	if(write(fd,strSend,3) < 0)// write to serial comm. port
	{	// if failed return fail.
		printf("Get time: Error in writing\n");			
		return NULL;
	}
        //printf("sent\n");
        fcntl(fd,F_SETFL,0);// wait for serail comm. event	
        

	//Now receive time from BM 
	while(1)
	{
		        TCIOFLUSH;
		br=0;
		if(recvCount > 20)
			break;

		memset(strRecv,0,25);
		br=read(fd,strRecv,25);
		if(recvCount==0)
		{
			//do these after first read
			if(br==_ERR)
			{	// if read failed, then print err.
				printf("Get time: Read: Err No. %d \n",br);
				return NULL;
			}
			if(strRecv[0]==0x15) return NULL;
			strncat(buff, strRecv+3, br);//5, 20
		}
		else
		{
        	strncat(buff, strRecv, br);
        }
		printf("br:%d %s\n", br, buff);
		recvCount += br;

	}//end while

	return buff;
*/
}

//Increases the counter by no
int IncCounter(int fd, int no)
{
	unsigned char strSend[16];
	unsigned char strRecv[16];
	memset(strSend,0,16);
	memset(strRecv,0,16);
	int i, br;

	for (i=0; i < no; i++)
	{
		//set the relay for increasing counter
		strSend[0]=0x1B;
		strSend[1]='S';
		strSend[2]='r';
		strSend[3]=COUNTER_ON;
		//Try sending until we get the expected return
		while(1)
		{
			if(write(fd,strSend,4) < 0)
			{
				printf("IncCounter: Error in writing\n");
				return _FAIL;
			}
			else
				printf("IncCounter: Written Successfully\n");
			//Set to blocking
			fcntl(fd,F_SETFL,0);
			
			if ((br=read(fd,strRecv,16))==_ERR)
			{
				printf("IncCounter: Read: Err No. %d \n",br);
				return _FAIL;
			}
			else
			{
				if (strRecv[0]==0x1D)
				{
					printf("IncCounter: Read: %x %x\n", strRecv[0], strRecv[1]);
					break;
				}
				else
				{
					printf("IncCounter->Counter misbehaving set: Read: %x %x\n", strRecv[0], strRecv[1]);
				}
			}
		}
 		//End: Try sending until we get the expected return
		strSend[0]=0x1B;
		strSend[1]='R';
		strSend[2]='r';
		strSend[3]=COUNTER_ON;
		#ifdef DELAY
		sleep(DELAY);
		#endif
		//Try sending until we get the expected return
		while(1)
		{
			if(write(fd,strSend,4) < 0)
				printf("IncCounter: Error in writing\n");
			else
				printf("IncCounter: Written Successfully\n");
			//Set to blocking		
			fcntl(fd,F_SETFL,0);
			
			if ((br=read(fd,strRecv,16))==_ERR)
			{
				printf("IncCounter: Read: Err No. %d \n",br);
			}
			else
			{
				if (strRecv[0]==0x1D)
				{
					printf("IncCounter: Read: %x %x\n", strRecv[0], strRecv[1]);
					break;
				}
				else
				{
					printf("IncCounter->Counter misbehaving reset: Read: %x %x\n", strRecv[0], strRecv[1]);
				}
			}
		}
	}
	usleep(1000*50);
	return _SUCCESS;
}
int ResCounter(int fd, char counter)
{
	unsigned char strSend[16];
	unsigned char strRecv[16];
	memset(strSend,0,16);
	memset(strRecv,0,16);
	int i, br;
	_retflag retval;
	//set the relay for increasing counter
	strSend[0]=0x1B;
	strSend[1]='S';
	strSend[2]='r';
	strSend[3]=counter;
	//Try sending until we get the expected return
	while(1)
	{

		if(write(fd,strSend,4) < 0)
		{
			printf("ResCounter: Error in writing\n");
			return _FAIL;
		}
		else
			printf("ResCounter: Written Successfully\n");
		//Set to blocking
		fcntl(fd,F_SETFL,0);
		
		if ((br=read(fd,strRecv,16))==_ERR)
		{
			printf("ResCounter: Read: Err No. %d \n",br);
			return _FAIL;
		}
		else
		{
			if (strRecv[0]==0x1D)
			{
				printf("ResCounter: Read: %x %x\n", strRecv[0], strRecv[1]);
				break;
			}
			else
			{
				printf("ResCounter->Counter misbehaving set: Read: %x %x\n", strRecv[0], strRecv[1]);
			}
		}
	}//End: Try sending until we get the expected return
	strSend[0]=0x1B;
	strSend[1]='R';
	strSend[2]='r';
	strSend[3]=counter;
	#ifdef DELAY
	sleep(DELAY);
	#endif
	//Try sending until we get the expected return
	while(1)
	{
		if(write(fd,strSend,4) < 0)
		{
			printf("ResCounter: Error in writing\n");
			return _FAIL;
		}
		else
			printf("ResCounter: Written Successfully\n");
		//Set to blocking
		fcntl(fd,F_SETFL,0);
			
		if ((br=read(fd,strRecv,16))==_ERR)
		{
			printf("ResCounter: Read: Err No. %d \n",br);
			printf("ResCounter: Read: %x %x\n", strRecv[0], strRecv[1]);
			return _FAIL;
		}
		else
		{
			if (strRecv[0]==0x1D)
			{
				printf("ResCounter: Read: %x %x\n", strRecv[0], strRecv[1]);
				break;
			}
			else
			{
				printf("ResCounter->Counter misbehaving reset: Read: %x %x\n", strRecv[0], strRecv[1]);
			}
		}
	}//End: Try sending until we get the expected return

	return _SUCCESS;
}

int OpenBarrier(int fd)
{
	unsigned char strSend[16];
	unsigned char strRecv[16];
	memset(strSend,0,16);
	memset(strRecv,0,16);
	int i, br;
	if (iris)
	{
		if(BlockRoad(fd)==_SUCCESS)
		{
			printf("****** Road Blocked ******\n");
		}
	}
/*	if(BlockRoad(fd)==_SUCCESS)
	{
		printf("****** Road Blocked ******\n");
		//usleep(1000);
	}
*/	//set the relay for increasing counter
	strSend[0]=0x1B;
	strSend[1]='S';
	strSend[2]='r';
	strSend[3]=BARRIER_ON;
	if(write(fd,strSend,4) < 0)
		printf("OpenBarrier: Error in writing\n");
	else
		printf("OpenBarrier: Written Successfully\n");
	//Set to blocking
	fcntl(fd,F_SETFL,0);
	
	if ((br=read(fd,strRecv,16))==_ERR)
	{
		printf("OpenBarrier: Read: Err No. %d \n",br);
	}
	else
	{
		printf("OpenBarrier: Bytes read: %d\n", br);
#ifdef PULSE_TRIGGER
		strSend[0]=0x1B;
		strSend[1]='R';
		strSend[2]='r';
		strSend[3]=BARRIER_ON;
		usleep(500000);
		//sleep(1);
		if(write(fd,strSend,4) < 0)
			printf("OpenBarrier Reset: Error in writing\n");
		else
		{
			printf("OpenBarrier Reset: Written Successfully\n");
		}
		//Set to blocking
		fcntl(fd,F_SETFL,0);
	
		if ((br=read(fd,strRecv,16))==_ERR)
		{
			printf("OpenBarrier Reset: Read: Err No. %d \n",br);
		}
		else
		{
			printf("OpenBarrier Reset: Bytes read: %d\n", br);

		}
		printf("OpenBarrier Reset: Read: %x %x\n", strRecv[0], strRecv[1]);
#endif
	}
	printf("OpenBarrier: Read: %x %x\n", strRecv[0], strRecv[1]);
}

int CloseBarrier(int fd)
{
	unsigned char strSend[16];
	unsigned char strRecv[16];
	memset(strSend,0,16);
	memset(strRecv,0,16);
	int i, br;

	strSend[0]=0x1B;
	strSend[1]='G';
	strSend[2]='I';
	strSend[3]=BARRIER_OFF;
	if(write(fd,strSend,3) < 0)
		printf("CloseBarrier: Error in writing\n");
	else
		printf("CloseBarrier: Written Successfully\n");
	
	//Set to blocking
	fcntl(fd,F_SETFL,0);
	
	if ((br=read(fd,strRecv,16))==_ERR)
	{
		printf("CloseBarrier: Read: Err No. %d \n",br);
	}
	else
	{
		printf("CloseBarrier: Bytes read: %d %x\n", br, strRecv[3]);
		if (br>=4)
		{
			if(((~strRecv[3]) & BARRIER_OFF)==BARRIER_OFF)
			{
				return _SUCCESS;
			}
			else
			{
				return _FAIL;
			}
		}
		else
		{
			return _FAIL;
		}
	return _SUCCESS;
	}
	i=0;
	printf("CloseBarrier: Read: \n");
	for(i=0;i<br;i++)
		printf(" %x ", strRecv[i]);
	
	printf("\n");
	return _FAIL;
}

//Return status of ACK switch
int GetAck(int fd)
{
	unsigned char strSend[16];
	unsigned char strRecv[16];
	memset(strSend,0,16);
	memset(strRecv,0,16);
	int i, br;

	strSend[0]=0x1B;
	strSend[1]='G';
	strSend[2]='I';
	strSend[3]=ACK_ON;
	if(write(fd,strSend,3) < 0)
		printf("ACK: Error in writing\n");
	else
		//printf("ACK: Written Successfully\n");
	
	//Set to blocking
	fcntl(fd,F_SETFL,0);
	
	if ((br=read(fd,strRecv,16))==_ERR)
	{
		printf("ACK: Read: Err No. %d \n",br);
	}
	else
	{
		if(br>=4)
		{
			if (((~strRecv[3]) & ACK_ON)==ACK_ON)
			{
				printf("ACK On: Bytes read: %d %x\n", br, strRecv[3]);
				return _SUCCESS;
			}
			else
			{
				printf("ACK Off: Bytes read: %d %x\n", br, strRecv[3]);
				return _FAIL;
			}
		}
		
	}
	return _FAIL;
}

int GetSwitch(int fd, char ch)
{
	unsigned char strSend[16];
	unsigned char strRecv[16];
	memset(strSend,0,16);
	memset(strRecv,0,16);
	int i, br;

	strSend[0]=0x1B;
	strSend[1]='G';
	strSend[2]='I';
	//strSend[3]=TAIL_FIRST;
	if(write(fd,strSend,3) < 0)
		printf("MI: Error in writing\n");
	else
		//Set to blocking
		fcntl(fd,F_SETFL,0);
	
	if ((br=read(fd,strRecv,16))==_ERR)
	{
		printf("MI: Error in receiving");
	}
	else
	{
		if (br>=4)
		{
			//printf("Get switch : %d %x %x %x\n",br,strRecv[3],ch,(~strRecv[3] & TAIL_FIRST));
			if (((strRecv[3]) & ch) == ch)
			{
				return _SUCCESS;
			}
			else
			{
				return _FAIL;
			}
		}
	}
	return _FAIL;
}

//Return status of MI switch
int GetMI(int fd)
{
	unsigned char strSend[16];
	unsigned char strRecv[16];
	memset(strSend,0,16);
	memset(strRecv,0,16);
	int i, br;

	strSend[0]=0x1B;
	strSend[1]='G';
	strSend[2]='I';
	if(write(fd,strSend,3) < 0)
		printf("MI: Error in writing\n");
	else
	
	//Set to blocking
	fcntl(fd,F_SETFL,0);
	
	if ((br=read(fd,strRecv,16))==_ERR)
	{
		printf("MI: Error in receiving");
	}
	else
	{
		if (br>=4)
		{
			if (((~strRecv[3]) & MI_ON) == MI_ON)
			{
				printf("MI On: Bytes read: %d %x\n", br, strRecv[3]);
				return _SUCCESS;
			}
			else
			{
				return _FAIL;
			}
		}
	}
	return _FAIL;
}

//Return status of MI switch when the MI switch has already been pressed once
int GetMIwithinMI(int fd)
{
	unsigned char strSend[16];
	unsigned char strRecv[16];
	memset(strSend,0,16);
	memset(strRecv,0,16);
	int i, br;

	strSend[0]=0x1B;
	strSend[1]='G';
	strSend[2]='I';
	if(write(fd,strSend,3) < 0)
		printf("MI: Error in writing\n");

	//Set to blocking
	fcntl(fd,F_SETFL,0);
	
	if ((br=read(fd,strRecv,16))==_ERR)
	{
		printf("MI: Read: Err No. %d \n",br);
	}
	else
	{
		if (br>=4) //Check whether received the string as expected
		{
			if (((~strRecv[3]) & MI_ON)==MI_ON)
			{
				printf("MI On: Bytes read: %d %x\n", br, strRecv[3]);
				return _SUCCESS;
			}
			else
			{
				printf("MI Off: Bytes read: %d %x\n", br, strRecv[3]);
				return _FAIL;
			}
		}
		else
		{
			return _SUCCESS;
		}
	}
	return _SUCCESS;
}
//Check initial status of the barrier
int GetBarStatus(int fd,unsigned char ch)
{
	unsigned char strSend[16];
	unsigned char strRecv[16];
	memset(strSend,0,16);
	memset(strRecv,0,16);
	int i, br;

	strSend[0]=0x1B;
	strSend[1]='G';
	strSend[2]='I';
	strSend[3]=TAIL_FIRST;
	if(write(fd,strSend,3) < 0)
		printf("Barrier Status Check: Error in writing\n");
	else
		printf("Barrier Status Check: Written Successfully\n");
	
	//Set to blocking
	fcntl(fd,F_SETFL,0);
	
	if ((br=read(fd,strRecv,16))==_ERR)
	{
		printf("Barrier Status Check: Read: Err No. %d \n",br);
	}
	else
	{
		if(br>=4)
		{
      			if (((~strRecv[3]) & ch)!=0x00)//==0x00)
			{
				printf("Barrier Ready: Bytes read: %d %x\n", br, strRecv[3]);
				return _SUCCESS;
			}
			else
			{
				printf("Barrier Not Ready: Bytes read: %d %x %x %x %x\n", br, strRecv[3],ch,~ch,((strRecv[3]) & ~ch));
				return _FAIL;
			}
		}
	}
	return _FAIL;

}

//Return status of the feedback from first barrier (tailgating only)
int GetFirstBar(int fd)
{
	unsigned char strSend[16];
	unsigned char strRecv[16];
	memset(strSend,0,16);
	memset(strRecv,0,16);
	int i, br;

	strSend[0]=0x1B;
	strSend[1]='G';
	strSend[2]='I';
	strSend[3]=TAIL_FIRST;
	if(write(fd,strSend,3) < 0)
		printf("Tail First Feedback: Error in writing\n");
	else
		printf("Tail First Feedback: Written Successfully\n");
	
	//Set to blocking
	fcntl(fd,F_SETFL,0);
	
	if ((br=read(fd,strRecv,16))==_ERR)
	{
		printf("Tail First Feedback: Read: Err No. %d \n",br);
	}
	else
	{
		if(br>=4)
		{
			if (((~strRecv[3]) & TAIL_FIRST)==TAIL_FIRST)
			{
				printf("Tail First Recd.: Bytes read: %d %x\n", br, strRecv[3]);
				return _SUCCESS;
			}
			else
			{
				printf("Tail First not Recd.: Bytes read: %d %x\n", br, strRecv[3]);
				return _FAIL;
			}
		}
	}
	return _FAIL;
}

int chooseFunction(int fd)
{
    int i, br=0;
    char strSend[3];
    char strRecv[3];
    
    memset(strSend,0,3);//init send buffer
    memset(strRecv,0,3);//init receive buffer
    
    strSend[0]=0x1B;
    strSend[1]='C';
    if(write(fd,strSend,2) < 0)
    {
            printf("pwr sync: Error in writing\n");
            return _FAIL;
    }
    if ((br=read(fd,strRecv,16))==_ERR)
    {
            printf("pwr sync: Read: Err No. %d \n",br);
            return _FAIL;
    }
    if (strRecv[0] != 0x1d)
    {
        printf("pwr sync recv error !");
        return _FAIL;   
    }
}

int OpenRoad(int fd)
{
	unsigned char strSend[16];
	unsigned char strRecv[16];
	memset(strSend,0,16);
	memset(strRecv,0,16);
	int i, br;

	//set the relay for increasing counter
	strSend[0]=0x1B;
	strSend[1]='R';
	strSend[2]='r';
	strSend[3]=ROAD_BLOCK;
	//Try sending until we get the expected return
	while(1)
	{
		if(write(fd,strSend,4) < 0)
		{
			printf("ROAD_BLOCK: Error in writing\n");
			return _FAIL;
		}
		else
			printf("ROAD_BLOCK: Written Successfully\n");
		//Set to blocking
		fcntl(fd,F_SETFL,0);
		
		if ((br=read(fd,strRecv,16))==_ERR)
		{
			printf("ROAD_BLOCK: Read: Err No. %d \n",br);
			return _FAIL;
		}
		else
		{
			if (strRecv[0]==0x1D)
			{
				printf("ROAD_BLOCK: Read: %x %x\n", strRecv[0], strRecv[1]);
				break;
			}
			else
			{
				printf("ROAD_BLOCK misbehaving set: Read: %x %x\n", strRecv[0], strRecv[1]);
			}
		}
	}
		//End: Try sending until we get the expected return
	usleep(1000*50);
	return _SUCCESS;
}

int BlockRoad(int fd)
{
	unsigned char strSend[16];
	unsigned char strRecv[16];
	memset(strSend,0,16);
	memset(strRecv,0,16);
	int i, br;

	//set the relay for increasing counter
	strSend[0]=0x1B;
	strSend[1]='S';
	strSend[2]='r';
	strSend[3]=ROAD_BLOCK;
	//Try sending until we get the expected return
	while(1)
	{
		if(write(fd,strSend,4) < 0)
		{
			printf("ROAD_BLOCK: Error in writing\n");
			return _FAIL;
		}
		else
			printf("ROAD_BLOCK: Written Successfully\n");
		//Set to blocking
		fcntl(fd,F_SETFL,0);
		
		if ((br=read(fd,strRecv,16))==_ERR)
		{
			printf("ROAD_BLOCK: Read: Err No. %d \n",br);
			return _FAIL;
		}
		else
		{
			if (strRecv[0]==0x1D)
			{
				printf("ROAD_BLOCK: Read: %x %x\n", strRecv[0], strRecv[1]);
				break;
			}
			else
			{
				printf("ROAD_BLOCK misbehaving set: Read: %x %x\n", strRecv[0], strRecv[1]);
			}
		}
	}
		//End: Try sending until we get the expected return
	usleep(1000*50);
	return _SUCCESS;
}

//End: BM specific functions
