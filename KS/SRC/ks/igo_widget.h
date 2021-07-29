
#ifndef INCLUDED_IGO_WIDGET
#define INCLUDED_IGO_WIDGET

#include "global.h"

// IGOWidget - an IGO graphic like a clock or a meter
class IGOWidget
{
protected:
	bool	display;

public:
	// Creators.
	IGOWidget();
	virtual ~IGOWidget();

	// Modifiers.
	virtual void SetDisplay(const bool d = true);
	virtual void Update(const float dt);
	virtual void Draw(void);

	// Accessors.
	virtual bool GetDisplay(void) const;
};

#endif INCLUDED_IGO_WIDGET