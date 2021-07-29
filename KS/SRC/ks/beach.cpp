//
// Beach class - keeps track of everything that goes on a level
//

#include "global.h"

#include "beach.h"
#include "colgeom.h"
#include "entity_maker.h"
#include "file_finder.h"
#include "floatobj.h"
#include "text_parser.h"
#include "wds.h"
#include "ini_parser.h"
#include "beachdata.h"
#include "game.h"
#include "ks_camera.h"
#include "physical_interface.h"
#include "kellyslater_controller.h"
#include "FrontEndManager.h"
#include "SFXEngine.h"
#include "careerdata.h"
#include "wipeoutdata.h"
#include "SoundScript.h"
#include "SoundData.h"
#include "colmesh.h"

beach *g_beach_ptr;

beach::beach ()
{
	my_objects = NULL;
	travel_distance = ZEROVEC;
	smashedEntity = NULL;
	
	challenges.icon = NULL;
	challenges.photo = NULL;
}

beach::~beach ()
{
	cleanup ();
}

void beach::cleanup ()
{
	beach_object *tmp;
	
	while (my_objects)
	{
		tmp = my_objects->next;
		delete my_objects;
		my_objects = tmp;
	}
	
	delete challenges.icon;
	challenges.icon = NULL;
	delete challenges.photo;
	challenges.photo = NULL;
}

// This function is called at the beginning of the run
void beach::reset ()
{
	beach_object *tmp;
	
	for (tmp = my_objects; tmp != NULL; tmp = tmp->next)
	{
		if (tmp->spawned)
			tmp->despawn ();
		
		tmp->times_spawned = 0;
	}
	
	for (beach_object *fobj = my_objects; fobj != NULL; fobj = fobj->next)
	{
		fobj->active = ((fobj->timer_type == current_breakmap) || (fobj->timer_type == -1));
	}
	
	travel_distance = ZEROVEC;
	current_player_info.ClearAccomplishments();
	
	if (challenges.icon) challenges.icon->Retry();
	if (challenges.photo) challenges.photo->Retry();

	// If this is career mode and the E3 build, clear the level goal completion status every time.
	if(g_game_ptr->get_game_mode() == GAME_MODE_CAREER && 
	   os_developer_options::inst()->is_flagged(os_developer_options::FLAG_E3_BUILD))
	{
		g_career->ClearAllGoals();
	}
	if (g_game_ptr->get_game_mode() == GAME_MODE_CAREER)
		g_career->StartNewRun();	
}

void beach::add_object (beach_object *obj)
{
	obj->next = my_objects;
	my_objects = obj;
}

void beach::remove_object (beach_object *obj)
{
	if (obj == my_objects)
	{
		my_objects = obj->next;
	}
	else
	{
		for (beach_object *fobj = my_objects; fobj != NULL; fobj = fobj->next)
			if (fobj->next == obj)
			{
				fobj->next = obj->next;
				break;
			}
	}
	
	delete obj;
}

beach_object* beach::get_object (int num) const
{
	int i = 0;
	
	for (beach_object *fobj = my_objects; fobj != NULL; fobj = fobj->next)
	{
		if (i == num)
			return fobj;
		
		i++;
	}
	
	return NULL;
}

