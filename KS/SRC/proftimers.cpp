// proftimers.cpp
// Copyright (c) 2000 Treyarch Invention LLC.  ALL RIGHTS RESERVED.

#include "global.h"

#include "profiler.h"

  profiler_timer proftimer_frame_total("Frame total",NULL,1.0f/30.0f);
	profiler_timer proftimer_frame_flip    ("Flip",       &proftimer_frame_total);
  profiler_timer proftimer_cpu_net("CPU net",NULL,1.0f/30.0f);
  profiler_timer proftimer_gpu_net("GPU net",NULL,1.0f/30.0f);
#ifdef TARGET_PS2
  profiler_timer proftimer_gfx_pipe      ("Graphics Pipe", &proftimer_cpu_net);
#endif
    profiler_timer proftimer_poll_devices  ("Poll devices",  &proftimer_cpu_net);
    profiler_timer proftimer_streams       ("Streams",       &proftimer_cpu_net);
    profiler_timer proftimer_vector_math   ("Vector math",   &proftimer_cpu_net);
    profiler_timer proftimer_random        ("Random",        &proftimer_cpu_net);

// ---- Advance
    profiler_timer proftimer_advance       ("Advance",       &proftimer_cpu_net,0.0f,true, true);
      profiler_timer proftimer_adv_kelly_slater       ("KS Tick",       &proftimer_advance,0.0f,true, true);
        profiler_timer proftimer_adv_wave           ("Wave Tick",       &proftimer_adv_kelly_slater,0.0f,true, true);
          profiler_timer proftimer_wave_compute_grid   ("Compute Grid",&proftimer_adv_wave);
        profiler_timer proftimer_adv_particles      ("Particle Update", &proftimer_adv_kelly_slater,0.0f,true, true);
          profiler_timer proftimer_adv_crash      ("Crash", &proftimer_adv_particles,0.0f,true, true);
          profiler_timer proftimer_adv_trail      ("Trail", &proftimer_adv_particles,0.0f,true, true);
          profiler_timer proftimer_adv_misc      ("Misc", &proftimer_adv_particles,0.0f,true, true);
#ifdef TARGET_PS2
      profiler_timer proftimer_adv_sound_gas ("GAS",        &proftimer_advance,0.0f,true, false);
        profiler_timer proftimer_adv_sound_add_inst ("add_inst", &proftimer_adv_sound_gas);
        profiler_timer proftimer_adv_sound_play     ("play", &proftimer_adv_sound_gas);
        profiler_timer proftimer_adv_sound_stop     ("stop", &proftimer_adv_sound_gas);
        profiler_timer proftimer_adv_sound_pause    ("pause", &proftimer_adv_sound_gas);
        profiler_timer proftimer_adv_sound_unpause    ("unpause", &proftimer_adv_sound_gas);
        profiler_timer proftimer_adv_sound_cmd      ("cmd", &proftimer_adv_sound_gas);
        profiler_timer proftimer_adv_sound_is_ready ("is_ready", &proftimer_adv_sound_gas);
        profiler_timer proftimer_adv_sound_is_playing ("is_playing", &proftimer_adv_sound_gas);
