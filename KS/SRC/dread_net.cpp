#if 0
// dread_net.cpp
// Copyright (c) 1999-2000 Treyarch Invention LLC.  ALL RIGHTS RESERVED.

#include "dread_net.h"
#include "entity.h"
//!#include "character.h"
//!#include "char_group.h"
//!#include "wds.h"
#include "brain.h"
// BIGCULL #include "scanner.h"
#include "beam.h"
#include "trigger.h"
#include "debug_render.h"
// BIGCULL #include "thrown_item.h"
#include "collide.h"
#include "file_finder.h"

#if !defined(BUILD_BOOTABLE)
  #define _MAX_RECENT_DREAD_NET_CUES 10
#endif


#if !defined(BUILD_BOOTABLE)
list<av_cue> g_recent_cues;
#endif

#define _AV_CUE_DECAY_RATE  15.0f
#define _AV_CUE_DELAY       0.25f


dread_net::eAVCueType dread_net::get_cue_type(const stringx &cue)
{
  for(int i=0; i<num_cues; ++i)
  {
    if(av_cue_name[i] == cue)
      return((eAVCueType)i);
  }

  #if !defined(BUILD_BOOTABLE)
  warning("AV Cue '%s' not found!", cue.c_str());
  #endif

  return(UNDEFINED_AV_CUE);
}

stringx& dread_net::get_cue_name(eAVCueType type)
{
  static stringx UNDEFINED("<UNDEFINED>");

  if(type == UNDEFINED_AV_CUE)
    return(UNDEFINED);
  else
    return(av_cue_name[type]);
}


static void dread_net_activate_signaller_callback(signaller* sig, const char *data)
{
  dread_net *net = (dread_net *)data;

  dread_net_link *net_link = net->get_link(sig);
  if(net_link)
    net_link->process_activation(net);
}

static void dread_net_destruction_signaller_callback(signaller* sig, const char *data)
{
  dread_net *net = (dread_net *)data;

  dread_net_link *net_link = net->get_link(sig);
  if(net_link)
    net_link->process_destruction(net);
}

static void dread_net_damaged_signaller_callback(signaller* sig, const char *data)
{
  dread_net *net = (dread_net *)data;

  dread_net_link *net_link = net->get_link(sig);
  if(net_link)
    net_link->process_damaged(net);
}

static void dread_net_detection_signaller_callback(signaller* sig, const char *data)
{
  dread_net *net = (dread_net *)data;

  dread_net_link *net_link = net->get_link(sig);
  if(net_link)
    net_link->process_hero_detected(net);
}

static void dread_net_undetection_signaller_callback(signaller* sig, const char *data)
{
  dread_net *net = (dread_net *)data;

  dread_net_link *net_link = net->get_link(sig);
  if(net_link)
    net_link->process_hero_undetected(net);
}


/*
stringx dread_net::av_cue_name[dread_net::UNDEFINED_AV_CUE+1] =
{
#define MAC(a, b, c, d, e, f, g, h, i) b,
#include "av_cues.h"
#undef MAC
  "UNDEFINED_AV_CUE"
};

rational_t dread_net::av_cue_power[dread_net::UNDEFINED_AV_CUE+1] =
{
#define MAC(a, b, c, d, e, f, g, h, i) c,
#include "av_cues.h"
#undef MAC
  0.0f
};

rational_t dread_net::av_cue_stealth_mod[dread_net::UNDEFINED_AV_CUE+1] =
{
#define MAC(a, b, c, d, e, f, g, h, i) d,
#include "av_cues.h"
#undef MAC
  0.0f
};

rational_t dread_net::av_cue_turbo_mod[dread_net::UNDEFINED_AV_CUE+1] =
{
#define MAC(a, b, c, d, e, f, g, h, i) e,
#include "av_cues.h"
#undef MAC
  0.0f
};

rational_t dread_net::av_cue_radius[dread_net::UNDEFINED_AV_CUE+1] =
{
#define MAC(a, b, c, d, e, f, g, h, i) f,
#include "av_cues.h"
#undef MAC
  0.0f
};

rational_t dread_net::av_cue_radius_sqr[dread_net::UNDEFINED_AV_CUE+1] =
{
#define MAC(a, b, c, d, e, f, g, h, i) f*f,
#include "av_cues.h"
#undef MAC
  0.0f
};

bool dread_net::av_cue_investigate[dread_net::UNDEFINED_AV_CUE+1] =
{
#define MAC(a, b, c, d, e, f, g, h, i) g,
#include "av_cues.h"
#undef MAC
  false
};

bool dread_net::av_cue_search[dread_net::UNDEFINED_AV_CUE+1] =
{
#define MAC(a, b, c, d, e, f, g, h, i) h,
#include "av_cues.h"
#undef MAC
  false
};

bool dread_net::av_cue_run[dread_net::UNDEFINED_AV_CUE+1] =
{
#define MAC(a, b, c, d, e, f, g, h, i) i,
#include "av_cues.h"
#undef MAC
  true
};
*/

rational_t dread_net::get_cue_mod_power(eAVCueType type, const entity *ent)
{
/*!  if(ent && ent->get_flavor() == ENTITY_CHARACTER)
    return(get_cue_power(type) * (((const character *)ent)->is_stealth() ? av_cue_stealth_mod[type] : 1.0f) * (((const character *)ent)->is_turbo() ? av_cue_turbo_mod[type] : 1.0f));
  else
!*/
    return(get_cue_power(type));
}



