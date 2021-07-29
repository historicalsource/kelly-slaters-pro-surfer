////////////////////////////////////////////////////////////////////////////////
/*
  limb.cpp

  Home of 'limb' class.  Limbs are nodes in the animation hierarchy for the bodies of an actor
  Also home of 'hinge', a simple degree of freedom between limbs.

*/
////////////////////////////////////////////////////////////////////////////////
#include "global.h"

#include "osalloc.h"

//!#include "character.h"
//!#include "limb.h"
#include "hinge.h"
#include "wds.h"
//!#include "actor.h"
#include "oserrmsg.h"
#include "actuator.h"
#include "colmesh.h"
#include "capsule.h"
#include "hinge.h"
#include "actuator.h"
#include "pmesh.h"
#include "stringx.h"
#include "profiler.h"
#include "hwmath.h"

////////////////////////////////////////////////////////////////////////////////
// globals
////////////////////////////////////////////////////////////////////////////////
instance_bank<limb_body_meat> limb_body_meat_bank;


////////////////////////////////////////////////////////////////////////////////
//    limb
////////////////////////////////////////////////////////////////////////////////

limb::limb( limb_body* _lb, actor* _my_actor, limb* _parent )
  : lb( _lb ),
    my_actor( _my_actor ),
    parent( _parent ),
    save_colmesh( false )
{
  if ( parent )
    lb->set_parent( parent->get_body() );
  else
    lb->set_parent( my_actor );
  lb->set_my_limb(this);
}

/*
int iasdf;

void recursive_function( vector<int> &v )
{
	iasdf = v[0];

	recursive_function( v );
}

bool foo(branch b, const vector3d &p, const rational_t r, const vector3d &v, vector3d &n, vector3d &pi, plane_list_t &plane_list, plane_list_t &show_plane_list, hit_list_t &hit_list, hit_dir_list_t & hit_dir_list)
{
}
*/

#pragma todo("Is this function obsolete?  jdf 3-21-01")
// Old Format
limb::limb( chunk_file& fs, actor* _my_actor, limb* _parent )
{
  stringx chunk_name,cmesh_fname;
  int numchildren=0;
  int actuator_hinge = 0;
  int linear_actuator_hinge = 0;

//  recursive_function(vector<int>());

  my_actor = _my_actor;
  parent = _parent;
  save_colmesh = false;
  serial_in(fs,&cmesh_fname);

  stringx cmesh_pathname = fs.get_dir() + cmesh_fname;
  int bidx = g_world_ptr->add_body( this, cmesh_pathname, ((vr_pmesh*)_my_actor->get_vrep()) );
  lb = g_world_ptr->get_body(bidx );
  my_actor->add_body(lb);
  if (parent)
    lb->set_parent(parent->get_body());
  else
    lb->set_parent(my_actor);

  lb->set_flag(EFLAG_MISC_MEMBER_OF_VARIANT, true);
  bool done = false;
  while (!done)
  {
    serial_in(fs,&chunk_name);
    if (chunk_name==chunkend_label)
      done = true;
    else if (chunk_name=="numchildren")
    {
      serial_in(fs,&numchildren);
    }
    else if (chunk_name=="relpos")
    {
      // obsolete !!!!
      vector3d pos;
      serial_in(fs,&pos);
      lb->set_rel_position(pos);
    }
    else if (chunk_name=="id")
    {
      // Soon this will be obsolete, but kludge it in for now
      stringx id_name;
      serial_in(fs,&id_name);
      lb->set_anim_id( id_name );
    }
    else if (chunk_name=="hinge")
    {
      vector3d axis;
      rational_t min,max;
      serial_in(fs,&axis);
      serial_in(fs,&min);
      serial_in(fs,&max);
      hinge * h = NEW hinge(axis,min,max,this);
      hinges.push_back(h);
    }
    else if (chunk_name=="rest")
    {
      rational_t rest;
      serial_in(fs,&rest);
      hinges[hinges.size()-1]->set_rest(rest);
    }
    else if (chunk_name=="tension")
    {
      rational_t tension;
      serial_in(fs,&tension);
      hinges[hinges.size()-1]->set_tension(tension);
    }
    else if (chunk_name=="bounded")
    {
      hinges[hinges.size()-1]->set_bounded(true);
    }
    else if (chunk_name=="actuator")
    {
      rational_t k,drag;
      serial_in(fs,&k);
      serial_in(fs,&drag);
      get_hinge(actuator_hinge++)->add_actuator(k, drag, my_actor->get_strike_scale());
    }
    else if (chunk_name=="l_hinge")
    {
      vector3d axis;
      rational_t min,max;
      serial_in(fs,&axis);
      serial_in(fs,&min);
      serial_in(fs,&max);
      hinge * h = NEW hinge(axis,min,max,this);
      linear_hinges.push_back(h);
    }
    else if (chunk_name=="l_rest")
    {
      rational_t rest;
      serial_in(fs,&rest);
      linear_hinges[linear_hinges.size()-1]->set_rest(rest);
    }
    else if (chunk_name=="l_tension")
    {
      rational_t tension;
      serial_in(fs,&tension);
      linear_hinges[linear_hinges.size()-1]->set_tension(tension);
    }
    else if (chunk_name=="l_bounded")
    {
      linear_hinges[linear_hinges.size()-1]->set_bounded(true);
    }
    else if (chunk_name=="l_actuator")
    {
      rational_t k,drag;
      serial_in(fs,&k);
      serial_in(fs,&drag);
      get_linear_hinge(linear_actuator_hinge++)->add_actuator(k, drag, my_actor->get_strike_scale());
    }
    else if (chunk_name=="shockdamper")
    {
      rational_t damper;
      serial_in(fs,&damper);
      lb->set_shock_damper(damper);
    }
    else if (chunk_name=="l_shockdamper")
    {
      rational_t damper;
      serial_in(fs,&damper);
      lb->set_l_shock_damper(damper);
    }
    else if (chunk_name=="h_shockdamper")
    {
      rational_t damper;
      serial_in(fs,&damper);
      lb->set_h_shock_damper(damper);
    }
    else if (chunk_name=="freebody")
    {
      rational_t freedom;
      serial_in(fs,&freedom);
      lb->set_free_body(freedom);
    }
    else if (chunk_name=="invis")
    {
      lb->set_visible(false);
      lb->set_flag(EFLAG_MISC_MEMBER_OF_VARIANT, false);
    }
    else if (chunk_name=="inactive")
    {
      lb->set_active(false);
    }
    else if (chunk_name=="force_light")
    {
      if( lb->get_vrep() )
	      lb->get_vrep()->set_light_method( LIGHT_METHOD_DIFFUSE );
    }
    else
    {
      stringx composite = stringx("Invalid chunk_name: ") + chunk_name + stringx(" in file ")+
                          fs.get_name();
      error(composite.c_str());
    }
  }
  for (int i=0;i<numchildren;++i)
  {
    limb * lptr = NEW limb(fs,my_actor,this);
    children.push_back(lptr);
  }
}

