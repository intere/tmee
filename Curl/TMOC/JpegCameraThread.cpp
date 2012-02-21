// JpegCameraThread.cpp : implementation file
//

#include "stdafx.h"
#include "curl/curl.h"
#include "TalkMasterConsole.h"
#include "JpegCameraThread.h"

#include <sstream>
#include <string>

using namespace std;
using namespace JpegCameraFeed;



/** 
 * The namespace that is used to manage the C-Style handling of the 
 * Video Feed.  (curl is a C-library).
 */
namespace JpegCameraFeed
{
	//static JpegCameraThread *thread = NULL;
	static CURL *curl = NULL;	
	
	size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {

		ostringstream buff;
		buff << "read (" << size << ", " << nmemb << ") bytes" << std::endl;
		std::basic_string<TCHAR> out = buff.str().c_str();
		TRACE(out.c_str());
		
		for(size_t i=0; i<nmemb; ++i)
		{
			if(JpegCameraThread::getThread()->isStopReading())
			{
				break;
			}
			JpegCameraThread::getThread()->getStream()->put(((char*)ptr)[i]);
		}
		
		if(JpegCameraThread::getThread()->isStopReading())
		{
			return -1;
		}
		
		return nmemb;
	}

	/**
	* This function will cleanup existing video feeds, and register
	* the provided VideoFeedThread with this namespace.  Then it will
	* ask the new feed thread to resume.
	*/
	JpegCameraThread* registerVideoFeed(CWnd *m_cameraPreview, CameraData *data)
	{
		// Cleanup old CURL object:
		if(curl)
		{
			curl_easy_pause(curl, CURLPAUSE_ALL);
			curl_easy_cleanup(curl);
			curl = NULL;
		}

		// Cleanup old Thread
		if(JpegCameraThread::getThread())
		{
			JpegCameraThread::getThread()->KillThread();
		}

		// Set the new references
		JpegCameraThread *thread = new JpegCameraThread();
		thread->CreateThread(CREATE_SUSPENDED);
		thread->setPreviewWindow(m_cameraPreview);
		thread->setCameraData(data);
		thread->ResumeThread();

		return thread;
	}
};

// JpegCameraThread

IMPLEMENT_DYNCREATE(JpegCameraThread, CWinThread)

/** Static in itialization of the singleton reference.  */
JpegCameraThread* JpegCameraThread::thread = NULL;

/** Default Constructor.  */
JpegCameraThread::JpegCameraThread() : stopReading(false)
{
	JpegCameraThread::thread = this;
	stream = new MJpegStream(this);
	stream->addListener(static_cast<Listener>(*this));
	m_Thumbnail = NULL;

	mBitMapBlank.LoadBitmap(IDB_BITMAP_VU_BLANK);
}

/** Destructor.  */
JpegCameraThread::~JpegCameraThread()
{	
	if(stream)
	{
		delete stream;
		stream = NULL;
	}

	thread = NULL;
}

/** Yes; we want to run this thread!.  */
BOOL JpegCameraThread::InitInstance()
{
	return TRUE;
}

/**
 * This function is what signals the thread to die; it works by
 * setting the "stopReading" flag to true and this causes the
 * JpegCameraFeed::write_data(void *ptr, size_t size, size_t nmemb, FILE *stream)
 * to break out and return -1.
 */
void JpegCameraThread::KillThread()
{
	this->stopReading = true;
}

/**
 * Thread Runner function.
 */
int JpegCameraThread::Run()
{
	curl = curl_easy_init();
	CURLcode res;

	// Configure CURL from the thread:
	if(curl)
	{
		if(thread->getData()->getUsername().size()>0)
		{
			curl_easy_setopt(curl, CURLOPT_USERNAME, 
				thread->getData()->getUsername().c_str());

			if(thread->getData()->getPassword().size()>0)
			{
				curl_easy_setopt(curl, CURLOPT_PASSWORD,
					thread->getData()->getPassword().c_str());
			}
		}

		curl_easy_setopt(curl, CURLOPT_URL, 
			thread->getData()->getUrl().c_str());

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, JpegCameraFeed::write_data);
		res = curl_easy_perform(curl);
		
		/* always cleanup */
		curl_easy_cleanup(curl);
		curl = NULL;
	}

	return 0;
}

/**
 * Overridden delete function.
 */
void JpegCameraThread::Delete()
{
	CWinThread::Delete();
}

int JpegCameraThread::ExitInstance()
{
	return CWinThread::ExitInstance();
}

/**
 * Event handler - new JPEG Image received.
 */
void JpegCameraThread::eventOccurred(MJpegEvent *event)
{
	loadImage(event->getFilename());
	drawPreview();

	if(lastImage.size()>0)
	{
		try
		{
			CFile::Remove(event->getFilename().c_str());
		} catch(...)
		{
			cerr << "Error removing file: " 
				<< event->getFilename().c_str() << endl;
		}
	}

	lastImage = event->getFilename().c_str();
}

void JpegCameraThread::setCameraData(CameraData *data)
{
	this->data = data;
}

void JpegCameraThread::setPreviewWindow(CWnd *previewWindow)
{
	this->previewWindow = previewWindow;
}

JpegCameraThread* JpegCameraThread::getThread()
{
	return JpegCameraThread::thread;
}

MJpegStream* JpegCameraThread::getStream()
{
	return this->stream;
}

