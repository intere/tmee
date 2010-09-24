//
// DALoginDlg.cpp : implementation file
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
#include "DALoginDlg.h"
#include ".\dalogindlg.h"
#include "TalkMasterConsoleDlg.h"


// CDALoginDlg dialog

IMPLEMENT_DYNAMIC(CDALoginDlg, CDialog)
CDALoginDlg::CDALoginDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDALoginDlg::IDD, pParent)
	, m_csLoginName(_T(""))
	, m_csLoginPassword(_T(""))
	, m_csLogonIpAddress(_T(""))
	, m_csLogonIPPort(_T(""))
{
	bOptions = 0;
	m_nAttempts = 0;
	m_bAutoRetry = FALSE;
}

CDALoginDlg::~CDALoginDlg()
{
}

void CDALoginDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_LOGIN_NAME, m_csLoginName);
	DDV_MaxChars(pDX, m_csLoginName, 127);
	DDX_Text(pDX, IDC_EDIT_LOGIN_PASSWORD, m_csLoginPassword);
	DDV_MaxChars(pDX, m_csLoginPassword, 127);
	DDX_Control(pDX, IDC_EDIT_LOGIN_NAME, m_editLoginName);
	DDX_Text(pDX, IDC_EDIT_LOGON_IP_ADDRESS, m_csLogonIpAddress);
	DDV_MaxChars(pDX, m_csLogonIpAddress, 15);
	DDX_Text(pDX, IDC_EDIT_LOGON_PORT, m_csLogonIPPort);
	DDV_MaxChars(pDX, m_csLogonIPPort, 5);
	DDX_Control(pDX, IDC_BUTTON_LOGON_OPTIONS, m_btnOptions);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDOK, m_btnLogin);
	DDX_Control(pDX, IDC_EDIT_LOGIN_PASSWORD, m_editLoginPassword);
	DDX_Control(pDX, IDC_EDIT_LOGON_IP_ADDRESS, m_editLogonIpAddress);
	DDX_Control(pDX, IDC_EDIT_LOGON_PORT, m_editLoginPort);
}


BEGIN_MESSAGE_MAP(CDALoginDlg, CDialog)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON_LOGON_OPTIONS, OnBnClickedButtonLogonOptions)
	ON_BN_CLICKED(IDC_CHECK_SAVE_LOGON, OnBnClickedCheckSaveLogon)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
END_MESSAGE_MAP()


// CDALoginDlg message handlers

BOOL CDALoginDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	CTalkMasterConsoleDlg *dlg = (CTalkMasterConsoleDlg*)theApp.m_pMainWnd;
	int cx, cy;												// Where on the screen?
	int len = 160;											// Default length of the dialog

	RECT screenView, clientView;
	dlg->GetWindowRect(&screenView);

	GetClientRect(&clientView);
	ClientToScreen(&clientView);

	cx = screenView.left + ((screenView.right  - screenView.left) / 2) - (clientView.right - clientView.left) / 2;
	cy = screenView.top  + ((screenView.bottom - screenView.top)  / 2) - (clientView.bottom - clientView.top) / 2;

	if( !strcmp(m_csLogonIpAddress, "0.0.0.0" ) )
	{
		len = 200;
		m_btnOptions.SetWindowText(dlg->getResourceString(IDS_STRING_LOGON_OPTIONS_LESS));		// "<< Options"
	}

	SetWindowPos(NULL, cx, cy, 290, len,		// 187
		SWP_NOOWNERZORDER | SWP_SHOWWINDOW | SWP_NOZORDER );

	bLoggedOn = FALSE;
	m_editLoginName.SetFocus();

	if( m_bAutoLogon && m_csLoginName != "" && m_csLoginPassword != "" )
		OnBnClickedOk();

	return FALSE;  // return TRUE  unless you set the focus to a control
}

