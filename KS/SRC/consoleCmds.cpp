#include "global.h"

#if _CONSOLE_ENABLE

#include "consoleCmds.h"
#include "consoleVars.h"
#include "console.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Register the commands
///////////////////////////////////////////////////////////////////////////////////////////////////
register_console_cmd(CommandList);
register_console_cmd(VariableList);
register_console_cmd(HelpCommand);
register_console_cmd(ExecCommand);
register_console_cmd(GetCommand);
register_console_cmd(SetCommand);
register_console_cmd(ConsoleLogCommand);
///////////////////////////////////////////////////////////////////////////////////////////////////




extern list<ConsoleCommand *> *g_console_cmds;
extern list<ConsoleVariable *> *g_console_vars;

ConsoleCommand::ConsoleCommand()
{
	setName("");

  if(g_console_cmds == NULL)
    g_console_cmds = NEW list<ConsoleCommand *>;

  g_console_cmds->push_back(this);
}

ConsoleCommand::~ConsoleCommand()
{
}

void ConsoleCommand::deconstruct_all()
{
  delete g_console_cmds;
  g_console_cmds = NULL;
}

int ConsoleCommand::process_cmd(const vector<stringx> &args)
{
	return(0);
}

void ConsoleCommand::setName(const stringx &pName)
{
  assert( pName.size()<MAX_COMMAND_NAME_LEN );
  strcpy( name, pName.c_str() );
  strlwr( name );
}



int CommandList::process_cmd(const vector<stringx> &args)
{
	console_log("");
	console_log("<-- Console Commands -->");

	if(g_console_cmds && g_console_cmds->size())
	{
		list<ConsoleCommand *>::iterator node = g_console_cmds->begin();
		while(node != g_console_cmds->end())
		{
      if((*node))
			  console_log("%s", (*node)->getName().c_str());

      node++;
		}
	}

	return(1);
}




int VariableList::process_cmd(const vector<stringx> &args)
{
	console_log("");
	console_log("<-- Console Variables -->");

	if(g_console_vars && g_console_vars->size())
	{
		list<ConsoleVariable *>::iterator node = g_console_vars->begin();
		while(node != g_console_vars->end())
		{
      if((*node))
			  console_log("%s %s", (*node)->getName().c_str(), (*node)->getValue().c_str());

      node++;
		}
	}

	return(1);
}


int HelpCommand::process_cmd(const vector<stringx> &args)
{
	if(args.size() == 0)
	{
		console_log("");
		console_log("<-- Keys -->");
		console_log("Up Arrow    Scroll command buffer up");
		console_log("Down Arrow  Scroll command buffer down");
		console_log("Tab         Complete current command");
		console_log("Page Up     Scroll log buffer up");
		console_log("Page Down   Scroll log buffer down");
		console_log("Home        Scroll log buffer top");
		console_log("End         Scroll log buffer bottom");
		console_log("ESC         Show / hide console");
		console_log("");
		console_log("<-- Commands -->");
		console_log("cmdlist     List all commands");
		console_log("varlist     List all variables");
		console_log("help <?>    Get help for a command");
	}
	else
	{
		console_log("");

    stringx cmd_name = args[0];
    cmd_name.to_lower();

		ConsoleCommand *cmd = g_console->getCommand(cmd_name);
		if(cmd != NULL)
		{
			console_log(cmd->helpText());
		}
		else
		{
			ConsoleVariable *var = g_console->getVariable(cmd_name);
			if(var != NULL)
			{
				console_log(var->helpText());
			}
			else
			{
				console_log("Command '%s' not found!", cmd_name.c_str());
			}
		}
	}

	return(1);
}

int ExecCommand::process_cmd(const vector<stringx> &args)
{
	if(args.size() > 0)
		g_console->exec(args[0]);

	return(1);
}

int GetCommand::process_cmd(const vector<stringx> &args)
{
	if(args.size() > 0)
	{
		ConsoleVariable *var = g_console->getVariable(args[0]);
		if(var != NULL)
		{
			stringx val = var->getValue();
			console_log("'%s' is %s", var->getName().c_str(), val.c_str());
		}
		else
			console_log("??? %s", args[0].c_str());
	}

	return(1);
}



int SetCommand::process_cmd(const vector<stringx> &args)
{
	if(args.size() > 1)
	{
		ConsoleVariable *var = g_console->getVariable(args[0]);
		if(var != NULL)
			var->setValue(args[1]);
		else
			console_log("??? %s", args[0].c_str());
	}

	return(1);
}


int ConsoleLogCommand::process_cmd(const vector<stringx> &args)
{
  if(args.size() > 0)
  {
    stringx param = args[0];
    param.to_lower();

    if(param == "end" || param == "close")
    {
      g_console->endLogToFile();
    }
    else
    {
      bool append = false;
      bool hard = false;

      for(unsigned i=1; i<args.size(); i++)
      {
        param = args[i];
        param.to_lower();

        if(param == "append")
          append = true;

        if(param == "hard")
          hard = true;
      }

      g_console->logToFile(args[0], append, hard);
    }
  }

  if(g_console->getFileLogging())
 	  console_log("Currently logging console to: '%s'", g_console->getFileLogName().c_str());
  else
 		console_log("Not logging console");

	return(1);
}

#endif  // _CONSOLE_ENABLE
