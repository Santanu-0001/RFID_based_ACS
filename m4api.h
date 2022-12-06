#ifndef _M4API_H_
#define _M4API_H_
/* m4api.h		definition of the M4 tag reader API
 *
 * \file m4api.h
 * \author Rehmi Post <rehmi@thingmagic.com>
 * \ingroup API
 *
 * Copyright (c) 2004 ThingMagic LLC. All Rights Reserved.
 */

#include <sys/types.h>


#ifdef WIN32
#include <windows.h>
#define int64_t __int64
#endif


#define EXTERN extern


// Windows DLL linkage specifiers
#ifdef API_EXPORTS
	#undef EXTERN
	#define EXTERN __declspec(dllexport)
#else
	#ifdef WIN32
		#undef EXTERN
		#ifdef __cplusplus
		// linkage when API is used in C++ programs
		#define EXTERN extern "C" __declspec(dllimport)
		#else
		// linkage when API is used in C programs
		#define EXTERN __declspec(dllimport)
		#endif
	#endif
#endif


#define THINGMAGIC_MAGIC_NUMBER                 0x20000920

/* The API Version number is maintained in hex is broken down so that
 * 0x02020000
 * represents version
 * 2.2.0
 * */
#define M4_API_VERSION		(0x02030013)

/**
 * tag operation return values
 */

#define THINGMAGIC_SUCCESS			(0)

#define OPERATION_IDLE           		(0)
#define OPERATION_WAITING_TO_RUN 		(-1)
#define OPERATION_IN_PROGRESS    		(-2)
#define OPERATION_DONE           		(-3)
#define OPERATION_TIMED_OUT      		(-4)
#define OPERATION_BUILD_IN_PROGRESS  	(-5)
#define OPERATION_ERROR		 		(-5)

#define STATE_ACTIVE				(-10)
#define STATE_CLEANUP         			(-11)
#define STATE_ERROR           			(-12)
#define STATE_IDLE            			(-13)
#define STATE_POWERUP         			(-14)
#define STATE_RUNNING         			(-15)
#define STATE_WAITING         			(-16)
#define STATE_CATCHUP				(-17)

#ifdef WIN32
#undef ERROR_SUCCESS
#undef ERROR_INVALID_DATA
#endif

/* XXX - this is ugly and we'll do something about it soon - ERP */
#define ERROR_SUCCESS  				(0)

#define ERROR_INVALID_ARGUMENTS 		(-100)
#define ERROR_INVALID_DATA      		(-101)
#define ERROR_REMOTE            		(-102)
#define ERROR_UNABLE_TO_COMPLY  		(-103)
#define ERROR_MISCONFIGURATION  		(-104)
#define ERROR_NO_RESPONSE       		(-105)
#define ERROR_INVALID_ANTENNA   		(-106)	/* Nonexistent antenna port */
#define ERROR_UNCONNECTED_ANTENNA 		(-107) /* No antenna connected to port */
#define ERROR_VERIFY_FAILURE    		(-108) /* Result not as expected */

#define ERROR_API_ALLOC				(-120)
#define ERROR_API_MAX_HANDLES			(-121)
#define ERROR_NYI				(-127) /* Feature Not Yet Implemented */

/* to go in the error codes */
#define ERROR_FIRMWARE_UNSUPPORTED_PROTOCOL  	(-150)
#define ERROR_FIRMWARE_UPLOAD_FAILED         	(-151)
#define ERROR_FIRMWARE_INSTALL_SCRIPT        	(-152)
#define ERROR_FIRMWARE_DOWNGRADE             	(-153)
#define ERROR_FIRMWARE_CORRUPT_LOCAL_VERSION 	(-154)
#define ERROR_FIRMWARE_NO_VERSION            	(-155)
#define ERROR_FIRMWARE_MACADDRESS            	(-156)
#define ERROR_FIRMWARE_INVALID_URI           	(-157)
#define ERROR_FIRMWARE_UPDATE_INCOMPLETE     	(-158)
#define ERROR_FIRMWARE_THREAD_FAILURE        	(-159)
#define ERROR_FIRMWARE_MALFORMED_VERSION     	(-160)
#define ERROR_FIRMWARE_DISALLOWED_DOWNGRADE     (-161)
#define ERROR_FIRMWARE_BAD_PACKAGE              (-162)

#define ERROR_GPIO_RANGE          		(-170)
#define ERROR_GPIO_NOT_SET        		(-171)
#define ERROR_GPIO_EVENT_NOT_SET  		(-172)

#define ERROR_UNKNOWN           		(-199) /* end-of-sequence marker */




