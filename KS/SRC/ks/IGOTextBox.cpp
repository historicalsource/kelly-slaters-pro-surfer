
#include "IGOTextBox.h"

//	IGOTextBox()
// Constructor with size initializers.
IGOTextBox::IGOTextBox(const int w, const int h)
{
	int	row;
	
	// Allocate chars.
	width = w;
	height = h;
	text = NEW char *[height];
	for (row = 0; row < height; row++)
		text[row] = NEW char[width];

	// Default formatting.
	scroll = SCROLL_DOWN;
	horizJust = HJ_LEFT;
	vertJust = VJ_TOP;
}

//	~IGOTextBox()
// Destructor.
IGOTextBox::~IGOTextBox()
{
	int	row;

	for (row = 0; row < height; row++)
		delete text[row];
	delete [] text;
}

//	SetText()
// Changes this box's text.
// The number of characters in the new text must not exceed width*height.
void IGOTextBox::SetText(const stringx & s)
{
	stringx	word;
	char *	line = NULL;
	int		rowIdx = 0;
	int		lineSize = 0;
	int		sIdx = 0, i = 0;

	// Initialize.
	if (scroll == SCROLL_DOWN)
		rowIdx = 0;
	else
		rowIdx = height-1;

	// Process the string into words and add each word to our text array.
	for (sIdx = 0; sIdx < s.size(); sIdx++)
	{
		// Ignore color codes.
		if (s[sIdx] == '@')
		{
			sIdx++;
			if (s[sIdx] != '@')
				continue;
		}
		
		// Append to word.
		if (s[sIdx] != ' ')
		{
			word += s[sIdx];
			continue;
		}

		// Append word to this line.
		assert(word.size() <= width);
		if (lineSize + 1 + word.size() <= width)
		{
			line[lineSize++] = ' ';
			for (i = 0; i < word.size(); i++)
				line[lineSize++] = word[i];
		}
		// Append word to next line.
		else
		{
			// Advance to next line.
			if (scroll == SCROLL_DOWN)
			{
				rowIdx++;
				assert(rowIdx < height);
			}
			else
			{
				rowIdx--;
				assert(rowIdx >= 0);
			}
			line = text[rowIdx];
			lineSize = 0;

			// Append word to this line.
			for (i = 0; i < word.size(); i++)
				line[lineSize++] = word[i];
		}

		// Search for next word.
		word.resize(0);
	}
}