#ifndef _AI_INTERFACE_H_
#define _AI_INTERFACE_H_

#include "global.h"
#include "entity_interface.h"
#include "ostimer.h"
#include "ai_locomotion.h"
// BIGCULL #include "ai_cue.h"
#include "anim.h"
// BIGCULL #include "damage_interface.h"
#include "ai_constants.h"

//#include "ai_threat.h"

//class ai_sight_sense;
//class ai_sound_sense;
class ai_goal;
class path_graph;
//class ai_radio;
//class ai_radio_message;
//class ai_auto_aim;
#ifdef GCCULL
class ai_voice_box;
#endif
class item;

/*
class ai_dropshadow
{
public:
  ai_dropshadow(entity *e, entity *s, rational_t sm = 0.75f)
  {
    ent = e;
    dropshadow = s;
    scale_mod = sm;
  }

  ai_dropshadow(const ai_dropshadow &b)
  {
    ent = b.ent;
    dropshadow = b.dropshadow;
    scale_mod = b.scale_mod;
  }

  ai_dropshadow& operator=(const ai_dropshadow &b)
  {
    ent = b.ent;
    dropshadow = b.dropshadow;
    scale_mod = b.scale_mod;
    return(*this);
  }

  void update(bool visible, const color32 &render_color);

  entity *ent;
  entity *dropshadow;
  rational_t scale_mod;
};
*/


class ai_interface : public entity_interface
{
protected:
  unsigned int flags;
  unsigned int team;
  unsigned int enemy_team;

  static list<ai_interface *> all_ai_interfaces;

  //BIGCULLai_radio *radio;

#ifdef GCCULL
  ai_voice_box *voice;
#endif

  //BIGCULL ai_sight_sense *eyes;
  //BIGCULL ai_sound_sense *ears;
  ai_locomotion *locomotion;

  // BIGCULL vector<ai_auto_aim *> aimers;

//  ai_cue current_cue;

  vector<ai_goal *
		#ifdef TARGET_PS2
	       ,malloc_alloc
		#endif
				 > goals;
  ai_goal *current_goal;

  entity *target;

#if 0 // BIGCULL
  ai_threat threat;
  entity *desired_threat;
  vector3d last_threat_pos;
  rational_t threat_timer;
  rational_t threat_sticky_timer;
  rational_t threat_assesment_freq;

  vector<ai_interface *> attackers;

  rational_t internal_fear_timer;
  rational_t internal_fear;
  rational_t external_fear;
  rational_t fear_mod;
  rational_t fear_decay;
#endif

  rational_t help_timer;

  // BIGCULL damage_info dmg_info;

//  vector<ai_dropshadow> dropshadows;
//  vector<ai_threat> threats;

  int disable_count;

  void read_ai_file(const stringx &file);
  void read_goal_pack(const stringx &file);
  void read_goal(chunk_file &fs, const stringx &label);
  // BIGCULL void read_aimer(chunk_file &fs, const stringx &label);


//  friend class ai_sense;
//  friend class ai_sight_sense;
//  friend class ai_sound_sense;
  friend class ai_locomotion;
  //friend class ai_radio;

  void render(char level);

public:
  enum eAiInterfaceFlags
  {
    _AI_ACTIVE            = 0x00000001,
    _AI_NEW_THREAT        = 0x00000002,
    _AI_LOST_THREAT       = 0x00000004,
    _AI_CHANGE_THREAT     = 0x00000008,
    _AI_SEE_SOMETHING     = 0x00000010,
    _AI_HEAR_SOMETHING    = 0x00000020,
    _AI_REACHED_DEST      = 0x00000040,
    _AI_COSMETIC          = 0x00000080,
    _AI_INVESTIGATED_CUE  = 0x00000100,
    _AI_ALLOW_WOUNDED     = 0x00000200,
    _AI_NEW_CUE           = 0x00000400,
    _AI_WOUNDED           = 0x00000800,
    _AI_PRONE_WOUNDED     = 0x00001000,
    _AI_SYNC_WOUNDED      = 0x00002000,
    _AI_PRONE_FRONT       = 0x00004000,
    _AI_PRONE_BACK        = 0x00008000,
    _AI_PRONE_EXPLOSION   = 0x00010000,
    _AI_HEAD_LOCKED       = 0x00020000,
    _AI_HEAD_DISABLED     = 0x00040000,
    _AI_COMBAT_MODE       = 0x00080000,
    _AI_DETECTOR          = 0x00100000,
    _AI_ADV_SHADOW        = 0x00200000,
    _AI_THREAT_LOCK       = 0x00400000,
    _AI_THREAT_IN_LOS     = 0x00800000,

