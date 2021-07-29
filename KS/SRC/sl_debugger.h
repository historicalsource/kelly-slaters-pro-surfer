#ifndef _SL_DEBUGGER_H_
#define _SL_DEBUGGER_H_


#if defined(TARGET_PC) && !defined(BUILD_BOOTABLE)

//#include "w32_dialog.h"

#include "script_object.h"
#include "global.h"
#include "pstring.h"

class DebuggerDialog;
class AddBreakpointDialog;
class AddWatchDialog;
class vm_thread;

class sl_breakpoint
{
public:
  typedef enum
  {
    _BREAK_OBJECT,
    _BREAK_INSTANCE,
    _BREAK_FUNCTION,
    _BREAK_MAX
  } eBreakpointType;

  eBreakpointType type;
  stringx data;
  bool active;


  sl_breakpoint(eBreakpointType t = _BREAK_FUNCTION, const stringx &d = empty_string, bool a = true)
  {
    type = t;
    data = d;
    active = a;
  }

  sl_breakpoint(const sl_breakpoint &b) 
  { 
    copy(b); 
  }

  ~sl_breakpoint()  {}

  void copy(const sl_breakpoint &b)
  {
    type = b.type;
    data = b.data;
    active = b.active;
  }

  sl_breakpoint& operator=(const sl_breakpoint &b) 
  {
		copy( b );
    return *this;
  }
};

class entity;
class sl_watch
{
public:
  typedef enum
  {
    _WATCH_IFC,
    _WATCH_MAX
  } eWatchType;

  typedef enum
  {
    _WATCH_DATA_NUM,
    _WATCH_DATA_STR,
    _WATCH_DATA_PTR,
    _WATCH_DATA_MAX
  } eWatchDataType;

  eWatchType type;
  eWatchDataType data_type;

  stringx old_val_str;
  rational_t old_val_num;

  stringx ifc_ent_name;
  entity *ifc_ent;
  pstring ifc_att;

  sl_watch(eWatchType t = _WATCH_IFC, eWatchDataType dt = _WATCH_DATA_NUM)
  {
    type = t;
    data_type = dt;
  }

  sl_watch(const sl_watch &b) 
  { 
    copy(b); 
  }

  ~sl_watch()  {}

  void copy(const sl_watch &b)
  {
    type = b.type;
    data_type = b.data_type;

    old_val_str = b.old_val_str;
    old_val_num = b.old_val_num;

    ifc_ent_name = b.ifc_ent_name;
    ifc_ent = b.ifc_ent;
    ifc_att = b.ifc_att;
  }

  sl_watch& operator=(const sl_watch &b) 
  {
		copy( b );
    return *this;
  }
};


class sl_debugger
{
protected:
  friend BOOL CALLBACK gDebuggerDialogFunc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
  friend BOOL CALLBACK gAddBreakpointDialogFunc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
  friend BOOL CALLBACK gAddWatchDialogFunc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
  friend long far pascal DebuggerWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

  friend class DebuggerDialog;

  HWND main_hwnd;
  DebuggerDialog *debugger_dlg;
  AddBreakpointDialog *add_breakpoint_dlg;
  AddWatchDialog *add_watch_dlg;

  bool window_created;
  bool window_visible;
  bool thread_broke;

  void destroy();

  vector<sl_breakpoint> breakpoints;
  vector<sl_watch> watches;

  script_object *cur_object;
  script_object::instance *cur_instance;
  vm_thread *cur_thread;

  int object_count;
  int instance_count;
  int thread_count;

  int object_total;
  int instance_total;
  int thread_total;

  bool run_thread;
  bool break_at_threads;
  bool update_at_threads;

  bool run_to_next_object;
  bool run_to_next_instance;
  bool run_to_next_frame;

  bool new_frame;
  bool new_object;
  bool new_instance;
  bool new_thread;

  bool continue_run;

public:
  sl_debugger();
  ~sl_debugger();

  void show();
  void hide();
  void update();

  void wait_thread();

  bool thread_break();
  bool misc_thread_break();

  void add_breakpoint(const sl_breakpoint& break_pt);
  void remove_breakpoint(int index);
  void enable_breakpoint(int index);
  void disable_breakpoint(int index);
  void toggle_breakpoint(int index);
  void enable_all_breakpoints();
  void disable_all_breakpoints();

  void set_new_frame(int num_objects);
  void set_new_object(script_object *obj, int num_instances);
  void set_new_instance(script_object::instance *inst, int num_threads);
  void set_new_thread(vm_thread *thr);

  void update_threads();
  void update_breakpoints();
  void update_watches();

  void add_watch(const sl_watch& watch_var);
  void remove_watch(int index);
};


class DebuggerDialog : public Dialog
{
protected:

public:
  DebuggerDialog(HWND pParent);
  ~DebuggerDialog();

  virtual void setup();
	virtual void show();
	virtual void hide();
	virtual void update();
  
  virtual void frame_advance(time_value_t t);
  virtual void handle_command(int ctrl);

  virtual void handle_double_click(int ctrl);
};

class AddBreakpointDialog : public Dialog
{
protected:

public:
  AddBreakpointDialog(HWND pParent);
  ~AddBreakpointDialog();

  virtual void setup();
	virtual void show();
	virtual void hide();
	virtual void update();

	virtual void submit();
};

class AddWatchDialog : public Dialog
{
protected:

public:
  AddWatchDialog(HWND pParent);
  ~AddWatchDialog();

  virtual void setup();
	virtual void show();
	virtual void hide();
	virtual void update();

	virtual void submit();
};

void show_sl_debugger();
void hide_sl_debugger();

extern bool g_script_debugger_running;
extern sl_debugger g_sl_debugger;

#else

#define show_sl_debugger()          {}
#define hide_sl_debugger()          {}

#define g_script_debugger_running 0

#endif

#endif