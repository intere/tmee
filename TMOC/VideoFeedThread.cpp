// VideoFeedThread.cpp : implementation file
//

#include "stdafx.h"

#include "VideoFeedThread.h"
#include "MJpegStream.h"
#include "TalkMasterConsoleDlg.h"
#include "curl/curl.h"

#include <sstream>
#include <string>
#include <iostream>

using namespace VideoFeed;

/** 
 * The namespace that is used to manage the C-Style handling of the 
 * Video Feed.  
 */
namespace VideoFeed
{
	static VideoFeedThread *thread = NULL;
	static CURL *curl = NULL;
	static int count;

	size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {

		ostringstream buff;
		buff << "read (" << size << ", " << nmemb << ") bytes" << std::endl;
		std::basic_string<TCHAR> out = buff.str().c_str();
		TRACE(out.c_str());
		
		for(size_t i=0; i<nmemb; ++i)
		{
			VideoFeedThread::getThread()->getStream()->put(((char*)ptr)[i]);
		}
    
		return nmemb;
	}

	/**
	* This function will cleanup existing video feeds, and register
	* the provided VideoFeedThread with this namespace.  Then it will
	* ask the new feed thread to resume.
	*/
	void registerVideoFeed(CTalkMasterConsoleDlg *m_cameraPreview, CameraData* data)
	{
		// Cleanup old CURL object:
		if(curl)
		{
			curl_easy_pause(curl, CURLPAUSE_ALL);
			curl_easy_cleanup(curl);
		}

		// Cleanup old Thread
		if(thread)
		{
			thread->Finish();
		}

		// Set the new references
		thread = new VideoFeedThread();
		thread->CreateThread(CREATE_SUSPENDED);
		thread->setPreviewWindow(m_cameraPreview);
		thread->setUsername(data->getUsername());
		thread->setPassword(data->getPassword());
		thread->setUrl(data->getUrl());
		thread->ResumeThread();
	}
};


// VideoFeedThread

IMPLEMENT_DYNCREATE(VideoFeedThread, CWinThread)

VideoFeedThread::VideoFeedThread()
{
	stream = new MJpegStream(this);
	stream->addListener(static_cast<Listener>(*this));
	thread = this;
}

VideoFeedThread::~VideoFeedThread()
{
	if(stream)
	{
		delete stream;
	}
}

BOOL VideoFeedThread::InitInstance()
{
	// TODO:  perform and per-thread initialization here
	return TRUE;
}

int VideoFeedThread::ExitInstance()
{
	// TODO:  perform any per-thread cleanup here
	return CWinThread::ExitInstance();
}

int VideoFeedThread::Run()
{
	curl = curl_easy_init();
	CURLcode res;

	// Configure CURL from the thread:
	if(curl)
	{
		if(thread->getUsername().size()>0)
		{
			curl_easy_setopt(curl, CURLOPT_USERNAME, 
				thread->getUsername().c_str());

			if(thread->getPassword().size()>0)
			{
				curl_easy_setopt(curl, CURLOPT_PASSWORD,
					thread->getPassword().c_str());
			}
		}

		curl_easy_setopt(curl, CURLOPT_URL, 
			thread->getUrl().c_str());

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, VideoFeed::write_data);
		res = curl_easy_perform(curl);
		
		/* always cleanup */
		curl_easy_cleanup(curl);
	}

	return 0;
}

void VideoFeedThread::Finish()
{
	AfxEndThread(0, TRUE);
}

string VideoFeedThread::getUsername()
{
	return this->username;
}

void VideoFeedThread::setUsername(string username)
{
	this->username = username;
}

string VideoFeedThread::getPassword()
{
	return this->password;
}

void VideoFeedThread::setPassword(string password)
{
	this->password = password;
}

string VideoFeedThread::getUrl()
{
	return this->url;
}

void VideoFeedThread::setUrl(string url)
{
	this->url = url;
}

void VideoFeedThread::setPreviewWindow(CTalkMasterConsoleDlg *m_cameraPreview)
{
	this->m_cameraPreview = m_cameraPreview;
}

VideoFeedThread *VideoFeedThread::getThread()
{
	return VideoFeed::thread;
}

MJpegStream *VideoFeedThread::getStream()
{
	return this->stream;
}

void VideoFeedThread::eventOccurred(MJpegEvent *event)
{
	m_cameraPreview->setImage(event->getFilename());
	m_cameraPreview->doRender();

	if(lastImage.size()>0)
	{
		try
		{
			CFile::Remove(lastImage.c_str());	
		//} catch(CFileException* ex)
		}catch(...)
		{
			cerr << "Unable to remove file: " << lastImage.c_str() << endl;
		}
	}

	this->lastImage = event->getFilename().c_str();
}


BEGIN_MESSAGE_MAP(VideoFeedThread, CWinThread)
END_MESSAGE_MAP()


// VideoFeedThread message handlers