    // These are used by individual goals and AI's
    // Be careful using these in goals that can be mixed
    // These are usually used for bosses though

    // For 'Spiderman', 3 and 4 are globally reserved for web trapping
    _AI_USER_FLAG_1       = 0x10000000,
    _AI_USER_FLAG_2       = 0x20000000,
    _AI_USER_FLAG_3       = 0x40000000,
    _AI_USER_FLAG_4       = 0x80000000,

    _AI_MAX               = 0xFFFFFFFF
  };

  enum eTeamID
  {
                      //EENNGGid
    _TEAM_GOOD      = 0x0000FF00,
    _TEAM_GOOD1     = 0x00000100,
    _TEAM_GOOD2     = 0x00000200,
    _TEAM_GOOD3     = 0x00000400,
    _TEAM_GOOD4     = 0x00000800,
    _TEAM_GOOD5     = 0x00001000,
    _TEAM_GOOD6     = 0x00002000,
    _TEAM_GOOD7     = 0x00004000,
    _TEAM_GOOD8     = 0x00008000,

    _TEAM_NEUT      = 0x00FF0000,
    _TEAM_NEUT1     = 0x00010000,
    _TEAM_NEUT2     = 0x00020000,
    _TEAM_NEUT3     = 0x00040000,
    _TEAM_NEUT4     = 0x00080000,
    _TEAM_NEUT5     = 0x00100000,
    _TEAM_NEUT6     = 0x00200000,
    _TEAM_NEUT7     = 0x00400000,
    _TEAM_NEUT8     = 0x00800000,

    _TEAM_EVIL      = 0xFF000000,
    _TEAM_EVIL1     = 0x01000000,
    _TEAM_EVIL2     = 0x02000000,
    _TEAM_EVIL3     = 0x04000000,
    _TEAM_EVIL4     = 0x08000000,
    _TEAM_EVIL5     = 0x10000000,
    _TEAM_EVIL6     = 0x20000000,
    _TEAM_EVIL7     = 0x40000000,
    _TEAM_EVIL8     = 0x80000000,

    _TEAM_HERO          = _TEAM_GOOD8,
    _TEAM_HERO_ALLY     = _TEAM_GOOD7,
    _TEAM_EVIL_BOSS     = _TEAM_EVIL8,
    _TEAM_EVIL_SUB_BOSS = _TEAM_EVIL7,

    _TEAM_ID_MASK       = 0x000000FF,
    _TEAM_ALIGNMENT_MASK= 0xFFFFFF00,

    _TEAM_ALL       = 0xFFFFFFFF
  };

  ai_interface(entity *ent);
  virtual ~ai_interface();

  void copy(ai_interface *b);

  void read_data(chunk_file &fs);

  static void frame_advance_ai_interfaces(time_value_t t);

  // Generic flag interface
  inline bool is_flagged(unsigned int f) const                { return((flags & f) != 0); }
  inline void set_flag(unsigned int f, bool val = true)       { if(val) flags |= f; else flags &= ~f; }

  inline bool is_active() const                               { return(is_flagged(_AI_ACTIVE) && my_entity != NULL); }
  inline void set_active(bool a)                              { set_flag(_AI_ACTIVE, a); }

  inline bool is_disabled() const                             { return(disable_count > 0); }
  void push_disable();
  void pop_disable();
  static void push_disable_all(bool disable_cosmetic=false);
  static void pop_disable_all(bool disable_cosmetic=false);

  inline bool new_threat() const                              { return(is_flagged(_AI_NEW_THREAT)); }
  inline void set_new_threat(bool a)                          { set_flag(_AI_NEW_THREAT, a); }

  inline bool lost_threat() const                             { return(is_flagged(_AI_LOST_THREAT)); }
  inline void set_lost_threat(bool a)                         { set_flag(_AI_LOST_THREAT, a); }

  inline bool change_threat() const                           { return(is_flagged(_AI_CHANGE_THREAT)); }
  inline void set_change_threat(bool a)                       { set_flag(_AI_CHANGE_THREAT, a); }

  inline bool threat_in_los() const                           { return(is_flagged(_AI_THREAT_IN_LOS)); }
  inline void set_threat_in_los(bool a)                       { set_flag(_AI_THREAT_IN_LOS, a); }

  inline bool threat_locked() const                           { return(is_flagged(_AI_THREAT_LOCK)); }
  void lock_threat();
  void unlock_threat();

  inline bool see_something() const                           { return(is_flagged(_AI_SEE_SOMETHING)); }
  inline void set_see_something(bool a)                       { set_flag(_AI_SEE_SOMETHING, a); }

