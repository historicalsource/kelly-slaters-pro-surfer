#ifndef _CONSOLECMDS_HPP_
#define _CONSOLECMDS_HPP_

#include "global.h"

#if _CONSOLE_ENABLE

#include "console.h"
#include <string.h>

class ConsoleCommand
{
protected:
	friend class Console;

  enum {MAX_COMMAND_NAME_LEN=32};
  char name[MAX_COMMAND_NAME_LEN];

public:
	ConsoleCommand();
	virtual ~ConsoleCommand();

	virtual int process_cmd(const vector<stringx> &args);

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


class CommandList : public ConsoleCommand
{
public:
  CommandList()
	{
		setName("cmdlist");
	}

	virtual ~CommandList()
	{
	}

	virtual int process_cmd(const vector<stringx> &args);

	virtual char *helpText()
	{
		return("Lists all available commands");
	}
};

class VariableList : public ConsoleCommand
{
public:
	VariableList()
	{
		setName("varlist");
	}

	virtual ~VariableList()
	{
	}

	virtual int process_cmd(const vector<stringx> &args);

	virtual char *helpText()
	{
		return("Lists all available variables");
	}
};

class HelpCommand : public ConsoleCommand
{
public:
	HelpCommand()
	{
		setName("help");
	}

	virtual ~HelpCommand()
	{
	}

	virtual int process_cmd(const vector<stringx> &args);

	virtual char *helpText()
	{
		return("Retrieves help for console / command");
	}
};

class ExecCommand : public ConsoleCommand
{
public:
	ExecCommand()
	{
		setName("exec");
	}

	virtual ~ExecCommand()
	{
	}

	virtual int process_cmd(const vector<stringx> &args);

	virtual char *helpText()
	{
		return("exec [file] -> Executes a console script file");
	}
};

class GetCommand : public ConsoleCommand
{
public:
	GetCommand()
	{
		setName("get");
	}

	virtual ~GetCommand()
	{
	}

	virtual int process_cmd(const vector<stringx> &args);

	virtual char *helpText()
	{
		return("get [name] -> Gets the value of a variable");
	}
};

class SetCommand : public ConsoleCommand
{
public:
	SetCommand()
	{
		setName("set");
	}

	virtual ~SetCommand()
	{
	}

	virtual int process_cmd(const vector<stringx> &args);

	virtual char *helpText()
	{
		return("set [name] [val] -> Sets the value of a variable");
	}
};

class ConsoleLogCommand : public ConsoleCommand
{
public:
	ConsoleLogCommand()
	{
		setName("console_log");
	}

	virtual ~ConsoleLogCommand()
	{
	}

	virtual int process_cmd(const vector<stringx> &args);

	virtual char *helpText()
	{
		return("console_log [filename | end] -> open / end file logging of console");
	}
};

#endif  // _CONSOLE_ENABLE

#endif