//
// DAGroups.cpp
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


void CTalkMasterConsoleDlg::OnTcnSelchangeTabMain(NMHDR *pNMHDR, LRESULT *pResult)
{
	if( m_tabMain.GetCurSel() == 1 )
	{
		if( m_sessionSocket )
			m_bHeldSessionSocket = TRUE;

		m_holdSessionSocket = m_sessionSocket;

		shutDownIcomAndHold();
	}

	displayTabMain();

	*pResult = 0;
}

void CTalkMasterConsoleDlg::displayTabMain()
{
	int selectedItem;
	POSITION selectedPos;

	if( m_tabMain.GetCurSel() == 0 )
	{
		m_btnSelected.EnableWindow(TRUE);
		m_btnListen.EnableWindow(TRUE);
		m_btnAllActive.EnableWindow(TRUE);

		m_btnManual.EnableWindow(TRUE);
		m_btnAutomatic.EnableWindow(TRUE);

		showHdxFdxControls();

		switch( m_mainMode )
		{
		case TALK_SELECTED:
			m_btnSelected.SetCheck(BST_CHECKED);
			m_btnGroup.SetCheck(BST_UNCHECKED);
			m_btnAllActive.SetCheck(BST_UNCHECKED);

			OnBnClickedRadioSelected();
#if 0
			switch( m_ListenModeHold )
			{
			case LISTEN_MANUAL:
				m_btnManual.SetCheck(BST_CHECKED);
				m_btnAutomatic.SetCheck(BST_UNCHECKED);

				m_ListenMode = m_ListenModeHold;
				m_ListenModeHold = -1;
				break;

			case LISTEN_AUTO:
				m_btnManual.SetCheck(BST_UNCHECKED);
				m_btnAutomatic.SetCheck(BST_CHECKED);

				m_ListenMode = m_ListenModeHold;
				m_ListenModeHold = -1;
				break;
			}
#endif
			break;
		case TALK_GROUP:
#if 0				// Should already be this way from the other tab
			m_btnSelected.SetCheck(BST_UNCHECKED);
			m_btnGroup.SetCheck(BST_CHECKED);
			m_btnAllActive.SetCheck(BST_UNCHECKED);

			OnBnClickedRadioGroup();
#endif
			break;
		case TALK_ALL_ACTIVE:
			m_btnSelected.SetCheck(BST_UNCHECKED);
			m_btnGroup.SetCheck(BST_UNCHECKED);
			m_btnAllActive.SetCheck(BST_CHECKED);

			OnBnClickedRadioAllActive();
			break;
		}

		resetHoldEnd();
	}
	else
	{
		m_btnTalk.ShowWindow( SW_SHOW );
		m_btnListen.ShowWindow( SW_SHOW );

		m_btnNoListen.ShowWindow( SW_HIDE );
		m_btnFdxMute.ShowWindow( SW_HIDE );
		m_btnFdxStart.ShowWindow( SW_HIDE );

		m_btnSelected.EnableWindow(FALSE);
		m_btnListen.EnableWindow(FALSE);
		m_btnAllActive.EnableWindow(FALSE);

		m_btnManual.EnableWindow(FALSE);
		m_btnAutomatic.EnableWindow(FALSE);

		m_btnSessionHold.EnableWindow(FALSE);
		m_btnSessionEnd.EnableWindow(FALSE);

		m_btnSelected.SetCheck(BST_UNCHECKED);
		m_btnGroup.SetCheck(BST_CHECKED);
		m_btnAllActive.SetCheck(BST_UNCHECKED);

		m_btnManual.SetCheck(BST_CHECKED);
		m_btnAutomatic.SetCheck(BST_UNCHECKED);

		m_mainMode = m_TalkMode;

		if( m_mainMode == TALK_SELECTED )
			m_ListenModeHold = m_ListenMode;

		m_ListenMode = LISTEN_MANUAL;
		m_TalkMode = TALK_GROUP;

// Clear all of the selected items in the calls waiting listbox when this tab is selected

		if( m_listCallsWaiting.GetSelectedCount() > 0 )
		{
			selectedPos = m_listCallsWaiting.GetFirstSelectedItemPosition();

			while(selectedPos)
			{
				selectedItem = m_listCallsWaiting.GetNextSelectedItem(selectedPos);
				m_listCallsWaiting.SetItemState(selectedItem, 0, LVIS_SELECTED|LVIS_FOCUSED);
			}
		}
	}

	doSize();
}

