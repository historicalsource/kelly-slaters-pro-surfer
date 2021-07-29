#include "global.h"

#include "script_object.h"
#include "script_library_class.h"
#include "vm_thread.h"
#include "vm_symbol_list.h"
#include "vm_symbol.h"
#include "vm_stack.h"
#include "vm_executable.h"
#include "vm_executable_vector.h"
#include "so_data_block.h"
#include "opcodes.h"
#include "winapp.h"
#include "entity.h"
#include "wds.h"

#include "sl_debugger.h"

#if defined(TARGET_PC) && !defined(BUILD_BOOTABLE)

sl_debugger g_sl_debugger;

#include "resource.h"

bool g_script_debugger_running = false;

void show_sl_debugger()
{
  g_sl_debugger.show();
}

void hide_sl_debugger()
{
  g_sl_debugger.hide();
}

void system_idle(); // in w32_main or sy_main

BOOL CALLBACK gDebuggerDialogFunc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
  assert(g_sl_debugger.debugger_dlg);
  return(g_sl_debugger.debugger_dlg->handle_message(hDlg, msg, wParam, lParam));
}

BOOL CALLBACK gAddBreakpointDialogFunc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
  assert(g_sl_debugger.add_breakpoint_dlg);
  return(g_sl_debugger.add_breakpoint_dlg->handle_message(hDlg, msg, wParam, lParam));
}

BOOL CALLBACK gAddWatchDialogFunc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
  assert(g_sl_debugger.add_watch_dlg);
  return(g_sl_debugger.add_watch_dlg->handle_message(hDlg, msg, wParam, lParam));
}


long far pascal DebuggerWindowProc ( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam )
{
  switch ( message )
	{
  	// window going away
		case WM_DESTROY:
		{
      g_sl_debugger.destroy();
  	}
 		break;

		case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
        case IDM_THREAD_BREAK:
        {
          g_sl_debugger.break_at_threads = !g_sl_debugger.break_at_threads;
          check_menu(hwnd, IDM_THREAD_BREAK, g_sl_debugger.break_at_threads);
        }
        break;

        case IDM_AUTO_UPDATES:
        {
          g_sl_debugger.update_at_threads = !g_sl_debugger.update_at_threads;
          check_menu(hwnd, IDM_AUTO_UPDATES, g_sl_debugger.update_at_threads);
        }
        break;

				case IDM_ADD_BREAKPOINT:
				{
          g_sl_debugger.add_breakpoint_dlg->show();
				}
				break;

				case IDM_ADD_WATCH:
				{
          g_sl_debugger.add_watch_dlg->show();
				}
				break;

				case IDM_ENABLE_ALL_BREAKPOINTS:
				{
          g_sl_debugger.enable_all_breakpoints();
				}
				break;

				case IDM_DISABLE_ALL_BREAKPOINTS:
				{
          g_sl_debugger.disable_all_breakpoints();
				}
				break;

        case IDM_EXIT:
				{
					hide_sl_debugger();
          SetForegroundWindow(windows_app::inst()->get_hwnd());
				}
				break;
			}
		}

 		default:
 			return DefWindowProc ( hwnd, message, wParam, lParam );
  }

	return 0;
}


sl_debugger::sl_debugger()
{
  main_hwnd = NULL;
  debugger_dlg = NULL;
  add_breakpoint_dlg = NULL;
  add_watch_dlg = NULL;

  window_created = false;
  window_visible = false;

  cur_object = NULL;
  cur_instance = NULL;
  cur_thread = NULL;

  object_count = 0;
  instance_count = 0;
  thread_count = 0;

  object_total = 0;
  instance_total = 0;
  thread_total = 0;

  run_thread = false;
  break_at_threads = true;
  update_at_threads = false;

  run_to_next_object = false;
  run_to_next_instance = false;
  run_to_next_frame = false;

  new_frame = false;
  new_object = false;
  new_instance = false;
  new_thread = false;

  continue_run = true;
}

sl_debugger::~sl_debugger()
{
  destroy();
}

