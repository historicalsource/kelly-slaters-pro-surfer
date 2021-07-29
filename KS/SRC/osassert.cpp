#include "global.h"

#include "osassert.h"
#include "game_info.h"	// For g_debug (dc 06/10/02)

#define assert_msg(exp)

#ifdef TARGET_PS2
  #undef assert
  #define assert(exp) ((void)0)
#endif


/*	Not currently used.  (dc 06/10/02)
#if !defined(BUILD_FINAL)
	#if defined(TARGET_PC)
		void _internal_assert ( const char *exp, const char *file, int line_no )
		{
		  _assert( (void*)( error_context::is_inst() ?
		                    (error_context::inst()->get_context()+"\n"+stringx(exp)).c_str() :
		                    exp ) ,
		          (void*)file, line_no);
		}
	#else
		bool _internal_assert ( const char *exp, const char *file, int line_no )
		{
		  if( error_context::is_inst() )
		  {
		    char errstring[256];
		    sprintf( errstring, "%.195s\n%.63s", error_context::inst()->get_context().c_str(), exp );
				#if defined(TARGET_XBOX) || defined(TARGET_GC)
		    	// JIV FIXME
		    	_assert( errstring, file, line_no );
		    	return true;
				#else
		    	return _assert( errstring, file, line_no );
				#endif // TARGET_XBOX JIV DEBUG
		  }
		  else
		  {
				#if defined(TARGET_XBOX) || defined(TARGET_GC)
		    	// JIV FIXME
		    	_assert( exp, file, line_no);
		    	return true;
				#else
			    return _assert( exp, file, line_no);
				#endif // TARGET_XBOX JIV DEBUG
		  }
		}
	#endif
#endif
*/

// Defined in this file so that "countof" macro works properly.  (dc 06/10/02)
const char *assert_helptext[] = {
#ifdef TARGET_PS2
	"L1+R1+CROSS    -- Ignore.", 
	"L1+R1+CIRCLE   -- Disable this assertion.", 
	"L1+R1+TRIANGLE -- Disable all assertions.", 
	"L1+R1+SQUARE   -- Halt execution.", 
#elif defined(TARGET_XBOX)
	"L+R+A -- Ignore.", 
	"L+R+B -- Disable this assertion.", 
	"L+R+Y -- Disable all assertions.", 
	"L+R+X   -- Halt execution.", 
	"L+R+Black -- Reboot to dashboard.", 
#elif defined(TARGET_GC)
	"L + R + A   -- Ignore.", 
	"L + R + X   -- Disable this assertion.", 
	"L + R + Y   -- Disable all assertions.", 
	"L + R + B   -- Halt execution.", 
#endif
};

inline void halt_execution(void)
{
#ifdef TARGET_PS2
	asm ("break 1");
#elif defined(TARGET_XBOX)
	__asm { int 3 }
#elif defined(TARGET_GC)
	asm ( trap );
#endif
}

#ifdef TARGET_PS2
#define ASSERT_RGBA32(r, g, b, a) NGL_RGBA32((2*(r)), (2*(g)), (2*(b)), (a))
#else
#define ASSERT_RGBA32 NGL_RGBA32
#endif

// platform independent units
#define ASSERT_SCREENWIDTH (640)
#define ASSERT_SCREENHEIGHT (480)

