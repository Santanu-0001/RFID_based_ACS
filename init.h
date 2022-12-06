#ifndef _INIT_H
#define _INIT_H
#include "rfx.h"
#include "gpio.h"

//Global variables
//extern reader_handle_t rh;

extern int  sConnect;
extern sigset_t usr1_mask1;
extern sigset_t usr1_mask2;
extern op_mode_t opMode;
extern rdr_stat g_rdr_state;
extern char readerId[3];
extern bool statusPendingToBeSent;


// The structure to store all tag id for runtime traversal
struct TagList{
	char TagId[TAG_ID_SIZE + 1];
	struct TagList * next;
};
typedef struct TagList TagList;
//The structure to store all valid tag id read at runtime

struct TagArray{
	char	TagId[TAG_ID_SIZE];
	u_int	txnTimeSec;
	u_int	txnTimeMicSec;
	u_short antenna_id;
	char	auth[2];
	struct TagArray * next;
};
typedef struct TagArray TagArray;

//Tag info 
typedef struct tagInfo_t
{
	char tagId[TAG_ID_SIZE];
	char timestamp[TIMESTAMP_SIZE];
}tagInfo_t;

// The structure to store all system and business param
struct ParamList{
	char pname[PARAM_SIZE];
	char pvalue[PARAM_SIZE];
	struct ParamList * next;
};
typedef struct ParamList ParamList;

// Read all tag id from tag_id.txt and create a linked list containing all those ids.
int CreateTagList();

// Free the memory allocated by the linked list containing tag ids.
int DeleteTagList();

/* Read all param name and value from system_param.txt and business_param.txt and create a linked list
	containing all those param and values.*/
int CreateParamList();

// Free the memory allocated by the linked list containing param and values.
int DeleteParamList();

// Search the tag id specified by input parameter "readId" in the linked list containing tag ids.
int SearchTagList(char * readId);

/* Search the param specified by input parameter "param" in the linked list containing param and values
	and if the param found store the value in output paramater "value".*/
int SearchParamList(char * param, char * value);

/* The function to initialize client socket and connect for date time request */
int initializesocket();

/* The function to set date time and exit from the system if not succeed */
int m4SetDateTime();

/***************************************/
//Initialize ACS
void acsInit();

//Handler for sigusr1
int 	handleSigusr1(int i);
int 	handleSigusr2(int i);
int 	handleSigtstp(int i);
int 	handleSigquit(int i);
int	handleSigabrt(int i);
int 	handleSigint(int i);
int	handleSigsegv(int i);
int 	handleSigterm(int i);

#endif	//_INIT_H
