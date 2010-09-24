//
// DAControl.cpp
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

//
// The intercom session control stuff - behind the dialog work
//
#include "stdafx.h"
#include "TalkMasterConsole.h"
#include "TalkMasterConsoleDlg.h"
#include "DAPassThruDll.h"
#include "DACallQueue.h"
#include "DACodec.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

void CallBack( int event, int socket, struct _iComStructure *pIcom, void *eventData, void *myData )
{
	CTalkMasterConsoleDlg *dlg = (CTalkMasterConsoleDlg*)theApp.m_pMainWnd;
	struct _AudioData *ad;
	struct _AudioRequest *ar;
	struct _KeyPress *kp;
	struct _CQData *cq;
	int	index, count;
	struct _itemData *tItemData;
	struct _iComQueueInfo *pIcomQueue;
	struct _consoleData *cd;
	time_t	*serverTime;
	struct _groupData *gd;
	struct _MAC *pMac;
	struct _messageData *md;
	struct _messagePlayingData *mp;

	struct _iComQueueList *listEntry=NULL;

	BOOL	bDisplayIcom = FALSE;

	char	buffer[128];

//	struct iComStructure *tIcom;

	if( dlg == NULL )
		return;

//	dlg->outputDebug("CallBack: Event %d", event);

	switch(event)
	{
	case EVENT_PRINT_STRING:
		dlg->outputDebug((char *)eventData);
		break;

	case EVENT_NEW_CONNECTION:
		pIcomQueue = (struct _iComQueueInfo *)eventData;

		dlg->outputDebug(_T("EVENT:New Connection '%s' on socket %d : Status = %d : Priority = %d"), pIcom->name, socket, pIcom->status, pIcomQueue->priority);

		dlg->doNewIntercom( socket, pIcom, pIcomQueue );

		break;

	case EVENT_RECV_DATA:
		ad = (struct _AudioData *)eventData;
//		dlg->outputDebug("Intercom %d Received %d bytes of data", socket, ad->len);
//
// Eventually call something that will deal with the incoming audio
//
		dlg->doAudioData( socket, ad );
		break;

	case EVENT_ERROR:
		dlg->outputDebug(_T("Intercom %d Event Error"), socket);
		break;

	case EVENT_AUDIO_LOW_MARK:
		ar = (struct _AudioRequest *)eventData;
//		dlg->outputDebug("Intercom %d Event Audio Low Mark", socket);
		dlg->doAudioRequest( socket, ar->len );
		break;

	case EVENT_AUDIO_HIGH_MARK:
		dlg->outputDebug(_T("Intercom %d Event Audio High Mark"), socket);
		break;

	case EVENT_AUDIO_DONE:
//		dlg->outputDebug("EVENT:Audio Done. Status = %d. Socket = %d", pIcom->status, socket);
		dlg->doneAudioFile( socket );
#if 0
		tItemData = (struct _itemData *)dlg->findItemData(socket);

		if( tItemData )
			memcpy(&tItemData->iCom, pIcom, sizeof(struct _iComStructure));

		count = dlg->m_listIcoms.GetItemCount();
		index = findIntercom(pIcom);

		if( index < count )
			dlg->updateListIcomsStatus(pIcom, index);
#endif
		break;

	case EVENT_AUDIO_UNDERFLOW:
//		dlg->outputDebug("Intercom %d Event: Audio Underflow", socket);
		break;

	case EVENT_AUDIO_WATERMARK:
//		dlg->outputDebug("Intercom %d Event: Audio Watermark", socket);
		break;

	case EVENT_INTERCOM_DONE_TALKING:
		dlg->outputDebug(_T("EVENT:Done Talking. Status = %s(%d).  Socket = %d"), dlg->szStatus(pIcom->status, buffer), pIcom->status, socket);
//		dlg->outputDebug("Intercom %d Audio Send Complete", socket);
//
// Deal with the end of audio event for this intercom
//

//		if( dlg->bTalking )
//			break;

		tItemData = (struct _itemData *)dlg->findItemData(socket);

		if( tItemData )
			memcpy(&tItemData->iCom, pIcom, sizeof(struct _iComStructure));

		count = dlg->m_listIcoms.GetItemCount();
		index = findIntercom(pIcom);

		if( index < count )
			dlg->updateListIcomsStatus(pIcom, index);

		dlg->doneAudioData( socket );

		break;

	case EVENT_RECONNECT:
		tItemData = (struct _itemData *)dlg->findItemData(&pIcom->MAC);

		dlg->outputDebug(_T("EVENT:Reconnect '%s' on socket %d Status = %d"), pIcom->name, socket, pIcom->status);

		if( tItemData && tItemData->bDisplayed )
			--dlg->m_nIntercomsConsole;
		else
			dlg->outputDebug(_T("Reconnect not displayed"));

		pIcomQueue = (struct _iComQueueInfo *)eventData;
		dlg->doNewIntercom( socket, pIcom, pIcomQueue );

		break;

	case EVENT_INTERCOM_KEYPRESS:
		kp = (struct _KeyPress *)eventData;

		tItemData = (struct _itemData *)dlg->findItemData(socket);
		if( !tItemData )
			break;

		if( !(tItemData->iCom.picRev.feature2 & OPNIC_2_HANDSET))
			dlg->outputDebug(_T("Intercom %d Event Keypress %s:%c"), socket, kp->upDown?_T("Up"):_T("Down"), kp->keycode);

		index = findIntercom(&tItemData->iCom);			// Get the line # to adjust

		if( index == -1 )								// Not displayed
			break;

		if( !(tItemData->iCom.picRev.opMode2 & OPMODE_GENERAL_SENSOR) &&
			!(tItemData->iCom.picRev.feature2 & OPNIC_2_HANDSET) &&					// Only change the door OPEN icon if we are NOT in handset mode
			( kp->keycode == 's' ||
			  kp->keycode == 'S' ) )
		{
			if( kp->upDown )		// Up is TRUE
			{
				if( tItemData->doorOpenTimer )
					dlg->m_listIcoms.SetItem( index, COL_DOOR, LVIF_IMAGE, 0, 1, 0, 0, NULL );
				else if( tItemData->iCom.picRev.opMode1 & OPMODE_RELAY )
					dlg->m_listIcoms.SetItem( index, COL_DOOR, LVIF_IMAGE, 0, 0, 0, 0, NULL );
				else
					dlg->m_listIcoms.SetItem( index, COL_DOOR, LVIF_IMAGE, 0, 4, 0, 0, NULL );		// No image

				tItemData->sensorState = 0;
			}
			else
			{
				if( tItemData->doorOpenTimer || kp->keycode == 's' )				// Unlocked
					dlg->m_listIcoms.SetItem( index, COL_DOOR, LVIF_IMAGE, 0, 3, 0, 0, NULL );
				else																// Locked and open
					dlg->m_listIcoms.SetItem( index, COL_DOOR, LVIF_IMAGE, 0, 2, 0, 0, NULL );

				tItemData->sensorState = kp->keycode;
			}
		}

#if 0
		if( tItemData )
		{
			index = findIntercom(tItemData->iCom);
			dlg->m_listIcoms.SetItemState(index, LVIS_SELECTED, LVIS_SELECTED);
			dlg->m_listIcoms.EnsureVisible(index, FALSE);

			dlg->m_selectedRow = index;
		}

		dlg->m_pSelectedItem = dlg->findItemData(socket);

		dlg->m_btnGetVolume.EnableWindow(dlg->m_pSelectedItem != NULL);
		dlg->m_btnSetVolume.EnableWindow(dlg->m_pSelectedItem != NULL);
#endif

		break;

	case EVENT_GPIO_STATE_CHANGE:
		dlg->outputDebug(_T("Event GPIO State Change"));
		break;

	case EVENT_RELAY_DEACTIVATED:
		break;

	case EVENT_INTERCOM_LISTENING:
		dlg->outputDebug(_T("EVENT:Listening. Socket %d, Status = %s(%d)"), socket, dlg->szStatus(pIcom->status, buffer), pIcom->status);

		tItemData = (struct _itemData *)dlg->findItemData(socket);

		if( tItemData )
			memcpy(&tItemData->iCom, pIcom, sizeof(struct _iComStructure));

		count = dlg->m_listIcoms.GetItemCount();
		index = findIntercom(pIcom);

		if( index < count )
			dlg->updateListIcomsStatus(pIcom, index);

//		dlg->outputDebug("Intercom %d Listening", socket);
		break;

	case EVENT_INTERCOM_TALKING:
		dlg->outputDebug(_T("EVENT:Talking. Socket %d, Status = %s(%d)"), socket, dlg->szStatus(pIcom->status, buffer), pIcom->status);

		tItemData = (struct _itemData *)dlg->findItemData(socket);

		if( tItemData )
			memcpy(&tItemData->iCom, pIcom, sizeof(struct _iComStructure));

		count = dlg->m_listIcoms.GetItemCount();
		index = findIntercom(pIcom);

		if( index < count )
			dlg->updateListIcomsStatus(pIcom, index);

//		dlg->outputDebug("Intercom %d Talking", socket);
		break;

	case EVENT_DISCONNECT:							// Must add code to shut off Talk/Listen if disconnect
		dlg->outputDebug(_T("EVENT:Disconnected. intercom %d"), socket);

		if( dlg->m_pSelectedItem && dlg->m_pSelectedItem->socket == socket )
		{
			dlg->m_pSelectedItem = NULL;
			dlg->m_selectedRow = -1;
			dlg->m_listIcoms.SetItemState(-1, 0, LVIS_SELECTED|LVIS_FOCUSED);

			dlg->clearFdx();						// The row was the selected row, make sure we are back at nothing
		}

		tItemData = (struct _itemData *)dlg->findItemData(socket);
		if( tItemData )
			memcpy(&tItemData->iCom, pIcom, sizeof(struct _iComStructure));

		count = dlg->m_listIcoms.GetItemCount();
		index = findIntercom(pIcom);

		if( index < count )
		{
			if( socket == dlg->sendTextDlg.m_socket )
				dlg->sendTextDlg.m_bWaitingForCompletion = FALSE;;

			if( socket == dlg->m_sessionSocket )
			{
				dlg->clearTalk();
				dlg->clearListen();					// Just clear the UI stuff since this is already disconnected
				dlg->clearFdx();

//				dlg->showHdxFdxControls();

				dlg->lockControls(FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE);	// Talk, Listen, Chime, PlayFile, Radio Buttons
				dlg->bTalking = FALSE;
				dlg->bTalkRequest = FALSE;
				dlg->bListening = FALSE;
				dlg->bListenRequest = FALSE;
				dlg->bMuting = FALSE;

				dlg->bNoRestart	= FALSE;				// Allow restarts

				dlg->doBnClickedButtonEnd(TRUE);		// End the session too
//				dlg->m_sessionSocket = 0;
			}
			else											// If this intercom disconnected, try to delete it's CQ reference
				dlg->deleteCallQueue(socket, TRUE, FALSE);			// Delete and force local delete (From 1.4.1, 2DEC08)

//
// See if we were playing a file to this intercom
//
		if( socket && dlg->m_playFileSocket == socket )				// If we are we are playing something
		{
			dlg->stopAudioFile(dlg->m_playFileSocket);		// Stop playing the file to this intercom
			dlg->m_btnPlayFile.SetWindowText(_T(dlg->getResourceString(IDS_STRING_PLAY_FILE)));	// Reset the button title
			dlg->lockControls(FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE);	// Reset the button enable settings
			dlg->m_playFileSocket = 0;						// No longer in play file mode
			dlg->bPlayFileLocal = TRUE;
			dlg->m_tabMain.EnableWindow(TRUE);
		}

//			tIcom = (struct iComStructure *)dlg->m_listIcoms.GetItemData(index);
//			free(tIcom);

//			dlg->m_listIcoms.DeleteItem(index);
			dlg->m_listIcoms.SetItemText(index,COL_STATUS,_T("Disconnected"));

			if( tItemData && tItemData->bDisplayed && tItemData->bCounted )
			{
				dlg->m_nIntercomsConsoleAlive--;
				tItemData->bCounted = FALSE;
			}
		}
		else
		{
			dlg->outputDebug("CallBack: EVENT_DISCONNECT, index=%d < count=%d", index, count);
		}

//		dlg->outputDebug("Intercom %d disconnected", socket);

		dlg->updateGroupIcons();

		dlg->m_nIntercomsServerAlive--;

		dlg->displayIntercomCount();

		break;

	case EVENT_STOP:
		if( !dlg->bShuttingDown )
			dlg->outputDebug(_T("Event:Stop. Console disconnected from server"));
		else
			OutputDebugString(_T("Event:Stop. Console Shutdown disconnected from server\n"));

		dlg->m_nIntercomsConsole = 0;
		dlg->m_nIntercomsConsoleAlive = 0;
		dlg->m_nIntercomsServer = 0;
		dlg->m_nIntercomsServerAlive = 0;

		dlg->KillTimer(TIMER_DOOR);

		dlg->bLoggedOn = FALSE;

		if( dlg->bTalking )
			dlg->audioPlayer.StopMic();

		dlg->sendTextDlg.m_bWaitingForCompletion = FALSE;

		dlg->clearTalk();
		dlg->clearListen();
		dlg->clearFdx();

		dlg->bTalking = FALSE;
		dlg->bTalkRequest = FALSE;
		dlg->bListening = FALSE;
		dlg->bListenRequest = FALSE;
		dlg->bMuting = FALSE;

		dlg->bNoRestart = FALSE;							// Allow restarts

		if( dlg->m_playFileSocket )							// If we are shut down and we are playing something
		{
			dlg->stopAudioFile(dlg->m_playFileSocket);
			dlg->m_btnPlayFile.SetWindowText(_T(dlg->getResourceString(IDS_STRING_PLAY_FILE)));	// Reset the button title
			dlg->lockControls(FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE);	// Reset the button enable settings
			dlg->m_playFileSocket = 0;						// No longer in play file mode
			dlg->bPlayFileLocal = TRUE;
			dlg->m_tabMain.EnableWindow(TRUE);
		}

		if( !dlg->bShuttingDown )
		{
			dlg->deleteAllCallQueue(TRUE);				// Kill anything in the call queue including local items
			dlg->deleteIcomsData();
			dlg->deleteGroups();
			dlg->deleteMessages();

			dlg->displayIntercomCount();
		}

		dlg->SetHoldEnd(FALSE, FALSE);

//		if( !dlg->bShuttingDown )
//			dlg->RedrawWindow();

		break;

	case EVENT_INTERCOM_STATUS:

		dlg->outputDebug(_T("EVENT:STATUS = %s(%d).  Socket = %d"), dlg->szStatus(pIcom->status, buffer), pIcom->status, socket);

		tItemData = (struct _itemData *)dlg->findItemData(socket);

		if( !tItemData )
			break;

		memcpy(&tItemData->iCom, pIcom, sizeof(struct _iComStructure));

		if( pIcom->status && tItemData->bCounted == FALSE && tItemData->bDisplayed )
		{
			dlg->m_nIntercomsConsoleAlive++;
			tItemData->bCounted = TRUE;
			dlg->displayIntercomCount();
		}
		else if( pIcom->status==STATUS_DISCONNECTED && tItemData->bCounted && tItemData->bDisplayed )
		{
			dlg->m_nIntercomsConsoleAlive--;
			tItemData->bCounted = FALSE;
			dlg->displayIntercomCount();
		}

		count = dlg->m_listIcoms.GetItemCount();
		index = findIntercom(pIcom);

		if( index < count )
		{
			if( !tItemData )
				dlg->outputDebug(_T("EVENT:Status item for socket %d, name='%s' is NULL"), socket, pIcom->name);

			dlg->updateListIcomsStatus(pIcom, index);
		}

		break;

	case EVENT_CALL_QUEUE_NEW:
		cq = (struct _CQData *)eventData;
		dlg->addCallQueue(socket, cq);
		break;

	case EVENT_CALL_QUEUE_UPDATE:
		cq = (struct _CQData *)eventData;
		dlg->updateCallQueue(socket, cq);
		break;

	case EVENT_CALL_QUEUE_DELETE:
		dlg->deleteCallQueue(socket, FALSE, TRUE);		// Delete and force local delete?
		break;

	case EVENT_CALL_QUEUE_AUDIO:
		ad = (struct _AudioData *)eventData;

//		dlg->outputDebug("Calling doCQ with %d bytes", ad->len);
		dlg->doCallQueueAudio(socket, ad);
		break;

	case EVENT_CALL_QUEUE_AUDIO_DONE:
		dlg->doCallQueueAudioDone(socket);
		break;

	case EVENT_CALL_QUEUE_MEMBERSHIP:				// What we are supposed to do with each queue
		pIcomQueue = (struct _iComQueueInfo *)eventData;
		dlg->newCallQueueMembership(pIcomQueue);
		break;

	case EVENT_CALL_QUEUE_STATUS:					// What is happening with each queue
		pIcomQueue = (struct _iComQueueInfo *)eventData;
		dlg->updateCallQueueStatus(pIcomQueue);
		break;

	case EVENT_CONSOLE_CONNECT:
		cd = (struct _consoleData*)eventData;
		dlg->outputDebug(_T("Console user %s connected"), cd->fullName);
		break;

	case EVENT_CONSOLE_DISCONNECT:
		cd = (struct _consoleData*)eventData;
		dlg->outputDebug(_T("Console user %s disconnected"), cd->fullName);
		break;

	case EVENT_IMIN:
	case EVENT_SETC:
		break;

	case EVENT_CONSOLE_SERVER_TIME:
		serverTime = (time_t *)eventData;

		dlg->m_serverTimeDelta = *serverTime - time(NULL);

		dlg->outputDebug(_T("Server Delta = %d"), dlg->m_serverTimeDelta);

		break;

	case EVENT_CONSOLE_GROUP_REPORT:					// All of the groups are going to be reported to the console one at a time
		gd = (_groupData*)eventData;
		gd->groupSocket = socket;

		dlg->doNewGroupInfo(gd);
		break;

	case EVENT_CONSOLE_GROUP_MEMBER:
		pMac = (_MAC *)eventData;
		dlg->doNewGroupMember(socket, pMac);
		break;

	case EVENT_CONSOLE_GROUP_MESSAGE:
		md = (_messageData*)eventData;
		dlg->doNewMessageInfo(md);						// We don't need to know which socket (group) this came from, but the socket would indicate that
		break;

	case EVENT_CONSOLE_GROUP_PLAYING:
		mp = (_messagePlayingData*)eventData;
		dlg->doMessagePlaying(mp);
		break;

	case EVENT_CONSOLE_CONSOLE_READY:
		dlg->SetTimer(TIMER_INIT_DONE, 10, NULL);		// DO the post init processing

		dlg->outputDebug(_T("Console initialization is complete") );
		break;

	case EVENT_CONSOLE_UPDATE_AVAILABLE:				// The server has some updated settings available, start the timer that will do the update when idle
		dlg->SetTimer(TIMER_UPDATE_READY, 100, NULL);
		dlg->outputDebug(_T("Console settings update is available"));
		break;

	default:
		dlg->outputDebug(_T("Unserviced Event %d called back"), event);
		break;
	}

//	dlg->outputDebug("CallBack: Done");
}