CameraData* JpegCameraThread::getData()
{
	return this->data;
}

CWnd* JpegCameraThread::getPreviewWindow()
{
	return this->previewWindow;
}

bool JpegCameraThread::isStopReading()
{
	return this->stopReading;
}

void JpegCameraThread::loadImage(std::string jpeg)
{
	USES_CONVERSION;

	CString cstrWidth, cstrHeight, cstrType;
	UINT uiWidth, uiHeight;
	UINT thumbWidth, thumbHeight;
	int iCount = 0;
	double dRatio;

	RECT rect;
	BOOL bExists = TRUE;

	std::basic_string<TCHAR> buff = jpeg.c_str();

	// Make sure the image exists
	CFile file;
	if (file.Open(buff.c_str(), CFile::modeRead))
	{
		file.Close();				
	} else
	{
		bExists = FALSE;
	}

	previewWindow->GetClientRect(&rect);
	rect.left += 1;
	rect.top += 1;
	rect.right -= 1;
	rect.bottom -= 1;

	if (bExists)
	{
		// Use the Image class to display a thumbnail of the image.
	    Image image(T2CW(buff.c_str()));

		// Determine the appropriate size of the thumbnail preview
		// given the image size ratio and the preview window size ratio.
		uiWidth = image.GetWidth();
		uiHeight = image.GetHeight();
		dRatio = ((double)uiWidth)/((double)uiHeight);

		// If the width is larger than the height of the image, 
		// set the width of the thumbnail to the width of the preview area
		// and calculate the thumbnail height by using the ratios.
		// If the height is larger than the width of the image, 
		// set the height of the thumbnail to the height of the preview area
		// and calculate the thumbnail width by using the ratios.
		if (uiWidth > uiHeight)
		{
			thumbWidth = rect.right - rect.left;
			thumbHeight = (UINT)(thumbWidth/dRatio);

			if (thumbHeight == 0) thumbHeight = 1;
			if (thumbHeight > (UINT)(rect.bottom - rect.top)) thumbHeight = rect.bottom - rect.top;
		}
		else
		{
			thumbHeight = rect.bottom - rect.top;
			if(uiHeight > 0)
			{
				thumbWidth = (UINT)(uiWidth*thumbHeight/uiHeight);
			} else {
				thumbWidth = rect.right - rect.left;
			}
			if (thumbWidth == 0) thumbWidth = 1;
			if (thumbWidth > (UINT)(rect.right - rect.left)) thumbWidth = rect.right - rect.left;
		}

		// Cleanup - so we don't blow up the heap:
		if(m_Thumbnail)
		{
			try
			{
				delete m_Thumbnail;
			} catch( ... ) {
				cerr << "Error cleaning up thumbnail image" << endl;
			}
		}

		// Get the thumbnail and display it in the preview control.
		m_Thumbnail = image.GetThumbnailImage(thumbWidth, thumbHeight, NULL, NULL);

		// Display the image bits per pixel (color depth).
		switch (image.GetPixelFormat())
		{
		case PixelFormat1bppIndexed:
			cstrType = "Type: 1bpp";
			break;

		case PixelFormat4bppIndexed:
			cstrType = "Type: 4bpp";
			break;

		case PixelFormat8bppIndexed:
			cstrType = "Type: 8bpp";
			break;

		case PixelFormat16bppARGB1555: 
		case PixelFormat16bppGrayScale: 
		case PixelFormat16bppRGB555: 
		case PixelFormat16bppRGB565:
			cstrType = "Type: 16bpp";
			break;

		case PixelFormat24bppRGB:
			cstrType = "Type: 24bpp";
			break;

		case PixelFormat32bppARGB: 
		case PixelFormat32bppPARGB: 
		case PixelFormat32bppRGB: 
			cstrType = "Type: 32bpp";
			break;

		case PixelFormat48bppRGB:
			cstrType = "Type: 48bpp";
			break;

		case PixelFormat64bppARGB: 
		case PixelFormat64bppPARGB:
			cstrType = "Type: 64bpp";
			break;

		default:
			cstrType = "Type: Unknown";
			break;
		}
		//m_stType.SetWindowText(cstrType);
		
	}
}

void JpegCameraThread::drawPreview()
{
	USES_CONVERSION;

	CDC dcMemory, *pDC;
	RECT rect;
	BITMAP bm;

	mBitMapBlank.GetBitmap(&bm);

	// Initialize the Graphics class in GDI+.
	pDC = previewWindow->GetWindowDC();

	// Fill the preview ara with a WHITE background.
	previewWindow->GetClientRect(&rect);
	rect.left += 1;
	rect.top += 1;
	rect.right -= 1;
	rect.bottom -= 1;
	pDC->FillSolidRect(&rect, RGB(255,255,255));

	if(m_Thumbnail)
	{
		Graphics graphics(pDC->m_hDC);

		if(m_Thumbnail!=NULL)
		{
			graphics.DrawImage(m_Thumbnail, 1, 1, m_Thumbnail->GetWidth(), m_Thumbnail->GetHeight());
		}
	} else
	{
		TRACE("NO thumbnail image to render");
	}

	// Cleanup
	if (pDC) pDC->ReleaseOutputDC();
}


BEGIN_MESSAGE_MAP(JpegCameraThread, CWinThread)
END_MESSAGE_MAP()


// JpegCameraThread message handlers
