#pragma once
#include "afxwin.h"


// CTestRTPSpeakerFromDlg dialog

class CTestRTPSpeakerFromDlg : public CDialog
{
	DECLARE_DYNAMIC(CTestRTPSpeakerFromDlg)

public:
	CTestRTPSpeakerFromDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTestRTPSpeakerFromDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_TEST_RTP };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_editRTPSpeakerFromIP;
	CEdit m_editRTPSpeakerFromPort;
	afx_msg void OnBnClickedOk();
	CString m_csRTPSpeakerFromIP;
	int m_nRTPSpeakerFromPort;
};
