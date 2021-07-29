/* This file should only contain common stuff to NSL that
 * the PS2's IOP C compiler can also compile.  For any
 * other common functions/etc. use nsl_common.cpp
 */


const char *nslLanguageStr[] = // keep this array parallel with nslLanguageEnum in nsl.h
{
	"NONE",

  // The "Big 4" european languages
  "ENGLISH",
  "FRENCH",
  "GERMAN", 
  "SPANISH",

  // Asian languages
  "JAPANESE",
  "CANTONESE",
  "MANDARIN",

  // Phony languages used for Spider-Man's cheats.
  // Why are they in general NSL?  it's the only way we could
  // get it to work and it doesn't hurt any other projects adversely...
  "GGENG",
  "GGFRE",
  "GGGER",
  "GGSPA",

	"N/A"
};

const char *nslPlatformStr[] = // keep this array parallel with nslPlatformEnum in nsl.h
{
  "PS2",
  "XBOX",
  "GAMECUBE",
  "PC",

	"N/A"
};

const char *nslSourceTypesStr[] = // keep this array parallel with nslSourceTypeEnum in nsl.h
{
  "SFX",
  "AMBIENT",
  "MUSIC",
  "VOICE",
  "MOVIE",
  "USER1",
  "USER2",
  "N/A"
};