  inline bool hear_something() const                          { return(is_flagged(_AI_HEAR_SOMETHING)); }
  inline void set_hear_something(bool a)                      { set_flag(_AI_HEAR_SOMETHING, a); }

  inline bool reached_dest() const                            { return(is_flagged(_AI_REACHED_DEST)); }
  inline void set_reached_dest(bool a)                        { set_flag(_AI_REACHED_DEST, a); }

  inline bool cosmetic() const                                { return(is_flagged(_AI_COSMETIC)); }
  inline void set_cosmetic(bool a)                            { set_flag(_AI_COSMETIC, a); }

  inline bool detector() const                                { return(is_flagged(_AI_DETECTOR)); }
  inline void set_detector(bool a)                            { set_flag(_AI_DETECTOR, a); }

  inline bool investigated_cue() const                        { return(is_flagged(_AI_INVESTIGATED_CUE)); }
  inline void set_investigated_cue(bool a)                    { set_flag(_AI_INVESTIGATED_CUE, a); }

  inline bool allow_wounded() const                           { return(is_flagged(_AI_ALLOW_WOUNDED)); }
  inline void set_allow_wounded(bool a)                       { set_flag(_AI_ALLOW_WOUNDED, a); }

  inline bool new_cue() const                                 { return(is_flagged(_AI_NEW_CUE)); }
  inline void set_new_cue(bool a)                             { set_flag(_AI_NEW_CUE, a); }

  inline bool wounded() const                                 { return(is_flagged(_AI_WOUNDED)); }
  inline void set_wounded(bool a)                             { set_flag(_AI_WOUNDED, a); }

  inline bool sync_wounded() const                            { return(is_flagged(_AI_SYNC_WOUNDED)); }
  inline void set_sync_wounded(bool a)                        { set_flag(_AI_SYNC_WOUNDED, a); }

  inline bool was_prone_wounded() const                       { return(is_flagged(_AI_PRONE_WOUNDED)); }
  inline void set_prone_wounded(bool a)                       { set_flag(_AI_PRONE_WOUNDED, a); }

  inline bool is_head_locked() const                          { return(is_flagged(_AI_HEAD_LOCKED)); }
  inline void set_head_locked(bool a)                         { set_flag(_AI_HEAD_LOCKED, a); }

  inline bool is_head_disabled() const                        { return(is_flagged(_AI_HEAD_DISABLED)); }
  inline void set_head_disabled(bool a)                       { set_flag(_AI_HEAD_DISABLED, a); }

  inline bool is_combat_mode() const                          { return(is_flagged(_AI_COMBAT_MODE)); }
  inline void set_combat_mode(bool a)                         { set_flag(_AI_COMBAT_MODE, a); }

#if 0 // BIGCULL
  inline bool has_dropshadow() const                          { return(!dropshadows.empty() || is_flagged(_AI_ADV_SHADOW)); }
  inline bool has_adv_dropshadow() const                      { return(is_flagged(_AI_ADV_SHADOW) && dropshadows.empty()); }
#endif // BIGCULL


  inline bool is_prone_back() const                           { return(is_flagged(_AI_PRONE_BACK)); }
  inline void set_prone_back(bool a)                          { set_flag(_AI_PRONE_BACK, a); }
  inline bool is_prone_front() const                          { return(is_flagged(_AI_PRONE_FRONT)); }
  inline void set_prone_front(bool a)                         { set_flag(_AI_PRONE_FRONT, a); }
  inline bool is_prone() const                                { return(is_flagged(_AI_PRONE_BACK|_AI_PRONE_FRONT)); }
  inline bool is_prone_explosion() const                      { return(is_flagged(_AI_PRONE_EXPLOSION)); }
  inline void set_prone_explosion(bool a)                     { set_flag(_AI_PRONE_EXPLOSION, a); }

  // Team interface
  inline bool is_on_team(unsigned int t) const                { return((team & (t & _TEAM_ALIGNMENT_MASK)) != 0); }
  inline void set_team(unsigned int t, bool on = true)        { if(on) team |= (t & _TEAM_ALIGNMENT_MASK); else team &= ~(t & _TEAM_ALIGNMENT_MASK); }
  inline void set_enemy_team(unsigned int t, bool on = true)  { if(on) enemy_team |= (t & _TEAM_ALIGNMENT_MASK); else enemy_team &= ~(t & _TEAM_ALIGNMENT_MASK); }

