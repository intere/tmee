//
// =======================================================================================
//
//	Copyright 2005-2006 Digital Acoustics
//
//	The source code included in this file is created as an example.  There is no specific
//	fitness or purpose for this code, and it is presented in an AS-IS basis.
//
//	Methods and processes specified by these files are purely for the purpose of example.
//
// =======================================================================================
//
#ifndef __DAPassThruDll__

#define __DAPassThruDll__

#include "winsock2.h"

#define	MAX_NAMELEN	50
#define	MAX_LOGONLEN 100

#define	EVENT_PRINT_STRING			0
#define EVENT_NEW_CONNECTION		1
#define	EVENT_RECV_DATA				2
#define	EVENT_ERROR					3
#define	EVENT_AUDIO_LOW_MARK		4
#define EVENT_AUDIO_HIGH_MARK		5
#define EVENT_AUDIO_UNDERFLOW		6
#define EVENT_AUDIO_DONE			7
#define	EVENT_INTERCOM_DONE_TALKING	8
#define	EVENT_RECONNECT				9
#define	EVENT_INTERCOM_KEYPRESS		10
#define EVENT_GPIO_STATE_CHANGE		11
#define	EVENT_INTERCOM_LISTENING	12
#define	EVENT_INTERCOM_TALKING		13
#define	EVENT_DISCONNECT			14
#define	EVENT_STOP					15
#define	EVENT_INTERCOM_STATUS		16			// Status change on the intercom
#define	EVENT_AUDIO_WATERMARK		17

#define	EVENT_CALL_QUEUE_NEW		20
#define	EVENT_CALL_QUEUE_UPDATE		21
#define	EVENT_CALL_QUEUE_DELETE		22
#define	EVENT_CALL_QUEUE_AUDIO		23			// Audio coming in, record it!
#define	EVENT_CALL_QUEUE_AUDIO_DONE	24			// Audio done

#define EVENT_CONSOLE_CONNECT		30
#define EVENT_CONSOLE_DISCONNECT	31
#define EVENT_CONSOLE_SERVER_TIME	32			// This is the server time

#define	EVENT_IMIN					40
#define	EVENT_SETC					41

#define	EVENT_RELAY_DEACTIVATED		50

#define	EVENT_CALL_QUEUE_MEMBERSHIP	70
#define	EVENT_CALL_QUEUE_STATUS		71

#define	EVENT_CONSOLE_GROUP_REPORT	80			// A group report
#define EVENT_CONSOLE_GROUP_MEMBER	81			// A member report
#define	EVENT_CONSOLE_GROUP_MESSAGE 82			// A group message
#define	EVENT_CONSOLE_GROUP_PLAYING	83			// Something is playing on the server

#define	EVENT_CONSOLE_CONSOLE_READY	100			// The remote console initialization is complete
#define	EVENT_CONSOLE_UPDATE_AVAILABLE	101		// The server has some updated settings available

#define	OPMODE_NORM					0	//000000000
#define	OPMODE_SUPPORTS_DLE_AUDIO	1	//.0	Supports DLE encoded audio
#define	OPMODE_REMLISTN_DISA		2	//.1   ' USE FOR GETLISTENDISABLE ON COMM ss0524
#define	OPMODE_INPUT				4	//.2
#define	OPMODE_WAKEMON				8	//.3   ' controlled locally
#define	OPMODE_RELAY				16	//.4   'set noMon too!
#define	OPMODE_VOX					32	//.5 set noMon too!
#define	OPMODE_NOBEEP				64	//.6
#define	OPMODE_NOMON				128	//.7
#define	OPMODE_MASK					OPMODE_WAKEMON + OPMODE_REMLISTN_DISA

