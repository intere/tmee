#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>

#include "MjpegEvent.h"
#include "Observable.h"
#include "VideoFeedThread.h"

#define CURL_DEBUG 1

using namespace std;

class JpegCameraThread;



/**
 * The purpose of this class is to extend the ostream object.
 */
class MJpegStream : public Observable
{
public:

	/** Enumeration to keep track of the state.  */
	enum State
	{
		ReadingHeader,  // We're reading the header
		CreatingFile,	// We're creating the JPEG File.
		ReadingContent, // We're reading the content
		PostContent
	};

	/** Enumeration of Header states.  */
	enum HeaderState
	{
		Boundary,
		ContentType,
		ContentLength,
		CrLf
	};

	/** Constructor that sets the JpegCamerThread.  */
	MJpegStream(JpegCameraThread* thread);

	/** Default Constructor */
	MJpegStream(VideoFeedThread* thread);

	/** Destructor.  */
	virtual ~MJpegStream(void);

	/** Overloaded put function.  */
	virtual MJpegStream& put(char c);

protected:

	/** Resets the header to its initial state.  */
	void resetHeaderState(void);

	/** Worker function that will manage the state of the header parsing.  */
	void parseHeaderField(char &c);

	/** Helper function that will transition the header parameter for you.  */
	void helpParseHeaderField(char &c);

	/** Helper function that creates the file and begins the content reading.  */
	void createNewFile(char &c);

	/** Helper function that will read the content.  */
	void readContent(char &c);

	/** Setter for the state.  */
	void setState(State state);

	/** Setter for the Header State. */
	void setHeaderState(HeaderState hState);

	/** Sends the MJpegEvent out for you.  */
	void sendMJpegEvent(MJpegEvent *evt);

private:

	/** The VideoFeedThread to be notified.  */
	VideoFeedThread *vThread;

	/** The JpegCamerThread to be notified.  */
	JpegCameraThread *jThread;

	/** The content length of the current JPEG from the M-JPEG stream.  */
	long contentLength;

	/** The number of bytes that have been read in the content thus far.  */
	long bytesRead;

	/** The Index into the current header string that we're at.  */
	size_t headerStringIndex;

	/** The state of the Stream.  */
	State state;

	/** The Header state of the stream.  */
	HeaderState hState;

	/** The Header state that we're reading a parameter for.  */
	HeaderState paramState;

	/** The ostringstream to temporarily hold the parameter we're parsing in.  */
	ostringstream tmpstream;

	/** The current file that we're writing to.  */
	string filename;

	/** The output file stream - for writing jpegs.  */
	ofstream jpegstream;

	/** The file index.  */
	long fileIndex;

	/** Was the last character a Carriage Return?.  */
	bool lastIsCr;

#if CURL_DEBUG==1
	/** Debugging - we will reset after the resetCount hits a certain threshold.  */
	long resetCount;
#endif
};
