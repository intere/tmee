#pragma once

#include "CameraData.h"
#include "Listener.h"
#include "CameraData.h"
#include "MJpegStream.h"

#include <objidl.h>
#include <gdiplus.h>
#include <string>
using namespace Gdiplus;
#pragma comment (lib,"Gdiplus.lib")

namespace JpegCameraFeed
{
	size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream);
	JpegCameraThread* registerVideoFeed(CWnd *m_cameraPreview, CameraData *data);
};

// JpegCameraThread

class JpegCameraThread : public CWinThread, public Listener
{
	DECLARE_DYNCREATE(JpegCameraThread)

public:
	JpegCameraThread();           // protected constructor used by dynamic creation
	virtual ~JpegCameraThread();
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	virtual int Run();
	virtual void Delete();

	void KillThread();

	/** Handles an MJpegEvent (new image available).  */
	void eventOccurred(MJpegEvent *event);

	/** Sets the Camera Data.  */
	void setCameraData(CameraData *data);

	/** Sets the Preview Window Handle.  */
	void setPreviewWindow(CWnd *previewWindow);

	/** Getter for the stream.  */
	MJpegStream* getStream();

	/** Getter for the Data.  */
	CameraData* getData();

	/** Getter for the Preview Window.  */
	CWnd* getPreviewWindow();

	/** Should the reading continue?  */
	bool isStopReading();

	/** Get the static instance.  */
	static JpegCameraThread *getThread();

protected:
	/** Helper function which will load the image.  */
	void loadImage(string jpeg);

	/** Helper function that will draw the preview.  */
	void drawPreview();

	/** Static thread instance.  */
	static JpegCameraThread *thread;


public:		// Attributes
	HANDLE m_hEventKill;
	HANDLE m_hEventDead;

private:
	CBitmap	mBitMapBlank;
	CameraData *data;

	/** 
	 * Reference to the Image control (down-cast to a CWnd) that 
	 * render the image within.
	 */
	CWnd *previewWindow;

	/** The MJpegStream that we're getting messages back from.  */
	MJpegStream *stream;

	/** The current thumbnail image. */
	Image* m_Thumbnail;				/* Thumbnail Image.  */

	int m_ThumbWidth;	// width of the thumbnail image
	int m_ThumbHeight;	// height of the thumbnail image.


	/** Reference to the last image we read.  */
	string lastImage;

	/** Should we stop reading from the HTTP Stream? */
	bool stopReading;

	//static map<int, JpegCameraThread*> threads;

protected:
	DECLARE_MESSAGE_MAP()
};


