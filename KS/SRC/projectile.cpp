#if 0

#include "global.h"

//!#include "character.h"
//#include "projectile.h"
#include "wds.h"
#include "colgeom.h"
#include "capsule.h"
#include "collide.h"
#include "vm_thread.h"
//!#include "actor.h"
#include "terrain.h"
//!#include "attrib.h"
//!#include "character.h"
#include "entity_maker.h"

int ranged_attack::next_id = 0;

ranged_attack::ranged_attack( entity *a, const stringx& script_object, const stringx& script_func )
{
	ranged_attack *ra;

	attacker = a;
	id = next_id++;

	cast_so = g_world_ptr->get_script_manager()->find_object(script_object);
	if(!cast_so){
		stringx err = "script object type " + script_object + " not found.\n";
		error(err.c_str());
	}
	cast_func = cast_so->find_func(script_func + "(ranged_attack)");
	if(cast_func == -1){
		stringx err = script_func + " is not a member of script object " + cast_so->get_name();
		error(err.c_str());
	}
//P	int before = memtrack::get_total_alloced();
	cast_inst = cast_so->add_instance("ra_" + script_object + itos(id));
	// create script thread
	cast_vm = cast_so->add_thread(cast_inst, cast_func);
	ra = this;
	cast_vm->get_data_stack().push((char *)&ra, 4);
}

projectile *ranged_attack::create_projectile( stringx entity_name )
{
  // create the NEW projectile
  projectile* p = static_cast<projectile*>( g_entity_maker->create_entity_or_subclass( entity_name, ANONYMOUS, po_identity_matrix, stringx(), 0 ) );
  p->set_parent( this );
  p->set_visible( true );
  p->set_active( true );
  // add it to the projectile list
  projectile_list.push_back( p );
  // make the next projectile created use a NEW id
  id = next_id++;
  return p;
}

void ranged_attack::remove_projectile(projectile *p)
{
//warning("projectile_list.remove");
	projectile_list.remove(p);
	// if no more projectiles exist, destroy the ranged attack
	if(!projectile_list.size()) delete this;
}


///////////////////////////////////////////////////////////////////////////////
// Projectile
///////////////////////////////////////////////////////////////////////////////

projectile::projectile( const entity_id& _id, unsigned int _flags )
  :   physical_entity( _id, ENTITY_PROJECTILE, _flags )
{
}


projectile::projectile( const stringx& entity_fname, const entity_id& _id )
  :   physical_entity( entity_fname, _id, ENTITY_PROJECTILE )
{
  initialize_variables();

	// read the extra projectile specific info
	read_info();

  // compute collision stuff
  compute_colgeom();
}


projectile::~projectile()
{
  parent->remove_projectile( this );
  g_world_ptr->remove_entity( this );
}


void projectile::initialize_variables()
{
	parent = NULL;
	flags = 0;
	hit = HIT_None;
	source = NULL;
	target = NULL;
	launch_velocity = 0.0f;
	launch_angle = 0.0f;
	set_radius( get_visual_radius() );
	in_flight = false;
	fizzle_time = 0.0f;
  prep_so = NULL;
  prep_func = -1;
  prep_vm = NULL;
  launch_so = NULL;
	launch_func = -1;
	launch_vm = NULL;
  hit_so = NULL;
	hit_func = -1;
	hit_vm = NULL;
}


void projectile::compute_colgeom()
{
	collision_geometry* colgeom = get_colgeom();
	assert( colgeom->get_type() == collision_geometry::CAPSULE );
	capsule cap;
	cap.base   = vector3d(0.0f, 0.0f, 0.0f);
	cap.end    = vector3d(0.0f, 0.0f, 0.0f);
	cap.radius = 0.25f;
	((collision_capsule*)colgeom)->set_capsule( cap );
	set_collision_flags( 0 );
}


const stringx projectile_chunk( "projectile:" );
const stringx auto_target_chunk( "auto_target" );
const stringx seek_chunk( "seek" );
const stringx mass_chunk( "mass" );
const stringx launch_velocity_chunk( "launch_velocity" );
const stringx launch_angle_chunk( "launch_angle" );
const stringx prep_func_chunk( "prep_func" );
const stringx launch_func_chunk( "launch_func" );
const stringx hit_func_chunk( "hit_func" );
const stringx fizzle_time_chunk( "fizzle_time" );

