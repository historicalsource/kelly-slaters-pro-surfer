#ifndef PS2_AUDIO_H
#define PS2_AUDIO_H                           
/*-------------------------------------------------------------------------------------------------------

  PS2_AUDIO.H - PS2 implementation of sound module.

-------------------------------------------------------------------------------------------------------*/
#include "algebra.h"
#include "singleton.h"
#include "pstring.h"
#include "gas.h"

#define MAX_STREAM_BUFFERS 10
typedef int sound_id_t;
const sound_id_t INVALID_SOUND_ID = -1;
const sound_id_t MAX_SOUND_ID     = 512;    // Also the number of sounds to allocate storage for.

// Pass this constant to play functions to make a sound loop infinitely.
const int SOUND_LOOP_INFINITE = -1;

// set this to 1 if you want to disable audio (except for initialization)
#define DISABLE_AUDIO 0

#define AUDIO_ENABLED ( !DISABLE_AUDIO \
                      && !os_developer_options::inst()->is_flagged(os_developer_options::FLAG_SAVE_LEVEL_IMAGES) \
                      && !os_developer_options::inst()->is_flagged(os_developer_options::FLAG_SAVE_RESTART_IMAGE) \
                      && !os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_SOUND) \
                      )

inline void check_playback_system_locked( const stringx& name ) {}

#if 0
class sound_data;
class sound_instance;
class sound_device;
class sound_emitter;


/*-------------------------------------------------------------------------------------------------------

  sound_stream

  This class provides a means for streaming large sounds from the storage device.  Only a the part of
  the sound currently being played stays in memory.

-------------------------------------------------------------------------------------------------------*/

class sound_stream
{
public:
  enum flags_t
  {
    PLAYING = 0x0001,
    LOOPING = 0x0002
  };

  static void advance_queue_requests() {}
  static bool streams_are_paused;

  sound_stream() {}
  sound_stream( const stringx& _file_name ) {}
  void construct();
	~sound_stream() {}

  // Make sure to call this when you're done with the stream.
 	void release() {}

  // VCR controls.
	void queue() {}  // call this .3s before you call start to avoid latency on startup.
	void start( bool _looping ) {}
	void stop(bool out_of_range_deactivation = false) {}
	
	// 0.0=silent to 1.0=full volume.
	void set_volume( rational_t _volume ) {}
  rational_t get_volume() const { return volume; }

  // State queries.
	bool is_playing() { return false; }		
	bool is_looping() { return false; }

  void set_flag( unsigned int f, bool v )
  {
    if ( v )
      flags |= f;
    else
      flags &= ~f;
  }

  bool is_flagged( unsigned int f ) const { return (flags & f); }
	
  // Set ranges for positional streams.
  void set_ranges( rational_t min, rational_t max ) {}
	
  stringx const & get_file_name() const { return file_name; }

  bool is_out_of_range_deactivated() const { return out_of_range_deactivated; }

//    void request_queue(bool auto_release = false);
  void request_queue(bool auto_release = false, bool looping = false) {}
  bool is_ready_to_play() const { return ready_to_play; } 

  bool is_playing_or_waiting_to_play() { return false; }

private:
//  static map<sound_stream *, bool> sound_stream::stream_queue_requests;
//  static list<sound_stream *> stream_queue_auto_release_requests;
  // State info.
	stringx file_name;
  bool ready_to_play;
  bool queue_requested;
  bool queued;
  unsigned int flags;
  rational_t volume;

  // Link to sound_emitter, 0 for sound_device.
  sound_emitter* emitter;

  // Positional info.
  rational_t min_dist, max_dist;
  rational_t positional_volume;
  bool out_of_range_deactivated;

  void stream_buffer_setup() {}

	friend class sound_device;
  friend class sound_emitter;
};


