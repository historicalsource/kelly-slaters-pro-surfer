#include "global.h"

#ifndef _TOOL_DIALOGS_H_
#define _TOOL_DIALOGS_H_

#if _ENABLE_WORLD_EDITOR

#include "hwospc\w32_dialog.h"
#include "resource.h"
#include "ostimer.h"
#include "path.h"

class entity;

void get_screen_to_world_segment(vector2di screen_pos, vector3d &origin, vector3d &end, vector3d &normal, rational_t surface_dist = 0.0f);


class SpawnDialog : public Dialog
{
public:
	SpawnDialog(HWND pParent);
	~SpawnDialog();
	
	virtual void setup();
	virtual void hide();
	virtual void update();
  virtual void render();


  void spawn_entity();

  virtual void populate_ent_files();

  virtual void frame_advance(time_value_t t);

  virtual void handle_command(int ctrl);
	virtual void handle_selection_change(int ctrl);

  virtual void handle_user_mouse_down(int button, vector3d pos, DWORD flags);
  virtual void handle_user_mouse_up(int button, vector3d pos, DWORD flags);

  int last_radio;
  vector3d spawn_point;

  bool gen_name;
  bool gen_suffix;
  bool orient;

  float surface_dist;

  virtual void generate_name();
};

class AlterDialog;

class MarkerDialog : public Dialog
{
public:
	MarkerDialog(HWND pParent);
	~MarkerDialog();

	virtual void setup();
	virtual void hide();
	virtual void update();
  virtual void render();


  virtual void frame_advance(time_value_t t);

  virtual void handle_slider(int ctrl, int pos);

  virtual void handle_command(int ctrl);

  AlterDialog *alter_dlg;

  virtual void handle_user_mouse_down(int button, vector3d pos, DWORD flags) {};
  virtual void handle_user_mouse_up(int button, vector3d pos, DWORD flags) {};
};

class ItemDialog : public Dialog
{
public:
	ItemDialog(HWND pParent);
	~ItemDialog();

	virtual void setup();
	virtual void hide();
	virtual void update();
  virtual void render();


  virtual void frame_advance(time_value_t t);

  virtual void handle_slider(int ctrl, int pos);

  virtual void handle_command(int ctrl);

  AlterDialog *alter_dlg;

  virtual void handle_user_mouse_down(int button, vector3d pos, DWORD flags) {};
  virtual void handle_user_mouse_up(int button, vector3d pos, DWORD flags) {};
};

class DestroyDialog : public Dialog
{
public:
	DestroyDialog(HWND pParent);
	~DestroyDialog();

	virtual void setup();
	virtual void hide();
	virtual void update();
  virtual void render();


  virtual void frame_advance(time_value_t t);

  virtual void handle_slider(int ctrl, int pos);

  virtual void handle_command(int ctrl);

  AlterDialog *alter_dlg;

  virtual void handle_user_mouse_down(int button, vector3d pos, DWORD flags) {};
  virtual void handle_user_mouse_up(int button, vector3d pos, DWORD flags) {};
};

class EntityDialog : public Dialog
{
public:
	EntityDialog(HWND pParent);
	~EntityDialog();

	virtual void setup();
	virtual void hide();
	virtual void update();
  virtual void render();


  virtual void frame_advance(time_value_t t);

  virtual void handle_command(int ctrl);

  AlterDialog *alter_dlg;

  virtual void handle_user_mouse_down(int button, vector3d pos, DWORD flags) {};
  virtual void handle_user_mouse_up(int button, vector3d pos, DWORD flags) {};
};

class RotateDialog : public Dialog
{
public:
	RotateDialog(HWND pParent);
	~RotateDialog();

  rational_t inc;
  bool continuous;
  bool relative;
	
	virtual void setup();
	virtual void hide();
	virtual void update();
  virtual void render();


  virtual void frame_advance(time_value_t t);

  virtual void handle_command(int ctrl);

