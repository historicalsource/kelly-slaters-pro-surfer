
// With precompiled headers enabled, all text up to and including
// the following line are ignored by the compiler (dc 01/18/02)
#include "global.h"
#include "lightmgr.h"
#include "FEEntityManager.h"
#include "FrontEndManager.h"
#include "BoardManager.h"
#include "wds.h"
#include "game.h"
#include "osdevopts.h"
#include "file_finder.h"
#include "heap.h"
#include "semaphores.h"
#include "unlock_manager.h"

static int frameDelay = 3;
MemSize startMarker1=0, startMarker2=0, endMarker1=0, endMarker2=0;


extern SurferTrick GTrickList[TRICK_NUM];

static int first_run;

/****************************** FEEntityManager ************************/
static int boardCount = 0;
FEEntityManager::FEEntityManager(FEManager* man)
{
	turnaround_length = 6.0f;
	manager = man;
	first_run = 1;
	stops[CAM_POS_WALL_1] = 16.67f;
	stops[CAM_POS_WALL_2_IN] = 19.67f;
	stops[CAM_POS_WALL_2_OUT] = 20.67f;
	stops[CAM_POS_WALL_3_MAP] = 23.33f;
	stops[CAM_POS_WALL_3_CLOSET] = 25.00f;
	stops[CAM_POS_WALL_4] = 27.33f;
	cam_anim_tree = NULL;
	skip_map_zoom = false;
	cam_pos_goal = -1;
	in_flyin = true;

	loading_surfer_ent = 0;
	surfer_ent = NULL;
	
	dont_draw_surfer = false;

	old_pers = false;
	if (g_world_ptr == NULL)
	{
		app::inst()->get_game()->the_world = NEW world_dynamics_system;
		g_world_ptr = app::inst()->get_game()->the_world;
	}
	surfer_loaded = false;
	all_loaded = false;
	btwn_idle_count = 0;

	boards_loaded = false;
	tricks_loaded = false;
	//tricks_loaded[1] = false;

	mm_anims_loaded = false;
	previous_surfer_index[0] = previous_surfer_index[1] = -1;
	loop_around = false;
	op_ext = false;
	cur_state = STATE_OTHER;
	last_state = STATE_OTHER;
	surfer_tree = NULL;
	board_tree = NULL;
	current_surfer_index = loading_surfer_index = g_game_ptr->GetSurferIdx(0);;

	fe_camera = NULL;
	map = NULL;
	porthole = NULL;
	closet_door[0] = NULL;
	closet_door[1] = NULL;
	closet_state = CLOSET_NOT_MOVING;
	closet_open[0] = false;
	closet_open[1] = false;

	cam_stopped = false;
	cam_stopped_at = -1;
	surfer_anim_count = 0;

	po trans( po_identity_matrix );
	trans.set_rotate_y(0.5f*PI);
	surfer_trick_po = po(1, 0, 0,   0, 1, 0,   0, 0, 1,   0.0f, 0.0f, 0.0f);
	surfer_trick_po = trans*surfer_trick_po;
	surfer_trick_po.set_position(vector3d(-3.6, 2.3, -9));
	//surfer_trick_po = po(1, 0, 0,   0, 1, 0,   0, 0, 1,   -3.6, 2.3, -9);
//	board_trick_po = po(1, 0, 0,   0, 1, 0,   0, 0, 1,   -2.7, 1.2, -9);
	board_trick_po = surfer_trick_po;
	board_trick_po.set_position(surfer_trick_po.get_position() + vector3d(0, -.6, 0));
	board_select_po = po(1, 0, 0,  0, 1, 0,  0, 0, 1, 0.0f, 0.0f, 0.0f);
	board_select_po = trans*board_select_po;
	board_select_po.set_position(vector3d(-2.7, .62, -1.05));
	//board_select_po = po(1, 0, 0,  0, 1, 0,  0, 0, 1, -3.82, .6, -0.8);
//	surfer_select_po = po(1, 0, 0,   0, 1, 0,   0, 0, 1,   -.2, .8, -4);

	// turned just slightly towards camera
//	surfer_select_po = po(.9889, 0, .1483,   0, 1, 0,   -.1483, 0, .9889,   -.2, .8, -4);
	// sa & ca are sin & cos of angle that kelly must turn to to face camera
	float sa = 0.9662f;
	float ca = 0.2577f;
	surfer_select_po = po(sa, 0, ca,   0, 1, 0,   -ca, 0, sa,   -.06, .9, -4);

	board = NULL;

	camera_roll_time = 0;
	camera_roll_stop = 0;
	active_camera = NULL;
	trick_playing = false;

	my_board = NEW entity();
	my_rotate_object = NEW entity();

	my_ik_object = NEW ik_object();
	cur_trick_anim = -2;
	doorEvent = -1;
}

FEEntityManager::~FEEntityManager()
{
	if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
		nslReleaseEmitter(behindTheCamera);
/*
  for (int i=0; i < 5; i++)
    KSReleaseFile(&IFLFiles[i]);
*/
	memcpy(&g_career->cfg, StoredConfigData::inst()->getGameConfig(), sizeof(ksConfigData));
	if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
		nslShutdown();
	delete app::inst()->get_game()->the_world;
	g_world_ptr = NULL;
	app::inst()->get_game()->the_world = NULL;
/*#if defined(TARGET_GC)
  nglReleaseAllMeshes();
#else
*/
  nglReleaseAllMeshFiles();
//#endif // defined(TARGET_XBOX)


  // Now, we only want to close the stash of the one surfer
  // we aren't using
  stringx stashname;
  if (g_game_ptr->GetUsingPersonalitySuit(0))
    stashname = stringx("SURFERS\\PERSON") + SurferDataArray[g_game_ptr->GetSurferIdx(0)].abbr + stringx(".ST2");
  else
    stashname = stringx("SURFERS\\") + SurferDataArray[g_game_ptr->GetSurferIdx(0)].stashfile + stringx(".ST2");
  int closeStash = 0;

  // We figure out which stashes to close!
  if (g_game_ptr->get_num_players() == 1)
  {
    if (stash::is_stash_open(STASH_SURFER) &&  (stricmp(stash::get_stash_name(STASH_SURFER), stashname.c_str()) != 0))
      closeStash |=  0x1;
    else if (stash::is_stash_open(STASH_SURFER_2) &&  (stricmp(stash::get_stash_name(STASH_SURFER_2), stashname.c_str())!=0))
      closeStash |= 0x2;
  }
  else
  {


	if ((stash::is_stash_open(STASH_SURFER) && (strstr(stash::get_stash_name(STASH_SURFER), SurferDataArray[g_game_ptr->GetSurferIdx(0)].stashfile) != NULL)) &&
		(stash::is_stash_open(STASH_SURFER) && (strstr(stash::get_stash_name(STASH_SURFER), SurferDataArray[g_game_ptr->GetSurferIdx(1)].stashfile) != NULL)))
		closeStash = 0x1;

    
    if ((stash::is_stash_open(STASH_SURFER_2) &&  (strstr(stash::get_stash_name(STASH_SURFER_2), SurferDataArray[g_game_ptr->GetSurferIdx(0)].stashfile)!=NULL)) &&
		(stash::is_stash_open(STASH_SURFER_2) &&  (strstr(stash::get_stash_name(STASH_SURFER_2), SurferDataArray[g_game_ptr->GetSurferIdx(1)].stashfile)!=NULL)))
      closeStash |=  0x2;

    if (closeStash == 3)
      assert(0 && "WE SHOULD HAVE THE CURRENT SURFER IN ONE OF THE STASHES");
  }

#if defined( TARGET_XBOX ) || defined( TARGET_GC ) 	 || 1// Problem with mesh rebasing / unrebasing.  Too messy to fix now.  (dc 03/04/02)
//  if (closeStash & 0x1)
  {
    stash::close_stash(STASH_SURFER);
    stash::close_stash(STASH_SURFER_AUX);
    stash::close_stash(STASH_SURFER_BOARD);
		stash::free_stored(STASH_SURFER);
    stash::free_stored(STASH_SURFER_AUX);
		stash::free_stored(STASH_SURFER_BOARD);
  }


//  if (closeStash & 0x2)
  {
    stash::close_stash(STASH_SURFER_2);
    stash::close_stash(STASH_SURFER_2_AUX);
		stash::close_stash(STASH_SURFER_2_BOARD);
    stash::free_stored(STASH_SURFER_2);
    stash::free_stored(STASH_SURFER_2_AUX);
		stash::free_stored(STASH_SURFER_2_BOARD);
    
  }
  if (g_game_ptr->get_num_players() < 2)
  {
//	  if (!stash::is_stash_open(STASH_SURFER))
//		mem_destroy_heap(SURFER_HEAP);
//	  else if (!stash::is_stash_open(STASH_SURFER_2))
		//mem_destroy_heap(SURFER_HEAP2);
  }
#else
  if (closeStash & 0x1)
  {
    stash::close_stash(STASH_SURFER);
    stash::close_stash(STASH_SURFER_AUX);
    stash::free_stored(STASH_SURFER);
    stash::free_stored(STASH_SURFER_AUX);
    
  }


  if (closeStash & 0x2)
  {
    stash::close_stash(STASH_SURFER_2);
    stash::close_stash(STASH_SURFER_2_AUX);
    stash::free_stored(STASH_SURFER_2);
    stash::free_stored(STASH_SURFER_2_AUX);
    
  }
  if (g_game_ptr->get_num_players() < 2)
  {
	  if (!stash::is_stash_open(STASH_SURFER))
		mem_destroy_heap(SURFER_HEAP);
	  /*else if (!stash::is_stash_open(STASH_SURFER_2))
		mem_destroy_heap(SURFER_HEAP2);*/
  }
#endif

  delete my_board;
  delete my_rotate_object;
  delete my_ik_object;
}

