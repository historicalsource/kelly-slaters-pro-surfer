
#ifndef INCLUDED_CHALLENGE_PHOTO_H
#define INCLUDED_CHALLENGE_PHOTO_H

#include "global.h"
#include "challenge.h"
#include "floatobj.h"

class kellyslater_controller;

// PhotoChallenge: functionality for the photoshoot beach challenges.
class PhotoChallenge : public Challenge
{
private:
	enum STATE
	{
		STATE_NONE,		// waiting for surfer and cameraman to find each other
		STATE_RETICLE,	// photo triggered, waiting for reticle to fade in
		STATE_TAKE,		// rendering the picture
		STATE_SHOW,		// showing the result onscreen
	};

public:
	static const float TIME_RETICLE;
	static const float TAKE_RANGE2;		// distance to surfer squared
	static const float TIME_SPECIAL_WAIT;

private:
	// Photo: a pretty picture with a score and stuff.
	class Photo
	{
	private:
		nglTexture *	texture;
		int				score;
		bool			isOfSpecialTrick;

	public:
		// Creators.
		Photo();
		~Photo();

		// Modifiers.
		void Init(const int width, const int height);
		void Reset(void);
		void Show(const int label);
		void SetScore(const int s) { score = s; }
		void CheckProperties(kellyslater_controller * subject);

		// Accessors.
		nglTexture * GetTexture(void) { return texture; }
		int * GetScore(void) { return &score; }
		bool IsOfSpecialTrick(void) const { return isOfSpecialTrick; }
	};

	// Cameraman: dude floatin' in the water what takes 'em pics.
	class Cameraman
	{
	public:
		enum CSTATE
		{
			CSTATE_NONE,
			CSTATE_TAKING,
			CSTATE_TOOK,
		};

	private:
		kellyslater_controller *	targetKsctrl;
		entity *					ent;	// dude in the water
		Photo *						destPhoto;
		CSTATE						state;

	public:
		// Creators.
		Cameraman();

		// Modifiers.
		void Init(entity * e);
		void Reset(void);
		void Update(const float dt);
		void BeginTakingPicture(kellyslater_controller * target, Photo * photo);

		// Accessors.
		bool IsCloseToSurfer(kellyslater_controller * ksctrl) const;
		CSTATE GetState(void) const { return state; }
	};

private:
	kellyslater_controller *	ksctrl;
	int							goal;
	int							requiredScore;
	STATE						state;
	bool						recordChain;
	float						specialPhotoTimer;

	int							numCameramen;
	Cameraman *					cameramen;
	int							activeCameramanIdx;

	int							numTaken;
	int							numPhotos;
	Photo *						photos;

public:
	// Creators.
	PhotoChallenge();
	~PhotoChallenge();

	// Modifiers.
	void Init(kellyslater_controller * ks, const int g, const int reqScore, beach_object * firstObject,
		const int maxPhotos, const int photoWidth, const int photoHeight);
	void Retry(void);
	void Update(const float dt);
	void OnEvent(const EVENT event, const int param1 = 0, const int param2 = 0);
	bool CheckForCompletion(void);
#ifdef TOBY
	void Debug_TakePhoto(void);
#endif

	// Accessors.
	int GetNumTaken(void) const { return numTaken; }
	int * GetPhotoScore(const int photoIdx) const;
	nglTexture * GetPhotoTexture(const int photoIdx);
	bool GetPhotoIsOfSpecialTrick(const int photoIdx);
	bool IsTakingPhoto(void) const { return state != STATE_NONE; }
};

#endif INCLUDED_CHALLENGE_PHOTO_H