  AlterDialog *alter_dlg;
  bool mouse_down;
  vector3d mouse_pos;

  bool rot_z;
  bool speed;

  bool enable_mouse;
  bool mouse_relative;
  rational_t mouse_speed;

  virtual void handle_user_mouse_down(int button, vector3d pos, DWORD flags);
  virtual void handle_user_mouse_up(int button, vector3d pos, DWORD flags);
};

class TranslateDialog : public Dialog
{
public:
	TranslateDialog(HWND pParent);
	~TranslateDialog();

  rational_t inc;
  bool continuous;
  bool relative;
  virtual void render();

	
	virtual void setup();
	virtual void hide();
	virtual void update();

  virtual void frame_advance(time_value_t t);

  virtual void handle_command(int ctrl);

  AlterDialog *alter_dlg;
  bool mouse_down;
  vector3d mouse_pos;

  bool trans_z;
  bool speed;

  bool enable_mouse;
  bool mouse_relative;
  rational_t mouse_speed;

  virtual void handle_user_mouse_down(int button, vector3d pos, DWORD flags);
  virtual void handle_user_mouse_up(int button, vector3d pos, DWORD flags);
};

class ScannerDialog : public Dialog
{
public:
	ScannerDialog(HWND pParent);
	~ScannerDialog();

	virtual void setup();
	virtual void hide();
	virtual void update();
  virtual void render();


  virtual void frame_advance(time_value_t t);

	virtual void handle_slider(int ctrl, int pos);
  virtual void handle_command(int ctrl);

  AlterDialog *alter_dlg;

  virtual void handle_user_mouse_down(int button, vector3d pos, DWORD flags) {};
  virtual void handle_user_mouse_up(int button, vector3d pos, DWORD flags) {};
};


class MaterialDialog : public Dialog
{
public:
	MaterialDialog(HWND pParent);
	~MaterialDialog();

	virtual void setup();
	virtual void hide();
	virtual void update();
  virtual void render();


  virtual void populate_materials();

  virtual void frame_advance(time_value_t t);

	virtual void handle_slider(int ctrl, int pos);
	virtual void handle_selection_change(int ctrl);
  virtual void handle_command(int ctrl);

  static void export_material_groups(ofstream &fout);

  AlterDialog *alter_dlg;

  virtual void handle_user_mouse_down(int button, vector3d pos, DWORD flags) {};
  virtual void handle_user_mouse_up(int button, vector3d pos, DWORD flags) {};
};

class particle_generator;
class ParticleDialog : public Dialog
{
public:
	ParticleDialog(HWND pParent);
	~ParticleDialog();

	virtual void setup();
	virtual void hide();
	virtual void update();
  virtual void render();

  particle_generator *part;
  int num_part_sys;

  void select_sys(particle_generator *p);

  virtual void frame_advance(time_value_t t);

	virtual void handle_slider(int ctrl, int pos);
  virtual void handle_command(int ctrl);
	virtual void handle_selection_change(int ctrl);

  AlterDialog *alter_dlg;

  virtual void handle_user_mouse_down(int button, vector3d pos, DWORD flags) {};
  virtual void handle_user_mouse_up(int button, vector3d pos, DWORD flags) {};
};



/*!
class CharacterDialog : public Dialog
{
public:
	CharacterDialog(HWND pParent);
	~CharacterDialog();

	virtual void setup();
	virtual void hide();
	virtual void update();
  virtual void render();

  virtual void populate_character_ai();

  virtual void frame_advance(time_value_t t);

	virtual void handle_selection_change(int ctrl);
  virtual void handle_command(int ctrl);

  static void export_brain_info(ofstream &fout);

  AlterDialog *alter_dlg;

  virtual void handle_user_mouse_down(int button, vector3d pos, DWORD flags) {};
  virtual void handle_user_mouse_up(int button, vector3d pos, DWORD flags) {};
};

!*/
class AlterDialog : public Dialog
{
public:
	AlterDialog(HWND pParent);
	~AlterDialog();
	