vector<stringx> dread_net::av_cue_name;
vector<rational_t> dread_net::av_cue_power;
vector<rational_t> dread_net::av_cue_stealth_mod;
vector<rational_t> dread_net::av_cue_turbo_mod;
vector<rational_t> dread_net::av_cue_radius;
vector<bool> dread_net::av_cue_investigate;
vector<bool> dread_net::av_cue_search;
vector<bool> dread_net::av_cue_run;





dread_net_link::dread_net_link(signaller *s, int f)
{
  assert(s);

  sig = s;
  flags = (char)f;

  hero_detected = false;
//!  groups.clear();
  entities.clear();
}

dread_net_link::~dread_net_link()
{
  hero_detected = false;
//!  groups.clear();
  entities.clear();
}

void dread_net_link::process_activation(dread_net *net)
{
  assert(flags & _DREAD_NET_LINK_ACTIVATION);
  assert(sig->is_an_entity());

  send_cue(net, dread_net::get_cue_type("MAX_IS_DETECTED"), g_world_ptr->get_hero_ptr()->get_abs_position());
}

void dread_net_link::process_destruction(dread_net *net)
{
  assert(flags & _DREAD_NET_LINK_DESTRUCTION);
  assert(sig->is_an_entity());

  send_cue(net, dread_net::get_cue_type("MAX_IS_DETECTED"), ((entity *)sig)->get_abs_position());
//  send_cue(net, dread_net::get_cue_type("MAX_IS_DETECTED"), g_world_ptr->get_hero_ptr()->get_abs_position());
}

void dread_net_link::process_damaged(dread_net *net)
{
  assert(flags & _DREAD_NET_LINK_DAMAGED);
  assert(sig->is_an_entity());

  send_cue(net, dread_net::get_cue_type("MAX_IS_DETECTED"), ((entity *)sig)->get_abs_position());
//  send_cue(net, dread_net::get_cue_type("MAX_IS_DETECTED"), g_world_ptr->get_hero_ptr()->get_abs_position());
}

void dread_net_link::process_hero_detected(dread_net *net)
{
  assert(flags & _DREAD_NET_LINK_DETECTION);

  hero_detected = true;
}

void dread_net_link::process_hero_undetected(dread_net *net)
{
  assert(flags & _DREAD_NET_LINK_DETECTION);

  hero_detected = false;
}

bool dread_net_link::linked(entity *ent)
{
  assert(ent);

  list<entity *>::iterator i = entities.begin();
  while(i != entities.end())
  {
    if((*i) == ent)
      return(true);

    ++i;
  }

  return(false);
}

/*!bool dread_net_link::linked(char_group *grp)
{
  assert(grp);

  list<char_group *>::iterator i = groups.begin();
  while(i != groups.end())
  {
    if((*i) == grp)
      return(true);

    ++i;
  }

  return(false);
}
!*/
void dread_net_link::link(entity *ent)
{
  if(!linked(ent))
    entities.push_back(ent);
}

/*!void dread_net_link::link(char_group *grp)
{
  if(!linked(grp))
    groups.push_back(grp);
}
!*/
void dread_net_link::unlink(entity *ent)
{
  assert(ent);

  entities.remove(ent);
}

/*!void dread_net_link::unlink(char_group *grp)
{
  assert(grp);

  groups.remove(grp);
}!*/

void dread_net_link::frame_advance(dread_net *net, time_value_t time_inc)
{
  if(hero_detected || (sig->is_an_entity() && ((entity *)sig)->get_brain() != NULL && ((entity *)sig)->get_brain()->is_hero_acquired()))
    send_cue(net, dread_net::get_cue_type("MAX_IS_DETECTED"), g_world_ptr->get_hero_ptr()->get_abs_position());
}

void dread_net_link::send_cue(dread_net *net, dread_net::eAVCueType type, const vector3d &pos)
{
  list<entity *>::iterator i = entities.begin();
  while(i != entities.end())
  {
    if(*i)
      net->send_cue(*i, type, pos);

    ++i;
  }

/*!  list<char_group *>::iterator j = groups.begin();
  while(j != groups.end())
  {
    if(*j)
      net->send_cue(*j, type, pos);

    ++j;
  }
!*/
}

void dread_net_link::render(color32 col, char level)
{
  if(level >= 1)
  {
    if(sig->is_an_entity())
    {
      entity *ent = (entity *)sig;

      #ifndef BUILD_BOOTABLE
      render_sphere(ent->get_abs_position(), 1.0f, col);
      print_3d_text(ent->get_abs_position(), col, "%s (%s%s%s%s)", ent->get_name().c_str(), (flags & _DREAD_NET_LINK_ACTIVATION && ent->is_actionable()) ? "ACT " : "", (flags & _DREAD_NET_LINK_DAMAGED && ent->allow_targeting()) ? "DMG " : "", (flags & _DREAD_NET_LINK_DESTRUCTION && ent->allow_targeting()) ? "DST " : "", (flags & _DREAD_NET_LINK_DETECTION && (ent->get_flavor() == ENTITY_SCANNER || ent->get_flavor() == ENTITY_BEAM)) ? "DET " : "");

      list<entity *>::iterator i = entities.begin();
      while(i != entities.end())
      {
        if(*i && (*i)->is_alive())
          render_beam(ent->get_abs_position(), (*i)->get_abs_position(), col, 0.1f);

        ++i;
      }

/*!      list<char_group *>::iterator j = groups.begin();
      while(j != groups.end())
      {
        if(*j)
        {
          char_group::iterator i = (*j)->begin();
          while(i != (*j)->end())
          {
            if(*i && (*i)->is_alive())
              render_beam(ent->get_abs_position(), (*i)->get_abs_position(), col, 0.1f);

            ++i;
          }
        }

        ++j;
      }
!*/
      #endif // DEBUG
    }
    else if(sig->is_a_trigger())
    {
    }
  }
}







