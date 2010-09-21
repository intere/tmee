#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>

#include "MjpegEvent.h"
#include "Observable.h"

#define CURL_DEBUG 1

using namespace std;

namespace mjpeg
{
	namespace stream
	{
		static const string BOUNDARY = "--myboundary\r\n";
		static const string CONTENT_TYPE = "Content-Type: ";
		static const string CONTENT_LEN = "Content-Length: ";
		static const string CR_LF = "\r\n";
	}

	namespace file
	{
		static const string TEMP_DIR = "C:\\tmp";
		static const string SEPARATOR = "\\";
		static const string BASE_NAME = "mjpeg_file";
		static const string SUFFIX = ".jpg";
	}
};

/**
 * The purpose of this class is to extend the ostream object.
 */
class MJpegStream : public ostream, public Observable
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

	/** Default Constructor */
	MJpegStream(void);

	/** Destructor.  */
	virtual ~MJpegStream(void);

	/** Overloaded put function.  */
	virtual ostream& put(char c);

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

private:

	/** The vector of files that we're creating.  */
	vector<string> jpegs;

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
