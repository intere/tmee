//
// DAPassThruTest.cpp : Defines the class behaviors for the application.
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
#include "CameraDataManager.h"
#include "direct.h"


#include "shlobj.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CDAPassThruTestApp

BEGIN_MESSAGE_MAP(CTalkMasterConsoleApp, CWinApp)
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
	ON_COMMAND(ID_FILE_EXIT, OnFileExit)
	ON_COMMAND(ID_VIEW_PREFERENCES, OnViewPreferences)
	ON_COMMAND(ID_FILE_CAPTUREARCHIVEAUDIO, OnFileCapturearchiveaudio)
	ON_COMMAND(ID_TOOLS_RECORDPROGRAMEVENTSTOFILE, OnToolsRecordprogrameventstofile)
	ON_COMMAND(ID_TOOLS_OPENSUPPORTFOLDER, OnToolsOpenSupportFolder)
	ON_COMMAND(ID_HELP_ABOUT, OnHelpAbout)
	ON_COMMAND(ID_HELP_DIGITALACOUSTICSWEBSITE, OnHelpDigitalacousticswebsite)
	ON_COMMAND(ID_HELP_ONLINEDOCUMENTATION, OnHelpOnlinedocumentation)
	ON_COMMAND(ID_FILE_LOGOFF, OnFileLogoff)
	ON_COMMAND(ID_SCREENPOSITION_SAVECURRENTVIEW, OnScreenpositionSavecurrentview)
	ON_COMMAND(ID_SCREENPOSITION_RESETTODEFAULT, OnScreenpositionResettodefault)
	ON_COMMAND(ID_HELP_HELPONUSINGTALKMASTER, OnHelpHelponusingtalkmaster)
END_MESSAGE_MAP()


// CDAPassThruTestApp construction

CTalkMasterConsoleApp::CTalkMasterConsoleApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CDAPassThruTestApp object

CTalkMasterConsoleApp theApp;


// CDAPassThruTestApp initialization

BOOL CTalkMasterConsoleApp::InitInstance()
{
	// InitCommonControls() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	InitCommonControls();

	CWinApp::InitInstance();

	AfxEnableControlContainer();
#ifdef __SKINS
	CXTPWinDwmWrapper().SetProcessDPIAware();
#endif

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization

	SetRegistryKey(_T("Digital Acoustics"));		// How does the "DAPassThruTest" name get set?

	CTalkMasterConsoleDlg dlg;

	dlg.loadAlternateResourceDLL();

	m_pMainWnd = &dlg;

//	StartTimer(1);

//	tic = getTimer();
//	tic2 = getTimer();

	ReadRegistry();

//	UpdateMenu();

	EnableHtmlHelp();

	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

//	OutputDebugString("WriteRegistry\n");
	WriteRegistry();

//	releaseTimer(tic);
//	releaseTimer(tic2);

//	StopTimer();

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
#if 0
	if( _CrtCheckMemory() )
		OutputDebugString("Memory OK");
	else
		OutputDebugString("Memory Failed");
#endif

	return FALSE;
}

void CTalkMasterConsoleApp::OnFileExit()
{
	// TODO: Add your command handler code here
	CTalkMasterConsoleDlg *dlg = (CTalkMasterConsoleDlg*)theApp.m_pMainWnd;

	dlg->outputDebug("OnFileExit: Calling Cancel");

	dlg->doCancel();

//	dlg->EndDialog(IDCANCEL);
}

