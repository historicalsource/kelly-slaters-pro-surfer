// scanner.cpp
#include "global.h"

// BIGCULL #include "scanner.h"
#include "wds.h"
#include "terrain.h"
#include "app.h"
#include "geomgr.h"
#include "renderflav.h"
#include "profiler.h"
#include "ai_interface.h"
#if defined(TARGET_XBOX)
#include "scanner.h"
#else
#include "ai_senses.h"
#include "ai_communication.h"
#endif /* TARGET_XBOX JIV DEBUG */
#include "game.h"

extern game * g_game_ptr;

// Macro to compare rational numbers allowing for floating point error
#define DAMN_CLOSE(a, b)           (__fabs((a) - (b)) < 0.0000001f)

// Utility function to intersect 2D line segments in YZ space
static bool intersect_yz_segment(const vector3d &u0, const vector3d &u1, const vector3d &v0, const vector3d &v1, vector3d &result)
{
	rational_t m = 0, n = 0, b = 0, c = 0, x, y;

	// compute slope and intercept (segment u is y=mx+b, segment v is y=nx+c)
	// and check for special case of infinity slope
	bool uinf = (__fabs(u1.z - u0.z) < 0.000001f);
	bool vinf = (__fabs(v1.z - v0.z) < 0.000001f);
	if (uinf && vinf)
		return false;

	if (!uinf)
	{
		m = (u1.y - u0.y) / (u1.z - u0.z);
		b = u0.y - (m * u0.z);
	}

	if (!vinf)
	{
		n = (v1.y - v0.y) / (v1.z - v0.z);
		c = v0.y - (n * v0.z);
	}

	if (!uinf && !vinf)
	{
		// if slopes are equal, they are parallel and intersect nowhere
		if (__fabs(m - n) < 0.000001f)
			return false;

		// locate the intersection of the lines
		x = (c - b) / (m - n);
		y = (m * x) + b;
	}
	else if (uinf)
	{
		x =	u0.z;
		y = (n * x) + c;
	}
	else
	{
		x = v0.z;
		y = (m * x) + b;
	}

	// check to see if that value is in range of either segment
	if (x < min(u0.z, u1.z) || x > max(u0.z, u1.z) || x < min(v0.z, v1.z) || x > max(v0.z, v1.z))
		return false;

	// store the result and return success
	result.x = 0;
	result.y = y;
	result.z = x;

	return true;
}

render_flavor_t scanner::render_passes_needed() const
{
  render_flavor_t flav = RENDER_TRANSLUCENT_PORTION;
	if ( my_visrep != the_pmesh )
    flav |= entity::render_passes_needed();
  return flav;
}


bool scanner::seg_in_front_of(vector3d &p0, vector3d &p1, rational_t d0, rational_t d1, segment *s)
{
  vector3d dummy;

  // check if both points of seg 1 are in front of seg 2
  rational_t closest = min(s->d0, s->d1);
  if (d0 < closest && d1 < closest)
    return true;

  // check if both points of seg 2 are in front of seg 1
  rational_t furthest = max(s->d0, s->d1);
  if (d0 > furthest && d1 > furthest)
    return false;

  // they share a region, so do an intersection test from the origin to the center of seg 1
  vector3d center((p0 + p1) * 0.5f);
  if (intersect_yz_segment(ZEROVEC, center, s->p0, s->p1, dummy))
    return false;

  return true;
}

///////////////////////////////////////////////////////////////////////////////
// Construction
///////////////////////////////////////////////////////////////////////////////

vr_pmesh *scanner::the_pmesh = NULL;
int scanner::detected_count = 0;

scanner::scanner( chunk_file& fs,
                  const entity_id& _id,
                  entity_flavor_t _flavor,
                  unsigned int _flags )
  : entity( fs, _id, _flavor, _flags )
{
  initialize_variables();
}

scanner::scanner( const entity_id& _id,
                  entity_flavor_t _flavor,
                  unsigned int _flags )
  : entity( _id, _flavor, _flags )
{
  initialize_variables();
}

scanner::~scanner()
{
  clear_poly_list();
	clear_seg_list();

  // make sure alarm sound gets shut off
  if ( contact )
    DEC_detector();

#ifndef NGL
  // PEH BETA this sheet_pmesh deletion
  if( sheet_pmesh )
  {
    delete sheet_pmesh;
    sheet_pmesh = 0;
  }
#endif

  /*
  // THE THING THESE POINT TO HAVE ALREADY BEEN DESTROYED
  // IN A PURGING RITUAL ELSEWHERE
  if( the_pmesh != NULL )
  {
  vr_pmesh_bank.delete_instance( the_pmesh );
  }

  if( sheet_pmesh )
  {
  delete sheet_pmesh;
  sheet_pmesh = 0;
  }
  */
}

void scanner::initialize_variables()
{
  lookat = ZEROVEC;
  max_aperture = _SCANNER_DEFAULT_MAX_APERTURE;
  max_sweep = _SCANNER_DEFAULT_MAX_SWEEP;
  max_length = _SCANNER_DEFAULT_MAX_LENGTH;
  scan_flags = 0;
  curr_aperture = max_aperture;
  curr_sweep = max_sweep;
  speed = _SCANNER_DEFAULT_SPEED;
  angle = 0;
  thickness = _SCANNER_DEFAULT_THICKNESS;
  my_color = _SCANNER_DEFAULT_COLOR;
  sheet_offset = ZEROVEC;
  contact = false;
  contact_color_ramp = 0;
  tag_mode = false;
	poly_lists = NULL;
	num_lists = 0;
	segments = pending = pending_check = NULL;

#ifndef NGL
	// Need an instance of the pmesh
	vr_pmesh *m = vr_pmesh_bank.find_instance("_scan_sheet");
	if (m)
  {
		vr_pmesh_bank.new_instance(m);
  }
	else
	{
    //		if (the_pmesh == NULL)
		{
			// Set up the pmesh object
			the_pmesh = NEW vr_pmesh;
			the_pmesh->make_rectangle();
			the_pmesh->shrink_memory_footprint();
		}
		// insert the pmesh object into the instance bank
		m = the_pmesh;
		vr_pmesh_bank.insert_new_object(m, "_scan_sheet");
	}

	if (!my_visrep)
		my_visrep = m;

  sheet_pmesh = NEW vr_pmesh;
#endif

  entity::set_flag(EFLAG_GRAPHICS, true);
  entity::set_flag(EFLAG_GRAPHICS_VISIBLE, true);
  entity::set_flag(EFLAG_MISC_NO_LOD, true);
  entity::set_flag(EFLAG_GRAPHICS_MOTION_TRAIL, false);
  entity::set_stationary( true );
  entity::set_active( true );

  owner = NULL;
  scan_target = NULL;
}

void scanner::initialize()
{
	build_poly_list();
}

// This function allows parsing instance data according to entity type.
// If it recognizes the given stringx as a chunk of instance
// data for this type, it will parse the data; otherwise it will hand
// the parsing up to the parent class.
bool scanner::parse_instance( const stringx& pcf, chunk_file& fs )
{
  if ( pcf == stringx("scanner") )
  {
    stringx cf;
    for ( serial_in(fs,&cf); cf!=chunkend_label; serial_in(fs,&cf) )
    {
      if ( cf == stringx("lookat") )
        serial_in( fs, &lookat );
      else if ( cf == stringx("aperture") )
      {
        serial_in( fs, &max_aperture );
        // value is read in as degrees; must convert to radians
        max_aperture = DEG_TO_RAD(max_aperture);
        assert ( max_aperture <= PI );
        curr_aperture = max_aperture;
      }
      else if ( cf == stringx("sweep") )
      {
        serial_in( fs, &max_sweep);
        // value is read in as degrees; must convert to radians
        if ( max_sweep == 360 )
          max_sweep = PI * 2;
        else
          max_sweep = DEG_TO_RAD(max_sweep);
        assert( max_sweep <= PI*2 );
        curr_sweep = max_sweep;
      }
      else if ( cf == stringx("speed") )
      {
        // value is read in as degrees per second; must convert to radians per second
        serial_in( fs, &speed );
        speed = DEG_TO_RAD(speed);
      }
      else if ( cf == stringx("max_len") )
        serial_in( fs, &max_length );
      else if ( cf == stringx("flags") )
        serial_in( fs, &scan_flags );
      else if ( cf == stringx("thicknes") )
        serial_in( fs, &thickness );
      else if ( cf == stringx("color") )
        serial_in( fs, &my_color );
    }

    reset();
  }
  else
    return entity::parse_instance( pcf, fs );
  return true;
}

