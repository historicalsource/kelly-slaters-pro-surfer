//
// The FloatingObject class is used to represent any inactive objects on the water
// like buoys/tubers
//
// Author: Leonardo Zide
//

// With precompiled headers enabled, all text up to and including 
// the following line are ignored by the compiler (dc 01/18/02)
#include "global.h"

#ifdef _XBOX
#include "ngl.h"
#include "color.h"
#include "entity.h"
#include "app.h"
#include "game.h"
#include "camera.h"
#include "file_finder.h"
#include "entity_anim.h"
#include "beachdata.h"
#include "inputmgr.h"
#include "conglom.h"
#include "kellyslater_controller.h"
#include "osdevopts.h"
#include "entity_maker.h"
#include "timer.h"
#include "wds.h"
extern float world_gravity;
#elif defined(TARGET_GC)
#include "ngl.h"
#else
// let's get cute
#include "ngl_ps2.h"
#endif /* TARGET_XBOX JIV DEBUG */

#include "physical_interface.h"
#include "floatobj.h"
#include "water.h"
#include "text_parser.h"
#include "ksfx.h"
#include "trail.h"
#include "lightmgr.h"


// ============================================================================
// beach_object class

beach_object::beach_object (entity* ent, const stringx& path)
{
  my_entity = ent;
	my_path = path;
  spawned = false;
  spawn_count = -1;
  smashable = false;
  timer_type = -1;
  active = true;
  times_spawned = 0;
	never_despawn = false;

  next = NULL;
}

beach_object::~beach_object ()
{
}

void beach_object::get_settings (const beach_object& obj)
{
	my_path = obj.my_path;
  spawn_time = obj.spawn_time;
  timer_type = obj.timer_type;
  spawned = obj.spawned;
  spawn_count = obj.spawn_count;
  times_spawned = obj.times_spawned;
  smashable = obj.smashable;
  active = obj.active;
  physical = obj.physical;
	never_despawn = obj.never_despawn;
}

bool beach_object::parse_params (char** argp, int argc)
{
  read_int_param (argp, argc, "spawn_count", &spawn_count);

  if (find_param (argp, argc, "never_despawn"))
    never_despawn = true;

  if (find_param (argp, argc, "smashable"))
    smashable = true;

  return read_float_param (argp, argc, "time", &spawn_time);
}

bool beach_object::find_param (char** argp, int argc, const char *name) const
{
  for (int i = 0; i < argc; i++)
    if (!strcmp (argp[i], name))
      return true;

  return false;
}

bool beach_object::read_int_param (char** argp, int argc, const char *name, int *value) const
{
  for (int i = 0; i < argc - 1; i++)
    if (!strcmp (argp[i], name))
      return (sscanf (argp[i+1], "%d", value) == 1);

  return false;
}

bool beach_object::read_float_param (char** argp, int argc, const char *name, float *value) const
{
  for (int i = 0; i < argc - 1; i++)
    if (!strcmp (argp[i], name))
      return (sscanf (argp[i+1], "%f", value) == 1);

  return false;
}

bool beach_object::read_vector3d_param (char** argp, int argc, const char *name, vector3d *value) const
{
  for (int i = 0; i < argc - 3; i++)
    if (!strcmp (argp[i], name))
    {
      if (sscanf (argp[i+1], "%f", &value->x) != 1)
        return false;

      if (sscanf (argp[i+2], "%f", &value->y) != 1)
        return false;

      if (sscanf (argp[i+3], "%f", &value->z) != 1)
        return false;

      return true;
    }

  return false;
}

bool beach_object::read_text_param (char** argp, int argc, const char *name, stringx& value) const
{
  for (int i = 0; i < argc - 1; i++)
    if (!strcmp (argp[i], name))
		{
      value = argp[i+1];
			return true;
		}

  return false;
}

// ============================================================================
// beach_event class

beach_event::beach_event (bool (*func) (float dt, void **func_data))
  : beach_object(NULL, "")
{
  my_func = func;
  my_func_data = NULL;

  physical = false;
}

beach_event::~beach_event ()
{
  despawn ();
}

beach_event* beach_event::create_from_file (text_parser& parser, int timer_relative)
{
  beach_event *event = NULL;
  char *argp[32];
  int argc = 0;
  char type[64];

  // event type
  parser.get_token (false, false);
  strcpy (type, parser.token);

  // store all parameters for later use
  while (parser.get_token (false, true))
  {
    // Evan, why can't I use strdup ?
//    argp[argc++] = strdup (parser.token);
    argp[argc] = (char*)malloc (strlen (parser.token) + 1);
    strcpy (argp[argc], parser.token);
    argc++;
  }

  if (!strcmp (type, "tube_spit"))
    event = NEW beach_event (&ks_fx_create_tube_spit);
  else if (!stricmp (type, "lightning_l"))
	  event = NEW beach_event (&ks_fx_create_lightning_l);
  else if (!stricmp (type, "lightning_r"))
	  event = NEW beach_event (&ks_fx_create_lightning_r);
  else if (!stricmp (type, "photo"))
	  event = NEW beach_event (&TakePhoto);
  else
  {
    nglPrintf ("Unknown event type: %s\n", type);
  }

  if (event)
  {
    event->timer_type = timer_relative;
    if (!event->parse_params (argp, argc))
    {
      nglPrintf ("Cannot parse params for %s, ignoring event.\n", type);
      delete event;
      event = NULL;
    }
  }

  while (argc)
    free (argp[--argc]);

  return event;
}

void beach_event::spawn ()
{
  if (times_spawned == spawn_count)
    return;

  spawned = true;
  times_spawned++;
}

void beach_event::despawn ()
{
  my_func (-1, &my_func_data);
	my_func_data = NULL;
  spawned = false;
}

bool beach_event::update (float dt)
{
  return my_func (dt, &my_func_data);
}

void beach_object::collide (entity *ent, const vector3d& dir)
{
}

void beach_object::jumped_over (entity *ent)
{
}

void beach_object::sprayed (entity *ent)
{
}

// ============================================================================
// beach_billboard class

beach_billboard::beach_billboard (entity *ent, const stringx& path)
  : beach_object(ent, path)
{
  my_entity = ent;
  my_velocity = ZEROVEC;
  rotate = true;
  my_life = 0;

  physical = false;
}

beach_billboard::~beach_billboard ()
{
  despawn ();
}

beach_billboard* beach_billboard::create_from_file (text_parser& parser, int timer_relative)
{
  beach_billboard *obj = NULL;
  stringx path, name;
  char *argp[32];
  int argc = 0;
  entity *ent;

  // file name
  parser.get_token (false, false);
  path = parser.token;
  parser.get_token (false, false);
  name = parser.token;

  stringx mesh_path = path + "\\entities\\";
  nglSetMeshPath ((char *)mesh_path.c_str ());
  stringx texture_path = path + "\\" + os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR) + "\\";
  nglSetTexturePath ((char *)texture_path.c_str ());

  // store all parameters for later use
  while (parser.get_token (false, true))
  {
    // Evan, why can't I use strdup ?
//    argp[argc++] = strdup (parser.token);
    argp[argc] = (char*)malloc (strlen (parser.token) + 1);
    strcpy (argp[argc], parser.token);
    argc++;
  }

  if (!file_finder_exists (mesh_path + name, ".ent", 0))
  {
    nglPrintf ("Could not find entity %s\n", name.c_str ());
  }
  else
  {
    ent = g_entity_maker->create_entity_or_subclass (mesh_path + name, entity_id::make_unique_id (), po_identity_matrix, empty_string, entity::ACTIVE, NULL);

    if (ent != NULL)
    {
      ent->set_visible (false);
      ent->set_parsed_name (name);

      obj = NEW beach_billboard (ent, path);

      obj->timer_type = timer_relative;

      if (!obj->parse_params (argp, argc))
      {
        nglPrintf ("Cannot parse params for %s, ignoring object.\n", name.c_str ());
        delete obj;
        obj = NULL;
      }
    }
  }

  while (argc)
    free (argp[--argc]);

  return obj;
}

bool beach_billboard::parse_params (char** argp, int argc)
{
  vector3d pos;
  float scale;

  if (!beach_object::parse_params (argp, argc))
    return false;

  if (!read_vector3d_param (argp, argc, "position", &pos))
    return false;

  if (!read_vector3d_param (argp, argc, "velocity", &my_velocity))
    return false;

  if (read_float_param (argp, argc, "scale", &scale))
    my_entity->set_render_scale (vector3d (scale, scale, scale));

  read_float_param (argp, argc, "life", &my_life);

  if (find_param (argp, argc, "dont_rotate"))
    rotate = false;

  my_initial_po = po_identity_matrix;
  my_initial_po.set_position (pos);

  return true;
}

void beach_billboard::spawn ()
{
  if (times_spawned == spawn_count)
    return;

  my_entity->set_visible (true);
  my_entity->update_region ();

  //my_entity->set_rel_po (po_identity_matrix);
  my_entity->set_rel_po(my_initial_po);

  my_age = 0;
  spawned = true;
  times_spawned++;
}

void beach_billboard::despawn ()
{
  my_entity->set_visible (false);
  my_entity->set_active (false);

  spawned = false;
}

bool beach_billboard::update (float dt)
{
  vector3d position (my_entity->get_rel_position () + my_velocity * dt);

  if (rotate)
    my_entity->set_rel_po (po (1,0,0, 0,1,0, 0,0,1, 0,0,0) * app::inst()->get_game()->get_current_view_camera()->get_rel_po ());
  else
    my_entity->set_rel_po (po_identity_matrix);//  * app::inst()->get_game()->get_current_view_camera()->get_rel_po () );
  my_entity->set_rel_position (position);

  my_age += dt;
  if ((my_life != 0) && (my_age > my_life))
    return false;

  return true;
}

// ============================================================================
// water_object class
float g_current_x_mult = 1.0f;

water_object::water_object (entity *ent, const stringx& path)
  : beach_object(ent, path)
{
  my_entity = ent;

  ren_col = my_entity->get_render_color ();
	my_max_alpha = 1;

  physical = true;
  grindable = false;
  fade_distance = 0;
  spawn_by_marker = false;
  current_x_mult = g_current_x_mult;
}

water_object::~water_object ()
{
  despawn ();
}

#define BUOY_DISTANCE 5

water_object* water_object::create_from_file (text_parser& parser, int timer_relative)
{
  stringx path, name;
  water_object *obj = NULL;
  char type[64];
  entity *ent;
  char *argp[32];
  int argc = 0;

  // entity type
  parser.get_token (false, false);
  strcpy (type, parser.token);

  // file name
  parser.get_token (false, false);
  path = parser.token;
  parser.get_token (false, false);
  name = parser.token;

  stringx mesh_path = path + "\\entities\\";
  nglSetMeshPath ((char *)mesh_path.c_str ());
  stringx texture_path = path + "\\" + os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR) + "\\";
  nglSetTexturePath ((char *)texture_path.c_str ());

  // store all parameters for later use
  while (parser.get_token (false, true))
  {
    // Evan, why can't I use strdup ?
//    argp[argc++] = strdup (parser.token);
    argp[argc] = (char*)malloc (strlen (parser.token) + 1);
    strcpy (argp[argc], parser.token);
    argc++;
  }

  if (!file_finder_exists (mesh_path + name, ".ent", 0))
  {
    nglPrintf ("Could not find entity %s\n", name.c_str ());
  }
  else
  {
    ent = g_entity_maker->create_entity_or_subclass (mesh_path + name, entity_id::make_unique_id (), po_identity_matrix, empty_string, entity::ACTIVE, NULL);

    if (ent != NULL)
    {
      ent->set_visible (false);
      ent->set_parsed_name (name);

      if (!strcmp (type, "floating_entity"))
        obj = NEW floating_object (ent, path);
      else if (!strcmp (type, "static_entity"))
        obj = NEW static_object (ent, path);
      else if (!strcmp (type, "surfing_entity"))
        obj = NEW surfing_object (ent, path, name);
      else
      {
        nglPrintf ("Unknown entity type: %s\n", type);
        obj = NULL;
      }

			if (obj != NULL)
			{
				obj->timer_type = timer_relative;

	      if (!obj->parse_params (argp, argc))
		    {
			    nglPrintf ("Cannot parse params for %s, ignoring object.\n", name.c_str ());
				  delete obj;
					obj = NULL;
				}
      }
    }
  }

  while (argc)
    free (argp[--argc]);

  // Some water objects are smashable.
  if (obj != NULL)
  {
	  if ((stricmp (name.c_str(), "boogieboarder1") == 0) ||
		  (stricmp (name.c_str (), "buoy_b") == 0) ||
		  (stricmp(name.c_str(), "boogieboarder2") == 0) ||
		  (stricmp(name.c_str(), "hammerhead") == 0) ||
		  (stricmp(name.c_str(), "cameraman") == 0) ||
		  (stricmp(name.c_str(), "fatbastard") == 0) ||
		  (stricmp(name.c_str(), "pancakeice_a") == 0) ||
		  (stricmp(name.c_str(), "pancakeice_b") == 0) ||
		  (stricmp(name.c_str(), "pancakeice_c") == 0) ||
		  (stricmp(name.c_str(), "iceberg_a") == 0) ||
		  (stricmp(name.c_str(), "iceberg_b") == 0))
	  {
		  obj->smashable = true;
	  }
  }

#ifdef SLALOM_TEST

  // this is a lot of crap, shouldn't be like this at all... perfect example of shitty code to add new features
  // maybe adding a "clone" parameter to the beach file would make this cleaner
  if ((obj != NULL) && (stricmp (name.c_str (), "buoy_b") == 0))
  {
    ent = g_entity_maker->create_entity_or_subclass (mesh_path + name, entity_id::make_unique_id (), po_identity_matrix, empty_string, entity::ACTIVE, NULL);
    ent->set_visible (false);
    ent->set_parsed_name (name);

    water_object *obj2 = NEW floating_object (ent, path);
    obj2->get_settings (*obj);
    ((floating_object*)obj)->my_other_buoy = (floating_object*)obj2;

    vector3d disp (BUOY_DISTANCE, 0, 0);
    disp = obj->my_initial_po.slow_xform (disp);
    obj2->my_initial_po.set_position (disp);

    g_beach_ptr->add_object (obj2);
  }

#endif

  return obj;
}

bool water_object::parse_params (char** argp, int argc)
{
	float rotate, scale;
	vector3d pos;
	bool found_marker = false;
	bool found_mag = false;

	my_initial_po = po_identity_matrix;

	if (!beach_object::parse_params (argp, argc))
		return false;

	if (find_param (argp, argc, "grindable"))
		grindable = true;

	if (read_float_param (argp, argc, "marker", &marker_num))
		found_marker = true;

	if (read_float_param (argp, argc, "magnitude", &marker_offset))
		found_mag = true;

	if (read_float_param (argp, argc, "scale", &scale))
		my_entity->set_render_scale (vector3d (scale, scale, scale));

	if (read_float_param (argp, argc, "rotate_x", &rotate))
		my_initial_po.set_rotate_x (DEG_TO_RAD (rotate));

	if (read_float_param (argp, argc, "rotate_y", &rotate))
		my_initial_po.set_rotate_y (DEG_TO_RAD (rotate));

	if (read_float_param (argp, argc, "rotate_z", &rotate))
		my_initial_po.set_rotate_z (DEG_TO_RAD (rotate));

	read_float_param (argp, argc, "fade_distance", &fade_distance);

	if (found_marker && found_mag)
		spawn_by_marker = true;
	else if (!read_vector3d_param (argp, argc, "position", &pos))
	{
		nglPrintf ("No marker or position provided.\n");
		spawn_by_marker = false;
		return false;
	}

	my_initial_po.set_position (pos);

	return true;
}

// base update function that makes the objects fade
bool water_object::update (float dt)
{
  vector3d position (my_entity->get_rel_position ());

  // remove tinting to fix the sorting bug for now until we find the cause of the problem
  my_entity->set_render_color (color32_white);

  // fade object
  if (((fade_distance >= 0) && (position.z > WAVE_MeshMaxZ + fade_distance)) || (my_max_alpha != 1.0f))
  {
    float blend;

    blend = min (my_max_alpha, 1.0f - ((position.z - (WAVE_MeshMaxZ + fade_distance)) / 50));

    if (blend < 0.025f)
    {
      color32 col = ren_col;
      col.set_alpha (0);
      my_entity->set_render_color (col);

      return false;
    }
    else
    {
      color32 col = ren_col;
      col.set_alpha (ren_col.get_alpha () * blend);
      my_entity->set_render_color (col);
    }
  }
  /*
  else if (position.z < WAVE_MeshMinZ)
  {
    float blend;

    blend = 1.0f - ((WAVE_MeshMinZ - position.z) / 50);

    if (blend < 0.025f)
    {
      color32 col = ren_col;
      col.set_alpha (0);
      my_entity->set_render_color (col);

      return false;
    }
    else
    {
      color32 col = ren_col;
      col.set_alpha (ren_col.get_alpha () * blend);
      my_entity->set_render_color (col);
    }
  }
  */

  return true;
}

void water_object::spawn ()
{
	if (times_spawned == spawn_count)
		return;

	my_max_alpha = 1;

	if (spawn_by_marker)
	{
		WaveMarkerEnum obj_enum = (WaveMarkerEnum) (WAVE_MarkerFloatSpawn1 + (int) marker_num);
		int curbeach = g_game_ptr->get_beach_id();
		vector3d water_current;

		if (BeachDataArray[curbeach].bdir)
			water_current.x = WaveDataArray[curbeach].speedx;
		else
			water_current.x = -WaveDataArray[curbeach].speedx;

		water_current.x *= current_x_mult;
		water_current.y = 0.0f;
		water_current.z = -WaveDataArray[curbeach].speedz;

		vector3d offset = - marker_offset*water_current;

		vector3d pos = *WAVE_GetMarker(obj_enum) + offset;
		my_initial_po.set_position (pos);
		spawn_by_marker = false;
	}

	// is this all that's needed ?
	my_entity->set_visible (true);
	my_entity->update_region ();

	//my_entity->set_rel_po (po_identity_matrix);
	my_entity->set_rel_po(my_initial_po);
	use_hint = false;

	spawned = true;
	times_spawned++;
}

