#if defined(GAME_INFO_NUMS)

MAC(int, difficulty, 1, "DIFFICULTY");

#endif


#if defined(GAME_INFO_STRS)

MAC(stringx, hero_name_0, os_developer_options::inst()->get_hero_name(0), "HERO_NAME_0");
MAC(stringx, hero_name_1, os_developer_options::inst()->get_hero_name(1), "HERO_NAME_1");

#endif