#endif
      profiler_timer proftimer_adv_sound ("Sound",        &proftimer_advance,0.0f,true, false);
      profiler_timer proftimer_adv_overlays ("Overlay Tick",        &proftimer_advance,0.0f,true, false);
      profiler_timer proftimer_adv_world       ("World Tick",       &proftimer_advance,0.0f,true, true);
        profiler_timer proftimer_adv_scripts   ("Scripts",        &proftimer_adv_world);
        profiler_timer proftimer_adv_entities  ("Entities",        &proftimer_adv_world,0.0f,true,false);
          profiler_timer proftimer_IFC_sound     ("Sound IFC",           &proftimer_adv_entities);
          profiler_timer proftimer_IFC_damage    ("Damage IFC",           &proftimer_adv_entities);
          profiler_timer proftimer_IFC_physical  ("Physical IFC",           &proftimer_adv_entities);
          profiler_timer proftimer_grenade  ("Grenade",           &proftimer_adv_entities);
          profiler_timer proftimer_gun  ("Gun",           &proftimer_adv_entities);

          profiler_timer proftimer_adv_visrep("Adv Visrep",           &proftimer_adv_entities);
          profiler_timer proftimer_adv_light_mgr("Adv Light Mgr",           &proftimer_adv_entities);
        profiler_timer proftimer_physics  ("Physics",            &proftimer_adv_world, 0.0f, true, false);
          profiler_timer proftimer_adv_controllers("Controllers",     &proftimer_physics);
          profiler_timer proftimer_adv_AI("AI",     &proftimer_physics);
            profiler_timer proftimer_adv_AI_locomotion("Locomotion",     &proftimer_adv_AI);
              profiler_timer proftimer_adv_AI_pathfinding("Pathing",     &proftimer_adv_AI_locomotion);
              profiler_timer proftimer_adv_AI_walk_locomotion("Walk",     &proftimer_adv_AI_locomotion);
                profiler_timer proftimer_adv_AI_walk_rot           ("Apply Rot",  &proftimer_adv_AI_walk_locomotion);
                profiler_timer proftimer_adv_AI_walk_po_update     ("Po Update",   &proftimer_adv_AI_walk_locomotion);
                  profiler_timer proftimer_adv_AI_walk_compute_sector("Comp Sec", &proftimer_adv_AI_walk_po_update);
              profiler_timer proftimer_adv_AI_heli_locomotion("Heli",     &proftimer_adv_AI_locomotion);
                profiler_timer proftimer_adv_AI_heli_rot           ("Apply Rot",  &proftimer_adv_AI_heli_locomotion);
                profiler_timer proftimer_adv_AI_heli_po_update     ("Po Update",   &proftimer_adv_AI_heli_locomotion);
                  profiler_timer proftimer_adv_AI_heli_compute_sector("Comp Sec", &proftimer_adv_AI_heli_po_update);
                profiler_timer proftimer_adv_AI_heli_fly           ("Fly",         &proftimer_adv_AI_heli_locomotion);
              profiler_timer proftimer_adv_AI_winged_locomotion("Winged",     &proftimer_adv_AI_locomotion);
              profiler_timer proftimer_adv_AI_fly_path_locomotion("FlyPath", &proftimer_adv_AI_locomotion);
              profiler_timer proftimer_adv_AI_rot_point("Rot point", &proftimer_adv_AI_locomotion);
            profiler_timer proftimer_adv_AI_hero("Hero",     &proftimer_adv_AI);
            profiler_timer proftimer_adv_AI_threat("Threat",     &proftimer_adv_AI);
            profiler_timer proftimer_adv_AI_senses("Senses",     &proftimer_adv_AI);
            profiler_timer proftimer_adv_AI_fear("Fear",     &proftimer_adv_AI);
            profiler_timer proftimer_adv_AI_priority("Priority",     &proftimer_adv_AI);
            profiler_timer proftimer_adv_AI_goal("Goal",     &proftimer_adv_AI);