int findIntercom(struct _iComStructure *pIcom)
{
	int	count, index;
	struct _itemData *tItemData;
	CTalkMasterConsoleDlg *dlg = (CTalkMasterConsoleDlg*)theApp.m_pMainWnd;

	count = dlg->m_listIcoms.GetItemCount();

	for(index=0;index<count;index++)
	{
		tItemData = (struct _itemData *)dlg->getIcomItemData(index);
		if( tItemData && !memcmp( &tItemData->iCom.MAC, &pIcom->MAC, 6 ) )
			break;
	}

	return(index);
}
void CTalkMasterConsoleDlg::doListen()
{
	struct _itemData *tItemData;
	char buffer[128];
	int clearCount = 0;
	int	sessionSocket = m_sessionSocket;

	outputDebug(_T("doListen Session=%d, bTalk=%d, Request=%d, bListen=%d, Request=%d, mode=%d, status=%s"), 
		m_sessionSocket, 
		bTalking, 
		bTalkRequest,
		bListening, 
		bListenRequest,
		m_TalkMode,
		(m_pSelectedItem)?szStatus(m_pSelectedItem->iCom.status, buffer):_T("Nothing selected")
		);

	if( !bLoggedOn )
	{
		bListening = FALSE;
		bListenRequest = FALSE;
		if( bSessionRequest )						// This is how you end a session request
		{
			bSession = TRUE;
			bSessionRequest = FALSE;
		}

		outputDebug(_T("doListen: Can not doTalk, not logged on"));
		return;
	}

	if( bShuttingDown )
		return;

	if( !m_pSelectedItem )		// No intercom selected yet (Used to be m_ListenMode != 0 &&)
	{
		MessageBox(_T(getResourceString(IDS_STRING_SELECT_ICOM)), _T(getResourceString(IDS_STRING_INFORMATION)));
		bListenRequest = FALSE;
		bListening =	 FALSE;

		clearTestMode();

		return;
	}
	else if ( m_TalkMode != TALK_SELECTED )
	{
		MessageBox(_T(getResourceString(IDS_STRING109)), _T(getResourceString(IDS_STRING_ERROR)));
		bListenRequest = FALSE;
		bListening =	 FALSE;

		clearTestMode();

		return;
	}
	else if( m_pSelectedItem->iCom.GPIO & OPMODE_REMLISTN_DISA )			// Page Only
	{
		MessageBox(_T(getResourceString(IDS_STRING111)), _T(getResourceString(IDS_STRING_ERROR)));
		bListenRequest = FALSE;
		bListening =	 FALSE;

		clearTestMode();

		return;
	}
	else if( m_TalkMode == TALK_SELECTED && m_playFileSocket && m_playFileSocket == m_pSelectedItem->socket )
	{
		MessageBox(_T(getResourceString(IDS_STRING112)), _T(getResourceString(IDS_STRING_ERROR)));
		bListenRequest = FALSE;
		bListening =	 FALSE;

		clearTestMode();

		return;
	}

	if( !m_pSelectedItem->iCom.status )
	{
		MessageBox(_T(getResourceString(IDS_STRING113)), _T(getResourceString(IDS_STRING114)));
		bListenRequest = FALSE;
		bListening =	 FALSE;

		clearTestMode();

		return;
	}

	if( m_TalkMode == TALK_SELECTED && 
		m_sessionSocket &&
		m_pSelectedItem &&
		m_pSelectedItem->socket != m_sessionSocket &&
		( (m_pSelectedItem->iCom.status==STATUS_COMMAND) ) )
	{
		bListenRequest = FALSE;
		bTalkRequest = FALSE;
		MessageBox(_T(getResourceString(IDS_STRING115)), _T(getResourceString(IDS_STRING116)));

		clearTestMode();

		return;
	}


// 1st, check to see if we are currently in a session
// This is the same check for doTalk and doHold

	if( !m_sessionSocket )
	{
		m_sessionSocket = m_pSelectedItem->socket;			// Get the socket # of this intercom
		sessionSocket = m_sessionSocket;

		if( !selectSocket(sessionSocket) )
		{
#if 0
			MessageBox(_T(getResourceString(IDS_STRING117)), _T(getResourceString(IDS_STRING_INFORMATION)));
#endif
			outputDebug("doListen: Clearing m_sessionSocket from %d to 0", sessionSocket);
			m_sessionSocket = 0;							// Not in a session

			m_pSelectedItem->bSelected = FALSE;				// Track whether this item is selected
			bListenRequest = FALSE;
			return;
		}

		m_pSelectedItem->bSelected = TRUE;					// Track whether this item is selected

		outputDebug(_T("doListen: startListen(%d) 1"), sessionSocket);

		if( m_pSelectedItem->iCom.status == STATUS_LISTENING )
		{
			bListening = TRUE;
			paintListen();
		}
		else
		{
			startListen(sessionSocket);										// Tell the intercom to start listening & set guard timer
			while( !waitSet(sessionSocket, STATUS_LISTENING) )
				startListen(sessionSocket);
		}

		StartLocalCQSession(sessionSocket, FALSE);		// That is if it is there at all

		if( isCallQueue(sessionSocket) )
			SetHoldEnd(TRUE, TRUE);

		lockControls(FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE);		// LOCK Talk, Listen, Chime, PlayFile, Radio Buttons
	}

	else if( bTalking )			// Already talking, need to end the talk and start the listen
	{
		MarkCallQueueSession(sessionSocket, FALSE);			// Not held

		tItemData = findItemData(sessionSocket);

		if( (tItemData->iCom.status&STATUS_MASK) == STATUS_FDX )
		{
			endTalk(sessionSocket);			// Will stop the microphone on the PC and clear bTalking
#if 0
			waitClear(sessionSocket, STATUS_TALKING);
#else
			clearCount = 0;
			while( !waitClear(sessionSocket, 0) )					// Wait 200 ms for the intercom to clear down
			{
				if( ++clearCount > 5 )
				{
					outputDebug(_T("doListen: Could not clear Talk after %d attempts"), clearCount);
					return;
				}
			}
#endif
		}
		else
		{
			outputDebug("doListen: Intercom already status %s, not Talking.  Skipping endTalk()", szStatus(tItemData->iCom.status, buffer) );
			bTalking = FALSE;
		}

		if( tItemData->iCom.status == STATUS_LISTENING )
		{
			outputDebug(_T("doListen: Already listening on socket %d"), sessionSocket);
		}
		else
		{
			outputDebug(_T("doListen: startListen(%d) 2"), sessionSocket);
			startListen(sessionSocket);		// Tell the other end to start sending data
			while( !waitSet(sessionSocket, STATUS_LISTENING) )
				startListen(sessionSocket);
		}
							// startListen sets a guard flag/timer

		lockControls(FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE);		// LOCK Talk, Listen, Chime, PlayFile, Radio Buttons
	}

	else if( bListening )	// Already listening, need to end the session
	{
		MarkCallQueueSession(sessionSocket, TRUE);			// Hold

		endListen(sessionSocket);

		clearCount = 0;
		while( !waitClear(sessionSocket, STATUS_LISTENING) )		// Wait 200 ms for the intercom to clear down
		{
			if( ++clearCount > 2 )
			{
				bListening = FALSE;
				bNoRestart = FALSE;								// Allow restarts again

				outputDebug(_T("doListen: Could not clear Listen after %d attempts"), clearCount);
				return;
			}
		}

		if( !isCallQueue(sessionSocket) )			// Is this call on the call queue?
		{
//			deleteCallQueue(m_sessionSocket, TRUE);	// Not in the call queue - don't delete it

			deselectSocket( sessionSocket );

			outputDebug("doListen: No longer in session,  sessionSocket from %d to 0", m_sessionSocket);
			m_sessionSocket = 0;	// No longer in a session

			if( m_pSelectedItem )
				m_pSelectedItem->bSelected = FALSE;			// Not selected anymore

			SetHoldEnd(FALSE, FALSE);					// Change the buttons to NO
		}
		else
			SetHoldEnd(FALSE, TRUE);

		bListening = FALSE;
		bNoRestart = FALSE;							// Allow restarts again

		lockControls(FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE);		// unLOCK Talk, Listen, Chime, PlayFile, Radio Buttons
	}

	else
	{
		if( isCallQueue(sessionSocket) )
			SetHoldEnd(TRUE, TRUE);

		MarkCallQueueSession(sessionSocket, FALSE);			// Not hend
		outputDebug(_T("doListen: startListen(%d) 3"), sessionSocket);

		if( m_pSelectedItem->iCom.status == STATUS_LISTENING )
		{
			bListening = TRUE;
			paintListen();
		}
		else
		{
			startListen(sessionSocket);
			while( !waitSet(sessionSocket, STATUS_LISTENING) && m_pSelectedItem && m_pSelectedItem->iCom.status )
				startListen(sessionSocket);
		}

		lockControls(FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE);		// LOCK Talk, Listen, Chime, PlayFile, Radio Buttons
	}

	outputDebug(_T("doListen: Done"));
}

