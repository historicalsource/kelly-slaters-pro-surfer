////////////////////////////////////////////////////////////////////////////////
//
//camera.h
//
//  A camera
//  
//	It's an entity so we can move it around using the same engine
//	as everything else.  Which is very beautiful.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef CAMERA_H
#define CAMERA_H

#include "algebra.h"
#include "po.h"
#include "entity.h"
#include "mic.h"
#include "constants.h"

class kellyslater_controller;

// the basic camera, which stays relative to its parent entity
class camera : public entity
{
protected:
    mic *microphone;
	
public:
    camera( entity* _parent, 
		const entity_id& entity, 
		entity_flavor_t _flavor = ENTITY_CAMERA);
    virtual ~camera();
	
	/////////////////////////////////////////////////////////////////////////////
	// entity class identification
public:
    virtual bool is_a_camera() const { return true; }
	
public:
    virtual void sync( camera& b );
	virtual void Reset() { }
	
    // apply the transform this camera calls for to geometry pipe
    // only done for one camera per viewport per frame
    // scene_analyzer is used to set the scene analyzer position instead of the camera.
    void adjust_geometry_pipe( bool scene_analyzer = false );
	
    void get_look_and_up(vector3d *look, vector3d *up);
	
    void set_externally_controlled( bool torf ){ entity::set_externally_controlled( torf );}
    bool is_externally_controlled(){ return entity::get_externally_controlled();}
	
    mic *get_microphone() { return(microphone); }
};

inline camera* find_camera( const entity_id& id )
{
	return (camera*)entity_manager::inst()->find_entity( id, ENTITY_CAMERA );
}


class game_camera : public camera
{
public:
    game_camera( const entity_id& entity, entity* _target_entity = NULL );
    virtual ~game_camera() {}
	
    virtual void sync( camera& b );
    virtual void frame_advance( time_value_t t );
	
    entity *get_target_entity() const { return target_entity; }
    void set_target_entity( entity* e ) { target_entity = e; }
	
    kellyslater_controller *get_ks_controller() {return ksctrl;}
    void set_ks_controller(kellyslater_controller *k) {ksctrl=k;}
	
    void invalidate( void ) { last_frame_valid = false; }
	virtual void init() {}

	virtual vector3d GetStartPosition(void) { return ZEROVEC; }
	
	/////////////////////////////////////////////////////////////////////////////
	// entity class identification
public:
    virtual bool is_a_game_camera() const { return true; }
	
protected:
    void blend(vector3d p0, vector3d p1, time_value_t t);
    void first_person_cam(entity *ch, time_value_t t);
    vector3d compute_entity_center( rational_t elevation );
    bool last_frame_valid;
	
    kellyslater_controller *ksctrl;
	
private:
    entity* target_entity;
    bool reset_old_elevation;
    vector3d targ_ent_pos;
    rational_t targ_ent_elev;
    rational_t last_frame_target_elev;
    rational_t last_frame_focus_y;
    bool temporary_lock;
    po ground_pitch_po;
    po last_frame_po;
    bool crawl_mode;
    bool crawl_mode_firstperson;
};

// Note: this is actually the SCENE camera
class marky_camera : public game_camera
{
public:
    marky_camera( const entity_id& id );
    virtual ~marky_camera() {}
	
	/////////////////////////////////////////////////////////////////////////////
	// entity class identification
public:
    virtual bool is_a_marky_camera() const { return true; }
	
public:
    virtual void frame_advance( time_value_t t );
    virtual void sync( camera& b );
	
    virtual void camera_set_target( const vector3d& pos );
    virtual vector3d camera_get_target( );
    virtual void camera_set_roll( rational_t angle );
    virtual void camera_set_collide_with_world( bool v ) { do_collide_with_world = v; }
	
    // logarithmically transitions to the target parameters.  returns true when there.
    virtual bool camera_slide_to( const vector3d& new_pos, const vector3d& new_target, rational_t new_roll, rational_t speed );
	
    // places the camera somewhere in a circle around the center, looking at the center.
    virtual bool camera_slide_to_orbit( const vector3d& center, rational_t range, rational_t theta, rational_t psi, rational_t speed );
    virtual void camera_orbit( const vector3d& center, rational_t range, rational_t theta, rational_t psi );
	
    rational_t get_roll()             { return(roll); }
	
    rational_t get_priority()         { return(current_priority); }
    void reset_priority()             { current_priority = (MIN_CAMERA_PRIORITY - 1.0f); }
    void set_priority(rational_t pr)  { current_priority = pr; }
	
private:
    // creates my_po from my_po's position component, target, and roll.
    void make_po();
	
    vector3d target;
    float roll;
    bool do_collide_with_world;
    vector3d last_frame_pos;
    rational_t current_priority;
};

class beach_camera;
class debug_camera;
class look_back_camera;
class replay_camera;
class old_shoulder_camera;
class shoulder_camera;
class stationary_camera;
class flyby_camera;
class wipeout_camera;
class follow_camera;
class follow_close_camera;
class buoy_camera;
class fps_camera;
#endif  // CAMERA_H
