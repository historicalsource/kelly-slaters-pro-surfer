#include "global.h"
#include "ai_interface.h"

// BIGCULL #include "ai_senses.h"
// BIGCULL #include "ai_communication.h"

#include "ai_locomotion.h"
// BIGCULL #include "ai_locomotion_heli.h"
// BIGCULL #include "ai_locomotion_walk.h"
// BIGCULL #include "ai_locomotion_winged.h"
// BIGCULL #include "ai_locomotion_direct.h"

#include "ai_goals.h"
// BIGCULL #include "ai_goals_combat.h"
// BIGCULL #include "ai_goals_heli.h"
// BIGCULL #include "ai_goals_shocker.h"
// BIGCULL #include "ai_goals_vulture.h"
// BIGCULL #include "ai_goals_kraven.h"
// BIGCULL #include "ai_aim.h"
#ifdef GCCULL
#include "ai_voice.h"
#endif

// BIGCULL #include "ai_fear_constants.h"
// BIGCULL #include "ai_constants.h"

#include "entity.h"
#include "random.h"
#include "wds.h"
#include "entity_maker.h"
#include "entityflags.h"
// BIGCULL #include "handheld_item.h"
#include "item.h"
 // BIGCULL #include "gun.h"
// BIGCULL #include "thrown_item.h"
#include "file_finder.h"
#include "debug_render.h"
#include "sound_interface.h"
#include "time_interface.h"
#include "terrain.h"
#include "collide.h"
#include "profiler.h"
#include "osdevopts.h"
#include "physical_interface.h"
#include "animation_interface.h"
// BIGCULL #include "scanner.h"

#include "kellyslater_ai_goals.h"
#include "kellyslater_controller.h"

#if 0 // BIGCULL
void ai_threat::clear(ai_interface *own)
{
  if(own != NULL && ai != NULL)
    ai->remove_attacker(own);

  ai = NULL;
  ent = NULL;
  dist = 0.0f;
  power = 0.0f;
  threat_level = 0;
}


void ai_dropshadow::update(bool visible, const color32 &render_color)
{
  if(visible)
  {
    vector3d pos = ent->get_abs_position();
    vector3d end = pos - (YVEC*1000.0f);
    vector3d hit, hitn;

    bool intersect = find_intersection(pos, end, ent->get_primary_region(), (FI_COLLIDE_WORLD | FI_COLLIDE_ENTITY | FI_COLLIDE_ENTITY_NO_CAPSULES), &hit, &hitn);

    if(intersect)
      pos = hit + (hitn*0.01f);
    else
    {
      pos.y = ent->get_abs_position().y;
      assert(0);
    }

		vector3d x1, z1;
    x1.x = hitn.y;
    x1.y = hitn.x;
    x1.z = 0.0f;
    x1.normalize();

    z1.x = (x1.y*hitn.z - x1.z*hitn.y);
    z1.y = (x1.z*hitn.x - x1.x*hitn.z);
    z1.z = (x1.x*hitn.y - x1.y*hitn.x);
    z1.normalize();

    po the_po(ent->get_abs_po().get_facing(), hitn, pos);

    rational_t rad1 = dropshadow->get_radius();
    rational_t rad2 = ent->get_radius();
    rational_t scale = (rad2 / rad1) * scale_mod;

    if(scale <= 0.0f)
      scale = scale_mod;

    if(scale > 0.0f)
    {
      po scale_po;
      vector3d scale_vec = vector3d(scale, scale, scale);
      scale_po.set_scale(scale_vec);

      fast_po_mul(the_po, scale_po, the_po);

      dropshadow->set_rel_po(the_po);
      dropshadow->force_regions(ent);
    }
    else
      visible = false;
  }

  dropshadow->set_visible(visible);
  dropshadow->set_render_color(render_color);
}



#endif // BIGCULL


list<ai_interface *> ai_interface::all_ai_interfaces;


ai_interface::ai_interface(entity *ent)
  : entity_interface(ent),
  flags(0),
  team(0),
  enemy_team(0),
  disable_count(0)
{
// BIGCULL   desired_threat = NULL;

  //BIGCULL radio = NEW ai_radio(this);
#ifdef GCCULL
  voice = NEW ai_voice_box(this);
#endif

  //BIGCULL eyes = NEW ai_sight_sense(this);
  //BIGCULL ears = NEW ai_sound_sense(this);
  locomotion = NULL;

  all_ai_interfaces.push_back(this);

  target = NULL;

  current_goal = NULL;
#if 0 // BIGCULL
  threat_assesment_freq = 0.5f;
  threat_timer = threat_assesment_freq * __fabs(PLUS_MINUS_ONE);
  threat_sticky_timer = 0.0f;

  attackers.reserve(3);
  attackers.resize(0);

  set_allow_wounded(true);

  fear_decay = 5.0f;
  fear_mod = 1.0f;
  internal_fear = 0.0f;
  external_fear = 0.0f;
  internal_fear_timer = 0.25f * __fabs(PLUS_MINUS_ONE);
#endif // BIGCULL

  help_timer = 0.0f;

  goals.reserve(10);
  goals.resize(0);

// BIGCULL   goals.push_back(NEW disable_ai_goal(this));

/*
  if(my_entity->is_a_conglomerate())
  {
    scanner *scan = ((conglomerate *)my_entity)->get_scanner();
    if(scan)
      scan->set_ai_owner(this);
  }
*/
}

ai_interface::~ai_interface()
{
  list<ai_interface *>::iterator ai = all_ai_interfaces.begin();
  while(ai != all_ai_interfaces.end())
  {
    if(*ai == this)
    {
      all_ai_interfaces.erase(ai);
      break;
    }
    else
      ++ai;
  }

//BIGCULL
  /*
  if(radio != NULL)
  {
    delete radio;
    radio = NULL;
  }
  */

#ifdef GCCULL
  if(voice != NULL)
  {
    delete voice;
    voice = NULL;
  }
#endif

#if 0 //BIGCULL
  if(eyes != NULL)
  {
    delete eyes;
    eyes = NULL;
  }

  if(ears != NULL)
  {
    delete ears;
    ears = NULL;
  }
#endif//BIGCULL
  if(locomotion != NULL)
  {
    delete locomotion;
    locomotion = NULL;
  }

  vector<ai_goal *>::iterator g = goals.begin();
  while(g != goals.end())
  {
    delete (*g);
    ++g;
  }
  goals.resize(0);

#if 0 // BIGCULL
  vector<ai_auto_aim *>::iterator aim = aimers.begin();
  while(aim != aimers.end())
  {
    delete (*aim);
    ++aim;
  }
  aimers.resize(0);

  attackers.resize(0);
#endif // BIGCULL
}

void ai_interface::copy(ai_interface *b)
{
  goals.reserve(b->goals.size());

  vector<ai_goal *>::iterator g = b->goals.begin();
  while(g != b->goals.end())
  {
    static pstring disable_goal("DISABLE");
    if((*g)->get_type() != disable_goal)
      goals.push_back((*g)->make_copy(this));
    ++g;
  }

#if 0 // BIGCULL
  vector<ai_auto_aim *>::iterator aim = b->aimers.begin();
  while(aim != b->aimers.end())
  {
    ai_auto_aim *aimer = (*aim)->make_copy(this);
    aimer->set_active(false);
    aimers.push_back(aimer);
    ++aim;
  }

  vector<ai_dropshadow>::iterator di = b->dropshadows.begin();
  while(di != b->dropshadows.end())
  {
    entity *ent = my_entity;
    entity *shadow = g_entity_maker->create_entity_or_subclass("items\\entities\\dropshadow", entity_id::make_unique_id(), po_identity_matrix, empty_string);
    rational_t scale = (*di).scale_mod;

    if((*di).ent != b->my_entity && my_entity->is_a_conglomerate() && b->my_entity->is_a_conglomerate())
    {
      stringx node = ((conglomerate *)b->my_entity)->get_member_nodename((*di).ent);
      ent = ((conglomerate *)my_entity)->get_member(node);
    }

    if(ent && shadow)
      dropshadows.push_back(ai_dropshadow(ent, shadow, scale));

    ++di;
  }
#endif // BIGCULL

  if(b->locomotion)
    locomotion = b->locomotion->make_copy(this);

#if 0 // BIGCULL
  team = b->team;
  enemy_team = b->enemy_team;
  threat_assesment_freq = b->threat_assesment_freq;
  threat_timer = threat_assesment_freq * __fabs(PLUS_MINUS_ONE);
#endif

  //BIGCULL eyes->copy(b->eyes);
  //BIGCULL ears->copy(b->ears);

  set_cosmetic(b->cosmetic());
  set_detector(b->detector());
  // BIGCULL set_flag(_AI_ADV_SHADOW, b->has_adv_dropshadow());
}

void ai_interface::read_goal_pack(const stringx &file)
{
  stringx       filename;

  if (!file_finder_exists(file, stringx(".gpk"), &filename) )
  {
    error("File '%s' not found!", file.c_str());
    return;
  }

	chunk_file    fs;
  fs.open( filename, os_file::FILE_READ | chunk_file::FILE_TEXT );

  stringx label;
  for ( serial_in(fs,&label); label!=chunkend_label && label.size() > 0; serial_in(fs,&label) )
    read_goal(fs, label);

  fs.close();
}

void ai_interface::read_ai_file(const stringx &file)
{
  stringx       filename;
  if ( !file_finder_exists(file, stringx(".ai"), &filename) )
  {
    error("File '%s' not found!", file.c_str());
    return;
  }

	chunk_file    fs;
  fs.open( filename, os_file::FILE_READ | chunk_file::FILE_TEXT );
  read_data(fs);
  fs.close();
}