void CTalkMasterConsoleDlg::doTalk()
{
	struct _itemData *tItemData;
	int clearCount, status;
	char buffer[128];
	int sessionSocket = m_sessionSocket;
	int lListenRequest = bListenRequest;

	outputDebug(_T("doTalk Session=%d, bTalk=%d, request=%d, bListen=%d, request=%d, mode=%d, status=%s"), 
		m_sessionSocket, 
		bTalking, 
		bTalkRequest,
		bListening, 
		bListenRequest,
		m_TalkMode,
		(m_pSelectedItem)?szStatus(m_pSelectedItem->iCom.status, buffer):_T("Nothing selected")
		);

	if( !bLoggedOn )
	{
		bTalking = FALSE;
		bTalkRequest = FALSE;

		outputDebug(_T("doTalk: Can not doTalk, not logged on"));
		return;
	}

	if( m_sessionSocket == 0 && bTalking )
		outputDebug("** doTalk is not in a valid state - Session=%d, bTalk=%d **", m_sessionSocket, bTalking );

	if( m_TalkMode == TALK_SELECTED && m_ListenMode == 0 && !m_pSelectedItem )		// No intercom selected yet
	{
		bTalkRequest = FALSE;

		MessageBox(_T(getResourceString(IDS_STRING_SELECT_ICOM)), _T(getResourceString(IDS_STRING_INFORMATION)));
		bTalkRequest = FALSE;
		bTalking =	 FALSE;

		clearTestMode();

		return;
	}

	if( m_TalkMode == TALK_SELECTED && 
		( !m_pSelectedItem || 
		  !m_pSelectedItem->iCom.status ) )
	{
		bTalkRequest = FALSE;

		MessageBox(_T(getResourceString(IDS_STRING113)), _T(getResourceString(IDS_STRING114)));

		clearTestMode();

		return;
	}

	if( m_TalkMode == TALK_SELECTED && 
		m_sessionSocket && 
		m_pSelectedItem &&
		m_pSelectedItem->socket != m_sessionSocket &&
		( (m_pSelectedItem->iCom.status==STATUS_COMMAND) ) )
	{
		bTalkRequest = FALSE;

		MessageBox(_T(getResourceString(IDS_STRING119)), _T(getResourceString(IDS_STRING116)));

		clearTestMode();

		return;
	}

	if( m_TalkMode != TALK_SELECTED && countGroup() == 0 )
	{
		bTalkRequest = FALSE;

		MessageBox(_T(getResourceString(IDS_STRING120)), _T(getResourceString(IDS_STRING121)));

		clearTestMode();

		return;
	}
	else if( m_TalkMode == TALK_SELECTED && m_pSelectedItem && m_playFileSocket == m_pSelectedItem->socket )
	{
		MessageBox(_T(getResourceString(IDS_STRING122)), _T(getResourceString(IDS_STRING_ERROR)));
		bListenRequest = FALSE;

		clearTestMode();

		return;
	}

	if( !m_sessionSocket || (!bTalking && m_TalkMode != TALK_SELECTED) )
	{
		if( m_TalkMode == TALK_SELECTED )
		{
			m_sessionSocket = m_pSelectedItem->socket;			// Start a session with this intercom
			sessionSocket = m_sessionSocket;

			outputDebug(_T("doTalk: New Session Socket=%d"), m_sessionSocket);
		}
		else
		{
			m_oldSessionSocket = m_sessionSocket;				// Save the old session socket (if there is one)
			m_sessionSocket = makeGroupSocket();				// So we can restore it after this page
			sessionSocket = m_sessionSocket;

			outputDebug(_T("doTalk: New Group Session Socket=%d"), m_sessionSocket);
		}

		if( !selectSocket(sessionSocket) )
		{
			if( m_TalkMode != TALK_SELECTED )					// Must delete this group
				m_DADeleteGroup(m_hDA, sessionSocket);

			if( m_pSelectedItem )								// Could have been cleared during the selectSocket
				m_pSelectedItem->bSelected = FALSE;				// Track whether this item is selected

			outputDebug(_T("doTalk: selectSocket(%d) failed, item=0x%x, sessionSocket set to %d"),
				sessionSocket, m_pSelectedItem, m_oldSessionSocket);

			m_sessionSocket = m_oldSessionSocket;				// No longer in a session
			m_oldSessionSocket = 0;

			bTalkRequest = FALSE;

			return;
		}

		if( m_pSelectedItem )
			m_pSelectedItem->bSelected = TRUE;					// Track whether this item is selected


		if( m_TalkMode == TALK_SELECTED &&
			( m_pSelectedItem->iCom.GPIO & OPMODE_REMLISTN_DISA ) )		// Automatically do the listen
		{
			m_btnNoListen.SetBitmap( HBITMAP(mBitMapNoListenOn) );
		}

		sessionSocket = m_sessionSocket;						// In case it changed between the select etc.

		if( lListenRequest && 
			m_pSelectedItem && m_pSelectedItem->iCom.picRev.feature1 & OPNIC_FULL_DUPLEX )								// FDX?  (local ListenRequest)
		{
			startListen(sessionSocket);
			while( !waitSet(sessionSocket, STATUS_LISTENING) )
				startListen(sessionSocket);
		}

		startTalk(sessionSocket);
		while( !waitSet(sessionSocket, (bListenRequest)?STATUS_FDX:STATUS_TALKING) )
			startTalk(sessionSocket);

		StartLocalCQSession(sessionSocket, FALSE);			// That is if it is there at all (not held)

		if( isCallQueue(sessionSocket) )
			SetHoldEnd(TRUE, TRUE);

		lockControls(FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE);		// LOCK Talk, Listen, Chime, PlayFile, Radio Buttons
	}
	else if( bTalking )
	{
		MarkCallQueueSession(sessionSocket, FALSE);			// Not Hold

		tItemData = findItemData(sessionSocket);

		if( tItemData )
			status = tItemData->iCom.status;
		else
			status = STATUS_TALKING;

//		outputDebug("doTalk: Status is %d", status);
		if( (status&STATUS_MASK) == STATUS_TALKING || (status&STATUS_MASK) == STATUS_FDX )
		{
			endTalk(sessionSocket);
		}
		else
		{
			outputDebug("doTalk: Intercom already status %s, not Talking.  Skipping endTalk()", szStatus(status, buffer) );
			bTalking = FALSE;
		}

		if( bListening )									// Full duplex
		{
			waitClear(sessionSocket, status&STATUS_MASK);

			endListen(sessionSocket);
			waitClear(sessionSocket, STATUS_LISTENING);

			bListening = FALSE;
		}

		sessionSocket = m_sessionSocket;

//		outputDebug("doTalk: Done endTalk");
		if( m_ListenMode == LISTEN_AUTO && m_pSelectedItem &&
			!( m_pSelectedItem->iCom.GPIO & OPMODE_REMLISTN_DISA ) &&		// Automatically do the listen
			( !( m_pSelectedItem->iCom.picRev.feature1 & OPNIC_FULL_DUPLEX ) || theApp.UserOptions.forceHdx ) && 
			!(bNoRestart) && m_pSelectedItem->iCom.status )					// And is still connected
		{
#if 0
//			outputDebug("doTalk: Waiting for clear");
			if( (status&STATUS_MASK) == STATUS_TALKING )
				waitClear(sessionSocket);
//			outputDebug("doTalk: Done waiting");
#else
			if( (status&STATUS_MASK) == STATUS_TALKING )
			{
				clearCount = 0;

				while( !waitClear(sessionSocket, STATUS_TALKING) )		// Wait 200 ms for the intercom to clear down
				{
					if( ++clearCount > 2 )
					{
						outputDebug(_T("doTalk: Could not clear Talk after %d attempts"), clearCount);
						return;
					}
				}
			}
#endif

			if( m_sessionSocket == 0 || !m_pSelectedItem || m_pSelectedItem->iCom.status == STATUS_DISCONNECTED )							// Disconnected while waiting for the Listen to stop
			{
				outputDebug("doTalk: Done(3) - Session ended - Talk not started");
				return;
			}

			sessionSocket = m_sessionSocket;

			if( (status&STATUS_MASK) == STATUS_LISTENING )
			{
				outputDebug(_T("doTalk: Already Listening socket = %d"), sessionSocket);
			}
			else
			{
				outputDebug(_T("doTalk: startListen(socket=%d) [bTalking]"), sessionSocket);

				startListen(sessionSocket);

				outputDebug(_T("doTalk: after startListen(%d)"), 
					sessionSocket );

				while( !waitSet(sessionSocket, STATUS_LISTENING) )
					startListen(sessionSocket);

				outputDebug(_T("doTalk: after while waitSet"));
			}

			if( isCallQueue(sessionSocket) )
				SetHoldEnd(TRUE, TRUE);
		}
		else
		{
			if( !isCallQueue(sessionSocket) )			// Is this call on the call queue?
			{
//				deleteCallQueue(m_sessionSocket, TRUE);

				if( m_TalkMode != TALK_SELECTED )		// Some group call or all call
				{
					if( m_pSelectedItem )
						m_pSelectedItem->bSelected = FALSE;		// Not selected anymore

					deselectSocket( sessionSocket );

					m_DADeleteGroup(m_hDA, sessionSocket);

					outputDebug("doTalk: No longer in session, sessionSocket from %d to 0", sessionSocket);
					m_sessionSocket = 0;					// No longer in a session if not in call queue

					if( (tItemData=(struct _itemData *)getCQItemDataFromSocket(m_oldSessionSocket)) ||	// Old item?
						(tItemData=(struct _itemData *)getCallsWaitingItemData(0)) )	// Is there a call waiting item?
					{
						if( tItemData->bLocalRecord )		// If the 0'th or oldSessionSocket session is local then use it
						{
							SetHoldEnd(FALSE, TRUE);
							m_sessionSocket = m_oldSessionSocket;		// Restore the old session socket if there is one
							m_oldSessionSocket = 0;						// Clear this so we don't always get it back
						}
						else
							SetHoldEnd(TRUE, FALSE);
					}
					else
						SetHoldEnd(FALSE, FALSE);
				}
				else
				{
					return;
				}
			}
			else
				SetHoldEnd(FALSE, TRUE);

			lockControls(FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE);		// LOCK Talk, Listen, Chime, PlayFile, Radio Buttons
		}
	}
	else if ( bListening )
	{
		MarkCallQueueSession(sessionSocket, FALSE);				// Not held

#if 1
		endListen(sessionSocket);
		clearCount = 0;
		while( !waitClear(sessionSocket, STATUS_LISTENING) )		// Wait 200 ms for the intercom to clear down
		{
			if( ++clearCount > 2 )
			{
				bListening = FALSE;
				bNoRestart = FALSE;							// Allow restarts again

				outputDebug(_T("doTalk: Could not clear Listen after %d attempts"), clearCount);
				return;
			}
		}
#else
		if( m_pSelectedItem->iCom.status != STATUS_LISTENING )
			outputDebug(_T("doTalk: Listen->Talk not status LISTENING is %s"),
				szStatus(m_pSelectedItem->iCom.status, buffer));

		clearListen();
#endif
		bListening = FALSE;
		bNoRestart = FALSE;									// Allow restarts again

		if( m_sessionSocket == 0 || !m_pSelectedItem || m_pSelectedItem->iCom.status == STATUS_DISCONNECTED )							// Disconnected while waiting for the Listen to stop
		{
			outputDebug("doTalk: Done - Session ended - Talk not started");
			return;
		}

		startTalk(sessionSocket);
		while( !waitSet(sessionSocket, STATUS_TALKING) )
			startTalk(sessionSocket);

		if( isCallQueue(sessionSocket) )
			SetHoldEnd(TRUE, TRUE);

		if( m_sessionSocket != sessionSocket )							// 17Apr09 - Trying to fix a bug where the intercom reconnects during all of this
		{
			outputDebug("doTalk: session ended.  Attempting to stop talking");
			endTalk(sessionSocket);
		}

		lockControls(FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE);		// LOCK Talk, Listen, Chime, PlayFile, Radio Buttons
	}
	else
	{
		if( m_pSelectedItem->iCom.status == STATUS_COMMAND )
		{
			clearCount = 0;

			while( !waitClear(sessionSocket, STATUS_TALKING) )
			{
			if( ++clearCount > 2 )
				{
					bTalking = FALSE;
					bTalkRequest = FALSE;

					outputDebug(_T("doTalk: Could not clear Command after %d attempts"), clearCount);
					return;
				}
			}
		}

		MarkCallQueueSession(sessionSocket, FALSE);			// Not hend

		if( bListenRequest &&
			m_pSelectedItem && m_pSelectedItem->iCom.picRev.feature1 & OPNIC_FULL_DUPLEX)								// FDX?
		{
			startListen(sessionSocket);
			while( !waitSet(sessionSocket, STATUS_LISTENING) )
				startListen(sessionSocket);
		}

		if( m_sessionSocket == 0 || !m_pSelectedItem || m_pSelectedItem->iCom.status == STATUS_DISCONNECTED )							// Disconnected while waiting for the Listen to stop
		{
			outputDebug("doTalk: Done(2) - Session ended - Talk not started");
			return;
		}

		startTalk(sessionSocket);
		while( !waitSet(sessionSocket, bListenRequest?STATUS_FDX:STATUS_TALKING) )
			startTalk(sessionSocket);

		if( isCallQueue(sessionSocket) )
			SetHoldEnd(TRUE, TRUE);

		if( m_sessionSocket != sessionSocket )							// 17Apr09 - Trying to fix a bug where the intercom reconnects during all of this
		{
			outputDebug("doTalk: session ended(2).  Attempting to stop talking");
			endTalk(sessionSocket);
		}

		lockControls(FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE);		// LOCK Talk, Listen, Chime, PlayFile, Radio Buttons
	}

	outputDebug(_T("doTalk: Done"));
}