void water_object::despawn ()
{
  my_entity->set_render_color (ren_col);
  my_entity->set_visible (false);
  my_entity->set_active (false);

  spawned = false;
}

// only implemented for water_objects right now
void water_object::get_settings (const water_object& obj)
{
  ren_col = obj.ren_col;
  wave_hint = obj.wave_hint;
  use_hint = obj.use_hint;
  my_initial_po = obj.my_initial_po;
  grindable = obj.grindable;
  fade_distance = obj.fade_distance;

  beach_object::get_settings (obj);
}

void water_object::collide (entity *ent, const vector3d& dir)
{
}

void water_object::jumped_over (entity *ent)
{
}

void water_object::sprayed (entity *ent)
{
}

// ============================================================================
// floating_object class

// variables to allow control of the objects using the debug menu
float g_max_dy;
float g_speed_dy;
float g_max_angle;
float g_speed_angle;

floating_object::floating_object (entity *ent, const stringx& path)
  : water_object (ent, path)
{
  max_dy = 0.25f;
  speed_dy = 1;
  max_angle = 20;
  speed_angle = 1;

  water_interaction = 1.0f;
  g_max_dy = max_dy;
  g_speed_dy = speed_dy;
  g_max_angle = max_angle;
  g_speed_angle = speed_angle;

#ifdef SLALOM_TEST
  my_other_buoy = NULL;
#endif
}

floating_object::~floating_object ()
{
}

bool floating_object::parse_params (char** argp, int argc)
{
  if (!water_object::parse_params (argp, argc))
    return false;

  read_float_param (argp, argc, "water", &water_interaction);

  return true;
}

void floating_object::get_settings (const floating_object& obj)
{
  desired_dy = obj.desired_dy;
  current_dy = obj.current_dy;
  max_dy = obj.max_dy;
  speed_dy = obj.speed_dy;

  desired_angle = obj.desired_angle;
  current_angle = obj.current_angle;
  max_angle = obj.max_angle;
  speed_angle = obj.speed_angle;

  water_interaction = obj.water_interaction;

#ifdef SLALOM_TEST
  my_other_buoy = obj.my_other_buoy;
  slalom = obj.slalom;
#endif

  water_object::get_settings (obj);
}

void floating_object::spawn ()
{
  if (times_spawned == spawn_count)
    return;

  desired_dy = 0;
  current_dy = 0;
  desired_angle = 0;
  current_angle = 0;

#ifdef SLALOM_TEST
  slalom = SLALOM_NOTTESTED;
#endif

  water_object::spawn ();
}

bool floating_object::update (float dt)
{
  vector3d position, current, normal;

  position = my_entity->get_rel_position ();

#ifdef SLALOM_TEST
  {

  if ((my_entity->get_parsed_name ().find ("buoy_b") != stringx::npos) && (my_other_buoy != NULL) && (slalom == SLALOM_NOTTESTED))
  {
    vector3d p = g_world_ptr->get_board_ptr (g_game_ptr->get_active_player ())->get_abs_position ();
    vector3d p1 = my_entity->get_abs_position ();
    vector3d p2 = my_other_buoy->get_entity ()->get_abs_position ();
//    vector3d p1 = ((conglomerate *)my_entity)->get_member ("BUOY1")->get_abs_position ();
//    vector3d p2 = ((conglomerate *)my_entity)->get_member ("BUOY2")->get_abs_position ();
    vector3d v = p2 - p1;
    vector3d w = p - p1;
    vector3d pb;
    float b, d, c1, c2;

    c1 = dot (w, v);

    if (c1 <= 0)
      d = (p - p1).length ();
    else
    {
      c2 = dot (v, v);

      if (c2 <= c1)
        d = (p - p2).length ();
      else
      {
        b = c1 / c2;
        pb = p1 + b * v;
        d = (p - pb).length ();
      }
    }

    float d1 = (p - p1).length2 ();
    float d2 = (p - p2).length2 ();

    if ((d < 2) && (d1 < BUOY_DISTANCE * BUOY_DISTANCE) && (d2 < BUOY_DISTANCE * BUOY_DISTANCE))
    {
      frontendmanager.IGO->Print("Crossed gate");
      slalom = SLALOM_OK;
    }
  }

/*
  if (my_entity->get_parsed_name ().find ("buoya") != stringx::npos)
  {
    if ((slalom == SLALOM_NOTTESTED) && (position.z > p.z))
    {
      if (position.x > p.x)
      {
        frontendmanager.IGO->Print("Missed buoy");
        slalom = SLALOM_MISSED;
      }
      else
      {
        frontendmanager.IGO->Print("Got buoy");
        slalom = SLALOM_OK;
      }
    }
  }
  else if (my_entity->get_parsed_name ().find ("buoy_b") != stringx::npos)
  {
    if ((slalom == SLALOM_NOTTESTED) && (position.z > p.z))
    {
      if (position.x < p.x)
      {
        frontendmanager.IGO->Print("Missed buoy");
        slalom = SLALOM_MISSED;
      }
      else
      {
        frontendmanager.IGO->Print("Got buoy");
        slalom = SLALOM_OK;
      }
    }
  }
*/
  }
#endif

  // variables to allow control of the objects using the debug menu
  max_dy = g_max_dy;
  speed_dy = g_speed_dy;
  max_angle = g_max_angle;
  speed_angle = g_speed_angle;

  // make the object flow with the water
  if ((position.x < WAVE_MeshMaxX) && (position.x > WAVE_MeshMinX) &&
      (position.z < WAVE_MeshMaxZ) && (position.z > WAVE_MeshMinZ))
  {
    if (water_interaction == 1)
    {
      if (use_hint)
      {
#if defined(TARGET_XBOX)
        WaveFloaterArgs wfa(
          (WaveQueryFlags) (WAVE_HINTGIVEN | WAVE_HINTSOUGHT | WAVE_NORMALSOUGHT), 
            &wave_hint, NULLREF(WaveVelocityHint), &position, &normal, NULL );
#else
        WaveFloaterArgs wfa = {
          (WaveQueryFlags) (WAVE_HINTGIVEN | WAVE_HINTSOUGHT | WAVE_NORMALSOUGHT), 
            &wave_hint, NULLREF(WaveVelocityHint), &position, &normal, NULL, };
#endif /* TARGET_XBOX JIV DEBUG */
        WAVE_TrackFloater (wfa);
      }
      else
      {
        CollideCallStruct collide (position, &normal, &current, NULL, &wave_hint);
        use_hint = WAVE_CheckCollision (collide, use_hint, false, false);
        position = collide.position;
      }
    }
    else
    {
      CollideCallStruct collide (position, &normal, &current, NULL, &wave_hint);
      use_hint = WAVE_CheckCollision (collide, use_hint, false, false);

      position += (collide.position - position) * water_interaction * dt;
      position += current * dt;

      normal = 40 * my_entity->get_rel_po ().get_y_facing () + normal;
      normal.normalize ();
    }
  }
  else
  {
    position.y = WATER_Altitude (position.x, position.z);
    WAVE_GlobalCurrent (&current);
    position += current * dt;
    WATER_Normal (position.x, position.z, normal.x, normal.y, normal.z);
    use_hint = false;
  }

  vector3d x, z;

  // make sure normal is valid (dc 10/30/01)
  assert(fabsf(normal.length() - 1) < 0.001f);

  x = cross (normal, my_entity->get_rel_po ().get_z_facing ());
  x.normalize();
  z = cross (x, normal);
  z.normalize ();

  po new_po (x, normal, z, position);

  if (water_interaction == 1)
  {
    // add a little bouncing up/down
    float dy = desired_dy - current_dy;

    if (__fabs (dy) < 0.03f)
    {
      // Should not make calls to rand() or srand() - rbroner
      //srand ((int) TIMER_GetTotalSec() * 100);  

      if (desired_dy > 0)
        desired_dy = -max_dy * random_r();
      else
        desired_dy = max_dy * random_r();
    }
    else
    {
      current_dy += dy * dt * speed_dy;
    }

    new_po.set_position (new_po.get_position () + vector3d (0, current_dy, 0));

    // add some tilting
    float da = desired_angle - current_angle;

    if (__fabs (da) < 1)
    {
      // Should not make calls to rand() or srand() - rbroner
	    //srand ((int) TIMER_GetTotalSec() * 100);

      if (desired_angle > 0)
        desired_angle = -max_angle * random_r();
      else
        desired_angle = max_angle * random_r();
    }
    else
    {
      current_angle += da * dt * speed_angle;
    }

    po rot_po;
    rot_po.set_rotate_x (DEG_TO_RAD (current_angle));

    my_entity->set_rel_po (rot_po * new_po);
  }
  else
    my_entity->set_rel_po (new_po);

  return water_object::update (dt);
}

void floating_object::collide (entity *ent, const vector3d& dir)
{
#ifdef SLALOM_TEST
  /*
  if (my_entity->get_parsed_name ().find ("buoy_b_slalom") != stringx::npos)
  {
    entity *ent = ((conglomerate *)my_entity)->get_member ("INBOUNDS");
    
    if (ent && g_world_ptr->entity_entity_collision_check (g_world_ptr->get_board_ptr (g_game_ptr->get_active_player ()), ent, 0))
    {
      if (slalom == SLALOM_NOTTESTED)
      {
        frontendmanager.IGO->Print("Buoy Passed");
        slalom = SLALOM_OK;
      }
    }
  }
  */
#endif
}

// ============================================================================
// static_object class

static_object::static_object (entity *ent, const stringx& path)
  : water_object (ent, path)
{
  dont_move = false;

  stringx anim = path + "\\ANIMATIONS\\" + ent->get_parsed_name ();

  if (file_finder_exists (anim, entity_track_tree::extension ()))
  {
    anim += entity_track_tree::extension ();
    my_entity->load_anim (anim);
    my_entity->play_anim (ANIM_PRIMARY, ent->get_parsed_name (), BLEND_TIME, ANIM_LOOPING | ANIM_TWEEN | ANIM_NONCOSMETIC);
  }
}

static_object::~static_object ()
{
}

bool static_object::parse_params (char** argp, int argc)
{
	stringx anim;

  if (!water_object::parse_params (argp, argc))
    return false;

  if (find_param (argp, argc, "dont_move"))
    dont_move = true;

	if (read_text_param (argp, argc, "play_animation", anim))
	{
		anim = my_path + "\\ANIMATIONS\\" + anim;

		if (file_finder_exists (anim, entity_track_tree::extension ()))
		{
			anim += entity_track_tree::extension ();
			my_entity->load_anim (anim);
			my_entity->play_anim (ANIM_PRIMARY, my_entity->get_parsed_name (), BLEND_TIME, ANIM_LOOPING | ANIM_TWEEN | ANIM_NONCOSMETIC);
		}
		else
			nglPrintf ("Cannot load animation %s.\n", anim.c_str ());
	}

  return true;
}

bool static_object::update (float dt)
{
  if (!dont_move)
  {
    vector3d current, position (my_entity->get_rel_position ());

    WAVE_GlobalCurrent (&current);
    position += current * dt;

    my_entity->set_rel_position (position);
  }

  return water_object::update (dt);
}

// ============================================================================
// surfing_object class

// boogieboarder
typedef enum
{
  BOOGIEBOARDER_STATE_PADDLE,
  BOOGIEBOARDER_STATE_LEFT,
  BOOGIEBOARDER_STATE_RIGHT,
  BOOGIEBOARDER_STATE_COLLIDE,
  BOOGIEBOARDER_STATE_BEHIND
} BOOGIEBOARDER_STATES;

typedef enum
{
  BOOGIEBOARDER_COLLIDE,
  BOOGIEBOARDER_COLLIDEI,
  BOOGIEBOARDER_IDLE_0,
  BOOGIEBOARDER_IDLE_1,
  BOOGIEBOARDER_JUMP,
  BOOGIEBOARDER_LTURN,
  BOOGIEBOARDER_RTURN,
  BOOGIEBOARDER_SPRAY,
  BOOGIEBOARDER_NUM_ANIMS
} BOOGIEBOARDER_ANIMS;

static stringx boogie1_anims[] =
{
  "BOARDER1_COLLIDE_1",
  "BOARDER1_COLLIDEI_1",
  "BOARDER1_IDLE_0",
  "BOARDER1_IDLE_1",
  "BOARDER1_JUMP_1",
  "BOARDER1_LTURN_1",
  "BOARDER1_RTURN_1",
  "BOARDER1_SPRAY_1"
};

static stringx boogie2_anims[] =
{
  "BOARDER2_COLLIDE_1",
  "BOARDER2_COLLIDEI_1",
  "BOARDER2_IDLE_0",
  "BOARDER2_IDLE_1",
  "BOARDER2_JUMP_1",
  "BOARDER2_LTURN_1",
  "BOARDER2_RTURN_1",
  "BOARDER2_SPRAY_1"
};

// misc surfer 1
typedef enum
{
  MISC_SURFER1_STATE_IDLE,
  MISC_SURFER1_STATE_PADDLE,
  MISC_SURFER1_STATE_TAKEOFF,
  MISC_SURFER1_STATE_SURF,
  MISC_SURFER1_STATE_CARVE_LEFT,
  MISC_SURFER1_STATE_CARVE_RIGHT,
  MISC_SURFER1_STATE_LEFT,
  MISC_SURFER1_STATE_RIGHT,
  MISC_SURFER1_STATE_COLLIDE,
  MISC_SURFER1_STATE_COLLIDE_FADE,
  MISC_SURFER1_STATE_TURN_DROPOUT,
  MISC_SURFER1_STATE_DROPOUT,
  MISC_SURFER1_STATE_FADE,
} MISC_SURFER1_STATES;

typedef enum
{
  MISC_SURFER1_STAND,
  MISC_SURFER1_IDLE,
  MISC_SURFER1_PADDLE,
  MISC_SURFER1_LIE_TO_STAND,
  MISC_SURFER1_STAND_TO_LIE,
  MISC_SURFER1_TURN_LEFT,
  MISC_SURFER1_TURN_RIGHT,
  MISC_SURFER1_WIPEOUT_FALL_BACK,
  MISC_SURFER1_WIPEOUT_FALL_FRONT,
  MISC_SURFER1_WIPEOUT_FALL_LEFT,
  MISC_SURFER1_WIPEOUT_FALL_RIGHT,
  MISC_SURFER1_WIPEOUT_HIT_BY_LIP,
  MISC_SURFER1_WIPEOUT_LYING,
  MISC_SURFER1_WIPEOUT_STEP_OFF,
  MISC_SURFER1_SWIM,
  MISC_SURFER1_NUM_ANIMS
} MISC_SURFER1_ANIMS;

static stringx misc_surfer1_anims[] =
{
  "MS_BAS_STAND",
  "MS_IDLE_LAYING",
  "MS_LIE_PADDEL",
  "MS_TRANS_LIE_TO_STAND",
  "MS_TRANS_STAND_LIE_1",
  "MS_TRN_L_1",
  "MS_TRN_R_1",
  "MS_WIP_FALL_B_1",
  "MS_WIP_FALL_F_1",
  "MS_WIP_FALL_L_1",
  "MS_WIP_FALL_R_1",
  "MS_WIP_HIT_BY_LIP",
  "MS_WIP_LYING_1",
  "MS_WIP_STEP_OFF",
  "MS_WIP_TREAD_1",
};

// misc surfer 2
typedef enum
{

  MISC_SURFER2_STATE_PADDLE,
} MISC_SURFER2_STATES;

typedef enum
{
  MISC_SURFER2_IDLE_LAYING,
  MISC_SURFER2_PADDLE,
  MISC_SURFER2_NUM_ANIMS
} MISC_SURFER2_ANIMS;

static stringx misc_surfer2_anims[] =
{
	"ALL_IDLE_LAYING",
  "ALL_LIE_PADDEL"
};

// kayaker
typedef enum
{
  KAYAKER_STATE_UP,       // going to the wave
  KAYAKER_STATE_DOWN,     // riding the wave
  KAYAKER_STATE_BEHIND,   // going behind the wave
  KAYAKER_STATE_HIT_LEFT,
  KAYAKER_STATE_HIT_RIGHT,
  KAYAKER_STATE_JUMP
} KAYAKER_STATES;

typedef enum
{
  KAYAKER_AMBIENT,
  KAYAKER_PADDLE,
  KAYAKER_HIT,
  KAYAKER_POST_HIT,
  KAYAKER_JUMP,
  KAYAKER_NUM_ANIMS
} KAYAKER_ANIMS;

// this has to match the outrigger since we use the same AI for both
static stringx kayaker_anims[] =
{
  "KAYAKER_IDLE_0",
  "KAYAKER_IDLE_1",
  "KAYAKER_COLLIDE_1",
  "KAYAKER_COLLIDEI_1",
  "KAYAKER_JUMP_1"
};

// this has to match the kayaker since we use the same AI for both
static stringx outrigger_anims[] =
{
  "OUTRIGGER_IDLE_0",
  "OUTRIGGER_IDLE_1",
  "OUTRIGGER_COLLIDE_1",
  "OUTRIGGER_COLLIDEI_1",
  "OUTRIGGER_JUMP_1"
};

// dolphin (moved to floatobj.h)
typedef enum
{
  DOLPHIN_SWIM,
  DOLPHIN_FLIP,
  DOLPHIN_JUMP,
  DOLPHIN_NUM_ANIMS
} DOLPHIN_ANIMS;

static stringx dolphin_anims[] =
{
  "DOLPHIN_SWIM",
  "DOLPHIN_FLIP",
  "DOLPHIN_JUMP",
};

// great white
typedef enum
{
  GREATWHITE_STATE_SWIM,
  GREATWHITE_STATE_SWIMRIGHT,
  GREATWHITE_STATE_SWIMLEFT,
  GREATWHITE_STATE_BITE,
  GREATWHITE_STATE_HIDDEN,
  GREATWHITE_STATE_PRESURFACE,
  GREATWHITE_STATE_SURFACE,
  GREATWHITE_STATE_SINK,
} GREATWHITE_STATES;

typedef enum
{
  GREATWHITE_SWIM,
  GREATWHITE_SWIMLEFT,
  GREATWHITE_SWIMRIGHT,
  GREATWHITE_BITE,
  GREATWHITE_NUM_ANIMS
} GREATWHITE_ANIMS;

