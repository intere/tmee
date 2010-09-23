
#include "stdafx.h"
#include "MJpegEvent.h"
//#using <mscorlib.dll>

MJpegEvent::MJpegEvent(long bytes, string filename)
{
	this->bytes = bytes;
	this->filename = filename;
}

/*MJpegEvent::~MJpegEvent(void)
{

}*/


long MJpegEvent::getBytes(void) const
{
	return bytes;
}

string MJpegEvent::getFilename(void) const
{
	return filename;
}