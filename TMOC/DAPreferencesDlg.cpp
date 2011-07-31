//
// DAPreferencesDlg.cpp : implementation file
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
#include "DAPreferencesDlg.h"
#include ".\dapreferencesdlg.h"
#include "mmsystem.h"

// DAPreferencesDlg dialog

IMPLEMENT_DYNCREATE(DAPreferencesDlg, CDialog)

int pageMainIDs[] = {
						IDC_COMBO_ANNOUNCEMENT_AUDIBLE_SOUND,
						IDC_CHECK_HIDE,
						IDC_CHECK_PTT,
						IDC_CHECK_CALL_ANNOUNCE,
						IDC_CHECK_VISUAL_NOTIFICATION,
						IDC_CHECK_AUDIBLE_NOTIFICATION,
						IDC_STATIC_PREF_CALL_ANNOUNCE_GROUP,
						IDC_STATIC_PREF_ANNOUNCE_RULES_GROUP,
						IDC_STATIC_PREF_QUEUE_AGE,
						IDC_STATIC_PREF_QUEUE_SIZE,
						IDC_EDIT_PREF_QUEUE_AGE,
						IDC_EDIT_PREF_QUEUE_SIZE,
						IDC_COMBO_PREF_QUEUE_AGE_TYPE,
						IDC_STATIC_PREF_ENTRIES,
						0 };

int	pageDevicesIDs[] = {
						IDC_COMBO_WAVE_INPUT_DEVICE,
						IDC_COMBO_WAVE_OUTPUT_DEVICE,
						IDC_STATIC_PREF_WAVE_OUT_GROUP,
						IDC_STATIC_PREF_WAVE_IN_GROUP,
						IDC_CHECK_MICROPHONE_ULAW,
						IDC_CHECK_PLAY_AGAIN,
						IDC_CHECK_SHOW_DEBUG,
						IDC_CHECK_SETUP_VOLUME,
						IDC_EDIT_TEST_VOLUME_FILE,
						IDC_BUTTON_BROWSE_TEST_VOLUME_FILE,
						IDC_STATIC_TEST_VOLUME_FILE,
						IDC_CHECK_FORCE_HDX,
						IDC_CHECK_VAD,
						0 };

int pageRemoteIDs[] = {
						0 };

int pageLocalIDs[] = {
						IDC_COMBO_STARTUP_TYPE,
						IDC_COMBO_STARTUP_LANGUAGE,
						IDC_EDIT_PING_INTERVAL,
						IDC_SPIN_PING_INTERVAL,
						IDC_CHECK_RECORD_CALL_WAITING,
						IDC_STATIC_PREFERENCES_PING_GROUP,
						IDC_STATIC_PREF_STARTUP_AUDIO,
						IDC_STATIC_PREF_PING_SECS,
						IDC_CHECK_BEEPS,
						0 };

int *pages[4] = {	pageMainIDs,
					pageDevicesIDs,
					pageRemoteIDs,
					pageLocalIDs,
					};


DAPreferencesDlg::DAPreferencesDlg(CWnd* pParent /*=NULL*/)
	: CDialog(DAPreferencesDlg::IDD, pParent)
	, m_csPingInterval(_T(""))
	, m_bBeeps(FALSE)
	, m_bHide(FALSE)
	, m_bPTT(FALSE)
	, m_bCallAnnouncement(FALSE)
	, m_bVisualNotification(FALSE)
	, m_bAudibleNotification(FALSE)
	, m_bRecordCallsWaiting(FALSE)
	, m_nPrefQueueAge(0)
	, m_nPrefQueueSize(0)
	, m_bUseUlaw(FALSE)
	, m_bPlayAgain(FALSE)
	, m_bShowDebug(FALSE)
	, m_bSetupVolume(FALSE)
	, m_csTestVolumeFile(_T(""))
	, m_bForceHdx(FALSE)
	, m_bFilterNoise(FALSE)
{
	m_bWarning = FALSE;
	m_bDADebug = FALSE;
}

DAPreferencesDlg::~DAPreferencesDlg()
{
}

void DAPreferencesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_STARTUP_TYPE, m_comboStartupType);
	DDX_Control(pDX, IDC_COMBO_STARTUP_LANGUAGE, m_comboStartupLanguage);
	DDX_Control(pDX, IDC_COMBO_ANNOUNCEMENT_AUDIBLE_SOUND, m_comboAnnouncementSound);
	DDX_Control(pDX, IDC_EDIT_PING_INTERVAL, m_editPingInterval);
	DDX_Control(pDX, IDC_SPIN_PING_INTERVAL, m_cspinPingInterval);
	DDX_Control(pDX, IDC_CHECK_BEEPS, m_checkAddBeeps);
	DDX_Control(pDX, IDC_CHECK_HIDE, m_checkHide);
	DDX_Control(pDX, IDC_CHECK_PTT, m_checkPTT);
	DDX_Control(pDX, IDC_CHECK_CALL_ANNOUNCE, m_checkCallAnnouncement);
	DDX_Control(pDX, IDC_CHECK_RECORD_CALL_WAITING, m_checkRecordCallsWaiting);
	DDX_Control(pDX, IDC_CHECK_VISUAL_NOTIFICATION, m_checkVisualNotification);
	DDX_Control(pDX, IDC_CHECK_AUDIBLE_NOTIFICATION, m_checkAudibleNotification);
	DDX_Text(pDX, IDC_EDIT_PING_INTERVAL, m_csPingInterval);
	DDV_MaxChars(pDX, m_csPingInterval, 3);
	DDX_Check(pDX, IDC_CHECK_BEEPS, m_bBeeps);
	DDX_Check(pDX, IDC_CHECK_HIDE, m_bHide);
	DDX_Check(pDX, IDC_CHECK_PTT, m_bPTT);
	DDX_Check(pDX, IDC_CHECK_CALL_ANNOUNCE, m_bCallAnnouncement);
	DDX_Check(pDX, IDC_CHECK_VISUAL_NOTIFICATION, m_bVisualNotification);
	DDX_Check(pDX, IDC_CHECK_AUDIBLE_NOTIFICATION, m_bAudibleNotification);
	DDX_Check(pDX, IDC_CHECK_RECORD_CALL_WAITING, m_bRecordCallsWaiting);
	DDX_Control(pDX, IDC_COMBO_WAVE_INPUT_DEVICE, m_comboWaveInputDevice);
	DDX_Control(pDX, IDC_COMBO_WAVE_OUTPUT_DEVICE, m_comboWaveOutputDevice);
	DDX_Control(pDX, IDC_TAB_PREFERENCES, m_tabPreferences);
	DDX_Control(pDX, IDC_STATIC_PREFERENCES_PING_GROUP, m_staticPrefPingGroup);
	DDX_Control(pDX, IDC_STATIC_PREF_STARTUP_AUDIO, m_staticPrefStartupAudioGroup);
	DDX_Control(pDX, IDC_STATIC_PREF_CALL_ANNOUNCE_GROUP, m_staticPrefCallAnnounceGroup);
	DDX_Control(pDX, IDC_STATIC_PREF_WAVE_OUT_GROUP, m_staticPrefWaveOutGroup);
	DDX_Control(pDX, IDC_STATIC_PREF_WAVE_IN_GROUP, m_staticPrefWaveInGroup);
	DDX_Control(pDX, IDC_STATIC_PREF_PING_SECS, m_staticPrefPingSecs);
	DDX_Control(pDX, IDHELP, m_btnPrefHelp);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_STATIC_PREF_ANNOUNCE_RULES_GROUP, m_staticPrefAnnounceRulesGroup);
	DDX_Control(pDX, IDC_STATIC_PREF_QUEUE_AGE, m_staticPrefQueueAge);
	DDX_Control(pDX, IDC_STATIC_PREF_QUEUE_SIZE, m_staticPrefQueueSize);
	DDX_Control(pDX, IDC_EDIT_PREF_QUEUE_AGE, m_editPrefQueueAge);
	DDX_Text(pDX, IDC_EDIT_PREF_QUEUE_AGE, m_nPrefQueueAge);
	DDV_MinMaxInt(pDX, m_nPrefQueueAge, 0, 60);
	DDX_Control(pDX, IDC_EDIT_PREF_QUEUE_SIZE, m_editPrefQueueSize);
	DDX_Text(pDX, IDC_EDIT_PREF_QUEUE_SIZE, m_nPrefQueueSize);
	DDX_Control(pDX, IDC_COMBO_PREF_QUEUE_AGE_TYPE, m_comboPrefQueueAgeType);
	DDX_Control(pDX, IDC_STATIC_PREF_ENTRIES, m_staticPrefQueueEntries);
	DDX_Control(pDX, IDC_CHECK_MICROPHONE_ULAW, m_checkUseUlaw);
	DDX_Check(pDX, IDC_CHECK_MICROPHONE_ULAW, m_bUseUlaw);
	DDX_Control(pDX, IDC_CHECK_PLAY_AGAIN, m_btnPlayAgain);
	DDX_Check(pDX, IDC_CHECK_PLAY_AGAIN, m_bPlayAgain);
	DDX_Control(pDX, IDC_CHECK_SHOW_DEBUG, m_btnShowDebug);
	DDX_Check(pDX, IDC_CHECK_SHOW_DEBUG, m_bShowDebug);
	DDX_Control(pDX, IDC_CHECK_SETUP_VOLUME, m_checkSetupVolume);
	DDX_Check(pDX, IDC_CHECK_SETUP_VOLUME, m_bSetupVolume);
	DDX_Control(pDX, IDC_EDIT_TEST_VOLUME_FILE, m_editTestVolumeFile);
	DDX_Text(pDX, IDC_EDIT_TEST_VOLUME_FILE, m_csTestVolumeFile);
	DDX_Control(pDX, IDC_BUTTON_BROWSE_TEST_VOLUME_FILE, m_btnBrowseTestVolumeFile);
	DDX_Control(pDX, IDC_STATIC_TEST_VOLUME_FILE, m_staticTestVolumeFileGroup);
	DDX_Control(pDX, IDC_CHECK_FORCE_HDX, m_btnForceHdx);
	DDX_Check(pDX, IDC_CHECK_FORCE_HDX, m_bForceHdx);
	DDX_Control(pDX, IDC_CHECK_VAD, m_btnNoiseFilter);
	DDX_Check(pDX, IDC_CHECK_VAD, m_bFilterNoise);
}