int dread_net::num_cues = 0;

dread_net::dread_net()
{
#if !defined(BUILD_BOOTABLE)
  g_recent_cues.clear();
#endif

  av_cue_name.clear();
  av_cue_power.clear();
  av_cue_stealth_mod.clear();
  av_cue_turbo_mod.clear();
  av_cue_radius.clear();
  av_cue_investigate.clear();
  av_cue_search.clear();
  av_cue_run.clear();

  num_cues = 0;
}

dread_net::~dread_net()
{
#if !defined(BUILD_BOOTABLE)
  g_recent_cues.clear();
#endif

  av_cue_name.clear();
  av_cue_power.clear();
  av_cue_stealth_mod.clear();
  av_cue_turbo_mod.clear();
  av_cue_radius.clear();
  av_cue_investigate.clear();
  av_cue_search.clear();
  av_cue_run.clear();

  num_cues = 0;

  for(list<dread_net_link *>::iterator i = links.begin(); i != links.end(); ++i)
  {
    if(*i)
      delete (*i);
  }

  links.clear();
}

void dread_net::add_cue_recurse( const av_cue& cue, const vector3d& cur_pos, region_node* rgn, float cue_rad )
{
  if ( rgn==NULL || rgn->get_data()==NULL )
    return;

  region* r = rgn->get_data();
  r->visit();

  brain* brn = NULL;
  entity* ent = NULL;

  vector<entity *>::const_iterator i = r->get_entities().begin();
  vector<entity *>::const_iterator i_end = r->get_entities().end();
  for ( ; i!=i_end; ++i )
  {
    ent = *i;
    if ( ent
      && !ent->is_hero()
      && !ent->already_visited()
      && ent->get_region() == rgn
      && (brn = ent->get_brain()) != NULL
      && brn->is_connected()
      && brn->is_active()
      && brn->brain_active()
      )
    {
      ent->visit();
      if(brn->get_reaction_cue_radius() > 0.0f)
      {
        rational_t alert_rad = brn->get_reaction_cue_radius() + cue_rad;
        if ( alert_rad > 0 )
        {
          rational_t len = (ent->get_abs_position() - cur_pos).length2();
          if ( len <= alert_rad*alert_rad )
            add_cue( brn, cue );
        }
      }
    }
  }

  // check for intersection with portals leading from this region
  edge_iterator tei = rgn->begin();
  edge_iterator tei_end = rgn->end();

  for ( ; tei!=tei_end; ++tei )
  {
    region_node* dest = (*tei).get_dest();
    portal* port = (*tei).get_data();
    if ( dest && port && port->is_active() )
    {
      region* dr = dest->get_data();
      // don't bother with inactive regions or those we've already visited
      if ( dr && dr->is_active() && !dr->already_visited() )
      {
        sphere cue_sphere( cur_pos, cue_rad+10.0f );  // add padding to allow for brain cue radius
        if ( port->touches_sphere(cue_sphere) )
        {
          // compute nearest point on portal disc
          vector3d new_pos = cur_pos - port->get_effective_center();
          new_pos -= port->get_cylinder_normal() * dot( new_pos, port->get_cylinder_normal() );
          rational_t len2 = new_pos.length2();
          if ( len2 > port->get_effective_radius()*port->get_effective_radius() )
            new_pos *= port->get_effective_radius() / __fsqrt(len2);
          new_pos += port->get_effective_center();
          // recurse with adjusted radius at NEW position
          rational_t dist = (new_pos - cur_pos).length();
          add_cue_recurse( cue, new_pos, dest, cue_rad-dist );
        }
      }
    }
  }
}

void dread_net::add_cue_helper(eAVCueType type, const vector3d &pos, region_node* rgn, const entity *ent)
{
  if(type >= 0 && type < num_cues)
  {
    av_cue cue(type, pos, rgn, ent);

    #if !defined(BUILD_BOOTABLE)
      g_recent_cues.push_front(cue);
      while(g_recent_cues.size() > _MAX_RECENT_DREAD_NET_CUES)
        g_recent_cues.pop_back();
    #endif

    region::prepare_for_visiting();
    entity::prepare_for_visiting();
    add_cue_recurse( cue, pos, rgn, av_cue_radius[cue.type] );
  }
  else
    error("Undefined AV cue enum: %d", type);
}

void dread_net::add_cue(eAVCueType type, const vector3d &pos)
{
  sector *sect = g_world_ptr->get_the_terrain().find_sector(pos);

  if(sect && sect->get_region())
    add_cue_helper(type, pos, sect->get_region(), NULL);
}