void projectile::read_info()
{
  stringx label, script_object, script_func;
  rational_t r;

  serial_in( *my_fs, &label );
  assert( label == projectile_chunk );
  do
  {
    serial_in(*my_fs, &label);

    if ( label == auto_target_chunk )
    {
	    flags |= PROJECTILE_AutoTarget;
    }

    else if ( label == seek_chunk )
    {
	    flags |= PROJECTILE_Seek;
    }

    else if ( label == mass_chunk )
    {
	    serial_in( *my_fs, &r );
	    set_mass( r );
    }

    else if ( label == launch_velocity_chunk )
    {
	    serial_in( *my_fs, &launch_velocity );
    }

    else if ( label == launch_angle_chunk )
    {
	    serial_in( *my_fs, &launch_angle );
    }

    else if ( label == prep_func_chunk )
    {
      // read script object name
      serial_in( *my_fs, &script_object );
      prep_so = g_world_ptr->get_script_manager()->find_object( script_object );
      #if defined( TARGET_PC )
      if ( !prep_so )
	      error( "script object type " + script_object + " not found" );
      #endif
      // read event function name
      serial_in( *my_fs, &script_func );
      prep_func = prep_so->find_func( script_func + "(ranged_attack,projectile)" );
      #if defined( TARGET_PC )
      if ( prep_func == -1 )
        error( script_func + " is not a member of script object " + script_object );
      #endif
    }

    else if ( label == launch_func_chunk )
    {
	    // read script object name
	    serial_in( *my_fs, &script_object );
	    launch_so = g_world_ptr->get_script_manager()->find_object( script_object );
      #if defined( TARGET_PC )
	    if ( !launch_so )
		    error( "script object type " + script_object + " not found" );
      #endif
	    // read event function name
	    serial_in( *my_fs, &script_func );
	    launch_func = launch_so->find_func( script_func + "(ranged_attack,projectile,entity,entity)" );
      #if defined( TARGET_PC )
	    if ( launch_func == -1 )
		    error( script_func + " is not a member of script object " + script_object );
      #endif
	  }

    else if ( label == hit_func_chunk )
    {
	    // read script object name
	    serial_in( *my_fs, &script_object );
	    hit_so = g_world_ptr->get_script_manager()->find_object( script_object );
      #if defined( TARGET_PC )
	    if ( !hit_so )
		    error( "script object type " + script_object + " not found" );
      #endif
	    // read event function name
	    serial_in( *my_fs, &script_func );
	    hit_func = hit_so->find_func( script_func + "(ranged_attack,projectile,num,entity)" );
      #if defined( TARGET_PC )
	    if ( hit_func == -1 )
		    error( script_func + " is not a member of script object " + script_object );
      #endif
    }

    else if ( label == fizzle_time_chunk )
    {
	    serial_in( *my_fs, &r );
	    fizzle_time = r;
	  }

  } while ( label != chunkend_label );

  delete my_fs;
  my_fs = NULL;
}


// this is to be executed by ranged_attack after creating a NEW projectile
void projectile::set_parent( ranged_attack* p )
{
  assert( !parent );  // only do this once
  parent = p;
	if ( prep_so )
  {
    // create preparation script object instance
    prep_inst = prep_so->find_instance( "ra_"+prep_so->get_name()+itos(parent->get_id()) );
    if ( !prep_inst )
      prep_inst = prep_so->add_instance( "ra_"+prep_so->get_name()+itos(parent->get_id()) );
    // create thread to execute preparation script
    prep_vm = prep_so->add_thread( prep_inst, prep_func );
    prep_vm->get_data_stack().push( (char*)&parent, 4 );
    projectile* q = this;
    prep_vm->get_data_stack().push( (char*)&q, 4 );
  }
  if ( launch_so )
  {
    // create launch script object instance
    launch_inst = launch_so->find_instance( "ra_"+launch_so->get_name()+itos(parent->get_id()) );
    if ( !launch_inst )
	    launch_inst = launch_so->add_instance( "ra_"+launch_so->get_name()+itos(parent->get_id()) );
  }
  if ( hit_so )
  {
    // create hit script object instance
    hit_inst = hit_so->find_instance( "ra_"+hit_so->get_name()+itos(parent->get_id()) );
    if ( !hit_inst )
	    hit_inst = hit_so->add_instance( "ra_"+hit_so->get_name()+itos(parent->get_id()) );
  }
}


