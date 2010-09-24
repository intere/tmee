#pragma once


// CPleaseWaitDlg dialog

class CPleaseWaitDlg : public CDialog
{
	DECLARE_DYNAMIC(CPleaseWaitDlg)

public:
	CPleaseWaitDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CPleaseWaitDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_PLEASE_WAIT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