void FEEntityManager::Update(time_value_t time_inc)
{
	GETSEMA(StallSurferLoadSema);
	if(cam_anim_tree)
	{
		float time = g_world_ptr->get_scene_anim_time(cam_anim_handle);
		if(!cam_anim_tree->is_suspended())
		{
			if(cam_reverse)
			{
				if(time <= stops[cam_pos_goal])
				{
					cam_anim_tree->set_suspended(true);
					if(in_flyin) in_flyin = false;
					cam_stopped = true;
					cam_stopped_at = cam_pos_goal;
					
					if(cam_stopped_at == CAM_POS_WALL_2_OUT && cur_state == STATE_TRICK)
						ToTrickBook2();
				}
			}
			else
			{
				if(time >= stops[cam_pos_goal])
				{
					cam_anim_tree->set_suspended(true);
					if(in_flyin) in_flyin = false;
					cam_stopped = true;
					cam_stopped_at = cam_pos_goal;
					
					if(cam_stopped_at == CAM_POS_WALL_2_OUT && cur_state == STATE_TRICK)
						ToTrickBook2();
				}
			}
		}
	}
	
	if(closet_state != CLOSET_NOT_MOVING)
	{
		closet_timer += time_inc;
		if(closet_state == CLOSET_LEFT_OPENING || closet_state == CLOSET_LEFT_CLOSING)
		{
			float tmp = closet_timer;
			if(closet_state == CLOSET_LEFT_CLOSING) tmp = 1-tmp;
			vector3d v = closet_door[1]->get_rel_position();
			
			// cubic spline instead of linear
			float result = closet_pos[1]+closet_movement*(-2*tmp*tmp*tmp+3*tmp*tmp);
			closet_door[1]->set_rel_position(vector3d(v.x, v.y, result));
			vector3d m = map->get_rel_position();
			map->set_rel_position(vector3d(m.x, m.y, m.z+(result-v.z)));
			//			((BeachFrontEnd*)manager->gms->menus[GraphicalMenuSystem::BeachMenu])->UpdatePointLoc(result-v.z);
		}
		else
		{
			float tmp = closet_timer;
			if(closet_state == CLOSET_RIGHT_OPENING) tmp = 1-tmp;
			vector3d v = closet_door[0]->get_rel_position();
			float result = closet_pos[0]+closet_movement*(-2*tmp*tmp*tmp+3*tmp*tmp);
			closet_door[0]->set_rel_position(vector3d(v.x, v.y, result));
		}
		if(closet_timer >= 1)
		{
			switch(closet_state)
			{
			case CLOSET_LEFT_OPENING: closet_open[0] = true; break;
			case CLOSET_LEFT_CLOSING: closet_open[0] = false; break;
			case CLOSET_RIGHT_OPENING: closet_open[1] = true; break;
			case CLOSET_RIGHT_CLOSING: closet_open[1] = false; break;
			}
			closet_state = CLOSET_NOT_MOVING;
		}
	}
	
	PASSTHREADCONTROL(1);
	
	if(cur_state == STATE_BOARD)
	{
		if(boardCount >= 0) boardCount++;
		if(boardCount >= 2)
		{
			boardCount = -1;
		}
	}
	if(cur_state != STATE_OTHER)
	{
		g_world_ptr->frame_advance(time_inc);
		PROJ_ZOOM = .6;
		
		if(!manager->gms->usercam && active_camera && camera_roll_stop != 1)
		{
			float crt_start = camera_roll_time;
			camera_roll_time += time_inc;
			if(camera_roll_time > CAMERA_ROLL_TIME_MAX)
				camera_roll_time = 0;
			
			if(camera_roll_stop == 2 && (camera_roll_time == 0 || (crt_start <= CAMERA_ROLL_TIME_MAX/2.0f && camera_roll_time >= CAMERA_ROLL_TIME_MAX/2.0f)))
				camera_roll_stop = 1;
			else
			{
				float camera_roll_angle = CAMERA_ROLL_ANG_MAX * sinf(6.28f*camera_roll_time/CAMERA_ROLL_TIME_MAX);
				float ca, sa;
				po ac_po = active_camera->get_rel_po();
				fast_sin_cos_approx(camera_roll_angle*(3.14f/180), &sa, &ca);
				po roll = po(ca, -sa, 0,   sa, ca, 0,  0, 0, 1,  0, 0, 0);
				ac_po = ac_po*roll;
				active_camera->set_rel_po(ac_po);
				
				if(!porthole)
				{
					porthole = entity_manager::inst()->find_entity(entity_id("W1_FREESURF"), IGNORE_FLAVOR);
					porthole_start_po = porthole->get_rel_po();
					vector3d pos = porthole_start_po.get_position();
					pos.y = 0.85f;		// porthole was a little low
					porthole_start_po.set_position(pos);
				}
				porthole->set_rel_po(porthole_start_po*roll);
			}
		}
		
		active_camera->adjust_geometry_pipe();
		
		//		if(!mem_malloc_locked()) mem_lock_malloc(true);
	}
	if((cur_state == STATE_SURFER1 || cur_state == STATE_SURFER2 || cur_state == STATE_MAIN) && surfer_tree)
	{
		if(surfer_tree->is_finished())
			SurferPlay(next_anim);
	}
	
	// See if we need to load more
#if defined(ASYNC_PLAYER_LOAD) && !defined(REAL_ASYNC_LOAD)
	if((cur_state == STATE_SURFER1 || cur_state == STATE_SURFER2) && !surfer_loaded)
	{
		CheckStashLoadStatus();
	}
#endif
	
	
	if (os_developer_options::inst()->is_flagged(os_developer_options::FLAG_DO_CHAR_SCREEN_CAP))
	{
		// Do a screen capture
		if (frameDelay <= 6)
			frameDelay++;
#ifndef TARGET_GC
		if (frameDelay == 6)
			nglScreenShot();
#endif //TARGET_GC
	}
	
	if(cur_state == STATE_TRICK && OKtoDrawBio() && surfer_tree)
	{
		// only interrupt the current animation when the current is ambient, and the next is special (activated by user)
		if(surfer_tree->is_finished())
			SurferPlay(SA_TRICK, 0, -1);

		if(surfer_ent)
		{
			AlignSurferWithBoard();
		}
	}
	if(cur_state == STATE_BOARD && board_tree)
	{
		if(board_tree->is_finished())
			BoardPlay(BA_SPIN);
	}

	PerformIK();
	if (SurferDataArray[current_surfer_index].goofy && surfer_ent && board)
	{
		if ((cur_state == STATE_TRICK) && (cam_stopped_at == CAM_POS_WALL_2_OUT))
		{
			surfer_ent->UpdateHandedness();
			board->UpdateHandedness();
		}
	}

	if (fe_camera)
	{
		vector3d in;
		nlVector3d out, forw, up, pos;
		in = fe_camera->get_abs_position() - 5*fe_camera->get_abs_po().get_facing();
		out[0] = in.x; out[1] = in.y; out[2] = in.z;
		if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
			nslSetEmitterPosition(behindTheCamera, out);
		in = fe_camera->get_abs_po().get_facing();
		forw[0] = in.x; forw[1] = in.y; forw[2] = in.z; 
		in = fe_camera->get_abs_po().get_y_facing();
		up[0] = in.x; up[1] = in.y; up[2] = in.z; 
		in = fe_camera->get_abs_position();
		pos[0] = in.x; pos[1] = in.y; pos[2] = in.z; 
		if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
		{
			nslSetListenerPosition(pos);
			nslSetListenerOrientation(forw, up);
		}

	}

	RELEASESEMA(StallSurferLoadSema);
}

// total hack!!!
void FEEntityManager::SetConglomPo(entity* c, po p)
{
	if(!c) return;
	if(c->has_link_ifc())
	{
		link_interface* li = c->link_ifc();
		entity* c1 = (entity*) li->get_first_child();
		while(c1)
		{
			SetConglomPo(c1, p);
			c1 = (entity*) c1->link_ifc()->get_next_sibling();
		}
		c->set_rel_po(p);
	}
}



void FEEntityManager::RenderConglom(entity* c)
{
	if(!c) return;
	if(c->has_link_ifc())
	{
		link_interface* li = c->link_ifc();
		entity* c1 = (entity*) li->get_first_child();
		while(c1)
		{
			RenderConglom(c1);
			c1 = (entity*) c1->link_ifc()->get_next_sibling();
		}
		c->render(active_camera,1.0f, RENDER_OPAQUE_PORTION, 1.0f);
	}
}