void ai_interface::read_goal(chunk_file &fs, const stringx &label)
{
  ai_goal *goal = NULL;
  stringx tmp = label;
  tmp.to_lower();
  /*
#if defined (PROJECT_KELLYSLATER)
  nglPrintf("Project Kellyslater is working.");
#endif
#define MAC(a, b, c)  { krPrintf((b));}
  #include "ai_goals_mac.h"
#undef MAC
*/
#define MAC(a, b, c)  { static const char* check = ##a##; static pstring gname(##b##); if( strcmp( tmp.c_str(), check )==0) { goal = get_goal(gname); if(goal == NULL) { goal = NEW c##_ai_goal(this); goals.push_back(goal); } goal->read_data(fs); return; } }
  #include "ai_goals_mac.h"
#undef MAC

#ifdef WEENIEASSERT  // this triggers every time KSPS starts so it's gon
  error("Unknown goal type '%s' for entity '%s' in file '%s'", label.c_str(), my_entity->get_name().c_str(), fs.get_filename().c_str());
#endif
}


#if 0 // BIGCULL
void ai_interface::read_aimer(chunk_file &fs, const stringx &label)
{
  ai_auto_aim *aimer = NULL;
  stringx tmp = label;
  tmp.to_lower();

  stringx _id;
  serial_in(fs,&_id);

  static pstring id;
  id = _id;

  aimer = get_aimer(id);

  if(aimer == NULL)
  {
    aimer = NEW ai_auto_aim(this, id);

    if(aimer != NULL)
    {
      aimer->set_active(false);
      aimers.push_back(aimer);
    }
  }

  if(aimer != NULL)
    aimer->read_data(fs);
}
#endif // BIGCULL

void ai_interface::read_data(chunk_file& fs)
{
  stringx label;
  for ( serial_in(fs,&label); label!=chunkend_label && label.size() > 0; serial_in(fs,&label) )
  {
    if(label == "ai_file")
    {
      serial_in(fs,&label);
      read_ai_file(label);
    }
    else if(label == "goals:")
    {
      for ( serial_in(fs,&label); label!=chunkend_label; serial_in(fs,&label) )
      {
        if(label == "packages:")
        {
          for ( serial_in(fs,&label); label!=chunkend_label; serial_in(fs,&label) )
            read_goal_pack(label);
        }
        else
          read_goal(fs, label);
      }
    }
#if 0 // BIGCULL
    else if(label == "aimers:")
    {
      for ( serial_in(fs,&label); label!=chunkend_label; serial_in(fs,&label) )
        read_aimer(fs, label);
    }
#endif // BIGCULL
    else if(label == "locomotion:")
    {
      serial_in(fs,&label);
      label.to_lower();

      if(locomotion == NULL && label != "type")
        error("Need to specify a type of locomotion for entity '%s'", my_entity->get_name().c_str());

      if(label == "type")
      {
        serial_in(fs,&label);

        label.to_lower();
#if 0 // BIGCULL
        if(label == "walk" && (locomotion == NULL || locomotion->type != LOCOMOTION_WALK))
        {
          if(locomotion != NULL)
            delete locomotion;
          locomotion = NEW ai_locomotion_walk(this);
        }
        else if((label == "helicopter" || label == "heli") && (locomotion == NULL || locomotion->type != LOCOMOTION_HELI))
        {
          if(locomotion != NULL)
            delete locomotion;
          locomotion = NEW ai_locomotion_helicopter(this);
        }
        else if(label == "direct" && (locomotion == NULL || locomotion->type != LOCOMOTION_DIRECT))
        {
          if(locomotion != NULL)
            delete locomotion;
          locomotion = NEW ai_locomotion_direct(this);
        }
        else if(label == "winged" && (locomotion == NULL || locomotion->type != LOCOMOTION_WINGED))
        {
          if(locomotion != NULL)
            delete locomotion;
          locomotion = NEW ai_locomotion_winged(this);
        }
        else
#endif // BIGCULL
          error("Unknown locomotion type '%s' for entity '%s'", label.c_str(), my_entity->get_name().c_str());

        serial_in(fs,&label);
      }

      if(locomotion && label != chunkend_label)
      {
        for ( ; label!=chunkend_label; serial_in(fs,&label) )
        {
          label.to_lower();
          locomotion->handle_chunk(fs, label);
        }
      }
    }
#if 0 // BIGCULL
    else if(label == "teams:")
    {
      for ( serial_in(fs,&label); label!=chunkend_label; serial_in(fs,&label) )
      {
        if(label == "team")
        {
          serial_in(fs,&label);

          set_team(_TEAM_ALIGNMENT_MASK, false);

          if(label == "EVIL_1")
            set_team(_TEAM_EVIL1, true);
          else if(label == "EVIL_2")
            set_team(_TEAM_EVIL2, true);
          else if(label == "EVIL_3")
            set_team(_TEAM_EVIL3, true);
          else if(label == "EVIL_4")
            set_team(_TEAM_EVIL4, true);
          else if(label == "EVIL_5")
            set_team(_TEAM_EVIL5, true);
          else if(label == "EVIL_6")
            set_team(_TEAM_EVIL6, true);
          else if(label == "EVIL_7")
            set_team(_TEAM_EVIL7, true);
          else if(label == "EVIL_8")
            set_team(_TEAM_EVIL8, true);
          else if(label == "GOOD_1")
            set_team(_TEAM_GOOD1, true);
          else if(label == "GOOD_2")
            set_team(_TEAM_GOOD2, true);
          else if(label == "GOOD_3")
            set_team(_TEAM_GOOD3, true);
          else if(label == "GOOD_4")
            set_team(_TEAM_GOOD4, true);
          else if(label == "GOOD_5")
            set_team(_TEAM_GOOD5, true);
          else if(label == "GOOD_6")
            set_team(_TEAM_GOOD6, true);
          else if(label == "GOOD_7")
            set_team(_TEAM_GOOD7, true);
          else if(label == "GOOD_8")
            set_team(_TEAM_GOOD8, true);
          else if(label == "NEUT_1")
            set_team(_TEAM_NEUT1, true);
          else if(label == "NEUT_2")
            set_team(_TEAM_NEUT2, true);
          else if(label == "NEUT_3")
            set_team(_TEAM_NEUT3, true);
          else if(label == "NEUT_4")
            set_team(_TEAM_NEUT4, true);
          else if(label == "NEUT_5")
            set_team(_TEAM_NEUT5, true);
          else if(label == "NEUT_6")
            set_team(_TEAM_NEUT6, true);
          else if(label == "NEUT_7")
            set_team(_TEAM_NEUT7, true);
          else if(label == "NEUT_8")
            set_team(_TEAM_NEUT8, true);
          else if(label == "HERO")
            set_team(_TEAM_HERO, true);
          else if(label == "HERO_ALLY")
            set_team(_TEAM_HERO_ALLY, true);
          else if(label == "BOSS")
            set_team(_TEAM_EVIL_BOSS, true);
          else if(label == "SUB_BOSS")
            set_team(_TEAM_EVIL_SUB_BOSS, true);
          else
            error("Unknown team '%s' for entity '%s'", label.c_str(), my_entity->get_name().c_str());
        }
        else if(label == "team_id")
        {
          unsigned int id;
          serial_in(fs, &id);
          set_team_id(id);
        }
        else if(label == "enemy:")
        {
          set_enemy_team(_TEAM_ALIGNMENT_MASK, false);

          for ( serial_in(fs,&label); label!=chunkend_label; serial_in(fs,&label) )
          {
            if(label == "EVIL")
              set_enemy_team(_TEAM_EVIL, true);
            else if(label == "GOOD")
              set_enemy_team(_TEAM_GOOD, true);
            else if(label == "NEUT")
              set_enemy_team(_TEAM_NEUT, true);
            else if(label == "EVIL_1")
              set_enemy_team(_TEAM_EVIL1, true);
            else if(label == "EVIL_2")
              set_enemy_team(_TEAM_EVIL2, true);
            else if(label == "EVIL_3")
              set_enemy_team(_TEAM_EVIL3, true);
            else if(label == "EVIL_4")
              set_enemy_team(_TEAM_EVIL4, true);
            else if(label == "EVIL_5")
              set_enemy_team(_TEAM_EVIL5, true);
            else if(label == "EVIL_6")
              set_enemy_team(_TEAM_EVIL6, true);
            else if(label == "EVIL_7")
              set_enemy_team(_TEAM_EVIL7, true);
            else if(label == "EVIL_8")
              set_enemy_team(_TEAM_EVIL8, true);
            else if(label == "GOOD_1")
              set_enemy_team(_TEAM_GOOD1, true);
            else if(label == "GOOD_2")
              set_enemy_team(_TEAM_GOOD2, true);
            else if(label == "GOOD_3")
              set_enemy_team(_TEAM_GOOD3, true);
            else if(label == "GOOD_4")
              set_enemy_team(_TEAM_GOOD4, true);
            else if(label == "GOOD_5")
              set_enemy_team(_TEAM_GOOD5, true);
            else if(label == "GOOD_6")
              set_enemy_team(_TEAM_GOOD6, true);
            else if(label == "GOOD_7")
              set_enemy_team(_TEAM_GOOD7, true);
            else if(label == "GOOD_8")
              set_enemy_team(_TEAM_GOOD8, true);
            else if(label == "NEUT_1")
              set_enemy_team(_TEAM_NEUT1, true);
            else if(label == "NEUT_2")
              set_enemy_team(_TEAM_NEUT2, true);
            else if(label == "NEUT_3")
              set_enemy_team(_TEAM_NEUT3, true);
            else if(label == "NEUT_4")
              set_enemy_team(_TEAM_NEUT4, true);
            else if(label == "NEUT_5")
              set_enemy_team(_TEAM_NEUT5, true);
            else if(label == "NEUT_6")
              set_enemy_team(_TEAM_NEUT6, true);
            else if(label == "NEUT_7")
              set_enemy_team(_TEAM_NEUT7, true);
            else if(label == "NEUT_8")
              set_enemy_team(_TEAM_NEUT8, true);
            else if(label == "HERO")
              set_enemy_team(_TEAM_HERO, true);
            else if(label == "HERO_ALLY")
              set_enemy_team(_TEAM_HERO_ALLY, true);
            else if(label == "BOSS")
              set_enemy_team(_TEAM_EVIL_BOSS, true);
            else if(label == "SUB_BOSS")
              set_enemy_team(_TEAM_EVIL_SUB_BOSS, true);
            else
              error("Unknown team '%s' for entity '%s'", label.c_str(), my_entity->get_name().c_str());
          }
        }
      }
    }
    else if(label == "weapons:")
    {
      for ( serial_in(fs,&label); label!=chunkend_label; serial_in(fs,&label) )
      {
        if(label == "weapon:")
        {
          serial_in(fs,&label);

          entity* itm;
          stringx entity_name = get_fname_wo_ext( label );
          entity_name.to_upper();
          stringx entity_dir = get_dir( label );
          itm = g_entity_maker->create_entity_or_subclass( entity_name,
                                                        entity_id::make_unique_id(),
                                                        po_identity_matrix,
                                                        entity_dir,
                                                        ACTIVE_FLAG | NONSTATIC_FLAG );
          if ( itm->get_flavor() != ENTITY_ITEM )
          {
            error( "AI weapon: entity " + entity_name + " is not an item" );
          }
          else
          {
            if(!itm->is_a_handheld_item())
            {
              error( "AI weapon: entity " + entity_name + " is not an handheld item" );
            }
            else
            {
              handheld_item* hitm = ((handheld_item*)itm);

              itm->set_created_entity_default_active_status();
              hitm->set_handheld_flag( _HANDHELD_BRAIN_WEAPON, true );
              hitm->set_handheld_flag( _HANDHELD_COMMON_BRAIN_WEAPON, true );

              my_entity->add_item( (item*)itm );
              if(itm->is_a_gun())
                g_world_ptr->guarantee_active( itm );

              hitm->draw();

              for ( serial_in(fs,&label); label!=chunkend_label; serial_in(fs,&label) )
              {
                label.to_upper();
                if(label == "DRAWN")
                  hitm->draw();
                else if(label == "HOLSTERED")
                  hitm->holster();
                else if(label == "ORIENTATION:")
                  hitm->read_orientation_chunk(fs);
                else if(label == "AI_INFO:")
                  hitm->read_ai_info_chunk(fs);
                else if(label == "ID")
                {
                  serial_in(fs,&label);
                  hitm->set_item_id(label);
                }
              }

              // reset positions
              if(!hitm->is_drawn())
              {
                hitm->draw();
                hitm->holster();
              }
              else if(hitm->is_drawn())
              {
                hitm->holster();
                hitm->draw();
              }
            }
          }
        }
      }
    }
#endif // BIGCULL
/*
    else if(label == "eyes:")
    {
      eyes->read_data(fs);
    }
    else if(label == "ears:")
    {
      ears->read_data(fs);
    }
*/
    else if(label == "cosmetic")
    {
      set_cosmetic(true);
    }
    else if(label == "detector")
    {
      set_detector(true);
    }
#if 0 // BIGCULL
    else if(label == "adv_dropshadow")
    {
      set_flag(_AI_ADV_SHADOW, true);
    }
    else if(label == "dropshadow")
    {
      serial_in(fs, &label);
      label.to_upper();

      rational_t scale;
      serial_in(fs, &scale);

      if(my_entity->is_a_conglomerate())
      {
        entity *ent = (label == "SELF") ? my_entity : ((conglomerate *)my_entity)->get_member(label);
        if(ent)
        {
          entity *shadow = g_entity_maker->create_entity_or_subclass("items\\entities\\dropshadow", entity_id::make_unique_id(), po_identity_matrix, empty_string);

          if(shadow)
            dropshadows.push_back(ai_dropshadow(ent, shadow, scale));
        }
      }

      set_flag(_AI_ADV_SHADOW, false);
    }
#endif // BIGCULL
    else if(label == "patrol_id")
    {
      int id;
      serial_in(fs, &id);

      if(locomotion)
        locomotion->patrol_id = id;
    }
  }
}

