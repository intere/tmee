#pragma once

#include "StdAfx.h"
#include "Resource.h"
#include "CameraDialogController.h"

// CameraIntercomDlg dialog

class CameraIntercomDlg : public CDialog
{
	DECLARE_DYNCREATE(CameraIntercomDlg)

public:
	CameraIntercomDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CameraIntercomDlg();

	enum { IDD = IDD_DIALOG_CAMERA };

	CEdit &getTxtIntercom();
	CEdit &getTxtUrl();
	CEdit &getTxtUsername();
	CEdit &getTxtPassword();
	CButton &getTestButton();
	CWnd &getCameraPreview();
	CameraDialogController &getController();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	
private:

	CEdit m_txtIntercom;
	CEdit m_txtUrl;
	CEdit m_txtUsername;
	CEdit m_txtPassword;
	CButton m_testButton;
	CWnd m_cameraPreview;
	CameraDialogController controller;

public:
	afx_msg void OnBnClickedBtnTest();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
};
