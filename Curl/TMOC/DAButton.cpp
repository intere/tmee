//
// DAButton.cpp : implementation file
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
#include "TalkMasterConsole.h"
#include "DAButton.h"


// DAButton

IMPLEMENT_DYNAMIC(DAButton, CButton)
DAButton::DAButton()
{
}

DAButton::~DAButton()
{
}


BEGIN_MESSAGE_MAP(DAButton, CButton)
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_ENABLE()
END_MESSAGE_MAP()



// DAButton message handlers

void DAButton::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	CButton::OnLButtonDown(nFlags, point);

	UINT uI = MAKELONG(this->GetDlgCtrlID(),WM_LBUTTONDOWN);
	GetParent()->SendMessage(WM_COMMAND, uI, (LPARAM)this->m_hWnd);
}

void DAButton::OnLButtonDown(UINT nFlags, CPoint point)
{
	CButton::OnLButtonDown(nFlags, point);

	UINT uI = MAKELONG(this->GetDlgCtrlID(),WM_LBUTTONDOWN);
	GetParent()->SendMessage(WM_COMMAND, uI, (LPARAM)this->m_hWnd);
}

void DAButton::OnLButtonUp(UINT nFlags, CPoint point)
{
	CButton::OnLButtonUp(nFlags, point);

	UINT uI = MAKELONG(this->GetDlgCtrlID(),WM_LBUTTONUP);
	GetParent()->SendMessage(WM_COMMAND, uI, (LPARAM)this->m_hWnd);
}

void DAButton::OnEnable(BOOL enable)
{
	CButton::OnEnable(enable);

	if( !enable )
	{
		UINT uI = MAKELONG(this->GetDlgCtrlID(),BN_DISABLE);
		GetParent()->SendMessage(WM_COMMAND, uI, (LPARAM)this->m_hWnd);
	}
}