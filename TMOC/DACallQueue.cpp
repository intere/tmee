//
// DACallQueue.cpp
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

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

void CTalkMasterConsoleDlg::initCallQueue()
{
	struct _iComQueueList *listEntry = (struct _iComQueueList *)calloc(1, sizeof(struct _iComQueueList));

	strcpy( listEntry->queueMembership.queueKey, "{00000000-0000-0000-0000-000000000000}" );

	listEntry->next = m_pIcomQueueList;
	m_pIcomQueueList = listEntry;
}

BOOL CTalkMasterConsoleDlg::newCallQueueMembership(struct _iComQueueInfo *pQueueMembership)
{
	struct _iComQueueList *listEntry = findQueueMembership( pQueueMembership );
		
	if( !listEntry )
	{
		listEntry = (struct _iComQueueList *)calloc(1, sizeof(struct _iComQueueList));
		listEntry->next = m_pIcomQueueList;
		m_pIcomQueueList = listEntry;
	}

	memcpy( &listEntry->queueMembership, pQueueMembership, sizeof(struct _iComQueueInfo) );

	if( pQueueMembership->membership )
		outputDebug(_T("New Queue membership in '%s' and is %s"), 
			pQueueMembership->queueName,
			listEntry->queueMembership.bOverflow?"Overflowing":"Underflowing");

	return(TRUE);
}

BOOL CTalkMasterConsoleDlg::updateCallQueueStatus(struct _iComQueueInfo *pQueueMembership)
{
	struct _iComQueueList *listEntry = findQueueMembership( pQueueMembership );
	BOOL	wasForced = FALSE;

	if( !listEntry )
		return(FALSE);

	if( listEntry->queueMembership.bOverflow == 2 )
		wasForced = TRUE;

	listEntry->queueMembership.bOverflow = pQueueMembership->bOverflow;

	if( listEntry->queueMembership.membership || wasForced || pQueueMembership->bOverflow==2 )
	{
		outputDebug(_T("Update Queue membership in %s and is %s"), 
			listEntry->queueMembership.queueName, 
			listEntry->queueMembership.bOverflow?"Overflowing":"Underflowing");

		if( testIcomList() )
			redisplayIcomList();						// Redisplay the m_listIcoms depending on the intercom
	}

	return(TRUE);
}

struct _iComQueueList *CTalkMasterConsoleDlg::findQueueMembership(struct _iComQueueInfo *pQueueMembership)
{
	struct _iComQueueList *list = m_pIcomQueueList;

	if( pQueueMembership->queueKey[0] == 0 )			// Empty queueKey?  Must be unassigned
		strcpy(pQueueMembership->queueKey, "{00000000-0000-0000-0000-000000000000}");

	while(list)
	{
		if( !strcmp( list->queueMembership.queueKey, pQueueMembership->queueKey ) )
			break;

		list = list->next;
	}

	return(list);
}

BOOL CTalkMasterConsoleDlg::StartLocalCQSession(int m_sessionSocket, BOOL bHold)
{
	struct _itemData *tItemData;
	int index, image=3;

	if( bHold )
		image=4;

// Find out if this socket is even in the CQ list

	index = findCQIndexFromSocket(m_sessionSocket);
	if( index == -1 )
		return(FALSE);

	tItemData = findItemData(m_sessionSocket);

// First, mark the record as being locally owned
	tItemData->bLocalRecord = TRUE;

// Second, change the list to indicate it being locally owned
	m_listCallsWaiting.SetItem( index, 0, LVIF_IMAGE, 0, image, 0, 0, NULL );	// LVIS_SELECTED
//	outputDebug("StartLocalCQSession: setItem # %d to %d", index, image);
//	m_listCallsWaiting.SetItemText(index, CQ_COL_STATUS, (bHold)?"Hold":"local");

	tItemData->bXfer = TRUE;

// Third, ask for any waiting audio to be sent to the Console
	m_DATransferAudio( m_hDA, m_sessionSocket );					// Will call and wait for the transfer

// Fourth delete it from the server so no one else gets it

//	if(m_uFlags & DA_REMOTE)
//		m_DADeleteServerCQ( m_hDA, m_sessionSocket );

	return(TRUE);
}