void CTalkMasterConsoleApp::OnViewPreferences()
{
	CTalkMasterConsoleDlg *mDlg = (CTalkMasterConsoleDlg*)theApp.m_pMainWnd;
	DAPreferencesDlg dlg;

	int showMode = SW_HIDE;

	RECT view;
	int	cx, cy;

	mDlg->GetClientRect(&view);

	cx = view.right - view.left;
	cy = view.bottom - view.top;

//	dlg.m_csPingInterval.Format("%d", UserOptions.pingInterval);
//	dlg.m_bAudibleNotification = UserOptions.callAnnounceAudible;

	mDlg->bOffDialog = TRUE;							// Not on the main dialog

	INT_PTR nResponse = dlg.DoModal();

	mDlg->bOffDialog = FALSE;							// Not on the main dialog
	mDlg->bSkipNextKeyUp = TRUE;

	if (nResponse == IDOK)
	{
		//  dismissed with OK

		if( UserOptions.hideWindow )
			mDlg->showSystemTray();
		else
			mDlg->delSystemTray();
//		UserOptions.pingInterval = atoi(dlg.m_csPingInterval);

		if( mDlg->bUseUlaw )
			mDlg->m_staticCodec.SetWindowText(mDlg->getResourceString(IDS_STRING_ULAW));
		else
			mDlg->m_staticCodec.SetWindowText(mDlg->getResourceString(IDS_STRING_PCM));

		if( dlg.m_bShowDebug )
			showMode = SW_SHOW;

		if( mDlg->m_bTestMode && showMode == SW_HIDE )				// Currently running and we are hiding the controls
		{
			mDlg->m_btnTestMode.SetCheck(BST_UNCHECKED);
			mDlg->OnBnClickedCheckTest();
		}

		mDlg->m_btnTestMode.ShowWindow(showMode);
		mDlg->m_ctrlMaxMsec.ShowWindow(showMode);
		mDlg->m_staticMinMsecText.ShowWindow(showMode);
		mDlg->m_cstaticMaxMsec.ShowWindow(showMode);

		if( mDlg->bSetupVolume )									// If we are setting up the volume, we need to hide
		{															// the call queue stuff
			mDlg->m_listCallsWaiting.ShowWindow(SW_HIDE);
			mDlg->m_btnPlayPause.ShowWindow(SW_HIDE);
			mDlg->m_btnMute.ShowWindow(SW_HIDE);
			mDlg->m_btnStop.ShowWindow(SW_HIDE);
			mDlg->m_sliderCQPlayer.ShowWindow(SW_HIDE);
			mDlg->m_groupboxPlayer.ShowWindow(SW_HIDE);

			mDlg->m_ctrlSetupVolume.SetWindowPos(mDlg, 15, cy-150, cx-289, 50, 
						SWP_SHOWWINDOW | SWP_FRAMECHANGED | SWP_NOZORDER );
			mDlg->m_ctrlVolumeSetting.SetWindowPos(mDlg, 25, cy-100, cx-309, 20, 
						SWP_SHOWWINDOW | SWP_FRAMECHANGED | SWP_NOZORDER );
			mDlg->m_btnSetVolume.SetWindowPos(mDlg, cx-269, cy-150, 0, 0,
						SWP_SHOWWINDOW | SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOSIZE );
			mDlg->m_btnGetVolume.SetWindowPos(mDlg, cx-269, cy-100, 0, 0,
						SWP_SHOWWINDOW | SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOSIZE );
			mDlg->m_btnTestVolume.SetWindowPos(mDlg, cx-269, cy-65, 0, 0,
						SWP_SHOWWINDOW | SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOSIZE );

			mDlg->m_btnTestVolume.EnableWindow(mDlg->m_pSelectedItem != NULL && GetFileAttributes(UserOptions.testVolumeFile)!=INVALID_FILE_ATTRIBUTES);

			mDlg->m_ctrlSetupVolume.SetRange(1, 7);
			mDlg->m_ctrlSetupVolume.SetPos(4);

			mDlg->m_ctrlVolumeSetting.SetRange(0, 7);
			mDlg->m_ctrlVolumeSetting.SetPos(0);
		}
		else
		{
			mDlg->m_ctrlSetupVolume.ShowWindow(SW_HIDE);
			mDlg->m_ctrlVolumeSetting.ShowWindow(SW_HIDE);
			mDlg->m_btnSetVolume.ShowWindow(SW_HIDE);
			mDlg->m_btnGetVolume.ShowWindow(SW_HIDE);
			mDlg->m_btnTestVolume.ShowWindow(SW_HIDE);

			mDlg->m_listCallsWaiting.ShowWindow(SW_SHOW);
			mDlg->m_btnPlayPause.ShowWindow(SW_SHOW);
			mDlg->m_btnMute.ShowWindow(SW_SHOW);
			mDlg->m_btnStop.ShowWindow(SW_SHOW);
			mDlg->m_sliderCQPlayer.ShowWindow(SW_SHOW);
			mDlg->m_groupboxPlayer.ShowWindow(SW_SHOW);

			mDlg->doSize();
		}

	}
	else if (nResponse == IDCANCEL)
	{
		//  dismissed with Cancel
	}
}