void CTalkMasterConsoleDlg::doNewGroupInfo(struct _groupData *pGroupData)
{
	struct _groupData *pNewGroup;
	struct _groupDataList *pNewGroupListEntry;
	struct _groupItems *pGroupItem;
	int index;

	pNewGroupListEntry = getGroupEntry( pGroupData->groupKey );			// Let's see if this group is already here so we can clean things up to a new listing

	if( pNewGroupListEntry )
	{
// If the group already exists, we need to free the group members from the list so they can be re-read later because the list is empty
		while( pNewGroupListEntry->pGroupItems )
		{
			pGroupItem = pNewGroupListEntry->pGroupItems->next;

			if( pNewGroupListEntry->pGroupItems->bLocalIcomRecord )	// Groups created this entry, we must free it
				free( pNewGroupListEntry->pGroupItems );

			pNewGroupListEntry->pGroupItems = pGroupItem;
		}

		pNewGroup = pNewGroupListEntry->pGroupData;

		index = findGroupRow(pNewGroup);				// Find old information
		if( index >= 0 )
			m_listGroups.DeleteItem(index);				// Get rid of it
	}
	else
	{
		pNewGroupListEntry = (_groupDataList*)calloc(1, sizeof(_groupDataList));
		pNewGroup = (_groupData *)calloc(1, sizeof(_groupData));

// Link the new entry into the current list

		pNewGroupListEntry->next = m_pGroupData;			// Point it at where the head is pointing
		m_pGroupData = pNewGroupListEntry;

		pNewGroupListEntry->pGroupData = pNewGroup;

		pNewGroupListEntry->type = 0;						// This is a GROUP entry
	}

// Now copy the data around

	memcpy( pNewGroup, pGroupData, sizeof(_groupData) );

// Now add this group if it is going to be displayed

	if( pNewGroup->bGlobalGroup ||
		pNewGroup->bMember )
	{
		index = findGroupInsert(pGroupData);

		m_listGroups.InsertItem(LVIF_TEXT|LVIF_IMAGE, index, _T(""), 0, 0, 10, NULL);
		m_listGroups.SetItem(index, GROUP_COL_ICON, LVIF_IMAGE, 0, GROUP_IMAGE_EMPTY, 0, 0, NULL, 0 );
//		m_listGroups.SetItemText(index, 1, pGroupData->bGlobalGroup?getResourceString(IDS_STRING_GLOBAL_GROUP):"");
		m_listGroups.SetItemText(index, GROUP_COL_NAME, pGroupData->groupName);

		m_listGroups.SetItemData(index, (DWORD_PTR)pNewGroupListEntry);
	}

}

void CTalkMasterConsoleDlg::doNewGroupInfo(struct _itemData *pItemData)
{
	struct _itemData *pNewIcom;
	struct _groupDataList *pNewGroupListEntry;
	int index;

	pNewGroupListEntry = getGroupEntry( &pItemData->iCom.MAC );			// Let's see if this group is already here so we can clean things up to a new listing

	if( pNewGroupListEntry )
	{
		pNewIcom = pNewGroupListEntry->pIcomData;

		index = findGroupRow(pNewIcom);					// Find old information
		if( index >= 0 )
			m_listGroups.DeleteItem(index);				// Get rid of it
	}
	else
	{
		pNewGroupListEntry = (_groupDataList*)calloc(1, sizeof(_groupDataList));
		pNewIcom = (_itemData *)calloc(1, sizeof(_itemData));

// Link the new entry into the current list

		pNewGroupListEntry->next = m_pGroupData;			// Point it at where the head is pointing
		m_pGroupData = pNewGroupListEntry;

		pNewGroupListEntry->pIcomData = pNewIcom;

		pNewGroupListEntry->type = 1;						// This is an INTERCOM entry
	}

// Now copy the data around

	memcpy( pNewIcom, pItemData, sizeof(_itemData) );

// Now add this intercom if it is going to be displayed

	index = findGroupInsert(pItemData);

	m_listGroups.InsertItem(LVIF_TEXT|LVIF_IMAGE, index, _T(""), 0, 0, 10, NULL);
	m_listGroups.SetItem(index, GROUP_COL_ICON, LVIF_IMAGE, 0, GROUP_IMAGE_EMPTY, 0, 0, NULL, 0 );
//	m_listGroups.SetItemText(index, 1, "");
	m_listGroups.SetItemText(index, GROUP_COL_NAME, (LPCTSTR)pItemData->iCom.name);

	m_listGroups.SetItemData(index, (DWORD_PTR)pNewGroupListEntry);
}

