// TestRTPSpeakerFromDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TalkMasterConsole.h"
#include "TalkMasterConsoleDlg.h"
#include "TestRTPSpeakerFromDlg.h"
#include ".\testrtpspeakerfromdlg.h"


// CTestRTPSpeakerFromDlg dialog

IMPLEMENT_DYNAMIC(CTestRTPSpeakerFromDlg, CDialog)
CTestRTPSpeakerFromDlg::CTestRTPSpeakerFromDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTestRTPSpeakerFromDlg::IDD, pParent)
	, m_csRTPSpeakerFromIP(_T(""))
	, m_nRTPSpeakerFromPort(0)
{
}

CTestRTPSpeakerFromDlg::~CTestRTPSpeakerFromDlg()
{
}

void CTestRTPSpeakerFromDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_RTP_SPEAKER_FROM_IP, m_editRTPSpeakerFromIP);
	DDX_Control(pDX, IDC_EDIT_RTP_SPEAKER_FROM_PORT, m_editRTPSpeakerFromPort);
	DDX_Text(pDX, IDC_EDIT_RTP_SPEAKER_FROM_IP, m_csRTPSpeakerFromIP);
	DDX_Text(pDX, IDC_EDIT_RTP_SPEAKER_FROM_PORT, m_nRTPSpeakerFromPort);
}


BEGIN_MESSAGE_MAP(CTestRTPSpeakerFromDlg, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()


// CTestRTPSpeakerFromDlg message handlers

BOOL CTestRTPSpeakerFromDlg::OnInitDialog()
{
//	loadAlternateResourceDLL();

	CDialog::OnInitDialog();

	m_editRTPSpeakerFromIP.SetWindowText("225.0.0.254");
	m_editRTPSpeakerFromPort.SetWindowText("4000");

	m_editRTPSpeakerFromIP.SetFocus();

	return FALSE;  // return TRUE  unless you set the focus to a control
}


void CTestRTPSpeakerFromDlg::OnBnClickedOk()
{
	int m_socket;

	UpdateData();

	CTalkMasterConsoleDlg *dlg = (CTalkMasterConsoleDlg*)theApp.m_pMainWnd;

	if( dlg->m_TalkMode == TALK_SELECTED )
	{
		if( dlg->m_pSelectedItem )
			m_socket = dlg->m_pSelectedItem->socket;
		else
		{
			MessageBox(_T("No Intercom Selected or Group not Selected"), "Select Intercom");
			return;
		}
	}
	else
		m_socket = dlg->makeGroupSocket();

	if( !dlg->selectSocket( m_socket ) )
	{
		MessageBox(_T("Could not select Intercoms"), "argh");
		return;
	}

	dlg->m_DARTPStartSpeakerFrom(dlg->m_hDA, m_socket, WAVE_FORMAT_ULAW, m_nRTPSpeakerFromPort, m_csRTPSpeakerFromIP.GetBuffer(), m_nRTPSpeakerFromPort);

	OnOK();
}
