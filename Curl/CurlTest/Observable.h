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
template <class T>
class Observable
{
public:

	/** Destructor.  */
	virtual ~Observable(void)
	{
		vector<Listener<T> >::iterator i;		
		for(i=listeners.begin(); i!=listeners.end(); ++i)
		{
			delete &(*i);
		}
	}

	/** Adds the listener to the list.  */
	void addListener(Listener<T> *listener)
	{
		listeners.push_back(listener);
	}

	/** Removes a listener from the list.  */
	void removeListener(Listener<T> *listener)
	{
		for(vector<Listener<T> >::iterator i=listeners.begin();
			i!=listeners.end(); ++i)
		{
			if((*i)==listener)
			{
				listeners.erase(i);
				return;
			}
		}
	}

	/** Notifies all of the listeners of an event. */
	void alertListeners(T event)
	{
		for(vector<Listener<T> >::iterator i=listeners.begin();
			i!=listeners.end(); ++i)
		{
			i->eventOccurred(event);
		}
	}

protected:

	/** Private vector of Listeners.  */
	vector< Listener< T > > listeners;
};
