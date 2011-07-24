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
#pragma once
#include "afxwin.h"
#include "DAPassThruDll.h"

#define TIMER_EVENT		1

// CDALoginDlg dialog

class CDALoginDlg : public CDialog
{
	DECLARE_DYNAMIC(CDALoginDlg)

public:
	CDALoginDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDALoginDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_LOGON };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	void OnTimer(UINT nIDEvent);

	DECLARE_MESSAGE_MAP()

	void enableControls(BOOL bEnable);
	void DoEvents();
public:
	CString m_csLoginName;
	CString m_csLoginPassword;
	CEdit m_editLoginName;
	afx_msg void OnEnChangeEdit2();
	CString m_csLogonIpAddress;
	CString m_csLogonIPPort;

	struct sockaddr_in m_sockaddr_in;

	UINT	m_uRights;
	char	m_szUserName[MAX_NAMELEN];

	BOOL	m_bAutoLogon;
	BOOL	m_bAutoRetry;

	int		m_nCountDown;
	int		m_nAttempts;

	BOOL	bOptions;
	BOOL	bLoggedOn;
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButton1();
	CButton m_btnOptions;
	afx_msg void OnBnClickedButtonLogonOptions();

	afx_msg void OnBnClickedCheckSaveLogon();
	afx_msg void OnBnClickedCancel();
	CButton m_btnCancel;
	CButton m_btnLogin;
	CEdit m_editLoginPassword;
	CEdit m_editLogonIpAddress;
	CEdit m_editLoginPort;
};