void CTalkMasterConsoleApp::ReadRegistry()
{
	CTalkMasterConsoleDlg *dlg = (CTalkMasterConsoleDlg*)theApp.m_pMainWnd;
	CString csPath, csError;
	WAVEINCAPS WIC;
	WAVEOUTCAPS WOC;
	int i, maxDevice;

	UserOptions.fpRecordProgramEvents = NULL;
	UserOptions.recordEventsStack = NULL;

	UserOptions.callAnnounceAudible = GetProfileInt("options", "audibleNotification", 1);
	UserOptions.callAnnounceVisual = GetProfileInt("options", "visualNotification", 1);
	UserOptions.callAnnounceSound = GetProfileInt("options", "audibleSoundType", 1);

	UserOptions.callAnnounceQueueAge = GetProfileInt("options", "callAnnounceQueueAge", 0);
	UserOptions.callAnnounceQueueType = GetProfileInt("options", "callAnnounceQueueType", 0);
	UserOptions.callAnnounceQueueSize = GetProfileInt("options", "callAnnounceQueueSize", 1);

	UserOptions.PTT = GetProfileInt("options", "mousePTT", 1);
	UserOptions.callAnnounce = GetProfileInt("options", "callButton", 1);
	UserOptions.recordCallsWaiting = GetProfileInt("options", "RecordAudio", 1);

	UserOptions.pingInterval = GetProfileInt("options", "pingtime", 20);
	UserOptions.useBeeps = GetProfileInt("options", "callBeeps", 1);

	UserOptions.startupAudioType = GetProfileInt("options", "startupAudioType", 1);
	UserOptions.startupAudioSound = GetProfileInt("options", "startupAudioSound", 2);

	UserOptions.cx = GetProfileInt("settings", "startWidth", 0);
	UserOptions.cy = GetProfileInt("settings", "startHeight", 0);
	UserOptions.hideWindow = GetProfileInt("settings", "hideMinimized", FALSE);

	csPath = GetProfileString("settings", "path", "");
	if( csPath == "" )
		getcwd( UserOptions.path, MAX_PATH );
	else
		strcpy( UserOptions.path, csPath );

	if( strlen(UserOptions.path) && UserOptions.path[strlen(UserOptions.path)-1] == '\\' )
		UserOptions.path[strlen(UserOptions.path)-1] = 0;

	strcpy( UserOptions.pathTemp, getenv("temp") );
	if( strlen(UserOptions.pathTemp) && UserOptions.pathTemp[strlen(UserOptions.pathTemp)-1] == '\\' )
		UserOptions.pathTemp[strlen(UserOptions.pathTemp)-1] = 0;

	csPath = UserOptions.pathTemp;
	csPath += "\\CallQueue\\";

	DeleteFiles( csPath.GetBuffer() );
	rmdir(csPath);

	if( mkdir(csPath) )
	{
		csError.Format( _T(dlg->getResourceString(IDS_STRING153)), csPath );
		MessageBox(dlg->m_hWnd, csError, dlg->getResourceString(IDS_STRING154), MB_OK);
	}

	SHGetSpecialFolderPath(NULL, UserOptions.pathStore, CSIDL_APPDATA, FALSE);
#ifdef PRIMEX_BUILD
	PathAppend(UserOptions.pathStore, "Primex Wireless");
#else
	PathAppend(UserOptions.pathStore, "Digital Acoustics");
#endif
	mkdir(UserOptions.pathStore);

	UserOptions.archiveAudio = GetProfileInt("settings", "archiveAudio", 0);

	csPath = GetProfileString("settings", "IPAddress", "0.0.0.0");
	strcpy( UserOptions.IPAddress, csPath);

	UserOptions.IPPort = GetProfileInt("settings", "IPPort", 3010);

	UserOptions.recordProgramEvents = GetProfileInt("settings", "recordEvents", 0);

	UserOptions.listenModeDefault = GetProfileInt("settings", "listenModeDefault", 0);

	UserOptions.waveInputDeviceName = GetProfileString("options", "waveInputDeviceName", "");
	UserOptions.waveInputDevice = GetProfileInt("options", "waveInputDevice", WAVE_MAPPER);
	if( UserOptions.waveInputDeviceName.IsEmpty() )
	{
		waveInGetDevCaps(UserOptions.waveInputDevice, &WIC, sizeof(WIC));		
		UserOptions.waveInputDeviceName = WIC.szPname;
	}

	UserOptions.waveOutputDeviceName = GetProfileString("options", "waveOutputDeviceName", "");
	UserOptions.waveOutputDevice = GetProfileInt("options", "waveOutputDevice", WAVE_MAPPER);
	if( UserOptions.waveOutputDeviceName.IsEmpty() )
	{
		waveOutGetDevCaps(UserOptions.waveOutputDevice, &WOC, sizeof(WOC));		
		UserOptions.waveOutputDeviceName = WOC.szPname;
	}

	UserOptions.useUlaw = GetProfileInt("options", "useUlaw", FALSE);
	dlg->bUseUlaw = UserOptions.useUlaw;

#ifdef PRIMEX_BUILD
	csPath = GetProfileString("program", "onlineDocsURL", "http://www.primexwireless/ipi");
#else
	csPath = GetProfileString("program", "onlineDocsURL", "http://www.digitalacoustics.com/enterprise/index.htm");
#endif
	strcpy( UserOptions.onlineDocsURL, csPath );

#ifdef PRIMEX_BUILD
	csPath = GetProfileString("program", "onlineWebURL", "http://www.primexwireless.com");
#else
	csPath = GetProfileString("program", "onlineWebURL", "http://www.digitalacoustics.com");
#endif
	strcpy( UserOptions.onlineWebURL, csPath );

#ifdef PRIMEX_BUILD
	csPath = GetProfileString("program", "onlineWebMenu", "Primex Wireless Web Site");
#else
	csPath = GetProfileString("program", "onlineWebMenu", "Digital Acoustics Web Site");
#endif
	strcpy( UserOptions.onlineWebMenu, csPath );

	UserOptions.startSize.top = GetProfileInt("settings", "startTop", 0);
	UserOptions.startSize.bottom = GetProfileInt("settings", "startBottom", 0);
	UserOptions.startSize.left = GetProfileInt("settings", "startLeft", 0);
	UserOptions.startSize.right = GetProfileInt("settings", "startRight", 0);

	dlg->m_threadPriority = GetProfileInt("options", "threadPriority", THREAD_BASE_PRIORITY_MAX+12);

    maxDevice = waveInGetNumDevs();
	for(i=0;i<maxDevice;i++)
	{
		waveInGetDevCaps(i, &WIC, sizeof(WIC));		
		if( !strcmp(WIC.szPname, UserOptions.waveInputDeviceName) )
			break;
	}

	if( i == maxDevice )
		UserOptions.waveInputDevice = -1;
	else
		UserOptions.waveInputDevice = i;

    maxDevice = waveOutGetNumDevs();
	for(i=0;i<maxDevice;i++)
	{
		waveOutGetDevCaps(i, &WOC, sizeof(WOC));		
		if( !strcmp(WOC.szPname, UserOptions.waveOutputDeviceName) )
			break;
	}

	if( i == maxDevice )
		UserOptions.waveOutputDevice = -1;
	else
		UserOptions.waveOutputDevice = i;

	dlg->audioPlayer.Close();
	dlg->audioPlayer.Open(FALSE);		// Open both in and out
	dlg->audioCQPlayer.Close();
	dlg->audioCQPlayer.Open(TRUE);		// Open only the out device

	csPath = GetProfileString("settings", "testVolumeFile", "");
	strcpy( UserOptions.testVolumeFile, csPath );

	UserOptions.forceHdx = GetProfileInt("settings", "forceHdx", FALSE);
	UserOptions.noiseFilter = GetProfileInt("settings", "filterNoise", FALSE);

	CameraDataManager::getInstance().ReadRegistry(this, UserOptions);
}