void render_to_screen( const char* string, const char* file, int line_no )
{
	int screen_left = 0, screen_right = ASSERT_SCREENWIDTH, screen_top = 0, screen_bottom = ASSERT_SCREENHEIGHT;
	adjustCoords(screen_left, screen_top);
	adjustCoords(screen_right, screen_bottom);

	static int assert_box_margin = 20;
	static int assert_text_margin = 5;
	static u_int assert_box_color = ASSERT_RGBA32(0, 0, 0, 128);
	static u_int assert_text_color = ASSERT_RGBA32(255, 255, 255, 255);
	static u_int assert_helpbox_color = ASSERT_RGBA32(0, 0, 128, 128);
	static u_int assert_helptext_color = ASSERT_RGBA32(255, 255, 0, 255);
	static float assert_scale_x = 1.f;
	static float assert_scale_y = 1.f;

	nglQuad q;
	int left, right, top, bottom;
	int line_x, line_y;
	u_int lineheight;
	u_int textwidth, textheight;
	u_int textlength, textlines;
	u_int linelength;
	char line[256];

	static char assert_scale_token[64];
	sprintf(assert_scale_token, NGLFONT_TOKEN_SCALEXY "[%f, %f]", assert_scale_x, assert_scale_y); 

	u_int assert_X_width, assert_X_height;
	nglGetStringDimensions(nglSysFont, &assert_X_width, &assert_X_height, "%sX", assert_scale_token);
	lineheight = assert_X_height + assert_text_margin;

	nglListInit ();

/*	Replaced by new API. (dc 05/30/02)
	KSNGL_SetFont (0);
	KSNGL_SetFontZ (0.2f);
	KSNGL_SetFontScale (1, 1);
*/
	float FontZ = 0.2f;

	left = screen_left + assert_box_margin;
	right = screen_right - assert_box_margin;

	nglGetStringDimensions(nglSysFont, &textwidth, &textheight, string);
	textlength = strlen(string);
	textlines = textwidth / (right - left) + 1;
	linelength = textlength / textlines;

	top = screen_top + assert_box_margin;
	bottom = top + (2 + textlines) * lineheight + 2 * assert_text_margin;

	nglInitQuad (&q);
	nglSetQuadRect (&q, left, top, right, bottom);
	nglSetQuadColor (&q, assert_box_color);
	nglSetQuadZ (&q, 0.3f);
	nglListAddQuad (&q);

	line_x = left + assert_text_margin;
	line_y = top + assert_text_margin;

/*	Replaced by new API. (dc 05/30/02)
	KSNGL_SetFontColor (assert_text_color);
*/

	nglListAddString (nglSysFont, line_x, line_y, 0, assert_text_color, "%sAssertion failed!", assert_scale_token);
	line_y += lineheight;
	nglListAddString (nglSysFont, line_x, line_y, 0, assert_text_color, "%sFile %s, line %d", assert_scale_token, file, line_no);
	line_y += lineheight;
	for (u_int linenum = 0; linenum < textlines; ++linenum)
	{
		strcpy(line, "%s");
		strncpy(line + sizeof("%s") - 1, string + linenum * linelength, linelength);
		line[sizeof("%s") + linelength - 1] = '\0';
		nglListAddString (nglSysFont, line_x, line_y, 0, assert_text_color, line, assert_scale_token);
		line_y += lineheight;
	}

	left = screen_left + assert_box_margin;
	right = screen_right - assert_box_margin;
	bottom = screen_bottom - assert_box_margin;
	top = bottom - countof(assert_helptext) * lineheight - 2 * assert_text_margin;

	nglInitQuad (&q);
	nglSetQuadRect (&q, left, top, right, bottom);
	nglSetQuadColor (&q, assert_helpbox_color);
	nglSetQuadZ (&q, 0.3f);
	nglListAddQuad (&q);

	line_x = left + assert_text_margin;
	line_y = top + assert_text_margin;

/*	Replaced by new API. (dc 05/30/02)
	KSNGL_SetFontColor (assert_helptext_color);
*/

	for (u_int linenum = 0; linenum < countof(assert_helptext); ++linenum)
	{
		nglListAddString (nglSysFont, line_x, line_y, FontZ, assert_helptext_color, "%s%s", 
			assert_scale_token, assert_helptext[linenum]);
		line_y += lineheight;
	}

	nglListSend(true);
	nglFlip();	// send what we just rendered to the front buffer
}
static bool inAssert = false;
bool _assert( const char* string, const char* file, int line_no )
{
	if (inAssert)
		return true;
  STOP_PS2_PC;
	nglPrintf( "Assertion failed, file %s line %d: %s\n", file, line_no, string );
	inAssert = true;
	if (g_debug.halt_on_asserts)
	{

		if (g_debug.assert_screen && input_mgr::is_inst())
		{
			input_mgr* inputmgr = input_mgr::inst();
			const device_id_t default_joystick = JOYSTICK_TO_DEVICE_ID(inputmgr->GetDefaultController() + 1);

#if defined(TARGET_PS2) && defined(DEBUG)
			string = get_backtrace(string);
#endif

			render_to_screen(string, file, line_no);
			while (1)
			{
				inputmgr->poll_devices();

				if ((inputmgr->get_control_state (default_joystick, PSX_L1) == AXIS_MAX) &&
					(inputmgr->get_control_state (default_joystick, PSX_R1) == AXIS_MAX))
				{
					if (inputmgr->get_control_state (default_joystick, PSX_X) == AXIS_MAX)
					{
            START_PS2_PC;
						inAssert = false;
						return true;
					}
					else if (inputmgr->get_control_state (default_joystick, PSX_CIRCLE) == AXIS_MAX)
					{
            START_PS2_PC;
						inAssert = false;
						return false;
					}
					else if (inputmgr->get_control_state (default_joystick, PSX_TRIANGLE) == AXIS_MAX)
					{
						g_debug.halt_on_asserts = false;
            START_PS2_PC;
						inAssert = false;
						return true;
					}
					else if (inputmgr->get_control_state (default_joystick, PSX_SQUARE) == AXIS_MAX)
					{
						halt_execution();
					}
#ifdef TARGET_XBOX
					else if (inputmgr->get_control_state (default_joystick, PSX_L2) == AXIS_MAX)
					{
						XLaunchNewImage(NULL, NULL);
					}
#endif
				}
			}
		}
		else
		{
			halt_execution();
		}
	}
  START_PS2_PC;
	inAssert = false;
	return true;
}

