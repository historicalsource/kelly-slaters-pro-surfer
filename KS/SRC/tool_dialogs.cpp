#include "global.h"

// tool_dialogs.cpp
// Copyright (c) 2000 Treyarch Invention LLC.  ALL RIGHTS RESERVED.
#include "tool_dialogs.h"

#if _ENABLE_WORLD_EDITOR

#include "console.h"
#include "consolecmds.h"
#include "consolevars.h"
// BIGCULL#include "spidey_consolecmds.h"
// BIGCULL#include "spidey_consolevars.h"
#include "wds.h"
#include "po.h"
//!#include "character.h"
#include "game.h"
#include "app.h"
#include "collide.h"
//!#include "char_group.h"
// BIGCULL #include "scanner.h"
#include "item.h"
#include "entityflags.h"
#include "colgeom.h"
#include "colmesh.h"
//#include "brain.h"
#include "geomgr.h"
//#include "winapp.h"
#include "marker.h"
#include "path.h"
#include "particle.h"
#include "debug_render.h"
#include "entity_maker.h"
#include "file_finder.h"
#include "terrain.h"

//#define _KEY_DOWN(a)   (GetAsyncKeyState((a)) & 0x80)
#define _KEY_DOWN(a)   (GetAsyncKeyState((a)))

// MK_CONTROL, MK_SHIFT flags for mouse presses
// 0,1,2 = L,M,R buttons

#define g_hwnd (windows_app::inst()->get_hwnd())

ToolsDialog *g_tools_dlg = NULL;
entity *g_target_fx = NULL;

BOOL CALLBACK gSpawnDialogFunc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
  return(g_tools_dlg->spawn_dlg->handle_message(hDlg, msg, wParam, lParam));
}

BOOL CALLBACK gAlterDialogFunc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
  return(g_tools_dlg->alter_dlg->handle_message(hDlg, msg, wParam, lParam));
}

BOOL CALLBACK gAlterTransDialogFunc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
  return(g_tools_dlg->alter_dlg->trans_dlg->handle_message(hDlg, msg, wParam, lParam));
}

BOOL CALLBACK gAlterRotDialogFunc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
  return(g_tools_dlg->alter_dlg->rot_dlg->handle_message(hDlg, msg, wParam, lParam));
}

BOOL CALLBACK gAlterScannerDialogFunc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
  return(g_tools_dlg->alter_dlg->scanner_dlg->handle_message(hDlg, msg, wParam, lParam));
}

BOOL CALLBACK gAlterMaterialDialogFunc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
  return(g_tools_dlg->alter_dlg->material_dlg->handle_message(hDlg, msg, wParam, lParam));
}

BOOL CALLBACK gAlterParticleDialogFunc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
  return(g_tools_dlg->alter_dlg->particle_dlg->handle_message(hDlg, msg, wParam, lParam));
}

BOOL CALLBACK gAlterItemDialogFunc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
  return(g_tools_dlg->alter_dlg->item_dlg->handle_message(hDlg, msg, wParam, lParam));
}

BOOL CALLBACK gAlterMarkerDialogFunc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
  return(g_tools_dlg->alter_dlg->marker_dlg->handle_message(hDlg, msg, wParam, lParam));
}

BOOL CALLBACK gAlterEntityDialogFunc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
  return(g_tools_dlg->alter_dlg->entity_dlg->handle_message(hDlg, msg, wParam, lParam));
}

BOOL CALLBACK gAlterDestroyDialogFunc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
  return(g_tools_dlg->alter_dlg->destroy_dlg->handle_message(hDlg, msg, wParam, lParam));
}
/*!
BOOL CALLBACK gAlterCharacterDialogFunc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
  return(g_tools_dlg->alter_dlg->character_dlg->handle_message(hDlg, msg, wParam, lParam));
}
!*/
BOOL CALLBACK gGroupDialogFunc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
  return(g_tools_dlg->group_dlg->handle_message(hDlg, msg, wParam, lParam));
}

BOOL CALLBACK gContainerDialogFunc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
  return(g_tools_dlg->container_dlg->handle_message(hDlg, msg, wParam, lParam));
}

BOOL CALLBACK gExportDialogFunc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
  return(g_tools_dlg->export_dlg->handle_message(hDlg, msg, wParam, lParam));
}

BOOL CALLBACK gConsoleDialogFunc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
  return(g_tools_dlg->console_dlg->handle_message(hDlg, msg, wParam, lParam));
}

BOOL CALLBACK gPathsDialogFunc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
  return(g_tools_dlg->paths_dlg->handle_message(hDlg, msg, wParam, lParam));
}

BOOL CALLBACK gPathsDialog2Func(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
  return(g_tools_dlg->paths_dlg2->handle_message(hDlg, msg, wParam, lParam));
}

BOOL CALLBACK gToolsDialogFunc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
  return(g_tools_dlg->handle_message(hDlg, msg, wParam, lParam));
}

extern COLORREF gCustomDialogColors[16];

rational_t g_x_mod = 1.0f;
rational_t g_y_mod = 1.0f;

matrix4x4 dlg_w2v;
matrix4x4 dlg_v2vp;

POINT DialogWorldToScreen(vector3d vec)
{
  POINT pt;
  pt.x = 0;
  pt.y = 0;

  {
    vec = xform3d_1(dlg_w2v, vec);

    if(vec.z > 0.0f)
    {
      vec = xform3d_1_homog(dlg_v2vp, vec);

      pt.x = vec.x;
      pt.y = vec.y;
    }
  }

  return pt;
}

#define _MOUSE_SEGMENT_TOLERANCE          1
#define _MOUSE_SEGMENT_CORRECTION         20
#define _MOUSE_SEGMENT_CORRECTION_2_PASS  0

void get_screen_to_world_segment(vector2di screen_pos, vector3d &origin, vector3d &end, vector3d &normal, rational_t surface_dist)
{
#if 1
  rational_t uni_x = (rational_t)((rational_t)screen_pos.x - ((rational_t)get_client_width (g_hwnd)*0.5f)) / (rational_t)((rational_t)get_client_width(g_hwnd)/2.0f);
  rational_t uni_y = (rational_t)((rational_t)screen_pos.y - ((rational_t)get_client_height(g_hwnd)*0.5f)) / (rational_t)((rational_t)get_client_height(g_hwnd)/2.0f);

  if(uni_x < -1.0f)
    uni_x = -1.0f;
  if(uni_x > 1.0f)
    uni_x = 1.0f;

  if(uni_y < -1.0f)
    uni_y = -1.0f;
  if(uni_y > 1.0f)
    uni_y = 1.0f;

  screen_pos.x = (((uni_x + 1.0f) * 0.5f) * nglGetScreenWidth());
  screen_pos.y = (((uni_y + 1.0f) * 0.5f) * nglGetScreenHeight());

  rational_t x_ang = uni_y * (PROJ_FIELD_OF_VIEW*0.5f) * g_x_mod;
  rational_t y_ang = uni_x * (PROJ_FIELD_OF_VIEW*0.5f) * g_y_mod;

  po cam_po = app::inst()->get_game()->get_current_view_camera()->get_abs_po();

  vector3d face = cam_po.get_facing();

  po test = po_identity_matrix;
  test.set_rot(cam_po.get_x_facing(), -x_ang);
  face = test.non_affine_slow_xform(face);

  test = po_identity_matrix;
  test.set_rot(cam_po.get_y_facing(), -y_ang);
  face = test.non_affine_slow_xform(face);

  vector3d delta = face*1000;
  origin = cam_po.get_position();

  origin += cam_po.get_y_facing()*uni_y*0.2f;
  origin += cam_po.get_x_facing()*uni_x*0.2f;

  entity *hit_entity = NULL;
  if ( find_intersection( origin, origin+delta,
                          app::inst()->get_game()->get_current_view_camera()->get_region(),
                          FI_COLLIDE_WORLD|FI_COLLIDE_ENTITY,
                          &end, &normal,
                          NULL, &hit_entity ) )
  {
    normal.normalize();
    end = (end + (normal * surface_dist));
  }

  POINT scr = DialogWorldToScreen(end);

#if _MOUSE_SEGMENT_CORRECTION_2_PASS

  int num = _MOUSE_SEGMENT_CORRECTION;
  float dir = 1.0f;
  int dist;

  while((dist = abs(scr.x - screen_pos.x)) > _MOUSE_SEGMENT_TOLERANCE && num > 0)
  {
    y_ang += 0.005f * dir;

    face = cam_po.get_facing();

    test = po_identity_matrix;
    test.set_rot(cam_po.get_x_facing(), -x_ang);
    face = test.non_affine_slow_xform(face);

    test = po_identity_matrix;
    test.set_rot(cam_po.get_y_facing(), -y_ang);
    face = test.non_affine_slow_xform(face);

    delta = face*1000.0f;

    origin = cam_po.get_abs_position();

    origin += cam_po.get_y_facing()*uni_y*0.2f;
    origin += cam_po.get_x_facing()*uni_x*0.2f;

    if ( find_intersection( origin, origin+delta,
                            FI_COLLIDE_WORLD|FI_COLLIDE_ENTITY,
                            app::inst()->get_game()->get_current_view_camera()->get_region(),
                            &end, &normal,
                            NULL, &hit_entity ) )
    {
      normal.normalize();
      end = (end + (normal * surface_dist));
    }

    scr = DialogWorldToScreen(end);

    if(abs(scr.x - screen_pos.x) > dist)
      dir *= -1.0f;

    --num;
  }

  num = _MOUSE_SEGMENT_CORRECTION;
  dir = 1.0f;
  while((dist = abs(scr.y - screen_pos.y)) > _MOUSE_SEGMENT_TOLERANCE && num > 0)
  {
    x_ang += 0.005f * dir;

    face = cam_po.get_facing();

    test = po_identity_matrix;
    test.set_rot(cam_po.get_x_facing(), -x_ang);
    face = test.non_affine_slow_xform(face);

    test = po_identity_matrix;
    test.set_rot(cam_po.get_y_facing(), -y_ang);
    face = test.non_affine_slow_xform(face);

    delta = face*1000.0f;

    origin = cam_po.get_abs_position();

    origin += cam_po.get_y_facing()*uni_y*0.2f;
    origin += cam_po.get_x_facing()*uni_x*0.2f;

    if ( find_intersection( origin, origin+delta,
                            FI_COLLIDE_WORLD|FI_COLLIDE_ENTITY,
                            app::inst()->get_game()->get_current_view_camera()->get_region(),
                            &end, &normal,
                            NULL, &hit_entity ) )
    {
      normal.normalize();
      end = (end + (normal * surface_dist));
    }

    scr = DialogWorldToScreen(end);

    if(abs(scr.y - screen_pos.y) > dist)
      dir *= -1.0f;

    --num;
  }

#else

  int num = _MOUSE_SEGMENT_CORRECTION;

  float dirX = 1.0f;
  int distX = nglGetScreenWidth();

  float dirY = 1.0f;
  int distY = nglGetScreenHeight();

  distX = abs(scr.x - screen_pos.x);
  distY = abs(scr.y - screen_pos.y);
  while((distX > _MOUSE_SEGMENT_TOLERANCE || distY > _MOUSE_SEGMENT_TOLERANCE) && num > 0)
  {
    if(distX > _MOUSE_SEGMENT_TOLERANCE)
      y_ang += 0.005f * dirX;

    if(distY > _MOUSE_SEGMENT_TOLERANCE)
      x_ang += 0.005f * dirY;

    face = cam_po.get_facing();

    test = po_identity_matrix;
    test.set_rot(cam_po.get_x_facing(), -x_ang);
    face = test.non_affine_slow_xform(face);

    test = po_identity_matrix;
    test.set_rot(cam_po.get_y_facing(), -y_ang);
    face = test.non_affine_slow_xform(face);

    delta = face*1000.0f;

    origin = cam_po.get_position();

    origin += cam_po.get_y_facing()*uni_y*0.2f;
    origin += cam_po.get_x_facing()*uni_x*0.2f;

    if ( find_intersection( origin, origin+delta,
                            app::inst()->get_game()->get_current_view_camera()->get_region(),
                            FI_COLLIDE_WORLD|FI_COLLIDE_ENTITY,
                            &end, &normal,
                            NULL, &hit_entity ) )
    {
      normal.normalize();
      end = (end + (normal * surface_dist));
    }

    scr = DialogWorldToScreen(end);

    if(abs(scr.x - screen_pos.x) > distX)
      dirX *= -1.0f;

    if(abs(scr.y - screen_pos.y) > distY)
      dirY *= -1.0f;

    distX = abs(scr.x - screen_pos.x);
    distY = abs(scr.y - screen_pos.y);

    --num;
  }
#endif

#else

  rational_t det;

  matrix4x4 v2vp_inv = dlg_v2vp.inverse();
  matrix4x4 w2v_inv = dlg_w2v.inverse();

  matrix4x4 test;
  test = dlg_v2vp * v2vp_inv;

  matrix4x4 test2;
  test2 = dlg_w2v * w2v_inv;

  int x = 1;

  vector3d pos;
  vector3d pos2;
  pos.x = get_client_mouse_position(g_hwnd).x;
  pos.y = get_client_mouse_position(g_hwnd).y;
  pos.z = PROJ_NEAR_PLANE_D;

  pos = xform3d_1_homog(v2vp_inv,pos);
  pos = xform3d_1_homog(w2v_inv,pos);

  pos2.x = get_client_mouse_position(g_hwnd).x;
  pos2.y = get_client_mouse_position(g_hwnd).y;
  pos2.z = PROJ_FAR_PLANE_D;

  pos2 = xform3d_1_homog(v2vp_inv,pos2);
  pos2 = xform3d_1_homog(w2v_inv,pos2);


  po ent_po = app::inst()->get_game()->get_current_view_camera()->get_abs_po();

  entity *hit_entity = NULL;
  vector3d hitp, hitn;
  if ( find_intersection( pos, pos2,
                          FI_COLLIDE_WORLD|FI_COLLIDE_ENTITY,
                          app::inst()->get_game()->get_current_view_camera()->get_region(),
                          &hitp, &hitn,
                          NULL, &hit_entity ) )
  {
    hitn.normalize();

    ent_po = po_identity_matrix;
    ent_po.set_facing(hitn);

    ent_po.set_rel_position(hitp+(hitn*surface_dist));
  }

  target->set_rel_po(ent_po);
  target->compute_sector(g_world_ptr->get_the_terrain());

#endif
}






















ToolsDialog::ToolsDialog(HWND pParent)
{
  file.setFile("max_steel_editor.ini");

  char header[64] = "";
  for(int i=0; i<16; i++)
  {
    sprintf(header, "dialog_color_%d", i);
    gCustomDialogColors[i] = file.getInt("Colors", header, 0);
  }

	init(IDD_TOOLS, gToolsDialogFunc, pParent);

  spawn_dlg = NEW SpawnDialog(NULL);
  alter_dlg = NEW AlterDialog(NULL);
//  group_dlg = NEW GroupDialog(NULL);
  paths_dlg = NEW PathsDialog(NULL);
  paths_dlg2 = NEW PathsDialog2(NULL);
  container_dlg = NEW ContainerDialog(NULL);
  export_dlg = NEW ExportDialog(NULL);
  console_dlg = NEW ConsoleDialog(NULL);


  file.getString("Export", "path", export_dlg->path, _MAX_PATH, "");
  file.getString("Export", "file", export_dlg->file, _MAX_PATH, "");
  export_dlg->exp_level = file.getBool("Export", "level", export_dlg->exp_level);
  export_dlg->exp_sin_ents = file.getBool("Export", "sin_ents", export_dlg->exp_sin_ents);
  export_dlg->exp_spawn_ents = file.getBool("Export", "spawn_ents", export_dlg->exp_spawn_ents);
  export_dlg->exp_scanners = file.getBool("Export", "scanners", export_dlg->exp_scanners);
  export_dlg->exp_items = file.getBool("Export", "items", export_dlg->exp_items);
  export_dlg->exp_char_groups = file.getBool("Export", "char_groups", export_dlg->exp_char_groups);
  export_dlg->exp_containers = file.getBool("Export", "containers", export_dlg->exp_containers);
  export_dlg->exp_material_groups = file.getBool("Export", "material_groups", export_dlg->exp_material_groups);
  export_dlg->exp_paths = file.getBool("Export", "paths", export_dlg->exp_paths);
  export_dlg->exp_brains = file.getBool("Export", "brains", export_dlg->exp_brains);

  spawn_dlg->last_radio = file.getInt("Spawn", "radio", spawn_dlg->last_radio);
  spawn_dlg->gen_name = file.getBool("Spawn", "gen_name", spawn_dlg->gen_name);
  spawn_dlg->gen_suffix = file.getBool("Spawn", "gen_suffix", spawn_dlg->gen_suffix);
  spawn_dlg->orient = file.getBool("Spawn", "orient", spawn_dlg->orient);
  spawn_dlg->surface_dist = file.getFloat("Spawn", "surface_dist", spawn_dlg->surface_dist);


  alter_dlg->trans_dlg->inc = file.getFloat("Alter_Translation", "inc", alter_dlg->trans_dlg->inc);
  alter_dlg->trans_dlg->mouse_speed = file.getFloat("Alter_Translation", "mouse_speed", alter_dlg->trans_dlg->mouse_speed);
  alter_dlg->trans_dlg->continuous = file.getBool("Alter_Translation", "continuous", alter_dlg->trans_dlg->continuous);
  alter_dlg->trans_dlg->relative = file.getBool("Alter_Translation", "relative", alter_dlg->trans_dlg->relative);
  alter_dlg->trans_dlg->enable_mouse = file.getBool("Alter_Translation", "enable_mouse", alter_dlg->trans_dlg->enable_mouse);
  alter_dlg->trans_dlg->mouse_relative = file.getBool("Alter_Translation", "mouse_relative", alter_dlg->trans_dlg->mouse_relative);

  alter_dlg->rot_dlg->inc = file.getFloat("Alter_Rotation", "inc", alter_dlg->rot_dlg->inc);
  alter_dlg->rot_dlg->mouse_speed = file.getFloat("Alter_Rotation", "mouse_speed", alter_dlg->rot_dlg->mouse_speed);
  alter_dlg->rot_dlg->continuous = file.getBool("Alter_Rotation", "continuous", alter_dlg->rot_dlg->continuous);
  alter_dlg->rot_dlg->relative = file.getBool("Alter_Rotation", "relative", alter_dlg->rot_dlg->relative);
  alter_dlg->rot_dlg->enable_mouse = file.getBool("Alter_Rotation", "enable_mouse", alter_dlg->rot_dlg->enable_mouse);
  alter_dlg->rot_dlg->mouse_relative = file.getBool("Alter_Rotation", "mouse_relative", alter_dlg->rot_dlg->mouse_relative);

  first_time = true;

  g_target_fx = g_world_ptr->get_entity("TOOL_MARKER_FX");
  if(g_target_fx == NULL)// && (target = g_world_ptr->get_entity("SPAWN_DLG_TARGET")) == NULL)
  {
    warning("TOOL_MARKER_FX entity was not loaded!!!");

    bool filelock = os_file::is_system_locked();
    os_file::system_unlock();

    po ent_po = po_identity_matrix;
    g_target_fx = g_entity_maker->create_entity_or_subclass("laserpistolmuzzlefx", entity_id("TOOL_MARKER_FX"), ent_po, empty_string);

    if ( filelock )
      os_file::system_lock();
  }
}

ToolsDialog::~ToolsDialog()
{
  char header[64] = "";
  for(int i=0; i<16; i++)
  {
    sprintf(header, "dialog_color_%d", i);
    file.writeInt("Colors", header, gCustomDialogColors[i]);
  }

  file.writeString("Export", "path", "%s", export_dlg->path);
  file.writeString("Export", "file", "%s", export_dlg->file);
  file.writeBool("Export", "level", export_dlg->exp_level);
  file.writeBool("Export", "sin_ents", export_dlg->exp_sin_ents);
  file.writeBool("Export", "spawn_ents", export_dlg->exp_spawn_ents);
  file.writeBool("Export", "scanners", export_dlg->exp_scanners);
  file.writeBool("Export", "items", export_dlg->exp_items);
  file.writeBool("Export", "char_groups", export_dlg->exp_char_groups);
  file.writeBool("Export", "containers", export_dlg->exp_containers);
  file.writeBool("Export", "material_groups", export_dlg->exp_material_groups);
  file.writeBool("Export", "paths", export_dlg->exp_paths);
  file.writeBool("Export", "brains", export_dlg->exp_brains);

  file.writeInt("Spawn", "radio", spawn_dlg->last_radio);
  file.writeBool("Spawn", "gen_name", spawn_dlg->gen_name);
  file.writeBool("Spawn", "gen_suffix", spawn_dlg->gen_suffix);
  file.writeBool("Spawn", "orient", spawn_dlg->orient);
  file.writeFloat("Spawn", "surface_dist", spawn_dlg->surface_dist);

  file.writeFloat("Alter_Translation", "inc", alter_dlg->trans_dlg->inc);
  file.writeFloat("Alter_Translation", "mouse_speed", alter_dlg->trans_dlg->mouse_speed);
  file.writeBool("Alter_Translation", "continuous", alter_dlg->trans_dlg->continuous);
  file.writeBool("Alter_Translation", "relative", alter_dlg->trans_dlg->relative);
  file.writeBool("Alter_Translation", "enable_mouse", alter_dlg->trans_dlg->enable_mouse);
  file.writeBool("Alter_Translation", "mouse_relative", alter_dlg->trans_dlg->mouse_relative);

  file.writeFloat("Alter_Rotation", "inc", alter_dlg->rot_dlg->inc);
  file.writeFloat("Alter_Rotation", "mouse_speed", alter_dlg->rot_dlg->mouse_speed);
  file.writeBool("Alter_Rotation", "continuous", alter_dlg->rot_dlg->continuous);
  file.writeBool("Alter_Rotation", "relative", alter_dlg->rot_dlg->relative);
  file.writeBool("Alter_Rotation", "enable_mouse", alter_dlg->rot_dlg->enable_mouse);
  file.writeBool("Alter_Rotation", "mouse_relative", alter_dlg->rot_dlg->mouse_relative);


  dump_dialogs();
}

void ToolsDialog::dump_dialogs()
{
  _DELETE_NULLIFY(spawn_dlg);
  _DELETE_NULLIFY(alter_dlg);
//  _DELETE_NULLIFY(group_dlg);
  _DELETE_NULLIFY(paths_dlg);
  _DELETE_NULLIFY(paths_dlg2);
  _DELETE_NULLIFY(container_dlg);
  _DELETE_NULLIFY(export_dlg);
  _DELETE_NULLIFY(console_dlg);
}

void ToolsDialog::setup()
{
  init_tab_control(IDC_TOOLTABS, 7);

  insert_tab_control_item(IDC_TOOLTABS, 0, spawn_dlg, _TABCONTROL_SIZE_DIALOG | _TABCONTROL_CENTER_DIALOG, "Spawn");
  insert_tab_control_item(IDC_TOOLTABS, 1, alter_dlg, _TABCONTROL_SIZE_DIALOG | _TABCONTROL_CENTER_DIALOG, "Alter");
//  insert_tab_control_item(IDC_TOOLTABS, 2, group_dlg, _TABCONTROL_SIZE_DIALOG | _TABCONTROL_CENTER_DIALOG, "Group");
  insert_tab_control_item(IDC_TOOLTABS, 2, paths_dlg, _TABCONTROL_SIZE_DIALOG | _TABCONTROL_CENTER_DIALOG, "Paths");
  insert_tab_control_item(IDC_TOOLTABS, 3, paths_dlg2, _TABCONTROL_SIZE_DIALOG | _TABCONTROL_CENTER_DIALOG, "PolyPaths");
  insert_tab_control_item(IDC_TOOLTABS, 4, container_dlg, _TABCONTROL_SIZE_DIALOG | _TABCONTROL_CENTER_DIALOG, "Containers");
  insert_tab_control_item(IDC_TOOLTABS, 5, export_dlg, _TABCONTROL_SIZE_DIALOG | _TABCONTROL_CENTER_DIALOG, "Export");
  insert_tab_control_item(IDC_TOOLTABS, 6, console_dlg, _TABCONTROL_SIZE_DIALOG | _TABCONTROL_CENTER_DIALOG, "Console");

  set_tab_control_index(IDC_TOOLTABS, 0);


  if(first_time)
  {
    POINT pt = get_window_pos(parent_hwnd);
    pt.x += get_window_width(parent_hwnd);
    int w = get_width();
    int wx = pt.x + w;
    int dw = get_window_width(GetDesktopWindow());
    if(wx > dw)
      pt.x -= (wx - dw);
    set_pos(pt.x, pt.y);

    first_time = false;
  }


	update();
}

void ToolsDialog::hide()
{
	Dialog::hide();
}

void ToolsDialog::show()
{
	Dialog::show();

  SetActiveWindow(parent_hwnd);
}

void ToolsDialog::update()
{
	if(!is_visible())
		return;

  spawn_dlg->update();
  alter_dlg->update();
//  group_dlg->update();
  paths_dlg->update();
  paths_dlg2->update();
  container_dlg->update();
  export_dlg->update();
  console_dlg->update();

	Dialog::update();
}

void ToolsDialog::render()
{
	if(!is_visible())
		return;

  spawn_dlg->render();
  alter_dlg->render();
//  group_dlg->render();
  paths_dlg->render();
  paths_dlg2->render();
  container_dlg->render();
  export_dlg->render();
  console_dlg->render();
}

void ToolsDialog::frame_advance(time_value_t t)
{
	if(!is_visible())
		return;

  ShowCursor( true );
}


void ToolsDialog::handle_command(int ctrl)
{
	switch(ctrl)
	{
    default:
      Dialog::handle_command(ctrl);
      break;
  }
}

void ToolsDialog::handle_user_mouse_down(int button, vector3d pos, DWORD flags)
{
	if(!is_visible())
		return;

  spawn_dlg->handle_user_mouse_down(button, pos, flags);
  alter_dlg->handle_user_mouse_down(button, pos, flags);
//  group_dlg->handle_user_mouse_down(button, pos, flags);
  paths_dlg->handle_user_mouse_down(button, pos, flags);
  paths_dlg2->handle_user_mouse_down(button, pos, flags);
  container_dlg->handle_user_mouse_down(button, pos, flags);
  export_dlg->handle_user_mouse_down(button, pos, flags);
  console_dlg->handle_user_mouse_down(button, pos, flags);
}

