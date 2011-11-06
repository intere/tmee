//
// TalkMasterConsoleDlg.h : header file
//
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
#ifdef __SKINS
#if !defined(AFX_SKINCONTROLSDLG_H__1B411B9D_1D57_4E6B_854C_0B07916080A9__INCLUDED_)
#define AFX_SKINCONTROLSDLG_H__1B411B9D_1D57_4E6B_854C_0B07916080A9__INCLUDED_
#endif
#endif

#pragma once
#include "afxwin.h"
#include "shellapi.h"
#include "DAPassThruDll.h"
#include "winsock2.h"
#include "afxcmn.h"

#include "DAAudioStuff.h"
#include "DAButton.h"
#include "DAAudioArchive.h"

#include "announceDlg.h"
#include "SendTextMessage.h"
#include <sys/stat.h>
#include "CameraDataManager.h"
#include "Mutex.h"

#include <objidl.h>
#include <gdiplus.h>
#include <string>
using namespace Gdiplus;
#pragma comment (lib,"Gdiplus.lib")

#define	USE_GLOBAL_MEMORY 0

#ifndef	APP_MAJOR						// Compiler overrides possible
#define	APP_MAJOR		1
#endif
#ifndef APP_MINOR
#define APP_MINOR		3
#endif
#ifndef APP_RELEASE
#define	APP_RELEASE		1
#endif

#define	COL_GROUP		0
#define COL_LOCATION	1
#define COL_QUEUE		2
#define COL_STATUS		3
#define COL_DOOR		4
#define COL_ADDRESS		5

#define	CQ_COL_STATUS	0
#define	CQ_COL_WAITING	1
#define	CQ_COL_DETAILS	2
#define	CQ_COL_TIME		3

#define	COL_MSG_PRIORITY	0
#define COL_MSG_TITLE		1

#define	GROUP_COL_CHECK		0
#define	GROUP_COL_ICON		1
#define	GROUP_COL_NAME		2

#define	GROUP_IMAGE_EMPTY	0
#define	GROUP_IMAGE_PARTIAL	1
#define	GROUP_IMAGE_FULL	2

#define	TIMER_WAIT			1
#define	TIMER_DOOR			2
#define	TIMER_CQPLAY		3
#define	TIMER_ANNOUNCE		4
#define	TIMER_ALERT			5
#define	TIMER_TALK			6
#define	TIMER_TEST			7
#define	TIMER_CHECK_LOGON	8
#define	TIMER_TEST_AUDIO	9
#define	TIMER_REPEAT		10
#define	TIMER_INIT_DONE		11
#define	TIMER_UPDATE_READY	12
#define	TIMER_WORK			13
#define	TIMER_RING			14

#define TALK_SELECTED	0
#define TALK_GROUP		1
#define	TALK_ALL_ACTIVE	2

#define	LISTEN_AUTO		0
#define	LISTEN_MANUAL	1

#define	WORK_DESELECT		10
#define	WORK_DELETE_GROUP	20

#define	MAX_LOGONLEN	100

struct _consoleData
{
	char fullName[MAX_LOGONLEN];
	char keyValue[MAX_LOGONLEN];
};

struct _itemData
{
	struct _itemData *next;

	int	socket;
	struct _iComStructure	iCom;
	struct _iComQueueInfo	iComQueue;
	struct _CQData			cq;

	BOOL	bLocalRecord;
	BOOL	bXfer;					// Currently transferring

	HANDLE	hTempCQ;				// Used when the call queue audio data is being received
	HANDLE	hCallQueue;
	HANDLE	hArchive;

	BOOL	bDisplayed;				// Is it already displayed?
	BOOL	bCounted;				// Has this intercom already been added into the list?

	BOOL	bSelected;				// have we currently selected this intercom?

	BOOL	bChecked;				// Is it currently checked?

	time_t	doorOpenTimer;			// When is the door time going to be complete?

	UCHAR	sensorState;			// What state is the sensor in?  Alarm='S', Open='s', non-active = 0

	BOOL	bUpdateStatus;			// Should the foreground update the intercom status?
};