void CTalkMasterConsoleDlg::startListen(int sessionSocket, BOOL skipTest)
{
	struct _itemData *tItemData;

	if( (!skipTest && bListening) ||
		!sessionSocket )										// Cleared during a reconnect?
	{
		outputDebug(_T("startListen(%d): Exiting with bListening=%x, sessionSocket=%x"), skipTest, bListening, sessionSocket);
		return;
	}

	outputDebug(_T("startListen(%d)"), sessionSocket);

	if( tItemData = findItemData(sessionSocket) )
	{
//		if( tItemData->iCom.status != STATUS_LISTENING )					// Otherwise it is already listening, just continue
		{
			tItemData->iCom.status = STATUS_COMMAND;				// Make the local one not idle in case it is
			m_DAListenIcom( m_hDA, sessionSocket, WAVE_FORMAT_ULAW, TRUE );
		}
	}

//	Sleep(100);

	paintListen();

	bListening = TRUE;

	return;
}

void CTalkMasterConsoleDlg::paintListen()
{
	m_btnListen.SetBitmap( HBITMAP(mBitMapListenOn) );
}

void CTalkMasterConsoleDlg::endListen(int sessionSocket, BOOL skipWait)
{
	if( !bListening )
	{
		nListeningLevel = 0;
		drawVUMeter( &m_staticVUMeterSpeaker, bListening, 0 );
		return;
	}

//	bListening = FALSE;								// Don't clear this so we do not start recording data already coming in
	bNoRestart = TRUE;								// Don't let the received data restart the listen

	outputDebug(_T("endListen(%d)"), sessionSocket);

	m_DAListenIcom( m_hDA, sessionSocket, 0, FALSE );

	outputDebug(_T("endListen(%d) Completed"), sessionSocket);

	if( bSlow )
		Sleep(300);

//	if( !skipWait )
//		waitClear(m_sessionSocket);					// Wait for the listen to clear down

	clearListen();
}

void CTalkMasterConsoleDlg::clearListen()
{
	nListeningLevel = 0;
	drawVUMeter( &m_staticVUMeterSpeaker, bListening, 0 );

	m_btnListen.SetBitmap( HBITMAP(mBitMapListenOff) );
}

void CTalkMasterConsoleDlg::clearFdx()
{
	m_btnTalk.ShowWindow(SW_SHOW);
	m_btnFdxStart.ShowWindow(SW_HIDE);

	m_btnListen.ShowWindow(SW_SHOW);
	m_btnFdxMute.ShowWindow(SW_HIDE);

	m_btnNoListen.ShowWindow( SW_HIDE );

	m_btnFdxStart.SetBitmap( HBITMAP(mBitMapFdxTalkOff) );
	m_btnFdxMute.SetBitmap( HBITMAP(mBitMapFdxMute) );
}

void CTalkMasterConsoleDlg::startTalk(int sessionSocket)
{
	struct _itemData *tItemData;
	int type = TYPE_MICROPHONE;

	if( bUseUlaw )
		type |= TYPE_ULAW;

	if( bTalking || !sessionSocket )				// Already talking or session cleared during reconnect or disconnect
	{
		outputDebug(_T("startTalk: Exiting bTalking = %x, sessionSocket = %d"), bTalking, sessionSocket);
		return;
	}

	outputDebug(_T("startTalk(%d)"), sessionSocket);

	if( tItemData = findItemData(sessionSocket) )
		tItemData->iCom.status = STATUS_COMMAND;

	if( sessionSocket < 0 )
		type |= TYPE_UDP;

	m_DAStartAudio( m_hDA, sessionSocket, type );

//	Sleep(4000);

	micAudioSize = 0;
	audioPlayer.StartMic();

	m_btnTalk.SetBitmap( HBITMAP(mBitMapTalkOn) );

	bTalking = TRUE;

//	m_extraAudio = 0;
//	m_underflow = 0;

	return;
}

void CTalkMasterConsoleDlg::endTalk(int sessionSocket)
{
	if( !bTalking )
	{
		clearTalk();
		return;
	}

	outputDebug(_T("endTalk(%d)"), sessionSocket);

	bTalking = FALSE;															// Moved up to here from after clearTalk() 2/1/10 to try to not get audio sent after end

	m_DAEndAudio( m_hDA, sessionSocket, FALSE );

	audioPlayer.StopMic();

	if( bSlow || sessionSocket < 0 )											// Groups need to be status clear before leaving???
	{
		Sleep(300);
	}

//	outputDebug("endTalk(%d): Mic Stopped", m_sessionSocket);

	clearTalk();

	outputDebug("endTalk(%d): Completed", sessionSocket);
}

void CTalkMasterConsoleDlg::clearTalk()
{
	nTalkingLevel = 0;
	drawVUMeter( &m_staticVUMeterMicrophone, bTalking, 0 );

	m_btnTalk.SetBitmap( HBITMAP(mBitMapTalkOff) );
}

int CTalkMasterConsoleDlg::makeGroupSocket()
{
	if( m_tabMain.GetCurSel() == 0 )			// Intercoms page
		return(makeGroupSocketIcoms());
	else
		return(makeGroupSocketGroups());
}

int CTalkMasterConsoleDlg::makeGroupSocketIcoms()
{
	struct _itemData *tItemData;
	int	*intercoms, count, i, groupCount=0, socket;
	int	groupSocket=0;

	count = m_listIcoms.GetItemCount();

	intercoms = (int *)calloc(count, sizeof(int));

	for( i=0 ; i<count ; i++ )
	{
		if( m_TalkMode == TALK_GROUP )
		{
			if( m_listIcoms.GetCheck(i) )
			{
				socket = getIntercomSocket(i);
				tItemData = findItemData(socket);

				intercoms[groupCount++] = getIntercomSocket(i);
			}
		}
		else		// TALK_ALL_ACTIVE
		{
			socket = getIntercomSocket(i);

			tItemData = findItemData(socket);

			if( socket && tItemData && tItemData->iCom.status == STATUS_IDLE )				// If it is active
			{
				intercoms[groupCount++] = socket;
			}
		}
	}

	m_DACreateGroup(m_hDA, &groupSocket, intercoms, groupCount, FALSE);

	free(intercoms);

	return(groupSocket);
}

int CTalkMasterConsoleDlg::makeGroupSocketGroups()
{
//	struct _groupData *pGroupData;
	struct _groupDataList *pGroupList;
	struct _itemData *pItemData;
	struct _groupItems *pGroupItems;
	int count, groupSocket=0, index, groupCount=0, *sockets=NULL, memberIndex=0;

	count = getCheckedGroupsCount();										// How many groups are checked?

	if( count == 1 )
	{
		pGroupList = (_groupDataList*)m_listGroups.GetItemData(getFirstCheckedGroupIndex());

		if( pGroupList->type == 0 )
			groupSocket = pGroupList->pGroupData->groupSocket;
		else
		{
			pItemData = findItemData( &pGroupList->pIcomData->iCom.MAC );

			if( pItemData )
				groupSocket = pItemData->socket;
			else
				outputDebug("makeGroupSocketGroups: Intercom socket not found");
		}

		m_DACreateGroup(m_hDA, &groupSocket, NULL, 0, TRUE);				// Make a temp group so we can send stuff to the server based group

	}
	else if( count > 1 )				// More than one group selected
	{
// 1st we need to find out how many possible entries are in this list
		index = getFirstCheckedGroupIndex();

		while( index >= 0 )
		{
			pGroupList = (_groupDataList*)m_listGroups.GetItemData( index );

			if( pGroupList->type == 0 )				// Group entry
				groupCount += pGroupList->pGroupData->groupCount;
			else
				groupCount++;						// Intercom Entry

			index = getNextCheckedGroupIndex(index);
		}

// Now we need to allocate enough memory for all of the sockets

		sockets = (int*)calloc(groupCount, sizeof(int));

// Now we need to read in all of the sockets into this list, eliminating duplicates

		index = getFirstCheckedGroupIndex();

		while( index >= 0 )
		{
			pGroupList = (_groupDataList*)m_listGroups.GetItemData( index );

			if( pGroupList->type == 0 )							// We don't need to follow this list to other lists, we only get checked ones
			{
				m_DACreateGroup(m_hDA, &pGroupList->pGroupData->groupSocket, NULL, 0, TRUE);				// Make a temp group so we can send stuff to the server based group

				if( getListIcomsCount( pGroupList ) != pGroupList->pGroupData->groupCount )
					getListIcoms( pGroupList );

				refreshListIcoms( pGroupList );			// Make sure the icom data is up to date

				m_DADeleteGroup(m_hDA, pGroupList->pGroupData->groupSocket);

				pGroupItems = pGroupList->pGroupItems;

				while( pGroupItems )
				{
					if( pGroupItems->pIcomData->iCom.status &&										// Online
						!icomInGroup(pGroupItems->pIcomData->socket, sockets, memberIndex) )		// Not currently in the group
						sockets[memberIndex++] = pGroupItems->pIcomData->socket;

					pGroupItems = pGroupItems->next;
				}
			}
			else				// Only an intercom
			{
				if( pGroupList->pIcomData->iCom.status &&
					!icomInGroup(pGroupList->pIcomData->socket, sockets, memberIndex) )
				{
					pItemData = findItemData( &pGroupList->pIcomData->iCom.MAC );

					if( pItemData )
						sockets[memberIndex++] = pItemData->socket;
					else
						outputDebug("makeGroupSocketGroups: Intercom socket not found (2)");
				}
			}

			index = getNextCheckedGroupIndex(index);
		}

		m_DACreateGroup(m_hDA, &groupSocket, sockets, memberIndex, FALSE);				// Make a temp group so we can send stuff to the server based group

		if( sockets )
			free( sockets );
	}

	return(groupSocket);
}

int CTalkMasterConsoleDlg::countGroup()
{
	if( m_tabMain.GetCurSel() == 0 )
		return(countIcomGroup());
	else
		return(countServerGroup());
}

int CTalkMasterConsoleDlg::countIcomGroup()
{
	int groupCount = 0;
	int	count, i;

	count = m_listIcoms.GetItemCount();

	for( i=0 ; i<count ; i++ )
	{
		if( m_TalkMode == TALK_GROUP )
		{
			if( m_listIcoms.GetCheck(i) )
				groupCount++;
		}
		else		// TALK_ALL_ACTIVE
			groupCount++;
	}

	return(groupCount);
}

int CTalkMasterConsoleDlg::countServerGroup()
{
	struct _groupDataList *pGroupInfo;
	int groupCount = 0;
	int	count, i;

	count = m_listGroups.GetItemCount();

	for( i=0 ; i<count ; i++ )
	{
		if( m_listGroups.GetCheck(i) )
		{
			pGroupInfo = (_groupDataList*)m_listGroups.GetItemData(i);

			if( pGroupInfo->type == 0 )
				groupCount += pGroupInfo->pGroupData->groupCount;
			else
				groupCount++;
		}
	}

	return(groupCount);
}

short vuMeterData[4096];