void ToolsDialog::handle_user_mouse_up(int button, vector3d pos, DWORD flags)
{
	if(!is_visible())
		return;

  spawn_dlg->handle_user_mouse_up(button, pos, flags);
  alter_dlg->handle_user_mouse_up(button, pos, flags);
//  group_dlg->handle_user_mouse_up(button, pos, flags);
  paths_dlg->handle_user_mouse_up(button, pos, flags);
  paths_dlg2->handle_user_mouse_up(button, pos, flags);
  container_dlg->handle_user_mouse_up(button, pos, flags);
  export_dlg->handle_user_mouse_up(button, pos, flags);
  console_dlg->handle_user_mouse_up(button, pos, flags);
}














SpawnDialog::SpawnDialog(HWND pParent)
{
	init(IDD_SPAWN, gSpawnDialogFunc, pParent);

  last_radio = IDC_RADIO_HERO;
  surface_dist = 0.1f;

  gen_name = false;
  gen_suffix = false;
  orient = false;
}

SpawnDialog::~SpawnDialog()
{
}

void SpawnDialog::setup()
{
  if(last_radio != IDC_RADIO_HERO && last_radio != IDC_RADIO_MOUSE && last_radio != IDC_RADIO_CAMERA && last_radio != IDC_RADIO_LOOKAT)
    last_radio = IDC_RADIO_HERO;

  populate_ent_files();

  check_radio_button(IDC_RADIO_HERO, last_radio);

  if((last_radio == IDC_RADIO_LOOKAT || last_radio == IDC_RADIO_MOUSE) && g_target_fx)
  {
    g_target_fx->set_visible(true);
    enable_ctrl(IDC_DIST);
  }
  else
  {
    if(g_target_fx)
      g_target_fx->set_visible(false);

    disable_ctrl(IDC_DIST);
  }

  set_ctrl_float(IDC_DIST, surface_dist);

  set_check_box(IDC_GEN_NAME, gen_name);
  set_check_box(IDC_GEN_SUFFIX, gen_suffix);
  set_check_box(IDC_ORIENT, orient);

  enable_ctrl(IDC_GEN_SUFFIX, !get_check_box(IDC_GEN_NAME));
  enable_ctrl(IDC_ENTITY_ID, !get_check_box(IDC_GEN_NAME));

  generate_name();

	update();
}

void SpawnDialog::hide()
{
  if(g_target_fx)
    g_target_fx->set_visible(false);

  gen_name = get_check_box(IDC_GEN_NAME);
  gen_suffix = get_check_box(IDC_GEN_SUFFIX);
  orient = get_check_box(IDC_ORIENT);

	Dialog::hide();
}

void SpawnDialog::update()
{
	if(!is_visible())
		return;

	Dialog::update();
}

void SpawnDialog::render()
{
	if(!is_visible())
		return;
}

extern vector<entity *> spawned_entities;
extern bool g_spawn_warn;

void SpawnDialog::generate_name()
{
  if(get_check_box(IDC_GEN_NAME))
  {
    char ent_file_c[128] = "";
    char name[128] = "";

    get_ctrl_text(IDC_ENTITY_FILE, ent_file_c, 127);

    int i = 0;

    sprintf(name, "%s_%03d", ent_file_c, i);
    str_toupper(name);
    while(g_world_ptr->get_entity(name))
    {
      i++;
      sprintf(name, "%s_%03d", ent_file_c, i);
      str_toupper(name);
    }

    set_ctrl_text(IDC_ENTITY_ID, name);
  }
  else if(!get_check_box(IDC_GEN_SUFFIX))
    set_ctrl_text(IDC_ENTITY_ID, "");
}

void SpawnDialog::spawn_entity()
{
  char ent_file_c[128] = "";
  char ent_id_c[128] = "";

  po ent_po = po_identity_matrix;

  switch(get_radio_button(IDC_RADIO_HERO))
  {
    case IDC_RADIO_HERO:
    {
      ent_po = g_world_ptr->get_hero_ptr()->get_abs_po();
    }
    break;

    case IDC_RADIO_CAMERA:
    {
      ent_po = app::inst()->get_game()->get_current_view_camera()->get_abs_po();
    }
    break;

    case IDC_RADIO_LOOKAT:
    case IDC_RADIO_MOUSE:
    {
      if(g_target_fx)
      {
        ent_po = g_target_fx->get_abs_po();
        ent_po.set_position(spawn_point);
//        ent_po.set_rel_position(ent_po.get_abs_position()+(ent_po.get_facing()*surface_dist));
      }
    }
    break;
  }

  vector3d hit_loc;
  if(!in_world(ent_po.get_position(), 0.001f, ZEROVEC, app::inst()->get_game()->get_current_view_camera()->get_region(), hit_loc))
  {
    alert_dialog("Spawn point is not within the world");
    return;
  }

  get_ctrl_text(IDC_ENTITY_FILE, ent_file_c, 127);
  get_ctrl_text(IDC_ENTITY_ID, ent_id_c, 127);

  if(strlen(ent_file_c) <= 0)
  {
    alert_dialog("Please enter or select an entity file");
    return;
  }

  if(strlen(ent_id_c) <= 0)
  {
    alert_dialog("Please enter an entity ID");
    return;
  }

  if(get_check_box(IDC_GEN_SUFFIX))
  {
    char name[128] = "";

    int i = 0;

    sprintf(name, "%s_%03d", ent_id_c, i);
    str_toupper(name);
    while(g_world_ptr->get_entity(name))
    {
      i++;
      sprintf(name, "%s_%03d", ent_id_c, i);
      str_toupper(name);
    }

    strcpy(ent_id_c, name);
  }

  str_tolower(ent_file_c);
  stringx ent_name(ent_file_c);

  str_toupper(ent_id_c);

  if(!g_world_ptr->get_entity(ent_id_c))
  {
    entity_id ent_id(ent_id_c);

    SET_CURSOR(IDC_WAIT);

    bool filelock = os_file::is_system_locked();
    os_file::system_unlock();

    entity *ent = NULL;
    if(os_developer_options::inst()->is_flagged(os_developer_options::FLAG_STASH_ONLY))
      ent = g_entity_maker->create_entity_or_subclass(ent_name, ent_id, ent_po, empty_string);
    else if (!(g_file_finder->find_file( ent_name, ".ent", true ) == empty_string))
      ent = g_entity_maker->create_entity_or_subclass(ent_name, ent_id, ent_po, empty_string);

    SET_CURSOR(IDC_ARROW);

    if(ent)
    {
/*!      if(ent->get_flavor() == ENTITY_CHARACTER)
      {
        SpawnCommand::create_variant(ent_name);
        (static_cast<character*>(ent))->switch_variant( "default" );

        if(((character *)ent)->get_brain())
        {
//          ((character *)ent)->get_brain()->push_state_safe_idle();
          ((character *)ent)->get_brain()->set_ai_state(BRAIN_REACT_IDLE, BRAIN_AI_STATE_SAFE_IDLE);
          ((character *)ent)->get_brain()->set_ai_state(BRAIN_REACT_ALERTED, BRAIN_AI_STATE_NONE);
          ((character *)ent)->get_brain()->set_ai_state(BRAIN_REACT_COMBAT, BRAIN_AI_STATE_NONE);
        }

        po newpo = po_identity_matrix;
        newpo.set_rel_position( ent->get_abs_position() );

        ent->set_rel_po( newpo );
      }
!*/
      if(ent->get_flavor() == ENTITY_ITEM || !get_check_box(IDC_ORIENT))
      {
        po newpo = po_identity_matrix;
        newpo.set_position( ent->get_abs_position() );

        ent->set_rel_po( newpo );

        ent->preload();
      }

      ent->set_spawned(true);
      ent->set_from_sin_file(true); // fake it.
      spawned_entities.push_back(ent);
      g_selected_entity = ent;
      g_spawn_warn = true;

      console_log("Spawned entity '%s'", ent_id.get_val().c_str());

      set_ctrl_text(IDC_STATIC_SPAWN, "Last entity spawned: %s", ent_id.get_val().c_str());

      generate_name();
    }
    else
    {
      console_log("Failed to spawn entity '%s'. Try a different entity file/type", ent_id.get_val().c_str());
      alert_dialog("Failed to spawn entity '%s'\n\nTry a different entity file/type", ent_id.get_val().c_str());
    }

    if ( filelock )
      os_file::system_lock();
  }
  else
  {
    console_log("Entity ID '%s' is already in use", ent_id_c);
    alert_dialog("Entity ID '%s' is already in use", ent_id_c);
  }

  populate_ent_files();

  set_ctrl_text(IDC_ENTITY_FILE, "%s", ent_file_c);

  update();
}

void SpawnDialog::frame_advance(time_value_t t)
{
	if(!is_visible())
		return;

  surface_dist = get_ctrl_float(IDC_DIST);
  if(surface_dist <= 0.0f)
    surface_dist = 0.01f;

  if(g_target_fx && g_target_fx->is_visible())
  {
    switch(get_radio_button(IDC_RADIO_HERO))
    {
      case IDC_RADIO_MOUSE:
      {
        vector3d origin, dest, norm;
        POINT pt = get_client_mouse_position(g_hwnd);
//        get_screen_to_world_segment(vector2di(pt.x, pt.y), origin, dest, norm, surface_dist);
        get_screen_to_world_segment(vector2di(pt.x, pt.y), origin, dest, norm);

        spawn_point = (dest + (norm*surface_dist));

        po  ent_po = po_identity_matrix;
        ent_po.set_facing(norm);
        ent_po.set_position(dest+(norm*0.01f));
        g_target_fx->set_rel_po(ent_po);
        g_target_fx->compute_sector(g_world_ptr->get_the_terrain());
      }
      break;

      case IDC_RADIO_LOOKAT:
      {
        po ent_po = app::inst()->get_game()->get_current_view_camera()->get_abs_po();

        vector3d face = ent_po.get_facing();
        vector3d delta = face*1000;

        entity *hit_entity = NULL;
        vector3d hitp, hitn;
        if ( find_intersection( ent_po.get_position(), ent_po.get_position()+delta,
                                app::inst()->get_game()->get_current_view_camera()->get_region(),
                                FI_COLLIDE_WORLD|FI_COLLIDE_ENTITY,
                                &hitp, &hitn,
                                NULL, &hit_entity ) )
        {
          hitn.normalize();

          ent_po = po_identity_matrix;
          ent_po.set_facing(hitn);

//          ent_po.set_rel_position(hitp+(hitn*surface_dist));
          ent_po.set_position(hitp+(hitn*0.01f));

          spawn_point = (hitp+(hitn*surface_dist));
        }


        g_target_fx->set_rel_po(ent_po);
        g_target_fx->compute_sector(g_world_ptr->get_the_terrain());
      }
      break;
    }
  }
}


void SpawnDialog::populate_ent_files()
{
  clear_combo(IDC_ENTITY_FILE);

  entfile_map::const_iterator fi = g_world_ptr->get_entfiles().begin();
  entfile_map::const_iterator fi_end = g_world_ptr->get_entfiles().end();

  while(fi != fi_end)
  {
    add_combo_string(IDC_ENTITY_FILE, "%s", (*fi).first.c_str());
    fi++;
  }

  set_combo_index(IDC_ENTITY_FILE, 0);
}

void SpawnDialog::handle_command(int ctrl)
{
	switch(ctrl)
	{
    case IDCANCEL:
      break;

		case IDC_SPAWN:
		{
      spawn_entity();
		}
		break;

    case IDC_RADIO_HERO:
    {
      if(g_target_fx)
        g_target_fx->set_visible(false);

      last_radio = IDC_RADIO_HERO;

      disable_ctrl(IDC_DIST);
    }
    break;

    case IDC_RADIO_CAMERA:
    {
      if(g_target_fx)
        g_target_fx->set_visible(false);

      last_radio = IDC_RADIO_CAMERA;

      disable_ctrl(IDC_DIST);
    }
    break;

    case IDC_RADIO_LOOKAT:
    {
      if(g_target_fx)
        g_target_fx->set_visible(true);

      last_radio = IDC_RADIO_LOOKAT;

      enable_ctrl(IDC_DIST);
    }
    break;

    case IDC_RADIO_MOUSE:
    {
      if(g_target_fx)
        g_target_fx->set_visible(true);

      last_radio = IDC_RADIO_MOUSE;

      enable_ctrl(IDC_DIST);
    }
    break;

    case IDC_GEN_NAME:
    {
      gen_name = get_check_box(IDC_GEN_NAME);
      enable_ctrl(IDC_GEN_SUFFIX, !get_check_box(IDC_GEN_NAME));
      enable_ctrl(IDC_ENTITY_ID, !get_check_box(IDC_GEN_NAME));

      generate_name();
    }
    break;

    case IDC_GEN_SUFFIX:
    {
      gen_suffix = get_check_box(IDC_GEN_SUFFIX);
    }
    break;

    case IDC_ORIENT:
    {
      orient = get_check_box(IDC_ORIENT);
    }
    break;

    default:
      Dialog::handle_command(ctrl);
      break;
	}
}

void SpawnDialog::handle_selection_change(int ctrl)
{
  switch(ctrl)
  {
    case IDC_ENTITY_FILE:
    {
      if(get_check_box(IDC_GEN_NAME))
      {
        char name[128] = "";
        get_combo_text(IDC_ENTITY_FILE, get_combo_index(IDC_ENTITY_FILE), name);
        set_ctrl_text(IDC_ENTITY_FILE, name);
        generate_name();
      }

    	update();
    }
    break;

    default:
      Dialog::handle_selection_change(ctrl);
      break;
  }
}



void SpawnDialog::handle_user_mouse_down(int button, vector3d pos, DWORD flags)
{
	if(!is_visible())
		return;

  if(button == 0 && get_radio_button(IDC_RADIO_HERO) == IDC_RADIO_MOUSE && g_target_fx)
    spawn_entity();
}

void SpawnDialog::handle_user_mouse_up(int button, vector3d pos, DWORD flags)
{
	if(!is_visible())
		return;
}










MarkerDialog::MarkerDialog(HWND pParent)
{
	init(IDD_MARKER, gAlterMarkerDialogFunc, pParent);
}

MarkerDialog::~MarkerDialog()
{
}

void MarkerDialog::setup()
{
  set_spinner_range(IDC_X_SPIN, 0, 20);
  set_spinner_pos(IDC_X_SPIN, 10);

  set_spinner_range(IDC_Y_SPIN, 0, 20);
  set_spinner_pos(IDC_Y_SPIN, 10);

  set_spinner_range(IDC_Z_SPIN, 0, 20);
  set_spinner_pos(IDC_Z_SPIN, 10);

	update();
}

void MarkerDialog::hide()
{
	Dialog::hide();
}

void MarkerDialog::update()
{
	if(!is_visible())
		return;

  set_spinner_pos(IDC_X_SPIN, 10);
  set_spinner_pos(IDC_Y_SPIN, 10);
  set_spinner_pos(IDC_Z_SPIN, 10);

  if(alter_dlg->curr_ent && (alter_dlg->curr_ent->get_flavor() == ENTITY_MARKER /*!|| alter_dlg->curr_ent->get_flavor() == ENTITY_CRAWL_MARKER !*/))
  {
    hide_all_ctrl();
    show_ctrl(IDC_STATIC_MARKER);

    marker *mark = (marker *)alter_dlg->curr_ent;
/*!    if(mark->is_a_crawl_marker())
    {
      set_ctrl_text(IDC_STATIC_MARKER, "Type: CRAWL_MARKER");

      crawl_marker *crawl = (crawl_marker *)mark;

      show_ctrl(IDC_STATIC_X_WIDTH);
      show_ctrl(IDC_X_WIDTH);
      show_ctrl(IDC_X_SPIN);

      show_ctrl(IDC_STATIC_Z_WIDTH);
      show_ctrl(IDC_Z_WIDTH);
      show_ctrl(IDC_Z_SPIN);

      show_ctrl(IDC_CRAWL_PINPOINT);
      show_ctrl(IDC_APPLY);

      set_check_box(IDC_CRAWL_PINPOINT, crawl->is_ext_flagged(EFLAG_EXT_CRAWL_PINPOINT));

      set_ctrl_text(IDC_X_WIDTH, "%.2f", crawl->x_rad);
      set_ctrl_text(IDC_Z_WIDTH, "%.2f", crawl->z_rad);
    }
    else !*/if(mark->is_a_cube_marker())
    {
      set_ctrl_text(IDC_STATIC_MARKER, "Type: CUBE_MARKER");
    }
    else if(mark->is_a_rectangle_marker())
    {
      set_ctrl_text(IDC_STATIC_MARKER, "Type: RECTANGLE_MARKER");
    }
    else
    {
      set_ctrl_text(IDC_STATIC_MARKER, "Type: MARKER");
    }
  }
  else
  {
    hide_all_ctrl();
    show_ctrl(IDC_STATIC_ERROR);
  }

  show_ctrl(IDC_RENDER);
  set_check_box(IDC_RENDER, g_render_markers);

	Dialog::update();
}

void MarkerDialog::render()
{
	if(!is_visible())
		return;
}



void MarkerDialog::frame_advance(time_value_t t)
{
	if(!is_visible())
		return;
}

void MarkerDialog::handle_slider(int ctrl, int pos)
{
	switch(ctrl)
	{
    case IDC_X_SPIN:
    {
      if(alter_dlg->curr_ent && (alter_dlg->curr_ent->get_flavor() == ENTITY_MARKER /*! || alter_dlg->curr_ent->get_flavor() == ENTITY_CRAWL_MARKER !*/))
      {
        float x = get_ctrl_float(IDC_X_WIDTH);
        x += (((float)(pos-10))/100.0f);
        set_ctrl_text(IDC_X_WIDTH, "%.2f", x);
        set_spinner_pos(IDC_X_SPIN, 10);

        handle_command(IDC_APPLY);
      }
    }
    break;

    case IDC_Y_SPIN:
    {
      if(alter_dlg->curr_ent && (alter_dlg->curr_ent->get_flavor() == ENTITY_MARKER/*! || alter_dlg->curr_ent->get_flavor() == ENTITY_CRAWL_MARKER !*/))
      {
        float y = get_ctrl_float(IDC_Y_WIDTH);
        y += (((float)(pos-10))/100.0f);
        set_ctrl_text(IDC_Y_WIDTH, "%.2f", y);
        set_spinner_pos(IDC_Y_SPIN, 10);

        handle_command(IDC_APPLY);
      }
    }
    break;

    case IDC_Z_SPIN:
    {
      if(alter_dlg->curr_ent && (alter_dlg->curr_ent->get_flavor() == ENTITY_MARKER/*! || alter_dlg->curr_ent->get_flavor() == ENTITY_CRAWL_MARKER !*/))
      {
        float z = get_ctrl_float(IDC_Z_WIDTH);
        z += (((float)(pos-10))/100.0f);
        set_ctrl_text(IDC_Z_WIDTH, "%.2f", z);
        set_spinner_pos(IDC_Z_SPIN, 10);

        handle_command(IDC_APPLY);
      }
    }
    break;

    default:
      Dialog::handle_slider(ctrl, pos);
      break;
  }
}

void MarkerDialog::handle_command(int ctrl)
{
	switch(ctrl)
	{
    case IDCANCEL:
      break;

    case IDC_CRAWL_PINPOINT:
    {
      if(alter_dlg->curr_ent && (alter_dlg->curr_ent->get_flavor() == ENTITY_MARKER/*! || alter_dlg->curr_ent->get_flavor() == ENTITY_CRAWL_MARKER !*/))
      {
        marker *mark = (marker *)alter_dlg->curr_ent;
/*!        if(mark->is_a_crawl_marker())
        {
          crawl_marker *crawl = (crawl_marker *)mark;
          crawl->set_ext_flag(EFLAG_EXT_CRAWL_PINPOINT, get_check_box(IDC_CRAWL_PINPOINT));

          crawl->set_needs_export(true);
        }
!*/
      }
    }
    break;

    case IDC_RENDER:
    {
      g_render_markers = get_check_box(IDC_RENDER);
    }
    break;

    case IDC_APPLY:
    {
      set_spinner_pos(IDC_X_SPIN, 10);
      set_spinner_pos(IDC_Y_SPIN, 10);
      set_spinner_pos(IDC_Z_SPIN, 10);

      if(alter_dlg->curr_ent && (alter_dlg->curr_ent->get_flavor() == ENTITY_MARKER/*! || alter_dlg->curr_ent->get_flavor() == ENTITY_CRAWL_MARKER !*/))
      {
        marker *mark = (marker *)alter_dlg->curr_ent;
/*!        if(mark->is_a_crawl_marker())
        {
          crawl_marker *crawl = (crawl_marker *)mark;

          crawl->set_ext_flag(EFLAG_EXT_CRAWL_PINPOINT, get_check_box(IDC_CRAWL_PINPOINT));

          crawl->x_rad = get_ctrl_float(IDC_X_WIDTH);
          crawl->z_rad = get_ctrl_float(IDC_Z_WIDTH);

          crawl->set_needs_export(true);
        }
        else
!*/
        if(mark->is_a_cube_marker())
        {
        }
        else if(mark->is_a_rectangle_marker())
        {
        }
        else
        {
        }
      }
    }
    break;

    default:
      Dialog::handle_command(ctrl);
      break;
  }
}










ItemDialog::ItemDialog(HWND pParent)
{
	init(IDD_ITEM, gAlterItemDialogFunc, pParent);
}

ItemDialog::~ItemDialog()
{
}

void ItemDialog::setup()
{
  set_spinner_buddy(IDC_COUNT_SPIN, IDC_COUNT);
  set_spinner_range(IDC_COUNT_SPIN, 1, 32767);

	update();
}

void ItemDialog::hide()
{
	Dialog::hide();
}

void ItemDialog::update()
{
	if(!is_visible())
		return;

  if(alter_dlg->curr_ent && alter_dlg->curr_ent->get_flavor() == ENTITY_ITEM)
  {
    show_all_ctrl();
    hide_ctrl(IDC_STATIC_ERROR);

    set_ctrl_int(IDC_COUNT, ((item *)alter_dlg->curr_ent)->get_original_count());
    set_spinner_pos(IDC_COUNT_SPIN, ((item *)alter_dlg->curr_ent)->get_original_count());
  }
  else
  {
    hide_all_ctrl();
    show_ctrl(IDC_STATIC_ERROR);
  }

	Dialog::update();
}

void ItemDialog::render()
{
	if(!is_visible())
		return;
}


void ItemDialog::frame_advance(time_value_t t)
{
	if(!is_visible())
		return;
}

void ItemDialog::handle_slider(int ctrl, int pos)
{
	switch(ctrl)
	{
    case IDC_COUNT_SPIN:
    {
      if(alter_dlg->curr_ent && alter_dlg->curr_ent->get_flavor() == ENTITY_ITEM)
      {
        ((item *)alter_dlg->curr_ent)->set_count(pos);
        ((item *)alter_dlg->curr_ent)->set_original_count(pos);
      }
    }
    break;

    default:
      Dialog::handle_slider(ctrl, pos);
      break;
  }
}

void ItemDialog::handle_command(int ctrl)
{
	switch(ctrl)
	{
    case IDCANCEL:
      break;

    case IDC_APPLY:
    {
      if(alter_dlg->curr_ent && alter_dlg->curr_ent->get_flavor() == ENTITY_ITEM)
      {
        int count = get_ctrl_int(IDC_COUNT);
        if(count <= 0)
          count = 1;

        ((item *)alter_dlg->curr_ent)->set_count(count);
        ((item *)alter_dlg->curr_ent)->set_original_count(count);

        update();
      }
    }
    break;

    default:
      Dialog::handle_command(ctrl);
      break;
  }
}










DestroyDialog::DestroyDialog(HWND pParent)
{
	init(IDD_DESTROY, gAlterDestroyDialogFunc, pParent);
}

DestroyDialog::~DestroyDialog()
{
}

void DestroyDialog::setup()
{
  set_spinner_buddy(IDC_HP_SPIN, IDC_HP);
  set_spinner_range(IDC_HP_SPIN, 0, 32767);

	update();
}

void DestroyDialog::hide()
{
	Dialog::hide();
}

void DestroyDialog::update()
{
	if(!is_visible())
		return;

  if(alter_dlg->curr_ent)
  {
    if(alter_dlg->curr_ent->has_destroy_info())
    {
      destroyable_info *dest = alter_dlg->curr_ent->get_destroy_info();

      show_all_ctrl();
      hide_ctrl(IDC_CREATE);

      set_ctrl_int(IDC_HP, dest->get_hit_points());
      set_spinner_pos(IDC_HP_SPIN, dest->get_hit_points());

      switch(alter_dlg->curr_ent->get_target_type())
      {
        case TARGET_TYPE_BIO:
          check_radio_button(IDC_BIO, IDC_BIO);
          break;

        case TARGET_TYPE_MECHANICAL:
          check_radio_button(IDC_BIO, IDC_MECH);
          break;
      }


      set_check_box(IDC_AUTO_AIM, alter_dlg->curr_ent->is_ext_flagged(EFLAG_EXT_TARGETABLE));
      set_check_box(IDC_REMAIN_ACTIVE, dest->remain_active());
      set_check_box(IDC_REMAIN_VISIBLE, dest->remain_visible());
      set_check_box(IDC_SINGLE_BLOW, dest->is_single_blow());
      set_check_box(IDC_INVULNERABLE, alter_dlg->curr_ent->is_invulnerable());

      set_check_box(IDC_EFFECT, dest->has_destroy_fx());
      enable_ctrl(IDC_EFFECT_STRING, dest->has_destroy_fx());
      enable_ctrl(IDC_LIFETIME, dest->has_destroy_fx());
      if(dest->has_destroy_fx())
      {
        set_ctrl_text(IDC_EFFECT_STRING, "%s", dest->get_destroy_fx().c_str());
        set_ctrl_float(IDC_LIFETIME, dest->get_destroy_lifetime());
      }
      else
      {
        set_ctrl_text(IDC_EFFECT_STRING, "");
        set_ctrl_text(IDC_LIFETIME, "");
      }

      set_check_box(IDC_SOUND, dest->has_destroy_sound());
      enable_ctrl(IDC_SOUND_STRING, dest->has_destroy_sound());
      if(dest->has_destroy_sound())
        set_ctrl_text(IDC_SOUND_STRING, "%s", dest->get_destroy_sound().c_str());
      else
        set_ctrl_text(IDC_SOUND_STRING, "");

      set_check_box(IDC_VISREP, dest->destroyed_visrep_flag());
      enable_ctrl(IDC_VISREP_STRING, dest->destroyed_visrep_flag());
      if(dest->destroyed_visrep_flag())
        set_ctrl_text(IDC_VISREP_STRING, "%s", dest->get_destroyed_visrep().c_str());
      else
        set_ctrl_text(IDC_VISREP_STRING, "");
    }
    else
    {
      hide_all_ctrl();
      show_ctrl(IDC_CREATE);
//!      enable_ctrl(IDC_CREATE, alter_dlg->curr_ent->get_flavor() != ENTITY_CHARACTER);
    }
  }
  else
  {
    hide_all_ctrl();
  }

	Dialog::update();
}