void CTalkMasterConsoleDlg::doNewGroupMember(int GroupSocket, struct _MAC *pMac)
{
	struct _groupDataList *pGroupList;
	struct _itemData *pItemData;
	struct _groupItems *pGroupItems;

	pGroupList = getGroupList( GroupSocket );
	pItemData = findItemData( pMac );
	pGroupItems = (_groupItems*)calloc(1, sizeof(_groupItems));

	if( pItemData == NULL )						// MAC was not in our list
	{
		outputDebug("doNewGroupMember: MAC address %02x:%02x:%02x:%02x:%02x:%02x not found",
			pMac->MAC1,
			pMac->MAC2,
			pMac->MAC3,
			pMac->MAC4,
			pMac->MAC5,
			pMac->MAC6);

		pItemData = (_itemData*)calloc(1, sizeof(_itemData));
		memcpy( &pItemData->iCom.MAC, pMac, 6 );

		pGroupItems->bLocalIcomRecord = TRUE;	// Say that we created it so we can free it later
	}

	pGroupItems->pIcomData = pItemData;

	pGroupItems->next = pGroupList->pGroupItems;
	pGroupList->pGroupItems = pGroupItems;
}

void CTalkMasterConsoleDlg::doNewMessageInfo( struct _messageData *pMessageData )
{
	struct _messageDataList *pNewMessageListEntry;
	struct _messageData *pNewMessage;

	outputDebug("doNewMessageInfo: new message received '%s'", pMessageData->title);

	pNewMessageListEntry = (_messageDataList*)calloc(1, sizeof(_messageDataList));
	pNewMessage			 = (_messageData*)calloc(1, sizeof(_messageData));

// Link the new entry into the current list

	pNewMessageListEntry->next = m_pMessageDataList;			// Point it at where the head is pointing
	m_pMessageDataList = pNewMessageListEntry;

	pNewMessageListEntry->pMessageData = pNewMessage;

// Now copy the data around

	memcpy( pNewMessage, pMessageData, sizeof(_messageData) );
}

//
// doMessagePlaying
//
// This function is called when the server reports (during login) that a message is currently playing on the server that was started by this console.
// The message key (GUID) is sent along with the groupID that is playing.
//

void CTalkMasterConsoleDlg::doMessagePlaying( struct _messagePlayingData *pMessagePlayingData )
{
	struct _fileList *newFile;

	m_DADeleteGroup(m_hDA, pMessagePlayingData->groupSocket);					// Kill the group that is probably in the DLL from the last login
	m_DACreateGroup(m_hDA, &pMessagePlayingData->groupSocket, NULL, 0, TRUE);	// Create a temporary group so we can deselect it when it is done

	m_btnPlayFile.SetWindowText(_T(getResourceString(IDS_STRING_STOP_FILE)));
	bPlayFileLocal = FALSE;								// We are playing something on the server

	m_playFileSocket = pMessagePlayingData->groupSocket;

	if( m_playFileSocket )
		m_tabMain.EnableWindow(FALSE);

	m_DAPlayServerAudio( m_hDA, m_playFileSocket, pMessagePlayingData->key, 0 );				// The type of ZERO will cause the audio started flag to be set so we can end audio it

// Now create a _fileList entry for this playing audio file

	newFile = (struct _fileList *)calloc(1, sizeof(struct _fileList));

	newFile->bSelect = TRUE;

	newFile->next = m_fileList.next;
	m_fileList.next = newFile;

	newFile->socket = m_playFileSocket;
}

int CTalkMasterConsoleDlg::findGroupInsert(struct _groupData *pGroup)
{
	int i, count;
	struct _groupDataList *pGroupItemData;

	count = m_listGroups.GetItemCount();

	for(i=0;i<count;i++)
	{
		pGroupItemData = (struct _groupDataList *)m_listGroups.GetItemData(i);

		if( pGroupItemData->type == 0 )
		{
			if( stricmp( pGroup->groupName, pGroupItemData->pGroupData->groupName ) < 0 )
				return(i);
		}
		else
		{
			if( stricmp( pGroup->groupName, (const char *)pGroupItemData->pIcomData->iCom.name ) < 0 )
				return(i);
		}
	}

	return(i);
}

