
#ifndef _BEACH_H_
#define _BEACH_H_

#include "global.h"
#include "judge.h"
#include "player.h"
#include "timer.h"
#include "challenge_icon.h"
#include "challenge_photo.h"

class beach_object;
class entity;
class kellyslater_controller;

class beach
{
public:
	struct BeachChallenges
	{
		IconChallenge *		icon;
		PhotoChallenge *	photo;
		//SlalomChallenge *	buoy;
	};

public:
	JudgingSystem	judges;				// used to score competitions
	vector3d travel_distance;
	beach_object *my_objects;

protected:
	int current_breakmap;
	int num_breakmap;
	entity *smashedEntity;
	BeachChallenges	challenges;			// holds state for complex challenges
	
protected:
	void check_challenges(kellyslater_controller *ksctrl);
	bool check_spray_challenge (kellyslater_controller *ksctrl, int object_type, int object_count, int gt_partially_done, int gt_done);
	bool check_smash_challenge (kellyslater_controller *ksctrl, int object_type, int object_count, int gt_partially_done, int gt_done);

	// a helper function for check_challenges()
  bool CheckEnvGoal(kellyslater_controller *ksctrl, stringx entity_name, int num_to_jump, int gap_to_add, 
                    int gt_partially_done, int gt_done, int goal_num);

public:
	// Creators.
	beach ();
	~beach ();

	// Basic stuff.
	void load ();
	void loadbreakmap (const stringx &breakmapname);
	void update (float dt);
	void cleanup ();
	void reset ();
	entity *get_smashed_entity() { return smashedEntity; }
	// Entity collisions.
	entity *check_entity_collision(kellyslater_controller *ksctrl, beach_object * ignoreObj = NULL);
	entity *surfer_over_entity(vector3d c1, float r1) const;
	bool get_nearest_object_pos(vector3d surfer_pos, vector3d &object_pos, vector3d lip_norm); //  returns false if no objects found
	void complete_goal(const int a);
	
	// Eh.
	bool IsDolphinActive(void);
	void OnNewWave(void);
#ifdef DEBUG
	entity *GetDolphin(void);
#endif
	
	// linked list helper functions
	void add_object (beach_object *obj);
	void remove_object (beach_object *obj);
	beach_object* get_object (int num) const;
	beach_object* find_object (entity *ent) const;
	
	// Challenge functions.
	BeachChallenges * get_challenges(void) { return &challenges; }
};

extern beach *g_beach_ptr;

#endif // _BEACH_H_



