//
// CTalkMasterConsoleDlg.cpp : implementation file
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

#include "stdafx.h"
#include "TalkMasterConsole.h"
#include "TalkMasterConsoleDlg.h"
#include "DAPassThruDll.h"
#include "direct.h"
#include "DALoginDlg.h"
#include "afxwin.h"
#include ".\talkmasterconsoledlg.h"
#include "AnnounceDlg.h"
#include "SendTextMessage.h"
#include "TestLogonDlg.h"

#include "utility.h"
#include "windows.h"
#include "ControlThread.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
public:
	CListBox m_listAboutComponents;
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_ABOUT_COMPONENTS, m_listAboutComponents);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

BOOL CAboutDlg::OnInitDialog()
{
	CTalkMasterConsoleDlg *dlg = (CTalkMasterConsoleDlg*)theApp.m_pMainWnd;
	char buf[128];

	CDialog::OnInitDialog();

#ifdef PRIMEX_BUILD
	SetWindowText( dlg->getResourceString(IDS_STRING155) );
#endif

#ifdef PRIMEX_BUILD
	sprintf(buf, _T(dlg->getResourceString(IDS_STRING156)), getFileVersionString().GetBuffer());
#else
	sprintf(buf, _T(dlg->getResourceString(IDS_STRING157)), getFileVersionString().GetBuffer());
#endif

	m_listAboutComponents.InsertString(-1, buf);
#ifdef PRIMEX_BUILD
	sprintf(buf, _T(dlg->getResourceString(IDS_STRING158)), dlg->m_DAMajor, dlg->m_DAMinor, dlg->m_DARelease);
#else
	sprintf(buf, _T(dlg->getResourceString(IDS_STRING159)), dlg->m_DAMajor, dlg->m_DAMinor, dlg->m_DARelease);
#endif
	m_listAboutComponents.InsertString(-1, buf);
	
#ifdef PRIMEX_BUILD
	sprintf(buf, _T(dlg->getResourceString(IDS_STRING160)), dlg->m_csLibVer);
#else
	sprintf(buf, _T(dlg->getResourceString(IDS_STRING161)), dlg->m_csLibVer);
#endif
	m_listAboutComponents.InsertString(-1, buf);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// CTalkMasterConsoleDlg dialog



CTalkMasterConsoleDlg::CTalkMasterConsoleDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTalkMasterConsoleDlg::IDD, pParent)
	, m_bTestMode(FALSE)
	, m_maxMsec(0)
	, m_csMaxMsec(_T(""))
	, m_nSetupVolume(0)
	, m_csLoginName(_T(""))
	, m_csLoginPassword(_T(""))
	, m_bTestAudio(0)
	, m_threadControl(0)
	, m_bAbortControl(FALSE)
{
#ifdef	PRIMEX_BUILD
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
#else
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
#endif

#ifdef	__SKINS
// Code Jock Code

	TCHAR szStylesPath[_MAX_PATH];

	VERIFY(::GetModuleFileName(
		AfxGetApp()->m_hInstance, szStylesPath, _MAX_PATH));		
	
	m_strStylesPath = szStylesPath;
	int nIndex  = m_strStylesPath.ReverseFind(_T('\\'));
	if (nIndex > 0) {
		m_strStylesPath = m_strStylesPath.Left(nIndex);
	}
	else {
		m_strStylesPath.Empty();
	}
	m_strStylesPath += _T("\\Styles\\");

	XTPSkinManager()->SetApplyOptions(XTPSkinManager()->GetApplyOptions() | xtpSkinApplyMetrics);
//	XTPSkinManager()->LoadSkin(m_strStylesPath + _T("Vista.cjstyles"), _T("NormalBlack2.ini"));	
	XTPSkinManager()->LoadSkin(m_strStylesPath + _T("Office2007.cjstyles"));	

//	MessageBox(m_strStylesPath, "Styles");
//	XTPSkinManager()->LoadSkin(m_strStylesPath + _T("Vista.cjstyles"));	

// end code jock code
#endif	// __SKINS

	strcpy( m_UserData, _T("Jon") );

	InitializeCriticalSection(&CriticalSection);
	InitializeCriticalSection(&CriticalSectionCallQueue);
	InitializeCriticalSection(&CriticalSectionDebug);
	InitializeCriticalSection(&CriticalSectionWork);

	m_DAMajor = 0;
	m_DAMinor = 0;
	m_DARelease = 0;

	m_uFlags = 0;					// Default this to ZERO so we don't do something strange in the DAOpen call

	m_DAOpen = NULL;
	m_DAClose = NULL;
	m_DAStart = NULL;
	m_DAStop = NULL;
	m_DADllVersion = NULL;
	m_DASetDebug = NULL;

	m_selectedRow = -1;				// None of the list is selected
	m_selectedCQRow = -1;			// None of the call queue list is selected (yet)

	m_sessionSocket		= 0;		// Not in a session with an intercom
	m_oldSessionSocket	= 0;		// Not an old session held yet either
	m_sessionDead		= FALSE;	// Session is not dead yet

	m_sortBy = 1;					// sort by name by default
	m_bOrder = 0;

	m_sortMessagesBy = 0;
	m_bOrderMessagesBy = FALSE;

	m_pSelectedItem = NULL;			// No item selected yet
	m_pReleasedItem = NULL;			// No item to be released yet

	m_mainMode = TALK_SELECTED;
	m_ListenMode = 0;
	m_TalkMode = 0;

	bTalking = FALSE;
	bListening = FALSE;
	bMuting = FALSE;
	bNoRestart	= FALSE;			// Should we restart listen?
	bSlow		= FALSE;			// Assume a fast network
	bSession	= FALSE;			// Not in a session

	m_bConsoleReady = FALSE;		// We have not connected yet

	nTalkingLevel = 0;
	nListeningLevel = 0;

	m_nTalkCount = 0;
	bTalkRequest = FALSE;			// Init to Talk has not been requested
	bListenRequest = FALSE;			// Init to Listen has not been requested
	bReleaseRequest = FALSE;		// Init to Release has not been requested
	bSessionRequest = FALSE;		// Init to Session has not been requested

	m_nReleaseSocket = 0;

	m_nIntercomsServer = 0;
	m_nIntercomsServerAlive = 0;
	m_nIntercomsConsole = 0;
	m_nIntercomsConsoleAlive = 0;

	bPlaying = FALSE;				// Not playing anything from the CQ list
	bMuted = FALSE;					// Not muted yet

	bSetupVolume = FALSE;

	bOnce = FALSE;

	bOffDialog = FALSE;
	bSkipNextKeyUp = FALSE;

	bLoggedOn = FALSE;				// Not logged on yet
	bShuttingDown = FALSE;			// Not shutting down

	resetList = TRUE;
	resetCQList = TRUE;

	m_pIcomQueueList	= NULL;
	m_pCallWaitingItem	= NULL;
	m_pItemData			= NULL;
	m_pGroupData		= NULL;
	m_pMessageDataList	= NULL;
	m_pWorkData			= NULL;

	m_pListFocus		= &m_listCallsWaiting;				// By default this list gets focus
//	m_listCallsWaiting.SetFocus();

	m_crPriority = RGB(255, 0, 0);
	m_crWhite = RGB(255,255,255);
	m_crBlack = RGB(0,0,0);

	m_fileList.fp = NULL;
	m_fileList.next = NULL;
	m_fileList.socket = 0;

	m_playFileSocket = 0;
	bPlayFileLocal = TRUE;

	m_announceStartTime = 0;
	m_announceAgeTrigger = 0;
	m_announceSizeTrigger = 0;
	m_announceGuardTime = 0;
	m_announceRunning	= FALSE;

	m_serverTimeDelta = 0;					// No time difference before we get the server time event

	m_ListenModeHold = -1;					// Not set yet, 0 - automatic, 1 - manual

	m_talkTimerGuard = 0;
	m_listenTimerGuard = 0;

//	m_CWItemDataCount = 0;
//	m_CWItemData = NULL;

	callsWaiting.data = NULL;
	callsWaiting.dataSize = 0;

	icomList.data = NULL;
	icomList.dataSize = 0;

	strcpy(consoleName, getResourceString(IDS_STRING_NAME_NOT_SET));

	m_bHeldSessionSocket	= FALSE;		// Nothing held

	m_min = FALSE;
}

CTalkMasterConsoleDlg::~CTalkMasterConsoleDlg(/*=NULL*/)
{
	struct _iComQueueList *list = m_pIcomQueueList;
	struct _itemData *item = m_pItemData;

	m_bAbortControl = TRUE;
	while(m_threadControl)
		Sleep(0);

	if( m_hLib )
		FreeLibrary(m_hLib);

	while(list)
	{
		m_pIcomQueueList = list->next;
		free(list);
		list = m_pIcomQueueList;
	}

	while(item)
	{
		m_pItemData = item->next;
		free(item);
		item = m_pItemData;
	}

	if( callsWaiting.data )
	{
#if USE_GLOBAL_MEMORY
		GlobalUnlock(callsWaiting.data);
		GlobalFree(callsWaiting.data);
#else
		free(callsWaiting.data);
#endif
		callsWaiting.data = NULL;
		callsWaiting.dataSize = 0;
	}

	if( icomList.data )
	{
#if USE_GLOBAL_MEMORY
		GlobalUnlock(icomList.data);
		GlobalFree(icomList.data);
#else
		free(icomList.data);
#endif
		icomList.data = NULL;
		icomList.dataSize = 0;
	}

	audioPlayer.Close();
	audioCQPlayer.Close();

	DeleteCriticalSection(&CriticalSection);
	DeleteCriticalSection(&CriticalSectionCallQueue);
	DeleteCriticalSection(&CriticalSectionDebug);
	DeleteCriticalSection(&CriticalSectionWork);

	OutputDebugString(_T("~CTalkMasterConsoleDlg\n"));
}

void CTalkMasterConsoleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_ICOMS, m_listIcoms);
	DDX_Control(pDX, IDC_LIST_CALLS_WAITING, m_listCallsWaiting);
	DDX_Control(pDX, IDC_RADIO_AUTOMATIC, m_btnAutomatic);
	DDX_Control(pDX, IDC_RADIO_MANUAL, m_btnManual);
	DDX_Control(pDX, IDC_RADIO_SELECTED, m_btnSelected);
	DDX_Control(pDX, IDC_RADIO_GROUP, m_btnGroup);
	DDX_Control(pDX, IDC_RADIO_ALL_ACTIVE, m_btnAllActive);
	DDX_Control(pDX, IDC_BUTTON_CHIME, m_btnChime);
	DDX_Control(pDX, IDC_BUTTON_PLAY_FILE, m_btnPlayFile);
	DDX_Control(pDX, IDC_BUTTON_TALK, m_btnTalk);
	DDX_Control(pDX, IDC_BUTTON_LISTEN, m_btnListen);
	DDX_Control(pDX, IDC_BUTTON_NO_LISTEN, m_btnNoListen);
	DDX_Control(pDX, IDC_GROUP_LISTEN_MODE, m_groupboxListenMode);
	DDX_Control(pDX, IDC_GROUP_TALK_MODE, m_groupboxTalkMode);

	DDX_Control(pDX, IDC_BUTTON_PLAY_PAUSE, m_btnPlayPause);
	DDX_Control(pDX, IDC_BUTTON_STOP, m_btnStop);
	DDX_Control(pDX, IDC_BUTTON_MUTE, m_btnMute);
	DDX_Control(pDX, IDC_SLIDER_CQ_PLAYER, m_sliderCQPlayer);
	DDX_Control(pDX, IDC_GROUP_PLAYER, m_groupboxPlayer);
	DDX_Control(pDX, IDC_BUTTON_END, m_btnSessionEnd);
	DDX_Control(pDX, IDC_BUTTON_HOLD, m_btnSessionHold);
	DDX_Control(pDX, IDC_STATIC_LOGO, m_staticLogo);
	DDX_Control(pDX, IDC_STATIC_VU_METER_MICROPHONE, m_staticVUMeterMicrophone);
	DDX_Control(pDX, IDC_STATIC_VU_METER_SPEAKER, m_staticVUMeterSpeaker);
	DDX_Control(pDX, IDC_STATIC_STATUS_CONNECTED, m_staticStatusConnected);
	DDX_Control(pDX, IDC_STATIC_STATUS_PTT, m_staticStatusPTT);
	DDX_Control(pDX, IDC_STATIC_CODEC, m_staticCodec);
	DDX_Check(pDX, IDC_CHECK_TEST, m_bTestMode);
	DDX_Slider(pDX, IDC_SLIDER_MAX_MSEC, m_maxMsec);
	DDX_Control(pDX, IDC_SLIDER_MAX_MSEC, m_ctrlMaxMsec);
	DDX_Text(pDX, IDC_STATIC_MAX_MSEC, m_csMaxMsec);
	DDX_Control(pDX, IDC_STATIC_MAX_MSEC, m_cstaticMaxMsec);
	DDX_Control(pDX, IDC_CHECK_TEST, m_btnTestMode);
	DDX_Control(pDX, IDC_STATIC_MIN_MSEC_TEXT, m_staticMinMsecText);
	DDX_Control(pDX, IDC_BUTTON_SET_VOLUME, m_btnSetVolume);
	DDX_Control(pDX, IDC_BUTTON_GET_VOLUME, m_btnGetVolume);
	DDX_Control(pDX, IDC_SLIDER_SETUP_VOLUME, m_ctrlSetupVolume);
	DDX_Control(pDX, IDC_PROGRESS_SETUP_VOLUME, m_ctrlVolumeSetting);
	DDX_Slider(pDX, IDC_SLIDER_SETUP_VOLUME, m_nSetupVolume);
	DDX_Control(pDX, IDC_BUTTON_TEST_VOLUME, m_btnTestVolume);
	DDX_Control(pDX, IDC_BUTTON_FDX_MUTE, m_btnFdxMute);
	DDX_Control(pDX, IDC_BUTTON_FDX_START, m_btnFdxStart);
	DDX_Control(pDX, IDC_BUTTON_TEST_AUDIO, m_btnTestAudio);
	DDX_Control(pDX, IDC_TAB_MAIN, m_tabMain);
	DDX_Control(pDX, IDC_LIST_GROUPS, m_listGroups);
	DDX_Control(pDX, IDC_LIST_MESSAGES, m_listMessages);
	DDX_Control(pDX, IDC_BUTTON_SESSION_START, m_btnTalkSessionStart);
	DDX_Control(pDX, IDC_BUTTON_SESSION_END, m_btnTalkSessionEnd);
}

BEGIN_MESSAGE_MAP(CTalkMasterConsoleDlg, CDialog)
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
//	ON_WM_ACTIVATE()
//	ON_WM_ENTERIDLE()
//	ON_WM_SHOWWINDOW()
//}}AFX_MSG_MAP
	ON_COMMAND(ID_HELP, OnHelp)

	ON_MESSAGE(WM_USER+0x1000, OnSystemTray)

//	ON_NOTIFY(HDN_ITEMCLICK, 0, OnHdnItemclickListIcoms)
	ON_NOTIFY(NM_CLICK, IDC_LIST_ICOMS, OnNMClickListIcoms)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST_ICOMS, OnLvnColumnclickListIcoms)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_ICOMS, OnNMRclickListIcoms)
	ON_NOTIFY(LVN_DELETEITEM, IDC_LIST_ICOMS, OnLvnDeleteitemListIcoms)
	ON_CONTROL(WM_LBUTTONDOWN, IDC_BUTTON_LISTEN, OnBtnDnButtonListen)
	ON_CONTROL(WM_LBUTTONUP, IDC_BUTTON_LISTEN, OnBtnUpButtonListen)
	ON_CONTROL(WM_LBUTTONDOWN, IDC_BUTTON_NO_LISTEN, OnBtnDnButtonNoListen)
	ON_CONTROL(WM_LBUTTONUP, IDC_BUTTON_NO_LISTEN, OnBtnUpButtonNoListen)
	ON_CONTROL(WM_LBUTTONDOWN, IDC_BUTTON_TALK, OnBtnDnButtonTalk)
	ON_CONTROL(WM_LBUTTONUP, IDC_BUTTON_TALK, OnBtnUpButtonTalk)
	ON_CONTROL(WM_LBUTTONDOWN, IDC_BUTTON_FDX_START, OnBtnDnButtonFdxStart)
	ON_CONTROL(WM_LBUTTONUP, IDC_BUTTON_FDX_START, OnBtnUpButtonFdxStart)
	ON_BN_CLICKED(IDC_RADIO_GROUP, OnBnClickedRadioGroup)
	ON_BN_CLICKED(IDC_RADIO_SELECTED, OnBnClickedRadioSelected)
	ON_BN_CLICKED(IDC_RADIO_ALL_ACTIVE, OnBnClickedRadioAllActive)
	ON_BN_CLICKED(IDC_RADIO_AUTOMATIC, OnBnClickedRadioAutomatic)
	ON_BN_CLICKED(IDC_RADIO_MANUAL, OnBnClickedRadioManual)
//	ON_BN_CLICKED(IDC_BUTTON_LISTEN, OnBnClickedButtonListen)
//	ON_BN_CLICKED(IDC_BUTTON_TALK, OnBnClickedButtonTalk)

	ON_BN_DISABLE(IDC_BUTTON_TALK, OnBnDisableTalk)
	ON_BN_DISABLE(IDC_BUTTON_LISTEN, OnBnDisableListen)

	ON_NOTIFY(LVN_KEYDOWN, IDC_LIST_ICOMS, OnLvnKeydownListIcoms)
	ON_NOTIFY(LVN_ITEMACTIVATE, IDC_LIST_CALLS_WAITING, OnLvnItemActivateListCallsWaiting)
	ON_NOTIFY(NM_CLICK, IDC_LIST_CALLS_WAITING, OnNMClickListCallsWaiting)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_CALLS_WAITING, OnNMRclickListCallsWaiting)
	ON_NOTIFY(LVN_DELETEITEM, IDC_LIST_CALLS_WAITING, OnLvnDeleteitemListCallsWaiting)
	ON_BN_CLICKED(IDC_BUTTON_PLAY_PAUSE, OnBnClickedButtonPlayPause)
	ON_BN_CLICKED(IDC_BUTTON_STOP, OnBnClickedButtonStop)
	ON_BN_CLICKED(IDC_BUTTON_MUTE, OnBnClickedButtonMute)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_CQ_PLAYER, OnNMCustomdrawSliderCqPlayer)
//	ON_NOTIFY(NM_KILLFOCUS, IDC_LIST_CALLS_WAITING, OnNMKillfocusListCallsWaiting)
	ON_NOTIFY(NM_SETFOCUS, IDC_LIST_CALLS_WAITING, OnNMSetfocusListCallsWaiting)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_CALLS_WAITING, OnLvnItemchangedListCallsWaiting)
	ON_BN_CLICKED(IDC_BUTTON_PLAY_FILE, OnBnClickedButtonPlayFile)

	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_ICOMS, OnLvnItemchangedListIcoms)
	ON_BN_CLICKED(IDC_BUTTON_CHIME, OnBnClickedButtonChime)

	ON_BN_CLICKED(IDC_BUTTON_END, OnBnClickedButtonEnd)
	ON_BN_CLICKED(IDC_BUTTON_HOLD, OnBnClickedButtonHold)
	ON_NOTIFY(LVN_DELETEALLITEMS, IDC_LIST_ICOMS, OnLvnDeleteallitemsListIcoms)

//	ON_CONTROL(CDDS_ITEMPOSTPAINT, IDC_STATIC_LOGO, OnItemPostPaintLogo)
//	ON_CONTROL(CDRF_NOTIFYPOSTPAINT, IDC_STATIC_LOGO, OnItemPostPaintLogo)
//	ON_CONTROL(CDRF_NOTIFYSUBITEMDRAW, IDC_STATIC_LOGO, OnItemPostPaintLogo)
//	ON_CONTROL(CDRF_NOTIFYITEMDRAW, IDC_STATIC_LOGO, OnItemPostPaintLogo)

