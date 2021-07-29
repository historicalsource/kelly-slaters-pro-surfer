MAC(STRING_SOUND_LIST,"SOUND_LIST","ks.snd")
MAC(STRING_SCENE_NAME,"SCENE_NAME","Ask")
MAC(STRING_STASH_NAME,"STASH_NAME","Bells")

//---------------------------------------------------------------------------------
//there needs to be one of these for as many heroes as are possible in multiplayer
//currently MAX_PLAYERS = 2
// (because I cant figure out an easy way to make an array out of these)  -DL
MAC(STRING_HERO_NAME_0,"HERO_NAME_0","KellySlater")       // hero 0
MAC(STRING_HERO_NAME_1,"HERO_NAME_1","RobMachado")        // hero 1
MAC(STRING_END_HERO_NAMES,"END_HERO_NAMES","xxx")         // placeholder for error checking
//---------------------------------------------------------------------------------
MAC(STRING_GAME_MODE,"GAME_MODE","Freesurf") 
//---------------------------------------------------------------------------------

MAC(STRING_GAME_TITLE,"GAME_TITLE","Kelly Slater")
MAC(STRING_GAME_LONG_TITLE,"GAME_LONG_TITLE","Kelly Slater's Pro Surfer")
MAC(STRING_SAVE_GAME_PREFIX,"SAVE_GAME_PREFIX","KELLYSLATER")
MAC(STRING_SAVE_GAME_DESC,"SAVE_GAME_DESC","Kelly Slater's Pro Surfer saved game")
MAC(STRING_SONG_NAME,"SONG_NAME","")
MAC(STRING_VIDEO_MODE,"VIDEO_MODE","512x448")
MAC(STRING_GFX_DEVICE,"GFX_DEVICE","display")
//P MAC(STRING_PFE_SCRIPT,"PFE_SCRIPT","interface\\pages.pfe")
//MAC(STRING_HERO_STATE_FILE,"HERO_STATE_FILE","SPIDERMAN.001")
MAC(STRING_LOCALE,"LOCALE","english")
MAC(STRING_WRITE_BINARY_DIR,"WRITE_BINARY_DIR","ksbin")

MAC(STRING_ROOT_DIR,"ROOT_DIR","data")
MAC(STRING_PRE_ROOT_DIR,"PRE_ROOT_DIR",".") // empty string won't work
MAC(STRING_PC_SOUNDS_DIR,"PC_SOUNDS_DIR","..\\pc_sounds")
//MAC(STRING_PC_MOVIES_DIR,"PC_MOVIES_DIR","..\\pc_movies")

MAC(STRING_TEXTURE_DIR,"TEXTURE_DIR","textures")

// names of intro and credit movies
MAC(STRING_LOGOS_MOVIE,"LOGOS_MOVIE","logos")
MAC(STRING_INTRO_MOVIE,"INTRO_MOVIE","intro")
MAC(STRING_CREDITS_MOVIE,"CREDITS_MOVIE","credits")

// get all debug output by default, you can set this to NONE if you wish,
// or to your initials (see users.h)
MAC(STRING_DEBUG_USER,"DEBUG_USER","ALL")

// allow this higher level to dictate what VMU pictures get used
//MAC(STRING_VMU_GAME,"VMU_GAME","interface\\vmumax.bmp")
//MAC(STRING_VMU_LOAD,"VMU_LOAD","interface\\vmuload.bmp")
//MAC(STRING_VMU_SAVE,"VMU_SAVE","interface\\vmusave.bmp")
//MAC(STRING_VMU_OK,"VMU_OK","interface\\vmuok.bmp")
//MAC(STRING_VMU_FAIL,"VMU_FAIL","interface\\vmufailure.bmp")

// allow the artists to load a model and play an animation
MAC(STRING_PLAY_MODEL, "PLAY_MODEL", "")
MAC(STRING_PLAY_ANIM, "PLAY_ANIM", "")

MAC(STRING_MEM_DUMP_FILE, "MEM_DUMP_FILE", "memory_dump.txt")
MAC(STRING_PROFILE_DUMP_FILE, "PROFILE_DUMP_FILE", "profiler.txt")
MAC(STRING_BACKFACE_CULL, "BACKFACE_CULL", "DEFAULT")