///////////////////////////////////////
// Projectile Instancing
///////////////////////////////////////

entity* projectile::make_instance( const entity_id& _id,
                                   unsigned int _flags ) const
{
  projectile* newp = NEW projectile( _id, _flags );
  newp->copy_instance_data( *this );
  return (entity*)newp;
}

void projectile::copy_instance_data( const projectile& b )
{
  physical_entity::copy_instance_data( b );

  initialize_variables();

  // copy local data
  flags = b.flags;
  launch_velocity = b.launch_velocity;
  launch_angle = b.launch_angle;
  prep_so = b.prep_so;
  prep_func = b.prep_func;
  launch_so = b.launch_so;
  launch_func = b.launch_func;
  hit_so = b.hit_so;
  hit_func = b.hit_func;
  fizzle_time = b.fizzle_time;

  compute_colgeom();
}


///////////////////////////////////////
// Projectile Miscellaneous
///////////////////////////////////////


void projectile::launch(entity *s, entity *t)
{
	projectile *p;

	source = s;
	target = t;
//	if(prep_vm){
//		prep_inst->kill_thread(prep_vm->get_executable());
//		prep_vm = NULL;
//	}
	if(launch_func >= 0)
  {
		// create script object instance
		launch_vm = launch_so->add_thread(launch_inst, launch_func);
		launch_vm->get_data_stack().push((char *)&parent, 4);
		p = this;
		launch_vm->get_data_stack().push((char *)&p, 4);
		launch_vm->get_data_stack().push((char *)&source, 4);
		launch_vm->get_data_stack().push((char *)&target, 4);
	}
	set_launch_vector();
	orient();
	in_flight = true;
  total_life = 0.0F;
	//fizzle_time += game_clock::inst()->get_time();
}

static bool capsule_collide_with_camera_collision( collision_capsule *cap, vector3d &v, vector3d &n, vector3d &pi )
{
  vector3d pt1 = cap->get_abs_capsule().base;
  vector3d pt2 = cap->get_abs_capsule().end;

  vector3d vec = pt2 - pt1;

  if(vec == ZEROVEC)
    vec = v;

  pt1 -= v;

  vec.normalize();
  vec *= cap->get_abs_capsule().radius;
  pt2 += vec;
  pt1 -= vec;

  region_node *hit_region = NULL;
  entity *hit_entity = NULL;

  region_node* start_region = NULL;
  sector *sect = g_world_ptr->get_the_terrain().find_sector(pt1);

  if ( sect && (start_region = sect->get_region()) != NULL )
  {
    return find_intersection( pt1, pt2,
                              start_region,
                              FI_COLLIDE_WORLD|FI_COLLIDE_CAMERA,
                              &pi, &n,
                              &hit_region, &hit_entity);
  }
  return false;
}

