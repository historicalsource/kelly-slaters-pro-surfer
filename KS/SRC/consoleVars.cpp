#include "global.h"

#if _CONSOLE_ENABLE

#include "consoleVars.h"
#include "consoleCmds.h"
#include "console.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Register the variables
///////////////////////////////////////////////////////////////////////////////////////////////////
register_console_var(ConsoleHeightVariable);
///////////////////////////////////////////////////////////////////////////////////////////////////



extern list<ConsoleVariable *> *g_console_vars;
extern list<ConsoleCommand *> *g_console_cmds;


ConsoleVariable::ConsoleVariable()
{
	setName("");

  if(g_console_vars == NULL)
    g_console_vars = NEW list<ConsoleVariable *>;

  g_console_vars->push_back(this);
}

ConsoleVariable::~ConsoleVariable()
{
}

void ConsoleVariable::deconstruct_all()
{
  delete g_console_vars;
  g_console_vars = NULL;
}

void ConsoleVariable::setValue(const stringx &value)
{
}

const stringx ConsoleVariable::getValue()
{
  return stringx();
}

void ConsoleVariable::setName(const stringx &pName)
{
  assert( pName.size()<MAX_VARIABLE_NAME_LEN );
  strcpy( name, pName.c_str() );
  strlwr( name );
}

void ConsoleHeightVariable::setValue(const stringx &value)
{
	rational_t height = 0.0f;

	if(value.length() > 0 && (height = (rational_t)atof(value.c_str())) >= 25.0f && height <= nglGetScreenHeight())
		g_console->setHeight(height);
	else
		console_log("'%s' must be between 25 and %d", getName().c_str(), nglGetScreenHeight());
}

const stringx ConsoleHeightVariable::getValue()
{
  char value[32];
  sprintf(value, "%.2f", g_console->getHeight());

  return stringx(value);
}

#endif  // _CONSOLE_ENABLE
