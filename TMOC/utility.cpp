//
// Utility.cpp : implementation file
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
#include "windows.h"
#include "windowsx.h"
#include "TalkMasterConsole.h"
#include "TalkMasterConsoleDlg.h"
#include "utility.h"
#include "pleaseWaitDlg.h"
#include "shellapi.h"

void CTalkMasterConsoleDlg::DrawTransparent(CBitmap *bitmap, CDC * pDC, int x, int y, COLORREF crColour)
{
	COLORREF crOldBack = pDC->SetBkColor(m_crWhite);
	COLORREF crOldText = pDC->SetTextColor(m_crBlack);
	CDC dcImage, dcTrans;
	BITMAP	bm;

	// Create two memory dcs for the image and the mask
	dcImage.CreateCompatibleDC(pDC);
	dcTrans.CreateCompatibleDC(pDC);

	// Select the image into the appropriate dc
	CBitmap* pOldBitmapImage = dcImage.SelectObject(bitmap);

	// Create the mask bitmap
	CBitmap bitmapTrans;

	bitmap->GetBitmap(&bm);
	int nWidth = bm.bmWidth;
	int nHeight = bm.bmHeight;
	bitmapTrans.CreateBitmap(nWidth, nHeight, 1, 1, NULL);

	// Select the mask bitmap into the appropriate dc
	CBitmap* pOldBitmapTrans = dcTrans.SelectObject(&bitmapTrans);

	// Build mask based on transparent colour
	dcImage.SetBkColor(crColour);
	dcTrans.BitBlt(0, 0, nWidth, nHeight, &dcImage, 0, 0, SRCCOPY);

	// Do the work - True Mask method - cool if not actual display
	pDC->BitBlt(x, y, nWidth, nHeight, &dcImage, 0, 0, SRCINVERT);
	pDC->BitBlt(x, y, nWidth, nHeight, &dcTrans, 0, 0, SRCAND);
	pDC->BitBlt(x, y, nWidth, nHeight, &dcImage, 0, 0, SRCINVERT);

	// Restore settings
	dcImage.SelectObject(pOldBitmapImage);
	dcTrans.SelectObject(pOldBitmapTrans);
	pDC->SetBkColor(crOldBack);
	pDC->SetTextColor(crOldText);
}

void CTalkMasterConsoleDlg::readTransparent(CBitmap *bitmap, CDC * pDC)
{
#if 0
	BITMAP	bm;
	COLORREF cr;
	BYTE *bitmapMem;
	CDC dcFrom;

	dcFrom.CreateCompatibleDC(pDC);

	// Select the image into the appropriate dc
	CBitmap* pOldBitmapImage = dcFrom.SelectObject(bitmap);

	cr = dcFrom.GetPixel(0,0);

	cr = pDC->GetPixel(0,0);
//	bitmap->SetBitmapBits();

	bitmap->GetBitmap(&bm);
	int nWidth = bm.bmWidth;
	int nHeight = bm.bmHeight;

	bitmapMem = (BYTE *)calloc(1, nWidth * nHeight);

#else
	CDC dcTo, dcFrom;
	BITMAP	bm;

	// Create the mask bitmap
//	CBitmap bitmapTrans;

	// Create two memory dcs for the image and the mask
	dcTo.CreateCompatibleDC(pDC);
//	dcFrom.CreateCompatibleDC(pDC);

	// Select the image into the appropriate dc
	CBitmap* pOldBitmapTo = dcTo.SelectObject(bitmap);
//	CBitmap* pOldBitmapFrom = dcFrom.SelectObject(bitmap);

	bitmap->GetBitmap(&bm);
	int nWidth = bm.bmWidth;
	int nHeight = bm.bmHeight;

//	bitmapTrans.CreateBitmap(nWidth, nHeight, 1, 1, NULL);

	// Build mask based on transparent colour
	dcTo.BitBlt(0, 0, nWidth, nHeight, pDC, 60, 60, SRCCOPY);

	// Restore settings
	dcTo.SelectObject(pOldBitmapTo);
//	dcFrom.SelectObject(pOldBitmapFrom);
#endif
}