limb::~limb()
{
  int i;
  for (i = 0;i<children.size();++i)
    delete children[i];
  for (i = 0;i<hinges.size();++i)
    delete hinges[i];
  for (i = 0;i<linear_hinges.size();++i)
    delete linear_hinges[i];
}

const stringx numchildren_chunk( "numchildren" );
const stringx hinge_chunk( "hinge" );
const stringx rest_chunk( "rest" );
const stringx tension_chunk( "tension" );
const stringx bounded_chunk( "bounded" );
const stringx actuator_chunk( "actuator" );
const stringx l_hinge_chunk( "l_hinge" );
const stringx l_rest_chunk( "l_rest" );
const stringx l_tension_chunk( "l_tension" );
const stringx l_bounded_chunk( "l_bounded" );
const stringx l_actuator_chunk( "l_actuator" );
const stringx shockdamper_chunk( "shockdamper" );
const stringx l_shockdamper_chunk( "l_shockdamper" );
const stringx h_shockdamper_chunk( "h_shockdamper" );
const stringx freebody_chunk("freebody");
const stringx speed_chunk( "speed" );
const stringx regen_chunk( "regen" );
const stringx mass_chunk( "mass" );
const stringx range_chunk( "range" );
const stringx rngideal_chunk( "rngideal" );
const stringx rngvar_chunk( "rngvar" );
const stringx rngmin_chunk( "rngmin" );
const stringx collision_chunk( "collision" );
const stringx mesh_chunk( "mesh" );
const stringx plane_chunk( "plane" );
const stringx sphere_chunk( "sphere" );
const stringx invis_chunk( "invis" );
const stringx inactive_chunk( "inactive" );
const stringx force_light_chunk( "force_light" );
const stringx min_detail_chunk( "min_detail" );


