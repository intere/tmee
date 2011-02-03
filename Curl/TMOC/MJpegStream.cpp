#include "StdAfx.h"
#include "MJpegStream.h"
#include "Observable.h"
#include "MJpegEvent.h"
#include "JpegCameraThread.h"
#include "CameraDataManager.h"

namespace mjpeg
{
	namespace stream
	{
		static const string BOUNDARY = "--myboundary\r\n";
		static const string CONTENT_TYPE = "Content-Type: ";
		static const string CONTENT_LEN = "Content-Length: ";
		static const string CR_LF = "\r\n";
		static const char CR = '\r';
		static const char LF = '\n';
	}

	namespace file
	{
		//static const string TEMP_DIR = "C:\\tmp";
		//static const string TEMP_DIR = CameraDataManager::getInstance().getOptions()->pathTemp;
		static const string SEPARATOR = "\\";
		static const string BASE_NAME = "mjpeg_file";
		static const string SUFFIX = ".jpg";
	}
};

MJpegStream::MJpegStream(JpegCameraThread* thread) : Observable()
{	
	this->vThread = NULL;
	this->jThread = thread;
	contentLength = 0;
	bytesRead = 0;
	setState(ReadingHeader);
	setHeaderState(Boundary);
	lastIsCr = false;
	headerStringIndex = 0;
	fileIndex = 1;

#if CURL_DEBUG==1
	resetCount=0;
#endif
}

MJpegStream::MJpegStream(VideoFeedThread* thread) : Observable()
{
	this->vThread = thread;
	this->jThread = NULL;
	contentLength = 0;
	bytesRead = 0;
	setState(ReadingHeader);
	setHeaderState(Boundary);
	lastIsCr = false;
	headerStringIndex = 0;
	fileIndex = 1;

#if CURL_DEBUG==1
	resetCount=0;
#endif
}

MJpegStream::~MJpegStream(void)
{
	if(jpegstream.is_open())
	{
		jpegstream.close();
		CFile::Remove(filename.c_str());
	}

}

/**
 * This is where the finite state machine begins.  We look at the state
 * of the MJpegStream before we determine where to delegate to...
 */
MJpegStream& MJpegStream::put(char c)
{
	switch(state)
	{

	// Reads the Header
	case ReadingHeader:
		{
			parseHeaderField(c);
			break;
		}

	// Creates the JPEG File
	case CreatingFile:
		{
			state = ReadingContent;
			return put(c);
		}

	// Reads the conent
	case ReadingContent:
		{
			readContent(c);
			break;
		}

	// Reads the Post Content: \r\n
	case PostContent:
		{
			++headerStringIndex;
			if(headerStringIndex==2)
			{
				setState(ReadingHeader);
				resetHeaderState();
			}
			break;
		}

	default:
		{
			TRACE("Unknown State");
			break;
		}
	}

	return (*this);
}

/**
 * Resets the header to its initial state.
 */
void MJpegStream::resetHeaderState(void)
{
	setHeaderState(Boundary);
	headerStringIndex = 0;
}

/**
 * Worker function that will manage the state of the header parsing.
 */
void MJpegStream::parseHeaderField(char &c)
{
	if(ReadingHeader!=state)
	{
		return;
	}
	
	string param;
	switch(hState)
	{
	case Boundary:
		{
			param = mjpeg::stream::BOUNDARY;
			break;
		}

	case ContentType:
		{
			param = mjpeg::stream::CONTENT_TYPE;
			break;
		}

	case ContentLength:
		{
			param = mjpeg::stream::CONTENT_LEN;
			break;
		}

	case CrLf:
		{
			param = mjpeg::stream::CR_LF;
			break;
		};

	default:
		{
			cerr << "Unknown Header State" << endl;
			return;
		}
	}

	if(param.size()==0)
	{
		cerr << "No param to parse; bailing" << endl;
		return;
	}

	if(headerStringIndex==param.size()-1)
	{
#if CURL_DEBUG==1
		resetCount = 0;
#endif
		helpParseHeaderField(c);
	}
    else if(param.at(headerStringIndex)==c)
	{
#if CURL_DEBUG==1
		resetCount = 0;
#endif
		++headerStringIndex;
	} else
	{
		headerStringIndex = 0;
#if CURL_DEBUG==1
		++resetCount;
#endif
		cerr << "Resetting the string index" << endl;

#if CURL_DEBUG==1
		if(resetCount>10)
		{
			cerr << "problems" << endl;
		}
#endif
	}
}

