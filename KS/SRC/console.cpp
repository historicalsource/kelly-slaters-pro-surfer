#include "global.h"


#if _CONSOLE_ENABLE

#include "console.h"
#include "consoleCmds.h"
#include "consoleVars.h"
#include "inputmgr.h"

#if defined(TARGET_PC)
//  #include <fstream.h>
#endif

#if defined(TARGET_MKS)
//  #include <stdarg.h>
#endif

//#include <string.h>
#include "hwrasterize.h"


list<ConsoleCommand *> *g_console_cmds = NULL;
list<ConsoleVariable *> *g_console_vars = NULL;



Console *g_console = NULL;

#if !_CONSOLE_USE_WIN32_MSG
  static void console_event_callback(KeyEvent eventtype, int key, void* userdata)
  {
    if(g_console)
      g_console->handle_event(eventtype, key, userdata);
  }

  static void console_char_callback(char ch, void* userdata)
  {
    if(g_console)
      g_console->handle_char(ch, userdata);
  }
#endif


bool get_boolean_value(const char *val)
{
  return(val && (!stricmp(val, "on") || !stricmp(val, "true") || !strcmp(val, "1")));
}

void str_tolower(char *str)
{
  for(unsigned i = 0; i<strlen(str); ++i)
  {
    if(str[i] >= 'A' && str[i] <= 'Z')
      str[i] = tolower(str[i]);
  }
}

void str_toupper(char *str)
{
  for(unsigned i = 0; i<strlen(str); ++i)
  {
    if(str[i] >= 'a' && str[i] <= 'z')
      str[i] = toupper(str[i]);
  }
}


#if _CONSOLE_USE_WIN32_MSG
#define VK_0				0x30	// 0 key 
#define VK_1				0x31	// 1 key 
#define VK_2				0x32	// 2 key 
#define VK_3				0x33	// 3 key 
#define VK_4				0x34	// 4 key 
#define VK_5				0x35	// 5 key 
#define VK_6				0x36	// 6 key 
#define VK_7				0x37	// 7 key 
#define VK_8				0x38	// 8 key 
#define VK_9				0x39	// 9 key 
#define VK_A				0x41	// A key 
#define VK_B				0x42	// B key 
#define VK_C				0x43	// C key 
#define VK_D				0x44	// D key 
#define VK_E				0x45	// E key 
#define VK_F				0x46	// F key 
#define VK_G				0x47	// G key 
#define VK_H				0x48	// H key 
#define VK_I				0x49	// I key 
#define VK_J				0x4A	// J key 
#define VK_K				0x4B	// K key 
#define VK_L				0x4C	// L key 
#define VK_M				0x4D	// M key 
#define VK_N				0x4E	// N key 
#define VK_O 				0x4F	// O key 
#define VK_P 				0x50	// P key 
#define VK_Q 				0x51	// Q key 
#define VK_R 				0x52	// R key 
#define VK_S				0x53	// S key 
#define VK_T 				0x54	// T key 
#define VK_U 				0x55	// U key 
#define VK_V 				0x56	// V key 
#define VK_W 				0x57	// W key 
#define VK_X 				0x58	// X key 
#define VK_Y				0x59	// Y key 
#define VK_Z 				0x5A	// Z key 

// The following key codes are declared as OEM specific
#define VK_COMMA			0xBC
#define VK_PERIOD			0xBE
#define VK_SLASH			0xBF
#define VK_LBRACKET			0xDB
#define VK_RBRACKET			0xDD
#define VK_BACKSLASH		0xDC
#define VK_COLON			0xBA
#define VK_QUOTE			0xDE
#define VK_TILDE			0xC0
#endif


Console::Console() : bitmap_widget("Console", NULL, 0, 0, 1)
{
	visible = 0;
	current[0] = '\0';
	oldCurrent[0] = '\0';

	cursor = 0;

	consoleHeight = 240.0f;


	log.resize(0);
	cmdLog.resize(0);


  cursorTick = 0.0f;

	lineNumber = 0;

#if defined(TARGET_PC)
	logMaxSize = 1024;
#else
	logMaxSize = 128;
#endif

#if defined(TARGET_PC)
	cmdLogMaxSize = 128;
#else
	cmdLogMaxSize = 16;
#endif

	cmdLogNumber = 0;

	renderCursor = 1;

  stringx dafile = stringx("interface\\")+os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR) + stringx("\\black1");
  open(dafile.c_str());

  hardLogging = false;
  fileLogging = false;
  fileLogName = "console_log.txt";

  setHeight(200);
  hide();