bool scanner::handle_enx_chunk(chunk_file &fs, stringx &label)
{
  if (label == "scan_offset")
  {
    serial_in(fs, &sheet_offset);

    return true;
  }
  else
    return entity::handle_enx_chunk(fs, label);
}

///////////////////////////////////////////////////////////////////////////////
// Instancing
///////////////////////////////////////////////////////////////////////////////

entity *scanner::make_instance(const entity_id &_id, unsigned int _flags) const
{
  scanner *new_scanner = NEW scanner(_id, ENTITY_SCANNER, _flags);
  new_scanner->copy_instance_data(*this);
  return (entity *)new_scanner;
}

void scanner::copy_instance_data(const scanner &b)
{
  entity::copy_instance_data(b);

  initialize_variables();

  lookat = b.lookat;
  max_aperture = b.max_aperture;
  max_sweep = b.max_sweep;
	max_length = b.max_length;
  scan_flags = b.scan_flags;
  curr_aperture = b.curr_aperture;
  curr_sweep = b.curr_sweep;
  speed = b.speed;
  angle = b.angle;
  thickness = b.thickness;
  my_color = b.my_color;
  sheet_offset = b.sheet_offset;
}

/////////////////////////////////////////////////////////////////////////////
// Scanner settings interface
/////////////////////////////////////////////////////////////////////////////

void scanner::set_color( const color32& _color )
{
  my_color = _color;
}


/////////////////////////////////////////////////////////////////////////////
// Event signals
/////////////////////////////////////////////////////////////////////////////

static const char* scanner_signal_names[] =
{
  #define MAC(label,str)  str,
  #include "scanner_signals.h"
  #undef MAC
};


//extern profiler_timer proftimer_advance;
//profiler_timer proftimer_scanner_signal("Scanner signal",&proftimer_advance);
//profiler_counter profcounter_scanner_signal("Scanner signal",NULL,0);

unsigned short scanner::get_signal_id( const char *name )
{
  unsigned idx;
  //proftimer_scanner_signal.start();
  //profcounter_scanner_signal.add_count(1);
  for( idx = 0; idx < (sizeof(scanner_signal_names)/sizeof(char*)); ++idx )
  {
    unsigned offset = strlen(scanner_signal_names[idx])-strlen(name);

    if( offset > strlen( scanner_signal_names[idx] ) )
      continue;

    if( !strcmp(name,&scanner_signal_names[idx][offset]) )
    {
      //proftimer_scanner_signal.stop();
      return( idx + PARENT_SYNC_DUMMY + 1 );
    }
  }

  // not found
  unsigned short outval = entity::get_signal_id( name );

  //proftimer_scanner_signal.stop();
  return outval;
}

// This static function must be implemented by every class which can generate
// signals, and is called once only by the application for each such class;
// the effect is to register the name and local id of each signal with the
// signal_manager.  This call must be performed before any signal objects are
// actually created for this class (via signaller::signal_ptr(); see signal.h).
void scanner::register_signals()
{
  // for descendant class, replace "entity" with appropriate string
  #define MAC(label,str)  signal_manager::inst()->insert( str, label );
  #include "scanner_signals.h"
  #undef MAC
}

// This virtual function, used only for debugging purposes, returns the
// name of the given local signal
const char* scanner::get_signal_name( unsigned short idx ) const
{
  assert( idx < N_SIGNALS );
  if ( idx <= PARENT_SYNC_DUMMY )
    return entity::get_signal_name( idx );
  else
    return scanner_signal_names[idx-PARENT_SYNC_DUMMY-1];
}


///////////////////////////////////////////////////////////////////////////////
// World interface
///////////////////////////////////////////////////////////////////////////////

const rational_t CONTACT_COLOR_RAMP_VEL = 2.0f;

//extern profiler_timer proftimer_advance;
extern profiler_timer proftimer_scanner_advance;
  //profiler_timer proftimer_scanner_segcheck("seg check",&proftimer_scanner_advance);
//profiler_counter profcounter_scanner_advance("Scanner advance",NULL,0);
  //profiler_counter profcounter_scanner_segcheck("seg check",&profcounter_scanner_advance,0);