void DestroyDialog::render()
{
	if(!is_visible())
		return;
}



void DestroyDialog::frame_advance(time_value_t t)
{
	if(!is_visible())
		return;
}

void DestroyDialog::handle_slider(int ctrl, int pos)
{
	switch(ctrl)
	{
    case IDC_HP_SPIN:
    {
      if(alter_dlg->curr_ent && alter_dlg->curr_ent->has_destroy_info())
      {
        alter_dlg->curr_ent->get_destroy_info()->set_hit_points(pos);
        alter_dlg->curr_ent->set_needs_export(true);
      }
    }
    break;

    default:
      Dialog::handle_slider(ctrl, pos);
      break;
  }
}

void DestroyDialog::handle_command(int ctrl)
{
	switch(ctrl)
	{
    case IDCANCEL:
      break;

    case IDC_BIO:
    case IDC_MECH:
    {
      if(alter_dlg->curr_ent)
      {
        switch(get_radio_button(IDC_BIO))
        {
          case IDC_BIO:
            alter_dlg->curr_ent->set_target_type(TARGET_TYPE_BIO);
            break;

          case IDC_MECH:
            alter_dlg->curr_ent->set_target_type(TARGET_TYPE_MECHANICAL);
            break;
        }

        update();
      }
    }
    break;

    case IDC_APPLY:
    {
      if(alter_dlg->curr_ent && alter_dlg->curr_ent->has_destroy_info())
      {
        char value[256] = "";
        destroyable_info *dest = alter_dlg->curr_ent->get_destroy_info();

        int hp = get_ctrl_int(IDC_HP);
        if(hp < 0)
          hp = 0;
        dest->set_hit_points(hp);


        dest->set_has_destroy_fx(get_check_box(IDC_EFFECT));
        if(get_check_box(IDC_EFFECT))
        {
          get_ctrl_text(IDC_EFFECT_STRING, value, 255);
          dest->set_destroy_fx(value);
          dest->set_destroy_lifetime(get_ctrl_float(IDC_LIFETIME));
        }

        dest->set_has_destroy_sound(get_check_box(IDC_SOUND));
        if(get_check_box(IDC_SOUND))
        {
          get_ctrl_text(IDC_SOUND_STRING, value, 255);
          dest->set_destroy_sound(value);
        }

        dest->set_has_destroyed_visrep(get_check_box(IDC_VISREP));
        if(get_check_box(IDC_VISREP))
        {
          get_ctrl_text(IDC_VISREP_STRING, value, 255);
          dest->set_destroyed_visrep(value);
        }

        alter_dlg->curr_ent->set_needs_export(true);

        update();
      }
    }
    break;

    case IDC_CREATE:
    {
      if(alter_dlg->curr_ent && !alter_dlg->curr_ent->has_destroy_info())
      {
        alter_dlg->curr_ent->create_destroy_info();
        alter_dlg->curr_ent->set_needs_export(true);

        update();
      }
    }
    break;

    case IDC_AUTO_AIM:
    {
      if(alter_dlg->curr_ent && alter_dlg->curr_ent->has_destroy_info())
      {
        alter_dlg->curr_ent->set_ext_flag(EFLAG_EXT_TARGETABLE, get_check_box(IDC_AUTO_AIM));
        update();
      }
    }
    break;

    case IDC_REMAIN_ACTIVE:
    {
      if(alter_dlg->curr_ent && alter_dlg->curr_ent->has_destroy_info())
      {
        alter_dlg->curr_ent->get_destroy_info()->set_remain_active(get_check_box(IDC_REMAIN_ACTIVE));
        update();
      }
    }
    break;

    case IDC_REMAIN_VISIBLE:
    {
      if(alter_dlg->curr_ent && alter_dlg->curr_ent->has_destroy_info())
      {
        alter_dlg->curr_ent->get_destroy_info()->set_remain_visible(get_check_box(IDC_REMAIN_VISIBLE));
        update();
      }
    }
    break;

    case IDC_SINGLE_BLOW:
    {
      if(alter_dlg->curr_ent && alter_dlg->curr_ent->has_destroy_info())
      {
        alter_dlg->curr_ent->get_destroy_info()->set_single_blow(get_check_box(IDC_SINGLE_BLOW));
        update();
      }
    }
    break;

    case IDC_INVULNERABLE:
    {
      if(alter_dlg->curr_ent)
      {
        alter_dlg->curr_ent->set_invulnerable(get_check_box(IDC_INVULNERABLE));

        update();
      }
    }
    break;

    case IDC_EFFECT:
    {
      if(alter_dlg->curr_ent && alter_dlg->curr_ent->has_destroy_info())
      {
        alter_dlg->curr_ent->get_destroy_info()->set_has_destroy_fx(get_check_box(IDC_EFFECT));
        update();
      }
    }
    break;

    case IDC_SOUND:
    {
      if(alter_dlg->curr_ent && alter_dlg->curr_ent->has_destroy_info())
      {
        alter_dlg->curr_ent->get_destroy_info()->set_has_destroy_sound(get_check_box(IDC_SOUND));
        update();
      }
    }
    break;

    case IDC_VISREP:
    {
      if(alter_dlg->curr_ent && alter_dlg->curr_ent->has_destroy_info())
      {
        alter_dlg->curr_ent->get_destroy_info()->set_has_destroyed_visrep(get_check_box(IDC_VISREP));
        update();
      }
    }
    break;


    default:
      Dialog::handle_command(ctrl);
      break;
  }
}










EntityDialog::EntityDialog(HWND pParent)
{
	init(IDD_ENTITY, gAlterEntityDialogFunc, pParent);
}

EntityDialog::~EntityDialog()
{
}

void EntityDialog::setup()
{
	update();
}

void EntityDialog::hide()
{
	Dialog::hide();
}

void EntityDialog::update()
{
	if(!is_visible())
		return;

  if(alter_dlg->curr_ent)
  {
    show_all_ctrl();
    hide_ctrl(IDC_STATIC_ERROR);

    set_ctrl_text(IDC_STATIC_FLAVOR, "%s: %s", entity_flavor_names[alter_dlg->curr_ent->get_flavor()], alter_dlg->curr_ent->ent_filename.c_str());

    set_check_box(IDC_BEAMABLE, alter_dlg->curr_ent->scene_flags & BEAMABLE_FLAG);
    set_check_box(IDC_SCANABLE, alter_dlg->curr_ent->scene_flags & SCANABLE_FLAG);

    if ( alter_dlg->curr_ent->get_colgeom() && alter_dlg->curr_ent->get_colgeom()->get_type() == collision_geometry::MESH )
    {
      cg_mesh* m = static_cast<cg_mesh*>( alter_dlg->curr_ent->get_colgeom() );

      enable_ctrl(IDC_CAMERA_COLLISION);
      enable_ctrl(IDC_ENTITY_COLLISION);
      set_check_box(IDC_CAMERA_COLLISION, alter_dlg->curr_ent->scene_flags & CAMERA_COLL_FLAG);
      set_check_box(IDC_ENTITY_COLLISION, alter_dlg->curr_ent->scene_flags & ENTITY_COLL_FLAG);
    }
    else
    {
      disable_ctrl(IDC_CAMERA_COLLISION);
      disable_ctrl(IDC_ENTITY_COLLISION);
    }

    set_check_box(IDC_ACTIONABLE, alter_dlg->curr_ent->scene_flags & ACTIONABLE_FLAG);
    set_check_box(IDC_ACTION_FACING, alter_dlg->curr_ent->scene_flags & ACTION_FACING_FLAG);

    set_check_box(IDC_DOOR, alter_dlg->curr_ent->scene_flags & IS_DOOR_FLAG);
    enable_ctrl(IDC_DOOR_OPEN, alter_dlg->curr_ent->scene_flags & IS_DOOR_FLAG);
    set_check_box(IDC_DOOR_OPEN, alter_dlg->curr_ent->scene_flags & DOOR_OPEN_FLAG);

    set_check_box(IDC_ACTIVE, alter_dlg->curr_ent->scene_flags & ACTIVE_FLAG);
    set_check_box(IDC_STATIONARY, alter_dlg->curr_ent->scene_flags & STATIONARY_FLAG);
    set_check_box(IDC_NONSTATIC, alter_dlg->curr_ent->scene_flags & NONSTATIC_FLAG);
    set_check_box(IDC_DIST_CLIP, !(alter_dlg->curr_ent->scene_flags & NO_DISTANCE_CLIP_FLAG));
    set_check_box(IDC_WALKABLE, alter_dlg->curr_ent->scene_flags & WALKABLE_FLAG);
    set_check_box(IDC_INVISIBLE, alter_dlg->curr_ent->scene_flags & INVISIBLE_FLAG);
    set_check_box(IDC_REPULSION, alter_dlg->curr_ent->scene_flags & REPULSION_FLAG);
    set_check_box(IDC_COSMETIC, alter_dlg->curr_ent->scene_flags & COSMETIC_FLAG);

//    enable_all_ctrl(alter_dlg->curr_ent->from_sin_file());
  }
  else
  {
    hide_all_ctrl();
    show_ctrl(IDC_STATIC_ERROR);
    enable_ctrl(IDC_STATIC_ERROR);
  }

	Dialog::update();
}

void EntityDialog::render()
{
	if(!is_visible())
		return;
}


void EntityDialog::frame_advance(time_value_t t)
{
	if(!is_visible())
		return;
}

void EntityDialog::handle_command(int ctrl)
{
	switch(ctrl)
	{
    case IDCANCEL:
      break;

    case IDC_BEAMABLE:
    {
      if(alter_dlg->curr_ent)
      {
        if(get_check_box(IDC_BEAMABLE))
          alter_dlg->curr_ent->scene_flags |= BEAMABLE_FLAG;
        else
          alter_dlg->curr_ent->scene_flags &= ~BEAMABLE_FLAG;
      }

      update();
    }
    break;

    case IDC_SCANABLE:
    {
      if(alter_dlg->curr_ent)
      {
        if(get_check_box(IDC_SCANABLE))
          alter_dlg->curr_ent->scene_flags |= SCANABLE_FLAG;
        else
          alter_dlg->curr_ent->scene_flags &= ~SCANABLE_FLAG;
      }

      update();
    }
    break;

    case IDC_ACTIONABLE:
    {
      if(alter_dlg->curr_ent)
      {
        if(get_check_box(IDC_ACTIONABLE))
          alter_dlg->curr_ent->scene_flags |= ACTIONABLE_FLAG;
        else
          alter_dlg->curr_ent->scene_flags &= ~ACTIONABLE_FLAG;
      }

      update();
    }
    break;

    case IDC_ACTION_FACING:
    {
      if(alter_dlg->curr_ent)
      {
        if(get_check_box(IDC_ACTION_FACING))
          alter_dlg->curr_ent->scene_flags |= ACTION_FACING_FLAG;
        else
          alter_dlg->curr_ent->scene_flags &= ~ACTION_FACING_FLAG;
      }

      update();
    }
    break;

    case IDC_DOOR:
    {
      if(alter_dlg->curr_ent)
      {
        if(get_check_box(IDC_DOOR))
          alter_dlg->curr_ent->scene_flags |= IS_DOOR_FLAG;
        else
          alter_dlg->curr_ent->scene_flags &= ~IS_DOOR_FLAG;
      }

      update();
    }
    break;

    case IDC_DOOR_OPEN:
    {
      if(alter_dlg->curr_ent)
      {
        if(get_check_box(IDC_DOOR_OPEN))
          alter_dlg->curr_ent->scene_flags |= DOOR_OPEN_FLAG;
        else
          alter_dlg->curr_ent->scene_flags &= ~DOOR_OPEN_FLAG;
      }

      update();
    }
    break;

    case IDC_CAMERA_COLLISION:
    {
      if ( alter_dlg->curr_ent && alter_dlg->curr_ent->get_colgeom() && alter_dlg->curr_ent->get_colgeom()->get_type() == collision_geometry::MESH )
      {
        if(get_check_box(IDC_CAMERA_COLLISION))
          alter_dlg->curr_ent->scene_flags |= CAMERA_COLL_FLAG;
        else
          alter_dlg->curr_ent->scene_flags &= ~CAMERA_COLL_FLAG;
      }

      update();
    }
    break;

    case IDC_ENTITY_COLLISION:
    {
      if ( alter_dlg->curr_ent && alter_dlg->curr_ent->get_colgeom() && alter_dlg->curr_ent->get_colgeom()->get_type() == collision_geometry::MESH )
      {
        if(get_check_box(IDC_ENTITY_COLLISION))
          alter_dlg->curr_ent->scene_flags |= ENTITY_COLL_FLAG;
        else
          alter_dlg->curr_ent->scene_flags &= ~ENTITY_COLL_FLAG;
      }

      update();
    }
    break;

    case IDC_ACTIVE:
    {
      if(alter_dlg->curr_ent)
      {
        if(get_check_box(IDC_ACTIVE))
          alter_dlg->curr_ent->scene_flags |= ACTIVE_FLAG;
        else
          alter_dlg->curr_ent->scene_flags &= ~ACTIVE_FLAG;
      }

      update();
    }
    break;

    case IDC_STATIONARY:
    {
      if(alter_dlg->curr_ent)
      {
        if(get_check_box(IDC_STATIONARY))
          alter_dlg->curr_ent->scene_flags |= STATIONARY_FLAG;
        else
          alter_dlg->curr_ent->scene_flags &= ~STATIONARY_FLAG;
      }

      update();
    }
    break;

    case IDC_NONSTATIC:
    {
      if(alter_dlg->curr_ent)
      {
        if(get_check_box(IDC_NONSTATIC))
          alter_dlg->curr_ent->scene_flags |= NONSTATIC_FLAG;
        else
          alter_dlg->curr_ent->scene_flags &= ~NONSTATIC_FLAG;
      }

      update();
    }
    break;

    case IDC_DIST_CLIP:
    {
      if(alter_dlg->curr_ent)
      {
        if(!get_check_box(IDC_DIST_CLIP))
          alter_dlg->curr_ent->scene_flags |= NO_DISTANCE_CLIP_FLAG;
        else
          alter_dlg->curr_ent->scene_flags &= ~NO_DISTANCE_CLIP_FLAG;
      }

      update();
    }
    break;

    case IDC_WALKABLE:
    {
      if(alter_dlg->curr_ent)
      {
        if(get_check_box(IDC_WALKABLE))
          alter_dlg->curr_ent->scene_flags |= WALKABLE_FLAG;
        else
          alter_dlg->curr_ent->scene_flags &= ~WALKABLE_FLAG;
      }

      update();
    }
    break;

    case IDC_INVISIBLE:
    {
      if(alter_dlg->curr_ent)
      {
        if(get_check_box(IDC_INVISIBLE))
          alter_dlg->curr_ent->scene_flags |= INVISIBLE_FLAG;
        else
          alter_dlg->curr_ent->scene_flags &= ~INVISIBLE_FLAG;
      }

      update();
    }
    break;

    case IDC_REPULSION:
    {
      if(alter_dlg->curr_ent)
      {
        if(get_check_box(IDC_REPULSION))
          alter_dlg->curr_ent->scene_flags |= REPULSION_FLAG;
        else
          alter_dlg->curr_ent->scene_flags &= ~REPULSION_FLAG;
      }

      update();
    }
    break;

    case IDC_COSMETIC:
    {
      if(alter_dlg->curr_ent)
      {
        if(get_check_box(IDC_COSMETIC))
          alter_dlg->curr_ent->scene_flags |= COSMETIC_FLAG;
        else
          alter_dlg->curr_ent->scene_flags &= ~COSMETIC_FLAG;
      }

      update();
    }
    break;

    default:
      Dialog::handle_command(ctrl);
      break;
  }

  if(alter_dlg->curr_ent)
  {
    alter_dlg->curr_ent->compute_sector(g_world_ptr->get_the_terrain());
    alter_dlg->curr_ent->set_needs_export(true);
  }
}










RotateDialog::RotateDialog(HWND pParent)
{
  inc = 1.0f;
  continuous = false;
  relative = false;

	init(IDD_ROTATE, gAlterRotDialogFunc, pParent);

  mouse_down = false;
  rot_z = false;
  speed = false;

  enable_mouse = true;
  mouse_relative = true;
  mouse_speed = 0.1f;
}

RotateDialog::~RotateDialog()
{
}

void RotateDialog::setup()
{
  set_ctrl_text(IDC_ROT_INC, "%.4f", inc);
  set_check_box(IDC_CONTINUOUS, continuous);
  set_check_box(IDC_RELATIVE, relative);

  mouse_down = false;
  rot_z = false;
  speed = false;

  set_check_box(IDC_MOUSE_ROT, enable_mouse);
  set_check_box(IDC_MOUSE_ROT_REL, mouse_relative);
  set_ctrl_text(IDC_MOUSE_ROT_SPEED, "%.4f", mouse_speed);
  enable_ctrl(IDC_MOUSE_ROT_REL, get_check_box(IDC_MOUSE_ROT));
  enable_ctrl(IDC_MOUSE_ROT_SPEED, get_check_box(IDC_MOUSE_ROT));

  update();
}

void RotateDialog::hide()
{
  inc = get_ctrl_float(IDC_ROT_INC);
  continuous = get_check_box(IDC_CONTINUOUS);
  relative = get_check_box(IDC_RELATIVE);

  enable_mouse = get_check_box(IDC_MOUSE_ROT);
  mouse_relative = get_check_box(IDC_MOUSE_ROT_REL);
  mouse_speed = get_ctrl_float(IDC_MOUSE_ROT_SPEED);

  mouse_down = false;
  rot_z = false;
  speed = false;

  Dialog::hide();
}

void RotateDialog::update()
{
	if(!is_visible())
		return;

  if(continuous)
    set_ctrl_text(IDC_STATIC_INC, "Degrees / sec");
  else
    set_ctrl_text(IDC_STATIC_INC, "Increment");

  if(alter_dlg->curr_ent && alter_dlg->curr_ent->from_sin_file())
  {
    show_all_ctrl();
    hide_ctrl(IDC_STATIC_ERROR);
  }
  else
  {
    hide_all_ctrl();
    show_ctrl(IDC_STATIC_ERROR);
  }

	Dialog::update();
}

void RotateDialog::render()
{
	if(!is_visible())
		return;
}


void RotateDialog::frame_advance(time_value_t t)
{
	if(!is_visible())
		return;

  if(alter_dlg->curr_ent && continuous)
  {
    eAxis rot = _AXIS_INVALID;
    bool neg = false;

    if(is_button_pressed(IDC_ROT_POSX) || (neg = is_button_pressed(IDC_ROT_NEGX)) != false)
    {
      rot = _AXIS_X;
    }
    else if(is_button_pressed(IDC_ROT_POSY) || (neg = is_button_pressed(IDC_ROT_NEGX)) != false)
    {
      rot = _AXIS_Y;
    }
    else if(is_button_pressed(IDC_ROT_POSZ) || (neg = is_button_pressed(IDC_ROT_NEGX)) != false)
    {
      rot = _AXIS_Z;
    }

    if(rot != _AXIS_INVALID)
    {
      rational_t ang = inc = get_ctrl_float(IDC_ROT_INC);

      if(neg)
        ang = -ang;

      rotate_entity(alter_dlg->curr_ent, rot, ( DEG_TO_RAD(ang) * t ), relative);

//      update();
    }
  }

  if(enable_mouse && mouse_down)
  {
    if((mouse_down = mouse_inside_client(g_hwnd)) != false)
    {
      POINT pt = get_mouse_position();

      vector3d delta = ZEROVEC;

      delta.y = pt.y - mouse_pos.y;

      bool shift = _KEY_DOWN(VK_SHIFT);
      if(rot_z || shift)
        delta.z = pt.x - mouse_pos.x;
      else
        delta.x = pt.x - mouse_pos.x;

      if(delta != ZEROVEC)
      {
        rational_t ang = get_ctrl_float(IDC_MOUSE_ROT_SPEED);

        if(speed)
          ang *= 5;

        if(delta.x != 0.0f)
          rotate_entity(alter_dlg->curr_ent, _AXIS_Y, ( DEG_TO_RAD(ang) * delta.x ), mouse_relative);

        if(delta.y != 0.0f)
          rotate_entity(alter_dlg->curr_ent, _AXIS_X, ( DEG_TO_RAD(ang) * delta.y ), mouse_relative);

        if(delta.z != 0.0f)
          rotate_entity(alter_dlg->curr_ent, _AXIS_Z, ( DEG_TO_RAD(ang) * delta.z ), mouse_relative);

        center_mouse(g_hwnd);
        pt = get_mouse_position();
        mouse_pos.x = pt.x;
        mouse_pos.y = pt.y;
      }

//      ShowCursor(false);
    }
    else
    {
      mouse_down = false;
//      ShowCursor(true);
    }
  }
}


void RotateDialog::handle_command(int ctrl)
{
  eAxis rot = _AXIS_INVALID;
  bool neg = false;

	switch(ctrl)
	{
    case IDCANCEL:
      break;

    case IDC_CONTINUOUS:
    {
      continuous = get_check_box(IDC_CONTINUOUS);

      update();
    }
    break;

    case IDC_RELATIVE:
    {
      relative = get_check_box(IDC_RELATIVE);
    }
    break;

    case IDC_MOUSE_ROT:
    {
      enable_mouse = get_check_box(IDC_MOUSE_ROT);
      enable_ctrl(IDC_MOUSE_ROT_REL, get_check_box(IDC_MOUSE_ROT));
      enable_ctrl(IDC_MOUSE_ROT_SPEED, get_check_box(IDC_MOUSE_ROT));
    }
    break;

    case IDC_MOUSE_ROT_REL:
    {
      mouse_relative = get_check_box(IDC_MOUSE_ROT_REL);
    }
    break;

    case IDC_RESET:
    {
      if(alter_dlg->curr_ent)
      {
        po newpo = po_identity_matrix;
        newpo.set_position( alter_dlg->curr_ent->get_abs_position() );

        alter_dlg->curr_ent->set_rel_po( newpo );
      }
    }
    break;

    case IDC_ROT_POSX:
    case IDC_ROT_NEGX:
    {
      if(alter_dlg->curr_ent && !continuous)
      {
        rot = _AXIS_X;

        if(ctrl == IDC_ROT_NEGX)
          neg = true;
      }
    }
    break;

    case IDC_ROT_POSY:
    case IDC_ROT_NEGY:
    {
      if(alter_dlg->curr_ent && !continuous)
      {
        rot = _AXIS_Y;

        if(ctrl == IDC_ROT_NEGY)
          neg = true;
      }
    }
    break;

    case IDC_ROT_POSZ:
    case IDC_ROT_NEGZ:
    {
      if(alter_dlg->curr_ent && !continuous)
      {
        rot = _AXIS_Z;

        if(ctrl == IDC_ROT_NEGZ)
          neg = true;
      }
    }
    break;

    default:
      Dialog::handle_command(ctrl);
      break;
  }

  if(rot != _AXIS_INVALID)
  {
    rational_t ang = inc = get_ctrl_float(IDC_ROT_INC);

    if(neg)
      ang = -ang;

    rotate_entity(alter_dlg->curr_ent, rot, DEG_TO_RAD(ang), relative );

//    update();
  }
}


void RotateDialog::handle_user_mouse_down(int button, vector3d pos, DWORD flags)
{
	if(!is_visible())
		return;

  if(enable_mouse && button == 2)
  {
    mouse_down = true;

    center_mouse(g_hwnd);

    POINT pt = get_mouse_position();

    mouse_pos.x = pt.x;
    mouse_pos.y = pt.y;

    rot_z = (flags & MK_CONTROL) != 0;
    speed = (flags & MK_SHIFT) != 0;

//    ShowCursor(false);
  }
}

void RotateDialog::handle_user_mouse_up(int button, vector3d pos, DWORD flags)
{
	if(!is_visible())
		return;

  if(button == 2)
  {
//    ShowCursor(true);

    mouse_down = false;
    rot_z = false;
    speed = false;
  }
}






TranslateDialog::TranslateDialog(HWND pParent)
{
  inc = 0.1f;
  continuous = false;
  relative = false;

	init(IDD_TRANSLATE, gAlterTransDialogFunc, pParent);

  mouse_down = false;
  trans_z = false;
  speed = false;

  enable_mouse = true;
  mouse_relative = true;
  mouse_speed = 0.001f;
}

TranslateDialog::~TranslateDialog()
{
}

void TranslateDialog::setup()
{
  set_ctrl_text(IDC_TRANS_INC, "%.4f", inc);
  set_check_box(IDC_CONTINUOUS, continuous);
  set_check_box(IDC_RELATIVE, relative);

  mouse_down = false;
  trans_z = false;
  speed = false;

  set_check_box(IDC_MOUSE_TRANS, enable_mouse);
  set_check_box(IDC_MOUSE_TRANS_REL, mouse_relative);
  set_ctrl_text(IDC_MOUSE_TRANS_SPEED, "%.4f", mouse_speed);
  enable_ctrl(IDC_MOUSE_TRANS_REL, get_check_box(IDC_MOUSE_TRANS));
  enable_ctrl(IDC_MOUSE_TRANS_SPEED, get_check_box(IDC_MOUSE_TRANS));

	update();
}

void TranslateDialog::hide()
{
  inc = get_ctrl_float(IDC_TRANS_INC);
  continuous = get_check_box(IDC_CONTINUOUS);
  relative = get_check_box(IDC_RELATIVE);

  enable_mouse = get_check_box(IDC_MOUSE_TRANS);
  mouse_relative = get_check_box(IDC_MOUSE_TRANS_REL);
  mouse_speed = get_ctrl_float(IDC_MOUSE_TRANS_SPEED);

  mouse_down = false;
  trans_z = false;
  speed = false;

	Dialog::hide();
}

