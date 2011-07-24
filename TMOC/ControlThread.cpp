//
// DACodec.cpp : implementation file
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
#include "ControlThread.h"

void controlThread(void *data)
{
	BOOL bTalkingLast=FALSE, bTalkRequestLast=FALSE;
	BOOL bListeningLast=FALSE, bListenRequestLast=FALSE;
	BOOL bSessionLast=FALSE, bSessionRequestLast=FALSE;

	CTalkMasterConsoleDlg *dlg = (CTalkMasterConsoleDlg*)theApp.m_pMainWnd;

	while( dlg->m_bAbortControl == FALSE && dlg->bLoggedOn )
	{
		Sleep(250);

		if((bTalkingLast != dlg->bTalking ||
			bTalkRequestLast != dlg->bTalkRequest ||
			bListeningLast != dlg->bListening ||
			bListenRequestLast != dlg->bListenRequest ||
			bSessionRequestLast != dlg->bSessionRequest )
	//		1
			)
		{
			dlg->outputDebug(_T("ControlThread: bTalking=%s, Request=%s / bListening=%s, request=%s / bSession=%s, request=%s"), 
				(dlg->bTalking)?_T("TRUE"):_T("FALSE"),
				(dlg->bTalkRequest)?_T("TRUE"):_T("FALSE"),
				(dlg->bListening)?_T("TRUE"):_T("FALSE"),
				(dlg->bListenRequest)?_T("TRUE"):_T("FALSE"),
				(dlg->bSession)?_T("TRUE"):_T("FALSE"),
				(dlg->bSessionRequest)?_T("TRUE"):_T("FALSE") );
		}

		bTalkingLast = dlg->bTalking;
		bTalkRequestLast = dlg->bTalkRequest;
		bListeningLast = dlg->bListening;
		bListenRequestLast = dlg->bListenRequest;
		bSessionRequestLast = dlg->bSessionRequest;

		if( dlg->bTalking != dlg->bTalkRequest )
		{
//			dlg->SetHoldEnd(FALSE, FALSE);					// Don't want these working while we are in the routines

			dlg->doTalk();
		}

// Now make sure that we doListen() propely

		else if( dlg->bListening != dlg->bListenRequest )
		{
//			dlg->SetHoldEnd(FALSE, FALSE);					// Don't want these working while we are in the routines

			dlg->doListen();
		}
		
		else if( dlg->bSession != dlg->bSessionRequest )
			dlg->doSessionStartStop();

		if( !dlg->bTalking && !dlg->bListening && !dlg->bTalkRequest && !dlg->bListenRequest && dlg->bReleaseRequest )
		{
			dlg->doRelease();
		}
	}

	OutputDebugString("controlThread: Exitting Thread");	// the dlg->outputDebugString() is probably shut down at this point

	dlg->m_threadControl = NULL;				// Let the world know that we are done
}