void ai_interface::frame_advance_ai_interfaces(time_value_t t)
{
  rational_t tx;
  list<ai_interface *>::iterator ai = all_ai_interfaces.begin();
  list<ai_interface *>::iterator ai_end = all_ai_interfaces.end();
  while(ai != ai_end)
  {
    if((*ai)->is_active())
    {
      tx = CALC_ENTITY_TIME_DILATION(t, (*ai)->my_entity);
      (*ai)->frame_advance(tx);
    }
    ++ai;
  }
}

#include "polytube.h"

void ai_interface::render_ai(char level)
{
  list<ai_interface *>::iterator ai = all_ai_interfaces.begin();
  list<ai_interface *>::iterator ai_end = all_ai_interfaces.end();
  while(ai != ai_end)
  {
    (*ai)->render(level);
    ++ai;
  }

/*
/////////////////////////////////////
#if 1
  vector3d pts[5] = { -YVEC,
                      YVEC*2,
                      YVEC*2+XVEC*2,
                      YVEC*2+XVEC*2-ZVEC*2,
                      YVEC*2+XVEC*2-ZVEC*2+YVEC*3+XVEC*3-ZVEC*3  };
  int num_points = 5;
#else
  vector3d pts[9] = {YVEC,
                      YVEC*2+XVEC,
                      YVEC*3+XVEC*3,
                      YVEC*4+XVEC*5,
                      YVEC*5+XVEC*8,
                      YVEC*6+XVEC*12,
                      YVEC*7+XVEC*17,
                      YVEC*8+XVEC*23,
                      YVEC*9+XVEC*31,
    };
  int num_points = 9;
#endif

  static rational_t len = 0.0f;
  static rational_t modlen = 3.0f;
  static bool type = true;
  len += g_world_ptr->get_cur_time_inc() * modlen;

  if(len > 15.0f || len <= 0.0f)
  {
    if(len <= 0.0f)
    {
      len = 0.0f;
      type = !type;
    }

    modlen = -modlen;
  }



  b_spline spline;
  spline.reserve_control_pts(num_points+2);

  vector3d pt;
  pt = (pts[0] - pts[1]) + pts[0];
  spline.add_control_pt(pt);

  for(int p=0; p<num_points; ++p)
    spline.add_control_pt(pts[p]);

  pt = (pts[num_points-1] - pts[num_points-2]) + pts[num_points-1];
  spline.add_control_pt(pt);

  spline.build(10, type ? b_spline::_CATMULL_ROM : b_spline::_B_SPLINE);

  hw_rasta::inst()->set_zbuffering(true, true);

  render_polytube(spline.get_curve_pts(), 0.025f, 5, color32_white, g_spiderman_controller_ptr->get_webline_texture(), 1.0f, identity_matrix, len);
  if(level > 1)
    render_polytube(spline.get_control_pts(), 0.025f, 5, color32_blue);

  hw_rasta::inst()->set_zbuffering(true, false);
/////////////////////////////////////
//*/
}

void ai_interface::push_disable()
{
#ifdef BUILD_DEBUG
  stringx ent_name = my_entity->get_name();
#endif

  ++disable_count;

  assert(disable_count > 0);

  goto_position(my_entity->get_abs_position(), 25.0f, true, false, true);
}

void ai_interface::pop_disable()
{
#ifdef BUILD_DEBUG
  stringx ent_name = my_entity->get_name();
#endif

  --disable_count;

  assert(disable_count >= 0);

  if(disable_count < 0)
    disable_count = 0;
}

void ai_interface::push_disable_all(bool disable_cosmetic)
{
  list<ai_interface *>::iterator ai = all_ai_interfaces.begin();
  list<ai_interface *>::iterator ai_end = all_ai_interfaces.end();
  while(ai != ai_end)
  {
    if(disable_cosmetic || !(*ai)->cosmetic())
      (*ai)->push_disable();

    ++ai;
  }
}

void ai_interface::pop_disable_all(bool disable_cosmetic)
{
  list<ai_interface *>::iterator ai = all_ai_interfaces.begin();
  list<ai_interface *>::iterator ai_end = all_ai_interfaces.end();
  while(ai != ai_end)
  {
    if(disable_cosmetic || !(*ai)->cosmetic())
      (*ai)->pop_disable();

    ++ai;
  }
}

#if 0 //BIGCULL

void ai_interface::lock_threat()
{
  set_flag(_AI_THREAT_LOCK, (get_threat() != NULL));
}

void ai_interface::unlock_threat()
{
  set_flag(_AI_THREAT_LOCK, false);
}

bool ai_interface::is_enemy(ai_interface *ai) const
{
  assert(ai);

  unsigned int my_team_id = get_team_id();
  unsigned int ai_team_id = ai->get_team_id();

  if(my_team_id != 0 && ai_team_id != 0 && my_team_id == ai_team_id)
    return(false);
  else
    return(is_enemy_team(ai->team));
}

bool ai_interface::is_ally(ai_interface *ai) const
{
  assert(ai);

  unsigned int my_team_id = get_team_id();
  unsigned int ai_team_id = ai->get_team_id();

  return((my_team_id != 0 && ai_team_id != 0 && my_team_id == ai_team_id) || is_ally_team(ai->team));
}





