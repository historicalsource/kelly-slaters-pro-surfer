@echo off
textcvt -tm2File:ngl_sysfont.tm2 -tm2Mipmaps:1 -tm2Pixel:c8 -tm2Clut:rgb32 ngl_sysfont.tga
bin2cpp ngl_sysfont.tm2 > ngl_sysfont_tm2.i
bin2cpp ngl_sysfont.fdf > ngl_sysfont_fdf.i
sed -e "s/data\[\([0-9]*\)\]/nglSysFontTM2[\1] __attribute__((aligned(128)))/" ngl_sysfont_tm2.i > ngl_sysfont_tm2.inc
sed -e "s/data\[\([0-9]*\)\]/nglSysFontFDF[\1] __attribute__((aligned(128)))/" ngl_sysfont_fdf.i > ngl_sysfont_fdf.inc
