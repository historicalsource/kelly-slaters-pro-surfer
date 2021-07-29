// profcounters.cpp
// Copyright (c) 2000 Treyarch Invention LLC.  ALL RIGHTS RESERVED. 

#include "global.h"

#include "profiler.h"

profiler_counter profcounter_frame_delta("Frame msecs",NULL,1000/30,true,false);
  profiler_counter profcounter_frame_min_delta("Min msecs",&profcounter_frame_delta,1000/30);
  profiler_counter profcounter_frame_avg_delta("Avg msecs",&profcounter_frame_delta,1000/30);
  profiler_counter profcounter_frame_max_delta("Max msecs",&profcounter_frame_delta,1000/30);

profiler_group profgroup_rend_count("Graphics counters",NULL,false);
  profiler_counter profcounter_tri_rend("Tri rend", &profgroup_rend_count, 15000);
  profiler_counter profcounter_node_rend("Node rend",&profgroup_rend_count, 5000 );
  profiler_counter profcounter_sfx_tri_rend("Sfx tri rend",&profgroup_rend_count, 1000);
  profiler_counter profcounter_particle_systems_rend("Particle systems rend",&profgroup_rend_count, 75);
  profiler_counter profcounter_particle_rend("Particle rend",&profgroup_rend_count, 750);
    profiler_counter profcounter_particle_rend_bb("BB particle rend",&profcounter_particle_rend, 700);
    profiler_counter profcounter_particle_scratch_bb("BB scratches",&profcounter_particle_rend, 700);
    profiler_counter profcounter_particle_rend_mesh("Mesh particle rend",&profcounter_particle_rend, 50);
  profiler_counter profcounter_path_collision("Path collision",&profgroup_rend_count);
  profiler_counter profcounter_lod_tri_estimate("LOD tri estimate",&profgroup_rend_count);
  profiler_counter profcounter_rgn_tri_rend("Rgn tri rend",&profgroup_rend_count);

profiler_group profgroup_ent_count("Entity counters",NULL,false);
  profiler_counter profcounter_compute_sector("Compute Sectors",&profgroup_ent_count);
  profiler_counter profcounter_regions("Active Rgns",&profgroup_ent_count);
  profiler_counter profcounter_ents("Ents",&profgroup_ent_count);
  profiler_counter profcounter_poss_active_ents("PossActive Ents",&profgroup_ent_count);
  profiler_counter profcounter_poss_render_ents("PossRender Ents",&profgroup_ent_count);
  profiler_counter profcounter_poss_collide_ents("PossCollide Ents",&profgroup_ent_count);
  profiler_counter profcounter_active_ents("Active Ents",&profgroup_ent_count);
  profiler_counter profcounter_rendered_ents("Rendered Ents",&profgroup_ent_count);
  profiler_counter profcounter_anims("Anims",&profgroup_ent_count);
#ifdef TARGET_PS2
  profiler_counter profcounter_gas_add_inst("gas add inst",&profgroup_ent_count);
  profiler_counter profcounter_gas_add_inst_legit("gas add inst legit",&profgroup_ent_count);
#endif

  profiler_group profgroup_misc_count("Misc counters",NULL,false);
  profiler_counter profcounter_find_intersection("find intersect",&profgroup_misc_count);

/*
profiler_counter profcounter_total_blocks("Total blocks",NULL,  80000,false);
profiler_counter profcounter_alloced_mem("System mem",    NULL,8300000,false);
  profiler_counter profcounter_terrain_mem("Terrain",  &profcounter_alloced_mem,1000000,false);
  profiler_counter profcounter_entity_mem ("Entity",   &profcounter_alloced_mem, 700000,false);
  profiler_counter profcounter_physent_mem("Phys Ent", &profcounter_alloced_mem, 500000,false);
  profiler_counter profcounter_actor_mem  ("Actor",    &profcounter_alloced_mem, 100000,false);
  profiler_counter profcounter_character_mem("Character",&profcounter_alloced_mem,15000000,false);
    profiler_counter profcounter_hero_mem    ("Hero",    &profcounter_character_mem, 500000,false);
  profiler_counter profcounter_conglom_mem ("Conglom", &profcounter_alloced_mem, 200000,false);
  profiler_counter profcounter_item_mem    ("Item",    &profcounter_alloced_mem, 100000,false);
  profiler_counter profcounter_marker_mem  ("Marker",  &profcounter_alloced_mem,  10000,false);
  profiler_counter profcounter_particle_mem("Particle",&profcounter_alloced_mem, 300000,false);
  profiler_counter profcounter_light_mem   ("Light",   &profcounter_alloced_mem,  60000,false);
  profiler_counter profcounter_ladder_mem  ("Ladder",  &profcounter_alloced_mem,  10000,false);
  profiler_counter profcounter_script_mem  ("Script",  &profcounter_alloced_mem, 500000,false);
profiler_counter profcounter_texture_mem("Texture mem",   NULL,3500000,false);
profiler_counter profcounter_audio_mem  ("Audio mem",     NULL,1300000,false);
*/