void ai_interface::threat_assesment(time_value_t t)
{
  threats.reserve(all_ai_interfaces.size());
  threats.resize(0);

  if(threat_assesment_freq >= 0.0f)
  {
    threat_timer -= t;

    if(threat_timer <= 0.0f)
    {
      threat_timer = threat_assesment_freq;

      ai_threat old_threat = threat;

      threat.clear(this);
//      rational_t dist = 0.0f;

      if(desired_threat)
      {
        assert(desired_threat->is_alive());

        rational_t d = (my_entity->get_abs_position() - desired_threat->get_abs_position()).length2();
        if(d < 1.0f)
          d = 1.0f;

        threat = ai_threat(desired_threat, desired_threat->has_ai_ifc() ? desired_threat->ai_ifc() : NULL, d, 255.0f, 255);
        if(threat.ai)
          threat.ai->add_attacker(this);

        if(threat.ent != old_threat.ent)
          threat_sticky_timer = 4.0f + PLUS_MINUS_ONE;
      }
      else
      {
        list<ai_interface *>::iterator ai = all_ai_interfaces.begin();
        list<ai_interface *>::iterator ai_end = all_ai_interfaces.end();
        while(ai != ai_end)
        {
          ai_interface *possible_threat = (*ai);
          ++ai;

          rational_t rad = 0.0f;

          bool ignore_sight = (old_threat.ai == possible_threat) && threat_locked();

          if(old_threat.ai == possible_threat)
            rad = (eyes->get_max_dist() * 0.5f);
          else if(threat_sticky_timer <= 0.0f && wounded() && dmg_info.attacker == possible_threat->get_my_entity())
            rad = (eyes->get_max_dist() * 0.25f);

          if(possible_threat != this && possible_threat->get_my_entity() != NULL && possible_threat->get_my_entity()->is_alive() && possible_threat->get_my_entity()->is_visible() && possible_threat->is_active() && is_enemy(possible_threat) && (ignore_sight || eyes->check_ent(possible_threat->get_my_entity(), rad)))
          {
            unsigned int threat_level_chk = threat_level(possible_threat);

            if(threat_level_chk > 0)
            {
              rational_t d = (my_entity->get_abs_position() - possible_threat->get_my_entity()->get_abs_position()).length2();
              if(d < 1.0f)
                d = 1.0f;

              rational_t power = threat_level_chk;//((((rational_t)threat_level_chk * 50.0f))/d) * (old_threat == possible_threat->get_my_entity() ? 3.0f : 1.0f);
              if(old_threat.ai == possible_threat)
                power *= 2.0f;

              int num_att = possible_threat->get_num_attackers();
  //            if(num_att > 1)
  //              power *= (1.0f - ((rational_t)(num_att - 1) / 5.0f));
              if(num_att > 0)
                power /= (1<<num_att);

              if(wounded() && threat_sticky_timer <= 0.0f && dmg_info.attacker == possible_threat->get_my_entity())
              {
                if(dmg_info.attacker_itm != NULL)
                  power *= 6.0f;
                else
                  power *= 3.0f;
              }

              threats.push_back(ai_threat(possible_threat->get_my_entity(), possible_threat, d, power, threat_level_chk));
            }
          }
        }

        if(!threats.empty())
        {
          if(threats.size() > 1)
            qsort( &(*threats.begin()), threats.size(), sizeof(ai_threat), ai_threat::compare );

          threat = *threats.begin();
          assert(threat.ai != NULL);
          threat.ai->add_attacker(this);

          if(threat.ai != old_threat.ai)
            threat_sticky_timer = 4.0f + PLUS_MINUS_ONE;
        }
      }
    }
  }
}

unsigned int ai_interface::threat_level(ai_interface *ai) const
{
  unsigned int match_team = ((enemy_team & ai->team) & _TEAM_ALIGNMENT_MASK);
  return(get_good_level(match_team) + get_neut_level(match_team) + get_evil_level(match_team));
}

void ai_interface::add_attacker(ai_interface *ai)
{
  vector<ai_interface *>::iterator ait = attackers.begin();
  while(ait != attackers.end())
  {
    if((*ait) == ai)
      return;
    else
      ++ait;
  }

  attackers.push_back(ai);
}

void ai_interface::remove_attacker(ai_interface *ai)
{
  vector<ai_interface *>::iterator ait = attackers.begin();
  while(ait != attackers.end())
  {
    if((*ait) == ai)
    {
      attackers.erase(ait);
      return;
    }
    else
      ++ait;
  }
}


void ai_interface::calculate_fear(time_value_t t)
{
  if(external_fear > 0.0f)
    external_fear -= t*fear_decay;
  if(external_fear < 0.0f)
    external_fear = 0.0f;

  if(internal_fear > 0.0f)
    internal_fear -= t*fear_decay;
  if(internal_fear < 0.0f)
    internal_fear = 0.0f;

  internal_fear_timer -= t;
  if(internal_fear_timer <= 0.0f)
  {
    internal_fear_timer = 0.25f;

    internal_fear = 0.0f;

    if(fear_mod > 0.0f)
    {
      internal_fear += (get_num_attackers() * _AI_FEAR_ATTACKER * fear_mod);

      if(threat.ai)
      {
        internal_fear -= (threat.ai->get_num_attackers() * _AI_FEAR_ALLY_ATTACKER * (1.0f / fear_mod));

        rational_t diff = (rational_t)(threat.ai->get_power_level() - get_power_level()) / (255.0f*3.0f);
        if(diff > 0)
          internal_fear += (_AI_FEAR_THREAT_POWER_DIFF * diff * fear_mod);
        else
          internal_fear += (_AI_FEAR_THREAT_POWER_DIFF * diff * (1.0f / fear_mod));
      }
    }
  }


  if(wounded())
  {
    rational_t mod = 1.0f;

    // damage type affects fear
    switch(dmg_info.type)
    {
      case DAMAGE_GUN:
        mod *= _AI_FEAR_DAMAGE_GUN_MOD;
        break;

      case DAMAGE_MELEE:
        mod *= _AI_FEAR_DAMAGE_MELEE_MOD;
        break;

      case DAMAGE_EXPLOSIVE:
        mod *= _AI_FEAR_DAMAGE_EXPLODE_MOD;
        break;

      default:
        break;
    }

    // Fear is greater if hit from behind
    dmg_info.dir.normalize();
    rational_t dir = dot(dmg_info.dir, my_entity->get_abs_po().get_facing()) - 1.0f;
    mod *= (1.0f + ((dir*dir)*_AI_FEAR_DAMAGE_DIR_MOD));

    inc_fear(_AI_FEAR_DAMAGE * mod);
  }

  if(internal_fear < -external_fear)
    internal_fear = -external_fear;

  assert(get_fear() >= 0.0f);
}

#endif //BIGCULL


eLocomotionType ai_interface::get_locomotion_type() const
{
  return(locomotion->type);
}

bool ai_interface::goto_position(const vector3d &pos, rational_t rad, bool fast, bool path_find, bool force_finish)
{
  set_reached_dest((my_entity->get_abs_position() - pos).length2() <= (rad*rad));

  return(locomotion == NULL ? true : locomotion->set_destination(pos, rad, fast, path_find, force_finish));
}