#define FIRMWARE_DOWNGRADE_DISALLOWED		(0)
#define FIRMWARE_DOWNGRADE_ALLOWED		(1<<0)
#define FIRMWARE_CLEAN_INSTALL			(1<<1)
#define FIRMWARE_RESTORE_SETTINGS		(1<<2)
#define FIRMWARE_OUTPUT_MSG			(1<<3)
#define FIRMWARE_AUTOMATIC_HOSTNAME		(1<<4)
#define FIRMWARE_REBOOT                         (1<<5)

typedef enum
{
    TA_TAGS_TO_POP,
    TA_TAGS_TO_CONSUME
}TAG_AVAILABILITY_T;

typedef struct {
    int retval;
    char *package;
    char *version;
    char *maintainer;
    unsigned int flags;
} firmware_desc_t;


/*** reader error messages and return values ***/

EXTERN int reader_errno;	/* error code from above list */
EXTERN const char *reader_errlist[];
EXTERN int reader_nerr;
EXTERN void reader_perror (const char *s);
EXTERN const char* reader_strerror (int err);

/*** reader connection initialization & teardown ***/

/**
 * an opaque handle into the reader API specifying
 * a connection to the radiOS server on a particular tag reader
 */
typedef void *reader_handle_t;

/**
 * Establish a connection to the specified radiOS server
 *
 * \param hostname string describing host name or IP address
 * \return a connection handle or NULL on failure
 */
#define M4API_OPEN_READER
EXTERN reader_handle_t open_reader (char *hostname);

/**
 * Shutdown the given radiOS server connection
 *
 * \param h radiOS server connection handle
 * \return nothing (always successful)
 */
#define M4API_CLOSE_READER
EXTERN int close_reader (reader_handle_t);

/**
 * Reconnect the given radiOS server connection
 *
 * \param h radiOS server connection handle
 * \return the connection handle or NULL on failure
 */
#define M4API_RECONNECT_READER
EXTERN reader_handle_t reconnect_reader (reader_handle_t);

/*** search ***/

/**
 * Reset the tag database
 *
 * \param h radiOS server connection handle
 * \return ERROR_SUCCESS on success, error code otherwise
 */
#define M4API_TAGDB_RESET_DB
EXTERN int tagdb_reset_db (reader_handle_t);

/**
 * search specification record
 */
typedef struct {
    unsigned short protocol_list_length;
    unsigned short protocol_list[16];
    unsigned short antenna_list_length;
    unsigned short antenna_list[16];
    unsigned short timeout_ms;
} search_params_t;

/**
 * operation to check on
 */
typedef void *search_handle_t;

/**
 * database tag record
 */
typedef struct {
    int status;
    unsigned short antenna_id;
    unsigned short protocol_id;
    unsigned short num_bits;
    unsigned short tag_id[16];
    unsigned short read_count;
    unsigned int khz;
    unsigned int tv_sec;
    unsigned int tv_usec;
    int64_t dspmicros;
} tagdb_record_t;

/**
 * database tag record Version 3
 */
typedef struct {
    int status;
    unsigned short antenna_id;
    unsigned short protocol_id;
    unsigned short num_bits;
    unsigned short tag_id[16];
    unsigned short read_count;
    unsigned int khz;
    unsigned int tv_sec;
    unsigned int tv_usec;
    unsigned long dspmicros;
    unsigned short hash;
    unsigned short lqi;
} tagdb_record_v3_t;

/**
 * tag operation record
 */
typedef struct {
    unsigned short protocol;
    unsigned short antenna;
    unsigned short id_length;
    unsigned short id_value[16];
    unsigned short data_type;
    unsigned short data_address;
    unsigned short data_length;
    unsigned short data_value[2048];
} tagop_args_t;