void scanner::frame_advance( time_value_t t )
{
  if(owner)
  {
    set_active(owner->get_my_entity()->is_alive() && owner->is_active());

    if(is_active())
    {
      if(!owner->detector())
      {
        if((get_abs_position() - last_ai_pos).length2() >= 1.0f)
          build_poly_list(false);

        owner->get_my_entity()->update_abs_po_reverse();
#if defined(TARGET_XBOX)
        STUB( "scanner::frame_advance" );

        vector3d off;
#else
        lookat = owner->get_my_entity()->get_abs_po().slow_xform(owner->get_eyes()->get_scan_lookat());

        vector3d off = owner->get_eyes()->get_scan_offset() + owner->get_my_entity()->get_abs_position();
#endif /* TARGET_XBOX JIV DEBUG */

        set_rel_position(off);
      }
    }
  }

  if ( !is_active() )
  {
    // This can happen because set_active(false) might occur prior to the
    // frame_advance() call in the same frame, in which case the world still
    // thinks of the scanner as being active.  Checking is_active() here is
    // easier and more efficient than removing the scanner from the active
    // entities list on the fly.
    return;
  }

  proftimer_scanner_advance.start();
  //profcounter_scanner_advance.add_count(1);

  // if we're in contact with the hero, track them
  if ( contact )
  {
    update_abs_po_reverse();

    vector3d facing = lookat - get_abs_po().get_position();
    vector3d diff = scan_target->get_abs_po().get_position();
    diff -= get_abs_po().get_position();
    diff.y = 0; facing.y = 0;
    facing.normalize(); diff.normalize();
    rational_t a = fast_acos(dot(diff, facing));
    if (a > curr_sweep*0.5f) a = curr_sweep*0.5f;
    if ((diff.x*facing.z-facing.x*diff.z) > 0)
      a *= -1;
    angle = a;
    // update color ramping for alerted mode
    contact_color_ramp += t * CONTACT_COLOR_RAMP_VEL;
    if ( contact_color_ramp > 2 )
      contact_color_ramp -= 2;
  }
  else
	{
		// update sweep/spin angle value
		angle += speed * t;
		if ( speed < 0 )
		{
			if ( angle < -curr_sweep*0.5f )
			{
				// value has hit minimum:
				if ( curr_sweep == PI*2 )
				{
					// sweeping full circle:
					// just wrap the value
					angle += curr_sweep;
				}
				else
				{
					// sweeping back and forth:
					// reverse direction
					angle = -curr_sweep - angle;
					speed *= -1;
				}
			}
		}
		else
		{
			if ( angle > curr_sweep*0.5f )
			{
				// value has hit maximum:
				if ( curr_sweep == PI*2 )
				{
					// sweeping full circle:
					// just wrap the value
					angle -= curr_sweep;
				}
				else
				{
					// sweeping back and forth:
					// reverse direction
					angle = curr_sweep - angle;
					speed *= -1;
				}
			}
		}
	}

  // compute orientation for scanner
  vector3d xb, yb;
  // lookat value determines pitch and initial yaw
  vector3d zb = lookat - get_abs_position();
  zb.normalize();
  if ( is_flagged(SPINNER) )
  {
    // scanner is flagged to spin about the lookat axis:
    // angle represents roll angle
    po rot1;
    rot1.set_rotate_z( angle );
    // compute secondary rotations for desired pitch and yaw
    if ( __fabs(zb.y) < 0.95f )
      xb = cross( YVEC, zb ).normalize();
    else
      xb = cross( XVEC, zb ).normalize();
    yb = cross( zb, xb );
    po rot2( xb, yb, zb, ZEROVEC );
    // multiply these matrices to obtain instantaneous orientation
    fast_po_mul(rot2, rot1, rot2);
//    rot2 = rot1 * rot2;

    if ( link_ifc()->get_parent() )
    {
      fast_po_mul(rot2, rot2, link_ifc()->get_parent()->get_abs_po().inverse());
//      rot2 = rot2 * link_ifc()->get_parent()->get_abs_po().inverse();
    }
    rot2.fixup();
    rot2.set_position( get_rel_position() );
    set_rel_po( rot2 );
  }
  else
  {
    // scanner is in sweep mode:
    // angle represents yaw angle
    po rot;
    rot.set_rotate_y( angle );
    zb = rot.slow_xform( zb );
    // there is no roll
    if ( __fabs(zb.y) < 0.95f )
      xb = cross( YVEC, zb ).normalize();
    else
      xb = cross( XVEC, zb ).normalize();
    yb = cross( zb, xb );
    // apply resulting instantaneous orientation
    po newpo( xb, yb, zb, ZEROVEC );
    if ( link_ifc()->get_parent() )
    {
      fast_po_mul(newpo, newpo, link_ifc()->get_parent()->get_abs_po().inverse());
//      newpo = newpo * link_ifc()->get_parent()->get_abs_po().inverse();
      newpo.fixup();
    }
    newpo.set_position( get_rel_position() );
    set_rel_po( newpo );
  }

	//// Paint the terrain ////
	clear_seg_list();

  po sheet_po = get_abs_po();
  sheet_po.set_position( sheet_po.slow_xform(sheet_offset) );
	po inv_po = sheet_po.inverse();

  rational_t aperture = curr_aperture * 0.5F;

  rational_t sin_aperture,cos_aperture;
  fast_sin_cos_approx( aperture, &sin_aperture, &cos_aperture );
  sin_aperture *= max_length;
  cos_aperture *= max_length;

  for (int l = 0; l < num_lists; ++l)
  {
    po xform_po;
    if (!poly_lists[l].xform_po)
      xform_po = inv_po;
    else
      xform_po = *poly_lists[l].xform_po * inv_po;

	  for (int p = 0; p < poly_lists[l].num_polys; p++)
	  {
      if (!tag_mode && !poly_lists[l].polys[p].active)
        continue;

		  vector3d verts[3];
		  int v;

		  for (v = 0; v < 3; ++v)
			  verts[v] = xform_po.slow_xform(poly_lists[l].polys[p].verts[v]);

		  rational_t dp[3];  // dot product at each vert
		  int sp[3];         // sp[0] is on one side of the split, sp[1] and sp[2] are on the other

		  for (v = 0; v < 3; ++v)
			  dp[v] = verts[v].x;

		  // figure out which vert (if any) is by itself on one side of the sheet's plane
		  bool zero_and_one = ((dp[0] < 0) == (dp[1] < 0));
		  bool one_and_two  = ((dp[1] < 0) == (dp[2] < 0));
		  bool two_and_zero = ((dp[2] < 0) == (dp[0] < 0));

		  sp[0] = -1;
		  if (one_and_two)
		  {
			  if (!zero_and_one)
				  { sp[0] = 0; sp[1] = 1; sp[2] = 2; }
		  }
		  else
		  {
			  if (two_and_zero)
				  { sp[0] = 1; sp[1] = 2; sp[2] = 0; }
			  else if (zero_and_one)
				  { sp[0] = 2; sp[1] = 0; sp[2] = 1; }
		  }

		  if (sp[0] < 0)
			  continue;         // no splits found, go to next poly

		  rational_t adp[3];  // absolute value dot product at each vert
		  for (v = 0; v < 3; ++v) adp[v] = __fabs(dp[v]);

		  // locate the two edges across which the split falls
		  vector3d edge0 = verts[sp[1]] - verts[sp[0]];
		  edge0 *= adp[sp[0]] / (adp[sp[0]] + adp[sp[1]]);

		  vector3d edge1 = verts[sp[2]] - verts[sp[0]];
		  edge1 *= adp[sp[0]] / (adp[sp[0]] + adp[sp[2]]);

		  // build the line segment (p0 -> p1) of the split itself
		  vector3d p0 = edge0 + verts[sp[0]];
		  vector3d p1 = edge1 + verts[sp[0]];

		  // front clip
		  if (p0.z < 0.01f && p1.z < 0.01f)
			  continue;

		  // compute normal
		  vector3d normal = inv_po.non_affine_slow_xform(poly_lists[l].polys[p].normal);

		  // backface cull
		  vector3d np0(p0), np1(p1);
		  np0.normalize();
		  np1.normalize();
		  if (dot(np0, normal) >= 0 && dot(np1, normal) >= 0)
			  continue;

		  // angle clip
		  rational_t a0 = atan2(p0.y, p0.z);	// fix trig (dc 08/16/01)
		  rational_t a1 = atan2(p1.y, p1.z);

		  if (DAMN_CLOSE(a0, a1))
			  continue;         // if gap has closed to nothing, get out

		  bool out0 = (__fabs(a0) > aperture) || (p0.z < 0.01f);
		  bool out1 = (__fabs(a1) > aperture) || (p1.z < 0.01f);

		  if (out0 && out1)
		  {
			  // check for case where both verts are completely off to one side
			  if ((a0 < 0) == (a1 < 0))
				  continue;

			  // check for case where either vert is out of range, but some portion of the segment is in range
			  vector3d i0, i1;
			  if (!intersect_yz_segment(p0, p1, vector3d(0, 0, 0), vector3d(0, sin_aperture, cos_aperture), i0) ||
			      !intersect_yz_segment(p0, p1, vector3d(0, 0, 0), vector3d(0, -sin_aperture, cos_aperture), i1))
				  continue;

			  // if we get to here, part of the segment was in range
			  p0 = i0;
			  p1 = i1;

			  // recompute angles
			  a0 = atan2(p0.y, p0.z);	// fix trig (dc 08/16/01)
			  a1 = atan2(p1.y, p1.z);
		  }
		  else if (out0 || out1)
		  {
			  // one vert or the other is outside, so clip it to the scanner's aperture
			  vector3d i0;
			  if (!intersect_yz_segment(p0, p1, vector3d(0, 0, 0), vector3d(0, sin_aperture, cos_aperture), i0))
				  if (!intersect_yz_segment(p0, p1, vector3d(0, 0, 0), vector3d(0, -sin_aperture, cos_aperture), i0))
					  continue;

			  // set the NEW vert depending on which one was outside
			  if (out0)
			  {
				  p0 = i0;
				  a0 = atan2(p0.y, p0.z);	// fix trig (dc 08/16/01)
			  }
			  else
			  {
				  p1 = i0;
				  a1 = atan2(p1.y, p1.z);	// fix trig (dc 08/16/01)
			  }
		  }

		  if (DAMN_CLOSE(a0, a1))
			  continue;         // if gap has closed to nothing, get out

		  if (a0 > a1)
		  {
			  // switch them around so that it's always a0 -> a1
			  rational_t tmp = a0; a0 = a1; a1 = tmp;
			  vector3d tmp2(p0); p0 = p1; p1 = tmp2;
		  }

		  // compute dist^2 values
		  rational_t d0 = p0.length2();
		  rational_t d1 = p1.length2();

		  //// Run the clipping check ////
		  if (check_segment(p0, p1, normal, a0, a1, d0, d1, l, p))
      {
        //proftimer_scanner_segcheck.start();
        //profcounter_scanner_segcheck.add_count(1);
			  add_segment(p0, p1, normal, a0, a1, d0, d1, l, p);
        //proftimer_scanner_segcheck.stop();
      }

		  // check_segment() can generate NEW segments, so check and add those before continuing
		  segment *s, *next;
		  for (s = pending; s; s = next)
		  {
			  // pending segments have already been checked, just add them directly
			  next = s->next;
			  s->next = segments;
			  segments = s;
		  }

		  pending = NULL;

		  while (pending_check)
		  {
			  // this stuff is necessary because more segments can be added to the list inside this loop
			  segment *old_list = pending_check;
			  pending_check = NULL;

			  for (s = old_list; s; s = next)
			  {
				  // pending_check segments have NOT been checked, only add them after doing a check
				  next = s->next;
				  if (check_segment(s->p0, s->p1, s->normal, s->a0, s->a1, s->d0, s->d1, s->list, s->poly))
				  {
					  s->next = segments;
					  segments = s;
				  }
				  else
					  delete s;
			  }
		  }
	  }
  }

	//// Check hero collision ////
#define HERO_RADIUS    0.5f       // this is the radius of the collision cylinder for the hero

	bool hit = check_scan_collisions(inv_po, aperture, sin_aperture, cos_aperture);

	// raise the appropriate signal if contact status has changed
  set_contact( hit );

  concatenate_segments();

  if ( tag_mode )
  {
    for (segment *s = segments; s; s = s->next)
      if (s->list >= 0 && s->poly >= 0)
        if (poly_lists[s->list].xform_po == NULL)
          poly_lists[s->list].polys[s->poly].active = true;
  }

  proftimer_scanner_advance.stop();
}


