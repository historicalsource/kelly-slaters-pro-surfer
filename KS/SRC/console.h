#ifndef _CONSOLE_HPP_
#define _CONSOLE_HPP_

#include "global.h"

#if _CONSOLE_ENABLE

  #include "ostimer.h"
  #include "visrep.h"
  #include "widget.h"

  #if defined(TARGET_PC)
//    #include <fstream.h>

    #define _CONSOLE_ADV_COMMANDS_OVERRIDE    0
    #define _CONSOLE_ADV_VARIABLES_OVERRIDE   0
    #define _CONSOLE_USE_WIN32_MSG            0
  #endif

  #if defined(TARGET_MKS)
    #define _CONSOLE_ADV_COMMANDS_OVERRIDE    0
    #define _CONSOLE_ADV_VARIABLES_OVERRIDE   0
    #define _CONSOLE_USE_WIN32_MSG            0
  #endif


  #include "keyboard.h"

  class ConsoleCommand;
  class ConsoleVariable;

  bool get_boolean_value(const char *val);

  void str_tolower(char *str);
  void str_toupper(char *str);


  class Console : public bitmap_widget
  {
  protected:
    friend class CommandList;
    friend class VariableList;
    friend class HelpCommand;
    friend class GetCommand;
    friend class SetCommand;
    friend class ConsoleDialog;

	  unsigned char visible;

    #if defined(TARGET_PC)
	    char oldCurrent[1024];
	    char current[1024];
    #else
	    char oldCurrent[256];
	    char current[256];
    #endif

	  list<stringx> log;
	  list<stringx> cmdLog;

	  unsigned char cursor;
	  time_value_t cursorTick;

	  rational_t consoleHeight;
	  int lineNumber;

	  int logMaxSize;
	  int cmdLogMaxSize;
	  int cmdLogNumber;

    bool hardLogging;
    stringx fileLogName;
    bool fileLogging;

    #if defined(TARGET_PC)
      ofstream fileLog;
    #endif

	  char renderCursor;

    bool get_param(const char *parms, char *p, int &index);

    virtual ConsoleCommand *getCommand(const stringx &name);
	  virtual ConsoleVariable *getVariable(const stringx &name);

  public:
	  Console();
	  virtual ~Console();

    #if _CONSOLE_USE_WIN32_MSG
	    virtual bool handleWindowsMessage(UINT msg, WPARAM wParam, LPARAM lParam);
    #else
      virtual void handle_event(KeyEvent eventtype, int key, void* userdata);
      virtual void handle_char(char ch, void* userdata);
    #endif

    virtual void partialCompleteCmd(char *command, list<stringx> &lst);
	  virtual void getMatchingCmds(const char *command, list<stringx> &lst);

	  virtual unsigned char isVisible()	{ return(visible); }

	  virtual void show();
	  virtual void hide();

	  virtual void render();

	  virtual void exec(const stringx &file);

	  virtual void addToLog(const char *format, ...);
	  virtual void processCommand(const char *command, char log = 1);

	  virtual void addToCommandLog(const char *string);

	  virtual void setHeight(rational_t height);
	  virtual rational_t getHeight()	{ return(consoleHeight); }

    virtual void logToFile(const stringx &file, bool append = false, bool hard = false);
    virtual void endLogToFile();
    const stringx getFileLogName() const { return(fileLogName); }
    bool getHardLogging() { return(hardLogging); }
    bool getFileLogging() { return(fileLogging); }

    virtual void frame_advance(time_value_t time_inc);

	  virtual void setRenderCursor(char on = 1)	{ renderCursor = on; }
  };


  extern Console *g_console;

  #define console_log                       g_console->addToLog
  #define console_process                   g_console->processCommand

  #define register_console_cmd(command)     command g_##command
  #define register_console_var(variable)    variable g_##variable

  #define init_console()                    { if(g_console == NULL) { widget::set_rhw_2d_layer(widget::RHW9); g_console = NEW Console(); widget::restore_last_rhw_2d_layer(); } }

  #define destroy_console()                 { if(g_console != NULL) delete g_console; g_console = NULL; }
  #define frame_advance_console(time_inc)   { g_console->frame_advance( time_inc ); }
  #define render_console()                  { g_console->render(); }

#else

#if defined(TARGET_XBOX)
#ifdef console_log
#undef console_log
#endif /* console_log */
#endif /* TARGET_XBOX JIV DEBUG */

  inline void console_log(const char *format, ...)                {}
  inline void console_process(const char *command, char log = 1)  {}

  #define register_console_cmd(command)     {}
  #define register_console_var(variable)    {}

  #define init_console()                    {}
  #define destroy_console()                 {}
  #define frame_advance_console(time_inc)   {}
  #define render_console()                  {}

#endif



#endif