void beach::load ()
{
	extern char	g_scene_name[256];
	stringx		fname;
	text_parser	parser;
	stringx beach_file;
	int wave_height_fudge_index = 0;

	WAVE_ClearSchedule();
	WAVE_ClearBreakArray();

	if (g_game_ptr->get_game_mode () == GAME_MODE_CAREER)
		beach_file = CareerDataArray[g_game_ptr->get_level_id()].beach_file;
	else
		beach_file = g_scene_name;
	
	// Verify beach file exists.
//	if (!file_finder_exists (stringx (g_scene_name), stringx (".beach"), &fname))
	if (!file_finder_exists (beach_file, stringx (".beach"), &fname))
	{
		nglPrintf ("Couldn't open \"%s.beach\"", beach_file.c_str());
		return;
	}
	// Open beach file.
	if (!parser.load_file (fname))
		return;
	
	current_breakmap = num_breakmap = 0;
	
	// Read beach file.
	while (parser.get_token (true, true))
	{
		if (!stricmp (parser.token, "spawn"))
		{
			// Water objects are disabled in multiplayer.
			if ((g_game_ptr->get_num_players() == 1) /* && (g_game_ptr->get_num_ai_players() == 0) */)
			{
				water_object *obj = water_object::create_from_file  (parser);
				
				if (obj != NULL)
				{
					// don't create a cameraman if this is a competition level
					if(obj->get_entity()->get_parsed_name() == "cameraman" && g_game_ptr->is_competition_level())
						free(obj);
					else
						add_object(obj);
				}
			}
			else
			{
				while (parser.get_token (false, true)) continue;
			}
		}
		else if (!stricmp (parser.token, "billboard"))
		{
			beach_billboard *obj = beach_billboard::create_from_file  (parser);
			
			if (obj != NULL)
				add_object (obj);
		}
		else if (!stricmp (parser.token, "event"))
		{
			beach_event *event = beach_event::create_from_file  (parser);
			
			if (event != NULL)
				add_object (event);
		}
		else if (!stricmp (parser.token, "stationary_camera"))
		{
			vector3d pt;
			
			parser.get_vector3d (&pt, false, false);
			
			//g_world_ptr->get_stationary_cam_ptr ()->add_point (pt);
		}
		else if (!stricmp (parser.token, "flyby_camera_point"))
		{
			vector3d pt;
			float time;
			
			parser.get_vector3d (&pt, false, false);
			parser.get_float (&time, false, false);
			
			//      g_world_ptr->get_flyby_cam_ptr ()->add_point (pt, time);
		}
		else if (!stricmp (parser.token, "break"))	// only for backwards compatibility, should go in loadbreakmap
		{
			stringx typestr;
			float time;
			
			parser.get_token (false, false);
			typestr = parser.token;
			parser.get_float (&time, false, false);
			
			WAVE_AddBreak("default", typestr, time);
		}
		else if (!stricmp (parser.token, "wave"))
		{
			stringx wavename, breakmapname, wdname;
			float duration, scale;
			char waveclass;
			
			parser.get_token (false, false);
			waveclass = *parser.token;
#ifndef TARGET_GC
			assertmsg(parser.token[1] == '\0', "Bad .beach file format.\n");
#endif
			parser.get_token (false, false);
			wavename = parser.token;
			parser.get_token (false, false);
			breakmapname = parser.token;
			parser.get_token (false, false);
			wdname = parser.token;
			parser.get_float (&scale, false, false);

			float fudge_scale;
			parser.get_float(&fudge_scale, false, false);
			WAVE_AddHeightFudge(wave_height_fudge_index, fudge_scale);
			wave_height_fudge_index++;
			parser.get_float (&duration, false, false);
			
			WAVE_AddToSchedule(waveclass, wavename, breakmapname, wdname, scale, duration);
			loadbreakmap(breakmapname);
		}
		else if (!stricmp (parser.token, "aisurfer"))
		{
			stringx surfername;
			int surfer = -1;
			bool pers = false;
			parser.get_token(false,false);
			surfername = parser.token;
			if (!strnicmp(surfername.c_str(), "PERSON", strlen("PERSON")))
			{ 
				pers = true;
				int str_length = strlen(surfername.c_str());
				const char *abbr = surfername.c_str() + (str_length - 2);
				for (int i=0; i < SURFER_LAST; i++)
				{
					if (!stricmp(abbr, SurferDataArray[i].abbr))
					{
						surfer = i;
					}
					
				}
				
			}
			else
			{
				for (int i=0; i < SURFER_LAST; i++)
				{
					if (!stricmp(surfername.c_str(), SurferDataArray[i].  name))
					{
						surfer = i;
					}
				}
			}
			
			if (surfer != -1 && g_game_ptr->get_num_players() == 1 && g_game_ptr->get_game_mode() == GAME_MODE_CAREER)
			{
				g_game_ptr->SetSurferIdx(1, surfer);
				g_game_ptr->SetUsingPersonalitySuit(1, pers);
				g_game_ptr->set_num_ai_players(1);
				g_world_ptr->load_ai_hero(AI_JOYSTICK, surfer,pers);
			}
			
		}
		else
		{
			nglPrintf ("unexpected token: %s\n", parser.token);
			assert (false);
		}
	}
  
	// Load beach challenges.
	// Note that these must be initialized after the beach has been loaded.
	if (g_game_ptr->get_game_mode() == GAME_MODE_CAREER)
	{
		int current_level = g_game_ptr->get_level_id();
		for (int i = 0; i < MAX_GOALS_PER_LEVEL; i++)
		{
			// 3D Icon challenge.
			/*
			if (!challenges.icon && CareerDataArray[current_level].goal[i] == GOAL_ICON_3D)
			{
				challenges.icon = NEW IconChallenge();
				challenges.icon->Init(g_world_ptr->get_ks_controller(g_game_ptr->get_active_player()));
			}
			*/
			
			// Photo challenge.
			if (!challenges.photo)
			{
				if (CareerDataArray[current_level].goal[i] == GOAL_PHOTO_1 || 
				    CareerDataArray[current_level].goal[i] == GOAL_PHOTO_2 || 
					CareerDataArray[current_level].goal[i] == GOAL_PHOTO_3)
				{
					challenges.photo = NEW PhotoChallenge();
					challenges.photo->Init(g_world_ptr->get_ks_controller(g_game_ptr->get_active_player()), 
					                       CareerDataArray[current_level].goal[i],
					                       CareerDataArray[current_level].goal_param[i], my_objects,
					                       game::MAX_SNAPSHOTS, game::SNAPSHOT_WIDTH, game::SNAPSHOT_HEIGHT);
				}
				else if((CareerDataArray[current_level].goal[i] == GOAL_COMPETITION_1 || 
				         CareerDataArray[current_level].goal[i] == GOAL_COMPETITION_2 || 
						 CareerDataArray[current_level].goal[i] == GOAL_COMPETITION_3) &&
				        !SurferDataArray[g_game_ptr->GetSurferIdx(g_game_ptr->get_active_player())].competitor)
				{
					int photo_goal_idx = GOAL_PHOTO_3 - (CareerDataArray[current_level].goal[i] - GOAL_COMPETITION_1);

					challenges.photo = NEW PhotoChallenge();
					challenges.photo->Init(g_world_ptr->get_ks_controller(g_game_ptr->get_active_player()), photo_goal_idx,
					                       CareerDataArray[current_level].goal_param_2[i], my_objects, game::MAX_SNAPSHOTS, 
					                       game::SNAPSHOT_WIDTH, game::SNAPSHOT_HEIGHT);
				}
			}
		}
	}
}

