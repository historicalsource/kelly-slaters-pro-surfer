#ifndef GAME_H
#define GAME_H
////////////////////////////////////////////////////////////////////////////////

// game.h
// Copyright (c) 1999-2000 Treyarch Invention LLC.  ALL RIGHTS RESERVED.

// game class
// manages the actual play of the game from when a given contiguous level is entered to
// when it's done.

////////////////////////////////////////////////////////////////////////////////
#include "ostimer.h"
#include "constants.h"
#include "script_lib_mfg.h"
#include "region_graph.h"
#include "beachdata.h"
#include "careerdata.h"
#include "po.h"
#include "mode_push.h"
#include "mode_timeattack.h"
#include "mode_meterattack.h"
#include "mode_headtohead.h"
#include "ksnvl.h"
#include "fepanel.h"
#include "hwmovieplayer.h"
#include "random.h"
#include "coords.h"

////////////////////////////////////////////////////////////////////////////////
//  forward-declared classes
////////////////////////////////////////////////////////////////////////////////
class anim_maker;
class background_widget;
class camera;
class entity_maker;
class file_finder;
class gamefile_t;
class game_camera;
class game_process;
class interface_widget;
class material;
class message_board;
class mic;
class portal;
class pstring;
class region;
class script_widget_holder_t;
class sg_entry;
//class sound_group;
//class sound_stream;
//class sound_instance;
class world_dynamics_system;
#include "game_process.h"
//#include "sound_group.h"
//typedef graph<stringx,region*,portal*> region_graph;

// PlayMode struct - manages runtime states for different gameplay modes like Push and
// icon challenges.
struct PlayMode
{
	// Multiplayer alternating modes.
	TimeAttackMode *	timeAttack;
	MeterAttackMode *	meterAttack;
	//SeaHorseMode		seaHorse;

	// Multiplayer splitscreen modes.
	HeadToHeadMode *	headToHead;
	PushMode *			push;
};



class game_info
{
protected:
  friend class game;

  #define MAC(type, var, def, str)    protected: type var##; public: const type &get_##var##() const { return(##var##); } void set_##var##(const type &val) { var = val; }
  #define GAME_INFO_NUMS
  #define GAME_INFO_STRS

  #include "game_info_vars.h"

  #undef GAME_INFO_NUMS
  #undef GAME_INFO_STRS
  #undef MAC

public:
  game_info();

  void reset();

  // script accessors
  bool set_str(const pstring &att, const stringx &val);
  bool get_str(const pstring &att, stringx &val) const;

  bool set_num(const pstring &att, rational_t val);
  bool get_num(const pstring &att, rational_t &val) const;

  const stringx &get_hero_name(int hero_num);     // emulates a hero_name array for hero_name_0, hero_name_1,..
};

struct sys_time_stuct
{
  short year;       // full year (i.e. 2001)
  short month;      // 1-12
  short weekday;    // 1-7
  short day;        // 1-31
  short hour;       // 0-23
  short minute;     // 0-59
  short second;     // 0-59
};

/*
class GameEventRecipient : public EventRecipient
{
private:
	int *	numSnapshots;
	int *	snapshotScores[MAX_SNAPSHOTS];
	bool	recordNextChain;

public:
	GameEventRecipient();
	virtual ~GameEventRecipient();

	void Init(int * numSnaps, int * snapScores[MAX_SNAPSHOTS]);
	void RecordNextChain(void) { recordNextChain = true; }
	virtual void OnEvent(const EVENT event, const int param1 = 0, const int param2 = 0);
};
*/

////////////////////////////////////////////////////////////////////////////////
//  game
////////////////////////////////////////////////////////////////////////////////

class game
{
public:
	enum { MAX_SNAPSHOTS = 3 };

	enum SNAPSTATE
	{
		SNAPSTATE_NONE = 0,
		SNAPSTATE_BEGIN = 1,
		SNAPSTATE_TEXTURE_RENDER = 2,
		SNAPSTATE_TEXTURE_DOWNLOAD = 3,
		SNAPSTATE_FLASH_BEGIN = 4,
		SNAPSTATE_FLASH_END = 8,
		SNAPSTATE_END = 9
	};

	enum RENDER_STATE
	{
		GAME_RENDER_LOADING_LEVEL,
		GAME_RENDER_INITALIZE_FE,
		GAME_RENDER_DRAW_FE,
		GAME_RENDER_GAME_LEVEL
	};

public :
	static const int SNAPSHOT_WIDTH;
	static const int SNAPSHOT_HEIGHT;

public:
    game();
    ~game();