void CTalkMasterConsoleApp::WriteRegistry()
{
	WAVEINCAPS WIC;
	WAVEOUTCAPS WOC;

	WriteProfileInt("options", "audibleNotification", UserOptions.callAnnounceAudible);
	WriteProfileInt("options", "visualNotification", UserOptions.callAnnounceVisual);
	WriteProfileInt("options", "audibleSoundType", UserOptions.callAnnounceSound);

	WriteProfileInt("options", "callAnnounceQueueAge", UserOptions.callAnnounceQueueAge);
	WriteProfileInt("options", "callAnnounceQueueType", UserOptions.callAnnounceQueueType);
	WriteProfileInt("options", "callAnnounceQueueSize", UserOptions.callAnnounceQueueSize);

	WriteProfileInt("options", "mousePTT", UserOptions.PTT);
	WriteProfileInt("options", "callButton", UserOptions.callAnnounce);
	WriteProfileInt("options", "RecordAudio", UserOptions.recordCallsWaiting);

	WriteProfileInt("options", "pingtime", UserOptions.pingInterval);
	WriteProfileInt("options", "callBeeps", UserOptions.useBeeps);

	WriteProfileInt("options", "startupAudioType", UserOptions.startupAudioType);
	WriteProfileInt("options", "startupAudioSound", UserOptions.startupAudioSound);

	WriteProfileInt("settings", "startWidth", UserOptions.cx);
	WriteProfileInt("settings", "startHeight", UserOptions.cy);
	WriteProfileInt("settings", "hideMinimized", UserOptions.hideWindow);

	WriteProfileString("settings", "path", UserOptions.path );

	WriteProfileInt("settings", "archiveAudio", UserOptions.archiveAudio);

	WriteProfileString("settings", "IPAddress", UserOptions.IPAddress);
	WriteProfileInt("settings", "IPPort", UserOptions.IPPort);

	WriteProfileInt("settings", "recordEvents", UserOptions.recordProgramEvents);

	WriteProfileInt("settings", "listenModeDefault", UserOptions.listenModeDefault);

//	WriteProfileInt("options", "waveInputDevice", UserOptions.waveInputDevice);
//	WriteProfileInt("options", "waveOutputDevice", UserOptions.waveOutputDevice);

	waveInGetDevCaps(UserOptions.waveInputDevice, &WIC, sizeof(WIC));
	WriteProfileString("options", "waveInputDeviceName", WIC.szPname);

	waveOutGetDevCaps(UserOptions.waveOutputDevice, &WOC, sizeof(WOC));
	WriteProfileString("options", "waveOutputDeviceName", WOC.szPname);

	WriteProfileInt("options", "useUlaw", UserOptions.useUlaw);

	WriteProfileString("settings", "testVolumeFile", UserOptions.testVolumeFile);

	WriteProfileInt("settings", "forceHdx", UserOptions.forceHdx);
	WriteProfileInt("settings", "filterNoise", UserOptions.noiseFilter);

	CameraDataManager::getInstance().WriteRegistry();
}