/*-------------------------------------------------------------------------------------------------------

  sound_instance

  This class is the interface to an instance of sound that can be played.  When a sound is to be played, 
  an instance is created through which the sound can be controlled.  

  When the instance is no longer needed, it should be released.  You should release instances as soon as 
  possible after you're done with them, since they occupy a hardware mixing channel as long as they exist.  

  If you don't need any control over a sound other than just starting it playing, use the play_sound 
  interface instead of the create_instance interface, since the instance created by play_sound is
  automatically released as soon as the sound finishes.

-------------------------------------------------------------------------------------------------------*/
class sound_instance
{
public:
  sound_instance();
  ~sound_instance();
  void initialize();      // resets all member data

  // NOTE: Call release() as soon as possible after you are done with the sound instance.
  void release();

  // VCR controls
  void play( bool looping = false );
  void stop();												// Note that stop doesn't rewind() the sound, just pauses it.
  void rewind();

  void dampen_guard(void);
  void dampen(void);
  void undampen(void);

  // get the buffers preloaded so we can start playback instantly, and a query fn for the preload
  void preload();
  bool is_ready();
  
  // Returns false if a sound instance has stopped either by reaching its end, by stop(), or if it hasn't been 
  // started.  Otherwise returns true.
  bool is_playing();

  // Position interface
  void set_pos( int _pos );
  int get_pos();

	rational_t get_scaled_pos();

  void set_volume( rational_t _volume );
  rational_t get_volume() const { return volume; }

  void set_angle( rational_t _angle ) { angle = _angle; }
  rational_t get_angle() { return angle; }

	void set_pitch( rational_t _pitch );
	rational_t get_pitch() const { return pitch; }

  void set_ranges(rational_t min, rational_t max);
	rational_t get_min_dist() const { return min_dist; }
	rational_t get_max_dist() const { return max_dist; }

  unsigned int get_id() { return(id); }

	int	dampen_count;

private:
  // Sound data object reference in case we lose the buffer and need to copy the data again.  
  sound_data * data;

  // Link to the sound emitter generating this sound instance, 0 if it's the sound_device.
  sound_emitter * emitter;    

  rational_t volume;
  rational_t positional_volume;

  rational_t last_positional_volume;
  rational_t last_angle;
	rational_t pitch;
  rational_t angle;  // 3d positional angle in RADIANS
  bool is_paused;
  bool loop;
  bool playback_begun;
  bool killed;
  bool user_stopped;
  bool stereo;
  short volume_left;  // derived values (from positional)
  short volume_right; 

  rational_t min_dist;
  rational_t max_dist;

  rational_t pan_left;  // these two should add up to 1.0f, or else
  rational_t pan_right; 

  int iop_instance_id; // internal id to the GAS IOP module
  unsigned int id;     // the id as the rest of the game knows it
  static unsigned int id_counter;

  bool check_internal_instance();

  friend class sound_data;
  friend class sound_device;
  friend class sound_emitter;
};

/*-------------------------------------------------------------------------------------------------------
  
  sound_data
  
  This class stores the data for a sound.  A single sound_data object can be shared by many sound_instance 
  objects, allowing the same sound to be played multiple times without allocating multiple copies of the 
  sound's data.

  Users of the sound module should never need to interface with this class, it's completely controlled by
  the sound_device and sound_instance.

-------------------------------------------------------------------------------------------------------*/
class sound_data
{
public:
  sound_data();
  ~sound_data();

  void load( const stringx& _file_name );
  void release();

  sound_instance * create_instance();
  void delete_instance( sound_instance * instance );

  pstring const & get_file_name() const { return file_name; }

	// public to allow for debugging routines.
//private:
  pstring file_name; 

  int iop_source_id;

  // Info for using this sound in a 3D environment.  This data comes from the WAV file's description string.
  rational_t      min_dist, max_dist;  // Distances in meters at which this sound is inaudible and at max volume.

  // List of instances of this sound.
  list<sound_instance *> instance_list;

  bool loaded;

  friend class sound_device;
  friend class sound_instance;
  friend class sound_emitter;
  friend void save_entire_state();
  friend void load_entire_state();
};