#define OPMODE_RELAYON_TALK		1
#define OPMODE_ALWAYS_DISPLAY	2
#define	OPMODE_CALL_BUTTON		4	// .2 byte 2  PTT sends a command rather than starting audio
#define	OPMODE_RELAYON_PTT		8	// .3 byte 2  PTT relay on
#define	OPMODE_MONITOR_MODE		16
#define	OPMODE_DENY_UNSOLICITED 32
#define OPMODE_GENERAL_SENSOR	128

#define	OPNIC_SENSOR_ACTIVE_HIGH	2	// .1 means that the sensor is active with high values
#define	OPNIC_INPUT_LINE_IN			4	// .2 means that line-in is in use rather than the microphone (IP7)
#define OPNIC_FULL_DUPLEX			8	// .3 means support for full-duplex
#define	OPNIC_ST_MODE				16	// .4 indicates whether the ST style intercom is in ST or 2W mode

//#define	OPNIC_2_PRIVACY_MODE		1	// .0 means that the server intercom is in privacy mode
#define	OPNIC_2_SUPPORTS_RTP_AUDIO		2	// .1 means whether the intercom supports RTP Audio
#define	OPNIC_2_ALLOW_RTP_AUDIO			4	// .2 means whether TMEE should send UDP/RTP Audio to this intercom (if it supports it)
#define	OPNIC_2_HANDSET				(1<<3)	// .3 means that the unit has a handset on it
#define	OPNIC_2_HANDSET_SPEAKER		(1<<4)	// .4 means that the unit has a handset and an external speaker (allow broadcasts onhook)

#define	TYPE_MICROPHONE		0
#define	TYPE_FILE			1
#define	TYPE_ULAW			2
#define TYPE_PCM			0
#define	TYPE_UDP			4
#define	TYPE_TCP			0
#define TYPE_NO_BEEPS		8
#define TYPE_ICMP			16

#define	DA_LOCAL			0
#define	DA_REMOTE			1

#define DA_ADMIN			1					// Login has admin rights

//#define ERROR_SUCCESS		0					// Already Defined
#define	ERROR_BAD_HANDLE				ERROR_SUCCESS-1
#define	ERROR_CONNECT_FAILED			ERROR_SUCCESS-2
#define ERROR_BAD_INTERCOM				ERROR_SUCCESS-3
#define	ERROR_BAD_GROUP					ERROR_SUCCESS-4
#define	ERROR_RESP_TIMEOUT				ERROR_SUCCESS-5
#define	ERROR_INTERCOM_BUSY				ERROR_SUCCESS-6
#define	ERROR_CALL_NOT_SUPPORTED		ERROR_SUCCESS-7
#define	ERROR_LOGON_FAILED				ERROR_SUCCESS-8
#define	ERROR_BAD_VERSION				ERROR_SUCCESS-9
#define	ERROR_BIND_FAILED				ERROR_SUCCESS-10
#define	ERROR_BAD_STRUCTURE				ERROR_SUCCESS-11
#define	ERROR_BAD_PLAYTONE				ERROR_SUCCESS-12

#define ERROR_BAD_USERID_PWD			ERROR_SUCCESS-30
#define ERROR_ALREADY_LOGGED_IN			ERROR_SUCCESS-31
#define	ERROR_OUT_OF_LICENSES			ERROR_SUCCESS-32
#define	ERROR_NOT_LICENSED				ERROR_SUCCESS-33
#define	ERROR_SERVER_FILE_NOT_FOUND		ERROR_SUCCESS-34

#define	STATUS_MASK						0x1fff

