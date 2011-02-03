
#include "stdafx.h"
#include "MJpegEvent.h"

MJpegEvent::MJpegEvent()
{

}

MJpegEvent::MJpegEvent(long bytes, string filename)
{
	this->bytes = bytes;
	this->filename = filename;
	this->eventType = NEW_FILE;
}

long MJpegEvent::getBytes(void) const
{
	return bytes;
}

string MJpegEvent::getFilename(void) const
{
	return filename;
}

MJpegEvent::EventType MJpegEvent::getEventType()
{
	return eventType;
}

MJpegEvent MJpegEvent::createInvalidStreamEvent()
{
	MJpegEvent evt;
	evt.eventType = INVALID_STREAM;

	return evt;
}