	virtual void setup();
	virtual void hide();
	virtual void update();
  virtual void render();


  virtual void frame_advance(time_value_t t);

  virtual void handle_command(int ctrl);

  virtual void dump_dialogs();

  virtual void populate_entities();
	virtual void handle_selection_change(int ctrl);

  RotateDialog *rot_dlg;
  TranslateDialog *trans_dlg;
  ScannerDialog *scanner_dlg;
  ParticleDialog *particle_dlg;
  ItemDialog *item_dlg;
  EntityDialog *entity_dlg;
  DestroyDialog *destroy_dlg;
  MaterialDialog *material_dlg;
  MarkerDialog *marker_dlg;
//!  CharacterDialog *character_dlg;

  entity *curr_ent;

  virtual void handle_user_mouse_down(int button, vector3d pos, DWORD flags);
  virtual void handle_user_mouse_up(int button, vector3d pos, DWORD flags);
};


class GroupDialog : public Dialog
{
public:
	GroupDialog(HWND pParent);
	~GroupDialog();
	
	virtual void setup();
	virtual void hide();
	virtual void update();
  virtual void render();


  virtual void populate_groups();
  virtual void populate_members();
  virtual void populate_characters();

  virtual void remove_group();
  virtual void remove_members();
//!  virtual void add_characters();

	virtual void handle_selection_change(int ctrl);
  virtual void frame_advance(time_value_t t);

  virtual void handle_command(int ctrl);

  virtual void handle_user_mouse_down(int button, vector3d pos, DWORD flags) {};
  virtual void handle_user_mouse_up(int button, vector3d pos, DWORD flags) {};
};

class ContainerDialog : public Dialog
{
public:
	ContainerDialog(HWND pParent);
	~ContainerDialog();
	
	virtual void setup();
	virtual void hide();
	virtual void update();
  virtual void render();


  virtual void populate_items();
  virtual void populate_item_list();
  virtual void populate_containers();

  virtual void remove_item();
  virtual void add_item();
  virtual void update_item();
  virtual bool item_exists(stringx fname);
  virtual int get_item_index(entity *ent, int list_index);

  virtual void handle_slider(int ctrl, int pos);
	virtual void handle_selection_change(int ctrl);
  virtual void frame_advance(time_value_t t);

  virtual entity *get_curr_ent();

  virtual void handle_command(int ctrl);

  virtual void handle_user_mouse_down(int button, vector3d pos, DWORD flags) {};
  virtual void handle_user_mouse_up(int button, vector3d pos, DWORD flags) {};
};

class ExportDialog : public Dialog
{
public:
  char path[_MAX_PATH+1];
  char file[_MAX_PATH+1];

  bool exp_level;
  bool exp_sin_ents;
  bool exp_spawn_ents;
  bool exp_scanners;
  bool exp_items;
  bool exp_char_groups;
  bool exp_containers;
  bool exp_material_groups;
  bool exp_paths;
  bool exp_brains;

	ExportDialog(HWND pParent);
	~ExportDialog();
	
	virtual void setup();
	virtual void hide();
	virtual void update();
  virtual void render();


  virtual bool export();

  virtual void frame_advance(time_value_t t);

  virtual void handle_command(int ctrl);

  virtual void handle_user_mouse_down(int button, vector3d pos, DWORD flags) {};
  virtual void handle_user_mouse_up(int button, vector3d pos, DWORD flags) {};
};

class ConsoleDialog : public Dialog
{
public:
	ConsoleDialog(HWND pParent);
	~ConsoleDialog();
	
	virtual void setup();
	virtual void hide();
	virtual void update();
  virtual void render();


  virtual bool validate();
	virtual void submit();

  virtual void refresh_log();

  virtual void frame_advance(time_value_t t);

  virtual void handle_command(int ctrl);

  virtual void handle_user_mouse_down(int button, vector3d pos, DWORD flags) {};
  virtual void handle_user_mouse_up(int button, vector3d pos, DWORD flags) {};
};