void CTalkMasterConsoleDlg::drawBitmaps()
{
	CDC *pDC = m_staticLogo.GetDC();
	DrawTransparent(&mBitMapLogo, pDC, 0, 0, m_crWhite);

	drawVUMeter( &m_staticVUMeterSpeaker, bListening, nListeningLevel );
	drawVUMeter( &m_staticVUMeterMicrophone, bTalking, nTalkingLevel );

	m_staticLogo.ReleaseDC(pDC);
}

void CTalkMasterConsoleDlg::drawVUMeter( CStatic *meter, BOOL bActive, int level  )
{
	int	i, maxLevel=1;
	CDC dcMemory, *pDC;
	BITMAP	bm;

	mBitMapRed.GetBitmap(&bm);

	pDC = meter->GetDC();

	// Create two memory dcs for the image and the mask
	dcMemory.CreateCompatibleDC(pDC);

    // Select the bitmap into the in-memory DC
	CBitmap* pOldBitmap = dcMemory.SelectObject(&mBitMapRed);
	pDC->BitBlt(0, 0, bm.bmWidth, bm.bmHeight, &dcMemory, 0, 0, SRCCOPY);
	dcMemory.SelectObject(pOldBitmap);

	if( level > 0 )
	{
		pOldBitmap = dcMemory.SelectObject(&mBitMapGreen);

		maxLevel = min((level+9) / 10, 6);

		for(i=1;i<maxLevel;i++)
			pDC->BitBlt((i*bm.bmWidth), 0, bm.bmWidth, bm.bmHeight, &dcMemory, 0, 0, SRCCOPY);

		dcMemory.SelectObject(pOldBitmap);
	}

	if( level > 64 )
	{
		pOldBitmap = dcMemory.SelectObject(&mBitMapYellow);

		maxLevel = min(level / 10, 11);

		for(i=6;i<maxLevel;i++)
			pDC->BitBlt((i*bm.bmWidth), 0, bm.bmWidth, bm.bmHeight, &dcMemory, 0, 0, SRCCOPY);

		dcMemory.SelectObject(pOldBitmap);
	}

	if( level > 112 )
	{
		pOldBitmap = dcMemory.SelectObject(&mBitMapRed);
		pDC->BitBlt((11*bm.bmWidth), 0, bm.bmWidth, bm.bmHeight, &dcMemory, 0, 0, SRCCOPY);
		dcMemory.SelectObject(pOldBitmap);

		maxLevel = 12;
	}

	if( maxLevel < 12 )
	{
		pOldBitmap = dcMemory.SelectObject(&mBitMapBlank);

		for(i=maxLevel;i<12;i++)
			pDC->BitBlt((i*bm.bmWidth), 0, bm.bmWidth, bm.bmHeight, &dcMemory, 0, 0, SRCCOPY);

		dcMemory.SelectObject(pOldBitmap);
	}

	meter->ReleaseDC(pDC);
}

#if 0
      // Get the size of the bitmap
      BITMAP bmpInfo;
      bmp.GetBitmap(&bmpInfo);

      // Create an in-memory DC compatible with the
      // display DC we're using to paint
      CDC dcMemory;
      dcMemory.CreateCompatibleDC(pDC);

      // Select the bitmap into the in-memory DC
      CBitmap* pOldBitmap = dcMemory.SelectObject(&bmp);

      // Find a centerpoint for the bitmap in the client area
      CRect rect;
      GetClientRect(&rect);
      int nX = rect.left + (rect.Width() - bmpInfo.bmWidth) / 2;
      int nY = rect.top + (rect.Height() - bmpInfo.bmHeight) / 2;

      // Copy the bits from the in-memory DC into the on-
      // screen DC to actually do the painting. Use the centerpoint
      // we computed for the target offset.
      pDC->BitBlt(nX, nY, bmpInfo.bmWidth, bmpInfo.bmHeight, &dcMemory, 
         0, 0, SRCCOPY);

      dcMemory.SelectObject(pOldBitmap);
#endif