//	ON_NOTIFY(CDRF_NOTIFYPOSTPAINT, IDC_STATIC_LOGO, OnItemPostPaintLogo)
//	ON_NOTIFY(CDRF_NOTIFYSUBITEMDRAW, IDC_STATIC_LOGO, OnItemPostPaintLogo)
//	ON_NOTIFY(CDRF_NOTIFYITEMDRAW, IDC_STATIC_LOGO, OnItemPostPaintLogo)
	
	ON_NOTIFY(LVN_ITEMCHANGING, IDC_LIST_ICOMS, OnLvnItemchangingListIcoms)
	ON_NOTIFY(LVN_ITEMCHANGING, IDC_LIST_CALLS_WAITING, OnLvnItemchangingListCallsWaiting)
	ON_NOTIFY(HDN_ENDTRACK, 0, OnHdnEndtrackListIcoms)
	ON_NOTIFY(NM_SETFOCUS, IDC_LIST_ICOMS, OnNMSetfocusListIcoms)
	ON_BN_CLICKED(IDC_CHECK_TEST, OnBnClickedCheckTest)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_MAX_MSEC, OnNMReleasedcaptureSliderMaxMsec)
	ON_BN_CLICKED(IDC_BUTTON_GET_VOLUME, OnBnClickedButtonGetVolume)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_SETUP_VOLUME, OnNMReleasedcaptureSliderSetupVolume)
	ON_BN_CLICKED(IDC_BUTTON_SET_VOLUME, OnBnClickedButtonSetVolume)
	ON_BN_CLICKED(IDC_BUTTON_TEST_VOLUME, OnBnClickedButtonTestVolume)
	ON_BN_CLICKED(IDC_BUTTON_FDX_MUTE, OnBnClickedButtonFdxMute)
	ON_COMMAND(ID_INVISIBLE_RESETINTERCOMCONNECTION, OnInvisibleResetintercomconnection)
	ON_BN_CLICKED(IDC_BUTTON_TEST_AUDIO, OnBnClickedButtonTestAudio)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_MAIN, OnTcnSelchangeTabMain)
	ON_NOTIFY(NM_CLICK, IDC_LIST_GROUPS, OnNMClickListGroups)
	ON_COMMAND(ID_TOOLS_SENDTEXTMESSAGE, OnToolsSendtextmessage)
	ON_NOTIFY(NM_CLICK, IDC_LIST_MESSAGES, OnNMClickListMessages)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_GROUPS, OnNMRclickListGroups)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST_MESSAGES, OnLvnColumnclickListMessages)
	ON_BN_CLICKED(IDC_BUTTON_SESSION_START, OnBnClickedButtonSessionStart)
	ON_BN_CLICKED(IDC_BUTTON_SESSION_END, OnBnClickedButtonSessionEnd)
	ON_COMMAND(ID_TOOLS_TESTCONSOLEDLL, OnToolsTestconsoledll)
END_MESSAGE_MAP()

// CTalkMasterConsoleDlg message handlers

BOOL CTalkMasterConsoleDlg::OnInitDialog()
{
//	loadAlternateResourceDLL();

	CDialog::OnInitDialog();
	COLORREF	cfWhite = RGB(255,255,255);
	BITMAP		bm;
	CDC			*pDC;

	srand( (unsigned)time( NULL ) );

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// Set up the help stuff for the application

	doLocalizedHelpSetup();

// Fix the menu's to be branded

	if( theApp.UserOptions.archiveAudio )
	{
		theApp.UserOptions.archiveAudio = FALSE;		// Will be set to TRUE by next call
		theApp.OnFileCapturearchiveaudio();
	}

	theApp.UserOptions.recordProgramEvents = ~theApp.UserOptions.recordProgramEvents;
	theApp.OnToolsRecordprogrameventstofile();

// Write the first debug message (branded)

#ifdef PRIMEX_BUILD
	writeDebug(_T("IPI Operator Console Version %s started"), getFileVersionString().GetBuffer());
#else
	writeDebug(_T("TalkMaster Console Version %s started"), getFileVersionString().GetBuffer());
#endif

	m_hLib = LoadLibrary(_T("DAPassThru.dll"));
	if( m_hLib || (m_hLib = LoadLibrary(_T("..\\..\\DAPassThruConsoleDll\\Debug\\DAPassThru.dll"))) )
	{
		if( !LoadProcs() )
			outputDebug(_T("Could NOT get all proc addresses"));

		if( m_DADllVersion )
			m_DADllVersion( &m_DAMajor, &m_DAMinor, &m_DARelease );

		writeDebug(_T("Loaded DAPassThruDll Version %d.%d.%d"), m_DAMajor, m_DAMinor, m_DARelease);

		m_uFlags = 0;				// We want cdecl calls to our callback

		if( m_DAOpen )
			m_DAOpen( &m_hDA, &m_uFlags, &CallBack, &m_UserData );

		writeDebug(_T("Opened DAPassThruDll with %d flags"), m_uFlags);

		updateMenus( m_uFlags );

		if( m_DASetDebug )
			m_DASetDebug( TRUE );

		m_csLibVer = getFileVersionString(_T("..\\..\\DAPassThruConsoleDll\\Debug\\DAPassThru.dll"));
		if( m_csLibVer == _T("") )
			m_csLibVer = getFileVersionString(_T("DAPassThru.dll"));
	}

	ListView_SetExtendedListViewStyle(m_listIcoms.m_hWnd, 
		LVS_EX_FULLROWSELECT |
		LVS_EX_CHECKBOXES |
		LVS_EX_SUBITEMIMAGES |
		LVS_EX_GRIDLINES |
		LVS_EX_SUBITEMIMAGES );
//		LVS_EX_FLATSB |
//		LVS_EX_HEADERDRAGDROP );

	m_listIcoms.InsertColumn(COL_GROUP, getResourceString(IDS_STRING_COL_GRP),LVCFMT_LEFT,32,-1);
	m_listIcoms.InsertColumn(COL_LOCATION, getResourceString(IDS_STRING_COL_LOCATION),LVCFMT_LEFT,150,-1);
	m_listIcoms.InsertColumn(COL_QUEUE, getResourceString(IDS_STRING_COL_QUEUE),LVCFMT_LEFT,90,-1);
	m_listIcoms.InsertColumn(COL_STATUS, getResourceString(IDS_STRING_COL_STATUS),LVCFMT_LEFT,101,-1);
	m_listIcoms.InsertColumn(COL_DOOR, getResourceString(IDS_STRING_COL_DOOR_GATE),LVCFMT_LEFT,65,-1);
	m_listIcoms.InsertColumn(COL_ADDRESS, getResourceString(IDS_STRING_COL_IP_ADDRESS),LVCFMT_LEFT,100,-1);

// Groups

	ListView_SetExtendedListViewStyle(m_listGroups.m_hWnd, 
		LVS_EX_FULLROWSELECT |
		LVS_EX_CHECKBOXES |
		LVS_EX_SUBITEMIMAGES |
		LVS_EX_GRIDLINES |
		LVS_EX_SUBITEMIMAGES );
//		LVS_EX_FLATSB |
//		LVS_EX_HEADERDRAGDROP );

	m_listGroups.InsertColumn(GROUP_COL_CHECK, getResourceString(IDS_STRING_COL_GRP),LVCFMT_CENTER,30,-1);
	m_listGroups.InsertColumn(GROUP_COL_ICON, getResourceString(IDS_STRING_GROUP_STATUS),LVCFMT_LEFT,30,-1);
	m_listGroups.InsertColumn(GROUP_COL_NAME, getResourceString(IDS_STRING_GROUP_NAME),LVCFMT_LEFT,250,-1);

	mBitMapGroupEmpty.LoadBitmap(IDB_BITMAP_GROUP_EMPTY);
	mBitMapGroupPartial.LoadBitmap(IDB_BITMAP_GROUP_PARTIAL);
	mBitMapGroupFull.LoadBitmap(IDB_BITMAP_GROUP_FULL);

	m_imageListGroups.Create(25, 19, ILC_COLOR16, 2, 2);

	m_imageListGroups.Add( &mBitMapGroupEmpty, &mBitMapGroupEmpty );
	m_imageListGroups.Add( &mBitMapGroupPartial, &mBitMapGroupPartial );
	m_imageListGroups.Add( &mBitMapGroupFull, &mBitMapGroupFull );

	m_listGroups.SetImageList( &m_imageListGroups, LVSIL_SMALL );

	ListView_SetExtendedListViewStyle(m_listMessages.m_hWnd, 
		LVS_EX_FULLROWSELECT |
//		LVS_EX_CHECKBOXES |
		LVS_EX_SUBITEMIMAGES |
		LVS_EX_GRIDLINES |
		LVS_EX_SUBITEMIMAGES );
//		LVS_EX_FLATSB |
//		LVS_EX_HEADERDRAGDROP );

//
	m_listMessages.InsertColumn(COL_MSG_PRIORITY, getResourceString(IDS_STRING_COL_MSG_PRIORITY),LVCFMT_CENTER,50,-1);
	m_listMessages.InsertColumn(COL_MSG_TITLE, getResourceString(IDS_STRING_COL_MSG_TITLE),LVCFMT_LEFT,230,-1);

	mBitMapPriorityLow.LoadBitmap(IDB_BITMAP_PRIORITY_LOW);
	mBitMapPriorityMedium.LoadBitmap(IDB_BITMAP_PRIORITY_MEDIUM);
	mBitMapPriorityHigh.LoadBitmap(IDB_BITMAP_PRIORITY_HIGH);

	m_imageListMsg.Create(16, 16, ILC_COLOR16, 2, 2);

	m_imageListMsg.Add(&mBitMapPriorityLow, &mBitMapPriorityLow);
	m_imageListMsg.Add(&mBitMapPriorityMedium, &mBitMapPriorityMedium);
	m_imageListMsg.Add(&mBitMapPriorityHigh, &mBitMapPriorityHigh);

	m_listMessages.SetImageList( &m_imageListMsg, LVSIL_SMALL );

	m_listMessages.InsertItem(LVIF_TEXT|LVIF_IMAGE, 0, " ", 0, 0, 4, NULL);
	m_listMessages.SetItemText(0,COL_LOCATION, getResourceString(IDS_STRING_GROUP_NO_GROUP_SELECTED));

//	m_listIcoms.InsertItem( LVIF_TEXT|LVIF_IMAGE, 0, "", 0, 0, 2, NULL);
//	m_listIcoms.SetItemText(0,COL_LOCATION, "Waiting for Connections");
//	m_listIcoms.SetItemText(0,COL_STATUS,"<Please Wait>");
//	m_listIcoms.SetItemData(0, NULL);

	ListView_SetExtendedListViewStyle(m_listCallsWaiting.m_hWnd, 
		LVS_EX_FULLROWSELECT | 
		LVS_EX_SUBITEMIMAGES );
//		LVS_EX_CHECKBOXES |
//		LVS_EX_SUBITEMIMAGES |
//		LVS_EX_GRIDLINES );
//		LVS_EX_SUBITEMIMAGES );
//		LVS_EX_FLATSB |
//		LVS_EX_HEADERDRAGDROP );

	m_listCallsWaiting.InsertColumn(CQ_COL_STATUS, _T(getResourceString(IDS_STRING168)), LVCFMT_LEFT, 30, -1);
	m_listCallsWaiting.InsertColumn(CQ_COL_WAITING, _T(getResourceString(IDS_STRING169)), LVCFMT_LEFT, 150, -1);
	m_listCallsWaiting.InsertColumn(CQ_COL_DETAILS, _T(getResourceString(IDS_STRING170)), LVCFMT_LEFT, 90, -1);
	m_listCallsWaiting.InsertColumn(CQ_COL_TIME, _T(getResourceString(IDS_STRING171)), LVCFMT_LEFT, 80, -1);

//	m_listCallsWaiting.SetToolTips(&m_listCallsWaitingToolTip);

	if( !m_announceStartTime )
	{
		stopAnnounce();
	}
	
	m_listCallsWaiting.InsertItem(0, _T(""));
	m_listCallsWaiting.SetItemText(0, CQ_COL_WAITING, _T(getResourceString(IDS_STRING106)));
	m_listCallsWaiting.SetItem( 0, 0, LVIF_IMAGE, 0, 0, 0, 0, NULL );	// LVIS_SELECTED
//	m_listCallsWaiting.RedrawItems(0,0);
	m_listCallsWaiting.SetItemData(0, NULL);
	insertCallsWaitingItemData(0, NULL);

	resetCQList = TRUE;

	mBitMapLogo.LoadBitmap(IDB_BITMAP_LOGO);

//	m_staticLogo.SetBitmap(HBITMAP(mBitMapLogo));
	mBitMapLogo.GetBitmap(&bm);
	m_staticLogo.SetWindowPos(NULL, 0, 0, bm.bmWidth, bm.bmHeight+5,
			SWP_NOOWNERZORDER | SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOZORDER );

//	DrawTransparent(&mBitMapLogo, m_staticLogo.GetDC(), 0, 0, cfWhite);
//	m_staticLogo.RedrawWindow();

	mBitMapBlank.LoadBitmap(IDB_BITMAP_VU_BLANK);
//	readTransparent(&mBitMapBlank, this->GetDC());
	readTransparent(&mBitMapBlank, (pDC=m_staticLogo.GetDC()));
	m_staticLogo.ReleaseDC(pDC);

	mBitMapGreen.LoadBitmap(IDB_BITMAP_VU_GREEN);
	mBitMapYellow.LoadBitmap(IDB_BITMAP_VU_YELLOW);
	mBitMapRed.LoadBitmap(IDB_BITMAP_VU_RED);

// Set the listen mode to the right value

	m_ListenMode = theApp.UserOptions.listenModeDefault;

	if( m_ListenMode )											// Manual is 1
	{
		m_btnAutomatic.SetCheck(BST_UNCHECKED);
		m_btnManual.SetCheck(BST_CHECKED);
	}
	else														// Auto is 0
	{
		m_btnAutomatic.SetCheck(BST_CHECKED);
		m_btnManual.SetCheck(BST_UNCHECKED);
	}

	m_btnSelected.SetCheck(TRUE);

	mBitMapListenDisable.LoadBitmap(IDB_BITMAP_LISTEN_DISABLE);
	mBitMapListenOff.LoadBitmap(IDB_BITMAP_LISTEN_OFF);
	mBitMapListenOn.LoadBitmap(IDB_BITMAP_LISTEN_ON);
	m_btnListen.SetBitmap( HBITMAP(mBitMapListenOff) );

	mBitMapNoListenOff.LoadBitmap(IDB_BITMAP_NO_LISTEN_OFF);
	mBitMapNoListenOn.LoadBitmap(IDB_BITMAP_NO_LISTEN_ON);
	m_btnNoListen.SetBitmap( HBITMAP(mBitMapNoListenOff) );
	
	mBitMapTalkDisable.LoadBitmap(IDB_BITMAP_TALK_DISABLE);
	mBitMapTalkOff.LoadBitmap(IDB_BITMAP_TALK_OFF);
	mBitMapTalkOn.LoadBitmap(IDB_BITMAP_TALK_ON);
	m_btnTalk.SetBitmap( HBITMAP(mBitMapTalkOff) );

	mBitMapFdxTalkDisable.LoadBitmap(IDB_BITMAP_FDX_TALK_DISABLED);
	mBitMapFdxTalkOff.LoadBitmap(IDB_BITMAP_FDX_TALK_OFF);
	mBitMapFdxTalkOn.LoadBitmap(IDB_BITMAP_FDX_TALK_ON);
	m_btnFdxStart.SetBitmap( HBITMAP(mBitMapFdxTalkOff) );

	mBitMapTalkSessionStart.LoadBitmap(IDB_BITMAP_SESSION_START);
	mBitMapTalkSessionEnd.LoadBitmap(IDB_BITMAP_SESSION_STOP);
	m_btnTalkSessionStart.SetBitmap( HBITMAP(mBitMapTalkSessionStart) );
	m_btnTalkSessionEnd.SetBitmap( HBITMAP(mBitMapTalkSessionEnd) );

	mBitMapFdxMuteDisable.LoadBitmap(IDB_BITMAP_FDX_MUTE_DISABLED);
	mBitMapFdxMute.LoadBitmap(IDB_BITMAP_FDX_MUTE);
	mBitMapFdxMuted.LoadBitmap(IDB_BITMAP_FDX_MUTED);
	m_btnFdxMute.SetBitmap( HBITMAP(mBitMapFdxMute) );

	mBitMapPlay.LoadBitmap(IDB_BITMAP_PLAY);
	mBitMapPause.LoadBitmap(IDB_BITMAP_PAUSE);
	m_btnPlayPause.SetBitmap( HBITMAP(mBitMapPlay) );

	mBitMapStop.LoadBitmap(IDB_BITMAP_STOP);
	m_btnStop.SetBitmap( HBITMAP(mBitMapStop) );

	mBitMapMuteOff.LoadBitmap(IDB_BITMAP_MUTE_OFF);
	mBitMapMuteOn.LoadBitmap(IDB_BITMAP_MUTE_ON);
	m_btnMute.SetBitmap( HBITMAP(mBitMapMuteOff) );

	m_imageList.Create(61, 16, ILC_COLOR16, 2, 2);

#if 0
	mBitMapDoorOpen.LoadBitmap(IDB_BITMAP_DOOR_OPEN);
	mBitMapDoorClosed.LoadBitmap(IDB_BITMAP_DOOR_CLOSED);
	m_imageList.Add(&mBitMapDoorClosed, &mBitMapDoorClosed);
	m_imageList.Add(&mBitMapDoorOpen, &mBitMapDoorOpen);
#else
	mBitMapDoorLockedClosed.LoadBitmap(IDB_BITMAP_DOOR_LOCKED_CLOSED);
	mBitMapDoorUnlockedClosed.LoadBitmap(IDB_BITMAP_DOOR_UNLOCKED_CLOSED);
	mBitMapDoorLockedOpen.LoadBitmap(IDB_BITMAP_DOOR_LOCKED_OPEN);
	mBitMapDoorUnlockedOpen.LoadBitmap(IDB_BITMAP_DOOR_UNLOCKED_OPEN);
	mBitMapDoorNone.LoadBitmap(IDB_BITMAP_DOOR_NONE);

	m_imageList.Add(&mBitMapDoorLockedClosed, &mBitMapDoorLockedClosed);
	m_imageList.Add(&mBitMapDoorUnlockedClosed, &mBitMapDoorUnlockedClosed);
	m_imageList.Add(&mBitMapDoorLockedOpen, &mBitMapDoorLockedOpen);
	m_imageList.Add(&mBitMapDoorUnlockedOpen, &mBitMapDoorUnlockedOpen);
	m_imageList.Add(&mBitMapDoorNone,         &mBitMapDoorNone);
#endif

	m_listIcoms.SetImageList( &m_imageList, LVSIL_SMALL );

	m_imageListCQ.Create(24, 16, ILC_COLOR16, 5, 5);

	mBitMapNormal.LoadBitmap(IDB_BITMAP_CQ_NORMAL);
	mBitMapOverflow.LoadBitmap(IDB_BITMAP_CQ_OVERFLOW);
	mBitMapPriority.LoadBitmap(IDB_BITMAP_CQ_PRIORITY);
	mBitMapLocal.LoadBitmap(IDB_BITMAP_CQ_LOCAL);
	mBitMapHold.LoadBitmap(IDB_BITMAP_CQ_HOLD);
	m_imageListCQ.Add(&mBitMapNormal, &mBitMapNormal);
	m_imageListCQ.Add(&mBitMapOverflow, &mBitMapOverflow);
	m_imageListCQ.Add(&mBitMapPriority, &mBitMapPriority);
	m_imageListCQ.Add(&mBitMapLocal, &mBitMapLocal);
	m_imageListCQ.Add(&mBitMapHold, &mBitMapHold);

	m_listCallsWaiting.SetImageList( &m_imageListCQ, LVSIL_SMALL );

	m_listIcoms.SetItemCount(1024);

	m_hookKb = SetWindowsHookEx(WH_KEYBOARD, KeyboardProc, (HINSTANCE) NULL, GetCurrentThreadId()); 

	if( theApp.UserOptions.hideWindow )
		showSystemTray();

	if( m_DAStart )
	{
		if( m_uFlags & DA_REMOTE )
		{
#ifdef PRIMEX_BUILD
			SetWindowText(_T(getResourceString(IDS_STRING172)));
#else
			SetWindowText(_T(getResourceString(IDS_STRING_OPERATOR_CONSOLE)));
#endif
#if 0
			m_sockaddr_in.sin_family = AF_INET;
			m_sockaddr_in.sin_port = htons(theApp.UserOptions.IPPort);
			m_sockaddr_in.sin_addr.S_un.S_addr = inet_addr(theApp.UserOptions.IPAddress);

			CDALoginDlg dlg;

			INT_PTR nResponse = dlg.DoModal();
			if (nResponse == IDOK)
				m_DAStart( m_hDA, &m_sockaddr_in, m_threadPriority, dlg.m_csLoginName.GetBuffer(), dlg.m_csLoginPassword.GetBuffer());
#endif
		}
		else				// Direct connect DLL loaded
		{
			m_sockaddr_in.sin_port = htons(3000);
			m_DAStart( m_hDA, &m_sockaddr_in, m_threadPriority, _T(""), _T(""), NULL, NULL);
			bLoggedOn = TRUE;			// Direct connect is always logged on
		}
	}

	initCallQueue();
	initAnnounce();

//	SetTimer(TIMER_TALK, 250, NULL);
	SetTimer(TIMER_CHECK_LOGON, 250, NULL);

	m_staticStatusPTT.EnableWindow(theApp.UserOptions.PTT);

	m_rClickMenu.LoadMenu(IDR_MENU_RCLICCK);
	m_rClickGroupMenu.LoadMenu(IDR_MENU_GROUP_RCLICK);

	if( bUseUlaw )
	{
		m_staticCodec.SetWindowText(_T(getResourceString(IDS_STRING_ULAW)));
	}
	else
	{
		m_staticCodec.SetWindowText(_T(getResourceString(IDS_STRING_PCM)));
	}

	m_ctrlMaxMsec.SetRange(100, 3000);

	m_ctrlMaxMsec.SetPos(3000);

	m_csMaxMsec = _T(getResourceString(IDS_STRING153));
	m_cstaticMaxMsec.SetWindowText(_T(getResourceString(IDS_STRING153)));

	m_tabMain.InsertItem(0, getResourceString(IDS_STRING_INTERCOMS));
	m_tabMain.InsertItem(1, getResourceString(IDS_STRING_GROUPS));

	if( theApp.UserOptions.startSize.right )
		this->SetWindowPos(NULL, 
		theApp.UserOptions.startSize.left,
		theApp.UserOptions.startSize.top,
		theApp.UserOptions.startSize.right - theApp.UserOptions.startSize.left,
		theApp.UserOptions.startSize.bottom - theApp.UserOptions.startSize.top,
		SWP_NOOWNERZORDER | SWP_SHOWWINDOW | SWP_NOZORDER );

	doSize();

	this->RedrawWindow();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CTalkMasterConsoleDlg::doCancel()		// This is a public version that calls the private version
{
	static int guard = 0;
	struct _iComQueueList *list = m_pIcomQueueList;
	struct _itemData *item = m_pItemData;

	if( ++guard == 1 )
	{
#ifdef PRIMEX_BUILD
		writeDebug(_T("IPI Operator Console Version %s Stopped"), getFileVersionString().GetBuffer());
#else
		outputDebug(_T("TalkMaster Console Version %s Stopped"), getFileVersionString().GetBuffer());
#endif

		KillTimer(TIMER_DOOR);
		KillTimer(TIMER_CQPLAY);
		KillTimer(TIMER_ANNOUNCE);
		KillTimer(TIMER_ALERT);
		KillTimer(TIMER_TALK);
		KillTimer(TIMER_CHECK_LOGON);
		KillTimer(TIMER_RING);

		if( m_hDA )
		{
			bShuttingDown = TRUE;

			if( m_pSelectedItem && !theApp.UserOptions.forceHdx &&
				m_pSelectedItem->iCom.picRev.feature1 & OPNIC_FULL_DUPLEX )
			{
				if( bTalking )
					doFdxStartStop();				// Stop it
			}
			else
			{
				if( bTalking )
				{
					endTalk(m_sessionSocket);
					bTalking = FALSE;

					Sleep(500);
				}
				if( bListening )
				{
					endListen(m_sessionSocket);
					bListening = FALSE;

					Sleep(500);
				}
			}

			bTalkRequest = FALSE;
			bListenRequest = FALSE;

			if( m_sessionSocket )			// If we have an intercom (group?) selected, deselect it
			{
				deselectSocket( m_sessionSocket );
				Sleep(500);
			}

			m_sessionSocket = 0;

			bLoggedOn = FALSE;

			outputDebug(_T("OnCancel: DAStop()"));
			m_DAStop( m_hDA );

			outputDebug(_T("OnCancel: DAClose()"));
			m_DAClose( m_hDA );

			outputDebug(_T("OnCancel: FreeLibrary()"));
			FreeLibrary( m_hLib );

			m_hLib = NULL;
			m_hDA = NULL;

			outputDebug(_T("OnCancel: Clearing Call Queue Item Data"));
			clearCallsWaitingItemData();
			m_listCallsWaiting.DeleteAllItems();

			while(list)										// Get rid of the queue information
			{
				m_pIcomQueueList = list->next;
				free(list);
				list = m_pIcomQueueList;
			}

			outputDebug(_T("OnCancel: Deleting Intercom Information"));

			Sleep(200);										// Give everything a chance to settle down

			deleteIcoms(TRUE);								// Clear the list

			while(item)
			{
				m_pItemData = item->next;
				free(item);
				item = m_pItemData;
			}

			deleteGroups();

			deleteMessages();

			UnhookWindowsHookEx( m_hookKb ); 
		}

		outputDebug(_T("OnCancel: Done"));

		OpenCloseDebugFile(TRUE);

		CDialog::OnCancel();
	}

	--guard;

	delSystemTray();
}

void CTalkMasterConsoleDlg::OnCancel()
{
	if( theApp.UserOptions.hideWindow )
		showMinimized(TRUE);
	else
		doCancel();
}

void CTalkMasterConsoleDlg::OnActivate(UINT nState,CWnd* pWndOther,BOOL bMinimized)
{
	if( nState == WA_ACTIVE || nState == WA_CLICKACTIVE )
	{
//		m_staticLogo.RedrawWindow();
//		DrawTransparent(&mBitMapLogo, m_staticLogo.GetDC(), 0, 0, m_crWhite);
//		this->SetWindowPos(NULL, 0, 0, 1, 1,
//				SWP_NOOWNERZORDER | SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOZORDER );
	}
}

void CTalkMasterConsoleDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	if( nStatus == SW_PARENTOPENING && bShow )
	{
//		m_staticLogo.RedrawWindow();
//		DrawTransparent(&mBitMapLogo, m_staticLogo.GetDC(), 0, 0, m_crWhite);
//		this->SetWindowPos(NULL, 0, 0, 1, 1,
//				SWP_NOOWNERZORDER | SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOZORDER );
	}
}

