//
// DAUnattendedDlg.cpp : implementation file
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
#include "AnnounceDlg.h"

void CTalkMasterConsoleDlg::initAnnounce()
{
	time_t now;

	time(&now);

	m_announceAgeTrigger = theApp.UserOptions.callAnnounceQueueAge;
	if( theApp.UserOptions.callAnnounceQueueType )
		m_announceAgeTrigger *= 60;						// Minutes
	m_announceSizeTrigger = theApp.UserOptions.callAnnounceQueueSize;
//	m_announceGuardTime = now;


	strcpy( soundFile[0], theApp.UserOptions.path );		// Build the chime filename that we are going to send
	strcat( soundFile[0], "\\iAudio\\custom_Announcement.wav" );

	strcpy( soundFile[1], theApp.UserOptions.path );		// Build the chime filename that we are going to send
	strcat( soundFile[1], "\\iAudio\\doorbell_Announcement.wav" );
}

void CTalkMasterConsoleDlg::announceCheck()
{
	CTalkMasterConsoleDlg *dlg = (CTalkMasterConsoleDlg*)theApp.m_pMainWnd;
	time_t now;

	if( !bLoggedOn ||
		!m_announceStartTime || 
		m_sessionSocket || 
		!theApp.UserOptions.callAnnounce || 
		m_listCallsWaiting.GetItemCount() == 0 ||
		!getCallsWaitingItemData(0) ||
		0 )
	{
		stopAnnounce();
		return;
	}

	if( dlg->m_tabMain.GetCurSel()==1 && m_playFileSocket )
		return;


	time(&now);

	if( ((int)(now-m_announceStartTime) >= m_announceAgeTrigger) &&
		( m_listCallsWaiting.GetItemCount() >= m_announceSizeTrigger) &&
		( !m_announceGuardTime || (now-m_announceGuardTime) > 30 )  && 
		  !bSetupVolume )
		announceActivity();
	else
	{
		outputDebug(_T("announceCheck( (%d>=%d) && (%d>=%d) && (NOT %s || %d>30) )"),
			(now-m_announceStartTime), m_announceAgeTrigger,
			(m_listCallsWaiting.GetItemCount()), m_announceSizeTrigger,
			m_announceGuardTime?"TRUE":"FALSE", (now-m_announceGuardTime));
	}

	if( m_announceGuardTime && (now-m_announceGuardTime) > 30 )
		m_announceGuardTime = 0;
}

void CTalkMasterConsoleDlg::announceActivity()
{
	INT_PTR nResponse;
	DWORD	threadID1, threadID2;
	static int guard = 0;

	if( ++guard > 1 )
	{
		--guard;
		return;
	}

//	stopAnnounce();

	if( theApp.UserOptions.callAnnounceAudible ||
		theApp.UserOptions.callAnnounceVisual )
		startAlert();

	if( GetForegroundWindow()==NULL )
	{
		--guard;
		return;
	}

	bOffDialog = TRUE;

	nResponse = announceDlg.DoModal();

	bOffDialog = FALSE;
	bSkipNextKeyUp = TRUE;

	if (nResponse == IDOK)
	{
		//  dismissed with OK

		if( m_hWnd != GetForegroundWindow()->m_hWnd )
		{
			threadID1 = GetWindowThreadProcessId( GetForegroundWindow()->m_hWnd, NULL );
			threadID2 = GetWindowThreadProcessId( m_hWnd, NULL );

			if( threadID1 != threadID2 )
			{
				AttachThreadInput( threadID1, threadID2, TRUE );
				SetForegroundWindow();

				::SetForegroundWindow(m_hWnd);
				AttachThreadInput( threadID1, threadID2, FALSE );
			}
			else
				this->SetForegroundWindow();
		}

		if( this->IsIconic() )
			ShowWindow(SW_RESTORE);
		else
			ShowWindow(SW_SHOW);

		m_tabMain.SetCurSel(0);
		displayTabMain();
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	if( theApp.UserOptions.callAnnounceAudible ||
		theApp.UserOptions.callAnnounceVisual )
		stopAlert();

	time(&m_announceGuardTime);
	resetAnnounce();

	--guard;
}

void CTalkMasterConsoleDlg::resetAnnounce()
{
    if(	m_announceStartTime = oldestCQEntry() )
		SetTimer(TIMER_ANNOUNCE, 1000, NULL);
	else
		stopAnnounce();
}

void CTalkMasterConsoleDlg::stopAnnounce()
{
	m_announceStartTime = 0;

	if( announceDlg.bActive )
	{
//		announceDlg.bActive = FALSE;
//		announceDlg.EndDialog(2);
	}

	m_announceGuardTime = 0;

	KillTimer(TIMER_ANNOUNCE);
}

void CTalkMasterConsoleDlg::startAlert()
{
	if( !m_announceRunning )
	{
		m_announceRunning = TRUE;
		SetTimer(TIMER_ALERT, 1, NULL);
	}
}

void CTalkMasterConsoleDlg::stopAlert()
{
	m_announceRunning = FALSE;

	KillTimer(TIMER_ALERT);
}

void CTalkMasterConsoleDlg::doAlerts()
{
	outputDebug(_T("doAlerts()"));

	if( m_announceRunning )
		SetTimer(TIMER_ALERT, 5000, NULL);

	if( theApp.UserOptions.callAnnounceAudible )
		doAudibleAlert();

	if( theApp.UserOptions.callAnnounceVisual )
		doVisualAlert();

	if( !announceTest() && announceDlg.bActive )
	{
		announceDlg.bActive = FALSE;
		announceDlg.EndDialog(2);

//		stopAlert();
	}
}

void CTalkMasterConsoleDlg::doAudibleAlert()
{
	if( announceTest() )
	{
		if( theApp.UserOptions.startupAudioType == 1 )			// Play file
			sndPlaySound( soundFile[theApp.UserOptions.callAnnounceSound], SND_ASYNC );
	}
}

void CTalkMasterConsoleDlg::doVisualAlert()
{
//	if( announceTest() )
//		sndPlaySound( soundFile[theApp.UserOptions.startupAudioType], SND_ASYNC );
}

BOOL CTalkMasterConsoleDlg::announceTest()
{
	BOOL	bRet;
	time_t	now;

	void *itemData = (void *)getCallsWaitingItemData(0);				// A test to see if there is anything in the call Queue

	time(&now);

	outputDebug(_T("Age Test is %d > %d"), (int)(now-m_announceStartTime), m_announceAgeTrigger );
	outputDebug(_T("Size Test is %d > %d"), m_listCallsWaiting.GetItemCount(), m_announceSizeTrigger );
	if( m_announceGuardTime )
		outputDebug(_T("Guard test is %d waiting for 30"), (now-m_announceGuardTime));
	else
		outputDebug(_T("Guard test is inactive"));

	bRet = ( ((int)(now-m_announceStartTime) >= m_announceAgeTrigger) &&
		( itemData && m_listCallsWaiting.GetItemCount() >= m_announceSizeTrigger) &&
		( !m_announceGuardTime || (now-m_announceGuardTime) > 30 ) );

	outputDebug(_T("announceTest %s"), (bRet)?"TRUE":"FALSE");

	return(bRet);
}

