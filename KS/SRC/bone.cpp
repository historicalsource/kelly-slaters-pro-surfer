#include "global.h"

#include "bone.h"
#include "link_interface.h"

#ifdef TARGET_PC
  #define TEST_WARP_BUGS 0
#else
  #define TEST_WARP_BUGS 0
#endif

ENTITY_INTERFACE_CPP(bone, link)
/*
link_interface * bone::create_link_ifc()
{
  assert(!my_link_interface);
  my_link_interface = NEW link_interface(this);
  return my_link_interface;
}

void bone::destroy_link_ifc()
{
  assert(my_link_interface);

  // it's fixed now, I swear.  -jdf
  delete my_link_interface;

  my_link_interface = NULL;
}
*/
int break_on_me = 0;



#if TEST_WARP_BUGS

#include "wds.h"
#include "game.h"
#include "console.h"
vector3d last_pos = ZEROVEC;
bool first_assert = true;
bool ignore_ZEROVEC = false;
vector3d test_spidey_front;

extern game *g_game_ptr;

stringx test_ent_name = "BLUE_DT_ALLY_001";

rational_t max_dist = 10.0f;

entity *test_ent = NULL;

#define TEST_HERO_ONLY 1

// GCS_RANDOMBURNA12
// 40.83f, 7.96f, 2.17f

void bone::po_changed()
{
  #if TEST_HERO_ONLY
    if(g_world_ptr && this == g_world_ptr->get_hero_ptr())
  #else
    if(g_world_ptr && (this == test_ent || (test_ent == NULL && test_ent_name == ((entity *)this)->get_name())))
  #endif
    {
      test_ent = (entity *)this;
      update_abs_po_reverse();

  //    if(!first_assert)
  //      assert(dot(test_spidey_front, get_abs_po().get_z_facing()) > 0.5f);
      test_spidey_front = get_abs_po().get_z_facing();

      vector3d pos=get_abs_position();
      vector3d delta = pos - last_pos;
      rational_t len = delta.length();
      if(len > max_dist)
      {
#ifdef _DEBUG
        assert(first_assert);
#elif _CONSOLE_ENABLE
        console_log("%s pos changed <%.2f, %.2f, %.2f> to <%.2f, %.2f, %.2f> (%.2fm)", test_ent->get_name().c_str(), last_pos.x, last_pos.y, last_pos.z, pos.x, pos.y, pos.z, len);
#endif
        first_assert = false;
      }
      last_pos = pos;
    }


  /*
    else if(g_game_ptr && g_game_ptr->level_is_loaded() && !ignore_ZEROVEC)
    {
      vector3d pos=get_abs_position();
      assert(pos != ZEROVEC);
    }
  */

  // derived classes may want to know when po changes
  // we just want to make sure po isn't corrupt
  vector3d pos=get_abs_position();
  if (!pos.is_valid())
    break_on_me ++;
  assert(pos.is_valid());
}

#else

void bone::po_changed()
{

  vector3d pos=get_abs_position();
#if defined(BUILD_DEBUG) && !defined(TOBY)
  if (!pos.is_valid())
	{
    break_on_me ++;
		warning("Bad bones, bad bones, watcha gonna do? Watcha do when your po is poo\n");
		return;
	}
  //assert(pos.is_valid());
#else
  if (!pos.is_valid())
    return;
#endif
  // update children if necessary
  if( has_link_ifc() )
  {
    bone* child_iterator = link_ifc()->get_first_child();
    for(; child_iterator;)
    {
      child_iterator->po_changed();
      if( child_iterator->has_link_ifc())
        child_iterator = child_iterator->link_ifc()->get_next_sibling();
      else
        break;
    }
  }
}

#endif


void bone::reset_scale ()
{
  // fix "balloon surfer" bug, not the ideal solution but it works
  vector3d x (my_rel_po.get_x_facing ());
  vector3d y (my_rel_po.get_y_facing ());
  vector3d z (my_rel_po.get_z_facing ());

  x.normalize ();
  y.normalize ();
  z.normalize ();

  my_rel_po = po (x, y, z, my_rel_po.get_position ());

  // update children if necessary
  if (has_link_ifc ())
  {
    bone* child_iterator = link_ifc ()->get_first_child ();
    for (; child_iterator;)
    {
      child_iterator->reset_scale ();
      if (child_iterator->has_link_ifc ())
        child_iterator = child_iterator->link_ifc ()->get_next_sibling ();
      else
        break;
    }
  }
}
