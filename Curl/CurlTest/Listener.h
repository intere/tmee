#pragma once

template <class E>
class Listener
{
public:
	virtual ~Listener()=0;

	/** 
	 * Callback function that notifies the listener that an
	 * event has occurred.  
	 */
	virtual eventOccurred(E event)=0;	
};
