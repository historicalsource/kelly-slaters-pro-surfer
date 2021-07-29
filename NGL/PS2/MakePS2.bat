@echo off
set oldpath=%path%
set Path=C:\usr\local\sce\ee\gcc\bin;c:\usr\local\snsys;c:\ngl\working\tools\bin;C:\Program Files\ProDG for Playstation2\

cd\ngl\working\ps2

set BUILD=%1
if "%BUILD%" == "" set BUILD=debug

make -fngl_ps2.mk BUILD=%BUILD% %2

set path=%oldpath%
set oldpath=
