// FEEntityManager.h

#ifndef FEENTITYMANAGER_H
#define FEENTITYMANAGER_H

#include "GraphicalMenuSystem.h"
#include "scene_anim.h"
#include "mustash.h"

#ifndef TARGET_GC
#define SURFER_STASH_MAX_SIZE_MEGS  (2.1)
#else
#define SURFER_STASH_MAX_SIZE_MEGS  (1)
#endif

class FEManager;

class FEEntityManager
{
public:
	enum {	STATE_MAIN,
			STATE_SURFER1, 
			STATE_SURFER2, 
			STATE_TRICK, 
			STATE_BEACH, 
			STATE_BOARD,
			STATE_OTHER_SET,	// other, but with the set
			STATE_OTHER };
	enum {	SA_IDLE,
			SA_SPECIAL_OCC,
			SA_OCC_1,
			SA_OCC_2,
			SA_SELECT,
			SA_TRICK };
	enum {	BA_SPIN,
			BA_TRICK };

	entity* surfer_ent;					// only one loaded at a time
	bool mm_anims_loaded;
	bool all_loaded;

	float camera_roll_time;
	int camera_roll_stop;		// 0 = don't stop, 1 = stopped, 2 = stop soon
	static const int CAMERA_ROLL_ANG_MAX = 3;
	static const int CAMERA_ROLL_TIME_MAX = 12;


private:
	int surfer_anim_count;
	void CheckStashLoadStatus();
	static const int num_surf_anim = 3;
	int current_surfer_index;
	int previous_surfer_index[2];
	int loading_surfer_index;
	int loading_surfer_ent;
	int surfer_index_2;
	int current_board_index;
	FEManager* manager;
	nglMaterial set_material;
	float turnaround_length;
 	entity* map;
	entity* porthole;
	po porthole_start_po;

public:
	nslEmitterId   behindTheCamera;

	entity* board;
	entity * my_board;
	entity * my_board_member;
	entity * my_rotate_object;
	entity * my_parent_node;
private:
	bool boards_loaded;
	bool board_draw;
	int board_tail_type;
	entity_anim_tree* board_tree;
/*
void UpdateBoardIFL(int csi);
*/

	SSEventId doorEvent;
public:
	// Camera Animation
	enum {	CAM_POS_WALL_1,
			CAM_POS_WALL_2_IN,
			CAM_POS_WALL_2_OUT,
			CAM_POS_WALL_3_MAP,
			CAM_POS_WALL_3_CLOSET,
			CAM_POS_WALL_4,
			CAM_POS_END };
private:
	int cam_pos_goal;
	bool cam_stopped;
	int cam_stopped_at;
	float stops[CAM_POS_END];
	scene_anim_handle_t cam_anim_handle;
	entity_anim_tree* cam_anim_tree;
	bool cam_reverse;
	bool skip_map_zoom;
  
 	// Closet door stuff
	entity* closet_door[2];
	float closet_pos[2];
	enum {	CLOSET_NOT_MOVING,
			CLOSET_LEFT_OPENING,
			CLOSET_LEFT_CLOSING,
			CLOSET_RIGHT_OPENING,
			CLOSET_RIGHT_CLOSING };
	float closet_movement;
	int closet_state;
	float closet_timer;
	bool closet_open[2];		// 0=left, 1=right
  

public:
	camera* fe_camera;
	camera* active_camera;
private:
	entity_anim_tree* surfer_tree;
	bool dont_draw_surfer;
	int btwn_idle_count;
	bool old_pers;
	static const int MAX_BTWN_IDLE_COUNT = 3;
	int next_anim;
	int cur_anim;
	int cur_b_anim;
	int cur_trick_anim;
	int cur_trick;
	int cur_state;
	int last_state;
	bool op_ext;
	bool forward;
	bool loop_around;
	float beaches[13];
	bool surfer_loaded;
	bool tricks_loaded;
	bool in_flyin;
	bool trick_playing;

	// don't access surfer_select_po directly, use SetSurferPo()
	po surfer_select_po;
	po surfer_trick_po;
	po board_select_po;
	po board_trick_po;