void CTalkMasterConsoleDlg::addCallQueue(int socket, struct _CQData *cq)
{
	struct _iComQueueList *listEntry;
	struct _itemData *tItemData, *tCQItemData, *tItemDataList;
	struct tm	atime;
	int index, cqImage;
	char buf[20];

// 1st need to get the item data for this socket from the object list that is always around

	tItemDataList = findItemData(socket);
	if( !tItemDataList )
		return;				// crap - should never happen.  This would be BAD

	strcpy( tItemDataList->cq.name, cq->name );
	strcpy( tItemDataList->cq.message, cq->message );
	tItemDataList->cq.timeLength = cq->timeLength;
	tItemDataList->cq.time = cq->time;

	listEntry = findQueueMembership( &tItemDataList->iComQueue );		// See if we need to priority override this

// Now we need to see if it is displayed - so get it from the following call

	tItemData = getItemDataFromSocket(socket);							// See if it is being displayed
	if( !tItemData )
	{
		if( tItemDataList->iComQueue.priority && listEntry && listEntry->queueMembership.bPriorityOverflow )
			redisplayIcomList();

		return;				// If it is not being displayed in the icomList, then don't display it
	}

// Set timer information if not already set for the announcement stuff
	if( !m_announceStartTime )
		SetTimer( TIMER_ANNOUNCE, 1000, NULL );				// Every second check

	if( !m_announceStartTime || (cq->time-m_serverTimeDelta) < m_announceStartTime )		// Oldest time please
		m_announceStartTime = cq->time - m_serverTimeDelta;	// Convert the server time to the local time for comparison

// Now see if there is already a call queue record
	tCQItemData = getCQItemDataFromSocket(socket);
	if( !tCQItemData )									// No call queue record
	{
		if( resetCQList )
		{
			clearCallsWaitingItemData();
			m_listCallsWaiting.DeleteAllItems();
			resetCQList = FALSE;
		}

		if( m_sessionSocket == 0 && m_selectedCQRow != -1 )
		{
			m_listCallsWaiting.SetItemState(m_selectedCQRow, 0, LVIS_SELECTED|LVIS_FOCUSED);
			m_selectedCQRow = -1;
		}

		index = getCQInsert(tItemData);

		m_listCallsWaiting.InsertItem(index, "");
		m_listCallsWaiting.SetItemText(index, CQ_COL_WAITING, (LPCTSTR)cq->name);
		m_listCallsWaiting.SetItemText(index,CQ_COL_DETAILS, cq->message);
		atime = *localtime( &cq->timeLength );
		sprintf(buf, "%d:%02d", atime.tm_min, atime.tm_sec);
		m_listCallsWaiting.SetItemText(index,CQ_COL_TIME, buf);

		m_listCallsWaiting.SetItemData(index, (DWORD_PTR)tItemData);
		insertCallsWaitingItemData(index, (void*)tItemData);

		if(tItemData->iComQueue.priority)
			cqImage = 2;
		else if( listEntry && listEntry->queueMembership.bOverflow )
			cqImage = 1;
		else
			cqImage = 0;

		m_listCallsWaiting.SetItem( index, 0, LVIF_IMAGE, 0, cqImage, 0, 0, NULL );	// LVIS_SELECTED

//		outputDebug("addCallQueue: setItem # %d to %d", index, cqImage);
//		m_listCallsWaiting.RedrawItems(index, index);
	}
	else
	{
		atime = *localtime( &cq->timeLength );
		sprintf(buf, "%d:%02d", atime.tm_min, atime.tm_sec);

		index = findCQIndexFromSocket( socket );
		m_listCallsWaiting.SetItemText(index,CQ_COL_TIME, buf);
	}

	if( m_sessionSocket == 0 )
		selectIcomRow( selectCQRow(0, TRUE) );
	else
		m_selectedCQRow = findCQIndexFromSocket( m_sessionSocket );
}