#if !_CONSOLE_USE_WIN32_MSG
  KB_register_event_callback(console_event_callback, NULL);
  KB_register_char_callback(console_char_callback, NULL);
#endif
}

Console::~Console()
{
	log.resize(0);
	cmdLog.resize(0);

  endLogToFile();
}

void Console::setHeight(rational_t height)
{
	consoleHeight = height;

  resize(nglGetScreenWidth(), height);
}

ConsoleCommand *Console::getCommand(const stringx &name)
{
  if(g_console_cmds)
  {
    list<ConsoleCommand *>::iterator i = g_console_cmds->begin();
    while(i != g_console_cmds->end())
    {
		  if((*i) && (*i)->match(name))
			  return((*i));

      i++;
    }
  }

	return(NULL);
}

ConsoleVariable *Console::getVariable(const stringx &name)
{
  if(g_console_vars)
  {
    list<ConsoleVariable *>::iterator i = g_console_vars->begin();
    while(i != g_console_vars->end())
    {
		  if((*i) && (*i)->match(name))
			  return((*i));

      i++;
    }
  }

	return(NULL);
}

void Console::partialCompleteCmd(char *command, list<stringx> &lst)
{
  int min_length = -1;
	list<stringx>::iterator node = lst.begin();

	while(node != lst.end())
	{
		stringx str1 = (*node);
    node++;

    if(node != lst.end())
    {
		  stringx str2 = (*node);

      const char *string1 = str1.c_str();
      const char *string2 = str2.c_str();

      int i=0;
      while(string1[i] == string2[i] && string1[i] != '\0' && string2[i] != '\0')
        i++;

      if(i < min_length || min_length == -1)
        min_length = i;
    }
	}

  if(min_length > 0 && min_length > (int)strlen(command))
  {
    strncpy(command, (*lst.begin()).c_str(), min_length);
    command[min_length] = '\0';
  }
}


void Console::getMatchingCmds(const char *command, list<stringx> &lst)
{
	lst.resize(0);

	int len = strlen(command);

  if(g_console_cmds)
  {
	  list<ConsoleCommand*>::iterator node = g_console_cmds->begin();
	  while(node != g_console_cmds->end())
	  {
		  if((*node) && !strncmp((*node)->name, command, len))
			  lst.push_back((*node)->name);

      node++;
	  }
  }

  if(g_console_vars)
  {
	  list<ConsoleVariable*>::iterator node2 = g_console_vars->begin();
	  while(node2 != g_console_vars->end())
	  {
		  if((*node2) && !strncmp((*node2)->name, command, len))
			  lst.push_back((*node2)->name);

      node2++;
	  }
  }
}