    void frame_advance();
    void render();

    // *** Member accessor functions ***
    world_dynamics_system*  get_world()       { assert(the_world); return the_world; }
    interface_widget*       get_interface_widget() const { assert(my_interface_widget); return my_interface_widget; }
    script_widget_holder_t* get_script_widget_holder() const { assert(script_widget_holder); return script_widget_holder; }
    gamefile_t *    get_gamefile() const { assert(gamefile); return gamefile; }
    camera *        get_current_view_camera() { assert (current_view_camera); return current_view_camera; }
    //game_camera *   get_current_game_camera() { assert (current_game_camera); return current_game_camera; }
    void            set_current_camera(camera *cam);
    message_board * get_message_board() { assert(mb); return mb; }
    material*       get_blank_material() const { return p_blank_material; }
    int             get_cur_state() const;

    // *** Audio controls ***
    void set_current_mic(mic *microphone)   { assert(microphone != NULL); current_mic = microphone; }
    mic *get_current_mic() const            { return current_mic; }
		int get_next_surfer_index(int current_index);
		int get_prev_surfer_index(int current_index);
		int get_first_surfer_index();
		int get_last_surfer_index();
    // --- outdated functions ---
#ifdef GCCULL
    sound_stream *get_music_stream() const { return music_stream; }
    rational_t    get_music_applied_volume() const { return music_applied_volume; }
    void set_music_stream( sound_stream *stream ) { music_stream = stream; }
    void set_music_applied_volume( rational_t volume ) { music_applied_volume = volume; }
    void music_fade_down( time_value_t time, rational_t volume ) { music_fade_time_left = time; music_fade_to_volume = volume; }
    void music_fade_up( time_value_t time ) { music_fade_time_left = time; music_fade_to_volume = 1.0f; }
    void update_music_applied_volume( time_value_t time_inc, bool force_update = false );

    // *** Music Manager control ***/
    enum music_state_t {
      MUSIC_STATE_QUIET,
      MUSIC_STATE_PLAY_PENDING,
      MUSIC_STATE_PLAYING,
      MUSIC_STATE_FORCED_CHANGE_PENDING
    };

    bool play_music( const pstring &music_name, bool force_it = false );
    void stop_music(bool signal_scripts = true);
    bool is_music_playing();
    bool is_music_queued();

		sg_entry* get_sound_group_entry( const pstring& name );   // Returns the next entry in a sound group.
    stringx & get_region_ambient_name( stringx &regname );
    void process_ambients( time_value_t delta_t );
    void clear_ambients();
#endif

	stringx get_beach_location_name();
	stringx get_beach_board_name(int location);

	// Gets the first beach id for the current beaches at the current location
  // IE - it will get the id for Tea if your current level is Tea Eve
	int			get_first_beach();
	camera *	get_player_camera(int n)		{assert(n<MAX_PLAYERS); return player_cam[n];}
    void		set_player_camera(int n, camera *cam);
	bool		is_user_cam_on;
	bool		user_cam_is_on() {return is_user_cam_on;}
	void		turn_user_cam_on(bool user_cam_status);
	bool		was_user_cam_ever_on;

	RENDER_STATE get_render_state() { return renderState; };
	void    set_render_state(RENDER_STATE newRenderState) { renderState = newRenderState; };

	// *** Game mode stuff ***
	void				set_game_mode(game_mode_t m);
	game_mode_t			get_game_mode(void) { return game_mode; }
	PushMode *			get_play_mode_push(void) { return play_mode.push; }
	TimeAttackMode *	get_play_mode_time_attack(void) { return play_mode.timeAttack; }
	MeterAttackMode *	get_play_mode_meter_attack(void) { return play_mode.meterAttack; }
	HeadToHeadMode *	get_play_mode_head_to_head(void) { return play_mode.headToHead; }

	int     get_num_ai_players() { return num_ai_players; }
	void    set_num_ai_players(int i);

	// *** Two-player stuff ***
	int           get_num_players() { return num_players; }
	int			  get_num_active_players(void) { return num_active_players; }
	bool		  is_splitscreen(void) { return num_active_players > 1 && num_ai_players == 0; }
	int           get_active_player(void) { return active_player; }
	const recti & get_player_viewport(const int playerIdx);
	void set_player_device( int player, int device ){ assert( player < MAX_PLAYERS ); player_devices[ player ] = device; };
	int get_player_device( int player ){ assert( player < MAX_PLAYERS ); return player_devices[ player ]; };
	int get_player_from_device( int device )
	{
		for( int i = 0; i < get_num_players(); i++ )
		{
			if( player_devices[ i ] == device )
				return i;
		}
		assert( 0 );
    return -1;
	}
	bool get_disconnect_status(){ return disconnect_status; };