void CTalkMasterConsoleDlg::doAudioData( int socket, struct _AudioData *ad )
{
	struct _itemData *tItemData, *tCQItemData;
	HANDLE hRec;
	int index=0, totalBytes=0, i, averageLevel=0, peakLevel=0;
	char buf[20];

	time_t ttime;
	struct tm	atime;

	tItemData = findItemData(socket);

	if( socket == m_sessionSocket &&
		( bListening ||
		  ( tItemData &&
		    tItemData->iCom.picRev.opMode1 & OPMODE_REMLISTN_DISA ) ) )
	{
#if 1
		cvtBuffer(vuMeterData, ad->buffer, ad->len, ad->audioFormat);

		peakLevel = 0;

		for( i=0;i<ad->len;i++ )
			if( peakLevel < abs(vuMeterData[i]) )
				peakLevel = abs(vuMeterData[i]);

		nListeningLevel = peakLevel/128;

//		outputDebug("Speaker Peak = %d", nTalkingLevel);
#else
		averageLevel = 0;

		for( i=0;i<dataLen;i++ )
			averageLevel += abs(abs(data[i])-128);

		averageLevel /= dataLen;
		nTalkingLevel = averageLevel;

		outputDebug(_T("Speaker = %d"), nTalkingLevel);
#endif
		drawVUMeter( &m_staticVUMeterSpeaker, bListening, nListeningLevel );

		audioPlayer.Write((PUCHAR)ad->buffer, ad->len, ad->audioFormat);
	}
	else if( theApp.UserOptions.recordCallsWaiting && !bShuttingDown )
	{
		if( resetCQList )
		{
			m_listCallsWaiting.DeleteAllItems();
			clearCallsWaitingItemData();
			resetCQList = FALSE;
		}

		tItemData = findItemData(socket);
		tCQItemData = getCQItemDataFromSocket(socket);
		if( !tCQItemData )						// No call queue record
		{
			time_t now;
			time( &now );
			atime = *localtime( &now );

			index = getCQInsert(tItemData);

// Set timer information if not already set for the announcement stuff
			if( !m_announceStartTime )
				SetTimer( TIMER_ANNOUNCE, 1000, NULL );				// Every second check

			if( !m_announceStartTime || (tItemData->cq.time-m_serverTimeDelta) < m_announceStartTime )		// Oldest time please
			{
				m_announceStartTime = tItemData->cq.time-m_serverTimeDelta;		// Convert the server time to local time
			}

			m_listCallsWaiting.InsertItem(index, _T(""));
			m_listCallsWaiting.SetItemText(index, CQ_COL_WAITING, (char *)tItemData->iCom.name);
			sprintf(buf, _T("Msg in at %d:%02d"), atime.tm_hour, atime.tm_min);
			m_listCallsWaiting.SetItemText(index,CQ_COL_DETAILS, buf);
			m_listCallsWaiting.SetItemText(index,CQ_COL_TIME, _T("0:00"));

			m_listCallsWaiting.SetItemData(index, (DWORD_PTR)tItemData);
			insertCallsWaitingItemData(index, (void*)tItemData);
		}

		if( tCQItemData && 
			tCQItemData == m_pCallWaitingItem &&					// Is it currently selected in CQ?
			m_pCallWaitingItem->hCallQueue )						// Does it have a call queue entry to close?
		{
				audioArchive.Close(m_pCallWaitingItem->hCallQueue);
//				m_pCallWaitingItem = NULL;
				activatePlayer(FALSE);
		}
//		outputDebug("doAudioData: getRecordingHandle(%d, %s, %d)", socket, "in", TRUE);

		hRec = audioArchive.getRecordingHandle( socket, _T("in"), TRUE, ad->audioFormat );		// Get or make a recording handle
		totalBytes = audioArchive.Write( hRec, ad->buffer, ad->len );
		tItemData->hCallQueue = hRec;

		ttime = totalBytes / 8000;
		atime = *localtime( &ttime );

		sprintf(buf, _T("%d:%02d"), atime.tm_min, atime.tm_sec);

		index = findCQIndexFromSocket( socket );
		m_listCallsWaiting.SetItemText(index,CQ_COL_TIME, buf);
	}

	if ( theApp.UserOptions.archiveAudio )
	{
		tItemData = findItemData(socket);
//		outputDebug("doAudioData: 2 getRecordingHandle(%d, %s, %d)", socket, "in", FALSE);
		hRec = audioArchive.getRecordingHandle( socket, _T("in"), FALSE, ad->audioFormat );		// Get or make a recording handle
		audioArchive.Write( hRec, ad->buffer, ad->len );
		tItemData->hArchive = hRec;
	}
}

void CTalkMasterConsoleDlg::doneAudioData( int socket )
{
	struct _itemData *tItemData;
	int	totalBytes;
	char buf[20];
	int index;

	time_t time;
	struct tm	atime;

	nListeningLevel = 0;

	if( socket == m_sessionSocket )	// && bListening )
	{
		if( bListening && !m_sessionDead && !bNoRestart )
		{
			outputDebug(_T("doneAudioData: re-startListen(%d) 1"), m_sessionSocket);
			startListen(socket, TRUE);					// Restart listening since we are really not done
//			waitSet(m_sessionSocket);
			return;
		}
		else
		{
			nTalkingLevel = 0;
			drawVUMeter( &m_staticVUMeterSpeaker, TRUE, 0 );
			audioPlayer.Flush();
		}
	}
	else if( theApp.UserOptions.recordCallsWaiting && !bShuttingDown )
	{
		if( !resetCQList )
		{
			tItemData = getCQItemDataFromSocket(socket);
			if( tItemData && tItemData->hCallQueue )
			{
				totalBytes = audioArchive.Close(tItemData->hCallQueue);

				time = totalBytes / 8000;
				atime = *localtime( &time );

				sprintf(buf, _T("%d:%02d"), atime.tm_min, atime.tm_sec);

				index = findCQIndexFromSocket( socket );
				m_listCallsWaiting.SetItemText(index,CQ_COL_TIME, buf);

				outputDebug(_T("doneAudioData: Writing calls waiting"));

				if( tItemData == m_pCallWaitingItem )
				{
					audioArchive.Open(tItemData->hCallQueue);
					m_sliderCQPlayer.SetRange(0, totalBytes / 8000);
					activatePlayer(TRUE);
				}
			}
		}
	}

	if( theApp.UserOptions.archiveAudio )
	{
		try
		{
			tItemData = findItemData(socket);
			if( tItemData && tItemData->hArchive && tItemData->bDisplayed )
			{
				audioArchive.Close(tItemData->hArchive);
				audioArchive.Delete(tItemData->hArchive, FALSE);
				tItemData->hArchive = NULL;
			}
			else if (!tItemData )
				outputDebug(_T("doneAudioData: tItemData NULL"));
		}
		catch(...)
		{
			outputDebug(_T("doneAudioData: Caught bad archive"));
		}
	}
}

int CTalkMasterConsoleDlg::findIndexFromSocket(int socket)
{
	int count, index;
	struct _itemData *tItemData=NULL;

	count = m_listIcoms.GetItemCount();

	for(index=0;index<count;index++)
	{
		tItemData = (struct _itemData *)getIcomItemData(index);
		if( tItemData && tItemData->socket == socket )
			break;

		tItemData = NULL;
	}

	if( index == count )
		index = -1;

	return(index);
}

int CTalkMasterConsoleDlg::findCQIndexFromSocket(int socket)
{
	int count, index;
	struct _itemData *tItemData=NULL;

	count = getCallsWaitingItemCount();

	for(index=0;index<count;index++)
	{
		tItemData = (struct _itemData *)getCallsWaitingItemData(index);

		if( tItemData && tItemData->socket == socket )
			break;

		tItemData = NULL;
	}

	if( index == count )
		index = -1;

	return(index);
}

struct _itemData *CTalkMasterConsoleDlg::getItemDataFromSocket(int socket)
{
	int count, index;
	struct _itemData *tItemData=NULL;

	count = m_listIcoms.GetItemCount();

	for(index=0;index<count;index++)
	{
		tItemData = (struct _itemData *)getIcomItemData(index);
		if( tItemData && tItemData->socket == socket )
			break;

		tItemData = NULL;
	}

	return(tItemData);
}

struct _itemData *CTalkMasterConsoleDlg::getCQItemDataFromSocket(int socket)
{
	int count, index;
	struct _itemData *tItemData=NULL;

	count = m_listCallsWaiting.GetItemCount();

	for(index=0;index<count;index++)
	{
		tItemData = (struct _itemData *)getCallsWaitingItemData(index);
		if( tItemData && tItemData->socket == socket )
			break;

		tItemData = NULL;
	}

	return(tItemData);
}

struct _itemData *CTalkMasterConsoleDlg::findItemData(int socket)
{
	struct _itemData *tItemData = m_pItemData;

	while( tItemData )
	{
		if( tItemData->socket == socket )
			break;

		tItemData = tItemData->next;
	}

	return(tItemData);
}

struct _itemData *CTalkMasterConsoleDlg::findItemData(struct _MAC *MAC)
{
	struct _itemData *tItemData = m_pItemData;

	while( tItemData )
	{
		if( !memcmp( &tItemData->iCom.MAC, MAC, 6 ) )
			break;

		tItemData = tItemData->next;
	}

	return(tItemData);
}

struct _groupDataList *CTalkMasterConsoleDlg::getGroupEntry(char *key)
{
	struct _groupDataList *pGroupDataList = m_pGroupData;

	while( pGroupDataList )
	{
		if( pGroupDataList->pGroupData && strcmp( pGroupDataList->pGroupData->groupKey, key ) == 0 )			// Match
			break;

		pGroupDataList = pGroupDataList->next;
	}

	return(pGroupDataList);
}

struct _groupDataList *CTalkMasterConsoleDlg::getGroupEntry(struct _MAC *MAC)
{
	struct _groupDataList *pGroupDataList = m_pGroupData;

	while( pGroupDataList )
	{
		if( pGroupDataList->pIcomData && memcmp(&pGroupDataList->pIcomData->iCom.MAC, MAC, 6) == 0 )			// Match
			break;

		pGroupDataList = pGroupDataList->next;
	}

	return(pGroupDataList);
}

struct _messageDataList *CTalkMasterConsoleDlg::getMessageEntry(struct _messageData *pMessageData)
{
	struct _messageDataList *pMessageDataList = m_pMessageDataList;

	while( pMessageDataList )
	{
		if( strcmp(pMessageDataList->pMessageData->key, pMessageData->key) == 0 )			// Match
			break;

		pMessageDataList = pMessageDataList->next;
	}

	return(pMessageDataList);
}

//char micAudio[2048];
int micAudioSize=0;
//
// serviceMicAudio
//
// Purpose		Service the microphone audio
//
void CTalkMasterConsoleDlg::serviceMicAudio(char *data, int len)
{
	int	peakLevel=0, i, testLevel;
	short *uLawData = (short *)data;

	if( !bTalking )
		return;

	if( bMuting )										// Throw away the microphone audio
	{
		if( bUseUlaw )									// uLaw center line audio only
		{
			for(i=0;i<len/2;i++)
				uLawData[i] = 0;
		}
		else
		{
			for(i=0;i<len;i++)
				data[i] = 0x7f;
		}
	}

	if( !bUseUlaw )
	{
		for( i=0;i<len;i++ )
			if( peakLevel < abs(abs(data[i])-128) )
				peakLevel = abs(abs(data[i])-128);
	}
	else
	{
//			if( peakLevel < (testLevel=abs(ulaw_to_s16(data[i]))>>7) )
		len /= 2;		// Convert from short -> char so 1/2 the length

		for( i=0;i<len;i++ )
		{
			if( peakLevel < (testLevel=abs(uLawData[i]) >> 7 ) )
				peakLevel = testLevel;

			data[i] = s16_to_ulaw(uLawData[i]);
		}
	}

	nTalkingLevel = min(peakLevel, 127);

	drawVUMeter( &m_staticVUMeterMicrophone, bTalking, nTalkingLevel );

//	outputDebug("serviceMicAudio: Talking Level = %d", nTalkingLevel);

	if( bTalking )
		m_DASendAudio(m_hDA, m_sessionSocket, data, len);
}

int CTalkMasterConsoleDlg::getIntercomSocket( int index )
{
	struct _itemData *tItemData;

	tItemData = (struct _itemData *)getIcomItemData(index);
	if( tItemData )
		return(tItemData->socket);
	else
		return(0);
}

BOOL CTalkMasterConsoleDlg::selectSocket( int socket )
{
	if( m_DASelectIntercom( m_hDA, socket, TRUE, SELECT_SOME ) != ERROR_SUCCESS )	// SELECT_SOME only affects groups
	{
		MessageBox(getResourceString(IDS_STRING_INTERCOM_ALREADY_IN_USE), getResourceString(IDS_STRING_ERROR));
		return(FALSE);
	}

	return(TRUE);
}

BOOL CTalkMasterConsoleDlg::deselectSocket( int socket )
{
	m_DASelectIntercom( m_hDA, socket, FALSE, NULL );

	return(TRUE);
}

