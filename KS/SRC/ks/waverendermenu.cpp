#include "menu.h"

#if defined(TARGET_GC)
#define TARGET_MENUTEMPLATE "hwosgc\\gc_waverenderdebug.txt"
#elif defined(TARGET_PS2)
#define TARGET_MENUTEMPLATE "hwosps2\\ps2_waverenderdebug.txt"
#elif defined(TARGET_XBOX)
#define TARGET_MENUTEMPLATE "hwosxb\\xb_waverenderdebug.txt"
#else
#error Unknown target console!
#endif

// Debugging flags and debug menu
#undef USEINFO
#undef USEINFOINT
#undef USEINFOFLOAT
#define USEINFO(item, type, lo, hi, step, init, menu) USEINFO##type(item)
#define USEINFOINT(item) int item;
#define USEINFOFLOAT(item) float item;
#ifndef TARGET_GC
	// I hate Metrowerks I hate Metrowerks I hate Metrowerks I hate Metrowerks
static
#endif
struct WaverenderDebugStruct
{
#include TARGET_MENUTEMPLATE
}
#undef USEINFO
#define USEINFO(item, type, lo, hi, step, init, menu) init,
WaverenderDebug =
{
#include TARGET_MENUTEMPLATE
};

#undef USEINFO
#undef USEINFONO
#undef USEINFOYES
#undef USEINFOINT
#undef USEINFOFLOAT
#define USEINFO(item, type, lo, hi, step, init, menu) USEINFO##menu(item, type, lo, hi, step, init)
#define USEINFONO(item, type, lo, hi, step, init)
#define USEINFOYES(item, type, lo, hi, step, init) USEINFO##type(item)
#define USEINFOINT(item) 			\
static MenuEntryIntEdit * WAVERENDER_MenuEntry##item = NULL;
#define USEINFOFLOAT(item) 			\
static MenuEntryFloatEdit * WAVERENDER_MenuEntry##item = NULL;
#include TARGET_MENUTEMPLATE

#undef USEINFOYES
#define USEINFOYES(item, type, lo, hi, step, init) WAVERENDER_MenuEntry##item,
static pMenuEntry WAVERENDER_MenuEntries[] =
{
#include TARGET_MENUTEMPLATE
};

Menu *waverender_menu = NULL;

Submenu *menu_waverender = NULL;

void WAVERENDER_StaticInit( void )
{
	int index = 0;
#undef USEINFOYES
#undef USEINFOINT
#undef USEINFOFLOAT
#define USEINFOYES(item, type, lo, hi, step, init) USEINFO##type(item, lo, hi, step)
#define USEINFOINT(item, lo, hi, step)							\
	WAVERENDER_MenuEntry##item =									\
	NEW MenuEntryIntEdit(#item, &WaverenderDebug.##item, lo, hi);
#define USEINFOFLOAT(item, lo, hi, step)						\
	WAVERENDER_MenuEntry##item =									\
	NEW MenuEntryFloatEdit(#item, &WaverenderDebug.##item, lo, hi, step);
#include TARGET_MENUTEMPLATE

#undef USEINFOYES
#define USEINFOYES(item, type, lo, hi, step, init) WAVERENDER_MenuEntries[index++] = WAVERENDER_MenuEntry##item;
#include TARGET_MENUTEMPLATE

	waverender_menu = NEW Menu(NULL, countof(WAVERENDER_MenuEntries), WAVERENDER_MenuEntries);

	menu_waverender = NEW Submenu( "Wave Render Menu", waverender_menu );
}