void dread_net::add_cue(brain *brn, const av_cue &cue)
{
  if(brn->is_active() && brn->brain_active() && cue.power >= brn->get_min_cue_power())
  {
    av_cue my_cue = cue;

    if ( cue.type == get_cue_type("RICOCHET")
      && cue.owner != NULL
      && cue.get_region() != NULL
      && brn->am_i_facing_point( my_cue.pos, DEG_TO_RAD(180.0f), true )
      )
    {
      vector3d delta = cue.get_owner_position() - cue.pos;
      rational_t len = delta.length();

      if ( len > 0.0f )
      {
        delta *= (1.0f / len);
        len += PLUS_MINUS_ONE*(0.5f*len);
        if(len < 3.0f)
          len = 3.0f;
        delta *= len;

        po rot_po = po_identity_matrix;
        rot_po.set_rotate_y(PLUS_MINUS_ONE*DEG_TO_RAD(45.0f));
        delta = rot_po.non_affine_slow_xform(delta);

        vector3d dest = cue.pos + delta;

        vector3d hit, hitn;
        if ( find_intersection( cue.pos, dest,
                                cue.get_region(),
                                FI_COLLIDE_WORLD|FI_COLLIDE_BEAMABLE,
                                &hit, &hitn ) )
        {
          hitn.y = 0.0f;
          dest = hit + hitn;
        }

        my_cue.pos = dest;
      }
    }

    #if !defined(BUILD_BOOTABLE)
      if(cue.type == get_cue_type("I_WAS_DAMAGED") || cue.type == get_cue_type("RICOCHET"))
      {
        g_recent_cues.push_front(my_cue);

        while(g_recent_cues.size() > _MAX_RECENT_DREAD_NET_CUES)
          g_recent_cues.pop_back();
      }
    #endif

    if(brn->cue_FIFO.size() == 0)
    {
      if(!brn->next_cue.reached)
      {
        rational_t rel_power = brn->next_cue.power - (cue.delay * _AV_CUE_DECAY_RATE);
        if(cue.power > rel_power)
        {
          brn->cue_FIFO.push_back(my_cue);
        }
      }
      else if(!brn->current_cue.reached)
      {
        rational_t rel_power = brn->current_cue.power - (cue.delay * _AV_CUE_DECAY_RATE);
        if(cue.power > rel_power)
        {
          brn->cue_FIFO.push_back(my_cue);
        }
      }
      else
      {
        brn->cue_FIFO.push_back(my_cue);
      }
    }
    else
    {
      vector<av_cue>::iterator tmp_cue_iter = brn->cue_FIFO.begin();
      vector<av_cue>::iterator cue_iter = brn->cue_FIFO.begin();

      while(tmp_cue_iter != brn->cue_FIFO.end())
      {
        cue_iter = tmp_cue_iter;
        tmp_cue_iter++;
      }

      av_cue fifo_cue = (*cue_iter);

      if(fifo_cue.delay == cue.delay)
      {
        if(cue.power > fifo_cue.power)
        {
          (*cue_iter) = my_cue;
        }
      }
      else if(fifo_cue.type == cue.type)
      {
        // copy the position of identical cue
        (*cue_iter).pos = my_cue.pos;
      }
      else
      {
        rational_t rel_power = fifo_cue.power - ((cue.delay - fifo_cue.delay) * _AV_CUE_DECAY_RATE);
        if(cue.power > rel_power)
        {
          brn->cue_FIFO.push_back(my_cue);
        }
      }
    }
  }
}


void dread_net::send_cue(entity *ent, const av_cue &cue)
{
  assert(ent);
  brain *brn = NULL;

  if(!ent->is_hero() && (brn = ent->get_brain()) != NULL && brn->is_connected())
    add_cue(brn, cue);
}


void dread_net::send_cue(entity *ent, eAVCueType type, const vector3d &pos)
{
  if(type >= 0 && type < num_cues)
  {
    av_cue cue(type, pos);
    send_cue(ent, cue);
  }
  else
    error("Undefined AV cue enum: %d", type);
}

/*!
void dread_net::send_cue(char_group *grp, const av_cue &cue)
{
  assert(grp);

  char_group::iterator i = grp->begin();
  while(i != grp->end())
  {
    send_cue(*i, cue);

    ++i;
  }

}

void dread_net::send_cue(char_group *grp, eAVCueType type, const vector3d &pos)
{
  if(type >= 0 && type < num_cues)
  {
    av_cue cue(type, pos);
    send_cue(grp, cue);
  }
  else
    error("Undefined AV cue enum: %d", type);
}
!*/

void dread_net::spotted_hero(entity *ent)
{
  assert(ent);
  brain *brn = NULL;

  if(!ent->is_hero() && (brn = ent->get_brain()) != NULL)
  {
    av_cue cue(get_cue_type("MAX_IS_SEEN"), g_world_ptr->get_hero_ptr()->get_abs_position());
    add_cue(brn, cue);

    if(brn->is_connected())
    {
      // dunno what yet!
    }

/*!
    if(ent->get_flavor() == ENTITY_CHARACTER && brn->radio_enabled())
    {
      character *chr = (character *)ent;
      av_cue cue(get_cue_type("BUDDY_SEES_MAX"), g_world_ptr->get_hero_ptr()->get_abs_position());

      // tell all my buddies (who have their radios turned on)
      vector<char_group*>::const_iterator i = chr->get_char_groups().begin();
      vector<char_group*>::const_iterator i_end = chr->get_char_groups().end();

      for ( ; i!=i_end; ++i )
      {
        char_group *grp = (*i);

        if(grp != char_group_manager::inst()->get_group_all())
        {
          vector<character *>::iterator c = grp->begin();
          while(c != grp->end())
          {
            character *chrX = (*c);
            brain *bud_brn = NULL;
            if(!chrX->is_hero() && (bud_brn = chrX->get_brain()) != NULL && bud_brn->radio_enabled() && bud_brn != brn)
              add_cue(bud_brn, cue);

            ++c;
          }
        }
      }
    }
!*/
  }
}