static stringx greatwhite_anims[] =
{
  "GREATWHITE_SWIMCYCLE",
  "GREATWHITE_SWIMLEFT",
  "GREATWHITE_SWIMRIGHT",
  "GREATWHITE_BITE",
};

// windsurfer
typedef enum
{
	WINDSURFER_STATE_IDLE,
  WINDSURFER_STATE_LEFT,
  WINDSURFER_STATE_RIGHT,
	WINDSURFER_STATE_PREPARE_TRICK,
	WINDSURFER_STATE_TRICK,
  WINDSURFER_STATE_COLLIDE,
	WINDSURFER_STATE_COLLIDE_AIR,
  WINDSURFER_STATE_BEHIND
} WINDSURFER_STATES;

typedef enum
{
	WINDSURFER_AIR_TRICK,
	WINDSURFER_COLLIDE,
	WINDSURFER_COLLIDEI,
	WINDSURFER_COLLIDE_IN_AIR,
	WINDSURFER_COLLIDE_IN_AIRI,
	WINDSURFER_IDLE,
	WINDSURFER_JUMP,
	WINDSURFER_LTURN,
	WINDSURFER_RTURN,
	WINDSURFER_SPRAY,
	WINDSURFER_NUM_ANIMS
} WINDSURFER_ANIMS;

static stringx windsurfer1_anims[] =
{
	"WS1_AIR_TRICK_1",
	"WS1_COLLIDE_1",
	"WS1_COLLIDEI_1",
	"WS1_COLLIDE_IN_AIR_1",
	"WS1_COLLIDE_IN_AIRI_1",
	"WS1_IDLE_1",
	"WS1_JUMP_1",
	"WS1_LTURN_1",
	"WS1_RTURN_1",
	"WS1_SPRAY_1",
};

static stringx windsurfer1_board_anims[] =
{
	"WS1_BOARD_AIR_TRICK_1",
	"WS1_BOARD_COLLIDE_1",
	"WS1_BOARD_COLLIDEI_1",
	"WS1_BOARD_COLLIDE_IN_AIR_1",
	"WS1_BOARD_COLLIDE_IN_AIRI_1",
	"WS1_BOARD_IDLE_1",
	"WS1_BOARD_JUMP_1",
	"WS1_BOARD_LTURN_1",
	"WS1_BOARD_RTURN_1",
	"WS1_BOARD_SPRAY_1",
};

static stringx windsurfer2_anims[] =
{
	"WS2_AIR_TRICK_1",
	"WS2_COLLIDE_1",
	"WS2_COLLIDEI_1",
	"WS2_COLLIDE_IN_AIR_1",
	"WS2_COLLIDE_IN_AIRI_1",
	"WS2_IDLE_1",
	"WS2_JUMP_1",
	"WS2_LTURN_1",
	"WS2_RTURN_1",
	"WS2_SPRAY_1",
};

static stringx windsurfer2_board_anims[] =
{
	"WS2_BOARD_AIR_TRICK_1",
	"WS2_BOARD_COLLIDE_1",
	"WS2_BOARD_COLLIDEI_1",
	"WS2_BOARD_COLLIDE_IN_AIR_1",
	"WS2_BOARD_COLLIDE_IN_AIRI_1",
	"WS2_BOARD_IDLE_1",
	"WS2_BOARD_JUMP_1",
	"WS2_BOARD_LTURN_1",
	"WS2_BOARD_RTURN_1",
	"WS2_BOARD_SPRAY_1",
};

// jetskier
typedef enum
{
  JETSKIER_STATE_IDLE,
  JETSKIER_STATE_LEFT,
  JETSKIER_STATE_RIGHT,
  JETSKIER_STATE_COLLIDE,
  JETSKIER_STATE_BEHIND
} JETSKIER_STATES;

typedef enum
{
  JETSKIER_COLLIDE,
  JETSKIER_COLLIDEI,
  JETSKIER_IDLE,
  JETSKIER_JUMP,
  JETSKIER_LTURN,
  JETSKIER_RTURN,
  JETSKIER_SPRAY,
  JETSKIER_NUM_ANIMS
} JETSKIER_ANIMS;

static stringx jetskier_anims[] =
{
  "JETSKIER_COLLIDE_1",
  "JETSKIER_COLLIDEI_1",
  "JETSKIER_IDLE_1",
  "JETSKIER_JUMP_1",
  "JETSKIER_LTURN_1",
  "JETSKIER_RTURN_1",
  "JETSKIER_SPRAY_1"
};

static void set_conglom_texture (entity* c, int b)
{
  if (!c)
    return;

  if (c->has_link_ifc())
  {
    link_interface* li = c->link_ifc();
    entity* c1 = (entity*) li->get_first_child();
    while(c1)
    {
      set_conglom_texture (c1, b);
      c1 = (entity*) c1->link_ifc()->get_next_sibling();
    }
    c->SetTextureFrame(b);
  }
}

surfing_object::surfing_object (entity *ent, const stringx& path, const stringx& name)
  : water_object (ent, path)
{
  int i;

  my_trail = NULL;
  my_board_entity = NULL;
  my_num_anims = 0;
  my_name_anims = NULL;
  my_board_num_anims = 0;
	my_board_name_anims = 0;
  my_anim_handler = NULL;
  my_base_name = name;
	my_third_entity = NULL;

  if (path.find ("misc_surfer1") != stringx::npos)
  {
    int outfit;

	// These need to go in the beach database! (dc 01/29/02)
    switch (g_game_ptr->get_beach_id())
    {
      case BEACH_ANTARCTICA : outfit = 1; break;
      case BEACH_BELLS : outfit = 1; break;
      case BEACH_CORTESBANK : outfit = 1; break;
      case BEACH_GLAND : outfit = 0; break;
      case BEACH_JAWS : outfit = 0; break;
      case BEACH_JEFFERSONBAY : outfit = 1; break;
      case BEACH_MAVERICKS : outfit = 1; break;
      case BEACH_MUNDAKA : outfit = 1; break;
      case BEACH_PIPELINE : outfit = 0; break;
      case BEACH_SEBASTIAN : outfit = 0; break;
      case BEACH_TEAHUPOO : outfit = 0; break;
      case BEACH_TRESTLES : outfit = 0; break;
      case BEACH_INDOOR : outfit = 0; break;
      case BEACH_COSMOS : outfit = 1; break;
      default : nglPrintf ("Unknown beach !\n"); assert (false); return;	// otherwise outfit possibly uninitialized (dc 01/29/02)
    }

    int texture = outfit + 2 * random_r(3);
    my_entity->SetTextureFrame (texture);

    my_num_anims = MISC_SURFER1_NUM_ANIMS;
    my_name_anims = misc_surfer1_anims;
    ai_func = &surfing_object::surfer1_ai;
    my_type = MISC_SURFER1;

    stringx board_path = path + "\\entities\\misc_surfer1_board";
    my_board_entity = g_entity_maker->create_entity_or_subclass (board_path, entity_id::make_unique_id (), po_identity_matrix, empty_string, entity::ACTIVE, NULL);
    my_board_entity->set_visible (false);
    set_conglom_texture (my_board_entity, texture);
  }
  else if (path.find ("misc_surfer2") != stringx::npos)
  {
    int outfit;

	// These need to go in the beach database! (dc 01/29/02)
    switch (g_game_ptr->get_beach_id())
    {
      case BEACH_ANTARCTICA : outfit = 1; break;
      case BEACH_BELLS : outfit = 1; break;
      case BEACH_CORTESBANK : outfit = 1; break;
      case BEACH_GLAND : outfit = 0; break;
      case BEACH_JAWS : outfit = 0; break;
      case BEACH_JEFFERSONBAY : outfit = 1; break;
      case BEACH_MAVERICKS : outfit = 1; break;
      case BEACH_MUNDAKA : outfit = 1; break;
      case BEACH_PIPELINE : outfit = 0; break;
      case BEACH_SEBASTIAN : outfit = 0; break;
      case BEACH_TEAHUPOO : outfit = 0; break;
      case BEACH_TRESTLES : outfit = 0; break;
      case BEACH_INDOOR : outfit = 0; break;
      case BEACH_COSMOS : outfit = 1; break;
      default : nglPrintf ("Unknown beach !\n"); assert (false); return;	// otherwise outfit possibly uninitialized (dc 01/29/02)
    }
		int texture = outfit * 2 + random_r(2);
    my_entity->SetTextureFrame (texture);

    my_num_anims = MISC_SURFER2_NUM_ANIMS;
    my_name_anims = misc_surfer2_anims;
    ai_func = &surfing_object::surfer2_ai;
    my_type = MISC_SURFER2;

    stringx board_path = "items\\misc_surfer2\\entities\\misc_surfer2_board";
    my_board_entity = g_entity_maker->create_entity_or_subclass (board_path, entity_id::make_unique_id (), po_identity_matrix, empty_string, entity::ACTIVE, NULL);
    my_board_entity->set_visible (false);
    set_conglom_texture (my_board_entity, texture);

  }
  else if (path.find ("boogieboarder") != stringx::npos)
  {
    my_num_anims = BOOGIEBOARDER_NUM_ANIMS;

		if (path.find ("boogieboarder1") != stringx::npos)
		  my_name_anims = boogie1_anims;
		else
	    my_name_anims = boogie2_anims;

    ai_func = &surfing_object::boogie_ai;
    my_type = BOOGIEBOARDER;
  }
  else if (path.find ("kayaker") != stringx::npos)
  {
    my_num_anims = KAYAKER_NUM_ANIMS;
    my_name_anims = kayaker_anims;
    ai_func = &surfing_object::kayaker_ai;
    my_type = KAYAKER;
  }
  else if (path.find ("outrigger") != stringx::npos)
  {
    my_num_anims = KAYAKER_NUM_ANIMS;
    my_name_anims = outrigger_anims;
    ai_func = &surfing_object::kayaker_ai;
    my_type = OUTRIGGER;
  }
  else if (path.find ("fatbastard") != stringx::npos)
  {
    ai_func = &surfing_object::fatbastard_ai;
    my_type = FATBASTARD;
    my_anim_handler = NEW generic_anim_misc (my_entity, path, name);
  }
  else if (path.find ("jetskier") != stringx::npos)
  {
    ai_func = &surfing_object::jetskier_ai;
    my_type = JETSKIER;
		my_num_anims = JETSKIER_NUM_ANIMS;
		my_name_anims = jetskier_anims;
  }
  else if (path.find ("wind_surfer") != stringx::npos)
  {
		stringx board_path;

		my_num_anims = WINDSURFER_NUM_ANIMS;
		my_board_num_anims = WINDSURFER_NUM_ANIMS;

		if (path.find ("wind_surfer1") != stringx::npos)
		{
	    my_name_anims = windsurfer1_anims;
	    my_board_name_anims = windsurfer1_board_anims;
			board_path = path + "\\entities\\wind_surfer1_board";
		}
		else
		{
	    my_name_anims = windsurfer2_anims;
	    my_board_name_anims = windsurfer2_board_anims;
			board_path = path + "\\entities\\wind_surfer2_board";
		}

		my_board_entity = g_entity_maker->create_entity_or_subclass (board_path, entity_id::make_unique_id (), po_identity_matrix, empty_string, entity::ACTIVE, NULL);
		if (my_board_entity != NULL)
			my_board_entity->set_visible (false);

    ai_func = &surfing_object::windsurfer_ai;
    my_type = WINDSURFER;
  }
  else if (path.find ("cameraman") != stringx::npos)
  {
    ai_func = &surfing_object::cameraman_ai;
    my_type = CAMERAMAN;
    my_anim_handler = NEW generic_anim_misc (my_entity, path, name);
  }
  else if (path.find ("snorkeler") != stringx::npos)
  {
    ai_func = &surfing_object::snorkeler_ai;
    my_type = SNORKELER;
    my_anim_handler = NEW generic_anim_misc (my_entity, path, name);
  }
  else if (path.find ("swimmer") != stringx::npos)
  {
    ai_func = &surfing_object::swimmer_ai;
    my_type = SWIMMER;
    my_anim_handler = NEW generic_anim_misc (my_entity, path, name);
  }
  else if (path.find ("iceberg") != stringx::npos)
  {
		ai_func = &surfing_object::icepatch_ai;
		my_type = ICEPATCH;
		my_anim_handler = NEW generic_anim_ice (my_entity, path, name);
  }
  else if (path.find ("kelp") != stringx::npos)
  {
		ai_func = &surfing_object::kelp_ai;
		my_type = KELP;
		my_anim_handler = NEW generic_anim_ice (my_entity, path, name);
  }
  else if (path.find ("pancake") != stringx::npos)
  {
		ai_func = &surfing_object::icepatch_ai;
		my_type = ICEPATCH;
		my_anim_handler = NEW generic_anim_ice (my_entity, path, name);
  }
  else if (path.find ("fisherman") != stringx::npos)
  {
    ai_func = &surfing_object::fisherman_ai;
    my_type = FISHERMAN;

    stringx board_path = path + "\\entities\\fisherman_boat";
    my_board_entity = g_entity_maker->create_entity_or_subclass (board_path, entity_id::make_unique_id (), po_identity_matrix, empty_string, entity::ACTIVE, NULL);
    my_board_entity->set_visible (false);

		entity* ents[] = { my_entity, my_board_entity };
		const char *names[] = { "", "BOAT" };
    my_anim_handler = NEW generic_anim_misc (ents, path, name, names, 2);
  }
  else if (path.find ("dingy") != stringx::npos)
  {
    ai_func = &surfing_object::dingy_ai;
    my_type = DINGY;

    stringx board_path = path + "\\entities\\dingy_driver";
    my_board_entity = g_entity_maker->create_entity_or_subclass (board_path, entity_id::make_unique_id (), po_identity_matrix, empty_string, entity::ACTIVE, NULL);
	if (my_board_entity)
	    my_board_entity->set_visible (false);

    stringx third_path = path + "\\entities\\dingy_cameraman";
    my_third_entity = g_entity_maker->create_entity_or_subclass (third_path, entity_id::make_unique_id (), po_identity_matrix, empty_string, entity::ACTIVE, NULL);
	if (my_third_entity)
	    my_third_entity->set_visible (false);

		entity* ents[] = { my_entity, my_board_entity, my_third_entity };
		const char *names[] = { "", "DRIVER", "CAMERAMAN" };
    my_anim_handler = NEW generic_anim_misc (ents, path, name, names, 3);
  }
  else if (path.find ("helicopter") != stringx::npos)
  {
    ai_func = &surfing_object::helicopter_ai;
    my_type = HELICOPTER;
    my_anim_handler = NEW generic_anim_animal (my_entity, path, name);
  }
  else if (path.find ("humpback") != stringx::npos)
  {
    ai_func = &surfing_object::humpback_ai;
    my_type = HUMPBACK;
    my_anim_handler = NEW generic_anim_animal (my_entity, path, name);
  }
  else if (path.find ("turtle") != stringx::npos)
  {
    ai_func = &surfing_object::turtle_ai;
    my_type = TURTLE;
    my_anim_handler = NEW generic_anim_animal (my_entity, path, name);
  }
  else if (path.find ("dolphin") != stringx::npos)
  {
    ai_func = &surfing_object::dolphin_ai;
    my_type = DOLPHIN;
    my_anim_handler = NEW generic_anim_animal (my_entity, path, name);
  }
  else if (path.find ("greatwhite") != stringx::npos)
  {
    ai_func = &surfing_object::greatwhite_ai;
    my_type = GREATWHITE;
    my_anim_handler = NEW generic_anim_animal (my_entity, path, name);
  }
  else if (path.find ("seal") != stringx::npos)
  {
    ai_func = &surfing_object::seal_ai;
    my_type = SEAL;
    my_anim_handler = NEW generic_anim_animal (my_entity, path, name);
  }
  else if (path.find ("hammerhead") != stringx::npos)
  {
    ai_func = &surfing_object::hammerhead_ai;
    my_type = HAMMERHEAD;
    my_anim_handler = NEW generic_anim_animal (my_entity, path, name);
  }
  else if (path.find ("mantaray") != stringx::npos)
  {
    ai_func = &surfing_object::mantaray_ai;
    my_type = MANTARAY;
    my_anim_handler = NEW generic_anim_animal (my_entity, path, name);
  }
  else if (path.find ("seagull") != stringx::npos)
  {
    ai_func = &surfing_object::seagull_ai;
    my_type = SEAGULL;
    my_anim_handler = NEW generic_anim_animal (my_entity, path, name);
  }
  else
  {
    nglPrintf ("Unknown AI for surfing object, using dummy.\n");

    my_num_anims = 0;
    my_name_anims = NULL;
    ai_func = &surfing_object::dummy_ai;
    my_type = DUMMY;
  }

  for (i = 0; i < my_num_anims; i++)
  {
    stringx anim = path + "\\ANIMATIONS\\" + my_name_anims[i];
		bool play = true;

    if (file_finder_exists (anim, entity_track_tree::extension ()))
    {
      anim += entity_track_tree::extension ();
      my_entity->load_anim (anim);

			if (play)
			{
		    my_entity->play_anim (ANIM_PRIMARY, anim, 0, ANIM_NONCOSMETIC);
				play = false;
			}
    }
    else
    {
      nglPrintf ("Missing animation: %s\n", anim.c_str());
    }
  }

  for (i = 0; i < my_board_num_anims; i++)
  {
    stringx anim = path + "\\ANIMATIONS\\" + my_board_name_anims[i];
		bool play = true;

    if (file_finder_exists (anim, entity_track_tree::extension ()))
    {
      anim += entity_track_tree::extension ();
      my_board_entity->load_anim (anim);

			if (play)
			{
		    my_board_entity->play_anim (ANIM_PRIMARY, anim, 0, ANIM_NONCOSMETIC);
				play = false;
			}
    }
    else
    {
      nglPrintf ("Missing animation: %s\n", anim.c_str());
    }
  }

#ifdef DEBUG
	if (my_anim_handler == NULL)
	{
		stringx none = "NONE";
		color32 yel = color32(255, 255, 0, 255);
		anim_label = new FloatingText(&frontendmanager.font_info, none, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, yel);
	}
	else
		anim_label = NULL;
#endif
}