#define	STATUS_ONHOOK					0x4000
#define	STATUS_ICOM_TIMEOUT				(0x2000|STATUS_TALKING)
#define	STATUS_DISCONNECTED				0
#define	STATUS_IDLE						1
#define STATUS_INUSE					(STATUS_IDLE|0x8000)
#define	STATUS_INUSE_ONHOOK				(STATUS_INUSE|STATUS_ONHOOK)
#define	STATUS_IDLE_ONHOOK				(STATUS_IDLE|STATUS_ONHOOK)
#define	STATUS_TALKING					2
#define STATUS_TALKING_INUSE			(STATUS_TALKING|0x8000)
#define	STATUS_TALKING_ONHOOK			(STATUS_TALKING|STATUS_ONHOOK)
#define STATUS_TALKING_ONHOOK_INUSE		(STATUS_TALKING|STATUS_ONHOOK|0x8000)
#define	STATUS_LISTENING				3
#define STATUS_LISTENING_INUSE			(STATUS_LISTENING|0x8000)
#define	STATUS_LISTENING_ONHOOK			(STATUS_LISTENING|STATUS_ONHOOK)
#define	STATUS_LISTENING_ONHOOK_INUSE	(STATUS_LISTENING|STATUS_ONHOOK|0x8000)
#define STATUS_COMMAND					4
#define	STATUS_COMMAND_INUSE			(STATUS_COMMAND|0x8000)
#define	STATUS_COMMAND_ONHOOK			(STATUS_COMMAND|STATUS_ONHOOK)
#define	STATUS_COMMAND_ONHOOK_INUSE		(STATUS_COMMAND|STATUS_ONHOOK|0x8000)
#define	STATUS_FDX						5							// Both talking and listening
#define	STATUS_FDX_INUSE				(STATUS_FDX|0x8000)
#define	STATUS_FDX_ONHOOK				(STATUS_FDX|STATUS_ONHOOK)
#define	STATUS_FDX_ONHOOK_INUSE			(STATUS_FDX|STATUS_ONHOOK|0x8000)

#define	SELECT_SOME				0x01						// Allow partial selection
#define SELECT_FORCE			0x02						// Force selection if already connected

struct _MAC
{
	unsigned char MAC1;
	unsigned char MAC2;
	unsigned char MAC3;
	unsigned char MAC4;
	unsigned char MAC5;
	unsigned char MAC6;
};

struct _PICfirmwareRev
{
	int	major;
	int	minor;
	int	release;
	int	build;
	int make;				// Make?  this is a letter usually

	int	opMode1;			// The operational bits of the intercom
	int	opMode2;			// The operational bits of the intercom

	int	feature1;			// The feature bits of the intercom
	int	feature2;			// The feature bits of the intercom
};

struct _NICfirmwareRev
{
	char	major;
	char	minor;
	char	build;
};

struct _iComStructure
{
	int	size;
	unsigned status;
	unsigned char name[48];
	struct _MAC MAC;
	struct sockaddr_in addr_in;
	unsigned GPIO;
	unsigned volume;

	time_t	lastActive;				// when was the last activity

	struct _PICfirmwareRev picRev;
	struct _NICfirmwareRev nicRev;
};

struct _iComQueueInfo
{
	char	queueKey[MAX_NAMELEN];
	char	queueName[MAX_NAMELEN];

	DWORD	priority;
	DWORD	membership;				// Used in the membership call

	BOOL	bOverflow;
	BOOL	bPriorityOverflow;		// Priority Intercoms are Overflow
};

struct _iComQueueList
{
	struct _iComQueueList *next;
	struct _iComQueueInfo queueMembership;
};

struct _AudioData
{
	char	*buffer;
	int		len;
	short	audioFormat;
};

struct _AudioRequest
{
	int len;
};

struct _KeyPress
{
	int upDown;						// 0 = down; 1 = up
	int keycode;					// The keycode
};

struct _CQData
{
	char	name[MAX_NAMELEN];
	char	message[MAX_NAMELEN];
	time_t	timeLength;
	time_t	time;
};

struct _msg
{
	struct _msg *next;

	int		socket;
	struct _iComStructure *iComStructure;
	void	*data;

	BOOL	bDone;				// Done with this message?
};

struct _groupData
{
	int  groupSocket;
	int  groupCount;
	char groupName[MAX_LOGONLEN];
	char groupKey[MAX_LOGONLEN];
	BOOL bMember;
	BOOL bGlobalGroup;
};