void dread_net::was_damaged(entity *ent)
{
  assert(ent);
  brain *brn = NULL;

  if(!ent->is_hero() && (brn = ent->get_brain()) != NULL)
  {
    assert(ent->has_damage_ifc());

    switch(ent->damage_ifc()->get_damage_type())
    {
      case DAMAGE_MELEE:
      case DAMAGE_NONBLOCK_MELEE:
      case DAMAGE_KNOCKING_DOWN:
      {
        av_cue cue(get_cue_type("I_WAS_DAMAGED"), ent->damage_ifc()->get_damage_attacker() != NULL ? ent->damage_ifc()->get_damage_attacker()->get_abs_position() : ent->get_abs_position());
        add_cue(brn, cue);
      }
      break;

      case DAMAGE_EXPLOSIVE:
      {
        if(ent->damage_ifc()->get_damage_item() != NULL && ent->damage_ifc()->get_damage_item()->is_a_thrown_item())
        {
          av_cue cue(get_cue_type("I_WAS_DAMAGED"), ((thrown_item *)ent->damage_ifc()->get_damage_item())->get_detonate_position());
          add_cue(brn, cue);
        }
        else
        {
          av_cue cue(get_cue_type("I_WAS_DAMAGED"), ent->damage_ifc()->get_damage_attacker() != NULL ? ent->damage_ifc()->get_damage_attacker()->get_abs_position() : ent->get_abs_position());
          add_cue(brn, cue);
        }
      }
      break;

      case DAMAGE_GUN:
      case DAMAGE_DIRECT_DIRECTIONAL:
      {
        if(ent->damage_ifc()->get_damage_attacker() != NULL)
        {
          vector3d delta = ent->damage_ifc()->get_damage_attacker()->get_abs_position() - ent->get_abs_position();
          rational_t len = delta.length();

          if(len < 2.0f || brn->LOS_check(ent->damage_ifc()->get_damage_attacker()))
          {
            av_cue cue(get_cue_type("I_WAS_DAMAGED"), ent->damage_ifc()->get_damage_attacker()->get_abs_position());
            add_cue(brn, cue);
          }
          else
          {
            delta *= 1.0f / len;
            len += PLUS_MINUS_ONE*(0.5f*len);
            if(len < 3.0f)
              len = 3.0f;
            delta *= len;

            po rot_po = po_identity_matrix;
            rot_po.set_rotate_y(PLUS_MINUS_ONE*DEG_TO_RAD(45.0f));
            delta = rot_po.non_affine_slow_xform(delta);

            vector3d dest = ent->get_abs_position() + delta;

            vector3d hit, hitn;
            if ( find_intersection( ent->get_abs_position(), dest,
                                    ent->get_region(),
                                    FI_COLLIDE_WORLD|FI_COLLIDE_BEAMABLE,
                                    &hit, &hitn ) )
            {
              hitn.y = 0.0f;
              dest = hit + hitn;
            }

            av_cue cue(get_cue_type("I_WAS_DAMAGED"), dest);
            add_cue(brn, cue);
          }
        }
        else
        {
          av_cue cue(get_cue_type("I_WAS_DAMAGED"), ent->get_abs_position());
          add_cue(brn, cue);
        }
      }
      break;

      case DAMAGE_DIRECT:
      default:
      {
        av_cue cue(get_cue_type("I_WAS_DAMAGED"), ent->get_abs_position());
        add_cue(brn, cue);
      }
      break;
    }

    if(brn->is_connected())
    {
      // dunno what yet!
    }

    // if it was a silent kill, radio has been disabled already
/*!    if(ent->is_a_character() && brn->radio_enabled())
    {
      assert(ent->get_damage_type() != DAMAGE_SILENT_KILL);

      character *chr = (character *)ent;
      av_cue cue(get_cue_type("BUDDY_DAMAGED"), chr->get_abs_position());

      // tell all my buddies (who have their radios turned on)
      vector<char_group*>::const_iterator i = chr->get_char_groups().begin();
      vector<char_group*>::const_iterator i_end = chr->get_char_groups().end();

      for ( ; i!=i_end; ++i )
      {
        char_group *grp = (*i);

        if(grp != char_group_manager::inst()->get_group_all())
        {
          vector<character *>::iterator c = grp->begin();
          while(c != grp->end())
          {
            character *chrX = (*c);
            brain *bud_brn = NULL;
            if(!chrX->is_hero() && (bud_brn = chrX->get_brain()) != NULL && bud_brn->radio_enabled() && bud_brn != brn)
              add_cue(bud_brn, cue);

            ++c;
          }
        }
      }
    }
!*/
  }
}