BOOL CTalkMasterConsoleDlg::LoadProcs()
{
	BOOL bRet = TRUE;

	if( !(m_DAOpen = (DAOpen)GetProcAddress(m_hLib, "DAOpen")) )
		bRet = FALSE;
	if( !(m_DAClose = (DAClose)GetProcAddress(m_hLib, "DAClose")) )
		bRet = FALSE;
	if( !(m_DAStart = (DAStart)GetProcAddress(m_hLib, "DAStart")) )
		bRet = FALSE;
	if( !(m_DAStop = (DAStop)GetProcAddress(m_hLib, "DAStop")) )
		bRet = FALSE;
	if( !(m_DADllVersion = (DADllVersion)GetProcAddress(m_hLib, "DADllVersion")) )
		bRet = FALSE;
	if( !(m_DASetDebug = (DASetDebug)GetProcAddress(m_hLib, "DASetDebug")) )
		bRet = FALSE;

	if( !(m_DADisconnect = (DADisconnect)GetProcAddress(m_hLib, "DADisconnect")) )
		bRet = FALSE;

	if( !(m_DACreateGroup = (DACreateGroup)GetProcAddress(m_hLib, "DACreateGroup")) )
		bRet = FALSE;

	if( !(m_DADeleteGroup = (DADeleteGroup)GetProcAddress(m_hLib, "DADeleteGroup")) )
		bRet = FALSE;

	if( !(m_DAGetGroupMembers = (DAGetGroupMembers)GetProcAddress(m_hLib, "DAGetGroupMembers")) )
		bRet = FALSE;

	if( !(m_DASelectIntercom = (DASelectIntercom)GetProcAddress(m_hLib, "DASelectIntercom")) )
		bRet = FALSE;

	if( !(m_DAStartAudio = (DAStartAudio)GetProcAddress(m_hLib, "DAStartAudio")) )
		bRet = FALSE;

	if( !(m_DASendAudio = (DASendAudio)GetProcAddress(m_hLib, "DASendAudio")) )
		bRet = FALSE;

	if( !(m_DAPlayServerAudio = (DAPlayServerAudio)GetProcAddress(m_hLib, "DAPlayServerAudio")) )
		bRet = FALSE;

	if( !(m_DAEndAudio = (DAEndAudio)GetProcAddress(m_hLib, "DAEndAudio")) )
		bRet = FALSE;

	if( !(m_DAGetGPIOValues = (DAGetGPIOValues)GetProcAddress(m_hLib, "DAGetGPIOValues")) )
		bRet = FALSE;

	if( !(m_DASetGPIOValue = (DASetGPIOValue)GetProcAddress(m_hLib, "DASetGPIOValue")) )
		bRet = FALSE;

	if( !(m_DAiComCount = (DAiComCount)GetProcAddress(m_hLib, "DAiComCount")) )
		bRet = FALSE;

	if( !(m_DAFirstIcom = (DAFirstIcom)GetProcAddress(m_hLib, "DAFirstIcom")) )
		bRet = FALSE;

	if( !(m_DANextIcom = (DANextIcom)GetProcAddress(m_hLib, "DANextIcom")) )
		bRet = FALSE;

	if( !(m_DAGetIcom = (DAGetIcom)GetProcAddress(m_hLib, "DAGetIcom")) )
		bRet = FALSE;

	if( !(m_DAOpenDoor = (DAOpenDoor)GetProcAddress(m_hLib, "DAOpenDoor")) )
		bRet = FALSE;

	if( !(m_DAGetVolume = (DAGetVolume)GetProcAddress(m_hLib, "DAGetVolume")) )
		bRet = FALSE;

	if( !(m_DASetVolume = (DASetVolume)GetProcAddress(m_hLib, "DASetVolume")) )
		bRet = FALSE;

	if( !(m_DAForwardIcom = (DAForwardIcom)GetProcAddress(m_hLib, "DAForwardIcom")) )
		bRet = FALSE;

	if( !(m_DARetrieveIcom = (DARetrieveIcom)GetProcAddress(m_hLib, "DARetrieveIcom")) )
		bRet = FALSE;

	if( !(m_DAListenIcom = (DAListenIcom)GetProcAddress(m_hLib, "DAListenIcom")) )
		bRet = FALSE;

	if( !(m_DADeleteServerCQ = (DADeleteServerCQ)GetProcAddress(m_hLib, "DADeleteServerCQ")) )
		bRet = FALSE;

	if( !(m_DATransferAudio = (DATransferAudio)GetProcAddress(m_hLib, "DATransferAudio")) )
		bRet = FALSE;

	if( !(m_DAResetConnection = (DAResetConnection)GetProcAddress(m_hLib, "DAResetConnection")) )
		bRet = FALSE;

	if( !(m_DASayText = (DASayText)GetProcAddress(m_hLib, "DASayText")) )
		bRet = FALSE;

	if( !(m_DAResendSettings = (DAResendSettings)GetProcAddress(m_hLib, "DAResendSettings")) )
		bRet = FALSE;

	if( !(m_DARingIntercom = (DARingIntercom)GetProcAddress(m_hLib, "DARingIntercom")) )
		bRet = FALSE;

	return(bRet);
}

//
// doAudioRequest
//
// Purpose:		Service the request for data
//
void CTalkMasterConsoleDlg::doAudioRequest( int socket, int dataLen )
{
	struct _fileList *fl;
	char readBuf[4096];
	int	readLen;

	fl = getFileList(socket);
	if( !fl )
		return;

	if( fl->filePos < fl->fileLen )
	{
		dataLen = min( fl->fileLen-fl->filePos, dataLen );

		readLen=(int)fread(readBuf, 1, dataLen, fl->fp);

		fl->filePos += readLen;

		m_DASendAudio(m_hDA, socket, readBuf, readLen);
	}
	else
	{
		m_DAEndAudio( m_hDA, socket, FALSE );		// The file list is released in the doneAudioFile() function
	}
}

BOOL CTalkMasterConsoleDlg::sendAudioFile( int socket, char *fname, BOOL bSelectIcom, BOOL repeat )
{
	struct _fileList *newFile;
	FILE *fp;
	WAVEFORMATEX waveHeader;
	int type=TYPE_FILE | TYPE_NO_BEEPS;

	if( bSelectIcom && !selectSocket( socket ) )
		return(FALSE);

	if( m_TalkMode != TALK_SELECTED )
		type |= TYPE_UDP;

	outputDebug(_T("sendAudioFile %s"), fname);

	if( getFileList(socket) )
	{
		outputDebug(_T("getFileList(%d) already exists for socket"), socket);
		deselectSocket( socket );

		return(FALSE);
	}

	if( !(fp=fopen(fname, _T("rb"))) )
	{
		outputDebug(_T("fopen(%s) failed"), fname);
		deselectSocket( socket );

		return(FALSE);
	}

	newFile = (struct _fileList *)calloc(1, sizeof(struct _fileList));

	newFile->bSelect = bSelectIcom;

	strcpy(newFile->fileName, fname);				// Save the filename so we can restart it if we are replaying audio
	newFile->bRepeat = repeat;

	newFile->next = m_fileList.next;
	m_fileList.next = newFile;

	newFile->fp = fp;
	newFile->socket = socket;

	newFile->fileLen = readWaveHeader(fp, &waveHeader);

	outputDebug(_T("Calling DAStartAudio"));

	if(waveHeader.wFormatTag == WAVE_FORMAT_ULAW)
		type |= TYPE_ULAW;

	m_DAStartAudio( m_hDA, socket, type );

	return(TRUE);
}

void CTalkMasterConsoleDlg::stopAudioFile( int socket )
{
	struct _fileList *fl;

	fl = getFileList(socket);
	if( !fl )
	{
		outputDebug(_T("stopAudioFile: File for socket %d NULL"), socket);
		return;
	}

	if( fl->fp )
		fclose(fl->fp);
	fl->fp = NULL;

	delFileList(socket);

	m_DAEndAudio( m_hDA, socket, TRUE );
//	waitClear(socket);

//	if( m_TalkMode != TALK_SELECTED )
//		m_DADeleteGroup(m_hDA, m_playFileSocket);		// Really can not do this since it will wait for the thread to end

//	m_playFileSocket = 0;

	//	deleteCallQueue(m_sessionSocket, TRUE);			// Delete it from the local list
}

void CTalkMasterConsoleDlg::doneAudioFile( int socket )
{
	struct _fileList *fl;
	char fname[MAX_PATH];
	BOOL	bRepeat=FALSE, bSelect=FALSE;
	struct _itemData *pItemData;

//	if( socket == m_playFileSocket )
	{
//		deleteCallQueue(m_sessionSocket, TRUE);			// Delete it from the local list

		fl = getFileList(socket);							// We need to get rid of this file list entry
		if( fl )
		{
			if( fl->fp )									// Can be NULL if it is a server file being sent
				fclose(fl->fp);									// Close the file
			fl->fp = NULL;

			strcpy(fname, fl->fileName);					// Copy the filename in case we are going to repeat
			bRepeat = fl->bRepeat;
			bSelect = fl->bSelect;

			if( bSelect )
			{
				if( !bRepeat )								// 30Mar10: Added to try to make the repeat test volume file to work, used to always do addWork
					addWork( socket, WORK_DESELECT, time(NULL)+1 );
				else
					m_DASelectIntercom( m_hDA, socket, FALSE, NULL );
			}

			delFileList(socket);							// Get rid of the entry
		}

		else
		{
			addWork( socket, WORK_DESELECT, time(NULL)+1 );

//			m_DASelectIntercom( m_hDA, socket, FALSE, NULL );

			sendTextDlg.m_bWaitingForCompletion = FALSE;		// It is done now
		}

		pItemData = getItemDataFromSocket(socket);

		if( bRepeat && (socket<0 || pItemData) )									// If we are supposed to repeat
		{
			if( socket < 0 || (pItemData->iCom.status&STATUS_MASK) == STATUS_IDLE )
				sendAudioFile(socket, fname, bSelect, TRUE);				// Send it again
			else
			{
				m_repeatSocket = socket;
				m_csRepeatFileName = fname;
				m_pRepeatItemData = pItemData;

				SetTimer( TIMER_REPEAT, 500, NULL );
			}
		}
		else	// if( m_TalkMode != TALK_SELECTED )
		{
			m_btnPlayFile.SetWindowText(_T(getResourceString(IDS_STRING_PLAY_FILE)));

			lockControls(FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE);

			if( m_playFileSocket < 0 )						// Is it a group?
			{
				addWork( m_playFileSocket, WORK_DELETE_GROUP, time(NULL)+1 );
//				m_DADeleteGroup(m_hDA, m_playFileSocket);	// Really can not do this since it														// will wait for the thread to end
			}
			m_playFileSocket = 0;
			bPlayFileLocal = TRUE;
			m_tabMain.EnableWindow(TRUE);
		}
	}
}

BOOL CTalkMasterConsoleDlg::playServerFile( int socket, char *fileKey )
{
	struct _fileList *newFile;
	int type=TYPE_FILE | TYPE_NO_BEEPS | TYPE_UDP;
	BOOL bRet;

	if( !selectSocket( socket ) )
	{
		MessageBox( _T(getResourceString(IDS_STRING_SERVER_GROUP_EMPTY_OR_UNAVAILABLE)), _T(getResourceString(IDS_STRING_SERVER_GROUP_EMPTY_OR_UNAVAILABLE)));
		return(FALSE);
	}

	newFile = (struct _fileList *)calloc(1, sizeof(struct _fileList));

	newFile->bSelect = TRUE;

	newFile->next = m_fileList.next;
	m_fileList.next = newFile;

	newFile->socket = socket;

	outputDebug(_T("Calling m_DAPlayServerAudio(0x%x, %d, %s, 0x%x)"), m_hDA, socket, fileKey, type);

	bRet = (m_DAPlayServerAudio( m_hDA, socket, fileKey, type ) == ERROR_SUCCESS);

	if( !bRet )					// Did not work, so we need to unselect the group (because this function selected it) and get out of here with a FALSE
	{
		MessageBox( _T(getResourceString(IDS_STRING_SERVER_FILE_NOT_FOUND)), _T(getResourceString(IDS_STRING_SERVER_FILE_NOT_FOUND)));
		deselectSocket( socket );
	}

	return(bRet);
}

struct _fileList *CTalkMasterConsoleDlg::getFileList(int socket)
{
	struct _fileList *fl = &m_fileList;

	while(fl)
	{
		if( fl->socket == socket )
			return(fl);

		fl = fl->next;
	}

	return(fl);
}

BOOL CTalkMasterConsoleDlg::delFileList(int socket)
{
	struct _fileList *fl = &m_fileList;
	struct _fileList *next;

	while(fl->next)
	{
		if( fl->next->socket == socket )
		{
			next = fl->next;
			fl->next = next->next;
			free(next);
			return(TRUE);
		}

		fl = fl->next;
	}

	return(FALSE);
}

int	 CTalkMasterConsoleDlg::readWaveHeader(FILE *fp, WAVEFORMATEX *waveHeader)
{
	char buff[20];
	struct _fmtHeader fmt;

	if( fread(buff, 1, 4, fp) != 4 )		// Should be "RIFF"
		return(0);

	if( fread(buff, 1, 8, fp) != 8 )		// length of the rest of the file
		return(0);

	while(1)
	{
		if( fread(buff, 1, 4, fp) != 4 )
			return(0);

		if( !memcmp(buff, (char *)"fmt ", 4) )
		{
			if( fread(buff, 1, 20, fp) != 20 )
				return(0);

			memcpy(&fmt, buff, 20);
			
			waveHeader->nAvgBytesPerSec = fmt.byteRate;
			waveHeader->nBlockAlign = fmt.blockAlign;
			waveHeader->nChannels = fmt.numChannels;
			waveHeader->nSamplesPerSec = fmt.sampleRate;
			waveHeader->wBitsPerSample = fmt.bitsPerSample;
			waveHeader->wFormatTag = fmt.audioFormat;

			if( fmt.len != 16 )
			{
				fmt.len -= 16;
				fread(buff, 1, fmt.len, fp);
			}
		}
		else if( !memcmp(buff, (char*)"data", 4) )
		{
			if( fread(&fmt, 1, 4, fp) != 4 )
				return(0);

			return(fmt.len);
		}
		else
		{
			if( fread(&fmt, 1, 4, fp) != 4 )
				return(0);

			fseek(fp, fmt.len, SEEK_CUR);
		}
	}

	return(0);
}

void CTalkMasterConsoleDlg::updateListIcomsStatus(struct _iComStructure *pIcom, int index)
{
	char *msg, buffer[128];

	if( !pIcom )
		msg = _T("Unknown");
	else
		msg = szStatus(pIcom->status, buffer);

	m_listIcoms.SetItemText(index,COL_STATUS,msg);
}