//          profiler_timer proftimer_adv_AI_radio("Radio",     &proftimer_adv_AI);
            profiler_timer proftimer_adv_AI_aimer("Aimers",     &proftimer_adv_AI);
            profiler_timer proftimer_adv_AI_cue_mgr("Cue Mgr",     &proftimer_adv_controllers);
            profiler_timer proftimer_adv_player_controller("Plyr ctrlr",&proftimer_adv_controllers);
          profiler_timer proftimer_adv_anims     ("Anims",            &proftimer_physics,0.0f,true,false);
            profiler_timer proftimer_adv_anims_ents  ("Ents",           &proftimer_adv_anims);
            profiler_timer proftimer_anim_adv ("anim_adv", &proftimer_adv_anims);
          profiler_timer proftimer_adv_fcs    ("FCS",                 &proftimer_physics);
          profiler_timer proftimer_adv_generators("Generators",       &proftimer_physics);

          profiler_timer proftimer_compute_sector("Compute Sector",   &proftimer_physics);
          profiler_timer proftimer_update_poss_active("UpdPosAct",   &proftimer_physics);
          profiler_timer proftimer_update_poss_render("UpdPosRend",   &proftimer_physics);
          profiler_timer proftimer_update_poss_collide("UpdPosColl",   &proftimer_physics);

          profiler_timer proftimer_add_pos_increment("AddPosInc",           &proftimer_IFC_physical);
          profiler_timer proftimer_guidance_sys("Guidesys",           &proftimer_IFC_physical);

          profiler_timer proftimer_adv_mcs    ("MCS",                 &proftimer_physics);
          profiler_timer proftimer_phys_render("Phys render",         &proftimer_physics);
          profiler_timer proftimer_get_elevation("Get elev",          &proftimer_physics);
          profiler_timer proftimer_collide    ("Collide",             &proftimer_physics);
            profiler_timer proftimer_wave_collide("Wave collide",     &proftimer_collide);
            profiler_timer proftimer_find_intersection    ("findintersect",             &proftimer_collide);
              profiler_timer proftimer_find_int_world    ("world",             &proftimer_find_intersection);
              profiler_timer proftimer_find_int_ent    ("entities",             &proftimer_find_intersection);
                profiler_timer proftimer_find_int_ent_ent    ("ent",             &proftimer_find_int_ent);
                  profiler_timer proftimer_find_int_ent_ent_col    ("col",             &proftimer_find_int_ent_ent);
                  profiler_timer proftimer_find_int_ent_ent_col_sph    ("sph",             &proftimer_find_int_ent_ent_col);
                  profiler_timer proftimer_find_int_ent_ent_col_geo    ("geo",             &proftimer_find_int_ent_ent_col);
                profiler_timer proftimer_find_int_ent_reg    ("reg",             &proftimer_find_int_ent);
                profiler_timer proftimer_find_int_ent_reg_col    ("col",             &proftimer_find_int_ent_reg);
            //profiler_timer proftimer_collide_mesh_rgn   ("Mesh-Rgn",     &proftimer_collide);
            //profiler_timer proftimer_collide_mesh_rgn_a ("Mesh-Rgn_A",   &proftimer_collide);
            //profiler_timer proftimer_collide_seg_rgn    ("Seg-Rgn",      &proftimer_collide);
            profiler_timer proftimer_collide_entity_entity("Ent-Ent",        &proftimer_collide);
            profiler_timer proftimer_collide_entity_entity_int("Ent-Ent Int",&proftimer_collide);
            //profiler_timer proftimer_collide_actor_ter  ("Act-Ter",      &proftimer_collide);
      //profiler_timer proftimer_events      ("Events",        &proftimer_advance);
      profiler_timer proftimer_camera        ("Camera",        &proftimer_advance);
      profiler_timer proftimer_special_fx    ("Special FX",    &proftimer_advance,0.0f, true, false);
      profiler_timer proftimer_advance_particles("Particles",    &proftimer_special_fx);
      //profiler_timer proftimer_dynamic_ents  ("Dynamic ents",  &proftimer_advance);
      profiler_timer proftimer_scanner_advance("Scanner",      &proftimer_advance);

      profiler_timer proftimer_adv_ent_timed("Ent timed",      &proftimer_advance);
      profiler_timer proftimer_adv_ent_setup("Ent setup",      &proftimer_advance);

// ---- Render
    profiler_timer proftimer_render        ("Render",    &proftimer_cpu_net,0.0f,true,true);
	//profiler_timer proftimer_frame_limiter ("Limiter",    &proftimer_render);
	  profiler_timer proftimer_profiler      ("Profiler",      &proftimer_render, 0.0f, true, false);
		profiler_timer proftimer_draw_prof_info("Draw info", &proftimer_profiler);
		profiler_timer proftimer_debug_info    ("Debug info",         &proftimer_profiler);
	  profiler_timer proftimer_render_cpu      ("CPU net",      &proftimer_render, 0.0f, true, true);
      profiler_timer proftimer_render_sendlist ("krListSend", &proftimer_render_cpu,0.0f,true,false);
#ifdef TARGET_PS2
        profiler_timer proftimer_send_addnodes("Add Nodes", &proftimer_render_sendlist );
          profiler_timer proftimer_send_texturestream("Textures", &proftimer_send_addnodes );
          profiler_timer proftimer_send_rendersetup("Render Setup", &proftimer_send_addnodes,0.0f,true,false );
          profiler_timer proftimer_send_setup_material("Material Setup", &proftimer_send_addnodes);
          profiler_timer proftimer_send_batches("Batches", &proftimer_send_addnodes );

        profiler_timer proftimer_send_addquads("Add Quads", &proftimer_render_sendlist );
        profiler_timer proftimer_send_cacheflush("Cache Flush", &proftimer_render_sendlist );
        profiler_timer proftimer_send_dmasend("DMA Send", &proftimer_render_sendlist );
        profiler_timer proftimer_send_interrupt("Interrupt", &proftimer_render_sendlist );
          profiler_timer proftimer_send_interrupt_dmawait("DMA Wait", &proftimer_send_interrupt );