/*	Load event timeline for a single wave. (dc 02/11/02)
*/
void beach::loadbreakmap (const stringx &breakmapname)
{
	stringx fname;
	
	if (!file_finder_exists (breakmapname, stringx (".break"), &fname))
	{
		nglPrintf ("Couldn't open \"%s.break\"\n", breakmapname.c_str());
		return;
	}
	
	if (WAVE_CheckBreakType(breakmapname) < (u_int) num_breakmap) 
	{
		return;	// already have this break map loaded (dc 02/26/02)
	}
	
	text_parser parser;
	
	if (!parser.load_file (fname))
		return;
	
	while (parser.get_token (true, true))
	{
		if (!stricmp (parser.token, "spawn"))
		{
			// Water objects are disabled in multiplayer.
			if ((g_game_ptr->get_num_players() == 1))
			{
				water_object *obj = water_object::create_from_file  (parser, num_breakmap);
				
				if (obj != NULL)
				{
					// don't create a cameraman if this is a competition level
					if(obj->get_entity()->get_parsed_name() == "cameraman" && g_game_ptr->is_competition_level())
						free(obj);
					else
						add_object(obj);
				}
			}
			else
			{
				while (parser.get_token (false, true)) continue;
			}
		}
		else if (!stricmp (parser.token, "event"))
		{
			beach_event *event = beach_event::create_from_file  (parser, num_breakmap);
			
			if (event != NULL)
				add_object (event);
		}
		else if (!stricmp (parser.token, "break"))
		{
			stringx typestr;
			float time, prob;
			
			parser.get_token (false, false);
			typestr = parser.token;
			parser.get_float (&time, false, false);
			if (!parser.get_float (&prob, false, true)) prob = 1;
			
			WAVE_AddBreak(breakmapname, typestr, time, prob);
		}
		else
		{
			nglPrintf ("Unexpected token: %s\n", parser.token);
			assert (false);
		}
	}
	
	++num_breakmap;
}

// This is called once per wave (potentially multiple times per run)
void beach::OnNewWave ()
{
	SoundScriptManager::inst()->clearEvents();

	if (g_game_ptr->get_game_mode() == GAME_MODE_CAREER  || 
			g_game_ptr->get_game_mode() == GAME_MODE_FREESURF_HIGHSCORE)
	{
		for (int i=0; i < 9; i++)
		{
			SoundScriptManager::inst()->addEvent(SS_TIMER, CareerDataArray[g_game_ptr->get_level_id()].level_duration - (i+2));
		}

		if (g_game_ptr->is_competition_level() && g_game_ptr->get_game_mode() == GAME_MODE_CAREER)
		{
			SoundScriptManager::inst()->addEvent(SS_END_COMPETITION, CareerDataArray[g_game_ptr->get_level_id()].level_duration-1);
		}
		else
		{
			SoundScriptManager::inst()->addEvent(SS_TIMER, CareerDataArray[g_game_ptr->get_level_id()].level_duration -1);
		}
	}

	current_breakmap = WAVE_GetScheduleIndex();
	
	for (beach_object *obj = my_objects; obj != NULL; obj = obj->next)
	{
		if ((!obj->never_despawn) && (obj->spawned))
			obj->despawn();

		obj->active = ((obj->timer_type == current_breakmap) || (obj->timer_type == -1));
	}
}

void beach::update (float dt)
{
	vector3d current;
	
	WAVE_GlobalCurrent (&current);
	
	travel_distance += current * TIMER_GetFrameSec ();
	float level_time = TIMER_GetLevelSec();
	float wave_time = WAVE_GetScheduleSec();
	
	if (g_game_ptr->get_game_mode() == GAME_MODE_CAREER)
		check_challenges(g_world_ptr->get_ks_controller(g_game_ptr->get_active_player()));    // Active Player s/b 0 in 1-player game
	
	// Update every beach object.
	for (beach_object *fobj = my_objects; fobj != NULL; fobj = fobj->next)
	{
		if (!fobj->is_active ())
			continue;
		
		bool timetospawn = false;
		
		if (fobj->timer_type == -1) // timed relative to level start
		{
			timetospawn = (fobj->spawn_time < level_time) && (fobj->times_spawned == 0);
		}
		else // timed relative to this wave's start
		{
			assert(fobj->timer_type == current_breakmap);
			timetospawn = (fobj->spawn_time < wave_time);
		}
		
		if (fobj->spawned)
		{
			if (!fobj->update (dt))
			{
				fobj->active = false;
				fobj->despawn ();
			}
		}
		else
		{
			if (timetospawn)
				fobj->spawn ();
		}
	}
	
	// Update complex challenges.
	if (challenges.icon) challenges.icon->Update(dt);
	if (challenges.photo) challenges.photo->Update(dt);
}

beach_object *beach::find_object (entity *ent) const
{
	for (beach_object *fobj = my_objects; fobj != NULL; fobj = fobj->next)
		if (fobj->get_entity () == ent)
			return fobj;
		
		return NULL;
}