int CTalkMasterConsoleDlg::getInsert(struct _itemData *item)
{
	int i, count;
	struct _itemData *tItemData;

	count = m_listIcoms.GetItemCount();

	for(i=0;i<count;i++)
	{
		tItemData = (struct _itemData *)getIcomItemData(i);

		switch( m_sortBy )
		{
		case COL_GROUP:
			break;

		case COL_LOCATION:
			if( m_bOrder )
			{
				if( strcmp((char*)tItemData->iCom.name, (char*)item->iCom.name) <= 0 )
					return(i);
			}
			else
			{
				if( strcmp((char*)tItemData->iCom.name, (char*)item->iCom.name) >= 0 )
					return(i);
			}

			break;

		case COL_QUEUE:
//			tItemData->iComQueue.queueName
			if( m_bOrder )
			{
				if( strcmp((char*)tItemData->iComQueue.queueName, (char*)item->iComQueue.queueName) <= 0 )
					return(i);
			}
			else
			{
				if( strcmp((char*)tItemData->iComQueue.queueName, (char*)item->iComQueue.queueName) >= 0 )
					return(i);
			}

			break;

		case COL_STATUS:

			if( m_bOrder )
			{
				if( tItemData->iCom.status < item->iCom.status )
					return(i);
			}
			else
			{
				if( tItemData->iCom.status > item->iCom.status )
					return(i);
			}

			break;

		case COL_ADDRESS:
			if( m_bOrder )
			{
				if( memcmp(&tItemData->iCom.addr_in.sin_addr, &item->iCom.addr_in.sin_addr, sizeof(in_addr)) <= 0 )
					return(i);
			}
			else
			{
				if( memcmp(&tItemData->iCom.addr_in.sin_addr, &item->iCom.addr_in.sin_addr, sizeof(in_addr)) >= 0 )
					return(i);
			}

			break;

		case COL_DOOR:

			if( m_bOrder )
			{
				if( (tItemData->iCom.picRev.opMode1 & OPMODE_RELAY) == (item->iCom.picRev.opMode1 & OPMODE_RELAY) )
				{
					if( strcmp((char*)tItemData->iCom.name, (char*)item->iCom.name) <= 0 )
						return(i);
				}
				else if( (tItemData->iCom.picRev.opMode1 & OPMODE_RELAY) > (item->iCom.picRev.opMode1 & OPMODE_RELAY) )
					return(i);
			}
			else
			{
				if( (tItemData->iCom.picRev.opMode1 & OPMODE_RELAY) == (item->iCom.picRev.opMode1 & OPMODE_RELAY) )
				{
					if( strcmp((char*)tItemData->iCom.name, (char*)item->iCom.name) >= 0 )
						return(i);
				}
				else if( (tItemData->iCom.picRev.opMode1 & OPMODE_RELAY) < (item->iCom.picRev.opMode1 & OPMODE_RELAY) )
					return(i);
			}

			break;

		}
	}

	return(i);
}

void CTalkMasterConsoleDlg::doTimerTest()
{
	int newTimer;

	if( !bLoggedOn || bShuttingDown )
	{
		KillTimer(TIMER_TEST);
		m_bTestMode = FALSE;
		UpdateData(FALSE);
		return;
	}

	newTimer = (rand() % (m_maxMsec-99))+100;

	SetTimer(TIMER_TEST, newTimer, NULL);

	if( bTalking == bTalkRequest )
	{
		if( !bTalking )
			OnBtnDnButtonTalk();
		else
			OnBtnUpButtonTalk();
	}
}

void CTalkMasterConsoleDlg::doTalkTimer()
{
	static BOOL bTalkingLast=FALSE, bTalkRequestLast=FALSE, bListeningLast=FALSE, bListenRequestLast=FALSE;

	if( !bLoggedOn )
	{
		return;
//		doLogon();
	}

	if( m_talkTimerGuard &&
	   (bTalkingLast != bTalking ||
	    bTalkRequestLast != bTalkRequest ||
		bListeningLast != bListening ||
		bListenRequestLast != bListenRequest )
//		1
		)
	{
		outputDebug(_T("doTalkTimer: guard=%d bTalking=%s, Request=%s / bListening=%s, request=%s"), 
			m_talkTimerGuard,
			(bTalking)?_T("TRUE"):_T("FALSE"),
			(bTalkRequest)?_T("TRUE"):_T("FALSE"),
			(bListening)?_T("TRUE"):_T("FALSE"),
			(bListenRequest)?_T("TRUE"):_T("FALSE") );
	}

	bTalkingLast = bTalking;
	bTalkRequestLast = bTalkRequest;
	bListeningLast = bListening;
	bListenRequestLast = bListenRequest;

	if( ++m_talkTimerGuard == 1 )
	{
		if( bTalking != bTalkRequest )
		{
			SetHoldEnd(FALSE, FALSE);					// Don't want these working while we are in the routines

			doTalk();
		}

// Now make sure that we doListen() propely

		else if( bListening != bListenRequest )
		{
			SetHoldEnd(FALSE, FALSE);					// Don't want these working while we are in the routines

			doListen();
		}

		if( !bTalking && !bListening && !bTalkRequest && !bListenRequest && bReleaseRequest )
		{
			doRelease();
		}
	}
	--m_talkTimerGuard;
}

void CTalkMasterConsoleDlg::doNewIntercom(int socket, struct _iComStructure *pIcom, struct _iComQueueInfo *pIcomQueue)
{
	struct _iComQueueList *listEntry=NULL;
	struct _itemData *tItemData;
	int		index, count;
	BOOL	bDisplayIcom = FALSE;
	char	buffer[128];

#if 0			// Now dealing with existing versus new entries in the actual display section of code

// We moved the next section to delete existing entries into this code to be able to work for both new connections
// and reconnections.  There seemed to be cases where we got a new connection for entries that we already in the list

	tItemData = (struct _itemData *)findItemData(&pIcom->MAC);

	if( tItemData )
	{
		count = m_listIcoms.GetItemCount();
		index = findIntercom(pIcom);

		if( index < count )
		{
			m_listIcoms.DeleteItem(index);
			deleteIcomItemData(index);
		}
	}
#endif

	if( bShuttingDown )				// If we are shutting down don't do this
		return;

// Now see if we can find it (we should not be able to)

	tItemData = (struct _itemData *)findItemData(&pIcom->MAC);

	if( !tItemData )
	{
		tItemData = (struct _itemData *)calloc(1, sizeof(struct _itemData));
		tItemData->next = m_pItemData;
		m_pItemData = tItemData;
	}

	m_nIntercomsServer++;
	m_nIntercomsServerAlive++;

	if( pIcomQueue )
	{
		listEntry = findQueueMembership( pIcomQueue );

		if( !listEntry )
			bDisplayIcom = TRUE;
		else if( listEntry->queueMembership.membership == 1 )
			bDisplayIcom = TRUE;
		else if (listEntry->queueMembership.membership == 2 && listEntry->queueMembership.bOverflow)
			bDisplayIcom = TRUE;
		else if (listEntry->queueMembership.bOverflow == 2)			// Forced EVERYONE list this queue
			bDisplayIcom = TRUE;
	}
	else
		bDisplayIcom = TRUE;										// No queue, display this please
//
// Only add this intercom to the displayed list if it is a primary or overflowing queue
//
	memcpy(&tItemData->iCom, pIcom, sizeof(struct _iComStructure));

	if( bDisplayIcom )
	{
		if( resetList )
		{
			deleteIcoms(TRUE);
			resetList = FALSE;
		}

		m_nIntercomsConsole++;

		if( pIcom->status && tItemData->bCounted == FALSE )
		{
			m_nIntercomsConsoleAlive++;
			tItemData->bCounted = TRUE;
		}

		count = m_listIcoms.GetItemCount();
		index = findIntercom(pIcom);

		if( count == index )							// Not in the list so find a spot
		{
			EnterCriticalSection(&CriticalSection); 

	//			index = dlg->m_listIcoms.GetItemCount();
			index = getInsert(tItemData);

			m_listIcoms.InsertItem( LVIF_TEXT|LVIF_IMAGE, index, _T(""), 0, 0, 10, NULL);

			insertIcomItemData(index, (void *)tItemData);

			LeaveCriticalSection(&CriticalSection);

			tItemData->bDisplayed = TRUE;
		}

		m_listIcoms.SetItemText(index,COL_LOCATION, (LPCTSTR)pIcom->name);
		m_listIcoms.SetItemText(index,COL_QUEUE, (LPCSTR)listEntry->queueMembership.queueName);
		m_listIcoms.SetItemText(index,COL_ADDRESS, inet_ntoa(pIcom->addr_in.sin_addr));
		m_listIcoms.SetItemText(index,COL_STATUS, szStatus(pIcom->status, buffer));

		if( pIcom->picRev.opMode1 & OPMODE_RELAY )
			m_listIcoms.SetItem( index, COL_DOOR, LVIF_IMAGE, 0, 0, 0, 0, NULL );	// LVIS_SELECTED
		else
			m_listIcoms.SetItem( index, COL_DOOR, LVIF_IMAGE, 0, 4, 0, 0, NULL );	// LVIS_SELECTED

//		m_listIcoms.SetItemData(index, (DWORD_PTR)tItemData);

		if( tItemData == m_pSelectedItem )
		{
			m_listIcoms.SetItemState(index, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
			showHdxFdxControls();
		}

//			dlg->m_listIcoms.SortItems( sortIcoms, (DWORD)&dlg->m_listIcoms );

//		if( tItemData->iCom.picRev.opMode2 & OPMODE_ALWAYS_DISPLAY )			// 10June2009:no longer show these intercoms on the groups tab
//			doNewGroupInfo(tItemData);
	}

//		dlg->outputDebug("EVENT:New Connection %d Status = %d", socket, pIcom->status);

	if( pIcomQueue )
	{
		memcpy( &tItemData->iComQueue, pIcomQueue, sizeof(struct _iComQueueInfo) );

		if( listEntry )
			strcpy( tItemData->iComQueue.queueName, listEntry->queueMembership.queueName );

//			dlg->outputDebug("EVENT:New Connection Queue '%s' %d", pIcomQueue->queueName, pIcomQueue->priority);
	}

	tItemData->socket = socket;

	if( !(m_uFlags & DA_REMOTE) )			// Only send if local
		sendAudioFile(socket, _T("iAudio\\i_activated_us.wav"), TRUE, FALSE);

//		dlg->outputDebug("Connected socket %d from %s:%d", socket, 
//						inet_ntoa(pIcom->addr_in.sin_addr), 
//						ntohs(pIcom->addr_in.sin_port));

	displayIntercomCount();

	updateGroupIcons();
}

void CTalkMasterConsoleDlg::lockControls(BOOL talk, BOOL listen, BOOL chime, BOOL playFile, BOOL radio, BOOL testVolume, BOOL volumeControls, BOOL groupControls)
{
	m_btnTalk.EnableWindow(!talk);
	m_btnFdxStart.EnableWindow(!talk);
	m_btnFdxMute.EnableWindow(!talk);

	m_btnChime.EnableWindow(!chime);
	m_btnPlayFile.EnableWindow(!playFile);

	if( m_tabMain.GetCurSel() == 0 )
	{
		m_btnListen.EnableWindow(!listen);
		m_btnNoListen.EnableWindow(!listen);

		m_btnSelected.EnableWindow(!radio);
		m_btnAllActive.EnableWindow(!radio);
	}

	m_btnGroup.EnableWindow(!radio);
	m_btnTestVolume.EnableWindow(!testVolume && GetFileAttributes(theApp.UserOptions.testVolumeFile)!=INVALID_FILE_ATTRIBUTES);
	m_btnSetVolume.EnableWindow(!volumeControls);
	m_btnGetVolume.EnableWindow(!volumeControls && m_TalkMode == TALK_SELECTED);

	m_listGroups.EnableWindow( !groupControls );
	m_listMessages.EnableWindow( !groupControls );
}

char *CTalkMasterConsoleDlg::szStatus( unsigned statusCode, char *szRetBuffer )
{
	CString status;

	switch(statusCode)
	{
	case STATUS_ICOM_TIMEOUT:
		status.LoadString(IDS_STRING_STATUS_TALKING_TO);		// "Talking (TO)"
		break;

	case STATUS_DISCONNECTED:
		status.LoadString(IDS_STRING_STATUS_DISCONNECTED);
		break;

	case STATUS_IDLE:
		status.LoadString(IDS_STRING_STATUS_CONNECTED);
		break;

	case STATUS_INUSE:
		status.LoadString(IDS_STRING_STATUS_IN_USE);
		break;

	case STATUS_INUSE_ONHOOK:
	case STATUS_IDLE_ONHOOK:
		status.LoadString(IDS_STRING_STATUS_ONHOOK);
		break;

	case STATUS_TALKING:
	case STATUS_TALKING_INUSE:
	case STATUS_TALKING_ONHOOK:
	case STATUS_TALKING_ONHOOK_INUSE:
		status.LoadString(IDS_STRING_STATUS_TALKING);
		break;

	case STATUS_LISTENING:
	case STATUS_LISTENING_INUSE:
	case STATUS_LISTENING_ONHOOK:
	case STATUS_LISTENING_ONHOOK_INUSE:
		status.LoadString(IDS_STRING_STATUS_LISTENING);
		break;

	case STATUS_COMMAND:
	case STATUS_COMMAND_INUSE:
	case STATUS_COMMAND_ONHOOK:
	case STATUS_COMMAND_ONHOOK_INUSE:
		status.LoadString(IDS_STRING_STATUS_COMMAND);
		break;

	case STATUS_FDX:
	case STATUS_FDX_INUSE:
	case STATUS_FDX_ONHOOK:
	case STATUS_FDX_ONHOOK_INUSE:
		status.LoadString(IDS_STRING_STATUS_FDX);
		break;

	default:
		status.LoadString(IDS_STRING_STATUS_UNKNOWN);
		break;
	}

	strcpy(szRetBuffer, status.GetBuffer());

	return(szRetBuffer);
}

int CTalkMasterConsoleDlg::getCallsWaitingItemCount()
{
	return(callsWaiting.dataSize);
}

void CTalkMasterConsoleDlg::insertCallsWaitingItemData(int index, void *data)
{
	EnterCriticalSection( &CriticalSectionCallQueue );
	insertItemData( &callsWaiting, index, data );
	LeaveCriticalSection( &CriticalSectionCallQueue );
}

void CTalkMasterConsoleDlg::deleteCallsWaitingItemData(int index)
{
	EnterCriticalSection( &CriticalSectionCallQueue );
	deleteItemData( &callsWaiting, index );
	LeaveCriticalSection( &CriticalSectionCallQueue );
}

void *CTalkMasterConsoleDlg::getCallsWaitingItemData(int index)
{
	return(getItemData( &callsWaiting, index ));
}

void CTalkMasterConsoleDlg::setCallsWaitingItemData(int index, void *data)
{
	setItemData( &callsWaiting, index, data );
}

void CTalkMasterConsoleDlg::clearCallsWaitingItemData()
{
	EnterCriticalSection( &CriticalSectionCallQueue );
	clearItemData(&callsWaiting);
	LeaveCriticalSection( &CriticalSectionCallQueue );
}

void CTalkMasterConsoleDlg::insertIcomItemData(int index, void *data)
{
	insertItemData( &icomList, index, data );
}

void CTalkMasterConsoleDlg::deleteIcomItemData(int index)
{
	deleteItemData( &icomList, index );
}

void *CTalkMasterConsoleDlg::getIcomItemData(int index)
{
	return(getItemData( &icomList, index ));
}

void CTalkMasterConsoleDlg::setIcomItemData(int index, void *data)
{
	setItemData( &icomList, index, data );
}

void CTalkMasterConsoleDlg::clearIcomItemData()
{
	clearItemData(&icomList);
}

void CTalkMasterConsoleDlg::insertItemData(struct _listData *list, int index, void *data)
{
	if( !list->data )
	{
#if	USE_GLOBAL_MEMORY
		list->data = (void **)GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, (list->dataSize+1) * sizeof(void*));
		list->data = (void **)GlobalLock(list->data);
#else
		list->data = (void **)calloc(1, ((list->dataSize+1) * sizeof(void*)));
#endif
	}
	else
	{
#if USE_GLOBAL_MEMORY
		GlobalUnlock(list->data);
		list->data = (void **)GlobalReAlloc(list->data, (list->dataSize+1) * sizeof(void*), GMEM_MOVEABLE | GMEM_ZEROINIT);
		list->data = (void **)GlobalLock(list->data);
#else
		list->data = (void **)realloc( list->data, ((list->dataSize+1) * sizeof(void*)));
#endif
	}

	if( list->dataSize && list->dataSize > index )
		memmove(&list->data[index+1], &list->data[index], (list->dataSize-index)*sizeof(void*));

	list->data[index] = data;

	list->dataSize++;
}