BOOL DAPreferencesDlg::OnInitDialog()
{
	struct _stat fileStat;
	RECT view;
	int	cx, cy;

	CTalkMasterConsoleDlg *dlg = (CTalkMasterConsoleDlg*)theApp.m_pMainWnd;

	CDialog::OnInitDialog();

	if( _stat("c:\\DATestFile.txt", &fileStat) == 0 )
		m_bDADebug = TRUE;

	m_mode = dlg->m_uFlags & DA_REMOTE;

	m_tabPreferences.InsertItem(0, dlg->getResourceString(IDS_STRING142));
	m_tabPreferences.InsertItem(1, dlg->getResourceString(IDS_STRING143));
#if 0
	CString message3;
	message3.LoadString(IDS_STRING144);
	CString message4;
	message4.LoadString(IDS_STRING145);
	if( m_mode )
		m_tabPreferences.InsertItem(2, message3);
	else
		m_tabPreferences.InsertItem(2, message4);
#endif
	this->GetClientRect(&view);
	this->SetWindowPos(NULL, 0, 0, 350, 450,
		SWP_NOOWNERZORDER | SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOMOVE);	// SWP_NOMOVE

	this->GetClientRect(&view);
	cx = view.right - view.left;
	cy = view.bottom - view.top;

	m_tabPreferences.SetWindowPos(NULL, 5, 10, cx-10, cy-60,
			SWP_NOOWNERZORDER | SWP_SHOWWINDOW | SWP_NOZORDER );	// SWP_NOMOVE

	m_btnPrefHelp.SetWindowPos(NULL, 160, cy-35, 0, 0,
			SWP_NOOWNERZORDER | SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOSIZE);	// SWP_NOMOVE

	m_btnOK.SetWindowPos(NULL, 250, cy-35, 0, 0,
			SWP_NOOWNERZORDER | SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOSIZE);	// SWP_NOMOVE

	showPage(0);

	m_comboStartupType.SetCurSel(theApp.UserOptions.startupAudioType);
	m_comboStartupLanguage.SetCurSel(theApp.UserOptions.startupAudioSound);
	if( theApp.UserOptions.startupAudioType == 0 )
		m_comboStartupLanguage.EnableWindow(FALSE);

	m_csPingInterval.Format("%d", theApp.UserOptions.pingInterval);
	m_bAudibleNotification = theApp.UserOptions.callAnnounceAudible;
	m_bBeeps = theApp.UserOptions.useBeeps;
	m_bCallAnnouncement = theApp.UserOptions.callAnnounce;
	m_bHide = theApp.UserOptions.hideWindow;
	m_bPTT = theApp.UserOptions.PTT;
	m_bRecordCallsWaiting = theApp.UserOptions.recordCallsWaiting;
	m_bVisualNotification = theApp.UserOptions.callAnnounceVisual;

	m_checkAudibleNotification.EnableWindow( m_bCallAnnouncement );
	m_checkVisualNotification.EnableWindow( m_bCallAnnouncement );
	m_comboAnnouncementSound.SetCurSel(theApp.UserOptions.callAnnounceSound);
	m_comboAnnouncementSound.EnableWindow(m_bCallAnnouncement && m_bAudibleNotification);

	m_editPrefQueueAge.EnableWindow( m_bCallAnnouncement );
	m_editPrefQueueSize.EnableWindow( m_bCallAnnouncement );
	m_comboPrefQueueAgeType.EnableWindow( m_bCallAnnouncement );

	loadWaveInputDevices();
	m_comboWaveInputDevice.SetCurSel( theApp.UserOptions.waveInputDevice+1 );
	m_nWaveInputDevice = theApp.UserOptions.waveInputDevice;

	loadWaveOutputDevices();
	m_comboWaveOutputDevice.SetCurSel( theApp.UserOptions.waveOutputDevice+1 );

	m_nPrefQueueAge = theApp.UserOptions.callAnnounceQueueAge;
	m_nPrefQueueSize = theApp.UserOptions.callAnnounceQueueSize;
	m_comboPrefQueueAgeType.SetCurSel( theApp.UserOptions.callAnnounceQueueType );

	m_bUseUlaw = dlg->bUseUlaw;
	m_bPlayAgain = theApp.UserOptions.repeatPlay;
	m_bForceHdx = theApp.UserOptions.forceHdx;
	m_bFilterNoise = theApp.UserOptions.noiseFilter;

	m_bSetupVolume = dlg->bSetupVolume;
	m_csTestVolumeFile = theApp.UserOptions.testVolumeFile;

	UpdateData(FALSE);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

BEGIN_MESSAGE_MAP(DAPreferencesDlg, CDialog)
	ON_CBN_SELCHANGE(IDC_COMBO_STARTUP_TYPE, OnCbnSelchangeComboStartupType)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDC_CHECK_AUDIBLE_NOTIFICATION, OnBnClickedCheckAudibleNotification)
	ON_BN_CLICKED(IDC_CHECK_CALL_ANNOUNCE, OnBnClickedCheckCallAnnounce)
	ON_CBN_SELCHANGE(IDC_COMBO_WAVE_OUTPUT_DEVICE, OnCbnSelchangeComboWaveOutputDevice)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_PREFERENCES, OnTcnSelchangeTabPreferences)
	ON_CBN_SELCHANGE(IDC_COMBO_WAVE_INPUT_DEVICE, OnCbnSelchangeComboWaveInputDevice)
	ON_BN_CLICKED(IDC_CHECK_MICROPHONE_ULAW, OnBnClickedCheckMicrophoneUlaw)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_TEST_VOLUME_FILE, OnBnClickedButtonBrowseTestVolumeFile)
