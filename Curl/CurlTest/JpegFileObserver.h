#pragma once

#include <string>

using namespace std;

class JpegFileObserver
{
public:
	virtual void jpegDecoded(long bytes, const string &filename);
}