void dread_net::was_killed(entity *ent)
{
  assert( ent != NULL );
  brain* brn = ent->get_brain();

  if ( !ent->is_hero() && brn!=NULL )
  {
    if ( brn->is_connected() )
    {
      // dunno what yet!
    }

    add_cue( get_cue_type("ENEMY_KILLED"), ent );

    // if it was a silent kill, radio has been disabled already
/*!    if ( ent->is_a_character() && brn->radio_enabled() )
    {
      assert(ent->get_damage_type() != DAMAGE_SILENT_KILL);

      character* chr = (character*)ent;

      av_cue cue( get_cue_type("BUDDY_KILLED"), chr->get_abs_position() );

      // tell all my buddies (who have their radios turned on)
      vector<char_group*>::const_iterator i = chr->get_char_groups().begin();
      vector<char_group*>::const_iterator i_end = chr->get_char_groups().end();

      for ( ; i!=i_end; ++i )
      {
        char_group *grp = (*i);

        if(grp != char_group_manager::inst()->get_group_all())
        {
          vector<character *>::iterator c = grp->begin();
          while(c != grp->end())
          {
            character *chrX = (*c);
            brain *bud_brn = NULL;
            if(!chrX->is_hero() && (bud_brn = chrX->get_brain()) != NULL && bud_brn->radio_enabled() && bud_brn != brn)
              add_cue(bud_brn, cue);

            ++c;
          }
        }
      }
    }
!*/
  }
}

void dread_net::initialize()
{
}

void dread_net::clear()
{
}

void dread_net::frame_advance(time_value_t time_inc)
{
  if(is_active())
  {
    list<dread_net_link *>::iterator i = links.begin();
    while(i != links.end())
    {
      if(*i)
        (*i)->frame_advance(this, time_inc);

      ++i;
    }
  }
}

void dread_net::add_signaller(signaller *sig, int event_flags)
{
  assert(sig);

  dread_net_link *net_link = get_link(sig);

  if(!net_link)
  {
    net_link = NEW dread_net_link(sig, event_flags);

    if(net_link)
    {
      links.push_back(net_link);

      signal* sptr = NULL;

      if(net_link->sig->is_an_entity())
      {
        entity *ent = (entity *)net_link->sig;

        if(net_link->flags & _DREAD_NET_LINK_ACTIVATION && ent->is_actionable())
        {
          sptr = ent->signal_ptr(entity::ACTIVATED_BY_CHARACTER);
          assert(sptr);

          if( sptr )
            sptr->add_callback(dread_net_activate_signaller_callback, (char *)this);
        }

        if(net_link->flags & _DREAD_NET_LINK_DAMAGED && ent->allow_targeting())
        {
          sptr = ent->signal_ptr(entity::DAMAGED);
          assert(sptr);

          if( sptr )
            sptr->add_callback(dread_net_damaged_signaller_callback, (char *)this);
        }

        if(net_link->flags & _DREAD_NET_LINK_DESTRUCTION && ent->allow_targeting())
        {
/*!          if(ent->get_flavor() == ENTITY_CHARACTER)
            sptr = ((character *)ent)->signal_ptr(character::KILLED);
          else
!*/
            sptr = ent->signal_ptr(entity::DESTROYED);

          assert(sptr);

          if( sptr )
            sptr->add_callback(dread_net_destruction_signaller_callback, (char *)this);
        }

        if(net_link->flags & _DREAD_NET_LINK_DETECTION && (ent->get_flavor() == ENTITY_SCANNER || ent->get_flavor() == ENTITY_BEAM || ent->get_brain() != NULL))
        {
          if(ent->get_flavor() == ENTITY_SCANNER)
            sptr = ((scanner *)ent)->signal_ptr(scanner::ENTER);
          else if(ent->get_flavor() == ENTITY_BEAM)
            sptr = ((beam *)ent)->signal_ptr(beam::ENTER);
          else
            sptr = NULL;

          if( sptr )
            sptr->add_callback(dread_net_detection_signaller_callback, (char *)this);


          if(ent->get_flavor() == ENTITY_SCANNER)
            sptr = ((scanner *)ent)->signal_ptr(scanner::LEAVE);
          else if(ent->get_flavor() == ENTITY_BEAM)
            sptr = ((beam *)ent)->signal_ptr(beam::LEAVE);
          else
            sptr = NULL;

          if( sptr )
            sptr->add_callback(dread_net_undetection_signaller_callback, (char *)this);
        }
      }
      else if(net_link->sig->is_a_trigger())
      {
        if(net_link->flags & _DREAD_NET_LINK_DETECTION)
        {
          sptr = ((trigger *)net_link->sig)->signal_ptr(trigger::ENTER);

          assert(sptr);

          if( sptr )
            sptr->add_callback(dread_net_detection_signaller_callback, (char *)this);


          sptr = ((trigger *)net_link->sig)->signal_ptr(trigger::LEAVE);

          assert(sptr);

          if( sptr )
            sptr->add_callback(dread_net_undetection_signaller_callback, (char *)this);
        }
      }
    }
  }
}


void dread_net::remove_signaller(signaller *sig)
{
  assert(sig);

  list<dread_net_link *>::iterator i = links.begin();
  while(i != links.end())
  {
    if((*i) && (*i)->sig == sig)
    {
      delete (*i);
      *i = NULL;
      i = links.erase(i);

      break;
    }

    ++i;
  }
}

/*!void dread_net::link(signaller *sig, char_group *grp)
{
  dread_net_link *net_link = get_link(sig);

  if(net_link)
    net_link->link(grp);
  else
    error("Signaller was not linked into DreadNet!");
}
!*/
void dread_net::link(signaller *sig, entity *ent)
{
  dread_net_link *net_link = get_link(sig);

  if(net_link)
    net_link->link(ent);
  else
    error("Signaller was not linked into DreadNet!");
}

/*!void dread_net::unlink(signaller *sig, char_group *grp)
{
  dread_net_link *net_link = get_link(sig);

  if(net_link)
    net_link->unlink(grp);
  else
    error("Signaller was not linked into DreadNet!");
}!*/