void projectile::frame_advance(time_value_t t)
{
  if ( is_active() )
  {
	  projectile *p;
	  rational_t  l, r;

	  if(in_flight)
    {
      total_life += t;
		  // Physical Representation
		  if(using_velocity())
      {
			  velocity += acceleration_factor;
			  vector3d posn;
			  vector3d v;
			  vector3d pi, n, nu, nv, vp;
			  po new_po;
			  bool valid = false;

				vector3d vel;
				get_velocity(&vel);
			  v = (vel - get_acceleration_correction_factor()) * t;

			  if((flags & PROJECTILE_Seek) && target /*! && ((target->get_flavor() != ENTITY_CHARACTER) || ((character *)target)->is_alive()) !*/)
        {
				  vector3d diff = target->get_abs_position() - get_abs_position();
				  diff.normalize();
				  diff *= launch_velocity * t;
  //				diff += cross(target->get_abs_position() - get_abs_position(), v) * 0.25f;
				  v += diff;
				  l = v.length();
				  if(l > (launch_velocity * t))
          {
					  v *= (launch_velocity * t) / l;
				  }
			  }

			  if(colgeom)
        {
				  assert(colgeom->get_type() == collision_geometry::CAPSULE);
				  collision_capsule *cap = (collision_capsule *)colgeom;

				  // get the current po
				  new_po = get_abs_po();
				  // update the position component according to this trial
				  posn = get_abs_position();
				  posn += v;
				  new_po.set_position(posn);

				  cap->xform(new_po);
				  if(!g_world_ptr->get_the_terrain().in_world(cap->get_abs_capsule().base, cap->get_abs_capsule().radius, v, n, pi) ||
					   !g_world_ptr->get_the_terrain().in_world(cap->get_abs_capsule().end, cap->get_abs_capsule().radius, v, n, pi) ||
             capsule_collide_with_camera_collision(cap, v, n, pi))
          {
					  hit = HIT_Terrain;
					  hit_entity = NULL;
				  }
				  else
          {
					  set_velocity((v / t) + get_acceleration_correction_factor());
					  // update the angular component if needed
						po ang_vel_rot;
						vector3d angvel;
						get_angular_velocity(&angvel);
						ang_vel_rot.set_rot(angvel * t);
						new_po.add_increment(&ang_vel_rot);
						new_po.fixup();
		        old_position = get_abs_position();
					  set_rel_po(new_po);
				  }
			  }
		  }
		  else
      {
			  set_velocity(ZEROVEC);
			  set_angular_velocity(ZEROVEC);
		  }

		  set_last_acceleration_correction_factor(get_acceleration_correction_factor());
		  set_acceleration_correction_factor(ZEROVEC);
		  acceleration_factor = ZEROVEC;

		  compute_dynamic_properties();
		  orient();
		  collide(t);
		  if(!hit && (total_life >= fizzle_time))
      {
			  hit = HIT_Fizzle;
			  hit_entity = NULL;
		  }
		  if(hit)
      {
			  in_flight = false;

        /*
			  if(prep_vm)
        {
				  prep_inst->kill_thread(prep_vm->get_executable());
				  prep_vm = NULL;
			  }

			  if(launch_vm)
        {
				  launch_inst->kill_thread(launch_vm->get_executable());
				  launch_vm = NULL;
			  }
        */
			  if(hit_func >= 0)
        {
				  // create script thread
				  hit_vm = launch_so->add_thread(launch_inst, hit_func);
				  hit_vm->get_data_stack().push((char *)&parent, 4);
				  p = this;
				  hit_vm->get_data_stack().push((char *)&p, 4);
				  r = (rational_t)hit;
				  hit_vm->get_data_stack().push((char *)&r, 4);
				  hit_vm->get_data_stack().push((char *)&hit_entity, 4);
			  }
		  }
	  }
	  else
    {
		  set_velocity(ZEROVEC);
		  set_angular_velocity(ZEROVEC);
		  set_acceleration_factor(ZEROVEC);
	  }
  }
}

bool projectile::set_launch_vector()
{
	bool      failed = true;
	vector3d  v, w;
	po        m;

	if((flags & PROJECTILE_AutoTarget) && target)
  {
		if(get_mass() != 0.0f)
    {
			vector3d   rel_targ = target->get_abs_position() - get_abs_position();
			vector2d   P((rel_targ - YVEC * rel_targ.y).length(), rel_targ.y);
			rational_t Vh, Vy;
			rational_t r = launch_velocity;

			// perform the computation
			if(__fabs(P.x) > SMALL_DIST)
      {
				rational_t norm2P = P.x * P.x + P.y * P.y;
				rational_t discriminant = (-GRAVITY * P.y + r * r) * (-GRAVITY * P.y + r * r) - GRAVITY * GRAVITY * norm2P;
				if(discriminant > 0)
        {
					failed = false;
					Vh = P.x * __fsqrt(((-GRAVITY * P.y + r * r) + __fsqrt(discriminant)) / (2.0f * norm2P));
					Vy = (Vh * Vh * P.y + 0.5f * GRAVITY * P.x * P.x) / (Vh * P.x); // = +/- (__fsqrt(r * r - Vh * Vh));
				}
				else
        {
					Vh=1;
					Vy=0;
				}
			}
			else
      {
				Vh = 1;
				Vy = 0;
			}

			v = (rel_targ - YVEC * rel_targ.y).set_length(Vh);
			v.y = Vy;
		}
		else
    {
			v = target->get_abs_position() - get_abs_position();
			v.normalize();
			v *= launch_velocity;
		}
	}
	else
  {
		v = source->get_abs_po().get_facing();
		w = cross(YVEC, v);
		if(w.length2() == 0.0f)
    {
			w = v;
			v *= launch_velocity;
		}
		else
    {
			w.normalize();
			m.set_rot(w, launch_angle * PI / 180.0f);
			v = m.non_affine_slow_xform(v);
			v *= launch_velocity;
		}
	}
	set_velocity(v);

	return failed;
}

