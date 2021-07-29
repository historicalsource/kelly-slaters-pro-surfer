set oldcd=%cd%

echo Copying executable file...
copy \KS\BIN\KellySlaterPS2_bootable.elf \KS\DISKIMGPS2\SLUS_203.34 >nul
if errorlevel 1 goto exit

echo Zipping up source files...
set ZIPEXE="\Program Files\pkzip\pkzip25.exe"
set ZIPOPT=-add -rec -path=root -pass=skag -excl=*.idb -excl=*.lib -excl=*.ncb -excl=*.obj -excl=*.plg -excl=*.scc
set ZIPFILE=\KS\DISKIMGPS2\src.zip
if exist %ZIPFILE% del /Q %ZIPFILE% >nul
%ZIPEXE% %ZIPOPT% %ZIPFILE% \KS\SRC\*.* \NGL\*.* \NSL\*.* \NVL\*.* >nul
if errorlevel 1 goto exit

:exit
cd %oldcd%
set oldcd=