void sl_debugger::destroy()
{
  window_created = false;
  window_visible = false;

  if(add_breakpoint_dlg)
  {
    add_breakpoint_dlg->hide();
    delete add_breakpoint_dlg;
    add_breakpoint_dlg = NULL;
  }

  if(add_watch_dlg)
  {
    add_watch_dlg->hide();
    delete add_watch_dlg;
    add_watch_dlg = NULL;
  }

  if(debugger_dlg)
  {
    debugger_dlg->hide();
    delete debugger_dlg;
    debugger_dlg = NULL;
  }

  if(IsWindow(main_hwnd))
  {
    DestroyWindow(main_hwnd);
    main_hwnd = NULL;
  }

  g_script_debugger_running = false;
}


void sl_debugger::show()
{
  if(!window_created)
  {
	  static char szClassName[30] = "sl_debugger_main";

	  RECT adjRect;

	  // create our display window class
	  WNDCLASS wc;

	  wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	  wc.lpfnWndProc = DebuggerWindowProc;
	  wc.cbClsExtra = 0;
	  wc.cbWndExtra = 0;
	  wc.hInstance = GetModuleHandle(NULL);
	  wc.hIcon = NULL;
	  wc.hCursor = NULL;
	  wc.hbrBackground = (struct HBRUSH__ *)GetStockObject(LTGRAY_BRUSH);
	  wc.lpszMenuName = MAKEINTRESOURCE ( IDR_SCRIPT_DEBUGGER_MENU );
	  wc.lpszClassName = szClassName;

	  RegisterClass ( &wc );

	  adjRect.left = 0;
	  adjRect.top = 0;
	  adjRect.right = 160;
	  adjRect.bottom = 120;

	  AdjustWindowRectEx ( &adjRect, WS_OVERLAPPEDWINDOW, 1, 0 );

	  RECT workArea;
	  SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);

	  int width = adjRect.right-adjRect.left;
	  int height = adjRect.bottom-adjRect.top;

	  if(width > (workArea.right-workArea.left))
		  width = workArea.right-workArea.left;

	  if(height > (workArea.bottom-workArea.top))
		  height = workArea.bottom-workArea.top;

	  main_hwnd = CreateWindowEx (WS_EX_CONTROLPARENT | WS_EX_OVERLAPPEDWINDOW,
		          szClassName,"Script Language Debugger",WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_CLIPCHILDREN,
		          workArea.left, workArea.top, width, height,
		          NULL,NULL,GetModuleHandle(NULL),NULL);

	  debugger_dlg = NEW DebuggerDialog(main_hwnd);
	  debugger_dlg->set_pos(0, 0);
	  debugger_dlg->show();

	  size_client(main_hwnd, debugger_dlg->get_width(), debugger_dlg->get_height());

    window_created = true;


	  add_breakpoint_dlg = NEW AddBreakpointDialog(main_hwnd);
	  add_watch_dlg = NEW AddWatchDialog(main_hwnd);
  }

	ShowWindow(main_hwnd, SW_SHOW);
  debugger_dlg->show();

	UpdateWindow(main_hwnd);
  window_visible = true;
  g_script_debugger_running = true;

  check_menu(main_hwnd, IDM_THREAD_BREAK, break_at_threads);
  check_menu(main_hwnd, IDM_AUTO_UPDATES, update_at_threads);
}

void sl_debugger::hide()
{
  if(window_created && window_visible)
  {
	  debugger_dlg->hide();
    add_breakpoint_dlg->hide();
  	ShowWindow(main_hwnd, SW_HIDE);
    window_visible = false;
  }

  g_script_debugger_running = false;
}

void sl_debugger::update()
{
}

bool sl_debugger::thread_break()
{
  if(breakpoints.size())
  {
    vector<sl_breakpoint>::iterator b = breakpoints.begin();
    vector<sl_breakpoint>::iterator b_end = breakpoints.end();

    while(b != b_end)
    {
      if((*b).active &&
          (
          ((*b).type == sl_breakpoint::_BREAK_FUNCTION && cur_thread != NULL && ((*b).data == cur_thread->get_executable()->get_name() || (*b).data == cur_thread->get_executable()->get_fullname()))
          || ((*b).type == sl_breakpoint::_BREAK_INSTANCE && cur_instance != NULL && (*b).data == cur_instance->get_name().c_str())
          || ((*b).type == sl_breakpoint::_BREAK_OBJECT && cur_object != NULL && (*b).data == cur_object->get_name())
          )
        )
      {
        return(true);
      }

      ++b;
    }
  }

  return(false);
}