void TranslateDialog::update()
{
	if(!is_visible())
		return;

  if(alter_dlg->curr_ent)
    set_ctrl_text(IDC_STATIC_POS, "< %.2f,  %.2f,  %.2f >", alter_dlg->curr_ent->get_abs_position().x, alter_dlg->curr_ent->get_abs_position().y, alter_dlg->curr_ent->get_abs_position().z);
  else
    set_ctrl_text(IDC_STATIC_POS, "< X,  Y,  Z >");

  if(continuous)
    set_ctrl_text(IDC_STATIC_INC, "Meters / sec");
  else
    set_ctrl_text(IDC_STATIC_INC, "Increment");

  if(alter_dlg->curr_ent && alter_dlg->curr_ent->from_sin_file())
  {
    show_all_ctrl();
    hide_ctrl(IDC_STATIC_ERROR);
  }
  else
  {
    hide_all_ctrl();
    show_ctrl(IDC_STATIC_ERROR);
  }

  Dialog::update();
}

void TranslateDialog::render()
{
	if(!is_visible())
		return;
}

void TranslateDialog::frame_advance(time_value_t t)
{
	if(!is_visible())
		return;

  if(alter_dlg->curr_ent && continuous)
  {
    vector3d trans = ZEROVEC;
    bool neg = false;

    if(is_button_pressed(IDC_TRANS_POSX) || (neg = is_button_pressed(IDC_TRANS_NEGX)) != false)
    {
      trans = XVEC;
    }
    else if(is_button_pressed(IDC_TRANS_POSY) || (neg = is_button_pressed(IDC_TRANS_NEGY)) != false)
    {
      trans = YVEC;
    }
    else if(is_button_pressed(IDC_TRANS_POSZ) || (neg = is_button_pressed(IDC_TRANS_NEGZ)) != false)
    {
      trans = ZVEC;
    }

    if(trans != ZEROVEC)
    {
      if(neg)
        trans = -trans;

      inc = get_ctrl_float(IDC_TRANS_INC);
      translate_entity(alter_dlg->curr_ent, trans * (inc * t), relative);
      update();
    }
  }

  if(enable_mouse && mouse_down)
  {
    if((mouse_down = mouse_inside_client(g_hwnd)) != false)
    {
      POINT pt = get_mouse_position();

      vector3d delta = ZEROVEC;
      delta.x = pt.x - mouse_pos.x;

      bool shift = _KEY_DOWN(VK_SHIFT);
      if(trans_z || shift)
        delta.z = -(pt.y - mouse_pos.y);
      else
        delta.y = -(pt.y - mouse_pos.y);

      if(delta.x != 0.0f || delta.y != 0.0f || delta.z != 0.0f)
      {
        rational_t s = get_ctrl_float(IDC_MOUSE_TRANS_SPEED);

        if(speed)
          s *= 5;

        translate_entity(alter_dlg->curr_ent, delta * s, mouse_relative);

        center_mouse(g_hwnd);
        pt = get_mouse_position();
        mouse_pos.x = pt.x;
        mouse_pos.y = pt.y;
      }

//      ShowCursor(false);
    }
    else
    {
      mouse_down = false;
//      ShowCursor(true);
    }
  }
}


void TranslateDialog::handle_command(int ctrl)
{
  vector3d trans = ZEROVEC;

	switch(ctrl)
	{
    case IDCANCEL:
      break;

    case IDC_CONTINUOUS:
    {
      continuous = get_check_box(IDC_CONTINUOUS);

      update();
    }
    break;

    case IDC_RELATIVE:
    {
      relative = get_check_box(IDC_RELATIVE);
    }
    break;

    case IDC_MOUSE_TRANS:
    {
      enable_mouse = get_check_box(IDC_MOUSE_TRANS);
      enable_ctrl(IDC_MOUSE_TRANS_REL, get_check_box(IDC_MOUSE_TRANS));
      enable_ctrl(IDC_MOUSE_TRANS_SPEED, get_check_box(IDC_MOUSE_TRANS));
    }
    break;

    case IDC_MOUSE_TRANS_REL:
    {
      mouse_relative = get_check_box(IDC_MOUSE_TRANS_REL);
    }
    break;

    case IDC_TRANS_POSX:
    case IDC_TRANS_NEGX:
    {
      if(alter_dlg->curr_ent && !continuous)
      {
        trans = XVEC;

        if(ctrl == IDC_TRANS_NEGX)
          trans = -trans;
      }
    }
    break;

    case IDC_TRANS_POSY:
    case IDC_TRANS_NEGY:
    {
      if(alter_dlg->curr_ent && !continuous)
      {
        trans = YVEC;

        if(ctrl == IDC_TRANS_NEGY)
          trans = -trans;
      }
    }
    break;

    case IDC_TRANS_POSZ:
    case IDC_TRANS_NEGZ:
    {
      if(alter_dlg->curr_ent && !continuous)
      {
        trans = ZVEC;

        if(ctrl == IDC_TRANS_NEGZ)
          trans = -trans;
      }
    }
    break;

    default:
      Dialog::handle_command(ctrl);
      break;
  }

  if(trans != ZEROVEC)
  {
    inc = get_ctrl_float(IDC_TRANS_INC);
    translate_entity(alter_dlg->curr_ent, trans * inc, relative);
    update();
  }
}


void TranslateDialog::handle_user_mouse_down(int button, vector3d pos, DWORD flags)
{
	if(!is_visible())
		return;

  if(enable_mouse && button == 2)
  {
    mouse_down = true;

    center_mouse(g_hwnd);

    POINT pt = get_mouse_position();

    mouse_pos.x = pt.x;
    mouse_pos.y = pt.y;

    trans_z = (flags & MK_CONTROL) != 0;
    speed = (flags & MK_SHIFT) != 0;

//    ShowCursor(false);
  }
}

void TranslateDialog::handle_user_mouse_up(int button, vector3d pos, DWORD flags)
{
	if(!is_visible())
		return;

  if(button == 2)
  {
//    ShowCursor(true);

    mouse_down = false;
    trans_z = false;
    speed = false;
  }
}





ScannerDialog::ScannerDialog(HWND pParent)
{
	init(IDD_SCANNER, gAlterScannerDialogFunc, pParent);
}

ScannerDialog::~ScannerDialog()
{
}

void ScannerDialog::setup()
{
  set_slider_range(IDC_APERTURE, 0, 180*10);
  set_slider_range(IDC_SWEEP, 0, 360*10);

  set_spinner_buddy(IDC_ALPHA_SPIN, IDC_COLOR_A);
  set_spinner_range(IDC_ALPHA_SPIN, 0, 255);


	update();
}

void ScannerDialog::hide()
{
	Dialog::hide();
}

void ScannerDialog::update()
{
	if(!is_visible())
		return;

  scanner *scan = NULL;

  if(alter_dlg->curr_ent)
  {
    if(alter_dlg->curr_ent->get_flavor() == ENTITY_SCANNER)
      scan = (scanner *)alter_dlg->curr_ent;
    else if(alter_dlg->curr_ent->get_flavor() == ENTITY_CONGLOMERATE)
      scan = ((conglomerate *)alter_dlg->curr_ent)->get_scanner();
  }

  if(scan)
  {
    if(scan->is_flagged(scanner::OPTIMIZED))
      scan->build_poly_list(false);

    show_all_ctrl();
    hide_ctrl(IDC_STATIC_ERROR);

    set_ctrl_text(IDC_LOOKAT_X, "%.2f", scan->lookat.x);
    set_ctrl_text(IDC_LOOKAT_Y, "%.2f", scan->lookat.y);
    set_ctrl_text(IDC_LOOKAT_Z, "%.2f", scan->lookat.z);

    set_ctrl_text(IDC_STATIC_COLOR, "< %d, %d, %d >", scan->my_color.c.r, scan->my_color.c.g, scan->my_color.c.b);

    set_ctrl_int(IDC_COLOR_A, scan->my_color.c.a);
    set_spinner_pos(IDC_ALPHA_SPIN, scan->my_color.c.a);

    rational_t val = 0.0f;

    val = __fabs(RAD_TO_DEG(scan->get_speed()));
    set_ctrl_text(IDC_SPEED, "%.2f", val);

    set_ctrl_text(IDC_THICKNESS, "%.2f", scan->thickness);
    set_ctrl_text(IDC_MAX_LENGTH, "%.2f", scan->max_length);

    set_check_box(IDC_SPINNER, scan->is_flagged(scanner::SPINNER));
    set_check_box(IDC_REVERSE, scan->is_flagged(scanner::REVERSE));

    val = RAD_TO_DEG(scan->get_aperture());
    set_ctrl_text(IDC_APERTURE_VAL, "%.2f", val);
    set_slider_pos(IDC_APERTURE, (int)((val*10) + 0.5f));

    val = RAD_TO_DEG(scan->get_sweep());
    set_ctrl_text(IDC_SWEEP_VAL, "%.2f", val);
    set_slider_pos(IDC_SWEEP, (int)((val*10) + 0.5f));
  }
  else
  {
    hide_all_ctrl();
    show_ctrl(IDC_STATIC_ERROR);
  }


	Dialog::update();
}

void ScannerDialog::render()
{
	if(!is_visible())
		return;
}


void ScannerDialog::frame_advance(time_value_t t)
{
	if(!is_visible())
		return;
}


void ScannerDialog::handle_slider(int ctrl, int pos)
{
  switch(ctrl)
  {
    case IDC_ALPHA_SPIN:
    {
      if(alter_dlg->curr_ent)
      {
        scanner *scan = NULL;

        if(alter_dlg->curr_ent->get_flavor() == ENTITY_SCANNER)
          scan = (scanner *)alter_dlg->curr_ent;
        else if(alter_dlg->curr_ent->get_flavor() == ENTITY_CONGLOMERATE)
          scan = ((conglomerate *)alter_dlg->curr_ent)->get_scanner();

        if(scan)
        {
          scan->my_color.c.a = pos;

          update();
        }
      }
    }
    break;

    case IDC_APERTURE:
    {
      if(alter_dlg->curr_ent)
      {
        set_ctrl_text(IDC_APERTURE_VAL, "%.2f", ((rational_t)pos/10.0f));

        scanner *scan = NULL;

        if(alter_dlg->curr_ent->get_flavor() == ENTITY_SCANNER)
          scan = (scanner *)alter_dlg->curr_ent;
        else if(alter_dlg->curr_ent->get_flavor() == ENTITY_CONGLOMERATE)
          scan = ((conglomerate *)alter_dlg->curr_ent)->get_scanner();

        if(scan)
          scan->curr_aperture = scan->max_aperture = DEG_TO_RAD((rational_t)pos/10.0f);
      }
    }
    break;

    case IDC_SWEEP:
    {
      if(alter_dlg->curr_ent)
      {
        set_ctrl_text(IDC_SWEEP_VAL, "%.2f", ((rational_t)pos/10.0f));

        scanner *scan = NULL;

        if(alter_dlg->curr_ent->get_flavor() == ENTITY_SCANNER)
          scan = (scanner *)alter_dlg->curr_ent;
        else if(alter_dlg->curr_ent->get_flavor() == ENTITY_CONGLOMERATE)
          scan = ((conglomerate *)alter_dlg->curr_ent)->get_scanner();

        if(scan)
          scan->curr_sweep = scan->max_sweep = DEG_TO_RAD((rational_t)pos/10.0f);
      }
    }
    break;

    default:
      Dialog::handle_slider(ctrl, pos);
      break;
  }
}

void ScannerDialog::handle_command(int ctrl)
{
	switch(ctrl)
	{
    case IDCANCEL:
      break;

    case IDC_COLOR:
    {
      if(alter_dlg->curr_ent)
      {
        scanner *scan = NULL;

        if(alter_dlg->curr_ent->get_flavor() == ENTITY_SCANNER)
          scan = (scanner *)alter_dlg->curr_ent;
        else if(alter_dlg->curr_ent->get_flavor() == ENTITY_CONGLOMERATE)
          scan = ((conglomerate *)alter_dlg->curr_ent)->get_scanner();

        if(scan)
        {
          COLORREF col_ref = winRGB(scan->my_color.c.r, scan->my_color.c.g, scan->my_color.c.b);

          if(choose_color_dialog(&col_ref))
          {
            scan->my_color.c.r = GetRValue(col_ref);
            scan->my_color.c.g = GetGValue(col_ref);
            scan->my_color.c.b = GetBValue(col_ref);

            update();
          }
        }
      }
    }
    break;

    case IDC_APPLY:
    {
      if(alter_dlg->curr_ent)
      {
        scanner *scan = NULL;

        if(alter_dlg->curr_ent->get_flavor() == ENTITY_SCANNER)
          scan = (scanner *)alter_dlg->curr_ent;
        else if(alter_dlg->curr_ent->get_flavor() == ENTITY_CONGLOMERATE)
          scan = ((conglomerate *)alter_dlg->curr_ent)->get_scanner();

        if(scan)
        {
          scan->lookat.x = get_ctrl_float(IDC_LOOKAT_X);
          scan->lookat.y = get_ctrl_float(IDC_LOOKAT_Y);
          scan->lookat.z = get_ctrl_float(IDC_LOOKAT_Z);

          int a = get_ctrl_int(IDC_COLOR_A);
          if(a < 0)
            a = 0;
          if(a > 255)
            a = 255;

          scan->my_color.c.a = a;

          rational_t speed = DEG_TO_RAD(get_ctrl_float(IDC_SPEED));
          if((scan->speed < 0.0f && speed > 0.0f) || (scan->speed > 0.0f && speed < 0.0f))
            speed *= -1.0f;
          scan->speed = speed;

          scan->thickness = get_ctrl_float(IDC_THICKNESS);

          scan->max_length = get_ctrl_float(IDC_MAX_LENGTH);

          scan->set_flag(scanner::SPINNER, get_check_box(IDC_SPINNER));
          scan->set_flag(scanner::REVERSE, get_check_box(IDC_REVERSE));

          rational_t sweep = get_ctrl_float(IDC_SWEEP_VAL);
          if(sweep < 0.0f)
            sweep = 0.0f;
          if(sweep > 360.0f)
            sweep = 360.0f;
          sweep = DEG_TO_RAD(sweep);
          scan->curr_sweep = scan->max_sweep = sweep;

          rational_t aperture = get_ctrl_float(IDC_APERTURE_VAL);
          if(aperture < 0.0f)
            aperture = 0.0f;
          if(aperture > 180.0f)
            aperture = 180.0f;
          aperture = DEG_TO_RAD(aperture);
          scan->curr_aperture = scan->max_aperture = aperture;

          update();
        }
      }
    }
    break;

    case IDC_SPINNER:
    {
      if(alter_dlg->curr_ent)
      {
        scanner *scan = NULL;

        if(alter_dlg->curr_ent->get_flavor() == ENTITY_SCANNER)
          scan = (scanner *)alter_dlg->curr_ent;
        else if(alter_dlg->curr_ent->get_flavor() == ENTITY_CONGLOMERATE)
          scan = ((conglomerate *)alter_dlg->curr_ent)->get_scanner();

        if(scan)
          scan->set_flag(scanner::SPINNER, get_check_box(IDC_SPINNER));
      }
    }
    break;

    case IDC_REVERSE:
    {
      if(alter_dlg->curr_ent)
      {
        scanner *scan = NULL;

        if(alter_dlg->curr_ent->get_flavor() == ENTITY_SCANNER)
          scan = (scanner *)alter_dlg->curr_ent;
        else if(alter_dlg->curr_ent->get_flavor() == ENTITY_CONGLOMERATE)
          scan = ((conglomerate *)alter_dlg->curr_ent)->get_scanner();

        if(scan)
          scan->set_flag(scanner::REVERSE, get_check_box(IDC_REVERSE));
      }
    }
    break;

    case IDC_RESET_SCANNER:
    {
      if(alter_dlg->curr_ent)
      {
        scanner *scan = NULL;

        if(alter_dlg->curr_ent->get_flavor() == ENTITY_SCANNER)
          scan = (scanner *)alter_dlg->curr_ent;
        else if(alter_dlg->curr_ent->get_flavor() == ENTITY_CONGLOMERATE)
          scan = ((conglomerate *)alter_dlg->curr_ent)->get_scanner();

        if(scan)
          scan->reset();
      }
    }
    break;

    case IDC_RESET_ALL_SCANNERS:
    {
      vector<entity*>::const_iterator i = g_world_ptr->get_entities().begin();
      vector<entity*>::const_iterator i_end = g_world_ptr->get_entities().end();

      while(i != i_end)
      {
        if((*i) && (*i)->get_flavor() == ENTITY_SCANNER)
          ((scanner *)(*i))->reset();

        i++;
      }
    }
    break;

    default:
      Dialog::handle_command(ctrl);
      break;
  }
}







MaterialDialog::MaterialDialog(HWND pParent)
{
	init(IDD_MATERIAL, gAlterMaterialDialogFunc, pParent);
}

MaterialDialog::~MaterialDialog()
{
}

void MaterialDialog::setup()
{
  set_spinner_buddy(IDC_ALPHA_SPIN, IDC_COLOR_A);
  set_spinner_range(IDC_ALPHA_SPIN, 0, 255);

  populate_materials();

  update();
}

void MaterialDialog::hide()
{
	Dialog::hide();
}

void MaterialDialog::update()
{
	if(!is_visible())
		return;

  char name[128] = "";

  if(alter_dlg->curr_ent)
  {
    show_all_ctrl();
    hide_ctrl(IDC_STATIC_ERROR);

    if(alter_dlg->curr_ent->get_alternative_material_set())
    {
      for(int x=0; x<get_combo_count(IDC_MATERIALS); x++)
      {
        get_combo_text(IDC_MATERIALS, x, name);

        if(!stricmp(alter_dlg->curr_ent->get_alternative_material_set()->name->c_str(), name))
        {
          set_combo_index(IDC_MATERIALS, x);
          break;
        }
      }
    }

    color32 col = alter_dlg->curr_ent->get_render_color();
    set_ctrl_text(IDC_STATIC_COLOR, "< %d, %d, %d >", col.c.r, col.c.g, col.c.b);
    set_ctrl_int(IDC_COLOR_A, col.c.a);

    set_spinner_pos(IDC_ALPHA_SPIN, col.c.a);
  }
  else
  {
    hide_all_ctrl();
    show_ctrl(IDC_STATIC_ERROR);
  }

	Dialog::update();
}

void MaterialDialog::render()
{
	if(!is_visible())
		return;
}


void MaterialDialog::populate_materials()
{
  clear_combo(IDC_MATERIALS);

  add_combo_string(IDC_MATERIALS, "<base>");

  world_dynamics_system::material_set_list::iterator msi;
  for( msi = g_world_ptr->material_sets.begin(); msi != g_world_ptr->material_sets.end(); ++msi )
    add_combo_string(IDC_MATERIALS, "%s", ((*msi)->name)->c_str());

  set_combo_index(IDC_MATERIALS, 0);
}

void MaterialDialog::export_material_groups(ofstream &fout)
{
  if(fout.is_open())
  {
    world_dynamics_system::material_set_list::iterator msi;
    for( msi = g_world_ptr->material_sets.begin(); msi != g_world_ptr->material_sets.end(); ++msi )
      fout<<"  "<<((*msi)->name)->c_str()<<"\n";
  }
}

void MaterialDialog::frame_advance(time_value_t t)
{
	if(!is_visible())
		return;
}


void MaterialDialog::handle_slider(int ctrl, int pos)
{
  switch(ctrl)
  {
    case IDC_ALPHA_SPIN:
    {
      if(alter_dlg->curr_ent)
      {
        color32 col = alter_dlg->curr_ent->get_render_color();
        col.c.a = pos;
        alter_dlg->curr_ent->set_render_color(col);

        update();
      }
    }
    break;

    default:
      Dialog::handle_slider(ctrl, pos);
      break;
  }
}

void MaterialDialog::handle_selection_change(int ctrl)
{
  switch(ctrl)
  {
    case IDC_MATERIALS:
    {
      if(alter_dlg->curr_ent)
      {
        int x = get_combo_index(IDC_MATERIALS);

        char name[128] = "";
        get_combo_text(IDC_MATERIALS, x, name);

        if(x == 0 || !stricmp(name, "<base>"))
          alter_dlg->curr_ent->set_alternative_materials(NULL);
        else
          alter_dlg->curr_ent->set_alternative_materials(stringx(name));

        update();
      }
    }
    break;

    default:
      Dialog::handle_selection_change(ctrl);
      break;
  }
}


void MaterialDialog::handle_command(int ctrl)
{
	switch(ctrl)
	{
    case IDCANCEL:
      break;

    case IDC_COLOR:
    {
      if(alter_dlg->curr_ent)
      {
        color32 col = alter_dlg->curr_ent->get_render_color();
        COLORREF col_ref = winRGB(col.c.r, col.c.g, col.c.b);

        if(choose_color_dialog(&col_ref))
        {
          col.c.r = GetRValue(col_ref);
          col.c.g = GetGValue(col_ref);
          col.c.b = GetBValue(col_ref);

          alter_dlg->curr_ent->set_render_color(col);

          update();
        }
      }
    }
    break;

    case IDC_APPLY:
    {
      if(alter_dlg->curr_ent)
      {
        char name[128] = "";
        get_ctrl_text(IDC_MATERIALS, name, 127);

        if(stricmp(name, "<base>"))
        {
          material_set *set = NULL;

          if((set = g_world_ptr->get_material_set(stringx(name))) == NULL)
          {
            bool filelock = os_file::is_system_locked();
            os_file::system_unlock();

            g_world_ptr->add_material_set( stringx(name) );

            if ( filelock )
              os_file::system_lock();
          }

          if((set = g_world_ptr->get_material_set(stringx(name))) != NULL)
            alter_dlg->curr_ent->set_alternative_materials(set);
          else
            alert_dialog("Material '%s' does not exist!");
        }
        else
          alter_dlg->curr_ent->set_alternative_materials(NULL);

        color32 col = alter_dlg->curr_ent->get_render_color();
        int a;

        a = get_ctrl_int(IDC_COLOR_A);
        if(a < 0)
          a = 0;
        if(a > 255)
          a = 255;

        col.c.a = a;

        alter_dlg->curr_ent->set_render_color(col);

        update();
      }
    }
    break;

    default:
      Dialog::handle_command(ctrl);
      break;
  }
}








ParticleDialog::ParticleDialog(HWND pParent)
{
	init(IDD_PARTICLE, gAlterParticleDialogFunc, pParent);
}

ParticleDialog::~ParticleDialog()
{
}

#define PART_SLIDER_RES 100

void ParticleDialog::setup()
{
  part = NULL;

  clear_combo(IDC_PART_SYS);
  if(alter_dlg->curr_ent)
  {
    if(alter_dlg->curr_ent->get_flavor() == ENTITY_PARTICLE_GENERATOR)
    {
      disable_ctrl(IDC_PART_SYS);
      part = (particle_generator *)alter_dlg->curr_ent;
      add_combo_string(IDC_PART_SYS, "%s", part->filename.c_str());
      num_part_sys = 1;
    }
    else if(alter_dlg->curr_ent->get_flavor() == ENTITY_CONGLOMERATE)
    {
      num_part_sys = 0;
      conglomerate *ent = (conglomerate *)alter_dlg->curr_ent;
      vector<entity *>::const_iterator ei = ent->get_members().begin();
      vector<entity *>::const_iterator ei_end = ent->get_members().end();

      while(ei != ei_end)
      {
        if((*ei)->get_flavor() == ENTITY_PARTICLE_GENERATOR)
        {
          particle_generator *p = (particle_generator *)(*ei);
          add_combo_string(IDC_PART_SYS, "%s", ent->get_member_nodename(p).c_str());
          if(num_part_sys == 0)
            part = p;

          ++num_part_sys;
        }

        ++ei;
      }

      if(num_part_sys <= 1)
        disable_ctrl(IDC_PART_SYS);

      set_combo_index(IDC_PART_SYS, 0);
    }
  }

  set_slider_range(IDC_RATE, 1, PART_SLIDER_RES);
  set_slider_range(IDC_LIFE, 1, PART_SLIDER_RES);
  set_slider_range(IDC_BASESPEED, 0, PART_SLIDER_RES);
  set_slider_range(IDC_GROW, 0, PART_SLIDER_RES);
  set_slider_range(IDC_FADE, 0, PART_SLIDER_RES);
  set_slider_range(IDC_RATE_VARIANCE, 0, PART_SLIDER_RES);
  set_slider_range(IDC_LIFE_VARIANCE, 0, PART_SLIDER_RES);
  set_slider_range(IDC_SPEED_VARIANCE, 0, PART_SLIDER_RES);
  set_slider_range(IDC_SCALE_VARIANCE, 0, PART_SLIDER_RES);

  select_sys(part);
}

void ParticleDialog::hide()
{
	Dialog::hide();
}