void FEEntityManager::DrawFront()
{
	// Trick Book uses same algorithm as Bio for when to draw
	if(cur_state == STATE_TRICK && OKtoDrawBio())
	{
		int x1 = 348;
		int y1 = 98;
		int x2 = 536;
		int y2 = 282;
		adjustCoords(x1, y1);
		adjustCoords(x2, y2);
		// adjust the viewport here to draw the surfer only in the window of the trick book
		// nglSetViewport operates differently in later versions of ngl
#if NGL < 0x010700
		nglSetViewport(x1, y1, x2, y2);
#endif 
		nglSetWorldToViewMatrix(*(nglMatrix*)&geometry_manager::inst()->xforms[geometry_manager::XFORM_WORLD_TO_VIEW] );
		//surfer_ent[loading_surfer_ent?0:1]->render(active_camera, 1.0f, RENDER_OPAQUE_PORTION, 1.0f);

		// Create a separate nglLightContext for the trickbook. (dc 07/12/02)
		// The color and direction are the same as for the front end scene, but we need to split off 
		// a second copy in order to properly use nglLightContexts.
		nglLightContext *new_lc = nglCreateLightContext();
		static nglVector trickbook_light_dir(-.718370f, -.676876f, -.160575f, 1.f);
		static nglVector trickbook_light_color(.75f, .720589f, .685294, 1.f);
		nglListAddDirLight(NGLLIGHT_LIGHTCAT_1, trickbook_light_dir, trickbook_light_color);
		nglLightContext *old_lc = g_world_ptr->set_current_light_context(new_lc);

		surfer_ent->render(active_camera, 1.0f, RENDER_OPAQUE_PORTION, 1.0f);
		RenderConglom(board);

		g_world_ptr->set_current_light_context(old_lc);
	}
}

void FEEntityManager::DrawBack()
{
	if(cur_state != STATE_OTHER)
	{
		// if in trickbook, surfer is drawn elsewhere
		// if in flyin, surfer is not drawn at all
		if(in_flyin || (cur_state == STATE_TRICK && OKtoDrawBio()))
		{
			surfer_ent->set_visible(false);
			if (board)
				board->set_visible(false);
		}
		g_world_ptr->render(active_camera, 0);
		if(in_flyin || (cur_state == STATE_TRICK && OKtoDrawBio()))
		{
			surfer_ent->set_visible(true);
			if (board)
				board->set_visible(true);
		}
	}
}


void FEEntityManager::ReloadBoardTextures(int which)
{
	stringx hero_filename = SurferFrontEnd::getName(which);
	stringx texture_path = "characters\\" + hero_filename + "\\" + os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR) + "\\";
	nglSetTexturePath( (char *)texture_path.c_str() );
	// In the future, we will modify the IFL in memory
	//#ifndef TARGET_GC
	// i don't know why this was 3 before (it should have been 4?)
	// but board_num makes more sense
//	BOARD_ReloadTextures(board, NULL, which, -1);
}

//#if defined(REAL_ASYNC_LOAD) || defined(ASYNC_PLAYER_LOAD)
#define POSHASH(a) ((a).x+2.7f*(a).y+5.92f*(a).z)
void FEEntityManager::DoneAsyncLoad()
{
	
	GETSEMA(StallSurferLoadSema);
	LoadSurfer(loading_surfer_index);
	//SetSurferPo(surfer_ent[(loading_surfer_ent?0:1)], loading_surfer_index);
	SetSurferPo(surfer_ent, loading_surfer_index);
	//LoadBoard(0);
	
	g_world_ptr->recompute_all_sectors();
	SurferPlay(SA_IDLE);
	//surfer_ent[(loading_surfer_ent?0:1)]->get_anim_tree(ANIM_PRIMARY)->set_time(0);
	if (surfer_ent->get_anim_tree(ANIM_PRIMARY))
		surfer_ent->get_anim_tree(ANIM_PRIMARY)->set_time(0);
	// Hack to prevent the surfer from "sitting" in his first animation
	g_world_ptr->frame_advance(.01f);
	g_world_ptr->frame_advance(.01f);
	if (loading_surfer_ent) 
		endMarker1 = mem_set_checkpoint();
	else
		endMarker2 = mem_set_checkpoint();
	
	RELEASESEMA(StallSurferLoadSema);
}

bool FEEntityManager::UnloadCurrentSurfer()
{
  if (surfer_ent)
  {
	  surfer_ent->kill_anim(ANIM_PRIMARY);
	  surfer_ent->unload_anim(makeString(current_surfer_index, SA_IDLE));
	  surfer_ent->unload_anim(makeString(current_surfer_index, SA_SPECIAL_OCC));
	  surfer_ent->unload_anim(makeString(current_surfer_index, SA_OCC_1));
	  surfer_ent->unload_anim(makeString(current_surfer_index, SA_OCC_2));
		nglReleaseMeshFile(((entity *)surfer_ent)->fileName.c_str());
		nglReleaseMeshFile((((entity *)surfer_ent)->fileName + stringx("_lo")).c_str());
		surfer_ent->kill_anim(ANIM_PRIMARY);
		g_entity_maker->destroy_entity(surfer_ent);
    surfer_ent = NULL;
    surfer_loaded = false;
    return true;
  }
  surfer_loaded = false;
  return false;
}
//#endif

void FEEntityManager::UnloadCurrentBoard()
{
	if (board)
	{
		if (my_rotate_object && my_board)
		{
			my_rotate_object->link_ifc()->remove_child(board);
			my_board->link_ifc()->remove_child(my_rotate_object);
		}

		BOARD_UnloadMesh(board);
		board->kill_anim(ANIM_PRIMARY);
		g_entity_maker->destroy_entity(board);
	}
	board = NULL;
}

bool FEEntityManager::UpdateSurferIndex(int csi, bool force_load)
{
	// if surfer index hasn't changed, don't do anything
	// if(csi == current_surfer_index && !force_load) return true;
#ifdef REAL_ASYNC_LOAD
	//stash::AbortAsyncRead(STASH_SURFER_2);
	//stash::AbortAsyncRead(STASH_SURFER);
#endif
	if(csi >= SURFER_LAST)
	{
		dont_draw_surfer = true;
		return false;
	}
	
	dont_draw_surfer = false;
	stringx abbr = SurferFrontEnd::getAbbr(csi);
	if(SurferFrontEnd::getPersonalityUp()) abbr = abbr+"_P";
  


		
	abbr = SurferFrontEnd::getAbbr(csi);
	// no reason to update if the surfer's already the same,
	// but also to elimiate redundant SurferPlay calls in ToSurferSelect1
	if((manager->gms->multiplayer && !manager->gms->multi_1 && surfer_index_2 != csi) ||!surfer_ent || (current_surfer_index != csi) || force_load || (old_pers != SurferFrontEnd::getPersonalityUp()))
	{
		UnloadCurrentBoard();
		UnloadCurrentSurfer();
		if(manager->gms->multiplayer && !manager->gms->multi_1)
		{
			surfer_index_2 = csi;
			loading_surfer_index = csi;
		}
		else loading_surfer_index = csi;
					
		
		stringx hero_filename;
		if(SurferFrontEnd::getPersonalityUp()) hero_filename = SurferDataArray[csi].name_ps;
		else hero_filename = SurferDataArray[csi].name;
		
		char name[40];
		hero_filename.to_upper();
		strcpy(name, "SURFERS\\");
		if(SurferFrontEnd::getPersonalityUp())
		{
			strcat(name, "PERSON");
			strcat(name, abbr.c_str());
		}
		else strcat(name, SurferDataArray[csi].stashfile);
		strcat(name, ".ST2");
		
		


		// change this thread's priority to 1
#ifdef TARGET_PS2
		mainThreadId = GetThreadId();
		ChangeThreadPriority(mainThreadId, 1);
#endif
		
		if (os_developer_options::inst()->is_flagged(os_developer_options::FLAG_STASH_ONLY))
		{
			// Close any open stashes
			stash::close_stash_async(STASH_SURFER);
			stash::close_stash_async(STASH_SURFER_AUX);
			stash::free_stored(STASH_SURFER);
			stash::free_stored(STASH_SURFER_AUX);
			stash::close_stash_async(STASH_SURFER_BOARD);
			stash::free_stored(STASH_SURFER_BOARD);

			startMarker1 = mem_set_checkpoint();
			mem_push_current_heap(SURFER_HEAP);
			stash::open_stash(name, STASH_SURFER);
			mem_pop_current_heap();
			DoneAsyncLoad();
		}
		else
		{
			DoneAsyncLoad();
			SurferPlay(SA_IDLE);
		}
	}
	
	// Originally all surfers were flipped so that their reflection would be "reversed"
	// but Jeff requested that TP and TH be flipped again so their animations would 
	// appear different. -Beth
	if(current_surfer_index == SURFER_TRAVIS_PASTRANA || current_surfer_index == SURFER_TONY_HAWK)
		surfer_ent->set_render_scale(vector3d(1, 1, 1));
	else surfer_ent->set_render_scale(vector3d(-1, 1, 1));

	// reset the animation "position"
	surfer_anim_count = 0;
	return true;
}


#if defined(ASYNC_PLAYER_LOAD) && !defined(REAL_ASYNC_LOAD)

void FEEntityManager::CheckStashLoadStatus()
{
	if (os_developer_options::inst()->is_flagged(os_developer_options::FLAG_STASH_ONLY))
	{
		if (stash::is_stash_open(STASH_SURFER))
			return;
		
		stash::read_stash_async(STASH_SURFER);
		// Are we done?
		if (stash::is_stash_open(STASH_SURFER))
		{
			DoneAsyncLoad();
		}
	}
	else
	{
		DoneAsyncLoad();
	}
	// Else do some loading
	
}
#endif


extern stringx g_surfer_anims[_SURFER_NUM_ANIMS];
extern stringx g_board_anims[_BOARD_NUM_ANIMS];