  inline bool is_enemy_team(unsigned int t) const             { return((enemy_team & (t & _TEAM_ALIGNMENT_MASK)) != 0); }
  inline bool is_ally_team(unsigned int t) const              { return((team & (t & _TEAM_ALIGNMENT_MASK)) != 0); }
  inline bool is_neutral_team(unsigned int t) const           { return(!is_enemy_team(t) && !is_ally_team(t)); }

  inline unsigned int get_team() const                        { return(team & _TEAM_ALIGNMENT_MASK); }
  inline unsigned int get_enemy_team() const                  { return(enemy_team & _TEAM_ALIGNMENT_MASK); }

  inline unsigned int get_good_level() const                  { return((team & _TEAM_GOOD) >> 8); }
  inline unsigned int get_neut_level() const                  { return((team & _TEAM_NEUT) >> 16); }
  inline unsigned int get_evil_level() const                  { return((team & _TEAM_EVIL) >> 24); }
  inline unsigned int get_power_level() const                 { return(get_good_level() + get_neut_level() + get_evil_level()); }


  inline unsigned int get_good_level(unsigned int t) const    { return((t & _TEAM_GOOD) >> 8); }
  inline unsigned int get_neut_level(unsigned int t) const    { return((t & _TEAM_NEUT) >> 16); }
  inline unsigned int get_evil_level(unsigned int t) const    { return((t & _TEAM_EVIL) >> 24); }

  inline void set_team_id(unsigned int id)                    { team |= (id & _TEAM_ID_MASK); }
  inline unsigned int get_team_id() const                     { return(team & _TEAM_ID_MASK); }

  bool is_enemy(ai_interface *ai) const;
  bool is_ally(ai_interface *ai) const;
  inline bool same_team_id(ai_interface *ai) const            { return(get_team_id() == ai->get_team_id()); }


  // Threat assessment
#if 0// BIGCULL
  void threat_assesment(time_value_t t);
  inline void threat_assesment()                              { threat_assesment(threat_timer); }
  inline ai_threat get_ai_threat() const                      { return(threat); }
  inline entity *get_threat() const                           { return(threat.get_entity()); }
  inline vector3d get_last_threat_pos() const                 { return(last_threat_pos); }
  unsigned int threat_level(ai_interface *ai) const;
  inline entity *get_desired_threat() const                   { return(desired_threat); }
  inline void set_desired_threat(entity *e)                   { desired_threat = e; }

  inline entity *get_target() const                           { return(target); }
  inline void set_target(entity *ent)                         { target = ent; }

  void add_attacker(ai_interface *ai);
  void remove_attacker(ai_interface *ai);
  inline int get_num_attackers() const                        { return(attackers.size()); }
  inline ai_interface *get_attacker(unsigned int i) const     { return( (i >= 0 && i < attackers.size()) ? attackers[i] : NULL ); }

  // Fear
  inline rational_t get_fear() const                          { return(internal_fear + external_fear); }
  inline rational_t get_fear_mod() const                      { return(fear_mod); }
  inline rational_t get_fear_decay() const                    { return(fear_decay); }
  inline void get_fear_mod(rational_t f)                      { fear_mod = f; }
  inline void get_fear_decay(rational_t f)                    { fear_decay = f; }
  inline void inc_fear(rational_t f)                          { if(fear_mod > 0.0f) external_fear += (f*fear_mod); }
  inline void dec_fear(rational_t f)                          { if(fear_mod > 0.0f) external_fear -= (f*(1.0f/fear_mod)); if(external_fear < 0.0f) external_fear = 0.0f; }
  inline void calculate_fear()                                { calculate_fear(internal_fear_timer); }
  void calculate_fear(time_value_t t);
#endif // BIGCULL

  // Senses
  //BIGCULL inline ai_sight_sense *get_eyes() const                     { return(eyes); }
  //BIGCULL inline ai_sound_sense *get_ears() const                     { return(ears); }
  //BIGCULL inline const ai_cue &get_current_cue() const                { return(current_cue); }

  ai_goal *get_goal(const pstring &goal_name) const;
#if 0 // BIGCULL
  ai_auto_aim *get_aimer(const pstring &aimer_name) const;
  void set_aimer_target(const pstring &aimer_name, const vector3d &target_pos);
  void set_aimer_target(const vector3d &target_pos);
  vector3d get_aimer_target(const pstring &aimer_name) const;

  void set_aimer_active(const pstring &aimer_name, bool active);
  void set_aimer_active(bool active);
  bool get_aimer_active(const pstring &aimer_name) const;
#endif

  void force_lookat(const vector3d &pos);
  void force_lookat(entity *ent);
  void release_lookat();

// BIGCULL   inline damage_info get_dmg_info() const                     { return(dmg_info); }