// Checks the "my_board_model" entity pointer of ksctrl for a collision against entities
// Note: collision will not be checked against new optional ignoreObj parameter.
entity *beach::check_entity_collision(kellyslater_controller *ksctrl, beach_object * ignoreObj)
{
	entity *ent;
	ent = ((conglomerate *)ksctrl->GetBoardModel())->get_member("BOARD");
	
	vector3d c1 (ent->get_abs_position ());
	float r1, r2;
	nglMesh *mesh;
	
	r1 = ent->get_radius ();
	
	if (r1 < 0.01f)
	{
		if (ent->get_colgeom ())
			r1 = ent->get_colgeom ()->get_radius ();
		
		if (r1 < 0.01f)
		{
			mesh = ent->get_mesh ();
			
			if (mesh)
			{
				c1.x += mesh->SphereCenter[0];
				c1.y += mesh->SphereCenter[1];
				c1.z += mesh->SphereCenter[2];
				r1 = mesh->SphereRadius;
			}
		}
	}
	
	for (beach_object *fobj = my_objects; fobj != NULL; fobj = fobj->next)
	{
		if (!fobj->is_active ())
			continue;
		
		if (!fobj->is_physical() || fobj == ignoreObj)
			continue;
		
		entity *e2 = ((water_object*)fobj)->get_entity ();
		vector3d c2 (e2->get_abs_position ());
		
		if (!e2->is_visible ())
			continue;
		
		r2 = e2->get_radius ();
		
		if (r2 < 0.01f)
		{
			if (e2->get_colgeom ())
				r2 = e2->get_colgeom ()->get_radius ();
			
			if (r2 < 0.01f)
			{
				mesh = e2->get_mesh ();
				
				if (mesh)
				{
					c2.x += mesh->SphereCenter[0];
					c2.y += mesh->SphereCenter[1];
					c2.z += mesh->SphereCenter[2];
					r2 = mesh->SphereRadius;
				}
			}
		}
		
		// sphere-sphere collision
		if ((c1 - c2).length2 () <= (r1 + r2) * (r1+ r2))
		{
			if (!e2->get_colgeom ())
				continue;
			
			// entity collision
			if (g_world_ptr->entity_entity_collision_check (ent, e2, 0))
			{
				vector3d v (c2 - c1);
				v.normalize ();
				
				((water_object*)fobj)->collide (ent, -v);
				
				// "Smashable" entities do not cause the surfer to wipeout.
				if (!((water_object*)fobj)->smashable)
				{
					float fd = -dot (ent->get_abs_po ().get_x_facing (), v);
					
					if (fd > 0.7071067f)
					{
						//  hit front
						ksctrl->get_board_controller().DoWipeOut(WIP_LOW_AIR_FOR);
					}
					else if (fd < -0.7071067f)
					{
						//  hit back
						ksctrl->get_board_controller().DoWipeOut(WIP_LOW_AIR_BACK);
					}
					else
					{
						if (dot (ent->get_abs_po ().get_z_facing (), v) > 0)
							ksctrl->get_board_controller().DoWipeOut(WIP_LOW_AIR_RIGHT);  //  hit right
						else
							ksctrl->get_board_controller().DoWipeOut(WIP_LOW_AIR_LEFT);  //  hit left
					}
					
					/*
					vector3d lm = ksctrl->get_board_controller ().rb->linMom;
					lm -= v * dot (v, lm);
					ksctrl->get_board_controller ().rb->linMom = lm;
					*/
					ksctrl->get_board_controller ().rb->linMom *= -1;
					
					return e2;
				}
				else
				{
					vector3d v (c2 - c1);
					v.normalize ();
					
					((water_object*)fobj)->collide (ent, -v);
					
					smashedEntity = e2;
					return NULL;
				}
			}
		}
	}
	
	smashedEntity = NULL;
	return NULL;
}


entity *beach::surfer_over_entity(vector3d c1, float r1) const
{
	float r2;
	nglMesh *mesh;
	for (beach_object *fobj = my_objects; fobj != NULL; fobj = fobj->next)
	{
		if (!fobj->is_active ())
			continue;
		
		if (!fobj->is_physical())
			continue;
		
		entity *e2 = ((water_object*)fobj)->get_entity ();
		vector3d c2 (e2->get_abs_position ());
		
		if (!e2->is_visible ())
			continue;
		
		r2 = e2->get_radius ();
		
		if (r2 < 0.01f)
		{
			if (e2->get_colgeom ())
				r2 = e2->get_colgeom ()->get_radius ();
			
			if (r2 < 0.01f)
			{
				mesh = e2->get_mesh ();
				
				if (mesh)
				{
					c2.x += mesh->SphereCenter[0];
					c2.y += mesh->SphereCenter[1];
					c2.z += mesh->SphereCenter[2];
					r2 = mesh->SphereRadius;
				}
			}
		}

		vector3d pc1 (c1), pc2 (c2);
		pc1.y = 0;
		pc2.y = 0;

		// sphere-sphere collision
		if ((pc1 - pc2).length2 () <= (r1 + r2) * (r1+ r2))
		{
			// pier is too big
			if (e2->get_parsed_name().find ("sebastian_pier") != stringx::npos)
			{
				vector3d box[2];
				cg_mesh *colgeom;

				colgeom = (cg_mesh *) (e2->get_colgeom());
				colgeom->get_min_extent (&box[0]);
				box[0] = e2->get_abs_po ().slow_xform (box[0]);
				colgeom->get_max_extent (&box[1]);
				box[1] = e2->get_abs_po ().slow_xform (box[1]);

				if ((c1.y < box[1].y) || (c1.x < box[0].x) || (c1.x > box[1].x))
					continue;
			}

			return e2;
		}
	}
	
	return NULL;
}