surfing_object::~surfing_object ()
{
  despawn ();
  delete my_anim_handler;

#ifdef DEBUG
  delete anim_label;
#endif
}

bool surfing_object::parse_params (char** argp, int argc)
{
  if (!water_object::parse_params (argp, argc))
    return false;

  if (find_param (argp, argc, "dummy"))
	{
    ai_func = &surfing_object::dummy_ai;
		if (my_anim_handler != NULL)
			my_anim_handler->set_dummy ();
	}

  return true;
}
static int total_misc_surfers = 0;
// From board.cpp

void surfing_object::spawn ()
{
  if (times_spawned == spawn_count)
    return;

//  bool first = first_spawn;
  int intial_anim = 0;

  timer = 0;
  turn_rate = DEG_TO_RAD (90);
  turn_amount = 0;
  lean_amount = 0;
  tilt_amount = 0;
  velocity = ZEROVEC;
	mySound=-1;
  water_object::spawn ();
  if (my_anim_handler != NULL)
    my_anim_handler->spawn ();

  switch (my_type)
  {
    case MISC_SURFER1:
    {
      intial_anim = MISC_SURFER2_IDLE_LAYING;
//      tilt_amount = DEG_TO_RAD (12.5f);
      my_state = my_previous_state = MISC_SURFER1_STATE_PADDLE;

      // turn around
//      po turn;
//      turn.set_rot (YVEC, DEG_TO_RAD(90));
//      my_entity->set_rel_po(turn);


      my_entity->set_rel_position(my_initial_po.get_position());

      my_board_entity->set_rel_po (my_entity->get_rel_po ());
      my_board_entity->set_visible (true);
      my_board_entity->update_region ();
    } break;

    case MISC_SURFER2:
    {
			po turn, my_po;
			vector3d surferPosition = g_game_ptr->calc_beach_spawn_pos();;

			float turnangle;
			if (BeachDataArray[g_game_ptr->get_beach_id()].bdir)
			{
				turnangle = -.5f;
			}
			else
			{
				turnangle = -2.5f;
			}
			// random delay between 0 and 1
			my_idle_delay = random_r();
			// A little variance (between -.125 and .125)
			turnangle += random_r(-0.125f, 0.125f);
			turn.set_rot(YVEC, turnangle);

			set_conglom_texture(my_entity, total_misc_surfers*2);

			my_po = my_initial_po;
			my_po = turn*my_po;
			total_misc_surfers++;

      intial_anim = MISC_SURFER2_IDLE_LAYING;
      
      my_state = my_previous_state = MISC_SURFER2_STATE_PADDLE;
			
			my_entity->set_rel_position(my_initial_po.get_position() + surferPosition);
			my_entity->set_rel_po(turn*my_entity->get_rel_po());
			my_board_entity->set_rel_po (my_entity->get_rel_po ());
			//my_board_entity->set_rel_po (my_board_entity->set_rel_po (
			
      my_board_entity->set_visible (true);
			my_board_entity->update_region ();

    } break;

    case BOOGIEBOARDER:
    {
      tilt_amount = DEG_TO_RAD (12.5f);
      my_state = my_previous_state = BOOGIEBOARDER_STATE_PADDLE;

      // turn around
      po turn;
      turn.set_rot (YVEC, DEG_TO_RAD(180));
      my_entity->set_rel_po(turn);
      my_entity->set_rel_position(my_initial_po.get_position());

			intial_anim = BOOGIEBOARDER_IDLE_0;
    } break;

    case JETSKIER:
    {
			mySound = SoundScriptManager::inst()->startEvent(SS_JETSKI, this->get_entity());
      my_state = my_previous_state = JETSKIER_STATE_IDLE;

      // turn around
      po turn;
      turn.set_rot (YVEC, DEG_TO_RAD(180));
      my_entity->set_rel_po(turn);
      my_entity->set_rel_position(my_initial_po.get_position());

			intial_anim = JETSKIER_IDLE;
    } break;

    case WINDSURFER:
		{
      my_state = my_previous_state = WINDSURFER_STATE_IDLE;
			mySound = SoundScriptManager::inst()->startEvent(SS_WINDSURFER, this->get_entity());

      // turn around
      po turn;
      turn.set_rot (YVEC, DEG_TO_RAD(180));
      my_entity->set_rel_po(turn);
      my_entity->set_rel_position(my_initial_po.get_position());

			if (my_board_entity)
			{
	      my_board_entity->set_rel_po (my_entity->get_rel_po ());
		    my_board_entity->set_visible (true);
			  my_board_entity->update_region ();
			}

			intial_anim = WINDSURFER_IDLE;
    } break;
	
    case KAYAKER:
    case OUTRIGGER:
    {
      my_state = my_previous_state = KAYAKER_STATE_UP;

			mySound = SoundScriptManager::inst()->startEvent(SS_KAYAK, this->get_entity());

      // turn around
      po turn;
      turn.set_rot (YVEC, DEG_TO_RAD(180));
      my_entity->set_rel_po (turn);
      my_entity->set_rel_position(my_initial_po.get_position());
    } break;

		case KELP:
		case ICEPATCH:
    case SEAGULL:
    case FATBASTARD:
    case CAMERAMAN:
		case SNORKELER:
    case HAMMERHEAD:
    case SEAL:
    case HUMPBACK:
    case HELICOPTER:
    case MANTARAY:
    case SWIMMER:
      my_state = my_previous_state = 0;
      break;

    case DOLPHIN:
    {
      my_state = my_previous_state = DOLPHIN_STATE_HIDDEN;

//      po turn;

//      if (BeachDataArray[g_game_ptr->get_beach_id()].bdir)
//        turn.set_rot (YVEC, DEG_TO_RAD(135));
//      else
//        turn.set_rot (YVEC, DEG_TO_RAD(225));

//      my_entity->set_rel_po (turn);
      my_entity->set_rel_position(my_initial_po.get_position());

      extra_turn = total_extra_turn = 0;
      offset = ZEROVEC;
    } break;

    case GREATWHITE:
    {
//      if (first)
        my_state = my_previous_state = GREATWHITE_STATE_HIDDEN;
//      else
//        my_state = my_previous_state = GREATWHITE_STATE_PRESURFACE;
//      po turn;
/*
      if (BeachDataArray[g_game_ptr->get_beach_id ()].bdir)
        turn.set_rot (YVEC, DEG_TO_RAD(90));
      else
        turn.set_rot (YVEC, DEG_TO_RAD(270));
*/
//      turn.set_rot (YVEC, DEG_TO_RAD (180));

//      my_entity->set_rel_po (turn);
      my_entity->set_rel_position(my_initial_po.get_position());

      extra_turn = total_extra_turn = 0;
      offset = ZEROVEC;
      timer = 1;
    } break;

    case FISHERMAN:
    {
			if (my_board_entity)
			{
	      my_board_entity->set_rel_po (my_entity->get_rel_po ());
		    my_board_entity->set_visible (true);
			  my_board_entity->update_region ();
			}
    } break;

    case DINGY:
    {
			if (my_board_entity)
			{
	      my_board_entity->set_rel_po (my_entity->get_rel_po ());
		    my_board_entity->set_visible (true);
			  my_board_entity->update_region ();
			}

			if (my_third_entity)
			{
	      my_third_entity->set_rel_po (my_entity->get_rel_po ());
		    my_third_entity->set_visible (true);
			  my_third_entity->update_region ();
			}
    } break;
  }

  if (my_anim_handler == NULL && my_name_anims)
    my_entity->play_anim (ANIM_PRIMARY, my_name_anims[intial_anim], 0, ANIM_LOOPING |  ANIM_NONCOSMETIC);

  if (my_anim_handler == NULL && my_board_entity && my_board_name_anims)
    my_board_entity->play_anim (ANIM_PRIMARY, my_board_name_anims[intial_anim], 0, ANIM_LOOPING |  ANIM_NONCOSMETIC);
}

void surfing_object::despawn ()
{
  if (my_trail)
  {
    ks_fx_trail_destroy (my_trail);
    my_trail = NULL;
  }
	
	if (mySound >= 0)
	{
		SoundScriptManager::inst()->endEvent(mySound);
	}

  if (my_board_entity)
  {
    my_board_entity->set_render_color (ren_col);
    my_board_entity->set_visible (false);
    my_board_entity->set_active (false);
  }

	if (my_third_entity)
  {
    my_third_entity->set_render_color (ren_col);
    my_third_entity->set_visible (false);
    my_third_entity->set_active (false);
  }

  water_object::despawn ();
}

bool surfing_object::update (float dt)
{
  vector3d position, normal;

  position = my_entity->get_rel_position ();

  if (my_entity->has_physical_ifc ())
    my_entity->physical_ifc ()->disable ();

  if (my_board_entity && my_board_entity->has_physical_ifc ())
    my_board_entity->physical_ifc ()->disable ();

  if (my_third_entity && my_third_entity->has_physical_ifc ())
    my_third_entity->physical_ifc ()->disable ();

  if (!((this->*ai_func) (position, normal, dt)))
    return false;

  if (my_anim_handler != NULL)
    my_anim_handler->update (false, false, false, &my_max_alpha);

  vector3d x, z;

  // rotate if needed
  if (__fabs (turn_amount) > DEG_TO_RAD (5))
  {
    float angle = turn_rate * dt;
    po rot;

    if (turn_amount < 0)
      angle = -angle;

    rot.set_rot (my_entity->get_rel_po ().get_y_facing (), angle);
    my_entity->set_rel_po (rot * my_entity->get_rel_po ());

    turn_amount -= angle;
  }
  else
    turn_amount = 0;

  // make sure ai_func filled in normal with something valid (dc 10/30/01)
  assert(fabsf(normal.length() - 1) < 0.001f);

  x = cross (normal, my_entity->get_rel_po ().get_z_facing ());
  x.normalize();
  z = cross (x, normal);
  z.normalize ();

  my_entity->set_rel_po (po (x, normal, z, position));
/*
  // lean if needed
  if (__fabs (lean_amount) > DEG_TO_RAD (5))
  {
    po rot;

    rot.set_rotate_z (lean_amount);
    my_entity->set_rel_po (rot * my_entity->get_rel_po ());
  }

  // tilt if needed
  if (__fabs (tilt_amount) > DEG_TO_RAD (5))
  {
    po rot;

    rot.set_rot (my_entity->get_rel_po ().get_x_facing (), tilt_amount);
    my_entity->set_rel_po (rot * my_entity->get_rel_po ());
  }
*/
	if (my_board_entity != NULL)
	{
	  if (ai_func == &surfing_object::dummy_ai)
		  my_board_entity->set_rel_po (my_entity->get_rel_po ());

		my_board_entity->set_render_color (my_entity->get_render_color ());
	}

	if (my_third_entity != NULL)
	{
	  if (ai_func == &surfing_object::dummy_ai)
		  my_third_entity->set_rel_po (my_entity->get_rel_po ());

		my_third_entity->set_render_color (my_entity->get_render_color ());
	}

  return water_object::update (dt);
}

#ifdef DEBUG
void surfing_object::draw_debug_labels()
{
	if (my_anim_handler != NULL)
		my_anim_handler->draw_debug_labels ();
	else
	{
		if (my_entity->get_anim_tree (ANIM_PRIMARY))
			anim_label->changeText(my_entity->get_anim_tree (ANIM_PRIMARY)->get_name());
		anim_label->SetLocation3D(my_entity->get_abs_position());
		anim_label->UpdateInScene(true);
		anim_label->Draw();
	}
}
#endif

void surfing_object::wave_check_collision (vector3d& position, vector3d& normal, vector3d *current, WaveRegionEnum *region)
{
  if ((position.x < WAVE_MeshMaxX) && (position.x > WAVE_MeshMinX) &&
      (position.z < WAVE_MeshMaxZ) && (position.z > WAVE_MeshMinZ))
  {
		current_x_mult = g_current_x_mult;
    CollideCallStruct collide (position, &normal, current, region, &wave_hint);
		current->x *= current_x_mult;
    use_hint = WAVE_CheckCollision (collide, use_hint, use_hint, use_hint);
    position = collide.position;
  }
  else
  {
    position.y = WATER_Altitude (position.x, position.z);

    WATER_Normal (position.x, position.z, normal.x, normal.y, normal.z);

    use_hint = false;

    if (current)
      WAVE_GlobalCurrent (current);

    if (region)
      *region = WAVE_REGIONBACK;
  }
}

bool surfing_object::dummy_ai (vector3d& position, vector3d& normal, float dt)
{
  if (my_anim_handler != NULL)
    my_anim_handler->switch_anims ();
	else
	{
		input_mgr* inputmgr = input_mgr::inst();
	  int cur_anim;

		for (cur_anim = 0; cur_anim < my_num_anims; cur_anim++)
		{
//			entity_track_tree* track = NULL;
			stringx fullname;

			fullname = my_name_anims[cur_anim];

			if (my_entity->get_anim_tree (ANIM_PRIMARY)->get_name() == fullname)
				break;
		}

		if (inputmgr->get_control_state(JOYSTICK_DEVICE, PSX_L1) == AXIS_MAX)
		{
			cur_anim--;

			if (cur_anim < 0)
				cur_anim = my_num_anims - 1;

			my_entity->play_anim (ANIM_PRIMARY, my_name_anims[cur_anim], BLEND_TIME, ANIM_LOOPING | ANIM_TWEEN | ANIM_NONCOSMETIC);
		}
		else if (inputmgr->get_control_state(JOYSTICK_DEVICE, PSX_R1) == AXIS_MAX)
		{
			cur_anim++;

			if (cur_anim == my_num_anims)
				cur_anim = 0;

			my_entity->play_anim(ANIM_PRIMARY, my_name_anims[cur_anim], BLEND_TIME, ANIM_LOOPING | ANIM_TWEEN | ANIM_NONCOSMETIC);
		}

		if (my_board_entity != NULL)
			my_board_entity->play_anim (ANIM_PRIMARY, my_board_name_anims[cur_anim], BLEND_TIME, ANIM_LOOPING | ANIM_TWEEN | ANIM_NONCOSMETIC);
	}

  if ((position.x < WAVE_MeshMaxX) && (position.x > WAVE_MeshMinX) &&
      (position.z < WAVE_MeshMaxZ) && (position.z > WAVE_MeshMinZ))
  {
    CollideCallStruct collide (position, &normal, NULL, NULL, &wave_hint);
    use_hint = WAVE_CheckCollision (collide, use_hint, false, false);
    position = collide.position;
  }
  else
  {
    position.y = WATER_Altitude (position.x, position.z);
    WATER_Normal (position.x, position.z, normal.x, normal.y, normal.z);
    use_hint = false;
  }

  return true;
}

bool surfing_object::floating_ai (vector3d& position, vector3d& normal, float dt)
{
  // make the object flow with the water
  if ((position.x < WAVE_MeshMaxX) && (position.x > WAVE_MeshMinX) &&
      (position.z < WAVE_MeshMaxZ) && (position.z > WAVE_MeshMinZ))
  {
    vector3d current;
    CollideCallStruct collide (position, &normal, &current, NULL, &wave_hint);
    use_hint = WAVE_CheckCollision (collide, use_hint, false, false);
		current.x *= current_x_mult;

    position = collide.position + current * dt;
  }
  else
  {
    vector3d current;

    position.y = WATER_Altitude (position.x, position.z);
    WAVE_GlobalCurrent (&current);
    WATER_Normal (position.x, position.z, normal.x, normal.y, normal.z);

    position += current * dt;
    use_hint = false;
  }

  return true;
}