void projectile::orient()
{
	vector3d vel;
	get_velocity(&vel);
	vector3d zvec = vel;
	rational_t n = zvec.length();

	if(n < SMALL_DIST)
		zvec = ZVEC;
	else
		zvec /= n;

	vector3d xvec = cross(zvec, YVEC);
	n = xvec.length();
	if(n < SMALL_DIST)
		xvec = XVEC;
	else
		xvec /= n;

	vector3d yvec = cross(zvec, xvec);

	po arrow_po(xvec, yvec, zvec, get_abs_position());
	set_rel_po(arrow_po);
}

void projectile::collide(time_value_t t)
{
	capsule cap(old_position, get_abs_position(), 0.125f);
	entity* pe;

  world_dynamics_system::entity_list::const_iterator ei = g_world_ptr->get_entities().begin();
  world_dynamics_system::entity_list::const_iterator ei_end = g_world_ptr->get_entities().end();
  for ( ; ei!=ei_end; ++ei )
  {
    pe = *ei;
		if ( pe )
    {
      bool fail = false;
      entity* e = source;

      while ((e = (entity *)e->link_ifc()->get_parent())!=NULL && !fail)
      {
        if (e==pe)
          fail=true;
      }

			// CTT 03/23/00: TODO: eliminate or update projectile class for Max Steel
      if(pe->is_active() && !fail && (pe != source) && (pe != this) /*&& pe->is_affected_by_weapons()*/)
      {
				collision_geometry *cg = (collision_geometry *)pe->get_updated_damage_capsule();
        rational_t dummy_dist;

				if(cg && (cg->get_type() == collision_geometry::CAPSULE) && collide_capsule_capsule(cap, ((collision_capsule *)cg)->get_abs_capsule(), &hit_list, &normal_list1, &normal_list2, dummy_dist, 0, vector3d(0.0f, 0.0f, 0.0f)))
        {
					set_rel_position(hit_list[0]);

/*!          if(pe->get_flavor() == ENTITY_CHARACTER)
          {
						hit = HIT_Character;
						hit_entity = pe;
					}
					else
!*/
          {
						hit = HIT_Terrain;
						hit_entity = pe;
					}

					hit_list.clear();
					normal_list1.clear();
					normal_list2.clear();
					return;
				}
			}
		}
	}
}

#define IS_ENEMY(a, b) ((a) != (b) && ((character *)a)->get_soft_attrib()->get_team() != ((character *)b)->get_soft_attrib()->get_team() && ((character *)b)->get_soft_attrib()->get_team() != character_soft_attributes::NEUTRAL_TEAM)