//
// doLogon
//
// This function calls the logon dialog and tells it whether it is supposed to auto-logon or not.
//
// This version of this function does not store the logon name/password between application execution.
// Instead, it relogs a valid user back onto a system that had to reboot.  Thus the static CString variables
// for the name and password rather than storing those values in the registry.
//

void CTalkMasterConsoleDlg::doLogon()
{
	static int guard = 0;

	if( bShuttingDown )
	{
		outputDebug(_T("doLogon: calling OnCancel() because bShuttingDown set"));
		doCancel();
		return;
	}

	if( ++guard == 1 && !bLoggedOn && (m_uFlags & DA_REMOTE) )
	{
		CDALoginDlg dlg;
		char buffer[100];

//		outputDebug(_T("guard=%d bLoggedOn=%d m_uFlags=%d"), guard, bLoggedOn, m_uFlags);
		resetDialog();

		dlg.m_csLoginName = m_csLoginName;			// These are empty unless a valid login has already occured
		dlg.m_csLoginPassword = m_csLoginPassword;	// so these having values means we should re-login

		if( !m_csLoginName.IsEmpty() && !m_csLoginPassword.IsEmpty() )
			dlg.m_bAutoLogon = TRUE;

		dlg.m_csLogonIpAddress = theApp.UserOptions.IPAddress;
		dlg.m_csLogonIPPort = itoa(theApp.UserOptions.IPPort, buffer, 10);

		bOffDialog = TRUE;							// Not on the main dialog

		INT_PTR nResponse = dlg.DoModal();
		bLoggedOn = dlg.bLoggedOn;
		if( nResponse == IDCANCEL )
		{
			bShuttingDown = TRUE;
//			doCancel();								// Do it in a different loop
		}
		else
		{
			m_threadControl = _beginthread( controlThread, 0, 0 );	// Start the controlling thread

			m_csLoginName = dlg.m_csLoginName;
			m_csLoginPassword = dlg.m_csLoginPassword;

			strcpy(theApp.UserOptions.IPAddress, dlg.m_csLogonIpAddress);
			theApp.UserOptions.IPPort = atoi(dlg.m_csLogonIPPort);

			strcpy(consoleName, dlg.m_szUserName);

			updateAdminMenus( dlg.m_uRights );

			OpenCloseDebugFile();					// Open it here after the console name and info has been saved
#ifdef PRIMEX_BUILD
			outputDebug(_T("*** IPI Operator Console Login ***"));
#else
			outputDebug(_T("*** TalkMaster Console Login ***"));
#endif

#ifdef PRIMEX_BUILD
			sprintf(buffer, _T("IPI Operator Console - Logon ID %s"), dlg.m_szUserName);
#else
			sprintf(buffer, getResourceString(IDS_STRING_OPERATOR_CONSOLE_LOGON_ID), dlg.m_szUserName);
#endif
			SetWindowText(buffer);
		}

		bOffDialog = FALSE;							// Back on the main dialog
		bSkipNextKeyUp = TRUE;
	}

	--guard;
}

void CTalkMasterConsoleDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		OnAbout();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

void CTalkMasterConsoleDlg::OnAbout() 
{
	CAboutDlg dlgAbout;

	bOffDialog = TRUE;							// Not on the main dialog

	dlgAbout.DoModal();

	bOffDialog = FALSE;							// Back on the main dialog
	bSkipNextKeyUp = TRUE;
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CTalkMasterConsoleDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDC *pDC;
		CDialog::OnPaint();
		if( !bOnce )
		{
			bOnce = TRUE;
			readTransparent(&mBitMapBlank, (pDC=m_staticLogo.GetDC()));
			m_staticLogo.ReleaseDC(pDC);
		}

		drawBitmaps();
		doLogon();

	}
}

int CTalkMasterConsoleDlg::MessageBox(LPCTSTR lpszText, LPCTSTR lpszCaption, UINT nType)
{
	int iRet;

	bOffDialog = TRUE;							// Not on the main dialog

	iRet = CWnd::MessageBox(lpszText, lpszCaption, nType);

	bOffDialog = FALSE;							// Back on the main dialog
	bSkipNextKeyUp = TRUE;

	return(iRet);
}


// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CTalkMasterConsoleDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CTalkMasterConsoleDlg::OnHdnItemclickListIcoms(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
	struct _itemData *tItemData;
	int i, itemCount, checkCount=0;

	if( phdr->iItem > 0 )				// Is not the group checkbox column
	{
		if( m_sortBy == phdr->iItem )
			m_bOrder = !m_bOrder;
		else
			m_bOrder = 0;

		m_sortBy = phdr->iItem;

		deleteIcoms(FALSE);				// Really force the redisplay
		redisplayIcomList();
	}
	else								// Clicked on the group header checkbox
	{
		itemCount = m_listIcoms.GetItemCount();

		for(i=0;i<itemCount;i++)								// Run through all of the items in the list box
		{
			if( m_listIcoms.GetCheck(i) )						// check to see if it is checked
				checkCount++;									// If it is, count them
		}

		for(i=0;i<itemCount;i++)								// Run through all of the items in the list box
		{
			tItemData = (struct _itemData *)getIcomItemData(i);	// Get the itemData so we can set values

			if( tItemData )										// Make sure there is some data before just setting a value
			{
				if( (checkCount>0) || tItemData->iCom.status )	// If we are clearing values, or the intercom is connected
				{
					m_listIcoms.SetCheck(i, (checkCount==0));		// Clear the flag if any items are set
					tItemData->bChecked = (checkCount==0);					// And set it in the control array
				}
			}
		}
	}

	*pResult = 0;
}

