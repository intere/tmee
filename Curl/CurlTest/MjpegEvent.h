#pragma once

#include <string>

using namespace std;

/**
 * This class is an event type that is used to notify Observers
 * of an Event.
 */
class MJpegEvent
{
public:

	/** Constructor that sets the Bytes and Filename. */
	MJpegEvent(long bytes, string filename);

	/** Destructor.  */
	~MJpegEvent(void);
	
	/** Getter for the number of bytes in the file. */
	long getBytes(void) const;
	
	/** Getter for the Filename. */
	string getFilename() const;
	
private:
	long bytes;
	string filename;
};