	ik_object *my_ik_object;

public:
	FEEntityManager(FEManager* man);
	~FEEntityManager();
	void Update(time_value_t time_inc);
	void RenderConglom(entity* c);
	void SetConglomPo(entity* c, po p);
	void DrawFront();
	void DrawBack();
	void LoadAll();
	int  GetCurrentSurferIndex() { return current_surfer_index; };
	bool UpdateSurferIndex(int csi, bool force_load=false); // returns false if no entity available
	void UpdateBoardIndex(int b, int tail_type, int size, bool draw);
	void ToMainScreen();
	void ToBeachSelect();
	void ToBoardSelect(int board, int tail_type, int size);
	void ToTrickBook();
	void ToTrickBook2();	// called only when OKtoDrawBio() is true
	void LoadBoard(int hero=0, int board=0);
	void LoadAuxStash();
	void ToSurferSelect();
	void ToAccomp() { cur_state = STATE_OTHER_SET; }
	void StopCameraRoll() { if(camera_roll_stop == 0) camera_roll_stop = 2; }
	void ResumeCameraRoll() { camera_roll_stop = 0; }

	void ToOtherState() { cur_state = STATE_OTHER; }
	void ExitState();
	bool InFlyin() { return in_flyin; }
	void JumpTo(int pos);
	void BioTBZoom(bool out);
	bool CamIsMoving() { return !cam_stopped; }
	bool OKtoDrawSurferSelect() { return cam_stopped && cam_stopped_at == CAM_POS_WALL_2_IN; }
	bool OKtoDrawBeachSelect() { return cam_stopped && cam_stopped_at == CAM_POS_WALL_3_MAP && closet_state == CLOSET_NOT_MOVING && camera_roll_stop == 1; }
	bool OKtoDrawBio() { return cam_stopped && cam_stopped_at == CAM_POS_WALL_2_OUT; }
	bool OKtoDrawBoardSelect() { return cam_stopped && cam_stopped_at == CAM_POS_WALL_3_CLOSET && closet_state == CLOSET_NOT_MOVING; }
	bool OKtoDrawMain() { return cam_stopped && cam_stopped_at == CAM_POS_WALL_1; }
	bool OKtoDrawScrapbook() { return cam_stopped && (cam_stopped_at == CAM_POS_WALL_2_OUT || cam_stopped_at==CAM_POS_WALL_1);  }
	bool OKtoDrawHelpbar() { return cur_state != STATE_OTHER && !in_flyin; }
	void StashesCleared();
	void SurferSelected() {}
	void SetSurferPo(entity* surfer, int surfer_num);
	bool UnloadCurrentSurfer();
	void UnloadCurrentBoard();
	void FEDB_ToggleDraw(int option);
	int GetCurrentSurfer() { return current_surfer_index; }

	void LoadTricks();
	void TrickPlay(int anim_id);
	void PlaySurferTrick(SurferTrick* trick_id);
	void Zoom(int state);
	void DoneAsyncLoad();
	bool TrickAnimDone() { return !trick_playing; }
	void PerformIK(void);

	int mainThreadId;

	static int remapSurferNum(int i);
	static int remapBeachNum(int i);
private:
	void SurferPlay(int type, float time=-1.0f, int trick_num=0, int b_trick_num=0, bool no_blend=false);
	void BoardPlay(int type, int anim_id=0, float time = -1.0f);
	stringx makeString(int surfer, int anim);
	void ReloadBoardTextures(int which);

	void Anim(stringx anim, float blend_time, bool loop, float time);
	void BoardAnim(int anim_id, float blend_time, bool loop, float time);

	void CameraAnim(int anim_pos, float start_time=-2);
	void moveClosetDoor(bool door_left, bool open);

	void LoadSurfer(int s);
	void LoadSet();
	void LoadSurferAnims(int surfer);
	void SetConglomTexture(entity* c, int b);
	void SetConglomScale(entity* c, vector3d s);
	void SetConglomForceHiRes(entity* c, bool fhr);
	// return desired distance from character's root to floor
	float get_floor_offset(void);
	void AlignSurferWithBoard(void);
};



#endif