void CTalkMasterConsoleDlg::selectIcomRow(int row)
{
	int	selectedRow;
	POSITION pos = m_listIcoms.GetFirstSelectedItemPosition();

	if( row > -1 )
	{

		while( pos )			// Loop through all the selected rows and clear them
		{
			selectedRow = m_listIcoms.GetNextSelectedItem(pos);

			m_listIcoms.SetItemState(selectedRow, 0, LVIS_SELECTED|LVIS_FOCUSED);
			m_listIcoms.EnsureVisible(selectedRow, FALSE);
		}

		m_pSelectedItem = (struct _itemData *)getIcomItemData(row);
		m_listIcoms.SetItemState(row, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
		m_listIcoms.EnsureVisible(row, FALSE);

		m_btnGetVolume.EnableWindow(m_pSelectedItem != NULL && m_TalkMode == TALK_SELECTED);
		m_btnSetVolume.EnableWindow(m_pSelectedItem != NULL);
		m_btnTestVolume.EnableWindow(m_pSelectedItem != NULL && GetFileAttributes(theApp.UserOptions.testVolumeFile)!=INVALID_FILE_ATTRIBUTES);
	}
	m_selectedRow = row;
}

void CTalkMasterConsoleDlg::OnLvnColumnclickListIcoms(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	struct _itemData *tItemData;
	int i, itemCount, checkCount=0;

	if( pNMLV->iSubItem > 0 )				// Is not the group checkbox column
	{
		if( m_sortBy == pNMLV->iSubItem )
			m_bOrder = !m_bOrder;
		else
			m_bOrder = 0;

		m_sortBy = pNMLV->iSubItem;

		deleteIcoms(FALSE);					// Really force the redisplay
		redisplayIcomList();
	}
	else								// Clicked on the group header checkbox
	{
		itemCount = m_listIcoms.GetItemCount();

		for(i=0;i<itemCount;i++)								// Run through all of the items in the list box
		{
			if( m_listIcoms.GetCheck(i) )						// check to see if it is checked
				checkCount++;									// If it is, count them
		}

		for(i=0;i<itemCount;i++)								// Run through all of the items in the list box
		{
			tItemData = (struct _itemData *)getIcomItemData(i);	// Get the itemData so we can set values

			if( tItemData )										// Make sure there is some data before just setting a value
			{
				if( (checkCount>0) || tItemData->iCom.status )	// If we are clearing values, or the intercom is connected
				{
					m_listIcoms.SetCheck(i, (checkCount==0));		// Clear the flag if any items are set
					tItemData->bChecked = (checkCount==0);					// And set it in the control array
				}
			}
		}
	}

	*pResult = 0;
}

void CTalkMasterConsoleDlg::OnNMRclickListIcoms(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	char buffer[128];
	CMenu *popupMenu;

	if( pNMLV->uOldState & LVIS_SELECTED && pNMLV->iItem != m_selectedRow )
	{
		loadIcomList(pNMHDR);
	}

	if( !m_pSelectedItem )
		return;

	POINT pPos; 

	GetCursorPos(&pPos); 

	popupMenu = m_rClickMenu.GetSubMenu(0);

	sprintf( buffer, getResourceString(IDS_STRING_INTERCOM_NAME_S), m_pSelectedItem->iCom.name );
	replaceMenuItem( popupMenu, 0, buffer, ID_INVISIBLE_INTERCOMNAME );

	sprintf( buffer, getResourceString(IDS_STRING_INTERCOM_ID_02X_02X_02X), 
		m_pSelectedItem->iCom.MAC.MAC4,
		m_pSelectedItem->iCom.MAC.MAC5,
		m_pSelectedItem->iCom.MAC.MAC6 );
	replaceMenuItem( popupMenu, 2, buffer, ID_INVISIBLE_INTERCOMID );

	sprintf( buffer, getResourceString(IDS_STRING_NIC_VERSION_D_D_D),
		m_pSelectedItem->iCom.nicRev.major,
		m_pSelectedItem->iCom.nicRev.minor,
		m_pSelectedItem->iCom.nicRev.build );
	replaceMenuItem( popupMenu, 3, buffer, ID_INVISIBLE_NICVERSION );

	if( m_pSelectedItem->iCom.picRev.major > 4 )
	{
		sprintf( buffer, getResourceString(IDS_STRING_OS_VERSION_D_D_D_D_D),
			m_pSelectedItem->iCom.picRev.major,
			m_pSelectedItem->iCom.picRev.minor,
			m_pSelectedItem->iCom.picRev.release,
			m_pSelectedItem->iCom.picRev.build,
			m_pSelectedItem->iCom.picRev.make );
	}
	else
	{
		sprintf( buffer, getResourceString(IDS_STRING_OS_VERSION_D_D_D_D_C),
			m_pSelectedItem->iCom.picRev.major,
			m_pSelectedItem->iCom.picRev.minor,
			m_pSelectedItem->iCom.picRev.release,
			m_pSelectedItem->iCom.picRev.build,
			m_pSelectedItem->iCom.picRev.make );
	}
	replaceMenuItem( popupMenu, 4, buffer, ID_INVISIBLE_OSVERSION );

	sprintf( buffer, getResourceString(IDS_STRING_NIC_OPTIONS_02X_02X_02X_02X),
		m_pSelectedItem->iCom.picRev.opMode1,
		m_pSelectedItem->iCom.picRev.opMode2,
		m_pSelectedItem->iCom.picRev.feature1,
		m_pSelectedItem->iCom.picRev.feature2 );

	replaceMenuItem( popupMenu, 5, buffer, ID_INVISIBLE_OPTIONS );

	popupMenu->TrackPopupMenu(TPM_RIGHTBUTTON, pPos.x, pPos.y, this, 0);

	setResetFocus(&m_listIcoms);
//	m_listCallsWaiting.SetFocus();

	*pResult = 0;
}

void CTalkMasterConsoleDlg::OnLvnDeleteitemListIcoms(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here

	struct _itemData *tItemData;

	tItemData = (struct _itemData *)getIcomItemData(pNMLV->iItem);
	if(tItemData)
	{
		if( tItemData->hArchive )
			audioArchive.Delete(tItemData->hArchive, FALSE);
//		if( tItemData->hCallQueue )								// Let the CQ list delete the file
//			audioArchive.Delete(tItemData->hCallQueue, TRUE);
//		free(tItemData);
	}

	*pResult = 0;
}

void CTalkMasterConsoleDlg::OnLvnDeleteallitemsListIcoms(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here

	struct _itemData *tItemData;
	int	i, count;

	count = m_listIcoms.GetItemCount();

	for(i=0;i<count;i++)
	{
		tItemData = (struct _itemData *)getIcomItemData(i);
		if(tItemData)
		{
			if( tItemData->hArchive )
				audioArchive.Delete(tItemData->hArchive, FALSE);

			tItemData->hArchive = NULL;
	//		if( tItemData->hCallQueue )								// Let the CQ list delete the file
	//			audioArchive.Delete(tItemData->hCallQueue, TRUE);
	//		free(tItemData);
		}
	}

	*pResult = 0;
}

void CTalkMasterConsoleDlg::OnLvnDeleteitemListCallsWaiting(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here

	struct _itemData *tItemData;

	tItemData = (struct _itemData *)getCallsWaitingItemData(pNMLV->iItem);
	if(tItemData)
	{
		if( tItemData->hCallQueue )
			audioArchive.Delete(tItemData->hCallQueue, TRUE);
	}

	*pResult = 0;
}

void CTalkMasterConsoleDlg::OnSize(UINT nType,int cx,int cy)
{
	showMinimized(nType == SIZE_MINIMIZED);

	doSize();
	CDialog::OnSize(nType, cx, cy);
}

void CTalkMasterConsoleDlg::doSize()
{
#define	BOTTOM_STATUS	10

	RECT view, listBox;
	int	cx, cy;
	int showFdx = SWP_HIDEWINDOW;
	int showHdx = SWP_SHOWWINDOW;
	int showTalk = SWP_SHOWWINDOW;
	int showNoListen = SWP_HIDEWINDOW;
	int showSession = SWP_HIDEWINDOW;
	int leftover;

	this->GetClientRect(&view);

	cx = view.right - view.left;
	cy = view.bottom - view.top;

	leftover = cx - 26 - 283 - 14;

	if( m_tabMain.GetSafeHwnd() == NULL || m_tabMain.GetCurSel() == 0 )
//	if( m_tabMain.GetCurSel() == 0 )
	{
		if( m_pSelectedItem &&
			!theApp.UserOptions.forceHdx &&
		(m_pSelectedItem->iCom.picRev.feature1 & OPNIC_FULL_DUPLEX) )
		{
			showFdx = SWP_SHOWWINDOW;
			showHdx = SWP_HIDEWINDOW;
			showTalk = SWP_HIDEWINDOW;
		}
		else if( m_pSelectedItem &&
				m_pSelectedItem->iCom.picRev.opMode1 & OPMODE_REMLISTN_DISA )
		{
			showHdx = SWP_HIDEWINDOW;
			showNoListen = SWP_SHOWWINDOW;
		}

//
// If this record is a handset record, we need to show the CALL and the LISTEN buttons
//
		if( m_pSelectedItem && m_TalkMode == TALK_SELECTED &&
			!getCQItemDataFromSocket(m_pSelectedItem->socket) &&				// Not a call queue entry
			m_pSelectedItem->iCom.picRev.feature2 & OPNIC_2_HANDSET )
		{
			showFdx = SWP_HIDEWINDOW;
			showHdx = SWP_SHOWWINDOW;
			showTalk = SWP_HIDEWINDOW;
			showSession = SWP_SHOWWINDOW;
		}
	}
	else
	{
		showFdx = SWP_HIDEWINDOW;
		showHdx = SWP_SHOWWINDOW;
		showTalk = SWP_SHOWWINDOW;
		showNoListen = SWP_HIDEWINDOW;
	}

	if( m_listIcoms.m_hWnd )
	{
// Move the logo
		m_staticLogo.SetWindowPos(NULL, cx-217, 11, 0, 0,
			SWP_NOOWNERZORDER | SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOZORDER );

// Move the tabs
		m_tabMain.SetWindowPos(&wndBottom, 0, 0, cx - 22, cy - 237 - BOTTOM_STATUS,
			SWP_NOOWNERZORDER | SWP_SHOWWINDOW | SWP_NOMOVE );

		if( m_tabMain.GetCurSel() == 0 )
		{
			m_listGroups.ShowWindow(SW_HIDE);
			m_listMessages.ShowWindow(SW_HIDE);
// Move the intercom list
			m_listIcoms.GetClientRect(&view);
			m_listIcoms.SetWindowPos(NULL, 0, 0, cx - 22, cy - 257 - BOTTOM_STATUS,
				SWP_NOOWNERZORDER | SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOZORDER );

			m_listIcoms.SetColumnWidth(COL_LOCATION, (leftover * 625) / 1000);
			m_listIcoms.SetColumnWidth(COL_QUEUE, (leftover * 375) / 1000);
		}
		else
		{
			m_listIcoms.ShowWindow(SW_HIDE);
			m_listGroups.ShowWindow(SW_SHOW);
			m_listGroups.SetWindowPos(NULL, view.left + 11, 56, (cx - 24)/2, cy - 257 - BOTTOM_STATUS,
				SWP_NOOWNERZORDER | SWP_SHOWWINDOW | SWP_NOZORDER );

			m_listGroups.GetClientRect(&listBox);
			m_listGroups.SetColumnWidth(COL_QUEUE, listBox.right - listBox.left - 80 );

			m_listMessages.ShowWindow(SW_SHOW);
			m_listMessages.SetWindowPos(NULL, ((cx-24)/2)+13, 56, (cx - 22)/2, cy - 257 - BOTTOM_STATUS,
				SWP_NOOWNERZORDER | SWP_SHOWWINDOW | SWP_NOZORDER );

			m_listMessages.GetClientRect(&listBox);
			m_listMessages.SetColumnWidth(COL_MSG_TITLE, listBox.right - listBox.left - 50 );
		}

		if( !bSetupVolume )
		{
// Move the Calls Waiting information list
			m_listCallsWaiting.GetClientRect(&view);
			m_listCallsWaiting.SetWindowPos(NULL, view.left + 11, cy - 191 - BOTTOM_STATUS, cx - 205, 78 + 56,
				SWP_NOOWNERZORDER | SWP_SHOWWINDOW | SWP_NOZORDER );

			m_listCallsWaiting.SetColumnWidth(CQ_COL_WAITING, (leftover * 625) / 1000);
			m_listCallsWaiting.SetColumnWidth(CQ_COL_DETAILS, (leftover * 375) / 1000);
		}

// Move the TalkMode group box
		m_groupboxTalkMode.GetClientRect(&view);
		m_groupboxTalkMode.SetWindowPos(NULL, cx-95, cy-198 - BOTTOM_STATUS, 0, 0, 
			SWP_FRAMECHANGED | SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOZORDER );

// Move the talk button
		m_btnTalk.GetClientRect(&view);
		m_btnTalk.SetWindowPos(NULL, cx-91, cy-66 - BOTTOM_STATUS, 0, 0, 
			showTalk | SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOZORDER );

// Move the Start button
		m_btnFdxStart.GetClientRect(&view);
		m_btnFdxStart.SetWindowPos(NULL, cx-91, cy-66 - BOTTOM_STATUS, 0, 0, 
			showFdx | SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOZORDER );

// Move the other Start button
		m_btnTalkSessionStart.GetClientRect(&view);
		m_btnTalkSessionStart.SetWindowPos(NULL, cx-91, cy-66 - BOTTOM_STATUS, 0, 0, 
			showSession | SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOZORDER );

// Move the Session Listen Button
		m_btnTalkSessionEnd.SetWindowPos(NULL, cx-91, cy-66 - BOTTOM_STATUS, 0, 0, 
			SWP_HIDEWINDOW | SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOZORDER );

// Move the chime button
		m_btnChime.GetClientRect(&view);
		m_btnChime.SetWindowPos(NULL, cx-91, cy-96 - BOTTOM_STATUS, 0, 0, 
			SWP_SHOWWINDOW | SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOZORDER );

// Move the Play File button
		m_btnPlayFile.GetClientRect(&view);
		m_btnPlayFile.SetWindowPos(NULL, cx-91, cy-125 - BOTTOM_STATUS, 0, 0, 
			SWP_SHOWWINDOW | SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOZORDER );

// Move the Microphone VU Meter
		m_staticVUMeterMicrophone.SetWindowPos(NULL, cx-89, cy-24 - BOTTOM_STATUS, 75, 10,
			SWP_SHOWWINDOW | SWP_FRAMECHANGED | SWP_NOZORDER );

// Move the All Active radio button
		m_btnAllActive.GetClientRect(&view);
		m_btnAllActive.SetWindowPos(NULL, cx-88, cy-148 - BOTTOM_STATUS, 0, 0, 
			SWP_SHOWWINDOW | SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOZORDER );

// Move the group radio button
		m_btnGroup.GetClientRect(&view);
		m_btnGroup.SetWindowPos(NULL, cx-88, cy-164 - BOTTOM_STATUS, 0, 0, 
			SWP_SHOWWINDOW | SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOZORDER );

// Move the selected radio button
		m_btnSelected.GetClientRect(&view);
		m_btnSelected.SetWindowPos(NULL, cx-88, cy-180 - BOTTOM_STATUS, 0, 0, 
			SWP_SHOWWINDOW | SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOZORDER );

// Move the Hold Button
		m_btnSessionHold.GetClientRect(&view);
		m_btnSessionHold.SetWindowPos(NULL, cx-178, cy-187 - BOTTOM_STATUS, 0, 0, 
			SWP_SHOWWINDOW | SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOZORDER );

// Move the End Button
		m_btnSessionEnd.GetClientRect(&view);
		m_btnSessionEnd.SetWindowPos(NULL, cx-178, cy-153 - BOTTOM_STATUS, 0, 0, 
			SWP_SHOWWINDOW | SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOZORDER );

// Move the Listen Mode group
		m_groupboxListenMode.GetClientRect(&view);
		m_groupboxListenMode.SetWindowPos(NULL, cx-178, cy-120 - BOTTOM_STATUS, 0, 0, 
			SWP_FRAMECHANGED | SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOZORDER );

// Move the Listen button
		m_btnListen.GetClientRect(&view);
		m_btnListen.SetWindowPos(NULL, cx-175, cy-66 - BOTTOM_STATUS, 0, 0, 
			showHdx | SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOZORDER );

// Move the Listen button
		m_btnNoListen.GetClientRect(&view);
		m_btnNoListen.SetWindowPos(NULL, cx-175, cy-66 - BOTTOM_STATUS, 0, 0, 
			showNoListen | SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOZORDER );

// Move the Listen button
		m_btnFdxMute.GetClientRect(&view);
		m_btnFdxMute.SetWindowPos(NULL, cx-175, cy-66 - BOTTOM_STATUS, 0, 0, 
			showFdx | SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOZORDER );

// Move the Speaker VU Meter
		m_staticVUMeterSpeaker.SetWindowPos(NULL, cx-175, cy-24 - BOTTOM_STATUS, 75, 10,
			SWP_SHOWWINDOW | SWP_FRAMECHANGED | SWP_NOZORDER );

// Move the Manual mode radio button
		m_btnManual.GetClientRect(&view);
		m_btnManual.SetWindowPos(NULL, cx-172, cy-86 - BOTTOM_STATUS, 0, 0, 
			SWP_SHOWWINDOW | SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOZORDER );

// Move the Automatic mode radio button
		m_btnAutomatic.GetClientRect(&view);
		m_btnAutomatic.SetWindowPos(NULL, cx-172, cy-104 - BOTTOM_STATUS, 0, 0, 
			SWP_SHOWWINDOW | SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOZORDER );

		if( !bSetupVolume )
		{
// Move the player buttons and group box
			m_groupboxPlayer.GetClientRect(&view);
			m_groupboxPlayer.SetWindowPos(NULL, view.left + 11, cy - 113 - BOTTOM_STATUS + 56, cx - 205, 46, 
				SWP_FRAMECHANGED | SWP_NOOWNERZORDER | SWP_NOZORDER );

// Move the Play button
			m_btnPlayPause.GetClientRect(&view);
			m_btnPlayPause.SetWindowPos(NULL, view.left + 23, cy-100 - BOTTOM_STATUS + 56, 0, 0, 
				SWP_SHOWWINDOW | SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOZORDER );

// Move the Stop button
			m_btnStop.GetClientRect(&view);
			m_btnStop.SetWindowPos(NULL, view.left + 62, cy-96 - BOTTOM_STATUS + 56, 0, 0, 
				SWP_SHOWWINDOW | SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOZORDER );

// Move the Mute button
			m_btnMute.GetClientRect(&view);
			m_btnMute.SetWindowPos(NULL, view.left + 110, cy-96 - BOTTOM_STATUS + 56, 0, 0, 
				SWP_SHOWWINDOW | SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOZORDER );

// Move the Slider
			m_sliderCQPlayer.GetClientRect(&view);
			m_sliderCQPlayer.SetWindowPos(NULL, view.left + 144, cy-96 - BOTTOM_STATUS + 56, cx - 349, 18, 
				SWP_SHOWWINDOW | SWP_FRAMECHANGED | SWP_NOZORDER );
		}
		else
		{
			m_ctrlSetupVolume.SetWindowPos(NULL, 15, cy-150, cx-289, 50, 
						SWP_SHOWWINDOW | SWP_FRAMECHANGED | SWP_NOZORDER );
			m_ctrlVolumeSetting.SetWindowPos(NULL, 25, cy-100, cx-309, 20, 
						SWP_SHOWWINDOW | SWP_FRAMECHANGED | SWP_NOZORDER );
			m_btnSetVolume.SetWindowPos(NULL, cx-269, cy-150, 0, 0,
						SWP_SHOWWINDOW | SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOSIZE );
			m_btnGetVolume.SetWindowPos(NULL, cx-269, cy-100, 0, 0,
						SWP_SHOWWINDOW | SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOSIZE );
			m_btnTestVolume.SetWindowPos(NULL, cx-269, cy-65, 0, 0,
						SWP_SHOWWINDOW | SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOSIZE );

		}

// Move and size the status bar(s)
		m_staticStatusConnected.SetWindowPos(NULL, view.left + 10, cy-18, cx - 205, 15,
			SWP_SHOWWINDOW | SWP_FRAMECHANGED | SWP_NOZORDER );

		m_staticStatusPTT.SetWindowPos(NULL, cx-175, cy-18, 30, 15,
			SWP_SHOWWINDOW | SWP_FRAMECHANGED | SWP_NOZORDER );

		m_staticCodec.SetWindowPos(NULL, cx-93, cy-18, 30, 15,
			SWP_SHOWWINDOW | SWP_FRAMECHANGED | SWP_NOZORDER );

// Redraw all of the controls now to not end up with _T("dirt") on the dialog
		RedrawWindow();

//		DrawTransparent(&mBitMapLogo, m_staticLogo.GetDC(), 0, 0, m_crWhite);
	}
}

void CTalkMasterConsoleDlg::OnTimer(UINT nIDEvent) 
{
	if (nIDEvent == TIMER_DOOR )
		clearDoor();
	else if (nIDEvent == TIMER_CQPLAY)
		playCallQueue();
	else if (nIDEvent == TIMER_ANNOUNCE)
		announceCheck();
	else if (nIDEvent == TIMER_ALERT)
		doAlerts();
	else if (nIDEvent == TIMER_TALK)
		doTalkTimer();
	else if (nIDEvent == TIMER_TEST)
		doTimerTest();
	else if (nIDEvent == TIMER_CHECK_LOGON)
		doLogon();
	else if (nIDEvent == TIMER_TEST_AUDIO)
		doTestAudio();
	else if (nIDEvent == TIMER_REPEAT)
		doRepeatAudio();
	else if (nIDEvent == TIMER_INIT_DONE)
		doPostInit();
	else if( nIDEvent == TIMER_UPDATE_READY)
		doGetSettingsUpdate();
	else if( nIDEvent == TIMER_WORK )
		doWork();
	else if( nIDEvent == TIMER_RING )
		doRing(FALSE);

	CDialog::OnTimer(nIDEvent);
}

void CTalkMasterConsoleDlg::clearDoor() 
{
	int count, index, countWaiting=0;

	count = m_listIcoms.GetItemCount();

	for ( index = 0; index < count ; index++ )
	{
		struct _itemData *tItemData;
		tItemData = (struct _itemData *)getIcomItemData(index);

		if( tItemData->doorOpenTimer && tItemData->doorOpenTimer < time(NULL) )	// Times UP
		{
			if( tItemData->iCom.picRev.opMode1 & OPMODE_RELAY )
			{
				if( tItemData->sensorState == 0 || 
					tItemData->iCom.picRev.opMode2 & OPMODE_GENERAL_SENSOR )
					m_listIcoms.SetItem( index, COL_DOOR, LVIF_IMAGE, 0, 0, 0, 0, NULL );
				else if( tItemData->sensorState == 'S' )
					m_listIcoms.SetItem( index, COL_DOOR, LVIF_IMAGE, 0, 2, 0, 0, NULL );
				else
					m_listIcoms.SetItem( index, COL_DOOR, LVIF_IMAGE, 0, 3, 0, 0, NULL );
			}

			tItemData->doorOpenTimer = 0;
		}
		else if( tItemData->doorOpenTimer )
			countWaiting++;

//			m_listIcoms.SetItemText(index,COL_DOOR,_T("Door"));
	}

	if( countWaiting == 0 )
		KillTimer(TIMER_DOOR);
}

void CTalkMasterConsoleDlg::OnBnClickedRadioSelected()
{
	m_TalkMode = TALK_SELECTED;

	m_btnListen.EnableWindow(TRUE);

	if( m_ListenModeHold > -1 )
	{
		m_ListenMode = m_ListenModeHold;
		m_ListenModeHold = -1;
	}

	if( m_ListenMode )										// Manual Mode
	{
		m_btnAutomatic.SetCheck(BST_UNCHECKED);
		m_btnManual.SetCheck(BST_CHECKED);
	}
	else													// Automatic Mode
	{
		m_btnAutomatic.SetCheck(BST_CHECKED);
		m_btnManual.SetCheck(BST_UNCHECKED);
	}

	m_btnAutomatic.EnableWindow(TRUE);						// Make it so the user can select automatic again

	m_btnGetVolume.EnableWindow(m_pSelectedItem != NULL && m_TalkMode == TALK_SELECTED);
	m_btnSetVolume.EnableWindow(m_pSelectedItem != NULL);
	m_btnTestVolume.EnableWindow(m_pSelectedItem != NULL && GetFileAttributes(theApp.UserOptions.testVolumeFile)!=INVALID_FILE_ATTRIBUTES);

	showHdxFdxControls();
}

void CTalkMasterConsoleDlg::OnBnClickedRadioGroup()
{
	if( m_TalkMode == TALK_SELECTED )							// If the hold value has not been set, set it to listenMode
		m_ListenModeHold = m_ListenMode;

	m_btnListen.EnableWindow(FALSE);

	m_TalkMode = TALK_GROUP;
	m_btnManual.SetCheck(BST_CHECKED);
	m_btnAutomatic.SetCheck(BST_UNCHECKED);

	m_ListenMode = LISTEN_MANUAL;

	m_btnAutomatic.EnableWindow(FALSE);						// Make it so the user can not select automatic

	m_btnSetVolume.EnableWindow(TRUE);
	m_btnGetVolume.EnableWindow(FALSE);

	showHdxFdxControls();
}

void CTalkMasterConsoleDlg::OnBnClickedRadioAllActive()
{
	if( m_TalkMode == TALK_SELECTED )							// If the hold value has not been set, set it to listenMode
		m_ListenModeHold = m_ListenMode;

	m_btnListen.EnableWindow(FALSE);

	m_TalkMode = TALK_ALL_ACTIVE;
	m_btnManual.SetCheck(BST_CHECKED);
	m_btnAutomatic.SetCheck(BST_UNCHECKED);

	m_ListenMode = LISTEN_MANUAL;

	m_btnAutomatic.EnableWindow(FALSE);						// Make it so the user can not select automatic

	m_btnSetVolume.EnableWindow(TRUE);
	m_btnGetVolume.EnableWindow(FALSE);

	showHdxFdxControls();
}

void CTalkMasterConsoleDlg::OnBnClickedRadioAutomatic()
{
	m_ListenMode = LISTEN_AUTO;
	m_ListenModeHold = -1;									// This means that this mode is the permanent mode

	m_btnAllActive.SetCheck(BST_UNCHECKED);
	m_btnGroup.SetCheck(BST_UNCHECKED);
	m_btnSelected.SetCheck(BST_CHECKED);
	m_TalkMode = TALK_SELECTED;

	theApp.UserOptions.listenModeDefault = LISTEN_AUTO;
}

void CTalkMasterConsoleDlg::OnBnClickedRadioManual()
{
	m_ListenMode = LISTEN_MANUAL;
	m_ListenModeHold = -1;									// This means that this mode is the permanent mode

	theApp.UserOptions.listenModeDefault = LISTEN_MANUAL;
}

void CTalkMasterConsoleDlg::OnWindowPosChanging(WINDOWPOS* lpwndpos)
{
	CDialog::OnWindowPosChanging(lpwndpos);
}

void CTalkMasterConsoleDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	lpMMI->ptMinTrackSize.x = 567;
	lpMMI->ptMinTrackSize.y = 438;

	CDialog::OnGetMinMaxInfo(lpMMI);
}

LRESULT CTalkMasterConsoleDlg::OnSystemTray(WPARAM wParam, LPARAM lParam) 
{
	if( wParam == WM_USER+0x1000 && lParam == WM_LBUTTONUP )	// && m_min )
	{
		forceForegroundWindow();

		if( IsIconic() )
			ShowWindow(SW_RESTORE);
		else
			ShowWindow(SW_SHOW);

//		ShowWindow(SW_SHOW);
//
//		m_listCallsWaiting.SetFocus();

	}

	return(TRUE);
}

void CTalkMasterConsoleDlg::OnBtnDnButtonNoListen()
{
//	if( theApp.UserOptions.PTT && !bListening )
//		doListen();
}

void CTalkMasterConsoleDlg::OnBtnUpButtonNoListen()
{
	if( m_sessionSocket == 0 || isCallQueue(m_sessionSocket) )
		return;

	if( bTalking )
	{
		bTalkRequest = FALSE;
		bListenRequest = FALSE;					// Should already be this

		m_pReleasedItem = m_pSelectedItem;
		m_nReleaseSocket = m_sessionSocket;

		bReleaseRequest = TRUE;					// Release the selected intercom when idle
		return;
	}

	if( m_pSelectedItem )
		m_pSelectedItem->bSelected = FALSE;		// Not selected anymore

	deselectSocket( m_sessionSocket );

	outputDebug("OnBtnUpButtonNoListen: Clearing sessionSocket from %d to 0", m_sessionSocket);

	m_sessionSocket = 0;
	SetHoldEnd(FALSE, FALSE);

	m_btnNoListen.SetBitmap( HBITMAP(mBitMapNoListenOff) );

	lockControls(FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE);		// LOCK Talk, Listen, Chime, PlayFile, Radio Buttons
}

void CTalkMasterConsoleDlg::OnBtnDnButtonListen()
{
//	if( theApp.UserOptions.PTT && !bListening )
//		doListen();
}

void CTalkMasterConsoleDlg::OnBtnUpButtonListen()
{
	outputDebug(_T("OnBtnUpButtonListen: bTalkRequest cleared"));

	bTalkRequest = FALSE;								// On Listen UP, Talk = NO

	if( bListening )									// Not in single intercom mode, or we are listening
	{
//
// If this record is a handset record, we need to change the TALK button back into a CALL button and turn off END and TALK
//
		if( m_pSelectedItem && !m_pSelectedItem->bLocalRecord &&
			m_pSelectedItem->iCom.picRev.feature2 & OPNIC_2_HANDSET )
		{
			m_btnTalkSessionStart.ShowWindow(SW_SHOW);
			m_btnTalkSessionEnd.ShowWindow(SW_HIDE);
			m_btnTalk.ShowWindow(SW_HIDE);

//			outputDebug("Hide Talk");
		}

		bListenRequest = FALSE;								// But Listen = YES

		while( bListening )
			DoEvents();
	}
	else
	{
//
// If this record is a handset record, we need to change the CALL button back into a TALK button and turn off END and CALL
//
		if( m_pSelectedItem &&
			m_pSelectedItem->iCom.picRev.feature2 & OPNIC_2_HANDSET )
		{
			m_btnTalkSessionStart.ShowWindow(SW_HIDE);
			m_btnTalkSessionEnd.ShowWindow(SW_HIDE);
			m_btnTalk.ShowWindow(SW_SHOW);

//			outputDebug("OnBtnUpButtonListen: Show Talk");

			if( bSessionRequest )
			{
				bSessionRequest = FALSE;
				bSession = TRUE;

				while( bSession )
				{
					bSessionRequest = FALSE;
					DoEvents();
					Sleep(0);
				}
			}
		}

		bListenRequest = TRUE;

		while( !bListening && bListenRequest )
		{
			DoEvents();
			Sleep(0);
		}
	}

//	doListen();

	resetFocus();
//	m_listCallsWaiting.SetFocus();
}

void CTalkMasterConsoleDlg::OnBtnDnButtonTalk()
{
//
// If this record is a handset record, we need to change the CALL button back into a TALK button and turn off END and CALL
//
	if( m_pSelectedItem && m_pSelectedItem->iCom.picRev.feature2 & OPNIC_2_HANDSET )	// Must turn OFF the CALL button and enable the TALK button
	{
//		outputDebug("OnBtnDnButtonTalk: Hide Talk");
		m_btnTalkSessionStart.ShowWindow(SW_HIDE);
		m_btnTalkSessionEnd.ShowWindow(SW_HIDE);
		m_btnTalk.ShowWindow(SW_SHOW);
	}

	if( theApp.UserOptions.PTT && !bTalking )
	{
		bTalkRequest = TRUE;
		bListenRequest = FALSE;

		outputDebug(_T("OnBtnDnButtonTalk: bTalkRequest Set"));
	}
}