#endif
      profiler_timer proftimer_render_interface("Interface",&proftimer_render_cpu,0.0f,true,false);
#ifdef TARGET_PC
        profiler_timer proftimer_widget_sendctx    ("Widget Send Ctx",&proftimer_render_interface);
        profiler_timer proftimer_widget_draw       ("Widget Draw",    &proftimer_render_interface);
#endif
      profiler_timer proftimer_render_world    ("World", &proftimer_render_cpu);
        profiler_timer proftimer_kelly_slater       ("Kelly Slater",     &proftimer_render_world);
          profiler_timer proftimer_kelly_slater_interface("Interface",&proftimer_kelly_slater);
          profiler_timer proftimer_water          ("Water",&proftimer_kelly_slater);
            profiler_timer proftimer_water_create          ("Water Create",&proftimer_water);
              profiler_timer proftimer_wave          ("Wave",&proftimer_water_create);
                profiler_timer proftimer_wave_compute_mesh   ("Compute Mesh",&proftimer_wave);
                profiler_timer proftimer_wave_build_mesh   ("Build Mesh",&proftimer_wave);
					profiler_timer proftimer_spline_coeffs  ("Spline Coeffs",&proftimer_wave_build_mesh);
              profiler_timer proftimer_water_seam          ("Water Seam",&proftimer_water_create);
              profiler_timer proftimer_water_far           ("Water Far",&proftimer_water_create);
              profiler_timer proftimer_water_horizon       ("Water Horizon",&proftimer_water_create);
            profiler_timer proftimer_water_submit          ("Water Submit",&proftimer_water);
          profiler_timer proftimer_kelly_slater_particle("Particles",&proftimer_kelly_slater);
            profiler_timer proftimer_render_crash ("Crash",&proftimer_kelly_slater_particle);
            profiler_timer proftimer_render_lip ("Lip",&proftimer_kelly_slater_particle);
            profiler_timer proftimer_render_loose ("Loose",&proftimer_kelly_slater_particle);
            profiler_timer proftimer_render_spray ("Spray",&proftimer_kelly_slater_particle);
            profiler_timer proftimer_render_trail ("Trail",&proftimer_kelly_slater_particle);
            profiler_timer proftimer_track_floater ("Track Floater",&proftimer_render_trail);
        profiler_timer proftimer_build_data       ("Build data",     &proftimer_render_world,0.0,true,false);
          profiler_timer proftimer_build_data_clear ("Clear data",   &proftimer_build_data);
          profiler_timer proftimer_det_vis_ents     ("Vis Entities", &proftimer_build_data);
          profiler_timer proftimer_det_vis_rgn      ("Vis Regions",  &proftimer_build_data);
#ifdef TARGET_PC
        profiler_timer proftimer_rgn_trisetup     ("Rgn trisetup",   &proftimer_render_world);
        profiler_timer proftimer_rgn_draw         ("Rgn draw",       &proftimer_render_world);
        profiler_timer proftimer_rgn_ent_trisetup ("Ent Tri Setup",  &proftimer_render_world);
        profiler_timer proftimer_rgn_ent_draw     ("Ent Draw",       &proftimer_render_world);
#endif
        profiler_timer proftimer_opaque_rgn       ("Opaque rgn",     &proftimer_render_world);
        profiler_timer proftimer_opaque_ent       ("Opaque ent",     &proftimer_render_world,0.0f,true,false);
          profiler_timer proftimer_render_entity    ("Render entity",    &proftimer_opaque_ent);
          profiler_timer proftimer_render_add_mesh  ("krListAddMesh",    &proftimer_opaque_ent,0.0f,true,false);
#ifdef TARGET_PS2
            profiler_timer proftimer_addmesh_culling    ("Culling",    &proftimer_render_add_mesh);
            profiler_timer proftimer_addmesh_meshnode   ("Mesh Node",    &proftimer_render_add_mesh);
            profiler_timer proftimer_addmesh_listnode   ("List Node",    &proftimer_render_add_mesh);
#endif

