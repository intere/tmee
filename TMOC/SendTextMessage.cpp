// SendTextMessage.cpp : implementation file
//

#include "stdafx.h"
#include "TalkMasterConsole.h"
#include "TalkMasterConsoleDlg.h"
#include "SendTextMessage.h"
#include ".\sendtextmessage.h"


// CSendTextMessageDlg dialog

IMPLEMENT_DYNAMIC(CSendTextMessageDlg, CDialog)
CSendTextMessageDlg::CSendTextMessageDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSendTextMessageDlg::IDD, pParent)
	, m_csTextMessage(_T(""))
	, m_bDialogInUse(FALSE)
	, m_socket(0)
	, m_bWaitingForCompletion(FALSE)
{
}

CSendTextMessageDlg::~CSendTextMessageDlg()
{
}

void CSendTextMessageDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_TEXT_MESSAGE, m_editTextMessage);
	DDX_Text(pDX, IDC_EDIT_TEXT_MESSAGE, m_csTextMessage);
	DDX_Control(pDX, IDC_BUTTON_SEND_TEXT, m_btnSend);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
}


BEGIN_MESSAGE_MAP(CSendTextMessageDlg, CDialog)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_SEND_TEXT, OnBnClickedButtonSendText)
	ON_EN_CHANGE(IDC_EDIT_TEXT_MESSAGE, OnEnChangeEditTextMessage)
END_MESSAGE_MAP()


// CSendTextMessage message handlers

BOOL CSendTextMessageDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_btnCancel.EnableWindow(TRUE);

	m_btnSend.EnableWindow(FALSE);
	m_editTextMessage.SetWindowText("");

	m_editTextMessage.SetFocus();

	return FALSE;  // return TRUE  unless you set the focus to a control
}

void CSendTextMessageDlg::OnOK()
{
	OnBnClickedButtonSendText();
}

void CSendTextMessageDlg::OnBnClickedButtonSendText()
{
	UpdateData();

	CTalkMasterConsoleDlg *dlg = (CTalkMasterConsoleDlg*)theApp.m_pMainWnd;

	if( dlg->m_TalkMode == TALK_SELECTED )
		m_socket = dlg->m_pSelectedItem->socket;
	else
		m_socket = dlg->makeGroupSocket();

	if( !dlg->selectSocket( m_socket ) )
	{
		MessageBox(_T("Could not select Intercoms"), "argh");
		return;
	}

	m_btnSend.EnableWindow(FALSE);
	m_btnCancel.EnableWindow(FALSE);

	dlg->m_DASayText( dlg->m_hDA, m_socket, m_csTextMessage.GetBuffer() );

	m_bWaitingForCompletion = TRUE;

	SetTimer(TIMER_WAIT, 100, NULL);
}

void CSendTextMessageDlg::OnEnChangeEditTextMessage()
{
	m_btnSend.EnableWindow(m_editTextMessage.GetWindowTextLength() > 0);
}

void CSendTextMessageDlg::OnTimer(UINT nIDEvent)
{
	CTalkMasterConsoleDlg *dlg = (CTalkMasterConsoleDlg*)theApp.m_pMainWnd;

	switch( nIDEvent )
	{
	case TIMER_WAIT:
		if( !m_bWaitingForCompletion )
		{
			dlg->deselectSocket( m_socket );

			if( m_socket < 0 )
				dlg->m_DADeleteGroup( dlg->m_hDA, m_socket );

			m_socket = 0;

			KillTimer(TIMER_WAIT);

			CDialog::OnCancel();
		}
		break;
	}
}