extern char g_render_paths;

class PathsDialog : public Dialog
{
public:
  enum
  {
    _MODE_NODE = 0,
    _MODE_EDGE,
    _MODE_TEST,
    _MODE_MAX
  };

  vector3d spawn_point;
  bool auto_edge;
  int last_mode;
  int point_count;
  path_graph *cur_graph;
  path_graph_node *selected_nodes[2];
  vector3d test_pos[2];
  path test_path;

	PathsDialog(HWND pParent);
	~PathsDialog();

	virtual void setup();
	virtual void hide();
	virtual void update();
  virtual void render();

  virtual void update_edge_info();
  virtual void update_node_info();

  virtual void frame_advance(time_value_t t);

  virtual void handle_slider(int ctrl, int pos);

  virtual void handle_command(int ctrl);

  virtual void populate_paths();

  virtual void handle_selection_change(int ctrl);

  virtual void handle_user_mouse_down(int button, vector3d pos, DWORD flags);
  virtual void handle_user_mouse_up(int button, vector3d pos, DWORD flags);
};


class PathsDialog2 : public Dialog
{
public:
  enum
  {
    _MODE_NODE = 0,
    _MODE_EDGE,
    _MODE_TEST,
    _MODE_MAX
  };

  class triangle
  {
  public:
    int index[3];

    triangle()
    {
    }

    triangle(const triangle &tri)
    {
      index[0] = tri.index[0];
      index[1] = tri.index[1];
      index[2] = tri.index[2];
    }

    triangle(int i0, int i1, int i2)
    {
      index[0] = i0;
      index[1] = i1;
      index[2] = i2;
    }

    triangle &operator=(const triangle &tri)
    {
      index[0] = tri.index[0];
      index[1] = tri.index[1];
      index[2] = tri.index[2];

      return(*this);
    }

    bool operator==(const triangle &tri)
    {
      bool equal = true;
      for(int i=0; i<3 && equal; ++i)
      {
        equal = false;
        for(int j=0; j<3 && !equal; ++j)
          equal = (index[i] == tri.index[j]);
      }

      return(equal);
    }

    bool operator!=(const triangle &tri)
    {
      return(!((*this) == tri));
    }
  };

  vector<vector3d> pts;
  vector<triangle> tris;

  stringx last_file;

  triangle cur;

  vector3d spawn_point;

	PathsDialog2(HWND pParent);
	~PathsDialog2();

	virtual void setup();
	virtual void hide();
	virtual void update();
  virtual void render();

  virtual void frame_advance(time_value_t t);

  virtual void handle_command(int ctrl);
  virtual void handle_selection_change(int ctrl);

  void export();
  void load();
  void remove_pt(int index);
  void toggle_triangle();

  virtual void handle_user_mouse_down(int button, vector3d pos, DWORD flags);
  virtual void handle_user_mouse_up(int button, vector3d pos, DWORD flags);
};


class ToolsDialog : public Dialog
{
protected:
  bool first_time;
  IniFile file;

public:
	ToolsDialog(HWND pParent);
	~ToolsDialog();
	
	virtual void setup();
	virtual void hide();
	virtual void show();
	virtual void update();
  virtual void render();


  virtual void frame_advance(time_value_t t);

  virtual void handle_command(int ctrl);

  virtual void dump_dialogs();

  SpawnDialog *spawn_dlg;
  AlterDialog *alter_dlg;
  GroupDialog *group_dlg;
  PathsDialog *paths_dlg;
  PathsDialog2 *paths_dlg2;
  ContainerDialog *container_dlg;
  ExportDialog *export_dlg;
  ConsoleDialog *console_dlg;

  virtual void handle_user_mouse_down(int button, vector3d pos, DWORD flags);
  virtual void handle_user_mouse_up(int button, vector3d pos, DWORD flags);
};

extern ToolsDialog *g_tools_dlg;

#endif

#endif