#if _CONSOLE_USE_WIN32_MSG

  bool Console::handleWindowsMessage(UINT msg, WPARAM wParam, LPARAM lParam)
  {
    switch(msg)
    {
		  case WM_CHAR:
		  {
    	  if(visible)
	      {
			    unsigned char key = (unsigned char)wParam;

			    if(key >= 32 && key <= 126 && key != '`' && key != '~')
			    {
			      int len = strlen(current);
			      current[len] = key;
			      current[len+1] = '\0';

			      strcpy(oldCurrent, current);
			    }
        }
		  }
		  break;

      case WM_KEYDOWN:
      {
        unsigned char vk = (unsigned char)wParam;

        switch(vk)
        {
          case VK_TILDE:
          {
		        if(!isVisible())
			        show();
		        else
			        hide();
          }
          break;

          case VK_BACK:
          {
        	  if(visible)
	          {
		          int len = strlen(current);
		          if(len > 0)
			          current[len-1] = '\0';

		          strcpy(oldCurrent, current);
            }
          }
          break;

          case VK_RETURN:
          {
        	  if(visible)
	          {
		          processCommand(current);

		          current[0] = '\0';

		          strcpy(oldCurrent, current);
            }
          }
          break;

          case VK_UP:
          {
        	  if(visible)
	          {
		          if(cmdLog.size() > 0)
		          {
			          cmdLogNumber++;
			          
			          while(cmdLogNumber > 0 && cmdLogNumber > cmdLog.size())
				          cmdLogNumber--;

			          if(cmdLogNumber > 0 && cmdLogNumber <= cmdLog.size())
                {
                  list<stringx>::iterator node = cmdLog.begin();
                
                  int i = cmdLogNumber-1;
                  while(i > 0 && node != cmdLog.end())
                  {
                    node++;
                    i--;
                  }

                  if(node != cmdLog.end())
  				          strcpy(current, (*node).c_str());
                }
		          }
            }
          }
          break;

          case VK_DOWN:
          {
        	  if(visible)
	          {
		          if(cmdLog.size() > 0)
		          {
			          cmdLogNumber--;

			          if(cmdLogNumber <= 0)
			          {
				          cmdLogNumber = 0;

				          strcpy(current, oldCurrent);
			          }
			          else
			          {
                  list<stringx>::iterator node = cmdLog.begin();
                
                  int i = cmdLogNumber-1;
                  while(i > 0 && node != cmdLog.end())
                  {
                    node++;
                    i--;
                  }

                  if(node != cmdLog.end())
				            strcpy(current, (*node).c_str());
			          }
		          }
            }
          }
          break;

          case VK_TAB:
          {
        	  if(visible)
	          {
		          if(strlen(current) > 0)
		          {
                list<stringx> lst;
			          getMatchingCmds(current, lst);

			          if(lst.size() > 0)
			          {
				          if(lst.size() == 1)
				          {
					          strcpy(current, (*(lst.begin())).c_str());
                    strcat(current, " ");
					          strcpy(oldCurrent, current);
				          }
				          else
				          {
                    partialCompleteCmd(current, lst);
        	          strcpy(oldCurrent, current);

					          addToLog("");
					          addToLog("<--- %d matches --->", lst.size());

					          list<stringx>::iterator node = lst.begin();

					          while(node != lst.end())
					          {
						          addToLog("%s", (*node).c_str());
                      node++;
					          }
				          }

               	  while(lst.size() > 0)
              		  lst.pop_front();
			          }
			          else
			          {
				          addToLog("");
				          addToLog("<--- No matches --->");
			          }
		          }
            }
          }
          break;

          case VK_PRIOR:
          {
        	  if(visible)
	          {
		          if(lineNumber < (log.size()-1))
			          lineNumber++;
            }
          }
          break;

          case VK_NEXT:
          {
        	  if(visible)
	          {
		          if(lineNumber > 0)
			          lineNumber--;
            }
          }
          break;

          case VK_HOME:
          {
        	  if(visible)
	          {
		          lineNumber = log.size()-1;
            }
          }
          break;

          case VK_END:
          {
        	  if(visible)
	          {
		          lineNumber = 0;
            }
          }
          break;
        }
      }
      break;


      default:
        break;
    }

    return(true);
  }