float g_past_lip_dist = 3.0f;
bool beach::get_nearest_object_pos(vector3d surfer_pos, vector3d &object_pos, vector3d lip_norm)
{
	object_pos = ZEROVEC;
	float sq_mag = 0.0f;
	bool object_found = false;
	bool first_time = true;
	for (beach_object *fobj = my_objects; fobj != NULL; fobj = fobj->next)
	{
        if (!fobj->is_active ())
			continue;
		
		if (!fobj->is_physical())
			continue;
		
		entity *e2 = ((water_object*)fobj)->get_entity ();
		if (!e2->is_visible ())
			continue;
		
		if ((e2->get_parsed_name() ==  "dolphin") || (e2->get_parsed_name() == "greatwhite")
				|| (e2->get_parsed_name() == "cameraman") || (e2->get_parsed_name() == "misc_surfer1")
				|| (e2->get_parsed_name() == "misc_surfer2"))
			continue;
		
		bool in_front_of_wave = false;
		vector3d obj_pos = e2->get_abs_position();
		vector3d start_point = *WAVE_GetMarker(WAVE_MarkerLipMark11);
		vector3d object_vec = obj_pos - start_point;
		if (dot(object_vec, lip_norm) < g_past_lip_dist)
			in_front_of_wave = true;
			
		vector3d new_vec = e2->get_abs_position() - surfer_pos;
		float smag = dot(new_vec, new_vec);
		if (in_front_of_wave)
		{
			if (first_time || (smag < sq_mag))
			{
				object_found = true;
				sq_mag = smag;
				object_pos = new_vec;
				first_time = false;
			}
		}
	}
	
	return object_found;
}

/////////////////////////////////////////////////////////////////////////////
// This function encapsulates the functionality that all the environmental
// challenges need for checking for completion.  If a new item has been
// jumpped or sprayed or whatever then a message is displayed saying 
// something like "1 of 3 tubers".  If all the things have been jumped then
// a completion message is displayed.
/////////////////////////////////////////////////////////////////////////////

bool beach::CheckEnvGoal(kellyslater_controller *ksctrl, 
                         stringx entity_name, 
                         int num_to_jump, 
                         int gap_to_add, 
                         int gt_partially_done,
                         int gt_done,
                         int goal_num)
{
	bool            gotOne = false;
	char            display_text[100]; // for printing out messages to the IGO
	ScoringManager &scoreManager = ksctrl->get_my_scoreManager();
	
	if(!ksctrl->get_board_controller().InAir())
	{
		// Go through the whole list of jumped entities and if we find one 
		// that is the type we're looking for but isn't in the 
		// accomplishments list, add it.
		for (int i = 0; i < ksctrl->GetNumJumpedEntities(); i++)
		{
			entity *	jumped = ksctrl->GetJumpedEntity(i);
			
			if ((jumped != NULL) && (jumped->get_parsed_name().find (entity_name.c_str()) != stringx::npos ))
			{
				if (!current_player_info.HasAccomplishment(jumped))
				{
					current_player_info.AddAccomplishment(jumped);
					scoreManager.AddGap(gap_to_add);
					
					if (current_player_info.GetNumAccomplishments()  < num_to_jump)
					{
						// Load display_text with something like "1 of 3 tubers", except 
						// appropriate to the current state of things by using the global
						// text array (this also allows for localization)

						if (num_to_jump > 1)
							sprintf(display_text, ksGlobalTextArray[gt_partially_done].c_str(), current_player_info.GetNumAccomplishments(), num_to_jump);
						else
							strcpy(display_text, ksGlobalTextArray[gt_partially_done].c_str());

						frontendmanager.IGO->Print(display_text);
					}
					else if (current_player_info.GetNumAccomplishments() == num_to_jump)
						gotOne = true;
				}
			}
		}
	}
	return gotOne;
}

#include "trail.h"

