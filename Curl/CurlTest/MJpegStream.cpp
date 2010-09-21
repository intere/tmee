#include "StdAfx.h"
#include "mjpegstream.h"
#using <mscorlib.dll>

// This is the "boundary" string; it tells you when you're beginning a new boundary.

MJpegStream::MJpegStream(void) : ostream(_Noinit)
{
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

}

/**
 * This is where the finite state machine begins.  We look at the state
 * of the MJpegStream before we determine where to delegate to...
 */
ostream& MJpegStream::put(char c)
{
	switch(state)
	{
	case ReadingHeader:
		{
			parseHeaderField(c);
			break;
		}

	case CreatingFile:
		{
			state = ReadingContent;
			return put(c);
		}

	case ReadingContent:
		{
			readContent(c);
			break;
		}

	case PostContent:
		{
			++headerStringIndex;
			//cout << "post content; read character: " << (int)c << endl;

			if(headerStringIndex==2)
			{
				setState(ReadingHeader);
				resetHeaderState();
			}
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
			exit(-1);
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

	if(c=='\r')
	{
		lastIsCr = true;
	} else if(lastIsCr && c=='\n')
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
	fbuilder << mjpeg::file::TEMP_DIR 
		<< mjpeg::file::SEPARATOR 
		<< mjpeg::file::BASE_NAME
		<< fileIndex 
		<< mjpeg::file::SUFFIX;

	filename = fbuilder.str();

#if CURL_DEBUG==1
	cout << "Writing to file: " << filename << " - " 
		<< contentLength << " bytes" << endl;
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
	// jpegstream << c;
	jpegstream.put(c);

	++bytesRead;

	if(bytesRead==contentLength)
	{
		jpegstream.flush();
		jpegstream.close();
		state = PostContent;
		resetHeaderState();

#if CURL_DEBUG==1
		cout << "Read " << bytesRead << " bytes" << endl;
#endif

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
	cout << "Set State to: ";
	switch(state)
	{
	case ReadingHeader:
		{
			cout << "ReadingHeader";
			break;
		}

	case CreatingFile:
		{
			cout << "CreatingFile";
			break;
		}

	case ReadingContent:
		{
			cout << "ReadingContent";
			break;
		}

	case PostContent:
		{
			cout << "PostContent";
			break;
		}
	}

	cout << endl;
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
	cout << "Set Header State to: ";
	
	switch(hState)
	{
	case Boundary:
		{
			cout << "Boundary";
			break;
		}

	case ContentType:
		{
			cout << "ContentType";
			break;
		}

	case ContentLength:
		{
			cout << "ContentLength";
			break;
		}

	case CrLf:
		{
			cout << "CrLf";
			break;
		}
	}

	cout << endl;
#endif
}