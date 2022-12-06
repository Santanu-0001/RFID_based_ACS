#ifndef _ACS_GEN
#define _ACS_GEN

#include "rfx.h"
#include "init.h"
#include "gpio.h"

#define ACK		1 //Car gate
#define TAIL	2 //Scooter/Cycle gate
#define PED     	3 //Pedestrian - vris

//Global variables

extern int  sConnect;
extern sigset_t usr1_mask1;
extern sigset_t usr1_mask2;
extern op_mode_t opMode;
extern rdr_stat g_rdr_state;
extern char readerId[3];
extern bool statusPendingToBeSent;


//Returns address of the machine to connect to (Eases portability between Reader/PC)
//RFU: Read the address from system settings file
char * getMachAddr();

//Opens a connection to read
reader_handle_t connect_reader(); 

//Initiates a search operation
search_uid_t st_search(reader_handle_t *rh, bool resetdb);

//The business rule, enters when a search is in progress
_retflag bRule(reader_handle_t *rh, search_uid_t *sh, int fd);

//Returns a tag read in the given structure
tagdb_record_v3_t getTags(reader_handle_t *rh);

//Returns max number of cards for a given gate
 _retflag getMaxCards();

 //Returns min number of cards for a given gate
 _retflag getMinCards();

 //Saves list of authenticated cards
 _retflag save_valid_cards(TagArray *lstCards, int len);

 //Returns tags from the given reader handle
tagdb_record_v3_t getTags(reader_handle_t *rh);

//Returns status of the reader
int ReaderStatus();

//Callback to each thread
void *thread_function(void *arg);

//For the server thread to response the status of the reader
int initializeserver();

// Make the txn string to save
_retflag make_txn_str(TagArray *lstCards, char* _txnstr, unsigned char tagType);

// Convert time stamp from big to little endian
void btolend(u_int  stamp);

// Check whether tag file is updated if so load new file into memory buffer
//_retflag chkForTagUpdt();

// For Fence Gate: Check whether all read tag are from same antenna else return false
//_retflag chk_antenna_id(TagArray *lstCards, int len);

// Business rule applicable for manual intervention 
_retflag mRule(reader_handle_t *rh, search_uid_t *sh, int fd);

//Checks whether LED is applicable and Increments/Resets the counter (p1=Handle of the port, count=No. of times to be incremented, -1 to reset)
_retflag ctrlLEDCounter(int fd, int count);

//Checks and open, close the Barrier if applicable
_retflag ctrlBarrier(int fd, int count);

//Checks and resets hardware controls before the program enters a running state
_retflag ctrlHWSet(int *fd);

// Stop Search and Reset tag database in Reader
//_retflag stop_search(reader_handle_t *rh, search_uid_t *sh);

// Action for failing fundamental operation
_retflag Fail_Action();

// Feedback/timeout for First Barrier (tailgating only)
int FeedFirstRecd(int fd);

//Feedback/timeout for barrier (all control elements)
int FeedRecd(int fd);

/*******Additions for acs_gen2 while breaking up bRule() into sub-functions
 for maintainability*******/
void initTransaction(int* cnt_max_cards, int* cnt_min_cards, short* option);
void incrementLedCount(int fd);
//void storeTagInfo(char *infoStr, char *tagid, short *cardCount);
_retflag validateTagsAndControlHW(reader_handle_t *rh, int fd, search_uid_t *sh, long raw_tag_count);
_retflag waitForAckSwitch(reader_handle_t *rh, int fd, short option);
int FeedRecdForPedestrian(int fd);
//_retflag logTransactionDetails(char* txnInfo, short cardCount, logTarget_t tagType);
_retflag logvalidTransaction(short cardCount, logTarget_t tagType);
_retflag loginvalidTransaction(short cardCount, logTarget_t tagType);
void read_tag(reader_handle_t *rh, int fd, short option);
#endif //_ACS_GEN