#ifdef TARGET_PC
            profiler_timer proftimer_instance_render  ("Instance render",    &proftimer_render_entity);
              profiler_timer proftimer_visrep_rend_inst ("Visrep rend inst",    &proftimer_instance_render);
#endif
        profiler_timer proftimer_ent_lights       ("Lights",  &proftimer_render_world);
#ifdef TARGET_PC
        profiler_timer proftimer_trans_ent        ("Trans ent/rgn",  &proftimer_render_world);
#endif
        profiler_timer proftimer_render_particles ("Particles",      &proftimer_render_world,0.0f,true,false);
          profiler_timer proftimer_render_particles_bb ("Billboard", &proftimer_render_particles);
          profiler_timer proftimer_render_particles_mesh ("Mesh",    &proftimer_render_particles);
        profiler_timer proftimer_render_billboards("Billboards",     &proftimer_render_world, 0.0f, true, false);
          profiler_timer proftimer_render_setup_billboards("Setup",  &proftimer_render_billboards);
          profiler_timer proftimer_render_c_billboards ("C bb",      &proftimer_render_billboards);
          profiler_timer proftimer_render_c_axis_billboards ("C axis bb",&proftimer_render_billboards);

          profiler_timer proftimer_render_billboard_create_mesh ("create Mesh",&proftimer_render_particles_bb);
            profiler_timer proftimer_render_bb_calc_size ("Calc size",&proftimer_render_billboard_create_mesh);
          profiler_timer proftimer_render_billboard_fill ("fill Mesh",&proftimer_render_particles_bb);
          profiler_timer proftimer_render_billboard_sphere ("calc sphere",&proftimer_render_particles_bb);
          profiler_timer proftimer_render_billboard_add_mesh ("add Mesh",&proftimer_render_particles_bb);
          profiler_timer proftimer_render_billboard_misc ("Misc",&proftimer_render_particles_bb);
#ifdef TARGET_PC
          profiler_timer proftimer_render_sendctx_billboards ("Send Ctx",&proftimer_render_billboards);
          profiler_timer proftimer_render_draw_billboards ("Draw",&proftimer_render_billboards);
#endif
#ifdef TARGET_PC
        profiler_timer proftimer_render_instance  ("Instance",     &proftimer_render_world,0.0f,true,false);
          profiler_timer proftimer_instance_light_setup("Lite Setup",  &proftimer_render_instance);
          profiler_timer proftimer_clip_xform_light ("Mesh Lite",      &proftimer_render_instance);
          profiler_timer proftimer_instance_sendctx ("Mesh Send Ctx",  &proftimer_render_instance);
          profiler_timer proftimer_instance_draw    ("Mesh Draw",      &proftimer_render_instance);
        profiler_timer proftimer_render_skin       ("Skin",     &proftimer_render_world,0.0f,true,false);
          profiler_timer proftimer_skin_xform       ("Skin Xform",     &proftimer_render_skin);
          profiler_timer proftimer_skin_lite        ("Skin Lite",      &proftimer_render_skin);
          profiler_timer proftimer_skin_trisetup    ("Skin Tri Setup", &proftimer_render_skin);
          profiler_timer proftimer_skin_draw        ("Skin Draw",      &proftimer_render_skin);
#endif
      #if !defined(BUILD_BOOTABLE)
      profiler_timer proftimer_render_debug   ("Debug",          &proftimer_render_cpu);
      #endif

#ifdef TARGET_PC
profiler_group profgroup_render_misc("Misc",NULL,false);
  profiler_timer proftimer_scanner_render   ("Scanner",        &profgroup_render_misc);
  profiler_timer proftimer_draw_shadow      ("Shadow",         &profgroup_render_misc);
  profiler_timer proftimer_motion_trail     ("Motion trail",   &profgroup_render_misc);
