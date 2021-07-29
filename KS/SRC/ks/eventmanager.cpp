
#include "global.h"
#include "eventmanager.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	EventManager class
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EventManager g_eventManager;

//	EventManager()
// Default constructor.
EventManager::EventManager()
{
	int	i;

	for (i = 0; i < MAX_EVENT_RECIPIENTS; i++)
		recipients[i] = NULL;

	numRecipients = 0;
}

//	~EventManager()
// Destructor.
EventManager::~EventManager()
{

}

//	RegisterRecipient()
// The specified object will be notified when events occur.
void EventManager::RegisterRecipient(EventRecipient * recipient)
{
	if (numRecipients == MAX_EVENT_RECIPIENTS)
	{
		nglPrintf("Error: event manager already has the maximum number of recipients allowed.  Increase MAX_EVENT_RECIPIENTS\n");
		assert(false);
	}

	recipients[numRecipients++] = recipient;
}

//	UnregisterRecipient()
// The specified object will no longer be notified when events occur.
void EventManager::UnregisterRecipient(EventRecipient * recipient)
{
	int	i, j;

	for (i = 0; i < MAX_EVENT_RECIPIENTS; i++)
	{
		if (recipients[i] == recipient)
		{
			recipients[i] = NULL;
			for (j = i; j < numRecipients-1; j++)
			{
				recipients[j] = recipients[j+1];
			}
			break;
		}
	}

	numRecipients--;
}

//	DispatchEvent()
// The specified event will be sent to all recipients.
void EventManager::DispatchEvent(const EVENT event, const int param1, const int param2)
{
	int	i;

	for (i = 0; i < numRecipients; i++)
		recipients[i]->OnEvent(event, param1, param2);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	EventRecipient class
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//	EventRecipient()
// Default constructor.
EventRecipient::EventRecipient()
{
	g_eventManager.RegisterRecipient(this);
}

//	~EventRecipient()
// Destructor.
EventRecipient::~EventRecipient()
{
	g_eventManager.UnregisterRecipient(this);
}