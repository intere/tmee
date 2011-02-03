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

	enum EventType
	{
		NEW_FILE,
		INVALID_STREAM
	};

	/** Private default Constructor.  */
	MJpegEvent();

	/** Constructor that sets the Bytes and Filename. */
	MJpegEvent(long bytes, string filename);

	/** Destructor.  
	virtual ~MJpegEvent(void);*/
	
	/** Getter for the number of bytes in the file. */
	long getBytes(void) const;
	
	/** Getter for the Filename. */
	string getFilename() const;

	/** Getter for the Event Type.  */
	EventType getEventType();

	static MJpegEvent createInvalidStreamEvent();
	
private:

	long bytes;
	string filename;
	EventType eventType;
};