bool beach::check_spray_challenge (kellyslater_controller *ksctrl, int object_type, int object_count, int gt_partially_done, int gt_done)
{
	entity *ent = ((conglomerate *)ksctrl->GetBoardModel())->get_member("BOARD");
	vector3d c1 (ent->get_abs_position ());
	float r1, r2;
	nglMesh *mesh;

	r1 = ent->get_radius ();

	if (r1 < 0.01f)
	{
		if (ent->get_colgeom ())
			r1 = ent->get_colgeom ()->get_radius ();
		
		if (r1 < 0.01f)
		{
			mesh = ent->get_mesh ();
			
			if (mesh)
			{
				c1.x += mesh->SphereCenter[0];
				c1.y += mesh->SphereCenter[1];
				c1.z += mesh->SphereCenter[2];
				r1 = mesh->SphereRadius;
			}
		}
	}

	for (beach_object *fobj = my_objects; fobj != NULL; fobj = fobj->next)
	{
		if (!fobj->is_active ())
			continue;
		
		if (!fobj->is_physical())
			continue;
		
		entity *ent = ((water_object*)fobj)->get_entity ();
		
		if (((water_object*)fobj)->get_type () != object_type)
			continue;
		
		if (!ent->is_visible ())
			continue;

		vector3d c2 (ent->get_abs_position ());
		r2 = ent->get_radius ();
		
		if (r2 < 0.01f)
		{
			if (ent->get_colgeom ())
				r2 = ent->get_colgeom ()->get_radius ();
			
			if (r2 < 0.01f)
			{
				mesh = ent->get_mesh ();
				
				if (mesh)
				{
					c2.x += mesh->SphereCenter[0];
					c2.y += mesh->SphereCenter[1];
					c2.z += mesh->SphereCenter[2];
					r2 = mesh->SphereRadius;
				}
			}
		}

		// check if we're close to the object
		if ((c1 - c2).length2 () > 9)
			continue;

		if (ksctrl->my_trail->spray_object (ent))
		{
			// check if already sprayed
			if (!current_player_info.HasAccomplishment (ent))
			{
				char buf[32];
				
				find_object (ent)->sprayed (ksctrl->get_board_controller().my_board_model);
				sfx.sprayed(ent);
				current_player_info.AddAccomplishment(ent);
				
				if (current_player_info.GetNumAccomplishments() < object_count)
				{
					sprintf (buf, ksGlobalTextArray[gt_partially_done].c_str(), current_player_info.GetNumAccomplishments(), object_count);
					frontendmanager.IGO->Print(buf);

				}
				else if (current_player_info.GetNumAccomplishments() == object_count) //  Now check to see if we're done.
					return true;
			}
		}
	}
	
	return false;
}

bool beach::check_smash_challenge (kellyslater_controller *ksctrl, int object_type, int object_count, int gt_partially_done, int gt_done)
{
	char display_text[100]; // for printing out messages to the IGO
	
	if ((smashedEntity != NULL) && (((water_object*)find_object (smashedEntity))->get_type () == object_type))
	{
		if (!current_player_info.HasAccomplishment(smashedEntity))
		{
			current_player_info.AddAccomplishment(smashedEntity);
			
			if (current_player_info.GetNumAccomplishments() < object_count)
			{
				// Load display_text with something like "1 of 3 tubers", except 
				// appropriate to the current state of things by using the global
				// text array (this also allows for localization)
				sprintf(display_text, ksGlobalTextArray[gt_partially_done].c_str(), current_player_info.GetNumAccomplishments(), object_count);
				frontendmanager.IGO->Print(display_text);
			}
			else if (current_player_info.GetNumAccomplishments() == object_count)
				return true;
		}
	}
	
	return false;
}