END_MESSAGE_MAP()

// DAPreferencesDlg message handlers


void DAPreferencesDlg::OnCbnSelchangeComboStartupType()
{
	// TODO: Add your control notification handler code here
	m_comboStartupLanguage.EnableWindow( m_comboStartupType.GetCurSel() );
}

void DAPreferencesDlg::OnBnClickedOk()
{
	CTalkMasterConsoleDlg *dlg = (CTalkMasterConsoleDlg*)theApp.m_pMainWnd;

	UpdateData();

	theApp.UserOptions.startupAudioType = m_comboStartupType.GetCurSel();
	theApp.UserOptions.startupAudioSound = m_comboStartupLanguage.GetCurSel();

	theApp.UserOptions.pingInterval = max(20, atoi(m_csPingInterval));
	theApp.UserOptions.callAnnounceAudible = m_bAudibleNotification;

	theApp.UserOptions.useBeeps = m_bBeeps;
	theApp.UserOptions.callAnnounce = m_bCallAnnouncement;
	theApp.UserOptions.hideWindow = m_bHide;
	theApp.UserOptions.PTT = m_bPTT;
	theApp.UserOptions.recordCallsWaiting = m_bRecordCallsWaiting;
	theApp.UserOptions.callAnnounceVisual = m_bVisualNotification;

	theApp.UserOptions.callAnnounceSound = m_comboAnnouncementSound.GetCurSel();

	theApp.UserOptions.waveInputDevice = m_comboWaveInputDevice.GetCurSel()-1;
	theApp.UserOptions.waveOutputDevice = m_comboWaveOutputDevice.GetCurSel()-1;
	theApp.UserOptions.useUlaw = m_bUseUlaw;

	theApp.UserOptions.callAnnounceQueueAge = m_nPrefQueueAge;
	theApp.UserOptions.callAnnounceQueueSize = m_nPrefQueueSize;
	theApp.UserOptions.callAnnounceQueueType = m_comboPrefQueueAgeType.GetCurSel();

	dlg->m_announceAgeTrigger = theApp.UserOptions.callAnnounceQueueAge;
	if( theApp.UserOptions.callAnnounceQueueType )
		dlg->m_announceAgeTrigger *= 60;						// Minutes
	dlg->m_announceSizeTrigger = theApp.UserOptions.callAnnounceQueueSize;

	dlg->m_staticStatusPTT.EnableWindow( theApp.UserOptions.PTT );

	dlg->bUseUlaw = m_bUseUlaw;

	dlg->bSetupVolume = m_bSetupVolume;

	strcpy( theApp.UserOptions.testVolumeFile, m_csTestVolumeFile );

	theApp.UserOptions.repeatPlay = m_bPlayAgain;
	theApp.UserOptions.forceHdx = m_bForceHdx;
	theApp.UserOptions.noiseFilter = m_bFilterNoise;

	dlg->audioPlayer.Close();
	dlg->audioCQPlayer.Close();

	dlg->audioPlayer.Open(FALSE);		// Open both in and out devices
	dlg->audioCQPlayer.Open(TRUE);		// Open only out device

	OnOK();
}