void ParticleDialog::update()
{
	if(!is_visible())
		return;

  if(part)
  {
    show_all_ctrl();
    hide_ctrl(IDC_STATIC_ERROR);

    set_ctrl_text(IDC_VISREP, "%s", part->tool_visrep_name.c_str());

    float min = get_ctrl_int(IDC_RATE_MIN);
    float max = get_ctrl_int(IDC_RATE_MAX);
    int pos = ((part->birthrate - min) / (max - min)) * (PART_SLIDER_RES);
    if(pos < 0)
      pos = 0;
    if(pos > PART_SLIDER_RES)
      pos = PART_SLIDER_RES;
    set_ctrl_text(IDC_STATIC_RATE, "rate %d", part->birthrate);
    set_slider_pos(IDC_RATE, pos);

    min = get_ctrl_float(IDC_LIFE_MIN);
    max = get_ctrl_float(IDC_LIFE_MAX);
    pos = ((part->particle_life_span - min) / (max - min)) * (PART_SLIDER_RES);
    if(pos < 0)
      pos = 0;
    if(pos > PART_SLIDER_RES)
      pos = PART_SLIDER_RES;
    set_ctrl_text(IDC_STATIC_LIFE, "life %.2f", part->particle_life_span);
    set_slider_pos(IDC_LIFE, pos);

    min = get_ctrl_float(IDC_SPEED_MIN);
    max = get_ctrl_float(IDC_SPEED_MAX);
    pos = ((part->base_speed - min) / (max - min)) * (PART_SLIDER_RES);
    if(pos < 0)
      pos = 0;
    if(pos > PART_SLIDER_RES)
      pos = PART_SLIDER_RES;
    set_ctrl_text(IDC_STATIC_SPEED, "speed %.2f", part->base_speed);
    set_slider_pos(IDC_BASESPEED, pos);

    min = get_ctrl_float(IDC_GROW_MIN);
    max = get_ctrl_float(IDC_GROW_MAX);
    pos = ((part->grow_for - min) / (max - min)) * (PART_SLIDER_RES);
    if(pos < 0)
      pos = 0;
    if(pos > PART_SLIDER_RES)
      pos = PART_SLIDER_RES;
    set_ctrl_text(IDC_STATIC_GROW, "growfor %.2f", part->grow_for);
    set_slider_pos(IDC_GROW, pos);

    min = get_ctrl_float(IDC_FADE_MIN);
    max = get_ctrl_float(IDC_FADE_MAX);
    pos = ((part->shrink_for - min) / (max - min)) * (PART_SLIDER_RES);
    if(pos < 0)
      pos = 0;
    if(pos > PART_SLIDER_RES)
      pos = PART_SLIDER_RES;
    set_ctrl_text(IDC_STATIC_FADE, "fadefor %.2f", part->shrink_for);
    set_slider_pos(IDC_FADE, pos);

    max = part->rate_variation > 1.0f ? part->rate_variation : 1.0f;
    pos = (part->rate_variation / max) * (PART_SLIDER_RES);
    set_ctrl_text(IDC_STATIC_VARIANCE_RATE, "variance ( rate %.2f )", part->rate_variation);
    set_slider_pos(IDC_RATE_VARIANCE, pos);

    max = part->life_variation > 1.0f ? part->life_variation : 1.0f;
    pos = (part->life_variation / max) * (PART_SLIDER_RES);
    set_ctrl_text(IDC_STATIC_VARIANCE_LIFE, "variance ( life %.2f )", part->life_variation);
    set_slider_pos(IDC_LIFE_VARIANCE, pos);

    max = part->speed_variation > 1.0f ? part->speed_variation : 1.0f;
    pos = (part->speed_variation / max) * (PART_SLIDER_RES);
    set_ctrl_text(IDC_STATIC_VARIANCE_SPEED, "variance ( speed %.2f )", part->speed_variation);
    set_slider_pos(IDC_SPEED_VARIANCE, pos);

    max = part->scale_variation > 1.0f ? part->scale_variation : 1.0f;
    pos = (part->scale_variation / max) * (PART_SLIDER_RES);
    set_ctrl_text(IDC_STATIC_VARIANCE_SCALE, "offscale %.2f", part->scale_variation);
    set_slider_pos(IDC_SCALE_VARIANCE, pos);
  }
  else
  {
    hide_all_ctrl();
    show_ctrl(IDC_STATIC_ERROR);
  }


	Dialog::update();
}

void ParticleDialog::render()
{
	if(!is_visible())
		return;
}


void ParticleDialog::frame_advance(time_value_t t)
{
	if(!is_visible())
		return;
}

void ParticleDialog::select_sys(particle_generator *p)
{
  if(p)
  {
    float min = part->birthrate - 50.0f;
    float max = part->birthrate + 50.0f;
    if(min < 0.0f)
    {
      max -= min;
      min = 0.0f;
    }
    set_ctrl_text(IDC_RATE_MIN, "%d", (int)min);
    set_ctrl_text(IDC_RATE_MAX, "%d", (int)max);

    min = part->particle_life_span - 2.0f;
    max = part->particle_life_span + 2.0f;
    if(min < 0.0f)
    {
      max -= min;
      min = 0.0f;
    }
    set_ctrl_text(IDC_LIFE_MIN, "%.2f", min);
    set_ctrl_text(IDC_LIFE_MAX, "%.2f", max);

    min = part->base_speed - 5.0f;
    max = part->base_speed + 5.0f;
    if(min < 0.0f)
    {
      max -= min;
      min = 0.0f;
    }
    set_ctrl_text(IDC_SPEED_MIN, "%.2f", min);
    set_ctrl_text(IDC_SPEED_MAX, "%.2f", max);

    min = part->grow_for - 2.0f;
    max = part->grow_for + 2.0f;
    if(min < 0.0f)
    {
      max -= min;
      min = 0.0f;
    }
    set_ctrl_text(IDC_GROW_MIN, "%.2f", min);
    set_ctrl_text(IDC_GROW_MAX, "%.2f", max);

    min = part->shrink_for - 2.0f;
    max = part->shrink_for + 2.0f;
    if(min < 0.0f)
    {
      max -= min;
      min = 0.0f;
    }
    set_ctrl_text(IDC_FADE_MIN, "%.2f", min);
    set_ctrl_text(IDC_FADE_MAX, "%.2f", max);
  }

  update();
}

void ParticleDialog::handle_slider(int ctrl, int pos)
{
  switch(ctrl)
  {
    case IDC_RATE:
    {
      if(part)
      {
        rational_t percent = (rational_t)pos / (rational_t)PART_SLIDER_RES;

        int min = get_ctrl_int(IDC_RATE_MIN);
        int max = get_ctrl_int(IDC_RATE_MAX);
        part->birthrate = min + (int)(((max - min)*percent) + 0.5f);
        set_ctrl_text(IDC_STATIC_RATE, "rate %d", part->birthrate);

        part->reset_sys(false);
      }
    }
    break;

    case IDC_LIFE:
    {
      if(part)
      {
        rational_t percent = (rational_t)pos / (rational_t)PART_SLIDER_RES;

        float min = get_ctrl_float(IDC_LIFE_MIN);
        float max = get_ctrl_float(IDC_LIFE_MAX);
        part->particle_life_span = min + ((max - min)*percent);
        set_ctrl_text(IDC_STATIC_LIFE, "life %.2f", part->particle_life_span);

        part->reset_sys(false);
      }
    }
    break;

    case IDC_BASESPEED:
    {
      if(part)
      {
        rational_t percent = (rational_t)pos / (rational_t)PART_SLIDER_RES;

        float min = get_ctrl_float(IDC_SPEED_MIN);
        float max = get_ctrl_float(IDC_SPEED_MAX);
        part->base_speed = min + ((max - min)*percent);
        set_ctrl_text(IDC_STATIC_SPEED, "speed %.2f", part->base_speed);

        part->reset_sys(false);
      }
    }
    break;

    case IDC_GROW:
    {
      if(part)
      {
        rational_t percent = (rational_t)pos / (rational_t)PART_SLIDER_RES;

        float min = get_ctrl_float(IDC_SPEED_MIN);
        float max = get_ctrl_float(IDC_SPEED_MAX);
        part->grow_for = min + ((max - min)*percent);
        set_ctrl_text(IDC_STATIC_GROW, "growfor %.2f", part->grow_for);

        part->reset_sys(false);
      }
    }
    break;

    case IDC_FADE:
    {
      if(part)
      {
        rational_t percent = (rational_t)pos / (rational_t)PART_SLIDER_RES;

        float min = get_ctrl_float(IDC_SPEED_MIN);
        float max = get_ctrl_float(IDC_SPEED_MAX);
        part->shrink_for = min + ((max - min)*percent);
        set_ctrl_text(IDC_STATIC_FADE, "fadefor %.2f", part->shrink_for);

        part->reset_sys(false);
      }
    }
    break;

    case IDC_RATE_VARIANCE:
    {
      if(part)
      {
        part->rate_variation = (rational_t)pos / (rational_t)PART_SLIDER_RES;
        set_ctrl_text(IDC_STATIC_VARIANCE_RATE, "variance ( rate %.2f )", part->rate_variation);
      }
    }
    break;

    case IDC_LIFE_VARIANCE:
    {
      if(part)
      {
        part->life_variation = (rational_t)pos / (rational_t)PART_SLIDER_RES;
        set_ctrl_text(IDC_STATIC_VARIANCE_LIFE, "variance ( life %.2f )", part->life_variation);
      }
    }
    break;

    case IDC_SPEED_VARIANCE:
    {
      if(part)
      {
        part->speed_variation = (rational_t)pos / (rational_t)PART_SLIDER_RES;
        set_ctrl_text(IDC_STATIC_VARIANCE_SPEED, "variance ( speed %.2f )", part->speed_variation);
      }
    }
    break;

    case IDC_SCALE_VARIANCE:
    {
      if(part)
      {
        part->speed_variation = (rational_t)pos / (rational_t)PART_SLIDER_RES;
        set_ctrl_text(IDC_STATIC_VARIANCE_SCALE, "offscale %.2f", part->scale_variation);
      }
    }
    break;

    default:
      Dialog::handle_slider(ctrl, pos);
      break;
  }
}

void ParticleDialog::handle_command(int ctrl)
{
	switch(ctrl)
	{
    case IDCANCEL:
      break;

    case IDC_RESET_SYS:
    {
      if(part)
        part->reset_sys(true);
    }
    break;

    case IDC_SAVE:
    {
      if(part)
        part->write(part->filename);
    }
    break;

    case IDC_RESET_ALL_SYS:
    {
      if(alter_dlg->curr_ent)
      {
        if(alter_dlg->curr_ent->get_flavor() == ENTITY_PARTICLE_GENERATOR)
        {
          ((particle_generator *)alter_dlg->curr_ent)->reset_sys(true);
        }
        else if(alter_dlg->curr_ent->get_flavor() == ENTITY_CONGLOMERATE)
        {
          conglomerate *ent = (conglomerate *)alter_dlg->curr_ent;
          vector<entity *>::const_iterator ei = ent->get_members().begin();
          vector<entity *>::const_iterator ei_end = ent->get_members().end();

          while(ei != ei_end)
          {
            if((*ei)->get_flavor() == ENTITY_PARTICLE_GENERATOR)
            {
              particle_generator *p = (particle_generator *)(*ei);
              p->reset_sys(true);
            }

            ++ei;
          }
        }
      }
    }
    break;

    default:
      Dialog::handle_command(ctrl);
      break;
  }
}

void ParticleDialog::handle_selection_change(int ctrl)
{
  switch(ctrl)
  {
    case IDC_PART_SYS:
    {
      if(alter_dlg->curr_ent)
      {
        if(alter_dlg->curr_ent->get_flavor() == ENTITY_PARTICLE_GENERATOR)
          part = (particle_generator *)alter_dlg->curr_ent;
        else if(alter_dlg->curr_ent->get_flavor() == ENTITY_CONGLOMERATE && num_part_sys > 0)
        {
          char buf[1024];
          get_combo_text(IDC_PART_SYS, get_combo_index(IDC_PART_SYS), buf);
          stringx member = stringx(buf);
          member.to_upper();
          entity *ent = ((conglomerate *)alter_dlg->curr_ent)->get_member(member);
          part = (ent && ent->get_flavor() == ENTITY_PARTICLE_GENERATOR) ? (particle_generator *)ent : NULL;
        }
        else
          part = NULL;
      }
      else
        part = NULL;

      select_sys(part);
    }
    break;

    default:
      Dialog::handle_selection_change(ctrl);
      break;
  }
}






/*!



CharacterDialog::CharacterDialog(HWND pParent)
{
	init(IDD_CHARACTER, gAlterCharacterDialogFunc, pParent);
}

CharacterDialog::~CharacterDialog()
{
}

void CharacterDialog::setup()
{
  populate_character_ai();

  update();
}

void CharacterDialog::hide()
{
	Dialog::hide();
}

void CharacterDialog::update()
{
	if(!is_visible())
		return;

  char name[128] = "";

  if(alter_dlg->curr_ent && alter_dlg->curr_ent->get_flavor() == ENTITY_CHARACTER && !((character *)alter_dlg->curr_ent)->is_hero() && ((character *)alter_dlg->curr_ent)->get_brain())
  {
    show_all_ctrl();
    hide_ctrl(IDC_STATIC_ERROR);

    brain *brn = ((character *)alter_dlg->curr_ent)->get_brain();

    set_ctrl_text(IDC_STATIC_CHARACTER, "%s", alter_dlg->curr_ent->get_name().c_str());

    set_combo_index(IDC_IDLE_AI, (brn->ai_state[BRAIN_REACT_IDLE] - BRAIN_AI_STATE_REACT_IDLE) - 1);
    set_combo_index(IDC_ALERT_AI, brn->ai_state[BRAIN_REACT_ALERTED] == BRAIN_AI_STATE_NONE ? 0 : brn->ai_state[BRAIN_REACT_ALERTED] - BRAIN_AI_STATE_REACT_ALERTED);
    set_combo_index(IDC_ACTIVE_AI, brn->ai_state[BRAIN_REACT_COMBAT] == BRAIN_AI_STATE_NONE ? 0 : brn->ai_state[BRAIN_REACT_COMBAT] - BRAIN_AI_STATE_REACT_COMBAT);
  }
  else

  {
    hide_all_ctrl();
    show_ctrl(IDC_STATIC_ERROR);
  }

	Dialog::update();
}

void CharacterDialog::render()
{
	if(!is_visible())
		return;
}


void CharacterDialog::populate_character_ai()
{
  int i = 0;

  clear_combo(IDC_IDLE_AI);
  for(i=BRAIN_AI_STATE_REACT_IDLE+1; i<BRAIN_AI_STATE_REACT_ALERTED; ++i)
    add_combo_string(IDC_IDLE_AI, brain_ai_states[i].c_str());

  clear_combo(IDC_ALERT_AI);
  add_combo_string(IDC_ALERT_AI, brain_ai_states[BRAIN_AI_STATE_NONE].c_str());
  for(i=BRAIN_AI_STATE_REACT_ALERTED+1; i<BRAIN_AI_STATE_REACT_COMBAT; ++i)
    add_combo_string(IDC_ALERT_AI, brain_ai_states[i].c_str());

  clear_combo(IDC_ACTIVE_AI);
  add_combo_string(IDC_ACTIVE_AI, brain_ai_states[BRAIN_AI_STATE_NONE].c_str());
  for(i=BRAIN_AI_STATE_REACT_COMBAT+1; i<BRAIN_AI_STATE_REACT_TURRET; ++i)
    add_combo_string(IDC_ACTIVE_AI, brain_ai_states[i].c_str());
}

void CharacterDialog::export_brain_info(ofstream &fout)
{
  if(fout.is_open())
  {
    vector<entity *>::const_iterator i = g_world_ptr->get_entities().begin();
    vector<entity *>::const_iterator i_end = g_world_ptr->get_entities().end();

    while(i != i_end)
    {
      entity *ent = (*i);

      if(ent && ent->get_flavor() == ENTITY_CHARACTER && !((character *)ent)->is_hero())
      {
        character *chr = (character *)ent;
        brain *brn = chr->get_brain();

        fout<<"  "<<chr->get_name().c_str();
        fout<<" brain";

        fout<<" reactions";

        for(int i=BRAIN_REACT_IDLE; i<_BRAIN_MAX_REACTION_STATES; i++)
          fout<<" "<<reaction_levels[i].c_str()<<" "<<brain_ai_states[brn->ai_state[i]].c_str();

        fout<<" chunkend";

        fout<<" chunkend";

        fout<<" chunkend";

        fout<<"\n";
      }

      i++;
    }

    fout<<"\n";
  }
}

void CharacterDialog::frame_advance(time_value_t t)
{
	if(!is_visible())
		return;
}

void CharacterDialog::handle_selection_change(int ctrl)
{
  switch(ctrl)
  {
    case IDC_IDLE_AI:
    {
      if(alter_dlg->curr_ent && alter_dlg->curr_ent->get_flavor() == ENTITY_CHARACTER && ((character *)alter_dlg->curr_ent)->get_brain())
      {
        brain *brn = ((character *)alter_dlg->curr_ent)->get_brain();

        int x = get_combo_index(IDC_IDLE_AI);

        brn->set_ai_state(BRAIN_REACT_IDLE, (eAIState)(x+BRAIN_AI_STATE_REACT_IDLE+1));

        update();
      }
    }
    break;

    case IDC_ALERT_AI:
    {
      if(alter_dlg->curr_ent && alter_dlg->curr_ent->get_flavor() == ENTITY_CHARACTER && ((character *)alter_dlg->curr_ent)->get_brain())
      {
        brain *brn = ((character *)alter_dlg->curr_ent)->get_brain();

        int x = get_combo_index(IDC_ALERT_AI);

        if(x == 0)
          brn->set_ai_state(BRAIN_REACT_ALERTED, BRAIN_AI_STATE_NONE);
        else
          brn->set_ai_state(BRAIN_REACT_ALERTED, (eAIState)(x+BRAIN_AI_STATE_REACT_ALERTED));

        update();
      }
    }
    break;

    case IDC_ACTIVE_AI:
    {
      if(alter_dlg->curr_ent && alter_dlg->curr_ent->get_flavor() == ENTITY_CHARACTER && ((character *)alter_dlg->curr_ent)->get_brain())
      {
        brain *brn = ((character *)alter_dlg->curr_ent)->get_brain();

        int x = get_combo_index(IDC_ACTIVE_AI);

        if(x == 0)
          brn->set_ai_state(BRAIN_REACT_COMBAT, BRAIN_AI_STATE_NONE);
        else
          brn->set_ai_state(BRAIN_REACT_COMBAT, (eAIState)(x+BRAIN_AI_STATE_REACT_COMBAT));

        update();
      }
    }
    break;

    default:
      Dialog::handle_selection_change(ctrl);
      break;
  }
}


void CharacterDialog::handle_command(int ctrl)
{
	switch(ctrl)
	{
    case IDCANCEL:
      break;

    default:
      Dialog::handle_command(ctrl);
      break;
  }
}


!*/




AlterDialog::AlterDialog(HWND pParent)
{
	init(IDD_ALTER, gAlterDialogFunc, pParent);

  rot_dlg = NEW RotateDialog(NULL);
  trans_dlg = NEW TranslateDialog(NULL);
  scanner_dlg = NEW ScannerDialog(NULL);
  particle_dlg = NEW ParticleDialog(NULL);
  item_dlg = NEW ItemDialog(NULL);
  entity_dlg = NEW EntityDialog(NULL);
  destroy_dlg = NEW DestroyDialog(NULL);
  material_dlg = NEW MaterialDialog(NULL);
  marker_dlg = NEW MarkerDialog(NULL);
//!  character_dlg = NEW CharacterDialog(NULL);

  rot_dlg->alter_dlg = this;
  trans_dlg->alter_dlg = this;
  scanner_dlg->alter_dlg = this;
  particle_dlg->alter_dlg = this;
  item_dlg->alter_dlg = this;
  entity_dlg->alter_dlg = this;
  destroy_dlg->alter_dlg = this;
  material_dlg->alter_dlg = this;
  marker_dlg->alter_dlg = this;
//!  character_dlg->alter_dlg = this;

  curr_ent = NULL;
}

AlterDialog::~AlterDialog()
{
  dump_dialogs();
}

void AlterDialog::dump_dialogs()
{
  _DELETE_NULLIFY(rot_dlg);
  _DELETE_NULLIFY(trans_dlg);
  _DELETE_NULLIFY(scanner_dlg);
  _DELETE_NULLIFY(particle_dlg);
  _DELETE_NULLIFY(item_dlg);
  _DELETE_NULLIFY(entity_dlg);
  _DELETE_NULLIFY(destroy_dlg);
  _DELETE_NULLIFY(material_dlg);
  _DELETE_NULLIFY(marker_dlg);
//!  _DELETE_NULLIFY(character_dlg);
}

void AlterDialog::setup()
{
  init_tab_control(IDC_ALTERTABS, 9);

  insert_tab_control_item(IDC_ALTERTABS, 0, trans_dlg, _TABCONTROL_SIZE_DIALOG | _TABCONTROL_CENTER_DIALOG, "Translate");
  insert_tab_control_item(IDC_ALTERTABS, 1, rot_dlg, _TABCONTROL_SIZE_DIALOG | _TABCONTROL_CENTER_DIALOG, "Rotate");
  insert_tab_control_item(IDC_ALTERTABS, 2, entity_dlg, _TABCONTROL_SIZE_DIALOG | _TABCONTROL_CENTER_DIALOG, "Entity");
//!  insert_tab_control_item(IDC_ALTERTABS, 3, character_dlg, _TABCONTROL_SIZE_DIALOG | _TABCONTROL_CENTER_DIALOG, "Character");
  insert_tab_control_item(IDC_ALTERTABS, 3, scanner_dlg, _TABCONTROL_SIZE_DIALOG | _TABCONTROL_CENTER_DIALOG, "Scanner");
  insert_tab_control_item(IDC_ALTERTABS, 4, item_dlg, _TABCONTROL_SIZE_DIALOG | _TABCONTROL_CENTER_DIALOG, "Item");
  insert_tab_control_item(IDC_ALTERTABS, 5, destroy_dlg, _TABCONTROL_SIZE_DIALOG | _TABCONTROL_CENTER_DIALOG, "Destroy");
  insert_tab_control_item(IDC_ALTERTABS, 6, material_dlg, _TABCONTROL_SIZE_DIALOG | _TABCONTROL_CENTER_DIALOG, "Material");
  insert_tab_control_item(IDC_ALTERTABS, 7, marker_dlg, _TABCONTROL_SIZE_DIALOG | _TABCONTROL_CENTER_DIALOG, "Marker");
  insert_tab_control_item(IDC_ALTERTABS, 8, particle_dlg, _TABCONTROL_SIZE_DIALOG | _TABCONTROL_CENTER_DIALOG, "Particle");

  set_tab_control_index(IDC_ALTERTABS, 0);

  populate_entities();

	update();
}

void AlterDialog::hide()
{
	Dialog::hide();
}

void AlterDialog::update()
{
	if(!is_visible())
		return;

  if(curr_ent != g_selected_entity || curr_ent == NULL)
    populate_entities();

  if(curr_ent == NULL)
    disable_all_ctrl();
  else
    enable_all_ctrl();

  rot_dlg->update();
  trans_dlg->update();
  scanner_dlg->update();
  particle_dlg->update();
  item_dlg->update();
  entity_dlg->update();
  destroy_dlg->update();
  material_dlg->update();
  marker_dlg->update();
//!  character_dlg->update();

	Dialog::update();
}

void AlterDialog::render()
{
	if(!is_visible())
		return;

  if(curr_ent != NULL)
    render_beam_cube( curr_ent->get_abs_position(), curr_ent->get_radius() > 0.25f ? curr_ent->get_radius() : 0.5f, color32(255, 0, 0, 128), 0.05f );

  rot_dlg->render();
  trans_dlg->render();
  scanner_dlg->render();
  particle_dlg->render();
  item_dlg->render();
  entity_dlg->render();
  destroy_dlg->render();
  material_dlg->render();
  marker_dlg->render();
//!  character_dlg->render();
}

void AlterDialog::frame_advance(time_value_t t)
{
	if(!is_visible())
		return;
}

void AlterDialog::populate_entities()
{
  clear_combo(IDC_ENTITIES);

  vector<entity*>::const_iterator i = g_world_ptr->get_entities().begin();
  vector<entity*>::const_iterator i_end = g_world_ptr->get_entities().end();

  while(i != i_end)
  {
    if(*i && (*i) != g_world_ptr->get_hero_ptr() /*&& (*i)->from_sin_file()*/ && !(*i)->is_conglom_member() && !(*i)->is_time_limited()
      && ((*i)->get_flavor() != ENTITY_PARTICLE_GENERATOR || (*i)->from_sin_file())
      && ((*i)->get_flavor() != ENTITY_LIGHT_SOURCE || (*i)->was_spawned())
      && ((*i)->get_flavor() != ENTITY_LIGHT || (*i)->was_spawned())
//      && (*i)->get_flavor() != ENTITY_PROJECTILE
      && (*i)->get_flavor() != ENTITY_CAMERA
//!      && (*i)->get_flavor() != ENTITY_LIMB_BODY
      && (*i)->get_flavor() != ENTITY_PHYSICAL
      && (*i)->get_flavor() != ENTITY_MIC
      && (*i)->get_flavor() != ENTITY_MOBILE
      )
    {
      if(strnicmp((*i)->get_name().c_str(), "_ENTID_", 7) || (*i)->was_spawned())
        add_combo_string(IDC_ENTITIES, "%s", (*i)->get_name().c_str());
    }

    i++;
  }

//*
  vector<item*>::const_iterator i2 = g_world_ptr->get_items().begin();
  vector<item*>::const_iterator i2_end = g_world_ptr->get_items().end();

  while(i2 != i2_end)
  {
    if(*i2 && !(*i2)->is_picked_up() /*&& (*i2)->is_visible() && (*i2)->from_sin_file()*/ && !(*i2)->is_time_limited())
    {
      if(strnicmp(((entity *)(*i2))->get_name().c_str(), "_ENTID_", 7) || (*i2)->was_spawned())
        add_combo_string(IDC_ENTITIES, "%s", ((entity *)(*i2))->get_name().c_str());
    }

    i2++;
  }
//*/

  set_combo_index(IDC_ENTITIES, 0);

  char name[128] = "";

  if(g_selected_entity)
  {
    for(int x=0; x<get_combo_count(IDC_ENTITIES); x++)
    {
      get_combo_text(IDC_ENTITIES, x, name);

      if(!stricmp(g_selected_entity->get_name().c_str(), name))
      {
        set_combo_index(IDC_ENTITIES, x);
        break;
      }
    }
  }

  get_combo_text(IDC_ENTITIES, get_combo_index(IDC_ENTITIES), name);
  g_selected_entity = curr_ent = g_world_ptr->get_entity(name);
}

void AlterDialog::handle_selection_change(int ctrl)
{
  switch(ctrl)
  {
    case IDC_ENTITIES:
    {
      char name[128] = "";
      get_combo_text(IDC_ENTITIES, get_combo_index(IDC_ENTITIES), name);
      g_selected_entity = curr_ent = g_world_ptr->get_entity(name);

    	update();
    }
    break;

    default:
      Dialog::handle_selection_change(ctrl);
      break;
  }
}

void AlterDialog::handle_command(int ctrl)
{
	switch(ctrl)
	{
    case IDCANCEL:
      break;

    case IDC_REMOVE_ENTITY:
    {
      if(curr_ent)
      {
        SpawnCommand::del_ent(curr_ent);

        populate_entities();
        update();
      }
    }
    break;

    case IDC_REFRESH_ENTITIES:
    {
      populate_entities();
      update();
    }
    break;

    default:
      Dialog::handle_command(ctrl);
      break;
  }
}