bool surfing_object::boogie_ai (vector3d& position, vector3d& normal, float dt)
{
  // FIXME: find out why the lights are not being updated correctly
  light_manager *lmr = ((conglomerate*)my_entity)->get_light_set();

  if (lmr != NULL)
  {
    lmr->goal_ambient = color_white;
    lmr->last_ambient = color_white;
  }

#if 0
	// temp
  return floating_ai (position, normal, dt);
#else
  WaveRegionEnum region;
  vector3d current;
  int enter_state = my_state;
	bool add_trail = false;

  switch (my_state)
  {
    case BOOGIEBOARDER_STATE_PADDLE:
    case BOOGIEBOARDER_STATE_COLLIDE:
    case BOOGIEBOARDER_STATE_BEHIND:
    {
      if ((position.x < WAVE_MeshMaxX) && (position.x > WAVE_MeshMinX) &&
          (position.z < WAVE_MeshMaxZ) && (position.z > WAVE_MeshMinZ))
      {
        vector3d last_position (position);

				CollideCallStruct collide (position, &normal, &current, &region, &wave_hint);
				use_hint = WAVE_CheckCollision (collide, use_hint, false, false);
				position = collide.position;
				current.x *= 0.5f;
				position += current * dt;

				if (my_state == BOOGIEBOARDER_STATE_BEHIND)
					add_trail = true;

				if (my_state == BOOGIEBOARDER_STATE_COLLIDE)
				{
	        entity_anim_tree* anim_tree = my_entity->get_anim_tree (ANIM_PRIMARY);

	        if (anim_tree->is_finished () && anim_tree->is_done_tween ())
					{
						my_state = BOOGIEBOARDER_STATE_BEHIND;
						my_entity->play_anim (ANIM_PRIMARY, my_name_anims[BOOGIEBOARDER_COLLIDEI], BLEND_TIME, ANIM_TWEEN | ANIM_NONCOSMETIC);
					}
				}

        if ((my_state == BOOGIEBOARDER_STATE_PADDLE) &&
            (region == WAVE_REGIONPOCKET))
        {
/*
          // Should not make calls to rand() or srand() - rbroner
      	  //srand ((int) TIMER_GetTotalSec() * 100);

          // 50% chance the boogieboarder will not drop the wave
          if (random_r() < 0.3f)
          {
            my_state = BOOGIEBOARDER_STATE_BEHIND;

            // turn him around
            if (random_r() < 0.5f)
              turn_amount = DEG_TO_RAD (90);
            else
              turn_amount = DEG_TO_RAD (-90);
          }
          else
*/
          {
        	  if (BeachDataArray[g_game_ptr->get_beach_id ()].bdir)
            {
              my_state = BOOGIEBOARDER_STATE_LEFT;
              turn_amount = DEG_TO_RAD (-70);

//              if (velocity.x > 0)
//                velocity.x = -velocity.x;
              tilt_amount = 0;

              my_entity->play_anim (ANIM_PRIMARY, my_name_anims[BOOGIEBOARDER_LTURN], BLEND_TIME, ANIM_LOOPING | ANIM_TWEEN | ANIM_NONCOSMETIC);
            }
            else
            {
              my_state = BOOGIEBOARDER_STATE_RIGHT;
              turn_amount = DEG_TO_RAD (70);

//              if (velocity.x < 0)
//                velocity.x = -velocity.x;
              tilt_amount = 0;

              my_entity->play_anim (ANIM_PRIMARY, my_name_anims[BOOGIEBOARDER_RTURN], BLEND_TIME, ANIM_LOOPING | ANIM_TWEEN | ANIM_NONCOSMETIC);
            }

            timer2 = 0;
            velocity = (position - last_position) / (1.25f * dt);
            timer = 3 + 2 * random_r();

            my_trail = ks_fx_trail_create (10, 2, 0, NULL);
          }
        }
      }
      else
      {
        position.y = WATER_Altitude (position.x, position.z);
        WAVE_GlobalCurrent (&current);
        position += current * dt;
        WATER_Normal (position.x, position.z, normal.x, normal.y, normal.z);
        use_hint = false;
      }
    } break;

    case BOOGIEBOARDER_STATE_LEFT:
    case BOOGIEBOARDER_STATE_RIGHT:
    {
      if ((position.x < WAVE_MeshMaxX) && (position.x > WAVE_MeshMinX) &&
          (position.z < WAVE_MeshMaxZ) && (position.z > WAVE_MeshMinZ))
      {
        vector3d current, forces (ZEROVEC);
//        extern float water_friction;

        CollideCallStruct collide (position, &normal, &current, NULL, &wave_hint);
        use_hint = WAVE_CheckCollision (collide, use_hint, false, false);
        position = collide.position;

        position += velocity * dt;

/*
        if (timer < 0)
        {
          // stop riding the wave

        }
        else
				*/
        {
          // HACK HACK HACK HACK (don't tell anyone and they won't notice)
          current.x *= 0.5f;

          vector3d relative_velocity = velocity - current;
          position = collide.position;

					const float boogie_weight = 3;
					const float boogie_force = 8;
					const float boogie_friction = -0.7f;
					const float boogie_skag = -4;

          // projection of the weight over the normal vector
          float scale = dot (normal, -YVEC * boogie_weight * world_gravity);
          if (scale < 0)
            scale = -scale;

          // fast physics: normal force, weight and water friction
          forces += (normal * scale);
          forces -= YVEC * world_gravity;
          forces += relative_velocity * boogie_friction;
          forces += (relative_velocity - my_entity->get_rel_po ().get_z_facing () * dot (relative_velocity, my_entity->get_rel_po ().get_z_facing ())) * boogie_skag;

					const float boogie_x = 0.5f;
					forces.x *= boogie_x;

					if (timer > 0)
						forces += my_entity->get_rel_po ().get_z_facing () * boogie_force;

          timer -= dt;

          if (timer < 0)
          {
        	  if (BeachDataArray[g_game_ptr->get_beach_id ()].bdir)
              turn_amount = DEG_TO_RAD (-90);
            else
              turn_amount = DEG_TO_RAD (90);

            tilt_amount = DEG_TO_RAD (12.5f);
            lean_amount = 0;
						my_state = BOOGIEBOARDER_STATE_BEHIND;
            my_entity->play_anim (ANIM_PRIMARY, my_name_anims[BOOGIEBOARDER_IDLE_1], BLEND_TIME, ANIM_LOOPING | ANIM_TWEEN | ANIM_NONCOSMETIC);
          }

          velocity += forces * dt;

          // adjust the momentum to the wave surface
          scale = velocity.length ();

          velocity -= normal * dot (normal, velocity);
          velocity.normalize ();
          velocity *= scale;
        }

        if (BeachDataArray[g_game_ptr->get_beach_id ()].bdir)
          lean_amount = DEG_TO_RAD (30);
        else
          lean_amount = DEG_TO_RAD (-30);

        // add some noise to make the riding feel more natural
        if (__fabs (turn_amount) < DEG_TO_RAD (5))
        {
          if (timer2 < 0)
          {
            if (my_state == BOOGIEBOARDER_STATE_RIGHT)
            {
              turn_amount = DEG_TO_RAD (-40);
              my_state = BOOGIEBOARDER_STATE_LEFT;
              timer2 = 0.5f;
            }
            else
            {
              turn_amount = DEG_TO_RAD (40);
              my_state = BOOGIEBOARDER_STATE_RIGHT;
              timer2 = 0.5f;
            }
          }
          else
            timer2 -= dt;
        }
/*
        velocity += forces * dt;

        // adjust the momentum to the wave surface
        scale = velocity.length ();

        velocity -= normal * dot (normal, velocity);
        velocity.normalize ();
        velocity *= scale;
        */
      }
      else
      {
        position.y = WATER_Altitude (position.x, position.z);
        WAVE_GlobalCurrent (&current);
        position += current * dt;
        WATER_Normal (position.x, position.z, normal.x, normal.y, normal.z);
        use_hint = false;
      }

			add_trail = true;
    } break;
  }

	if ((add_trail) && (my_trail != NULL))
	{
		vector3d point1, point3;//, zero (ZEROVEC);

		point1 = my_entity->get_abs_po ().slow_xform (vector3d (-0.2f, 0, 0));
		point3 = my_entity->get_abs_po ().slow_xform (vector3d (0.2f, 0, 0));

		if (use_hint && (timer > 0))
			my_trail->add_point (&point1, NULL, &point3, 0, 0, &wave_hint, false);
		else
			my_trail->add_point (&point1, NULL, &point3, 0, 0, NULL, false);
	}

  my_previous_state = enter_state;

  return true;
#endif
}

static rational_t get_floor_offset (entity *ent)
{
  // for now at least, this is entirely determined by the root animation
  entity_anim_tree* a = NULL;
  rational_t last_floor_offset = 0.0f;

  for(int i=(MAX_ANIM_SLOTS-1); i>=0 && (a == NULL); --i)
  {
    a = ent->get_anim_tree( i );
    if(a && (a->is_finished() || a->get_floor_offset() <= 0.0f || !a->is_root(ent)))
      a = NULL;
  }

  if ( a!=NULL && a->is_relative_to_start() )
  {
    vector3d relp = ZEROVEC;
    a->get_current_root_relpos( &relp );
    last_floor_offset = a->get_floor_offset();
    last_floor_offset += relp.y;
  }

  return last_floor_offset;
}

bool surfing_object::surfer1_ai (vector3d& position, vector3d& normal, float dt)
{
  int enter_state = my_state;
  WaveRegionEnum region;
  vector3d current;
  bool right = BeachDataArray[g_game_ptr->get_beach_id ()].bdir;
  bool add_trail = false;
  vector3d forward = -my_entity->get_rel_po ().get_x_facing ();

static float surfer1_takeoff_forward = 10;
static float surfer1_surf_forward = 2;
static float surfer1_pocket_force = 10;
static float surfer1_carve_degrees = 25;
static float surfer1_initial_degrees = 80;

#define MISC_SURFER1_BOARD_THICKNESS 0.1f

  position = my_board_entity->get_rel_position ();
  position -= my_board_entity->get_rel_po ().get_y_facing () * MISC_SURFER1_BOARD_THICKNESS;

  if ((position.x < WAVE_MeshMaxX) && (position.x > WAVE_MeshMinX) &&
      (position.z < WAVE_MeshMaxZ) && (position.z > WAVE_MeshMinZ))
  {
    position += velocity * dt;

    CollideCallStruct collide (position, &normal, &current, &region, &wave_hint);
    use_hint = WAVE_CheckCollision (collide, use_hint, use_hint, use_hint);
    position = collide.position;

    vector3d forces (ZEROVEC);

    vector3d relative_velocity = velocity - current;
    position = collide.position;
    relative_velocity.x += current.x * 0.5f;

    // projection of the weight over the normal vector
    float scale = dot (normal, -YVEC * world_gravity);
    if (scale < 0)
      scale = -scale;

    // fast physics: normal force, weight, water friction and skag force
    forces += (normal * scale);
    forces -= YVEC * world_gravity;
    forces += relative_velocity * -0.25f;
    forces += (relative_velocity - forward * dot (relative_velocity, forward)) * -skag_friction;

    velocity += forces * dt;

    // adjust the momentum to the wave surface
    scale = velocity.length ();

    velocity -= normal * dot (normal, velocity);
    velocity.normalize ();
    velocity *= scale;
  }
  else
  {
    position.y = WATER_Altitude (position.x, position.z);

    WATER_Normal (position.x, position.z, normal.x, normal.y, normal.z);
    WAVE_GlobalCurrent (&current);
    region = WAVE_REGIONBACK;

    position += current * dt;

    use_hint = false;
  }

  switch (my_state)
  {
    case MISC_SURFER1_STATE_IDLE:
    {
      if (region == WAVE_REGIONPOCKET)
        my_state = MISC_SURFER1_STATE_PADDLE;

    } break;

    case MISC_SURFER1_STATE_PADDLE:
    {
      if (my_previous_state != my_state)
        my_entity->play_anim (ANIM_PRIMARY, my_name_anims[MISC_SURFER1_PADDLE], BLEND_TIME, ANIM_LOOPING | ANIM_TWEEN | ANIM_NONCOSMETIC);

      if (region == WAVE_REGIONFACE)
        my_state = MISC_SURFER1_STATE_TAKEOFF;

    } break;

    case MISC_SURFER1_STATE_TAKEOFF:
    {
      if (my_previous_state != my_state)
        my_entity->play_anim (ANIM_PRIMARY, my_name_anims[MISC_SURFER1_LIE_TO_STAND], BLEND_TIME, ANIM_TWEEN | ANIM_NONCOSMETIC);
      else
      {
        entity_anim_tree* anim_tree = my_entity->get_anim_tree (ANIM_PRIMARY);

        if (anim_tree->is_finished () && anim_tree->is_done_tween ())
        {
          my_trail = ks_fx_trail_create (10, 2, 0, NULL);

          if (BeachDataArray[g_game_ptr->get_beach_id ()].bdir)
          {
            my_state = MISC_SURFER1_STATE_LEFT;
            turn_amount = DEG_TO_RAD (-surfer1_initial_degrees);

            my_entity->play_anim (ANIM_PRIMARY, my_name_anims[MISC_SURFER1_TURN_LEFT], BLEND_TIME, ANIM_LOOPING | ANIM_TWEEN | ANIM_NONCOSMETIC);
          }
          else
          {
            my_state = MISC_SURFER1_STATE_RIGHT;
            turn_amount = DEG_TO_RAD (surfer1_initial_degrees);

            my_entity->play_anim (ANIM_PRIMARY, my_name_anims[MISC_SURFER1_TURN_RIGHT], BLEND_TIME, ANIM_LOOPING | ANIM_TWEEN | ANIM_NONCOSMETIC);
          }
        }
      }

      velocity += forward * surfer1_takeoff_forward * dt;
    } break;

    case MISC_SURFER1_STATE_LEFT:
    case MISC_SURFER1_STATE_RIGHT:
    {
      add_trail = true;

      if (turn_amount == 0)
      {
        if (right)
          my_state = MISC_SURFER1_STATE_CARVE_LEFT;
        else
          my_state = MISC_SURFER1_STATE_CARVE_RIGHT;

        timer = 5 + 2 * random_r();
        timer2 = 0;
      }

      velocity += forward * surfer1_surf_forward * dt;

    } break;

    case MISC_SURFER1_STATE_SURF:
    case MISC_SURFER1_STATE_CARVE_LEFT:
    case MISC_SURFER1_STATE_CARVE_RIGHT:
    {
      add_trail = true;

      if (my_previous_state != my_state)
      {
        switch (my_state)
        {
        case MISC_SURFER1_STATE_SURF:
          my_entity->play_anim (ANIM_PRIMARY, my_name_anims[MISC_SURFER1_STAND], BLEND_TIME, ANIM_LOOPING | ANIM_TWEEN | ANIM_NONCOSMETIC);
          break;
        case MISC_SURFER1_STATE_CARVE_LEFT:
          my_entity->play_anim (ANIM_PRIMARY, my_name_anims[MISC_SURFER1_TURN_LEFT], BLEND_TIME, ANIM_LOOPING | ANIM_TWEEN | ANIM_NONCOSMETIC);
          break;
        case MISC_SURFER1_STATE_CARVE_RIGHT:
          my_entity->play_anim (ANIM_PRIMARY, my_name_anims[MISC_SURFER1_TURN_RIGHT], BLEND_TIME, ANIM_LOOPING | ANIM_TWEEN | ANIM_NONCOSMETIC);
          break;
        }
      }

      if (turn_amount == 0)
      {
        timer2 = 0;
        if (my_state == MISC_SURFER1_STATE_CARVE_LEFT)
        {
          my_state = MISC_SURFER1_STATE_CARVE_RIGHT;
          turn_amount = DEG_TO_RAD (-surfer1_carve_degrees);
        }
        else
        {
          my_state = MISC_SURFER1_STATE_CARVE_LEFT;
          turn_amount = DEG_TO_RAD (surfer1_carve_degrees);
        }
      }

      turn_rate = min (timer2 * 2, DEG_TO_RAD (90));
      timer2 += dt;
      timer -= dt;

      if (timer < 0)
      {
        if (BeachDataArray[g_game_ptr->get_beach_id ()].bdir)
        {
          turn_amount = DEG_TO_RAD (-70);

          my_entity->play_anim (ANIM_PRIMARY, my_name_anims[MISC_SURFER1_TURN_RIGHT], BLEND_TIME, ANIM_LOOPING | ANIM_TWEEN | ANIM_NONCOSMETIC);
        }
        else
        {
          turn_amount = DEG_TO_RAD (70);

          my_entity->play_anim (ANIM_PRIMARY, my_name_anims[MISC_SURFER1_TURN_LEFT], BLEND_TIME, ANIM_LOOPING | ANIM_TWEEN | ANIM_NONCOSMETIC);
        }

        my_state = MISC_SURFER1_STATE_TURN_DROPOUT;
      }

      if ((region == WAVE_REGIONLIP) || (region == WAVE_REGIONLIP2))
      {
        // surfer went behind the wave, pretend it was on purpose
        my_state = MISC_SURFER1_STATE_DROPOUT;
      }

      velocity += forward * surfer1_surf_forward * dt;

    } break;

    case MISC_SURFER1_STATE_COLLIDE:
    {
      entity_anim_tree* anim_tree = my_entity->get_anim_tree (ANIM_PRIMARY);

      if (anim_tree->is_finished () && anim_tree->is_done_tween ())
        my_state = MISC_SURFER1_STATE_COLLIDE_FADE;

      velocity += ZVEC * surfer1_takeoff_forward * dt;

    } break;

    case MISC_SURFER1_STATE_COLLIDE_FADE:
    {
      if (my_previous_state != my_state)
        my_entity->play_anim (ANIM_PRIMARY, my_name_anims[MISC_SURFER1_SWIM], BLEND_TIME, ANIM_LOOPING | ANIM_TWEEN | ANIM_NONCOSMETIC);

      velocity += ZVEC * surfer1_takeoff_forward * dt;

    } break;

    case MISC_SURFER1_STATE_TURN_DROPOUT:
    {
      add_trail = true;

      if (turn_amount == 0)
        my_state = MISC_SURFER1_STATE_DROPOUT;

      velocity += ZVEC * surfer1_takeoff_forward * dt;
    } break;

    case MISC_SURFER1_STATE_DROPOUT:
    {
      if (my_previous_state != my_state)
        my_entity->play_anim (ANIM_PRIMARY, my_name_anims[MISC_SURFER1_STAND_TO_LIE], BLEND_TIME, ANIM_TWEEN | ANIM_NONCOSMETIC);

      entity_anim_tree* anim_tree = my_entity->get_anim_tree (ANIM_PRIMARY);

      if (anim_tree->is_finished () && anim_tree->is_done_tween ())
        my_state = MISC_SURFER1_STATE_FADE;

      velocity += ZVEC * surfer1_takeoff_forward * dt;

    } break;

    case MISC_SURFER1_STATE_FADE:
    {
      if (my_previous_state != my_state)
        my_entity->play_anim (ANIM_PRIMARY, my_name_anims[MISC_SURFER1_IDLE], BLEND_TIME, ANIM_LOOPING | ANIM_TWEEN | ANIM_NONCOSMETIC);

      velocity += ZVEC * surfer1_takeoff_forward * dt;

    } break;
  }

  if (add_trail)
  {
    if (my_trail != NULL)
    {
      vector3d point1, point3;

      point1 = my_entity->get_abs_po ().slow_xform (vector3d (0, 0, 0.2f));
      point3 = my_entity->get_abs_po ().slow_xform (vector3d (0, 0, -0.2f));

      if (use_hint)
        my_trail->add_point (&point1, NULL, &point3, 0, 0, &wave_hint, false);
      else
        my_trail->add_point (&point1, NULL, &point3, 0, 0, NULL, false);
    }

    if (region == WAVE_REGIONPOCKET)
    {
      velocity += ZVEC * surfer1_pocket_force  * dt;
    }
    else if ((region == WAVE_REGIONCHIN) || (region == WAVE_REGIONFACE))
    {
      if (my_entity->get_abs_position ().z < position.z)
        velocity += ZVEC * -surfer1_pocket_force  * dt;

      velocity += ZVEC * -surfer1_pocket_force  * dt;
    }
  }

  vector3d x = cross (normal, my_entity->get_rel_po ().get_z_facing ());
  x.normalize();
  vector3d z = cross (x, normal);
  z.normalize ();

  // remove tinting to fix the sorting bug for now until we find the cause of the problem
  my_board_entity->set_render_color (color32_white);

  // fade object
  if ((fade_distance >= 0) && (position.z > WAVE_MeshMaxZ + fade_distance))
  {
    float blend;

    blend = 1.0f - ((position.z - (WAVE_MeshMaxZ + fade_distance)) / 50);

    if (blend < 0.025f)
    {
      color32 col = ren_col;
      col.set_alpha (0);
      my_board_entity->set_render_color (col);

      return false;
    }
    else
    {
      color32 col = ren_col;
      col.set_alpha (ren_col.get_alpha () * blend);
      my_board_entity->set_render_color (col);
    }
  }

  position += normal * MISC_SURFER1_BOARD_THICKNESS;

  my_board_entity->set_rel_po (po (x, normal, z, position));

  position += normal * get_floor_offset (my_entity);

  my_previous_state = enter_state;

  return true;
}