void sl_debugger::wait_thread()
{
  bool updated = false;

  if(update_at_threads)
  {
    update_threads();
    update_watches();
    updated = true;
  }

  thread_broke = thread_break();

  while(g_script_debugger_running && (thread_broke || misc_thread_break()))
  {
    continue_run = false;

    if(!updated)
    {
      update_threads();
      update_watches();
      updated = true;
    }

    system_idle();
    Sleep(0);
  }
}

bool sl_debugger::misc_thread_break()
{
  if(continue_run && !break_at_threads)
    return(false);

  if(run_thread)
    return(false);

  if(run_to_next_instance && new_instance)
    return(true);

  if(run_to_next_object && new_object)
    return(true);

  if(run_to_next_frame && new_frame)
    return(true);

//  (break_at_threads && (!run_thread && (!run_to_next_instance || (run_to_next_instance && new_instance)) && (!run_to_next_object || (run_to_next_object && new_object)) && (!run_to_next_frame || (run_to_next_frame && new_frame)))))
  return(break_at_threads && !run_thread && !run_to_next_frame && !run_to_next_object && !run_to_next_instance);
}


void sl_debugger::add_breakpoint(const sl_breakpoint& break_pt)
{
  breakpoints.push_back(break_pt);
  update_breakpoints();
}

void sl_debugger::remove_breakpoint(int index)
{
  int i = 0;

  vector<sl_breakpoint>::iterator b = breakpoints.begin();

  while(b != breakpoints.end())
  {
    if(i == index)
    {
      breakpoints.erase(b);
      break;
    }

    ++b;
    ++i;
  }

  update_breakpoints();
}

void sl_debugger::enable_breakpoint(int index)
{
  int i = 0;

  vector<sl_breakpoint>::iterator b = breakpoints.begin();
  vector<sl_breakpoint>::iterator b_end = breakpoints.end();
  while(b != b_end)
  {
    if(i == index)
    {
      (*b).active = true;
      break;
    }

    ++b;
    ++i;
  }

  update_breakpoints();
}

void sl_debugger::disable_breakpoint(int index)
{
  int i = 0;

  vector<sl_breakpoint>::iterator b = breakpoints.begin();
  vector<sl_breakpoint>::iterator b_end = breakpoints.end();
  while(b != b_end)
  {
    if(i == index)
    {
      (*b).active = false;
      break;
    }

    ++b;
    ++i;
  }

  update_breakpoints();
}

void sl_debugger::toggle_breakpoint(int index)
{
  int i = 0;

  vector<sl_breakpoint>::iterator b = breakpoints.begin();
  vector<sl_breakpoint>::iterator b_end = breakpoints.end();
  while(b != b_end)
  {
    if(i == index)
    {
      (*b).active = !(*b).active;
      break;
    }

    ++b;
    ++i;
  }

  update_breakpoints();
}

void sl_debugger::enable_all_breakpoints()
{
  vector<sl_breakpoint>::iterator b = breakpoints.begin();
  vector<sl_breakpoint>::iterator b_end = breakpoints.end();
  while(b != b_end)
  {
    (*b).active = true;
    ++b;
  }

  update_breakpoints();
}

void sl_debugger::disable_all_breakpoints()
{
  vector<sl_breakpoint>::iterator b = breakpoints.begin();
  vector<sl_breakpoint>::iterator b_end = breakpoints.end();
  while(b != b_end)
  {
    (*b).active = false;
    ++b;
  }

  update_breakpoints();
}


void sl_debugger::add_watch(const sl_watch& watch_var)
{
  watches.push_back(watch_var);
  update_watches();
}

void sl_debugger::remove_watch(int index)
{
  int i = 0;

  vector<sl_watch>::iterator w = watches.begin();

  while(w != watches.end())
  {
    if(i == index)
    {
      watches.erase(w);
      break;
    }

    ++w;
    ++i;
  }

  update_watches();
}


void sl_debugger::set_new_frame(int num_objects)
{
  cur_object = NULL;
  cur_instance = NULL;
  cur_thread = NULL;

  new_frame = true;
  new_object = false;
  new_instance = false;
  new_thread = false;

  object_total = num_objects;
  object_count = 0;
/*
  set_ctrl_text(IDC_OBJECT, "");
  set_ctrl_text(IDC_INSTANCE, "");
  set_ctrl_text(IDC_THREAD, "");

  set_ctrl_text(IDC_OBJECT_NUM, "0 / 0");
  set_ctrl_text(IDC_INSTANCE_NUM, "0 / 0");
  set_ctrl_text(IDC_THREAD_NUM, "0 / 0");
*/
}