void sphere_target::invoke(entity *invoker, vector3d &p)
{
//!  entity *invoking_char;

//!	int        i;
//!	character *c;
	entity    *pe;
	rational_t d2;

STUBBED(sphere_target_invoke, "sphere_target::invoke");
    switch(invoker->get_flavor())
	{
/*!		case ENTITY_CHARACTER:
			invoking_char = invoker;
			break;
!*/
/*!		case ENTITY_LIMB_BODY:
            invoking_char = invoker->get_parent();
            while(invoking_char != NULL && invoking_char->get_flavor() != ENTITY_CHARACTER)
                invoking_char = invoking_char->get_parent();

            assert(invoking_char != NULL && invoking_char->get_flavor() == ENTITY_CHARACTER);
			break;
!*/
		default:
			assert(0);
			break;
	}

	range_list.erase(range_list.begin(), range_list.end());
	if(flags & (TARGET_Myself | TARGET_Friends | TARGET_Enemies))
  {
/*!		for(i = 0; i < g_world_ptr->get_num_characters(); i++)
    {
			c = g_world_ptr->get_character(i);
			if ( c->is_alive() )
      {
				if(c == invoking_char)
        {
					if(flags & TARGET_Myself)
          {
						d2 = (c->get_abs_position() - p).length2();
						if(d2 <= radius2)
							range_list.push_back(range_t(c, __fsqrt(d2)));
					}
				}
				else if(IS_ENEMY(invoking_char, c))
        {
					if(flags & TARGET_Enemies)
          {
						d2 = (c->get_abs_position() - p).length2();
						if(d2 <= radius2)
							range_list.push_back(range_t(c, __fsqrt(d2)));
					}
				}
				else
        {
					if(flags & TARGET_Friends)
          {
						d2 = (c->get_abs_position() - p).length2();
						if(d2 <= radius2)
							range_list.push_back(range_t(c, __fsqrt(d2)));
					}
				}
			}
		}
!*/
	}

	if ( flags & TARGET_Entities )
  {
    world_dynamics_system::entity_list::const_iterator ei = g_world_ptr->get_entities().begin();
    world_dynamics_system::entity_list::const_iterator ei_end = g_world_ptr->get_entities().end();
    for ( ; ei!=ei_end; ++ei )
    {
      pe = *ei;
		  if ( pe/*! && pe->get_flavor()!=ENTITY_CHARACTER !*/ )
      {
				d2 = (pe->get_abs_position() - p).length2();
				if(d2 <= radius2)
					range_list.push_back(range_t(pe, __fsqrt(d2)));
			}
		}
	}
}

void xzcylinder_target::invoke(entity *invoker, vector3d &p)
{
	range_list.erase(range_list.begin(), range_list.end());
}

void cone_target::invoke(entity *invoker, vector3d &p)
{
STUBBED(cone_target_invoke, "cone_target::invoke");
/*!
	int        i;
	character *c;
	entity    *pe;
	vector3d   v, w;
	rational_t d, d2;

	range_list.erase(range_list.begin(), range_list.end());
	if(flags & (TARGET_Myself | TARGET_Friends | TARGET_Enemies))
  {
		for(i = 0; i < g_world_ptr->get_num_characters(); ++i)
    {
			c = g_world_ptr->get_character(i);
			if ( c->is_alive() )
      {
				if(c == invoker)
        {
					if(flags & TARGET_Myself)
          {
						v = c->get_abs_position() - p;
						w = v;
						w.normalize();
						d = dot(w, facing);
						d2 = v.length2();
						if((d >= 0.0f) && (d >= cos_half_angle) && (d2 <= distance2))
							range_list.push_back(range_t(c, __fsqrt(d2)));
					}
				}
				else if(IS_ENEMY(invoker, c))
        {
					if(flags & TARGET_Enemies)
          {
						v = c->get_abs_position() - p;
						w = v;
						w.normalize();
						d = dot(w, facing);
						d2 = v.length2();
						if((d >= 0.0f) && (d >= cos_half_angle) && (d2 <= distance2))
							range_list.push_back(range_t(c, __fsqrt(d2)));
					}
				}
				else
        {
					if(flags & TARGET_Friends)
          {
						v = c->get_abs_position() - p;
						w = v;
						w.normalize();
						d = dot(w, facing);
						d2 = v.length2();
						if((d >= 0.0f) && (d >= cos_half_angle) && (d2 <= distance2))
							range_list.push_back(range_t(c, __fsqrt(d2)));
					}
				}
			}
		}

	}

	if ( flags & TARGET_Entities )
  {
    world_dynamics_system::entity_list::const_iterator ei = g_world_ptr->get_entities().begin();
    world_dynamics_system::entity_list::const_iterator ei_end = g_world_ptr->get_entities().end();
    for ( ; ei!=ei_end; ++ei )
    {
      pe = *ei;
      if ( pe && pe->get_flavor()!=ENTITY_CHARACTER)
      {
				v = c->get_abs_position() - p;
				w = v;
				w.normalize();
				d = dot(w, facing);
				d2 = v.length2();
				if((d >= 0.0f) && (d >= cos_half_angle) && (d2 <= distance2))
					range_list.push_back(range_t(c, __fsqrt(d2)));
			}
		}
	}
!*/
}

#endif