#else 

  void Console::handle_char(char ch, void* userdata)
  {
    if(visible)
	  {
		  if(ch >= 32 && ch <= 126 && ch != '`' && ch != '~')
		  {
			  int len = strlen(current);
			  current[len] = ch;
			  current[len+1] = '\0';

			  strcpy(oldCurrent, current);
		  }
    }
  }


  void Console::handle_event(KeyEvent eventtype, int key, void* userdata)
  {
    if(eventtype != kePress)
      return;

    switch(key)
    {
      case KB_TILDE:
      {
		    if(!isVisible())
			    show();
		    else
			    hide();
      }
      break;

      case KB_BACKSPACE:
      {
        if(visible)
	      {
		      int len = strlen(current);
		      if(len > 0)
			      current[len-1] = '\0';

		      strcpy(oldCurrent, current);
        }
      }
      break;

      case KB_RETURN:
      {
        if(visible)
	      {
		      processCommand(current);

		      current[0] = '\0';

		      strcpy(oldCurrent, current);
        }
      }
      break;

      case KB_UP:
      {
        if(visible)
	      {
		      if(cmdLog.size() > 0)
		      {
			      cmdLogNumber++;
			      
			      while(cmdLogNumber > 0 && cmdLogNumber > (int)cmdLog.size())
				      cmdLogNumber--;

			      if(cmdLogNumber > 0 && cmdLogNumber <= (int)cmdLog.size())
            {
              list<stringx>::iterator node = cmdLog.begin();
            
              int i = cmdLogNumber-1;
              while(i > 0 && node != cmdLog.end())
              {
                node++;
                i--;
              }

              if(node != cmdLog.end())
  				      strcpy(current, (*node).c_str());
            }
		      }
        }
      }
      break;

      case KB_DOWN:
      {
        if(visible)
	      {
		      if(cmdLog.size() > 0)
		      {
			      cmdLogNumber--;

			      if(cmdLogNumber <= 0)
			      {
				      cmdLogNumber = 0;

				      strcpy(current, oldCurrent);
			      }
			      else
			      {
              list<stringx>::iterator node = cmdLog.begin();
            
              int i = cmdLogNumber-1;
              while(i > 0 && node != cmdLog.end())
              {
                node++;
                i--;
              }

              if(node != cmdLog.end())
				        strcpy(current, (*node).c_str());
			      }
		      }
        }
      }
      break;

      case KB_TAB:
      {
        if(visible)
	      {
		      if(strlen(current) > 0)
		      {
            list<stringx> lst;
			      getMatchingCmds(current, lst);

			      if(lst.size() > 0)
			      {
				      if(lst.size() == 1)
				      {
					      strcpy(current, (*(lst.begin())).c_str());
                strcat(current, " ");
					      strcpy(oldCurrent, current);
				      }
				      else
				      {
                partialCompleteCmd(current, lst);
        	      strcpy(oldCurrent, current);

					      addToLog("");
					      addToLog("<--- %d matches --->", lst.size());

					      list<stringx>::iterator node = lst.begin();

					      while(node != lst.end())
					      {
						      addToLog("%s", (*node).c_str());
                  node++;
					      }
				      }

              while(lst.size() > 0)
                lst.pop_front();
			      }
			      else
			      {
				      addToLog("");
				      addToLog("<--- No matches --->");
			      }
		      }
        }
      }
      break;

      case KB_PAGEUP:
      {
        if(visible)
	      {
		      if(lineNumber < ((int)log.size()-1))
			      lineNumber++;
        }
      }
      break;

      case KB_PAGEDOWN:
      {
        if(visible)
	      {
		      if(lineNumber > 0)
			      lineNumber--;
        }
      }
      break;

      case KB_HOME:
      {
        if(visible)
	      {
		      lineNumber = log.size()-1;
        }
      }
      break;

      case KB_END:
      {
        if(visible)
	      {
		      lineNumber = 0;
        }
      }
      break;
    }
  }

#endif

void Console::render()
{
  color32 color32_consoletext(100,100,100);
	if(visible)
	{
    bitmap_widget::render();

#if defined(TARGET_PC)
    rational_t fontSize = 14;
		rational_t yTextPos = consoleHeight - 20.0f;
#else
    rational_t fontSize = 18;
		rational_t yTextPos = consoleHeight - 24.0f;
#endif

    stringx print_str;

    print_str = stringx("> ") + current + (cursor ? "_" : "");
    hw_rasta::inst()->print( print_str, vector2di(10, yTextPos), color32_consoletext );

		yTextPos -= fontSize;

		if(lineNumber > 0)
		{
      hw_rasta::inst()->print( "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^", vector2di(10, yTextPos), color32_consoletext);		
			yTextPos -= fontSize;
		}

		list<stringx>::iterator node = log.begin();
		int i = 0;
		while(node != log.end() && yTextPos > 0)
		{
			if(i >= lineNumber)
			{
        if((*node).length() > 0)
        {
          hw_rasta::inst()->print( *node, vector2di(10, yTextPos), color32_consoletext );		
        }

				yTextPos -= fontSize;
			}

      ++node;
			++i;
		}
	}
}

void Console::show()
{
  widget::show();
	visible = 1;

  flush();
  set_color(0, 0.5f, color(1.0f, 1.0f, 1.0f, 1.0f));

  input_mgr::inst()->disable_keyboard();
}

void Console::hide()
{
  widget::hide();
	visible = 0;

  flush();
  set_color(0, 0.5f, color(1.0f, 1.0f, 1.0f, 0.0f));

  input_mgr::inst()->enable_keyboard();
}