/* David Johnston rework comments:
I've updated this to no longer be beach specific.  It now follows the level progression
defined in careerdata.xls.  It deals with each goal on its own terms, regardless of the
beach that's currently being used.
*/
void beach::check_challenges(kellyslater_controller *ksctrl)
{
	ScoringManager &			scoreManager = ksctrl->get_my_scoreManager();
	int							levelIdx = g_game_ptr->get_level_id();
	//int							surferIdx = g_game_ptr->GetSurferIdx(0);
	int							score = scoreManager.GetScore();
	int							faceScore;
	int							airScore;
	int							tubeScore;
	int							num540spins = scoreManager.Get540Spins();
	int							num360spins = scoreManager.Get360Spins();
	float						current_special_meter_time = g_world_ptr->get_ks_controller(g_game_ptr->get_active_player())->get_special_meter()->GetCurrentSpecialTime();
	int							a;
	
	scoreManager.GetPartialScores(faceScore, airScore, tubeScore);
	
	if (g_game_ptr->is_paused())
		return;
	
	for(a = 0; a < MAX_GOALS_PER_LEVEL; a++)
	{
		int goal = CareerDataArray[levelIdx].goal[a];
		int param = CareerDataArray[levelIdx].goal_param[a];
		int param2 = CareerDataArray[levelIdx].goal_param_2[a];
		
		// Some surfers don't do competitions.  Make this a photoshoot for them
		if(goal >= GOAL_COMPETITION_1 &&
			goal <= GOAL_COMPETITION_3 &&
			!SurferDataArray[g_game_ptr->GetSurferIdx(0)].competitor)
		{
			// If you have to get 1st place in the competition then make it a photo 3
			// If you have to get 2nd place in the competition then make it a photo 2
			// If you have to get 3rd place in the competition then make it a photo 1
			goal = GOAL_PHOTO_3 - (goal - GOAL_COMPETITION_1);
			// and switch over the parameters, because the points required for the photo shoot are in param2
			param2 = param;
		}
		
		
		bool this_goal_done;
		this_goal_done = false;
		
		switch(goal)
		{
		case GOAL_ICON_3D :
			if (challenges.icon && challenges.icon->IsCompleted())
				this_goal_done = true;
			break;
			
		case GOAL_COMPETITION_1:
		case GOAL_COMPETITION_2:
		case GOAL_COMPETITION_3:
			//	This was moved to judge.cpp because competitions are completed at the end of the level, not during play.  This causes problems.
			break;
			
		case GOAL_SPECIAL_METER:
			if (current_special_meter_time >= param && !g_career->levels[levelIdx].IsGoalDone(a))
				this_goal_done = true;
			break;

		case GOAL_PHOTO_1:
			if (challenges.photo && challenges.photo->CheckForCompletion() && score >= param2)
				this_goal_done = true;
			break;
			
		case GOAL_PHOTO_2:
			if (challenges.photo && challenges.photo->CheckForCompletion() && score >= param2)
				this_goal_done = true;
			break;
			
		case GOAL_PHOTO_3:
			if (challenges.photo && challenges.photo->CheckForCompletion() && score >= param2)
				this_goal_done = true;
			break;
			
		case GOAL_LOCAL:
		case GOAL_LOCAL_2:
			if (score >= param)
				this_goal_done = true;
			break;

		case GOAL_ICON_TETRIS :
			if(frontendmanager.IGO->GetIconManager() && frontendmanager.IGO->GetIconManager()->FirstTimeGettingThisMany(param))
			{
				char display_text[100]; // for printing out messages to the IGO
				sprintf(display_text, ksGlobalTextArray[GT_GOAL_COMPLETE_ICONS].c_str(), param);
				frontendmanager.IGO->Print(display_text,SS_COMPLETED_GOAL);
			}
			if(frontendmanager.IGO->GetIconManager() && frontendmanager.IGO->GetIconManager()->IconsCleared() >= param &&
				score >= param2)
				this_goal_done = true;
			break;
			
		case GOAL_SKILL_FACE:
			if(faceScore >= param)
				this_goal_done = true;
			break;

		case GOAL_SKILL_AIR:
			if(airScore >= param)
				this_goal_done = true;
			break;

		case GOAL_SKILL_TUBE:
			if(tubeScore >= param)
				this_goal_done = true;
			break;

		case GOAL_SKILL_360_SPIN:
			if(num360spins >= param)
				this_goal_done = true;
			break;

		case GOAL_SKILL_540_SPIN:
			if(num540spins >= param)
				this_goal_done = true;
			break;

		case GOAL_SCORE:
			if(score >= param)
				this_goal_done = true;
			break;

		case GOAL_ENV_SPRAY_WINDSURFERS:
			this_goal_done = check_spray_challenge (ksctrl, surfing_object::WINDSURFER, param, GT_GOAL_ENV_SPRAYED_N_WINDSURFERS, GT_GOAL_ENV_SPRAY_WINDSURFERS);
			break;
			
		case GOAL_ENV_SMASH_ICE:
			this_goal_done = check_smash_challenge (ksctrl, surfing_object::ICEPATCH, param, GT_GOAL_ENV_SMASHED_N_ICE, GT_GOAL_ENV_SMASH_ICE);
			break;
			
		case GOAL_ENV_SPRAY_JETSKIERS:
			this_goal_done = check_spray_challenge (ksctrl, surfing_object::JETSKIER, param, GT_GOAL_ENV_SPRAYED_N_JETSKIERS, GT_GOAL_ENV_SPRAY_JETSKIERS);
			break;

		case GOAL_ENV_SMASH_SPONGERS:
			this_goal_done = check_smash_challenge (ksctrl, surfing_object::BOOGIEBOARDER, param, GT_GOAL_ENV_SMASHED_N_SPONGERS, GT_GOAL_ENV_SMASH_SPONGERS);
			break;
			
		case GOAL_ENV_JUMP_TURTLES:
			this_goal_done = CheckEnvGoal(ksctrl, "turtle", param, GAP_TURTLE_JUMP,
				GT_GOAL_ENV_JUMPED_N_TURTLES, 
				GT_GOAL_ENV_JUMP_TURTLES, a);
			break;
			
		case GOAL_ENV_JUMP_KAYAKERS:
			this_goal_done = CheckEnvGoal(ksctrl, "kayaker", param, GAP_KAYAK_JUMP,
				GT_GOAL_ENV_JUMPED_N_KAYAKERS, 
				GT_GOAL_ENV_JUMP_KAYAKERS, a);
			break;
			
		case GOAL_ENV_SPRAY_TUBERS:
			this_goal_done = check_spray_challenge (ksctrl, surfing_object::FATBASTARD, param, GT_GOAL_ENV_SPRAYED_N_TUBERS, GT_GOAL_ENV_SPRAY_TUBERS);
			break;

		case GOAL_ENV_SPRAY_RAFTERS:
			this_goal_done = check_spray_challenge (ksctrl, surfing_object::DINGY, param, GT_GOAL_ENV_SPRAYED_N_RAFTERS, GT_GOAL_ENV_SPRAY_RAFTERS);
			break;

		case GOAL_ENV_JUMP_PIER:
			this_goal_done = CheckEnvGoal(ksctrl, "sebastian_pier", param, GAP_PIER_JUMP,
				GT_GOAL_ENV_JUMPED_N_PIERS, 
				GT_GOAL_ENV_JUMP_PIER, a);
			break;

		case GOAL_SKILL_FACE_SCORE:
			if(score >= param && faceScore >= param2)
				this_goal_done = true;
			break;

		case GOAL_SKILL_AIR_SCORE:
			if(score >= param && airScore >= param2)
				this_goal_done = true;
			break;

		case GOAL_SKILL_TUBE_SCORE:
			if(score >= param && tubeScore >= param2)
				this_goal_done = true;
			break;

		case GOAL_SKILL_360_SPIN_SCORE:
			if(score >= param && num360spins >= param2)
				this_goal_done = true;
			break;

		case GOAL_SKILL_540_SPIN_SCORE:
			if(score >= param && num540spins >= param2)
				this_goal_done = true;
			break;

		case GOAL_LONGEST_RIDE:
			if (ksctrl->GetRideTime() >= (float)param &&
				score >= (float)param2 && !ksctrl->get_board_controller().InAir() &&
				!ksctrl->IsDoingTrick() && !ksctrl->get_my_scoreManager().GetChain().IsInteresting())
			{
				this_goal_done = true;
			}
			break;

		case GOAL_LEARN_NEW_TRICK:
			this_goal_done = (frontendmanager.IGO->GetLearnNewTrickManager()->CheckTrickPerformed() && score >= param2);
//			if(frontendmanager.IGO->GetLearnNewTrickManager()->IconsCleared() >= 3)
//				this_goal_done = true;
			break;
		case GOAL_TUTORIAL_1:
		case GOAL_TUTORIAL_2:
		case GOAL_TUTORIAL_3:
			if(frontendmanager.IGO->GetTutorialManager()->AlmostFinished())
				this_goal_done = true;
			break;
		}
			
		if (this_goal_done)
		{
			complete_goal(a);
		}
	}
}

