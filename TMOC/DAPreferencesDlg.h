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
#include "afxcmn.h"
#include <sys/stat.h>


// DAPreferencesDlg dialog

class DAPreferencesDlg : public CDialog
{
	DECLARE_DYNCREATE(DAPreferencesDlg)

public:
	DAPreferencesDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~DAPreferencesDlg();
// Overrides

// Dialog Data
	enum { IDD = IDD_DIALOG_PREFERENCES };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnCbnSelchangeComboStartupType();
	CComboBox m_comboStartupType;
	CComboBox m_comboStartupLanguage;
	CComboBox m_comboAnnouncementSound;
	CComboBox m_comboWaveInputDevice;
	CComboBox m_comboWaveOutputDevice;
	CEdit m_editPingInterval;
	CSpinButtonCtrl m_cspinPingInterval;
	CButton m_checkAddBeeps;
	CButton m_checkHide;
	CButton m_checkPTT;
	CButton m_checkCallAnnouncement;
	CButton m_checkRecordCallsWaiting;
	CButton m_checkVisualNotification;
	CButton m_checkAudibleNotification;
	CString m_csPingInterval;
	afx_msg void OnBnClickedOk();
	BOOL m_bBeeps;
	BOOL m_bHide;
	BOOL m_bPTT;
	BOOL m_bCallAnnouncement;
	BOOL m_bVisualNotification;
	BOOL m_bAudibleNotification;
	BOOL m_bRecordCallsWaiting;
	afx_msg void OnBnClickedCheckAudibleNotification();
	afx_msg void OnBnClickedCheckCallAnnounce();

	void loadWaveInputDevices();
	void loadWaveOutputDevices();
	afx_msg void OnCbnSelchangeComboWaveOutputDevice();
	CTabCtrl m_tabPreferences;

	int		m_mode;
	void	hidePage(int pageNum);
	void	showPage(int pageNum);

	BOOL	m_bWarning;
	int		m_nWaveInputDevice;

	void DAPreferencesDlg::showMain();
	void DAPreferencesDlg::showDevices();
	void DAPreferencesDlg::showConsole();
	void DAPreferencesDlg::showLocal();

	CStatic m_staticPrefPingGroup;
	CStatic m_staticPrefStartupAudioGroup;
	CStatic m_staticPrefCallAnnounceGroup;
	CStatic m_staticPrefWaveOutGroup;
	CStatic m_staticPrefWaveInGroup;
	CStatic m_staticPrefPingSecs;
	afx_msg void OnTcnSelchangeTabPreferences(NMHDR *pNMHDR, LRESULT *pResult);
	CButton m_btnPrefHelp;
	CButton m_btnOK;
	CStatic m_staticPrefAnnounceRulesGroup;
	CStatic m_staticPrefQueueAge;
	CStatic m_staticPrefQueueSize;
	CEdit m_editPrefQueueAge;
	int m_nPrefQueueAge;
	CEdit m_editPrefQueueSize;
	int m_nPrefQueueSize;
	CComboBox m_comboPrefQueueAgeType;
	CStatic m_staticPrefQueueEntries;
	CButton m_checkUseUlaw;
	BOOL m_bUseUlaw;
	afx_msg void OnCbnSelchangeComboWaveInputDevice();
	afx_msg void OnBnClickedCheckMicrophoneUlaw();
	CButton m_btnPlayAgain;
	BOOL m_bPlayAgain;
	CButton m_btnShowDebug;
	BOOL m_bShowDebug;

	BOOL	m_bDADebug;
	CButton m_checkSetupVolume;
	BOOL m_bSetupVolume;
	CEdit m_editTestVolumeFile;
	CString m_csTestVolumeFile;
	CButton m_btnBrowseTestVolumeFile;
	afx_msg void OnBnClickedButtonBrowseTestVolumeFile();
	CStatic m_staticTestVolumeFileGroup;
	CButton m_btnForceHdx;
	BOOL m_bForceHdx;
	CButton m_btnNoiseFilter;
	BOOL m_bFilterNoise;
};