#else
profiler_group profgroup_render_misc("Misc",NULL,true);
  profiler_timer proftimer_ksc_anim         ("Anim change ",   &profgroup_render_misc);
  profiler_timer proftimer_ksc_ik           ("Anim IK",        &profgroup_render_misc);
  profiler_timer proftimer_play_anim1       ("Play anim1",     &proftimer_ksc_anim   );
  profiler_timer proftimer_play_anim2       ("Play anim2",     &proftimer_ksc_anim   );
  profiler_timer proftimer_play_anim1new    ("Play new anim",     &proftimer_play_anim1 );
  profiler_timer proftimer_play_anim1old    ("Play old anim",     &proftimer_play_anim1 );
  profiler_timer proftimer_create_anim1     ("Create anim1",     &proftimer_ksc_anim   );
  profiler_timer proftimer_create_anim2     ("Create anim2",     &proftimer_ksc_anim   );
  profiler_timer proftimer_create_anim3     ("Create anim3",     &proftimer_ksc_anim   );
  profiler_timer proftimer_create_anim4     ("Create anim4",     &proftimer_ksc_anim   );
  profiler_timer proftimer_anim_const1      ("Anim Construct1",     &proftimer_ksc_anim   );
  profiler_timer proftimer_anim_const1pre      ("Anim Const pre",     &proftimer_anim_const1 );
  profiler_timer proftimer_anim_const1rec      ("Anim Recursive",     &proftimer_anim_const1 );
  profiler_timer proftimer_anim_const1post     ("Anim Const post",     &proftimer_anim_const1 );
  profiler_timer proftimer_anim_scanner        ("Anim Scanner",     &proftimer_anim_const1rec );
  profiler_timer proftimer_anim_recursekids    ("Anim ReKids ",     &proftimer_anim_const1rec );
  profiler_timer proftimer_anim_recursemake    ("Anim ReMake ",     &proftimer_anim_const1rec );
  profiler_timer proftimer_anim_recursecrap    ("Anim ReCrap ",     &proftimer_anim_const1rec );
  profiler_timer proftimer_anim_const2      ("Anim Construct2",     &proftimer_ksc_anim   );
#endif

profiler_group profgroup_render_generic("Generic",NULL,true);
  profiler_timer proftimer_generic_0   ("Generic_0",        &profgroup_render_generic);
  profiler_timer proftimer_generic_1   ("Generic_1",        &profgroup_render_generic);
  profiler_timer proftimer_generic_2   ("Generic_2",        &profgroup_render_generic);
  profiler_timer proftimer_generic_3   ("Generic_3",        &profgroup_render_generic);
  profiler_timer proftimer_generic_4   ("Generic_4",        &profgroup_render_generic);
  profiler_timer proftimer_generic_5   ("Generic_5",        &profgroup_render_generic);
  profiler_timer proftimer_generic_6   ("Generic_6",        &profgroup_render_generic);
  profiler_timer proftimer_generic_7   ("Generic_7",        &profgroup_render_generic);
  profiler_timer proftimer_generic_8   ("Generic_8",        &profgroup_render_generic);
  profiler_timer proftimer_generic_9   ("Generic_9",        &profgroup_render_generic);
  profiler_timer proftimer_generic_10   ("Generic_10",        &profgroup_render_generic);
  profiler_timer proftimer_generic_11   ("Generic_11",        &profgroup_render_generic);
  profiler_timer proftimer_generic_12   ("Generic_12",        &profgroup_render_generic);
  profiler_timer proftimer_generic_13   ("Generic_13",        &profgroup_render_generic);
  profiler_timer proftimer_generic_14   ("Generic_14",        &profgroup_render_generic);
  profiler_timer proftimer_generic_15   ("Generic_15",        &profgroup_render_generic);
  profiler_timer proftimer_generic_16   ("Generic_16",        &profgroup_render_generic);
  profiler_timer proftimer_generic_17   ("Generic_17",        &profgroup_render_generic);
  profiler_timer proftimer_generic_18   ("Generic_18",        &profgroup_render_generic);
  profiler_timer proftimer_generic_19   ("Generic_19",        &profgroup_render_generic);

profiler_group profgroup_render_file("File",NULL,false);
  profiler_timer proftimer_file_exists("File Exists", &profgroup_render_file);
  profiler_timer proftimer_file_open  ("File Open",   &profgroup_render_file);
  profiler_timer proftimer_file_read  ("File Read",   &profgroup_render_file);

#ifdef TARGET_PC
profiler_group profgroup_renderer_lowlevel("Low-level render",NULL,false);
  profiler_timer proftimer_process_context  ("Process context",&profgroup_renderer_lowlevel);
  profiler_timer proftimer_send_context     ("Send context",   &profgroup_renderer_lowlevel);
  profiler_timer proftimer_draw_primitive   ("Draw primitive", &profgroup_renderer_lowlevel);
#endif