void limb::read_enx_data( chunk_file& fs )
{
  stringx chunk_name;
  int numchildren = 0;
  int actuator_hinge = 0;
  int linear_actuator_hinge = 0;
  rational_t damage_factor=0, attack_speed=0, regen_time=0, stun_factor = 1.0f;
  rational_t ideal_range=0, range_variance=0, hth_range=0;
  rational_t mass = -1;
  bool is_ranged = false;

  // This hack fixes leg and waist defaults.  Hopefully won't screw us.
  anim_id_t w = anim_id_manager::inst()->find_id( "WAIST" );
  if (lb->get_anim_id()==w)
  {
    lb->set_shock_damper(0);
  }
  stringx id_name = anim_id_manager::inst()->get_label(lb->get_anim_id());
  if (id_name.find("LEG")!=-1 || id_name.find("FOOT")!=-1 || id_name.find("TOE")!=-1)
  {
    lb->set_shock_damper(0);
  }

  lb->set_flag( EFLAG_MISC_MEMBER_OF_VARIANT, true );

  // read hinges, actuators, etc.

  serial_in( fs, &chunk_name );
  for ( ; chunk_name!=chunkend_label; serial_in(fs,&chunk_name) )
  {
    if ( chunk_name == numchildren_chunk )
      serial_in( fs, &numchildren );
    else if ( chunk_name == hinge_chunk )
    {
      vector3d axis;
      rational_t min,max;
      serial_in( fs, &axis );
      serial_in( fs, &min );
      serial_in( fs, &max );
      hinge* h = NEW hinge(axis,min,max,this);
      hinges.push_back( h );
    }
    else if ( chunk_name == rest_chunk )
    {
      rational_t rest;
      serial_in( fs, &rest );
      hinges[hinges.size()-1]->set_rest( rest );
    }
    else if ( chunk_name == tension_chunk )
    {
      rational_t tension;
      serial_in( fs, &tension );
      hinges[hinges.size()-1]->set_tension( tension );
    }
    else if ( chunk_name == bounded_chunk )
    {
      hinges[hinges.size()-1]->set_bounded( true );
    }
    else if ( chunk_name == actuator_chunk )
    {
      rational_t k, drag;
      serial_in( fs, &k );
      serial_in( fs, &drag );
      get_hinge( actuator_hinge++ )->add_actuator( k, drag, my_actor->get_strike_scale() );
    }
    else if ( chunk_name == l_hinge_chunk )
    {
      vector3d axis;
      rational_t min, max;
      serial_in( fs, &axis );
      serial_in( fs, &min );
      serial_in( fs, &max );
      hinge* h = NEW hinge(axis,min,max,this);
      linear_hinges.push_back( h );
    }
    else if ( chunk_name == l_rest_chunk )
    {
      rational_t rest;
      serial_in( fs, &rest );
      linear_hinges[linear_hinges.size()-1]->set_rest( rest );
    }
    else if ( chunk_name == l_tension_chunk )
    {
      rational_t tension;
      serial_in( fs, &tension );
      linear_hinges[linear_hinges.size()-1]->set_tension( tension );
    }
    else if ( chunk_name == l_bounded_chunk )
    {
      linear_hinges[linear_hinges.size()-1]->set_bounded( true );
    }
    else if ( chunk_name == l_actuator_chunk )
    {
      rational_t k, drag;
      serial_in( fs, &k );
      serial_in( fs, &drag );
      get_linear_hinge( linear_actuator_hinge++ )->add_actuator( k, drag, my_actor->get_strike_scale() );
    }
    else if ( chunk_name == shockdamper_chunk )
    {
      rational_t damper;
      serial_in(fs,&damper);
      lb->set_shock_damper(damper);
    }
    else if ( chunk_name == l_shockdamper_chunk )
    {
      rational_t damper;
      serial_in(fs,&damper);
      lb->set_l_shock_damper(damper);
    }
    else if ( chunk_name == h_shockdamper_chunk )
    {
      rational_t damper;
      serial_in(fs,&damper);
      lb->set_h_shock_damper(damper);
    }
    else if ( chunk_name == freebody_chunk )
    {
      rational_t freedom;
      serial_in(fs,&freedom);
      lb->set_free_body(freedom);
    }
    else if ( chunk_name == invis_chunk )
    {
      lb->set_flag( EFLAG_MISC_MEMBER_OF_VARIANT, false );
      lb->set_visible( false );
    }
    else if ( chunk_name == inactive_chunk )
    {
      lb->set_active( false );
    }
    else if (chunk_name==force_light_chunk)
    {
      if( lb->get_vrep() )
	      lb->get_vrep()->set_light_method( LIGHT_METHOD_DIFFUSE );
    }
    else if (chunk_name==min_detail_chunk)
    {
      int mindetaillvl;
      serial_in( fs, &mindetaillvl );
      lb->set_min_detail(mindetaillvl);
    }
#if defined(TARGET_PC)
    else
      error( fs.get_name() + ": unexpected chunk '" + chunk_name + "' in hinges" );
#endif
  }

  // read children
  for ( ; numchildren; --numchildren )
  {
    serial_in( fs, &chunk_name );
    // find limb or nonlimb matching given ID
    anim_id_t aid = anim_id_manager::inst()->find_id( chunk_name );
    if ( aid == NO_ID )
      error( fs.get_name() + ": anim_id '" + chunk_name + "' not found" );
    else if ( my_actor->limb_valid(aid) )
    {
      limb* child = my_actor->limb_ptr( aid );
      if ( find( children.begin(), children.end(), child ) == children.end() )
      {
        error( fs.get_name() + ": limb '" + chunk_name + "' is not a child of limb '" +
          to_string(get_id()) + "'" + ", but of " + to_string(child->get_parent()->get_id()));
      }
      // read extended limb data
      child->read_enx_data( fs );
    }
    else if ( my_actor->nonlimb_ptr(aid) )
    {
      entity* child = my_actor->nonlimb_ptr( aid );
      if ( !lb->has_children() || find(lb->get_children().begin(),lb->get_children().end(),child)==lb->get_children().end() )
      {
        error( fs.get_name() + ": non-limb '" + chunk_name + "' is not a child of limb '" +
               to_string(get_id()) + "'" + ", but of " + to_string(child->get_parent()->get_anim_id()) );
      }
      // read extended nonlimb data
      my_actor->read_extended_nonlimb_data( fs, child );
    }
    else
      error( fs.get_name() + ": anim_id '" + chunk_name + "' not found in actor" );
  }
}