void dread_net::unlink(signaller *sig, entity *ent)
{
  dread_net_link *net_link = get_link(sig);

  if(net_link)
    net_link->unlink(ent);
  else
    error("Signaller was not linked into DreadNet!");
}

dread_net_link *dread_net::get_link(signaller *sig)
{
  list<dread_net_link *>::iterator i = links.begin();
  while(i != links.end())
  {
    if((*i) && (*i)->sig == sig)
      return(*i);

    ++i;
  }

  return(NULL);
}

void dread_net::read_data(chunk_file &fs, stringx &label)
{
  assert(label == "dread_net:" || label == "DREAD_NET:");

  for( serial_in(fs, &label); label != chunkend_label; serial_in(fs, &label) )
  {
    if(label == "signaller" || label == "SIGNALLER")
    {
      int flags;
      serial_in(fs, &label);
      label.to_upper();

      serial_in(fs, &flags);

      signaller *sig_trig = trigger_manager::inst()->find_instance(label);
      signaller *sig_ent = g_world_ptr->get_entity(label);

      if(sig_trig == NULL && sig_ent == NULL)
        error("Signaller '%s' does not exist!", label.c_str());
      else if(sig_trig == NULL)
        add_signaller(sig_ent, flags);
      else if(sig_ent == NULL)
        add_signaller(sig_trig, flags);
      else
        error("Ambiguous signaller '%s' (name used for both an entity and a trigger)!", label.c_str());
    }
    else if(label == "link" || label == "LINK")
    {
      serial_in(fs, &label);
      label.to_upper();

      dread_net_link *net_link = NULL;

      signaller *sig_trig = trigger_manager::inst()->find_instance(label);
      signaller *sig_ent = g_world_ptr->get_entity(label);

      if(sig_trig == NULL && sig_ent == NULL)
        error("Signaller '%s' does not exist!", label.c_str());
      else if(sig_trig == NULL)
        net_link = get_link(sig_ent);
      else if(sig_ent == NULL)
        net_link = get_link(sig_trig);
      else
        error("Ambiguous signaller '%s' (name used for both an entity and a trigger)!", label.c_str());

      stringx linker;
      serial_in(fs, &linker);
      linker.to_upper();

/*!      if(net_link)
      {
        char_group *grp = char_group_manager::inst()->find(linker, char_group_manager::FAIL_OK);
        entity *ent = NULL;

        if(grp)
        {
          net_link->link(grp);
        }
        else if((ent = g_world_ptr->get_entity(linker)) != NULL)
        {
          if(!ent->is_hero())
            net_link->link((entity *)ent);
          else
            error("Cannot link the hero!");
        }
        else
          error("No group or entity '%s' to add!", linker.c_str());
      }
      else
        error("DreadNet does not have signaller '%s' added!", label.c_str());
!*/
    }
    else
      error("Bad keyword '%s' in DreadNet section!", label.c_str());
  }
}


#if _ENABLE_WORLD_EDITOR
void dread_net::write_data(ofstream &out)
{
}
#endif

void dread_net::render(color32 col, char level)
{
#if !defined(BUILD_BOOTABLE)
  if(level >= 1)
  {
    list<av_cue>::iterator i = g_recent_cues.begin();

    while(i != g_recent_cues.end())
    {
      render_marker((*i).pos, color32(255, 127, 0, 127), 0.35f);
      render_sphere((*i).pos, av_cue_radius[(*i).type], color32(255, 127, 0, 96));
      print_3d_text((*i).pos, color32(255, 127, 0, 96), "%s\npower %.2f\nradius %.2f\nflags (%s%s%s)", av_cue_name[(*i).type].c_str(), av_cue_power[(*i).type], av_cue_radius[(*i).type], av_cue_investigate[(*i).type] ? "I" : "", av_cue_search[(*i).type] ? "S" : "", av_cue_run[(*i).type] ? "R" : "");

      ++i;
    }
  }
#endif

  if(level >= 2)
  {
    list<dread_net_link *>::iterator i = links.begin();
    while(i != links.end())
    {
      if(*i)
        (*i)->render(color32(0, 0, 255, 96), 1);

      ++i;
    }
  }

  if(level >= 3)
  {
/*!    char_group_manager::iterator i = char_group_manager::inst()->begin();
    char_group_manager::iterator i_end = char_group_manager::inst()->end();

    while(i != i_end)
    {
      char_group *grp = (*i).second;
      ++i;

      if(grp != char_group_manager::inst()->get_group_all())
      {
        vector<character *>::iterator c = grp->begin();
        while(c != grp->end())
        {
          if((*c) && !(*c)->is_hero() && (*c)->is_alive())
          {
            vector<character *>::iterator c2 = grp->begin();
            while(c2 != grp->end())
            {
              if((*c2) && !(*c2)->is_hero() && (*c2)->is_alive())
                render_beam((*c)->get_abs_position(), (*c2)->get_abs_position(), ((*c)->get_brain()->radio_enabled() && (*c2)->get_brain()->radio_enabled()) ? color32(0, 255, 0, 64) : color32(255, 0, 0, 64), 0.1f);

              ++c2;
            }
          }

          ++c;
        }
      }
    }
!*/
  }
}








void dread_net::read_cue_file( const stringx& filename )
{
  chunk_file fs;

  // load NEW file
  stringx avc_filename = g_file_finder->find_file( filename, ".avc", false );

  fs.open( avc_filename );
  read_cue_file(fs);
  fs.close();
}