void FEEntityManager::LoadTricks()
{
	if(tricks_loaded) return;

	// I'll need the board, but more importantly, I need the aux stash loaded
	if(manager->gms->multiplayer && !manager->gms->multi_1)
		LoadBoard(1,0);
	else
		LoadBoard(0,0);

	stringx anim_dir = "characters\\COMMON\\ANIMATIONS\\";
	stringx surfer_anim = "";
	stringx board_anim = "";

	entity* cur_surf = surfer_ent;
	if(cur_surf)
	{
		// currently my idle anim
		cur_surf->load_anim(anim_dir+"ALL_TRK_TUBE_FS_1");
		for(int i=0; i<TRICK_NUM; i++)
		{
			if(TrickBookFrontEnd::TrickOK(i))
			{
				SurferTrick* st = &(GTrickList[i]);
				if( st->anim_id != SURFER_ANIM_NULL )
				{
					surfer_anim = anim_dir+g_surfer_anims[st->anim_id];
					if(file_finder_exists(surfer_anim, ".ANMX"))
						cur_surf->load_anim(surfer_anim);
					else
					{
#ifdef BETH
						nglPrintf("BETH: surfer anim %s load failed\n", surfer_anim.data());
#endif
						g_surfer_anims[st->anim_id] = g_surfer_anims[SURFER_ANIM_PLACEHOLDER];
					}
				}

				if( st->board_anim_id != BOARD_ANIM_NULL )
				{
					board_anim = anim_dir+g_board_anims[st->board_anim_id];
					if(file_finder_exists(board_anim, ".ANMX"))
						board->load_anim(board_anim);
					else
					{
#ifdef BETH
						nglPrintf("BETH: board anim %s load failed\n", board_anim.data());
#endif
						g_board_anims[st->board_anim_id] = g_board_anims[BOARD_ANIM_ZERO];
					}
				}
			}
		}
	}
	tricks_loaded = true;
}


void FEEntityManager::Anim(stringx anim, float blend_time, bool loop, float time)
{
	//int index = (loading_surfer_ent?0:1);
	surfer_tree = surfer_ent->get_anim_tree(ANIM_PRIMARY);
	bool no_tween;
	
	no_tween = !surfer_tree || (blend_time <= 0.01f);
	
	if(time > 0.0f)
	{
		surfer_tree = surfer_ent->play_anim(ANIM_PRIMARY, anim, time, (loop ? ANIM_LOOPING : 0) | (no_tween ? 0 : ANIM_TWEEN) | ANIM_NONCOSMETIC);
		surfer_tree->frame_advance(0.0f);
	}
	else
	{
		surfer_ent->play_anim(ANIM_PRIMARY, anim, 0.0f, (loop ? ANIM_LOOPING : 0) | (no_tween ? 0 : ANIM_TWEEN) | ANIM_NONCOSMETIC);
		surfer_tree = surfer_ent->get_anim_tree(ANIM_PRIMARY);
		if (!no_tween) surfer_tree->set_tween_duration(blend_time);
	}
}

void FEEntityManager::BoardAnim(int anim_id, float blend_time, bool loop, float time )
{
	board_tree = board->get_anim_tree(ANIM_PRIMARY);
	bool no_tween = false;
	
	if (anim_id == BOARD_ANIM_ZERO)
	  board->set_rel_po(po_identity_matrix);

	no_tween = !board_tree || (blend_time == 0.0f);
	
	if(time > 0.0f)
	{
		board_tree = board->play_anim(ANIM_PRIMARY, g_board_anims[anim_id], time, (loop ? ANIM_LOOPING : 0) | (no_tween ? 0 : ANIM_TWEEN) | ANIM_NONCOSMETIC);
		board_tree->frame_advance(0.0f);
	}
	else
	{
		board_tree = board->play_anim(ANIM_PRIMARY, g_board_anims[anim_id], 0.0f, (loop ? ANIM_LOOPING : 0) | (no_tween ? 0 : ANIM_TWEEN) | ANIM_NONCOSMETIC);
		if (!no_tween)
			board_tree->set_tween_duration(blend_time);
	}
}

void FEEntityManager::PlaySurferTrick(SurferTrick* trick_id)
{
	assert(cur_state == STATE_TRICK);
	assert(trick_id != NULL);
	cur_trick = trick_id->trick_id;
	int animid = trick_id->anim_id;
	int board_animid = trick_id->board_anim_id;

	//   total hack,  I don't know why I have to do this.  The bone handedness code should handle board flips on it's own
	if (SurferDataArray[current_surfer_index].goofy)
	{
		if (cur_trick == TRICK_LEFT_FLIP)
			board_animid = GTrickList[TRICK_RIGHT_FLIP].board_anim_id;
		else if (cur_trick == TRICK_RIGHT_FLIP)
			board_animid = GTrickList[TRICK_LEFT_FLIP].board_anim_id;
	}

	SurferPlay(SA_TRICK, 0.0f, animid, board_animid);
}

void FEEntityManager::SurferPlay(int type, float time, int trick_num, int b_trick_num, bool no_blend)
{
	if(type == SA_TRICK)
	{
		TrickPlay(trick_num);
		BoardPlay(BA_TRICK, b_trick_num);
	}
	else if(current_surfer_index < SURFER_LAST)
	{
//		float blend_time = 0.30f;
//		static bool same_type_blend = false;
//		if((cur_anim == type && !same_type_blend) || no_blend) blend_time = 0.0f;
		float blend_time = 0.0f;	// take out all non-trick blends, for now
		
		if(surfer_ent)
		{
			Anim(makeString(current_surfer_index, type), blend_time, false, time);
			if (surfer_tree)
				surfer_ent->get_anim_tree(ANIM_PRIMARY)->frame_advance(0.0f);
		}
		
		cur_anim = type;

		surfer_anim_count++;
		if(surfer_anim_count == 6)
		{
			next_anim = SA_SPECIAL_OCC;
			surfer_anim_count = 0;
		}
		else next_anim = SA_IDLE;

		
		cur_trick_anim = -2;	// no trick being played
	}
}

void FEEntityManager::TrickPlay(int anim_id)
{
	float blend_time = 0.3f;
	if(anim_id == cur_trick_anim || cur_trick_anim == -2) blend_time = 0.0f;

	if(surfer_ent)
	{
		if(anim_id != -1)
		{
			Anim(g_surfer_anims[anim_id], blend_time, false, 0.0f);
			trick_playing = true;
		}
		else
		{
			//  hack for exit moves to pop to idle
			for (int n = TRICK_EXIT_5; n <= TRICK_EXIT_4; n++)
			{
				if (GTrickList[n].anim_id == cur_trick_anim)
					blend_time = 0.0f;
			}

			stringx anim = "ALL_TRK_TUBE_FS_1";
			Anim(anim, blend_time, false, 0.0f);
			trick_playing = false;
		}
		next_anim = SA_TRICK;
		cur_anim = SA_TRICK;
		cur_trick_anim = anim_id;
	}
}

void FEEntityManager::BoardPlay(int type, int anim_id, float time)
{
	if(type == BA_TRICK)
	{
		if(anim_id == -1) anim_id = 0;
		float blend_time = 0.3f;
		if(anim_id == cur_b_anim) blend_time = 0.0f;

		if (anim_id == 0)
		{
			//  hack for exit moves to pop to idle
			for (int n = TRICK_EXIT_5; n <= TRICK_EXIT_4; n++)
			{
				if (GTrickList[n].board_anim_id == cur_b_anim)
					blend_time = 0.0f;
			}
		}

		BoardAnim(anim_id, blend_time, false, time);
		cur_b_anim = anim_id;
	}
	else
	{
		if(time == -1.0f) time = 0.0f;

		switch(type)
		{
		case BA_SPIN:
			board_tree = board->play_anim("characters\\frontend\\animations\\BOARD_SELECT_360." PLATFORM_ANIM_NAME, time);
			break;
		}
	}
}
nslSoundId watersnd = NSL_INVALID_ID;
nslSourceId watersrc = NSL_INVALID_ID;

void FEEntityManager::ToMainScreen()
{
	if(op_ext) { op_ext = false; return; }
	if(current_surfer_index >= SURFER_LAST) current_surfer_index = 0;

#ifdef REAL_ASYNC_LOAD
	stash::WaitForStashLoad();
#endif

//	if(!surfer_loaded) LoadSurfer(loading_surfer_index);
//	if(!mm_anims_loaded) LoadSurferAnims(loading_surfer_index);

	// this loads everything in the return to FE case
	if(!all_loaded) manager->gms->LoadAll();

	SetSurferPo(surfer_ent, loading_surfer_index);
	
	
	CameraAnim(CAM_POS_WALL_1);
	
	SurferPlay(SA_IDLE);

	op_ext = false;
	cur_state = STATE_MAIN;
	// These just get the camera positioned correctly
	for (int i=0; i < 6; i++)
		Update(.01f);
}

void FEEntityManager::ToSurferSelect()
{
	if(!all_loaded) manager->gms->LoadAll();
	if (SurferDataArray[current_surfer_index].goofy && surfer_ent && board)
	{
		surfer_ent->set_handed_axis(-1);
		board->set_handed_axis(-1);
	}
	
	CameraAnim(CAM_POS_WALL_2_IN);

	SurferPlay(SA_IDLE, -1, 0, 0, true);

	if(surfer_ent) 
		SetSurferPo(surfer_ent, loading_surfer_index);

	// Originally all surfers were flipped so that their reflection would be "reversed"
	// but Jeff requested that TP and TH be flipped again so their animations would 
	// appear different. -Beth
	if(current_surfer_index == SURFER_TRAVIS_PASTRANA || current_surfer_index == SURFER_TONY_HAWK)
		surfer_ent->set_render_scale(vector3d(1, 1, 1));
	else surfer_ent->set_render_scale(vector3d(-1, 1, 1));

	op_ext = false;
	cur_state = STATE_SURFER1;
	next_anim = SA_IDLE;
}


