
#include "StdAfx.h"
#include "Observable.h"
#using <mscorlib.dll>

Observable::Observable(void)
{
	/* noop.  */
}

Observable::~Observable(void)
{

}

void Observable::addListener(const Listener<MJpegEvent> &listener)
{
	// TODO
}

void Observable::removeListener(const Listener<MJpegEvent> &listener)
{
	// TODO
}

void Observable::alertListeners(const MJpegEvent &event)
{
	// TODO
}