// copy limb tree
limb* limb::make_instance( actor* act, limb* new_parent )
{
  entity_id newid;
  if ( !(lb->get_id() == entity_id()) )
  {
    // construct NEW entity id
    const stringx& nodename = lb->get_id().get_val();
    int subn = nodename.rfind( '.' );
    if ( subn != nodename.npos )
      newid = entity_id( (act->get_id().get_val() + nodename.substr( subn, nodename.length()-subn )).c_str() );
  }
  // make instance of limb entity and add to world and actor
  limb_body* newlb = static_cast<limb_body*>( lb->make_instance( this, newid, lb->get_flags() ) );
  g_world_ptr->add_body( newlb );
  act->add_body( newlb );
  // make limb
  limb* newlimb = NEW limb( newlb, act, new_parent );
  if ( new_parent )
    newlb->set_parent( new_parent->get_body() );
  else
    newlb->set_parent( act );
  newlb->set_rel_position( base_relpos );
  newlimb->base_relpos = base_relpos;
  newlimb->save_colmesh = save_colmesh;
  // make instances of all children, in order
  int i;
  newlimb->children.reserve( children.size() );
  for ( i=0; i<children.size(); ++i )
    newlimb->children.push_back( children[i]->make_instance( act, newlimb ) );
  // make instances of all hinges, in order
  newlimb->hinges.reserve( hinges.size() );
  for ( i=0; i<hinges.size(); ++i )
    newlimb->hinges.push_back( hinges[i]->make_instance() );
  newlimb->linear_hinges.reserve( linear_hinges.size() );
  for ( i=0; i<linear_hinges.size(); ++i )
    newlimb->linear_hinges.push_back( linear_hinges[i]->make_instance() );
  // return NEW instance of limb (tree)
  return newlimb;
}


void limb::advance_hinges(time_value_t t)
{
  vector<hinge *>::iterator h;
  for (h=hinges.begin();h<hinges.end();++h)
    if ((*h)->is_active())
      (*h)->advance(t);
  for (h=linear_hinges.begin();h<linear_hinges.end();++h)
    if ((*h)->is_active())
      (*h)->advance(t);

  vector<limb * >::iterator lptr;
  for (lptr=children.begin();lptr<children.end();++lptr)
    (*lptr)->advance_hinges(t);
}


void limb::phys_render(time_value_t t, unsigned int recurse_depth, bool shadow)
{
  limb_body *body = get_body();

  po the_po = po_identity_matrix;
  vector3d pos_offset = ZEROVEC;
  vector<hinge *>::iterator h;
  po temppo;

  // hinges are composed in order from innermost to outermost in the joint.
  for (h = hinges.begin();h<hinges.end();++h)
  {
#if defined(TARGET_MKS)
    asm_po_mul( &(*h)->build_po(&temppo, shadow), &the_po, &the_po );
#else
    the_po = (*h)->build_po(&temppo, shadow)*the_po;
#endif
//    the_po = (*h)->build_po(&temppo, shadow)*the_po;
  }
  // Then the linear joints are added.  Their order does not matter, only that they come after
  // the angular hinges
  for (h = linear_hinges.begin();h<linear_hinges.end();h++)
  {
    pos_offset += (*h)->build_position();
  }

  the_po.set_rel_position(base_relpos+pos_offset);
  if (!shadow)
    body->frame_advance(t);
  body->set_rel_po(the_po); //orientation(the_po);

  vector<limb * >::iterator li;
  if (recurse_depth>0)
    for (li=children.begin();li<children.end();++li)
      (*li)->phys_render(t, recurse_depth-1, shadow);

}