void FEEntityManager::ToBeachSelect()
{
	// this loads everything in the return to FE case
	if(!all_loaded) manager->gms->LoadAll();
	((BeachFrontEnd *)manager->gms->menus[GraphicalMenuSystem::BeachMenu])->SkipSlide(true);
	
	if (!surfer_ent)
	{
		if ((manager->gms->multiplayer && manager->gms->multi_1) || !manager->gms->multiplayer)
			UpdateSurferIndex(g_game_ptr->GetSurferIdx(0));
		else
			UpdateSurferIndex(g_game_ptr->GetSurferIdx(1));
	}
	
	SetSurferPo(surfer_ent, loading_surfer_index);

	CameraAnim(CAM_POS_WALL_3_MAP);
	moveClosetDoor(true, false);
	op_ext = false;
	cur_state = STATE_BEACH;
}

// size is height in inches
void FEEntityManager::ToBoardSelect(int bd, int tail_type, int size)
{
#ifdef REAL_ASYNC_LOAD
	stash::WaitForStashLoad();
#endif
	
	bool pers = SurferFrontEnd::getPersonalityUp();
	int csi = current_surfer_index;
	stringx abbr = SurferFrontEnd::getAbbr(csi);
	stringx hero_filename;

	if(pers) hero_filename = "PERSONALITY"+abbr;
	else hero_filename = SurferFrontEnd::getName(csi);

	if(pers) abbr = abbr+"_P";
	UnloadCurrentSurfer();
	LoadAuxStash();	
	CameraAnim(CAM_POS_WALL_3_CLOSET);
	if ((frontendmanager.gms->multiplayer && frontendmanager.gms->multi_1) || !frontendmanager.gms->multiplayer)
		LoadBoard(0, bd);
	else
		LoadBoard(1, bd);

	moveClosetDoor(true, true);
	current_board_index = bd;
	cur_state = STATE_BOARD;
	my_board->set_rel_po(board_select_po);
	SetConglomScale(board, vector3d(1, 1, 1));
	board->set_visible(true);

	UpdateBoardIndex(current_board_index, tail_type, size, true);
	if(board_tree)
		BoardPlay(BA_SPIN, 0, board_tree->get_time());
	else BoardPlay(BA_SPIN);

	op_ext = false;
}

void FEEntityManager::ToTrickBook()
{
	LoadAuxStash();
//	LoadBoard(0, 0);
	int hero_num = 0;
	if(manager->gms->multiplayer && !manager->gms->multi_1) hero_num = 1;
	LoadBoard(hero_num, 0);
	op_ext = false;
	cur_state = STATE_TRICK;
	LoadTricks();
}

void FEEntityManager::ToTrickBook2()
{
	float scale = 0.5f;
	if (SurferDataArray[current_surfer_index].goofy && board && surfer_ent)
	{
		board->set_handed_axis(2);
		surfer_ent->set_handed_axis(2);
		scale = -0.5f;
	}

	po trans( po_identity_matrix );
	trans.set_rotate_y(scale*PI);
	surfer_trick_po = po(1, 0, 0,   0, 1, 0,   0, 0, 1,   0.0f, 0.0f, 0.0f);
	surfer_trick_po = trans*surfer_trick_po;
	surfer_trick_po.set_position(vector3d(-3.6, 2.3, -9));
	board_trick_po = surfer_trick_po;
	board_trick_po.set_position(surfer_trick_po.get_position() + vector3d(0, -.6, 0));

	my_board->set_rel_po(board_trick_po);
	if(surfer_ent)
	{
		surfer_ent->set_render_scale(vector3d(1, 1, 1));
	}

	cur_anim = SA_TRICK;
	cur_b_anim = 0;
	BoardPlay(BA_TRICK, 0);
	SurferPlay(SA_TRICK, 0, -1);
}

void FEEntityManager::UpdateBoardIndex(int b, int tail_type, int size, bool draw)
{
	


	current_board_index = b;
	board_tail_type = tail_type;
	int surfer_index = current_surfer_index;
	if(manager->gms->multiplayer && !manager->gms->multi_1)
		surfer_index = surfer_index_2;

	if(manager->gms->multiplayer && !manager->gms->multi_1)
		LoadBoard(1,b);
	else
		LoadBoard(0,b);	
	board->set_visible(true);
	my_board->set_rel_po(board_select_po);
	g_world_ptr->recompute_all_sectors();
	if (my_board->get_anim_tree(ANIM_PRIMARY))
		my_board->get_anim_tree(ANIM_PRIMARY)->set_time(0);
	// Hack to prevent the surfer from "sitting" in his first animation
	g_world_ptr->frame_advance(.01f);
	g_world_ptr->frame_advance(.01f);

	//SetConglomTexture(board, b);
}

stringx FEEntityManager::makeString(int surfer, int type)
{
	stringx abbr = SurferFrontEnd::getAbbr(surfer);
	stringx ret = "characters\\FRONTEND\\animations\\";
	bool pers = SurferFrontEnd::getPersonalityUp();

	switch(type)
	{
	case SA_SPECIAL_OCC:
		if(pers) ret = ret + abbr + "_SUIT." PLATFORM_ANIM_NAME;
		else ret = ret + abbr + "_CUSTOM." PLATFORM_ANIM_NAME;
		break;
	case SA_OCC_1:			ret = ret + "ALL2_FE." PLATFORM_ANIM_NAME; break;
	case SA_OCC_2:			ret = ret + "ALL3_FE." PLATFORM_ANIM_NAME; break;
	default:				ret = ret + abbr + "_BREATHE." PLATFORM_ANIM_NAME; break;
	}
	return ret;

}


void FEEntityManager::ExitState()
{
	last_state = cur_state;
	cur_state = STATE_OTHER_SET;
}

void FEEntityManager::JumpTo(int pos)
{
	if(cam_pos_goal == pos && cam_anim_tree && cam_anim_tree->is_suspended()) return;
	if(pos == CAM_POS_WALL_2_IN || pos == CAM_POS_WALL_3_MAP || pos == CAM_POS_WALL_2_OUT)
		camera_roll_stop = 1;
	if(cam_reverse) CameraAnim(pos, stops[pos]+.1f);
	else CameraAnim(pos, stops[pos]-.1f);
}

void FEEntityManager::BioTBZoom(bool out)
{
	if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
	{
		SoundScriptManager::inst()->playEvent(SS_WALL_CAMERA_ZOOM);

		SSEventId s = SoundScriptManager::inst()->playEvent(SS_WALL_CAMERA_ZOOM);
		nslSoundId snd = SoundScriptManager::inst()->getSoundId(s);
		if (nslGetSoundStatus(snd) != NSL_SOUNDSTATUS_INVALID)
		{
			nslSetSoundEmitter(behindTheCamera, snd);
			nslSetSoundParam(snd, NSL_SOUNDPARAM_PITCH, .9);
		}
	}
	if(out) CameraAnim(CAM_POS_WALL_2_OUT);
	else CameraAnim(CAM_POS_WALL_2_IN);
}

void FEEntityManager::CameraAnim(int anim_pos, float start_time)
{
	if(cam_pos_goal == anim_pos && start_time == -2) return;	// nothing needs to be done
	cam_pos_goal = anim_pos;
	cam_stopped = false;

	if(cam_pos_goal == CAM_POS_WALL_2_IN || cam_pos_goal == CAM_POS_WALL_3_MAP || cam_pos_goal == CAM_POS_WALL_2_OUT)
	{
		if(camera_roll_stop == 0)
			camera_roll_stop = 2;	// prepare to stop roll anim
	}
	else if(camera_roll_stop == 1 || camera_roll_stop == 2)
		camera_roll_stop = 0;

	float goal = stops[anim_pos];
	float time;
	if(start_time == -2)
	{
		if(cam_anim_tree)
		{
			time = g_world_ptr->get_scene_anim_time(cam_anim_handle);
			if(time == -1) return;
		}
		else time = 0.0f;

		if(goal != time)
		{
			bool new_rev = goal < time;
			if(cam_anim_tree)
			{
				cam_anim_tree->set_suspended(false);
				if(cam_reverse && !new_rev)
					cam_anim_tree->set_flag(ANIM_REVERSE, false);
				if(!cam_reverse && new_rev)
					cam_anim_tree->set_flag(ANIM_REVERSE, true);
			}
			else
			{
				cam_anim_handle = g_world_ptr->play_scene_anim("levels\\frontend\\animations\\NewFE_Camera.SNMX", new_rev, time);
				cam_anim_tree = g_world_ptr->get_scene_anim_tree(cam_anim_handle);
			}

			cam_reverse = new_rev;
		}
		else
			if(cam_anim_tree) g_world_ptr->kill_scene_anim(cam_anim_handle);
	}
	else
	{
		time = start_time;
		bool new_rev = goal < time;
		if(cam_anim_tree)
		{
			cam_anim_tree->set_time(start_time);
		}
		else
		{
			cam_anim_handle = g_world_ptr->play_scene_anim("levels\\frontend\\animations\\NewFE_Camera.SNMX", new_rev, time);
			cam_anim_tree = g_world_ptr->get_scene_anim_tree(cam_anim_handle);
		}
	}
}