int CTalkMasterConsoleDlg::findGroupInsert(struct _itemData *pIntercom)
{
	int i, count;
	struct _groupDataList *pGroupItemData;

	count = m_listGroups.GetItemCount();

	for(i=0;i<count;i++)
	{
		pGroupItemData = (struct _groupDataList *)m_listGroups.GetItemData(i);

		if( pGroupItemData->type == 0 )
		{
			if( stricmp( (const char *)pIntercom->iCom.name, pGroupItemData->pGroupData->groupName ) < 0 )
				return(i);
		}
		else
		{
			if( stricmp( (const char *)pIntercom->iCom.name, (const char *)pGroupItemData->pIcomData->iCom.name ) < 0 )
				return(i);
		}
	}

	return(i);
}

int CTalkMasterConsoleDlg::findGroupRow(struct _groupData *pGroup)
{
	int i, count;
	struct _groupDataList *pGroupItemData;

	count = m_listGroups.GetItemCount();

	for(i=0;i<count;i++)
	{
		pGroupItemData = (struct _groupDataList *)m_listGroups.GetItemData(i);

		if( stricmp( pGroup->groupName, pGroupItemData->pGroupData->groupName ) == 0 )
			return(i);
	}

	return(-1);
}

int CTalkMasterConsoleDlg::findGroupRow(struct _itemData *pItemData)
{
	int i, count;
	struct _groupDataList *pGroupItemData;

	count = m_listGroups.GetItemCount();

	for(i=0;i<count;i++)
	{
		pGroupItemData = (struct _groupDataList *)m_listGroups.GetItemData(i);

		if( pGroupItemData->pIcomData && memcmp( &pItemData->iCom.MAC, &pGroupItemData->pIcomData->iCom.MAC, 6 ) == 0 )
			return(i);
	}

	return(-1);
}

int CTalkMasterConsoleDlg::findMessageInsert(struct _messageData *pMessageData)
{
	int i, count;
	struct _messageDataList *pMessageItemData;

	count = m_listMessages.GetItemCount();

	for(i=0;i<count;i++)
	{
		pMessageItemData = (struct _messageDataList *)m_listMessages.GetItemData(i);

		if( stricmp( (const char *)pMessageData->title, pMessageItemData->pMessageData->title ) < 0 )
			return(i);
	}

	return(i);
}

int CTalkMasterConsoleDlg::findMessageRow(struct _messageData *pMessageData)
{
	int i, count;
	struct _messageDataList *pMessageItemData;

	count = m_listMessages.GetItemCount();

	for(i=0;i<count;i++)
	{
		pMessageItemData = (struct _messageDataList *)m_listMessages.GetItemData(i);

		if( strcmp( pMessageItemData->pMessageData->key, pMessageData->key ) == 0 )
			return(i);
	}

	return(-1);
}

int CTalkMasterConsoleDlg::getMessageSelectedCount(struct _messageDataList *pMessageData)
{
	struct _messageDataList *pMessageDataPos = m_pMessageDataList;
	int count=0;

	while( pMessageDataPos )
	{
		if( !strcmp( pMessageDataPos->pMessageData->key, pMessageData->pMessageData->key ) && pMessageDataPos->nGroupsDisplayed>0 )
			count++;

		pMessageDataPos = pMessageDataPos->next;
	}

	return(count);
}

struct _messageDataList *CTalkMasterConsoleDlg::getMessageEntry(struct _groupDataList *pGroupData, struct _messageDataList *pMessageData)
{
	struct _messageDataList *pMessageDataPos;

	if( pGroupData && pGroupData->pGroupData )
		outputDebug("getMessageEntry(0x%x,0x%x) Looking for key '%s'", pGroupData, pMessageData, pGroupData->pGroupData->groupKey);
	else
		outputDebug("getMessageEntry(0x%x,0x%x) No key sent to be searched", pGroupData, pMessageData);

	if( pMessageData == NULL )
	{
		pMessageDataPos = m_pMessageDataList;					// Beginning of the list
	}
	else
		pMessageDataPos = pMessageData->next;					// Look at the next in the list