void limb::place_self_in_map( vector<limb*>& limb_map )
{
  int id = lb->get_anim_id();
  if (id!=NO_ID)
  {
    assert( id<limb_map.size() );
    limb_map[ id ] = this;
  }
  vector<limb*>::iterator li;
  for ( li=children.begin(); li<children.end(); ++li )
    (*li)->place_self_in_map(limb_map);
}


rational_t limb::get_thickness_below_origin() const
{
  return -lb->get_min_extent().y;
}


void limb::recompute_relpos_from_pivot()
{
  if (parent && parent->get_body()->get_colgeom()->is_pivot_valid() && get_body()->get_colgeom()->is_pivot_valid())
  {
    vector3d newpos = get_body()->get_colgeom()->get_pivot() - parent->get_body()->get_colgeom()->get_pivot();
    get_body()->set_rel_position(newpos);
    base_relpos = newpos;
  }
  else
    base_relpos = ZEROVEC;
  vector<limb * >::iterator li;
  for (li=children.begin();li<children.end();li++)
    (*li)->recompute_relpos_from_pivot();
}


void limb::shock( const vector3d& accel, const vector3d& internal_accel, rational_t time_inc, bool ignore_damping, bool impulse )
{
  vector3d eff_accel;

  if (lb->get_free_body())
  {
    eff_accel = accel+internal_accel;
  }
  else
  {
    eff_accel = accel-internal_accel;
  }

/*
  // This is continuous version of the above...
  eff_accel = accel + (lb->get_free_body()*-0.5f)*internal_accel;
*/

  // A hack for hanging from the dragon's mouth.
  if ( !linear_hinges.empty() )
    local_linear_shock(eff_accel, time_inc, ignore_damping, impulse);
  if (!hinges.empty())
    local_angular_shock(eff_accel, time_inc, ignore_damping, impulse);
  vector<limb * >::iterator li;
//  if (/*lb->get_shock_damper()!=0 || */ignore_damping)
//    {
    //rational_t damp = (ignore_damping)?1:lb->get_shock_damper();
    for (li=children.begin();li<children.end();++li)
      (*li)->shock(accel, internal_accel, time_inc, ignore_damping, impulse);
//    }
}


void limb::local_linear_shock( const vector3d& accel, rational_t time_inc, bool ignore_damping, bool impulse )
{
  if (linear_hinges.end()!=linear_hinges.begin())
  {
//    vector3d rel_accel = lb->get_abs_po().non_affine_inverse_xform(accel);
    vector3d rel_accel = lb->get_abs_po().fast_non_affine_inverse_xform(accel);

    vector3d rel_accel_inc = rel_accel*time_inc;

    vector<hinge *>::iterator hit;
    for (hit=linear_hinges.begin();hit!=linear_hinges.end();++hit)
    {
      hinge * h = (*hit);

      // only shock the resilient joints
      if (h->get_actuator())
        {
        rational_t hinge_accel = dot(h->get_axis(),rel_accel_inc);
        rational_t damp = (ignore_damping)?1:(lb->get_shock_damper()*lb->get_l_shock_damper());
        h->adjust_vel(hinge_accel*damp, impulse);
//        h->adjust_vel((ignore_damping)?hinge_accel:(hinge_accel*(lb->get_shock_damper()*lb->get_l_shock_damper())), impulse);
      }
    }
  }
}


void limb::local_angular_shock( const vector3d& accel, rational_t time_inc, bool ignore_damping, bool impulse )
{
  if (hinges.end()!=hinges.begin())
  {

//    vector3d rel_accel = lb->get_abs_po().non_affine_inverse_xform(accel);
    vector3d rel_accel = lb->get_abs_po().fast_non_affine_inverse_xform(accel);

    vector3d rel_accel_inc = rel_accel*time_inc;

    vector<hinge *>::iterator hit;
    for (hit=hinges.begin();hit!=hinges.end();++hit)
    {
      hinge * h = (*hit);

      // only shock the resilient joints
      if (h->get_actuator())
      {
        vector3d com;
        lb->get_c_o_m(&com);
        com.normalize();

        rational_t damp = (ignore_damping)?1:(lb->get_shock_damper()*lb->get_h_shock_damper());
        rational_t hinge_accel = dot(cross(com,h->get_axis()),rel_accel_inc);
        h->adjust_vel(hinge_accel*damp, impulse);
//        h->adjust_vel((ignore_damping)?hinge_accel:(hinge_accel*(lb->get_shock_damper()*lb->get_h_shock_damper())), impulse);
      }
    }
  }
}


