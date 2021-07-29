@echo off

if %1e == e goto error

copy %1\ngl_*.cpp %1\_ngl_*.cpp
copy %1\ngl_*.h   %1\_ngl_*.h
copy %1\ngl_*.dsm %1\_ngl_*.dsm

copy \ngl\working\ps2\ngl_*.cpp %1\
copy \ngl\working\ps2\ngl_*.h   %1\
copy \ngl\working\ps2\ngl_*.dsm %1\

copy \ngl\working\common\pstring.cpp %1\
copy \ngl\working\common\pstring.h %1\

copy \ngl\working\common\instbank.cpp %1\
copy \ngl\working\common\instbank.h %1\

copy \ngl\working\common\ngl.h %1\

goto exit

:error
echo To copy NGL to a new folder:
echo nglcpy.bat [dest dir] (no trailing backslash)

:exit
