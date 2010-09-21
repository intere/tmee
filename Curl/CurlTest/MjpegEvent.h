#pragma once

#include <string>

using namespace std;

/**
 * This class is an event type that is used to notify Observers
 * of an Event.
 */
class MjpegEvent
{
public:

	/**
	 * Constructor that sets the Bytes and Filename.
	 */
	inline MjpegEvent(long bytes, string filename)
	{
		this->bytes = bytes;
		this->filename = filename;
	}

	inline virtual ~MjpegEvent(void)
	{
	}

	/**
	 * Getter for the number of bytes in the file.
	 */
	inline public long getBytes() const
	{
		return bytes;
	}

	/**
	 * Getter for the Filename.
	 */
	inline public string getFilename() const
	{
		return filename;
	}

private:
	long bytes;
	string filename;
};