/*-------------------------------------------------------------------------------------------------------

  3d sound emitter class.

-------------------------------------------------------------------------------------------------------*/
class sound_emitter
{
public:
  sound_emitter();
  ~sound_emitter();
  void release();
  void initialize();

  // Position and speed of the sound for attenuation, pan, timing and doppler shift.
  void set_position( const vector3d & p );

  // Fire-and-forget(tm) playback interface.
  unsigned int play_sound( sound_id_t sound_id, rational_t vol = 1.0f, rational_t pitch = 1.0f, rational_t min_dist = -1.0f, rational_t max_dist = -1.0f );
	unsigned int play_sound( const stringx& name, rational_t vol = 1.0f, rational_t pitch = 1.0f, rational_t min_dist = -1.0f, rational_t max_dist = -1.0f );
	unsigned int play_sound( const pstring& name, rational_t vol = 1.0f, rational_t pitch = 1.0f, rational_t min_dist = -1.0f, rational_t max_dist = -1.0f );

  // Kill a playing sound
  bool kill_sound(unsigned int id);
  bool sound_playing(unsigned int id);

  // Create an instance of a sound for playback, etc.  Don't forget to release() it when you're done.
  sound_instance* create_sound( sound_id_t sound_id );
	sound_instance* create_sound( const stringx& name );
	sound_instance* create_sound( const pstring& name );

  // Create a positional stream for playback.  Don't forget to release() it when you're done.
  sound_stream * create_stream( const stringx & file_name ) { return 0; }

  int num_sounds_playing() const  { return(sounds_playing); }
  bool in_use() const             { return used; }

private:
  vector3d position;
  vector3d velocity;

  // Sounds currently playing through this emitter.
  list<sound_instance *> sound_list;

  bool used;
  bool waiting_for_action;
  bool automatic;

	// Storage for created streams.
  int sounds_playing;
int auto_count;

  void update_sounds();

  friend class sound_data;
  friend class sound_stream;
  friend class sound_device;
  friend class sound_instance;
};

/*-------------------------------------------------------------------------------------------------------

  sound_device

  This singleton represents the sound output device currently in use by the application.  It provides an
  interface for loading, playing and releasing sounds, and setting the 3d properties of the sound listener.
  
  Note that the playback interface within sound_device is for playing sounds without any 3d properties.
  To play sounds with 3d effects, create a sound_emitter object and play sounds through that.

-------------------------------------------------------------------------------------------------------*/
class sound_device : public singleton
{
public:
  sound_stream dummy_sound_stream;

  sound_device();
  ~sound_device();
  DECLARE_SINGLETON( sound_device )

  void init();
  void shutdown();

	// half versions of the above that don't unload sounds or release lists.
  void partial_shutdown();
  void partial_init();

  // Dump everything, tabula rasa, sounds of silence, leave none standing, blablabla
  void clear();

  // Releases all sound_data's and sound_instances.
  void release_all_sounds();

  // Load from disk.  Note that no file extension or path should be specified.  Example: load_sound( "jump" );
  sound_id_t load_sound( const stringx & file_name );
  void set_sound_ranges( sound_id_t id, rational_t min, rational_t max );

	// Returns the sound ID from a file name.  If fatal is true, the function generates an error if the sound isn't
	// loaded.  Otherwise, it returns -1.
  sound_id_t get_sound_id( const pstring& name, bool fatal = true );

  // Fire-and-forget(tm) playback interface.
  void play_sound( sound_id_t id, rational_t vol = 1.0f, rational_t pitch = 1.0f );
	void play_sound( const stringx& name, rational_t vol = 1.0f, rational_t pitch = 1.0f )
	{
    filespec sp(name);
    pstring snd( sp.name );
		play_sound( get_sound_id( snd ), vol, pitch );
	}
	void play_sound( const pstring& name, rational_t vol = 1.0f, rational_t pitch = 1.0f )
	{
		play_sound( get_sound_id( name ), vol, pitch );
	}