void AlterDialog::handle_user_mouse_down(int button, vector3d pos, DWORD flags)
{
	if(!is_visible())
		return;

  if(button == 0 && (flags & MK_CONTROL))
  {
    vector3d origin, dest, norm, hit;
    POINT pt = get_client_mouse_position(g_hwnd);
    get_screen_to_world_segment(vector2di(pt.x, pt.y), origin, dest, norm);

    vector3d delta = (dest - origin);
    rational_t min_d = delta.length() + 1000.0f;

    vector<region_node*>      regs;
    if ( app::inst()->get_game()->get_current_view_camera()->get_region() )
      build_region_list( &regs, app::inst()->get_game()->get_current_view_camera()->get_region(), origin, delta );

    for( vector<region_node*>::const_iterator ri = regs.begin(); ri != regs.end(); ri++ )
    {
      vector<entity*>::const_iterator i = (*ri)->get_data()->get_entities().begin();
      vector<entity*>::const_iterator i_end = (*ri)->get_data()->get_entities().end();

      for ( ; i<i_end; i++ )
      {
        entity* e = *i;
        if ( e )
        {
          if(e != g_world_ptr->get_hero_ptr() && !e->is_conglom_member()
            && (e->get_flavor() != ENTITY_PARTICLE_GENERATOR || e->from_sin_file())
            && (e->get_flavor() != ENTITY_LIGHT_SOURCE || e->was_spawned())
            && (e->get_flavor() != ENTITY_LIGHT || e->was_spawned())
//            && e->get_flavor() != ENTITY_PROJECTILE
            && e->get_flavor() != ENTITY_CAMERA
//!            && e->get_flavor() != ENTITY_LIMB_BODY
            && e->get_flavor() != ENTITY_PHYSICAL
            && e->get_flavor() != ENTITY_MIC
            && e->get_flavor() != ENTITY_MOBILE
            && (strnicmp(e->get_name().c_str(), "_ENTID_", 7) || e->was_spawned())
            && e->is_visible()
            )
          {
            if(collide_segment_entity(origin, dest, e, &hit, &norm, 0.5f))
            {
              vector3d dv = e->get_abs_position() - origin;
//              vector3d dv = hit - origin;
              rational_t d = dv.length();

              if(d < min_d)
              {
                g_selected_entity = curr_ent = e;
                min_d = d;
              }
            }
          }
        }
      }

/*
      vector<item*>::const_iterator i2 = g_world_ptr->get_items().begin();
      vector<item*>::const_iterator i2_end = g_world_ptr->get_items().end();

      while(i2 != i2_end)
      {
        if(*i2 && !(*i2)->is_picked_up() && (strnicmp(((entity *)(*i2))->get_name().c_str(), "_ENTID_", 7) || (*i2)->was_spawned()))
        {
          entity *e = *i2;

          if(collide_segment_entity(origin, dest, e, &hit, &norm, 0.5f))
          {
            vector3d dv = hit - origin;
            rational_t d = dv.length();

            if(d < min_d)
            {
              g_selected_entity = curr_ent = e;
              min_d = d;
            }
          }
        }

        i2++;
      }
*/
    }

    if(curr_ent)
    {
      char name[128] = "";
      for(int x=0; x<get_combo_count(IDC_ENTITIES); x++)
      {
        get_combo_text(IDC_ENTITIES, x, name);

        if(!stricmp(curr_ent->get_name().c_str(), name))
        {
          set_combo_index(IDC_ENTITIES, x);
          break;
        }
      }
    }

    update();
  }
  else
  {
    rot_dlg->handle_user_mouse_down(button, pos, flags);
    trans_dlg->handle_user_mouse_down(button, pos, flags);
    scanner_dlg->handle_user_mouse_down(button, pos, flags);
    particle_dlg->handle_user_mouse_down(button, pos, flags);
    item_dlg->handle_user_mouse_down(button, pos, flags);
    entity_dlg->handle_user_mouse_down(button, pos, flags);
    destroy_dlg->handle_user_mouse_down(button, pos, flags);
    material_dlg->handle_user_mouse_down(button, pos, flags);
    marker_dlg->handle_user_mouse_down(button, pos, flags);
//!    character_dlg->handle_user_mouse_down(button, pos, flags);
  }
}

void AlterDialog::handle_user_mouse_up(int button, vector3d pos, DWORD flags)
{
	if(!is_visible())
		return;

  if(button == 0)
  {
  }
  else
  {
    rot_dlg->handle_user_mouse_up(button, pos, flags);
    trans_dlg->handle_user_mouse_up(button, pos, flags);
    scanner_dlg->handle_user_mouse_up(button, pos, flags);
    particle_dlg->handle_user_mouse_up(button, pos, flags);
    item_dlg->handle_user_mouse_up(button, pos, flags);
    entity_dlg->handle_user_mouse_up(button, pos, flags);
    destroy_dlg->handle_user_mouse_up(button, pos, flags);
    material_dlg->handle_user_mouse_up(button, pos, flags);
    marker_dlg->handle_user_mouse_up(button, pos, flags);
//!    character_dlg->handle_user_mouse_up(button, pos, flags);
  }
}










GroupDialog::GroupDialog(HWND pParent)
{
	init(IDD_GROUP, gGroupDialogFunc, pParent);
}

GroupDialog::~GroupDialog()
{
}

void GroupDialog::populate_groups()
{
  clear_combo(IDC_GROUPS);
STUBBED(GroupDialog_populate_groups, "GroupDialog::populate_groups");
/*!  char_group_manager::_M::iterator i = char_group_manager::inst()->begin();
  char_group_manager::_M::iterator i_end = char_group_manager::inst()->end();

  while(i != i_end)
  {
    if((*i).second && stricmp((*i).second->get_id().get_val().c_str(), "GROUP_ALL"))
      add_combo_string(IDC_GROUPS, "%s", (*i).second->get_id().get_val().c_str());

    i++;
  }
!*/
  set_combo_index(IDC_GROUPS, 0);
}

void GroupDialog::populate_members()
{
  char name[128] = "";

  get_ctrl_text(IDC_GROUPS, name, 127);
  str_toupper(name);
STUBBED(GroupDialog_populate_members, "GroupDialog::populate_members");
/*!  char_group *grp = char_group_manager::inst()->find(char_group_id(name), char_group_manager::FAIL_OK);

  clear_list(IDC_MEMBERS);

  if(grp)
  {
    char_group::_V::iterator c = grp->begin();
    char_group::_V::iterator c_end = grp->end();

    while(c != c_end)
    {
      if(*c)
        add_list_string(IDC_MEMBERS, "%s", (*c)->get_name().c_str());

      c++;
    }
  }
!*/
  if(get_list_count(IDC_MEMBERS) > 0)
    enable_ctrl(IDC_REMOVE_MEMBER);
  else
    disable_ctrl(IDC_REMOVE_MEMBER);
}

void GroupDialog::populate_characters()
{
  char name[128] = "";

  get_ctrl_text(IDC_GROUPS, name, 127);
  str_toupper(name);
STUBBED(GroupDialog_populate_characters, "GroupDialog::populate_characters");
/*!  char_group *grp = char_group_manager::inst()->find(char_group_id(name), char_group_manager::FAIL_OK);

  clear_list(IDC_CHARACTERS);

  vector<character*>::const_iterator i = g_world_ptr->get_characters().begin();
  vector<character*>::const_iterator i_end = g_world_ptr->get_characters().end();

  while(i != i_end)
  {
    if(*i && stricmp((*i)->get_name().c_str(), "HERO") && (grp == NULL || !grp->find(*i)))
      add_list_string(IDC_CHARACTERS, "%s", (*i)->get_name().c_str());

    i++;
  }

  if(get_list_count(IDC_CHARACTERS) > 0)
    enable_ctrl(IDC_ADD_CHARACTER);
  else
    disable_ctrl(IDC_ADD_CHARACTER);
!*/
}

void GroupDialog::remove_group()
{
//  alert_dialog("Not implemented yet!");

  char name[128] = "";

  get_ctrl_text(IDC_GROUPS, name, 127);
  str_toupper(name);

  if(strlen(name) > 0)
  {
STUBBED(GroupDialog_remove_group, "GroupDialog::remove_group");
/*!    char_group *grp = char_group_manager::inst()->find(char_group_id(name), char_group_manager::FAIL_OK);

    if(grp)
      char_group_manager::inst()->remove(grp);


    populate_groups();
    populate_members();
    populate_characters();
!*/
  	update();
  }
}

void GroupDialog::remove_members()
{
  char name[128] = "";

  get_ctrl_text(IDC_GROUPS, name, 127);
  str_toupper(name);

  if(strlen(name) > 0)
  {
STUBBED(GroupDialog_populate_members, "GroupDialog::remove_members");
/*!    char_group *grp = char_group_manager::inst()->find(char_group_id(name), char_group_manager::FAIL_OK);

    if(grp)
    {
      int num = get_list_multiple_count(IDC_MEMBERS);
      int *sel = NEW int[num+1];

      get_list_multiple_selection(IDC_MEMBERS, sel, num);

      char buf[128]="";
      for(int i=0; i<num; i++)
      {
        get_list_text(IDC_MEMBERS, sel[i], buf);
//        str_toupper(buf);

        char_group::_V::iterator c = grp->begin();
        char_group::_V::iterator c_end = grp->end();

        char_group::relpos_list::iterator r = grp->formation.begin();
        char_group::relpos_list::iterator r_end = grp->formation.end();

        while(c != c_end && r != r_end)
        {
          if((*c) && !stricmp((*c)->get_name().c_str(), buf))
          {
            c = grp->erase(c);
            r = grp->formation.erase(r);

            break;
          }
          else
          {
            c++;
            r++;
          }
        }
      }

      delete []sel;

      populate_groups();
      set_ctrl_text(IDC_GROUPS, "%s", name);

      populate_members();
      populate_characters();

	    update();
    }
!*/
  }
}
/*!
void GroupDialog::add_characters()
{
  char name[128] = "";

  get_ctrl_text(IDC_GROUPS, name, 127);
  str_toupper(name);

  if(strlen(name) > 0)
  {

    char_group *grp = char_group_manager::inst()->find(char_group_id(name), char_group_manager::FAIL_OK);

    int num = get_list_multiple_count(IDC_CHARACTERS);
    int *sel = NEW int[num+1];

    get_list_multiple_selection(IDC_CHARACTERS, sel, num);

    char buf[128]="";
    for(int i=0; i<num; i++)
    {
      get_list_text(IDC_CHARACTERS, sel[i], buf);
//      str_toupper(buf);

      entity *ent = g_world_ptr->get_entity(buf);

      if(ent && ent->get_flavor() == ENTITY_CHARACTER && (!grp || !grp->find((character *)ent)))
      {
        if(!grp)
        {
          grp = NEW char_group(char_group_id(name));
          char_group_manager::inst()->add(grp);
        }

        grp->add((character *)ent);
      }
    }

    delete []sel;

    populate_groups();
    set_ctrl_text(IDC_GROUPS, "%s", name);

    populate_members();
    populate_characters();

	  update();
  }
}
!*/
void GroupDialog::setup()
{
  populate_groups();
  populate_members();
  populate_characters();

	update();
}

void GroupDialog::hide()
{
	Dialog::hide();
}

void GroupDialog::update()
{
	if(!is_visible())
		return;

	Dialog::update();
}

void GroupDialog::render()
{
	if(!is_visible())
		return;
}

void GroupDialog::frame_advance(time_value_t t)
{
	if(!is_visible())
		return;
}

void GroupDialog::handle_selection_change(int ctrl)
{
  switch(ctrl)
  {
    case IDC_GROUPS:
    {
      char buf[128]="";
      get_combo_text(IDC_GROUPS, get_combo_index(IDC_GROUPS), buf);
      set_ctrl_text(IDC_GROUPS, buf);

      populate_members();
      populate_characters();

    	update();
    }
    break;

    default:
      Dialog::handle_selection_change(ctrl);
      break;
  }
}


void GroupDialog::handle_command(int ctrl)
{
	switch(ctrl)
	{
    case IDCANCEL:
      break;

    case IDC_MEMBER_REFRESH:
    case IDC_CHARACTER_REFRESH:
    {
      populate_members();
      populate_characters();

    	update();
    }
    break;

    case IDC_REMOVE_GROUP:
    {
      remove_group();
    }
    break;

    case IDC_REMOVE_MEMBER:
    {
      remove_members();
    }
    break;

/*!    case IDC_ADD_CHARACTER:
    {
      add_characters();
    }
    break;
!*/
    default:
      Dialog::handle_command(ctrl);
      break;
  }
}













ContainerDialog::ContainerDialog(HWND pParent)
{
	init(IDD_CONTAINER, gContainerDialogFunc, pParent);
}

ContainerDialog::~ContainerDialog()
{
}

bool ContainerDialog::item_exists(stringx fname)
{
  char name[128]="";

  for(int i=1; i<get_list_count(IDC_ITEM_LIST); i++)
  {
    get_list_text(IDC_ITEM_LIST, i, name);

    if(fname == name)
      return(true);
  }

  return(false);
}

entity *ContainerDialog::get_curr_ent()
{
  char name[128] = "";
  entity *ent = NULL;
  get_combo_text(IDC_CONTAINERS, get_combo_index(IDC_CONTAINERS), name);

  if(strlen(name))
    ent = g_world_ptr->get_entity(name);

  return(ent);
}

void ContainerDialog::populate_containers()
{
  clear_combo(IDC_CONTAINERS);

  vector<entity*>::const_iterator i = g_world_ptr->get_entities().begin();
  vector<entity*>::const_iterator i_end = g_world_ptr->get_entities().end();

  while(i != i_end)
  {
/*!    if(*i && (*i) != g_world_ptr->get_hero_ptr() && ((*i)->get_flavor() == ENTITY_CHARACTER || (*i)->is_destroyable() || (*i)->is_container()))
    {
      if(strnicmp((*i)->get_name().c_str(), "_ENTID_", 7) || (*i)->was_spawned())
        add_combo_string(IDC_CONTAINERS, "%s", (*i)->get_name().c_str());
    }
!*/
    i++;
  }

  set_combo_index(IDC_CONTAINERS, 0);

  char name[128] = "";

  if(g_selected_entity)
  {
    for(int x=0; x<get_combo_count(IDC_CONTAINERS); x++)
    {
      get_combo_text(IDC_CONTAINERS, x, name);

      if(!stricmp(g_selected_entity->get_name().c_str(), name))
      {
        set_combo_index(IDC_CONTAINERS, x);
        break;
      }
    }
  }
}

void ContainerDialog::populate_items()
{
  clear_combo(IDC_ITEMS);

  entfile_map::const_iterator fi = g_world_ptr->get_entfiles().begin();
  entfile_map::const_iterator fi_end = g_world_ptr->get_entfiles().end();

  while(fi != fi_end)
  {
    if((*fi).second->get_flavor() == ENTITY_ITEM)
    {
      stringx name = (*fi).first;
      name.to_lower();

      if(!item_exists(name))
        add_combo_string(IDC_ITEMS, "%s", name.c_str());
    }

    fi++;
  }

  set_combo_index(IDC_ITEMS, 0);
}

int ContainerDialog::get_item_index(entity *ent, int list_index)
{
  if(ent)
  {
    int count = 0;

    for(int i=0; i<ent->get_num_items(); i++)
    {
      item *itm = ent->get_item(i);
      if(itm &&  itm->get_original_count() > 0)
        count++;

      if(count == list_index)
        return(i);
    }
  }

  return(-1);
}

void ContainerDialog::populate_item_list()
{
  clear_list(IDC_ITEM_LIST);
  add_list_string(IDC_ITEM_LIST, "(New Item)");

  entity *ent = get_curr_ent();

  if(ent != NULL)
  {
    for(int i=0; i<ent->get_num_items(); i++)
    {
      item *itm = ent->get_item(i);
      if(itm &&  itm->get_original_count() > 0)
        add_list_string(IDC_ITEM_LIST, "%s (%d)", itm->ent_filename.c_str(), itm->get_original_count());
    }
  }

  set_list_index(IDC_ITEM_LIST, 0);
  handle_selection_change(IDC_ITEM_LIST);
}

void ContainerDialog::remove_item()
{
  int index = get_list_index(IDC_ITEM_LIST);

  if(index != 0)
  {
    entity *ent = get_curr_ent();

    int itm_index = -1;

    if(ent && (itm_index = get_item_index(ent, index)) != -1 && ent->get_item(itm_index))
    {
      ent->get_item(itm_index)->set_original_count(0);
      ent->get_item(itm_index)->set_count(0);
    }

    populate_item_list();
  }
}

void ContainerDialog::add_item()
{
  entity *ent = get_curr_ent();

  if(ent)
  {
    SET_CURSOR(IDC_WAIT);

    bool filelock = os_file::is_system_locked();
    os_file::system_unlock();

    char ent_name[128]="";
    get_ctrl_text(IDC_ITEMS, ent_name, 127);

    if(strlen(ent_name))
    {
      entity *itm = NULL;
      if(!(g_file_finder->find_file( ent_name, ".ent", true ) == ""))
        itm = g_entity_maker->create_entity_or_subclass(ent_name, entity_id::make_unique_id(), po_identity_matrix, "", ACTIVE_FLAG | NONSTATIC_FLAG);

      if ( filelock )
        os_file::system_lock();

      SET_CURSOR(IDC_ARROW);

      if(itm && itm->get_flavor() == ENTITY_ITEM)
      {
        entity *ent = get_curr_ent();

        int cnt = get_ctrl_int(IDC_COUNT);
        if(cnt <= 0)
          cnt = 1;

        if(ent)
        {
          ((item *)itm)->set_count(cnt);
          ((item *)itm)->set_original_count(cnt);

          ent->entity::add_item(((item *)itm));

          populate_item_list();
        }
      }
      else if(itm)
      {
        alert_dialog("Entity '%s' is not an item!", ent_name);
        SpawnCommand::del_ent(itm);
      }
      else
        alert_dialog("could not load entity '%s'!", ent_name);
    }
  }
}

void ContainerDialog::update_item()
{
  int index = get_list_index(IDC_ITEM_LIST);

  entity *ent = get_curr_ent();

  int cnt = get_ctrl_int(IDC_COUNT);
  if(cnt <= 0)
    cnt = 1;

  int itm_index = -1;

  if(ent && (itm_index = get_item_index(ent, index)) != -1 && ent->get_item(itm_index))
  {
    ent->get_item(itm_index)->set_original_count(cnt);
    ent->get_item(itm_index)->set_count(cnt);
  }

  populate_item_list();
  set_list_index(IDC_ITEM_LIST, index);
  handle_selection_change(IDC_ITEM_LIST);
}

void ContainerDialog::setup()
{
  set_spinner_buddy(IDC_COUNT_SPIN, IDC_COUNT);
  set_spinner_range(IDC_COUNT_SPIN, 1, 32767);

  populate_containers();
  populate_item_list();
  populate_items();

	update();
}

void ContainerDialog::hide()
{
	Dialog::hide();
}

void ContainerDialog::update()
{
	if(!is_visible())
		return;

	Dialog::update();
}

void ContainerDialog::render()
{
	if(!is_visible())
		return;
}


void ContainerDialog::frame_advance(time_value_t t)
{
	if(!is_visible())
		return;
}

void ContainerDialog::handle_slider(int ctrl, int pos)
{
	switch(ctrl)
	{
    case IDC_COUNT_SPIN:
    {
      int index = get_list_index(IDC_ITEM_LIST);

      if(index != 0 && pos > 0)
      {
        entity *ent = get_curr_ent();

        int itm_index = -1;

        if(ent && (itm_index = get_item_index(ent, index)) != -1 && ent->get_item(itm_index))
        {
          ent->get_item(itm_index)->set_original_count(pos);
          ent->get_item(itm_index)->set_count(pos);
        }

        populate_item_list();
        set_list_index(IDC_ITEM_LIST, index);
        handle_selection_change(IDC_ITEM_LIST);
      }
    }
    break;

    default:
      Dialog::handle_slider(ctrl, pos);
      break;
  }
}

void ContainerDialog::handle_selection_change(int ctrl)
{
  switch(ctrl)
  {
    case IDC_CONTAINERS:
    {
      char name[128] = "";
      get_combo_text(IDC_CONTAINERS, get_combo_index(IDC_CONTAINERS), name);
      g_selected_entity = g_world_ptr->get_entity(name);

      populate_item_list();

    	update();
    }
    break;

    case IDC_ITEM_LIST:
    {
      int index = get_list_index(IDC_ITEM_LIST);

      enable_ctrl(IDC_ITEMS, (index == 0));
      enable_ctrl(IDC_DELETE, (index != 0));

      if(index == 0)
      {
        set_combo_index(IDC_ITEMS, 0);

        set_ctrl_text(IDC_APPLY, "Add Item");

        set_ctrl_text(IDC_COUNT, "%d", 1);
        set_spinner_pos(IDC_COUNT_SPIN, 1);
      }
      else
      {
        entity *ent = get_curr_ent();

        int itm_index = -1;

        if(ent && (itm_index = get_item_index(ent, index)) != -1 && ent->get_item(itm_index))
        {
          set_ctrl_text(IDC_ITEMS, "%s", ent->get_item(itm_index)->ent_filename.c_str());
          set_ctrl_text(IDC_COUNT, "%d", ent->get_item(itm_index)->get_original_count());

          set_spinner_pos(IDC_COUNT_SPIN, ent->get_item(itm_index)->get_original_count());
        }

        set_ctrl_text(IDC_APPLY, "Update Item");
      }

    	update();
    }
    break;

    default:
      Dialog::handle_selection_change(ctrl);
      break;
  }
}


void ContainerDialog::handle_command(int ctrl)
{
	switch(ctrl)
	{
    case IDCANCEL:
      break;

    case IDC_APPLY:
    {
      int index = get_list_index(IDC_ITEM_LIST);

      if(index == 0)
        add_item();
      else
        update_item();
    }
    break;

    case IDC_DELETE:
    {
      remove_item();
    }
    break;

    default:
      Dialog::handle_command(ctrl);
      break;
  }
}

void export_containers(ofstream &fout)
{
  if(fout.is_open())
  {
    vector<entity*>::const_iterator i = g_world_ptr->get_entities().begin();
    vector<entity*>::const_iterator i_end = g_world_ptr->get_entities().end();

    int cnt = 0;

    while(i != i_end)
    {
      if(*i && (*i) != g_world_ptr->get_hero_ptr() && (*i)->is_container())
      {
        entity *ent = (*i);
        for(int x=0; x<ent->get_num_items(); x++)
        {
          item *itm = ent->get_item(x);

          if(itm && itm->get_original_count() > 0)
          {
            fout<<"  CONTAINER_";

            if(cnt < 10)
              fout<<"00";
            else if(cnt < 100)
              fout<<"0";

            fout<<cnt<<" container "<<ent->get_name().c_str()<<" "<<itm->ent_filename.c_str()<<" "<<itm->get_original_count()<<"\n";

            cnt++;
          }
        }
      }

      i++;
    }
  }
}









ExportDialog::ExportDialog(HWND pParent)
{
	init(IDD_EXPORT, gExportDialogFunc, pParent);

  path[0] = '\0';
  file[0] = '\0';

  exp_level = false;
  exp_sin_ents = false;
  exp_spawn_ents = false;
  exp_scanners = false;
  exp_items = false;
  exp_char_groups = false;
  exp_containers = false;
  exp_material_groups = false;
  exp_paths = false;
  exp_brains = false;
}

ExportDialog::~ExportDialog()
{
}

void ExportDialog::setup()
{
  set_check_box(IDC_EXP_LEVEL, exp_level);
  set_check_box(IDC_EXP_ENTITIES, exp_sin_ents);
  set_check_box(IDC_EXP_SPAWN, exp_spawn_ents);
  set_check_box(IDC_EXP_SCANNERS, exp_scanners);
  set_check_box(IDC_EXP_ITEMS, exp_items);
  set_check_box(IDC_EXP_GROUPS, exp_char_groups);
  set_check_box(IDC_EXP_CONTAINERS, exp_containers);
  set_check_box(IDC_EXP_MATERIAL_GROUPS, exp_material_groups);
  set_check_box(IDC_EXP_PATHS, exp_paths);
  set_check_box(IDC_EXP_BRAIN_INFO, exp_brains);


	update();
}

void ExportDialog::hide()
{
  exp_level = get_check_box(IDC_EXP_LEVEL);
  exp_sin_ents = get_check_box(IDC_EXP_ENTITIES);
  exp_spawn_ents = get_check_box(IDC_EXP_SPAWN);
  exp_scanners = get_check_box(IDC_EXP_SCANNERS);
  exp_items = get_check_box(IDC_EXP_ITEMS);
  exp_char_groups = get_check_box(IDC_EXP_GROUPS);
  exp_containers = get_check_box(IDC_EXP_CONTAINERS);
  exp_material_groups = get_check_box(IDC_EXP_MATERIAL_GROUPS);
  exp_paths = get_check_box(IDC_EXP_PATHS);
  exp_brains = get_check_box(IDC_EXP_BRAIN_INFO);

	Dialog::hide();
}

void ExportDialog::update()
{
	if(!is_visible())
		return;

  enable_all_ctrl(!exp_level);
  enable_ctrl(IDC_EXP_SPAWN, !exp_sin_ents && !exp_level);

  enable_ctrl(IDC_EXPORT);
  enable_ctrl(IDC_EXP_LEVEL);
  enable_ctrl(IDC_STATIC_CATEGORIES);

	Dialog::update();
}

void ExportDialog::render()
{
	if(!is_visible())
		return;
}

void ExportDialog::frame_advance(time_value_t t)
{
	if(!is_visible())
		return;
}

extern bool tool_file_exists(const char *file);


void export_entity_info(ofstream &fout)
{
  if(fout.is_open())
  {
    vector<entity *>::const_iterator i = g_world_ptr->get_entities().begin();
    vector<entity *>::const_iterator i_end = g_world_ptr->get_entities().end();

    while(i != i_end)
    {
      entity *ent = (*i);

      if(ent && ent->needs_export() && (strnicmp(ent->get_name().c_str(), "_ENTID_", 7) || ent->was_spawned()))
        write_entity_info(fout, ent);

      i++;
    }

    fout<<"\n";
  }
}





