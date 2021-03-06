/*  THIS FILE IS AUTOMATICALLY GENERATED BY EXPORT FROM THE EXCEL FILE:  BEACHDATA.XLS
    ALL CHANGES SHOULD BE MADE IN EXCEL AND RE_EXPORTED.  DO NOT HAND EDIT.
    David Cook, Treyarch, 6/25/01
*/

#include "global.h"
#include "kshooks.h"
#include "beachdata.h"

BeachData BeachDataArray[25];
void BEACHDATA_Load(void)
{
    nglFileBuf F;
    KSReadFile("beachdata.dat", &F, 1);
    char *s = (char *) F.Buf;
    const char *format = "%[^,], %[^,], %[^,], %[^,], %[^,], %[^,], %[^,], %d, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %d, %f, %f, %f, %d, %f, %f, %f, %f, %d, %d, %d, %d, %d, %d, %f, %x, %x, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %x, %f, %f, %f, %f, %f, %f, %x, %x, %f, %f, %f, %f, %f, %f, %x, %f, %f, %f, %f, %f, %d, %x, %f, %f, %f, ";
    for (int i = 0; i < 25; ++i)
    {
         int n = sscanf(s, format,
              &BeachDataArray[i].name,
              &BeachDataArray[i].folder,
              &BeachDataArray[i].stashfile,
              &BeachDataArray[i].fe_name,
              &BeachDataArray[i].career_name,
              &BeachDataArray[i].bottomname,
              &BeachDataArray[i].capsulename,
              &BeachDataArray[i].bdir,
              &BeachDataArray[i].start_vx,
              &BeachDataArray[i].start_vz,
              &BeachDataArray[i].water_friction,
              &BeachDataArray[i].skag_friction,
              &BeachDataArray[i].stick_force,
              &BeachDataArray[i].gravity,
              &BeachDataArray[i].board_speed,
              &BeachDataArray[i].board_carving,
              &BeachDataArray[i].zoom_factor,
              &BeachDataArray[i].cutoff_freq,
              &BeachDataArray[i].break_zoom,
              &BeachDataArray[i].break_mod,
              &BeachDataArray[i].darkscale,
              &BeachDataArray[i].darkoff,
              &BeachDataArray[i].darkhiscale,
              &BeachDataArray[i].darkhioff,
              &BeachDataArray[i].darkunderscale,
              &BeachDataArray[i].darkunderoff,
              &BeachDataArray[i].highscale,
              &BeachDataArray[i].highoff,
              &BeachDataArray[i].corescale,
              &BeachDataArray[i].coreoff,
              &BeachDataArray[i].transcale,
              &BeachDataArray[i].transmin,
              &BeachDataArray[i].sunx,
              &BeachDataArray[i].suny,
              &BeachDataArray[i].sunz,
              &BeachDataArray[i].foamintube,
              &BeachDataArray[i].lensflarex,
              &BeachDataArray[i].lensflarey,
              &BeachDataArray[i].lensflarez,
              &BeachDataArray[i].level_duration,
              &BeachDataArray[i].is_small_wave,
              &BeachDataArray[i].is_big_wave,
              &BeachDataArray[i].use_wetsuit,
              &BeachDataArray[i].map_location,
              &BeachDataArray[i].framelock,
              &BeachDataArray[i].has_movie,
              &BeachDataArray[i].bumpscale,
              &BeachDataArray[i].colornear,
              &BeachDataArray[i].colorfar,
              &BeachDataArray[i].rgbfadescale,
              &BeachDataArray[i].rgbfadeoffset,
              &BeachDataArray[i].rgbfademin,
              &BeachDataArray[i].rgbfademax,
              &BeachDataArray[i].bumpfadescale,
              &BeachDataArray[i].bumpfadeoffset,
              &BeachDataArray[i].bumpfademin,
              &BeachDataArray[i].bumpfademax,
              &BeachDataArray[i].alphafadescale,
              &BeachDataArray[i].alphafadeoffset,
              &BeachDataArray[i].alphafademin,
              &BeachDataArray[i].alphafademax,
              &BeachDataArray[i].colorspecular,
              &BeachDataArray[i].specularfadescale,
              &BeachDataArray[i].specularfadeoffset,
              &BeachDataArray[i].sunx_xbox,
              &BeachDataArray[i].suny_xbox,
              &BeachDataArray[i].sunz_xbox,
              &BeachDataArray[i].gcbumpscale,
              &BeachDataArray[i].gccolornear,
              &BeachDataArray[i].gccolorfar,
              &BeachDataArray[i].gcrgbfadescale,
              &BeachDataArray[i].gcrgbfadeoffset,
              &BeachDataArray[i].gcbumpfadescale,
              &BeachDataArray[i].gcbumpfadeoffset,
              &BeachDataArray[i].gcalphafadescale,
              &BeachDataArray[i].gcalphafadeoffset,
              &BeachDataArray[i].gccolorspecular,
              &BeachDataArray[i].gcspecularfadescale,
              &BeachDataArray[i].gcspecularfadeoffset,
              &BeachDataArray[i].sunx_gc,
              &BeachDataArray[i].suny_gc,
              &BeachDataArray[i].sunz_gc,
              &BeachDataArray[i].has_beach_board,
              &BeachDataArray[i].underwaterambient,
              &BeachDataArray[i].ambient_boost,
              &BeachDataArray[i].underwater_boost,
              &BeachDataArray[i].dir_reduction
         );
         assert(87 == n);
         verify((s = strchr(s, '\n')) != NULL);
         ++s;
    }
    KSReleaseFile(&F);
}
