// FEAnim.h

#ifndef FEANIM_H
#define FEANIM_H

#include "FEPanel.h"

enum PanelAnimKind
{
	AnimInstance =0xA1,
	AnimSpecial  =0xA2,
};


class PanelAnimKeyframe
{
public:
	float idx; // in seconds (though the engine may scale the animation's speed)
	vector3d translation;
	vector3d euler; 
	vector3d scale;
	stringx event; // such as produce bullet, play sound, make particle fx, etc.
	float vis;  // visibility (0 == hidden, 1 == visible)
  
	PanelAnimKeyframe() : idx(0), vis(1) {}
	bool Load(unsigned char* buffer, int& index);
};

class PanelAnim
{
public:
	stringx name; // this is used to match up animated objects to the animations
	stringx properties;
	PanelAnim* parent;
	matrix4x4 matrix; // obj to parent
	unsigned numkeyframes;
	PanelAnimKeyframe* keyframes;
	PanelAnim* children;
	PanelAnim* next;
	PanelAnim() : parent(NULL), keyframes(NULL), children(NULL), next(NULL) {}
	virtual ~PanelAnim();
	virtual PanelAnimKind Kind() const=0;
	virtual bool Load(unsigned char* buffer, int& index, PanelFile *target_panel);
	static PanelAnim* LoadPanelAnim(unsigned char* buffer, int& index, PanelFile *target_panel);
	void Update(float time, const matrix4x4& parent_matrix);
	PanelQuad *quad;  //  The object that this animation is connected to.
	matrix4x4 GetXFormMatrix(float time, bool is_fe_cam=false);
	bool GetEvent(float start_time, float end_time, stringx& event);
	PanelAnim *FindObject(char *object_name);
};

class PanelAnimInstance : public PanelAnim
{
public:
	stringx filename;
	PanelAnimInstance() {}
	virtual PanelAnimKind Kind() const { return AnimInstance; }
	virtual bool Load(unsigned char* buffer, int& index, PanelFile *target_panel);
};

class PanelAnimFile
{
public:
	PanelAnim* obs;
	float totalseconds;

//	PanelAnimFile(char *filename, PanelFile *target_panel);
	PanelAnimFile() : obs(NULL), totalseconds(0.0f) {}
	virtual ~PanelAnimFile();
	bool Load(char *filename, PanelFile *target_panel);
	static bool ReadHeader(unsigned char* buffer, int& index);
	void Update(float anim_time, const matrix4x4& offset_matrix);
	matrix4x4 GetXFormMatrix(float time, bool is_fe_cam=false);
	PanelAnim *FindObject(char *object_name);
};

enum AnimType
{
  PLAY,                     //  Play the animation, then stop rendering object.
  PLAY_BACKWARD,            //  Play once in reverse.
  PING_PONG,                //  Play forward, then backward.
};


// Animation Management

//  These flags are stackable.  You can or multiple flags together.
enum
{
  NO_FLAGS = 0x00,                //  Launch thermonuclear warheads.
  ALREADY_PLAYING = 0x01,         //  Instead of adding a new animation, check to see if it is already on the list.
  HOLD_AFTER_PLAYING = 0x02,      //  After playing the animation, continue rendering freeze framed on the last keyframe.
  LOOP = 0x04,                    //  After playing this animation, play it again!  This overrides holding.
  PING_PONGING = 0x08,            //  Not used as parameter, only used internally.
};


//  This holds an individual animation being played.
class PanelAnimEvent
{
public:
  float current_time;
  float start_offset;
  float total_time;
  PanelAnimEvent *next;
  PanelAnimEvent *prev;
  int flags;  //  Flag for special cases.
  matrix4x4 offsets;
  AnimType type;
  PanelAnimFile *animation;

  PanelAnimEvent() { next = NULL; prev = NULL; animation = NULL; }
  void SetOffset(float x, float y) { offsets.wrow().x = x; offsets.wrow().y = y; }
  void SetTimes(float starttime, float totaltime) { start_offset = starttime; total_time = totaltime; }
};

//  This will hold all of the animations currently being displayed.
class PanelAnimManager
{

private:
  PanelAnimEvent eventlist;  //  Head of the list of all the events currently playing.
	hires_clock_t clock;

public:

  PanelAnimManager() { eventlist.next = NULL; eventlist.prev = NULL; }
  virtual ~PanelAnimManager();
  PanelAnimEvent *Play(PanelAnimFile* anim, AnimType play_type = PLAY, int flags = NO_FLAGS);
  void Stop(PanelAnimFile &anim);
  bool IsPlaying(PanelAnimFile &anim);
  PanelAnimEvent *PlayPart(PanelAnimFile &anim, float current_time, float end_time, AnimType play_type = PLAY, int flags = NO_FLAGS);
  PanelAnimEvent *HoldAtTime(PanelAnimFile &anim, float hold_time,  int flags = ALREADY_PLAYING);
//  void RemoveAll();
  PanelAnimEvent *Find(PanelAnimFile &anim);
  void UpdateAnims(time_value_t time_inc);
  static inline float FixZ(float maxz) { float zbufz = maxz*0.005f+0.5f; assert(zbufz >= 0.0f && zbufz <= 1.0f); return zbufz; }
  static inline float FixZ(const matrix4x4& animmtx) { return FixZ(animmtx.wrow().z); }
};



#endif