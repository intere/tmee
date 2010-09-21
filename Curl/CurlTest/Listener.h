#pragma once



template <class E>
class Listener
{
public:
	Listener(void);
	virtual ~Listener(void);

	/** 
	 * Callback function that notifies the listener that an
	 * event has occurred.  
	 */
	virtual eventOccurred(const E& event);	
};