struct _groupItems
{
	struct _groupItems *next;

	struct _itemData *pIcomData;

	BOOL	bLocalIcomRecord;
};

struct _groupDataList
{
	struct _groupDataList *next;

	int		type;

	struct _groupData *pGroupData;			// This is a Group Type

	struct _groupItems	*pGroupItems;		// This has an associated list of intercoms

	struct _itemData *pIcomData;			// This is an intercom type
};

struct _messageData
{
	char	title[MAX_LOGONLEN];
	char	key[MAX_LOGONLEN];

	char	groupKey[MAX_LOGONLEN];

	BOOL	bRepeats;
	int		priority;
};

struct _messageDataList
{
	struct _messageDataList *next;

	struct _messageData *pMessageData;

	int		nGroupsDisplayed;						// Used in determining the intersection between groups
};

struct _messagePlayingData
{
	int		groupSocket;					// Which group socket is still playing?
	char	key[MAX_LOGONLEN];				// What is the key to the message playing?
};

struct _workData
{
	struct _workData *next;

	int	socket;
	int command;
	time_t when;
};

//typedef int (__stdcall *DAInit)(
//		void *stuff);		// The Init returns a handle to theData

typedef BOOL (__stdcall *DAOpen)
	(	HANDLE *hOpen,
		UINT *flags,
		void *callback,
		void *userData );

typedef BOOL (__stdcall *DAStart)
	(	HANDLE hOpen,
		struct sockaddr_in *sock_in,
		int threadPriority,
		char *name,
		char *password,
		char *username,
		UINT *rights);

typedef BOOL (__stdcall *DAStartAtAddress)
	(	HANDLE hInfo,
		char *IPAddress,
		int IPPort,
		int threadPriority,
		char *name,
		char *password,
		char *userName,
		UINT *info);

typedef BOOL (__stdcall *DAStop)
	(	HANDLE hOpen );

typedef BOOL (__stdcall *DAClose)
	(	HANDLE hOpen );

typedef BOOL (__stdcall *DASetDebug )
	( int level );

typedef BOOL (__stdcall *DADllVersion)
	(	int *major, int *minor, int *version );

typedef BOOL (__stdcall *DADisconnect )
	( HANDLE hInfo, int iCom );

typedef BOOL (__stdcall *DASelectIntercom )
	( HANDLE hInfo, int iCom, int bSelect, unsigned bOptions );

typedef BOOL (__stdcall *DACreateGroup )
	( HANDLE hInfo, int *gId, int *idList, int count, BOOL bCreateOnly );

typedef	BOOL (__stdcall *DAGetGroup )
	( HANDLE hInfo, int gId, int *idList, int count );

typedef	BOOL (__stdcall *DAAddToGroup )
	( HANDLE hInfo, int gId, int iCom );

typedef	BOOL (__stdcall *DARemoveFromGroup )
	( HANDLE hInfo, int gId, int iCom );

typedef BOOL (__stdcall *DADeleteGroup )
	( HANDLE hInfo, int gId );

typedef BOOL (__stdcall *DAGetGroupMembers )
	( HANDLE hInfo, int gId );

typedef BOOL (__stdcall *DAStartAudio )
	( HANDLE hInfo, int iCom, int type );

typedef BOOL (__stdcall *DASendAudio )
	( HANDLE hInfo, int iCom, char *data, int len );

typedef BOOL (__stdcall *DAEndAudio )
	( HANDLE hInfo, int iCom, BOOL immediately );

typedef BOOL (__stdcall *DAGetGPIOValues )
	( HANDLE hInfo, int iCom, unsigned *values );

typedef BOOL (__stdcall *DASetGPIOValue )
	( HANDLE hInfo, int iCom, int port, BOOL value );

typedef BOOL (__stdcall *DAiComCount )
	( HANDLE hInfo, int *count );
	