void DAPreferencesDlg::OnBnClickedCheckAudibleNotification()
{
	// TODO: Add your control notification handler code here
	UpdateData();
	m_comboAnnouncementSound.EnableWindow( m_bAudibleNotification );
}

void DAPreferencesDlg::OnBnClickedCheckCallAnnounce()
{
	// TODO: Add your control notification handler code here
	UpdateData();

	m_checkAudibleNotification.EnableWindow( m_bCallAnnouncement );
	m_checkVisualNotification.EnableWindow( m_bCallAnnouncement );
	m_comboAnnouncementSound.EnableWindow( m_bCallAnnouncement && m_bAudibleNotification );

	m_editPrefQueueAge.EnableWindow( m_bCallAnnouncement );
	m_editPrefQueueSize.EnableWindow( m_bCallAnnouncement );
	m_comboPrefQueueAgeType.EnableWindow( m_bCallAnnouncement );
}

void DAPreferencesDlg::loadWaveInputDevices()
{
	CTalkMasterConsoleDlg *dlg = (CTalkMasterConsoleDlg*)theApp.m_pMainWnd;
	int i, numDevs;
	WAVEINCAPS WOC;

    numDevs = waveInGetNumDevs();
	
	m_comboWaveInputDevice.InsertString(0, dlg->getResourceString(IDS_STRING146));

	for(i=0;i<numDevs;i++)
	{
		waveInGetDevCaps(i, &WOC, sizeof(WOC));
		m_comboWaveInputDevice.InsertString(i+1, WOC.szPname);
	}
}

void DAPreferencesDlg::loadWaveOutputDevices()
{
	CTalkMasterConsoleDlg *dlg = (CTalkMasterConsoleDlg*)theApp.m_pMainWnd;
	int i, numDevs;
	WAVEOUTCAPS WOC;

    numDevs = waveOutGetNumDevs();

	m_comboWaveOutputDevice.InsertString(0, dlg->getResourceString(IDS_STRING146));

	for(i=0;i<numDevs;i++)
	{
		waveOutGetDevCaps(i, &WOC, sizeof(WOC));
		m_comboWaveOutputDevice.InsertString(i+1, WOC.szPname);
	}
}