/*** Protocol IDs ***/
enum PROTOCOL_IDS {
    PROTOCOL_ID_NULL                 = 0,
#define PROTOCOL_ID_ZERO           PROTOCOL_ID_NULL
    PROTOCOL_ID_CC915                = 1,
#define PROTOCOL_ID_CC915          PROTOCOL_ID_CC915
#define PROTOCOL_ID_EPC1           PROTOCOL_ID_CC915
#define PROTOCOL_ID_EPC_CLASS_1    PROTOCOL_ID_CC915
#define PROTOCOL_ID_UHF_EPC        PROTOCOL_ID_CC915
    PROTOCOL_ID_CC1356               = 2,
#define PROTOCOL_ID_CC1356         PROTOCOL_ID_CC1356
#define PROTOCOL_ID_HF_EPC_DRAFT   PROTOCOL_ID_CC1356
    PROTOCOL_ID_ISO15693             = 3,
#define PROTOCOL_ID_ISO15693       PROTOCOL_ID_ISO15693
    PROTOCOL_ID_ISO14443                = 4,
#define PROTOCOL_ID_ISO14443       PROTOCOL_ID_ISO14443
    PROTOCOL_ID_CC1356_PHILIPS       = 5,
#define PROTOCOL_ID_CC1356_PHILIPS PROTOCOL_ID_CC1356_PHILIPS
#define PROTOCOL_ID_HF_EPC         PROTOCOL_ID_CC1356_PHILIPS
    PROTOCOL_ID_CC915V0              = 6,
#define PROTOCOL_ID_CC915V0        PROTOCOL_ID_CC915V0
    PROTOCOL_ID_CC915V1              = 7,
#define PROTOCOL_ID_CC915V1        PROTOCOL_ID_CC915V1
    PROTOCOL_ID_I186B                = 8,
#define PROTOCOL_ID_I186B          PROTOCOL_ID_I186B
#define PROTOCOL_ID_ISO18000_6B    PROTOCOL_ID_I186B
#define PROTOCOL_ID_I_CODE_HSL     PROTOCOL_ID_ISO18000_6B
    PROTOCOL_ID_EPC0                 = 9,
#define PROTOCOL_ID_EPC0           PROTOCOL_ID_EPC0
#define PROTOCOL_ID_MATRICS        PROTOCOL_ID_EPC0
#define PROTOCOL_ID_EPC_CLASS_0    PROTOCOL_ID_EPC0
    PROTOCOL_ID_DIAGNOSTIC           = 10,
#define PROTOCOL_ID_DIAGNOSTIC     PROTOCOL_ID_DIAGNOSTIC
    PROTOCOL_ID_LBT                  = 11,
    PROTOCOL_ID_GEN2                 = 12,
#define PROTOCOL_ID_GEN2           PROTOCOL_ID_GEN2
    /* Obtain the range of valid protocol IDS */
    PROTOCOL_ID_LAST_VALUE
};

typedef enum TagID_Length
{
    TagIdLength_64,
    TagIdLength_96Only,
    TagIdLength_96,
    TagIdLength_LastValue,
} TagID_Length;

EXTERN char *protocol_names[];

/**
 * list of supported protocols (16x16=256 bit map)
 */

#define PROTOCOL_MASK_WORDS	(16)
#define PROTOCOL_MASK_MAX_NUM	(PROTOCOL_MASK_WORDS*16)

typedef struct {
    unsigned short bitmap[PROTOCOL_MASK_WORDS];
} protocol_mask_t;

/* set the bit corresponding to a given protocol ID in a protocol mask */
#define PROTO_SET(mask,bit)	{ (mask)[(bit)/16] |=  (1<<(bit)) }
/* clear the bit corresponding to a given protocol ID in a protocol mask */
#define PROTO_CLR(mask,bit)	{ (mask)[(bit)/16] &= ~(1<<(bit)) }
/* test the bit corresponding to a given protocol ID in a protocol mask */
#define PROTO_ISSET(mask,bit)	(((mask)[(bit)/16] &   (1<<(bit))) != 0)

#define MAX_ANTENNA_ARRAY_LEN	16
#define MAX_PROTOCOL_ARRAY_LEN	16

typedef struct {
    int num_antennas;
    int antenna_id[MAX_ANTENNA_ARRAY_LEN];
} antenna_array_t;

typedef struct {
    int num_protocols;
    int protocol_id[MAX_PROTOCOL_ARRAY_LEN];
} protocol_array_t;

#define GPIO_0		(0x04)
#define GPIO_1		(0x08)
#define GPIO_2		(0x10)
#define GPIO_3		(0x02)
#define GPIO_4		(0x20)

enum gpio_reader_event
{
    READER_EVENT_IMMEDIATE 		= 0, /* Special Case: Set GPIO now */
    READER_EVENT_SEARCH_START		= 1, /* Reader in active search mode */
    READER_EVENT_SEARCH_STOP		= 2, /* Reader not currently searching */
    READER_EVENT_READY			= 3, /* TMD is running */
    READER_EVENT_INPUT_SEARCH_START 	= 4, /* Reader must wait for input */
    READER_EVENT_INPUT_SEARCH_STOP 	= 5, /* Reader must wait for input */
    READER_EVENT_PROTOCOL_ACTIVE 	= 6, /* Reader is actively running a protocol */
    READER_EVENT_PROTOCOL_INACTIVE 	= 7, /* Reader has to a protocol inactive state */
    READER_EVENT_CD_START 	= 8, /* Reader has started a Carrier Detection */
    READER_EVENT_CD_STOP 	= 9, /* Reader has completed a Carrier Detection */
    NUM_READER_EVENTS 			= 10, /* Length of reader_event list */
};