void beach::complete_goal(const int a)
{
	int	levelIdx = g_game_ptr->get_level_id();

	assert(a >= 0 && a < MAX_GOALS_PER_LEVEL);

	if (CareerDataArray[levelIdx].goal[a] == GOAL_NOTHING)
		return;
	
	if (!g_career->levels[levelIdx].IsGoalDone(a))
	{
		g_career->levels[levelIdx].SetGoalDone(a);              // set the goal done
		g_career->OnGoalDone(levelIdx, a);
		stringx completion_message;
		if(g_career->GetGoalText(levelIdx, a, completion_message)) // get the goal text
			frontendmanager.IGO->Print(completion_message, SS_COMPLETED_GOAL);         // print it out
		if(g_career->GetRewardText(levelIdx, a, completion_message)) // get the reward text
		{
			frontendmanager.IGO->Print(completion_message, SS_UNLOCK_ITEM);         // print it out
			//SoundScriptManager::inst()->playEvent(SS_UNLOCK_ITEM);  // Play the reward sound
		}
		if(a == 0)
		{	
			// Check to see if any new levels or beaches were unlocked
			if(g_career->WasNewBeachUnlocked())
				frontendmanager.IGO->Print(ksGlobalTextArray[GT_NEW_BEACH_UNLOCKED],SS_UNLOCK_ITEM); 
			else if(g_career->WasNewLevelUnlocked())
				frontendmanager.IGO->Print(ksGlobalTextArray[GT_NEW_LEVEL_UNLOCKED],SS_UNLOCK_ITEM); 
		}
		
		// If we just completed an environmental challenge and all the env challenges are now 
		// done then this must have been the last one.  Therefore, unlock the bails movie.
		/*				if(goal >= GOAL_ENV_SPRAY_WINDSURFERS && 
		goal <= GOAL_ENV_JUMP_PIER &&
		g_career->AllEnvChallengesDone())*/
		if(g_career->GoalThatUnlockedBailsMovie() == a) // if this goal unlocked the bails movie
		{
			frontendmanager.IGO->Print(ksGlobalTextArray[GT_BAILS_MOVIE_UNLOCKED],SS_UNLOCK_ITEM);
		}
		
		// if the espn movie was just unlocked
		if(g_career->GoalThatUnlockedEspnMovie() == a)
		{
			frontendmanager.IGO->Print(ksGlobalTextArray[GT_ESPN_MOVIE_UNLOCKED],SS_UNLOCK_ITEM);
		}
		
		bool done_all = true;
		int num_goals = 0;
		// check if all goals have been completed
		for (int j=0; j<MAX_GOALS_PER_LEVEL; j++)
		{
			if (CareerDataArray[levelIdx].goal[j] != GOAL_NOTHING)
			{
				num_goals++;
				if(!g_career->levels[levelIdx].IsGoalDone(j))
					done_all = false;
			}
		}
				
		if (done_all)
		{
			if (num_goals == 1)
				frontendmanager.IGO->Print(ksGlobalTextArray[GT_GOAL_ONLY_COMPLETE],SS_UNLOCK_ITEM);
			else
			{
				char buff[100];
				sprintf(buff, ksGlobalTextArray[GT_GOAL_ALL_COMPLETE].c_str(), num_goals);
				frontendmanager.IGO->Print(buff,SS_UNLOCK_ITEM);
			}
		}
	}
	else // remember that a goal was reached but it was already done....
		g_career->OnGoalReDone(levelIdx, a);			
}

//	IsDolphinActive()
// Returns true if the dolphin is shown to be swimming around.
bool beach::IsDolphinActive(void)
{
	for (beach_object *fobj = my_objects; fobj != NULL; fobj = fobj->next)
	{
        if (!fobj->is_active ())
			continue;
		
		if (!fobj->is_physical ())
			continue;
		
		if ((((surfing_object*)fobj)->get_type () == surfing_object::DOLPHIN) &&
			(((surfing_object*)fobj)->get_state () != DOLPHIN_STATE_HIDDEN))
			return true;
	}
	
	return false;
}

#ifdef DEBUG
entity * beach::GetDolphin(void)
{
	for (beach_object *fobj = my_objects; fobj != NULL; fobj = fobj->next)
	{
        if (!fobj->is_active ())
			continue;
		
		if (!fobj->is_physical ())
			continue;
		
		if ((((surfing_object*)fobj)->ai_func) == &surfing_object::dolphin_ai)
			return fobj->get_entity();
	}
	
	return NULL;
}
#endif
