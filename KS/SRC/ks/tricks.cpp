#include "global.h"
//#pragma hdrstop

#include "tricks.h"
								 // 
TrickInfoType trickInfo[MAX_TRICK_TYPES] =
{
	// easy tricks
	// trick name					trick points
	{"one handed rail grab",		250},	//EASY_UP
	{"deadly one handed rail grab",	750},	//EASY_UP_RIGHT
	{"rocket air",					500},	//EASY_RIGHT
	{"killer one handed tail grab",	1000},	//EASY_DOWN_RIGHT
	{"one handed tail grab",		250},	//EASY_DOWN
	{"killer one handed tail grab",	1000},	//EASY_DOWN_LEFT
	{"crazy one handed tail grab",	500},	//EASY_LEFT
	{"deadly one handed rail grab",	750},	//EASY_UP_LEFT

	// hard tricks
	// trick name					trick points
	{"two handed rail grab",		300},	//HARD_UP
	{"deadly two handed rail grab",	900},	//HARD_UP_RIGHT
	{"crazy two handed rail grab",	600},	//HARD_RIGHT
	{"killer judo chop",   			1200},	//HARD_DOWN_RIGHT
	{"judo chop",					300},	//HARD_DOWN
	{"killer judo chop",			1200},	//HARD_DOWN_LEFT
	{"crazy judo chop",				600},	//HARD_LEFT
	{"deadly two handed rail grab",	900},	//HARD_UP_LEFT

};

char *trickDirs[MAX_TRICK_TYPES] =
{
	"U",
	"UR",
	"R",
	"DR",
	"D",
	"DL",
	"L",
	"UL",

	"U",
	"UR",
	"R",
	"DR",
	"D",
	"DL",
	"L",
	"UL",

};

