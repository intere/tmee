// PleaseWaitDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TalkMasterConsole.h"
#include "PleaseWaitDlg.h"


// CPleaseWaitDlg dialog

IMPLEMENT_DYNAMIC(CPleaseWaitDlg, CDialog)
CPleaseWaitDlg::CPleaseWaitDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPleaseWaitDlg::IDD, pParent)
{
}

CPleaseWaitDlg::~CPleaseWaitDlg()
{
}

void CPleaseWaitDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CPleaseWaitDlg, CDialog)
END_MESSAGE_MAP()


// CPleaseWaitDlg message handlers