bool surfing_object::surfer2_ai (vector3d& position, vector3d& normal, float dt)
{
  int enter_state = my_state;
	
  entity_anim_tree* anim_tree = my_entity->get_anim_tree (ANIM_PRIMARY);
	if (my_idle_delay > 0)
	{
		my_idle_delay -= TIMER_GetFrameSec();
		if (my_idle_delay < 0)
		{
			my_entity->play_anim (ANIM_PRIMARY, my_name_anims[MISC_SURFER2_PADDLE], BLEND_TIME, ANIM_LOOPING | ANIM_TWEEN | ANIM_NONCOSMETIC);
		}
	}
	
  if (anim_tree->is_finished () && anim_tree->is_done_tween ())
  {
		my_entity->play_anim (ANIM_PRIMARY, my_name_anims[MISC_SURFER2_PADDLE], BLEND_TIME, ANIM_LOOPING | ANIM_TWEEN | ANIM_NONCOSMETIC);
  }
  WaveRegionEnum region;
  vector3d current;
  vector3d forward = -my_entity->get_rel_po ().get_x_facing ();

  position = my_board_entity->get_rel_position ();
  position -= my_board_entity->get_rel_po ().get_y_facing () * MISC_SURFER1_BOARD_THICKNESS;

  if ((position.x < WAVE_MeshMaxX) && (position.x > WAVE_MeshMinX) &&
      (position.z < WAVE_MeshMaxZ) && (position.z > WAVE_MeshMinZ))
  {
    CollideCallStruct collide (position, &normal, &current, &region, &wave_hint);
    use_hint = WAVE_CheckCollision (collide, use_hint, use_hint, use_hint);
    position = collide.position;
    position += velocity * dt;

    vector3d forces (ZEROVEC);

    vector3d relative_velocity = velocity - current;
    position = collide.position;
    relative_velocity.x += current.x * 0.5f;

		

    // projection of the weight over the normal vector
    float scale = dot (normal, -YVEC * world_gravity);
    if (scale < 0)
      scale = -scale;

    // fast physics: normal force, weight, water friction and skag force
    forces += (normal * scale);
    forces -= YVEC * world_gravity;
    forces += relative_velocity * -0.25f;
    forces += (relative_velocity - forward * dot (relative_velocity, forward)) * -skag_friction;
		// paddling forces
		forces += .7 * forward;

    velocity += forces * dt;

    // adjust the momentum to the wave surface
    scale = velocity.length ();

    velocity -= normal * dot (normal, velocity);
    velocity.normalize ();
    velocity *= scale;
  }
  else
  {
    position.y = WATER_Altitude (position.x, position.z);

    WATER_Normal (position.x, position.z, normal.x, normal.y, normal.z);
    WAVE_GlobalCurrent (&current);
    region = WAVE_REGIONBACK;

    position += current * dt;

    use_hint = false;
  }

	
  vector3d x = cross (normal, my_entity->get_rel_po ().get_z_facing ());
  x.normalize();
  vector3d z = cross (x, normal);
  z.normalize ();

  // remove tinting to fix the sorting bug for now until we find the cause of the problem
  my_board_entity->set_render_color (color32_white);

  // fade object
  if (position.z > WAVE_MeshMaxZ)
  {
    float blend;

    blend = 1.0f - (position.z - (WAVE_MeshMaxZ) / 50);

    if (blend < 0.025f)
    {
      color32 col = ren_col;
      col.set_alpha (0);
      my_board_entity->set_render_color (col);
			despawn();
      return false;
    }
    else
    {
      color32 col = ren_col;
      col.set_alpha (ren_col.get_alpha () * blend);
      my_board_entity->set_render_color (col);
    }
  }

  position += normal * MISC_SURFER1_BOARD_THICKNESS;

  my_board_entity->set_rel_po (po (x, normal, z, position));

  position += normal * get_floor_offset (my_entity);

  my_previous_state = enter_state;

/*
  // make the object flow with the water
  if ((position.x < WAVE_MeshMaxX) && (position.x > WAVE_MeshMinX) &&
      (position.z < WAVE_MeshMaxZ) && (position.z > WAVE_MeshMinZ))
  {
    if (use_hint)
    {
#if defined(TARGET_XBOX)
      WaveFloaterArgs wfa((WaveQueryFlags) (WAVE_HINTGIVEN | WAVE_HINTSOUGHT | WAVE_NORMALSOUGHT), 
                          &wave_hint, NULLREF(WaveVelocityHint), &position, &normal, NULL );
#else
      WaveFloaterArgs wfa = { (WaveQueryFlags) (WAVE_HINTGIVEN | WAVE_HINTSOUGHT | WAVE_NORMALSOUGHT), 
          &wave_hint, NULLREF(WaveVelocityHint), &position, &normal, NULL, };
#endif 

      WAVE_TrackFloater (wfa);
    }
    else
    {
      CollideCallStruct collide (position, &normal, NULL, NULL, &wave_hint);
      use_hint = WAVE_CheckCollision (collide, use_hint, false, false);
      position = collide.position;
    }
  }
  else
  {
    vector3d current;

    position.y = WATER_Altitude (position.x, position.z);
    WAVE_GlobalCurrent (&current);
    WATER_Normal (position.x, position.z, normal.x, normal.y, normal.z);

    position += current * dt;
    use_hint = false;
  }
*/
  my_previous_state = enter_state;

  return true;
}

bool surfing_object::kayaker_ai (vector3d& position, vector3d& normal, float dt)
{
  // FIXME: find out why the lights are not being updated correctly
  light_manager *lmr = ((conglomerate*)my_entity)->get_light_set();

  if (lmr != NULL)
  {
    lmr->goal_ambient = color_white;
    lmr->last_ambient = color_white;
  }

	bool ret = floating_ai (position, normal, dt);

  if ((my_state == KAYAKER_STATE_HIT_LEFT) || (my_state == KAYAKER_STATE_HIT_RIGHT))
  {
    if (timer2 < 2)
    {
      po rot;

      if (my_state == KAYAKER_STATE_HIT_LEFT)
        rot.set_rotate_z (DEG_TO_RAD (dt * -180));
      else
        rot.set_rotate_z (DEG_TO_RAD (dt * 180));

      my_entity->set_rel_po (rot * my_entity->get_rel_po ());
      normal = my_entity->get_rel_po ().get_y_facing ();

      timer2 += dt;
    }
    else
    {
      my_state = my_previous_state;

      if (my_state == KAYAKER_STATE_DOWN)
        my_entity->play_anim (ANIM_PRIMARY, my_name_anims[KAYAKER_PADDLE], BLEND_TIME, ANIM_LOOPING | ANIM_TWEEN | ANIM_NONCOSMETIC);
      else
        my_entity->play_anim (ANIM_PRIMARY, my_name_anims[KAYAKER_AMBIENT], BLEND_TIME, ANIM_LOOPING | ANIM_TWEEN | ANIM_NONCOSMETIC);
    }
  }

	// looks like they changed their mind for 1e+12 time...
  return ret;

	/*
  vector3d current, forces (ZEROVEC);
//  extern float water_friction;
  WaveRegionEnum region;

  position += velocity * dt;

  if (my_state == KAYAKER_STATE_DOWN)
  {
    vector3d point1, point3;//, zero (ZEROVEC);

    point1 = my_entity->get_abs_po ().slow_xform (vector3d (-0.2f, 0, 0));
    point3 = my_entity->get_abs_po ().slow_xform (vector3d (0.2f, 0, 0));

    if (my_trail != NULL)
    {
      if (use_hint)
        my_trail->add_point (&point1, NULL, &point3, 0, 0, &wave_hint, false);
      else
        my_trail->add_point (&point1, NULL, &point3, 0, 0, NULL, false);
    }
  }

  // make the object flow with the water
  if ((position.x < WAVE_MeshMaxX) && (position.x > WAVE_MeshMinX) &&
      (position.z < WAVE_MeshMaxZ) && (position.z > WAVE_MeshMinZ))
  {
    CollideCallStruct collide (position, &normal, &current, &region, &wave_hint);
    use_hint = WAVE_CheckCollision (collide, use_hint, false, false);

    position = collide.position;
  }
  else
  {
    position.y = WATER_Altitude (position.x, position.z);
    WAVE_GlobalCurrent (&current);
    WATER_Normal (position.x, position.z, normal.x, normal.y, normal.z);
    use_hint = false;
  }

  // (don't tell anyone and they won't notice)
  current.x *= 0.5f;

  vector3d relative_velocity = velocity - current;

  // projection of the weight over the normal vector
  float scale = dot (normal, -YVEC * world_gravity);
  if (scale < 0)
    scale = -scale;

  if (!((my_state == KAYAKER_STATE_DOWN) && (region == WAVE_REGIONFRONT)))
  {
    // fast physics: normal force, weight and water friction
    forces += (normal * scale);
    forces -= 4 * YVEC * world_gravity;
		forces.z += -2; // paddling

    if ((timer > 0) || (my_state != KAYAKER_STATE_DOWN))
    {
      forces += relative_velocity * -0.75f;
      forces += (relative_velocity - my_entity->get_rel_po ().get_z_facing () * dot (relative_velocity, my_entity->get_rel_po ().get_z_facing ())) * -1;
      
      timer -= dt;
    }
  }

  velocity += forces * dt;

  // adjust the momentum to the wave surface
  scale = velocity.length ();

  velocity -= normal * dot (normal, velocity);
  velocity.normalize ();
  velocity *= scale;

  if ((region == WAVE_REGIONPOCKET) && (my_state == KAYAKER_STATE_UP))
  {
    my_entity->play_anim (ANIM_PRIMARY, my_name_anims[KAYAKER_PADDLE], BLEND_TIME, ANIM_LOOPING | ANIM_TWEEN | ANIM_NONCOSMETIC);
  }

  if ((region == WAVE_REGIONFACE) && (my_state == KAYAKER_STATE_UP))
  {
      // 25% chance the kayaker will go behing the wave
//      if (random_r() < 0.25f)
//        my_state = KAYAKER_STATE_BEHIND;
//      else
    my_state = KAYAKER_STATE_DOWN;

    my_trail = ks_fx_trail_create (10, 2, 0, NULL);

    timer = random_r();
  }

  if ((my_state == KAYAKER_STATE_HIT_LEFT) || (my_state == KAYAKER_STATE_HIT_RIGHT))
  {
    if (timer2 < 2)
    {
      po rot;

      if (my_state == KAYAKER_STATE_HIT_LEFT)
        rot.set_rotate_z (DEG_TO_RAD (dt * -180));
      else
        rot.set_rotate_z (DEG_TO_RAD (dt * 180));

      my_entity->set_rel_po (rot * my_entity->get_rel_po ());
      normal = my_entity->get_rel_po ().get_y_facing ();

      timer2 += dt;
    }
    else
    {
      my_state = my_previous_state;

      if (my_state == KAYAKER_STATE_DOWN)
        my_entity->play_anim (ANIM_PRIMARY, my_name_anims[KAYAKER_PADDLE], BLEND_TIME, ANIM_LOOPING | ANIM_TWEEN | ANIM_NONCOSMETIC);
      else
        my_entity->play_anim (ANIM_PRIMARY, my_name_anims[KAYAKER_AMBIENT], BLEND_TIME, ANIM_LOOPING | ANIM_TWEEN | ANIM_NONCOSMETIC);
    }
  }
*/
  return true;
}

bool surfing_object::icepatch_ai (vector3d& position, vector3d& normal, float dt)
{
  return floating_ai (position, normal, dt);
}

bool surfing_object::kelp_ai (vector3d& position, vector3d& normal, float dt)
{
  return floating_ai (position, normal, dt);
}

bool surfing_object::fatbastard_ai (vector3d& position, vector3d& normal, float dt)
{
  return floating_ai (position, normal, dt);
}

bool surfing_object::swimmer_ai (vector3d& position, vector3d& normal, float dt)
{
  return floating_ai (position, normal, dt);
}

bool surfing_object::snorkeler_ai (vector3d& position, vector3d& normal, float dt)
{
  return floating_ai (position, normal, dt);
}

bool surfing_object::cameraman_ai (vector3d& position, vector3d& normal, float dt)
{
  // FIXME: find out why the lights are not being updated correctly
  light_manager *lmr = ((conglomerate*)my_entity)->get_light_set();

  if (lmr != NULL)
  {
    lmr->goal_ambient = color_white;
    lmr->last_ambient = color_white;
  }

  return floating_ai (position, normal, dt);
}

bool surfing_object::dolphin_ai (vector3d& position, vector3d& normal, float dt)
{
  return floating_ai (position, normal, dt);
}

bool surfing_object::greatwhite_ai (vector3d& position, vector3d& normal, float dt)
{
  return floating_ai (position, normal, dt);
}

bool surfing_object::seal_ai (vector3d& position, vector3d& normal, float dt)
{
  return floating_ai (position, normal, dt);
}

bool surfing_object::seagull_ai (vector3d& position, vector3d& normal, float dt)
{
  return floating_ai (position, normal, dt);
}

bool surfing_object::outrigger_ai (vector3d& position, vector3d& normal, float dt)
{
  return floating_ai (position, normal, dt);
}

bool surfing_object::humpback_ai (vector3d& position, vector3d& normal, float dt)
{
  return floating_ai (position, normal, dt);
}

bool surfing_object::helicopter_ai (vector3d& position, vector3d& normal, float dt)
{
  return floating_ai (position, normal, dt);
}

bool surfing_object::hammerhead_ai (vector3d& position, vector3d& normal, float dt)
{
  return floating_ai (position, normal, dt);
}