	if( pMessageDataPos )
		outputDebug("getMessageEntry: Starting at key='%s' (0x%x)", pMessageDataPos->pMessageData->groupKey);
	else
		outputDebug("getMessageEntry: Starting at END - no keys");

	while( pMessageDataPos )
	{
		if( pGroupData && pGroupData->pGroupData )
			outputDebug("getMessageEntry: Checking key='%s' against key='%s'", pGroupData->pGroupData->groupKey, pMessageDataPos->pMessageData->groupKey);
		else
			outputDebug("getMessageEntry: No key sent");

		if( pGroupData->pGroupData && !strcmp( pGroupData->pGroupData->groupKey, pMessageDataPos->pMessageData->groupKey ) )
		{
			outputDebug("getMessageEntry: Selecting entry key='%s'", pGroupData->pGroupData->groupKey);
			return( pMessageDataPos );
		}
		pMessageDataPos = pMessageDataPos->next;
	}

	outputDebug("getMessageEntry: No Entry Found");

	return(NULL);
}

void CTalkMasterConsoleDlg::OnNMClickListGroups(NMHDR *pNMHDR, LRESULT *pResult)
{
	int selectedCount=0, i, nItem=-1, count, messageCount=0;
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	struct _messageDataList *pNewMessageListEntry;
	struct _messageDataList *pMessageEntry;
	struct _groupDataList *pGroupEntry;

	if( pNMLV->iSubItem == COL_GROUP )
	{
	}
	else if( (selectedCount = m_listGroups.GetSelectedCount()) )		// Selecting more intercoms for the group list
	{
		outputDebug("OnNMClickListGroups: %d group selected from group list", selectedCount);

		if( selectedCount == 1 )										// Clear all of the other checkboxes if we only have 1 checked
		{
			m_firstGroupSelected = pNMLV->iItem;

			count = m_listGroups.GetItemCount();

			outputDebug("OnNMClickListGroups: Clearing %d Group Checkboxes", count);

			for(i=0;i<count;i++)
				m_listGroups.SetCheck(i, FALSE);
		}
		else if( m_firstGroupSelected >= -1 )
			m_listGroups.SetCheck(m_firstGroupSelected, !m_listGroups.GetCheck(m_firstGroupSelected));

		for(i=0;i<selectedCount;i++)									// Loop through all of the selected items
		{
			nItem = m_listGroups.GetNextItem(nItem, LVNI_SELECTED);		// Get the next one (-1 is set for the 1st one)
			m_listGroups.SetCheck(nItem, !m_listGroups.GetCheck(nItem));			// the selected icom
		}
	}
	else 
		m_firstGroupSelected = -1;

// Now clear the m_listMessages list and fill it with the intersection of the selected groups

	m_listMessages.DeleteAllItems();							// Cleared
	messageCount = 0;											// No messages displayed yet

	count = m_listGroups.GetItemCount();

	outputDebug("OnNMClickListGroups: Searching %d groups for selected items", count);

	selectedCount = 0;

	for(i=0;i<count;i++)
	{
		if( (pNMLV->iItem == i && pNMLV->iSubItem == COL_GROUP && !m_listGroups.GetCheck(i)) ||
			((pNMLV->iItem != i || pNMLV->iSubItem != COL_GROUP) && m_listGroups.GetCheck(i)) )
		{
			outputDebug("OnNMClickListGroups: Group %d Selected", i);

			selectedCount++;
		}
	}

	outputDebug("OnNMClickListGroups: Selected %d Groups", selectedCount);

	if( selectedCount )											// If some groups were selected, let's figure out the intersection
	{
// Now set all of the messages to not displayed

		pNewMessageListEntry = m_pMessageDataList;					// Point to the top of the message list
		while(pNewMessageListEntry)
		{
			pNewMessageListEntry->nGroupsDisplayed = 0;				// ZERO groups have displayed this messages
			pNewMessageListEntry = pNewMessageListEntry->next;		// Loop through all of them
		}

		for(i=0;i<count;i++)						// Loop through all of the groups to see if they are checked or not
		{
			if( (pNMLV->iItem == i && pNMLV->iSubItem == COL_GROUP && !m_listGroups.GetCheck(i)) ||
				((pNMLV->iItem != i || pNMLV->iSubItem != COL_GROUP) && m_listGroups.GetCheck(i)) )
			{
				pGroupEntry = (_groupDataList*)m_listGroups.GetItemData(i);	// Entry we are looking for (key)

				if( pGroupEntry->pGroupData )
				{
					outputDebug("OnNMClickListGroups: Searching group %d (count=%d, name='%s', key='%s', member=%d, bGlobal=%d, socket=%d)", 
						i, 
						pGroupEntry->pGroupData->groupCount,
						pGroupEntry->pGroupData->groupName, 
						pGroupEntry->pGroupData->groupKey,
						pGroupEntry->pGroupData->bMember,
						pGroupEntry->pGroupData->bGlobalGroup,
						pGroupEntry->pGroupData->groupSocket);
				}
				else
					outputDebug("OnNMClickListGroups: No group info for group %d", i);

				pNewMessageListEntry = m_pMessageDataList;					// Point to the top of the message list
				while(pNewMessageListEntry)
				{
					if( pGroupEntry->pGroupData && !strcmp(pGroupEntry->pGroupData->groupKey, pNewMessageListEntry->pMessageData->groupKey) )	// They are the same!
					{
						outputDebug("OnNMClickListGroups: Group named '%s', key='%s' matched for group %d",
							pGroupEntry->pGroupData->groupName,
							pGroupEntry->pGroupData->groupKey,
							i);

						pNewMessageListEntry->nGroupsDisplayed++;
					}

					pNewMessageListEntry = pNewMessageListEntry->next;
				}
			}
		}


// Now the Message Data List has counts for how many groups have how many messages selected
// Now we have to go through the list and find out for each Message Key, how many of each key has selections
// We only have to do this for ONE group, since it is an intersection.  So for the 1st selected group, find out for each message
// associated with that group, how many times in the message list that message key was selected

		outputDebug("OnNMClickListGroups: Looping through %d groups to find common messages", count);

		for(i=0;i<count;i++)						// Loop through to find the 1st checked group
		{
			if( (pNMLV->iItem == i && pNMLV->iSubItem == COL_GROUP && !m_listGroups.GetCheck(i)) ||
				((pNMLV->iItem != i || pNMLV->iSubItem != COL_GROUP) && m_listGroups.GetCheck(i)) )
			{
				outputDebug("OnNMClickListGroups: Group %d was selected, searching for common messages", i);

				pGroupEntry = (_groupDataList*)m_listGroups.GetItemData(i);			// Entry we are looking for (key)
				pMessageEntry = NULL;												// no message entry yet

				while( pMessageEntry = getMessageEntry( pGroupEntry, pMessageEntry ) )	// Get the next (1st) message entry for this group
				{
					outputDebug("OnNMClickListGroups: Message title='%s' key='%s' being checked.  Selected %d times.  Looking for %d",
						pMessageEntry->pMessageData->title,
						pMessageEntry->pMessageData->groupKey,
						getMessageSelectedCount( pMessageEntry),
						selectedCount );

					if( getMessageSelectedCount( pMessageEntry ) == selectedCount )	// message exists in every selected group
					{
						outputDebug("OnNMClickListGroups: Found message title='%s'",
							pMessageEntry->pMessageData->title);

						m_listMessages.InsertItem( LVIF_TEXT|LVIF_IMAGE, messageCount, _T(" "), 0, 0, pMessageEntry->pMessageData->priority, NULL);
//	m_listMessages.SetItem(index, 0, LVIF_IMAGE, 0, pNewMessage->priority, 0, 0, NULL );
						m_listMessages.SetItemText(messageCount, 1, (LPCTSTR)pMessageEntry->pMessageData->title);

						m_listMessages.SetItemData(messageCount, (DWORD_PTR)pMessageEntry->pMessageData);

						messageCount++;											// Displaying a message
					}
				}

				outputDebug("OnNMClickListGroups: Found and displaying %d messages", messageCount);

				break;								// Get out of the loop when we find and display one
			}
		}
	}

	if( selectedCount == 0 )
	{
		outputDebug("OnNMClickListGroups: No groups selected");

		m_listMessages.InsertItem( LVIF_TEXT|LVIF_IMAGE, messageCount, _T(" "), 0, 0, 4, NULL);
		m_listMessages.SetItemText(messageCount, 1, getResourceString(IDS_STRING_GROUP_NO_GROUP_SELECTED));
	}
	else if( messageCount == 0 )
	{
		outputDebug("OnNMClickListGroups: No messages matched");

		m_listMessages.InsertItem( LVIF_TEXT|LVIF_IMAGE, messageCount, _T(" "), 0, 0, 4, NULL);
		m_listMessages.SetItemText(messageCount, 1, getResourceString(IDS_STRING_GROUP_NO_COMMON_MESSAGES_FOUND));
	}
	else
		m_listMessages.SortItems( &sortMessages, NULL );

	*pResult = 0;
}

