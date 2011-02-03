#pragma once

#include "MJpegEvent.h"
#include <sstream>
#include <iostream>

using namespace std;

class Listener
{
public:

	/** 
	 * Callback function that notifies the listener that an
	 * event has occurred.  
	 */
	virtual void eventOccurred(MJpegEvent* event)
	{
		ostringstream buff;
		buff << "Listener::eventOccurred(MJPegEvent*) - should not be called" << endl;
		basic_string<TCHAR> out = buff.str().c_str();
		TRACE(out.c_str());
	}
};
