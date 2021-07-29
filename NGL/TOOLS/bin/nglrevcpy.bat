@echo off

if %1e == e goto error

copy \ngl\working\ps2\ngl_ps2.cpp		\ngl\working\ps2\_ngl_ps2.cpp
copy \ngl\working\ps2\ngl_ps2.h			\ngl\working\ps2\_ngl_ps2.h	
copy \ngl\working\ps2\ngl_vu1.dsm		\ngl\working\ps2\_ngl_vu1.dsm
copy %1\ngl_ps2.cpp		\ngl\working\ps2\
copy %1\ngl_ps2.h			\ngl\working\ps2\
copy %1\ngl_vu1.dsm		\ngl\working\ps2\

goto exit

:error
echo To copy NGL from a new folder:
echo nglrevcpy.bat [src dir] (no trailing backslash)

:exit
