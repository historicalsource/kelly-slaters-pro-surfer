
#ifndef INCLUDED_IGO_WIDGET_SPECIALMETER_H
#define INCLUDED_IGO_WIDGET_SPECIALMETER_H

#include "global.h"
#include "igo_widget.h"
#include "fepanel.h"
#include "specialmeter.h"

// SpecialMeterWidget - the onscreen trick meter displayed in one-player modes.
class SpecialMeterWidget : public IGOWidget
{
private:
	static const color COLOR_NORMAL;
	static const color COLOR_SPECIAL;
	static const float SPEED_COLOR_FLASH;
	static const float FLASH_MIN;
	static const float FLASH_MAX;

private:
	SpecialMeter *	meter;
	PanelQuad *		bgPQ;
	PanelQuad *		colorPQ;
	PanelQuad *		fgPQ;

	int				flashDir;	// special meter color flash direction
	float			flashAmt;	// special meter flash color %

public:
	// Creators.
	SpecialMeterWidget();
	virtual ~SpecialMeterWidget();

	// Modifiers.
	virtual void SetDisplay(const bool d = true);
	virtual void Init(PanelFile & panel, SpecialMeter * specialMeter);
	virtual void Update(const float dt);
	virtual void Draw(void);
	virtual void SetFillage(const float amt);

	// Accessors.
};

#endif INCLUDED_IGO_WIDGET_SPECIALMETER_H