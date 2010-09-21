#pragma once

#include "Listener.h"
#include <vector>
#include "MJpegEvent.h"

using namespace std;

/**
 * This is a generic version of an observable object.  The idea
 * is that listeners register with this class (via the addListener 
 * function) and are notified of some sort of event via a 
 */
class Observable
{
public:
	/** Default constructor.  */
	Observable(void);

	/** Destructor.  */
	virtual ~Observable(void);

	/** Adds the listener to the list.  */
	void addListener(const Listener<MJpegEvent> &listener);

	/** Removes a listener from the list.  */
	void removeListener(const Listener<MJpegEvent> &listener);

	/** Notifies all of the listeners of an event. */
	void alertListeners(const MJpegEvent &event);

protected:

	/** Private vector of Listeners.  */
	vector< Listener< MJpegEvent > > listeners;
};