void CTalkMasterConsoleDlg::initSystemTray()
{
	m_nid.cbSize = sizeof(m_nid);
	m_nid.hWnd = this->m_hWnd;
	m_nid.uID = WM_USER+0x1000;
	m_nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
	m_nid.uCallbackMessage = WM_USER+0x1000;
#ifdef	PRIMEX_BUILD
	m_nid.hIcon = theApp.LoadIcon(IDR_MAINFRAME);
#else
	m_nid.hIcon = theApp.LoadIcon(IDI_ICON_TASK_BAR);
#endif
//	m_nid.uVersion = 5;
#ifdef PRIMEX_BUILD
	strcpy( m_nid.szTip ,getResourceString(IDS_STRING184));
#else
	strcpy( m_nid.szTip ,getResourceString(IDS_STRING185));
#endif

//	Shell_NotifyIcon( NIM_SETVERSION, &m_nid );
}

void CTalkMasterConsoleDlg::delSystemTray()
{
	Shell_NotifyIcon( NIM_DELETE, &m_nid );
}

void CTalkMasterConsoleDlg::showSystemTray()
{
	initSystemTray();
	Shell_NotifyIcon( NIM_ADD, &m_nid );
}

void CTalkMasterConsoleDlg::forceForegroundWindow()
{
	DWORD	threadID1, threadID2;

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
			SetForegroundWindow();
	}
}
void CTalkMasterConsoleDlg::showMinimized(BOOL min)
{
	if( !theApp.UserOptions.hideWindow )
		return;

	if( min )
	{
		ShowWindow(SW_HIDE);

		m_min = TRUE;
	}
	else if (m_min)
	{
		m_min = FALSE;
#if 0
		if( this->IsIconic() )
			ShowWindow(SW_RESTORE);
		else
			ShowWindow(SW_SHOW);

		DoEvents();
		Sleep(10);

		forceForegroundWindow();
#endif
	}
}


BOOL CTalkMasterConsoleDlg::replaceMenuItem( CMenu *menu, int row, char *buffer, int ID )
{
	MENUITEMINFO MenuItem = { 
		sizeof(MENUITEMINFO),
		MIIM_ID | MIIM_TYPE,
		MFT_STRING,
		NULL,
		ID_VIEW_PREFERENCES,
		NULL,
		NULL,
		NULL,
		NULL,
		getResourceString(IDS_STRING_PREFERENCES).GetBuffer(),
		getResourceString(IDS_STRING_PREFERENCES).GetLength()+1
//		NULL
	};

	MenuItem.dwTypeData = (LPSTR)buffer;
	MenuItem.cch = strlen(buffer)+1;
	MenuItem.wID = ID;

	menu->RemoveMenu(row, MF_BYPOSITION);
	menu->InsertMenuItem(row, &MenuItem, TRUE);

	return(TRUE);
}

BOOL CTalkMasterConsoleDlg::insertMenuItem( CMenu *menu, int row, char *buffer, int ID )
{
	MENUITEMINFO MenuItem = { 
		sizeof(MENUITEMINFO),
		MIIM_ID | MIIM_TYPE,
		MFT_STRING,
		NULL,
		ID_VIEW_PREFERENCES,
		NULL,
		NULL,
		NULL,
		NULL,
		getResourceString(IDS_STRING_PREFERENCES).GetBuffer(),
		getResourceString(IDS_STRING_PREFERENCES).GetLength()+1
//		NULL
	};

	MenuItem.dwTypeData = (LPSTR)buffer;
	MenuItem.cch = strlen(buffer)+1;
	MenuItem.wID = ID;

	menu->InsertMenuItem(row, &MenuItem, TRUE);

	return(TRUE);
}

BOOL CTalkMasterConsoleDlg::removeMenuItem( CMenu *menu, int row )
{
	return( menu->RemoveMenu(row, MF_BYPOSITION) );
}

CString getFileVersionString()
{
	CString csRet;

	char *name_ptr;
	char szAppName[MAX_PATH];

//	GetModuleFileName( AfxGetInstanceHandle(), szAppName, 128 ); 
	GetModuleFileName( NULL, szAppName, MAX_PATH-1 ); 

 	DWORD dummy;
	DWORD ver_size = ::GetFileVersionInfoSize( szAppName, &dummy ); 
	BOOL good_value;

	unsigned int value_length; 
	char *ver_buffer = (char *)calloc(1, ver_size+10); 

	GetFileVersionInfo( szAppName, 0, ver_size, (void *)ver_buffer ); 

	good_value = VerQueryValue( ver_buffer, 
								TEXT("\\StringFileInfo\\040904E4\\FileVersion"), 
					            (void**)&name_ptr, 
								&value_length ); 

	csRet = name_ptr;

	free(ver_buffer);

	return(csRet);
}