void sl_debugger::set_new_object(script_object *obj, int num_instances)
{
  cur_object = obj;
  new_object = true;
  instance_total = num_instances;
  instance_count = 0;
  ++object_count;
}

void sl_debugger::set_new_instance(script_object::instance *inst, int num_threads)
{
  cur_instance = inst;
  new_instance = true;
  thread_total = num_threads;
  thread_count = 0;
  ++instance_count;
}

void sl_debugger::set_new_thread(vm_thread *thr)
{
  cur_thread = thr;
  new_thread = true;
  run_thread = false;
  ++thread_count;
}

void sl_debugger::update_threads()
{
  debugger_dlg->set_ctrl_text(IDC_OBJECT, "%s", cur_object != NULL ? cur_object->get_name().c_str() : "");
  debugger_dlg->set_ctrl_text(IDC_OBJECT_NUM, "%d / %d", object_count, object_total);

  debugger_dlg->set_ctrl_text(IDC_INSTANCE, "%s", cur_instance != NULL ? cur_instance->get_name().c_str() : "");
  debugger_dlg->set_ctrl_text(IDC_INSTANCE_NUM, "%d / %d", instance_count, instance_total);

  debugger_dlg->set_ctrl_text(IDC_THREAD, "%s", cur_thread != NULL ? cur_thread->get_executable()->get_fullname().c_str() : "");
  debugger_dlg->set_ctrl_text(IDC_THREAD_NUM, "%d / %d", thread_count, thread_total);
}

void sl_debugger::update_breakpoints()
{
  debugger_dlg->clear_list_view(IDC_BREAKPOINTS);

  vector<sl_breakpoint>::iterator b = breakpoints.begin();
  vector<sl_breakpoint>::iterator b_end = breakpoints.end();
  int i = 0;
  while(b != b_end)
  {
    switch((*b).type)
    {
      case sl_breakpoint::_BREAK_OBJECT:
        debugger_dlg->insert_list_view_row(IDC_BREAKPOINTS, i, "Object|%s|%s", (*b).active ? "Y" : "N", (*b).data.c_str());
        break;

      case sl_breakpoint::_BREAK_INSTANCE:
        debugger_dlg->insert_list_view_row(IDC_BREAKPOINTS, i, "Instance|%s|%s", (*b).active ? "Y" : "N", (*b).data.c_str());
        break;

      case sl_breakpoint::_BREAK_FUNCTION:
        debugger_dlg->insert_list_view_row(IDC_BREAKPOINTS, i, "Function|%s|%s", (*b).active ? "Y" : "N", (*b).data.c_str());
        break;
    }

    ++i;
    ++b;
  }
}

void sl_debugger::update_watches()
{
  debugger_dlg->clear_list_view(IDC_WATCHES);

  vector<sl_watch>::iterator w = watches.begin();
  vector<sl_watch>::iterator w_end = watches.end();
  int i = 0;
  while(w != w_end)
  {
    switch((*w).type)
    {
      case sl_watch::_WATCH_IFC:
      {
        if((*w).data_type == sl_watch::_WATCH_DATA_NUM)
        {
          rational_t val = 0.0f;
          stringx vals = empty_string;

          if((*w).ifc_ent == NULL)
            vals = stringx(stringx::fmt, "No Entity with ID '%s'!", (*w).ifc_ent_name.c_str());
          else
          {
            if((*w).ifc_ent->get_ifc_num((*w).ifc_att, val))
              vals = stringx(val);
            else
             vals = stringx(stringx::fmt, "Entity '%s' does not have an IFC num '%s'!", (*w).ifc_ent_name.c_str(), (*w).ifc_att.c_str());
          }

          debugger_dlg->insert_list_view_row(IDC_WATCHES, i, "IFC|%s::%s|%s", (*w).ifc_ent_name.c_str(), (*w).ifc_att.c_str(), vals.c_str());
        }
        else if((*w).data_type == sl_watch::_WATCH_DATA_STR)
        {
          stringx val = empty_string;

          if((*w).ifc_ent == NULL)
            val = stringx(stringx::fmt, "No Entity with ID '%s'!", (*w).ifc_ent_name.c_str());
          else
          {
            if(!(*w).ifc_ent->get_ifc_str((*w).ifc_att, val))
             val = stringx(stringx::fmt, "Entity '%s' does not have an IFC str '%s'!", (*w).ifc_ent_name.c_str(), (*w).ifc_att.c_str());
          }

          debugger_dlg->insert_list_view_row(IDC_WATCHES, i, "IFC|%s::%s|%s", (*w).ifc_ent_name.c_str(), (*w).ifc_att.c_str(), val.c_str());
        }
      }
      break;
    }

    ++i;
    ++w;
  }
}