void official_error(const char* fmtp, ...)
{
	va_list vlist;
	va_start(vlist, fmtp);
	char fmtbuff[2048];
	vsprintf(fmtbuff, fmtp, vlist);

	nglListInit ();

	int screen_left = 0, screen_right = ASSERT_SCREENWIDTH, screen_top = 0, screen_bottom = ASSERT_SCREENHEIGHT;
	adjustCoords(screen_left, screen_top);
	adjustCoords(screen_right, screen_bottom);
	int error_box_margin = 20;
	float box_left = screen_left + error_box_margin;
	float box_right = screen_right - error_box_margin;
	float box_top = screen_top + error_box_margin;
	float box_bottom = screen_bottom - error_box_margin;
	u_int error_box_color = ASSERT_RGBA32(64, 0, 0, 255);

	nglQuad q;
	nglInitQuad (&q);
	nglSetQuadRect (&q, box_left, box_top, box_right, box_bottom);
	nglSetQuadColor (&q, error_box_color);
	nglSetQuadZ (&q, 0.3f);
	nglListAddQuad (&q);

	float error_scale_x = 1.5f;
	float error_scale_y = 1.5f;
	char error_scale_token[64];
	sprintf(error_scale_token, NGLFONT_TOKEN_SCALEXY "[%f, %f]", error_scale_x, error_scale_y); 
	char message[2048];
	sprintf(message, "%s%s", error_scale_token, fmtbuff);
	u_int error_text_color = ASSERT_RGBA32(0, 255, 255, 255);
	int error_text_margin = 20;
	float error_text_x = box_left + error_text_margin;
	float error_text_y = box_top + error_text_margin;

	nglListAddString (nglSysFont, error_text_x, error_text_y, 0, error_text_color, message);

	nglListSend(true);
	nglFlip();	// send what we just rendered to the front buffer

	// Wait for the medics
	for(;;);
}