struct gpio_binding
{
    unsigned event;		/* Event to bind to */
    unsigned state;		/* GPIO state to set */
    unsigned mask;		/* Mask against GPIO state */
};


/*** The new detailed tag search and operations structure **/
#define TMD_MAX_TUPLE_TYPES 16
enum durationEnum_t {
    DUR_NULL =              0,
    DUR_INFINITE =          0,
    DUR_TIME_AND_COUNT =    1,
    DUR_TIME_OR_COUNT = 2,
    DUR_TIME_ONLY =         3,
    DUR_COUNT_ONLY =        4,
    DUR_LAST_VALUE =        5,
};
typedef enum durationEnum_t durationEnum_t;

enum tagDataType_t {
    TDTYPE_NULL =       0,
    TDTYPE_KILL =       1,
    TDTYPE_LOCK =       2,
    TDTYPE_DATA =       3,
    TDTYPE_LAST_VALUE = 4,
};
typedef enum tagDataType_t tagDataType_t;

enum seqObj_t {
    ORDER_NULL =       0,
    ORDER_IN_ORDER =   1,
    ORDER_MIN_TOTAL_TIME =   2,
    ORDER_MIN_PROTOCOL_TIME =   3,
    ORDER_MIN_ANTENNA_TIME =   4,
    ORDER_LAST_VALUE = 5,
};
typedef enum seqObj_t seqObj_t;

enum synchObj_t {
    SYNCH_NULL =       0,
    SYNCH_NTP = 1,   /* Synchronize searches */
    SYNCH_LBT = 2,
    SYNCH_GPIO = 3,
    SYNCH_REGULATORY_TIMEOUT = 4, /* Synchronize Reg. timeouts */
    SYNCH_NTP_REGULATORY_TIMEOUT = 5, /* Synch Searches and Reg. timeouts */
    SYNCH_AUTO = 6,
    SYNCH_RESERVED_7 = 7,
    SYNCH_RESERVED_8 = 8,
    SYNCH_LAST_VALUE = 9,
};
typedef enum synchObj_t synchObj_t;

struct synchObjNtpData_t
{
	   unsigned short offset;
};
typedef struct synchObjNtpData_t synchObjNtpData_t;

struct synchObjRegTimeoutData_t
{
	   short powerThreshold;
};
typedef struct synchObjRegTimeoutData_t synchObjRegTimeoutData_t;

struct synchObjNtpRegTimeoutData_t
{
	   unsigned short offset;
		   short powerThreshold;
};
typedef struct synchObjNtpRegTimeoutData_t synchObjNtpRegTimeoutData_t;

struct synchObjAutoData_t
{
	   unsigned short offset;
};
typedef struct synchObjLbtData_t synchObjLbtData_t;

struct synchObjData_t
{
	unsigned short type;
	union
	{
		struct synchObjNtpData_t ntpData;
		struct synchObjAutoData_t autoData;
		struct synchObjRegTimeoutData_t regTimeoutData;
		struct synchObjNtpRegTimeoutData_t ntpRegTimeoutData;
		long value[8];
	}synchObjData_u;
};
typedef struct synchObjData_t synchObjData_t;

enum hopObj_t {
    HOP_NULL =       0,
    HOP_FREQUENCY =  1,
    HOP_RESERVED_1 = 2,
    HOP_CONVEYOR =   3,
    HOP_LAST_VALUE = 4,
};
typedef enum hopObj_t hopObj_t;

enum tupleType_t {
    TTYPE_NULL =           0,
    TTYPE_PROTOCOL =       1,
    TTYPE_ANTENNA =        2,
    TTYPE_POWER =          3,
    TTYPE_DURATION =       4,
    TTYPE_LAST_VALUE =     5,
};
typedef enum tupleType_t tupleType_t;

enum cycleObj_t {
    CYCLE_NULL =                  0,
    CYCLE_ANTENNA_GROUP_THEN_PROTOCOL = 1,
    CYCLE_ANTENNA_THEN_PROTOCOL = 1, /* The omission of the phrase GROUP was confusing
                                        This is here for backward compatibility */
    CYCLE_PROTOCOL_THEN_ANTENNA_GROUP = 2,
    CYCLE_PROTOCOL_THEN_ANTENNA = 2, /* The omission of the phrase GROUP was confusing
                                        This is here for backward compatibility */
    CYCLE_LAST_VALUE =            3,
};
typedef enum cycleObj_t cycleObj_t;

