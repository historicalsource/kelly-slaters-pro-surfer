
#ifndef INCLUDED_MODE_HEADTOHEAD_H
#define INCLUDED_MODE_HEADTOHEAD_H

#include "global.h"
#include "mode.h"
#include "kellyslater_controller.h"

// HeadToHeadMode class - manages the state for Head to Head multiplayer mode.
class HeadToHeadMode
{
protected:
	

protected:
	

protected:
	

protected:
	

public:
	// Creators.
	HeadToHeadMode();
	~HeadToHeadMode();

	// Modifiers.
	void Initialize(kellyslater_controller ** controllers);
	void Reset(void);
	void Update(const float dt);

	// Accessors.
};

#endif INCLUDED_MODE_HEADTOHEAD_H