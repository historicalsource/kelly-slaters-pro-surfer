
/************************************
*      KELLYSLATERS PRO SURFER 		*
*	(C) Interactive Republic Corp	*
*									*
*		 Author:Andi Smithers		*
*		   2nd July  2000			*
*									*
*         surfersubmenu.CPP	  		*
*-----------------------------------*
* 									*
*									*
************************************/


/********** include files **********/
#include "global.h"
#pragma hdrstop

#include "irclib.h"
#include "board.h"
#include "surfer.h"
#include "tricks.h"
#include "WaveClass.h"
#include "game.h"
#include <stdio.h>

#include "script.h"

// surfer model filenames
char *surferModelFiles[SURFER_LAST][NUMBER_OF_SUITS] =
{
	{ // Kelly Slater
		// model filename(.S3D)	materials filename(.MTL)
		"runtime/ks_surfer/ks_surfer.DBF",			// trunks
		"runtime/ks_surfer/ks_surfer_full.DBF",	// wetsuit
		"runtime/ks_surfer/ks_surfer_spring.DBF",	// spring wetsuit
		"runtime/ks_surfer/ks_surfer_pers1.DBF",	// personality suit
	},

	{ // Rob Machado
		"runtime/rm_surfer/rm_surfer.DBF",			// trunks
		"runtime/rm_surfer/rm_surfer_full.DBF",	// wetsuit
		"runtime/rm_surfer/rm_surfer_spring.DBF",	// spring wetsuit
		"runtime/rm_surfer/rm_surfer_pers1.DBF",	// personality suit
	},

	{ // Kalani Robb
		"runtime/kr_surfer/kr_surfer.DBF",			// trunks
		"runtime/kr_surfer/kr_surfer_full.DBF",	// wetsuit
		"runtime/kr_surfer/kr_surfer_spring.DBF",	// spring wetsuit
		"runtime/kr_surfer/kr_surfer_pers1.DBF",	// personality suit
	},

	{ // Lisa Andersen
		"runtime/la_surfer/la_surfer.DBF",			// trunks
		"runtime/la_surfer/la_surfer_full.DBF",	// wetsuit
		"runtime/la_surfer/la_surfer_spring.DBF",	// spring wetsuit
		"runtime/la_surfer/la_surfer_pers1.DBF",	// personality suit
	},

	{ // Donavon Frankenreiter
		"runtime/df_surfer/df_surfer.DBF",			// trunks
		"runtime/df_surfer/df_surfer_full.DBF",	// wetsuit
		"runtime/df_surfer/df_surfer_spring.DBF",	// spring wetsuit
		"runtime/df_surfer/df_surfer_pers1.DBF",	// personality suit
	},

	{ // Bruce Irons
		"runtime/bi_surfer/bi_surfer.DBF",			// trunks
		"runtime/bi_surfer/bi_surfer_full.DBF",	// wetsuit
		"runtime/bi_surfer/bi_surfer_spring.DBF",	// spring wetsuit
		"runtime/bi_surfer/bi_surfer_pers1.DBF",	// personality suit
	},

	{ // Nathan Fletcher
		"runtime/nf_surfer/nf_surfer.DBF",			// trunks
		"runtime/nf_surfer/nf_surfer_full.DBF",	// wetsuit
		"runtime/nf_surfer/nf_surfer_spring.DBF",	// spring wetsuit
		"runtime/nf_surfer/nf_surfer_pers1.DBF",	// personality suit
	},

	{ // Tom Carroll
		"runtime/tc_surfer/tc_surfer.DBF",			// trunks
		"runtime/tc_surfer/tc_surfer_full.DBF",	// wetsuit
		"runtime/tc_surfer/tc_surfer_spring.DBF",	// spring wetsuit
		"runtime/tc_surfer/tc_surfer_pers1.DBF",	// personality suit
	},

	{ // Tom Curren
		"runtime/tn_surfer/tn_surfer.DBF",			// trunks
		"runtime/tn_surfer/tn_surfer_full.DBF",	// wetsuit
		"runtime/tn_surfer/tn_surfer_spring.DBF",	// spring wetsuit
		"runtime/tn_surfer/tn_surfer_pers1.DBF",	// personality suit
	},
};

// surfer model filenames
char *surferScriptFiles[SURFER_LAST] =
{
	// Kelly Slater
	"runtime/scripts/slater.IRC",

	// Rob Machado
	"runtime/scripts/machado.IRC",

	// Kalani Robb
	"runtime/scripts/slater.IRC",

	// Lisa Andersen
	"runtime/scripts/slater.IRC",

	// Donavon Frankenreiter
	"runtime/scripts/slater.IRC",

	// Bruce Irons
	"runtime/scripts/slater.IRC",

	// Nathan Fletcher
	"runtime/scripts/slater.IRC",

	// Tom Carroll
	"runtime/scripts/slater.IRC",

	// Tom Curren
	"runtime/scripts/slater.IRC",
};

#include "dmahandlerclass.h"
#include "ircrenderer.h"

//extern DmaHandlerClass testDma;
//extern ViewClass testView;



// defines
#define Fatal(C) //HACKHACK

SurferScriptObjectClass surferObject;
//MeshObjectClass boardObject;
//SurfBoardObjectClass boardObject;

//////////////////////////////////////////////////////////////////////////////////

int GetLastAnimFrame(int animId)
{
	return 0;
}

void SetAnim(GameObjectType *gop, int animId, float startFrame, float endFrame)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