void Console::addToLog(const char *format, ...)
{
	if(format != NULL)
	{
		char string[4096] = "";

    va_list args;
		va_start(args, format);
		vsprintf(string, format, args);
    va_end(args);

    char *newline = NULL;
    char *strptr = &string[0];
    while((newline = strchr(strptr, '\n')) != NULL)
    {
    	char string2[1024] = "";

      int len = newline - strptr;
      strncpy(string2, strptr, len);
      string2[len] = '\0';

  		log.push_front(stringx(string2));

      strptr = newline+1;
    }
		log.push_front(stringx(strptr));

		while((int)log.size() > logMaxSize)
  		log.pop_back();

#if defined(TARGET_PC)
		if(fileLogging)
		{
			if(hardLogging)
			{
				fileLog.open(fileLogName.c_str(), ios::out | ios::app);
        if(fileLog.is_open())
        {
  				fileLog<<string<<"\n";
	  			fileLog.close();
        }
			}
			else
			{
				fileLog<<string<<"\n";
			}
		}
#endif
  }
}

void Console::addToCommandLog(const char *string)
{
	cmdLog.push_front(stringx(string));

	while((int)cmdLog.size() > cmdLogMaxSize)
		cmdLog.pop_back();

	cmdLogNumber = 0;
}

bool Console::get_param(const char *parms, char *p, int &index)
{
  if(parms)
  {
    while(parms[index] == ' ')
      index++;

    int start = index;

    while(parms[index] != ' ' && parms[index] != '\0')
      index++;

    int end = index;

    if(start != end && p != NULL)
    {
      strncpy(p, &parms[start], end - start);
      p[end - start] = '\0';

      while(parms[index] == ' ')
        index++;

      return(true);
    }
  }
    
  return(false);
}

void Console::processCommand(const char *command, char log)
{
	if(command != NULL && strlen(command) > 0)
	{
		if(log)
			addToCommandLog(command);

    /*static*/ vector<stringx> args;  // why was this static if it just got cleared right away?
    args.resize(0);

		char parm[256]="";
    int index = 0;
    get_param(command, parm, index);

    stringx cmd_name(parm);
    cmd_name.to_lower();

		ConsoleCommand *cmd = getCommand(cmd_name);
		if(cmd != NULL)
		{
      while(get_param(command, parm, index))
        args.push_back(stringx(parm));

			if(log)
				addToLog(command);

			if(!cmd->process_cmd(args) && log)
				addToLog("??? %s", command);
		}
		else
		{
			ConsoleVariable *var = getVariable(cmd_name);

      if(var != NULL)
			{
        args.push_back(cmd_name);

        while(get_param(command, parm, index))
          args.push_back(stringx(parm));

				if(log)
					addToLog(command);

        if(args.size() > 1)
          cmd = getCommand("set");
        else
          cmd = getCommand("get");

        cmd->process_cmd(args);
			}
			else if(log)
			{
				addToLog("??? %s", command);
			}
		}
	}
}

void Console::exec(const stringx &file)
{
#if defined(TARGET_PC)
	ifstream fin;
	fin.open(file.c_str(), ios::in | ios::nocreate);
	if(!fin.fail())
	{
		addToLog("Executing '%s'", file.c_str());
		char buf[4096]="";
		
		while(fin.getline(buf, 4095))
			processCommand(buf, 1);

		fin.close();
	}
	else
		addToLog("File '%s' not found", file.c_str());
#endif
}

void Console::logToFile(const stringx &file, bool append, bool hard)
{
#if defined(TARGET_PC)
  endLogToFile();

  if(file.length() > 0)
  {
    if(append)
      fileLog.open(file.c_str(), ios::out | ios::app);
    else
      fileLog.open(file.c_str(), ios::out | ios::trunc);
  }
  else
  {
    if(append)
      fileLog.open(fileLogName.c_str(), ios::out | ios::app);
    else
      fileLog.open(fileLogName.c_str(), ios::out | ios::trunc);
  }
	
  if(fileLog.is_open())
  {
    if(file.length() > 0)
      fileLogName = file;

	  hardLogging = hard;
    fileLogging = true;

	  if(hardLogging)
		  fileLog.close();
  }
#endif
}

void Console::endLogToFile()
{
#if defined(TARGET_PC)
	if(hardLogging)
	{
		fileLogging = false;
	}
	else
	{
		if(fileLogging)
		{
			fileLog.close();
			fileLogging = false;
		}
	}
#endif
}

void Console::frame_advance(time_value_t time_inc)
{
  bitmap_widget::frame_advance(time_inc);

  cursorTick += time_inc;
  if(renderCursor && cursorTick >= 0.5f)
	{
		if(cursor)
			cursor = 0;
		else
			cursor = 1;

		cursorTick = 0.0f;
	}
}

#endif  // _CONSOLE_ENABLE