bool surfing_object::windsurfer_ai (vector3d& position, vector3d& normal, float dt)
{
	WaveRegionEnum region;
	vector3d current;
	int enter_state = my_state;
	bool add_trail = false;

  switch (my_state)
  {
    case WINDSURFER_STATE_IDLE:
		case WINDSURFER_STATE_COLLIDE:
		case WINDSURFER_STATE_COLLIDE_AIR:
    case WINDSURFER_STATE_BEHIND:
		case WINDSURFER_STATE_PREPARE_TRICK:
		case WINDSURFER_STATE_TRICK:
    {
      if ((position.x < WAVE_MeshMaxX) && (position.x > WAVE_MeshMinX) &&
          (position.z < WAVE_MeshMaxZ) && (position.z > WAVE_MeshMinZ))
      {
        vector3d last_position (position);

				CollideCallStruct collide (position, &normal, &current, &region, &wave_hint);
				use_hint = WAVE_CheckCollision (collide, use_hint, false, false);
				position = collide.position;
				current.x *= 0.5f;
				position += current * dt;

				if (my_state == WINDSURFER_STATE_BEHIND)
					add_trail = true;

				if (my_state == WINDSURFER_STATE_PREPARE_TRICK)
				{ 
          if ((region == WAVE_REGIONLIP) || (region == WAVE_REGIONLIP2))
					{
						my_state = WINDSURFER_STATE_TRICK;
						my_entity->play_anim (ANIM_PRIMARY, my_name_anims[WINDSURFER_AIR_TRICK], BLEND_TIME, ANIM_TWEEN | ANIM_NONCOSMETIC);
						if (my_board_entity)
							my_board_entity->play_anim (ANIM_PRIMARY, my_board_name_anims[WINDSURFER_AIR_TRICK], BLEND_TIME, ANIM_TWEEN | ANIM_NONCOSMETIC);
					}

					add_trail = true;
				}

				if (my_state == WINDSURFER_STATE_TRICK)
				{
	        entity_anim_tree* anim_tree = my_entity->get_anim_tree (ANIM_PRIMARY);

	        if (anim_tree->is_finished () && anim_tree->is_done_tween ())
					{
						my_state = WINDSURFER_STATE_BEHIND;
						my_entity->play_anim (ANIM_PRIMARY, my_name_anims[WINDSURFER_IDLE], BLEND_TIME, ANIM_TWEEN | ANIM_NONCOSMETIC);
						if (my_board_entity)
							my_board_entity->play_anim (ANIM_PRIMARY, my_board_name_anims[WINDSURFER_IDLE], BLEND_TIME, ANIM_TWEEN | ANIM_NONCOSMETIC);
					}
				}

				if ((my_state == WINDSURFER_STATE_COLLIDE) || (my_state == WINDSURFER_STATE_COLLIDE_AIR))
				{
					if (my_state != my_previous_state)
					{
						kellyslater_controller *ksctrl=g_world_ptr->get_ks_controller(g_game_ptr->get_active_player());
						ksctrl->get_my_scoreManager().AddGap(GAP_SMACK_WINDSURFER);
					}

	        entity_anim_tree* anim_tree = my_entity->get_anim_tree (ANIM_PRIMARY);

	        if (anim_tree->is_finished () && anim_tree->is_done_tween ())
					{
						if (my_state == WINDSURFER_STATE_COLLIDE)
						{
							my_entity->play_anim (ANIM_PRIMARY, my_name_anims[WINDSURFER_COLLIDEI], BLEND_TIME, ANIM_TWEEN | ANIM_NONCOSMETIC);
							if (my_board_entity)
								my_board_entity->play_anim (ANIM_PRIMARY, my_board_name_anims[WINDSURFER_COLLIDEI], BLEND_TIME, ANIM_TWEEN | ANIM_NONCOSMETIC);
						}
						else
						{
							my_entity->play_anim (ANIM_PRIMARY, my_name_anims[WINDSURFER_COLLIDE_IN_AIRI], BLEND_TIME, ANIM_TWEEN | ANIM_NONCOSMETIC);
							if (my_board_entity)
								my_board_entity->play_anim (ANIM_PRIMARY, my_board_name_anims[WINDSURFER_COLLIDE_IN_AIRI], BLEND_TIME, ANIM_TWEEN | ANIM_NONCOSMETIC);
						}

						my_state = WINDSURFER_STATE_BEHIND;
					}
				}

        if ((my_state == WINDSURFER_STATE_IDLE) &&
            (region == WAVE_REGIONPOCKET))
        {
        	if (BeachDataArray[g_game_ptr->get_beach_id ()].bdir)
          {
            my_state = WINDSURFER_STATE_LEFT;
            turn_amount = DEG_TO_RAD (-70);
            my_entity->play_anim (ANIM_PRIMARY, my_name_anims[WINDSURFER_LTURN], BLEND_TIME, ANIM_LOOPING | ANIM_TWEEN | ANIM_NONCOSMETIC);
						if (my_board_entity)
	            my_board_entity->play_anim (ANIM_PRIMARY, my_board_name_anims[WINDSURFER_LTURN], BLEND_TIME, ANIM_LOOPING | ANIM_TWEEN | ANIM_NONCOSMETIC);
          }
          else
          {
            my_state = WINDSURFER_STATE_RIGHT;
            turn_amount = DEG_TO_RAD (70);
            my_entity->play_anim (ANIM_PRIMARY, my_name_anims[WINDSURFER_RTURN], BLEND_TIME, ANIM_LOOPING | ANIM_TWEEN | ANIM_NONCOSMETIC);
						if (my_board_entity)
	            my_board_entity->play_anim (ANIM_PRIMARY, my_board_name_anims[WINDSURFER_RTURN], BLEND_TIME, ANIM_LOOPING | ANIM_TWEEN | ANIM_NONCOSMETIC);
          }

          timer2 = 0;
          velocity = (position - last_position) / (1.25f * dt);
          timer = 5 + 2 * random_r();

          my_trail = ks_fx_trail_create (10, 2, 0, NULL);
        }
      }
      else
      {
        position.y = WATER_Altitude (position.x, position.z);
        WAVE_GlobalCurrent (&current);
        position += current * dt;
        WATER_Normal (position.x, position.z, normal.x, normal.y, normal.z);
        use_hint = false;
      }
    } break;

    case WINDSURFER_STATE_LEFT:
    case WINDSURFER_STATE_RIGHT:
    {
      if ((position.x < WAVE_MeshMaxX) && (position.x > WAVE_MeshMinX) &&
          (position.z < WAVE_MeshMaxZ) && (position.z > WAVE_MeshMinZ))
      {
        vector3d current, forces (ZEROVEC);
//        extern float water_friction;

        CollideCallStruct collide (position, &normal, &current, NULL, &wave_hint);
        use_hint = WAVE_CheckCollision (collide, use_hint, false, false);
        position = collide.position;
        position += velocity * dt;

        {
          // HACK HACK HACK HACK (don't tell anyone and they won't notice)
          current.x *= 0.5f;

          vector3d relative_velocity = velocity - current;
          position = collide.position;

					const float wind_weight = 3;
					const float wind_force = 6;
					const float wind_friction = -0.5f;
					const float wind_skag = -4;

          // projection of the weight over the normal vector
          float scale = dot (normal, -YVEC * wind_weight * world_gravity);
          if (scale < 0)
            scale = -scale;

          // fast physics: normal force, weight and water friction
          forces += (normal * scale);
          forces -= YVEC * world_gravity;
          forces += relative_velocity * wind_friction;
          forces += (relative_velocity - my_entity->get_rel_po ().get_z_facing () * dot (relative_velocity, my_entity->get_rel_po ().get_z_facing ())) * wind_skag;

					const float wind_x = 0.5f;
					forces.x *= wind_x;

					if (timer > 0)
						forces += my_entity->get_rel_po ().get_z_facing () * wind_force;

          timer -= dt;

          if (timer < 0)
          {
        	  if (BeachDataArray[g_game_ptr->get_beach_id ()].bdir)
              turn_amount = DEG_TO_RAD (-90);
            else
              turn_amount = DEG_TO_RAD (90);

						my_state = WINDSURFER_STATE_PREPARE_TRICK;
            my_entity->play_anim (ANIM_PRIMARY, my_name_anims[WINDSURFER_IDLE], BLEND_TIME, ANIM_LOOPING | ANIM_TWEEN | ANIM_NONCOSMETIC);
						if (my_board_entity)
	            my_board_entity->play_anim (ANIM_PRIMARY, my_board_name_anims[WINDSURFER_IDLE], BLEND_TIME, ANIM_LOOPING | ANIM_TWEEN | ANIM_NONCOSMETIC);
          }

          velocity += forces * dt;

          // adjust the momentum to the wave surface
          scale = velocity.length ();

          velocity -= normal * dot (normal, velocity);
          velocity.normalize ();
          velocity *= scale;
        }

        if (BeachDataArray[g_game_ptr->get_beach_id ()].bdir)
          lean_amount = DEG_TO_RAD (30);
        else
          lean_amount = DEG_TO_RAD (-30);

        // add some noise to make the riding feel more natural
        if (__fabs (turn_amount) < DEG_TO_RAD (5))
        {
          if (timer2 < 0)
          {
            if (my_state == WINDSURFER_STATE_RIGHT)
            {
              turn_amount = DEG_TO_RAD (-40);
              my_state = WINDSURFER_STATE_LEFT;
              timer2 = 0.5f;
            }
            else
            {
              turn_amount = DEG_TO_RAD (40);
              my_state = WINDSURFER_STATE_RIGHT;
              timer2 = 0.5f;
            }
          }
          else
            timer2 -= dt;
        }
      }
      else
      {
        position.y = WATER_Altitude (position.x, position.z);
        WAVE_GlobalCurrent (&current);
        position += current * dt;
        WATER_Normal (position.x, position.z, normal.x, normal.y, normal.z);
        use_hint = false;
      }

			add_trail = true;
    } break;

		default:
			assert (0);
			break;
  }

	if ((add_trail) && (my_trail != NULL))
	{
		vector3d point1, point3;//, zero (ZEROVEC);

		point1 = my_entity->get_abs_po ().slow_xform (vector3d (-0.2f, 0, 0));
		point3 = my_entity->get_abs_po ().slow_xform (vector3d (0.2f, 0, 0));

		if (use_hint && (timer > 0))
			my_trail->add_point (&point1, NULL, &point3, 0, 0, &wave_hint, false);
		else
			my_trail->add_point (&point1, NULL, &point3, 0, 0, NULL, false);
	}

  my_previous_state = enter_state;

	if (my_board_entity)
	  my_board_entity->set_rel_po (my_entity->get_rel_po ());

	return true;
}

bool surfing_object::jetskier_ai (vector3d& position, vector3d& normal, float dt)
{
	WaveRegionEnum region;
	vector3d current;
	int enter_state = my_state;
	bool add_trail = false;

  switch (my_state)
  {
    case JETSKIER_STATE_IDLE:
		case JETSKIER_STATE_COLLIDE:
    case JETSKIER_STATE_BEHIND:
    {
      if ((position.x < WAVE_MeshMaxX) && (position.x > WAVE_MeshMinX) &&
          (position.z < WAVE_MeshMaxZ) && (position.z > WAVE_MeshMinZ))
      {
        vector3d last_position (position);

				CollideCallStruct collide (position, &normal, &current, &region, &wave_hint);
				use_hint = WAVE_CheckCollision (collide, use_hint, false, false);
				position = collide.position;
				current.x *= 0.5f;
				position += current * dt;

				if (my_state == JETSKIER_STATE_BEHIND)
					add_trail = true;

				if (my_state == JETSKIER_STATE_COLLIDE)
				{
	        entity_anim_tree* anim_tree = my_entity->get_anim_tree (ANIM_PRIMARY);

	        if (anim_tree->is_finished () && anim_tree->is_done_tween ())
					{
						my_state = JETSKIER_STATE_BEHIND;
						my_entity->play_anim (ANIM_PRIMARY, my_name_anims[JETSKIER_COLLIDEI], BLEND_TIME, ANIM_TWEEN | ANIM_NONCOSMETIC);
					}
				}

        if ((my_state == JETSKIER_STATE_IDLE) &&
            (region == WAVE_REGIONPOCKET))
        {
        	if (BeachDataArray[g_game_ptr->get_beach_id ()].bdir)
          {
            my_state = JETSKIER_STATE_LEFT;
            turn_amount = DEG_TO_RAD (-70);
            my_entity->play_anim (ANIM_PRIMARY, my_name_anims[JETSKIER_LTURN], BLEND_TIME, ANIM_LOOPING | ANIM_TWEEN | ANIM_NONCOSMETIC);
          }
          else
          {
            my_state = JETSKIER_STATE_RIGHT;
            turn_amount = DEG_TO_RAD (70);
            my_entity->play_anim (ANIM_PRIMARY, my_name_anims[JETSKIER_RTURN], BLEND_TIME, ANIM_LOOPING | ANIM_TWEEN | ANIM_NONCOSMETIC);
          }

          timer2 = 0;
          velocity = (position - last_position) / (1.25f * dt);
          timer = 5 + 2 * random_r();

          my_trail = ks_fx_trail_create (10, 2, 0, NULL);
        }
      }
      else
      {
        position.y = WATER_Altitude (position.x, position.z);
        WAVE_GlobalCurrent (&current);
        position += current * dt;
        WATER_Normal (position.x, position.z, normal.x, normal.y, normal.z);
        use_hint = false;
      }
    } break;

    case JETSKIER_STATE_LEFT:
    case JETSKIER_STATE_RIGHT:
    {
      if ((position.x < WAVE_MeshMaxX) && (position.x > WAVE_MeshMinX) &&
          (position.z < WAVE_MeshMaxZ) && (position.z > WAVE_MeshMinZ))
      {
        vector3d current, forces (ZEROVEC);
//        extern float water_friction;

        CollideCallStruct collide (position, &normal, &current, NULL, &wave_hint);
        use_hint = WAVE_CheckCollision (collide, use_hint, false, false);
        position = collide.position;
        position += velocity * dt;

        {
          // HACK HACK HACK HACK (don't tell anyone and they won't notice)
          current.x *= 0.5f;

          vector3d relative_velocity = velocity - current;
          position = collide.position;

					const float jet_weight = 3;
					const float jet_force = 12;
					const float jet_friction = -0.5f;
					const float jet_skag = -4;

          // projection of the weight over the normal vector
          float scale = dot (normal, -YVEC * jet_weight * world_gravity);
          if (scale < 0)
            scale = -scale;

          // fast physics: normal force, weight and water friction
          forces += (normal * scale);
          forces -= YVEC * world_gravity;
          forces += relative_velocity * jet_friction;
          forces += (relative_velocity - my_entity->get_rel_po ().get_z_facing () * dot (relative_velocity, my_entity->get_rel_po ().get_z_facing ())) * jet_skag;

					const float jet_x = 0.5f;
					forces.x *= jet_x;

					if (timer > 0)
						forces += my_entity->get_rel_po ().get_z_facing () * jet_force;

          timer -= dt;

          if (timer < 0)
          {
        	  if (BeachDataArray[g_game_ptr->get_beach_id ()].bdir)
              turn_amount = DEG_TO_RAD (-90);
            else
              turn_amount = DEG_TO_RAD (90);

						my_state = JETSKIER_STATE_BEHIND;
            my_entity->play_anim (ANIM_PRIMARY, my_name_anims[JETSKIER_IDLE], BLEND_TIME, ANIM_LOOPING | ANIM_TWEEN | ANIM_NONCOSMETIC);
          }

          velocity += forces * dt;

          // adjust the momentum to the wave surface
          scale = velocity.length ();

          velocity -= normal * dot (normal, velocity);
          velocity.normalize ();
          velocity *= scale;
        }

        if (BeachDataArray[g_game_ptr->get_beach_id ()].bdir)
          lean_amount = DEG_TO_RAD (30);
        else
          lean_amount = DEG_TO_RAD (-30);

        // add some noise to make the riding feel more natural
        if (__fabs (turn_amount) < DEG_TO_RAD (5))
        {
          if (timer2 < 0)
          {
            if (my_state == JETSKIER_STATE_RIGHT)
            {
              turn_amount = DEG_TO_RAD (-40);
              my_state = JETSKIER_STATE_LEFT;
              timer2 = 0.5f;
            }
            else
            {
              turn_amount = DEG_TO_RAD (40);
              my_state = JETSKIER_STATE_RIGHT;
              timer2 = 0.5f;
            }
          }
          else
            timer2 -= dt;
        }
      }
      else
      {
        position.y = WATER_Altitude (position.x, position.z);
        WAVE_GlobalCurrent (&current);
        position += current * dt;
        WATER_Normal (position.x, position.z, normal.x, normal.y, normal.z);
        use_hint = false;
      }

			add_trail = true;
    } break;
  }

	if ((add_trail) && (my_trail != NULL))
	{
		vector3d point1, point3;//, zero (ZEROVEC);

		point1 = my_entity->get_abs_po ().slow_xform (vector3d (-0.2f, 0, 0));
		point3 = my_entity->get_abs_po ().slow_xform (vector3d (0.2f, 0, 0));

		if (use_hint && (timer > 0))
			my_trail->add_point (&point1, NULL, &point3, 0, 0, &wave_hint, false);
		else
			my_trail->add_point (&point1, NULL, &point3, 0, 0, NULL, false);
	}

  my_previous_state = enter_state;

  return true;
}

bool surfing_object::mantaray_ai (vector3d& position, vector3d& normal, float dt)
{
  return floating_ai (position, normal, dt);
}

bool surfing_object::turtle_ai (vector3d& position, vector3d& normal, float dt)
{
	vector3d old (position);
	bool ret;

	ret = floating_ai (position, normal, dt);

	position = old + ((position - old) * 0.5f);

	if (my_anim_handler != NULL)
	{
		if (((generic_anim_animal*) my_anim_handler)->is_diving ())
			normal = YVEC;
	}

  return ret;
}

bool surfing_object::fisherman_ai (vector3d& position, vector3d& normal, float dt)
{
  bool ret = floating_ai (position, normal, dt);

	if (my_board_entity)
	  my_board_entity->set_rel_po (my_entity->get_rel_po ());

	return ret;
}

bool surfing_object::dingy_ai (vector3d& position, vector3d& normal, float dt)
{
  bool ret = floating_ai (position, normal, dt);

	if (my_board_entity)
	{
	  my_board_entity->set_rel_po (my_entity->get_rel_po ());
	  my_board_entity->set_rel_position (position);
	}

	if (my_third_entity)
	{
	  my_third_entity->set_rel_po (my_entity->get_rel_po ());
	  my_third_entity->set_rel_position (position);
	}

	return ret;
}

void surfing_object::jumped_over (entity *ent)
{
  if (my_anim_handler != NULL)
    my_anim_handler->update (false, true, false, &my_max_alpha);
}

void surfing_object::sprayed (entity *ent)
{
  if (my_anim_handler != NULL)
    my_anim_handler->update (false, false, true, &my_max_alpha);
}

void surfing_object::collide (entity *ent, const vector3d& dir)
{
  my_previous_state = my_state;

  if (my_anim_handler != NULL)
    my_anim_handler->update (true, false, false, &my_max_alpha);

  if(!ksreplay.IsPlaying())
    ksreplay.SetCollisionInfo(this, ent, dir);

  switch (my_type)
  {
  case MISC_SURFER1:
    if ((my_state != MISC_SURFER1_STATE_COLLIDE) && (my_state != MISC_SURFER1_STATE_COLLIDE_FADE))
    {
      if ((my_state == MISC_SURFER1_STATE_IDLE) || (my_state == MISC_SURFER1_STATE_PADDLE) || (my_state == MISC_SURFER1_STATE_TAKEOFF))
      {
        my_entity->play_anim (ANIM_PRIMARY, my_name_anims[MISC_SURFER1_WIPEOUT_LYING], BLEND_TIME, ANIM_TWEEN | ANIM_NONCOSMETIC);
      }
      else
      {
        float fd = -dot (-my_entity->get_abs_po ().get_x_facing (), dir);

        if (fd > 0.7071067f)
        {
          my_entity->play_anim (ANIM_PRIMARY, my_name_anims[MISC_SURFER1_WIPEOUT_FALL_FRONT], BLEND_TIME, ANIM_TWEEN | ANIM_NONCOSMETIC);
        }
        else if (fd < -0.7071067f)
        {
          my_entity->play_anim (ANIM_PRIMARY, my_name_anims[MISC_SURFER1_WIPEOUT_FALL_BACK], BLEND_TIME, ANIM_TWEEN | ANIM_NONCOSMETIC);
        }
        else
        {
          if (dot (ent->get_abs_po ().get_z_facing (), dir) > 0)
            my_entity->play_anim (ANIM_PRIMARY, my_name_anims[MISC_SURFER1_WIPEOUT_FALL_RIGHT], BLEND_TIME, ANIM_TWEEN | ANIM_NONCOSMETIC);
          else
            my_entity->play_anim (ANIM_PRIMARY, my_name_anims[MISC_SURFER1_WIPEOUT_FALL_LEFT], BLEND_TIME, ANIM_TWEEN | ANIM_NONCOSMETIC);
        }
      }

      my_state = MISC_SURFER1_STATE_COLLIDE;
    }

    break;

	case BOOGIEBOARDER:
		if (my_state != BOOGIEBOARDER_STATE_COLLIDE)
		{
			my_entity->play_anim (ANIM_PRIMARY, my_name_anims[BOOGIEBOARDER_COLLIDE], BLEND_TIME, ANIM_TWEEN | ANIM_NONCOSMETIC);
			my_state = BOOGIEBOARDER_STATE_COLLIDE;
		}

		break;

	case JETSKIER:
		if (my_state != JETSKIER_STATE_COLLIDE)
		{
			my_entity->play_anim (ANIM_PRIMARY, my_name_anims[JETSKIER_COLLIDE], BLEND_TIME, ANIM_TWEEN | ANIM_NONCOSMETIC);
			my_state = JETSKIER_STATE_COLLIDE;
		}

		break;

	case WINDSURFER:
		if ((my_state != WINDSURFER_STATE_COLLIDE) && (my_state != WINDSURFER_STATE_COLLIDE_AIR))
		{
			if (my_state == WINDSURFER_STATE_TRICK)
			{
				my_entity->play_anim (ANIM_PRIMARY, my_name_anims[WINDSURFER_COLLIDE_IN_AIR], BLEND_TIME, ANIM_TWEEN | ANIM_NONCOSMETIC);
				if (my_board_entity)
					my_board_entity->play_anim (ANIM_PRIMARY, my_board_name_anims[WINDSURFER_COLLIDE_IN_AIR], BLEND_TIME, ANIM_TWEEN | ANIM_NONCOSMETIC);
				my_state = WINDSURFER_STATE_COLLIDE_AIR;
			}
			else
			{
				my_entity->play_anim (ANIM_PRIMARY, my_name_anims[WINDSURFER_COLLIDE], BLEND_TIME, ANIM_TWEEN | ANIM_NONCOSMETIC);
				if (my_board_entity)
					my_board_entity->play_anim (ANIM_PRIMARY, my_board_name_anims[WINDSURFER_COLLIDE], BLEND_TIME, ANIM_TWEEN | ANIM_NONCOSMETIC);
				my_state = WINDSURFER_STATE_COLLIDE;
			}
		}

		break;

  case KAYAKER:
  case OUTRIGGER:
		if (my_state != KAYAKER_STATE_HIT_RIGHT && my_state != KAYAKER_STATE_HIT_LEFT)
		{
	    my_entity->play_anim (ANIM_PRIMARY, my_name_anims[KAYAKER_HIT], BLEND_TIME, ANIM_TWEEN | ANIM_NONCOSMETIC);

		  if (ent->get_abs_position ().x > my_entity->get_abs_position ().x)
			  my_state = KAYAKER_STATE_HIT_RIGHT;
		  else
			  my_state = KAYAKER_STATE_HIT_LEFT;

	    timer2 = 0;
		}

    break;

  default:
    break;
  }
}

