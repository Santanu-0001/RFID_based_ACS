//Framework functions
#ifndef _RFX_H
#define _RFX_H
#include <stdio.h>
#include "m4api.h"
#include <sys/socket.h>
#include <resolv.h>
#include <sys/select.h>
#include <pthread.h>
#include <time.h>
#include <signal.h>
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */

#define		TRUE				1
#define 	FALSE				0
#define 	_YES				0x01
#define		_NO					0x00
#define 	_SUCCESS 			0x01
#define 	_FAIL				0x00
//#define 	DELAY				1
#define 	__DEBUG				1
#define 	MAXBUF          		1024
#define 	PARAM_SIZE			20
#define 	TAG_ID_SIZE			28
#define 	TIMESTAMP_SIZE 		14
#define 	TXN_STR_SIZE		(TAG_ID_SIZE + TIMESTAMP_SIZE)
#define 	ERR_NO     			-1
#define 	_TIMEOUT			0x02
#define 	_SCREEN 			0x01
#define 	_FILE				0x02

#define FOR_READER

typedef 	int			_retflag;
typedef 	int			bool;

extern int	boConnect;
extern int     boConnected;
extern reader_handle_t rh1;

typedef enum op_mode_t
{
	ACS_MASTER=1,
	ACS_SLAVE,
	AMS
}op_mode_t;

typedef enum rdr_stat
{
	_NULL,
	ACK_WAIT,
	BARRIER_WAIT,
	InMI,
	In_bRule,
	LOAD_TAG,
	LOAD_PAR,
	SET_DTTM
}rdr_stat;

typedef enum logTarget_t
{
	LOG_EVENT=1, 				//event.log
	LOG_VALID_ACS_TRANSACTION,	//valid_acs_txn.log in ACS mode
	LOG_INVALID_ACS_TRANSACTION,//invalid_acs_txn.log in ACS mode
	LOG_VALID_AMS_TRANSACTION,	//valid_ams_txn.log in AMS mode
	LOG_INVALID_AMS_TRANSACTION	//invalid_ams_txn.log in AMS mode
}logTarget_t;

//Prints a given string in the destination (_SCREEN,_FILE)
_retflag log_action(int dest, char * string);

//Returns ascii string of a given binary value
char * btoa(int len, char * str);

_retflag 	sendStatusToMonitor();
void 		connectMonitor();
void 		connectBO();
char*		getCurrentTimestamp();
void           sendPIDToMonitor();
#endif //_RFX_H