void CTalkMasterConsoleDlg::updateCallQueue(int socket, struct _CQData *cq)
{
	struct _itemData *tItemData, *tCQItemData;
	struct _iComQueueList *listEntry;
	struct tm	atime;
	int index, cqImage;
	char buf[20];

// 1st need to get the item data for this socket from the object list that is always around

	tItemData = findItemData(socket);
	if( !tItemData )
		return;				// crap - should never happen.  This would be BAD

	tItemData->cq.timeLength = cq->timeLength;

	tItemData = getItemDataFromSocket(socket);				// Is it being displayed?
	if( !tItemData )
		return;				// If this happens it means it is not being displayed

	tCQItemData = getCQItemDataFromSocket(socket);
	if( !tCQItemData )						// No call queue record
	{
		if( resetCQList )
		{
			clearCallsWaitingItemData();
			m_listCallsWaiting.DeleteAllItems();
			resetCQList = FALSE;
		}

		index = getCQInsert(tItemData);

		if( !m_announceStartTime )
		{
			stopAnnounce();
		}

		m_listCallsWaiting.InsertItem(index, "");
		m_listCallsWaiting.SetItemText(index, CQ_COL_WAITING, (LPCTSTR)cq->name);
		m_listCallsWaiting.SetItemText(index, CQ_COL_DETAILS, cq->message);
		m_listCallsWaiting.SetItemText(index, CQ_COL_TIME, "0:00");

		m_listCallsWaiting.SetItemData(index, (DWORD_PTR)tItemData);
		insertCallsWaitingItemData(index, (void*)tItemData);

		listEntry = findQueueMembership( &tItemData->iComQueue );
		if(tItemData->iComQueue.priority)
			cqImage = 2;
		else if( listEntry && listEntry->queueMembership.bOverflow )
			cqImage = 1;
		else
			cqImage = 0;

		m_listCallsWaiting.SetItem( index, 0, LVIF_IMAGE, 0, cqImage, 0, 0, NULL );	// LVIS_SELECTED
//		outputDebug("updateCallQueue: setItem # %d to %d", index, cqImage);
	}
	else
	{
		atime = *localtime( &cq->timeLength );
		sprintf(buf, "%d:%02d", atime.tm_min, atime.tm_sec);

		index = findCQIndexFromSocket( socket );
		m_listCallsWaiting.SetItemText(index,CQ_COL_TIME, buf);
	}
}

void CTalkMasterConsoleDlg::deleteAllCallQueue(BOOL bForceDelete)
{
	struct _itemData *tItemData = m_pItemData;
	int index;

//	m_listCallsWaiting.DeleteAllItems();

	while( tItemData )
	{
		if( !tItemData->bLocalRecord || bForceDelete )
		{
//			tItemData->cq.time = 0;							// May only be redisplaying, don't reset

			if( tItemData->hCallQueue )
				audioArchive.Delete(tItemData->hCallQueue, TRUE);

			tItemData->hCallQueue = NULL;

			index = findCQIndexFromSocket( tItemData->socket );
			if( index > -1 )
			{
				m_listCallsWaiting.DeleteItem( index );
				deleteCallsWaitingItemData( index );
			}
		}

		tItemData = tItemData->next;
	}

	if( m_listCallsWaiting.GetItemCount() == 0 )
	{
		if( !m_announceStartTime )
			stopAnnounce();

		m_listCallsWaiting.InsertItem(0, "");
		m_listCallsWaiting.SetItemText(0, CQ_COL_WAITING, getResourceString(IDS_STRING_NO_MESSAGES_WAITING));
		m_listCallsWaiting.SetItem( 0, 0, LVIF_IMAGE, 0, 0, 0, 0, NULL );	// LVIS_SELECTED
		m_listCallsWaiting.SetItemData(0, NULL);
		insertCallsWaitingItemData(0, NULL);
//		m_listCallsWaiting.RedrawItems(0,0);

		if( !m_sessionSocket || 
			( m_pSelectedItem && m_pSelectedItem->socket != m_sessionSocket ) )
			m_pSelectedItem = NULL;

		resetCQList = TRUE;
		m_announceGuardTime = 0;					// Clear the guard time if nothing in CQ
	}
}

