//
// Utility.cpp : implementation file
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

#include "stdafx.h"
#include "windows.h"
#include "windowsx.h"
#include "TalkMasterConsole.h"
#include "TalkMasterConsoleDlg.h"
#include "utility.h"
#include "pleaseWaitDlg.h"
#include "shellapi.h"

void CTalkMasterConsoleDlg::loadAlternateResourceDLL()
{
	if( loadResourceDLL( GetUserDefaultLangID() ) )
		outputDebug("Loaded language for 0x%04x", GetUserDefaultLangID());
	else if( loadResourceDLL( GetUserDefaultLangID()&0xff ) )
		outputDebug("Loaded language for 0x%04x", GetUserDefaultLangID()&0xff);
	else if( loadResourceDLL( GetSystemDefaultLangID() ) )
		outputDebug("Loaded language for 0x%04x", GetSystemDefaultLangID());
	else if( loadResourceDLL( GetSystemDefaultLangID()&0xff ) )
		outputDebug("Loaded language for 0x%04x", GetSystemDefaultLangID()&0xff);
	else
		outputDebug("Loaded language from TalkMasterConsole.EXE");
}

BOOL CTalkMasterConsoleDlg::loadResourceDLL( LANGID langID )
{
	CString csResourceLibrary;
	HINSTANCE hInst;

	csResourceLibrary.Format("TalkMasterConsole%04x.dll", langID);
	hInst = LoadLibrary(csResourceLibrary);
	if (hInst != NULL)
		AfxSetResourceHandle(hInst);

	return(hInst!=NULL);
}

CString CTalkMasterConsoleDlg::getAlternateHelpName()
{
	static CString csRet;

	csRet=testAlternateHelpName( GetUserDefaultLangID() );
	if( !csRet.IsEmpty() )
	{
		outputDebug("Loaded language for 0x%04x", GetUserDefaultLangID());
		return(csRet);
	}

	csRet=testAlternateHelpName( GetUserDefaultLangID()&0xff );
	if( !csRet.IsEmpty() )
	{
		outputDebug("Loaded language for 0x%04x", GetUserDefaultLangID()&0xff);
		return(csRet);
	}

	csRet=testAlternateHelpName( GetSystemDefaultLangID() );
	if( !csRet.IsEmpty() )
	{
		outputDebug("Loaded language for 0x%04x", GetSystemDefaultLangID());
		return(csRet);
	}

	csRet=testAlternateHelpName( GetSystemDefaultLangID()&0xff );
	if( !csRet.IsEmpty() )
	{
		outputDebug("Loaded language for 0x%04x", GetSystemDefaultLangID()&0xff);
		return(csRet);
	}

	csRet.Format("%s\\HelpFile.chm", theApp.UserOptions.path);
	outputDebug("Loaded Help from '%s'", csRet);

	return(csRet);
}

CString CTalkMasterConsoleDlg::testAlternateHelpName( LANGID langID )
{
	static CString csHelpFileName;
	FILE *fp;
	
	csHelpFileName.Format("%s\\HelpFile%04x.chm", theApp.UserOptions.path, langID);

	if( fp=fopen(csHelpFileName, "rb") )
		fclose(fp);

	else
		csHelpFileName = "";

	return(csHelpFileName);
}

void CTalkMasterConsoleDlg::OnHelp()
{
	outputDebug(_T("Help File is named '%s'"), theApp.m_pszHelpFilePath);
	WinHelp(1, HELP_CONTENTS);
}

void CTalkMasterConsoleDlg::WinHelp( DWORD dwData, UINT nCmd )
{
//      DWORD i;
	switch (nCmd)
	{
	case HELP_CONTEXT:
#if 0
         // If it is a help context command, search for the
         // control ID in the array.
         for (i= 0; i < numHelpIDs*2; i+=2)
         {
           if (aMenuHelpIDs[i] == LOWORD (dwData) )
           {
             i++;  // pass the help context id to HTMLHelp
             HtmlHelp(m_pMainWnd->m_hWnd,"sample.chm",
                HH_HELP_CONTEXT,aMenuHelpIDs[i]);
             return;
           }
         }

         // If the control ID cannot be found,
         // display the default topic.
         if (i == numHelpIDs*2)
#endif
//			HtmlHelp(HH_DISPLAY_TOPIC,0);
		::HtmlHelp(this->m_hWnd, theApp.m_pszHelpFilePath, HH_DISPLAY_TOPIC, 0);
		break;

	case HELP_CONTENTS:
//		  HtmlHelp(HH_DISPLAY_TOC, 0);
		::HtmlHelp(this->m_hWnd, theApp.m_pszHelpFilePath, HH_DISPLAY_TOC, 0);
		break;
	}
}

void CTalkMasterConsoleDlg::doLocalizedHelpSetup()
{
	CString csHelpFileName = getAlternateHelpName();		// Returns the help file name to use

	free((void *)theApp.m_pszHelpFilePath);

	theApp.m_pszHelpFilePath = (LPCTSTR)calloc(1, strlen(csHelpFileName)+1);

	strcpy((char *)theApp.m_pszHelpFilePath, csHelpFileName);
}