#pragma once

#include "Listener.h"
#include "MJpegEvent.h"

class JpegFileListener  : public Listener<MJpegEvent*>
	/*:
	public Listener<MJpegEvent*>*/
{
public:
	JpegFileListener(void);
	virtual ~JpegFileListener(void);

	/** Inherited from Listener: */
	virtual eventOccurred(MJpegEvent *event);
};