void CDALoginDlg::OnBnClickedOk()
{
	CTalkMasterConsoleDlg *dlg = (CTalkMasterConsoleDlg*)theApp.m_pMainWnd;
	int	error;
	char buf[128];
	CString csMsg;

	KillTimer(TIMER_EVENT);						// Make sure the timer is not running

	UpdateData();

	SetWindowText("Logging in - Please Wait");	// Change the dialog to say we are loggin in

	enableControls(FALSE);						// Disable all of the controls on the dialog

	DoEvents();									// Allow the controls to display themselves

	m_sockaddr_in.sin_family = AF_INET;
	m_sockaddr_in.sin_port = htons(atoi(m_csLogonIPPort));
	m_sockaddr_in.sin_addr.S_un.S_addr = inet_addr(m_csLogonIpAddress);

// Start and log into the server - this call does not return until it fails or connects

	if( (error=dlg->m_DAStart( dlg->m_hDA, &m_sockaddr_in, dlg->m_threadPriority, m_csLoginName.GetBuffer(), m_csLoginPassword.GetBuffer(), m_szUserName, &m_uRights)) == ERROR_SUCCESS )
	{
		bLoggedOn = TRUE;
		OnOK();
	}
	else
	{
		m_bAutoRetry = FALSE;					// By default we don't retry

		if( m_nAttempts++ > 0 )					// Try everything twice
		{
			CString message;
			CString messageP1;
			CString messageP2;
			switch(error)
			{
			case ERROR_RESP_TIMEOUT:
				m_bAutoRetry = TRUE;				// Retry this since we are ready
				csMsg.LoadString(IDS_STRING131);
				break;
			case ERROR_LOGON_FAILED:
				csMsg.LoadString(IDS_STRING132);
				break;
			case ERROR_BAD_VERSION:
				csMsg.LoadString(IDS_STRING133);
				break;
			case ERROR_CONNECT_FAILED:
				m_bAutoRetry = TRUE;				// Retry this since we are ready
				messageP1.LoadString(IDS_STRING_ERROR);
				messageP2.LoadString(IDS_STRING134);
				sprintf(buf, messageP1 + " %d: " + messageP2 + " %s:%d", error, m_csLogonIpAddress, atoi(m_csLogonIPPort));
				csMsg = buf;
				break;
			case ERROR_BAD_USERID_PWD:
				csMsg.LoadString(IDS_STRING135);
				break;
			case ERROR_ALREADY_LOGGED_IN:
				csMsg.LoadString(IDS_STRING136);
				break;
			case ERROR_OUT_OF_LICENSES:
				csMsg.LoadString(IDS_STRING137);
				break;
			case ERROR_NOT_LICENSED:
				csMsg.LoadString(IDS_STRING138);
				break;

			default:
				message.LoadString(IDS_STRING_ERROR);
				messageP1.LoadString(IDS_STRING139);
				sprintf(buf, message + " %d " + messageP1, error);
				csMsg = buf;
				break;
			}
		}
		else
			m_bAutoRetry = TRUE;

		if( m_bAutoRetry )
		{
			enableControls(FALSE);
			m_btnCancel.EnableWindow();
			
			SetWindowText(dlg->getResourceString(IDS_STRING_LOGIN_RETRY));
			SetTimer(TIMER_EVENT, 1000, 0);
			m_nCountDown = 10;
		}
		else
		{
			enableControls(TRUE);
			
			SetWindowText(dlg->getResourceString(IDS_STRING_LOGIN_REQUIRED));
			MessageBox(csMsg, dlg->getResourceString(IDS_STRING_ERROR));
		}
	}
}

void CDALoginDlg::OnBnClickedButtonLogonOptions()
{
	CTalkMasterConsoleDlg *dlg = (CTalkMasterConsoleDlg*)theApp.m_pMainWnd;

	int	cx, cy;

	if(!bOptions)
	{
		cx = 290;
		cy = 200;

		m_btnOptions.SetWindowText(dlg->getResourceString(IDS_STRING_LOGON_OPTIONS_LESS));		// "<< Options"
	}
	else
	{
		cx = 290;
		cy = 160;

		m_btnOptions.SetWindowText(dlg->getResourceString(IDS_STRING_LOGON_OPTIONS_MORE));		// "Options >>"
	}

	bOptions = ~bOptions;

	SetWindowPos(NULL, 0, 0, cx, cy,		// 187
		SWP_NOOWNERZORDER | SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOZORDER );
}

void CDALoginDlg::OnBnClickedCheckSaveLogon()
{
	// TODO: Add your control notification handler code here
}

void CDALoginDlg::OnTimer(UINT nIDEvent) 
{
	CTalkMasterConsoleDlg *dlg = (CTalkMasterConsoleDlg*)theApp.m_pMainWnd;
	CString csTitle;

	if (nIDEvent == TIMER_EVENT )
	{
		csTitle.Format(dlg->getResourceString(IDS_STRING_WAITING_FOR_LOGIN_RETRY), --m_nCountDown);
		SetWindowText(csTitle);

		if( m_nCountDown == 0 )
		{
			KillTimer(TIMER_EVENT);
			OnBnClickedOk();
		}
	}

	CDialog::OnTimer(nIDEvent);
}

void CDALoginDlg::OnBnClickedCancel()
{
	CTalkMasterConsoleDlg *dlg = (CTalkMasterConsoleDlg*)theApp.m_pMainWnd;
	if( m_bAutoRetry )
	{
		enableControls(TRUE);

		SetWindowText(dlg->getResourceString(IDS_STRING141));

		m_bAutoRetry = FALSE;

		KillTimer(TIMER_EVENT);

		return;
	}

	OnCancel();
}

void CDALoginDlg::enableControls(BOOL bEnable)
{
	m_btnLogin.EnableWindow(bEnable);
	m_btnCancel.EnableWindow(bEnable);
	m_btnOptions.EnableWindow(bEnable);
	m_editLoginName.EnableWindow(bEnable);
	m_editLoginPassword.EnableWindow(bEnable);
	m_editLogonIpAddress.EnableWindow(bEnable);
	m_editLoginPort.EnableWindow(bEnable);
}

void CDALoginDlg::DoEvents()
{
	MSG msg;

	while( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}