void ai_interface::frame_advance(time_value_t t)
{
  START_PROF_TIMER(proftimer_adv_AI_locomotion);
  set_reached_dest(locomotion == NULL ? true : locomotion->frame_advance(t));
  STOP_PROF_TIMER(proftimer_adv_AI_locomotion);

#if 0 //BIGCULL
  set_wounded(my_entity->has_damage_ifc() && my_entity->damage_ifc()->was_damaged());
  if(wounded())
    dmg_info = my_entity->damage_ifc()->get_last_dmg_info();

  START_PROF_TIMER(proftimer_adv_AI_aimer);
  if(!is_disabled() && my_entity->is_alive())
  {
    vector<ai_auto_aim *>::iterator aim = aimers.begin();
    vector<ai_auto_aim *>::iterator aim_end = aimers.end();
    while(aim != aim_end)
    {
      if((*aim)->id == head_aimer)
      {
        if(!is_head_disabled())
        {
          if(!is_head_locked())
          {
            (*aim)->set_active(!my_entity->playing_scene_anim() && threat.get_entity() != NULL);
            if((*aim)->is_active())
            {
              vector3d pos = calculate_eye_pos(threat.get_entity());
              (*aim)->set_target(pos);
            }
          }
          else
            (*aim)->set_active(true);
        }
        else
          (*aim)->set_active(false);
      }

      (*aim)->frame_advance(t);
      ++aim;
    }
  }
  STOP_PROF_TIMER(proftimer_adv_AI_aimer);

  if(!cosmetic() && !is_disabled() && my_entity->is_alive())
  {
    START_PROF_TIMER(proftimer_adv_AI_senses);
    set_see_something(eyes->frame_advance_detection(t));
    set_hear_something(ears->frame_advance_detection(t));
    STOP_PROF_TIMER(proftimer_adv_AI_senses);

    START_PROF_TIMER(proftimer_adv_AI_threat);
    threat_sticky_timer -= t;
    if(threat_sticky_timer < 0.0f)
      threat_sticky_timer = 0.0f;

    entity *old_desired_threat = desired_threat;
    if(threat.get_entity() && !threat.get_entity()->is_alive())
    {
      if(desired_threat == threat.get_entity())
        desired_threat = NULL;

      threat.clear(this);
      threat_sticky_timer = 0.0f;

      if(threat_locked())
        unlock_threat();

      dec_fear(_AI_FEAR_THREAT_KILLED);
    }
    else if(desired_threat && !desired_threat->is_alive())
      desired_threat = NULL;

    ai_threat old_threat = threat;
    if((wounded() && threat_sticky_timer <= 0.0f) || (!old_threat.get_entity() && see_something()) || ((desired_threat && old_threat.get_entity() != desired_threat) || (!desired_threat && old_desired_threat != NULL && old_threat.get_entity() == old_desired_threat)))
      threat_assesment();
    else
      threat_assesment(t);

    set_new_threat(threat.get_entity() != NULL && old_threat.get_entity() == NULL);
    set_lost_threat(threat.get_entity() == NULL && old_threat.get_entity() != NULL && old_threat.get_entity()->is_alive());
    set_change_threat(threat.get_entity() != NULL && old_threat.get_entity() != NULL && old_threat.get_entity() != threat.get_entity());

    if(threat.get_entity() != NULL)
    {
      set_threat_in_los(eyes->check_ent(threat.get_entity()));

      if(!threat_locked() || threat_in_los() || new_threat() || change_threat())
        last_threat_pos = threat.get_entity()->get_abs_position();
    }
    else
      set_threat_in_los(false);

    STOP_PROF_TIMER(proftimer_adv_AI_threat);

    START_PROF_TIMER(proftimer_adv_AI_radio);
    help_timer -= t;
    if(help_timer < 0.0f)
      help_timer = 0.0f;

    if(new_threat() || change_threat())
    {
      assert(threat.get_entity());

      ai_radio_message msg(this, _RADIO_ENEMY_SIGHTED, threat.power);
      msg.ent = threat.get_entity();
      msg.pos = msg.ent->get_abs_position();

      broadcast_message(_CHANNEL_ALLY, msg);
    }
    else if(threat.get_entity() != NULL && help_timer <= 0.0f)
    {
      ai_radio_message msg(this, _RADIO_HELP, threat.power);
      msg.ent = threat.get_entity();
      msg.pos = msg.ent->get_abs_position();

      broadcast_message(_CHANNEL_ALLY, msg);
      help_timer = 0.5f;
    }
    STOP_PROF_TIMER(proftimer_adv_AI_radio);

    if(wounded() && threat.get_entity() == NULL && locomotion != NULL)
    {
      if(dmg_info.type == DAMAGE_MELEE || dmg_info.type == DAMAGE_DIRECT_DIRECTIONAL || dmg_info.type == DAMAGE_GUN || dmg_info.type == DAMAGE_NONBLOCK_MELEE)
        locomotion->set_facing(-dmg_info.dir);
    }

    START_PROF_TIMER(proftimer_adv_AI_radio);
    if(radio && radio->new_message())
    {
      switch(radio->last_message.msg)
      {
        case _RADIO_HELP:
        case _RADIO_ENEMY_SIGHTED:
        {
          if(threat.get_entity() == NULL && locomotion != NULL)
            locomotion->set_facing_point(radio->last_message.pos);
        }
        break;

        case _RADIO_GOTO:
        {
          goto_position(radio->last_message.pos, 2.0f, true, true, true);
        }
        break;

        case _RADIO_ATTACK_TARGET:
        {
          desired_threat = radio->last_message.ent;
        }
        break;

        case _RADIO_THREATEN:
        {
          external_fear += _AI_FEAR_THREATEN;
        }
        break;

        default:
          break;
      }
    }
    STOP_PROF_TIMER(proftimer_adv_AI_radio);

    START_PROF_TIMER(proftimer_adv_AI_fear);
    calculate_fear(t);
    STOP_PROF_TIMER(proftimer_adv_AI_fear);

    if(new_threat())
      voice->say(_VOICE_NEW_THREAT);
    else if(lost_threat())
      voice->say(_VOICE_LOST_THREAT);
    else if(investigated_cue())
    {
      current_cue.power = 0.0f;

      if(current_cue.sound != empty_string)
      {
        if(current_cue.sound_group())
        {
          pstring pstr(current_cue.sound.c_str());
          voice->say(pstr);
        }
        else
          voice->say_file(current_cue.sound);
      }
      else
      {
        voice->say(_VOICE_INVESTIGATE);
      }
    }
  }
  else
  {
    threat.clear(this);

    set_see_something(false);
    set_hear_something(false);
    set_new_threat(false);
    set_lost_threat(false);
    set_change_threat(false);
  }

  current_cue.frame_advance(t);

  set_new_cue(false);
  if(see_something())
  {
    current_cue = eyes->current_cue;
    current_cue.decay = 25.0f;
    set_new_cue(true);
  }
  if(hear_something() && ears->current_cue.power > current_cue.power )
  {
    current_cue = ears->current_cue;
    current_cue.decay = 25.0f;
    set_new_cue(true);
  }

  vector<ai_goal *>::iterator g = goals.begin();
  vector<ai_goal *>::iterator g_end = goals.end();
  while(g != g_end)
  {
    (*g)->calculate_priority(t);
    ++g;
  }

  set_investigated_cue(false);
#endif //BIGCULL

  START_PROF_TIMER(proftimer_adv_AI_priority);
  if(!goals.empty())
  {
    if(goals.size() > 1)
      qsort( &(*goals.begin()), goals.size(), sizeof(ai_goal *), ai_goal::compare );

    ai_goal *goal = *goals.begin();

    // BIGCULL  PTA 5/16/01
    if(!current_goal || (goal->get_priority() > 0.0f && (current_goal != *goals.begin() && (!current_goal || goal->get_priority() > current_goal->get_priority()))))
    {
      if(current_goal)
        current_goal->going_out_of_service();

      current_goal = *goals.begin();
      current_goal->going_into_service();
    }
  }
  STOP_PROF_TIMER(proftimer_adv_AI_priority);

  START_PROF_TIMER(proftimer_adv_AI_goal);
  if(current_goal)
    current_goal->frame_advance(t);
  STOP_PROF_TIMER(proftimer_adv_AI_goal);

//BIGCULL
  /*
  START_PROF_TIMER(proftimer_adv_AI_radio);
  radio->frame_advance(t);
  STOP_PROF_TIMER(proftimer_adv_AI_radio);
  */

#ifdef GCCULL
  voice->frame_advance(t);
#endif

#if 0 //BIGCULL
  if(!dropshadows.empty())
  {
    color32 color = my_entity->get_render_color();

    if(!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_DISABLE_SHADOWS))
    {
      my_entity->update_abs_po();

      bool col_active = my_entity->are_collisions_active();
      my_entity->set_collisions_active(false, false);

      vector<ai_dropshadow>::iterator di = dropshadows.begin();
      vector<ai_dropshadow>::iterator di_end = dropshadows.end();
      while(di != di_end)
      {
        (*di).update(my_entity->is_visible(), color);
        ++di;
      }

      my_entity->set_collisions_active(col_active, false);
    }
    else
    {
      vector<ai_dropshadow>::iterator di = dropshadows.begin();
      vector<ai_dropshadow>::iterator di_end = dropshadows.end();
      while(di != di_end)
      {
        (*di).update(false, color);
        ++di;
      }
    }
  }
#endif //BIGCULL
}


path_graph *ai_interface::get_current_path_graph() const
{
  return(locomotion == NULL ? NULL : locomotion->get_current_path_graph());
}

void ai_interface::set_current_path_graph(path_graph *g)
{
  if(locomotion)
    locomotion->set_current_path_graph(g);
}

rational_t ai_interface::get_xz_rotation_to_point( const vector3d &target_pos ) const
{
  if(locomotion == NULL)
  {
    vector3d face = get_my_entity()->get_abs_po().get_facing();
    vector3d invvec = target_pos - get_my_entity()->get_abs_position();

    rational_t diff = safe_atan2( -invvec.x, invvec.z ) - safe_atan2( -face.x, face.z );

    if( diff > PI )
      diff -= _2PI;
    if( diff < -PI )
      diff += _2PI;

    return( diff );
  }
  else
    return(locomotion->get_xz_rotation_to_point(target_pos));
}

bool ai_interface::xz_facing_point( const vector3d &target_pos, rational_t rads ) const
{
  if(locomotion == NULL)
    return( __fabs( get_xz_rotation_to_point(target_pos) ) <= (rads*0.5f) );
  else
    return(locomotion->xz_facing_point(target_pos, rads));
}

void ai_interface::apply_rotation( rational_t rot )
{
  if(locomotion)
    locomotion->apply_rotation(rot);
  else
  {
    static rational_t sine;
    static rational_t cosine;
    static rational_t tmp;

    fast_sin_cos_approx( rot, &sine, &cosine );
    matrix4x4 &m = (matrix4x4 &)get_my_entity()->get_rel_po().get_matrix();

    tmp = m.x.x;
    m.x.x = tmp*cosine + m.x.z*(-sine);
    m.x.z = tmp*sine + m.x.z*cosine;

    tmp = m.y.x;
    m.y.x = tmp*cosine + m.y.z*(-sine);
    m.y.z = tmp*sine + m.y.z*cosine;

    tmp = m.z.x;
    m.z.x = tmp*cosine + m.z.z*(-sine);
    m.z.z = tmp*sine + m.z.z*cosine;
  }
}

bool ai_interface::get_next_patrol_point(const vector3d &last_pos, const vector3d &cur_pos, vector3d &next)
{
  return(locomotion == NULL ? false : locomotion->get_next_patrol_point(last_pos, cur_pos, next));
}

bool ai_interface::get_nearest_patrol_point(const vector3d &cur_pos, vector3d &patrol_pt)
{
  return(locomotion == NULL ? false : locomotion->get_nearest_patrol_point(cur_pos, patrol_pt));
}