void CTalkMasterConsoleDlg::deleteCallQueue(int socket, BOOL bForceDelete, BOOL bReselectListBoxes)
{
	struct _itemData *tItemData;
	int index;

	tItemData = findItemData(socket);
	if( !tItemData )
		return;				// crap - should never happen.  This would be BAD

	if( tItemData->bLocalRecord && !bForceDelete)
		return;

	if(tItemData)
	{
		tItemData->cq.time = 0;				// Clear the record so it does not pop up until it is re-added.
		tItemData->bLocalRecord = FALSE;	// Not local anymore

		if( tItemData->hCallQueue )
			audioArchive.Delete(tItemData->hCallQueue, TRUE);

		tItemData->hCallQueue = NULL;
	}

	index = findCQIndexFromSocket(socket);

	if( index != -1 )
	{
		if( index == m_selectedCQRow )
		{
			m_selectedCQRow = -1;
			m_pSelectedItem = NULL;
			m_selectedRow = -1;
			m_listIcoms.SetItemState(m_selectedRow, 0, LVIS_SELECTED|LVIS_FOCUSED);
			m_listIcoms.EnsureVisible(m_selectedRow, FALSE);
		}

		m_listCallsWaiting.DeleteItem(index);
		deleteCallsWaitingItemData(index);
	}

	if( m_listCallsWaiting.GetItemCount() == 0 )
	{
		if( !m_announceStartTime )
			stopAnnounce();
		
		m_listCallsWaiting.InsertItem(0, "");
		m_listCallsWaiting.SetItemText(0, CQ_COL_WAITING, getResourceString(IDS_STRING106));
		m_listCallsWaiting.SetItem( 0, 0, LVIF_IMAGE, 0, 0, 0, 0, NULL );	// LVIS_SELECTED
		m_listCallsWaiting.SetItemData(0, NULL);
		insertCallsWaitingItemData(0, NULL);
//			m_listCallsWaiting.RedrawItems(0,0);

		if( !m_sessionSocket || 
			( m_pSelectedItem && m_pSelectedItem->socket != m_sessionSocket ) )
		{
			m_pSelectedItem = NULL;
			SetHoldEnd(FALSE, FALSE);
		}

//		m_pSelectedItem = NULL;						// No item yet selected
		resetCQList = TRUE;
		m_announceGuardTime = 0;					// Clear the guard time if nothing in CQ
	}
	else if (!m_sessionSocket && bReselectListBoxes)						// Not currently in a session
	{
		selectIcomRow( selectCQRow(0, TRUE) );		// Select the 1st intercom in both the CQ and the Icom list
	}

	if( testIcomList() )							// See if we need to redisplay the intercom list
		redisplayIcomList();						// Redisplay the m_listIcoms depending on the intercom
}

void CTalkMasterConsoleDlg::doCallQueueAudio(int socket, struct _AudioData *ad)
{
	struct _itemData *tCQItemData;
	HANDLE hRec;

	tCQItemData = getCQItemDataFromSocket(socket);

	outputDebug(_T("doCallQueueAudio: getRecordingHandle(%d, %s, %d)"), socket, "temp", TRUE);

	hRec = audioArchive.getRecordingHandle( socket, "temp", TRUE, ad->audioFormat );		// Get or make a recording handle
	audioArchive.Write( hRec, ad->buffer, ad->len );
	tCQItemData->hTempCQ = hRec;

//	outputDebug("CQ Audio Transfer %d bytes", ad->len);
}

void CTalkMasterConsoleDlg::doCallQueueAudioDone(int socket)
{
	struct _itemData *tCQItemData;
	char buf[64];
	int totalBytes;
	struct tm	atime;
	int	index;

	tCQItemData = getCQItemDataFromSocket(socket);

	if( tCQItemData )
	{
		if ( !tCQItemData->hCallQueue )									// Not Already Opened
		{
			outputDebug(_T("doCallQueueAudioDone: getRecordingHandle(%d, %s, %d)"), socket, _T("in"), TRUE);
			tCQItemData->hCallQueue= audioArchive.getRecordingHandle( socket, _T("in"), TRUE, WAVE_FORMAT_PCM );		// Get or make a recording handle
			audioArchive.Close(tCQItemData->hCallQueue);							// Close it so we can open it
		}

		if ( tCQItemData->hTempCQ )
		{
			audioArchive.Merge( tCQItemData->hTempCQ, tCQItemData->hCallQueue );	// Append the 2nd file onto the 1st
			audioArchive.Delete( tCQItemData->hCallQueue, TRUE );					// Delete the 2nd file
			audioArchive.Rename( tCQItemData->hTempCQ, "in" );						// Rename the file

			tCQItemData->hCallQueue = tCQItemData->hTempCQ;							// The hCallQueue got released by the merge
			tCQItemData->hTempCQ = NULL;

			audioArchive.Close(tCQItemData->hCallQueue);							// Close it so we can open it
		}
		totalBytes = audioArchive.Open(tCQItemData->hCallQueue);				// Reopen for playback

		outputDebug(_T("CQ Audio Transfer Done with %d Bytes"), totalBytes);

		if(totalBytes)
		{
			m_sliderCQPlayer.SetPos(0);				// On the left to start
			m_sliderCQPlayer.SetRange(0, totalBytes/8000);
			activatePlayer(TRUE);
		}
		else
			activatePlayer(FALSE);

		if(m_uFlags & DA_REMOTE)
			m_DADeleteServerCQ( m_hDA, socket );

		if( m_sessionSocket == socket && !bTalking && !bListening)
			SetHoldEnd(FALSE, TRUE);				// Only reset if it is the current socket

		atime = *localtime( &tCQItemData->cq.timeLength );
		sprintf(buf, "%d:%02d", atime.tm_min, atime.tm_sec);

		index = findCQIndexFromSocket( socket );
		if( index >= 0 )
			m_listCallsWaiting.SetItemText(index,CQ_COL_TIME, buf);

		tCQItemData->bXfer = FALSE;
	}
	else
	{
		if(m_uFlags & DA_REMOTE)
			m_DADeleteServerCQ( m_hDA, socket );

		if( m_sessionSocket == socket && !bTalking && !bListening)
			SetHoldEnd(FALSE, TRUE);				// Only reset if it is the current socket
	}
}

