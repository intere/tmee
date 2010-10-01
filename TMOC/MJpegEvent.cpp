
#include "stdafx.h"
#include "MJpegEvent.h"

MJpegEvent::MJpegEvent(long bytes, string filename)
{
	this->bytes = bytes;
	this->filename = filename;
}

long MJpegEvent::getBytes(void) const
{
	return bytes;
}

string MJpegEvent::getFilename(void) const
{
	return filename;
}