bool scanner::check_scan_collisions(const po &inv_po, rational_t aperture, rational_t sin_aperture, rational_t cos_aperture)
{
  scan_target = NULL;

  if(owner)
  {
    unsigned int power = 0;

    vector<ai_interface *>::const_iterator aiit = ai_interface::get_all_ai_interfaces().begin();
    vector<ai_interface *>::const_iterator ai_end = ai_interface::get_all_ai_interfaces().end();

    while(aiit != ai_end)
    {
      ai_interface *ai = (*aiit);
      ++aiit;
      if(ai != owner && owner->is_enemy(ai))
      {
        if(ai->get_power_level() > power && check_scan_collision_ent(ai->get_my_entity(), inv_po, aperture, sin_aperture, cos_aperture))
        {
          scan_target = ai->get_my_entity();
          power = ai->get_power_level();
        }
      }
    }

    if(scan_target != NULL)
    {
#if defined(TARGET_XBOX)
      STUB( "scanner::check_scan_collisions" );
#else
      ai_radio_message msg(owner, _RADIO_ENEMY_SIGHTED, power);
      msg.set_msg_ent(scan_target);
      msg.set_msg_pos(scan_target->get_abs_position());

      owner->broadcast_message(_CHANNEL_ALLY, msg);
#endif /* TARGET_XBOX JIV DEBUG */
    }
  }
  else
  {
    if(check_scan_collision_ent(g_world_ptr->get_hero_ptr(), inv_po, aperture, sin_aperture, cos_aperture))
      scan_target = g_world_ptr->get_hero_ptr();
    else
      scan_target = NULL;
  }

  return(scan_target != NULL);
}


bool scanner::check_scan_collision_ent(entity *ent, const po &inv_po, rational_t aperture, rational_t sin_aperture, rational_t cos_aperture)
{
  if(!ent || !ent->is_alive() || !ent->is_visible())
    return(false);

  bool hit = false;

	segment *s, *next;

	// build the hero's segment (vector from feet to head)
  vector3d pos(ent->get_abs_position());
  pos.y -= 1.0f;
	vector3d feet = inv_po.slow_xform(pos);
  pos.y += 2.0f;
	vector3d head = inv_po.slow_xform(pos);

	// compute the segment of intersection against the scanner plane
	vector3d p0, p1;
	bool intersection = true;

	if (feet.z > 0.001f && head.z > 0.001f)
	{
		if (__fabs(feet.x - head.x) < HERO_RADIUS)
		{
			// vertical segment
			if (__fabs(feet.x) < HERO_RADIUS)
			{
				p0 = feet;
				p1 = head;
			}
			else
				intersection = false;
		}
		else if ((feet.x < 0) != (head.x < 0))
		{
			// compute intercept point against the YZ plane
			vector3d intercept(0, 0, 0);
			rational_t m = (feet.y - head.y) / (feet.x - head.x);
			intercept.y = feet.x - (m * feet.y);
			m = (feet.z - head.z) / (feet.y - head.y);
			intercept.z = feet.y - (m * feet.z);

			// determine the segment by turning the hero segment into a cylinder
			vector3d diff(head.y - feet.y, feet.x - head.x, 0);
			diff.normalize();
			diff *= HERO_RADIUS;

			// and compute the two intercepts against the YZ plane
			p0 = intercept + diff;
		  p1 = intercept - diff;
			p0.x = p1.x = 0;                // flatten into YZ plane
		}
		else
			intersection = false;

		if (intersection)
		{
		  // angle clip
			rational_t a0 = atan2(p0.y, p0.z);	// fix trig (dc 08/16/01)
			rational_t a1 = atan2(p1.y, p1.z);

		  bool out0 = (__fabs(a0) > aperture) || (p0.z < 0.01f);
		  bool out1 = (__fabs(a1) > aperture) || (p1.z < 0.01f);

      do
      {
		    if (out0 && out1)
		    {
			    // check for case where both verts are completely off to one side
			    if ((a0 < 0) == (a1 < 0))
          {
            intersection = false;
				    continue;
          }

			    // check for case where either vert is out of range, but some portion of the segment is in range
			    vector3d i0, i1;
			    if (!intersect_yz_segment(p0, p1, vector3d(0, 0, 0), vector3d(0, sin_aperture, cos_aperture), i0) ||
			        !intersect_yz_segment(p0, p1, vector3d(0, 0, 0), vector3d(0, -sin_aperture, cos_aperture), i1))
          {
            intersection = false;
				    continue;
          }

			    // if we get to here, part of the segment was in range
			    p0 = i0;
			    p1 = i1;

			    // recompute angles
			    a0 = atan2(p0.y, p0.z);	// fix trig (dc 08/16/01)
			    a1 = atan2(p1.y, p1.z);
		    }
		    else if (out0 || out1)
		    {
			    // one vert or the other is outside, so clip it to the scanner's aperture
			    vector3d i0;
			    if (!intersect_yz_segment(p0, p1, vector3d(0, 0, 0), vector3d(0, sin_aperture, cos_aperture), i0))
				    if (!intersect_yz_segment(p0, p1, vector3d(0, 0, 0), vector3d(0, -sin_aperture, cos_aperture), i0))
            {
              intersection = false;
					    continue;
            }

			    // set the NEW vert depending on which one was outside
			    if (out0)
			    {
				    p0 = i0;
				    a0 = atan2(p0.y, p0.z);	// fix trig (dc 08/16/01)
			    }
			    else
			    {
				    p1 = i0;
				    a1 = atan2(p1.y, p1.z);	// fix trig (dc 08/16/01)
			    }
		    }
      } while (0);          // one-time loop, just used so that we could hop out without using gotos

      if (intersection)
      {
			  if (a0 > a1)
			  {
				  // switch them around so that it's always a0 -> a1
				  rational_t tmp = a0; a0 = a1; a1 = tmp;
				  vector3d tmp2(p0); p0 = p1; p1 = tmp2;
			  }

			  rational_t d0 = p0.length2();
			  rational_t d1 = p1.length2();

			  // run the check
        vector3d check_vec = vector3d(0, 0, -1);
			  if (check_segment(p0, p1, check_vec, a0, a1, d0, d1, -1, -1, false))
				  hit = true;
			  else
			  {
				  // if any pending checks are left, there's a chance that we still have a hit
				  while (pending_check)
				  {
					  segment *old_list = pending_check;
					  pending_check = NULL;
					  for (s = old_list; s; s = next)
					  {
						  next = s->next;
						  if (!hit && check_segment(s->p0, s->p1, s->normal, s->a0, s->a1, s->d0, s->d1, s->list, s->poly, false))
							  hit = true;
						  delete s;
					  }
				  }
        }
			}

			// throw these lists away, we don't care about the actual segments generated
			for (s = pending; s; s = next)
				{	next = s->next;	delete s; }

			for (s = pending_check; s; s = next)
				{	next = s->next;	delete s; }

			pending = pending_check = NULL;
		}
	}

  return(hit);
}