void export_sin_entities(ofstream &fout)
{
  if(fout.is_open())
  {
    vector<entity *>::const_iterator i = g_world_ptr->get_entities().begin();
    vector<entity *>::const_iterator i_end = g_world_ptr->get_entities().end();

    while(i != i_end)
    {
      entity *ent = (*i);

      if(ent && ent->from_sin_file() && (strnicmp(ent->get_name().c_str(), "_ENTID_", 7) || ent->was_spawned()))
        write_entity(fout, ent);

      i++;
    }

    fout<<"\n";

    vector<item *>::const_iterator i2 = g_world_ptr->get_items().begin();
    vector<item *>::const_iterator i2_end = g_world_ptr->get_items().end();

    while(i2 != i2_end)
    {
      item *itm = (*i2);

      if(itm && itm->from_sin_file() && (strnicmp(itm->get_name().c_str(), "_ENTID_", 7) || itm->was_spawned()))
        write_entity(fout, itm);

      i2++;
    }
STUBBED(export_sin_entities, "export_sin_entities hero item dump");
/*!    entity *hero = g_world_ptr->get_hero_ptr();
    if(hero)
    {
      for(int i=0; i<hero->get_num_items(); i++)
      {
        item *itm = hero->get_item(i);
        if(itm && itm->from_sin_file() && (strnicmp(itm->get_name().c_str(), "_ENTID_", 7) || itm->was_spawned()))
          write_entity(fout, itm);
      }
    }
!*/
  }
}

void export_item_info(ofstream &fout)
{
  if(fout.is_open())
  {
    vector<item *>::const_iterator i2 = g_world_ptr->get_items().begin();
    vector<item *>::const_iterator i2_end = g_world_ptr->get_items().end();

    while(i2 != i2_end)
    {
      item *itm = (*i2);

      if(itm && itm->from_sin_file() && (strnicmp(itm->get_name().c_str(), "_ENTID_", 7) || itm->was_spawned()))
        write_item_info(fout, itm);

      i2++;
    }
STUBBED(export_item_info, "export_item_info item dump");

/*!    entity *hero = g_world_ptr->get_hero_ptr();
    if(hero)
    {
      for(int i=0; i<hero->get_num_items(); i++)
      {
        item *itm = hero->get_item(i);
        if(itm && itm->from_sin_file() && (strnicmp(itm->get_name().c_str(), "_ENTID_", 7) || itm->was_spawned()))
          write_item_info(fout, itm);
      }
    }
!*/
  }
}

bool ExportDialog::export()
{
  if(save_file_dialog(file, path, "sin|txt|sin_exp|*", "Max Steel Sin File|Text File|Max Steel Exported Sin Info|All Files", "Export Sin file Info"))
  {
    char fullpath[_MAX_PATH+1] = "";
    sprintf(fullpath, "%s\\%s", path, file);

    if(!tool_file_exists(fullpath) || yes_no_dialog("Overwrite File?", "File '%s' exists!\n\nOverwrite?", fullpath))
    {
      ofstream fout;

      fout.open(fullpath, ios::out | ios::trunc);
      if(!fout.fail())
      {
        fout<<"entities:\n";
        if(get_check_box(IDC_EXP_ENTITIES) || get_check_box(IDC_EXP_LEVEL))
        {
          fout<<"\n";
          export_sin_entities(fout);
        }
        else if(get_check_box(IDC_EXP_SPAWN))
        {
          fout<<"\n";
          SpawnCommand::export_spawned_entities(fout);
        }
        fout<<"\nchunkend\n";

        fout<<"\n\n\n";

        fout<<"entity_info:\n";
        if(get_check_box(IDC_EXP_BRAIN_INFO) || get_check_box(IDC_EXP_LEVEL))
        {
          fout<<"\n";
//!          CharacterDialog::export_brain_info(fout);
        }
        if(get_check_box(IDC_EXP_SCANNERS) || get_check_box(IDC_EXP_LEVEL))
        {
          fout<<"\n";
          ScannerCommand::export_scanners(fout);
        }
        if(get_check_box(IDC_EXP_ITEMS) || get_check_box(IDC_EXP_LEVEL))
        {
          fout<<"\n";
          export_item_info(fout);
        }
        if(get_check_box(IDC_EXP_ENTITIES) || get_check_box(IDC_EXP_LEVEL))
        {
          fout<<"\n";
          export_entity_info(fout);
        }
        else if(get_check_box(IDC_EXP_SPAWN))
        {
          fout<<"\n";
          SpawnCommand::export_spawned_entity_info(fout);
        }
        fout<<"\nchunkend\n";

        fout<<"\n\n\n";




        fout<<"maps:\n";
        fout<<"\nchunkend\n";




        fout<<"\n\n\n";


        fout<<"char_groups:\n";
        if(get_check_box(IDC_EXP_GROUPS) || get_check_box(IDC_EXP_LEVEL))
        {
          fout<<"\n";
//!          CharGroupCommand::export_groups(fout);
        }
        fout<<"\nchunkend\n";

        fout<<"\n\n\n";




        fout<<"triggers:\n";
        fout<<"\nchunkend\n";




        fout<<"\n\n\n";





        fout<<"script_instances:\n";
        if(get_check_box(IDC_EXP_CONTAINERS) || get_check_box(IDC_EXP_LEVEL))
        {
          fout<<"\n";
          export_containers(fout);
        }
        fout<<"\nchunkend\n";




        fout<<"crate_groups:\n";
        fout<<"\nchunkend\n";




        fout<<"\n\n\n";





        fout<<"variants:\n";
        fout<<"\nchunkend\n";




        fout<<"\n\n\n";





        fout<<"material_groups:\n";
        if(get_check_box(IDC_EXP_MATERIAL_GROUPS) || get_check_box(IDC_EXP_LEVEL))
        {
          fout<<"\n";
          MaterialDialog::export_material_groups(fout);
        }
        fout<<"\nchunkend\n";



        fout<<"\n\n\n";



        fout<<"paths:\n";
        if(get_check_box(IDC_EXP_PATHS) || get_check_box(IDC_EXP_LEVEL))
        {
          for(int i=0; i<g_world_ptr->get_num_path_graphs(); i++)
          {
            path_graph *graph = g_world_ptr->get_path_graph(i);
            if(graph != NULL)
            {
              fout<<"\n";
              graph->write_data(fout);
            }
          }
        }
        fout<<"\nchunkend\n";



        fout.close();
      }
    }

    return(true);
  }

  return(false);
}

void ExportDialog::handle_command(int ctrl)
{
	switch(ctrl)
	{
    case IDCANCEL:
      break;

    case IDC_EXP_LEVEL:
    {
      exp_level = get_check_box(IDC_EXP_LEVEL);
      update();
    }
    break;

    case IDC_EXP_ENTITIES:
    {
      exp_sin_ents = get_check_box(IDC_EXP_ENTITIES);
      update();
    }
    break;

    case IDC_EXP_SPAWN:
    {
      exp_spawn_ents = get_check_box(IDC_EXP_SPAWN);
      update();
    }
    break;

    case IDC_EXP_SCANNERS:
    {
      exp_scanners = get_check_box(IDC_EXP_SCANNERS);
      update();
    }
    break;

    case IDC_EXP_ITEMS:
    {
      exp_items = get_check_box(IDC_EXP_ITEMS);
      update();
    }
    break;

    case IDC_EXP_GROUPS:
    {
      exp_char_groups = get_check_box(IDC_EXP_GROUPS);
      update();
    }
    break;

    case IDC_EXP_CONTAINERS:
    {
      exp_containers = get_check_box(IDC_EXP_CONTAINERS);
      update();
    }
    break;

    case IDC_EXP_MATERIAL_GROUPS:
    {
      exp_material_groups = get_check_box(IDC_EXP_MATERIAL_GROUPS);
      update();
    }
    break;

    case IDC_EXP_PATHS:
    {
      exp_paths = get_check_box(IDC_EXP_PATHS);
      update();
    }
    break;

    case IDC_EXP_BRAIN_INFO:
    {
      exp_brains = get_check_box(IDC_EXP_BRAIN_INFO);
      update();
    }
    break;

    case IDC_EXPORT:
    {
      export();
    }
    break;

    default:
      Dialog::handle_command(ctrl);
      break;
  }
}







ConsoleDialog::ConsoleDialog(HWND pParent)
{
	init(IDD_CONSOLE, gConsoleDialogFunc, pParent);
}

ConsoleDialog::~ConsoleDialog()
{
}

void ConsoleDialog::refresh_log()
{
  clear_list(IDC_LOG);

	list<stringx>::reverse_iterator node = g_console->log.rbegin();
	list<stringx>::reverse_iterator node_end = g_console->log.rend();
	while(node != node_end)
	{
    add_list_string(IDC_LOG, "%s", (*node).c_str());
    node++;
  }

  if(get_list_count(IDC_LOG) > 0)
    set_list_index(IDC_LOG, get_list_count(IDC_LOG)-1);
}

void ConsoleDialog::setup()
{
  refresh_log();

	update();
}

void ConsoleDialog::hide()
{
	Dialog::hide();
}

void ConsoleDialog::update()
{
	if(!is_visible())
		return;

	Dialog::update();
}

void ConsoleDialog::render()
{
	if(!is_visible())
		return;
}

void ConsoleDialog::frame_advance(time_value_t t)
{
	if(!is_visible())
		return;
}

bool ConsoleDialog::validate()
{
  char cmd[1024] = "";
  get_ctrl_text(IDC_COMMAND, cmd, 1023);

  return(strlen(cmd) > 0);
}

void ConsoleDialog::submit()
{
  char cmd[1024] = "";
  get_ctrl_text(IDC_COMMAND, cmd, 1023);

  g_console->processCommand(cmd);

  set_ctrl_text(IDC_COMMAND, "");

  refresh_log();
  update();
}


void ConsoleDialog::handle_command(int ctrl)
{
	switch(ctrl)
	{
    case IDCANCEL:
      break;

    case IDC_REFRESH:
    {
      refresh_log();
      update();
    }
    break;

    default:
      Dialog::handle_command(ctrl);
      break;
  }
}












PathsDialog::PathsDialog(HWND pParent)
{
  auto_edge = true;
  last_mode = _MODE_NODE;
  cur_graph = NULL;
  test_path.resize(0);
  selected_nodes[0] = NULL;
  selected_nodes[1] = NULL;

  init(IDD_PATHS, gPathsDialogFunc, pParent);
}

PathsDialog::~PathsDialog()
{
}

void PathsDialog::setup()
{
  if(g_target_fx)
    g_target_fx->set_visible(true);

  point_count = 0;
  selected_nodes[0] = NULL;
  selected_nodes[1] = NULL;

  test_path.resize(0);

  populate_paths();

	update();
}

void PathsDialog::hide()
{
  if(g_target_fx)
    g_target_fx->set_visible(false);

  auto_edge = get_check_box(IDC_AUTO_EDGE);

  test_path.resize(0);
  point_count = 0;

	Dialog::hide();
}

void PathsDialog::populate_paths()
{
  clear_combo(IDC_PATHS);

  for(int i=0; i<g_world_ptr->get_num_path_graphs(); i++)
  {
    assert(g_world_ptr->get_path_graph(i));

    add_combo_string(IDC_PATHS, g_world_ptr->get_path_graph(i)->get_id().c_str());
  }

  set_combo_index(IDC_PATHS, 0);

  if(g_world_ptr->get_num_path_graphs())
  {
    char name[128] = "";
    if(cur_graph)
    {
      for(int x=0; x<get_combo_count(IDC_PATHS); x++)
      {
        get_combo_text(IDC_PATHS, x, name);

        if(!stricmp(cur_graph->get_id().c_str(), name))
        {
          set_combo_index(IDC_PATHS, x);
          break;
        }
      }
    }

    get_combo_text(IDC_PATHS, get_combo_index(IDC_PATHS), name);
    cur_graph = g_world_ptr->get_path_graph(name);
  }
  else
    cur_graph = NULL;
}


void PathsDialog::update()
{
	if(!is_visible())
		return;

  hide_all_ctrl();
  show_ctrl(IDC_RENDER);
  show_ctrl(IDC_STATIC_MODE);
  show_ctrl(IDC_STATIC_PATH);
  show_ctrl(IDC_PATHS);
  show_ctrl(IDC_ADD_PATH);
  show_ctrl(IDC_DEL_PATH);
  show_ctrl(IDC_CLEAR);
  show_ctrl(IDC_RADIO_NODE);
  show_ctrl(IDC_RADIO_EDGE);
  show_ctrl(IDC_RADIO_TEST);

  enable_all_ctrl(cur_graph != NULL);
  enable_ctrl(IDC_STATIC_PATH);
  enable_ctrl(IDC_PATHS);
  enable_ctrl(IDC_ADD_PATH);

  switch(last_mode)
  {
    case _MODE_NODE:
    {
      set_ctrl_text(IDC_STATIC_MODE, "Node");
      check_radio_button(IDC_RADIO_NODE, IDC_RADIO_NODE);
      show_ctrl(IDC_AUTO_EDGE);
      show_ctrl(IDC_DEL_POINT);

      show_ctrl(IDC_STATIC_NODE_FLAGS);
      show_ctrl(IDC_NODE_DUCK);
      show_ctrl(IDC_NODE_COVER);
      show_ctrl(IDC_NODE_SEEK);
      show_ctrl(IDC_NODE_PATROL_DELAY);

      update_node_info();
    }
    break;

    case _MODE_EDGE:
    {
      set_ctrl_text(IDC_STATIC_MODE, "Edge");
      check_radio_button(IDC_RADIO_NODE, IDC_RADIO_EDGE);
      show_ctrl(IDC_ADD_EDGE);
      show_ctrl(IDC_DEL_EDGE);
      show_ctrl(IDC_WEIGHT_MOD);
      show_ctrl(IDC_STATIC_WEIGHT_MOD);
      show_ctrl(IDC_EDGE_APPLY);
      show_ctrl(IDC_EDGE_PATROL);

      show_ctrl(IDC_EDGE_PATROL_0);
      show_ctrl(IDC_EDGE_PATROL_1);
      show_ctrl(IDC_EDGE_PATROL_2);
      show_ctrl(IDC_EDGE_PATROL_3);
      show_ctrl(IDC_EDGE_PATROL_4);
      show_ctrl(IDC_EDGE_PATROL_5);
      show_ctrl(IDC_EDGE_PATROL_6);
      show_ctrl(IDC_EDGE_PATROL_7);

      show_ctrl(IDC_PERPETUATE_ALL_PATROL);
      show_ctrl(IDC_PERPETUATE_EDGE_PATROL);
      show_ctrl(IDC_STATIC_PATROL);

      update_edge_info();
    }
    break;

    case _MODE_TEST:
    {
      set_ctrl_text(IDC_STATIC_MODE, "Test");
      check_radio_button(IDC_RADIO_NODE, IDC_RADIO_TEST);
    }
    break;
  }

  set_check_box(IDC_AUTO_EDGE, auto_edge);

  set_check_box(IDC_RENDER, g_render_paths);

	Dialog::update();
}

void PathsDialog::update_edge_info()
{
  if(cur_graph && selected_nodes[0] != NULL && selected_nodes[1] != NULL && selected_nodes[0] != selected_nodes[1] && cur_graph->edge_in_graph(selected_nodes[0], selected_nodes[1]))
  {
    path_graph_edge *edge = cur_graph->get_edge(selected_nodes[0], selected_nodes[1]);

    enable_ctrl(IDC_STATIC_WEIGHT_MOD);
    enable_ctrl(IDC_WEIGHT_MOD);
    enable_ctrl(IDC_EDGE_APPLY);
    enable_ctrl(IDC_EDGE_PATROL);

    set_ctrl_float(IDC_WEIGHT_MOD, edge->weight_modifier);
    set_check_box(IDC_EDGE_PATROL, edge->is_patrol());

    enable_ctrl(IDC_EDGE_PATROL_0, edge->is_patrol());
    enable_ctrl(IDC_EDGE_PATROL_1, edge->is_patrol());
    enable_ctrl(IDC_EDGE_PATROL_2, edge->is_patrol());
    enable_ctrl(IDC_EDGE_PATROL_3, edge->is_patrol());
    enable_ctrl(IDC_EDGE_PATROL_4, edge->is_patrol());
    enable_ctrl(IDC_EDGE_PATROL_5, edge->is_patrol());
    enable_ctrl(IDC_EDGE_PATROL_6, edge->is_patrol());
    enable_ctrl(IDC_EDGE_PATROL_7, edge->is_patrol());

    set_check_box(IDC_EDGE_PATROL_0, edge->is_patrol_id(0));
    set_check_box(IDC_EDGE_PATROL_1, edge->is_patrol_id(1));
    set_check_box(IDC_EDGE_PATROL_2, edge->is_patrol_id(2));
    set_check_box(IDC_EDGE_PATROL_3, edge->is_patrol_id(3));
    set_check_box(IDC_EDGE_PATROL_4, edge->is_patrol_id(4));
    set_check_box(IDC_EDGE_PATROL_5, edge->is_patrol_id(5));
    set_check_box(IDC_EDGE_PATROL_6, edge->is_patrol_id(6));
    set_check_box(IDC_EDGE_PATROL_7, edge->is_patrol_id(7));

    enable_ctrl(IDC_PERPETUATE_EDGE_PATROL, edge->is_patrol());
  }
  else
  {
    disable_ctrl(IDC_STATIC_WEIGHT_MOD);
    disable_ctrl(IDC_WEIGHT_MOD);
    disable_ctrl(IDC_EDGE_APPLY);
    disable_ctrl(IDC_EDGE_PATROL);

    disable_ctrl(IDC_EDGE_PATROL_0);
    disable_ctrl(IDC_EDGE_PATROL_1);
    disable_ctrl(IDC_EDGE_PATROL_2);
    disable_ctrl(IDC_EDGE_PATROL_3);
    disable_ctrl(IDC_EDGE_PATROL_4);
    disable_ctrl(IDC_EDGE_PATROL_5);
    disable_ctrl(IDC_EDGE_PATROL_6);
    disable_ctrl(IDC_EDGE_PATROL_7);

    disable_ctrl(IDC_PERPETUATE_EDGE_PATROL);
  }
}

void PathsDialog::update_node_info()
{
  if(cur_graph && selected_nodes[0] != NULL && cur_graph->node_in_graph(selected_nodes[0]))
  {
    enable_ctrl(IDC_STATIC_NODE_FLAGS);
    enable_ctrl(IDC_NODE_DUCK);
    enable_ctrl(IDC_NODE_COVER);
    enable_ctrl(IDC_NODE_SEEK);
    enable_ctrl(IDC_NODE_PATROL_DELAY);

    set_check_box(IDC_NODE_DUCK, selected_nodes[0]->is_duck_spot());
    set_check_box(IDC_NODE_COVER, selected_nodes[0]->is_cover_spot());
    set_check_box(IDC_NODE_SEEK, selected_nodes[0]->is_seek_spot());
    set_check_box(IDC_NODE_PATROL_DELAY, selected_nodes[0]->is_patrol_delay());
  }
  else
  {
    disable_ctrl(IDC_STATIC_NODE_FLAGS);
    disable_ctrl(IDC_NODE_DUCK);
    disable_ctrl(IDC_NODE_COVER);
    disable_ctrl(IDC_NODE_SEEK);
    disable_ctrl(IDC_NODE_PATROL_DELAY);
  }
}

void PathsDialog::render()
{
	if(!is_visible())
		return;

  if(cur_graph)
    cur_graph->render(color32(0, 255, 0, 96), 0.05f);

  switch(last_mode)
  {
    case _MODE_NODE:
    {
      if(selected_nodes[0] != NULL)
        render_beam_cube(selected_nodes[0]->get_abs_position(), 0.25f, color32(255, 0, 0, 128), 0.05f);
      if(selected_nodes[1] != NULL)
        render_beam_cube(selected_nodes[1]->get_abs_position(), 0.25f, color32(255, 255, 0, 128), 0.05f);
    }
    break;

    case _MODE_EDGE:
    {
      if(selected_nodes[0] != NULL)
        render_beam_cube(selected_nodes[0]->get_abs_position(), 0.25f, color32(255, 0, 0, 128), 0.05f);
      if(selected_nodes[1] != NULL)
        render_beam_cube(selected_nodes[1]->get_abs_position(), 0.25f, color32(255, 0, 0, 128), 0.05f);

      if(cur_graph && selected_nodes[0] != NULL && selected_nodes[1] != NULL && selected_nodes[0] != selected_nodes[1] && cur_graph->edge_in_graph(selected_nodes[0], selected_nodes[1]))
      {
        render_beam(selected_nodes[0]->pt, selected_nodes[1]->pt, color32(255, 0, 0, 128), 0.05f);
      }
    }
    break;

    case _MODE_TEST:
    {
      if(point_count >= 1)
        render_marker(test_pos[0], color32(255, 0, 255, 192), 0.25f);

      if(point_count >= 2)
        render_marker(test_pos[1], color32(255, 0, 255, 192), 0.25f);

      if(test_path.get_num_waypoints() > 0)
      {
        render_beam(test_pos[0], test_path.get_way_point(0), color32(255, 0, 255, 128), 0.05f);
        render_beam(test_pos[1], test_path.get_way_point(test_path.get_num_waypoints() - 1), color32(255, 0, 255, 128), 0.05f);
        test_path.render(color32(0, 0, 255, 128), 0.05f);
      }
      else if(point_count >= 2)
      {
        render_beam(test_pos[0], test_pos[1], color32(255, 0, 255, 128), 0.05f);
      }
    }
    break;
  }
}


void PathsDialog::frame_advance(time_value_t t)
{
	if(!is_visible())
		return;

  if(g_target_fx && g_target_fx->is_visible())
  {
    vector3d origin, dest, norm;
    POINT pt = get_client_mouse_position(g_hwnd);
    get_screen_to_world_segment(vector2di(pt.x, pt.y), origin, dest, norm);

    spawn_point = (dest + (norm*1.0f));

    po  ent_po = po_identity_matrix;
    ent_po.set_facing(norm);
    ent_po.set_position(dest+(norm*0.1f));
    g_target_fx->set_rel_po(ent_po);
    g_target_fx->compute_sector(g_world_ptr->get_the_terrain());
  }
}
void PathsDialog::handle_slider(int ctrl, int pos)
{
	switch(ctrl)
	{
    default:
      Dialog::handle_slider(ctrl, pos);
      break;
  }
}

