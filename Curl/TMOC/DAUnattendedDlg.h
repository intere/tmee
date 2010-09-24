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


// CDAUnattendedDlg dialog

class CDAUnattendedDlg : public CDialog
{
	DECLARE_DYNAMIC(CDAUnattendedDlg)

public:
	CDAUnattendedDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDAUnattendedDlg();
	virtual BOOL OnInitDialog();

// Dialog Data
	enum { IDD = IDD_DIALOG_UNATTENDED };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
