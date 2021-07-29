#ifndef __KSPS_SURFER_H_
	#define __KSPS_SURFER_H_

//	#include "gameobj.h"
//	#include "irclib.h"
//	#include "WaveClass.h"

//	#include "script.h"

//	#include "dmahandlerclass.h"
//	#include "ircrenderer.h"

	#define SURFER_RENDER_FLAGS	(S3D_OBJFLAG_ANIMATABLE | S3D_OBJFLAG_FASTRENDER)

	enum SuitsEnum
	{
		TRUNKS,
		FULL_WETSUIT,
		SPRING_WETSUIT,
		PERSONALITY1,

		NUMBER_OF_SUITS,
	};

	enum TrickPhaseEnum
	{
		ENTRY,
		HOLD,
		EXIT,

		MAX_PHASE_TYPES,
	};

	enum TurnDirEnum
	{
		LEFT_TURN,
		RIGHT_TURN,

		MAX_TURN_DIRECTIONS,
	};

	extern int GetLastAnimFrame(int animId);
	extern void SetAnim(GameObjectType *gop, int animId, float startFrame, float endFrame);
	extern void SurferObjectLoadModel(AnimObjectClass *gop);
	extern void SurferObjectInit(void);
	extern void SurferObjectUpdate(AnimObjectClass *gop);
	extern void SurferObjectAnimUpdate(AnimObjectClass *gop);
	extern void SurferObjectDraw(AnimObjectClass *gop);

	extern char *surferFileNames[];

	extern SurferScriptObjectClass surferObject;
//	extern MeshObjectClass boardObject;
//#include "board.h"
//	extern SurfBoardObjectClass boardObject;

	extern char *surferModelFiles[SURFER_LAST][NUMBER_OF_SUITS];
	extern char *surferScriptFiles[SURFER_LAST];

#endif //#ifndef _SURFER_H