void scanner::concatenate_segments()
{
  segment * s;
	for (s = segments; s; s = s->next)
  {
    if (s->next && s->p1 == s->next->p0 && s->normal==s->next->normal)
    {
      segment * skip = s->next;
      s->p1 = s->next->p1;
      s->next = s->next->next;
      delete skip;
    }
  }
}

extern profiler_timer proftimer_scanner_render;
//profiler_counter profcounter_scanner_render("Scanner render",NULL,0);

void scanner::render( camera *camera_link, rational_t detail, render_flavor_t flavor, rational_t entity_translucency_pct)
{
#ifdef TARGET_PC
  proftimer_scanner_render.start();
#endif // TARGET_PC
  //profcounter_scanner_render.add_count(1);

  if ( is_active() && (flavor&RENDER_TRANSLUCENT_PORTION) )
  {
		geometry_manager::inst()->set_xform(geometry_manager::XFORM_LOCAL_TO_WORLD, identity_matrix);
		segment *s;

    // Fixed so that original color is preserved! (JDB) 5/11/00
    color32 oldCol;
    if ( contact )
    {
      oldCol = my_color;

      my_color.c.r = (uint8)(191 + (64.0f * __fabs(contact_color_ramp - 1.0f)));
      my_color.c.g = 0;
      my_color.c.b = 0;
    }

    render_sheet(detail, flavor, entity_translucency_pct * 0.25f);

		for (s = segments; s; s = s->next)
			render_line(s->p0 + sheet_offset, s->p1 + sheet_offset, thickness, s->normal, flavor, entity_translucency_pct);

    if ( contact )
    {
      my_color = oldCol;
    }
	}

	// In case there is a normal pmesh...
#ifdef TARGET_PC
	if ( my_visrep != the_pmesh )
		entity::render( detail, flavor, entity_translucency_pct );
#else
		entity::render( detail, flavor, entity_translucency_pct );
#endif

#ifdef TARGET_PC
  proftimer_scanner_render.stop();
#endif // TARGET_PC
}

#include "vertwork.h"
#include "forceflags.h"

void scanner::render_sheet(rational_t detail, render_flavor_t flavor, rational_t entity_translucency_pct)
{
#if defined(NGL)

  segment * s;

  int n = 0;
  for (s = segments; s; s = s->next) ++n;

  if( n )
  {
	assert(false);	// If we use ScratchMesh here, we should make it locked (dc 08/23/01)
    KSNGL_CreateScratchMesh( n*3 );
//    scratchmesh->Material.MapBlendMode = NGLBM_BLEND;

    vector3d rel_cam = get_abs_po().inverse_xform(g_game_ptr->get_current_view_camera()->get_abs_position());

    for (s = segments; s; s = s->next)
    {
      nglMeshWriteStrip( 3 );
      nglMeshWriteVertexPC( sheet_offset.x, sheet_offset.y, sheet_offset.z, 0x80808080 );

      vector3d p0 = s->p0+sheet_offset;
      vector3d p1 = s->p1+sheet_offset;
      if (rel_cam.x<0)
      {
        nglMeshWriteVertexPC( p0.x, p0.y, p0.z, 0x80808080 );
        nglMeshWriteVertexPC( p1.x, p1.y, p1.z, 0x80808080 );
      }
      else
      {
        nglMeshWriteVertexPC( p1.x, p1.y, p1.z, 0x80808080 );
        nglMeshWriteVertexPC( p0.x, p0.y, p0.z, 0x80808080 );
      }
    }

    START_PROF_TIMER( proftimer_render_add_mesh );
    nglListAddMesh( nglCloseMesh(), native_to_ngl( get_abs_po() ), NULL);
    STOP_PROF_TIMER( proftimer_render_add_mesh );
  }

#else

  segment * s;

  static uint16 render_indices[2048];

  int n = 0;
  for (s = segments; s; s = s->next) ++n;

  assert((n*3) < 2048);

  if( n )
  {
    vert_workspace.lock(n*2+1);
    hw_rasta_vert_lit* vert_it = (hw_rasta_vert_lit*)vert_workspace.begin();

    vector3d rel_cam = get_abs_po().inverse_xform(g_game_ptr->get_current_view_camera()->get_abs_position());
    matrix4x4 render_mat = get_abs_po().get_matrix();

    vector3d sheet = xform3d_1(render_mat, sheet_offset);

    vert_it->set_xyz(sheet);
    vert_it->diffuse = my_color;
    vert_it->tc[0] = texture_coord(0.5f,0.5f);
    ++vert_it;

    int i = 1;
    int counter = 0;

    for (s = segments; s; s = s->next)
    {
      vector3d p0 = s->p0+sheet_offset;
      p0 = xform3d_1(render_mat, p0);

      vector3d p1 = s->p1+sheet_offset;
      p1 = xform3d_1(render_mat, p1);

      render_indices[counter] = 0;
      ++counter;

      if (rel_cam.x<0)
      {
        vert_it->set_xyz(p0);
        vert_it->diffuse = my_color;
        vert_it->tc[0] = texture_coord(0.5f,0.5f);
        render_indices[counter] = i;
        ++i;
        ++vert_it;
        ++counter;

        vert_it->set_xyz(p1);
        vert_it->diffuse = my_color;
        vert_it->tc[0] = texture_coord(0.5f,0.5f);
        render_indices[counter] = i;
        ++i;
        ++vert_it;
        ++counter;
      }
      else
      {
        vert_it->set_xyz(p1);
        vert_it->diffuse = my_color;
        vert_it->tc[0] = texture_coord(0.5f,0.5f);
        render_indices[counter] = i;
        ++i;
        ++vert_it;
        ++counter;

        vert_it->set_xyz(p0);
        vert_it->diffuse = my_color;
        vert_it->tc[0] = texture_coord(0.5f,0.5f);
        render_indices[counter] = i;
        ++i;
        ++vert_it;
        ++counter;
      }
    }

    vert_workspace.unlock();

    // send it to the card (using list because it could be double-sided)
    app::inst()->get_game()->get_blank_material()->send_context(0, MAP_DIFFUSE, FORCE_TRANSLUCENCY);
    hw_rasta::inst()->send_indexed_vertex_list(vert_workspace, n*2+1,
                                               render_indices, n*3);

  }
/*
  segment * s;

  int n = 0;
  for (s = segments; s; s = s->next) ++n;

  if( n )
  {
    sheet_pmesh->make_n_tangle(n);

    sheet_pmesh->shrink_memory_footprint();

    int i3;
    vector3d rel_cam = get_abs_po().inverse_xform(g_game_ptr->get_current_view_camera()->get_abs_position());

    for (i3 = 0, s = segments; s; i3 += 3, s = s->next)
    {
      if (rel_cam.x<0)
      {
        sheet_pmesh->set_xvert_unxform_pos(0+i3, sheet_offset);
        sheet_pmesh->set_xvert_unxform_pos(1+i3, s->p0+sheet_offset);
        sheet_pmesh->set_xvert_unxform_pos(2+i3, s->p1+sheet_offset);
      }
      else
      {
        sheet_pmesh->set_xvert_unxform_pos(0+i3, sheet_offset);
        sheet_pmesh->set_xvert_unxform_pos(2+i3, s->p0+sheet_offset);
        sheet_pmesh->set_xvert_unxform_pos(1+i3, s->p1+sheet_offset);
      }

      sheet_pmesh->set_xvert_unxform_diffuse(0+i3, my_color);
      sheet_pmesh->set_xvert_unxform_diffuse(1+i3, my_color);
      sheet_pmesh->set_xvert_unxform_diffuse(2+i3, my_color);
      sheet_pmesh->set_has_translucent_verts(my_color.c.a < 255);
    }

    visual_rep *old_vr = my_visrep;
    my_visrep = sheet_pmesh;

    entity::render( n, flavor | RENDER_NO_LIGHTS, entity_translucency_pct );

    my_visrep = old_vr;
  }
*/
#endif
}