    // *** Framerate info ***
    rational_t   get_instantaneous_fps() const; // 1 / duration of last frame
    rational_t   get_min_fps() const;           // the lowest frame we hit recently
    rational_t   get_max_fps() const;           // the highest frame we hit recently
    rational_t   get_avg_fps() const;           // average fps for past several frames
    rational_t   get_theoretical_fps() const;
    rational_t   get_console_fps() const;       // this version is designed for console development
    time_value_t get_flip_delta() const  { return flip_delta; }
    void         set_flip_delta(time_value_t new_delta) { flip_delta = new_delta; }
    time_value_t get_limit_delta() const { return limit_delta; }
    void         set_limit_delta(time_value_t new_delta) { limit_delta = new_delta; }
    time_value_t get_total_delta() const { return total_delta; }
    void         set_total_delta(time_value_t new_delta) { total_delta = new_delta; }

    // Flag accessors
    bool get_i_quit() { return flag.i_quit; }
    void set_i_quit(bool do_you_quit) { flag.i_quit = do_you_quit; }
    void set_disable_interface(bool turn_on) { flag.disable_interface = turn_on; }
    void set_disable_start_menu(bool turn_on) { flag.disable_start_menu = turn_on; }

    bool is_hero_frozen() const { return flag.hero_frozen; }
    void freeze_hero( bool freeze ); // used to disable the hero's controls for scripted sequences.
	void take_snapshot(nglTexture * dest);
	SNAPSTATE get_snapshot_state(void) const { return snapshotState; }
	//int get_num_snapshots_taken(void) const { return numSnapshots; }
	//int get_snapshot_score(const int sIdx) const { assert(sIdx >= 0 && sIdx < MAX_SNAPSHOTS); return snapshotScores[sIdx]; }

    bool is_letterbox_active() const;

  private:
    bool controller_is_plugged;
  public:
    void set_controller_is_plugged( bool plugged ) { controller_is_plugged = plugged; };
    bool get_controller_is_plugged() { return controller_is_plugged; };

	// Moved here to fix a bug
	void set_active_player(const int n);

    bool was_start_pressed() const;
    bool was_select_pressed() const;
    bool was_A_pressed() const;
    bool was_B_pressed() const;

    void pause();		// called when the user pauses the game
    void unpause();		// called when the user unpauses the game
    bool is_paused() const;

	  void on_goal_completed() {flag.goal_completed = true;}
  	bool was_goal_completed() {return flag.goal_completed;}

    void enable_marky_cam( bool enable = true, bool sync = true, rational_t priority = MIN_CAMERA_PRIORITY );
    void load_new_level( const stringx &new_level_name );

    game_info &get_game_info() { return(info); }
		int  get_player_handicap(int hero) { return handicap[hero]; }
		void set_player_handicap(int hero, int new_handicap);
    bool level_is_loaded()     { return(flag.level_is_loaded); }

#ifdef TARGET_PS2
    bool play_movie(const char *movie_name, bool canskip, int width=720, int height=480);
#else
    bool play_movie(const char *movie_name, bool canskip);
#endif
    void clear_screen();
    void enable_physics(bool on);

    const sys_time_stuct &get_systime() const { return(systime); }

	void draw_debug_labels();

#ifdef KSCULL
    script_mfg* get_script_mfg() { return &m_script_mfg; }
#endif

	void end_level(void);
	void end_run(void);
	void retry_mode(const bool fromMap = false);
	void retry_level(const bool fromMap = false);
    void go_next_state();
  	void reset_index();
    void push_process( game_process &process );
    void pop_process();

	vector3d calc_beach_spawn_pos();
	float get_player_share(const int playerIdx) const;
  private:
    // *** Private member functions ***
    void construct_game_widgets();
    void destroy_game_widgets();

    void show_debug_info();
    void do_the_cheat_codes(float time_inc);
		void move_snapshot_cam(void);

    void setup_inputs();
    void setup_input_registrations();

    void load_level_stash();             // loads level specified by current gamefile data
    void load_this_level();             // loads level specified by current gamefile data
    void unload_current_level();

    void frame_advance_level();
    void frame_advance_game_overlays( time_value_t time_inc );

