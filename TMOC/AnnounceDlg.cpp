//
// AnnounceDlg.cpp : implementation file
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
#include "AnnounceDlg.h"
#include ".\announcedlg.h"


// CAnnounceDlg dialog

IMPLEMENT_DYNAMIC(CAnnounceDlg, CDialog)
CAnnounceDlg::CAnnounceDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAnnounceDlg::IDD, pParent)
{
	bActive = FALSE;
}

CAnnounceDlg::~CAnnounceDlg()
{
}

void CAnnounceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CAnnounceDlg, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
END_MESSAGE_MAP()


// CAnnounceDlg message handlers

BOOL CAnnounceDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	forceForegroundWindow();

#ifdef PRIMEX_BUILD
	SetWindowText("IPI Console - Incoming Call");
#endif

	bActive = TRUE;

	return(TRUE);
}

void CAnnounceDlg::OnBnClickedOk()
{
	OnOK();
}

void CAnnounceDlg::OnBnClickedCancel()
{
	OnCancel();
}

void CAnnounceDlg::OnCancel()
{
	bActive = FALSE;

	CDialog::OnCancel();
}

void CAnnounceDlg::OnOK()
{
	bActive = FALSE;

	CDialog::OnOK();
}

void CAnnounceDlg::forceForegroundWindow()
{
	DWORD	threadID1, threadID2;

	CWnd	*pWnd = GetForegroundWindow();				// Need to get it so we can check it for NULL (locked screen)

	if( pWnd && m_hWnd != pWnd->m_hWnd )
	{
		threadID1 = GetWindowThreadProcessId( pWnd->m_hWnd, NULL );
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
}