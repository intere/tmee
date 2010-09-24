#pragma once
#include "afxwin.h"

#define TIMER_LOGON_TEST	1

#define	TEST_LOGON_STATE_IDLE		0
#define	TEST_LOGON_STATE_STARTED	1
#define	TEST_LOGON_STATE_STOP		2

// CTestLogonDlg dialog

class CTestLogonDlg : public CDialog
{
	DECLARE_DYNAMIC(CTestLogonDlg)

public:
	CTestLogonDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTestLogonDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_TEST_LOGON };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	void OnTimer(UINT nIDEvent);
	virtual void OnCancel();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonTest();
	CEdit m_editTestLogonName;
	CString m_csTestLogonName;
	CEdit m_editTestLogonPassword;
	CString m_csTestLogonPassword;

	void doLogonTest();
	void DoEvents();

	BOOL	bRunning;
	int		testState;
	CButton m_btnLogonTest;
	HANDLE	m_hDA;
	UINT	m_uFlags;
	struct sockaddr_in m_sockaddr_in;
	char	m_szUserName[MAX_PATH];
	UINT	m_uRights;
};

void CallBackTest( int event, int socket, struct _iComStructure *pIcom, void *eventData, void *myData );