CString getFileVersionString(char *appName)
{
	CString csRet;
	char *name_ptr;

	DWORD dummy;
	DWORD ver_size = ::GetFileVersionInfoSize( appName, &dummy ); 
	BOOL good_value;

	unsigned int value_length; 
	char *ver_buffer = (char *)calloc(1, ver_size+10); 

	GetFileVersionInfo( appName, 0, ver_size, (void *)ver_buffer ); 

	good_value = VerQueryValue( ver_buffer, 
								TEXT("\\StringFileInfo\\040904E4\\FileVersion"), 
					            (void**)&name_ptr, 
								&value_length ); 

	if( good_value )
		csRet = name_ptr;
	else
		csRet = "";

	free(ver_buffer);

	return(csRet);
}

void CTalkMasterConsoleDlg::showHdxFdxControls()
{
	int showFdx = SW_HIDE, showHdxTalk = SW_SHOW, showHdxListen = SW_SHOW, showNoListen = SW_HIDE, showSession = SW_HIDE;
	int	enableHdx = TRUE;

	if( m_pSelectedItem &&
		!theApp.UserOptions.forceHdx &&
	   (m_pSelectedItem->iCom.picRev.feature1 & OPNIC_FULL_DUPLEX) && 
	   m_TalkMode == TALK_SELECTED )
	{
		showFdx = SW_SHOW;
		showHdxTalk = SW_HIDE;
		showHdxListen = SW_HIDE;
		enableHdx = FALSE;
	}

//
// If this record is a handset record, we need to show the CALL and the LISTEN buttons
//
	if( m_pSelectedItem && m_TalkMode == TALK_SELECTED &&
		!getCQItemDataFromSocket(m_pSelectedItem->socket) &&				// Not a call queue entry
		m_pSelectedItem->iCom.picRev.feature2 & OPNIC_2_HANDSET )
	{
		showFdx = SW_HIDE;
		showHdxTalk = SW_HIDE;
		showHdxListen = SW_HIDE;
		showSession = SW_SHOW;
	}

	if( m_pSelectedItem &&
		m_pSelectedItem->iCom.picRev.opMode1 & OPMODE_REMLISTN_DISA )
	{
		showNoListen = SW_SHOW;
		showHdxListen = SW_HIDE;
	}

	m_btnAutomatic.EnableWindow(enableHdx);						// Make it so the user can select automatic again
	m_btnManual.EnableWindow(enableHdx);

//	outputDebug(showHdxTalk?"Show Talk 3":"Hide Talk 3");
	m_btnTalk.ShowWindow( showHdxTalk );

	m_btnNoListen.ShowWindow( showNoListen );
	m_btnListen.ShowWindow( showHdxListen );

	m_btnFdxMute.ShowWindow( showFdx );
	m_btnFdxStart.ShowWindow( showFdx );

	m_btnTalkSessionStart.ShowWindow( showSession );
	if( showSession == SW_SHOW )									// If we are showing the CALL button, also show the LISTEN button
		m_btnListen.ShowWindow( SW_SHOW );

	m_btnTalkSessionEnd.ShowWindow( SW_HIDE );
}

CString CTalkMasterConsoleDlg::getResourceString(int ID)
{
	static CString csRet;

	csRet.LoadString(ID);

	return(csRet);
}

