
#ifndef INCLUDED_CHALLENGE_H
#define INCLUDED_CHALLENGE_H

#include "global.h"
#include "eventmanager.h"

// Challenge class: abstract object for a complex beach challenge.
class Challenge : public EventRecipient
{
protected:
public:
	// Creators.
	Challenge();
	virtual ~Challenge();

	// Modifiers.
	virtual void OnEvent(const EVENT event, const int param1 = 0, const int param2 = 0) = 0;
};

#endif INCLUDED_CHALLENGE_H