//profiler_timer proftimer_scanner_render_line("lines",&proftimer_scanner_render);
//profiler_counter profcounter_scanner_render_line("Scanner render line",NULL,0);

// This function actually renders a quad lying along the vector from v0 to v1, with a thickness
// as specified and 'painted' on via the given normal.
void scanner::render_line(vector3d v0, vector3d v1, rational_t thickness, vector3d normal, render_flavor_t flavor, rational_t entity_translucency_pct)
{
  //proftimer_scanner_render_line.start();
  //profcounter_scanner_render_line.add_count(1);

	vector3d v(v1 - v0);
	v.normalize();

	vector3d xfacing=cross(normal, v);       // cross product
	vector3d offset(xfacing * (thickness * 0.5F));

	// offset it just a tad from the surface
#define POLYGON_OFFSET   0.003f

	vector3d surface_offset = normal * POLYGON_OFFSET;
	v0 += surface_offset;
	v1 += surface_offset;

	render_quad(v1 - offset, v1 + offset, v0 + offset, v0 - offset, flavor, entity_translucency_pct);

  //proftimer_scanner_render_line.stop();
}

//profiler_timer proftimer_scanner_render_quad("quads",&proftimer_scanner_render);
//profiler_counter profcounter_scanner_render_quad("Scanner render quad",NULL,0);

void scanner::render_quad(vector3d v0, vector3d v1, vector3d v2, vector3d v3, render_flavor_t flavor, rational_t entity_translucency_pct)
{
#pragma todo("Eliminate PC side when nglScratchMesh is supported by directX.  jdf  3-21-01")
#ifdef NGL
  assert(false);	// If we use ScratchMesh here, we should make it locked (dc 08/23/01)
  KSNGL_CreateScratchMesh( 4 );
//  scratchmesh->Material.MapBlendMode = NGLBM_BLEND;

  nglMeshWriteStrip( 4 );
  nglMeshWriteVertexPC( v0.x, v0.y, v0.z, 0x80808080 );
  nglMeshWriteVertexPC( v1.x, v1.y, v1.z, 0x80808080 );
  nglMeshWriteVertexPC( v2.x, v2.y, v2.z, 0x80808080 );
  nglMeshWriteVertexPC( v3.x, v3.y, v3.z, 0x80808080 );

  START_PROF_TIMER( proftimer_render_add_mesh );
  nglListAddMesh( nglCloseMesh(), native_to_ngl( get_abs_po() ), NULL);
  STOP_PROF_TIMER( proftimer_render_add_mesh );
#else
  //proftimer_scanner_render_quad.start();
  //profcounter_scanner_render_quad.add_count(1);

	visual_rep *old_vr = my_visrep;
	my_visrep = the_pmesh;

	assert(get_vrep()->get_type() == VISREP_PMESH);
	vr_pmesh *my_pmesh = static_cast<vr_pmesh*>(get_vrep());
	my_pmesh->set_xvert_unxform_pos(0, v0);
	my_pmesh->set_xvert_unxform_pos(1, v1);
	my_pmesh->set_xvert_unxform_pos(2, v2);
	my_pmesh->set_xvert_unxform_pos(3, v3);
	my_pmesh->set_xvert_unxform_diffuse(0, my_color);
	my_pmesh->set_xvert_unxform_diffuse(1, my_color);
	my_pmesh->set_xvert_unxform_diffuse(2, my_color);
	my_pmesh->set_xvert_unxform_diffuse(3, my_color);
	my_pmesh->set_has_translucent_verts(my_color.c.a < 255);

	entity::render( 2, flavor | RENDER_NO_LIGHTS, entity_translucency_pct );

	my_visrep = old_vr;
  //proftimer_scanner_render_quad.stop();
#endif
}

rational_t scanner::get_visual_radius() const
{
	return max_length;
}

void scanner::set_active( bool a )
{
  entity::set_active( a );
  // a scanner being turned off will lose contact with the hero
  if ( !a )
    set_contact( false );
}


//profiler_timer proftimer_scanner_clear("clear",&proftimer_scanner_advance);
//profiler_counter profcounter_scanner_clear("Scanner clear",NULL,0);

void scanner::clear_poly_list()
{
  //proftimer_scanner_clear.start();
  //profcounter_scanner_clear.add_count(1);
  for (int i = 0; i < num_lists; ++i)
    delete [] poly_lists[i].polys;

	delete [] poly_lists;
  poly_lists = NULL;
	num_lists = 0;
  //proftimer_scanner_clear.stop();
}

//profiler_timer proftimer_scanner_build("build",&proftimer_scanner_advance);
//profiler_counter profcounter_scanner_build("Scanner build",NULL,0);