int CTalkMasterConsoleDlg::getCheckedGroupsCount()
{
	int count, i, checkCount=0;

	count = m_listGroups.GetItemCount();

	for(i=0;i<count;i++)
	{
		if( m_listGroups.GetCheck(i) )
			checkCount++;
	}

	return(checkCount);
}

int CTalkMasterConsoleDlg::getFirstCheckedGroupIndex()
{
	int count, i;

	count = m_listGroups.GetItemCount();

	for(i=0;i<count;i++)
	{
		if( m_listGroups.GetCheck(i) )
			return(i);
	}

	return(-1);
}

int CTalkMasterConsoleDlg::getNextCheckedGroupIndex(int lastIndex)
{
	int count, i;

	count = m_listGroups.GetItemCount();

	for(i=lastIndex+1;i<count;i++)
	{
		if( m_listGroups.GetCheck(i) )
			return(i);
	}

	return(-1);
}

struct _groupDataList *CTalkMasterConsoleDlg::getGroupList(struct _groupData *pGroupData)
{
	struct _groupDataList *pNewGroupListEntry = m_pGroupData;

	while( pNewGroupListEntry )
	{
		if( pGroupData->groupSocket == pNewGroupListEntry->pGroupData->groupSocket )
			break;

		pNewGroupListEntry = pNewGroupListEntry->next;
	}