void dread_net::read_cue_file(chunk_file &fs)
{
  stringx label;

  assert(num_cues == 0);

  for( serial_in(fs, &label); label.length() > 0 && label != chunkend_label; serial_in(fs, &label) )
  {
    if(label == "cue:")
    {
      read_cue(fs, label);
      ++num_cues;
    }
    else
      error("Bad identifier '%s' in AV Cue file '%s'", label.c_str(), fs.get_filename().c_str());
  }
}

void dread_net::read_cue(chunk_file &fs, stringx &label)
{
  av_cue_name.push_back(stringx("UNDEFINED"));
  av_cue_power.push_back(0.0f);
  av_cue_radius.push_back(0.0f);
  av_cue_stealth_mod.push_back(1.0f);
  av_cue_turbo_mod.push_back(1.0f);
  av_cue_investigate.push_back(false);
  av_cue_search.push_back(false);
  av_cue_run.push_back(false);

  assert(num_cues == (int)(av_cue_name.size()-1));

  for( serial_in(fs, &label); label != chunkend_label; serial_in(fs, &label) )
  {
    if(label == "name")
    {
      serial_in(fs, &label);
      av_cue_name[num_cues] = label;
    }
    else if(label == "power")
    {
      rational_t power;
      serial_in(fs, &power);
      av_cue_power[num_cues] = power;
    }
    else if(label == "radius")
    {
      rational_t radius;
      serial_in(fs, &radius);
      av_cue_radius[num_cues] = radius;
    }
    else if(label == "stealth_mod")
    {
      rational_t stealth_mod;
      serial_in(fs, &stealth_mod);
      av_cue_stealth_mod[num_cues] = stealth_mod;
    }
    else if(label == "turbo_mod")
    {
      rational_t turbo_mod;
      serial_in(fs, &turbo_mod);
      av_cue_turbo_mod[num_cues] = turbo_mod;
    }
    else if(label == "investigate")
    {
      av_cue_investigate[num_cues] = true;
    }
    else if(label == "search")
    {
      av_cue_search[num_cues] = true;
    }
    else if(label == "run")
    {
      av_cue_run[num_cues] = true;
    }
    else
      error("Bad identifier '%s' in AV Cue file '%s'", label.c_str(), fs.get_filename().c_str());
  }
}





/////////////////////////////////////////////////////////////////////////////
// Event signals
/////////////////////////////////////////////////////////////////////////////

static const char* signal_names[] =
  {
  #define MAC(label,str)  str,
  #include "dread_net_signals.h"
  #undef MAC
  };

unsigned short dread_net::get_signal_id( const char *name )
  {
  unsigned idx;

  for( idx = 0; idx < (sizeof(signal_names)/sizeof(char*)); idx++ )
    {
    unsigned offset = strlen(signal_names[idx])-strlen(name);

    if( offset > strlen( signal_names[idx] ) )
      continue;

    if( !strcmp(name,&signal_names[idx][offset]) )
      return( idx + PARENT_SYNC_DUMMY + 1 );
    }

  // not found
  return signaller::get_signal_id( name );
  }

// This static function must be implemented by every class which can generate
// signals, and is called once only by the application for each such class;
// the effect is to register the name and local id of each signal with the
// signal_manager.  This call must be performed before any signal objects are
// actually created for this class (via signaller::signal_ptr(); see signal.h).
void dread_net::register_signals()
  {
  // for descendant class, replace "dread_net" with appropriate string
  #define MAC(label,str)  signal_manager::inst()->insert( str, label );
  #include "dread_net_signals.h"
  #undef MAC
  }

// This virtual function, used only for debugging purposes, returns the
// name of the given local signal
const char* dread_net::get_signal_name( unsigned short idx ) const
  {
  assert( idx < N_SIGNALS );
  if ( idx <= (unsigned short)PARENT_SYNC_DUMMY )
    return signaller::get_signal_name( idx );
  else
    return signal_names[idx-PARENT_SYNC_DUMMY-1];
  }



av_cue::av_cue()
{
  type = dread_net::UNDEFINED_AV_CUE;
  power = 0.0f;
  pos = ZEROVEC;
  reached = false;
  investigate = false;
  search = false;
  run = false;
  delay = _AV_CUE_DELAY;
  owner = NULL;
  region = NULL;
}

av_cue::av_cue(dread_net::eAVCueType t, const vector3d &p, region_node *rgn, const entity *ent)
{
  type = t;
  pos = p;
  owner = ent;
  region = rgn;

  reached = false;
  delay = _AV_CUE_DELAY;

  power = dread_net::get_cue_mod_power(type, owner);
  investigate = dread_net::get_cue_investigate(type);
  search = dread_net::get_cue_search(type);
  run = dread_net::get_cue_run(type);
}

av_cue::~av_cue()
{
}

void av_cue::copy(const av_cue &b)
{
  region = b.region;
  owner = b.owner;
  type = b.type;
  power = b.power;
  pos = b.pos;
  reached = b.reached;
  investigate = b.investigate;
  search = b.search;
  run = b.run;
  delay = b.delay;
}

void av_cue::set_reached(bool r)
{
  reached = r;
}

void av_cue::frame_advance(time_value_t time_inc)
{
  if(power > 0.0f)
  {
    power -= (_AV_CUE_DECAY_RATE * time_inc);

    if(power < 0.0f)
      power = 0.0f;
  }
}


#endif