enum searchMode_t {
    SM_NULL      =0,
    SM_STANDARD=1,
    SM_QUICK   =2,
    SM_MINIMAL =3,
    SM_LAST_VALUE=4
};
typedef enum searchMode_t searchMode_t;

struct durationObj_t {
    u_short type;
    u_short time;
    u_short count;
	 u_short pad[7];
};
typedef struct durationObj_t durationObj_t;

struct tupleData_t {
    u_short type;
    union {
        long value;
        long protocol_id;
        durationObj_t duration;
    } tupleData_u;
};
typedef struct tupleData_t tupleData_t;

struct tuple_t {
    tupleData_t data[TMD_MAX_TUPLE_TYPES];
};
typedef struct tuple_t tuple_t;

struct tupleObj_t {
    u_short setLength;
    u_short tupleSet[TMD_MAX_TUPLE_TYPES];
    struct {
        u_int tupleVal_len;
        tuple_t *tupleVal_val;
    } tupleVal;
};
typedef struct tupleObj_t tupleObj_t;

struct tagopObj_t {
    u_short dtype;
    u_short dataAddress;
    u_short dataLength;
    u_short dataValue[2048];
};
typedef struct tagopObj_t tagopObj_t;

struct tagDataObj_t {
    u_short protocol;
    u_short antenna;
    u_short id_length;
    u_short id_value[16];
    u_long frequency;
    u_long dspmicrosLow;
    u_long lqi;
    tagopObj_t *tagInfo;
};
typedef struct tagDataObj_t tagDataObj_t;

struct atuple_t {
    struct {
        u_int antenna_len;
        u_long *antenna_val;
    } antenna;
};
typedef struct atuple_t atuple_t;

struct antennaSet_t {
    struct {
        u_int groups_len;
        atuple_t *groups_val;
    } groups;
    struct {
        u_int sequentialSet_len;
        u_long *sequentialSet_val;
    } sequentialSet;
    struct {
        u_int simultaneousSet_len;
        u_long *simultaneousSet_val;
    } simultaneousSet;
};
typedef struct antennaSet_t antennaSet_t;

struct antGroupEvent_t {
    unsigned char groupIndex;
    unsigned char useCdca;
    unsigned char carrierDetectCount;/* currently unused */
    unsigned long carrierDetectThreshold;
    unsigned short gpioLineMask;
    unsigned short gpioLineState;
};
typedef struct antGroupEvent_t antGroupEvent_t;

typedef enum
{
  MINIMUM_RF_OUTPUT    =0,
  MAXIMUM_READTIME     =1
} searchModeVariant_t;


struct quickSearch_params_t {
	u_short quickSearchObjective;
	int interval;
	int count;
        u_int rfPowerLevel;
        u_int quickSearchAntennas;
};
typedef struct quickSearch_params_t quickSearch_params_t;

struct searchMode_data_t {
	u_short searchMode;
	union {
		quickSearch_params_t quickSearchParams;
		long value[8];
	} searchMode_data_u;
};
typedef struct searchMode_data_t searchMode_data_t;

struct searchTupleObj_t {
	antennaSet_t antennas;
	tupleObj_t global;
	tupleObj_t cycle;
	durationObj_t cycleDuration;
	u_short cycleOrder;
	u_short sequenceOrder;
	u_short synchType;
	u_short hopType;
	u_short searchMode;
	struct synchObjData_t synchData;
	struct searchMode_data_t searchModeData;
	struct {
		u_int antGroupEvents_len;
		struct antGroupEvent_t *antGroupEvents_val;
	} antGroupEvents;
	tagDataObj_t *tagInfo;
};
typedef struct searchTupleObj_t searchTupleObj_t;

/****************************************************************/

typedef int search_uid_t;
/**
 * Perform a search using the given antenna mask
 *
 * \param h mercuryOS server connection handle
 * \param s pointer to a search specification record
 * \return if negative, an error code;
 * otherwise, a UID (unique ID) for the search
 */
#define M4API_SEARCH_START
EXTERN search_uid_t search_start(reader_handle_t h, search_params_t *s);

/**
 * Perform a check on whether there are tags available in the DB
 *
 * \param h mercuryOS server connection handle
 * \param atype The Availability Type
 * \return the number of tags available in the DB
 */
#define M4API_TAGS_AVAILABLE
EXTERN unsigned short tags_available(reader_handle_t h,int atype);

/**
 * Perform a search using the given antenna mask that tags the conveyor 
 * belt environment
 *
 * \param h mercuryOS server connection handle
 * \param p The search parameters that define this search
 * \return if negative, an error code;
 * otherwise, a UID (unique ID) for the search
 */
