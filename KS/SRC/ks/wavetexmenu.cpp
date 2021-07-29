//-------------------------------------
// WaveTex menu

MenuEntryIntEdit *  shadows = NULL; //MenuEntryIntEdit  ( "Shadows", &WavetexDebug_ShadowPass             ,        0,         1,    1 );
MenuEntryFloatEdit *dscale = NULL; //MenuEntryFloatEdit( "Dark Scale",  &WAVETEX_DarkAlphaScale           , -4096.0f, 4096.0f, 1.0f );
MenuEntryFloatEdit *doff   = NULL; //MenuEntryFloatEdit( "Dark Offset", &WAVETEX_DarkAlphaOffset          , -4096.0f, 4096.0f, 1.0f );
MenuEntryFloatEdit *escale = NULL; //MenuEntryFloatEdit( "Dark Hi Scale",  &WAVETEX_DarkHighAlphaScale           , -4096.0f, 4096.0f, 1.0f );
MenuEntryFloatEdit *eoff   = NULL; //MenuEntryFloatEdit( "Dark Hi Offset", &WAVETEX_DarkHighAlphaOffset          , -4096.0f, 4096.0f, 1.0f );
MenuEntryFloatEdit *fscale = NULL; //MenuEntryFloatEdit( "Underwater Scale",  &WAVETEX_DarkUnderAlphaScale           , -4096.0f, 4096.0f, 1.0f );
MenuEntryFloatEdit *foff   = NULL; //MenuEntryFloatEdit( "Underwater Offset", &WAVETEX_DarkUnderAlphaOffset          , -4096.0f, 4096.0f, 1.0f );
MenuEntryFloatEdit *hscale = NULL; //MenuEntryFloatEdit( "High Scale",  &WAVETEX_HighlightAlphaScale      , -4096.0f, 4096.0f, 1.0f );
MenuEntryFloatEdit *hoff   = NULL; //MenuEntryFloatEdit( "High Offset", &WAVETEX_HighlightAlphaOffset     , -4096.0f, 4096.0f, 1.0f );
MenuEntryFloatEdit *cscale = NULL; //MenuEntryFloatEdit( "Core Scale",  &WAVETEX_CoreHighlightAlphaScale  , -4096.0f, 4096.0f, 1.0f );
MenuEntryFloatEdit *coff   = NULL; //MenuEntryFloatEdit( "Core Offset", &WAVETEX_CoreHighlightAlphaOffset , -4096.0f, 4096.0f, 1.0f );
MenuEntryFloatEdit *sunx   = NULL; //MenuEntryFloatEdit( "Sun X      ", &WAVETEX_SunPos[0]                , -10000.0f, 10000.0f, 100.0f );
MenuEntryFloatEdit *suny   = NULL; //MenuEntryFloatEdit( "Sun Y      ", &WAVETEX_SunPos[1]                , -10000.0f, 10000.0f, 100.0f );
MenuEntryFloatEdit *sunz   = NULL; //MenuEntryFloatEdit( "Sun Z      ", &WAVETEX_SunPos[2]                , -10000.0f, 10000.0f, 100.0f );
MenuEntryFloatEdit *tscale = NULL; //MenuEntryFloatEdit( "Trans Scale", &WAVETEX_transcale                ,     0.0f,    128.0f, 1.0f );
MenuEntryIntEdit *  trmin  = NULL; //MenuEntryIntEdit  ( "Trans Min  ", &WAVETEX_transmin                 ,        0,       128,    4 );

MENUENTRYBUTTONFUNCTION(ReloadWaveTextures)
{
	if ( buttonid==MENUCMD_CROSS )
		WAVETEX_ReloadTextureAnims();
	return true;
}

MenuEntryFunction *wavetexreload  = NULL; //MenuEntryFunction( "Reload Textures",   ReloadWaveTextures  );

#undef USEINFO
#undef USEINFONO
#undef USEINFOYES
#undef USEINFOINT
#undef USEINFOFLOAT
#define USEINFO(item, type, lo, hi, step, init, menu) USEINFO##menu(item, type, lo, hi, step, init)
#define USEINFONO(item, type, lo, hi, step, init)
#define USEINFOYES(item, type, lo, hi, step, init) USEINFO##type(item)
#define USEINFOINT(item) 			\
static MenuEntryIntEdit * WAVETEX_MenuEntry##item = NULL;
#define USEINFOFLOAT(item) 			\
static MenuEntryFloatEdit * WAVETEX_MenuEntry##item = NULL;
#include "wavetexdebug.txt"

#undef USEINFOYES
#define USEINFOYES(item, type, lo, hi, step, init) WAVETEX_MenuEntry##item,
static pMenuEntry WAVETEX_MenuEntries[] =
{
#ifndef DAVID
	shadows,
	dscale,
	doff,
	escale,
	eoff,
	fscale,
	foff,
	hscale,
	hoff,
	cscale,
	coff,
	sunx,
	suny,
	sunz,
	tscale,
	trmin,
	wavetexreload,
#endif	//#ifndef DAVID
#include "wavetexdebug.txt"
};

Menu *wavetex_menu = NULL; //Menu(NULL,17,WAVETEX_MenuEntries);

Submenu *menu_wavetex = NULL; //Submenu( "Wave Texture Menu", &wavetex_menu );

