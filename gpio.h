#ifndef _GPIO_H
#define _GPIO_H

#include "rfx.h" 
	
#define _ERR						-1
#define UPMENU				'f'
#ifdef FOR_READER
#define COM_PORT				"/dev/ttyS1"
#else
#define COM_PORT				"/dev/ttyS0"
#endif

#define ROAD_BLOCK			'1'
#define COUNTER_ON			'1'
#define COUNTER_OFF		'2'
#define BARRIER_ON			'3'
#define LIGHT_STACK			'4'
#define ACK_ON					0x02
#define BARRIER_OFF			0x01
#define MI_ON					0x04
#define TAIL_FIRST				0x08
#define PULSE_TRIGGER		1

int openPort(int *fd);	//Opens port
int getBaudRate(int fd);//Returns baud rate of the handle
int initPort(int fd);	//Initialize port settings
int IncCounter(int fd, int no);	//Increase the counter by no
int ResCounter(int fd, char counter);	//Resets the counter
int OpenBarrier(int fd); //Opens the barrier
int CloseBarrier(int fd); //Closes the barrier
int PwrSync(int fd);	//Sync after power up
char* GetBMTime(int fd);  //Get time from Board Module
int GetAck(int fd);	//Return status of ACK switch
int GetSwitch(int fd, char ch);
int GetMI(int fd);	//Return status of MI switch
int GetMIwithinMI(int fd);	//Return status of MI switch when MI has been pressed
int GetFirstBar(int fd);	//Return status of the first barrier (tailgating only)
int GetBarStatus(int fd,unsigned char ch);
int chooseFunction(int fd);
int BlockRoad(int fd);
int OpenRoad(int fd);
#endif // _GPIO_H