DebuggerDialog::DebuggerDialog(HWND pParent)
{
	init(IDD_SCRIPT_MAIN_DEBUGGER, gDebuggerDialogFunc, pParent);
}

DebuggerDialog::~DebuggerDialog()
{
}


void DebuggerDialog::setup()
{
  set_list_view_columns(IDC_BREAKPOINTS, "Type|Active|Data");
  auto_size_list_view_columns(IDC_BREAKPOINTS);

  set_list_view_columns(IDC_WATCHES, "Type|Name|Value");
  auto_size_list_view_columns(IDC_WATCHES);
}

void DebuggerDialog::show()
{
	Dialog::show();
}

void DebuggerDialog::hide()
{
	Dialog::hide();
}

void DebuggerDialog::update()
{
	if(!is_visible())
		return;

  Dialog::update();
}

void DebuggerDialog::frame_advance(time_value_t t)
{
	if(!is_visible())
		return;
}

void DebuggerDialog::handle_command(int ctrl)
{
  switch(ctrl)
  {
    case IDC_RUN_TO_NEXT_THREAD:
      g_sl_debugger.run_thread = true;
      g_sl_debugger.thread_broke = false;

      g_sl_debugger.continue_run = false;
      g_sl_debugger.run_to_next_instance = false;
      g_sl_debugger.run_to_next_object = false;
      g_sl_debugger.run_to_next_frame = false;
    break;

    case IDC_RUN_TO_NEXT_INSTANCE:
      g_sl_debugger.run_to_next_instance = true;
      g_sl_debugger.thread_broke = false;
      g_sl_debugger.new_instance = false;

      g_sl_debugger.continue_run = false;
      g_sl_debugger.run_to_next_object = false;
      g_sl_debugger.run_to_next_frame = false;
      g_sl_debugger.run_thread = false;
    break;

    case IDC_RUN_TO_NEXT_OBJECT:
      g_sl_debugger.run_to_next_object = true;
      g_sl_debugger.thread_broke = false;
      g_sl_debugger.new_object = false;

      g_sl_debugger.continue_run = false;
      g_sl_debugger.run_to_next_instance = false;
      g_sl_debugger.run_to_next_frame = false;
      g_sl_debugger.run_thread = false;
    break;

    case IDC_RUN_TO_NEXT_FRAME:
      g_sl_debugger.run_to_next_frame = true;
      g_sl_debugger.thread_broke = false;
      g_sl_debugger.new_frame = false;

      g_sl_debugger.continue_run = false;
      g_sl_debugger.run_to_next_instance = false;
      g_sl_debugger.run_to_next_object = false;
      g_sl_debugger.run_thread = false;
    break;

    case IDC_RUN:
      g_sl_debugger.continue_run = true;
      g_sl_debugger.run_thread = true;
      g_sl_debugger.thread_broke = false;
    break;

    case IDC_REMOVE_BREAKPOINT:
    {
	    int num = get_list_view_multiple_count(IDC_BREAKPOINTS);
      assert(num < 1024);
      int data[1024];
	    get_list_view_multiple_selection(IDC_BREAKPOINTS, &data[0], 1024);

      for(int i=0; i<num; i++)
        g_sl_debugger.remove_breakpoint(data[i]);
    }
    break;

    case IDC_TOGGLE_BREAKPOINT:
    {
	    int num = get_list_view_multiple_count(IDC_BREAKPOINTS);
      assert(num < 1024);
      int data[1024];
	    get_list_view_multiple_selection(IDC_BREAKPOINTS, &data[0], 1024);

      for(int i=0; i<num; i++)
        g_sl_debugger.toggle_breakpoint(data[i]);
    }
    break;

    case IDC_REMOVE_WATCH:
    {
	    int num = get_list_view_multiple_count(IDC_WATCHES);
      assert(num < 1024);
      int data[1024];
	    get_list_view_multiple_selection(IDC_WATCHES, &data[0], 1024);

      for(int i=0; i<num; i++)
        g_sl_debugger.remove_watch(data[i]);
    }
    break;

    default:
      Dialog::handle_command(ctrl);
      break;
  }
}

