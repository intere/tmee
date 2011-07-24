// TestLogonDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TalkMasterConsole.h"
#include "TalkMasterConsoleDlg.h"
#include "TestLogonDlg.h"
#include ".\testlogondlg.h"


// CTestLogonDlg dialog

IMPLEMENT_DYNAMIC(CTestLogonDlg, CDialog)
CTestLogonDlg::CTestLogonDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTestLogonDlg::IDD, pParent)
	, m_csTestLogonName(_T(""))
	, m_csTestLogonPassword(_T(""))
	, bRunning(FALSE)
	, testState(NULL)
{
}

CTestLogonDlg::~CTestLogonDlg()
{
}

void CTestLogonDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_TEST_LOGON_NAME, m_editTestLogonName);
	DDX_Text(pDX, IDC_EDIT_TEST_LOGON_NAME, m_csTestLogonName);
	DDX_Control(pDX, IDC_EDIT_TEST_LOGON_PASSWORD, m_editTestLogonPassword);
	DDX_Text(pDX, IDC_EDIT_TEST_LOGON_PASSWORD, m_csTestLogonPassword);
	DDX_Control(pDX, IDC_BUTTON_TEST, m_btnLogonTest);
}


BEGIN_MESSAGE_MAP(CTestLogonDlg, CDialog)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_TEST, OnBnClickedButtonTest)
END_MESSAGE_MAP()


// CTestLogonDlg message handlers

BOOL CTestLogonDlg::OnInitDialog()
{
//	loadAlternateResourceDLL();

	CDialog::OnInitDialog();

	m_editTestLogonName.SetFocus();

	return FALSE;  // return TRUE  unless you set the focus to a control
}
void CTestLogonDlg::OnBnClickedButtonTest()
{
	CTalkMasterConsoleDlg *dlg = (CTalkMasterConsoleDlg*)theApp.m_pMainWnd;

	if( !bRunning )
	{
		testState = TEST_LOGON_STATE_IDLE;

		m_btnLogonTest.SetWindowText("Stop");
		SetTimer( TIMER_LOGON_TEST, 100, NULL );
		bRunning = TRUE;
	}
	else
	{
		bRunning = FALSE;
		m_btnLogonTest.SetWindowText("Test");

		SetTimer( TIMER_LOGON_TEST, 100, NULL );		// Speed it up so we don't have to wait 5 seconds for it to stop
	}
	
}

void CTestLogonDlg::OnTimer(UINT nIDEvent) 
{
	CString csTitle;
	static int guard=0;

	if( ++guard > 1 )
	{
		--guard;
		return;
	}

	if (nIDEvent == TIMER_LOGON_TEST )
	{
		doLogonTest();
	}

	CDialog::OnTimer(nIDEvent);

	--guard;
}

void CTestLogonDlg::doLogonTest()
{
	int	error;

	CTalkMasterConsoleDlg *dlg = (CTalkMasterConsoleDlg*)theApp.m_pMainWnd;
	UpdateData();

	switch(testState)
	{
	case TEST_LOGON_STATE_IDLE:
		m_uFlags = 0;				// We want cdecl calls to our callback
		error = dlg->m_DAOpen( &m_hDA, &m_uFlags, &CallBackTest, NULL );

		if( error == ERROR_SUCCESS )
		{
			m_sockaddr_in.sin_family = AF_INET;
			m_sockaddr_in.sin_port = htons(theApp.UserOptions.IPPort);
			m_sockaddr_in.sin_addr.S_un.S_addr = inet_addr(theApp.UserOptions.IPAddress);

			error = dlg->m_DAStart( m_hDA, &m_sockaddr_in, THREAD_BASE_PRIORITY_MAX, m_csTestLogonName.GetBuffer(), m_csTestLogonPassword.GetBuffer(), m_szUserName, &m_uRights );

			if( error == ERROR_SUCCESS )
			{
				testState = TEST_LOGON_STATE_STARTED;
				SetTimer( TIMER_LOGON_TEST, 5000, NULL );
			}
			else
			{
				dlg->m_DAClose( m_hDA );

				m_btnLogonTest.SetWindowText("Test");
				KillTimer(TIMER_LOGON_TEST);
				bRunning = FALSE;

				MessageBox("Logon ID or Password Invalid", "Shutting down test");
			}
		}
		else
		{
			m_btnLogonTest.SetWindowText("Test");
			KillTimer(TIMER_LOGON_TEST);
			bRunning = FALSE;

			MessageBox("DAOpen Failed to Open", "Major Issue Found");
		}

		break;

	case TEST_LOGON_STATE_STARTED:

		dlg->m_DAStop( m_hDA );
		dlg->m_DAClose( m_hDA );

		m_hDA = NULL;

		if( !bRunning )
		{
			KillTimer(TIMER_LOGON_TEST);
			m_btnLogonTest.SetWindowText("Test");
		}

		testState = TEST_LOGON_STATE_IDLE;

		break;
	}
}

void CallBackTest( int event, int socket, struct _iComStructure *pIcom, void *eventData, void *myData )
{
	CTalkMasterConsoleDlg *dlg = (CTalkMasterConsoleDlg*)theApp.m_pMainWnd;

	switch( event )
	{
	case EVENT_PRINT_STRING:
		dlg->outputDebug((char *)eventData);
		break;
	}
}

void CTestLogonDlg::OnCancel()
{
	if( bRunning )
	{
		bRunning = FALSE;
		m_btnLogonTest.SetWindowText("Test");

		SetTimer( TIMER_LOGON_TEST, 100, NULL );		// Speed it up so we don't have to wait 5 seconds for it to stop

		while( testState )
		{
			DoEvents();
			Sleep(100);
		}
	}

	CDialog::OnCancel();
}

void CTestLogonDlg::DoEvents()
{
	MSG msg;

	while( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