void FEEntityManager::moveClosetDoor(bool door_left, bool open)
{
	// return if desired state is already current state
	if(closet_state == CLOSET_NOT_MOVING)
	{
		if(door_left && ((open && closet_open[0]) || (!open && !closet_open[0]))) return;
		if(!door_left && ((open && closet_open[1]) || (!open && !closet_open[1]))) return;
	}


	SoundScriptManager::inst()->endEvent(doorEvent);



	if (open)
		doorEvent=SoundScriptManager::inst()->playEvent(SS_CLOSETDOOR_OPEN);
	else
		doorEvent=SoundScriptManager::inst()->playEvent(SS_CLOSETDOOR_CLOSE);

	closet_movement = 1.2;
	if(door_left)
	{
		if(!map) map = entity_manager::inst()->find_entity(entity_id("W3_MAP"), IGNORE_FLAVOR);

		if(!closet_door[1])
		{
			closet_door[1] = entity_manager::inst()->find_entity(entity_id("CLOSET_DOOR02"), IGNORE_FLAVOR);
			closet_pos[1] = closet_door[1]->get_abs_position().z;
		}
		if(open)
		{
			closet_state = CLOSET_LEFT_OPENING;
			if(closet_pos[1] != closet_door[1]->get_abs_position().z)
			{
				float diff = closet_door[1]->get_abs_position().z - closet_pos[1];
				closet_timer = (diff/closet_movement);
			}
			else closet_timer = 0;
		}
		else
		{
			if(closet_pos[1] != closet_door[1]->get_abs_position().z)
			{
				float diff = (closet_pos[1]+closet_movement) - closet_door[1]->get_abs_position().z;
				closet_timer = (diff/closet_movement);
				closet_state = CLOSET_LEFT_CLOSING;
			}
			else closet_timer = 0;
		}
	}
	else
	{
		if(!closet_door[0])
		{
			closet_door[0] = entity_manager::inst()->find_entity(entity_id("CLOSET_DOOR01"), IGNORE_FLAVOR);
			closet_pos[0] = closet_door[0]->get_abs_position().z;
		}
		if(open)
		{
			closet_state = CLOSET_RIGHT_OPENING;
			if(closet_pos[0] != closet_door[0]->get_abs_position().z)
			{
				float diff = closet_pos[0] - closet_door[0]->get_abs_position().z;
				closet_timer = (diff/closet_movement);
			}
			else closet_timer = 0;
		}
		else
		{
			if(closet_pos[0] != closet_door[0]->get_abs_position().z)
			{
				float diff = closet_door[0]->get_abs_position().z - (closet_pos[0]-closet_movement);
				closet_timer = (diff/closet_movement);
				closet_state = CLOSET_RIGHT_CLOSING;
			}
			else closet_timer = 0;
		}
	}
}
void FEEntityManager::LoadAuxStash()
{
	int hero;
	if ((frontendmanager.gms->multiplayer && frontendmanager.gms->multi_1) || !frontendmanager.gms->multiplayer)
		hero =0;
	else
		hero =1;

	bool pers = g_game_ptr->GetUsingPersonalitySuit(hero);
	int csi = g_game_ptr->GetSurferIdx(hero);
	stringx abbr = SurferFrontEnd::getAbbr(csi);
	stringx hero_filename;
	if(pers) hero_filename = "PERSONALITY"+abbr;
	else hero_filename = SurferFrontEnd::getName(csi);
	if(pers) abbr = abbr+"P";
	char name[40];
	char auxName[40];
	hero_filename.to_upper();
	strcpy(name, "SURFERS\\");
	strcpy(auxName, "SURFERS\\AUXSTASH\\");
	strcat(auxName,  SurferDataArray[csi].stashfile);
	strcat(auxName, "\\");
	strcat(name, hero_filename.substr(0, 8).c_str());
	strcat(auxName, abbr.c_str());
	strcat(auxName, "_AUX.ST2");
	strcat(name, ".ST2");
	if (stash::is_stash_open(STASH_SURFER_AUX) && !stricmp(auxName, stash::get_stash_name(STASH_SURFER_AUX)))
		return;
	UnloadCurrentBoard();
	stash::close_stash(STASH_SURFER_AUX);
	stash::free_stored(STASH_SURFER_AUX);
	stash::close_stash(STASH_SURFER_BOARD);
	stash::free_stored(STASH_SURFER_BOARD);
	
	mem_push_current_heap(SURFER_HEAP);
	stash::open_stash(auxName, STASH_SURFER_AUX);
	mem_pop_current_heap();

}
void FEEntityManager::LoadAll()
{
	if(all_loaded) return;
	bool pers = SurferFrontEnd::getPersonalityUp();
	int csi = current_surfer_index;	
	stringx abbr = SurferFrontEnd::getAbbr(csi);
	stringx hero_filename;
	if(pers) hero_filename = "PERSONALITY"+abbr;
	else hero_filename = SurferFrontEnd::getName(csi);
	if(pers) abbr = abbr+"_P";
	char name[40];
	char auxName[40];
	hero_filename.to_upper();
	strcpy(name, "SURFERS\\");
	strcpy(auxName, "SURFERS\\AUXSTASH\\");
		
	stash::close_stash(STASH_SURFER);
	stash::close_stash(STASH_SURFER_AUX);
	
	stash::close_stash(STASH_SURFER_2);
	stash::close_stash(STASH_SURFER_2_AUX);
	loading_surfer_index = g_game_ptr->GetSurferIdx(0);
	previous_surfer_index[0] = loading_surfer_index;
	current_surfer_index = -1;

	
	boardCount = 0;

	board = NULL;	
	LoadSet();
	nlVector3d v;
	memset(&v, 0, sizeof(nlVector3d));
	if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_NO_AUDIO))
		behindTheCamera = nslCreateEmitter(v);
	// Use the right surfer from game_ptr
	UpdateSurferIndex(loading_surfer_index);
	//g_game_ptr->SetSurferIdx(0,loading_surfer_index);


	color32 c = surfer_ent->get_render_color();
	c.set_alpha(255);
	surfer_ent->set_render_color(c);
	LoadAuxStash();
	LoadBoard(0,0);
	board->set_visible(false);

	LoadSurferAnims(0);
	all_loaded = true;
}

void FEEntityManager::LoadSurfer(int s_num)
{
	if(surfer_loaded) return;
	bool pers = SurferFrontEnd::getPersonalityUp();
	
	old_pers =pers;
	
	
	stringx abbr = SurferFrontEnd::getAbbr(s_num);
	stringx hero_filename;
	if(pers) hero_filename = "PERSONALITY"+abbr;
	else hero_filename = SurferFrontEnd::getName(s_num);
	if(pers) abbr = abbr+"_P";
	char auxName[40];
	strcpy(auxName, "SURFERS\\AUXSTASH\\");
	strcat(auxName,  SurferDataArray[s_num].stashfile);
	strcat(auxName, "\\");
	strcat(auxName, abbr.c_str());
	strcat(auxName, "_AUX.ST2");
	
	
	/*** BETH: currently loading from the individual character directories.  Change this back to
	the "characters\\frontend" directory if more than one surfer at a time will be loaded ***/
	
	//	stringx mesh_path = "characters\\frontend\\entities\\";
	stringx mesh_path = stringx("characters\\") + hero_filename + stringx("\\entities\\");
	nglSetMeshPath( (char *)mesh_path.c_str() );
	//	stringx texture_path = "characters\\frontend\\" + os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR) + "\\";
	stringx texture_path = "characters\\" + hero_filename + "\\" + os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR) + "\\";
	nglSetTexturePath( (char *)texture_path.c_str() );
	
	surfer_ent = g_entity_maker->create_entity_or_subclass(stringx("characters\\") + hero_filename + stringx("\\entities\\") + hero_filename, entity_id::make_entity_id(("HERO_"+abbr).data()), po_identity_matrix, "characters\\", ACTIVE_FLAG|NONSTATIC_FLAG );
	if(surfer_ent )
	{
		// Let KS head and Surfreak's eyes animate...
		if(!pers && SurferDataArray[s_num].initially_unlocked)
			surfer_ent->SetTextureFrame(0);

		surfer_ent->load_anim(makeString(s_num, SA_IDLE));
		surfer_ent->load_anim(makeString(s_num, SA_SPECIAL_OCC));
		surfer_ent->load_anim(makeString(s_num, SA_OCC_1));
		surfer_ent->load_anim(makeString(s_num, SA_OCC_2));
		surfer_ent->set_force_hi_res(true);
		color32 c;
		
		current_surfer_index = loading_surfer_index;
		
		/*if (!os_developer_options::inst()->is_flagged(os_developer_options::FLAG_DO_CHAR_SCREEN_CAP))
		{*/
			c = surfer_ent->get_render_color();
			c.set_alpha(255);
			surfer_ent->set_render_color(c);
			
		//}
		int id;
		if (s_num != SURFER_NATHAN_FLETCHER)
		{
			if ((id = nglGetMaterialIdx(surfer_ent->get_mesh(), 99)) != 0)	// sunglasses
			{
				int mask = (1 << id);
				surfer_ent->SetMaterialMask(mask);
			}
		}
		surfer_ent->set_visible(true);
		
		// to flip the surfer when he's seen reflected in the picture
		surfer_ent->set_render_scale(vector3d(-1, 1, 1));

		if (my_ik_object)
			my_ik_object->InitIK(NULL, (conglomerate *) surfer_ent);

	}

	
	//loading_surfer_ent = (loading_surfer_ent?0:1);
	first_run = 0;
	surfer_loaded = true;
	tricks_loaded = false;
//	SurferPlay(SA_IDLE);
	

}

