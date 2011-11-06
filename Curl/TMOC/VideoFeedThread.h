#pragma once

#include "curl/curl.h"
#include "Listener.h"
#include "MJpegEvent.h"
#include "CameraData.h"
#include <string>

using namespace std;

class MJpegStream;
class CTalkMasterConsoleDlg;

namespace VideoFeed
{
	size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream);
	void registerVideoFeed(CTalkMasterConsoleDlg *m_cameraPreview, CameraData* data);
	void stopVideoFeed();
};

// VideoFeedThread

class VideoFeedThread : public CWinThread, public Listener
{
	DECLARE_DYNCREATE(VideoFeedThread)

public:
	VideoFeedThread();           // protected constructor used by dynamic creation
	virtual ~VideoFeedThread();

public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	virtual int Run();

	void eventOccurred(MJpegEvent *event);

	/** Call to terminate early.  */
	void Finish();

	/** Setter for the Camera Preview Window.  */
	void setPreviewWindow(CTalkMasterConsoleDlg *m_cameraPreview);
	
	string getUsername();
	void setUsername(std::string username);

	string getPassword();
	void setPassword(std::string password);

	string getUrl();
	void setUrl(string url);

	MJpegStream *getStream();

	static VideoFeedThread *getThread();
	bool isStopReading();
	
protected:
	DECLARE_MESSAGE_MAP()
	/** Static thread instance.  */
	static VideoFeedThread *thread;
	bool stopReading;
	string username;
	string password;
	string url;
	string lastImage;
	CTalkMasterConsoleDlg *m_cameraPreview;
	MJpegStream *stream;
};

