//
// DebugThread.cpp : implementation file
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
#include "shellapi.h"

#include "windows.h"

void recordEventsThread(void *data);

void CTalkMasterConsoleDlg::OpenCloseDebugFile(BOOL forceClose)
{
	char buffer[128];

	if( theApp.UserOptions.recordProgramEvents && !forceClose )
	{
		CString csFileName = theApp.UserOptions.pathStore;
		csFileName += "\\consoleTracking.txt";

		if( theApp.UserOptions.fpRecordProgramEvents )
			closeDebug();

		theApp.UserOptions.fpRecordProgramEvents = fopen(csFileName, "ac");

		if( !theApp.UserOptions.fpRecordProgramEvents )
		{
			sprintf(buffer, "OpenCloseDebugFile: Failed to open file '%s' as debug file",
				csFileName);
			::OutputDebugString(buffer);
		}
		else
		{
			theApp.UserOptions.recordEventsAbort	= FALSE;
			theApp.UserOptions.recordEventsHandle	= _beginthread( recordEventsThread, NULL, NULL );

#ifdef PRIMEX_BUILD
	writeDebug(_T("IPI Operator Console Version %s running"), getFileVersionString().GetBuffer());
#else
	writeDebug(_T("TalkMaster Console Version %s running"), getFileVersionString().GetBuffer());
#endif

			sprintf(buffer, "** Starting write for console user '%s' **\n", consoleName);
			WriteDebugString(buffer);							// Write the string so the thread will write it out
		}
	}
	else
	{
		closeDebug();
	}
}

void CTalkMasterConsoleDlg::closeDebug()
{
	struct _debugInfo *pDebugNext;

	if( theApp.UserOptions.recordEventsHandle )				// If we have started the debug thread
	{
		::OutputDebugString("closeDebug(): Signalled debug thread to quit\n");

		theApp.UserOptions.recordEventsAbort = TRUE;		// Tell the thread to end
		while( theApp.UserOptions.recordEventsHandle )		// While the events handle is still set
			Sleep(100);										// Wait for the thread to end
	}

	if( theApp.UserOptions.fpRecordProgramEvents )			// If the debug file is open
	{
		fclose( theApp.UserOptions.fpRecordProgramEvents );	// Close the file
		theApp.UserOptions.fpRecordProgramEvents = NULL;	// Clear the file handle
	}

	while( theApp.UserOptions.recordEventsStack )			// If there is anything on the stack, free the memory now
	{
		pDebugNext = theApp.UserOptions.recordEventsStack->next;	// Remember the next record since this one will be free'd

		free(theApp.UserOptions.recordEventsStack->string);			// Free the string data
		free(theApp.UserOptions.recordEventsStack);					// Free the record data

		theApp.UserOptions.recordEventsStack = pDebugNext;			// Point at next record to free
	}
}

void CTalkMasterConsoleDlg::outputDebug(LPCTSTR pFormat, ...)
{
    TCHAR   chMsg[256];
	TCHAR	chTime[300];
    va_list pArg;

	time_t now;
	struct tm	atime;

	time( &now );
	atime = *localtime( &now );

	sprintf(chTime, "%d/%02d/%02d %d:%02d:%02d ", 
		atime.tm_mon+1, atime.tm_mday, atime.tm_year+1900,
		atime.tm_hour, atime.tm_min, atime.tm_sec );

    va_start(pArg, pFormat);
    _vstprintf(chMsg, pFormat, pArg);
    va_end(pArg);

	strcat(chTime, chMsg);
	strcat(chTime, "\n");

#ifdef _DEBUG
	OutputDebugString(chTime);
#endif

	if( theApp.UserOptions.recordProgramEvents )
		WriteDebugString((char *)chTime);
}