void DebuggerDialog::handle_double_click(int ctrl)
{
  switch(ctrl)
  {
    case IDC_BREAKPOINTS:
    {
	    int num = get_list_view_multiple_count(IDC_BREAKPOINTS);
      assert(num < 1024);
      int data[1024];
	    get_list_view_multiple_selection(IDC_BREAKPOINTS, &data[0], 1024);

      for(int i=0; i<num; i++)
        g_sl_debugger.toggle_breakpoint(data[i]);
    }
    break;

    default:
      Dialog::handle_double_click(ctrl);
      break;
  }
}





AddBreakpointDialog::AddBreakpointDialog(HWND pParent)
{
	init(IDD_SCRIPT_ADD_BREAKPOINT, gAddBreakpointDialogFunc, pParent);
}

AddBreakpointDialog::~AddBreakpointDialog()
{
}


void AddBreakpointDialog::setup()
{
  clear_combo(IDC_TYPE);
  add_combo_string(IDC_TYPE, "Object");
  add_combo_string(IDC_TYPE, "Instance");
  add_combo_string(IDC_TYPE, "Function");
  set_combo_index(IDC_TYPE, 1);

  set_ctrl_text(IDC_DATA, "");
}

void AddBreakpointDialog::show()
{
	Dialog::show();
}

void AddBreakpointDialog::hide()
{
	Dialog::hide();
}

void AddBreakpointDialog::update()
{
	if(!is_visible())
		return;

  Dialog::update();
}


void AddBreakpointDialog::submit()
{
  char buf[1024];
  sl_breakpoint::eBreakpointType type = (sl_breakpoint::eBreakpointType)get_combo_index(IDC_TYPE);
  get_ctrl_text(IDC_DATA, &buf[0], 1024);
  g_sl_debugger.add_breakpoint(sl_breakpoint(type, stringx(buf), true));

  hide();
}



AddWatchDialog::AddWatchDialog(HWND pParent)
{
	init(IDD_SCRIPT_ADD_WATCH, gAddWatchDialogFunc, pParent);
}

AddWatchDialog::~AddWatchDialog()
{
}


void AddWatchDialog::setup()
{
  clear_combo(IDC_TYPE);
  add_combo_string(IDC_TYPE, "IFC");
  set_combo_index(IDC_TYPE, 0);

  clear_combo(IDC_DATA_TYPE);
  add_combo_string(IDC_DATA_TYPE, "Num");
  add_combo_string(IDC_DATA_TYPE, "Str");
  set_combo_index(IDC_DATA_TYPE, 0);

  set_ctrl_text(IDC_IFC_ENTITY, "");
  set_ctrl_text(IDC_IFC_NAME, "");
}

void AddWatchDialog::show()
{
	Dialog::show();
}

void AddWatchDialog::hide()
{
	Dialog::hide();
}

void AddWatchDialog::update()
{
	if(!is_visible())
		return;

  Dialog::update();
}


void AddWatchDialog::submit()
{
  char ent[1024];
  char ifc[1024];

  sl_watch::eWatchType type = (sl_watch::eWatchType)get_combo_index(IDC_TYPE);
  sl_watch::eWatchDataType data_type = (sl_watch::eWatchDataType)get_combo_index(IDC_DATA_TYPE);

  get_ctrl_text(IDC_IFC_ENTITY, &ent[0], 1024);
  get_ctrl_text(IDC_IFC_NAME, &ifc[0], 1024);

  sl_watch watch(type, data_type);
  watch.ifc_ent_name = stringx(ent);
  watch.ifc_ent_name.to_upper();
  watch.ifc_att = pstring(ifc);

  watch.ifc_ent = g_world_ptr->get_entity(watch.ifc_ent_name);


  g_sl_debugger.add_watch(watch);

  hide();
}



#endif