void FEEntityManager::LoadBoard(int hero, int whichboard)
{
	//if(boards_loaded) return; 
	char name[40];
	char justName[20];
	char auxName[40];
	
	int surf_ind = 0;
	int other_surfer_ind = 0;
	
	// if state is trick book, then SurferIdx hasn't been set yet
	if(cur_state == STATE_TRICK)
		surf_ind = loading_surfer_index; 
	else surf_ind = g_game_ptr->GetSurferIdx(hero);
	
    if(manager->gms->multiplayer)
		other_surfer_ind = g_game_ptr->GetSurferIdx((hero==0)?1:0);
	
	
	stringx abbr = SurferFrontEnd::getAbbr(surf_ind);
	stringx other_abbr = SurferFrontEnd::getAbbr(other_surfer_ind);
	stringx hero_filename, other_hero_filename;
	
	bool pers = g_game_ptr->GetUsingPersonalitySuit(hero);
	if(pers) hero_filename = "PERSON"+abbr;
	else hero_filename = SurferDataArray[surf_ind].stashfile;
	
	if(pers) abbr = abbr+"_P";
	
	//stringx hero_filename = SurferFrontEnd::getName(surf_ind);
	hero_filename.to_upper();
	strcpy(justName, abbr.c_str());
	strcat(justName, "_AUX.ST2");
	strcpy(name, "SURFERS\\");
	strcpy(auxName, "SURFERS\\AUXSTASH\\");
	strcat(auxName,  SurferDataArray[g_game_ptr->GetSurferIdx(hero)].stashfile);
	strcat(auxName, "\\");
	strcat(name, hero_filename.substr(0, 8).c_str());
	strcat(auxName, justName);
	strcat(name, ".ST2");
	
	stringx mesh_path = "interface\\FE\\boardmenu\\entities\\";
	nglSetMeshPath( (char *)mesh_path.c_str() );
	
	// In the future, we will modify the IFL in memory
	// that way, we won't have it parsing this huge long thing.  
	stringx texture_path = "interface\\FE\\boardmenu\\" + os_developer_options::inst()->get_string(os_developer_options::STRING_TEXTURE_DIR) + "\\";
	nglSetTexturePath( (char *)texture_path.c_str() );
	// Load up the aux stash if we need to.
	// into the correct spot that is associated with the original surfer.
	// (if we can).  Sometimes, the original surfer's stash won't be there
	// ie the 1st player when selecting for MP.
	UnloadCurrentBoard();
	

	if (whichboard >= MAX_BOARDS)		// Load up the location board
	{
			char myname[100];
			mem_push_current_heap(SURFER_HEAP);
			

			char shortbeachname[10];
			strcpy(shortbeachname, g_game_ptr->get_beach_board_name( whichboard - MAX_BOARDS ).c_str());
			shortbeachname[4] = '\0';
			stringx daname = stringx("SURFERS\\BCHBRD\\") + stringx(shortbeachname) + stringx("_BRD.ST2");
			stash::close_stash(STASH_SURFER_BOARD);
			stash::free_stored(STASH_SURFER_BOARD);
			strcpy(myname, daname.c_str());
			stash::open_stash(myname, STASH_SURFER_BOARD);
			mem_pop_current_heap();

			BOARD_CompletelyUnloadIFLFiles();
			BOARD_GenAndLoadBoardIFLs(g_game_ptr->GetSurferIdx(hero), whichboard, hero, -2, -2);

			nglSetMeshPath("items\\BOARD\\ENTITIES\\");
			nglSetTexturePath("BOARDS\\BEACHES\\TEXTURES\\");
			if (!board)
			{
				board = g_entity_maker->create_entity_or_subclass("items\\BOARD\\ENTITIES\\BOARD", entity_id::make_unique_id(), po_identity_matrix, empty_string, ACTIVE_FLAG);
				board->set_active(true);
				board->set_max_lights( ABSOLUTE_MAX_LIGHTS );
				SetConglomForceHiRes(board, true);	
				g_world_ptr->guarantee_active(board);
				board->create_light_set();
				
			}
	}
	else if (!pers)
	{
		if (stricmp(SurferDataArray[g_game_ptr->GetSurferIdx(hero)].board_ent_name, "board") == 0)
		{
			char myname[100];
			mem_push_current_heap(SURFER_HEAP);
			stringx daname = stringx("SURFERS\\AUXSTASH\\") + stringx(SurferDataArray[g_game_ptr->GetSurferIdx(hero)].stashfile) + stringx("\\");
			stash::close_stash(STASH_SURFER_BOARD);
			stash::free_stored(STASH_SURFER_BOARD);

			daname = daname + stringx(SurferDataArray[g_game_ptr->GetSurferIdx(hero)].abbr) + stringx("_") + stringx(whichboard) + stringx("_BRD.ST2");
			strcpy(myname, daname.c_str());
			stash::open_stash(myname, STASH_SURFER_BOARD);
			mem_pop_current_heap();

			BOARD_CompletelyUnloadIFLFiles();
			/*if ((manager->tmp_game_mode == GAME_MODE_CAREER && g_career->IsBoardUnlocked(whichboard)) ||
			    (!manager->tmp_game_mode == GAME_MODE_CAREER && globalCareerData.isSurferBoardUnlocked(g_game_ptr->GetSurferIdx(hero),whichboard)))
				BOARD_GenAndLoadBoardIFLs(g_game_ptr->GetSurferIdx(hero), whichboard, hero,  -2, -2, -2);*/
			if (unlockManager.isSurferBoardUnlocked(g_game_ptr->GetSurferIdx(hero),whichboard))
				BOARD_GenAndLoadBoardIFLs(g_game_ptr->GetSurferIdx(hero), whichboard, hero,  -2, -2, -2);
			
			nglSetMeshPath("items\\BOARD\\ENTITIES\\");
			nglSetTexturePath((stringx("BOARDS\\") + SurferDataArray[g_game_ptr->GetSurferIdx(hero)].name + stringx("\\TEXTURES\\")).c_str());
			if (!board)
			{
				board = g_entity_maker->create_entity_or_subclass("items\\BOARD\\ENTITIES\\BOARD", entity_id::make_unique_id(), po_identity_matrix, empty_string, ACTIVE_FLAG);
				board->set_active(true);
				board->set_max_lights( ABSOLUTE_MAX_LIGHTS );
				SetConglomForceHiRes(board, true);	
				g_world_ptr->guarantee_active(board);
				board->create_light_set();
				
			}
			/*if ((manager->tmp_game_mode == GAME_MODE_CAREER && g_career->IsBoardUnlocked(whichboard)) ||
			    (!manager->tmp_game_mode == GAME_MODE_CAREER && globalCareerData.isSurferBoardUnlocked(g_game_ptr->GetSurferIdx(hero),whichboard)))
				board->get_light_set()->my_ambient = color(1.0f, 1.0f, 1.0f,1.0f);*/
			if (unlockManager.isSurferBoardUnlocked(g_game_ptr->GetSurferIdx(hero),whichboard))
				board->get_light_set()->my_ambient = color(1.0f, 1.0f, 1.0f,1.0f);
			else
				board->get_light_set()->my_ambient = color(.1f, 0.1f, 0.1f,1.0f);
					
		}
		else
		{
			char myname[100];
			stringx boardName = SurferDataArray[g_game_ptr->GetSurferIdx(hero)].board_ent_name;
			mem_push_current_heap(SURFER_HEAP);
			stringx daname = stringx("SURFERS\\AUXSTASH\\") + stringx(SurferDataArray[g_game_ptr->GetSurferIdx(hero)].stashfile) + stringx("\\");
			stash::close_stash(STASH_SURFER_BOARD);
			stash::free_stored(STASH_SURFER_BOARD);

			daname = daname + stringx(SurferDataArray[g_game_ptr->GetSurferIdx(hero)].abbr)  + stringx("_") + stringx(whichboard) + stringx("_BRD.ST2");
			strcpy(myname, daname.c_str());
			stash::open_stash(myname, STASH_SURFER_BOARD);
			mem_pop_current_heap();


			nglSetMeshPath((stringx("CHARACTERS\\") + stringx(SurferDataArray[g_game_ptr->GetSurferIdx(hero)].name) + stringx("\\Entities\\")).c_str());
			nglSetTexturePath((stringx("BOARDS\\") + stringx(SurferDataArray[g_game_ptr->GetSurferIdx(hero)].name) + stringx("\\TEXTURES\\")).c_str());
			if (!board)
			{
				stringx boardFile = stringx("CHARACTERS\\") + SurferDataArray[g_game_ptr->GetSurferIdx(hero)].name + "\\ENTITIES\\" + boardName;
				board = g_entity_maker->create_entity_or_subclass(boardFile.c_str(), entity_id::make_unique_id(), po_identity_matrix, empty_string, ACTIVE_FLAG);
				board->set_active(true);
				board->set_max_lights( ABSOLUTE_MAX_LIGHTS );
				SetConglomForceHiRes(board, true);	
				g_world_ptr->guarantee_active(board);
				board->create_light_set();
				
			}
		}
	}
	else  // psuit
	{
	
		// Hmm.. default case, but really shouldn't happen
		if (stricmp(SurferDataArray[g_game_ptr->GetSurferIdx(hero)].p_board_name, "board") == 0)
		{

			char myname[100];
			mem_push_current_heap(SURFER_HEAP);
			stringx daname = stringx("SURFERS\\AUXSTASH\\") + stringx(SurferDataArray[g_game_ptr->GetSurferIdx(hero)].stashfile) + stringx("\\");
			stash::close_stash(STASH_SURFER_BOARD);
			stash::free_stored(STASH_SURFER_BOARD);

			daname = daname + stringx(SurferDataArray[g_game_ptr->GetSurferIdx(hero)].abbr) + stringx("P") + stringx("_BRD.ST2");
			strcpy(myname, daname.c_str());
			stash::open_stash(myname, STASH_SURFER_BOARD);
			mem_pop_current_heap();

			BOARD_CompletelyUnloadIFLFiles();
			BOARD_GenAndLoadBoardIFLs(g_game_ptr->GetSurferIdx(hero), whichboard, hero, -2, -2, -2);

			nglSetMeshPath("items\\BOARD\\ENTITIES\\");
			nglSetTexturePath((stringx("BOARDS\\") + stringx(SurferDataArray[g_game_ptr->GetSurferIdx(hero)].name_ps) + stringx("\\TEXTURES\\")).c_str());
			if (!board)
			{
				board = g_entity_maker->create_entity_or_subclass("items\\BOARD\\ENTITIES\\BOARD", entity_id::make_unique_id(), po_identity_matrix, empty_string, ACTIVE_FLAG);
				board->set_active(true);
				board->set_max_lights( ABSOLUTE_MAX_LIGHTS );
				SetConglomForceHiRes(board, true);	
				g_world_ptr->guarantee_active(board);
				board->create_light_set();
				
			}
		}
		else
		{

			char myname[100];
			mem_push_current_heap(SURFER_HEAP);
			stringx daname = stringx("SURFERS\\AUXSTASH\\") + stringx(SurferDataArray[g_game_ptr->GetSurferIdx(hero)].stashfile) + stringx("\\");
			daname = daname + stringx(SurferDataArray[g_game_ptr->GetSurferIdx(hero)].abbr) + stringx("P") + stringx("_BRD.ST2");
			stash::close_stash(STASH_SURFER_BOARD);
			stash::free_stored(STASH_SURFER_BOARD);
			strcpy(myname, daname.c_str());
			stash::open_stash(myname, STASH_SURFER_BOARD);
			mem_pop_current_heap();

			stringx boardName = SurferDataArray[g_game_ptr->GetSurferIdx(hero)].p_board_name;

			nglSetMeshPath((stringx("characters\\") + stringx(SurferDataArray[g_game_ptr->GetSurferIdx(hero)].name_ps) + stringx("\\Entities\\")).c_str());
			nglSetTexturePath((stringx("BOARDS\\") + stringx(SurferDataArray[g_game_ptr->GetSurferIdx(hero)].name_ps) + stringx("\\TEXTURES\\")).c_str());

			if (!board)
			{
				stringx boardFile = stringx("characters\\") + SurferDataArray[g_game_ptr->GetSurferIdx(hero)].name_ps + "\\ENTITIES\\" + boardName;
				board = g_entity_maker->create_entity_or_subclass(boardFile.c_str(), entity_id::make_unique_id(), po_identity_matrix, empty_string, ACTIVE_FLAG);
				board->set_active(true);
				board->set_max_lights( ABSOLUTE_MAX_LIGHTS );
				SetConglomForceHiRes(board, true);	
				g_world_ptr->guarantee_active(board);
				board->create_light_set();
				
			}
		}


	}
	assert(board);

	my_board_member = ((conglomerate*)board)->get_member("BOARD");

	if (board && board->has_link_ifc())
	{
		board->link_ifc()->set_parent(my_rotate_object);
		board->set_rel_po(po_identity_matrix);
	}

	if (my_rotate_object->has_link_ifc())
	{
		my_rotate_object->link_ifc()->set_parent(my_board);
		my_rotate_object->set_rel_po(po_identity_matrix);
		po trans( po_identity_matrix );
		trans.set_rotate_y(-0.5f*PI);
		my_rotate_object->set_rel_po(trans);
	}

	my_parent_node = my_rotate_object;
	
	// if board is null, then we can't exactly do this before the for loop, right?
	//ReloadBoardTextures(surf_ind);
	
	
	board->load_anim("characters\\frontend\\animations\\BOARD_SELECT_360." PLATFORM_ANIM_NAME);
	BoardPlay(BA_SPIN);
	boards_loaded = true;
}

