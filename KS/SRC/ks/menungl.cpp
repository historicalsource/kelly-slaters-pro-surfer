// With precompiled headers enabled, all text up to and including
// the following line are ignored by the compiler (dc 01/18/02)
#include "global.h"

#if defined(TARGET_XBOX) || defined(TARGET_GC)
#include "ngl.h"
#include "inputmgr.h"
#else
#include "ngl_ps2.h"
#endif /* TARGET_XBOX JIV DEBUG */
#include "menusys.h"
#include "menu.h"

#ifdef TARGET_XBOX
const float menufontscale=1.5f;
#else
const float menufontscale=1.f;
#endif

const int menutextsep=3;

/*
static float I2FColor( unsigned char c )
{
	return ((float) c)/256.0f;
}
*/

static unsigned int I2XColor( const MenuColor &color )
{
	//return (((128*color.a)/255)<<24)+(color.b<<16)+(color.g<<8)+(color.r);
	#ifdef TARGET_PS2
	return (((128*color.a)/255)<<0)+(color.b<<8)+(color.g<<16)+(color.r<<24);
	#else
	return (color.a<<0)+(color.b<<8)+(color.g<<16)+(color.r<<24);
	#endif
}

void MENU_InitMenus( void )
{
}

void MENU_TermMenus( void )
{
}

void MENU_DrawRect( int x0, int y0, int x1, int y1, const MenuColor &color )
{
	nglQuad q;
	nglInitQuad( &q );
	nglSetQuadRect( &q, x0, y0, x1, y1 );
	nglSetQuadColor(&q, NGL_RGBA32(color.r, color.g, color.b, color.a));
	nglSetQuadZ( &q, 1.0f );
	nglListAddQuad( &q );
}

void MENU_ClearRect( int x0, int y0, int x1, int y1 )
{
}

void MENU_DrawTextAt( int x, int y, const char *text, const MenuColor &color )
{
	nglListAddString( nglSysFont, x, y, 0, I2XColor(color), text );
}

void MENU_DrawStart( void )
{
}

void MENU_DrawEnd( void )
{
}

int MENU_TextHeight( const char *text )
{
	return (int) (10*menufontscale);
}

int MENU_TextWidth( const char *text )
{
	return (int) ((10*menufontscale + menutextsep)*strlen(text)-(strlen(text)?menutextsep:0));
}

int MENU_BorderSizeFactor( void )
{
	return 2;
}

int MENU_BorderSpaceFactor( void )
{
	return 10;
}

int MENU_TextSeparation( void )
{
	return menutextsep;
}

#if 0
struct mJoypadState
{
  u_int Start, Select;
  u_int Square, Triangle, X, Circle;
  u_int Up, Left, Right, Down;
  u_int L1, L2, R1, R2;
  u_int StickX, StickY;
  u_int RStickX, RStickY;
};

extern mJoypadState mJoypads[2];
#endif

int MENU_GetButtonPressed( int button )
{
	input_mgr* inputmgr = input_mgr::inst();
	if( inputmgr->get_control_state(ANY_LOCAL_JOYSTICK, button ) == AXIS_MAX )
		return 1;

	return 0;
}

int MENU_GetAnalogState( int button )
{
	input_mgr* inputmgr = input_mgr::inst();
	rational_t state = inputmgr->get_control_state(ANY_LOCAL_JOYSTICK, button );
	if( state > AXIS_MID )
		return 1;
	else if (state < AXIS_MID)
		return -1;

	return 0;
}

int MENU_GetButtonState( int buttoninfo )
{
	#if 0
	return 0;
	//assert(0);
	#else
	switch ( buttoninfo )
	{
		case MENUCMD_NONE    : return 0;
		case MENUCMD_SELECT  : return MENU_GetButtonPressed(PSX_SELECT);
		case MENUCMD_START   : return MENU_GetButtonPressed(PSX_START);
		case MENUCMD_UP      : return (MENU_GetAnalogState(PSX_UD) == -1)?1:0;
		case MENUCMD_DOWN    : return (MENU_GetAnalogState(PSX_UD) == 1)?1:0;
		case MENUCMD_LEFT    : return (MENU_GetAnalogState(PSX_LR) == -1)?1:0;
		case MENUCMD_RIGHT   : return (MENU_GetAnalogState(PSX_LR) == 1)?1:0;
		case MENUCMD_CROSS   : return MENU_GetButtonPressed(PSX_X);
		case MENUCMD_TRIANGLE: return MENU_GetButtonPressed(PSX_TRIANGLE);
		case MENUCMD_SQUARE  : return MENU_GetButtonPressed(PSX_SQUARE);
		case MENUCMD_CIRCLE  : return MENU_GetButtonPressed(PSX_CIRCLE);
		case MENUCMD_L1      : return MENU_GetButtonPressed(PSX_L1);
		case MENUCMD_R1      : return MENU_GetButtonPressed(PSX_R1);
		#ifdef TARGET_GC
		case MENUCMD_L2      : return 0;
		#else
		case MENUCMD_L2      : return MENU_GetButtonPressed(PSX_L2);
		#endif
		case MENUCMD_R2      : return MENU_GetButtonPressed(PSX_R2);
	}
	#endif
	return 0;
}

MenuCommand MENU_TranslateButton( int buttoninfo )
{
	return (MenuCommand) buttoninfo;
}






