rational_t ai_interface::get_patrol_radius() const
{
  return(locomotion == NULL ? 0.0f : locomotion->get_patrol_radius());
}

void ai_interface::set_patrol_radius(rational_t r)
{
  if(locomotion)
    locomotion->set_patrol_radius(r);
}

void ai_interface::stop_movement(bool forced)
{
  if(locomotion)
    locomotion->set_destination(my_entity->get_abs_position(), 100.0f, locomotion->running_speed, false, forced);
}

void ai_interface::jockey_to(const vector3d &pt)
{
  if(locomotion)
    locomotion->jockey_to(pt);
}

void ai_interface::jockey_dir_time(const vector3d &dir, time_value_t t)
{
  if(locomotion)
    locomotion->jockey_dir_time(dir, t);
}

void ai_interface::stop_jockey()
{
  if(locomotion)
    locomotion->stop_jockey();
}

bool ai_interface::is_jockeying() const
{
  return(locomotion != NULL ? locomotion->is_jockeying() : false);
}


bool ai_interface::using_animation_for_locomotion() const
{
  return(locomotion ? locomotion->using_animation() : false);
}

bool ai_interface::face_dir(const vector3d &dir, rational_t mod)
{
  return(locomotion ? locomotion->set_facing(dir, mod) : false);
}

bool ai_interface::face_point(const vector3d &dir, rational_t mod)
{
  return(locomotion ? locomotion->set_facing_point(dir, mod) : false);
}


bool ai_interface::play_animation(const stringx &anim_id, int slot, unsigned short anim_flags, int *damage_value, rational_t *recover, rational_t *recover_var, int *flags)
{
  if(my_entity->has_animation_ifc())
  {
    int my_flags;
    const stringx &res = my_entity->animation_ifc()->extract_random_anim_info_id_map_anim(anim_id, damage_value, recover, recover_var, &my_flags);
    if(flags)
      *flags = my_flags;


    if( res != empty_string )
    {
      if(my_flags & animation_interface::ANIM_IFC_REVERSE)
        anim_flags |= ANIM_REVERSE;

      get_my_entity()->play_anim( slot, res, 0.0f, anim_flags);
      return(true);
    }
    else
    {
      get_my_entity()->kill_anim( slot );
      return(false);
    }
  }

  return(false);
}



void ai_interface::render(char level)
{
#if 0 // BIGCULL
//  if(my_entity->is_hero() || !my_entity->is_alive())
//    return;

  stringx locomotion_str = "No Locomotion";
  if(locomotion)
  {
    if(!reached_dest())
    {
      locomotion->current_path.render(color32_blue);
      locomotion_str = stringx(stringx::fmt, "<%.2f, %.2f, %.2f> %s", locomotion->target_pos.x, locomotion->target_pos.y, locomotion->target_pos.z, locomotion->running_speed ? "fast" : "slow");
    }
    else
      locomotion_str = "Reached dest";
  }

  stringx goal_str = "No Goals";
  if(!goals.empty())
  {
    goal_str = stringx(stringx::fmt, "%s (%.2f)", (*goals.begin())->type.c_str(), (*goals.begin())->get_priority());

    if(level >= 2)
    {
      vector<ai_goal *>::iterator g = goals.begin();

      // skip first one
      ++g;

      while(g != goals.end())
      {
        if((*g)->get_priority() > 0.0f)
        {
          goal_str += "\n";
          goal_str += stringx(stringx::fmt, "%s (%.2f)", (*g)->type.c_str(), (*g)->get_priority());
        }

        ++g;
      }
    }
  }

  stringx misc_str = empty_string;

  if(!cosmetic())
    misc_str += stringx(stringx::fmt, "\nFear %.2f (%.2f : %.2f)", get_fear(), internal_fear, external_fear);

  if(!cosmetic() && level >= 2 && !my_entity->is_hero())
  {
    color32 col;
//    col = color32_blue;
//    col.set_alpha(96);
//    render_sphere(my_entity->get_abs_position(), ears->get_max_dist(), col);

    col = color32_green;
    col.set_alpha(96);
// BIGCULL     render_sphere(my_entity->get_abs_position(), eyes->get_min_dist(), col);

    col = color32_green;
    col.set_alpha(128);

    po my_po;

    eyes->node->update_abs_po_reverse();
    vector3d facing = eyes->node->get_abs_po().get_facing();
    if(dot(facing, get_my_entity()->get_abs_po().get_facing()) < 0.0f)
      facing = -facing;

//    facing.y = 0.0f;
    facing.normalize();

    my_po = po_identity_matrix;
    my_po.set_rotate_y(-eyes->get_detection_arc()*0.5f);
    vector3d last_face2 = my_po.non_affine_slow_xform(facing);
    vector3d tmp = eyes->node->get_abs_position()+last_face2*eyes->get_max_dist();

    const int sub = 6;
    for(int i=-(sub-1); i<=sub; i++)
    {
      my_po = po_identity_matrix;
      my_po.set_rotate_y(eyes->get_detection_arc()*0.5f * (((rational_t) i)/((rational_t)sub)));
      vector3d face2 = my_po.non_affine_slow_xform(facing);

      vector3d pt3 = eyes->node->get_abs_position() + (face2*eyes->get_max_dist());


      if(level >= 3 && eyes->detection_arc_y < DEG_TO_RAD(90.0f))
      {
        vector3d crossp = cross(YVEC, last_face2);
        my_po = po_identity_matrix;
        my_po.set_rot(crossp, eyes->detection_arc_y*0.5f);
        vector3d a = eyes->node->get_abs_position()+(my_po.non_affine_slow_xform(last_face2)*eyes->get_max_dist());

        crossp = cross(YVEC, face2);
        my_po = po_identity_matrix;
        my_po.set_rot(crossp, eyes->detection_arc_y*0.5f);
        vector3d b = eyes->node->get_abs_position()+(my_po.non_affine_slow_xform(face2)*eyes->get_max_dist());

        render_triangle(eyes->node->get_abs_position(), a, b, col, true);


        crossp = cross(YVEC, last_face2);
        my_po = po_identity_matrix;
        my_po.set_rot(crossp, eyes->detection_arc_y*0.5f * -1);
        vector3d c = eyes->node->get_abs_position()+(my_po.non_affine_slow_xform(last_face2)*eyes->get_max_dist());

        crossp = cross(YVEC, face2);
        my_po = po_identity_matrix;
        my_po.set_rot(crossp, eyes->detection_arc_y*0.5f * -1);
        vector3d d = eyes->node->get_abs_position()+(my_po.non_affine_slow_xform(face2)*eyes->get_max_dist());

        render_triangle(eyes->node->get_abs_position(), c, d, col, true);

        if(i == -(sub-1))
        {
          render_triangle(eyes->node->get_abs_position(), a, c, col, true);
        }

        if(i == sub)
        {
          render_triangle(eyes->node->get_abs_position(), b, d, col, true);
        }
      }
//      else
      {
        vector3d a = eyes->node->get_abs_position()+(last_face2*eyes->get_max_dist());
        vector3d b = eyes->node->get_abs_position()+(face2*eyes->get_max_dist());

        render_triangle(eyes->node->get_abs_position(), a, b, col, true);

        if(i == 0)
        {
          my_po = po_identity_matrix;
          my_po.set_rot(get_my_entity()->get_abs_po().get_x_facing(), eyes->detection_arc_y*0.5f);
          vector3d c = eyes->node->get_abs_position()+(my_po.non_affine_slow_xform(face2)*eyes->get_max_dist());
          render_beam(eyes->node->get_abs_position(), c, col, 0.1f);

          my_po = po_identity_matrix;
          my_po.set_rot(get_my_entity()->get_abs_po().get_x_facing(), eyes->detection_arc_y*0.5f * -1);
          vector3d d = eyes->node->get_abs_position()+(my_po.non_affine_slow_xform(face2)*eyes->get_max_dist());
          render_beam(eyes->node->get_abs_position(), d, col, 0.1f);

          render_beam(c, b, col, 0.1f);
          render_beam(b, d, col, 0.1f);
        }
      }

      tmp = pt3;
      last_face2 = face2;
    }
  }


/*
  vector<ai_auto_aim *>::const_iterator aim = aimers.begin();
  vector<ai_auto_aim *>::const_iterator aim_end = aimers.end();
  while(aim != aim_end)
  {
    if((*aim)->id == head_aimer)
      (*aim)->render();

    ++aim;
  }
*/
  color32 render_col = color32_blue;
  if(my_entity->is_hero())
    render_col = color32_magenta;
  else if(cosmetic())
    render_col = color32_green;
  else if(!my_entity->is_alive())
    render_col = color32_white;

  if(locomotion && locomotion->use_path)
    locomotion->my_path.render();

  print_3d_text(my_entity->get_abs_position()+YVEC, render_col, "%s\n%s\n%s%s", my_entity->get_name().c_str(), goal_str.c_str(), locomotion_str.c_str(), misc_str.c_str());
#endif  // BIGCULL
}


ai_goal *ai_interface::get_goal(const pstring &goal_name) const
{
  vector<ai_goal *>::const_iterator g = goals.begin();
  vector<ai_goal *>::const_iterator g_end = goals.end();
  while(g != g_end)
  {
    if((*g)->type == goal_name)
      return(*g);
    else
      ++g;
  }

  return(NULL);
}

#if 0 // BIGCULL

ai_auto_aim *ai_interface::get_aimer(const pstring &aimer_name) const
{
  vector<ai_auto_aim *>::const_iterator aim = aimers.begin();
  vector<ai_auto_aim *>::const_iterator aim_end = aimers.end();
  while(aim != aim_end)
  {
    if((*aim)->id == aimer_name)
      return(*aim);
    else
      ++aim;
  }

  return(NULL);
}