void FEEntityManager::LoadSet()
{
	if(fe_camera) return;

	g_world_ptr->load_fe_scene("levels\\frontend", "newfrontend");

	g_world_ptr->recompute_all_sectors();

	fe_camera = g_world_ptr->get_marky_cam_ptr();
	g_game_ptr->set_current_camera(fe_camera);
	PROJ_ZOOM = .6;
	fe_camera->adjust_geometry_pipe();
	active_camera = fe_camera;

	// tedious way to remove material fog flags
	world_dynamics_system::entity_list::const_iterator ei = g_world_ptr->get_entities().begin();
	world_dynamics_system::entity_list::const_iterator ei_end = g_world_ptr->get_entities().end();
	for ( ; ei!=ei_end; ei++ )
	{
		entity* ent = *ei;
		if(ent)
		{
			nglMesh *Mesh = ent->get_mesh();
			if(Mesh)
			{
				for(int i=0; i<(signed)Mesh->NSections; i++)
				{
					nglMeshSection* Section = &Mesh->Sections[i];
					nglMaterial* Material = Section->Material;
					Material->Flags &= ~NGLMAT_FOG;
				}
			}
		}
	}
	g_world_ptr->load_scene_anim("levels\\frontend\\animations\\NewFE_Camera.SNMX");
}


void FEEntityManager::LoadSurferAnims(int surfer)
{
	if(mm_anims_loaded) return;
	mm_anims_loaded = true;
}


void FEEntityManager::SetConglomTexture(entity* c, int b)
{
	if(!c) return;
	if(c->has_link_ifc())
	{
		link_interface* li = c->link_ifc();
		entity* c1 = (entity*) li->get_first_child();
		while(c1)
		{
			SetConglomTexture(c1, b);
			c1 = (entity*) c1->link_ifc()->get_next_sibling();
		}
		c->SetTextureFrame(b);
	}
}

void FEEntityManager::SetConglomScale(entity* c, vector3d s)
{
	if(!c) return;
	if(c->has_link_ifc())
	{
		link_interface* li = c->link_ifc();
		entity* c1 = (entity*) li->get_first_child();
		while(c1)
		{
			SetConglomScale(c1, s);
			c1 = (entity*) c1->link_ifc()->get_next_sibling();
		}
		c->set_render_scale(s);
	}
}

void FEEntityManager::SetConglomForceHiRes(entity* c, bool fhr)
{
	if(!c) return;
	if(c->has_link_ifc())
	{
		link_interface* li = c->link_ifc();
		entity* c1 = (entity*) li->get_first_child();
		while(c1)
		{
			SetConglomForceHiRes(c1, fhr);
			c1 = (entity*) c1->link_ifc()->get_next_sibling();
		}
		c->set_force_hi_res(fhr);
	}
}

void FEEntityManager::StashesCleared()
{
	surfer_loaded = false;
	all_loaded = false;

	if(surfer_ent) surfer_ent->kill_anim(ANIM_PRIMARY);

	if(board) board->kill_anim(ANIM_PRIMARY);
	boards_loaded = false;
}

void FEEntityManager::SetSurferPo(entity* s, int s_num)
{
	s->set_rel_po(surfer_select_po);
}

void FEEntityManager::FEDB_ToggleDraw(int option)
{
	switch(option)
	{
	case FEDB_Surfer:	surfer_ent->set_visible(option); break;
	case FEDB_Board:	board->set_visible(option); break;
	case FEDB_Room:		break;
	default: nglPrintf("This debug menu option is not implemented yet.\n"); break;
	}
}

float FEEntityManager::get_floor_offset(void)
{
	// for now at least, this is entirely determined by the root animation
	entity_anim_tree* a = surfer_tree;
	rational_t last_floor_offset = 0.0f;

	for(int i=(MAX_ANIM_SLOTS-1); i>=0 && (a == NULL); --i)
	//  for(int i=0; i<MAX_ANIM_SLOTS && (a == NULL); ++i)
	{
		a = surfer_ent->get_anim_tree( i );
		if(a && (a->is_finished() || a->get_floor_offset() <= 0.0f || !a->is_root(surfer_ent)))
		  a = NULL;
	}

	if ( a!=NULL && a->is_relative_to_start() )
	{
		vector3d relp = ZEROVEC;
		a->get_current_root_relpos( &relp );
		last_floor_offset = a->get_floor_offset();
		last_floor_offset += relp.y;
	}

	return last_floor_offset;
}

void FEEntityManager::AlignSurferWithBoard(void)
{
	po owner_po = my_parent_node->get_abs_po();
	po trans( po_identity_matrix );
	rational_t pos = get_floor_offset();// - last_offset;
	trans.set_position(vector3d(0, pos, 0));
	trans = trans*owner_po;
	surfer_ent->set_rel_po(trans);
}

void FEEntityManager::PerformIK(void)
{
	
	bool IK_enabled = false;
	if ((cur_state == STATE_TRICK) && (cam_stopped_at == CAM_POS_WALL_2_OUT))
	{
		my_ik_object->SetFloorObj(my_board_member);
		if (trick_playing)
		{
			int trick_id = cur_trick;
			int flags = GTrickList[trick_id].IK_flags;

			if (flags & DoIkFlag)
			{
				if (flags & IkBlendFlag)
				{
					if (surfer_tree->was_blended())
						IK_enabled = false;
					else
						IK_enabled = true;
				}
				else
					IK_enabled = true;
			}
			else
				IK_enabled = false;

			if (flags & BoardNodeFlag)
				my_ik_object->SetFloorObj(my_parent_node);
		}
		else
			IK_enabled = true;
	}

	if (IK_enabled)
		my_ik_object->PerformIK();
}

