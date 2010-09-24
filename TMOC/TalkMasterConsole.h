//
// DAPassThruTest.h : main header file for the PROJECT_NAME application
//
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

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

#include "shellapi.h"

//#include "DATimer.h"

//
//	This structure is used in the record events thread to record debug events to a file
//	The structure holds a pointer to the next debug information so we can clean up after we print it
//
struct	_debugInfo
{
	struct _debugInfo *next;

	char	*string;
};

// CDAPassThruTestApp:
// See DAPassThruTest.cpp for the implementation of this class
//
struct _Options
{
	int	cx;				// Width of main window on startup
	int	cy;				// Height of main window on startup
	int pingInterval;	// number of seconds between PINGs
	int	startupAudioType;	// 0-beep; 1-Speech
	int startupAudioSound;	// If speech, which one?

	BOOL	useBeeps;	// Beep tones at start / end of calls
	BOOL	hideWindow;	// Hide the window when minimized?
	BOOL	PTT;		// Are we in PTT mode?

	BOOL	useUlaw;				// Use uLaw for Microphone?
	BOOL	repeatPlay;				// A test mode
	BOOL	forceHdx;				// Ignore Full Duplex devices, force Half Duplex
	BOOL	noiseFilter;			// eliminate low frequency noice

	BOOL	callAnnounce;			// Are we in Call Announce mode?
	BOOL	callAnnounceVisual;		// Visually announce?
	BOOL	callAnnounceAudible;	// Play a sound?
	int		callAnnounceSound;		// Which sound?  1-Doorbell; 0-Custom

	int		callAnnounceQueueAge;
	int		callAnnounceQueueSize;	// in entries
	int		callAnnounceQueueType;	// 0 is seconds, 1 is minutes

	BOOL	recordCallsWaiting;		// Should we record incoming audio from unanswered calls

	BOOL	archiveAudio;			// Archive all inbound / outbound?

	char	path[MAX_PATH];
	char	pathTemp[MAX_PATH];		// Where the temporary call queue files reside
	char	pathStore[MAX_PATH];	// The MyDocuments folder location to store stuff

	char	testVolumeFile[MAX_PATH];

	char	IPAddress[20];
	UINT	IPPort;

	BOOL	recordProgramEvents;	// Tracking.txt?
	FILE	*fpRecordProgramEvents;	// The FILE pointer for the tracking file
	int		recordEventsHandle;		// Handle for the debug thread
	BOOL	recordEventsAbort;		// Should we abort the record events thread?
	struct	_debugInfo *recordEventsStack;	// A pointer to the events to be written to the tracking file

	int		waveInputDevice;
	CString	waveInputDeviceName;
	int		waveOutputDevice;
	CString	waveOutputDeviceName;

	char	onlineDocsURL[MAX_PATH];
	char	onlineWebURL[MAX_PATH];
	char	onlineWebMenu[MAX_PATH];

	int		listenModeDefault;		// 0 - Auto, 1 - Manual

	RECT	startSize;				// Built from top, left, bottom, right
};

class CTalkMasterConsoleApp : public CWinApp
{
public:
	CTalkMasterConsoleApp();

protected:
	void DeleteFiles(char *folderName);

	void ReadRegistry();
	void UpdateMenu();

// Overrides
public:
	virtual BOOL InitInstance();
	void WriteRegistry();

	struct _Options	UserOptions;
#if 0
	struct _tics *m_tics;
	UINT m_periodMin;
	UINT m_timerID;
	int m_timerMax;
	UINT m_nTimerGuard;

	struct _tics *tic;
	struct _tics *tic2;
#endif

// Implementation

	DECLARE_MESSAGE_MAP()
	afx_msg void OnFileExit();
	afx_msg void OnViewPreferences();
public:
	afx_msg void OnFileCapturearchiveaudio();
	afx_msg void OnToolsRecordprogrameventstofile();
	afx_msg void OnToolsOpenSupportFolder();
	afx_msg void OnHelpAbout();
	afx_msg void OnHelpDigitalacousticswebsite();
	afx_msg void OnHelpOnlinedocumentation();
	afx_msg void OnFileLogoff();
	afx_msg void OnScreenpositionSavecurrentview();
	afx_msg void OnScreenpositionResettodefault();
	afx_msg void OnHelpHelponusingtalkmaster();
};

extern CTalkMasterConsoleApp theApp;