struct _fileList
{
	struct _fileList *next;
	int	socket;
	char	fileName[MAX_PATH];
	FILE *fp;
	long	fileLen;
	long	filePos;
	BOOL	bRepeat;
	BOOL	bSelect;				// Should we select/deselect during this process?
};

struct _listData
{
	void **data;
	int dataSize;
};

void CallBack( int event, int socket, struct _iComStructure *pIcom, void *eventData, void *myData );
int findIntercom(struct _iComStructure *pIcom);




// CTalkMasterConsoleDlg dialog
class CTalkMasterConsoleDlg : public CDialog
{
// Construction
public:
	CTalkMasterConsoleDlg(CWnd* pParent = NULL);	// standard constructor
	~CTalkMasterConsoleDlg(/*=NULL*/);

	void outputDebug(LPCTSTR pFormat, ...);
	void writeDebug(LPCTSTR pFormat, ...);
	void WriteDebugString(char *pString);

	void	doLocalizedHelpSetup();
	void	doCancel();
	void WinHelp( DWORD dwData, UINT nCmd );

	

// Dialog Data
	enum { IDD = IDD_TALKMASTERCONSOLE_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual void OnCancel();

	void updateStatus();
	void clearDoor();
	BOOL LoadProcs();

	void startTalk(int socket);
	void startListen(int socket, BOOL skipTest=FALSE);
	void paintListen();

	BOOL waitClear(int socket, unsigned fromStatus);
	BOOL waitSet(int socket, unsigned toStatus);

	void initSystemTray();

	void forceForegroundWindow();

	void showMinimized(BOOL minimized);

	/** Draws the Preview Image.  */
	void drawPreview();

	/** Loads the JPEG Image.  */
	void loadImage(const std::string &jpeg);

	/** */
	void setImage(const std::string &jpeg);

	friend class VideoFeedThread;

	std::string jpeg;


public:
	void CTalkMasterConsoleDlg::OnHelp();

	void endTalk(int socket);
	void clearTalk();							// Clear downt he UI for the Talk stuff
	void endListen(int socket, BOOL skipWait=FALSE);
	void clearListen();							// Clear down the UI for the Listen stuff

	void clearFdx();							// Clear down everything for full duplex operations

	void DoEvents();

	void delSystemTray();
	void showSystemTray();

	BOOL CTalkMasterConsoleDlg::replaceMenuItem( CMenu *menu, int row, char *buffer, int ID );
	BOOL CTalkMasterConsoleDlg::insertMenuItem( CMenu *menu, int row, char *buffer, int ID );
	BOOL CTalkMasterConsoleDlg::removeMenuItem( CMenu *menu, int row );

	void CTalkMasterConsoleDlg::OpenCloseDebugFile(BOOL forceClose=FALSE);
	void	closeDebug();

	void showHdxFdxControls();

	CString getResourceString(int ID);

// Implementation
protected:
	HICON m_hIcon;

	void DrawTransparent(CBitmap *bitmap, CDC * pDC, int x, int y, COLORREF crColour);
	void CTalkMasterConsoleDlg::OnItemPostPaintLogo(NMHDR *pNMHDR, LRESULT *pResult);
	void CTalkMasterConsoleDlg::drawBitmaps();
	void CTalkMasterConsoleDlg::drawVUMeter( CStatic *meter, BOOL bActive, int level );
	void CTalkMasterConsoleDlg::readTransparent(CBitmap *bitmap, CDC * pDC);

	COLORREF	m_crWhite;
	COLORREF	m_crBlack;

	CMenu		m_rClickMenu;
	CMenu		m_rClickGroupMenu;

	CBitmap	mBitMapLogo;

	CBitmap mBitMapListenDisable;
	CBitmap mBitMapListenOff;
	CBitmap mBitMapListenOn;

	CBitmap	mBitMapNoListenOff;
	CBitmap mBitMapNoListenOn;

	CBitmap	mBitMapTalkSessionStart;						// Handsets
	CBitmap	mBitMapTalkSessionEnd;							// Handsets

	CBitmap mBitMapTalkDisable;
	CBitmap mBitMapTalkOff;
	CBitmap mBitMapTalkOn;

	CBitmap mBitMapFdxTalkOn;
	CBitmap mBitMapFdxTalkOff;
	CBitmap mBitMapFdxTalkDisable;

