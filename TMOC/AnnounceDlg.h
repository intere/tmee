#pragma once
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


// CAnnounceDlg dialog

class CAnnounceDlg : public CDialog
{
	DECLARE_DYNAMIC(CAnnounceDlg)

public:
	CAnnounceDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAnnounceDlg();
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	virtual void OnOK();

// Dialog Data
	enum { IDD = IDD_DIALOG_ANNOUNCE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();

	void CAnnounceDlg::forceForegroundWindow();

	BOOL	bActive;
};