// Only considers physicslly active limbs.  presumes the root of the search is an active limb.
limb_dist_t limb::find_closest_limb(vector3d p)
{
  vector<limb *>::iterator li;
  limb_dist_t              n;
  limb_dist_t              c;

  assert(lb->is_active());
  n.first = this;
  n.second = (p - get_abs_position()).length2();
  for(li = children.begin(); li != children.end(); ++li)
  {
    if ((*li)->get_body()->is_active())
    {
      c = (*li)->find_closest_limb(p);
      if(c.second < n.second){
        n = c;
      }
    }
  }

  return n;
}


void limb::delete_colgeom()
{
  vector<limb *>::iterator i;
  if (!save_colmesh)
    get_body()->delete_colgeom();
  for(i = children.begin();  i != children.end(); ++i)
    (*i)->delete_colgeom();
}


void limb::set_actuator_strike_scales(rational_t ss)
{
  hinge_actuator * a;
  int i;
  for (i = 0;i<hinges.size();++i)
    if ((a=hinges[i]->get_actuator())!=NULL)
      a->set_strike_scale(ss);
  for (i = 0;i<linear_hinges.size();++i)
    if ((a=linear_hinges[i]->get_actuator())!=NULL)
      a->set_strike_scale(ss);
  vector<limb *>::iterator ch;
  for(ch = children.begin();  ch != children.end(); ++ch)
    if (*ch)
      (*ch)->set_actuator_strike_scales(ss);
}


bool limb::any_strike_in_progress() const
{
  bool outval = false;

  hinge_actuator * a;
  int i;
  for (i = 0;i<hinges.size();++i)
    if ((a=hinges[i]->get_actuator())!=NULL)
    {
      outval = a->get_strike();
      if (outval) break;
    }
  for (i = 0;i<linear_hinges.size();++i)
    if ((a=linear_hinges[i]->get_actuator())!=NULL)
    {
      outval = a->get_strike();
      if (outval) break;
    }
  vector<limb *>::const_iterator ch;
  for(ch = children.begin();  !outval && ch != children.end(); ++ch)
    if (*ch)
    {
      outval = (*ch)->any_strike_in_progress();
      if (outval) break;
    }
  return outval;
}


void limb::set_my_actor( actor* a )
{
  my_actor = a;
  vector<limb*>::iterator ci;
  for ( ci=children.begin(); ci!=children.end(); ++ci )
  {
    if ( *ci )
      (*ci)->set_my_actor( a );
  }
}


void limb::set_hinges_from_old_times(time_value_t t)
{
  int i;
  for (i = 0;i<hinges.size();++i)
  {
    hinges[i]->set_val(hinges[i]->get_last_second_val()*t + (hinges[i]->get_this_second_val())*(1-t));
    hinges[i]->set_vel(0);
  }
  for (i = 0;i<linear_hinges.size();++i)
  {
    linear_hinges[i]->set_val(linear_hinges[i]->get_last_second_val()*t + linear_hinges[i]->get_this_second_val()*(1-t));
    linear_hinges[i]->set_vel(0);
  }
  vector<limb *>::iterator ch;
  for(ch = children.begin(); ch != children.end(); ++ch)
    if (*ch)
    {
      (*ch)->set_hinges_from_old_times(t);
    }
}

void limb::set_branch_visible( bool _vis )
{
  limb_body* lb = get_body();
  lb->set_family_visible( _vis );
  vector<limb *>::iterator ch;
  for(ch = children.begin(); ch != children.end(); ++ch)
    if (*ch)
    {
      (*ch)->set_branch_visible(_vis);
    }
}

void limb::restore_hinges_from_po()
{
  assert(lb);

  int num = 0;

  num = linear_hinges.size();
  if(num > 0)
  {
    assert(num <= 3);
    assert(lb->get_parent());

    vector3d pos = lb->get_abs_position();
    vector3d parent_pos = lb->get_parent()->get_abs_position();

    pos -= parent_pos;

    for (int i = 0; i < num; ++i)
    {
      assert(linear_hinges[i]);
      vector3d axis = linear_hinges[i]->get_axis();

      linear_hinges[i]->set_val((pos.x * axis.x) + (pos.y * axis.y) + (pos.z * axis.z));
      linear_hinges[i]->set_vel(0.0f);
    }
  }


  num = hinges.size();
  if(num > 0)
  {
    assert(num <= 3);

    rational_t val[3] = { 0.0f, 0.0f, 0.0f };

	// do angular
    po rel_po = lb->get_rel_po();
    rel_po.decompose_rot((2<num)?hinges[2]->get_axis():ZEROVEC, (1<num)?hinges[1]->get_axis():ZEROVEC, (0<num)?hinges[0]->get_axis():ZEROVEC, &val[2], &val[1], &val[0]);

    for (int i = 0; i < num; ++i)
    {
      assert(hinges[i]);

      hinges[i]->set_val(val[i]);
      hinges[i]->set_vel(0.0f);
    }
  }

  vector<limb *>::iterator ch;
  for(ch = children.begin(); ch != children.end(); ++ch)
  {
    if (*ch)
      (*ch)->restore_hinges_from_po();
  }
}


