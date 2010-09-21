#pragma once

#include "Listener.h"
#include <vector>

using namespace std;

/**
 * This is a generic version of an observable object.  The idea
 * is that listeners register with this class (via the addListener 
 * function) and are notified of some sort of event via a 
 */
template <class E>
class Observable
{
public:
	Observable(void);
	virtual ~Observable(void);

	/** Adds the listener to the list.  */
	void addListener(const Listener<E> &listener);

	/** Removes a listener from the list.  */
	void removeListener(const Listener<E> &listener);

	/** Notifies all of the listeners of an event. */
	void alertListeners(const E &event);

protected:

	/** Private vector of Listeners.  */
	vector< Listener<E> > listeners;
};