void WAVETEXMENU_StaticInit( void )
{
	int index=0;
#ifndef DAVID
#ifndef TARGET_GC
	shadows= NEW MenuEntryIntEdit  ( "Shadows", &WavetexDebug_ShadowPass             ,        0,         1,    1 );
	dscale= NEW MenuEntryFloatEdit( "Dark Scale",  &WAVETEX_DarkAlphaScale           , -4096.0f, 4096.0f, 1.0f );
	doff  = NEW MenuEntryFloatEdit( "Dark Offset", &WAVETEX_DarkAlphaOffset          , -4096.0f, 4096.0f, 1.0f );
	escale= NEW MenuEntryFloatEdit( "Dark Hi Scale",  &WAVETEX_DarkHighAlphaScale           , -4096.0f, 4096.0f, 1.0f );
	eoff  = NEW MenuEntryFloatEdit( "Dark Hi Offset", &WAVETEX_DarkHighAlphaOffset          , -4096.0f, 4096.0f, 1.0f );
	fscale= NEW MenuEntryFloatEdit( "Underwater Scale",  &WAVETEX_DarkUnderAlphaScale           , -4096.0f, 4096.0f, 1.0f );
	foff  = NEW MenuEntryFloatEdit( "Underwater Offset", &WAVETEX_DarkUnderAlphaOffset          , -4096.0f, 4096.0f, 1.0f );
	hscale= NEW MenuEntryFloatEdit( "High Scale",  &WAVETEX_HighlightAlphaScale      , -4096.0f, 4096.0f, 1.0f );
	hoff  = NEW MenuEntryFloatEdit( "High Offset", &WAVETEX_HighlightAlphaOffset     , -4096.0f, 4096.0f, 1.0f );
	cscale= NEW MenuEntryFloatEdit( "Core Scale",  &WAVETEX_CoreHighlightAlphaScale  , -4096.0f, 4096.0f, 1.0f );
	coff  = NEW MenuEntryFloatEdit( "Core Offset", &WAVETEX_CoreHighlightAlphaOffset , -4096.0f, 4096.0f, 1.0f );
	sunx  = NEW MenuEntryFloatEdit( "Sun X      ", &WAVETEX_SunPos[0]                , -10000.0f, 10000.0f, 100.0f );
	suny  = NEW MenuEntryFloatEdit( "Sun Y      ", &WAVETEX_SunPos[1]                , -10000.0f, 10000.0f, 100.0f );
	sunz  = NEW MenuEntryFloatEdit( "Sun Z      ", &WAVETEX_SunPos[2]                , -10000.0f, 10000.0f, 100.0f );
	tscale= NEW MenuEntryFloatEdit( "Trans Scale", &WAVETEX_transcale                ,     0.0f,    128.0f, 1.0f );
	trmin = NEW MenuEntryIntEdit  ( "Trans Min  ", &WAVETEX_transmin                 ,        0,       128,    4 );

	wavetexreload  = NEW MenuEntryFunction( "Reload Textures",   ReloadWaveTextures  );

	WAVETEX_MenuEntries[index++]= shadows;
	WAVETEX_MenuEntries[index++]= dscale;
	WAVETEX_MenuEntries[index++]= doff;
	WAVETEX_MenuEntries[index++]= escale;
	WAVETEX_MenuEntries[index++]= eoff;
	WAVETEX_MenuEntries[index++]= fscale;
	WAVETEX_MenuEntries[index++]= foff;
	WAVETEX_MenuEntries[index++]= hscale;
	WAVETEX_MenuEntries[index++]= hoff;
	WAVETEX_MenuEntries[index++]= cscale;
	WAVETEX_MenuEntries[index++]= coff;
	WAVETEX_MenuEntries[index++]= sunx;
	WAVETEX_MenuEntries[index++]= suny;
	WAVETEX_MenuEntries[index++]= sunz;
	WAVETEX_MenuEntries[index++]= tscale;
	WAVETEX_MenuEntries[index++]= trmin;
	WAVETEX_MenuEntries[index++]= wavetexreload;
#endif  // TARGET_GC
#endif	//#ifndef DAVID

#undef USEINFOYES
#undef USEINFOINT
#undef USEINFOFLOAT
#define USEINFOYES(item, type, lo, hi, step, init) USEINFO##type(item, lo, hi, step)
#define USEINFOINT(item, lo, hi, step)							\
	WAVETEX_MenuEntry##item =									\
	NEW MenuEntryIntEdit(#item, &WavetexDebug.##item, lo, hi);
#define USEINFOFLOAT(item, lo, hi, step)						\
	WAVETEX_MenuEntry##item =									\
	NEW MenuEntryFloatEdit(#item, &WavetexDebug.##item, lo, hi, step);
#include "wavetexdebug.txt"

#undef USEINFOYES
#define USEINFOYES(item, type, lo, hi, step, init) WAVETEX_MenuEntries[index++] = WAVETEX_MenuEntry##item;
#include "wavetexdebug.txt"

	wavetex_menu = NEW Menu(NULL, countof(WAVETEX_MenuEntries), WAVETEX_MenuEntries);

	menu_wavetex = NEW Submenu( "Wave Texture Menu", wavetex_menu );
}