void ai_interface::set_aimer_target(const pstring &aimer_name, const vector3d &target_pos)
{
  ai_auto_aim *aimer = get_aimer(aimer_name);
  if(aimer)
    aimer->set_target(target_pos);
}

void ai_interface::set_aimer_target(const vector3d &target_pos)
{
  vector<ai_auto_aim *>::const_iterator aim = aimers.begin();
  vector<ai_auto_aim *>::const_iterator aim_end = aimers.end();
  while(aim != aim_end)
  {
    (*aim)->set_target(target_pos);
    ++aim;
  }
}

vector3d ai_interface::get_aimer_target(const pstring &aimer_name) const
{
  vector3d target = ZEROVEC;

  ai_auto_aim *aimer = get_aimer(aimer_name);
  if(aimer)
    target = aimer->get_target();

  return(target);
}

void ai_interface::set_aimer_active(const pstring &aimer_name, bool active)
{
  ai_auto_aim *aimer = get_aimer(aimer_name);
  if(aimer)
    aimer->set_active(active);
}

void ai_interface::set_aimer_active(bool active)
{
  vector<ai_auto_aim *>::const_iterator aim = aimers.begin();
  vector<ai_auto_aim *>::const_iterator aim_end = aimers.end();
  while(aim != aim_end)
  {
    (*aim)->set_active(active);
    ++aim;
  }
}

bool ai_interface::get_aimer_active(const pstring &aimer_name) const
{
  ai_auto_aim *aimer = get_aimer(aimer_name);
  return(aimer != NULL ? aimer->is_active() : false);
}

void ai_interface::force_lookat(const vector3d &pos)
{
  ai_auto_aim *aimer = get_aimer(head_aimer);
  if(aimer)
  {
    aimer->set_active(!is_head_disabled());
    aimer->set_target(pos);
  }

  set_head_locked(true);
}

void ai_interface::force_lookat(entity *ent)
{
  if(ent != NULL)
  {
    vector3d pos = calculate_eye_pos(ent);
    force_lookat(pos);
  }
}

void ai_interface::release_lookat()
{
  set_head_locked(false);
}

#endif // BIGCULL


bool ai_interface::get_ifc_num(const pstring &att, rational_t &val)
{
  IFC_INTERNAL_FUNC_MACRO("AI_ACTIVE", is_active());
  IFC_INTERNAL_FUNC_MACRO("AI_COSMETIC", cosmetic());
#if 0 // BIGCULL
  IFC_INTERNAL_GET_MACRO("AI_EYE_ARC", eyes->detection_arc);
  IFC_INTERNAL_GET_MACRO("AI_EYE_ARC_Y", eyes->detection_arc_y);
  IFC_INTERNAL_GET_MACRO("AI_EYE_MIN", eyes->min_dist);
  IFC_INTERNAL_GET_MACRO("AI_EYE_MAX", eyes->max_dist);
  IFC_INTERNAL_GET_MACRO("AI_EAR_ARC", ears->detection_arc);
  IFC_INTERNAL_GET_MACRO("AI_EAR_MIN", ears->min_dist);
  IFC_INTERNAL_GET_MACRO("AI_EAR_MAX", ears->max_dist);
#endif // BIGCULL
  IFC_INTERNAL_GET_MACRO("AI_REACHED_DEST", reached_dest());

  return(false);
}

bool ai_interface::set_ifc_num(const pstring &att, rational_t val)
{
  IFC_INTERNAL_FUNC_MACRO("AI_ACTIVE", set_active(val != 0.0f));
  IFC_INTERNAL_FUNC_MACRO("AI_COSMETIC", set_cosmetic(val != 0.0f));
#if 0 // BIGCULL
  IFC_INTERNAL_SET_MACRO("AI_EYE_ARC", eyes->detection_arc);
  IFC_INTERNAL_SET_MACRO("AI_EYE_ARC_Y", eyes->detection_arc_y);
  IFC_INTERNAL_SET_MACRO("AI_EYE_MIN", eyes->min_dist);
  IFC_INTERNAL_SET_MACRO("AI_EYE_MAX", eyes->max_dist);
  IFC_INTERNAL_SET_MACRO("AI_EAR_ARC", ears->detection_arc);
  IFC_INTERNAL_SET_MACRO("AI_EAR_MIN", ears->min_dist);
  IFC_INTERNAL_SET_MACRO("AI_EAR_MAX", ears->max_dist);
#endif // BIGCULL

  return(false);
}

bool ai_interface::get_ifc_str(const pstring &att, stringx &val)
{
  return(false);
}

bool ai_interface::set_ifc_str(const pstring &att, const stringx &val)
{
  return(false);
}

bool ai_interface::set_goal_num(const pstring &goal_id, const pstring &att, rational_t val)
{
  ai_goal *g = get_goal(goal_id);

  if(g != NULL)
    return(g->set_num(att, val));
  else
    return(false);
}

bool ai_interface::get_goal_num(const pstring &goal_id, const pstring &att, rational_t &val)
{
  ai_goal *g = get_goal(goal_id);

  if(g != NULL)
    return(g->get_num(att, val));
  else
    return(false);
}

bool ai_interface::set_goal_str(const pstring &goal_id, const pstring &att, const stringx &val)
{
  ai_goal *g = get_goal(goal_id);

  if(g != NULL)
    return(g->set_str(att, val));
  else
    return(false);
}

bool ai_interface::get_goal_str(const pstring &goal_id, const pstring &att, stringx &val)
{
  ai_goal *g = get_goal(goal_id);

  if(g != NULL)
    return(g->get_str(att, val));
  else
    return(false);
}

bool ai_interface::set_loco_num(const pstring &att, rational_t val)
{
  return(locomotion != NULL ? locomotion->set_num(att, val) : false);
}

bool ai_interface::get_loco_num(const pstring &att, rational_t &val)
{
  return(locomotion != NULL ? locomotion->get_num(att, val) : false);
}

#if 0 // BIGCULL
class cover_pair
{
public:
  vector3d pt;
  rational_t dist;

  cover_pair(const vector3d &p, rational_t d)
  {
    pt = p;
    dist = d;
  }

  cover_pair(const cover_pair &cp)
  {
    pt = cp.pt;
    dist = cp.dist;
  }

  cover_pair& operator=(const cover_pair &b)
  {
    pt = b.pt;
    dist = b.dist;

    return *this;
  }

  static int compare( const void* x1, const void* x2 )
  {
    // lower comes first
    rational_t diff = ((cover_pair *)x1)->dist - ((cover_pair *)x2)->dist;

    if ( diff > 0.0f )
      return 1;

    if ( diff < 0.0f )
      return -1;

    return 0;
  }
};

bool ai_interface::find_cover_pt(const vector3d &from, vector3d &pos) const
{
  static vector<cover_pair> cover_pts;
  cover_pts.resize(0);

  static vector3d hit_pos, hit_norm, vec, inter2;

  if(my_entity->get_primary_region())
  {
    vector<entity*>::const_iterator i = my_entity->get_primary_region()->get_data()->get_possible_collide_entities().begin();
    vector<entity*>::const_iterator i_end = my_entity->get_primary_region()->get_data()->get_possible_collide_entities().end();

    for ( ; i!=i_end; ++i )
    {
      entity* e = *i;
      if ( e && e != my_entity && e->is_visible() && e->is_ai_cover() && e->is_beamable())
      {
        vec = e->get_abs_position() - from;
        vec.normalize();

        inter2 = e->get_abs_position() + (vec*100.0f);
        bool collide = find_intersection( e->get_abs_position(),
                                          inter2,
                                          e->get_primary_region(),
                                          (FI_COLLIDE_BEAMABLE | FI_COLLIDE_ENTITY_NO_REAR_CULL),
                                          &hit_pos,
                                          &hit_norm,
                                          NULL,
                                          NULL
                                        );

        if(collide)
        {
          assert(hit_norm.length2() > 0.8f);
          vector3d pt = hit_pos + hit_norm;

          if(in_world(pt, 0.1f, ZEROVEC, e->get_primary_region(), hit_pos))
            cover_pts.push_back(cover_pair(pt, (pt - my_entity->get_abs_position()).length2()));
        }
      }
    }
  }

  // entities have been exhausted, try the path
  if(cover_pts.empty() && get_current_path_graph() != NULL)
  {
    path_graph *path = get_current_path_graph();

    vector<path_graph_node *>::const_iterator i_end = path->get_nodes().end();
    for(vector<path_graph_node *>::const_iterator i = path->get_nodes().begin(); i!=i_end; ++i)
    {
      path_graph_node *node = (*i);

      if(node->get_my_region() != NULL)
      {
        bool collide = find_intersection( node->get_abs_position(),
                                          from,
                                          node->get_my_region(),
                                          (FI_COLLIDE_BEAMABLE | FI_COLLIDE_WORLD),
                                          &hit_pos,
                                          &hit_norm,
                                          NULL,
                                          NULL
                                        );

        if(collide && node->get_my_region() && in_world(node->get_abs_position(), 0.1f, ZEROVEC, node->get_my_region(), hit_pos))
          cover_pts.push_back(cover_pair(node->get_abs_position(), (node->get_abs_position() - my_entity->get_abs_position()).length2()));
      }
    }
  }

  if(!cover_pts.empty())
  {
    int size = cover_pts.size();
    if(size > 1)
      qsort( &(*cover_pts.begin()), size, sizeof(cover_pair), cover_pair::compare );

    if(size < 4)
      pos = cover_pts[(random(size))].pt;
    else
      pos = cover_pts[(random(4))].pt;

    return(true);
  }

  return(false);
}


