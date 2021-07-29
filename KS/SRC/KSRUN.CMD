@echo off
set OLDPATH=%PATH%
set PATH=%PATH%;c:\NPDP\GDEV\bin
call setcountrycode us
call setsmemsize.bat 0x03000000
setodenv "dvdroot" "C:\ks\datagc"
setodenv "DolphinRoot" "C:\DolphinSDK1.0"
odemrun -enfw ksgdevd.elf
rem call odrun KSGDevD.elf
rem -m=KSPSD.gcm
set PATH=%OLDPATH%
set OLDPATH=