	CBitmap mBitMapFdxMute;
	CBitmap mBitMapFdxMuted;
	CBitmap mBitMapFdxMuteDisable;

	CBitmap mBitMapPlay;
	CBitmap mBitMapPause;
	CBitmap mBitMapStop;
	CBitmap mBitMapMuteOff;
	CBitmap mBitMapMuteOn;

	CBitmap	mBitMapDoorOpen;
	CBitmap	mBitMapDoorClosed;

	CBitmap mBitMapDoorLockedClosed;
	CBitmap mBitMapDoorLockedOpen;
	CBitmap mBitMapDoorUnlockedClosed;
	CBitmap mBitMapDoorUnlockedOpen;
	CBitmap mBitMapDoorNone;

	CBitmap mBitMapNormal;
	CBitmap	mBitMapOverflow;
	CBitmap	mBitMapPriority;
	CBitmap	mBitMapLocal;
	CBitmap	mBitMapHold;

	CBitmap	mBitMapBlank;
	CBitmap	mBitMapGreen;
	CBitmap mBitMapYellow;
	CBitmap mBitMapRed;

	CBitmap	mBitMapPriorityLow;
	CBitmap	mBitMapPriorityMedium;
	CBitmap	mBitMapPriorityHigh;

	CBitmap	mBitMapGroupEmpty;
	CBitmap mBitMapGroupPartial;
	CBitmap mBitMapGroupFull;

	CImageList	m_imageList;
	CImageList	m_imageListMsg;
	CImageList	m_imageListCQ;
	CImageList	m_imageListGroups;

	BOOL	bMuted;
	BOOL	bPlaying;

	BOOL	bOnce;

	BOOL	m_min;

	int		m_talkTimerGuard;
	int		m_listenTimerGuard;

	struct _listData callsWaiting;
	struct _listData icomList;

//	void	**m_CWItemData;
//	int		m_CWItemDataCount;

	NOTIFYICONDATA	m_nid;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	virtual void OnActivate(UINT nState,CWnd* pWndOther,BOOL bMinimized);
	virtual void OnShowWindow(BOOL bShow, UINT nStatus);

	virtual int MessageBox(LPCTSTR lpszText, LPCTSTR lpszCaption = 0, UINT nType = MB_OK);

	void activatePlayer( BOOL active );

	void playCallQueue();

	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnSize(UINT nType,int cx,int cy);
	afx_msg void OnWindowPosChanging(WINDOWPOS* lpwndpos);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);

	BOOL loadResourceDLL( LANGID );

	CString getAlternateHelpName();
	CString testAlternateHelpName( LANGID );

	CWnd m_cameraPreview;			/* Preview window. */
	Image* m_Thumbnail;				/* Thumbnail Image.  */
	bool imageLoading;
	Mutex mutex;
	int m_ThumbWidth;	// width of the thumbnail image
	int m_ThumbHeight;	// height of the thumbnail image.

	
	void updateAdminMenus(UINT flag);
	void updateMenus(UINT flag);

	void resetFocus();
	void setResetFocus(CListCtrl *list);

public:
	void loadAlternateResourceDLL();

	BOOL	m_bConsoleReady;

	BOOL	bSlow;									// Slow the handshake down for slower networks
	BOOL	bTalking;
	BOOL	bListening;
	BOOL	bMuting;								// Muting the microphone audio?
	BOOL	bNoRestart;
	BOOL	bSession;

	int		nTalkingLevel;
	int		nListeningLevel;

	BOOL	bUseUlaw;
	BOOL	bSetupVolume;

	int		m_threadPriority;

	char	consoleName[MAX_NAMELEN];

	int		m_nTalkCount;

	BOOL	bTalkRequest;							// I want it to be Talking
	BOOL	bListenRequest;							// I want it to be Listening
	BOOL	bReleaseRequest;						// Are we trying to release this intercom?
	BOOL	bSessionRequest;						// Are we trying to start a session with a handset intercom?  RINGING etc

	int		m_nReleaseSocket;						// What socket do we have to release?

	int		m_nIntercomsServer;						// Total # in the intercoms list
	int		m_nIntercomsServerAlive;				// Total Connected intercoms
	int		m_nIntercomsConsole;					// Displayed count
	int		m_nIntercomsConsoleAlive;				// Displayed count of active intercoms