BOOL CTalkMasterConsoleDlg::isCallQueue(int socket)
{
	return(getCQItemDataFromSocket(socket)!= NULL);
}

void CTalkMasterConsoleDlg::MarkCallQueueSession(int socket, BOOL bHold)
{
	int index, image=3;
	struct _itemData *tCQItemData;

	if( bHold )
		image=4;

	index = findCQIndexFromSocket(socket);
	if( index == -1 )
		return;

	tCQItemData = getCQItemDataFromSocket(socket);

// First, mark the record as being locally owned
	tCQItemData->bLocalRecord = TRUE;

// Second, change the list to indicate it being locally owned
	m_listCallsWaiting.SetItem( index, 0, LVIF_IMAGE, 0, image, 0, 0, NULL );	// LVIS_SELECTED
//	outputDebug("MarkCallQueueSession: setItem # %d to %d", index, image);
//	m_listCallsWaiting.SetItemText(index, CQ_COL_STATUS, (bHold)?"Hold":"local");
}

int CTalkMasterConsoleDlg::getCQInsert(struct _itemData *item)
{
	int i, count;
	struct _itemData *tCQItemData;

	count = m_listCallsWaiting.GetItemCount();

	EnterCriticalSection( &CriticalSectionCallQueue );

	for(i=0;i<count;i++)
	{
		tCQItemData = (struct _itemData *)getCallsWaitingItemData(i);

		if( item->iComQueue.priority > tCQItemData->iComQueue.priority)
			break;										// Insert before since higher priority

		if( item->iComQueue.priority == tCQItemData->iComQueue.priority )
		{
			if( item->cq.time < tCQItemData->cq.time )
				break;
		}
	}

	LeaveCriticalSection( &CriticalSectionCallQueue );

	return(i);
}