void CTalkMasterConsoleDlg::deleteItemData(struct _listData *list, int index)
{
	if( index >= list->dataSize )
	{
		OutputDebugString(_T("deleteItemData: Overflow"));
		return;
	}

	if( list->dataSize == 1 )
	{
		list->dataSize = 0;
#if USE_GLOBAL_MEMORY
//		list->data = (void**)GlobalUnlock(list->data);
		GlobalFree(list->data);
#else
		free(list->data);
#endif
		list->data = NULL;
		return;
	}

	memcpy(&list->data[index], &list->data[index+1], (list->dataSize-index-1)*sizeof(void*));

#if USE_GLOBAL_MEMORY
	GlobalUnlock(list->data);
	list->data = (void **)GlobalReAlloc(list->data, (list->dataSize-1) * sizeof(void*), GMEM_MOVEABLE | GMEM_ZEROINIT);
	list->data = (void **)GlobalLock(list->data);
#else
	list->data = (void **)realloc( list->data, ((list->dataSize-1) * sizeof(void*)) );
#endif

	list->dataSize--;
}

void *CTalkMasterConsoleDlg::getItemData(struct _listData *list, int index)
{
	if( !list->dataSize || index >= list->dataSize )
		return(NULL);									// Is not in the current list

	return(list->data[index]);
}

void CTalkMasterConsoleDlg::setItemData(struct _listData *list, int index, void *data)
{
	if( index >= list->dataSize )
		OutputDebugString(_T("setItemData: Overflow"));
	else
		list->data[index] = data;
}

void CTalkMasterConsoleDlg::clearItemData(struct _listData *list)
{
	if( list->data )
	{
#if USE_GLOBAL_MEMORY
//		list->data = (void**)GlobalUnlock(list->data);
		GlobalFree(list->data);
#else
		free(list->data);
#endif
	}

	list->data = NULL;

	list->dataSize = 0;
}

void CTalkMasterConsoleDlg::doOpenDoor(struct _itemData *pItem)
{
	int index; 

	if( pItem && 
		pItem->iCom.picRev.opMode1 & OPMODE_RELAY &&
		pItem->iCom.status &&									// And it is not disconnected
	  ( pItem->iCom.picRev.opMode1 & OPMODE_SUPPORTS_DLE_AUDIO ||
	   (pItem->iCom.status&STATUS_MASK) != STATUS_TALKING  ) )		// And it is NOT TALKING or timed out talking
	{
		m_DAOpenDoor(m_hDA, pItem->socket, 5);

		index = findIntercom(&pItem->iCom);

		m_listIcoms.SetItem( index, COL_DOOR, LVIF_IMAGE, 0, 1, 0, 0, NULL );	// LVIS_SELECTED
//			m_listIcoms.SetItemText(index, COL_DOOR, "OPEN");
		SetTimer( TIMER_DOOR, 200, NULL );

		pItem->doorOpenTimer = time(NULL)+5;
	}
}

void CTalkMasterConsoleDlg::doRelease()
{
	if( m_pReleasedItem )
		m_pReleasedItem->bSelected = FALSE;		// Not selected anymore

	deselectSocket( m_nReleaseSocket );

	m_nReleaseSocket = 0;
	m_sessionSocket = 0;
//	SetHoldEnd(FALSE, FALSE);					// This is commented in 1.4.1.x

	m_btnNoListen.SetBitmap( HBITMAP(mBitMapNoListenOff) );

	lockControls(FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE);		// LOCK Talk, Listen, Chime, PlayFile, Radio Buttons

	m_pReleasedItem = NULL;
	bReleaseRequest = FALSE;					// fullfilled the request
}

//
// doSessionStartStop
//
// This is the code that get's call over and over until the session is actually answered by the intercom.
//
// This code will cause the intercom to start ringing, and then when the intercom handset is raised will send a new status called OFFHOOK.
// This will indicate to the code that it should go directly into the rest of the session, or regular TALK/LISTEN buttons
//
// This session ends as usual once the TALK/LISTEN cycle begins
//

void CTalkMasterConsoleDlg::doSessionStartStop()
{
	int index;

	if( !bLoggedOn )
	{
		bTalking = FALSE;
		bTalkRequest = FALSE;
		if( bSessionRequest )				// This is how you end a session (CALL) request
		{
			bSession = TRUE;
			bSessionRequest = FALSE;
		}

		outputDebug(_T("doSessionStartStop: Can not doSessionStartStop, not logged on"));
		return;
	}

	if( m_TalkMode == TALK_SELECTED && 
		m_sessionSocket && 
		m_pSelectedItem &&
		m_pSelectedItem->socket != m_sessionSocket &&
		( (m_pSelectedItem->iCom.status==STATUS_COMMAND) ) )
	{
		bTalkRequest = FALSE;

		MessageBox(_T(getResourceString(IDS_STRING119)), _T(getResourceString(IDS_STRING116)));

		clearTestMode();

		return;
	}

	if( !m_pSelectedItem )
	{
		bSessionRequest = FALSE;

		MessageBox(_T(getResourceString(IDS_STRING_SELECT_ICOM)), _T(getResourceString(IDS_STRING_INFORMATION)));

		bSession = TRUE;
		bSessionRequest = FALSE;

		return;
	}

	if( !m_sessionSocket )
	{
		m_sessionSocket = m_pSelectedItem->socket;			// Start a session with this intercom

		outputDebug(_T("doSessionStartStop: New Session Socket=%d"), m_sessionSocket);

		if( !selectSocket(m_sessionSocket) )
		{
			if( m_pSelectedItem )								// Could have been cleared during the selectSocket
				m_pSelectedItem->bSelected = FALSE;				// Track whether this item is selected

			outputDebug(_T("doSessionStartStop: selectSocket(%d) failed, item=0x%x, sessionSocket set to 0"),
				m_sessionSocket, m_pSelectedItem);

			index = findIntercom(&m_pSelectedItem->iCom);		// Find what row it is on the listbox

			if( index > -1 )
				updateListIcomsStatus(&m_pSelectedItem->iCom, index);

			bSessionRequest = FALSE;
			bSession = FALSE;
			m_sessionSocket = 0;

			return;
		}

		SetTimer(TIMER_RING, 20, NULL);
		doRing(TRUE);

		m_DARingIntercom(m_hDA, m_sessionSocket, TRUE);

		lockControls(FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE);		// LOCK Talk, Listen, Chime, PlayFile, Radio Buttons

//		bSession = TRUE;										// We are starting a session
	}
	else					// Waiting for session to begin
	{
		if( bSessionRequest == FALSE )
		{
			KillTimer(TIMER_RING);

			m_DARingIntercom(m_hDA, m_sessionSocket, FALSE);

			if( m_pSelectedItem )
				m_pSelectedItem->bSelected = FALSE;		// Not selected anymore

			deselectSocket( m_sessionSocket );

			m_sessionSocket = 0;

			lockControls(FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE);		// LOCK Talk, Listen, Chime, PlayFile, Radio Buttons

			bSession = FALSE;
		}

		if( (m_pSelectedItem->iCom.status & STATUS_ONHOOK) == 0 )
		{
			outputDebug("doSessionStartStop: Starting regular session from Handset");

			m_DARingIntercom(m_hDA, m_sessionSocket, FALSE);

//			outputDebug("doSessionStartStop: Show Talk");

			m_btnTalkSessionStart.ShowWindow( SW_HIDE );
			m_btnTalkSessionEnd.ShowWindow( SW_HIDE );

			m_btnTalk.ShowWindow( SW_SHOW );
			m_btnListen.ShowWindow( SW_SHOW );

			KillTimer(TIMER_RING);

			bSessionRequest = FALSE;
			bSession = FALSE;
			bListenRequest = TRUE;
		}
	}
}

const unsigned char ringBuffer[160] = {	0xff, 0x93, 0xff, 0x13,	0xff, 0x93, 0xff, 0x13,	0xff, 0x93, 0xff, 0x13, 0xff, 0x93, 0xff, 0x13,
										0xff, 0x93, 0xff, 0x13,	0xff, 0x93, 0xff, 0x13,	0xff, 0x93, 0xff, 0x13, 0xff, 0x93, 0xff, 0x13,
										0xff, 0x93, 0xff, 0x13,	0xff, 0x93, 0xff, 0x13,	0xff, 0x93, 0xff, 0x13, 0xff, 0x93, 0xff, 0x13,
										0xff, 0x93, 0xff, 0x13,	0xff, 0x93, 0xff, 0x13,	0xff, 0x93, 0xff, 0x13, 0xff, 0x93, 0xff, 0x13,
										0xff, 0x93, 0xff, 0x13,	0xff, 0x93, 0xff, 0x13,	0xff, 0x93, 0xff, 0x13, 0xff, 0x93, 0xff, 0x13,
										0xff, 0x93, 0xff, 0x13,	0xff, 0x93, 0xff, 0x13,	0xff, 0x93, 0xff, 0x13, 0xff, 0x93, 0xff, 0x13,
										0xff, 0x93, 0xff, 0x13,	0xff, 0x93, 0xff, 0x13,	0xff, 0x93, 0xff, 0x13, 0xff, 0x93, 0xff, 0x13,
										0xff, 0x93, 0xff, 0x13,	0xff, 0x93, 0xff, 0x13,	0xff, 0x93, 0xff, 0x13, 0xff, 0x93, 0xff, 0x13,
										0xff, 0x93, 0xff, 0x13,	0xff, 0x93, 0xff, 0x13,	0xff, 0x93, 0xff, 0x13, 0xff, 0x93, 0xff, 0x13,
										0xff, 0x93, 0xff, 0x13,	0xff, 0x93, 0xff, 0x13,	0xff, 0x93, 0xff, 0x13, 0xff, 0x93, 0xff, 0x13 };

const unsigned char silenceBuffer[160] = {	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
											0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
											0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
											0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
											0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
											0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
											0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
											0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
											0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
											0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

void CTalkMasterConsoleDlg::doRing(BOOL bStart)
{
	static int cadence=0;
	struct _AudioData ad;

	if( bStart )
		cadence = 0;

	if( cadence > 2000 )
		ad.buffer = (char*)silenceBuffer;
	else
		ad.buffer = (char*)ringBuffer;

	ad.audioFormat = WAVE_FORMAT_ULAW;
	ad.len = 160;

	audioPlayer.Write((PUCHAR)ad.buffer, ad.len, ad.audioFormat);

	cadence += 50;				// Add 20ms to this

	if( cadence >= 6000 )		// repeat pattern
		cadence = 0;

//	if( m_pSelectedItem )
//		doAudioData( m_pSelectedItem->socket, &ad );
}