void DAPreferencesDlg::hidePage(int pageNum)
{
	int i=0;
	int *item = pages[pageNum];

	while( item[i] )
		::ShowWindow(GetDlgItem(item[i++])->m_hWnd, SW_HIDE);
}

void DAPreferencesDlg::showPage(int pageNum)
{
	int i;

	for(i=0;i<4;i++)
	{
		if( i!=pageNum )
			hidePage(i);
	}

	switch(pageNum)
	{
	case 0:					// Main
		showMain();
		break;

	case 1:
		showDevices();
		break;

	case 2:
		if( m_mode )
			showConsole();
		else
			showLocal();

		break;
	}
}

void DAPreferencesDlg::showMain()
{
	RECT view;
	int	cx, cy;

	this->GetClientRect(&view);
	cx = view.right - view.left;
	cy = view.bottom - view.top;

	m_checkHide.SetWindowPos(NULL, 30, 40, 0, 0,
			SWP_NOOWNERZORDER | SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOSIZE);	// SWP_NOMOVE

	m_checkPTT.SetWindowPos(NULL, 30, 60, 0, 0,
			SWP_NOOWNERZORDER | SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOSIZE);	// SWP_NOMOVE

	m_checkCallAnnouncement.SetWindowPos(NULL, 30, 80, 0, 0,
			SWP_NOOWNERZORDER | SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOSIZE);	// SWP_NOMOVE

// Announce Group

	m_staticPrefCallAnnounceGroup.SetWindowPos(NULL, 20, 110, 287, 70,
			SWP_NOOWNERZORDER | SWP_SHOWWINDOW | SWP_NOZORDER );	// SWP_NOMOVE

	m_comboAnnouncementSound.SetWindowPos(NULL, 170, 140, 0, 0,
			SWP_NOOWNERZORDER | SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOSIZE);	// SWP_NOMOVE

	m_checkVisualNotification.SetWindowPos(NULL, 30, 130, 0, 0,
			SWP_NOOWNERZORDER | SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOSIZE);	// SWP_NOMOVE

	m_checkAudibleNotification.SetWindowPos(NULL, 30, 155, 0, 0,
			SWP_NOOWNERZORDER | SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOSIZE);	// SWP_NOMOVE

// Rules Group

	m_staticPrefAnnounceRulesGroup.SetWindowPos(NULL, 20, 190, 287, 70,
			SWP_NOOWNERZORDER | SWP_SHOWWINDOW | SWP_NOZORDER );	// SWP_NOMOVE

	m_staticPrefQueueAge.SetWindowPos(NULL, 30, 207, 0, 0,
			SWP_NOOWNERZORDER | SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOSIZE );	// SWP_NOMOVE

	m_editPrefQueueAge.SetWindowPos(NULL, 110, 205, 100, 20,
			SWP_NOOWNERZORDER | SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOSIZE );	// SWP_NOMOVE

	m_comboPrefQueueAgeType.SetWindowPos(NULL, 175, 205, 140, 20,
			SWP_NOOWNERZORDER | SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOSIZE );	// SWP_NOMOVE

	m_staticPrefQueueSize.SetWindowPos(NULL, 30, 232, 0, 0,
			SWP_NOOWNERZORDER | SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOSIZE );	// SWP_NOMOVE

	m_editPrefQueueSize.SetWindowPos(NULL, 110, 230, 100, 20,
			SWP_NOOWNERZORDER | SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOSIZE );	// SWP_NOMOVE

	m_staticPrefQueueEntries.SetWindowPos(NULL, 175, 232, 0, 0,
			SWP_NOOWNERZORDER | SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOSIZE  );	// SWP_NOMOVE
}