void CTalkMasterConsoleDlg::OnBtnUpButtonTalk()
{
	if( theApp.UserOptions.PTT )
	{
		outputDebug(_T("OnBtnUpButtonTalk: PTT, bTalkRequest cleared"));
		bTalkRequest = FALSE;

		if( m_TalkMode == TALK_SELECTED &&
			m_pSelectedItem && !(m_pSelectedItem->iCom.GPIO & OPMODE_REMLISTN_DISA) &&
			m_ListenMode == LISTEN_AUTO )
			bListenRequest = TRUE;						// Only set this on non-blocked intercoms
		else
			bListenRequest = FALSE;

		while( bTalking )						// Seems to kee
			DoEvents();
	}
	else
	{
		if( bTalking )
		{
			bTalkRequest = FALSE;

			if( m_TalkMode == TALK_SELECTED &&
				m_pSelectedItem && !(m_pSelectedItem->iCom.GPIO & OPMODE_REMLISTN_DISA) &&
				m_ListenMode == LISTEN_AUTO )
				bListenRequest = TRUE;					// Only set this on non-blocked intercoms (remote listen disable)
			else
				bListenRequest = FALSE;

			while( bTalking )
				DoEvents();
		}
		else
		{
			bTalkRequest = TRUE;
			bListenRequest = FALSE;
			while( !bTalking )
				DoEvents();
		}

		outputDebug(_T("OnBtnUpButtonTalk: noPTT, bTalkRequest %s"), (bTalkRequest)?_T("Set"):_T("Cleared"));
	}

	resetFocus();
//	m_listCallsWaiting.SetFocus();
}

void CTalkMasterConsoleDlg::OnBnDisableTalk()
{
	m_btnTalk.SetBitmap( HBITMAP(mBitMapTalkDisable) );
}

void CTalkMasterConsoleDlg::OnBnDisableListen()
{
	m_btnListen.SetBitmap( HBITMAP(mBitMapListenDisable) );
}

void CTalkMasterConsoleDlg::OnKeyDown(UINT nChar,UINT nRepCnt,UINT nFlags)
{
#if 1
	if( nChar == ' ' && nRepCnt == 1 && !m_playFileSocket )
	{
		if( m_pSelectedItem && !theApp.UserOptions.forceHdx &&
			m_pSelectedItem->iCom.picRev.feature1 & OPNIC_FULL_DUPLEX )
			OnBtnDnButtonFdxStart();
		else
			OnBtnDnButtonTalk();
	}
#else
	if( theApp.UserOptions.PTT && nChar == ' ' && nRepCnt == 1 && !m_playFileSocket )
	{
//		outputDebug(_T("OnKeyDown: doTalk() nTalkCount=%d"), m_nTalkCount);

#if 0
		if( (!bTalking && m_nTalkCount==1)  )
			m_nTalkCount = 1;					// No talking yet
		else if ( !bTalking && m_nTalkCount==2 )
			m_nTalkCount = 1;
		else if ( !bTalking && m_nTalkCount==0 )
			m_nTalkCount = 1;
		else if ( bTalking && m_nTalkCount==1 )
			m_nTalkCount = 2;
		else if ( bTalking && m_nTalkCount==2 )
			m_nTalkCount = 2;
		else if ( bTalking && m_nTalkCount==0 )
			m_nTalkCount = 0;
#else
		outputDebug(_T("OnKeyDown: bTalkRequest Set"));
		bTalkRequest = TRUE;

		while( !bTalking )
			DoEvents();
#endif
	}
#endif
}

void CTalkMasterConsoleDlg::OnKeyUp(UINT nChar,UINT nRepCnt,UINT nFlags)
{
//	outputDebug(_T("OnKeyUp: bTalking=%d nTalkCount=%d"), bTalking, m_nTalkCount);

	if( bLoggedOn )
	{
		if( nChar == ' ' && !m_playFileSocket )
		{
//			outputDebug(_T("OnKeyUp: doTalk() repCnt=%d"), nRepCnt);
#if 0
			if( (bTalking && m_nTalkCount==1) )
				m_nTalkCount = 1;
			else if ( bTalking && m_nTalkCount==2 )
				m_nTalkCount = 1;
			else if ( bTalking && m_nTalkCount==0 )
				m_nTalkCount = 1;
			else if ( !bTalking && m_nTalkCount==1 )
				m_nTalkCount = 2;
			else if ( !bTalking && m_nTalkCount==2 )
				m_nTalkCount = 2;
			else if ( !bTalking && m_nTalkCount==0 )
				m_nTalkCount = 0;
#else
#if 1
			if( m_pSelectedItem && !theApp.UserOptions.forceHdx &&
				m_pSelectedItem->iCom.picRev.feature1 & OPNIC_FULL_DUPLEX )
				OnBtnUpButtonFdxStart();
			else
				OnBtnUpButtonTalk();
#else
			if( bTalking || theApp.UserOptions.PTT )
				bTalkRequest = FALSE;
			else
				bTalkRequest = TRUE;
#endif

			outputDebug(_T("OnKeyUp: bTalkRequest %s"), (bTalkRequest)?_T("Set"):_T("Cleared"));
#endif
		}
		else if ( nChar == 'C' )
		{
			if( m_pSelectedItem &&
				m_pSelectedItem->iCom.picRev.feature2&OPNIC_2_HANDSET )
				OnBnClickedButtonSessionStart();
		}
		else if ( nChar == 'L' )
		{
			if( !m_pSelectedItem || theApp.UserOptions.forceHdx ||
				!(m_pSelectedItem->iCom.picRev.feature1 & OPNIC_FULL_DUPLEX) )
				OnBtnUpButtonListen();
		}
//			doListen();
		else if ( nChar == 8 )
			OnBnClickedButtonHold();
		else if ( nChar == 0x1b )
			doBnClickedButtonEnd();
		else if ( nChar == 0xd )
			doOpenDoor(m_pSelectedItem);
	}
}

BOOL CTalkMasterConsoleDlg::waitClear(int socket, unsigned fromStatus)
{
	struct _itemData *tItemData;
	int	count = 0;
	char buffer[32], buffer2[32];

	tItemData = findItemData(socket);

	if( !tItemData )
	{
		outputDebug(_T("No Waiting - No Item"));
		return(TRUE);
	}

	outputDebug(_T("waitClear: Waiting for socket %d to clear from status %s (%d). Current status is %s (%d)"),
		socket, szStatus(fromStatus, buffer), fromStatus, szStatus(tItemData->iCom.status, buffer2), tItemData->iCom.status );

	if( (tItemData->iCom.status&STATUS_MASK) == STATUS_IDLE )
		outputDebug(_T("No Waiting - already clear"));

	while ( bLoggedOn && 
			(tItemData->iCom.status&STATUS_MASK) && 
			( (tItemData->iCom.status&STATUS_MASK) == STATUS_COMMAND ||
			  (tItemData->iCom.status&STATUS_MASK) == fromStatus ) )
	{
		count++;
//		DoEvents();
		Sleep(10);

		if( count > ((bSlow)?400:200) )				// 1 seconds to wait???  too long
		{
			outputDebug(_T("waitClear: Timed out waiting - status = %s"), szStatus(tItemData->iCom.status, buffer));

			if( fromStatus == STATUS_LISTENING || (tItemData->iCom.status&STATUS_MASK) == STATUS_LISTENING )
			{
				bListening = TRUE;
				endListen(socket);
				bListening = FALSE;			// This may cause call queue entries...  jdnjdnjdn
			}
//			else if( fromStatus == STATUS_TALKING )
			{
				bTalking = TRUE;
				endTalk(socket);
			}

			bSlow = TRUE;					// Do things a bit slower so we don't get into this state as often
			Sleep(500);

			bTalking = FALSE;
//			bListening = FALSE;				// Trying to not get the call queue entry while hitting TALK jdnjdnjdn

			outputDebug(_T("waitClear: Timeout waiting to clear"));
			return(FALSE);
		}
	}

	nTalkingLevel = 0;
	drawVUMeter( &m_staticVUMeterMicrophone, bTalking, 0 );
	nListeningLevel = 0;
	drawVUMeter( &m_staticVUMeterSpeaker, bListening, 0 );

	return(TRUE);
}

BOOL CTalkMasterConsoleDlg::waitSet(int socket, unsigned toStatus)
{
	struct _itemData *tItemData;
	int	count = 0;
	char buffer[128];

	tItemData = findItemData(socket);

	if( !tItemData )
	{
		outputDebug(_T("No Waiting for set - No Item"));
		return(TRUE);
	}

	if( (tItemData->iCom.status&STATUS_MASK) != STATUS_IDLE && (tItemData->iCom.status&STATUS_MASK) != STATUS_COMMAND)
		outputDebug(_T("No Waiting - already set to %s"), szStatus(tItemData->iCom.status, buffer));

	while(bLoggedOn && (tItemData->iCom.status&STATUS_MASK) &&
		 (tItemData->iCom.status&STATUS_MASK) != toStatus )
	{
		count++;
//		DoEvents();
		Sleep(10);

		if( count > ((bSlow)?400:200) )				// 1 seconds to wait???  too long
		{
			outputDebug(_T("waitSet: Timed out waiting - status = %s"), szStatus(tItemData->iCom.status, buffer));

//			else if( toStatus == STATUS_LISTENING || tItemData->iCom.status == STATUS_LISTENING )
			{
				bListening = TRUE;
				endListen(socket);
			}
//			if( toStatus == STATUS_TALKING )
			{
				bTalking = TRUE;
				endTalk(socket);
			}

			bSlow = TRUE;					// Do things a bit slower so we don't get into this state as often
			Sleep(500);

			bListening = FALSE;
			bTalking = FALSE;

//			waitClear(socket);

			outputDebug(_T("waitSet: Timeout waiting for set"));
			return(FALSE);
		}
	}

	return(TRUE);
}