  // Locomotion
  inline ai_locomotion *get_locomotion()  const               { return(locomotion); }
  eLocomotionType get_locomotion_type() const;
  bool goto_position(const vector3d &pos, rational_t rad = 2.0f, bool fast = true, bool path_find = true, bool force_finish = false);
  path_graph *get_current_path_graph() const;
  void set_current_path_graph(path_graph *g);
  rational_t get_xz_rotation_to_point( const vector3d &target_pos ) const;
  bool xz_facing_point( const vector3d &target_pos, rational_t rads ) const;
  void apply_rotation( rational_t rot );
  bool get_next_patrol_point(const vector3d &last_pos, const vector3d &cur_pos, vector3d &next);
  bool get_nearest_patrol_point(const vector3d &cur_pos, vector3d &patrol_pt);
  rational_t get_patrol_radius() const;
  void set_patrol_radius(rational_t r);
  void stop_movement(bool forced = false);
  void jockey_to(const vector3d &pt);
  void jockey_dir_time(const vector3d &dir, time_value_t t);
  void stop_jockey();
  bool is_jockeying() const;
  bool using_animation_for_locomotion() const;
  bool face_dir(const vector3d &dir, rational_t mod = 1.0f);
  bool face_point(const vector3d &dir, rational_t mod = 1.0f);


  bool play_animation(const stringx &anim_id, int slot, unsigned short anim_flags = (ANIM_TWEEN|ANIM_RELATIVE_TO_START|ANIM_AUTOKILL|ANIM_NONCOSMETIC), int *damage_value = NULL, rational_t *recover = NULL, rational_t *recover_var = NULL, int *flags = NULL);
  bool play_animation(const char *anim_id, int slot, unsigned short anim_flags = (ANIM_TWEEN|ANIM_RELATIVE_TO_START|ANIM_AUTOKILL|ANIM_NONCOSMETIC), int *damage_value = NULL, rational_t *recover = NULL, rational_t *recover_var = NULL, int *flags = NULL)
  {
    stringx tmp_anim_id(anim_id);
    return(play_animation(tmp_anim_id, slot, anim_flags, damage_value, recover, recover_var, flags));
  }


  virtual void frame_advance(time_value_t t);

  const ai_goal *get_current_goal() const { return(current_goal); }

  virtual bool get_ifc_num(const pstring &att, rational_t &val);
  virtual bool set_ifc_num(const pstring &att, rational_t val);
  virtual bool get_ifc_str(const pstring &att, stringx &val);
  virtual bool set_ifc_str(const pstring &att, const stringx &val);

  bool set_goal_num(const pstring &goal, const pstring &att, rational_t val);
  bool get_goal_num(const pstring &goal, const pstring &att, rational_t &val);
  bool set_goal_str(const pstring &goal, const pstring &att, const stringx &val);
  bool get_goal_str(const pstring &goal, const pstring &att, stringx &val);

  bool set_loco_num(const pstring &att, rational_t val);
  bool get_loco_num(const pstring &att, rational_t &val);

  static void render_ai(char level);

  bool find_cover_pt(const vector3d &from, vector3d &pos) const;
  bool find_ambush_pt(const vector3d &to, vector3d &pos) const;

  bool compute_combat_target( const vector3d &target_pos, item *itm = NULL, rational_t accuracy = 1.0f );

  static vector3d calculate_eye_pos(entity *ent);
  vector3d calculate_jump_vector_xzvel(const vector3d &dest, rational_t xz_vel = 10.0f);


  // Communication
#if 0 // BIGCULL
  inline ai_radio *get_radio() const                          { return(radio); }
  bool send_message(ai_interface *ai, const ai_radio_message &message);
  int broadcast_message(int channel, const ai_radio_message &message);
  bool new_radio_message() const;
  const ai_radio_message &get_last_message() const;
#endif // BIGCULL

#ifdef GCCULL
  inline ai_voice_box *get_voice() const                      { return(voice); }
#endif

  // searchers
  enum
  {
    _FIND_AI_NEAREST    = 0x00000001,
    _FIND_AI_FARTHEST   = 0x00000002,
    _FIND_AI_FOV        = 0x00000004,
    _FIND_AI_LOS        = 0x00000008
  };

  ai_interface *find_ai_by_team(unsigned int t, unsigned int flags = (_FIND_AI_NEAREST | _FIND_AI_LOS), rational_t max_dist = 9999.0f);
  inline static const list<ai_interface *> &get_all_ai_interfaces()   { return(all_ai_interfaces); }
};


#endif
