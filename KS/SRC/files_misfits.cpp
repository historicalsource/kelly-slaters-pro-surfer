// misfit files that refuse to behave in other files_*.cpp files

// With precompiled headers enabled, all text up to and including 
// the following line are ignored by the compiler (dc 01/18/02)
#include "global.h"

#include "conglom.cpp"
#include "blur.cpp"
//#include "spiderman_combo_data.cpp"
#include "entity_maker.cpp"
#include "app.cpp"
#include "ini_parser.cpp"
#include "file_manager.cpp"
#include "menudraw.cpp"
#include "menu_scoring.cpp"
//#include "refract.cpp"
#include "timer.cpp"
//#include "FrontEndManager.cpp"
//#include "FEMenu.cpp"


// New addition.. #define users.cpp in users.h to include users.cpp
#include "users.h"
#ifdef USERSCPP
#include "users.cpp"
#endif