void CTalkMasterConsoleDlg::doPostInit()
{
	struct _groupDataList *pGroupList;
	struct _groupItems *pGroupIcoms;
	int i, count, itemCount;

	KillTimer(TIMER_INIT_DONE);

	count = m_listGroups.GetItemCount();

	for(i=0;i<count;i++)
	{
		itemCount = 0;

		pGroupList = (_groupDataList*)m_listGroups.GetItemData(i);

		if( pGroupList && pGroupList->pGroupData->groupCount )
		{
			m_DACreateGroup(m_hDA, &pGroupList->pGroupData->groupSocket, NULL, 0, TRUE);				// Make a temp group so we can send stuff to the server based group

			if( getListIcomsCount( pGroupList ) != pGroupList->pGroupData->groupCount )
				getListIcoms( pGroupList );

			refreshListIcoms( pGroupList );			// Make sure the icom data is up to date

			m_DADeleteGroup(m_hDA, pGroupList->pGroupData->groupSocket);

			pGroupIcoms = pGroupList->pGroupItems;

			while( pGroupIcoms )
			{
				if( pGroupIcoms->pIcomData->iCom.status )
					itemCount++;

				pGroupIcoms = pGroupIcoms->next;
			}
		}

		if( !pGroupList || itemCount == 0 )
			m_listGroups.SetItem(i, GROUP_COL_ICON, LVIF_IMAGE, 0, GROUP_IMAGE_EMPTY, 0, 0, NULL, 0);
		else if( pGroupList->pGroupData->groupCount == itemCount )
			m_listGroups.SetItem(i, GROUP_COL_ICON, LVIF_IMAGE, 0, GROUP_IMAGE_FULL, 0, 0, NULL, 0);
		else
			m_listGroups.SetItem(i, GROUP_COL_ICON, LVIF_IMAGE, 0, GROUP_IMAGE_PARTIAL, 0, 0, NULL, 0);
	}

	m_bConsoleReady = TRUE;
}

void CTalkMasterConsoleDlg::updateGroupIcons()
{
	struct _groupDataList *pGroupList;
	struct _groupItems *pGroupIcoms;
	int i, count, itemCount;

	count = m_listGroups.GetItemCount();

	for(i=0;i<count;i++)
	{
		itemCount = 0;

		pGroupList = (_groupDataList*)m_listGroups.GetItemData(i);

		if( pGroupList && pGroupList->pGroupData->groupCount )
		{
			pGroupIcoms = pGroupList->pGroupItems;

			while( pGroupIcoms )
			{
				if( pGroupIcoms->pIcomData->iCom.status )
					itemCount++;

				pGroupIcoms = pGroupIcoms->next;
			}
		}

		if( !pGroupList || itemCount == 0 )
			m_listGroups.SetItem(i, GROUP_COL_ICON, LVIF_IMAGE, 0, GROUP_IMAGE_EMPTY, 0, 0, NULL, 0);
		else if( pGroupList->pGroupData->groupCount == itemCount )
			m_listGroups.SetItem(i, GROUP_COL_ICON, LVIF_IMAGE, 0, GROUP_IMAGE_FULL, 0, 0, NULL, 0);
		else
			m_listGroups.SetItem(i, GROUP_COL_ICON, LVIF_IMAGE, 0, GROUP_IMAGE_PARTIAL, 0, 0, NULL, 0);
	}
}

void CTalkMasterConsoleDlg::doGetSettingsUpdate()
{
	if( !bTalking && !bListening && !m_sessionSocket && !m_playFileSocket && !sendTextDlg.m_bDialogInUse && !m_bHeldSessionSocket )
	{
		KillTimer(TIMER_UPDATE_READY);

		saveControls( &m_bTalkFlag, &m_bListenFlag, &m_bChimeFlag, &m_bPlayFileFlag, &m_bRadioFlag, &m_bTestVolumeFlag, &m_bVolumeControlsFlag, &m_bGroupControlsFlag );
		lockControls( TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE );

		m_dlgPleaseWait.Create(IDD_DIALOG_PLEASE_WAIT);
		m_dlgPleaseWait.ShowWindow(SW_SHOWNORMAL);

		m_bConsoleReady = FALSE;					// We are going to re-request all of the settings and then wait
		
		releaseCurrentSettings();

		m_DAResendSettings( m_hDA );

		SetTimer(TIMER_UPDATE_RELEASE, 100, NULL);
	}
}

void CTalkMasterConsoleDlg::doReleaseSettingsUpdate()
{
	if( !bLoggedOn || bShuttingDown || m_bConsoleReady )
	{
		KillTimer(TIMER_UPDATE_RELEASE);

		m_dlgPleaseWait.DestroyWindow();

		lockControls( !m_bTalkFlag, !m_bListenFlag, !m_bChimeFlag, !m_bPlayFileFlag, !m_bRadioFlag, !m_bTestVolumeFlag, !m_bVolumeControlsFlag, !m_bGroupControlsFlag );

		return;
	}
}