		// All parts of the render loop
    void render_level();
		void render_level_splitscreen();
		void render_level_onescreen();
		void render_fe();
    void render_igo();
		void render_interface();
		void render_snapshot(void);
		void render_trippy_cheat();
		void do_autobuild_stuff();
		void do_profiler_stuff();
		void render_mem_free_screen();
		void render_legal_screen();
#ifdef TARGET_GC
		void render_pal60_screen();
#endif
		void render_title_screen();
		void render_map_screen();

    // frame_advances for various game states
    void advance_state_startup( time_value_t time_inc );
    void advance_state_front_end( time_value_t time_inc );
    void advance_state_load_level( time_value_t time_inc );
    void advance_state_play_movie( time_value_t time_inc );
    void advance_state_play_fe_movie( time_value_t time_inc );
    void advance_state_paused( time_value_t time_inc );
    void advance_state_running( time_value_t time_inc );

    void soft_reset_process();

		void  set_field_of_view(float fov) { field_of_view = fov;};
		float get_field_of_view() { return field_of_view; };
    void update_music( time_value_t time_inc );
	void update_game_mode(const float time_inc);

	public:
    void render_shadows();

    // *** Member data ***

  // BETH: the_world was made public for front end purposes
  public:
    world_dynamics_system * the_world;
    Random                  rstream;
    Random                  rstream_r;
  private:
    file_finder *           the_file_finder;
    entity_maker *          the_entity_maker;
    anim_maker *            the_anim_maker;
    gamefile_t *            gamefile;
    //camera *                base_camera;
    camera *        		current_view_camera;
    //game_camera *       	current_game_camera;
    mic *           		current_mic;
    message_board*          mb;
    material*               p_blank_material;
    interface_widget*       my_interface_widget;
    script_widget_holder_t* script_widget_holder;
    background_widget       *menu_screen;
		float										field_of_view;

		int											handicap[2];
#ifdef GCCULL
    sound_instance *        music_track[2]; // 2 music tracks, accessed thru play_music, et al
#endif
#ifdef KSCULL
    script_mfg              m_script_mfg;
#endif
    list<game_process>      process_stack;      // front process in stack is only one active

	//po						snapshotPrevCamPo;
	camera *				snapshotPrevCam;
	nglTexture *			snapshot;
	SNAPSTATE				snapshotState;
	nglTexture *			destSnapshot;

	// What we're drawing in the game
	RENDER_STATE			renderState;
	/*
	int						numSnapshots;
	nglTexture *			snapshots[MAX_SNAPSHOTS];
	int						snapshotScores[MAX_SNAPSHOTS];
	bool					recordChain;
	GameEventRecipient		eventRecipient;
	*/

	// *** Two Player Data ***
	game_mode_t				game_mode;			// 1 player, 2 player alternating, etc.
	PlayMode				play_mode;			// game mode's runtime state data
	camera *				player_cam[MAX_PLAYERS];
	recti					player_viewports[MAX_PLAYERS];
    int                     num_players;        // # of players (heroes)
    int                     num_ai_players;
	int						num_active_players;
    int                     active_player;      // in multi-player alternating, which player is active

	int player_devices[MAX_PLAYERS];
  bool disconnect_status;  
	void render_menu_screen();


    // Flags
    struct {
      bool disable_interface:1;
      bool disable_start_menu:1;
      bool hero_frozen:1;
      bool i_win:1;
      bool i_lose:1;
      bool i_restart:1;
      bool i_quit:1;
      bool load_new_level:1;
      bool level_is_loaded:1;
      bool play_movie:1;
	  bool single_step:1;
      bool wait_for_start_release:1;
      bool game_paused:1;
      bool goal_completed:1;
    } flag;

    vector3d        base_cam_position;
    hires_clock_t   real_clock;
    stringx         level_prefix;

    vector3d cam_pos[10];
    vector3d cam_targ[10];
    int cam_pos_in_use_count;
    rational_t cam_in_motion_timer;

	  // frame rate stuff
    time_value_t total_delta;
    time_value_t flip_delta;
    time_value_t limit_delta;

    time_value_t min_delta;
    time_value_t max_delta;
    time_value_t avg_delta;

    // *** Sound stream related member data ***
#ifdef GCCULL
    music_state_t   music_state;
    sound_stream* 	music_stream;         // currently playing music stream
    rational_t      music_applied_volume; // affected by script and pda_apply_music_volume()
    time_value_t    music_fade_time_left;
    rational_t      music_fade_to_volume;

