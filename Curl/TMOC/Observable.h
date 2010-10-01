#pragma once

#include "Listener.h"
#include "MJpegEvent.h"
#include <vector>

using namespace std;

/**
 * This is a generic version of an observable object.  The idea
 * is that listeners register with this class (via the addListener 
 * function) and are notified of some sort of event via a 
 */
class Observable
{
public:

	/** Destructor.  */
	virtual ~Observable(void)
	{
		/*
		vector<Listener>::iterator i;		
		for(i=listeners.begin(); i!=listeners.end(); ++i)
		{
			delete *i;
		}
		*/
	}

	/** Adds the listener to the list.  */
	virtual void addListener(Listener &listener)
	{
		listeners.push_back(listener);
	}

	/** Removes a listener from the list.  */
	virtual void removeListener(Listener &listener)
	{
		for(vector<Listener>::iterator i=listeners.begin();
			i!=listeners.end(); ++i)
		{
			if(&(*i)==&listener)
			{
				listeners.erase(i);
				return;
			}
		}
	}

	/** Notifies all of the listeners of an event. */
	virtual void alertListeners(MJpegEvent *event)
	{
		for(vector<Listener>::iterator i=listeners.begin();
			i!=listeners.end(); ++i)
		{
			i->eventOccurred(event);
		}
	}

protected:

	/** Private vector of Listeners.  */
	vector<Listener> listeners;
};