	return( pNewGroupListEntry );
}

struct _groupDataList *CTalkMasterConsoleDlg::getGroupList(int GroupSocket)
{
	struct _groupDataList *pNewGroupListEntry = m_pGroupData;

	while( pNewGroupListEntry )
	{
		if( GroupSocket == pNewGroupListEntry->pGroupData->groupSocket )
			break;

		pNewGroupListEntry = pNewGroupListEntry->next;
	}

	return( pNewGroupListEntry );
}


BOOL icomInGroup( int socket, int *groupSocket, int count )
{
	int i;

	for( i=0;i<count;i++ )
	{
		if( socket == groupSocket[i] )
			return(TRUE);
	}

	return(FALSE);
}

int CTalkMasterConsoleDlg::getListIcomsCount(struct _groupDataList *pGroupList)
{
	struct _groupItems *pListIcoms = pGroupList->pGroupItems;
	int count = 0;
	
	while( pListIcoms )
	{
		count++;

		pListIcoms = pListIcoms->next;
	}

	return(count);
}

int CTalkMasterConsoleDlg::getListIcoms(struct _groupDataList *pGroupList)
{
	m_DAGetGroupMembers( m_hDA, pGroupList->pGroupData->groupSocket );			// Start to have the intercoms sent to us

	while( pGroupList->pGroupData->groupCount > getListIcomsCount(pGroupList) )
	{
		DoEvents();
		Sleep(100);
	}

	outputDebug("getListIcoms: Done with group");

	return( pGroupList->pGroupData->groupCount );
}