    struct _ambient_stream
    {
      _ambient_stream(){ sp_stream=0; sp_region_node=0; s_base_volume=s_cur_volume=0; }
      sound_stream* 	sp_stream;
      region_graph::node *sp_region_node;
      rational_t s_base_volume;
      rational_t s_cur_volume;
    } ambient_stream, next_ambient_stream;

		// Global sound groups provide lists of sounds and the ability to draw from
		// them randomly, with a built in history checker.  Be careful to add groups to
		// the map<> if they're ever loaded outside game.

		// Example:
		// sg_entry* entry = g_game_ptr->get_sound_group_entry( "spidey_jump" );
		// sound_device::inst()->play_sound( entry->name, entry->volume, entry->pitch );
		// spidey->sound_ifc()->emitter->play_sound( entry->name, entry->volume, entry->pitch );
		vector<sound_group> sound_groups;
		map<pstring,sound_group*> sound_group_map;
#endif

    game_info info;
    sys_time_stuct systime;

		protected:
		int levelid, beachid;
    stringx movie_name;

		public:
    void set_beach( int bid ); // For non-career, sets which beach to use
		void set_level( int lid ); // For career, sets which level to use
		void set_level( stringx lname, bool career = false ); // sets which level to use (if career is true), otherwise which beach
    void set_movie( stringx mname );

		int get_level_id( void ) { return levelid; }
		int get_beach_id( void ) { return beachid; }
		bool is_competition_level(void);
		stringx get_level_name( void );
		stringx get_beach_name( void );
		stringx get_beach_stash_name( void );

		// BETH/Toby
protected:
	stringx		heroname[MAX_PLAYERS];
	int	  		boardIdx[MAX_PLAYERS];
	int     	surferIdx[MAX_PLAYERS];	// 0 thru SURFER_LAST-1 specifying current surfer (should sync with heroname[])
	bool      personality[MAX_PLAYERS];

	// for non-career modes, to have a "home" beach for the map
	int most_recent_beach;
	int most_recent_level;

public:
	stringx &   getHeroname(int hero_num) { assert(0 && "DONT USE THIS"); return heroname[hero_num]; }
	void        setHeroname(int hero_num, stringx &name) { heroname[hero_num] = name; }
	int         GetBoardIdx(int hero_num) const { return boardIdx[hero_num]; }
	void        SetBoardIdx(int hero_num, const int idx);
	int         GetSurferIdx(int hero_num) const { return surferIdx[hero_num]; }
	bool        GetUsingPersonalitySuit(int hero_num) const { return personality[hero_num]; }
	void        SetUsingPersonalitySuit(int hero_num, bool val);
	void        SetSurferIdx(int hero_num, int surfer);

	int GetMostRecentBeach() { return most_recent_beach; }
	int GetMostRecentLevel() { return most_recent_level; }
	int multiplayer_difficulty;

public:
	// this is for breakdown of the loading function
	enum LoadingStates
	{
		LOADING_INITIAL,			// always first
		LOADING_WORLD_CREATE,
		LOADING_RESET_SOUND,
		LOADING_COMMON_STASH,
		LOADING_BEACH_STASH,
		LOADING_INIT_DBMENU,
		LOADING_SCENE,
		LOADING_HERO_1_STASH,
		LOADING_HERO_1_AUX_STASH,
		LOADING_HERO_1_REST,
		LOADING_HERO_2_STASH,
		LOADING_HERO_2_AUX_STASH,
		LOADING_HERO_2_REST,
		LOADING_SCENE_END,
		LOADING_GAME_MODE,
		LOADING_LENS_FLARE,
		LOADING_SCRIPT,
		LOADING_PERFORMANCE,
		LOADING_STATE_DONE,			// always last
	};

private:
	int total_sub_states_common;
	int total_sub_states_beach;
	int total_sub_states_surfer1;
	int total_sub_states_surfer1_aux;
	int total_sub_states_surfer2;
	int total_sub_states_surfer2_aux;

public:
	bool start_drawing_map;
	int current_loading_state;
	int current_loading_sub_state;
	float loading_progress;			// 0-1
	float last_loading_progress;

	void LoadingStateInit();
	void LoadingStateReset();
	void LoadingStateUpdate();
	void LoadingStatePrint();
	void LoadingStateSkipSurfer(bool surfer1);
	void LoadingProgressUpdate();
	void SetStashSize(int loading_state, int size);
};

extern game* g_game_ptr;
extern Random *g_random_ptr;
extern Random *g_random_r_ptr;

#endif  // GAME_H