// ============================================================================
// split this file, please

const char* generic_anim_misc::generic_anim_names[] = { "IDLE", "COLLIDE", "COLLIDEI", "JUMP", "SPRAY" };
const char* generic_anim_animal::generic_anim_names[] = { "IDLE", "DIVE" };
const char* generic_anim_ice::generic_anim_names[] = { "IDLE", "COLLIDE", "COLLIDEI" };

generic_anim_misc::generic_anim_misc (entity *entity, const stringx& path, const stringx& name)
	: generic_anim (path, name)
{
	construct (&entity, path, name, NULL, 1);
}

generic_anim_misc::generic_anim_misc (entity **entities, const stringx& path, const stringx& name, const char **prefixes, int count)
	: generic_anim (path, name)
{
	construct (entities, path, name, prefixes, count);
}

void generic_anim_misc::construct (entity **entities, const stringx& path, const stringx& name, const char **prefixes, int count)
{
	int i, j;

	items_count = count;
	items_prefixes = NEW stringx[count];
	my_entities = NEW entity* [count];

	for (i = 0; i < count; i++)
	{
		if (i != 0)
			items_prefixes[i] = prefixes[i];
		my_entities[i] = entities[i];
	}

	for (i = 0; i < 5; i++)
	{
		generic_anims[i] = 0;

		for (;;)
	  {
			for (j = 0; j < count; j++)
			{
				char anim[256];
				bool play = (i == 0);

				if (j == 0)
					sprintf (anim, "%s\\ANIMATIONS\\%s_%s_%d", path.c_str(), name.c_str(), generic_anim_names[i], generic_anims[i] + 1);
				else
					sprintf (anim, "%s\\ANIMATIONS\\%s_%s_%s_%d", path.c_str(), name.c_str(), prefixes[j], generic_anim_names[i], generic_anims[i] + 1);

				if (file_finder_exists (anim, entity_track_tree::extension ()))
		    {
					strcat (anim, entity_track_tree::extension ());
					my_entities[j]->load_anim (anim);

					if (play)
					{
					  my_entities[j]->play_anim (ANIM_PRIMARY, anim, 0, ANIM_NONCOSMETIC);
						play = false;
					}
				}
		    else
					break;
			}

			if (j == count)
				generic_anims[i]++;
			else
				break;
	  }
	}

#ifdef DEBUG
	anim_labels = (FloatingText**)malloc (count * sizeof (FloatingText*));
	stringx none = "NONE";
	color32 yel = color32(255, 255, 0, 255);
	for (int i = 0; i < count; i++)
		anim_labels[i] = NEW FloatingText(&frontendmanager.font_info, none, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, yel);
#endif
}

void generic_anim_misc::update (bool collide, bool jump, bool spray, float *alpha)
{
	if (dummy)
		return;

  entity_anim_tree* anim_tree = my_entities[0]->get_anim_tree (ANIM_PRIMARY);
  int play = -1;

  if (generic_anim_state == GA_SPAWN)
  {
    play = generic_anim_state = GA_IDLE;
  }
  else if ((generic_anim_state == GA_IDLE) && (collide || jump || spray))
  {
    // check for collision/spray/jump
    if (collide)
    {
      generic_anim_state = play = GA_COLLIDE;
    }
    else if (jump)
    {
      generic_anim_state = play = GA_JUMP;
    }
    else if (spray)
    {
      generic_anim_state = play = GA_SPRAY;
    }
  }
  else if (anim_tree && anim_tree->is_finished () && anim_tree->is_done_tween ())
  {
    switch (generic_anim_state)
    {
    case GA_IDLE: generic_anim_state = GA_IDLE; break;
    case GA_COLLIDE: generic_anim_state = GA_AFTER_COLLIDE; break;
    case GA_AFTER_COLLIDE: generic_anim_state = GA_AFTER_COLLIDE; break;
    case GA_JUMP: generic_anim_state = GA_IDLE; break;
    case GA_SPRAY: generic_anim_state = GA_IDLE; break;
    }
    play = generic_anim_state;
  }

  if ((play != -1) && (generic_anims[play] > 0))
  {
		int num = random(generic_anims[play]) + 1;
		for (int j = 0; j < items_count; j++)
		{
			char anim[256];
			if (j == 0)
				sprintf (anim, "%s_%s_%d", my_base_name.c_str(), generic_anim_names[play], num);
			else
				sprintf (anim, "%s_%s_%s_%d", my_base_name.c_str(), items_prefixes[j].c_str(), generic_anim_names[play], num);

	    my_entities[j]->play_anim (ANIM_PRIMARY, anim, BLEND_TIME, ANIM_TWEEN | ANIM_NONCOSMETIC);
		}
  }
}

void generic_anim_misc::switch_anims ()
{
	if (!dummy)
		return;

	input_mgr* inputmgr = input_mgr::inst();

	if (inputmgr->get_control_state(JOYSTICK_DEVICE, PSX_L1) == AXIS_MAX)
	{
		if (!left_down)
		{
			left_down = true;
			cur_anim--;

			if (cur_anim <= 0)
			{
				cur_state--;

				if (cur_state < 0)
					cur_state = GA_SPRAY;

				cur_anim = generic_anims[cur_state];
			}

			for (int j = 0; j < items_count; j++)
			{
				char anim[256];
				if (j == 0)
					sprintf (anim, "%s_%s_%d", my_base_name.c_str(), generic_anim_names[cur_state], cur_anim);
				else
					sprintf (anim, "%s_%s_%s_%d", my_base_name.c_str(), items_prefixes[j].c_str(), generic_anim_names[cur_state], cur_anim);

		    my_entities[j]->play_anim (ANIM_PRIMARY, anim, BLEND_TIME, ANIM_LOOPING | ANIM_TWEEN | ANIM_NONCOSMETIC);
			}
		}
	}
	else
	{
		left_down = false;
	}

	if (inputmgr->get_control_state(JOYSTICK_DEVICE, PSX_R1) == AXIS_MAX)
	{
		if (!right_down)
		{
			right_down = true;
			cur_anim++;

			if (cur_anim > generic_anims[cur_state])
			{
				cur_anim = 1;
				cur_state++;
				if (cur_state > GA_SPRAY)
					cur_state = 0;
			}

			for (int j = 0; j < items_count; j++)
			{
				char anim[256];
				if (j == 0)
					sprintf (anim, "%s_%s_%d", my_base_name.c_str(), generic_anim_names[cur_state], cur_anim);
				else
					sprintf (anim, "%s_%s_%s_%d", my_base_name.c_str(), items_prefixes[j].c_str(), generic_anim_names[cur_state], cur_anim);

		    my_entities[j]->play_anim (ANIM_PRIMARY, anim, BLEND_TIME, ANIM_LOOPING | ANIM_TWEEN | ANIM_NONCOSMETIC);
			}
		}
	}
	else
	{
		right_down = false;
	}
}

#ifdef DEBUG
void generic_anim_misc::draw_debug_labels ()
{
	for (int i = 0; i < items_count; i++)
	{
		if (my_entities[i]->get_anim_tree (ANIM_PRIMARY))
			anim_labels[i]->changeText(my_entities[i]->get_anim_tree (ANIM_PRIMARY)->get_name());
		anim_labels[i]->SetLocation3D(my_entities[i]->get_abs_position());
		anim_labels[i]->UpdateInScene(true);
		anim_labels[i]->Draw();
	}
}
#endif

generic_anim_animal::generic_anim_animal (entity *entity, const stringx& path, const stringx& name)
	: generic_anim (path, name)
{
	bool play = true;

	my_entity = entity;

  for (int i = 0; i < 2; i++)
  {
    generic_anims[i] = 0;

    for (;;)
    {
      char anim[256];
      sprintf (anim, "%s\\ANIMATIONS\\%s_%s_%d", path.c_str(), name.c_str(), generic_anim_names[i], generic_anims[i] + 1);

      if (file_finder_exists (anim, entity_track_tree::extension ()))
      {
        strcat (anim, entity_track_tree::extension ());
        my_entity->load_anim (anim);
        generic_anims[i]++;

				if (play)
				{
				  my_entity->play_anim (ANIM_PRIMARY, anim, 0, ANIM_NONCOSMETIC);
					play = false;
				}
      }
      else
        break;
    }
  }

#ifdef DEBUG
	stringx none = "NONE";
	color32 yel = color32(255, 255, 0, 255);
	anim_label = NEW FloatingText(&frontendmanager.font_info, none, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, yel);
#endif
}

void generic_anim_animal::update (bool collide, bool jump, bool spray, float *alpha)
{
  entity_anim_tree* anim_tree = my_entity->get_anim_tree (ANIM_PRIMARY);
  int play = -1;

  if (generic_anim_state == AA_SPAWN)
  {
    play = generic_anim_state = AA_IDLE;
  }
  else if ((generic_anim_state == AA_IDLE) && (collide || jump || spray))
  {
    // check for collision/spray/jump
    if (collide || jump || spray)
    {
      generic_anim_state = play = AA_DIVE;
    }
  }
  else if (anim_tree && anim_tree->is_finished () && anim_tree->is_done_tween ())
  {
    switch (generic_anim_state)
    {
    case AA_IDLE: generic_anim_state = AA_IDLE; break;
    case AA_DIVE: generic_anim_state = AA_IDLE; break;
    }
    play = generic_anim_state;
  }

  if ((play != -1) && (generic_anims[play] > 0))
  {
    char anim[256];
    sprintf (anim, "%s_%s_%d", my_base_name.c_str (), generic_anim_names[play], random(generic_anims[play]) + 1);

    my_entity->play_anim (ANIM_PRIMARY, anim, BLEND_TIME, ANIM_TWEEN | ANIM_NONCOSMETIC);
  }

	if (generic_anim_state == AA_DIVE)
	{
		*alpha = anim_tree->get_time_remaining () / anim_tree->get_duration ();
	}
}

void generic_anim_animal::switch_anims ()
{
	if (!dummy)
		return;

	input_mgr* inputmgr = input_mgr::inst();

	if (inputmgr->get_control_state(JOYSTICK_DEVICE, PSX_L1) == AXIS_MAX)
	{
		if (!left_down)
		{
			left_down = true;
			cur_anim--;

			if (cur_anim <= 0)
			{
				cur_state--;

				if (cur_state < 0)
					cur_state = AA_DIVE;

				cur_anim = generic_anims[cur_state];
			}

			char anim[256];
			sprintf (anim, "%s_%s_%d", my_base_name.c_str(), generic_anim_names[cur_state], cur_anim);
	    my_entity->play_anim (ANIM_PRIMARY, anim, BLEND_TIME, ANIM_LOOPING | ANIM_TWEEN | ANIM_NONCOSMETIC);
		}
	}
	else
	{
		left_down = false;
	}

	if (inputmgr->get_control_state(JOYSTICK_DEVICE, PSX_R1) == AXIS_MAX)
	{
		if (!right_down)
		{
			right_down = true;
			cur_anim++;

			if (cur_anim > generic_anims[cur_state])
			{
				cur_anim = 1;
				cur_state++;
				if (cur_state > AA_DIVE)
					cur_state = 0;
			}

			char anim[256];
			sprintf (anim, "%s_%s_%d", my_base_name.c_str(), generic_anim_names[cur_state], cur_anim);
			my_entity->play_anim (ANIM_PRIMARY, anim, BLEND_TIME, ANIM_LOOPING | ANIM_TWEEN | ANIM_NONCOSMETIC);
		}
	}
	else
	{
		right_down = false;
	}
}

#ifdef DEBUG
void generic_anim_animal::draw_debug_labels ()
{
	if (my_entity->get_anim_tree (ANIM_PRIMARY))
		anim_label->changeText(my_entity->get_anim_tree (ANIM_PRIMARY)->get_name());
	anim_label->SetLocation3D(my_entity->get_abs_position());
	anim_label->UpdateInScene(true);
	anim_label->Draw();
}
#endif

generic_anim_ice::generic_anim_ice (entity *entity, const stringx& path, const stringx& name)
	: generic_anim (path, name)
{
	my_entity = entity;

  for (int i = 0; i < 3; i++)
  {
    generic_anims[i] = 0;

    for (;;)
    {
      char anim[256];
      sprintf (anim, "%s\\ANIMATIONS\\%s_%s_%d", path.c_str(), name.c_str(), generic_anim_names[i], generic_anims[i] + 1);

      if (file_finder_exists (anim, entity_track_tree::extension ()))
      {
        strcat (anim, entity_track_tree::extension ());
        my_entity->load_anim (anim);
        generic_anims[i]++;
      }
      else
        break;
    }
  }

#ifdef DEBUG
	stringx none = "NONE";
	color32 yel = color32(255, 255, 0, 255);
	anim_label = NEW FloatingText(&frontendmanager.font_info, none, Font::HORIZJUST_CENTER, Font::VERTJUST_CENTER, yel);
#endif
}

void generic_anim_ice::update (bool collide, bool jump, bool spray, float *alpha)
{
  entity_anim_tree* anim_tree = my_entity->get_anim_tree (ANIM_PRIMARY);
	float blend = BLEND_TIME;
  int play = -1;

  if (generic_anim_state == IA_SPAWN)
  {
    play = generic_anim_state = IA_IDLE;
  }
  else if ((generic_anim_state == IA_IDLE) && (collide || jump || spray))
  {
    // check for collision/spray/jump
    if (collide)
    {
      generic_anim_state = play = IA_COLLIDE;
    }
  }
  else if (anim_tree && anim_tree->is_finished () && anim_tree->is_done_tween ())
  {
    switch (generic_anim_state)
    {
    case IA_IDLE: blend = 0; generic_anim_state = IA_IDLE; break;
    case IA_COLLIDE: generic_anim_state = IA_COLLIDEI; break;
		case IA_COLLIDEI: generic_anim_state = IA_COLLIDEI; break;
    }
    play = generic_anim_state;
  }

  if ((play != -1) && (generic_anims[play] > 0))
  {
    char anim[256];
    sprintf (anim, "%s_%s_%d", my_base_name.c_str (), generic_anim_names[play], random(generic_anims[play]) + 1);

    my_entity->play_anim (ANIM_PRIMARY, anim, blend, ANIM_TWEEN | ANIM_NONCOSMETIC);
  }

	if (generic_anim_state == IA_COLLIDE)
	{
		*alpha = anim_tree->get_time_remaining () / anim_tree->get_duration ();
	}
}

void generic_anim_ice::switch_anims ()
{
	if (!dummy)
		return;

	input_mgr* inputmgr = input_mgr::inst();

	if (inputmgr->get_control_state(JOYSTICK_DEVICE, PSX_L1) == AXIS_MAX)
	{
		if (!left_down)
		{
			left_down = true;
			cur_anim--;

			if (cur_anim <= 0)
			{
				cur_state--;

				if (cur_state < 0)
					cur_state = IA_COLLIDEI;

				cur_anim = generic_anims[cur_state];
			}

			char anim[256];
			sprintf (anim, "%s_%s_%d", my_base_name.c_str(), generic_anim_names[cur_state], cur_anim);
	    my_entity->play_anim (ANIM_PRIMARY, anim, BLEND_TIME, ANIM_LOOPING | ANIM_TWEEN | ANIM_NONCOSMETIC);
		}
	}
	else
	{
		left_down = false;
	}

	if (inputmgr->get_control_state(JOYSTICK_DEVICE, PSX_R1) == AXIS_MAX)
	{
		if (!right_down)
		{
			right_down = true;
			cur_anim++;

			if (cur_anim > generic_anims[cur_state])
			{
				cur_anim = 1;
				cur_state++;
				if (cur_state > IA_COLLIDEI)
					cur_state = 0;
			}

			char anim[256];
			sprintf (anim, "%s_%s_%d", my_base_name.c_str(), generic_anim_names[cur_state], cur_anim);
			my_entity->play_anim (ANIM_PRIMARY, anim, BLEND_TIME, ANIM_LOOPING | ANIM_TWEEN | ANIM_NONCOSMETIC);
		}
	}
	else
	{
		right_down = false;
	}
}

#ifdef DEBUG
void generic_anim_ice::draw_debug_labels ()
{
	if (my_entity->get_anim_tree (ANIM_PRIMARY))
		anim_label->changeText(my_entity->get_anim_tree (ANIM_PRIMARY)->get_name());
	anim_label->SetLocation3D(my_entity->get_abs_position());
	anim_label->UpdateInScene(true);
	anim_label->Draw();
}
#endif

