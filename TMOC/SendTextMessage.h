#pragma once
#include "afxwin.h"


// CSendTextMessageDlg dialog

class CSendTextMessageDlg : public CDialog
{
	DECLARE_DYNAMIC(CSendTextMessageDlg)

public:
	CSendTextMessageDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSendTextMessageDlg();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	void OnTimer(UINT nIDEvent);

// Dialog Data
	enum { IDD = IDD_DIALOG_SEND_MESSAGE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_editTextMessage;
	CString m_csTextMessage;

	BOOL	m_bCancel;
	BOOL	m_bDialogInUse;
	BOOL	m_bWaitingForCompletion;

	int		m_socket;
	CButton m_btnSend;
	afx_msg void OnBnClickedButtonSendText();
	afx_msg void OnEnChangeEditTextMessage();
	CButton m_btnCancel;
};