#define M4API_CONVEYOR_READ_START
EXTERN search_uid_t conveyor_read_start(reader_handle_t h,search_params_t *p);

/**
 * Perform a check on whether there are tags available in the DB
 *
 * \param h mercuryOS server connection handle
 * \param sh The search_uid of the search one is looking to stop
 * \return The ID of the search stopped
 */
#define M4API_CONVEYOR_READ_STOP
EXTERN int conveyor_read_stop(reader_handle_t h,search_uid_t sh);

/**
 * Perform a search using the given searchTupleObj
 *
 * \param h mercuryOS server connection handle
 * \param p The search parameters that define this search
 * \return if negative, an error code;
 * otherwise, a UID (unique ID) for the search
 */
#define M4API_DETAILED_READ_START
EXTERN search_uid_t detailed_read_start(reader_handle_t h,searchTupleObj_t *p);

/**
 * Perform a check on whether there are tags available in the DB
 *
 * \param h mercuryOS server connection handle
 * \param sh The search_uid of the search one is looking to stop
 * \return The ID of the search stopped
 */
#define M4API_DETAILED_READ_STOP
EXTERN int detailed_read_stop(reader_handle_t,search_uid_t sh);


/** Remove next record from tagdb
 *
 * \param h radiOS server connection handle
 * \param tp pointer to user-allocated tag database record
 * \return ERROR_SUCCESS on success, error code otherwise.
 * Contents of removed record in tp. If status field is 0, then
 * that record is invalid and the tagdb is empty.
 */
#define M4API_TAGDB_POP
EXTERN int tagdb_pop(reader_handle_t h, tagdb_record_t *tp);
#define M4API_TAGDB_CONSUME
EXTERN int tagdb_consume(reader_handle_t h, tagdb_record_v3_t *tp);


/*** status check ***/

/**
 * Return the status of an operation in progress
 *
 * For now, all operations are blocking so this always returns -1 == done
 * \param h radiOS server connection handle
 * \param s handle for the search request of interest
 * \retval OPERATION_DONE done
 * \retval OPERATION_IN_PROGRESS not done
 * \return an error value on error
 */
#define M4API_OPERATION_STATUS
EXTERN int operation_status(reader_handle_t h, search_uid_t u);


/*** Tag write operations ***/

/**
 * Write ID to a tag
 *
 * \param h radiOS server connection handle
 * \param a pointer to a tag operation record
 * \return ERROR_SUCCESS on success, error code otherwise.
 */
#define M4API_TAGID_WRITE
EXTERN int tagid_write(reader_handle_t h, tagop_args_t *a);

/**
 * Lock a tag's ID
 *
 * \param h radiOS server connection handle
 * \param a pointer to a tag operation record
 * \return a tag operation return value code
 */
#define M4API_TAGID_LOCK
EXTERN int tagid_lock(reader_handle_t h, tagop_args_t *a);

/**
 * Kill a tag
 *
 * \param h radiOS server connection handle
 * \param a pointer to a tag operation record
 * \return ERROR_SUCCESS on success, error code otherwise.
 */
#define M4API_TAG_KILL
EXTERN int tag_kill(reader_handle_t h, tagop_args_t *a);

/**
 * Set a tag's password
 *
 * \param h radiOS server connection handle
 * \param a pointer to a tag operation record
 * \return ERROR_SUCCESS on success, error code otherwise.
 */
#define M4API_TAG_PASSWD
EXTERN int tag_passwd(reader_handle_t h, tagop_args_t *a);

/**
 * Read a tag's data
 *
 * \param h radiOS server connection handle
 * \param a pointer to a tag operation record
 * \return ERROR_SUCCESS on success, error code otherwise.
 */
#define M4API_TAGDATA_READ
EXTERN int tagdata_read(reader_handle_t h, tagop_args_t *a);

/**
 * Write data to a tag
 *
 * \param h radiOS server connection handle
 * \param a pointer to a tag operation record
 * \return ERROR_SUCCESS on success, error code otherwise.
 */
#define M4API_TAGDATA_WRITE
EXTERN int tagdata_write(reader_handle_t h, tagop_args_t *a);

/**
 * Lock a tag's data
 *
 * \param h radiOS server connection handle
 * \param a pointer to a tag operation record
 * \return ERROR_SUCCESS on success, error code otherwise.
 */
#define M4API_TAGDATA_LOCK
EXTERN int tagdata_lock(reader_handle_t h, tagop_args_t *a);


/**
 * Get the map of supported protocols
 *
 * \param h radiOS server connection handle
 * \param p a pointer to a protocol_array_t structure
 * \return number of protocols on success, error code otherwise.
 * If successful, p is filled in with a list of supported prtocol IDs
 */