	void OnAbout();
	void doLogon();
	void	doSize();

	afx_msg void OnKeyDown(UINT nChar,UINT nRepCnt,UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar,UINT nRepCnt,UINT nFlags);
protected:
	DECLARE_MESSAGE_MAP()

public:
	CListCtrl				m_listIcoms;
	CListCtrl				m_listCallsWaiting;
	CToolTipCtrl			m_listCallsWaitingToolTip;

	struct _iComQueueList	*m_pIcomQueueList;
	struct _itemData		*m_pItemData;
	struct _groupDataList	*m_pGroupData;
	struct _messageDataList	*m_pMessageDataList;

	struct _workData		*m_pWorkData;

	COLORREF				m_crPriority;
	
	BOOL			bOffDialog;
	BOOL			bSkipNextKeyUp;
	BOOL			bLoggedOn;
	BOOL			bShuttingDown;

	DAAudioStuff	audioPlayer;
	DAAudioArchive	audioArchive;

	DAAudioStuff	audioCQPlayer;

	struct _fileList m_fileList;

	BOOL	resetList;
	BOOL	resetCQList;
	CRITICAL_SECTION CriticalSection; 
	CRITICAL_SECTION CriticalSectionCallQueue; 
	CRITICAL_SECTION CriticalSectionDebug; 
	CRITICAL_SECTION CriticalSectionWork;

	int		m_sortBy;
	BOOL	m_bOrder;

	int		m_sortMessagesBy;
	BOOL	m_bOrderMessagesBy;

	int		m_selectedRow;
	int		m_selectedCQRow;

	int		m_firstGroupSelected;	// Used in the mouse click m_listGroups routine to remember what the last single click was that may have started a multi-select

	int		m_sessionSocket;		// Which intercom are we in a session with?
	int		m_oldSessionSocket;		// Which one were we talking to before the session was interrupted (Group?)
	int		m_holdSessionSocket;
	BOOL	m_bHeldSessionSocket;
	BOOL	m_sessionDead;			// We are waiting for the session to die

	int m_TalkMode;
	int m_ListenMode;
	int m_ListenModeHold;

	int	m_mainMode;						// The tab holder for the tab 0 mode

	int m_serverTimeDelta;				// Now many seconds are we off from the server time?

	struct _itemData *m_lastSelectedItem;
	struct _itemData *m_pSelectedItem;
	struct _itemData *m_pReleasedItem;
	//VideoFeedThread* thread;

	struct _messageData *m_pCurrentMessage;

	HMODULE m_hLib;
	CString	m_csLibVer;
	struct sockaddr_in m_sockaddr_in;

	int	m_DAMajor, m_DAMinor, m_DARelease;

	DAOpen m_DAOpen;
	DAClose m_DAClose;
	DAStart m_DAStart;
	DAStop m_DAStop;
	DADllVersion m_DADllVersion;
	DASetDebug m_DASetDebug;

	DADisconnect		m_DADisconnect;
	DACreateGroup		m_DACreateGroup;
	DADeleteGroup		m_DADeleteGroup;
	DAGetGroupMembers	m_DAGetGroupMembers;
	DASelectIntercom	m_DASelectIntercom;
	DAStartAudio		m_DAStartAudio;
	DASendAudio			m_DASendAudio;
	DAEndAudio			m_DAEndAudio;
	DAPlayServerAudio	m_DAPlayServerAudio;
	DAGetGPIOValues		m_DAGetGPIOValues;
	DASetGPIOValue		m_DASetGPIOValue;
	DAiComCount			m_DAiComCount;
	DAFirstIcom			m_DAFirstIcom;
	DANextIcom			m_DANextIcom;
	DAGetIcom			m_DAGetIcom;
	DAOpenDoor			m_DAOpenDoor;
	DAGetVolume			m_DAGetVolume;
	DASetVolume			m_DASetVolume;
	DAForwardIcom		m_DAForwardIcom;
	DARetrieveIcom		m_DARetrieveIcom;
	DAListenIcom		m_DAListenIcom;
	DADeleteServerCQ	m_DADeleteServerCQ;
	DATransferAudio		m_DATransferAudio;
	DAResetConnection	m_DAResetConnection;
	DASayText			m_DASayText;
	DAResendSettings	m_DAResendSettings;
	DARingIntercom		m_DARingIntercom;