  // 3D Fire-and-forget(tm) playback interface.
  sound_emitter* play_3d_sound( sound_id_t id, const vector3d& pos, rational_t vol = 1.0f, rational_t pitch = 1.0f, rational_t min = -1.0f, rational_t max = -1.0f );
	sound_emitter* play_3d_sound( const stringx& name, const vector3d& pos, rational_t vol = 1.0f, rational_t pitch = 1.0f, rational_t min = -1.0f, rational_t max = -1.0f )
	{
    filespec sp(name);
    pstring snd( sp.name );
		return play_3d_sound( get_sound_id( snd ), pos, vol, pitch, min, max );
	}
	sound_emitter* play_3d_sound( const pstring& name, const vector3d& pos, rational_t vol = 1.0f, rational_t pitch = 1.0f, rational_t min = -1.0f, rational_t max = -1.0f )
	{
		return play_3d_sound( get_sound_id( name ), pos, vol, pitch, min, max );
	}

  // Create an instance of a sound for playback, etc.  Don't forget to release() it when you're done.
  sound_instance* create_sound( sound_id_t sound_id );
	sound_instance* create_sound( const stringx& name )
	{
    filespec sp(name);
    pstring snd( sp.name );
		return create_sound( get_sound_id( snd ) );
	}
	sound_instance* create_sound( const pstring& name )
	{
		return create_sound( get_sound_id( name ) );
	}

  // Create a new sound emitter for playing 3D sounds.
  sound_emitter * create_emitter(const vector3d &pos = ZEROVEC);
  void frame_advance(const time_value_t &time_inc);

  // Listener control
  void set_listener_position( const vector3d & p );
  void set_listener_velocity( const vector3d & v );
  void set_listener_orientation( const vector3d & front, const vector3d & top );

  void kill_sound() {}
  void kill_sound( sound_id_t sound_id ) {}

  bool get_stereo_enabled() const { return true; }
  void set_stereo_enabled(bool stereo) {}  

  void release_instance(sound_instance* si);
  list<sound_emitter *>::iterator  release_emitter(sound_emitter* se);
  void kill_death_row_instances();

  void stop_all_instances();

	void set_dampen_value( int new_value );
  int get_dampen_value();
  void dampen_all_instances();
  void undampen_all_instances();

  void set_master_volume(rational_t new_volume);

  // Stream control
	sound_stream* create_stream( const stringx& file_name ) { return &dummy_sound_stream; }
	void update_streams() {}

  void pause_all_streams() {}
  void resume_all_streams() {}

  unsigned long get_used_audio_memory() const { return used_audio_memory; }

  void lock_playback_system() { lock_playback = true; }
  void unlock_playback_system() { lock_playback = false; }
  bool is_playback_system_locked() const { return lock_playback; }

  int dampen_value;

private:
  char listfile_name[FILENAME_LENGTH];
  int  listfile_size;
  vector3d listener_position;
	vector3d listener_velocity;
	vector3d listener_orientation_front;
	vector3d listener_orientation_top;

  // This function cleans up any fire-and-forget sound instances, and is called before a new buffer
  // is created.  It handles fire-and-forget sounds from the sound device and all emitters.
  void cleanup_instances();
  sound_instance * create_instance();
  sound_emitter dummy_emitter;
  sound_instance dummy_instance;

  friend class sound_data;
  friend class sound_instance;
  friend class sound_emitter;

	// These are all public for console/debugging stuff.
public:
  // Storage for loaded sound data.
  sound_data sound_data_list[MAX_SOUND_ID];
  int sound_data_list_count;

  // Storage for sound emitters.
  list<sound_emitter *> emitter_list;

  // Emitters awaiting ressurection
  list<sound_emitter *> emitter_purgatory;

  // Master instance list
  list<sound_instance *> instance_list;

  // Instances awaiting the reaper
  list<sound_instance *> instance_death_row;

  // Instances awaiting resurrection
  list<sound_instance *> instance_purgatory;

  unsigned long used_audio_memory;

  bool lock_playback;
  bool dampened_flag;
};
#endif
#endif