void scanner::build_poly_list(bool optimize)
{
  //proftimer_scanner_build.start();
  //profcounter_scanner_build.add_count(1);
  set_flag(OPTIMIZED, optimize);

  clear_poly_list();

#ifndef NGL
	// First pass: count the number of lists

  build_region_list_radius(&scan_regs, get_primary_region(), get_abs_position(), max_length);

  vector<region_node*>::iterator ri = scan_regs.begin();
  vector<region_node*>::iterator ri_end = scan_regs.end();
/*
  region_node_pset::const_iterator ri = get_regions().begin();
  region_node_pset::const_iterator ri_end = get_regions().end();
*/

  region::entity_list::const_iterator ei;
  region::entity_list::const_iterator ei_end;
	for ( ; ri!=ri_end; ++ri )
	{
		// region will be one or more lists
		region *r = (*ri)->get_data();
#if defined(NGL)
    num_lists += r->get_num_meshes();
#else
    num_lists += r->get_num_visreps();
#endif

		// entities flagged as scanable within the region
		ei = r->get_entities().begin();
		ei_end = r->get_entities().end();
		for ( ; ei!=ei_end; ++ei )
		{
			entity *e = *ei;
			if ( e
        && e->is_scannable()
        && e->is_visible()
        && !e->is_destroyable()
        && !e->is_a_crate()
        )
      {
        ++num_lists;
      }
		}
	}

	// Allocate the list
  poly_lists = NEW poly_list[num_lists];

	// Second pass: store the polys
	int list = 0;
	ri = scan_regs.begin();
	ri_end = scan_regs.end();
/*
	ri = get_regions().begin();
	ri_end = get_regions().end();
*/
  for ( ; ri!=ri_end; ++ri)
	{
		// terrain mesh for region
		region *r = (*ri)->get_data();
    int vri;

#if defined(NGL)
    for ( vri=0; vri<r->get_num_meshes(); ++vri )
    {
		  nglMesh *mesh = r->get_mesh(vri);
/*
      poly_lists[list].xform_po = NULL;          // terrain doesn't need to be xformed
      poly_lists[list].num_polys = mesh->get_max_faces();
          // this takes more memory than it should
      poly_lists[list].polys = NEW polygon[mesh->get_max_faces()];

      int f, v;
		  for ( f=v=0; f<poly_lists[list].num_polys; ++f, ++v )
		  {
        poly_lists[list].polys[v].verts[0] = mesh->get_xvert_unxform(mesh->get_wedge_ref(f, 0));
        poly_lists[list].polys[v].verts[1] = mesh->get_xvert_unxform(mesh->get_wedge_ref(f, 1));
        poly_lists[list].polys[v].verts[2] = mesh->get_xvert_unxform(mesh->get_wedge_ref(f, 2));
			  poly_lists[list].polys[v].normal = cross(poly_lists[list].polys[v].verts[1] - poly_lists[list].polys[v].verts[0], poly_lists[list].polys[v].verts[2] - poly_lists[list].polys[v].verts[0]);
			  poly_lists[list].polys[v].normal.normalize();
      }
*/
      ++list;
    }
#else
    for ( vri=0; vri<r->get_num_visreps(); ++vri )
    {
		  vr_pmesh *mesh = static_cast<vr_pmesh *>(r->get_visrep(vri));
      poly_lists[list].xform_po = NULL;          // terrain doesn't need to be xformed
      poly_lists[list].num_polys = mesh->get_max_faces();
          // this takes more memory than it should
      poly_lists[list].polys = NEW polygon[mesh->get_max_faces()];

      int f, v;
		  for ( f=v=0; f<poly_lists[list].num_polys; ++f, ++v )
		  {
        poly_lists[list].polys[v].verts[0] = mesh->get_xvert_unxform(mesh->get_wedge_ref(f, 0));
        poly_lists[list].polys[v].verts[1] = mesh->get_xvert_unxform(mesh->get_wedge_ref(f, 1));
        poly_lists[list].polys[v].verts[2] = mesh->get_xvert_unxform(mesh->get_wedge_ref(f, 2));
			  poly_lists[list].polys[v].normal = cross(poly_lists[list].polys[v].verts[1] - poly_lists[list].polys[v].verts[0], poly_lists[list].polys[v].verts[2] - poly_lists[list].polys[v].verts[0]);
			  poly_lists[list].polys[v].normal.normalize();
      }

      ++list;
    }
#endif

		// entities flagged as scannable within the region
		ei = r->get_entities().begin();
		ei_end = r->get_entities().end();
		for ( ; ei!=ei_end; ++ei )
		{
			entity *e = *ei;
			if ( e
        && e->is_scannable()
        && e->is_visible()
        && !e->is_destroyable()
        && !e->is_a_crate()
        )
			{
#pragma todo("Needs PS2 equivalent.  jdf 3-21-01")
#if defined(NGL)
      	assert( e->get_mesh() );
				nglMesh* mesh = e->get_mesh();

/*
        poly_lists[list].xform_po = &e->get_abs_po();
        poly_lists[list].num_polys = mesh->get_max_faces();
        poly_lists[list].polys = NEW polygon[mesh->get_max_faces()];

				for (int f = 0; f < mesh->get_max_faces(); ++f)
				{
					poly_lists[list].polys[f].verts[0] = mesh->get_xvert_unxform(mesh->get_wedge_ref(f, 0));
					poly_lists[list].polys[f].verts[1] = mesh->get_xvert_unxform(mesh->get_wedge_ref(f, 1));
					poly_lists[list].polys[f].verts[2] = mesh->get_xvert_unxform(mesh->get_wedge_ref(f, 2));
					poly_lists[list].polys[f].normal = cross(poly_lists[list].polys[f].verts[1] - poly_lists[list].polys[f].verts[0], poly_lists[list].polys[f].verts[2] - poly_lists[list].polys[f].verts[0]);
				}
*/
#else
      	assert( e->get_vrep() && e->get_vrep()->get_type()==VISREP_PMESH );
				vr_pmesh* mesh = static_cast<vr_pmesh *>(e->get_vrep());

        poly_lists[list].xform_po = &e->get_abs_po();
        poly_lists[list].num_polys = mesh->get_max_faces();
        poly_lists[list].polys = NEW polygon[mesh->get_max_faces()];

				for (int f = 0; f < mesh->get_max_faces(); ++f)
				{
					poly_lists[list].polys[f].verts[0] = mesh->get_xvert_unxform(mesh->get_wedge_ref(f, 0));
					poly_lists[list].polys[f].verts[1] = mesh->get_xvert_unxform(mesh->get_wedge_ref(f, 1));
					poly_lists[list].polys[f].verts[2] = mesh->get_xvert_unxform(mesh->get_wedge_ref(f, 2));
					poly_lists[list].polys[f].normal = cross(poly_lists[list].polys[f].verts[1] - poly_lists[list].polys[f].verts[0], poly_lists[list].polys[f].verts[2] - poly_lists[list].polys[f].verts[0]);
				}
#endif
        ++list;
			}
		}
	}

  // Third pass: Do a scan of our region, tag the polys we hit, and deactivate the rest
  for (int l = 0; l < num_lists; ++l)
    for (int p = 0; p < poly_lists[l].num_polys; ++p)
      if (optimize)
        poly_lists[l].polys[p].active = poly_lists[l].xform_po;
      else
        poly_lists[l].polys[p].active = true;

  if (optimize)
  {
    angle = 0;
    tag_mode = true;

    for (int sign_changes = 0; sign_changes < (speed < 0 ? 3 : 2); )
    {
      rational_t old_angle = angle;
      frame_advance(0.05f);
      if ((angle < 0) != (old_angle < 0))
        ++sign_changes;
    }

    angle = 0;
    tag_mode = false;
  }
#endif
  //proftimer_scanner_build.stop();
}

//profiler_timer proftimer_scanner_checkseg("check seg",&proftimer_scanner_advance);
//  profiler_timer proftimer_scanner_checkseg_pending("pending",&proftimer_scanner_checkseg);
//profiler_counter profcounter_scanner_checkseg("Scanner check seg",NULL,0);
//  profiler_counter profcounter_scanner_checkseg_pending("pending",&profcounter_scanner_checkseg,0);