void CTalkMasterConsoleDlg::DoEvents()
{
	MSG msg;

	while( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

/**************************************************************** 
  WH_KEYBOARD hook procedure 
 ****************************************************************/ 
 
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) 
{ 
	static int guard=0;
	BOOL eatKey=FALSE;

	CTalkMasterConsoleDlg *dlg = (CTalkMasterConsoleDlg*)theApp.m_pMainWnd;

//    CHAR szBuf[128]; 
//    HDC hdc; 
//    static int c = 0; 
//    size_t cch; 
//	size_t * pcch;
//	HRESULT hResult;
 
    if (nCode < 0 || dlg->bOffDialog )  // do not process message 
        return CallNextHookEx(dlg->m_hookKb, nCode, wParam, lParam); 

	guard++;
 
	if ( (wParam == ' ' || wParam == 0x1b || wParam == 'C' || wParam == 'L' || wParam == 8 || wParam == 0xd ) && guard == 1 && !dlg->m_playFileSocket )
	{
		if( (lParam & 0xffff)==1 && lParam & 0x80000000 )
		{
			eatKey = TRUE;
			if( !dlg->bSkipNextKeyUp )
				dlg->OnKeyUp(wParam, lParam&0xffff, 0);
		}
		else if( (lParam & 0xffff)==1 && !(lParam & 0xC0000000) )
		{
			eatKey = TRUE;
			dlg->OnKeyDown(wParam, lParam&0xffff, 0);
		}

		eatKey = TRUE;			// Added always eating keys so we don't shoot the space bar into the checkbox when held down
								// Probably do not need it only done above since we have the off dialog stuff anyways
//		dlg->outputDebug(_T("KeyboardProc: key=%x lParam=%08x"), wParam, lParam);
	}

	dlg->bSkipNextKeyUp = FALSE;

	guard--;

	if( wParam == 0x1b || wParam == 13 )
		return (-1);

	if( !eatKey )
		return CallNextHookEx(dlg->m_hookKb, nCode, wParam, lParam); 
	else
		return -1;
} 

void CTalkMasterConsoleDlg::OnLvnKeydownListIcoms(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLVKEYDOWN pLVKeyDown = reinterpret_cast<LPNMLVKEYDOWN>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;
}

void CTalkMasterConsoleDlg::OnLvnItemActivateListCallsWaiting(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMIA = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;
}

void CTalkMasterConsoleDlg::OnNMRclickListCallsWaiting(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	*pResult = 0;
}

void CTalkMasterConsoleDlg::activatePlayer( BOOL active )
{
	m_btnPlayPause.EnableWindow(active);
	m_btnStop.EnableWindow(bPlaying);
	m_btnMute.EnableWindow(active);
	m_sliderCQPlayer.EnableWindow(active);
}
void CTalkMasterConsoleDlg::OnBnClickedButtonPlayPause()
{
	// TODO: Add your control notification handler code here

	char buf[WAVE_OUT_BUF_SIZE];
	int	readLen;

#define READRATE	((1000*sizeof(buf))/8000)
//#define	READRATE	64

	bPlaying ^= TRUE;

#if 0		// This code will not get run because it is done at the click
	if( !audioArchive.isOpen(m_pCallWaitingItem->hCallQueue) )
	{
		int totalSize = audioArchive.Open(m_pCallWaitingItem->hCallQueue);

		m_sliderCQPlayer.SetPos(0);				// On the left to start
		m_sliderCQPlayer.SetRange(0, totalSize/8000);
	}
#endif

	if( bPlaying )
	{
		readLen = audioArchive.Read(m_pCallWaitingItem->hCallQueue, buf, sizeof(buf));
		audioCQPlayer.Write((PUCHAR)buf, readLen, audioArchive.GetAudioFormat(m_pCallWaitingItem->hCallQueue));

		SetTimer(TIMER_CQPLAY, READRATE, NULL);

		m_btnPlayPause.SetBitmap( HBITMAP(mBitMapPause) );
	}
	else
	{
		m_btnPlayPause.SetBitmap( HBITMAP(mBitMapPlay) );
		KillTimer(TIMER_CQPLAY);
	}

	activatePlayer(TRUE);

	resetFocus();
//	m_listCallsWaiting.SetFocus();
}

void CTalkMasterConsoleDlg::playCallQueue()
{
	char buf[WAVE_OUT_BUF_SIZE];
	int readLen;

	if( m_pCallWaitingItem && bPlaying )			// Why would this NOT be set?
	{
		readLen = audioArchive.Read(m_pCallWaitingItem->hCallQueue, buf, sizeof(buf));
		if( readLen )
		{
			audioCQPlayer.Write((PUCHAR)buf, readLen, audioArchive.GetAudioFormat(m_pCallWaitingItem->hCallQueue));
			m_sliderCQPlayer.SetPos(audioArchive.Tell(m_pCallWaitingItem->hCallQueue)/8000);
			if( readLen < sizeof(buf) )
				audioCQPlayer.Flush();
		}
		else
		{
			audioCQPlayer.Flush();
			bPlaying = FALSE;

			KillTimer(TIMER_CQPLAY);

			audioArchive.Close(m_pCallWaitingItem->hCallQueue);
			audioArchive.Open(m_pCallWaitingItem->hCallQueue);

			activatePlayer(TRUE);

			m_sliderCQPlayer.SetPos(0);				// On the left to start
			m_btnPlayPause.SetBitmap( HBITMAP(mBitMapPlay) );
		}
	}
}

void CTalkMasterConsoleDlg::OnBnClickedButtonStop()
{
	// TODO: Add your control notification handler code here

	bPlaying = FALSE;

	m_btnPlayPause.SetBitmap( HBITMAP(mBitMapPlay) );

	KillTimer(TIMER_CQPLAY);

	audioCQPlayer.Flush();
	bPlaying = FALSE;

//	audioArchive.Close(m_pCallWaitingItem->hCallQueue);

	audioArchive.Close(m_pCallWaitingItem->hCallQueue);
	audioArchive.Open(m_pCallWaitingItem->hCallQueue);

	m_sliderCQPlayer.SetPos(0);				// On the left to start
	activatePlayer(TRUE);

	resetFocus();
//	m_listCallsWaiting.SetFocus();
}

void CTalkMasterConsoleDlg::OnBnClickedButtonMute()
{
	// TODO: Add your control notification handler code here

	bMuted ^= TRUE;

	if( bMuted )
		m_btnMute.SetBitmap( HBITMAP(mBitMapMuteOn) );
	else
		m_btnMute.SetBitmap( HBITMAP(mBitMapMuteOff) );

	audioCQPlayer.Mute(bMuted);
	resetFocus();
//	m_listCallsWaiting.SetFocus();
}

void CTalkMasterConsoleDlg::OnNMCustomdrawSliderCqPlayer(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Add your control notification handler code here

	resetFocus();
//	m_listCallsWaiting.SetFocus();

	*pResult = 0;
}

void CTalkMasterConsoleDlg::OnNMKillfocusListCallsWaiting(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	CWnd *ctrl = GetFocus();

//	OutputDebugString(_T("Kill Focus"));

	if( ctrl )
	{
		int	ctrlID = ctrl->GetDlgCtrlID();

		if( !(	ctrlID == IDC_BUTTON_PLAY_PAUSE ||
				ctrlID == IDC_BUTTON_STOP ||
				ctrlID == IDC_BUTTON_MUTE ||
				ctrlID == IDC_SLIDER_CQ_PLAYER) )
		{
			activatePlayer(FALSE);

			if( m_pCallWaitingItem && m_pCallWaitingItem->hCallQueue )
			{
				audioArchive.Close(m_pCallWaitingItem->hCallQueue);
				m_pCallWaitingItem = NULL;
			}
		}

		ctrl->SetFocus();
	}

	*pResult = 0;
}

void CTalkMasterConsoleDlg::OnNMSetfocusListCallsWaiting(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
//	activatePlayer(TRUE);
	*pResult = 0;
}

void CTalkMasterConsoleDlg::OnLvnItemchangedListCallsWaiting(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;
}

void CTalkMasterConsoleDlg::OnBnClickedButtonPlayFile()
{

	CString buffer;
//	HANDLE	hInfo;
//	char	*pInfo;

	if( m_playFileSocket )								// We are currently playing something
	{
		if( bPlayFileLocal )
			playFileLocal();
		else
			playFileServer();

		if( !m_playFileSocket )							// Stopped
			m_tabMain.EnableWindow(TRUE);

		return;
	}

	if( m_TalkMode == TALK_SELECTED && !m_pSelectedItem )
	{
		MessageBox(_T(getResourceString(IDS_STRING_SELECT_ICOM)), _T(getResourceString(IDS_STRING_INFORMATION)));
		resetFocus();
//		m_listCallsWaiting.SetFocus();
		return;
	}

	if( m_TalkMode == TALK_SELECTED && !m_pSelectedItem->iCom.status )
	{
		MessageBox(_T(getResourceString(IDS_STRING113)), _T(getResourceString(IDS_STRING114)));
		resetFocus();
		return;
	}

	if( m_TalkMode != TALK_SELECTED && countGroup() == 0 )
	{
		MessageBox(_T(getResourceString(IDS_STRING120)), _T(getResourceString(IDS_STRING121)));
		resetFocus();
		return;
	}

	if( m_tabMain.GetCurSel() == 1 )				// Groups and Messages Tab
	{
		playFileServer();
	}
	else
	{
		playFileLocal();
	}

	if( m_playFileSocket )
		m_tabMain.EnableWindow(FALSE);
}

void CTalkMasterConsoleDlg::playFileLocal()
{
//_T("Chart Files (*.xlc)|*.xlc|Worksheet Files (*.xls)|*.xls|Data Files (*.xlc;*.xls)|*.xlc; *.xls|All Files (*.*)|*.*||");
	
	CFileDialog pFD(	TRUE, 
						_T(getResourceString(IDS_STRING_WAV)),
						NULL,
						OFN_OVERWRITEPROMPT|OFN_HIDEREADONLY,
						getResourceString(IDS_STRING_WAV_FILE_FILTER),
						this);

	if( m_playFileSocket == 0 )
	{
		chdir(theApp.UserOptions.path);

		bOffDialog = TRUE;							// Not on the main dialog

		if( pFD.DoModal()!=IDOK )
		{
			bOffDialog = FALSE;						// Back on the main dialog
			bSkipNextKeyUp = TRUE;

			resetFocus();
			return;
		}

		bOffDialog = FALSE;							// Back on the main dialog
		bSkipNextKeyUp = TRUE;

		if( m_TalkMode == TALK_SELECTED )
			m_playFileSocket = m_pSelectedItem->socket;
		else
			m_playFileSocket = makeGroupSocket();

		if( !m_playFileSocket )
			{
			MessageBox(_T(getResourceString(IDS_STRING113)), _T(getResourceString(IDS_STRING114)));
			resetFocus();
			return;
			}

		if( !sendAudioFile(m_playFileSocket, pFD.GetPathName().GetBuffer(), TRUE, theApp.UserOptions.repeatPlay) )
		{
			if( m_TalkMode != TALK_SELECTED )
				m_DADeleteGroup(m_hDA, m_playFileSocket);		// Really can not do this since it														// will wait for the thread to end

			m_playFileSocket = 0;
			bPlayFileLocal = TRUE;
			resetFocus();
			return;
		}

	//		sendAudioFile(m_playFileSocket, _T("iAudio\\i_activated_us.wav"));

		m_btnPlayFile.SetWindowText(_T(getResourceString(IDS_STRING_STOP_FILE)));
		lockControls(TRUE, TRUE, TRUE, FALSE, TRUE, TRUE, TRUE, TRUE);
	}
	else
	{
		stopAudioFile(m_playFileSocket);
		m_btnPlayFile.SetWindowText(_T(getResourceString(IDS_STRING_PLAY_FILE)));

		lockControls(FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE);
//		selectSocket(m_playFileSocket, FALSE);

//		m_DADeleteGroup(m_hDA, m_playFileSocket);		// Don't call, already being deleted
//		m_playFileSocket = 0;
	}

	resetFocus();
}

void CTalkMasterConsoleDlg::playFileServer()
{
	if( m_playFileSocket == NULL )
	{
		if( m_listMessages.GetSelectedCount() == 0 )
		{
			MessageBox(_T(getResourceString(IDS_STRING_NO_MESSAGES_SELECTED)), _T(getResourceString(IDS_STRING_SELECT_MESSAGE)));
			resetFocus();
			return;
		}

		m_playFileSocket = makeGroupSocket();

		if( !playServerFile(m_playFileSocket, m_pCurrentMessage->key) )	// This function selects the intercom and returns if it can not
		{
			if( m_TalkMode != TALK_SELECTED )
				m_DADeleteGroup(m_hDA, m_playFileSocket);		// Really can not do this since it														// will wait for the thread to end

			m_playFileSocket = 0;
			bPlayFileLocal = TRUE;
		}
		else
		{
			m_btnPlayFile.SetWindowText(_T(getResourceString(IDS_STRING_STOP_FILE)));
			bPlayFileLocal = FALSE;								// We are playing something on the server
			lockControls(TRUE, TRUE, TRUE, FALSE, TRUE, TRUE, TRUE, TRUE);
		}
	}
	else					// We are playing something - stop it
	{
		stopAudioFile(m_playFileSocket);						// Stop this file on the server...
		m_btnPlayFile.SetWindowText(_T(getResourceString(IDS_STRING_PLAY_FILE)));

		m_playFileSocket = 0;
		bPlayFileLocal = TRUE;

		lockControls(FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE);
	}

	resetFocus();

}

//
// =======================================================================================
//
//	This function is fired when the listbox m_listIcoms entries are changed.
//
//  We are using this function to clear the checkbox on reconnect
//
// =======================================================================================
//

void CTalkMasterConsoleDlg::OnLvnItemchangedListIcoms(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	struct _itemData *tItemData;

	if( pNMLV->iItem > -1 )													// It really is for an item
	{
		tItemData = (struct _itemData *)getIcomItemData(pNMLV->iItem);		// Get the data associated with this item

		if( tItemData &&													// If there is a record assocated with this item
			tItemData->iCom.status == 0 &&									// and, the affected record is not online
			m_listIcoms.GetCheck(pNMLV->iItem) )							// and, it is checked
		{
			tItemData->bChecked = FALSE;									// uncheck the affected record
			m_listIcoms.SetCheck(pNMLV->iItem, FALSE);						// so it is not checked in the list box
		}
	}

	*pResult = 0;
}

void CTalkMasterConsoleDlg::OnBnClickedButtonChime()
{
	char	chimeFilename[MAX_PATH];

	if( m_TalkMode == TALK_SELECTED && !m_pSelectedItem )		// No intercom selected yet
	{
		MessageBox(_T(getResourceString(IDS_STRING_SELECT_ICOM)), _T(getResourceString(IDS_STRING_INFORMATION)));
		resetFocus();
//		m_listCallsWaiting.SetFocus();

		return;
	}

	if( m_TalkMode == TALK_SELECTED && !m_pSelectedItem->iCom.status )
	{
		MessageBox(_T(getResourceString(IDS_STRING113)), _T(getResourceString(IDS_STRING114)));
		resetFocus();
		return;
	}

	if( m_TalkMode != TALK_SELECTED && countGroup() == 0 )
	{
		MessageBox(_T(getResourceString(IDS_STRING120)), _T(getResourceString(IDS_STRING121)));
		resetFocus();
		return;
	}

	if( m_playFileSocket )
	{
		MessageBox(_T(getResourceString(IDS_STRING_CURRENTLY_PLAYING_AUDIO)));
		resetFocus();
//		m_listCallsWaiting.SetFocus();
		return;
	}

	if( m_TalkMode == TALK_SELECTED )
		m_playFileSocket = m_pSelectedItem->socket;
	else
		m_playFileSocket = makeGroupSocket();

	strcpy( chimeFilename, theApp.UserOptions.path );		// Build the chime filename that we are going to send
	strcat( chimeFilename, _T("\\iAudio\\page.wav") );

	if( !sendAudioFile(m_playFileSocket, chimeFilename, TRUE, FALSE) )	// This function selects the intercom and returns if it can not
	{
		if( m_TalkMode != TALK_SELECTED )
			m_DADeleteGroup(m_hDA, m_playFileSocket);		// Really can not do this since it														// will wait for the thread to end

		m_playFileSocket = 0;
		bPlayFileLocal = TRUE;
	}

	m_tabMain.EnableWindow(m_playFileSocket == 0);							// Don't allow tab changes until the audio chime has completed

	resetFocus();
}

void CTalkMasterConsoleDlg::deleteIcomsData()
{
	struct _itemData *item = m_pItemData;

	deleteIcoms(TRUE);									// Clear the list

	while(item)
	{
		m_pItemData = item->next;
		free(item);
		item = m_pItemData;
	}
}

void CTalkMasterConsoleDlg::deleteIcoms(BOOL bClearCount)
{
	struct _itemData *item = m_pItemData;

	while(item)
	{
		item->bDisplayed = FALSE;
		item = item->next;
	}

	m_listIcoms.DeleteAllItems();

	clearIcomItemData();

	m_nIntercomsConsole = 0;							// None are listed since all have been deleted

	if( bClearCount )
	{
		m_nIntercomsConsoleAlive = 0;						// None of these are there either
	}
}

void CTalkMasterConsoleDlg::deleteGroups()
{
	struct _groupDataList *pList = m_pGroupData;
	struct _groupItems *pItemNext;

	m_listGroups.DeleteAllItems();									// Clear the list

	while(pList)
	{
		m_pGroupData = pList->next;

		while(pList->pGroupItems)										// Must get rid of the intercoms associated with this also
		{
			pItemNext = pList->pGroupItems->next;

			if( pList->pGroupItems->bLocalIcomRecord )					// Groups created the record, we must delete it
				free(pList->pGroupItems->pIcomData);

			free(pList->pGroupItems);

			pList->pGroupItems = pItemNext;
		}

		if( pList->pIcomData )
			free(pList->pIcomData);

		if( pList->pGroupData )
			free(pList->pGroupData);

		free(pList);
		pList = m_pGroupData;
	}
}

void CTalkMasterConsoleDlg::deleteMessages()
{
	struct _messageDataList *pList = m_pMessageDataList;

	m_listMessages.DeleteAllItems();									// Clear the list

	while(pList)
	{
		m_pMessageDataList = pList->next;
		free(pList->pMessageData);
		free(pList);
		pList = m_pMessageDataList;
	}
}

void CTalkMasterConsoleDlg::resetDialog()
{
//	int i, count;
//	struct _itemData *tItemData;

	m_btnListen.SetBitmap( HBITMAP(mBitMapListenOff) );
	m_btnTalk.SetBitmap( HBITMAP(mBitMapTalkOff) );
	m_btnPlayPause.SetBitmap( HBITMAP(mBitMapPlay) );
	m_btnStop.SetBitmap( HBITMAP(mBitMapStop) );
	m_btnMute.SetBitmap( HBITMAP(mBitMapMuteOff) );

	m_btnSelected.SetCheck(BST_CHECKED);
	m_btnGroup.SetCheck(BST_UNCHECKED);
	m_btnAllActive.SetCheck(BST_UNCHECKED);

	m_btnAutomatic.SetCheck(BST_CHECKED);
	m_btnManual.SetCheck(BST_UNCHECKED);
	m_btnAutomatic.EnableWindow(TRUE);

	m_selectedRow = -1;				// None of the list is selected
	m_pSelectedItem = NULL;
	m_pReleasedItem = NULL;
	m_sessionSocket = 0;			// Not in a session with an intercom

	m_ListenMode = 0;
	m_TalkMode = 0;

	m_tabMain.SetCurSel(0);			// Select the main page
	displayTabMain();				// Display the main page

	doSize();

	m_bConsoleReady = FALSE;		// We have not connected yet

	bTalking = FALSE;
	bTalkRequest = FALSE;
	bListening = FALSE;
	bListenRequest = FALSE;
	bReleaseRequest = FALSE;		// No release request yet
	bSession = FALSE;
	bSessionRequest = FALSE;		// No Session request yet

	bMuting = FALSE;

	bNoRestart = FALSE;				// Allow listen restart again
	bSlow = FALSE;					// Assume a fast network

	bPlaying = FALSE;				// Not playing anything from the CQ list
	bMuted = FALSE;					// Not muted yet

	bLoggedOn = FALSE;				// Not logged on yet

	resetList = TRUE;
	resetCQList = TRUE;

	m_pCallWaitingItem = NULL;

	m_playFileSocket = 0;
	bPlayFileLocal = TRUE;

	m_tabMain.EnableWindow(TRUE);

	if( IsIconic() )
		ShowWindow(SW_RESTORE);

#if 0
// Reset Intercom list
	count = m_listIcoms.GetItemCount();
	for(i=0;i<count;i++)
	{
		tItemData = (struct _itemData *)getIcomItemData(0);
		if( tItemData )
			free(tItemData);

		m_listIcoms.DeleteItem(0);
	}
#endif

//	m_pItemData = NULL;
	m_nIntercomsConsole = 0;
	m_nIntercomsConsoleAlive = 0;

	m_listIcoms.InsertItem( LVIF_TEXT|LVIF_IMAGE, 0, _T(""), 0, 0, 10, NULL);
	m_listIcoms.SetItemText(0,COL_LOCATION, _T(getResourceString(IDS_STRING178)));
	m_listIcoms.SetItemText(0,COL_STATUS,_T(getResourceString(IDS_STRING179)));
	m_listIcoms.SetItemData(0, NULL);
	insertIcomItemData(0, NULL);

// Reset Call Queue

//	deleteAllCallQueue(TRUE);			// Done in DAControl.cpp during the shutdown

	audioPlayer.StopMic();

	resetFocus();
//	m_listCallsWaiting.SetFocus();
}

void CTalkMasterConsoleDlg::SetHoldEnd(BOOL bHold, BOOL bEnd)
{
	if( m_tabMain.GetCurSel() == 0 )
	{
		m_btnSessionEnd.EnableWindow(bEnd);
		m_btnSessionHold.EnableWindow(bHold);
	}

	m_bSaveEnd = bEnd;
	m_bSaveHold = bHold;
}

void CTalkMasterConsoleDlg::resetHoldEnd()
{
	if( m_bHeldSessionSocket )
		m_sessionSocket = m_holdSessionSocket;

	m_btnSessionEnd.EnableWindow(m_bSaveEnd);
	m_btnSessionHold.EnableWindow(m_bSaveHold);
}

void CTalkMasterConsoleDlg::OnBnClickedButtonEnd()
{
	doBnClickedButtonEnd();
}

void CTalkMasterConsoleDlg::doBnClickedButtonEnd(BOOL skipDeselect)
{
	struct _itemData *tCQItemData = NULL, *tItemData = NULL;
	int i=90;

	resetFocus();
//	m_listCallsWaiting.SetFocus();

	if( bPlaying )
		OnBnClickedButtonStop();

	m_sessionDead = TRUE;

	if( m_sessionSocket )
		tCQItemData = getCQItemDataFromSocket(m_sessionSocket);

	if( m_sessionSocket && (!tCQItemData || (tCQItemData && !tCQItemData->bXfer)) )
	{
//		if( !m_btnSessionEnd.IsWindowEnabled() )
//			return;

		if( m_pSelectedItem && !theApp.UserOptions.forceHdx &&
			m_pSelectedItem->iCom.picRev.feature1 & OPNIC_FULL_DUPLEX )
		{
			if( bTalking )
				doFdxStartStop();				// Stop it
		}
		else									// Half Duplex Stopping
		{
			bTalkRequest = FALSE;
			bListenRequest = FALSE;

			if( bTalking )
			{
				while( bTalking )
				{
					bTalkRequest = FALSE;
					bListenRequest = FALSE;
					DoEvents();											// Wait for the timer to shut down the Talk button
					Sleep(0);
				}

				lockControls(FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE);		// LOCK Talk, Listen, Chime, PlayFile, Radio Buttons
			}
			if( bListening )
			{
				while(bListening)					// This is the loop where we die.  jdnjdnjdn
				{
					bTalkRequest = FALSE;
					bListenRequest = FALSE;
					DoEvents();
					Sleep(0);
				}
#if 0
	//			doListen();
				endListen(m_sessionSocket);
				waitClear(m_sessionSocket, STATUS_LISTENING);
#endif

				bListening = FALSE;
				bNoRestart	= FALSE;									// Allow restarts again

				lockControls(FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE);		// LOCK Talk, Listen, Chime, PlayFile, Radio Buttons
			}

			if( bSessionRequest )
			{
				bSessionRequest = FALSE;
				bSession = TRUE;

				while( bSession )
				{
					bTalkRequest = FALSE;
					bListenRequest = FALSE;
					bSessionRequest = FALSE;
					DoEvents();
				}
			}
		}

		if( tCQItemData )
			tCQItemData->bSelected = FALSE;

		if( !skipDeselect )
		{
			if( !tCQItemData )
				tItemData = findItemData( m_sessionSocket );

			if( (tCQItemData && tCQItemData->iCom.picRev.opMode1 & OPMODE_REMLISTN_DISA) ||
				(tItemData && tItemData->iCom.picRev.opMode1 & OPMODE_REMLISTN_DISA) )
			{
				m_btnNoListen.SetBitmap( HBITMAP(mBitMapNoListenOff) );				// RLD needs to reset the bitmap
			}

			deselectSocket( m_sessionSocket );
		}

		deleteCallQueue(m_sessionSocket, TRUE, TRUE);		// Delete it from the local list & will clear m_sessionSocket

		activatePlayer(FALSE);

		m_sessionDead = FALSE;	// reset since we are no longer in a session
		outputDebug("doBnClickedButtonEnd: Cleared Session from %d to 0", m_sessionSocket);
		m_sessionSocket = 0;	// No longer in a session
		m_oldSessionSocket = 0;	// No longer have a held socket either

		m_selectedCQRow = -1;
		m_selectedRow = -1;		// Neither row is now selected

		SetHoldEnd(FALSE, FALSE);
	}

//
// If this record is a handset record, we need to change the TALK button back into a CALL button and turn off END and TALK
//
	if( tCQItemData && 
		tCQItemData->iCom.picRev.feature2 & OPNIC_2_HANDSET )
	{
		if( bSessionRequest )											// Only end the session request if we are currently requesting one
		{
			bSessionRequest = FALSE;
			bSession = TRUE;
		}

		m_btnTalkSessionEnd.ShowWindow(SW_HIDE);
		m_btnTalk.ShowWindow(SW_HIDE);
		m_btnTalkSessionStart.ShowWindow(SW_SHOW);

//		outputDebug("Hide Talk 2");
	}

	if( m_selectedCQRow == -1 &&
		m_listCallsWaiting.GetItemCount() &&
		getCallsWaitingItemData(0) )
	{
		selectIcomRow(selectCQRow(0, TRUE));
		time(&m_announceGuardTime);
	}
	else
		showHdxFdxControls();

	resetAnnounce();

	if( testIcomList() )
		redisplayIcomList();
}

void CTalkMasterConsoleDlg::OnBnClickedButtonHold()
{
	struct _itemData *tItemData;
	int i=2;

	if( !m_btnSessionHold.IsWindowEnabled() || !m_pSelectedItem )
		return;

	resetFocus();
//	m_listCallsWaiting.SetFocus();

	if( !m_sessionSocket && m_pSelectedItem )
	{
		tItemData = findItemData(m_pSelectedItem->socket);

		if( tItemData && tItemData->iCom.status && !selectSocket(m_pSelectedItem->socket) )
		{
			m_pSelectedItem->bSelected = FALSE;					// Track whether this item is selected
			if( !tItemData || !tItemData->iCom.status )
				MessageBox(_T(getResourceString(IDS_STRING117)), _T(getResourceString(IDS_STRING_INFORMATION)));
			return;
		}

		m_pSelectedItem->bSelected = TRUE;					// Track whether this item is selected

		m_sessionSocket = m_pSelectedItem->socket;			// Get the socket # of this intercom

		SetHoldEnd(FALSE, FALSE);							// Until the transfer is complete
		StartLocalCQSession(m_sessionSocket, TRUE);			// That is if it is there at all (HELD)
	}
	else
	{
		tItemData = findItemData(m_sessionSocket);

		if( tItemData && tItemData->bLocalRecord )
		{
			if( m_pSelectedItem && !theApp.UserOptions.forceHdx &&
				m_pSelectedItem->iCom.picRev.feature1 & OPNIC_FULL_DUPLEX )
			{
				if( bTalking )
					doFdxStartStop();				// Stop it
			}
			else									// Half Duplex
			{
				bListenRequest = FALSE;
				bTalkRequest = FALSE;

				if( bTalking )
				{
					while( bTalking )
					{
						bTalkRequest = FALSE;
						bListenRequest = FALSE;
						DoEvents();
						Sleep(0);
					}

					lockControls(FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE);		// LOCK Talk, Listen, Chime, PlayFile, Radio Buttons
				}
				else if (bListening)
				{
					while( bListening )
					{
						bTalkRequest = FALSE;
						bListenRequest = FALSE;
						DoEvents();
						Sleep(0);
					}

	//				doListen();												// bListenRequest = FALSE;
	//				endListen();
	//				waitClear(m_sessionSocket);
					lockControls(FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE);		// LOCK Talk, Listen, Chime, PlayFile, Radio Buttons
				}
			}

			MarkCallQueueSession(m_sessionSocket, TRUE);			// Held
		}

		SetHoldEnd(FALSE, TRUE);
	}

}

void CTalkMasterConsoleDlg::updateAdminMenus(UINT flags)
{
	CMenu *menu;
	CMenu *sub;
	UINT check = MF_CHECKED;

	MENUITEMINFO prefMenu = { 
		sizeof(MENUITEMINFO),
		MIIM_ID | MIIM_TYPE,
		MFT_STRING,
		NULL,
		ID_VIEW_PREFERENCES,
		NULL,
		NULL,
		NULL,
		NULL,
		_T(getResourceString(IDS_STRING_PREFERENCES).GetBuffer()),
		getResourceString(IDS_STRING_PREFERENCES).GetLength()+1,
//		NULL
	};

	MENUITEMINFO curMenu;
	curMenu.cbSize = sizeof (MENUITEMINFO); // must fill up this field
	curMenu.fMask = MIIM_STATE;             // get the state of the menu item

	menu = theApp.GetMainWnd()->GetMenu();

	if( menu )
	{
		sub = menu->GetSubMenu(1);

		if( !(flags & DA_ADMIN) )
		{
			if( sub->GetMenuItemInfo(ID_VIEW_PREFERENCES, &curMenu) )
				sub->RemoveMenu(1, MF_BYPOSITION);
		}
		else
		{
			if( !sub->GetMenuItemInfo(ID_VIEW_PREFERENCES, &curMenu) )
				sub->InsertMenuItem(1, &prefMenu, TRUE);
		}
	}

}

void CTalkMasterConsoleDlg::updateMenus(UINT flags)
{
	CMenu *menu;
	CMenu *sub;
	UINT check = MF_CHECKED;

	if( flags & DA_REMOTE )
	{
		menu = theApp.GetMainWnd()->GetMenu();
		if( menu )
		{

			sub = menu->GetSubMenu(4);
#ifdef PRIMEX_BUILD
			sub->ModifyMenu(ID_HELP_HELPONUSINGTALKMASTER, MF_BYCOMMAND | MF_STRING, ID_HELP_HELPONUSINGTALKMASTER, _T(getResourceString(IDS_STRING181)));
#endif
			sub->ModifyMenu(ID_HELP_DIGITALACOUSTICSWEBSITE, MF_BYCOMMAND | MF_STRING, ID_HELP_DIGITALACOUSTICSWEBSITE, theApp.UserOptions.onlineWebMenu);

			sub = menu->GetSubMenu(0);
			sub->RemoveMenu(0, MF_BYPOSITION);
			sub->RemoveMenu(0, MF_BYPOSITION);
			sub->RemoveMenu(0, MF_BYPOSITION);

			sub = menu->GetSubMenu(2);
			sub->RemoveMenu(6, MF_BYPOSITION);			// 5 now that there is a send text menu item
			sub->RemoveMenu(6, MF_BYPOSITION);
			sub->RemoveMenu(6, MF_BYPOSITION);
			sub->RemoveMenu(6, MF_BYPOSITION);
			sub->RemoveMenu(6, MF_BYPOSITION);

			sub = menu->GetSubMenu(4);
			sub->RemoveMenu(5, MF_BYPOSITION);
			sub->RemoveMenu(5, MF_BYPOSITION);

			menu->RemoveMenu(3, MF_BYPOSITION);
		}
	}
	else
	{
		menu = theApp.GetMainWnd()->GetMenu();
		if( menu )
		{
			sub = menu->GetSubMenu(0);
			sub->RemoveMenu(3, MF_BYPOSITION);
			sub->RemoveMenu(3, MF_BYPOSITION);
			sub->RemoveMenu(3, MF_BYPOSITION);
		}
	}

	struct _stat fileStat;
	if( _stat("c:\\DATestFile.txt", &fileStat) != 0 )
	{
		sub = menu->GetSubMenu(2);			// Tools menu
		sub->RemoveMenu(3, MF_BYPOSITION);
		sub->RemoveMenu(3, MF_BYPOSITION);
		sub->RemoveMenu(3, MF_BYPOSITION);
	}
}

//
// testIcomList
//
// Test whether the IcomList should be redisplayed
//
BOOL CTalkMasterConsoleDlg::testIcomList()
{
	struct _itemData *item = m_pItemData;
	struct _itemData *itemCurrentCQ = NULL;
	struct _iComQueueList *listEntry;

	int	count=m_listIcoms.GetItemCount(), items = 0;
	BOOL bDisplayIcom;

	if( m_selectedCQRow > (m_listCallsWaiting.GetItemCount()-1) )		// See if we deleted the one it is pointing at
		m_selectedCQRow = -1;
	else if( m_selectedCQRow > -1 )
		itemCurrentCQ = (struct _itemData *)getCallsWaitingItemData(m_selectedCQRow);

	item = m_pItemData;
	while( item )
	{
		listEntry = findQueueMembership( &item->iComQueue );
		if( !listEntry || item == itemCurrentCQ )
			bDisplayIcom = TRUE;
		else if( listEntry->queueMembership.membership == 1 )
			bDisplayIcom = TRUE;
		else if (listEntry->queueMembership.membership == 2 && listEntry->queueMembership.bOverflow)
			bDisplayIcom = TRUE;
		else if (listEntry->queueMembership.bOverflow == 2)			// Forced EVERYONE list this queue
			bDisplayIcom = TRUE;
		else if (listEntry->queueMembership.bPriorityOverflow && item->iComQueue.priority < 10 && item->cq.time)
			bDisplayIcom = TRUE;
		else
			bDisplayIcom = FALSE;

		if( bDisplayIcom )
		{
			items++;

			if( !item->bDisplayed )						// If it is not already displayed, then redisplay everything
				return(TRUE);
		}

		item = item->next;
	}

	return(items != count);
}

void CTalkMasterConsoleDlg::redisplayIcomList()
{
	struct _itemData *item = m_pItemData;
	struct _itemData *itemCurrentCQ = NULL;
	struct _iComQueueList *listEntry;
	struct tm	atime;

	int	index, cqImage;
	BOOL bDisplayIcom;
	char buf[10], buffer[128];

//	outputDebug(_T("redisplayIcomList"));

	if( m_selectedCQRow > (m_listCallsWaiting.GetItemCount()-1) )		// See if we deleted the one it is pointing at
		m_selectedCQRow = -1;
	else if( m_selectedCQRow > -1 )
		itemCurrentCQ = (struct _itemData *)getCallsWaitingItemData(m_selectedCQRow);

//	deleteAllCallQueue(FALSE);				// Delete everything but the local ones
//	deleteIcoms();

//	m_nIntercomsConsole = 0;
//	m_nIntercomsConsoleAlive = 0;

	while( item )
	{
//		item->bDisplayed = FALSE;				// At least start it off not being displayed

		listEntry = findQueueMembership( &item->iComQueue );
		if( !listEntry || item == itemCurrentCQ )
			bDisplayIcom = TRUE;
		else if( listEntry->queueMembership.membership == 1 )
			bDisplayIcom = TRUE;
		else if (listEntry->queueMembership.membership == 2 && listEntry->queueMembership.bOverflow)
			bDisplayIcom = TRUE;
		else if (listEntry->queueMembership.bOverflow == 2)			// Forced EVERYONE list this queue
			bDisplayIcom = TRUE;
		else if (listEntry->queueMembership.bPriorityOverflow && item->iComQueue.priority > 0 && item->cq.time)
			bDisplayIcom = TRUE;
		else
			bDisplayIcom = FALSE;

		if( bDisplayIcom && item->bDisplayed == FALSE )
		{
			if( resetList )
			{
				deleteIcoms(TRUE);
				resetList = FALSE;
			}

			item->bDisplayed = TRUE;			// Going to be displayed, mark it so we can test for it above

			EnterCriticalSection(&CriticalSection); 

			m_nIntercomsConsole++;

			index = getInsert(item);
//			index = 0;

			m_listIcoms.InsertItem( LVIF_TEXT|LVIF_IMAGE, index, _T(""), 0, 0, 10, NULL);

//			m_listIcoms.SetItemData(index, (DWORD_PTR)item);		// Why are we still setting this when we have our own data list?
			insertIcomItemData(index, (void*)item);

			LeaveCriticalSection(&CriticalSection);

// Add to the intercom list

			m_listIcoms.SetItemText(index,COL_LOCATION, (LPCTSTR)item->iCom.name);
			m_listIcoms.SetItemText(index,COL_QUEUE, (LPCTSTR)item->iComQueue.queueName);
			m_listIcoms.SetItemText(index,COL_ADDRESS, inet_ntoa(item->iCom.addr_in.sin_addr));
			m_listIcoms.SetItemText(index,COL_STATUS, szStatus(item->iCom.status, buffer));

			if( item->iCom.status && item->bCounted == FALSE )
			{
				m_nIntercomsConsoleAlive++;
				item->bCounted = TRUE;
			}

			m_listIcoms.SetCheck(index, item->bChecked);

			if( item->iCom.picRev.opMode1 & OPMODE_RELAY )
				m_listIcoms.SetItem( index, COL_DOOR, LVIF_IMAGE, 0, 0, 0, 0, NULL );	// LVIS_SELECTED

// If there is a call queue record, add to the cq list too

			if( item->cq.time && (findCQIndexFromSocket(item->socket) == (-1)) )
			{
				if( resetCQList )
				{
					clearCallsWaitingItemData();
					m_listCallsWaiting.DeleteAllItems();
					resetCQList = FALSE;
				}

				index = getCQInsert(item);

				if(item->iComQueue.priority)
					cqImage = 2;
				else if( listEntry && listEntry->queueMembership.bOverflow )
					cqImage = 1;
				else
					cqImage = 0;

				index = m_listCallsWaiting.InsertItem(index, _T(""));

				m_listCallsWaiting.SetItem( index, 0, LVIF_IMAGE, 0, cqImage, 0, 0, NULL );	// LVIS_SELECTED
//				outputDebug(_T("redisplayIcomList: setItem # %d to %d"), index, cqImage);

				m_listCallsWaiting.SetItemText(index, CQ_COL_WAITING, (LPCTSTR)item->cq.name);
				m_listCallsWaiting.SetItemText(index,CQ_COL_DETAILS, item->cq.message);
				atime = *localtime( &item->cq.timeLength );
				sprintf(buf, _T("%d:%02d"), atime.tm_min, atime.tm_sec);
				m_listCallsWaiting.SetItemText(index,CQ_COL_TIME, buf);

				m_listCallsWaiting.SetItemData(index, (DWORD_PTR)item);
				insertCallsWaitingItemData(index, (void*)item);
			}

//			m_listIcoms.SortItems( &sortIcoms, (DWORD)&m_listIcoms );
		}
		else if( bDisplayIcom && item->bDisplayed )
		{
		}
		else if( !bDisplayIcom && item->bDisplayed && !item->bLocalRecord )
		{
			m_nIntercomsConsoleAlive--;
			m_nIntercomsConsole--;

			item->bCounted = FALSE;							// Not counted anymore

			item->bDisplayed = FALSE;

			if( (index = findIntercom(&item->iCom)) != -1 )
			{
				m_listIcoms.DeleteItem(index);
				deleteIcomItemData(index);
			}

			if( (index = findCQIndexFromSocket(item->socket)) != -1 )
			{
				m_listCallsWaiting.DeleteItem(index);
				deleteCallsWaitingItemData(index);
			}

		}
		else if( !bDisplayIcom && item->bDisplayed == FALSE )
		{
		}

		item = item->next;
	}

// Set timer information if not already set for the announcement stuff
	m_announceStartTime = oldestCQEntry();
	if( m_announceStartTime )
		SetTimer( TIMER_ANNOUNCE, 1000, NULL );				// Every second check

//
// Now reselect the icom row
//
	if( m_pSelectedItem )
	{
		index = findIndexFromSocket(m_pSelectedItem->socket);
		if( index > -1 )
		{
			m_listIcoms.SetItemState(index, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
			m_listIcoms.EnsureVisible(index, FALSE);
		}

		index = findCQIndexFromSocket(m_pSelectedItem->socket);
		if( index > -1 )
			selectCQRow(index, TRUE);
		else if( itemCurrentCQ )			// Intercom must have been removed from the list - reselect the cq row
		{
			index = findCQIndexFromSocket(itemCurrentCQ->socket);
			if( index > -1 )
				selectCQRow(index, TRUE);
		}
	}
	else if (!m_sessionSocket)
	{
		selectIcomRow( selectCQRow( 0, TRUE ) );
	}

	displayIntercomCount();
}
#if 0
int CALLBACK sortIcoms(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	CTalkMasterConsoleDlg *dlg = (CTalkMasterConsoleDlg*)theApp.m_pMainWnd;

	struct _itemData *item1 = (struct _itemData *)lParam1;
	struct _itemData *item2 = (struct _itemData *)lParam2;

	switch( dlg->m_sortBy )
	{
	case COL_GROUP:
		break;

	case COL_LOCATION:
		return( strcmp((char*)item2->iCom.name, (char*)item1->iCom.name) );

	case COL_STATUS:
		break;

	case COL_ADDRESS:
		return( memcmp(&item2->iCom.addr_in.sin_addr, &item1->iCom.addr_in.sin_addr, sizeof(in_addr)) );

	case COL_DOOR:
		break;

	case COL_NOTES:
		break;
	}

	return(0);
}
#endif

void CTalkMasterConsoleDlg::OnItemPostPaintLogo(NMHDR *pNMHDR, LRESULT *pResult)
{
	int i=0;
}

void CTalkMasterConsoleDlg::OnNMClickListIcoms(NMHDR *pNMHDR, LRESULT *pResult)
{
	int selectedCount, i, nItem=-1;
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	struct _itemData *tItemData;

	BOOL	bOldState = (pNMLV->uOldState & LVIS_SELECTED);
	BOOL	bNewState = (pNMLV->uNewState & LVIS_SELECTED);
	BOOL	bSelItem  = (pNMLV->iItem == m_selectedRow);

#if 0
	outputDebug(_T("OnNMClickListIcoms: OldState = %s, NewState = %s, Selected Item = %s"),
		bOldState?_T("TRUE"):_T("FALSE"),
		bNewState?_T("TRUE"):_T("FALSE"),
		bSelItem?_T("TRUE"):_T("FALSE"));
#endif

	if( pNMLV->iSubItem == COL_GROUP )
	{
		if( pNMLV->iItem > -1 )
		{
			tItemData = (struct _itemData *)getIcomItemData(pNMLV->iItem);		// Use a temp rather than
#if 1
			tItemData->bChecked = !m_listIcoms.GetCheck(pNMLV->iItem);		// the selected icom
#else
			if( tItemData->iCom.status )
				tItemData->bChecked = !m_listIcoms.GetCheck(pNMLV->iItem);		// the selected icom
			else
			{
				tItemData->bChecked = FALSE;									// the selected icom
				m_listIcoms.SetCheck(pNMLV->iItem, TRUE);
			}
#endif
			*pResult = 0;
		}
	}
	else if( (selectedCount = m_listIcoms.GetSelectedCount()) > 1 )									// Selecting more intercoms for the group list
	{
		for(i=0;i<selectedCount;i++)									// Loop through all of the selected items
		{
			nItem = m_listIcoms.GetNextItem(nItem, LVNI_SELECTED);		// Get the next one (-1 is set for the 1st one)
			
			tItemData = (struct _itemData *)getIcomItemData(nItem);		// Get the data pointer for this item
			tItemData->bChecked = !tItemData->bChecked;					// Invert the check
			
			m_listIcoms.SetCheck(nItem, tItemData->bChecked);			// the selected icom

//			m_listIcoms.SetItemState(nItem, 0, LVIS_SELECTED|LVIS_FOCUSED);	// Clear the selection for this row
		}

		shutDownIcomAndHold();											// Shut down the intercom and put on hold if in use
		loadIcomList(pNMHDR);											// Load the selected intercom (last keypress)

																		// Select and set as focus (where the cursor is pointed)
		m_listIcoms.SetItemState(pNMLV->iItem, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
		m_listIcoms.EnsureVisible(pNMLV->iItem, FALSE);					// Make sure it is on the page
	}
	else if( pNMLV->iSubItem == COL_DOOR && pNMLV->iItem != -1 )		// Ok, it was the door column, is it a relay call?
	{
		struct _itemData *tItemData;

		tItemData = (struct _itemData *)getIcomItemData(pNMLV->iItem);

		doOpenDoor(tItemData);
	}
//	else if( pNMLV->uOldState & LVIS_SELECTED && pNMLV->iItem == m_selectedRow )	//LVIS_FOCUSED
//	{
//		shutDownIcomAndHold();
//	}
	else if( pNMLV->uOldState & LVIS_SELECTED && pNMLV->iItem != m_selectedRow )
	{
		shutDownIcomAndHold();

		loadIcomList(pNMHDR);
	}

	setResetFocus(&m_listIcoms);
	m_listIcoms.SetFocus();
//	m_listCallsWaiting.SetFocus();
}

void CTalkMasterConsoleDlg::OnLvnItemchangingListIcoms(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

//	BOOL	bOldState = (pNMLV->uOldState & (LVIS_SELECTED|LVIS_FOCUSED)) == (LVIS_SELECTED|LVIS_FOCUSED);
	BOOL	bOldState = (pNMLV->uOldState & (LVIS_SELECTED));		// OldState never has both selected & focused
	BOOL	bNewState = (pNMLV->uNewState & LVIS_SELECTED);			// NewState has both selected and focused
	BOOL	bSelItem  = (pNMLV->iItem == m_selectedRow);
#if 0
	outputDebug(_T("OnLvnItemchangingListIcoms: OldState = %s, NewState = %s, Selected Item = %s"),
		bOldState?_T("TRUE"):_T("FALSE"),
		bNewState?_T("TRUE"):_T("FALSE"),
		bSelItem?_T("TRUE"):_T("FALSE"));
#endif

#if 0																// Next code gets the column # of the click
	DWORD dwPos = ::GetMessagePos(); 
	CPoint pointCur((int)LOWORD(dwPos), (int)HIWORD(dwPos)); 
	m_listIcoms.ScreenToClient(&pointCur); 

	LVHITTESTINFO lvhti; 
	lvhti.pt = pointCur; 
	m_listIcoms.SubItemHitTest(&lvhti); 
	if(lvhti.flags & LVHT_ONITEM)
		int i = 0;
#endif

	if( bOldState && !bNewState && bSelItem )
	{
		if( bTalking && m_pSelectedItem && m_pSelectedItem->bSelected && m_pSelectedItem->iCom.picRev.opMode1 & OPMODE_REMLISTN_DISA )
		{
			m_pReleasedItem = m_pSelectedItem;
			m_nReleaseSocket = m_sessionSocket;
			bReleaseRequest = TRUE;
		}

		shutDownIcomAndHold();
	}

// Commented back in by Jon 16 March 2007 to try to get the keyboard movement working

	if( bNewState && !bSelItem )
	{
		loadIcomList(pNMHDR);
	}

	*pResult = 0;
}

void CTalkMasterConsoleDlg::loadIcomList(NMHDR *pNMHDR)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
//	LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
//	LPNMHDDISPINFO pinfo = reinterpret_cast<LPNMHDDISPINFO>(pNMHDR);
	int	index;

	m_selectedRow = pNMLV->iItem;

	if( m_selectedRow > -1 )
	{
		m_pSelectedItem = (struct _itemData *)getIcomItemData(m_selectedRow);

		if( m_pSelectedItem )
		{
			m_pSelectedItem->bChecked = m_listIcoms.GetCheck(m_selectedRow);

			index = findCQIndexFromSocket(m_pSelectedItem->socket);
			selectCQRow(index, TRUE);
		}
	}
	else
	{
		m_pSelectedItem = NULL;
		selectCQRow(-1, TRUE);			// Deselect the call queue entry too
	}

	showHdxFdxControls();

	m_btnGetVolume.EnableWindow(m_pSelectedItem != NULL && m_TalkMode == TALK_SELECTED);
	m_btnSetVolume.EnableWindow(m_pSelectedItem != NULL);
	m_btnTestVolume.EnableWindow(m_pSelectedItem != NULL && GetFileAttributes(theApp.UserOptions.testVolumeFile)!=INVALID_FILE_ATTRIBUTES);

	m_ctrlVolumeSetting.SetPos(0);
}

void CTalkMasterConsoleDlg::OnLvnItemchangingListCallsWaiting(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	if( (pNMLV->uOldState & LVIS_SELECTED && !(pNMLV->uNewState & LVIS_SELECTED)) && pNMLV->iItem == m_selectedCQRow )
	{
		shutDownIcomAndHold();
	}


	if( pNMLV->uNewState & LVIS_SELECTED && pNMLV->iItem > -1 && pNMLV->iItem != m_selectedCQRow )
	{
		if( getCallsWaitingData(pNMLV->iItem) )
		{
			selectIcomRow( selectCQRow(pNMLV->iItem) );
			outputDebug(_T("Itemchanging: Selecting new item %d"), pNMLV->iItem);
		}
		else
			outputDebug(_T("Itemchanging: Can not select item - no item found"));
	}

	*pResult = 0;
}

void CTalkMasterConsoleDlg::OnNMClickListCallsWaiting(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	if( m_tabMain.GetCurSel() == 1 )
	{
		if( m_playFileSocket == 0 )
		{
			m_tabMain.SetCurSel(0);
			displayTabMain();
		}
	}

	if( !m_playFileSocket && !bTalking && !bListening )
	{
		OnBnClickedRadioSelected();
		m_btnSelected.SetCheck(BST_CHECKED);
		m_btnGroup.SetCheck(BST_UNCHECKED);
		m_btnAllActive.SetCheck(BST_UNCHECKED);
	}

//	if( pNMLV->iItem > -1 )
//	{
//		selectIcomRow( selectCQRow(pNMLV->iItem) );
//	}

	setResetFocus(&m_listCallsWaiting);
	resetFocus();

	*pResult = 0;
}

void CTalkMasterConsoleDlg::displayIntercomCount()
{
	char	buffer[100];

	sprintf(buffer, getResourceString(IDS_STRING_INTERCOMS_CONNECTED_D_D), m_nIntercomsConsoleAlive, m_nIntercomsConsole);
	m_staticStatusConnected.SetWindowText(buffer);
}

void CTalkMasterConsoleDlg::shutDownIcomAndHold()
{
		if( m_pSelectedItem && !theApp.UserOptions.forceHdx &&
			m_pSelectedItem->iCom.picRev.feature1 & OPNIC_FULL_DUPLEX )
		{
			if( bTalking )
				doFdxStartStop();				// Stop it
		}
		else									// Half Duplex
		{
			if( bTalking || bTalkRequest)		// If we are talking or going into talk
			{
				DoEvents();
				Sleep(0);

				bTalkRequest = FALSE;
				bListenRequest = FALSE;

				while(bTalking)
				{
					bTalkRequest = FALSE;
					bListenRequest = FALSE;
					DoEvents();
					Sleep(0);
				}

				bTalking = FALSE;
				lockControls(FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE);		// LOCK Talk, Listen, Chime, PlayFile, Radio Buttons

				if( !bListening )
				{
					if( m_sessionSocket && !isCallQueue(m_sessionSocket) )
					{
						bReleaseRequest = TRUE;
//						selectSocket(m_sessionSocket, FALSE);						// Don't do this here, let the release routine do it

						m_nReleaseSocket = m_sessionSocket;		// Save this for the release routine
					}

					outputDebug("shutDownIcomAndHold: Cleared Session from %d to 0", m_sessionSocket);
					m_sessionSocket = 0;

					if( bReleaseRequest )
						outputDebug("shutDownIcomAndHold: Releasing session %d", m_nReleaseSocket);

					while( bReleaseRequest )
					{
						bTalkRequest = FALSE;
						bListenRequest = FALSE;

						DoEvents();
						Sleep(0);
					}
				}
			}

			if( bListening || bListenRequest )
			{
				bTalkRequest = FALSE;
				bListenRequest = FALSE;

				DoEvents();
				Sleep(0);

				while(bListening)
				{
					bTalkRequest = FALSE;
					bListenRequest = FALSE;
					DoEvents();
					Sleep(0);
				}

				bListening = FALSE;
				bNoRestart = FALSE;
				lockControls(FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE);		// LOCK Talk, Listen, Chime, PlayFile, Radio Buttons

//
//	Don't want to clear m_sessionSocket here because we are about to use it in the next IF() statement, plus it is cleared in both of the
//	sides of the if() statement after use
//
//				outputDebug("shutDownIcomAndHold: (2) Cleared Session from %d to 0", m_sessionSocket);
//				m_sessionSocket = 0;
			}

			if( bSessionRequest )
			{
				bSession = TRUE;
				bSessionRequest = FALSE;

				while( bSession )
				{
					bSessionRequest = FALSE;

					DoEvents();
					Sleep(0);
				}
			}
		}

		if( isCallQueue(m_sessionSocket) )			// Is this call on the call queue?
		{
			MarkCallQueueSession(m_sessionSocket, TRUE);			// Held
			outputDebug("shutDownIcomAndHold: (3) Cleared Session from %d to 0", m_sessionSocket);
			m_sessionSocket = 0;					// jdnjdnjdn trying to get the END to work

			activatePlayer(FALSE);

//			m_selectedCQRow = -1;									// reselect the call queue row
//			SetHoldEnd(FALSE, TRUE);
		}
		else
		{
			if( m_sessionSocket )
			{
				if( m_pSelectedItem && 
					m_pSelectedItem->iCom.picRev.opMode1 & OPMODE_REMLISTN_DISA )
				{
					m_btnNoListen.SetBitmap( HBITMAP(mBitMapNoListenOff) );				// RLD needs to reset the bitmap
				}

				m_nReleaseSocket = m_sessionSocket;

				bReleaseRequest = TRUE;
				bListenRequest = FALSE;
				bTalkRequest = FALSE;			// Shut this down

				while( m_sessionSocket )
					DoEvents();

//				m_DASelectIntercom( m_hDA, m_sessionSocket, FALSE, NULL );
				outputDebug("shutDownIcomAndHold: (4) Cleared Session from %d to 0", m_sessionSocket);
				m_sessionSocket = 0;
				activatePlayer(FALSE);
			}
		}

}
void CTalkMasterConsoleDlg::OnHdnEndtrackListIcoms(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);

	resetFocus();
//	m_listCallsWaiting.SetFocus();

	*pResult = 0;
}

void CTalkMasterConsoleDlg::OnNMSetfocusListIcoms(NMHDR *pNMHDR, LRESULT *pResult)
{

	resetFocus();
//	m_listCallsWaiting.SetFocus();

	*pResult = 0;
}

void CTalkMasterConsoleDlg::resetFocus()
{
	if( m_pListFocus )
		m_pListFocus->SetFocus();
}

void CTalkMasterConsoleDlg::setResetFocus(CListCtrl *list)
{
	m_pListFocus = list;
}
void CTalkMasterConsoleDlg::OnBnClickedCheckTest()
{
	UpdateData();

	srand( (unsigned)time( NULL ) );

	if( !m_pSelectedItem || !bLoggedOn || bShuttingDown )
	{
		MessageBox(_T("Must select an intercom before entering test mode"), _T("No Intercom Selected"));

		m_bTestMode = FALSE;
		UpdateData(FALSE);
	}

	this->bSlow = FALSE;

	if( m_bTestMode )
		SetTimer(TIMER_TEST, 100, NULL);
	else
		KillTimer(TIMER_TEST);
}

void CTalkMasterConsoleDlg::clearTestMode()
{
	if( m_bTestMode )
	{
		m_btnTestMode.SetCheck(BST_UNCHECKED);
		OnBnClickedCheckTest();
	}
}

void CTalkMasterConsoleDlg::OnNMReleasedcaptureSliderMaxMsec(NMHDR *pNMHDR, LRESULT *pResult)
{
	UpdateData();

	m_csMaxMsec.Format(_T("%dms"), m_maxMsec);

	UpdateData(FALSE);

	*pResult = 0;
}

void CTalkMasterConsoleDlg::OnBnClickedButtonGetVolume()
{
	int volume;

	if( m_TalkMode == TALK_SELECTED &&  m_pSelectedItem )
	{
		m_btnGetVolume.EnableWindow(FALSE);

		volume = m_DAGetVolume( m_hDA, m_pSelectedItem->socket );
		m_ctrlVolumeSetting.SetPos(volume);
		outputDebug("GetVolume: Volume is %d", volume);

		m_btnGetVolume.EnableWindow(TRUE);
	}
}

void CTalkMasterConsoleDlg::OnNMReleasedcaptureSliderSetupVolume(NMHDR *pNMHDR, LRESULT *pResult)
{
	UpdateData();

	*pResult = 0;
}

void CTalkMasterConsoleDlg::OnBnClickedButtonSetVolume()
{
	int i, count, volume, socket;
	struct _itemData *tItemData;

	count = m_listIcoms.GetItemCount();
	volume = m_ctrlSetupVolume.GetPos();

	if( m_TalkMode == TALK_SELECTED && m_pSelectedItem )
	{
		m_DASetVolume( m_hDA, m_pSelectedItem->socket, 0, volume );

		Sleep(2000);

		OnBnClickedButtonGetVolume();
	}
	else if( m_TalkMode == TALK_GROUP )				// Read the check boxes
	{
		for( i=0 ; i<count ; i++ )
		{
			socket = getIntercomSocket(i);
			tItemData = findItemData(socket);

			if( tItemData && tItemData->iCom.status == STATUS_IDLE && m_listIcoms.GetCheck(i) )
				m_DASetVolume( m_hDA, socket, 0, volume );
		}
	}
	else if( m_TalkMode == TALK_ALL_ACTIVE )		// Find all that have a status idle
	{
		for( i=0 ; i<count ; i++ )
		{
			socket = getIntercomSocket(i);
			tItemData = findItemData(socket);

			if( tItemData && tItemData->iCom.status == STATUS_IDLE )
				m_DASetVolume( m_hDA, socket, 0, volume );
		}
	}
}

void CTalkMasterConsoleDlg::OnBnClickedButtonTestVolume()
{
	if( m_TalkMode == TALK_SELECTED && !m_pSelectedItem )
	{
		MessageBox(_T(getResourceString(IDS_STRING_SELECT_ICOM)), _T(getResourceString(IDS_STRING_INFORMATION)));
		resetFocus();
//		m_listCallsWaiting.SetFocus();
		return;
	}

	if( m_TalkMode == TALK_SELECTED && !m_pSelectedItem->iCom.status )
	{
		MessageBox(_T(getResourceString(IDS_STRING113)), _T(getResourceString(IDS_STRING114)));
		resetFocus();
		return;
	}

	if( m_TalkMode != TALK_SELECTED && countGroup() == 0 )
	{
		MessageBox(_T(getResourceString(IDS_STRING120)), _T(getResourceString(IDS_STRING121)));
		resetFocus();
		return;
	}


	if( m_playFileSocket == 0 )
	{
		if( m_TalkMode == TALK_SELECTED )
			m_playFileSocket = m_pSelectedItem->socket;
		else
			m_playFileSocket = makeGroupSocket();

		if( !m_playFileSocket )
			{
			MessageBox(_T(getResourceString(IDS_STRING113)), _T(getResourceString(IDS_STRING114)));
			resetFocus();
			return;
			}

		if( !sendAudioFile(m_playFileSocket, theApp.UserOptions.testVolumeFile, TRUE, TRUE) )
		{
			if( m_TalkMode != TALK_SELECTED )
				m_DADeleteGroup(m_hDA, m_playFileSocket);		// Really can not do this since it														// will wait for the thread to end

			m_playFileSocket = 0;
			bPlayFileLocal = TRUE;
			resetFocus();
			return;
		}

		bHoldHold = m_btnSessionHold.IsWindowEnabled();
		bHoldEnd  = m_btnSessionEnd.IsWindowEnabled();

		SetHoldEnd(FALSE, FALSE);
		
		m_btnTestVolume.SetWindowText(_T(getResourceString(IDS_STRING182)));
		lockControls(TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, TRUE, TRUE);
	}
	else
	{
		stopAudioFile(m_playFileSocket);
		m_btnTestVolume.SetWindowText(_T(getResourceString(IDS_STRING183)));

		SetHoldEnd(bHoldHold, bHoldEnd);

		lockControls(FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE);
	}

	m_tabMain.EnableWindow( m_playFileSocket == 0 );

	resetFocus();
}

void CTalkMasterConsoleDlg::OnBtnDnButtonFdxStart()
{
	if( theApp.UserOptions.PTT && !bTalking )
	{
		doFdxStartStop();

		outputDebug(_T("OnBtnDnButtonFdxStart: Full-duplex Start"));
	}
}

void CTalkMasterConsoleDlg::OnBtnUpButtonFdxStart()
{
	if( theApp.UserOptions.PTT )
	{
		if( bTalking )
		{
			doFdxStartStop();
			outputDebug(_T("OnBtnDnButtonFdxStart: Full-duplex Stop"));
		}
	}
	else
		doFdxStartStop();
}

void CTalkMasterConsoleDlg::doFdxStartStop()
{
//	m_btnFdxStart.EnableWindow(FALSE);

	if( bTalking || bListening )		// Already running
	{
		bNoRestart = TRUE;						// Don't allow a Listen Restart to occur
#if 0
		m_DAListenIcom( m_hDA, m_sessionSocket, WAVE_FORMAT_ULAW, FALSE );

		while( !waitClear(m_sessionSocket, STATUS_TALKING) )		// Wait 200 ms for the intercom to clear down
			;

		m_DAEndAudio( m_hDA, m_sessionSocket, FALSE );
		audioPlayer.StopMic();

		bTalking = FALSE;
		bTalkRequest = FALSE;

		bListening = FALSE;
		bListenRequest = FALSE;

		bMuting = FALSE;
		bNoRestart = FALSE;

		m_btnFdxStart.SetBitmap( HBITMAP(mBitMapFdxTalkOff) );
		m_btnFdxMute.SetBitmap( HBITMAP(mBitMapFdxMute) );

		if( !isCallQueue(m_sessionSocket) )			// Is this call on the call queue?
		{
			selectSocket(m_sessionSocket, FALSE);

			outputDebug("doFdxStartStop: Cleared Session from %d to 0", m_sessionSocket);
			m_sessionSocket = 0;	// No longer in a session

			if( m_pSelectedItem )
				m_pSelectedItem->bSelected = FALSE;			// Not selected anymore

			SetHoldEnd(FALSE, FALSE);					// Change the buttons to NO
		}
		else
			SetHoldEnd(FALSE, TRUE);

		lockControls(FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE);		// Unlock everything
#else
		m_nReleaseSocket = m_sessionSocket;					// We will need the session socket # for later if we are going to release it

		bListenRequest = FALSE;								// The way the control thread works is we "request" it to do stuff for us
		bTalkRequest = FALSE;								// and then we wait for it to be done

		while( bTalking || bListening )						// While we are still waiting for both Talk and Listen to end
		{
			DoEvents();										// Allow other messages to be processed
			Sleep(1);										// Allow other threads to be run

			bListenRequest = FALSE;							// Make sure the request did not change
			bTalkRequest = FALSE;
		}

		if( !isCallQueue( m_nReleaseSocket ) )				// If this entry is NOT a call queue entry, then we want to release it (deselect)
		{
			bReleaseRequest = TRUE;							// Ask the release control thread code to release this socket (set above)

			while( bReleaseRequest )						// While we are waiting for the release to complete
			{
				DoEvents();									// Allow other messages to be processed
				Sleep(1);									// Allow the control thread to finish
			}
		}

		m_nReleaseSocket = 0;								// Clear it when we are done, or if we never went in because it was a call queue entry

		bMuting = FALSE;									// Clear the muting flag just in case it was set before
		bNoRestart = FALSE;									// Allow Talk->Listen restarts to occur if in auto mode
	
		m_btnFdxStart.SetBitmap( HBITMAP(mBitMapFdxTalkOff) );	// Change the START button bitmap back to a Start value
		m_btnFdxMute.SetBitmap( HBITMAP(mBitMapFdxMute) );		// Change the MUTE button bitmap back to MUTE

#endif
	}
	else
	{
		if( !m_pSelectedItem )
			return;

		if( (m_pSelectedItem->iCom.status&STATUS_MASK) != STATUS_IDLE )
		{
			MessageBox(_T(getResourceString(IDS_STRING115)), _T(getResourceString(IDS_STRING116)));
			return;
		}

#if 0			// Now being done in the control thread
		if( !selectSocket(m_pSelectedItem->socket, TRUE) )
		{
			outputDebug(_T("OnBnClickedButtonFdxStart: selectSocket(%d) failed, item=0x%x)"),
				m_sessionSocket, m_pSelectedItem);
			return;
		}
#endif

#if 0
		m_sessionSocket = m_pSelectedItem->socket;

		bTalking = TRUE;
		bTalkRequest = TRUE;
		bListening = TRUE;
		bListenRequest = TRUE;

		if( bUseUlaw )
			m_DAListenIcom( m_hDA, m_sessionSocket, WAVE_FORMAT_ULAW, TRUE );
		else
			m_DAListenIcom( m_hDA, m_sessionSocket, WAVE_FORMAT_PCM, TRUE );

		waitSet(m_sessionSocket, STATUS_LISTENING);

		if( bUseUlaw )
			m_DAStartAudio( m_hDA, m_sessionSocket, TYPE_ULAW |TYPE_MICROPHONE );
		else
			m_DAStartAudio( m_hDA, m_sessionSocket, TYPE_PCM | TYPE_MICROPHONE );

		micAudioSize = 0;
		audioPlayer.StartMic();

		StartLocalCQSession(m_sessionSocket, FALSE);			// That is if it is there at all (not held)

		if( isCallQueue(m_sessionSocket) )
			SetHoldEnd(TRUE, TRUE);

		lockControls(FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE);		// LOCK Talk, Listen, Chime, PlayFile, Radio Buttons
#else
		bTalkRequest = TRUE;									// We are trying to start the Talk and the Listen (FDX)
		bListenRequest = TRUE;									// Setting these requests causes the control thread to start up both talk and listen

		while( !bTalking || !bListening )						// Wait for both the Talk and the Listen to start
		{
			DoEvents();											// While waiting, process other messages
			Sleep(1);											// Allow the control thread to run

			bTalkRequest = TRUE;								// Make sure this request did not get cleared
			bListenRequest = TRUE;								// Or this one
		}

#endif
		m_btnFdxStart.SetBitmap( HBITMAP(mBitMapFdxTalkOn) );	// Change the START button to END
	}

//	m_btnFdxStart.EnableWindow(TRUE);
}

//
// OnBnClickedButtonFdxMute
//
// We don't actually mute anything other than setting a variable that does not send the audio data to the other end
// Mute means mute OUR microphone, not the distant ends microphone
//

void CTalkMasterConsoleDlg::OnBnClickedButtonFdxMute()
{
	if( !bTalking && !bListening )								// If we are not in FDX mode
		return;													// Just leave

	if( bMuting )												// If we are currently muting
		m_btnFdxMute.SetBitmap( HBITMAP(mBitMapFdxMute) );		// Change the bitmap to MUTE
	else
		m_btnFdxMute.SetBitmap( HBITMAP(mBitMapFdxMuted) );		// otherwise change the bitmap to UNMUTE

	bMuting = ~bMuting;											// Now switch muting to not muting (and visa versa)
}

//
// OnInvisibleResetintercomconnection
//
// I have no idea how this next code gets executed...  I do not see any way that it can get called from the message map
//

void CTalkMasterConsoleDlg::OnInvisibleResetintercomconnection()
{
	if( !m_pSelectedItem )
		return;

	m_DAResetConnection(m_hDA, m_pSelectedItem->socket);
}

void CTalkMasterConsoleDlg::OnBnClickedButtonTestAudio()
{
	if( m_bTestAudio )
	{
		m_btnTestAudio.SetWindowText(_T(getResourceString(IDS_STRING_TEST_AUDIO)));
		m_bTestAudio = FALSE;
	}
	else
	{
		if( !m_pSelectedItem )
		{
			MessageBox(_T(getResourceString(IDS_STRING_MUST_SELECT_ICOMS)), _T(getResourceString(IDS_STRING_NO_ICOM_SELECTED)));
			return;
		}

		CFileDialog pFD(	TRUE, 
							_T(getResourceString(IDS_STRING_WAV)),
							NULL,
							OFN_OVERWRITEPROMPT|OFN_HIDEREADONLY,
							getResourceString(IDS_STRING_WAV_FILE_FILTER),
							this);

		chdir(theApp.UserOptions.path);

		bOffDialog = TRUE;							// Not on the main dialog

		if( pFD.DoModal()!=IDOK )
		{
			bOffDialog = FALSE;						// Back on the main dialog
			bSkipNextKeyUp = TRUE;

			resetFocus();
			return;
		}

		bOffDialog = FALSE;							// Back on the main dialog
		bSkipNextKeyUp = TRUE;

		if( !selectSocket( m_pSelectedItem->socket ) )
		{
			MessageBox("Can not select intercom", "Intercom already in use?" );
			return;
		}

		m_playFileSocket = m_pSelectedItem->socket;

		if( !m_playFileSocket )
			{
			MessageBox(_T("You must select connected intercoms"), _T("Intercom Not Connected"));
			resetFocus();
			return;
			}

		m_csTestAudioFile = pFD.GetFileName().GetBuffer();
		m_testAudioSocket = m_playFileSocket;

		sendAudioFile(m_playFileSocket, m_csTestAudioFile.GetBuffer(), FALSE, FALSE);

		m_DAListenIcom( m_hDA, m_playFileSocket, WAVE_FORMAT_ULAW, TRUE );

		SetTimer(TIMER_TEST_AUDIO, 1000, NULL);
		m_btnTestAudio.SetWindowText("Stop Test");
		m_bTestAudio = TRUE;
	}
}

void CTalkMasterConsoleDlg::doTestAudio()
{
	struct _itemData *tItemData;

	if( !m_bTestAudio )				// End of test
	{
		if( m_playFileSocket )		// Still playing the file
			stopAudioFile(m_playFileSocket);

		m_DAListenIcom( m_hDA, m_testAudioSocket, WAVE_FORMAT_ULAW, FALSE );

		deselectSocket( m_testAudioSocket );

		KillTimer(TIMER_TEST_AUDIO);

		return;
	}

	if( !m_playFileSocket )			// File done
	{
		tItemData = findItemData(m_testAudioSocket);

		if( !tItemData )
		{
			KillTimer(TIMER_TEST_AUDIO);
			return;
		}

		if( (tItemData->iCom.status == STATUS_COMMAND ) )		// Waiting for the status to end
			;
		else if( (tItemData->iCom.status&STATUS_MASK) != STATUS_IDLE )
			m_DAListenIcom( m_hDA, m_playFileSocket, WAVE_FORMAT_ULAW, FALSE );
		else
		{
			m_playFileSocket = m_testAudioSocket;

			sendAudioFile(m_playFileSocket, m_csTestAudioFile.GetBuffer(), FALSE, FALSE );
			m_DAListenIcom( m_hDA, m_playFileSocket, WAVE_FORMAT_ULAW, TRUE );
		}
	}
}

void CTalkMasterConsoleDlg::doRepeatAudio()
{
	if( !m_pRepeatItemData )
	{
		KillTimer(TIMER_REPEAT);
		return;
	}

	if( (m_pRepeatItemData->iCom.status&STATUS_MASK) == STATUS_IDLE )
	{
		sendAudioFile(m_repeatSocket, m_csRepeatFileName.GetBuffer(), TRUE, TRUE);				// Send it again
		KillTimer(TIMER_REPEAT);
	}
	
}

void CTalkMasterConsoleDlg::OnToolsSendtextmessage()
{
	if( m_TalkMode == TALK_SELECTED && !m_pSelectedItem )		// No intercom selected yet
	{
		MessageBox(_T(getResourceString(IDS_STRING_SELECT_ICOM)), _T(getResourceString(IDS_STRING_INFORMATION)));
		resetFocus();
//		m_listCallsWaiting.SetFocus();

		return;
	}

	if( m_TalkMode == TALK_SELECTED && !m_pSelectedItem->iCom.status )
	{
		MessageBox(_T(getResourceString(IDS_STRING113)), _T(getResourceString(IDS_STRING114)));
		resetFocus();
		return;
	}

	if( m_TalkMode != TALK_SELECTED && countGroup() == 0 )
	{
		MessageBox(_T(getResourceString(IDS_STRING120)), _T(getResourceString(IDS_STRING121)));
		resetFocus();
		return;
	}

	bOffDialog = TRUE;							// Not on the main dialog

	sendTextDlg.DoModal();

	bOffDialog = FALSE;							// On the main dialog
	bSkipNextKeyUp = TRUE;
}

void CTalkMasterConsoleDlg::OnNMClickListMessages(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	if( pNMLV->iItem >= 0 )
		m_pCurrentMessage = (_messageData*)m_listMessages.GetItemData(pNMLV->iItem);
	else
		m_pCurrentMessage = NULL;

	*pResult = 0;
}

void CTalkMasterConsoleDlg::OnBnClickedButtonSessionStart()
{
	bTalkRequest = FALSE;
	bListenRequest = FALSE;
	bSessionRequest = TRUE;

	m_btnTalkSessionStart.ShowWindow(SW_HIDE);
	m_btnTalkSessionEnd.ShowWindow(SW_SHOW);

	outputDebug(_T("OnBnClickedButtonSessionStart: bSessionRequest Set"));
}

void CTalkMasterConsoleDlg::OnBnClickedButtonSessionEnd()
{
	if( !bSessionRequest )
		return;

	bTalkRequest = FALSE;
	bListenRequest = FALSE;

	bSessionRequest = FALSE;
	bSession = TRUE;

	m_btnTalkSessionStart.ShowWindow(SW_SHOW);
	m_btnTalkSessionEnd.ShowWindow(SW_HIDE);

	outputDebug(_T("OnBnClickedButtonSessionEnd: bSessionRequest Cleared"));
}

void CTalkMasterConsoleDlg::OnToolsTestconsoledll()
{
	CTestLogonDlg dlg;

	dlg.DoModal();
}
