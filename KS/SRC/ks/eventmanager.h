
#ifndef INCLUDED_EVENTMANAGER_H
#define INCLUDED_EVENTMANAGER_H

#include "global.h"

enum EVENT
{
	EVT_SURFER_LAND,			// player landed (param1 = player index, param2 = landing type)
	EVT_SURFER_WIPEOUT,			// player wiped out (param1 = player index, param2 = 0 for normal, 1 for kluge)
	EVT_SURFER_DUCK_DIVE,		// player began a duck dive (param1 = player index)
	
	EVT_TRICK_REGION_CHANGE,	// surfer moved into a new trick region (param1 = player index)
	EVT_TRICK_FACE_BEGIN,		// player began a face trick (param1 = player index)
	EVT_TRICK_FACE_END,			// player ended a face trick (param1 = player index)

	EVT_SCORING_SERIES_END,		// player completed a trick series (param1 = player index, param2 = successful)
	EVT_SCORING_CHAIN_END,		// player completed a trick chain (param1 = player index, param2 = successful)
	
	// add more events here
};

const int MAX_EVENT_RECIPIENTS = 50;

class EventRecipient;

// EventManager class: dispatches events to recipients.
class EventManager
{
protected:
	int					numRecipients;
	EventRecipient *	recipients[MAX_EVENT_RECIPIENTS];

public:
	// Creators.
	EventManager();
	~EventManager();

	// Modifiers.
	void RegisterRecipient(EventRecipient * recipient);
	void UnregisterRecipient(EventRecipient * recipient);
	void DispatchEvent(const EVENT event, const int param1 = 0, const int param2 = 0);

	// Accessors.
};

extern EventManager	g_eventManager;

// Derive from this class to implement an event listener.
class EventRecipient
{
public:
	// Creators.
	EventRecipient();
	virtual ~EventRecipient();

	// Modifiers.
	virtual void OnEvent(const EVENT event, const int param1 = 0, const int param2 = 0) = 0;
};


#endif INCLUDED_EVENTMANAGER_H