void DAPreferencesDlg::showDevices()
{
	RECT view;
	int	cx, cy;

	this->GetClientRect(&view);
	cx = view.right - view.left;
	cy = view.bottom - view.top;

	m_staticPrefWaveOutGroup.SetWindowPos(NULL, 20, 40, 0, 0,
			SWP_NOOWNERZORDER | SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOSIZE);	// SWP_NOMOVE
	
	m_comboWaveOutputDevice.SetWindowPos(NULL, 30, 55, 0, 0,
			SWP_NOOWNERZORDER | SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOSIZE);	// SWP_NOMOVE

	m_staticPrefWaveInGroup.SetWindowPos(NULL, 20, 90, 0, 0,
			SWP_NOOWNERZORDER | SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOSIZE);	// SWP_NOMOVE

	m_comboWaveInputDevice.SetWindowPos(NULL, 30, 105, 0, 0,
			SWP_NOOWNERZORDER | SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOSIZE);	// SWP_NOMOVE

	m_checkUseUlaw.SetWindowPos(NULL, 30, 145, 0, 0,
			SWP_NOOWNERZORDER | SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOSIZE);	// SWP_NOMOVE

	m_checkSetupVolume.SetWindowPos(NULL, 30, 165, 0, 0,
			SWP_NOOWNERZORDER | SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOSIZE);	// SWP_NOMOVE

	m_staticTestVolumeFileGroup.SetWindowPos(NULL, 20, 190, 0, 0,
			SWP_NOOWNERZORDER | SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOSIZE);	// SWP_NOMOVE

	m_editTestVolumeFile.SetWindowPos(NULL, 30, 205, 0, 0,
			SWP_NOOWNERZORDER | SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOSIZE);	// SWP_NOMOVE

	m_btnBrowseTestVolumeFile.SetWindowPos(NULL, 230, 205, 0, 0,
			SWP_NOOWNERZORDER | SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOSIZE);	// SWP_NOMOVE

	if( m_bDADebug )
	{
		m_btnForceHdx.SetWindowPos(NULL, 30, 245, 0, 0,
				SWP_NOOWNERZORDER | SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOSIZE);	// SWP_NOMOVE

		m_btnNoiseFilter.SetWindowPos(NULL, 30, 265, 0, 0,
				SWP_NOOWNERZORDER | SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOSIZE);	// SWP_NOMOVE

		m_btnPlayAgain.SetWindowPos(NULL, 30, 285, 0, 0,
				SWP_NOOWNERZORDER | SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOSIZE);	// SWP_NOMOVE

		m_btnShowDebug.SetWindowPos(NULL, 30, 305, 0, 0,
				SWP_NOOWNERZORDER | SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOSIZE);	// SWP_NOMOVE
	}
}

void DAPreferencesDlg::showConsole()
{
}

void DAPreferencesDlg::showLocal()
{
}

void DAPreferencesDlg::OnTcnSelchangeTabPreferences(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	showPage(TabCtrl_GetCurSel(pNMHDR->hwndFrom));

	*pResult = 0;
}

void DAPreferencesDlg::OnCbnSelchangeComboWaveOutputDevice()
{
	CTalkMasterConsoleDlg *dlg = (CTalkMasterConsoleDlg*)theApp.m_pMainWnd;

	if( !m_bWarning )
		MessageBox(dlg->getResourceString(IDS_STRING147), dlg->getResourceString(IDS_STRING148));

	m_bWarning = TRUE;
}

void DAPreferencesDlg::OnCbnSelchangeComboWaveInputDevice()
{
}

void DAPreferencesDlg::OnBnClickedCheckMicrophoneUlaw()
{
}


void DAPreferencesDlg::OnBnClickedButtonBrowseTestVolumeFile()
{
	CTalkMasterConsoleDlg *dlg = (CTalkMasterConsoleDlg*)theApp.m_pMainWnd;

	UpdateData(TRUE);

//_T("Chart Files (*.xlc)|*.xlc|Worksheet Files (*.xls)|*.xls|Data Files (*.xlc;*.xls)|*.xlc; *.xls|All Files (*.*)|*.*||");
	
	CFileDialog pFD(	TRUE, 
						_T(dlg->getResourceString(IDS_STRING_WAV)),
						NULL,
						OFN_OVERWRITEPROMPT|OFN_HIDEREADONLY,
						dlg->getResourceString(IDS_STRING_WAV_FILE_FILTER),
						this);

	if( pFD.DoModal()!=IDOK )
		return;

	m_csTestVolumeFile = pFD.GetPathName().GetBuffer();

	UpdateData(FALSE);
}