void CTalkMasterConsoleDlg::releaseCurrentSettings()
{
	struct _iComQueueList *list = m_pIcomQueueList;
	struct _itemData *item = m_pItemData;

	outputDebug(_T("releaseCurrentSettings: Deleting Calls Waiting Information"));

	if( m_pCallWaitingItem )							// Is one already selected?
	{
		if( m_pCallWaitingItem->hCallQueue )			// If we have an archive open
			audioArchive.Close(m_pCallWaitingItem->hCallQueue);

//		m_pCallWaitingItem->hCallQueue = NULL;			// Should we be clearing this out???

		m_pCallWaitingItem = NULL;						// No more item
	}

	clearCallsWaitingItemData();
	m_listCallsWaiting.DeleteAllItems();

	outputDebug(_T("releaseCurrentSettings: Deleting Queue Information"));

	while(list)										// Get rid of the queue information
	{
		m_pIcomQueueList = list->next;
		free(list);
		list = m_pIcomQueueList;
	}

	outputDebug(_T("releaseCurrentSettings: Deleting Intercom Information"));

	Sleep(200);										// Give everything a chance to settle down

	deleteIcoms(TRUE);								// Clear the list

	while(item)
	{
		m_pItemData = item->next;
		free(item);
		item = m_pItemData;
	}

	outputDebug(_T("releaseCurrentSettings: Deleting Server Group Information"));

	deleteGroups();
	deleteMessages();
}

void CTalkMasterConsoleDlg::addWork(int socket, int command, time_t when)
{
	struct _workData *pWorkData, *prev=NULL, *pNewWorkData;

	pNewWorkData = (_workData*)calloc(1, sizeof(_workData));

	pNewWorkData->command	= command;
	pNewWorkData->socket	= socket;
	pNewWorkData->when		= when;

	EnterCriticalSection(&CriticalSectionWork);

	pWorkData = m_pWorkData;

	while( pWorkData )
	{
		prev = pWorkData;

		pWorkData = pWorkData->next;
	}

	if( prev == NULL )			// Beginning
		m_pWorkData = pNewWorkData;
	else
		prev->next = pNewWorkData;

	LeaveCriticalSection(&CriticalSectionWork);

	SetTimer( TIMER_WORK, 100, NULL );
}

void CTalkMasterConsoleDlg::doWork()
{
	struct _workData *pWorkData = m_pWorkData, *prev=NULL, *pNextWorkData;
	time_t now;

	static int guard = 0;

	if( ++guard > 1 )
	{
		--guard;
		return;
	}

	while( pWorkData )
	{
		if( pWorkData->when <= time(&now) )					// Time to fire
		{
			pNextWorkData = pWorkData->next;				// Save the next pointer so we can go to it after releasing this memory

			EnterCriticalSection(&CriticalSectionWork);

			if( prev == NULL )
				m_pWorkData = pWorkData->next;
			else
				prev->next = pWorkData->next;

			LeaveCriticalSection(&CriticalSectionWork);

			doWorkCommand(pWorkData->socket, pWorkData->command);	// Do the work before freeing it

			free(pWorkData);

			pWorkData = pNextWorkData;						// Point at the next record.
		}
		else
		{
			break;
		}
	}

	if( !m_pWorkData )								// No work to do
		KillTimer(TIMER_WORK);

	--guard;
}

void CTalkMasterConsoleDlg::doWorkCommand(int socket, int command)
{
	switch(command)
	{
	case WORK_DESELECT:
		deselectSocket( socket );
		break;

	case WORK_DELETE_GROUP:
		m_btnPlayFile.SetWindowText(_T(getResourceString(IDS_STRING_PLAY_FILE)));

		lockControls(FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE);

		m_DADeleteGroup(m_hDA, socket);	

		m_playFileSocket = 0;
		bPlayFileLocal = TRUE;
		m_tabMain.EnableWindow(TRUE);

		break;
	}
}