	HANDLE m_hDA;		// Handle to the Logon Dialog.
	UINT m_uFlags;

	CString m_csLoginName, m_csLoginPassword;

	char m_UserData[10];

	void shutDownIcomAndHold();
	void CTalkMasterConsoleDlg::displayIntercomCount();
	void CTalkMasterConsoleDlg::redisplayIcomList();
	BOOL CTalkMasterConsoleDlg::testIcomList();

	afx_msg void loadIcomList(NMHDR *pNMHDR);

	afx_msg void OnHdnItemclickListIcoms(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickListIcoms(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnColumnclickListIcoms(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMRclickListIcoms(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnDeleteitemListIcoms(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnDeleteitemListCallsWaiting(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBtnDnButtonListen();
	afx_msg void OnBtnUpButtonListen();
	afx_msg void OnBtnDnButtonNoListen();
	afx_msg void OnBtnUpButtonNoListen();
	afx_msg void OnBtnDnButtonTalk();
	afx_msg void OnBtnUpButtonTalk();
	void CTalkMasterConsoleDlg::OnBnDisableTalk();
	void CTalkMasterConsoleDlg::OnBnDisableListen();

	afx_msg LRESULT OnSystemTray(WPARAM wParam, LPARAM lParam);

	afx_msg void OnBnClickedRadioGroup();
	afx_msg void OnBnClickedRadioSelected();
	afx_msg void OnBnClickedRadioAllActive();
	afx_msg void OnBnClickedRadioAutomatic();
	afx_msg void OnBnClickedRadioManual();
	CStatic m_groupboxListenMode;
	CStatic m_groupboxTalkMode;
//	afx_msg void OnBnClickedButtonListen();
//	afx_msg void OnBnClickedButtonTalk();
	afx_msg void doListen();
	afx_msg void doTalk();
	afx_msg void doRelease();
	afx_msg void doSessionStartStop();

	afx_msg void doOpenDoor(struct _itemData *pItem);
	afx_msg void doAudioData( int socket, struct _AudioData *ad );
	afx_msg void doneAudioData( int socket );

	void CTalkMasterConsoleDlg::insertItemData(struct _listData *list, int index, void *data);
	void CTalkMasterConsoleDlg::deleteItemData(struct _listData *list, int index);
	void *CTalkMasterConsoleDlg::getItemData(struct _listData *list, int index);
	void CTalkMasterConsoleDlg::setItemData(struct _listData *list, int index, void *data);
	void CTalkMasterConsoleDlg::clearItemData(struct _listData *list);

	int	CTalkMasterConsoleDlg::getCallsWaitingItemCount();
	void CTalkMasterConsoleDlg::insertCallsWaitingItemData(int index, void *data);
	void CTalkMasterConsoleDlg::deleteCallsWaitingItemData(int index);
	void *CTalkMasterConsoleDlg::getCallsWaitingItemData(int index);
	void CTalkMasterConsoleDlg::setCallsWaitingItemData(int index, void *data);
	void CTalkMasterConsoleDlg::clearCallsWaitingItemData();

	void CTalkMasterConsoleDlg::insertIcomItemData(int index, void *data);
	void CTalkMasterConsoleDlg::deleteIcomItemData(int index);
	void *CTalkMasterConsoleDlg::getIcomItemData(int index);
	void CTalkMasterConsoleDlg::setIcomItemData(int index, void *data);
	void CTalkMasterConsoleDlg::clearIcomItemData();

	void CTalkMasterConsoleDlg::lockControls(BOOL talk, BOOL listen, BOOL chime, BOOL playFile, BOOL radio, BOOL testVolume, BOOL volumeControls, BOOL groupControls);

	char *CTalkMasterConsoleDlg::szStatus( unsigned statusCode, char *szRetBuffer );

	void CTalkMasterConsoleDlg::doTalkTimer();
	void doTimerTest();

	void CTalkMasterConsoleDlg::doNewIntercom(int socket, struct _iComStructure *pIcom, struct _iComQueueInfo *pQueueInfo);

	afx_msg BOOL sendAudioFile( int socket, char *fname, BOOL bSelectIcom, BOOL bRepeatFile );
	afx_msg void doAudioRequest( int socket, int dataLen );
	struct _fileList *getFileList(int socket);
	BOOL delFileList(int socket);
	void stopAudioFile( int socket );
	void doneAudioFile( int socket );

	BOOL playServerFile( int socket, char *fileKey );

	void CTalkMasterConsoleDlg::initCallQueue();

	BOOL CTalkMasterConsoleDlg::newCallQueueMembership(struct _iComQueueInfo *pQueueMembership);
	BOOL CTalkMasterConsoleDlg::updateCallQueueStatus(struct _iComQueueInfo *pQueueMembership);

	struct _iComQueueList *CTalkMasterConsoleDlg::findQueueMembership(struct _iComQueueInfo *pQueueMembership);

	BOOL StartLocalCQSession(int m_sessionSocket, BOOL bHold);

	int	 readWaveHeader(FILE *fp, WAVEFORMATEX *waveHeader);

	void CTalkMasterConsoleDlg::updateListIcomsStatus(struct _iComStructure *pIcom, int index);

	int CTalkMasterConsoleDlg::getInsert(struct _itemData *item);

	void doNewGroupInfo( struct _groupData *pGroupInfo );
	void doNewGroupInfo(struct _itemData *pItemData);			// Either can be on the group tab

	void doNewGroupMember(int GroupSocket, struct _MAC *pMac);
	void doNewMessageInfo( struct _messageData *pMessageData );
	void doMessagePlaying( struct _messagePlayingData *pMessagePlayingData );

	int makeGroupSocket();
	int makeGroupSocketIcoms();				// Make a group of sockets from the intercom page
	int makeGroupSocketGroups();			// Make a group of sockets from the groups page

	int countGroup();
	int countIcomGroup();					// Local Group
	int countServerGroup();					// Count the members selected on the server based group tab

	void deleteIcomsData();					// Delete the icoms list and the data

	void deleteIcoms(BOOL bClearCount);		// Delete the intercom list data before rebuilding or exitting
	void resetDialog();

	void deleteGroups();

	void deleteMessages();

	void selectIcomRow(int row);

	struct _itemData *getItemDataFromSocket(int socket);
	struct _itemData *getCQItemDataFromSocket(int socket);
	int findCQIndexFromSocket(int socket);
	int findIndexFromSocket(int socket);

	struct _itemData *findItemData(int socket);
	struct _itemData *findItemData(struct _MAC *MAC);

	struct _groupDataList *getGroupEntry(char *key);
	struct _groupDataList *getGroupEntry(struct _MAC *MAC);

	struct _messageDataList *getMessageEntry(struct _messageData *pMessageData);
	struct _messageDataList *getMessageEntry(struct _groupDataList *pGroupData, struct _messageDataList *pMessageData);

	int getMessageSelectedCount(struct _messageDataList *pMessageData);

	int getCQInsert(struct _itemData *item);
	int selectCQRow(int row, BOOL bSetItem=FALSE);
	time_t oldestCQEntry(int *row=NULL, time_t *time=NULL);

	int findGroupInsert(struct _groupData *pGroup);
	int findGroupInsert(struct _itemData *pIntercom);

	int findGroupRow(struct _groupData *pGroup);
	int findGroupRow(struct _itemData *pItemData);

	int findMessageInsert(struct _messageData *pMessageData);
	int findMessageRow(struct _messageData *pMessageData);

	struct _itemData *CTalkMasterConsoleDlg::getCallsWaitingData(int row);

	BOOL isCallQueue(int socket);
	void MarkCallQueueSession(int socket, BOOL bHold);

	void	addCallQueue(int socket, struct _CQData *cq);
	void	updateCallQueue(int socket, struct _CQData *cq);
	void	deleteCallQueue(int socket, BOOL bForceDelete, BOOL bReselectListBox);
	void	deleteAllCallQueue(BOOL bForceDelete);

	void doCallQueueAudio(int socket, struct _AudioData *cq);
	void doCallQueueAudioDone(int socket);

	void	serviceMicAudio(char *data, int len);

	BOOL selectSocket( int socket );
	BOOL deselectSocket( int socket );
	int getIntercomSocket( int index );

	CButton m_btnAutomatic;
	CButton m_btnManual;
	CButton m_btnSelected;
	CButton m_btnGroup;
	CButton m_btnAllActive;
	CButton m_btnChime;
	CButton m_btnPlayFile;
	DAButton m_btnTalk;
	DAButton m_btnListen;
	DAButton m_btnNoListen;

	int	m_playFileSocket;
	BOOL bPlayFileLocal;

	int m_repeatSocket;
	CString m_csRepeatFileName;
	struct _itemData *m_pRepeatItemData;

	HHOOK m_hookKb;

	afx_msg void OnLvnKeydownListIcoms(NMHDR *pNMHDR, LRESULT *pResult);

	CButton m_btnPlayPause;
	CButton m_btnStop;
	CButton m_btnMute;
	CSliderCtrl m_sliderCQPlayer;
	struct _itemData *m_pCallWaitingItem;

	afx_msg void OnLvnItemActivateListCallsWaiting(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickListCallsWaiting(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMRclickListCallsWaiting(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonPlayPause();
	afx_msg void OnBnClickedButtonStop();
	afx_msg void OnBnClickedButtonMute();
	afx_msg void OnNMCustomdrawSliderCqPlayer(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMKillfocusListCallsWaiting(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMSetfocusListCallsWaiting(NMHDR *pNMHDR, LRESULT *pResult);

	CStatic m_groupboxPlayer;
	afx_msg void OnLvnItemchangedListCallsWaiting(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonPlayFile();
#ifndef DA_CALLBACK
	void OnPrintString2(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnPrintString(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnNewConnection(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnRecvData(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnEventError(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnAudioLowMark(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnAudioHighMark(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnAudioUnderflow(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnAudioDone(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDoneTalking(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnReconnect(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnIntercomKeyPress(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnGPIOstateChange(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnIntercomListening(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnIntercomTalking(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnIntercomDisconnect(WPARAM wParam, LPARAM lParam);
#endif
	afx_msg void OnLvnItemchangedListIcoms(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonChime();

	CButton m_btnSessionEnd;
	CButton m_btnSessionHold;

	BOOL	bHoldHold;
	BOOL	bHoldEnd;

	void SetHoldEnd(BOOL bHold, BOOL bEnd);
	void resetHoldEnd();								// If we are on the main screen, set it to the real value of hold and end

	afx_msg void OnBnClickedButtonEnd();
	afx_msg void OnBnClickedButtonHold();
	afx_msg void OnLvnDeleteallitemsListIcoms(NMHDR *pNMHDR, LRESULT *pResult);

	void doBnClickedButtonEnd(BOOL bSkipDeselect=FALSE);

	CAnnounceDlg announceDlg;
	CSendTextMessageDlg sendTextDlg;

	char soundFile[2][MAX_PATH];

	void initAnnounce();
	void announceCheck();
	BOOL announceTest();
	void announceActivity();
	void resetAnnounce();
	void stopAnnounce();
	void startAlert();
	void stopAlert();
	void doAudibleAlert();
	void doVisualAlert();
	void doAlerts();

	time_t	m_announceStartTime;
	int		m_announceAgeTrigger;
	int		m_announceSizeTrigger;
	time_t	m_announceGuardTime;
	BOOL	m_announceRunning;

	CStatic m_staticLogo;
	CStatic m_staticVUMeterMicrophone;
	CStatic m_staticVUMeterSpeaker;
	afx_msg void OnLvnItemchangingListIcoms(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnItemchangingListCallsWaiting(NMHDR *pNMHDR, LRESULT *pResult);
	CStatic m_staticStatusConnected;
	CStatic m_staticStatusPTT;
	afx_msg void OnHdnEndtrackListIcoms(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMSetfocusListIcoms(NMHDR *pNMHDR, LRESULT *pResult);
	CStatic m_staticCodec;

	CListCtrl	*m_pListFocus;
	BOOL m_bTestMode;
	afx_msg void OnBnClickedCheckTest();
	void clearTestMode();
	int m_maxMsec;
	CSliderCtrl m_ctrlMaxMsec;
	afx_msg void OnNMReleasedcaptureSliderMaxMsec(NMHDR *pNMHDR, LRESULT *pResult);
	CString m_csMaxMsec;
	CStatic m_cstaticMaxMsec;
	CButton m_btnTestMode;
	CStatic m_staticMinMsecText;
	CButton m_btnSetVolume;
	CButton m_btnGetVolume;
	CSliderCtrl m_ctrlSetupVolume;
	CProgressCtrl m_ctrlVolumeSetting;
	int m_nSetupVolume;
	afx_msg void OnBnClickedButtonGetVolume();
	afx_msg void OnNMReleasedcaptureSliderSetupVolume(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonSetVolume();
	CButton m_btnTestVolume;
	afx_msg void OnBnClickedButtonTestVolume();
	CButton m_btnFdxMute;
	DAButton m_btnFdxStart;
	afx_msg void doFdxStartStop();
	afx_msg void OnBtnDnButtonFdxStart();
	afx_msg void OnBtnUpButtonFdxStart();
	afx_msg void OnBnClickedButtonFdxMute();
	afx_msg void OnInvisibleResetintercomconnection();
	afx_msg void OnConfigureCamera();
	CButton m_btnTestAudio;
	BOOL	m_bTestAudio;
	afx_msg void OnBnClickedButtonTestAudio();
	void doTestAudio();
	void doRepeatAudio();
	void doPostInit();
	void doGetSettingsUpdate();
	void releaseCurrentSettings();

	void CTalkMasterConsoleDlg::updateGroupIcons();

	void addWork(int socket, int command, time_t when);
	void doWork();
	void doWorkCommand(int socket, int command);

	CString m_csTestAudioFile;
	int m_testAudioSocket;
	int	m_threadControl;
	BOOL	m_bAbortControl;
	CTabCtrl m_tabMain;
	afx_msg void OnTcnSelchangeTabMain(NMHDR *pNMHDR, LRESULT *pResult);
	CListCtrl m_listGroups;
	afx_msg void OnNMClickListGroups(NMHDR *pNMHDR, LRESULT *pResult);
	
	int CTalkMasterConsoleDlg::getCheckedGroupsCount();
	int CTalkMasterConsoleDlg::getFirstCheckedGroupIndex();
	int CTalkMasterConsoleDlg::getNextCheckedGroupIndex(int lastIndex);

	struct _groupDataList *CTalkMasterConsoleDlg::getGroupList(struct _groupData *pGroupData);
	struct _groupDataList *CTalkMasterConsoleDlg::getGroupList(int GroupSocket);

	int CTalkMasterConsoleDlg::getListIcomsCount(struct _groupDataList *pGroupList);
	int CTalkMasterConsoleDlg::getListIcoms(struct _groupDataList *pGroupList);

	void CTalkMasterConsoleDlg::refreshListIcoms(struct _groupDataList *pGroupList);
	afx_msg void OnToolsSendtextmessage();

	void displayTabMain();
	void playFileLocal();
	void playFileServer();

	CListCtrl m_listMessages;
	afx_msg void OnNMClickListMessages(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMRclickListGroups(NMHDR *pNMHDR, LRESULT *pResult);

	BOOL m_bSaveEnd, m_bSaveHold;

#ifdef __SKINS
	CString m_strStylesPath;
#endif

	afx_msg void OnLvnColumnclickListMessages(NMHDR *pNMHDR, LRESULT *pResult);
	CButton m_btnTalkSessionStart;
	CButton m_btnTalkSessionEnd;
	afx_msg void OnBnClickedButtonSessionStart();
	afx_msg void OnBnClickedButtonSessionEnd();
	void doRing(BOOL bStart);
	afx_msg void OnToolsTestconsoledll();

	// for GDI
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	void showCameraStream();

	// For Multithreading:
	CWinThread* threadHandle;
	void doRender();
	void stopRender();
	static UINT run(LPVOID p);
	void run();
	volatile BOOL running;
};

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
int CALLBACK sortIcoms(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

int CALLBACK sortMessages(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

BOOL icomInGroup( int socket, int *groupSocket, int count );

extern int micAudioSize;