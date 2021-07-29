@echo off

echo Build started: %time%

rem -=- Ensure directories
if not exist release mkdir release
if not exist release\hwosps2 mkdir release\hwosps2

rem -=- Run Wade's preprocessor chain on the assembler code -=-
m4 < hwosps2/krvu1.dsm > release/hwosps2/krvu1.pp
ee-gcc -x c++ -E release/hwosps2/krvu1.pp > release/hwosps2/krvu1.pp2
ee-dvp-as -g -I/usr/local/sce/ee/include -I/usr/local/sce/ee/gcc/ee/include -I/usr/local/sce/ee/gcc/include/g++-2 -I/sm/src/vsim -I/sm/src -o release/hwosps2/krvu1.o release/hwosps2/krvu1.pp2 > release/hwosps2/krvu1.lst

rem -=- make the iop modules -=-
rem cd hwosps2\gas_iop
rem make
rem copy *.irx ..\..\..
rem cd ..\..

if e%NUMBER_OF_PROCESSORS%==e2 goto dual_processor

:single_processor

make release BUILD=release

goto exit

:dual_processor

make -j2 release BUILD=release

goto exit

:exit

echo Build finished: %time%

rem play your 'tada' sounds if you like them in this batch file, otherwise, leave it empty
call build_done.bat