void CTalkMasterConsoleDlg::writeDebug(LPCTSTR pFormat, ...)
{
    TCHAR   chMsg[256];
	TCHAR	chTime[300];
    va_list pArg;
	BOOL	b_crlf = FALSE;

	time_t now;
	struct tm	atime;

	time( &now );
	atime = *localtime( &now );

	sprintf(chTime, "%d/%02d/%02d %d:%02d:%02d ", 
		atime.tm_mon+1, atime.tm_mday, atime.tm_year+1900,
		atime.tm_hour, atime.tm_min, atime.tm_sec);

    va_start(pArg, pFormat);
    _vstprintf(chMsg, pFormat, pArg);
    va_end(pArg);

	strcat(chTime, chMsg);

	strcat(chTime, "\n");
	b_crlf = TRUE;
	WriteDebugString((char *)chTime);

	if( !b_crlf )
		strcat(chTime, "\n");

	OutputDebugString(chTime);
}

void CTalkMasterConsoleDlg::WriteDebugString(char * pString)
{
	struct _debugInfo *pDebug;
	struct _debugInfo *pPrevDebug = NULL;
	struct _debugInfo *pCurrDebug;

	if( theApp.UserOptions.fpRecordProgramEvents )
	{
		pDebug = (struct _debugInfo *)calloc(1, sizeof(struct _debugInfo));
		pDebug->string = (char *)calloc(1, strlen(pString)+1);

		strcpy( pDebug->string, pString );

		EnterCriticalSection(&CriticalSectionDebug); 

		pCurrDebug = theApp.UserOptions.recordEventsStack;		// Grab the pointer here so we know it is clean

		while( pCurrDebug )
		{
			pPrevDebug = pCurrDebug;
			pCurrDebug = pCurrDebug->next;
		}

		if( !pPrevDebug )				// No Records, add at beginning
			theApp.UserOptions.recordEventsStack = pDebug;
		else
			pPrevDebug->next = pDebug;

		LeaveCriticalSection(&CriticalSectionDebug); 

	}
}

void recordEventsThread(void *data)
{
	CTalkMasterConsoleDlg *dlg = (CTalkMasterConsoleDlg*)theApp.m_pMainWnd;
//	CRITICAL_SECTION	CriticalSection;
	BOOL	bSleep = FALSE;					// A flag so we can have the thread sleep for 100ms if there is no work to do
	BOOL	bFlush = FALSE;					// Do we need to flush after a write?
	struct _debugInfo *pDebug=NULL;
	int		writeLen;

//	InitializeCriticalSection(&CriticalSection);

	::OutputDebugString("Starting debugThread\n");

	while( !theApp.UserOptions.recordEventsAbort ||	// Exit this thread when we are told to abort
		    theApp.UserOptions.recordEventsStack )	// and there are no events to output
	{
		if( bSleep )								// If there was no work the last time through
			Sleep(100);								// Sleep 100ms

		if( theApp.UserOptions.recordEventsStack )	// Is there something to write out?
		{
			bSleep = FALSE;							// There is some work, don't sleep next time through the loop
			bFlush = TRUE;							// But when we do get ready to sleep, first flush the file

			EnterCriticalSection(&dlg->CriticalSectionDebug); 
			pDebug = theApp.UserOptions.recordEventsStack;			// Take the 1st entry off of the stack
			theApp.UserOptions.recordEventsStack = pDebug->next;	// Remove the 1st entry off of the stack
			LeaveCriticalSection(&dlg->CriticalSectionDebug); 

			writeLen = fwrite(pDebug->string, strlen(pDebug->string), 1, theApp.UserOptions.fpRecordProgramEvents);

			free(pDebug->string);					// Free the string associated with this entry
			free(pDebug);							// Free the record

			if( !writeLen )							// If the write faield
			{
				::OutputDebugString("debugThread: Write to debug tracking file failed - exitting thread\n");
				break;
			}
		}
		else										// No more work to write out
		{
			if( bFlush )							// Only flush after a pause and after a write
			{
				if( !theApp.UserOptions.recordEventsAbort )				// Only flush if we are not aborting
					fflush(theApp.UserOptions.fpRecordProgramEvents);	// The fclose will flush things for us

				bFlush = FALSE;						// Already flushed
			}

			bSleep = TRUE;							// Ready to take a 100ms break
		}
	}

//	DeleteCriticalSection(&CriticalSection);

	::OutputDebugString("Ending debugThread\n");
	theApp.UserOptions.recordEventsHandle = NULL;	// We are leaving
}