void CTalkMasterConsoleApp::UpdateMenu()
{
	CMenu *menu;
	CMenu *sub;

	menu = GetMainWnd()->GetMenu();
	if( menu )
	{
		sub = menu->GetSubMenu(4);
		sub->ModifyMenu(3, MF_BYCOMMAND, ID_HELP_DIGITALACOUSTICSWEBSITE, UserOptions.onlineWebMenu);
	}
}

void CTalkMasterConsoleApp::OnFileCapturearchiveaudio()
{
	// TODO: Add your command handler code here

	CMenu *menu;
	CMenu *sub;
	UINT check = MF_CHECKED;

	UserOptions.archiveAudio = ~UserOptions.archiveAudio;
	if( !UserOptions.archiveAudio )
		check = MF_UNCHECKED;

	menu = GetMainWnd()->GetMenu();
	if( menu )
	{
		sub = menu->GetSubMenu(0);
		sub->CheckMenuItem(ID_FILE_CAPTUREARCHIVEAUDIO, check);
	}
}

void CTalkMasterConsoleApp::OnToolsRecordprogrameventstofile()
{
	CTalkMasterConsoleDlg *dlg = (CTalkMasterConsoleDlg *)m_pMainWnd;
	CMenu *menu;
	CMenu *sub;
	UINT check = MF_CHECKED;

	UserOptions.recordProgramEvents = ~UserOptions.recordProgramEvents;
	if( !UserOptions.recordProgramEvents )
		check = MF_UNCHECKED;

	menu = GetMainWnd()->GetMenu();
	if( menu )
	{
		sub = menu->GetSubMenu(2);
		sub->CheckMenuItem(ID_TOOLS_RECORDPROGRAMEVENTSTOFILE, check);
	}

	dlg->OpenCloseDebugFile();
}