bool ai_interface::find_ambush_pt(const vector3d &to, vector3d &pos) const
{
  static vector<cover_pair> ambush_pts;
  ambush_pts.resize(0);

  static vector3d hit_pos, hit_norm;

  if(get_current_path_graph() != NULL)
  {
    path_graph *path = get_current_path_graph();

    vector<path_graph_node *>::const_iterator i_end = path->get_nodes().end();
    for(vector<path_graph_node *>::const_iterator i = path->get_nodes().begin(); i!=i_end; ++i)
    {
      path_graph_node *node = (*i);

      if(node->get_my_region() != NULL)
      {
        bool collide = find_intersection( node->get_abs_position(),
                                          to,
                                          node->get_my_region(),
                                          (FI_COLLIDE_BEAMABLE | FI_COLLIDE_WORLD),
                                          &hit_pos,
                                          &hit_norm,
                                          NULL,
                                          NULL
                                        );

        if(!collide && node->get_my_region() && in_world(node->get_abs_position(), 0.1f, ZEROVEC, node->get_my_region(), hit_pos))
          ambush_pts.push_back(cover_pair(node->get_abs_position(), (node->get_abs_position() - my_entity->get_abs_position()).length2()));
      }
    }
  }

  if(!ambush_pts.empty())
  {
    int size = ambush_pts.size();
    if(size > 1)
      qsort( &(*ambush_pts.begin()), size, sizeof(cover_pair), cover_pair::compare );

    if(size < 3)
      pos = ambush_pts[(random(size))].pt;
    else
      pos = ambush_pts[(random(3))].pt;

    return(true);
  }

  return(false);
}


ai_interface *ai_interface::find_ai_by_team(unsigned int t, unsigned int flags, rational_t max_dist)
{
  vector<ai_interface *>::iterator ai = all_ai_interfaces.begin();
  vector<ai_interface *>::iterator ai_end = all_ai_interfaces.end();

  max_dist *= max_dist;

  rational_t min = FLT_MAX;
  rational_t max = -FLT_MAX;

  ai_interface *best = NULL;

  while(ai != ai_end)
  {
    ai_interface *ai_int = (*ai);
    ++ai;

    if(ai_int->is_active() && ai_int->get_my_entity()->is_visible() && ai_int->is_on_team(t))
    {
      rational_t dist2 = (get_my_entity()->get_abs_position() - ai_int->get_my_entity()->get_abs_position()).length2();

      bool accept = dist2 <= max_dist;

      if(accept && (flags & (_FIND_AI_NEAREST | _FIND_AI_FARTHEST)))
      {
        if(flags & _FIND_AI_NEAREST)
        {
          if(dist2 < min)
            min = dist2;
          else
            accept = false;
        }

        if(flags & _FIND_AI_FARTHEST)
        {
          if(dist2 > max)
            max = dist2;
          else
            accept = false;
        }
      }

      if(accept && (flags & (_FIND_AI_FOV | _FIND_AI_LOS)) && eyes != NULL)
        accept = eyes->check_ent(ai_int->get_my_entity(), (flags & _FIND_AI_LOS) ? 10000.0f : 0.0f);

      if(accept)
        best = ai_int;
    }
  }

  return(best);
}





bool ai_interface::compute_combat_target( const vector3d &target_pos, item *itm, rational_t accuracy )
{
  entity *hit_entity = NULL;

  my_entity->set_current_target( NULL );

  vector3d hitn, beam_n;

  vector3d vec;

  vector3d fire_pos = my_entity->get_abs_position();
  region_node *rgn = my_entity->get_primary_region();

  if(itm == NULL)
  {
    int num = my_entity->get_num_items();
    for(int i=0; i<num; i++)
    {
      // returns NULL if index is out-of=range
      itm = my_entity->get_item(i);
      if(itm->is_brain_weapon())
        break;
      else
        itm = NULL;
    }
  }

//  assert(itm && itm->is_a_gun());

  if(itm)
  {
    if(itm && itm->is_a_gun())
    {
      fire_pos = ((gun *)itm)->calculate_fire_pos();

      // This is necessary because the fire pos may not be in the same region as the owner (guys arm extends gun across a portal. Yes, it has happened...) (JDB)
      sector *sect = g_world_ptr->get_the_terrain().find_sector(fire_pos);
      if(sect && sect->get_region())
        rgn = sect->get_region();
      else
        return(false);
    }
  }

  vector3d aim_delta = target_pos - fire_pos;
  aim_delta.normalize();

  if(accuracy < 1.0f)
  {
    rational_t mod = 1.0f - accuracy;

    po the_po;

    the_po = po_identity_matrix;
    the_po.set_rot(my_entity->get_abs_po().get_x_facing(), ((DEG_TO_RAD(15.0f) * mod) * PLUS_MINUS_ONE));
    aim_delta = the_po.non_affine_slow_xform(aim_delta);

    the_po = po_identity_matrix;
    the_po.set_rotate_y((DEG_TO_RAD(15.0f) * mod) * PLUS_MINUS_ONE);
    aim_delta = the_po.non_affine_slow_xform(aim_delta);
  }

  vector3d inter2 = fire_pos+(aim_delta*1000.0f);
  if ( !find_intersection( fire_pos, inter2,
                           rgn,
                           FI_COLLIDE_WORLD|FI_COLLIDE_BEAMABLE,
                           &vec, &hitn,
                           NULL, &hit_entity ) )
  {
    vec = fire_pos + aim_delta;
    hitn = -aim_delta;
  }

  vec += hitn * 0.01f;

  my_entity->set_current_target_pos( vec );
  hitn.normalize();
  my_entity->set_current_target_norm( hitn );
  beam_n = hitn;

  vector3d vec1 = (vec - fire_pos);
  rational_t min_d = vec1.length();

  vector3d delta = (vec - fire_pos);

  vector<region_node*>      regs;

  if ( rgn )
    build_region_list( &regs, rgn, fire_pos, delta );

  for( vector<region_node*>::const_iterator ri = regs.begin(); ri != regs.end(); ++ri )
  {
    vector<entity*>::const_iterator i = (*ri)->get_data()->get_entities().begin();
    vector<entity*>::const_iterator i_end = (*ri)->get_data()->get_entities().end();

    for ( ; i<i_end; ++i )
    {
      entity* e = *i;
      if ( e!= NULL && e!=my_entity && (e->is_visible() || e == g_world_ptr->get_hero_ptr()) && e->allow_targeting() && (e == hit_entity || e->test_combat_target(fire_pos, vec, &vec1, &hitn, 1.0f)))
      {
        vector3d dv;

        if(e == hit_entity)
          dv = vec - fire_pos;
        else
          dv = vec1 - fire_pos;

        rational_t d = dv.length();

        if(d <= min_d || (e == hit_entity && d <= (min_d+0.25f)))
        {
          my_entity->set_current_target( e );

          if(e == hit_entity)
          {
            beam_n.normalize();
            my_entity->set_current_target_pos( vec + beam_n*0.01f );
            my_entity->set_current_target_norm( beam_n );
          }
          else
          {
            hitn.normalize();
            my_entity->set_current_target_pos( vec1 + hitn*0.01f );
            my_entity->set_current_target_norm( hitn );
          }

          min_d = d;
        }
      }
    }
  }

  return(true);
}


extern rational_t g_gravity;
vector3d ai_interface::calculate_jump_vector_xzvel(const vector3d &dest, rational_t xzvel)
{
  #pragma todo("Try to find a way to create better parabolas for __fabs(delta_y) > 2.0f >>> pull from D2 (projectile/weapon)")

  vector3d delta = dest - my_entity->get_abs_position();

  rational_t delta_y = delta.y;
  delta.y = 0.0f;
  rational_t delta_xz = delta.length();
  if(delta_xz > 0.0f)
  {
    delta *= (1.0f / delta_xz);
  }

  rational_t vely;
  rational_t time = delta_xz / xzvel;

  rational_t grav_mod = g_gravity * (get_my_entity()->has_physical_ifc() ? get_my_entity()->physical_ifc()->get_gravity_multiplier() : 1.0f);

  if(__fabs(delta_y) <= 2.0f)
    vely = (0.5f*grav_mod*time);
  else
  {
    if(delta_y <= 0.0f)
    {
      vely = (grav_mod * 0.5f * 0.75f);
    }
    else
    {
      vely = (delta_y/time) + (0.5f*grav_mod*time);
    }
  }

  return((vely*YVEC) + (xzvel*delta));
}

#endif // BIGCULL


vector3d ai_interface::calculate_eye_pos(entity *ent)
{
  if(ent != NULL)
    return(ent->has_ai_ifc() ? (ent->get_abs_position() + (ent->get_abs_po().get_y_facing() * ent->get_radius() * 0.5f)) : ent->get_abs_position());
  else
    return(ZEROVEC);
}



/*
//BIGCULL
bool ai_interface::send_message(ai_interface *ai, const ai_radio_message &message)
{
  return(radio->send_message(ai, message));
}

int ai_interface::broadcast_message(int channel, const ai_radio_message &message)
{
  return(radio->broadcast_message(channel, message));
}

bool ai_interface::new_radio_message() const
{
  return(radio->new_message());
}

const ai_radio_message &ai_interface::get_last_message() const
{
  return(radio->get_last_message());
}
*/


bool ai_interface::is_enemy(ai_interface *ai) const
{
  assert(ai);

  unsigned int my_team_id = get_team_id();
  unsigned int ai_team_id = ai->get_team_id();

  if(my_team_id != 0 && ai_team_id != 0 && my_team_id == ai_team_id)
    return(false);
  else
    return(is_enemy_team(ai->team));
}