void limb::kill_hinge_velocities()
{
  vector<hinge *>::iterator hi;
  for (hi = hinges.begin();hi!=hinges.end(); ++hi)
  {
    (*hi)->set_vel(0);
  }
  for (hi = linear_hinges.begin();hi!=linear_hinges.end(); ++hi)
  {
    (*hi)->set_vel(0);
  }
  vector<limb *>::iterator ch;
  for(ch = children.begin(); ch != children.end(); ++ch)
    if (*ch)
    {
      (*ch)->kill_hinge_velocities();
    }
}

////////////////////////////////////////////////////////////////
// limb_body stuff
////////////////////////////////////////////////////////////////

limb_body::limb_body( limb * _my_limb, const entity_id& _id,
                      unsigned int _flags )
  :   entity( _id, ENTITY_LIMB_BODY, _flags )
{
  my_limb = _my_limb;
  my_meat = NULL;
}


limb_body::limb_body( limb * _my_limb,
                      const entity_id& _id,
                      entity_flavor_t _flavor,
                      unsigned int _flags )
  :   entity( _id, _flavor, _flags )
{
  my_limb = _my_limb;
  my_meat = NULL;
}


limb_body::~limb_body()
{
  if ( my_meat )
    limb_body_meat_bank.delete_instance( my_meat );
}


static const char* signal_names[] =
{
  #define MAC(label,str)  str,
  #include "limb_body_signals.h"
  #undef MAC
};

unsigned short limb_body::get_signal_id( const char *name )
{
  int idx;

  for( idx = 0; idx < (sizeof(signal_names)/sizeof(char*)); idx++ )
  {
    int offset = strlen(signal_names[idx])-strlen(name);

    if( offset > strlen( signal_names[idx] ) )
      continue;

    if( !strcmp(name,&signal_names[idx][offset]) )
      return( idx + PARENT_SYNC_DUMMY + 1 );
  }

  // not found
  return entity::get_signal_id( name );
}

void limb_body::register_signals()
{
  #define MAC(label,str)  signal_manager::inst()->insert( str, label );
  #include "limb_body_signals.h"
  #undef MAC
}


///////////////////////////////////////////////////////////////////////////////
// NEWENT File I/O
///////////////////////////////////////////////////////////////////////////////

limb_body::limb_body( limb * _my_limb,
                      chunk_file& fs,
                      const entity_id& _id,
                      entity_flavor_t _flavor,
                      unsigned int _flags )
  :   entity( fs, _id, _flavor, _flags )
{
  my_limb = _my_limb;
  my_meat = NEW limb_body_meat();
  my_meat->shock_damper = 1;
  my_meat->l_shock_damper = 1;
  my_meat->h_shock_damper = 1;
  my_meat->free_body = 0;
  limb_body_meat_bank.insert_new_object( my_meat, id.get_val() );
  set_flag( EFLAG_GRAPHICS, true );
  set_flag( EFLAG_GRAPHICS_VISIBLE, true );
  set_flag( EFLAG_PHYSICS_MOVING, true );
  set_flag( EFLAG_MISC_MEMBER_OF_VARIANT, true );  // until proven otherwise
  set_active( true );

  // read limb_body-specific data
  chunk_flavor cf;
  bool read_physical_properties = false;
  bool read_limb_properties = false;
  for ( serial_in(fs,&cf); cf!=CHUNK_END; serial_in(fs,&cf) )
  {
    if ( cf == chunk_flavor("physprop") )
    {
      // read physical properties
      read_physical_properties = true;
      for ( serial_in(fs,&cf); cf!=CHUNK_END; serial_in(fs,&cf) )
      {
        if ( cf == chunk_flavor("mass") )
        {
          rational_t m;
          serial_in( fs, &m );
          set_mass( m );
        }
        else if ( cf == chunk_flavor("moi") )
        {
          vector3d v;
          serial_in( fs, &v );
          set_I_vector( v );
        }
        else if ( cf == chunk_flavor("volume") )
        {
          rational_t v;
          serial_in( fs, &v );
          set_volume( v );
        }
        else if ( cf == chunk_flavor("radius") )
        {
          rational_t r;
          serial_in( fs, &r );
          set_radius( r );
        }
        else if ( cf == chunk_flavor("com") )
        {
          vector3d v;
          serial_in( fs, &v );
          set_c_o_m( v );
        }
        else
          error( fs.get_name() + ": unknown chunk found in limb physical properties" );
      }
    }
    else if ( cf == chunk_flavor("limbprop") )
    {
      // read limb properties
      read_limb_properties = true;
      for ( serial_in(fs,&cf); cf!=CHUNK_END; serial_in(fs,&cf) )
      {
        if ( cf == chunk_flavor("minext") )
        {
          // minimum extents (x, y, z)
          vector3d v;
          serial_in( fs, &v );
          set_min_extent( v );
        }
        else if ( cf == chunk_flavor("maxext") )
        {
          // maximum extents (x, y, z)
          vector3d v;
          serial_in( fs, &v );
          set_max_extent( v );
        }
        else
          error( fs.get_name() + ": unknown chunk found in limb properties" );
      }
    }
    else
      error( fs.get_name() + ": unknown chunk found in limb_body" );
  }

  if ( !read_physical_properties || !read_limb_properties)
  {
    // need a collision geometry in order to compute properties
    if ( my_visrep && my_visrep->get_type()==VISREP_PMESH )
      colgeom = NEW cg_mesh( static_cast<vr_pmesh*>(my_visrep), id.get_val(), cg_mesh::ALLOW_WARNINGS );
    else
      colgeom = NEW collision_capsule( this );
    if ( !read_physical_properties )
    {
      // compute physical properties from collision geometry
      colgeom->estimate_physical_properties( this );
    }
    if ( !read_limb_properties )
    {
      compute_limb_properties();
    }
    delete colgeom;
  }
}


