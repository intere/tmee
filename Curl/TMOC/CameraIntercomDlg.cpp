// CameraIntercomDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TalkMasterConsole.h"
#include "CameraIntercomDlg.h"
#include "CameraDialogController.h"
#include ".\cameraintercomdlg.h"

IMPLEMENT_DYNCREATE(CameraIntercomDlg, CDialog)

CameraIntercomDlg::CameraIntercomDlg(CWnd* pParent /*=NULL*/) 
	: CDialog(CameraIntercomDlg::IDD, pParent)
{
	controller.setView(this);
}

CameraIntercomDlg::~CameraIntercomDlg()
{

}

void CameraIntercomDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_BTN_TEST, m_testButton);
	DDX_Control(pDX, IDC_IMG_TEST, m_cameraPreview);
	DDX_Control(pDX, IDC_EDIT_INTERCOM, m_txtIntercom);
	DDX_Control(pDX, IDC_EDIT_URL, m_txtUrl);
	DDX_Control(pDX, IDC_EDIT_USERNAME, m_txtUsername);
	DDX_Control(pDX, IDC_EDIT_PASSWORD, m_txtPassword);

	getController().refresh();
}

BOOL CameraIntercomDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	return TRUE;  // return TRUE  unless you set the focus to a control
}

BEGIN_MESSAGE_MAP(CameraIntercomDlg, CDialog)
	ON_BN_CLICKED(IDC_BTN_TEST, OnBnClickedBtnTest)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
END_MESSAGE_MAP()

CEdit &CameraIntercomDlg::getTxtIntercom()
{
	return this->m_txtIntercom;
}

CEdit &CameraIntercomDlg::getTxtUrl()
{
	return this->m_txtUrl;
}

CEdit &CameraIntercomDlg::getTxtUsername()
{
	return this->m_txtUsername;
}

CEdit &CameraIntercomDlg::getTxtPassword()
{
	return this->m_txtPassword;
}

CButton &CameraIntercomDlg::getTestButton()
{
	return this->m_testButton;
}

CWnd &CameraIntercomDlg::getCameraPreview()
{
	return this->m_cameraPreview;
}

CameraDialogController &CameraIntercomDlg::getController()
{
	return this->controller;
}


void CameraIntercomDlg::OnBnClickedBtnTest()
{
	getController().testButtonClicked();
}

void CameraIntercomDlg::OnBnClickedOk()
{
	controller.onOk();
	OnOK();
}

void CameraIntercomDlg::OnBnClickedCancel()
{
	controller.onCancel();
	OnCancel();
}