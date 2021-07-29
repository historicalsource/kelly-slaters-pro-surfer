#ifndef _TRICKS_H
	#define _TRICKS_H

enum TrickIndicesEnum
{
	EASY_UP,
	EASY_UP_RIGHT,
	EASY_RIGHT,
	EASY_DOWN_RIGHT,
	EASY_DOWN,
	EASY_DOWN_LEFT,
	EASY_LEFT,
	EASY_UP_LEFT,

	HARD_UP,
	HARD_UP_RIGHT,
	HARD_RIGHT,
	HARD_DOWN_RIGHT,
	HARD_DOWN,
	HARD_DOWN_LEFT,
	HARD_LEFT,
	HARD_UP_LEFT,

	MAX_TRICK_TYPES,
};

typedef union TricksUnion
{
	struct
	{
		unsigned short	number 	: 3; 
		unsigned short	hard 	: 1;
	} bits;

	unsigned short		all;
} TricksType;

typedef struct TrickInfoStruct
{
	char			*name;
	unsigned long	points;
} TrickInfoType;

extern TrickInfoType trickInfo[MAX_TRICK_TYPES];

#endif //#ifndef _TRICKS_H