void limb_body::compute_limb_properties()
{
  // compute other limb properties from collision geometry
  vector3d minext, maxext;
  colgeom->get_min_extent( &minext);
  colgeom->get_max_extent( &maxext);
  set_min_extent( minext );
  set_max_extent( maxext );
}

///////////////////////////////////////////////////////////////////////////////
// Old File I/O
///////////////////////////////////////////////////////////////////////////////

limb_body::limb_body( limb * _my_limb,
                      const stringx& colgeom_fname,
                      vr_pmesh* single_mesh ,
                      const entity_id& id_name,
                      entity_flavor_t _flavor )
      :   entity( id_name, _flavor )
{
  my_limb = _my_limb;
  colgeom = NEW cg_mesh(colgeom_fname.c_str());
  colgeom->set_owner(this);
  my_meat = limb_body_meat_bank.new_instance(colgeom_fname, 0 );

  // For DEBUG <<<< ing purposes
  stringx fname = stringx(colgeom_fname);
  assert(fname.find( ".txtmesh" )!=stringx::npos);
  my_visrep = vr_pmesh_bank.new_instance(fname, 0);

  set_flag(EFLAG_GRAPHICS,true);
  set_flag(EFLAG_GRAPHICS_VISIBLE,true);
  set_flag(EFLAG_PHYSICS_MOVING,true);
  set_active(true);

  if ( colgeom )
    colgeom->estimate_physical_properties( this );

  stringx colgeom_dir;
  stringx colgeom_prefix;
  stringx colgeom_suffix;

  colgeom_dir = get_dir( colgeom_fname );
  colgeom_prefix = get_fname_wo_ext( colgeom_fname );
  colgeom_suffix = get_ext( colgeom_fname );

/*
  for(int i=0;i<single_mesh->get_num_bones();i++)
  {
    if( single_mesh->get_bone_name(i)==colgeom_prefix )
    {
      bone_idx = i;
      break;
    }
  }
*/

  compute_limb_properties();
}


///////////////////////////////////////////////////////////////////////////////
// Instancing
///////////////////////////////////////////////////////////////////////////////

entity* limb_body::make_instance( limb * _my_limb,
                                  const entity_id& _id,
                                  unsigned int _flags ) const
{
  limb_body* newlb = NEW limb_body( _my_limb, _id, _flags );
  newlb->copy_instance_data( *this );
  return newlb;
}


void limb_body::copy_instance_data( const limb_body& b )
{
  entity::copy_instance_data( b );

  if ( b.my_meat )
    my_meat = limb_body_meat_bank.new_instance( b.my_meat );
}


///////////////////////////////////////////////////////////////////////////////
// Misc.
///////////////////////////////////////////////////////////////////////////////

void limb_body::frame_advance( time_value_t t )
{
  entity::frame_advance( t );
}

region_node* limb_body::get_region() const
{
  // return owning character's region by going up the chain until we find one
  return get_parent()->get_region();
}


actor *limb_body::get_my_actor() const
{
  return ((my_limb != NULL) ? my_limb->get_my_actor() : NULL);
}

/*
Note that this feature is somewhat of a kludge, and that the system probably
needs to be rewritten in the future (e.g., allow signals to carry data,
allow multiple signal tracks running on a given node, etc.).
*/
void limb_body::raise_signal( signal_list::size_t idx ) const
{
  if(get_my_actor() != NULL)
    get_my_actor()->raise_signal(idx);
//  else
//    entity::raise_signal(idx);
}
