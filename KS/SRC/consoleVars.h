#ifndef _CONSOLEVARS_HPP_
#define _CONSOLEVARS_HPP_


#include "global.h"


#if _CONSOLE_ENABLE

#include "console.h"
#include <string.h>


class ConsoleVariable 
{
protected:
	friend class Console;

  enum {MAX_VARIABLE_NAME_LEN=32};
	char name[MAX_VARIABLE_NAME_LEN];

public:
	ConsoleVariable();
	virtual ~ConsoleVariable();

	virtual void setValue(const stringx &value);
	virtual const stringx getValue();

	void setName(const stringx &pName);
	const stringx getName() const   { return(name); }

	bool match(const stringx pName)
	{
    return(name == pName);
	}

	virtual char *helpText()
	{
		return("No help available.");
	}

  static void deconstruct_all();
};

class ConsoleHeightVariable : public ConsoleVariable
{
public:
	ConsoleHeightVariable()
	{
		setName("console_height");
	}

	~ConsoleHeightVariable()
	{
	}

	virtual void setValue(const stringx &value);
	virtual const stringx getValue();

	virtual char *helpText()
	{
		return("Height of the console in pixels");
	}

};


#endif  // _CONSOLE_ENABLE

#endif