int CTalkMasterConsoleDlg::selectCQRow(int row, BOOL bSetItemState)
{
	int	icomRow=-1;
	struct _itemData *tItemData;

	if( row != m_selectedCQRow && m_selectedCQRow != -1 )		// Is it a different row than we have selected?
	{
		m_listCallsWaiting.SetItemState(m_selectedCQRow, 0, LVIS_SELECTED|LVIS_FOCUSED);
		m_selectedCQRow = -1;
	}

	if( m_sessionSocket )					// Already in a session?
	{
		try
		{
			if( !m_pCallWaitingItem && ((row>-1) && (tItemData = getCallsWaitingData(row)) && (tItemData->socket != m_sessionSocket) ) )
			{
				if( m_pSelectedItem && !theApp.UserOptions.forceHdx &&
					m_pSelectedItem->iCom.picRev.feature1 & OPNIC_FULL_DUPLEX )
				{
					if( bTalking )
						doFdxStartStop();				// Stop it
				}
				else
				{
					if( bTalking )						// Stop it!
					{
						bTalkRequest = FALSE;
						while(bTalking)
						{
							bTalkRequest = FALSE;
							bListenRequest = FALSE;

							if( bSessionRequest )
							{
								bSession = TRUE;
								bSessionRequest = FALSE;
							}

							DoEvents();
						}
					}
	//					doTalk();						// Probably should just set the flag that we want to not be talking
	//					endTalk();
					if( bListening )
					{
						bListenRequest = FALSE;
						while(bListening)
						{
							bTalkRequest = FALSE;
							bListenRequest = FALSE;
							if( bSessionRequest )
							{
								bSession = TRUE;
								bSessionRequest = FALSE;
							}
							DoEvents();
						}
					}

					if( bSessionRequest )						// Trying to start a sessoin
					{
						bSession = TRUE;
						while(bSession)
						{
							bTalkRequest = FALSE;
							bListenRequest = FALSE;
							bSessionRequest = FALSE;
							DoEvents();
						}
					}
				}
//					doListen();						// Probably should just set the flag that we want to not be listening
//					endListen();

				outputDebug("selectCQRow: Clearing session socket from %d to 0", m_sessionSocket);
				m_sessionSocket = 0;				// Not talking or listening to it anymore, so we need to clear it
			}
		}
		catch(...)
		{
		}
	}

	if( m_pCallWaitingItem )							// Is one already selected?
	{
		if( m_pCallWaitingItem->hCallQueue )			// If we have an archive open
			audioArchive.Close(m_pCallWaitingItem->hCallQueue);

//		m_pCallWaitingItem->hCallQueue = NULL;			// Should we be clearing this out???

		m_pCallWaitingItem = NULL;						// No more item
	}

	if( (row > -1) &&
		(m_pCallWaitingItem = (struct _itemData *)getCallsWaitingItemData(row)) &&
		(m_pCallWaitingItem->hCallQueue) &&
		(m_pCallWaitingItem->bSelected) )
	{													// If the selected item has a local call queue item available
		int totalSize = audioArchive.Open(m_pCallWaitingItem->hCallQueue);

		m_sliderCQPlayer.SetPos(0);				// On the left to start
		m_sliderCQPlayer.SetRange(0, totalSize/8000);

		m_sessionSocket = m_pCallWaitingItem->socket;	// jdnjdnjdn

		activatePlayer(TRUE);
		SetHoldEnd(FALSE, TRUE);
	}
	else
	{
//		m_sessionSocket = 0;							// jdnjdnjdn This was clearing a listen session when an overflow came in
		SetHoldEnd((m_pCallWaitingItem!=NULL), FALSE);
	}


	if (m_pCallWaitingItem)			// Is there a real item selected?
	{
//		m_sessionSocket = m_pCallWaitingItem->socket;						// Trying to give this entry a session #

		icomRow = findIndexFromSocket(m_pCallWaitingItem->socket);			// The selected icom #

//		if( m_selectedRow != selectedIcomRow && m_selectedRow > -1 )
//			m_listIcoms.SetItemState( m_selectedRow, 0, LVIS_SELECTED );
//
//		m_pSelectedItem = m_pCallWaitingItem;

//		if( selectedIcomRow > -1 )
//		{
//			m_listIcoms.SetItemState(selectedIcomRow, LVIS_SELECTED, LVIS_SELECTED);
//			m_selectedRow = selectedIcomRow;
//		}

		m_selectedCQRow = row;

		if( bSetItemState )
		{
			m_listCallsWaiting.SetItemState(row, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
			m_listCallsWaiting.EnsureVisible(row, FALSE);
		}

//		m_btnSessionHold.EnableWindow(TRUE);
	}
	else
	{
		activatePlayer(FALSE);
//		m_btnSessionHold.EnableWindow(FALSE);

		m_listCallsWaiting.SetItemState(row, 0, LVIS_SELECTED|LVIS_FOCUSED);
		m_selectedCQRow = -1;

//		m_sessionSocket = 0;					// jdnjdnjdn this was clearing a listen session when an overflow came in
//		m_pSelectedItem = NULL;
	}

	return(icomRow);
}

time_t CTalkMasterConsoleDlg::oldestCQEntry(int *row, time_t *time)
{
	struct _itemData *tCQItemData;
	int i, count, oldrow = -1;
	time_t	oldest=0;

	count = m_listCallsWaiting.GetItemCount();

	EnterCriticalSection( &CriticalSectionCallQueue );

	for(i=0;i<count;i++)
	{
		tCQItemData = (struct _itemData *)getCallsWaitingItemData(i);
		if( tCQItemData )
		{
			if( !oldest || oldest > tCQItemData->cq.time )
			{
				oldest = tCQItemData->cq.time;
				oldrow = i;
			}
		}
	}

	LeaveCriticalSection( &CriticalSectionCallQueue );

	if( time )
		*time = oldest;
	if( row )
		*row = oldrow;

	return(oldest);
}

struct _itemData *CTalkMasterConsoleDlg::getCallsWaitingData(int row)
{
	return((struct _itemData *)getCallsWaitingItemData(row));
}