typedef BOOL (__stdcall *DAFirstIcom )
	( HANDLE hInfo, int *iCom, struct _iComStructure *p_Icom, int len );

typedef BOOL (__stdcall *DANextIcom )
	( HANDLE hInfo, int *iCom, struct _iComStructure *p_Icom, int len );

typedef BOOL (__stdcall *DAGetIcom )
	( HANDLE hInfo, int iCom, struct _iComStructure *p_Icom, int len );

typedef BOOL (__stdcall *DAOpenDoor )
	( HANDLE hInfo, int iCom, int seconds );

typedef int (__stdcall *DAGetVolume )
	( HANDLE hInfo, int iCom );

typedef BOOL (__stdcall *DASetVolume )
	( HANDLE hInfo, int iCom, int type, int volume );

typedef BOOL (__stdcall *DAForwardIcom )
	( HANDLE hInfo, int iCom, struct sockaddr_in *addr_in );

typedef BOOL (__stdcall *DARetrieveIcom )
	( HANDLE hInfo, int iCom );

typedef BOOL (__stdcall *DAListenIcom )
	( HANDLE hInfo, int iCom, short audioFormat, int onOff );

typedef int (__stdcall *DAGetMicType)
	( HANDLE hInfo, int iCom );

typedef BOOL (__stdcall *DADeleteServerCQ )
	( HANDLE hInfo, int iCom );

typedef BOOL (__stdcall *DATransferAudio )
	( HANDLE hInfo, int iCom );

typedef BOOL (__stdcall *DAFindIntercom )
	( HANDLE hInfo, struct _MAC *mac, BOOL bBroadcastOnly );

typedef BOOL (__stdcall *DASetIntercom )
	( HANDLE hInfo, int iCom, struct _configuration34b *pC34b, int len, BOOL bBroadcastOnly );

typedef int (__stdcall *DARingIntercom)
	( HANDLE hInfo, int iCom, BOOL bStartRing );

typedef BOOL (__stdcall *DAPlayTones )
	( HANDLE hInfo, int iCom, char *tones, int len );

typedef BOOL (__stdcall *DAResendSettings )
	( HANDLE hInfo );

typedef BOOL (__stdcall *DARefreshSettings )
	( HANDLE hInfo );

typedef int (__stdcall *DASetMulticastFeatures)
	( HANDLE hInfo, char *address, int TTL, unsigned featureBits );

typedef BOOL (__stdcall *DAGetServerTime )
	( HANDLE hInfo, int iCom );

typedef BOOL (__stdcall *DAResetConnection )
	( HANDLE hInfo, int iCom );

typedef int (__stdcall *DASayText)
	( HANDLE hInfo, int iCom, char *text );

typedef int (__stdcall *DAPlayServerAudio)
	( HANDLE hInfo, int iCom, char *fileKey, int type );

typedef int (__stdcall *DARTPConfig)
	( HANDLE hInfo, int mediaPort, BOOL bShared );

typedef int (__stdcall *DARTPIcomConfig)
	( HANDLE hInfo, int socket, int mediaPort, BOOL bSet );

typedef int (__stdcall *DARTPStart)
	( HANDLE hInfo, int iCom, int type, short localRecvPort, char *toAddress, short toPort, char *fromAddress, short fromPort, BOOL bRTCP, unsigned SSRC );

typedef int (__stdcall *DARTPStartMicTo)
	( HANDLE hInfo, int iCom, int type, short localSendPort, char *toAddress, short toPort );

typedef int (__stdcall *DARTPStartSpeakerFrom)
	( HANDLE hInfo, int iCom, int type, int localRecvPort, char *fromAddress, int fromPort );

typedef int (__stdcall *DARTPStop)
	( HANDLE hInfo, int iCom );

typedef int (__stdcall *DARTPStopMic)
	( HANDLE hInfo, int iCom );

typedef int (__stdcall *DARTPStopSpeaker)
	( HANDLE hInfo, int iCom );

#endif