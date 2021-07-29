@echo off

echo Build started: %time%

rem -=- Ensure directories
if not exist debug mkdir debug
if not exist debug\hwosps2 mkdir debug\hwosps2

rem -=- Run Wade's preprocessor chain on the assembler code -=-
m4 < hwosps2/krvu1.dsm > debug/hwosps2/krvu1.pp
ee-gcc -x c++ -E debug/hwosps2/krvu1.pp > debug/hwosps2/krvu1.pp2
ee-dvp-as -g -I/usr/local/sce/ee/include -I/usr/local/sce/ee/gcc/ee/include -I/usr/local/sce/ee/gcc/include/g++-2 -I/sm/src/vsim -I/sm/src -o debug/hwosps2/krvu1.o debug/hwosps2/krvu1.pp2 > debug/hwosps2/krvu1.lst

rem -=- make the iop modules -=-
rem cd hwosps2\gas_iop
rem make
rem copy *.irx ..\..\..
rem cd ..\..

if e%NUMBER_OF_PROCESSORS%==e2 goto dual_processor

:single_processor

make debug BUILD=debug

goto exit

:dual_processor

make -j2 debug BUILD=debug

goto exit

:exit

echo Build finished: %time%

rem play your 'tada' sounds if you like them in this batch file, otherwise, leave it empty
call build_done.bat
