#pragma once

#include "StdAfx.h"
#include "CameraData.h"
#include <string>

class CameraIntercomDlg;
class JpegCameraThread;

using namespace std;



/**
 * This class is the Controller behind the CameraIntercomDlg class.
 *  By "Controller" I'm referring to the Controller in the
 *  "Model-View-Controller" (abbreviated MVC) design pattern.
 */
class CameraDialogController
{
public:
	CameraDialogController();
	~CameraDialogController(void);

	/** Called by the CameraIntercomDlg class to set our reference to it.  */
	void setView(CameraIntercomDlg *dlg);

	/** Handles when the user clicks the "Test" button.  */
	void testButtonClicked();

	/** Gives you the model behind the view & controller.  */
	CameraData* getModel();

	/** Sets the model behind the view.  */
	void setModel(CameraData *model);

	/** Refreshes the state of the view using the model.  */
	void refresh();

	/** Handler for "OK" Button.  */
	void onOk();

	/** Handler for the "Cancel" Button.  */
	void onCancel();

protected:
	/** Gets the window text from the provided CWnd object. */
	void getWindowText(CWnd &wnd, string &copy);

private:
	CameraIntercomDlg *dlg;	
	CameraData *data;
	bool isRunning;
	bool isClosing;
	JpegCameraThread* thread;
};


//---------------------------------------------------------------------
// 
//---------------------------------------------------------------------