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
#include "DAUnattendedDlg.h"


// CDAUnattendedDlg dialog

IMPLEMENT_DYNAMIC(CDAUnattendedDlg, CDialog)

CDAUnattendedDlg::CDAUnattendedDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDAUnattendedDlg::IDD, pParent)
{
}

CDAUnattendedDlg::~CDAUnattendedDlg()
{
}

void CDAUnattendedDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDAUnattendedDlg, CDialog)
END_MESSAGE_MAP()


// CDAUnattendedDlg message handlers

BOOL CDAUnattendedDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

//	forceForegroundWindow();

#ifdef PRIMEX_BUILD
	SetWindowText("IPI Console - Unattended");
#endif

	return(TRUE);			// Return TRUE unless focus is set on something
}