#define M4API_GET_SUPPORTED_PROTOCOLS
EXTERN int get_supported_protocols(reader_handle_t h, protocol_array_t *p);

/**
 * Get an array of connected antennas
 *
 * \param h radiOS server connection handle
 * \param a a pointer to an antenna_array_t structure
 * \return number of antennas on success, error code otherwise.
 * If successful, a is filled in with a list of connected antenna IDs
 */
#define M4API_GET_CONNECTED_ANTENNAS
EXTERN int get_connected_antennas(reader_handle_t h, antenna_array_t *a);



/*** diagnostics ***/

/**
 * Get the firmware version
 *
 * \param h radiOS server connection handle
 * \return firmware version string
 */
#define M4API_GET_FIRMWARE_VERSION
EXTERN char *get_firmware_version(reader_handle_t h);

/**
 * Get the operating system version string
 *
 * \param h radiOS server connection handle
 * \return OS version string
 */
#define M4API_GET_OS_VERSION
EXTERN char *get_os_version(reader_handle_t h);

/**
 * Get the current device settings
 *
 * \param h radiOS server connection handle
 * \return OS version string
 */
#define M4API_READ_SETTINGS
EXTERN char *read_settings(reader_handle_t h);

/**
 * Set the tag reader's TX power level
 *
 * \param h radiOS server connection handle
 * \param power a real value to set the reader output power (in dBm)
 * \reader_errno = ERROR_SUCCESS on success, error code otherwise.
 * returns actual power (>= 0) if successful, -1.0 otherwise
 *
 */
#define M4API_SET_TX_POWER
EXTERN double set_tx_power(reader_handle_t h, float power);

/**
 * Set the tag reader's TX power level on a given slot
 *
 * \param h radiOS server connection handle
 * \param power a real value to set the reader output power (in dBm)
 * \param slot AE slot being referenced
 * \reader_errno = ERROR_SUCCESS on success, error code otherwise.
 * returns actual power (>= 0) if successful, -1.0 otherwise
 *
 */
#define M4API_SET_SLOT_TX_POWER
EXTERN double set_slot_tx_power(reader_handle_t h, float power, short slot);

/**
 * Set the tag reader's TX power level on a given antenna
 *
 * \param h radiOS server connection handle
 * \param power a real value to set the reader output power (in dBm)
 * \param antenna AE antenna being referenced
 * \reader_errno = ERROR_SUCCESS on success, error code otherwise.
 * returns actual power (>= 0) if successful, -1.0 otherwise
 *
 */
#define M4API_SET_ANTENNA_TX_POWER
EXTERN double set_antenna_tx_power(reader_handle_t h, float power, short antenna);

/**
 * Get the tag reader's TX power level on a given slot
 *
 * \param h radiOS server connection handle
 * \param slot slot being referenced
 * \reader_errno = ERROR_SUCCESS on success, error code otherwise.
 * returns actual power (>= 0) if successful, -1.0 otherwise
 *
 */
#define M4API_GET_SLOT_TX_POWER
EXTERN double get_slot_tx_power(reader_handle_t h, short slot);

/**
 * Get the tag reader's TX power level on a given slot
 *
 * \param h radiOS server connection handle
 * \param antenna antenna being referenced
 * \reader_errno = ERROR_SUCCESS on success, error code otherwise.
 * returns actual power (>= 0) if successful, -1.0 otherwise
 *
 */
#define M4API_GET_ANTENNA_TX_POWER
EXTERN double get_antenna_tx_power(reader_handle_t h, short antenna);

/**
 * Get the tag reader's TX power level on a given slot
 *
 * \param h radiOS server connection handle
 * \param antenna antenna being referenced
 * \reader_errno = ERROR_SUCCESS on success, error code otherwise.
 * returns actual power (>= 0) if successful, -1.0 otherwise
 *
 */
#define M4API_GET_CURRENT_TX_POWER
EXTERN double get_current_tx_power(reader_handle_t h, short* antenna);


#define M4API_REBOOT_READER
EXTERN void reboot_reader(reader_handle_t reader_handle,
                          unsigned long magic,
                          unsigned int safe_mode_flag);

#define M4API_CONFIG_GET_TAGOP_RETRY_TIMEOUT_MS
EXTERN unsigned short config_get_tagop_retry_timeout_ms(reader_handle_t h);
#define M4API_CONFIG_SET_TAGOP_RETRY_TIMEOUT_MS
EXTERN int config_set_tagop_retry_timeout_ms(reader_handle_t h,unsigned short timeout_ms);

