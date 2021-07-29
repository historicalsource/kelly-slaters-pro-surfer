
#ifndef INCLUDED_IGOTEXTBOX_H
#define INCLUDED_IGOTEXTBOX_H

// IGOTextBox - multiple lines of text to be displayed onscreen (trick text, surfer bios, etc.)
class IGOTextBox
{
public:
	enum JUSTIFY_HORIZ { JH_LEFT, JH_RIGHT };
	enum JUSTIFY_VERT { JV_TOP, JV_BOTTOM };
	enum SCROLL { SCROLL_UP, SCROLL_DOWN };

protected:
	char **			text;			// in [row][col] order
	int				width, height;	// size if text array
	int				posX, posY;		// onscreen position (top/left)
	JUSTIFY_HORIZ	horizJust;		// formatting
	JUSTIFY_VERTE	vertJust;		// formatting
	SCROLL			scroll;			// formatting

public:
	// Creators.
	IGOTextBox(const int width, const int height);
	~IGOTextBox();

	// Modifiers.
	void SetPosition(const int x, const int y) { posX = x; posY = y; }
	void SetHorizJustify(const JUSTIFY_HORIZ just) { horizJust = just; }
	void SetVertJustify(const JUSTIFY_VERT just) { vertJust = just; }
	void SetScrolling(const SCROLL sc) { scroll = sc; }
	void SetText(const stringx & s);

	// Accessors.
	void GetPosition(int & x, int & y) const { x = posX; y = posY; }
	JUSTIFY_HORIZ GetHorizJustify(void) const { return horizJust; }
	JUSTIFY_VERT GetVertJustify(void) const { return VertJust; }
	SCROLL GetScrolling(void) const { return scroll; }
};

#endif INCLUDED_IGOTEXTBOX_H