/**
 * Helper function that will transition the header parameter for you.
 */
void MJpegStream::helpParseHeaderField(char &c)
{
	switch(hState)
	{
	case Boundary:
		{
			setHeaderState(ContentType);
			headerStringIndex = 0;
			return;
		}

	case ContentType:
	case ContentLength:
		{
			break;
		}

	case CrLf:
		{
			setState(CreatingFile);
			createNewFile(c);
			break;
		}

	default:
		{
			cerr << "Unknown state" << endl;
			return;
		}
	}

	if(c==mjpeg::stream::CR)
	{
		lastIsCr = true;
	} else if(lastIsCr && c==mjpeg::stream::LF)
	{
		lastIsCr = false;
		switch(hState)
		{
		case ContentType:
			{
				tmpstream.str("");
				headerStringIndex = 0;
				setHeaderState(ContentLength);
				break;
			}

		case ContentLength:
			{
				contentLength = atol(tmpstream.str().c_str());
				bytesRead = 0;
				headerStringIndex = 0;
				setHeaderState(CrLf);
				tmpstream.str("");
				break;
			}
		}
	} else 
	{
		lastIsCr = false;
		tmpstream << c;
	}
}

/**
 * Helper function that creates the file and begins the content reading.
 */
void MJpegStream::createNewFile(char &c)
{	
	ostringstream fbuilder;

	// Build the string that contains the filename:
	fbuilder << CameraDataManager::getInstance().getOptions()->pathTemp
		<< mjpeg::file::SEPARATOR 
		<< mjpeg::file::BASE_NAME
		<< fileIndex 
		<< mjpeg::file::SUFFIX;

	filename = fbuilder.str();

#if CURL_DEBUG==1

	ostringstream out;
	out << "Writing to file: " << filename.c_str() << " - " 
		<< contentLength << " bytes" << endl;

	TRACE(out.str().c_str());

#endif

	jpegstream.open(filename.c_str(), std::ios_base::binary);

	setState(ReadingContent);

	++fileIndex;
}

/**
 * Helper function that reads the content into a file.
 */
void MJpegStream::readContent(char &c)
{
	jpegstream.put(c);
	++bytesRead;

	if(bytesRead==contentLength)
	{
		jpegstream.flush();
		jpegstream.close();
		state = PostContent;
		resetHeaderState();

#if CURL_DEBUG==1
		ostringstream oss;
		oss << "Read " << bytesRead << " bytes" << endl;

		TRACE(oss.str().c_str());
#endif

		MJpegEvent *evt = new MJpegEvent(bytesRead, filename);
		// Alert Listeners always delegates to the base implementation of the
		// listener interface for some reason...
        //alertListeners(evt);
		if(NULL!=vThread)
		{
			vThread->eventOccurred(evt);
		}
		if(NULL!=jThread)
		{
			jThread->eventOccurred(evt);
		}

		delete evt;
	}
}

/** 
 * Setter for the state.  
 */
void MJpegStream::setState(State state)
{
	this->state = state;

#if CURL_DEBUG==1
	// Debug Printing:
	TRACE("Set State to: ");
	switch(state)
	{
	case ReadingHeader:
		{
			TRACE("ReadingHeader");
			break;
		}

	case CreatingFile:
		{
			TRACE("CreatingFile");
			break;
		}

	case ReadingContent:
		{
			TRACE("ReadingContent");
			break;
		}

	case PostContent:
		{
			TRACE("PostContent");
			break;
		}
	}

	TRACE("\n");
#endif

}

/**
 * Setter for the Header State.
 */
void MJpegStream::setHeaderState(HeaderState hState)
{
	this->hState = hState;

#if CURL_DEBUG==1
	// Debugging output:
	TRACE("Set Header State to: ");
	
	switch(hState)
	{
	case Boundary:
		{
			TRACE("Boundary");
			break;
		}

	case ContentType:
		{
			TRACE("ContentType");
			break;
		}

	case ContentLength:
		{
			TRACE("ContentLength");
			break;
		}

	case CrLf:
		{
			TRACE("CrLf");
			break;
		}
	}

	TRACE("\n");
#endif
}