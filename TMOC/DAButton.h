#pragma once

// DAButton

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

class DAButton : public CButton
{
	DECLARE_DYNAMIC(DAButton)

public:
	DAButton();
	virtual ~DAButton();

	BOOL	isButtonDown();
	BOOL	isButtonUp();

protected:
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnEnable(BOOL enable);
	DECLARE_MESSAGE_MAP()
};