bool scanner::check_segment(vector3d &p0, vector3d &p1, vector3d &normal, rational_t &a0, rational_t &a1, rational_t &d0, rational_t &d1, int list, int poly, bool occlude)
{
  //proftimer_scanner_checkseg.start();
  //profcounter_scanner_checkseg.add_count(1);
	// search the list for the proper place to insert it
	segment *s, *prev = NULL, *p_prev, *next;

	for (s = segments; s; prev = p_prev, s = next)
	{
		next = s->next;
		p_prev = s;

		// first check to see if they overlap at all
		if (a1 <= s->a0 || s->a1 <= a0)
			continue;

		// find the shared space between the two segments
		rational_t sh0 = max(a0, s->a0);
		rational_t sh1 = min(a1, s->a1);
		if (sh0 >= sh1 || DAMN_CLOSE(sh0, sh1))
			continue;         // no shared space, skip it

		// check for total occlusion of NEW segment
		if (a0 >= s->a0 && a1 <= s->a1)
		{
      if (!seg_in_front_of(p0, p1, d0, d1, s))
			{
				// it's behind the other segment, throw it out completely
        //proftimer_scanner_checkseg.stop();
				return false;
			}
			else
			{
				if (!occlude)
					continue;         // no occlusion of old segments?  just skip it, then

				// it's in front of the other segment
				if (DAMN_CLOSE(a0, s->a0) && DAMN_CLOSE(a1, s->a1))
				{
					// they match up, just throw out the old one
					if (prev)
						prev->next = next;
					else
						segments = next;
					delete s;
					p_prev = prev;
					continue;
				}

				// split the other segment into two pieces
				// generate a NEW top piece
				rational_t na0 = a1, na1 = s->a1;
				vector3d np0, np1(s->p1);

        rational_t sinx, cosx;
        fast_sin_cos_approx( na0, &sinx, &cosx );

				if (intersect_yz_segment(s->p0, s->p1, vector3d(0, 0, 0), vector3d(0, max_length * sinx, max_length * cosx), np0))
        {
          //proftimer_scanner_checkseg_pending.start();
          //profcounter_scanner_checkseg_pending.add_count(1);
					add_segment_pending(np0, np1, s->normal, na0, na1, np0.length2(), np1.length2(), s->list, s->poly);
          //proftimer_scanner_checkseg_pending.stop();
        }

				// shrink the old segment to make the bottom piece
				s->a1 = a0;
				vector3d i0;
        fast_sin_cos_approx( s->a1, &sinx, &cosx );
				if (!intersect_yz_segment(s->p0, s->p1, vector3d(0, 0, 0), vector3d(0, max_length * sinx, max_length * cosx), i0))
				{
					// bah, get rid of it then
					if (prev)
						prev->next = next;
					else
						segments = next;
					delete s;
					p_prev = prev;
					continue;
				}

				s->p1 = i0;
				s->d1 = p1.length2();

				continue;
			}
		}

		// check for total occlusion of other segment
		if (s->a0 >= a0 && s->a1 <= a1)
		{
      if (seg_in_front_of(p0, p1, d0, d1, s))
			{
				if (!occlude)
					continue;         // no occlusion of old segments?  just skip it, then

				// it's behind the NEW segment, remove it from the list and continue
				if (prev)
					prev->next = next;
				else
					segments = next;
				delete s;
				p_prev = prev;
				continue;
			}
			else
			{
				// it's in front of the NEW segment
				if (DAMN_CLOSE(a0, s->a0) && DAMN_CLOSE(a1, s->a1))
				{
					// they match up, just throw out the NEW segment
          //proftimer_scanner_checkseg.stop();
					return false;
				}

				// split the NEW segment into two pieces
				// generate a NEW top piece
				rational_t na0 = s->a1, na1 = a1;
				vector3d np0, np1(p1);

        rational_t sinx, cosx;
        fast_sin_cos_approx( na0, &sinx, &cosx );

				if (intersect_yz_segment(p0, p1, vector3d(0, 0, 0), vector3d(0, max_length * sinx, max_length * cosx), np0))
        {
          //proftimer_scanner_checkseg_pending.start();
          //profcounter_scanner_checkseg_pending.add_count(1);
					add_segment_pending_check(np0, np1, normal, na0, na1, np0.length2(), np1.length2(), list, poly);
          //proftimer_scanner_checkseg_pending.stop();
        }

				// shrink the segment that we're currently checking to make the bottom piece
				a1 = s->a0;
				vector3d i0;
        fast_sin_cos_approx( a1, &sinx, &cosx );
				if (!intersect_yz_segment(p0, p1, vector3d(0, 0, 0), vector3d(0, max_length * sinx, max_length * cosx), i0))
				{
					// bah, throw it out
          //proftimer_scanner_checkseg.stop();
					return false;
				}

				p1 = i0;
				d1 = p1.length2();

				continue;
			}
		}

		// build a point which is located in the middle of the shared space
    vector3d middle;
		if (sh0 == a0)
			middle = (p0 + s->p1) * 0.5f;
		else
			middle = (p1 + s->p0) * 0.5f;

		// the only case that is left is partial occlusion (top or bottom) of one of the segments
		if (!seg_in_front_of(p0, p1, d0, d1, s))
		{
			// NEW segment is behind the other segment, clip NEW segment
			if (a0 > s->a0)
			{
				// clip off the bottom of the NEW segment
				a0 = sh1;
				vector3d i0;
        rational_t sinx, cosx;
        fast_sin_cos_approx( a0, &sinx, &cosx );
				intersect_yz_segment(p0, p1, vector3d(0, 0, 0), vector3d(0, max_length * sinx, max_length * cosx), i0);
				p0 = i0;
				d0 = p0.length2();
			}
			else
			{
				// clip off the top of the NEW segment
				a1 = sh0;
				vector3d i0;
        rational_t sinx, cosx;
        fast_sin_cos_approx( a1, &sinx, &cosx );
				intersect_yz_segment(p0, p1, vector3d(0, 0, 0), vector3d(0, max_length * sinx, max_length * cosx), i0);
				p1 = i0;
				d1 = p1.length2();
			}
		}
		else
		{
			if (!occlude)
				continue;         // no occlusion of old segments?  just skip it, then

			// NEW segment is in front of the other segment, clip other segment
			if (s->a0 > a0)
			{
				// clip off the bottom of the other segment
				s->a0 = sh1;
				vector3d i0;
        rational_t sinx, cosx;
        fast_sin_cos_approx( s->a0, &sinx, &cosx );
				intersect_yz_segment(s->p0, s->p1, vector3d(0, 0, 0), vector3d(0, max_length * sinx, max_length * cosx), i0);
				s->p0 = i0;
				s->d0 = s->p0.length2();
			}
			else
			{
				// clip off the top of the other segment
				s->a1 = sh0;
				vector3d i0;
        rational_t sinx, cosx;
        fast_sin_cos_approx( s->a1, &sinx, &cosx );
				intersect_yz_segment(s->p0, s->p1, vector3d(0, 0, 0), vector3d(0, max_length * sinx, max_length * cosx), i0);
				s->p1 = i0;
				s->d1 = s->p1.length2();
			}

			// it's possible that the segment is now zero size
			if (DAMN_CLOSE(s->a0, s->a1))
			{
				// if so, get rid of it
				if (prev)
					prev->next = next;
				else
					segments = next;
				delete s;
				p_prev = prev;
			}

			continue;
		}

		// check this every iteration because it can change each time
		if (DAMN_CLOSE(a0, a1))
    {
      //proftimer_scanner_checkseg.stop();
			return false;         // gap has closed to nothing
    }
	}

  //proftimer_scanner_checkseg.stop();
	return true;
}

void scanner::reset()
{
  angle = 0;

  speed = __fabs(speed);

  if(is_flagged(REVERSE))
    speed *= -1.0f;
}

//---------------------------------------------------------------
void scanner::INC_detector()
{
  if ( detected_count == 0 )
  {
//    g_sound_group_list.play_sound_group_looping( "ScannerAlarm" );
  }
  ++detected_count;
}

void scanner::DEC_detector()
{
  --detected_count;
  if ( detected_count == 0 )
  {
//    g_sound_group_list.kill_sound_group_looping( "ScannerAlarm" );
  }
}


void scanner::set_contact( bool _contact )
{
  if ( contact != _contact )
  {
    contact = _contact;
    if ( contact )
    {
      raise_signal( ENTER );
      INC_detector();
    }
    else
    {
      raise_signal( LEAVE );
      DEC_detector();
    }
  }
}

void scanner::set_ai_owner(ai_interface *own)
{
  owner = own;
}

void scanner::set_visible(bool a)
{
  bool changed = entity::is_visible() != a;
  entity::set_visible(a);
  if(changed)
    entity::region_update_poss_active();
}