void CTalkMasterConsoleDlg::refreshListIcoms(struct _groupDataList *pGroupList)
{
	struct _groupItems *pListIcoms = pGroupList->pGroupItems, *pListIcomsNext;
	struct _itemData *pItemData;
	int count = 0;
	
	while( pListIcoms )
	{
		pListIcomsNext = pListIcoms->next;

		pItemData = findItemData(&pListIcoms->pIcomData->iCom.MAC);

		if( pItemData && pListIcoms->bLocalIcomRecord )						// Should we refresh this entry?  There is a record and this is a local record
		{
			free( pListIcoms->pIcomData );									// Free the temporary record
			pListIcoms->pIcomData = pItemData;								// Point at the real record
			pListIcoms->bLocalIcomRecord = FALSE;							// Clear the local record flag
		}

		pListIcoms->next = pListIcomsNext;								// Reset the original next record

		pListIcoms = pListIcomsNext;									// Go to the next record
	}
}

	
void CTalkMasterConsoleDlg::OnNMRclickListGroups(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	char buffer[128];
	CMenu *popupMenu;
	struct _groupDataList *pGroupList;
	struct _groupItems *pGroupItems;
	int itemCount = 0;
	CString csMenuItem;

	if( pNMLV->iItem < 0 )
		return;

	popupMenu = m_rClickGroupMenu.GetSubMenu(0);

	pGroupList = (_groupDataList*)m_listGroups.GetItemData( pNMLV->iItem );

	if( pGroupList->type == 0 )							// We don't need to follow this list to other lists, we only get checked ones
	{
		csMenuItem.Format( getResourceString(IDS_STRING_GROUP_INTERCOMS_IN_GROUP), pGroupList->pGroupData->bGlobalGroup?"Global ":"" );

		replaceMenuItem( popupMenu, 0, csMenuItem.GetBuffer(), ID_INVISIBLE_OPTIONS );

		while( removeMenuItem( popupMenu, 2 ) )			// Get rid of all of the old menu items from last time
			;

		if( pGroupList->pGroupData->groupCount )
		{
			m_DACreateGroup(m_hDA, &pGroupList->pGroupData->groupSocket, NULL, 0, TRUE);				// Make a temp group so we can send stuff to the server based group

			if( getListIcomsCount( pGroupList ) != pGroupList->pGroupData->groupCount )
				getListIcoms( pGroupList );

			refreshListIcoms( pGroupList );			// Make sure the icom data is up to date

			m_DADeleteGroup(m_hDA, pGroupList->pGroupData->groupSocket);

			pGroupItems = pGroupList->pGroupItems;

			while( pGroupItems )
			{
//				if( pGroupIcoms->bDisplayed )
				{
					itemCount++;

					sprintf(buffer, "%s\t%s (%s)", pGroupItems->pIcomData->iCom.status?"OnLine":"OffLine", pGroupItems->pIcomData->iCom.name, inet_ntoa(pGroupItems->pIcomData->iCom.addr_in.sin_addr));
					insertMenuItem(popupMenu, 2, buffer, ID_INVISIBLE_OPTIONS);
				}

				pGroupItems = pGroupItems->next;
			}
		}

		if( !itemCount )
			insertMenuItem(popupMenu, 2, getResourceString(IDS_STRING_GROUP_NO_INTERCOMS_IN_GROUP).GetBuffer(), ID_INVISIBLE_OPTIONS);
	}

	POINT pPos; 

	GetCursorPos(&pPos); 

	popupMenu->TrackPopupMenu(TPM_RIGHTBUTTON, pPos.x, pPos.y, this, 0);

	setResetFocus(&m_listIcoms);

	*pResult = 0;
}

int CALLBACK sortMessages(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	CTalkMasterConsoleDlg *dlg = (CTalkMasterConsoleDlg*)theApp.m_pMainWnd;

	struct _messageData *item1 = (struct _messageData *)lParam1;
	struct _messageData *item2 = (struct _messageData *)lParam2;

	int iRet;

	switch(dlg->m_sortMessagesBy)
	{
	case 1:
		if( !dlg->m_bOrderMessagesBy )					// Down
		{
			iRet = strcmpi( (char*)item1->title, (char*)item2->title );

			if( iRet == 0 )
				iRet = item1->priority - item2->priority;
		}
		else
		{
			iRet = strcmpi( (char*)item2->title, (char*)item1->title );

			if( iRet == 0 )
				iRet = item2->priority - item1->priority;
		}
		break;

	default:
	case 0:															// Priority

		if( !dlg->m_bOrderMessagesBy )					// Down
		{
			iRet = item1->priority - item2->priority;

			if( iRet == 0 )						// Equal, sort by name
				iRet = strcmpi( (char*)item1->title, (char*)item2->title );
		}
		else
		{
			iRet = item2->priority - item1->priority;

			if( iRet == 0 )						// Equal, sort by name
				iRet = strcmpi( (char*)item2->title, (char*)item1->title );
		}

		break;
	}

	return( iRet );
}

void CTalkMasterConsoleDlg::OnLvnColumnclickListMessages(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	if( m_sortMessagesBy == pNMLV->iSubItem )
		m_bOrderMessagesBy = !m_bOrderMessagesBy;
	else
		m_bOrderMessagesBy = FALSE;

	m_sortMessagesBy = pNMLV->iSubItem;

	m_listMessages.SortItems( &sortMessages, NULL );

	*pResult = 0;
}