void CTalkMasterConsoleApp::OnToolsOpenSupportFolder()
{
	HINSTANCE const xResult = ShellExecute( 
		0,                  // hWnd 
		"open",         // lpOperation 
		UserOptions.pathStore, // lpFile 
		NULL,               // lpParameters 
		NULL,               // lpDirectory 
		SW_SHOWNORMAL       // nShowCommand  --  restore, activate, display. 
		); 
}

void CTalkMasterConsoleApp::OnHelpAbout()
{
	CTalkMasterConsoleDlg *dlg = (CTalkMasterConsoleDlg *)m_pMainWnd;

	dlg->OnAbout();
}

void CTalkMasterConsoleApp::OnHelpDigitalacousticswebsite()
{
	ShellExecute(::GetDesktopWindow(), "open", UserOptions.onlineWebURL, NULL, NULL, SW_SHOWNORMAL);
}

void CTalkMasterConsoleApp::OnHelpOnlinedocumentation()
{
	ShellExecute(::GetDesktopWindow(), "open", UserOptions.onlineDocsURL, NULL, NULL, SW_SHOWNORMAL);
}

void CTalkMasterConsoleApp::OnFileLogoff()
{
	CTalkMasterConsoleDlg *dlg = (CTalkMasterConsoleDlg *)m_pMainWnd;
	BOOL	bWait = dlg->bTalking || dlg->bListening;

	if( dlg->m_pSelectedItem && !theApp.UserOptions.forceHdx &&
		dlg->m_pSelectedItem->iCom.picRev.feature1 & OPNIC_FULL_DUPLEX )
	{
		if( dlg->bTalking )
			dlg->doFdxStartStop();				// Stop it
	}
	else
	{
		if( dlg->bTalking )
		{
			dlg->bTalkRequest = FALSE;
			while( dlg->bTalking )
				dlg->DoEvents();

	//		dlg->endTalk();
		}

		if( dlg->bListening )
		{
			dlg->bListenRequest = FALSE;
			while(dlg->bListening)
				dlg->DoEvents();
	//		dlg->endListen();
		}
	}

	if( bWait )
		Sleep(500);

	dlg->m_csLoginName = "";
	dlg->m_csLoginPassword = "";
	
	if(dlg->m_hDA)
	{
		dlg->m_DAStop( dlg->m_hDA );
	}

//	dlg->doLogon();
}

void CTalkMasterConsoleApp::OnScreenpositionSavecurrentview()
{
	CTalkMasterConsoleDlg *dlg = (CTalkMasterConsoleDlg *)m_pMainWnd;
	RECT view;
	dlg->GetWindowRect(&view);

	WriteProfileInt("settings", "startTop", view.top);
	WriteProfileInt("settings", "startBottom", view.bottom);
	WriteProfileInt("settings", "startLeft", view.left);
	WriteProfileInt("settings", "startRight", view.right);
}

void CTalkMasterConsoleApp::OnScreenpositionResettodefault()
{
	CTalkMasterConsoleDlg *dlg = (CTalkMasterConsoleDlg *)m_pMainWnd;

	WriteProfileInt("settings", "startTop", 0);
	WriteProfileInt("settings", "startBottom", 0);
	WriteProfileInt("settings", "startLeft", 0);
	WriteProfileInt("settings", "startRight", 0);

	dlg->SetWindowPos(NULL, 
	0,
	0,
	567,
	438,
	SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_SHOWWINDOW | SWP_NOZORDER );
}

void CTalkMasterConsoleApp::OnHelpHelponusingtalkmaster()
{
	CTalkMasterConsoleDlg *dlg = (CTalkMasterConsoleDlg *)m_pMainWnd;
	dlg->OnHelp();
}

void CTalkMasterConsoleApp::DeleteFiles(char *folderName)
{
	WIN32_FIND_DATA find;
	CString csAllFiles;
	HANDLE hFind;

	csAllFiles = folderName;
	csAllFiles += "*.*";

	hFind = FindFirstFile( csAllFiles, &find );
	while(hFind != INVALID_HANDLE_VALUE)
	{
		if( !(find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
		{
			csAllFiles = folderName;
			csAllFiles += find.cFileName;
			DeleteFile( csAllFiles );
		}
		if( ! FindNextFile( hFind, &find ) )
			break;
	}

	if( hFind != INVALID_HANDLE_VALUE )
		FindClose( hFind );
}