#define M4API_CONFIG_GET_EPC0_DEGHOST
EXTERN int config_get_EPC0_deghost(reader_handle_t h);
#define M4API_CONFIG_SET_EPC0_DEGHOST
EXTERN int config_set_EPC0_deghost(reader_handle_t h,int on);

#define M4API_CONFIG_GET_EPC0_DEGHOST_LEN
EXTERN unsigned long  config_get_EPC0_deghost_len(reader_handle_t h);
#define M4API_CONFIG_SET_EPC0_DEGHOST_LEN
EXTERN int config_set_EPC0_deghost_len(reader_handle_t h,unsigned long len);

#define M4API_CONFIG_GET_EPC1_DEGHOST
EXTERN int config_get_EPC1_deghost(reader_handle_t h);
#define M4API_CONFIG_SET_EPC1_DEGHOST
EXTERN int config_set_EPC1_deghost(reader_handle_t h,int on);

/* New stuff, not yet documented */

#define M4API_CONFIG_SET_EPC1_ID_LENGTH
int config_set_epc1_id_length(reader_handle_t h, enum TagID_Length len_enum);
#define M4API_CONFIG_GET_EPC1_ID_LENGTH
enum TagID_Length config_get_epc1_id_length(reader_handle_t handle);

#define M4API_CONFIG_SET_EPC0_SEARCH_DEPTH
EXTERN int config_set_EPC0_search_depth(reader_handle_t h, int depth);
#define M4API_CONFIG_GET_EPC0_SEARCH_DEPTH
EXTERN int config_get_EPC0_search_depth(reader_handle_t h);
#define M4API_CONFIG_SET_EPC1_96_BIT_SUPPORT
EXTERN int config_set_EPC1_96_bit_support(reader_handle_t h, int on);
#define M4API_CONFIG_GET_EPC1_96_BIT_SUPPORT
EXTERN int config_get_EPC1_96_bit_support(reader_handle_t h);
#define M4API_CONFIG_SET_SYNCH_REGULATORY_TIMEOUT
EXTERN int config_set_synch_regulatory_timeout(reader_handle_t h,int val);
#define M4API_CONFIG_GET_SYNCH_REGULATORY_TIMEOUT
EXTERN int config_get_synch_regulatory_timeout(reader_handle_t h);

#define M4API_READER_CONFIG_GET
char * reader_config_get (reader_handle_t h, char *key);
#define M4API_READER_CONFIG_SET
int reader_config_set (reader_handle_t h, char *key, char *value);

#define M4API_GPIO_get
EXTERN int GPIO_get(reader_handle_t h, int gpio);
#define M4API_GPIO_set
EXTERN int GPIO_set(reader_handle_t h, int gpio, int value);
#define M4API_GPIO_set_reader_event
EXTERN int GPIO_set_reader_event(reader_handle_t h, int gpio, int value, int event);

#define M4API_READER_FW_UPDATE
EXTERN int reader_fw_update(reader_handle_t h, char *fw_uri, int flags);
#define M4API_READER_FW_UPDATE_COMPLETE
EXTERN int reader_fw_update_complete(reader_handle_t h, firmware_desc_t *fp);

/* Return NULL on error */
#define M4API_AUTO_SEARCH_START
EXTERN search_handle_t auto_search_start(reader_handle_t h, search_params_t *s, double T0, double period, double offset, double T1);
#define M4API_AUTO_SEARCH_STOP
EXTERN int auto_search_stop(search_handle_t s);
#define M4API_SEARCH_ACTIVE
EXTERN int search_active(search_handle_t sh);
#define M4API_GET_RFSTATE
EXTERN short get_rfState (reader_handle_t h, long* retVal);

/* retrieve the key parameter from the reader */
#define M4API_READER_PARAMS_GET
EXTERN char *reader_params_get (reader_handle_t h, char *key);
/* set the key parameter on the reader, with value */
#define M4API_READER_PARAMS_SET
EXTERN int reader_params_set (reader_handle_t h, char *key, char *value);
/* retrieve the list of parameters from the reader */
#define M4API_READER_PARAMS_LIST
EXTERN char** reader_params_list(reader_handle_t h,int* list_length);

/****************************************************************/

/* XXX -- not yet implemented */
#if 0
/**
 * Get the safe mode firmware version
 *
 * \param h radiOS server connection handle
 * \return safe mode firmware version string
 */
EXTERN char *get_safemode_version(reader_handle_t h);

/*** set per-protocol params (proposed) ***/
EXTERN int protocol_params_read(reader_handle_t h, tagop_args_t *a);
EXTERN int protocol_params_write(reader_handle_t h, tagop_args_t *a);
#endif

/****************************************************************/
#endif