void PathsDialog::handle_command(int ctrl)
{
	switch(ctrl)
	{
    case IDCANCEL:
      break;

    case IDC_RADIO_NODE:
    {
      point_count = 0;
      selected_nodes[0] = NULL;
      selected_nodes[1] = NULL;

      test_path.resize(0);

      last_mode = _MODE_NODE;
      update();
    }
    break;

    case IDC_RADIO_EDGE:
    {
      point_count = 0;
      selected_nodes[0] = NULL;
      selected_nodes[1] = NULL;

      test_path.resize(0);

      last_mode = _MODE_EDGE;
      update();
    }
    break;

    case IDC_RADIO_TEST:
    {
      point_count = 0;
      selected_nodes[0] = NULL;
      selected_nodes[1] = NULL;

      test_path.resize(0);

      last_mode = _MODE_TEST;
      update();
    }
    break;

    case IDC_AUTO_EDGE:
    {
      auto_edge = get_check_box(IDC_AUTO_EDGE);
    }
    break;

    case IDC_ADD_PATH:
    {
      char name[256];
      get_ctrl_text(IDC_PATHS, name, 255);
      stringx path_name(name);
      path_name.to_upper();

      if((cur_graph = g_world_ptr->get_path_graph(path_name)) == NULL)
      {
        cur_graph = NEW path_graph();
        cur_graph->set_id(path_name);

        g_world_ptr->add_path_graph(cur_graph);

        point_count = 0;
        selected_nodes[0] = NULL;
        selected_nodes[1] = NULL;

        test_path.resize(0);

        populate_paths();
        update();
      }
    }
    break;

    case IDC_DEL_PATH:
    {
      char name[256];
      get_ctrl_text(IDC_PATHS, name, 255);
      stringx path_name(name);
      path_name.to_upper();

      if((cur_graph = g_world_ptr->get_path_graph(path_name)) != NULL)
      {
        g_world_ptr->remove_path_graph(cur_graph);
        cur_graph = NULL;

        point_count = 0;
        selected_nodes[0] = NULL;
        selected_nodes[1] = NULL;

        test_path.resize(0);

        populate_paths();
        update();
      }
    }
    break;

    case IDC_CLEAR:
    {
      if(cur_graph)
      {
        cur_graph->resize(0);

        point_count = 0;
        selected_nodes[0] = NULL;
        selected_nodes[1] = NULL;

        test_path.resize(0);
        update();
      }
    }
    break;

    case IDC_DEL_POINT:
    {
      if(cur_graph && selected_nodes[0] != NULL && cur_graph->node_in_graph(selected_nodes[0]))
      {
        cur_graph->remove_node(selected_nodes[0]);
      }

      selected_nodes[0] = selected_nodes[1];
      selected_nodes[1] = NULL;

      update_node_info();
    }
    break;

    case IDC_NODE_DUCK:
    {
      if(cur_graph && selected_nodes[0] != NULL && cur_graph->node_in_graph(selected_nodes[0]))
        selected_nodes[0]->set_duck_spot(get_check_box(IDC_NODE_DUCK));

      update_node_info();
    }
    break;

    case IDC_NODE_COVER:
    {
      if(cur_graph && selected_nodes[0] != NULL && cur_graph->node_in_graph(selected_nodes[0]))
        selected_nodes[0]->set_cover_spot(get_check_box(IDC_NODE_COVER));

      update_node_info();
    }
    break;

    case IDC_NODE_SEEK:
    {
      if(cur_graph && selected_nodes[0] != NULL && cur_graph->node_in_graph(selected_nodes[0]))
        selected_nodes[0]->set_seek_spot(get_check_box(IDC_NODE_SEEK));

      update_node_info();
    }
    break;

    case IDC_NODE_PATROL_DELAY:
    {
      if(cur_graph && selected_nodes[0] != NULL && cur_graph->node_in_graph(selected_nodes[0]))
        selected_nodes[0]->set_patrol_delay(get_check_box(IDC_NODE_PATROL_DELAY));

      update_node_info();
    }
    break;

    case IDC_ADD_EDGE:
    {
      if(cur_graph && selected_nodes[0] != NULL && selected_nodes[1] != NULL && selected_nodes[0] != selected_nodes[1] && !cur_graph->edge_in_graph(selected_nodes[0], selected_nodes[1]))
        cur_graph->add_edge(selected_nodes[0], selected_nodes[1], 0, 1.0f);

      update_edge_info();
    }
    break;

    case IDC_DEL_EDGE:
    {
      if(cur_graph && selected_nodes[0] != NULL && selected_nodes[1] != NULL && selected_nodes[0] != selected_nodes[1] && cur_graph->edge_in_graph(selected_nodes[0], selected_nodes[1]))
        cur_graph->remove_edge(selected_nodes[0], selected_nodes[1]);

      update_edge_info();
    }
    break;

    case IDC_EDGE_APPLY:
    {
      if(cur_graph && selected_nodes[0] != NULL && selected_nodes[1] != NULL && selected_nodes[0] != selected_nodes[1] && cur_graph->edge_in_graph(selected_nodes[0], selected_nodes[1]))
      {
        path_graph_edge *edge = cur_graph->get_edge(selected_nodes[0], selected_nodes[1]);
        edge->weight_modifier = get_ctrl_float(IDC_WEIGHT_MOD);
      }

      update_edge_info();
    }
    break;

    case IDC_PERPETUATE_EDGE_PATROL:
    {
      if(cur_graph && selected_nodes[0] != NULL && selected_nodes[1] != NULL && selected_nodes[0] != selected_nodes[1] && cur_graph->edge_in_graph(selected_nodes[0], selected_nodes[1]))
      {
        path_graph_edge *edge = cur_graph->get_edge(selected_nodes[0], selected_nodes[1]);

        cur_graph->perpetuate_patrol_id(edge, -1, true);
      }

      update_edge_info();
    }
    break;

    case IDC_PERPETUATE_ALL_PATROL:
    {
      if(cur_graph)
        cur_graph->perpetuate_all_patrol_id(true);

      update_edge_info();
    }
    break;

    case IDC_EDGE_PATROL:
    {
      if(cur_graph && selected_nodes[0] != NULL && selected_nodes[1] != NULL && selected_nodes[0] != selected_nodes[1] && cur_graph->edge_in_graph(selected_nodes[0], selected_nodes[1]))
      {
        path_graph_edge *edge = cur_graph->get_edge(selected_nodes[0], selected_nodes[1]);
        edge->set_patrol(get_check_box(IDC_EDGE_PATROL));
      }

      update_edge_info();
    }
    break;

    case IDC_EDGE_PATROL_0:
    {
      if(cur_graph && selected_nodes[0] != NULL && selected_nodes[1] != NULL && selected_nodes[0] != selected_nodes[1] && cur_graph->edge_in_graph(selected_nodes[0], selected_nodes[1]))
      {
        path_graph_edge *edge = cur_graph->get_edge(selected_nodes[0], selected_nodes[1]);
        edge->set_patrol_id(0, get_check_box(IDC_EDGE_PATROL_0));
      }

      update_edge_info();
    }
    break;

    case IDC_EDGE_PATROL_1:
    {
      if(cur_graph && selected_nodes[0] != NULL && selected_nodes[1] != NULL && selected_nodes[0] != selected_nodes[1] && cur_graph->edge_in_graph(selected_nodes[0], selected_nodes[1]))
      {
        path_graph_edge *edge = cur_graph->get_edge(selected_nodes[0], selected_nodes[1]);
        edge->set_patrol_id(1, get_check_box(IDC_EDGE_PATROL_1));
      }

      update_edge_info();
    }
    break;

    case IDC_EDGE_PATROL_2:
    {
      if(cur_graph && selected_nodes[0] != NULL && selected_nodes[1] != NULL && selected_nodes[0] != selected_nodes[1] && cur_graph->edge_in_graph(selected_nodes[0], selected_nodes[1]))
      {
        path_graph_edge *edge = cur_graph->get_edge(selected_nodes[0], selected_nodes[1]);
        edge->set_patrol_id(2, get_check_box(IDC_EDGE_PATROL_2));
      }

      update_edge_info();
    }
    break;

    case IDC_EDGE_PATROL_3:
    {
      if(cur_graph && selected_nodes[0] != NULL && selected_nodes[1] != NULL && selected_nodes[0] != selected_nodes[1] && cur_graph->edge_in_graph(selected_nodes[0], selected_nodes[1]))
      {
        path_graph_edge *edge = cur_graph->get_edge(selected_nodes[0], selected_nodes[1]);
        edge->set_patrol_id(3, get_check_box(IDC_EDGE_PATROL_3));
      }

      update_edge_info();
    }
    break;

    case IDC_EDGE_PATROL_4:
    {
      if(cur_graph && selected_nodes[0] != NULL && selected_nodes[1] != NULL && selected_nodes[0] != selected_nodes[1] && cur_graph->edge_in_graph(selected_nodes[0], selected_nodes[1]))
      {
        path_graph_edge *edge = cur_graph->get_edge(selected_nodes[0], selected_nodes[1]);
        edge->set_patrol_id(4, get_check_box(IDC_EDGE_PATROL_4));
      }

      update_edge_info();
    }
    break;

    case IDC_EDGE_PATROL_5:
    {
      if(cur_graph && selected_nodes[0] != NULL && selected_nodes[1] != NULL && selected_nodes[0] != selected_nodes[1] && cur_graph->edge_in_graph(selected_nodes[0], selected_nodes[1]))
      {
        path_graph_edge *edge = cur_graph->get_edge(selected_nodes[0], selected_nodes[1]);
        edge->set_patrol_id(5, get_check_box(IDC_EDGE_PATROL_5));
      }

      update_edge_info();
    }
    break;

    case IDC_EDGE_PATROL_6:
    {
      if(cur_graph && selected_nodes[0] != NULL && selected_nodes[1] != NULL && selected_nodes[0] != selected_nodes[1] && cur_graph->edge_in_graph(selected_nodes[0], selected_nodes[1]))
      {
        path_graph_edge *edge = cur_graph->get_edge(selected_nodes[0], selected_nodes[1]);
        edge->set_patrol_id(6, get_check_box(IDC_EDGE_PATROL_6));
      }

      update_edge_info();
    }
    break;

    case IDC_EDGE_PATROL_7:
    {
      if(cur_graph && selected_nodes[0] != NULL && selected_nodes[1] != NULL && selected_nodes[0] != selected_nodes[1] && cur_graph->edge_in_graph(selected_nodes[0], selected_nodes[1]))
      {
        path_graph_edge *edge = cur_graph->get_edge(selected_nodes[0], selected_nodes[1]);
        edge->set_patrol_id(7, get_check_box(IDC_EDGE_PATROL_7));
      }

      update_edge_info();
    }
    break;


    case IDC_RENDER:
    {
      if(get_check_box(IDC_RENDER))
        g_render_paths = 2;
      else
        g_render_paths = 0;
    }
    break;

    default:
      Dialog::handle_command(ctrl);
      break;
  }
}

void PathsDialog::handle_selection_change(int ctrl)
{
  switch(ctrl)
  {
    case IDC_PATHS:
    {
      char name[128] = "";
      get_combo_text(IDC_PATHS, get_combo_index(IDC_PATHS), name);
      cur_graph = g_world_ptr->get_path_graph(name);

    	update();
    }
    break;

    default:
      Dialog::handle_selection_change(ctrl);
      break;
  }
}



void PathsDialog::handle_user_mouse_down(int button, vector3d pos, DWORD flags)
{
	if(!is_visible())
		return;

  switch(last_mode)
  {
    case _MODE_NODE:
    {
      if(button == 0 && (flags & MK_CONTROL))
      {
        handle_command(IDC_ADD_PATH);

        if(cur_graph != NULL)
        {
          if(!cur_graph->node_in_graph(spawn_point))
          {
            cur_graph->add_node(spawn_point, 0);

            selected_nodes[1] = selected_nodes[0];
            selected_nodes[0] = cur_graph->get_node(spawn_point);

            if(auto_edge && selected_nodes[0] != NULL && selected_nodes[1] != NULL && selected_nodes[0] != selected_nodes[1])
              cur_graph->add_edge(selected_nodes[0], selected_nodes[1], 0, 1.0f);
          }
        }
      }
      else if(button == 2)
      {
        vector3d origin, dest, norm, hit;
        POINT pt = get_client_mouse_position(g_hwnd);
        get_screen_to_world_segment(vector2di(pt.x, pt.y), origin, dest, norm);

        vector3d hit_loc;
        for(int i=0; i<cur_graph->get_num_nodes(); i++)
        {
          path_graph_node *node = cur_graph->get_node(i);
          if(node && collide_segment_sphere(origin, dest, node->get_abs_position(), 0.25f, &hit_loc))
          {
            selected_nodes[0] = node;
            break;
          }
        }
      }

      update_node_info();
    }
    break;

    case _MODE_EDGE:
    {
      vector3d origin, dest, norm, hit;
      POINT pt = get_client_mouse_position(g_hwnd);
      get_screen_to_world_segment(vector2di(pt.x, pt.y), origin, dest, norm);

      vector3d hit_loc;
      for(int i=0; i<cur_graph->get_num_nodes(); i++)
      {
        path_graph_node *node = cur_graph->get_node(i);
        if(node && collide_segment_sphere(origin, dest, node->get_abs_position(), 0.25f, &hit_loc))
        {
          selected_nodes[point_count++] = node;
          break;
        }
      }

      if(point_count > 1)
        point_count = 0;

      if(cur_graph && selected_nodes[0] != NULL && selected_nodes[1] != NULL && selected_nodes[0] != selected_nodes[1])
      {
        if(button == 0 && (flags & MK_CONTROL))
        {
          if(cur_graph->edge_in_graph(selected_nodes[0], selected_nodes[1]))
          {
            cur_graph->remove_edge(selected_nodes[0], selected_nodes[1]);
          }
          else
          {
            cur_graph->add_edge(selected_nodes[0], selected_nodes[1], 0, 1.0f);
          }
        }
        else
        {
        }
      }

      update_edge_info();
    }
    break;

    case _MODE_TEST:
    {
      if(button == 0 && (flags & MK_CONTROL))
      {
        test_path.resize(0);

        if(point_count == 0 || point_count == 2)
          point_count = 0;

        test_pos[point_count++] = spawn_point;

        if(point_count == 2 && cur_graph)
          cur_graph->find_path(test_pos[0], test_pos[1], &test_path);
      }
    }
    break;
  }
}

void PathsDialog::handle_user_mouse_up(int button, vector3d pos, DWORD flags)
{
}









/*
bool path_ready = false;
path test;
int path_node1 = 0;
int path_node2 = 16;
*/

void tool_rendering()
{
  if(g_tools_dlg)
    g_tools_dlg->render();

/*
  if(path_ready)
    test.render(color32(0, 0, 255, 128), 0.05);
  else
  {
    path_graph *pg = g_world_ptr->get_path_graph(stringx("TEST_PATH"));
    if(pg)
    {
      if(!pg->find_path(pg->get_node(path_node1), pg->get_node(path_node2), &test))
        warning("WTF!!!!\n\nNo path?!?!?!?!");

      path_ready = true;
    }
  }
*/
}



















#include "ai_polypath.h"
ai_polypath global_polypath;
bool test_path = false;
ai_path global_path;

PathsDialog2::PathsDialog2(HWND pParent)
{
  cur.index[0] = -1;
  cur.index[1] = -1;
  cur.index[2] = -1;

  init(IDD_PATHS2, gPathsDialog2Func, pParent);

  last_file = empty_string;
}

PathsDialog2::~PathsDialog2()
{
}

void PathsDialog2::setup()
{
  if(g_target_fx)
    g_target_fx->set_visible(true);

  cur.index[0] = -1;
  cur.index[1] = -1;
  cur.index[2] = -1;

	update();
}

void PathsDialog2::hide()
{
  if(g_target_fx)
    g_target_fx->set_visible(false);

  cur.index[0] = -1;
  cur.index[1] = -1;
  cur.index[2] = -1;

	Dialog::hide();
}

void PathsDialog2::update()
{
	if(!is_visible())
		return;

  hide_all_ctrl();
  show_ctrl(IDC_LOAD_PATH);
  show_ctrl(IDC_EXPORT_PATH);
  show_ctrl(IDC_TOGGLE_TRI);
  show_ctrl(IDC_REMOVE_PT);

  show_ctrl(IDC_TEST_PATH);
/*
  show_ctrl(IDC_RENDER);
  show_ctrl(IDC_STATIC_MODE);
  show_ctrl(IDC_STATIC_PATH);
  show_ctrl(IDC_PATHS);
  show_ctrl(IDC_ADD_PATH);
  show_ctrl(IDC_DEL_PATH);
  show_ctrl(IDC_CLEAR);
  show_ctrl(IDC_RADIO_NODE);
  show_ctrl(IDC_RADIO_EDGE);
  show_ctrl(IDC_RADIO_TEST);

  enable_all_ctrl(cur_graph != NULL);
  enable_ctrl(IDC_STATIC_PATH);
  enable_ctrl(IDC_PATHS);
  enable_ctrl(IDC_ADD_PATH);

  switch(last_mode)
  {
    case _MODE_NODE:
    {
      set_ctrl_text(IDC_STATIC_MODE, "Node");
      check_radio_button(IDC_RADIO_NODE, IDC_RADIO_NODE);
      show_ctrl(IDC_AUTO_EDGE);
      show_ctrl(IDC_DEL_POINT);

      show_ctrl(IDC_STATIC_NODE_FLAGS);
      show_ctrl(IDC_NODE_DUCK);
      show_ctrl(IDC_NODE_COVER);
      show_ctrl(IDC_NODE_SEEK);
      show_ctrl(IDC_NODE_PATROL_DELAY);

      update_node_info();
    }
    break;

    case _MODE_EDGE:
    {
      set_ctrl_text(IDC_STATIC_MODE, "Edge");
      check_radio_button(IDC_RADIO_NODE, IDC_RADIO_EDGE);
      show_ctrl(IDC_ADD_EDGE);
      show_ctrl(IDC_DEL_EDGE);
      show_ctrl(IDC_WEIGHT_MOD);
      show_ctrl(IDC_STATIC_WEIGHT_MOD);
      show_ctrl(IDC_EDGE_APPLY);
      show_ctrl(IDC_EDGE_PATROL);

      show_ctrl(IDC_EDGE_PATROL_0);
      show_ctrl(IDC_EDGE_PATROL_1);
      show_ctrl(IDC_EDGE_PATROL_2);
      show_ctrl(IDC_EDGE_PATROL_3);
      show_ctrl(IDC_EDGE_PATROL_4);
      show_ctrl(IDC_EDGE_PATROL_5);
      show_ctrl(IDC_EDGE_PATROL_6);
      show_ctrl(IDC_EDGE_PATROL_7);

      show_ctrl(IDC_PERPETUATE_ALL_PATROL);
      show_ctrl(IDC_PERPETUATE_EDGE_PATROL);
      show_ctrl(IDC_STATIC_PATROL);

      update_edge_info();
    }
    break;

    case _MODE_TEST:
    {
      set_ctrl_text(IDC_STATIC_MODE, "Test");
      check_radio_button(IDC_RADIO_NODE, IDC_RADIO_TEST);
    }
    break;
  }

  set_check_box(IDC_AUTO_EDGE, auto_edge);

  set_check_box(IDC_RENDER, g_render_paths);
*/
	Dialog::update();
}

void PathsDialog2::render()
{
	if(!is_visible())
		return;

  vector<vector3d>::iterator pt = pts.begin();
  vector<vector3d>::iterator pt_end = pts.end();

  while(pt != pt_end)
  {
    render_marker((*pt), color32(0, 255, 255, 192), 0.25f);
    ++pt;
  }

  for(int i=0; i<3; ++i)
  {
    if(cur.index[i] >= 0 && cur.index[i] < pts.size())
      render_beam_cube(pts[cur.index[i]], 0.25f, color32(255, 0, 0, 128), 0.05f);
    else
      cur.index[i] = -1;
  }

  vector<triangle>::iterator tri = tris.begin();
  vector<triangle>::iterator tri_end = tris.end();

  while(tri != tri_end)
  {
    triangle t = *tri;

    if(t.index[0] < pts.size() && t.index[1] < pts.size() && t.index[2] < pts.size() && t.index[0] != t.index[1] && t.index[1] != t.index[2] && t.index[2] != t.index[0])
    {
      render_beam(pts[t.index[0]], pts[t.index[1]], color32(0, 255, 255, 128), 0.05f);
      render_beam(pts[t.index[1]], pts[t.index[2]], color32(0, 255, 255, 128), 0.05f);
      render_beam(pts[t.index[2]], pts[t.index[0]], color32(0, 255, 255, 128), 0.05f);

      render_triangle(pts[t.index[0]], pts[t.index[1]], pts[t.index[2]], color32(0, 255, 0, 128), true);

      ++tri;
    }
    else
      tri = tris.erase(tri);
  }

  if(test_path)
  {
    global_path.render();
//    global_polypath.render();
  }
}


void PathsDialog2::frame_advance(time_value_t t)
{
	if(!is_visible())
		return;

  if(g_target_fx && g_target_fx->is_visible())
  {
    vector3d origin, dest, norm;
    POINT pt = get_client_mouse_position(g_hwnd);
    get_screen_to_world_segment(vector2di(pt.x, pt.y), origin, dest, norm);

    spawn_point = (dest + (norm*0.25f));

    po ent_po = po_identity_matrix;
    ent_po.set_facing(norm);
    ent_po.set_position(dest+(norm*0.1f));
    g_target_fx->set_rel_po(ent_po);
    g_target_fx->compute_sector(g_world_ptr->get_the_terrain());
  }
}

void PathsDialog2::handle_command(int ctrl)
{
	switch(ctrl)
	{
    case IDCANCEL:
      break;

    case IDC_EXPORT_PATH:
      export();
      break;

    case IDC_LOAD_PATH:
      load();
      break;

    case IDC_TEST_PATH:
    {
      global_polypath.resize(0);

      vector<triangle>::iterator tri = tris.begin();
      vector<triangle>::iterator tri_end = tris.end();

      while(tri != tri_end)
      {
        triangle t = *tri;

        if(t.index[0] < pts.size() && t.index[1] < pts.size() && t.index[2] < pts.size() && t.index[0] != t.index[1] && t.index[1] != t.index[2] && t.index[2] != t.index[0])
        {
          global_polypath.add_cell(pts[t.index[0]], pts[t.index[1]], pts[t.index[2]]);

          ++tri;
        }
        else
          tri = tris.erase(tri);
      }

      global_polypath.link();
      test_path = global_polypath.find_path(global_path, ZEROVEC, g_world_ptr->get_hero_ptr()->get_abs_position());
      assert(test_path);
    }
    break;

    case IDC_REMOVE_PT:
    {
      remove_pt(cur.index[0]);
      remove_pt(cur.index[1]);
      remove_pt(cur.index[2]);

      cur.index[0] = -1;
      cur.index[1] = -1;
      cur.index[2] = -1;
    }
    break;

    case IDC_CLEAR_PATH:
    {
      pts.resize(0);
      tris.resize(0);

      cur.index[0] = -1;
      cur.index[1] = -1;
      cur.index[2] = -1;
    }
    break;

    case IDC_TOGGLE_TRI:
    {
      toggle_triangle();
    }
    break;

    default:
      Dialog::handle_command(ctrl);
      break;
  }
}

void PathsDialog2::handle_selection_change(int ctrl)
{
  switch(ctrl)
  {
    default:
      Dialog::handle_selection_change(ctrl);
      break;
  }
}

void PathsDialog2::toggle_triangle()
{
  if(cur.index[0] >= 0 && cur.index[0] < pts.size()
     && cur.index[1] >= 0 && cur.index[1] < pts.size()
     && cur.index[2] >= 0 && cur.index[2] < pts.size() &&
     cur.index[0] != cur.index[1] && cur.index[1] != cur.index[2] && cur.index[2] != cur.index[0])
  {
    bool found = false;
    vector<triangle>::iterator tri = tris.begin();
    while(tri != tris.end())
    {
      if(cur == (*tri))
      {
        tri = tris.erase(tri);
        found = true;
        break;
      }
      else
        ++tri;
    }

    if(!found)
    {
      vector3d ptA = (pts[cur.index[1]] - pts[cur.index[0]]);
      vector3d ptB = (pts[cur.index[2]] - pts[cur.index[0]]);
      vector3d norm = cross(ptA,ptB);

      if(norm.y < 0.0f)
      {
        int temp = cur.index[1];
        cur.index[1] = cur.index[2];
        cur.index[2] = temp;
      }

      // MAKE sure tris are in clocwise order!!!!!!!
      tris.push_back(cur);
    }
  }
}


void PathsDialog2::handle_user_mouse_down(int button, vector3d pos, DWORD flags)
{
	if(!is_visible())
		return;

  if(button == 0)
  {
    if(flags & MK_CONTROL)
    {
      pts.push_back(spawn_point);
      cur.index[2] = cur.index[1];
      cur.index[1] = cur.index[0];
      cur.index[0] = pts.size() - 1;
    }
    else if(flags & MK_SHIFT)
    {
      toggle_triangle();
    }
  }
  else if(button == 2)
  {
    vector3d origin, dest, norm, hit;
    POINT ptX = get_client_mouse_position(g_hwnd);
    get_screen_to_world_segment(vector2di(ptX.x, ptX.y), origin, dest, norm);

    vector3d hit_loc;
    int i = 0;

    vector<vector3d>::iterator pt = pts.begin();
    vector<vector3d>::iterator pt_end = pts.end();

    while(pt != pt_end)
    {
      if(collide_segment_sphere(origin, dest, (*pt), 0.25f, &hit_loc))
      {
        if(flags & MK_CONTROL)
        {
          cur.index[2] = -1;
          cur.index[1] = -1;
          cur.index[0] = i;
        }
        else
        {
          cur.index[2] = cur.index[1];
          cur.index[1] = cur.index[0];
          cur.index[0] = i;
        }

        break;
      }

      ++pt;
      ++i;
    }
  }
}

void PathsDialog2::remove_pt(int index)
{
  if(index >= 0 && index < pts.size())
  {
    vector<vector3d>::iterator pt = pts.begin();

    int i = 0;
    while(pt != pts.end())
    {
      if(i == index)
      {
        vector<triangle>::iterator tri = tris.begin();

        while(tri != tris.end())
        {
          if((*tri).index[0] == i || (*tri).index[1] == i || (*tri).index[2] == i)
          {
            tri = tris.erase(tri);
          }
          else
          {
            if((*tri).index[0] > i)
              (*tri).index[0]--;
            if((*tri).index[1] > i)
              (*tri).index[1]--;
            if((*tri).index[2] > i)
              (*tri).index[2]--;

            ++tri;
          }
        }

        pt = pts.erase(pt);

        break;
      }

      ++pt;
      ++i;
    }
  }
}

void PathsDialog2::handle_user_mouse_up(int button, vector3d pos, DWORD flags)
{
}

void PathsDialog2::export()
{
  filespec spec(last_file);

  char file[_MAX_PATH+1];
  char path[_MAX_PATH+1];

  strcpy(file, (spec.name+spec.ext).c_str());
  strcpy(path, (spec.path).c_str());

  if(save_file_dialog(file, path, "path", "ArchEngine Path", "Export PolyPath"))
  {
    char fullpath[_MAX_PATH+1] = "";
    sprintf(fullpath, "%s\\%s", path, file);

    if(!tool_file_exists(fullpath) || yes_no_dialog("Overwrite File?", "File '%s' exists!\n\nOverwrite?", fullpath))
    {
      ofstream fout;

      fout.open(fullpath, ios::out | ios::trunc);
      if(!fout.fail())
      {
        last_file = fullpath;

        vector<vector3d>::iterator pt = pts.begin();
        vector<vector3d>::iterator pt_end = pts.end();

        while(pt != pt_end)
        {
          fout<<"pt "<<(*pt).x<<" "<<(*pt).y<<" "<<(*pt).z<<"\n";
          ++pt;
        }

        vector<triangle>::iterator tri = tris.begin();
        vector<triangle>::iterator tri_end = tris.end();

        while(tri != tri_end)
        {
          vector3d ptA = (pts[(*tri).index[1]] - pts[(*tri).index[0]]);
          vector3d ptB = (pts[(*tri).index[2]] - pts[(*tri).index[0]]);
          vector3d norm = cross(ptA,ptB);

          if(norm.y < 0.0f)
          {
            int temp = (*tri).index[1];
            (*tri).index[1] = (*tri).index[2];
            (*tri).index[2] = temp;
          }

          fout<<"tri "<<(*tri).index[0]<<" "<<(*tri).index[1]<<" "<<(*tri).index[2]<<"\n";
          ++tri;
        }

        fout.close();
      }
    }
  }
}


void PathsDialog2::load()
{
  filespec spec(last_file);

  char file[_MAX_PATH+1];
  char path[_MAX_PATH+1];

  strcpy(file, (spec.name+spec.ext).c_str());
  strcpy(path, (spec.path).c_str());

  if(open_file_dialog(file, path, "path", "ArchEngine Path", "Load PolyPath"))
  {
    char fullpath[_MAX_PATH+1] = "";
    sprintf(fullpath, "%s\\%s", path, file);

    if(tool_file_exists(fullpath))
    {
      bool filelock = os_file::is_system_locked();
      os_file::system_unlock();

      chunk_file fs;
      fs.open(fullpath);

      pts.resize(0);
      tris.resize(0);

      stringx label;
      for(serial_in(fs, &label); label != chunkend_label && label.size() > 0; serial_in(fs, &label))
      {
        if(label == "pt")
        {
          vector3d pt;
          serial_in(fs, &pt);
          pts.push_back(pt);
        }
        else if(label == "tri")
        {
          int x;
          for(int i=0; i<3; ++i)
          {
            serial_in(fs, &x);
            cur.index[i] = x;
          }

          tris.push_back(cur);
        }
      }

      cur.index[0] = -1;
      cur.index[1] = -1;
      cur.index[2] = -1;

      fs.close();

      if ( filelock )
        os_file::system_lock();
    }
  }
}










#endif  // _ENABLE_WORLD_EDITOR





