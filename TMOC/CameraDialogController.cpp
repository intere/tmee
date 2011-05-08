#include "stdafx.h"
#include "CameraDialogController.h"
#include "CameraIntercomDlg.h"
#include "JpegCameraThread.h"
#include "CameraDataManager.h"

CameraDialogController::CameraDialogController()
{
	dlg = NULL;
	isRunning = false;
	isClosing = false;
}

CameraDialogController::~CameraDialogController(void)
{
	if(JpegCameraThread::getThread())
	{
		JpegCameraThread::getThread()->KillThread();
	}
}

/** Setter for the View.  */
void CameraDialogController::setView(CameraIntercomDlg *dlg)
{
	this->dlg = dlg;
}

void CameraDialogController::getWindowText(CWnd &wnd, string &copy)
{
	// Allocate a buffer:
	LPTSTR str = new TCHAR[2048];
	
	if(!isClosing)
	{
		// copy it out of the window object, and assign it to a std::string:
		wnd.GetWindowText(str, 2048);
	}

	ostringstream out;

	out << str;

	copy = out.str().c_str();

	// cleanup
	delete str;
}

void CameraDialogController::testButtonClicked()
{
	if(!isRunning)
	{
		if(!isClosing)
		{
			//---------------------------------------------------------------------
			// Case 1 - Camera Thread is not running:
			//---------------------------------------------------------------------
			// 1. Grab options from view
			string intercom, url, username, password;
			getWindowText(dlg->getTxtIntercom(), intercom);
			getWindowText(dlg->getTxtUrl(), url);
			getWindowText(dlg->getTxtUsername(), username);
			getWindowText(dlg->getTxtPassword(), password);

			//CameraData tmp(intercom, url, username, password);
			CameraData *tmp = new CameraData(intercom, url, username, password);
			
			// 2. Spawn thread
			thread = JpegCameraFeed::registerVideoFeed(&(dlg->getCameraPreview()), tmp);

			// 3. Thread updates view infinitely (until interrupted).
			// this step is done via a callback mechanism (go see the 
			// eventOccurred function of the JpegCameraThread.cpp)

			// 4. Set text of "Test" button to "Stop
			dlg->getTestButton().SetWindowText("Stop");		
			isRunning = true;
		}
	} else
	{
		//---------------------------------------------------------------------
		// Case 2 - Camera thread is running, user clicks "Stop" (test button)
		//---------------------------------------------------------------------
		// 1.  Terminate running thread
		thread->KillThread();
		
		// 2.  Blank out image
		dlg->SetRedraw(TRUE);

		if(!isClosing)
		{
			// 3.  Set text of "Test" button back to "Test"
			dlg->getTestButton().SetWindowText("Test");
			isRunning = false;
		}
	}
}

void CameraDialogController::onOk()
{
	if(isRunning)
	{
		testButtonClicked();
	}

	string url, username, password;
	getWindowText(dlg->getTxtUrl(), url);
	getWindowText(dlg->getTxtUsername(), username);
	getWindowText(dlg->getTxtPassword(), password);

	data->setUrl(url);
	data->setUsername(username);
	data->setPassword(password);
	CameraDataManager::getInstance().addCameraData(data);

	isClosing = true;
}

void CameraDialogController::onCancel()
{
	if(isRunning)
	{
		testButtonClicked();
	}

	isClosing = true;
}

void CameraDialogController::refresh()
{
	if(!isClosing)
	{
		dlg->getTxtIntercom().SetWindowText(data->getIntercom().c_str());
		dlg->getTxtUrl().SetWindowText(data->getUrl().c_str());
		dlg->getTxtUsername().SetWindowText(data->getUsername().c_str());
		dlg->getTxtPassword().SetWindowText(data->getPassword().c_str());
	}
}

void CameraDialogController::setModel(CameraData *model)
{
	this->data = model;
}

CameraData* CameraDialogController::getModel()
{
	return this->data;
}
