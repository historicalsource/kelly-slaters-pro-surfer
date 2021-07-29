# Microsoft Developer Studio Project File - Name="KellySlater" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101
# TARGTYPE "Xbox Application" 0x0b01

CFG=KellySlater - Win32 PS2 EE Final
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "KellySlater.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "KellySlater.mak" CFG="KellySlater - Win32 PS2 EE Final"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "KellySlater - Xbox Debug" (based on "Xbox Application")
!MESSAGE "KellySlater - Xbox Bootable" (based on "Xbox Application")
!MESSAGE "KellySlater - Win32 PS2 EE Debug" (based on "Win32 (x86) Application")
!MESSAGE "KellySlater - Win32 PS2 EE Bootable" (based on "Win32 (x86) Application")
!MESSAGE "KellySlater - Win32 PS2 EE Final" (based on "Win32 (x86) Application")
!MESSAGE "KellySlater - Xbox Final" (based on "Xbox Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/ks/src", JQFAAAAA"
# PROP Scc_LocalPath "."

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\bin"
# PROP Intermediate_Dir "Xbox_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
MTL=midl.exe
RSC=rc.exe
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_XBOX" /D "_DEBUG" /YX /FD /G6 /Zvc6 /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /I "\ks\src" /I "\ks\src\ks" /I "\ks\src\mpeg" /I "..\..\ngl\xbox" /I "..\..\nvl\xbox" /I "..\..\nsl" /D "WIN32" /D "_XBOX" /D "_DEBUG" /D "ARCH_ENGINE" /D "TARGET_XBOX" /D "BUILD_DEBUG" /Yu"global.h" /FD /G6 /Ztmp /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 xapilibd.lib d3d8d.lib d3dx8d.lib xgraphicsd.lib dsoundd.lib dmusicd.lib xnetd.lib xboxkrnl.lib /nologo /incremental:no /debug /machine:I386 /subsystem:xbox /fixed:no /debugtype:vc6
# ADD LINK32 binkxbox.lib xapilibd.lib d3d8d.lib d3dx8d.lib xgraphicsd.lib dsoundd.lib dmusicd.lib xnetd.lib wmvdec.lib xboxkrnl.lib xbdm.lib ngl_xbox.lib nsl_xbox.lib /nologo /incremental:no /debug /machine:I386 /nodefaultlib:"libc" /nodefaultlib:"libcmt" /out:"..\bin/KellySlaterXB_Debug.exe" /libpath:"\ngl\xbox\Xbox_Debug" /libpath:"\nsl\xbox\Xbox_Debug" /libpath:"\nvl\xbox\Xbox_Debug" /subsystem:xbox /fixed:no /TMP
# SUBTRACT LINK32 /pdb:none /map
XBE=imagebld.exe
# ADD BASE XBE /nologo /stack:0x10000 /debug
# ADD XBE /nologo /testid:"41560002" /testname:"Kelly Slater's Pro Surfer" /stack:0x20000 /initflags:0x0 /debug /out:"..\bin/KellySlaterXB_Debug.xbe" /titleimage:"ks\titleimage.xbx" /testratings:4 /testregion:0x1 /defaultsaveimage:"ks\saveimage.xbx"
# SUBTRACT XBE /limitmem
XBCP=xbecopy.exe
# ADD BASE XBCP /NOLOGO
# ADD XBCP /NOLOGO

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "KellySlater___Xbox_Bootable"
# PROP BASE Intermediate_Dir "KellySlater___Xbox_Bootable"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\bin"
# PROP Intermediate_Dir "Xbox_Bootable"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
MTL=midl.exe
RSC=rc.exe
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /I "\ks\src" /I "\ks\src\ks" /I "\ks\src\sgi_stl" /I "\ks\src\mpeg" /I "..\..\ngl\xbox" /I "..\..\nsl\working" /I "..\..\nvl\working\xbox" /D "WIN32" /D "_XBOX" /D "NDEBUG" /D "ARCH_ENGINE" /D "TARGET_XBOX" /Yu"global.h" /FD /G6 /Zvc6 /c
# ADD CPP /nologo /W3 /GX /Zi /O2 /I "\ks\src" /I "\ks\src\ks" /I "\ks\src\mpeg" /I "..\..\ngl\xbox" /I "..\..\nvl\xbox" /I "..\..\nsl" /D "WIN32" /D "_XBOX" /D "NDEBUG" /D "ARCH_ENGINE" /D "TARGET_XBOX" /D "BUILD_BOOTABLE" /Yu"global.h" /FD /G6 /Ztmp /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 xapilib.lib d3d8.lib d3dx8.lib xgraphics.lib dsound.lib dmusic.lib xnet.lib xboxkrnl.lib ngl_xbox.lib nsl.lib mpeg.lib /nologo /machine:I386 /libpath:"\ngl\xbox" /libpath:"\nsl\working\xbox" /libpath:"\ks\src\mpeg" /subsystem:xbox /fixed:no /debugtype:vc6 /OPT:REF
# ADD LINK32 binkxbox.lib xapilib.lib d3d8.lib d3dx8.lib xgraphics.lib dsound.lib dmusic.lib xnet.lib wmvdec.lib xboxkrnl.lib xbdm.lib ngl_xbox.lib nsl_xbox.lib nvl_xbox.lib /nologo /debug /machine:I386 /nodefaultlib:"libc" /out:"..\bin/KellySlaterXB_Bootable.exe" /libpath:"\ngl\xbox\Xbox_Bootable" /libpath:"\nsl\xbox\Xbox_Bootable" /libpath:"\nvl\xbox\Xbox_Bootable" /subsystem:xbox /fixed:no /TMP /OPT:REF /OPT:ICF
# SUBTRACT LINK32 /pdb:none
XBE=imagebld.exe
# ADD BASE XBE /nologo /stack:0x10000
# ADD XBE /nologo /testid:"41560002" /testname:"Kelly Slater's Pro Surfer" /stack:0x20000 /debug /out:"..\bin/KellySlaterXB_Bootable.xbe" /titleimage:"ks\titleimage.xbx" /testratings:4 /testregion:0x1 /defaultsaveimage:"ks\saveimage.xbx"
# SUBTRACT XBE /limitmem
XBCP=xbecopy.exe
# ADD BASE XBCP /NOLOGO
# ADD XBCP /NOLOGO
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=if exist userbuildxb.bat userbuildxb.bat
# End Special Build Tool

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "PS2_EE_Debug"
# PROP BASE Intermediate_Dir "PS2_EE_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../bin"
# PROP Intermediate_Dir "PS2_EE_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
CPP=snCl.exe
# ADD BASE CPP /nologo /W4 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W4 /Od /I "..\..\nsl\common\\" /I "..\..\nvl\ps2" /I "ks" /I "..\..\ngl\ps2" /I "..\..\ngl\common" /I "..\..\nsl" /D "DEBUG" /D "BUILD_DEBUG" /D "SN_TARGET_PS2" /D "__PS2_EE__" /D "ARCH_ENGINE" /D "PROJECT_PS2" /D "PS2" /debug -G0 -mvu0-use-vf0-vf31 /c
MTL=midl.exe
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=snLink.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 nvl_ps2.lib libsdr.a nsl.lib libmc.a libz_ps2.lib ngl_ps2.lib libstdc++.a libsn.a libgraph.a libdma.a libdev.a libpad.a libpkt.a libvu0.a libcdvd.a libmpeg.a libipu.a libpc.a sntty.lib dmadebug.a libscf.a app.cmd /verbose /pdb:none /debug /machine:IX86 /out:"..\bin\KellySlaterPS2_debug.elf" /libpath:"..\..\ngl\ps2\PS2_EE_Debug" /libpath:"zlib\PS2_EE_Debug" /libpath:"..\..\nsl\ps2\ps2_EE_DEBUG" /libpath:"..\..\nvl\ps2\ps2_EE_DEBUG" /D:SN_TARGET_PS2 -strip-unused -collect2

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "PS2_EE_Bootable"
# PROP BASE Intermediate_Dir "PS2_EE_Bootable"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\bin"
# PROP Intermediate_Dir "PS2_EE_Bootable"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
CPP=snCl.exe
# ADD BASE CPP /nologo /W4 /Gm /GX /ZI /O2 /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W4 /I "..\..\nvl\ps2" /I "ks" /I "..\..\ngl\ps2" /I "..\..\ngl\common" /I "..\..\nsl" /D "BUILD_BOOTABLE" /D "SN_TARGET_PS2" /D "__PS2_EE__" /D "ARCH_ENGINE" /D "PROJECT_PS2" /D "PS2" -G0 -g -mvu0-use-vf0-vf31 /c
MTL=midl.exe
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=snLink.exe
# ADD BASE LINK32 ngl_ps2.lib libstdc++.a libsn.a libgraph.a libdma.a libdev.a libpad.a libpkt.a libvu0.a libcdvd.a libmpeg.a libipu.a libpc.a sntty.lib dmadebug.a /nologo /pdb:none /debug /machine:IX86 /out:"..\bin\KellySlaterPS2_release.elf" /libpath:"src\hwosps2" /libpath:"..\..\ngl\ps2\PS2_EE_Release" /D:SN_TARGET_PS2
# ADD LINK32 nvl_ps2.lib libsdr.a nsl.lib libmc.a libz_ps2.lib ngl_ps2.lib libstdc++.a libsn.a libgraph.a libdma.a libdev.a libpad.a libpkt.a libvu0.a libcdvd.a libmpeg.a libipu.a libpc.a sntty.lib dmadebug.a libscf.a app.cmd /nologo /pdb:none /debug /machine:IX86 /out:"..\bin\KellySlaterPS2_bootable.elf" /libpath:"..\..\ngl\ps2\PS2_EE_Bootable" /libpath:"zlib\PS2_EE_Bootable" /libpath:"..\..\nsl\ps2\PS2_EE_Bootable" /libpath:"..\..\nvl\ps2\ps2_EE_Bootable" /D:SN_TARGET_PS2 -strip-unused -collect2
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=if exist userbuildps2.bat userbuildps2.bat
# End Special Build Tool

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "KellySlater___Win32_PS2_EE_Final"
# PROP BASE Intermediate_Dir "KellySlater___Win32_PS2_EE_Final"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\bin"
# PROP Intermediate_Dir "PS2_EE_Final"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
CPP=snCl.exe
# ADD BASE CPP /nologo /W4 /I "..\..\nvl\ps2" /I "ks" /I "..\..\ngl\ps2" /I "..\..\ngl\common" /I "..\..\nsl" /D "BUILD_BOOTABLE" /D "SN_TARGET_PS2" /D "__PS2_EE__" /D "ARCH_ENGINE" /D "PROJECT_PS2" /D "PS2" -G0 -mvu0-use-vf0-vf31 /c
# ADD CPP /nologo /W4 /I "..\..\nvl\ps2" /I "ks" /I "..\..\ngl\ps2" /I "..\..\ngl\common" /I "..\..\nsl" /D "BUILD_FINAL" /D "SN_TARGET_PS2" /D "__PS2_EE__" /D "ARCH_ENGINE" /D "PROJECT_PS2" /D "PS2" -G0 -mvu0-use-vf0-vf31 /c
MTL=midl.exe
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=snLink.exe
# ADD BASE LINK32 nvl_ps2.lib libsdr.a nsl.lib libmc.a libz_ps2.lib ngl_ps2.lib libstdc++.a libsn.a libgraph.a libdma.a libdev.a libpad.a libpkt.a libvu0.a libcdvd.a libmpeg.a libipu.a libpc.a sntty.lib dmadebug.a libscf.a app.cmd /nologo /pdb:none /debug /machine:IX86 /out:"..\bin\KellySlaterPS2_bootable.elf" /libpath:"..\..\ngl\ps2\PS2_EE_Bootable" /libpath:"zlib\PS2_EE_Bootable" /libpath:"..\..\nsl\ps2\PS2_EE_Bootable" /libpath:"..\..\nvl\ps2\ps2_EE_Bootable" /D:SN_TARGET_PS2 -strip-unused -collect2
# ADD LINK32 nvl_ps2.lib libsdr.a nsl.lib libmc.a libz_ps2.lib ngl_ps2.lib libstdc++.a libsn.a libgraph.a libdma.a libdev.a libpad.a libpkt.a libvu0.a libcdvd.a libmpeg.a libipu.a libpc.a sntty.lib dmadebug.a libscf.a app.cmd /nologo /pdb:none /machine:IX86 /out:"..\bin\KellySlaterPS2_bootable.elf" /libpath:"..\..\ngl\ps2\PS2_EE_Final" /libpath:"zlib\PS2_EE_Bootable" /libpath:"..\..\nsl\ps2\PS2_EE_Final" /libpath:"..\..\nvl\ps2\ps2_EE_Final" /D:SN_TARGET_PS2 -strip-unused -collect2
# SUBTRACT LINK32 /debug
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=if exist userbuildps2.bat userbuildps2.bat
# End Special Build Tool

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "KellySlater___Xbox_Final"
# PROP BASE Intermediate_Dir "KellySlater___Xbox_Final"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\bin"
# PROP Intermediate_Dir "Xbox_Final"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
MTL=midl.exe
RSC=rc.exe
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /GX /Zi /O2 /I "\ks\src" /I "\ks\src\ks" /I "\ks\src\mpeg" /I "..\..\ngl\xbox" /I "..\..\nvl\xbox" /I "..\..\nsl" /D "WIN32" /D "_XBOX" /D "NDEBUG" /D "ARCH_ENGINE" /D "TARGET_XBOX" /D "BUILD_BOOTABLE" /Yu"global.h" /FD /G6 /Ztmp /c
# ADD CPP /nologo /W3 /GX /Zi /O2 /I "\ks\src" /I "\ks\src\ks" /I "\ks\src\mpeg" /I "..\..\ngl\xbox" /I "..\..\nvl\xbox" /I "..\..\nsl" /D "WIN32" /D "_XBOX" /D "NDEBUG" /D "ARCH_ENGINE" /D "TARGET_XBOX" /D "BUILD_FINAL" /Yu"global.h" /FD /G6 /Ztmp /GL /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 binkxbox.lib xapilib.lib d3d8.lib d3dx8.lib xgraphics.lib dsound.lib dmusic.lib xnet.lib wmvdec.lib xboxkrnl.lib xbdm.lib ngl_xbox.lib nsl_xbox.lib nvl_xbox.lib /nologo /debug /machine:I386 /nodefaultlib:"libc" /out:"..\bin/KellySlaterXB_Bootable.exe" /libpath:"\ngl\xbox\Xbox_Bootable" /libpath:"\nsl\xbox\Xbox_Bootable" /libpath:"\nvl\xbox\Xbox_Bootable" /libpath:"\ks\src\mpeg" /subsystem:xbox /fixed:no /TMP /OPT:REF /OPT:ICF
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 binkxbox.lib xapilib.lib d3d8ltcg.lib d3dx8.lib xgraphicsltcg.lib dsound.lib dmusicltcg.lib xnet.lib wmvdec.lib xboxkrnl.lib xbdm.lib ngl_xbox.lib nsl_xbox.lib nvl_xbox.lib /nologo /debug /machine:I386 /nodefaultlib:"libc" /out:"..\bin/KellySlaterXB_Bootable.exe" /libpath:"\ngl\xbox\Xbox_Final" /libpath:"\nsl\xbox\Xbox_Final" /libpath:"\nvl\xbox\Xbox_Final" /subsystem:xbox /fixed:no /TMP /OPT:REF /OPT:ICF /LTCG
# SUBTRACT LINK32 /pdb:none
XBE=imagebld.exe
# ADD BASE XBE /nologo /stack:0x10000 /debug /out:"..\bin/KellySlaterXB_Bootable.xbe"
# SUBTRACT BASE XBE /limitmem
# ADD XBE /nologo /testid:"41560002" /testname:"Kelly Slater's Pro Surfer" /stack:0x20000 /debug /out:"..\bin/KellySlaterXB_Bootable.xbe" /titleimage:"ks\titleimage.xbx" /testratings:4 /testregion:0x1 /defaultsaveimage:"ks\saveimage.xbx"
# SUBTRACT XBE /limitmem
XBCP=xbecopy.exe
# ADD BASE XBCP /NOLOGO
# ADD XBCP /NOLOGO
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=if exist userbuildxb.bat userbuildxb.bat
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "KellySlater - Xbox Debug"
# Name "KellySlater - Xbox Bootable"
# Name "KellySlater - Win32 PS2 EE Debug"
# Name "KellySlater - Win32 PS2 EE Bootable"
# Name "KellySlater - Win32 PS2 EE Final"
# Name "KellySlater - Xbox Final"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "a*.cpp"

# PROP Default_Filter ""
# Begin Group "ai*.cpp"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ai_actions.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ai_goals.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ai_interface.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ai_locomotion.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ai_locomotion_direct.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ai_polypath.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ai_polypath_cell.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ai_voice.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\AIController.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=.\ks\AccompFrontEnd.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\actuator.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\aggvertbuf.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\anim_flavor.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\anim_maker.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\anim_user.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\animation_interface.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\app.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\archalloc.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "b*.cpp"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\b_spline.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\beach.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\beachdata.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\BeachFrontEnd.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\beam.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\billboard.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\bitplane.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\blur.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\board.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\boarddata.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\BoardFrontEnd.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\bone.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\box_trigger_interface.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "c*.cpp"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\camera.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\camera_tool.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\capsule.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\career.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\careerdata.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cface.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\challenge.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\challenge_icon.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\challenge_photo.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\cheat.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\CheatFrontEnd.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\cheatmenu.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\chunkfile.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\colgeom.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\collide.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\color.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\combodata.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\compressedphoto.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\conglom.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\console.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\consoleCmds.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\consoleVars.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\controller.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\convex_box.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\crate.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "d*.cpp"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\damage_interface.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\debug_render.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\debugutil.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\DemoMode.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\dxt1_codebook.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\dxt1_gen.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\dxt1_imagedxt1.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "e*.cpp"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\element.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\entity.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\entity_anim.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\entity_maker.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\errorcontext.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\eventmanager.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\eventmgr.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\ExtrasFrontEnd.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "f*.cpp"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\fcs.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\FEAnim.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\FEEntityManager.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\FEMenu.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\FEPanel.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\file.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\file_finder.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\file_manager.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\filespec.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\floatobj.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\fogmgr.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\frame_info.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\FrontEndManager.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\FrontEndMenus.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "g*.cpp"

# PROP Default_Filter ""
# Begin Group "gc*.cpp"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\HWOSGC\gc_algebra.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\HWOSGC\gc_alloc.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\hwosgc\gc_archalloc.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\HWOSGC\gc_audio.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\HWOSGC\gc_errmsg.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\HWOSGC\gc_file.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\HWOSGC\gc_gamesaver.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\HWOSGC\gc_graphics.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\HWOSGC\gc_input.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\HWOSGC\gc_math.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\HWOSGC\gc_movieplayer.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\HWOSGC\gc_rasterize.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\HWOSGC\gc_storage.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\HWOSGC\gc_texturemgr.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\HWOSGC\gc_timer.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\HWOSGC\ngl_font.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\HWOSGC\ngl_gc.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\HWOSGC\ngl_gc_profile.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\HWOSGC\ngl_instbank.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=.\game.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game_process.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\GameData.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gamefile.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\generator.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\geometry.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\geomgr.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\global.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# ADD CPP /Yc"global.h"

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# ADD CPP /Yc"global.h"

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# ADD BASE CPP /Yc"global.h"
# ADD CPP /Yc"global.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\GlobalData.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\globaltextenum.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\GraphicalMenuSystem.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\guidance_sys.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "h*.cpp"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\hard_attrib_interface.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\heap.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\HighScoreFrontEnd.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\hull.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "i*.cpp"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ks\igo_widget.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\igo_widget_analogclock.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\igo_widget_balance.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\igo_widget_breakindicator.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\igo_widget_camera.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\igo_widget_fanmeter.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\igo_widget_grid.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\igo_widget_iconcount.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\igo_widget_iconradar.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\igo_widget_meterchallenge.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\igo_widget_objectalert.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\igo_widget_photo.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\igo_widget_simple.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\igo_widget_skillchallenge.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\igo_widget_specialmeter.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\igo_widget_splitclock.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\igo_widget_splitmeter.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\igo_widget_splitscore.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\igo_widget_splitter.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\igo_widget_timeattack.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\igo_widget_waveindicator.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\IGOFrontEnd.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\igohintmanager.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\igoiconmanager.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\igolearn_new_trickmanager.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\ik_object.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ini_parser.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\inputmgr.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\interface.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\item.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "j*.cpp"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\joint.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\judge.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "k*.cpp"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ks\kellyslater_ai_goals.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\kellyslater_controller.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\kellyslater_main.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\ks_camera.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\ksdbmenu.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\ksfx.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\ksngl.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\ksngl_wave_gc.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\ksreplay.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "l*.cpp"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\lensflare.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\light.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\lightmgr.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\link_interface.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\localize.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "m*.cpp"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ks\MainFrontEnd.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\manip_obj.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\marker.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\material.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\matfac.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\MCDetectFrontEnd.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\mcs.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\menu.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\menu_scoring.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\menudraw.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\menungl.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\menusys.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\mic.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\mode_headtohead.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\mode_meterattack.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\mode_push.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\mode_timeattack.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\msgboard.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\MultiFrontEnd.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\MusicMan.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\mustash.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "o*.cpp"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ks\ode.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\opcodes.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\osassert.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\owner_interface.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "p*.cpp"

# PROP Default_Filter ""
# Begin Group "ps2*.cpp"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\HWOSPS2\ps2_algebra.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\HWOSPS2\ps2_alloc.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\HWOSPS2\ps2_archalloc.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\HWOSPS2\ps2_audio.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\HWOSPS2\ps2_devopts.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\HWOSPS2\ps2_errmsg.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\HWOSPS2\ps2_file.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\HWOSPS2\ps2_gamesaver.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\HWOSPS2\ps2_graphics.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\HWOSPS2\ps2_input.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\HWOSPS2\ps2_m2vplayer.c

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\HWOSPS2\ps2_math.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\HWOSPS2\ps2_movie_audiodec.c

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\HWOSPS2\ps2_movie_disp.c

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\HWOSPS2\ps2_movie_read.c

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\HWOSPS2\ps2_movie_readbuf.c

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\HWOSPS2\ps2_movie_strfile.c

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\HWOSPS2\ps2_movie_util.c

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\HWOSPS2\ps2_movie_vibuf.c

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\HWOSPS2\ps2_movie_videodec.c

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\HWOSPS2\ps2_movie_vobuf.c

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\HWOSPS2\ps2_movieplayer.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\HWOSPS2\ps2_rasterize.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\HWOSPS2\ps2_storage.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\HWOSPS2\ps2_texturemgr.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\HWOSPS2\ps2_timer.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\HWOSPS2\ps2_util.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\HWOSPS2\ps2_waverender.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ps2main.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=.\particle.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\particlecleaner.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\path.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\PhotoFrontEnd.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\physical_interface.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\physics.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\plane.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\player.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\pmesh.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\po.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\po_anim.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\polytube.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\portal.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\profcounters.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\profiler.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\proftimers.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\pstring.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "r*.cpp"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\region.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\render_interface.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\rumbleManager.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "s*.cpp"

# PROP Default_Filter ""
# Begin Group "script*.cpp"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\script_access.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\script_data_interface.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\script_lib_anim.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\script_lib_beam.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\script_lib_controller.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\script_lib_entity.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\script_lib_entity_widget.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\script_lib_item.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\script_lib_list.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\script_lib_mfg.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\script_lib_scene_anim.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\script_lib_signal.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\script_lib_sound_inst.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\script_lib_sound_stream.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\script_lib_switch.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\script_lib_trigger.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\script_lib_vector3d.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\script_lib_widget.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\script_library_class.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\script_object.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\script_register.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=.\ks\SaveLoadFrontEnd.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\scene_anim.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\scoringmanager.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\script_lib.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\serialin.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\SFXEngine.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\signal.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\signal_anim.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\SimpleAssert.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\singleton.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\skeleton_interface.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sky.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sl_debugger.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\slave_interface.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\so_data_block.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\soft_attrib_interface.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sound_group.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sound_interface.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\sounddata.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\SoundScript.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\specialmeter.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\spline.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\stash_support.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\StatsFrontEnd.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\stringx.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\surferdata.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\SurferFrontEnd.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\switch_obj.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "t*.cpp"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\terrain.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\text_font.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\text_parser.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\textfile.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\time_interface.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\timer.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\tool_dialogs.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\trail.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\trick_system.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\TrickBookFrontEnd.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\trickdata.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\tricks.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\trigger.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\turret.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\tutorialdata.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\TutorialFrontEnd.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\tutorialmanager.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "u*.cpp"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ks\underwtr.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "v*.cpp"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\vectorx.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\visrep.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\vm_executable.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\vm_stack.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\vm_symbol.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\vm_thread.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=ks\VOEngine.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "w*.cpp"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ks\water.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\wave.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\wavedata.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\wavemenu.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\waverendermenu.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\wavesound.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\wavetex.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\wavetexmenu.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\wds.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\widget.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\widget_entity.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\widget_script.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\wipeoutdata.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "x*.cpp"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\hwosxb\xb_algebra.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\hwosxb\xb_alloc.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\hwosxb\xb_archalloc.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\hwosxb\xb_audio.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\HWOSXB\xb_devopts.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\hwosxb\xb_errmsg.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\hwosxb\xb_file.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\HWOSXB\xb_gamesaver.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\hwosxb\xb_input.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\hwosxb\xb_m2vplayer.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\hwosxb\xb_math.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\hwosxb\xb_movieplayer.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\HWOSXB\xb_particle.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\HWOSXB\xb_portDVDCache.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\hwosxb\xb_rasterize.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\HWOSXB\xb_SaveLoadFrontEnd.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\hwosxb\xb_storage.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\HWOSXB\xb_storagedevice.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\hwosxb\xb_texturemgr.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\hwosxb\xb_timer.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\HWOSXB\xb_waverender.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\xbmain.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\xbstub.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "z*.cpp"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\zip_filter.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "a*.h"

# PROP Default_Filter ""
# Begin Group "ai*.h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ai_actions.h
# End Source File
# Begin Source File

SOURCE=.\ai_constants.h
# End Source File
# Begin Source File

SOURCE=.\ai_goals.h
# End Source File
# Begin Source File

SOURCE=.\ai_goals_mac.h
# End Source File
# Begin Source File

SOURCE=.\ai_interface.h
# End Source File
# Begin Source File

SOURCE=.\ai_locomotion.h
# End Source File
# Begin Source File

SOURCE=.\ai_locomotion_direct.h
# End Source File
# Begin Source File

SOURCE=.\ai_polypath.h
# End Source File
# Begin Source File

SOURCE=.\ai_polypath_cell.h
# End Source File
# Begin Source File

SOURCE=.\ai_polypath_heap.h
# End Source File
# Begin Source File

SOURCE=.\ai_voice.h
# End Source File
# Begin Source File

SOURCE=.\AIController.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\ks\AccompFrontEnd.h
# End Source File
# Begin Source File

SOURCE=.\actuator.h
# End Source File
# Begin Source File

SOURCE=.\aggvertbuf.h
# End Source File
# Begin Source File

SOURCE=.\algebra.h
# End Source File
# Begin Source File

SOURCE=.\anim.h
# End Source File
# Begin Source File

SOURCE=.\anim_flavor.h
# End Source File
# Begin Source File

SOURCE=.\anim_flavors.h
# End Source File
# Begin Source File

SOURCE=.\anim_ids.h
# End Source File
# Begin Source File

SOURCE=.\anim_maker.h
# End Source File
# Begin Source File

SOURCE=.\animation_interface.h
# End Source File
# Begin Source File

SOURCE=.\app.h
# End Source File
# Begin Source File

SOURCE=.\archalloc.h
# End Source File
# Begin Source File

SOURCE=.\attribute_template.h
# End Source File
# Begin Source File

SOURCE=.\avltree.h
# End Source File
# End Group
# Begin Group "b*.h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\b_spline.h
# End Source File
# Begin Source File

SOURCE=.\ks\beach.h
# End Source File
# Begin Source File

SOURCE=.\ks\beachdata.h
# End Source File
# Begin Source File

SOURCE=.\ks\BeachFrontEnd.h
# End Source File
# Begin Source File

SOURCE=.\beam.h
# End Source File
# Begin Source File

SOURCE=.\beam_signals.h
# End Source File
# Begin Source File

SOURCE=.\billboard.h
# End Source File
# Begin Source File

SOURCE=.\binary_tree.h
# End Source File
# Begin Source File

SOURCE=.\biped_ids.h
# End Source File
# Begin Source File

SOURCE=.\bitplane.h
# End Source File
# Begin Source File

SOURCE=.\blendmodes.h
# End Source File
# Begin Source File

SOURCE=.\ks\blur.h
# End Source File
# Begin Source File

SOURCE=.\ks\board.h
# End Source File
# Begin Source File

SOURCE=.\ks\boarddata.h
# End Source File
# Begin Source File

SOURCE=.\ks\BoardFrontEnd.h
# End Source File
# Begin Source File

SOURCE=.\bone.h
# End Source File
# Begin Source File

SOURCE=.\bound.h
# End Source File
# Begin Source File

SOURCE=.\box_trigger_interface.h
# End Source File
# Begin Source File

SOURCE=.\bp_tree.h
# End Source File
# Begin Source File

SOURCE=.\bsp_collide.h
# End Source File
# Begin Source File

SOURCE=.\bsp_tree.h
# End Source File
# End Group
# Begin Group "c*.h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\camera.h
# End Source File
# Begin Source File

SOURCE=.\ks\camera_tool.h
# End Source File
# Begin Source File

SOURCE=.\capsule.h
# End Source File
# Begin Source File

SOURCE=.\ks\career.h
# End Source File
# Begin Source File

SOURCE=.\ks\careerdata.h
# End Source File
# Begin Source File

SOURCE=.\cface.h
# End Source File
# Begin Source File

SOURCE=.\ks\challenge.h
# End Source File
# Begin Source File

SOURCE=.\ks\challenge_icon.h
# End Source File
# Begin Source File

SOURCE=.\ks\challenge_photo.h
# End Source File
# Begin Source File

SOURCE=.\character_hard_attribs.h
# End Source File
# Begin Source File

SOURCE=.\character_soft_attribs.h
# End Source File
# Begin Source File

SOURCE=.\ks\cheat.h
# End Source File
# Begin Source File

SOURCE=.\ks\CheatFrontEnd.h
# End Source File
# Begin Source File

SOURCE=.\chunkfile.h
# End Source File
# Begin Source File

SOURCE=.\clipflags.h
# End Source File
# Begin Source File

SOURCE=.\colgeom.h
# End Source File
# Begin Source File

SOURCE=.\collide.h
# End Source File
# Begin Source File

SOURCE=.\colmesh.h
# End Source File
# Begin Source File

SOURCE=.\color.h
# End Source File
# Begin Source File

SOURCE=.\ks\combodata.h
# End Source File
# Begin Source File

SOURCE=.\commands.h
# End Source File
# Begin Source File

SOURCE=.\ks\compressedphoto.h
# End Source File
# Begin Source File

SOURCE=.\conglom.h
# End Source File
# Begin Source File

SOURCE=.\console.h
# End Source File
# Begin Source File

SOURCE=.\consoleCmds.h
# End Source File
# Begin Source File

SOURCE=.\consoleVars.h
# End Source File
# Begin Source File

SOURCE=.\constants.h
# End Source File
# Begin Source File

SOURCE=.\controller.h
# End Source File
# Begin Source File

SOURCE=.\convex_box.h
# End Source File
# Begin Source File

SOURCE=.\ks\coords.h
# End Source File
# Begin Source File

SOURCE=.\crate.h
# End Source File
# Begin Source File

SOURCE=.\crawl_box.h
# End Source File
# Begin Source File

SOURCE=.\custom_stl.h
# End Source File
# End Group
# Begin Group "d*.h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\damage_interface.h
# End Source File
# Begin Source File

SOURCE=.\debug.h
# End Source File
# Begin Source File

SOURCE=.\debug_render.h
# End Source File
# Begin Source File

SOURCE=.\ks\DemoMode.h
# End Source File
# Begin Source File

SOURCE=.\devoptflags.h
# End Source File
# Begin Source File

SOURCE=.\devoptints.h
# End Source File
# Begin Source File

SOURCE=.\devoptstrs.h
# End Source File
# Begin Source File

SOURCE=.\ks\dxt1_codebook.h
# End Source File
# Begin Source File

SOURCE=.\ks\dxt1_gen.h
# End Source File
# Begin Source File

SOURCE=.\ks\dxt1_imagedxt1.h
# End Source File
# Begin Source File

SOURCE=.\ks\dxt1_table.h
# End Source File
# End Group
# Begin Group "e*.h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\element.h
# End Source File
# Begin Source File

SOURCE=.\entflavor.h
# End Source File
# Begin Source File

SOURCE=.\entity.h
# End Source File
# Begin Source File

SOURCE=.\entity_anim.h
# End Source File
# Begin Source File

SOURCE=.\entity_hard_attribs.h
# End Source File
# Begin Source File

SOURCE=.\entity_interface.h
# End Source File
# Begin Source File

SOURCE=.\entity_maker.h
# End Source File
# Begin Source File

SOURCE=.\entity_signals.h
# End Source File
# Begin Source File

SOURCE=.\entity_soft_attribs.h
# End Source File
# Begin Source File

SOURCE=.\entityflags.h
# End Source File
# Begin Source File

SOURCE=.\entityid.h
# End Source File
# Begin Source File

SOURCE=.\errorcontext.h
# End Source File
# Begin Source File

SOURCE=.\ks\eventmanager.h
# End Source File
# Begin Source File

SOURCE=.\eventmgr.h
# End Source File
# Begin Source File

SOURCE=.\ks\ExtrasFrontEnd.h
# End Source File
# End Group
# Begin Group "f*.h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\face.h
# End Source File
# Begin Source File

SOURCE=.\faceflags.h
# End Source File
# Begin Source File

SOURCE=.\fast_vector.h
# End Source File
# Begin Source File

SOURCE=.\fcs.h
# End Source File
# Begin Source File

SOURCE=.\ks\FEAnim.h
# End Source File
# Begin Source File

SOURCE=.\ks\FEEntityManager.h
# End Source File
# Begin Source File

SOURCE=.\ks\FEMenu.h
# End Source File
# Begin Source File

SOURCE=.\ks\FEPanel.h
# End Source File
# Begin Source File

SOURCE=.\file.h
# End Source File
# Begin Source File

SOURCE=.\file_finder.h
# End Source File
# Begin Source File

SOURCE=.\file_manager.h
# End Source File
# Begin Source File

SOURCE=.\filespec.h
# End Source File
# Begin Source File

SOURCE=.\fixedstring.h
# End Source File
# Begin Source File

SOURCE=.\ks\floatobj.h
# End Source File
# Begin Source File

SOURCE=.\fogmgr.h
# End Source File
# Begin Source File

SOURCE=.\forceflags.h
# End Source File
# Begin Source File

SOURCE=.\frame_info.h
# End Source File
# Begin Source File

SOURCE=.\ks\FrontEndManager.h
# End Source File
# Begin Source File

SOURCE=.\ks\FrontEndMenus.h
# End Source File
# End Group
# Begin Group "g*.h"

# PROP Default_Filter ""
# Begin Group "gc*.h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\hwosgc\gc_algebra.h
# End Source File
# Begin Source File

SOURCE=.\hwosgc\gc_alloc.h
# End Source File
# Begin Source File

SOURCE=.\hwosgc\gc_audio.h
# End Source File
# Begin Source File

SOURCE=.\hwosgc\gc_errmsg.h
# End Source File
# Begin Source File

SOURCE=.\hwosgc\gc_file.h
# End Source File
# Begin Source File

SOURCE=.\hwosgc\gc_graphics.h
# End Source File
# Begin Source File

SOURCE=.\hwosgc\gc_input.h
# End Source File
# Begin Source File

SOURCE=.\hwosgc\gc_math.h
# End Source File
# Begin Source File

SOURCE=.\hwosgc\gc_movieplayer.h
# End Source File
# Begin Source File

SOURCE=.\hwosgc\gc_rasterize.h
# End Source File
# Begin Source File

SOURCE=.\hwosgc\gc_storage.h
# End Source File
# Begin Source File

SOURCE=.\hwosgc\gc_texturemgr.h
# End Source File
# Begin Source File

SOURCE=.\hwosgc\gc_timer.h
# End Source File
# Begin Source File

SOURCE=.\hwosgc\ngl_fixedstr.h
# End Source File
# Begin Source File

SOURCE=.\hwosgc\ngl_gc.h
# End Source File
# Begin Source File

SOURCE=.\hwosgc\ngl_gc_profile.h
# End Source File
# Begin Source File

SOURCE=.\HWOSGC\ngl_gc_profile_internal.h
# End Source File
# Begin Source File

SOURCE=.\HWOSGC\ngl_instbank.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\game.h
# End Source File
# Begin Source File

SOURCE=.\game_info.h
# End Source File
# Begin Source File

SOURCE=.\game_info_vars.h
# End Source File
# Begin Source File

SOURCE=.\game_process.h
# End Source File
# Begin Source File

SOURCE=.\ks\GameData.h
# End Source File
# Begin Source File

SOURCE=.\gamefile.h
# End Source File
# Begin Source File

SOURCE=.\hwosxb\gas.h
# End Source File
# Begin Source File

SOURCE=.\HWOSGC\gc_gamesaver.h
# End Source File
# Begin Source File

SOURCE=.\generator.h
# End Source File
# Begin Source File

SOURCE=.\geometry.h
# End Source File
# Begin Source File

SOURCE=.\geomgr.h
# End Source File
# Begin Source File

SOURCE=.\global.h
# End Source File
# Begin Source File

SOURCE=.\global_signals.h
# End Source File
# Begin Source File

SOURCE=.\ks\GlobalData.h
# End Source File
# Begin Source File

SOURCE=.\ks\globaltextenum.h
# End Source File
# Begin Source File

SOURCE=.\graph.h
# End Source File
# Begin Source File

SOURCE=.\ks\GraphicalMenuSystem.h
# End Source File
# Begin Source File

SOURCE=.\grenade_signals.h
# End Source File
# Begin Source File

SOURCE=.\guidance_sys.h
# End Source File
# End Group
# Begin Group "h*.h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\hard_attrib_interface.h
# End Source File
# Begin Source File

SOURCE=.\hard_attribs.h
# End Source File
# Begin Source File

SOURCE=.\heap.h
# End Source File
# Begin Source File

SOURCE=.\ks\HighScoreFrontEnd.h
# End Source File
# Begin Source File

SOURCE=.\hull.h
# End Source File
# Begin Source File

SOURCE=.\hwaudio.h
# End Source File
# Begin Source File

SOURCE=.\hwmath.h
# End Source File
# Begin Source File

SOURCE=.\hwmovieplayer.h
# End Source File
# Begin Source File

SOURCE=.\hwrasterize.h
# End Source File
# Begin Source File

SOURCE=.\hyperplane.h
# End Source File
# End Group
# Begin Group "i*.h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ks\igo_widget.h
# End Source File
# Begin Source File

SOURCE=.\ks\igo_widget_analogclock.h
# End Source File
# Begin Source File

SOURCE=.\ks\igo_widget_balance.h
# End Source File
# Begin Source File

SOURCE=.\ks\igo_widget_breakindicator.h
# End Source File
# Begin Source File

SOURCE=.\ks\igo_widget_camera.h
# End Source File
# Begin Source File

SOURCE=.\ks\igo_widget_fanmeter.h
# End Source File
# Begin Source File

SOURCE=.\ks\igo_widget_grid.h
# End Source File
# Begin Source File

SOURCE=.\ks\igo_widget_iconcount.h
# End Source File
# Begin Source File

SOURCE=.\ks\igo_widget_iconradar.h
# End Source File
# Begin Source File

SOURCE=.\ks\igo_widget_meterchallenge.h
# End Source File
# Begin Source File

SOURCE=.\ks\igo_widget_objectalert.h
# End Source File
# Begin Source File

SOURCE=.\ks\igo_widget_photo.h
# End Source File
# Begin Source File

SOURCE=.\ks\igo_widget_simple.h
# End Source File
# Begin Source File

SOURCE=.\ks\igo_widget_skillchallenge.h
# End Source File
# Begin Source File

SOURCE=.\ks\igo_widget_specialmeter.h
# End Source File
# Begin Source File

SOURCE=.\ks\igo_widget_splitclock.h
# End Source File
# Begin Source File

SOURCE=.\ks\igo_widget_splitmeter.h
# End Source File
# Begin Source File

SOURCE=.\ks\igo_widget_splitscore.h
# End Source File
# Begin Source File

SOURCE=.\ks\igo_widget_splitter.h
# End Source File
# Begin Source File

SOURCE=.\ks\igo_widget_timeattack.h
# End Source File
# Begin Source File

SOURCE=.\ks\igo_widget_waveindicator.h
# End Source File
# Begin Source File

SOURCE=.\ks\IGOFrontEnd.h
# End Source File
# Begin Source File

SOURCE=.\ks\igohintmanager.h
# End Source File
# Begin Source File

SOURCE=.\ks\igoiconmanager.h
# End Source File
# Begin Source File

SOURCE=.\ks\igolearn_new_trickmanager.h
# End Source File
# Begin Source File

SOURCE=.\ks\ik_object.h
# End Source File
# Begin Source File

SOURCE=.\ini_parser.h
# End Source File
# Begin Source File

SOURCE=.\inputmgr.h
# End Source File
# Begin Source File

SOURCE=.\instance.h
# End Source File
# Begin Source File

SOURCE=.\interface.h
# End Source File
# Begin Source File

SOURCE=.\iri.h
# End Source File
# Begin Source File

SOURCE=.\item.h
# End Source File
# Begin Source File

SOURCE=.\item_signals.h
# End Source File
# End Group
# Begin Group "j*.h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\joint.h
# End Source File
# Begin Source File

SOURCE=.\joystick.h
# End Source File
# Begin Source File

SOURCE=.\ks\judge.h
# End Source File
# End Group
# Begin Group "k*.h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ks\kellyslater_ai_goals.h
# End Source File
# Begin Source File

SOURCE=.\ks\kellyslater_controller.h
# End Source File
# Begin Source File

SOURCE=.\ks\kellyslater_main.h
# End Source File
# Begin Source File

SOURCE=.\ks\kellyslater_states_mac.h
# End Source File
# Begin Source File

SOURCE=.\keyboard.h
# End Source File
# Begin Source File

SOURCE=.\ks\ks_camera.h
# End Source File
# Begin Source File

SOURCE=.\ks\ksdbmenu.h
# End Source File
# Begin Source File

SOURCE=.\ks\ksfx.h
# End Source File
# Begin Source File

SOURCE=.\ks\ksngl.h
# End Source File
# Begin Source File

SOURCE=.\ksnsl.h
# End Source File
# Begin Source File

SOURCE=.\ksnvl.h
# End Source File
# Begin Source File

SOURCE=.\ks\ksreplay.h
# End Source File
# End Group
# Begin Group "l*.h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\lensflare.h
# End Source File
# Begin Source File

SOURCE=.\light.h
# End Source File
# Begin Source File

SOURCE=.\lightmgr.h
# End Source File
# Begin Source File

SOURCE=.\linear_anim.h
# End Source File
# Begin Source File

SOURCE=.\link_interface.h
# End Source File
# Begin Source File

SOURCE=.\localize.h
# End Source File
# End Group
# Begin Group "m*.h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ks\MainFrontEnd.h
# End Source File
# Begin Source File

SOURCE=.\manip_obj.h
# End Source File
# Begin Source File

SOURCE=.\map_e.h
# End Source File
# Begin Source File

SOURCE=.\marker.h
# End Source File
# Begin Source File

SOURCE=.\material.h
# End Source File
# Begin Source File

SOURCE=.\matfac.h
# End Source File
# Begin Source File

SOURCE=.\maxiri.h
# End Source File
# Begin Source File

SOURCE=.\maxskinbones.h
# End Source File
# Begin Source File

SOURCE=.\mbi.h
# End Source File
# Begin Source File

SOURCE=.\ks\MCDetectFrontEnd.h
# End Source File
# Begin Source File

SOURCE=.\mcs.h
# End Source File
# Begin Source File

SOURCE=.\ks\menu.h
# End Source File
# Begin Source File

SOURCE=.\ks\menu_scoring.h
# End Source File
# Begin Source File

SOURCE=.\menudraw.h
# End Source File
# Begin Source File

SOURCE=.\ks\menusys.h
# End Source File
# Begin Source File

SOURCE=.\meshrefs.h
# End Source File
# Begin Source File

SOURCE=.\mic.h
# End Source File
# Begin Source File

SOURCE=.\mobject.h
# End Source File
# Begin Source File

SOURCE=.\ks\mode.h
# End Source File
# Begin Source File

SOURCE=.\ks\mode_headtohead.h
# End Source File
# Begin Source File

SOURCE=.\ks\mode_meterattack.h
# End Source File
# Begin Source File

SOURCE=.\ks\mode_push.h
# End Source File
# Begin Source File

SOURCE=.\ks\mode_timeattack.h
# End Source File
# Begin Source File

SOURCE=.\mouse.h
# End Source File
# Begin Source File

SOURCE=.\msgboard.h
# End Source File
# Begin Source File

SOURCE=.\ks\MultiFrontEnd.h
# End Source File
# Begin Source File

SOURCE=.\ks\MusicMan.h
# End Source File
# Begin Source File

SOURCE=.\mustash.h
# End Source File
# End Group
# Begin Group "n*.h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ngl.h
# End Source File
# End Group
# Begin Group "o*.h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ks\ode.h
# End Source File
# Begin Source File

SOURCE=.\opcodes.h
# End Source File
# Begin Source File

SOURCE=.\osalloc.h
# End Source File
# Begin Source File

SOURCE=.\osassert.h
# End Source File
# Begin Source File

SOURCE=.\osdevopts.h
# End Source File
# Begin Source File

SOURCE=.\oserrmsg.h
# End Source File
# Begin Source File

SOURCE=.\osfile.h
# End Source File
# Begin Source File

SOURCE=.\ks\osGameSaver.h
# End Source File
# Begin Source File

SOURCE=.\ks\osparticle.h
# End Source File
# Begin Source File

SOURCE=.\osstorage.h
# End Source File
# Begin Source File

SOURCE=.\ostimer.h
# End Source File
# Begin Source File

SOURCE=.\ks\oswaverender.h
# End Source File
# Begin Source File

SOURCE=.\owner_interface.h
# End Source File
# End Group
# Begin Group "p*.h"

# PROP Default_Filter ""
# Begin Group "ps2*.h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\HWOSPS2\ps2_algebra.h
# End Source File
# Begin Source File

SOURCE=.\HWOSPS2\ps2_alloc.h
# End Source File
# Begin Source File

SOURCE=.\HWOSPS2\ps2_audio.h
# End Source File
# Begin Source File

SOURCE=.\HWOSPS2\ps2_devopts.h
# End Source File
# Begin Source File

SOURCE=.\HWOSPS2\ps2_errmsg.h
# End Source File
# Begin Source File

SOURCE=.\HWOSPS2\ps2_file.h
# End Source File
# Begin Source File

SOURCE=.\HWOSPS2\ps2_gamesaver.h
# End Source File
# Begin Source File

SOURCE=.\HWOSPS2\ps2_graphics.h
# End Source File
# Begin Source File

SOURCE=.\HWOSPS2\ps2_input.h
# End Source File
# Begin Source File

SOURCE=.\HWOSPS2\ps2_m2vplayer.h
# End Source File
# Begin Source File

SOURCE=.\HWOSPS2\ps2_math.h
# End Source File
# Begin Source File

SOURCE=.\HWOSPS2\ps2_movie_audiodec.h
# End Source File
# Begin Source File

SOURCE=.\HWOSPS2\ps2_movie_defs.h
# End Source File
# Begin Source File

SOURCE=.\HWOSPS2\ps2_movie_disp.h
# End Source File
# Begin Source File

SOURCE=.\HWOSPS2\ps2_movie_readbuf.h
# End Source File
# Begin Source File

SOURCE=.\HWOSPS2\ps2_movie_strfile.h
# End Source File
# Begin Source File

SOURCE=.\HWOSPS2\ps2_movie_vibuf.h
# End Source File
# Begin Source File

SOURCE=.\HWOSPS2\ps2_movie_videodec.h
# End Source File
# Begin Source File

SOURCE=.\HWOSPS2\ps2_movie_vobuf.h
# End Source File
# Begin Source File

SOURCE=.\HWOSPS2\ps2_movieplayer.h
# End Source File
# Begin Source File

SOURCE=.\HWOSPS2\ps2_rasterize.h
# End Source File
# Begin Source File

SOURCE=.\HWOSPS2\ps2_storage.h
# End Source File
# Begin Source File

SOURCE=.\HWOSPS2\ps2_texturemgr.h
# End Source File
# Begin Source File

SOURCE=.\HWOSPS2\ps2_timer.h
# End Source File
# Begin Source File

SOURCE=.\HWOSPS2\ps2_types.h
# End Source File
# Begin Source File

SOURCE=.\HWOSPS2\ps2_util.h
# End Source File
# Begin Source File

SOURCE=.\HWOSPS2\ps2_waverender.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\particle.h
# End Source File
# Begin Source File

SOURCE=.\particlecleaner.h
# End Source File
# Begin Source File

SOURCE=.\path.h
# End Source File
# Begin Source File

SOURCE=.\pc_port.h
# End Source File
# Begin Source File

SOURCE=.\ks\PhotoFrontEnd.h
# End Source File
# Begin Source File

SOURCE=.\physical_interface.h
# End Source File
# Begin Source File

SOURCE=.\ks\physics.h
# End Source File
# Begin Source File

SOURCE=.\plane.h
# End Source File
# Begin Source File

SOURCE=.\ks\player.h
# End Source File
# Begin Source File

SOURCE=.\pmesh.h
# End Source File
# Begin Source File

SOURCE=.\po.h
# End Source File
# Begin Source File

SOURCE=.\po_anim.h
# End Source File
# Begin Source File

SOURCE=.\polytube.h
# End Source File
# Begin Source File

SOURCE=.\portal.h
# End Source File
# Begin Source File

SOURCE=.\profiler.h
# End Source File
# Begin Source File

SOURCE=.\projconst.h
# End Source File
# Begin Source File

SOURCE=.\project.h
# End Source File
# End Group
# Begin Group "r*.h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\random.h
# End Source File
# Begin Source File

SOURCE=.\rect.h
# End Source File
# Begin Source File

SOURCE=.\refptr.h
# End Source File
# Begin Source File

SOURCE=.\region.h
# End Source File
# Begin Source File

SOURCE=.\region_graph.h
# End Source File
# Begin Source File

SOURCE=.\render_data.h
# End Source File
# Begin Source File

SOURCE=.\render_interface.h
# End Source File
# Begin Source File

SOURCE=.\renderflav.h
# End Source File
# Begin Source File

SOURCE=.\ks\replay.h
# End Source File
# Begin Source File

SOURCE=.\ks\rumbleManager.h
# End Source File
# End Group
# Begin Group "s*.h"

# PROP Default_Filter ""
# Begin Group "script*.h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\script_access.h
# End Source File
# Begin Source File

SOURCE=.\script_controller_signals.h
# End Source File
# Begin Source File

SOURCE=.\script_data_interface.h
# End Source File
# Begin Source File

SOURCE=.\script_lib_anim.h
# End Source File
# Begin Source File

SOURCE=.\script_lib_beam.h
# End Source File
# Begin Source File

SOURCE=.\script_lib_controller.h
# End Source File
# Begin Source File

SOURCE=.\script_lib_entity.h
# End Source File
# Begin Source File

SOURCE=.\script_lib_entity_widget.h
# End Source File
# Begin Source File

SOURCE=.\script_lib_item.h
# End Source File
# Begin Source File

SOURCE=.\script_lib_list.h
# End Source File
# Begin Source File

SOURCE=.\script_lib_mfg.h
# End Source File
# Begin Source File

SOURCE=.\script_lib_scene_anim.h
# End Source File
# Begin Source File

SOURCE=.\script_lib_signal.h
# End Source File
# Begin Source File

SOURCE=.\script_lib_sound_inst.h
# End Source File
# Begin Source File

SOURCE=.\script_lib_sound_stream.h
# End Source File
# Begin Source File

SOURCE=.\script_lib_switch.h
# End Source File
# Begin Source File

SOURCE=.\script_lib_trigger.h
# End Source File
# Begin Source File

SOURCE=.\script_lib_vector3d.h
# End Source File
# Begin Source File

SOURCE=.\script_lib_widget.h
# End Source File
# Begin Source File

SOURCE=.\script_library_class.h
# End Source File
# Begin Source File

SOURCE=.\script_mfg_signals.h
# End Source File
# Begin Source File

SOURCE=.\script_object.h
# End Source File
# Begin Source File

SOURCE=.\script_register.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\ks\SaveLoadFrontEnd.h
# End Source File
# Begin Source File

SOURCE=.\scanner_signals.h
# End Source File
# Begin Source File

SOURCE=.\scene_anim.h
# End Source File
# Begin Source File

SOURCE=.\ks\scoringmanager.h
# End Source File
# Begin Source File

SOURCE=.\ks\SFXEngine.h
# End Source File
# Begin Source File

SOURCE=.\shock_mods.h
# End Source File
# Begin Source File

SOURCE=.\signal_anim.h
# End Source File
# Begin Source File

SOURCE=.\signals.h
# End Source File
# Begin Source File

SOURCE=.\simple_classes.h
# End Source File
# Begin Source File

SOURCE=.\ks\simple_list.h
# End Source File
# Begin Source File

SOURCE=.\SimpleAssert.h
# End Source File
# Begin Source File

SOURCE=.\singleton.h
# End Source File
# Begin Source File

SOURCE=.\skeleton_interface.h
# End Source File
# Begin Source File

SOURCE=.\sky.h
# End Source File
# Begin Source File

SOURCE=.\sl_debugger.h
# End Source File
# Begin Source File

SOURCE=.\slave_interface.h
# End Source File
# Begin Source File

SOURCE=.\so_data_block.h
# End Source File
# Begin Source File

SOURCE=.\soft_attrib_interface.h
# End Source File
# Begin Source File

SOURCE=.\soft_attribs.h
# End Source File
# Begin Source File

SOURCE=.\sound_group.h
# End Source File
# Begin Source File

SOURCE=.\sound_interface.h
# End Source File
# Begin Source File

SOURCE=.\ks\sounddata.h
# End Source File
# Begin Source File

SOURCE=.\ks\SoundScript.h
# End Source File
# Begin Source File

SOURCE=.\ks\specialmeter.h
# End Source File
# Begin Source File

SOURCE=.\sphere.h
# End Source File
# Begin Source File

SOURCE=.\ks\spline.h
# End Source File
# Begin Source File

SOURCE=.\stash_support.h
# End Source File
# Begin Source File

SOURCE=.\staticmem.h
# End Source File
# Begin Source File

SOURCE=.\ks\StatsFrontEnd.h
# End Source File
# Begin Source File

SOURCE=.\stl_adapter.h
# End Source File
# Begin Source File

SOURCE=.\stringx.h
# End Source File
# Begin Source File

SOURCE=.\ks\surferdata.h
# End Source File
# Begin Source File

SOURCE=.\ks\SurferFrontEnd.h
# End Source File
# Begin Source File

SOURCE=.\switch_obj.h
# End Source File
# Begin Source File

SOURCE=.\switch_obj_signals.h
# End Source File
# End Group
# Begin Group "t*.h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\terrain.h
# End Source File
# Begin Source File

SOURCE=.\text_font.h
# End Source File
# Begin Source File

SOURCE=.\ks\text_parser.h
# End Source File
# Begin Source File

SOURCE=.\textfile.h
# End Source File
# Begin Source File

SOURCE=.\time_interface.h
# End Source File
# Begin Source File

SOURCE=.\timer.h
# End Source File
# Begin Source File

SOURCE=.\tool_dialogs.h
# End Source File
# Begin Source File

SOURCE=.\ks\trail.h
# End Source File
# Begin Source File

SOURCE=.\ks\trick_system.h
# End Source File
# Begin Source File

SOURCE=.\ks\TrickBookFrontEnd.h
# End Source File
# Begin Source File

SOURCE=.\ks\trickdata.h
# End Source File
# Begin Source File

SOURCE=.\ks\trickregion.h
# End Source File
# Begin Source File

SOURCE=.\ks\tricks.h
# End Source File
# Begin Source File

SOURCE=.\trigger.h
# End Source File
# Begin Source File

SOURCE=.\trigger_signals.h
# End Source File
# Begin Source File

SOURCE=.\turret.h
# End Source File
# Begin Source File

SOURCE=.\ks\tutorialdata.h
# End Source File
# Begin Source File

SOURCE=.\ks\TutorialFrontEnd.h
# End Source File
# Begin Source File

SOURCE=.\ks\tutorialmanager.h
# End Source File
# Begin Source File

SOURCE=.\txtcoord.h
# End Source File
# Begin Source File

SOURCE=.\types.h
# End Source File
# End Group
# Begin Group "u*.h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\usefulmath.h
# End Source File
# Begin Source File

SOURCE=.\users.h
# End Source File
# End Group
# Begin Group "v*.h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\vector_alloc.h
# End Source File
# Begin Source File

SOURCE=.\vectorx.h
# End Source File
# Begin Source File

SOURCE=.\vert.h
# End Source File
# Begin Source File

SOURCE=.\vertnorm.h
# End Source File
# Begin Source File

SOURCE=.\vertwork.h
# End Source File
# Begin Source File

SOURCE=.\visrep.h
# End Source File
# Begin Source File

SOURCE=.\vm_executable.h
# End Source File
# Begin Source File

SOURCE=.\vm_executable_vector.h
# End Source File
# Begin Source File

SOURCE=.\vm_stack.h
# End Source File
# Begin Source File

SOURCE=.\vm_symbol.h
# End Source File
# Begin Source File

SOURCE=.\vm_symbol_list.h
# End Source File
# Begin Source File

SOURCE=.\vm_thread.h
# End Source File
# Begin Source File

SOURCE=.\ks\VOEngine.h
# End Source File
# Begin Source File

SOURCE=.\vsplit.h
# End Source File
# End Group
# Begin Group "w*.h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\warnlvl.h
# End Source File
# Begin Source File

SOURCE=.\ks\water.h
# End Source File
# Begin Source File

SOURCE=.\ks\watercolors.h
# End Source File
# Begin Source File

SOURCE=.\ks\wave.h
# End Source File
# Begin Source File

SOURCE=.\ks\wavedata.h
# End Source File
# Begin Source File

SOURCE=.\ks\waveenum.h
# End Source File
# Begin Source File

SOURCE=.\ks\wavesound.h
# End Source File
# Begin Source File

SOURCE=.\ks\wavetex.h
# End Source File
# Begin Source File

SOURCE=.\wds.h
# End Source File
# Begin Source File

SOURCE=.\wedge.h
# End Source File
# Begin Source File

SOURCE=.\widget.h
# End Source File
# Begin Source File

SOURCE=.\widget_entity.h
# End Source File
# Begin Source File

SOURCE=.\widget_script.h
# End Source File
# Begin Source File

SOURCE=.\ks\wipeoutdata.h
# End Source File
# End Group
# Begin Group "x*.h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\hwosxb\xb_algebra.h
# End Source File
# Begin Source File

SOURCE=.\hwosxb\xb_alloc.h
# End Source File
# Begin Source File

SOURCE=.\hwosxb\xb_audio.h
# End Source File
# Begin Source File

SOURCE=.\HWOSXB\xb_devopts.h
# End Source File
# Begin Source File

SOURCE=.\hwosxb\xb_errmsg.h
# End Source File
# Begin Source File

SOURCE=.\hwosxb\xb_file.h
# End Source File
# Begin Source File

SOURCE=.\HWOSXB\xb_FileCacheSubdirTable.h
# End Source File
# Begin Source File

SOURCE=.\HWOSXB\xb_FileCacheTable.h
# End Source File
# Begin Source File

SOURCE=.\HWOSXB\xb_gamesaver.h
# End Source File
# Begin Source File

SOURCE=.\hwosxb\xb_input.h
# End Source File
# Begin Source File

SOURCE=.\hwosxb\xb_m2vplayer.h
# End Source File
# Begin Source File

SOURCE=.\hwosxb\xb_math.h
# End Source File
# Begin Source File

SOURCE=.\hwosxb\xb_movie_audiodec.h
# End Source File
# Begin Source File

SOURCE=.\hwosxb\xb_movie_defs.h
# End Source File
# Begin Source File

SOURCE=.\hwosxb\xb_movie_disp.h
# End Source File
# Begin Source File

SOURCE=.\hwosxb\xb_movie_readbuf.h
# End Source File
# Begin Source File

SOURCE=.\hwosxb\xb_movie_strfile.h
# End Source File
# Begin Source File

SOURCE=.\hwosxb\xb_movie_vibuf.h
# End Source File
# Begin Source File

SOURCE=.\hwosxb\xb_movie_videodec.h
# End Source File
# Begin Source File

SOURCE=.\hwosxb\xb_movie_vobuf.h
# End Source File
# Begin Source File

SOURCE=.\hwosxb\xb_movieplayer.h
# End Source File
# Begin Source File

SOURCE=.\HWOSXB\xb_particle.h
# End Source File
# Begin Source File

SOURCE=.\HWOSXB\xb_portDVDCache.h
# End Source File
# Begin Source File

SOURCE=.\hwosxb\xb_rasterize.h
# End Source File
# Begin Source File

SOURCE=.\HWOSXB\xb_SaveLoadFrontEnd.h
# End Source File
# Begin Source File

SOURCE=.\hwosxb\xb_storage.h
# End Source File
# Begin Source File

SOURCE=.\HWOSXB\xb_storagedevice.h
# End Source File
# Begin Source File

SOURCE=.\hwosxb\xb_texturemgr.h
# End Source File
# Begin Source File

SOURCE=.\hwosxb\xb_timer.h
# End Source File
# Begin Source File

SOURCE=.\hwosxb\xb_types.h
# End Source File
# Begin Source File

SOURCE=.\HWOSXB\xb_waverender.h
# End Source File
# End Group
# Begin Group "z*.h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\zip_filter.h
# End Source File
# End Group
# End Group
# Begin Group "Text Files"

# PROP Default_Filter ".txt"
# Begin Source File

SOURCE=.\acostable.txt
# End Source File
# Begin Source File

SOURCE=.\ks\displacedebug.txt
# End Source File
# Begin Source File

SOURCE=.\ks\menudrawmisc.txt
# End Source File
# Begin Source File

SOURCE=.\ks\menudrawparticle.txt
# End Source File
# Begin Source File

SOURCE=.\ks\menudrawwater.txt
# End Source File
# Begin Source File

SOURCE=.\opcode_arg_t
# End Source File
# Begin Source File

SOURCE=.\opcode_t
# End Source File
# Begin Source File

SOURCE=.\HWOSPS2\ps2_waverenderdebug.txt
# End Source File
# Begin Source File

SOURCE=.\ks\sin.txt
# End Source File
# Begin Source File

SOURCE=.\sintable.txt
# End Source File
# Begin Source File

SOURCE=.\ks\wavebreak.txt
# End Source File
# Begin Source File

SOURCE=.\ks\wavebreakstage.txt
# End Source File
# Begin Source File

SOURCE=.\ks\wavedebug.txt
# End Source File
# Begin Source File

SOURCE=.\ks\waveemitter.txt
# End Source File
# Begin Source File

SOURCE=.\ks\wavemarker.txt
# End Source File
# Begin Source File

SOURCE=.\ks\waveregion.txt
# End Source File
# Begin Source File

SOURCE=.\ks\wavestage.txt
# End Source File
# Begin Source File

SOURCE=.\ks\wavetexdebug.txt
# End Source File
# Begin Source File

SOURCE=.\HWOSXB\xb_waverenderdebug.txt
# End Source File
# End Group
# Begin Group "Excel Files"

# PROP Default_Filter ".xls, .csv"
# Begin Source File

SOURCE=.\ks\beachdata.xls
# End Source File
# Begin Source File

SOURCE=.\ks\boarddata.xls
# End Source File
# Begin Source File

SOURCE=.\ks\careerdata.xls
# End Source File
# Begin Source File

SOURCE=.\ks\globaltext.xls
# End Source File
# Begin Source File

SOURCE=.\ks\SoundData.xls
# End Source File
# Begin Source File

SOURCE=.\ks\surferdata.xls
# End Source File
# Begin Source File

SOURCE=.\ks\trickdata.xls
# End Source File
# Begin Source File

SOURCE=.\ks\wavedata.xls
# End Source File
# End Group
# Begin Group "AnimHeaders"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ks\andersen_ind_anims_mac.h
# End Source File
# Begin Source File

SOURCE=.\ks\board_anims_mac.h
# End Source File
# Begin Source File

SOURCE=.\ks\carrol_ind_anims_mac.h
# End Source File
# Begin Source File

SOURCE=.\ks\curren_ind_anims_mac.h
# End Source File
# Begin Source File

SOURCE=.\ks\fletcher_ind_anims_mac.h
# End Source File
# Begin Source File

SOURCE=.\ks\frankenreiter_ind_anims_mac.h
# End Source File
# Begin Source File

SOURCE=.\ks\hawk_ind_anims_mac.h
# End Source File
# Begin Source File

SOURCE=.\ks\irons_ind_anims_mac.h
# End Source File
# Begin Source File

SOURCE=.\ks\kellyslater_shared_anims_mac.h
# End Source File
# Begin Source File

SOURCE=.\ks\kennelly_ind_anims_mac.h
# End Source File
# Begin Source File

SOURCE=.\ks\machado_ind_anims_mac.h
# End Source File
# Begin Source File

SOURCE=.\ks\pastrana_ind_anims_mac.h
# End Source File
# Begin Source File

SOURCE=.\ks\personalityks_ind_anims_mac.h
# End Source File
# Begin Source File

SOURCE=.\ks\robb_ind_anims_mac.h
# End Source File
# Begin Source File

SOURCE=.\ks\silver_ind_anims_mac.h
# End Source File
# Begin Source File

SOURCE=.\ks\slater_ind_anims_mac.h
# End Source File
# End Group
# Begin Group "MetaSource files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\files_ai.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

DEP_CPP_FILES=\
	"..\..\ngl\ps2\ngl_fixedstr.h"\
	"..\..\ngl\ps2\ngl_ps2.h"\
	"..\..\ngl\ps2\tim2.h"\
	"..\..\NSL\common\nl.h"\
	"..\..\NSL\common\nsl.h"\
	"..\..\NSL\common\ProjectOptions.h"\
	"..\..\NSL\gamecube\nl_gc.h"\
	"..\..\NSL\ps2\fifo_queue.cpp"\
	"..\..\NSL\ps2\fifo_queue.h"\
	"..\..\NSL\PS2\gas.h"\
	"..\..\NSL\PS2\gas_utility.h"\
	"..\..\NSL\PS2\nl_ps2.h"\
	"..\..\NSL\PS2\nsl_ps2.h"\
	"..\..\NSL\XBOX\nl_xbox.h"\
	"..\..\NSL\XBOX\nsl_xbox.h"\
	"..\..\NSL\XBOX\nsl_xbox_ext.h"\
	"..\..\NSL\XBOX\nsl_xbox_source.h"\
	"..\..\nsl\xbox\nsl_xbox_streams.h"\
	"..\..\NVL\ps2\nvl_ps2.h"\
	"..\..\NVL\ps2\nvlMPEG_ps2.h"\
	"..\..\NVL\ps2\nvlstream_ps2.h"\
	".\aggvertbuf.h"\
	".\ai_actions.cpp"\
	".\ai_actions.h"\
	".\ai_constants.h"\
	".\ai_goals.cpp"\
	".\ai_goals.h"\
	".\ai_goals_mac.h"\
	".\ai_interface.cpp"\
	".\ai_interface.h"\
	".\ai_locomotion.cpp"\
	".\ai_locomotion.h"\
	".\ai_locomotion_direct.cpp"\
	".\ai_locomotion_direct.h"\
	".\ai_polypath.cpp"\
	".\ai_polypath.h"\
	".\ai_polypath_cell.cpp"\
	".\ai_polypath_cell.h"\
	".\ai_polypath_heap.h"\
	".\ai_voice.cpp"\
	".\ai_voice.h"\
	".\AIController.cpp"\
	".\AIController.h"\
	".\algebra.h"\
	".\anim.h"\
	".\anim_flavor.h"\
	".\anim_flavors.h"\
	".\anim_ids.h"\
	".\animation_interface.h"\
	".\app.h"\
	".\archalloc.h"\
	".\attribute_template.h"\
	".\avltree.h"\
	".\b_spline.h"\
	".\beam.h"\
	".\beam_signals.h"\
	".\billboard.h"\
	".\bone.h"\
	".\bound.h"\
	".\box_trigger_interface.h"\
	".\camera.h"\
	".\capsule.h"\
	".\cface.h"\
	".\character_hard_attribs.h"\
	".\chunkfile.h"\
	".\colgeom.h"\
	".\collide.h"\
	".\colmesh.h"\
	".\color.h"\
	".\commands.h"\
	".\conglom.h"\
	".\constants.h"\
	".\controller.h"\
	".\convex_box.h"\
	".\custom_stl.h"\
	".\debug.h"\
	".\debug_render.h"\
	".\devoptflags.h"\
	".\devoptints.h"\
	".\devoptstrs.h"\
	".\entflavor.h"\
	".\entity.h"\
	".\entity_anim.h"\
	".\entity_hard_attribs.h"\
	".\entity_interface.h"\
	".\entity_maker.h"\
	".\entity_signals.h"\
	".\entityflags.h"\
	".\entityid.h"\
	".\errorcontext.h"\
	".\face.h"\
	".\faceflags.h"\
	".\fast_vector.h"\
	".\file.h"\
	".\file_finder.h"\
	".\filespec.h"\
	".\frame_info.h"\
	".\game.h"\
	".\game_info_vars.h"\
	".\game_process.h"\
	".\gcglobals.h"\
	".\generator.h"\
	".\geomgr.h"\
	".\global.h"\
	".\global_signals.h"\
	".\graph.h"\
	".\guidance_sys.h"\
	".\hard_attrib_interface.h"\
	".\hull.h"\
	".\hwaudio.h"\
	".\hwmath.h"\
	".\hwmovieplayer.h"\
	".\hwrasterize.h"\
	".\hyperplane.h"\
	".\ini_parser.h"\
	".\inputmgr.h"\
	".\instance.h"\
	".\item.h"\
	".\item_signals.h"\
	".\joystick.h"\
	".\ks\kellyslater_ai_goals.h"\
	".\ks\ksheaps.h"\
	".\ks\wipeoutdata.h"\
	".\kshooks.h"\
	".\ksnsl.h"\
	".\ksnvl.h"\
	".\light.h"\
	".\linear_anim.h"\
	".\link_interface.h"\
	".\map_e.h"\
	".\material.h"\
	".\matfac.h"\
	".\maxskinbones.h"\
	".\meshrefs.h"\
	".\mic.h"\
	".\mobject.h"\
	".\mustash.h"\
	".\ngl.h"\
	".\osalloc.h"\
	".\osassert.h"\
	".\osdevopts.h"\
	".\oserrmsg.h"\
	".\osfile.h"\
	".\ostimer.h"\
	".\path.h"\
	".\pc_port.h"\
	".\physical_interface.h"\
	".\plane.h"\
	".\platform_defines.h"\
	".\pmesh.h"\
	".\po.h"\
	".\po_anim.h"\
	".\polytube.h"\
	".\portal.h"\
	".\profiler.h"\
	".\projconst.h"\
	".\project.h"\
	".\pstring.h"\
	".\random.h"\
	".\rect.h"\
	".\refptr.h"\
	".\region.h"\
	".\region_graph.h"\
	".\render_data.h"\
	".\renderflav.h"\
	".\scene_anim.h"\
	".\script_lib_mfg.h"\
	".\script_library_class.h"\
	".\script_mfg_signals.h"\
	".\script_object.h"\
	".\signal_anim.h"\
	".\signals.h"\
	".\simple_classes.h"\
	".\SimpleAssert.h"\
	".\singleton.h"\
	".\so_data_block.h"\
	".\sound_group.h"\
	".\sound_interface.h"\
	".\sphere.h"\
	".\stashes.h"\
	".\staticmem.h"\
	".\stl_adapter.h"\
	".\stringx.h"\
	".\terrain.h"\
	".\text_font.h"\
	".\textfile.h"\
	".\time_interface.h"\
	".\timer.h"\
	".\txtcoord.h"\
	".\types.h"\
	".\usefulmath.h"\
	".\users.h"\
	".\vert.h"\
	".\visrep.h"\
	".\vm_executable.h"\
	".\vm_symbol.h"\
	".\vm_symbol_list.h"\
	".\warnlvl.h"\
	".\wds.h"\
	".\wedge.h"\
	".\xbglobals.h"\
	{$(INCLUDE)}"eekernel.h"\
	{$(INCLUDE)}"eeregs.h"\
	{$(INCLUDE)}"eestruct.h"\
	{$(INCLUDE)}"eetypes.h"\
	{$(INCLUDE)}"hwosgc\gc_algebra.h"\
	{$(INCLUDE)}"hwosgc\gc_alloc.h"\
	{$(INCLUDE)}"hwosgc\gc_audio.h"\
	{$(INCLUDE)}"hwosgc\gc_errmsg.h"\
	{$(INCLUDE)}"hwosgc\gc_file.h"\
	{$(INCLUDE)}"HWOSGC\gc_gamesaver.h"\
	{$(INCLUDE)}"hwosgc\gc_graphics.h"\
	{$(INCLUDE)}"hwosgc\gc_input.h"\
	{$(INCLUDE)}"hwosgc\gc_math.h"\
	{$(INCLUDE)}"hwosgc\gc_movieplayer.h"\
	{$(INCLUDE)}"hwosgc\gc_rasterize.h"\
	{$(INCLUDE)}"hwosgc\gc_texturemgr.h"\
	{$(INCLUDE)}"hwosgc\gc_timer.h"\
	{$(INCLUDE)}"hwosgc\ngl_fixedstr.h"\
	{$(INCLUDE)}"hwosgc\ngl_gc.h"\
	{$(INCLUDE)}"hwosgc\ngl_version.h"\
	{$(INCLUDE)}"hwosgc\nvl_gc.h"\
	{$(INCLUDE)}"HWOSPS2\gas.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_algebra.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_alloc.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_audio.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_devopts.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_errmsg.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_file.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_gamesaver.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_graphics.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_input.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_m2vplayer.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_math.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_movieplayer.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_rasterize.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_texturemgr.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_timer.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_types.h"\
	{$(INCLUDE)}"hwosxb\pstring.h"\
	{$(INCLUDE)}"hwosxb\xb_algebra.h"\
	{$(INCLUDE)}"hwosxb\xb_alloc.h"\
	{$(INCLUDE)}"hwosxb\xb_audio.h"\
	{$(INCLUDE)}"hwosxb\xb_errmsg.h"\
	{$(INCLUDE)}"hwosxb\xb_file.h"\
	{$(INCLUDE)}"HWOSXB\xb_gamesaver.h"\
	{$(INCLUDE)}"hwosxb\xb_graphics.h"\
	{$(INCLUDE)}"hwosxb\xb_input.h"\
	{$(INCLUDE)}"hwosxb\xb_math.h"\
	{$(INCLUDE)}"hwosxb\xb_movieplayer.h"\
	{$(INCLUDE)}"HWOSXB\xb_portDVDCache.h"\
	{$(INCLUDE)}"hwosxb\xb_rasterize.h"\
	{$(INCLUDE)}"hwosxb\xb_string.h"\
	{$(INCLUDE)}"hwosxb\xb_texturemgr.h"\
	{$(INCLUDE)}"hwosxb\xb_timer.h"\
	{$(INCLUDE)}"hwosxb\xb_types.h"\
	{$(INCLUDE)}"ks\AccompFrontEnd.h"\
	{$(INCLUDE)}"ks\andersen_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\beach.h"\
	{$(INCLUDE)}"ks\beachdata.h"\
	{$(INCLUDE)}"ks\BeachFrontEnd.h"\
	{$(INCLUDE)}"ks\board.h"\
	{$(INCLUDE)}"ks\board_anims_mac.h"\
	{$(INCLUDE)}"ks\boarddata.h"\
	{$(INCLUDE)}"ks\BoardFrontEnd.h"\
	{$(INCLUDE)}"ks\career.h"\
	{$(INCLUDE)}"ks\careerdata.h"\
	{$(INCLUDE)}"ks\carrol_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\challenge.h"\
	{$(INCLUDE)}"ks\challenge_icon.h"\
	{$(INCLUDE)}"ks\challenge_photo.h"\
	{$(INCLUDE)}"ks\cheat.h"\
	{$(INCLUDE)}"ks\CheatFrontEnd.h"\
	{$(INCLUDE)}"ks\compressedphoto.h"\
	{$(INCLUDE)}"ks\coords.h"\
	{$(INCLUDE)}"ks\curren_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\eventmanager.h"\
	{$(INCLUDE)}"ks\ExtrasFrontEnd.h"\
	{$(INCLUDE)}"ks\FEAnim.h"\
	{$(INCLUDE)}"ks\FEEntityManager.h"\
	{$(INCLUDE)}"ks\FEMenu.h"\
	{$(INCLUDE)}"ks\FEPanel.h"\
	{$(INCLUDE)}"ks\fletcher_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\floatobj.h"\
	{$(INCLUDE)}"ks\frankenreiter_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\FrontEndManager.h"\
	{$(INCLUDE)}"ks\FrontEndMenus.h"\
	{$(INCLUDE)}"ks\GameData.h"\
	{$(INCLUDE)}"ks\GlobalData.h"\
	{$(INCLUDE)}"ks\globaltextenum.h"\
	{$(INCLUDE)}"ks\GraphicalMenuSystem.h"\
	{$(INCLUDE)}"ks\HighScoreFrontEnd.h"\
	{$(INCLUDE)}"ks\igo_widget.h"\
	{$(INCLUDE)}"ks\igo_widget_analogclock.h"\
	{$(INCLUDE)}"ks\igo_widget_balance.h"\
	{$(INCLUDE)}"ks\igo_widget_camera.h"\
	{$(INCLUDE)}"ks\igo_widget_fanmeter.h"\
	{$(INCLUDE)}"ks\igo_widget_grid.h"\
	{$(INCLUDE)}"ks\igo_widget_iconcount.h"\
	{$(INCLUDE)}"ks\igo_widget_iconradar.h"\
	{$(INCLUDE)}"ks\igo_widget_meterchallenge.h"\
	{$(INCLUDE)}"ks\igo_widget_objectalert.h"\
	{$(INCLUDE)}"ks\igo_widget_photo.h"\
	{$(INCLUDE)}"ks\igo_widget_replay.h"\
	{$(INCLUDE)}"ks\igo_widget_simple.h"\
	{$(INCLUDE)}"ks\igo_widget_skillchallenge.h"\
	{$(INCLUDE)}"ks\igo_widget_specialmeter.h"\
	{$(INCLUDE)}"ks\igo_widget_splitclock.h"\
	{$(INCLUDE)}"ks\igo_widget_splitmeter.h"\
	{$(INCLUDE)}"ks\igo_widget_splitscore.h"\
	{$(INCLUDE)}"ks\igo_widget_splitter.h"\
	{$(INCLUDE)}"ks\igo_widget_timeattack.h"\
	{$(INCLUDE)}"ks\igo_widget_waveindicator.h"\
	{$(INCLUDE)}"ks\IGOFrontEnd.h"\
	{$(INCLUDE)}"ks\igohintmanager.h"\
	{$(INCLUDE)}"ks\igoiconmanager.h"\
	{$(INCLUDE)}"ks\igolearn_new_trickmanager.h"\
	{$(INCLUDE)}"ks\ik_object.h"\
	{$(INCLUDE)}"ks\irons_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\judge.h"\
	{$(INCLUDE)}"ks\kellyslater_controller.h"\
	{$(INCLUDE)}"ks\kellyslater_shared_anims_mac.h"\
	{$(INCLUDE)}"ks\kellyslater_states_mac.h"\
	{$(INCLUDE)}"ks\ks_camera.h"\
	{$(INCLUDE)}"ks\ksdbmenu.h"\
	{$(INCLUDE)}"ks\ksngl.h"\
	{$(INCLUDE)}"ks\ksreplay.h"\
	{$(INCLUDE)}"ks\LogbookFrontEnd.h"\
	{$(INCLUDE)}"ks\machado_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\MainFrontEnd.h"\
	{$(INCLUDE)}"ks\Map.h"\
	{$(INCLUDE)}"ks\MCDetectFrontEnd.h"\
	{$(INCLUDE)}"ks\menu.h"\
	{$(INCLUDE)}"ks\menusys.h"\
	{$(INCLUDE)}"ks\mode.h"\
	{$(INCLUDE)}"ks\mode_headtohead.h"\
	{$(INCLUDE)}"ks\mode_meterattack.h"\
	{$(INCLUDE)}"ks\mode_push.h"\
	{$(INCLUDE)}"ks\mode_timeattack.h"\
	{$(INCLUDE)}"ks\MultiFrontEnd.h"\
	{$(INCLUDE)}"ks\MusicMan.h"\
	{$(INCLUDE)}"ks\ode.h"\
	{$(INCLUDE)}"ks\osGameSaver.h"\
	{$(INCLUDE)}"ks\PAL60FrontEnd.h"\
	{$(INCLUDE)}"ks\PathData.h"\
	{$(INCLUDE)}"ks\PhotoFrontEnd.h"\
	{$(INCLUDE)}"ks\physics.h"\
	{$(INCLUDE)}"ks\player.h"\
	{$(INCLUDE)}"ks\replay.h"\
	{$(INCLUDE)}"ks\robb_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\rumbleManager.h"\
	{$(INCLUDE)}"ks\SaveLoadFrontEnd.h"\
	{$(INCLUDE)}"ks\scoringmanager.h"\
	{$(INCLUDE)}"ks\slater_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\sounddata.h"\
	{$(INCLUDE)}"ks\SoundScript.h"\
	{$(INCLUDE)}"ks\specialmeter.h"\
	{$(INCLUDE)}"ks\StatsFrontEnd.h"\
	{$(INCLUDE)}"ks\surferdata.h"\
	{$(INCLUDE)}"ks\SurferFrontEnd.h"\
	{$(INCLUDE)}"ks\trick_system.h"\
	{$(INCLUDE)}"ks\TrickBookFrontEnd.h"\
	{$(INCLUDE)}"ks\trickdata.h"\
	{$(INCLUDE)}"ks\trickregion.h"\
	{$(INCLUDE)}"ks\tricks.h"\
	{$(INCLUDE)}"ks\tutorialdata.h"\
	{$(INCLUDE)}"ks\TutorialFrontEnd.h"\
	{$(INCLUDE)}"ks\tutorialmanager.h"\
	{$(INCLUDE)}"ks\underwtr.h"\
	{$(INCLUDE)}"ks\VOEngine.h"\
	{$(INCLUDE)}"ks\wave.h"\
	{$(INCLUDE)}"ks\wavebreak.txt"\
	{$(INCLUDE)}"ks\wavebreakstage.txt"\
	{$(INCLUDE)}"ks\waveemitter.txt"\
	{$(INCLUDE)}"ks\waveenum.h"\
	{$(INCLUDE)}"ks\wavemarker.txt"\
	{$(INCLUDE)}"ks\waveregion.txt"\
	{$(INCLUDE)}"ks\wavestage.txt"\
	{$(INCLUDE)}"libcdvd.h"\
	{$(INCLUDE)}"libdma.h"\
	{$(INCLUDE)}"libgraph.h"\
	{$(INCLUDE)}"libipu.h"\
	{$(INCLUDE)}"libmc.h"\
	{$(INCLUDE)}"libmpeg.h"\
	{$(INCLUDE)}"libpad.h"\
	{$(INCLUDE)}"libpc.h"\
	{$(INCLUDE)}"libusbkb.h"\
	{$(INCLUDE)}"libvu0.h"\
	{$(INCLUDE)}"sifdev.h"\
	{$(INCLUDE)}"xgraphics.h"\
	
NODEP_CPP_FILES=\
	"..\..\NSL\pc\nl_pc.h"\
	"..\..\NSL\pc\nsl_pc_ext.h"\
	".\algo.h"\
	".\defalloc.h"\
	".\gc_arammgr.h"\
	".\hwosmks\am_audio.h"\
	".\hwosmks\mks_devopts.h"\
	".\hwosmks\mks_errmsg.h"\
	".\hwosmks\mks_movieplayer.h"\
	".\hwosmks\set5_rasterize.h"\
	".\hwosmks\set5_timer.h"\
	".\hwosmks\sh4_math.h"\
	".\hwosmks\sy_alloc.h"\
	".\hwosmks\sy_file.h"\
	".\hwosnglpc\algebra.h"\
	".\hwosnglpc\pc_algebra.h"\
	".\hwosnull\null_alloc.h"\
	".\hwosnull\null_audio.h"\
	".\hwosnull\null_devopts.h"\
	".\hwosnull\null_errmsg.h"\
	".\hwosnull\null_file.h"\
	".\hwosnull\null_math.h"\
	".\hwosnull\null_movieplayer.h"\
	".\hwosnull\null_rasterize.h"\
	".\hwosnull\null_timer.h"\
	".\hwospc\d3d_rasterize.h"\
	".\hwospc\ds_audio.h"\
	".\hwospc\pc_algebra.h"\
	".\hwospc\pc_movieplayer.h"\
	".\hwospc\w32_alloc.h"\
	".\hwospc\w32_devopts.h"\
	".\hwospc\w32_errmsg.h"\
	".\hwospc\w32_file.h"\
	".\hwospc\w32_timer.h"\
	".\hwospc\x86_math.h"\
	".\hwosxb\ngl_xbox.h"\
	".\hwosxb\nvl_xbox.h"\
	".\LipSync_Anim.h"\
	".\ngl_xbox.h"\
	".\nvl_gc.h"\
	".\nvl_xbox.h"\
	".\nvlstream_gc.h"\
	".\stl_heap.h"\
	
# ADD BASE CPP /Yu
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

DEP_CPP_FILES=\
	"..\..\ngl\ps2\ngl_fixedstr.h"\
	"..\..\ngl\ps2\ngl_ps2.h"\
	"..\..\ngl\ps2\tim2.h"\
	"..\..\NSL\common\nl.h"\
	"..\..\NSL\common\nsl.h"\
	"..\..\NSL\common\ProjectOptions.h"\
	"..\..\NSL\gamecube\nl_gc.h"\
	"..\..\NSL\ps2\fifo_queue.cpp"\
	"..\..\NSL\ps2\fifo_queue.h"\
	"..\..\NSL\PS2\gas.h"\
	"..\..\NSL\PS2\gas_utility.h"\
	"..\..\NSL\PS2\nl_ps2.h"\
	"..\..\NSL\PS2\nsl_ps2.h"\
	"..\..\NSL\XBOX\nl_xbox.h"\
	"..\..\NSL\XBOX\nsl_xbox.h"\
	"..\..\NSL\XBOX\nsl_xbox_ext.h"\
	"..\..\NSL\XBOX\nsl_xbox_source.h"\
	"..\..\nsl\xbox\nsl_xbox_streams.h"\
	"..\..\NVL\ps2\nvl_ps2.h"\
	"..\..\NVL\ps2\nvlMPEG_ps2.h"\
	"..\..\NVL\ps2\nvlstream_ps2.h"\
	".\aggvertbuf.h"\
	".\ai_actions.cpp"\
	".\ai_actions.h"\
	".\ai_constants.h"\
	".\ai_goals.cpp"\
	".\ai_goals.h"\
	".\ai_goals_mac.h"\
	".\ai_interface.cpp"\
	".\ai_interface.h"\
	".\ai_locomotion.cpp"\
	".\ai_locomotion.h"\
	".\ai_locomotion_direct.cpp"\
	".\ai_locomotion_direct.h"\
	".\ai_polypath.cpp"\
	".\ai_polypath.h"\
	".\ai_polypath_cell.cpp"\
	".\ai_polypath_cell.h"\
	".\ai_polypath_heap.h"\
	".\ai_voice.cpp"\
	".\ai_voice.h"\
	".\AIController.cpp"\
	".\AIController.h"\
	".\algebra.h"\
	".\anim.h"\
	".\anim_flavor.h"\
	".\anim_flavors.h"\
	".\anim_ids.h"\
	".\animation_interface.h"\
	".\app.h"\
	".\archalloc.h"\
	".\attribute_template.h"\
	".\avltree.h"\
	".\b_spline.h"\
	".\beam.h"\
	".\beam_signals.h"\
	".\billboard.h"\
	".\bone.h"\
	".\bound.h"\
	".\box_trigger_interface.h"\
	".\camera.h"\
	".\capsule.h"\
	".\cface.h"\
	".\character_hard_attribs.h"\
	".\chunkfile.h"\
	".\colgeom.h"\
	".\collide.h"\
	".\colmesh.h"\
	".\color.h"\
	".\commands.h"\
	".\conglom.h"\
	".\constants.h"\
	".\controller.h"\
	".\convex_box.h"\
	".\custom_stl.h"\
	".\debug.h"\
	".\debug_render.h"\
	".\devoptflags.h"\
	".\devoptints.h"\
	".\devoptstrs.h"\
	".\entflavor.h"\
	".\entity.h"\
	".\entity_anim.h"\
	".\entity_hard_attribs.h"\
	".\entity_interface.h"\
	".\entity_maker.h"\
	".\entity_signals.h"\
	".\entityflags.h"\
	".\entityid.h"\
	".\errorcontext.h"\
	".\face.h"\
	".\faceflags.h"\
	".\fast_vector.h"\
	".\file.h"\
	".\file_finder.h"\
	".\filespec.h"\
	".\frame_info.h"\
	".\game.h"\
	".\game_info_vars.h"\
	".\game_process.h"\
	".\gcglobals.h"\
	".\generator.h"\
	".\geomgr.h"\
	".\global.h"\
	".\global_signals.h"\
	".\graph.h"\
	".\guidance_sys.h"\
	".\hard_attrib_interface.h"\
	".\hull.h"\
	".\hwaudio.h"\
	".\hwmath.h"\
	".\hwmovieplayer.h"\
	".\hwrasterize.h"\
	".\hyperplane.h"\
	".\ini_parser.h"\
	".\inputmgr.h"\
	".\instance.h"\
	".\item.h"\
	".\item_signals.h"\
	".\joystick.h"\
	".\ks\kellyslater_ai_goals.h"\
	".\ks\ksheaps.h"\
	".\ks\wipeoutdata.h"\
	".\kshooks.h"\
	".\ksnsl.h"\
	".\ksnvl.h"\
	".\light.h"\
	".\linear_anim.h"\
	".\link_interface.h"\
	".\map_e.h"\
	".\material.h"\
	".\matfac.h"\
	".\maxskinbones.h"\
	".\meshrefs.h"\
	".\mic.h"\
	".\mobject.h"\
	".\mustash.h"\
	".\ngl.h"\
	".\osalloc.h"\
	".\osassert.h"\
	".\osdevopts.h"\
	".\oserrmsg.h"\
	".\osfile.h"\
	".\ostimer.h"\
	".\path.h"\
	".\pc_port.h"\
	".\physical_interface.h"\
	".\plane.h"\
	".\platform_defines.h"\
	".\pmesh.h"\
	".\po.h"\
	".\po_anim.h"\
	".\polytube.h"\
	".\portal.h"\
	".\profiler.h"\
	".\projconst.h"\
	".\project.h"\
	".\pstring.h"\
	".\random.h"\
	".\rect.h"\
	".\refptr.h"\
	".\region.h"\
	".\region_graph.h"\
	".\render_data.h"\
	".\renderflav.h"\
	".\scene_anim.h"\
	".\script_lib_mfg.h"\
	".\script_library_class.h"\
	".\script_mfg_signals.h"\
	".\script_object.h"\
	".\signal_anim.h"\
	".\signals.h"\
	".\simple_classes.h"\
	".\SimpleAssert.h"\
	".\singleton.h"\
	".\so_data_block.h"\
	".\sound_group.h"\
	".\sound_interface.h"\
	".\sphere.h"\
	".\stashes.h"\
	".\staticmem.h"\
	".\stl_adapter.h"\
	".\stringx.h"\
	".\terrain.h"\
	".\text_font.h"\
	".\textfile.h"\
	".\time_interface.h"\
	".\timer.h"\
	".\txtcoord.h"\
	".\types.h"\
	".\usefulmath.h"\
	".\users.h"\
	".\vert.h"\
	".\visrep.h"\
	".\vm_executable.h"\
	".\vm_symbol.h"\
	".\vm_symbol_list.h"\
	".\warnlvl.h"\
	".\wds.h"\
	".\wedge.h"\
	".\xbglobals.h"\
	
NODEP_CPP_FILES=\
	"..\..\NSL\pc\nl_pc.h"\
	"..\..\NSL\pc\nsl_pc_ext.h"\
	"..\..\NSL\XBOX\nsl.h"\
	".\algo.h"\
	".\defalloc.h"\
	".\ekernel.h"\
	".\eregs.h"\
	".\estruct.h"\
	".\etypes.h"\
	".\gc_arammgr.h"\
	".\graphics.h"\
	".\hwosgc\nsl.h"\
	".\hwosmks\am_audio.h"\
	".\hwosmks\mks_devopts.h"\
	".\hwosmks\mks_errmsg.h"\
	".\hwosmks\mks_movieplayer.h"\
	".\hwosmks\set5_rasterize.h"\
	".\hwosmks\set5_timer.h"\
	".\hwosmks\sh4_math.h"\
	".\hwosmks\sy_alloc.h"\
	".\hwosmks\sy_file.h"\
	".\hwosnglpc\algebra.h"\
	".\hwosnglpc\pc_algebra.h"\
	".\hwosnull\null_alloc.h"\
	".\hwosnull\null_audio.h"\
	".\hwosnull\null_devopts.h"\
	".\hwosnull\null_errmsg.h"\
	".\hwosnull\null_file.h"\
	".\hwosnull\null_math.h"\
	".\hwosnull\null_movieplayer.h"\
	".\hwosnull\null_rasterize.h"\
	".\hwosnull\null_timer.h"\
	".\hwospc\d3d_rasterize.h"\
	".\hwospc\ds_audio.h"\
	".\hwospc\pc_algebra.h"\
	".\hwospc\pc_movieplayer.h"\
	".\hwospc\w32_alloc.h"\
	".\hwospc\w32_devopts.h"\
	".\hwospc\w32_errmsg.h"\
	".\hwospc\w32_file.h"\
	".\hwospc\w32_timer.h"\
	".\hwospc\x86_math.h"\
	".\hwosxb\ngl_xbox.h"\
	".\hwosxb\nvl_xbox.h"\
	".\ibcdvd.h"\
	".\ibdma.h"\
	".\ibgraph.h"\
	".\ibipu.h"\
	".\ibmc.h"\
	".\ibmpeg.h"\
	".\ibpad.h"\
	".\ibpc.h"\
	".\ibusbkb.h"\
	".\ibvu0.h"\
	".\ifdev.h"\
	".\LipSync_Anim.h"\
	".\ngl_xbox.h"\
	".\nvl_gc.h"\
	".\nvl_xbox.h"\
	".\nvlstream_gc.h"\
	".\s\AccompFrontEnd.h"\
	".\s\andersen_ind_anims_mac.h"\
	".\s\beach.h"\
	".\s\beachdata.h"\
	".\s\BeachFrontEnd.h"\
	".\s\board.h"\
	".\s\board_anims_mac.h"\
	".\s\boarddata.h"\
	".\s\BoardFrontEnd.h"\
	".\s\career.h"\
	".\s\careerdata.h"\
	".\s\carrol_ind_anims_mac.h"\
	".\s\challenge.h"\
	".\s\challenge_icon.h"\
	".\s\challenge_photo.h"\
	".\s\cheat.h"\
	".\s\CheatFrontEnd.h"\
	".\s\compressedphoto.h"\
	".\s\coords.h"\
	".\s\curren_ind_anims_mac.h"\
	".\s\eventmanager.h"\
	".\s\ExtrasFrontEnd.h"\
	".\s\FEAnim.h"\
	".\s\FEEntityManager.h"\
	".\s\FEMenu.h"\
	".\s\FEPanel.h"\
	".\s\fletcher_ind_anims_mac.h"\
	".\s\floatobj.h"\
	".\s\frankenreiter_ind_anims_mac.h"\
	".\s\FrontEndManager.h"\
	".\s\FrontEndMenus.h"\
	".\s\GameData.h"\
	".\s\GlobalData.h"\
	".\s\globaltextenum.h"\
	".\s\GraphicalMenuSystem.h"\
	".\s\HighScoreFrontEnd.h"\
	".\s\igo_widget.h"\
	".\s\igo_widget_analogclock.h"\
	".\s\igo_widget_balance.h"\
	".\s\igo_widget_camera.h"\
	".\s\igo_widget_fanmeter.h"\
	".\s\igo_widget_iconradar.h"\
	".\s\igo_widget_photo.h"\
	".\s\igo_widget_replay.h"\
	".\s\igo_widget_simple.h"\
	".\s\igo_widget_specialmeter.h"\
	".\s\igo_widget_splitclock.h"\
	".\s\igo_widget_splitmeter.h"\
	".\s\igo_widget_splitscore.h"\
	".\s\igo_widget_splitter.h"\
	".\s\igo_widget_timeattack.h"\
	".\s\igo_widget_waveindicator.h"\
	".\s\IGOFrontEnd.h"\
	".\s\igohintmanager.h"\
	".\s\igoiconmanager.h"\
	".\s\igolearn_new_trickmanager.h"\
	".\s\ik_object.h"\
	".\s\irons_ind_anims_mac.h"\
	".\s\judge.h"\
	".\s\kellyslater_controller.h"\
	".\s\kellyslater_shared_anims_mac.h"\
	".\s\kellyslater_states_mac.h"\
	".\s\ks_camera.h"\
	".\s\ksdbmenu.h"\
	".\s\ksngl.h"\
	".\s\ksreplay.h"\
	".\s\machado_ind_anims_mac.h"\
	".\s\MainFrontEnd.h"\
	".\s\Map.h"\
	".\s\MCDetectFrontEnd.h"\
	".\s\menu.h"\
	".\s\menusys.h"\
	".\s\mode.h"\
	".\s\mode_headtohead.h"\
	".\s\mode_meterattack.h"\
	".\s\mode_push.h"\
	".\s\mode_timeattack.h"\
	".\s\MultiFrontEnd.h"\
	".\s\MusicMan.h"\
	".\s\ode.h"\
	".\s\osGameSaver.h"\
	".\s\PAL60FrontEnd.h"\
	".\s\PathData.h"\
	".\s\PhotoFrontEnd.h"\
	".\s\physics.h"\
	".\s\player.h"\
	".\s\replay.h"\
	".\s\robb_ind_anims_mac.h"\
	".\s\rumbleManager.h"\
	".\s\SaveLoadFrontEnd.h"\
	".\s\scoringmanager.h"\
	".\s\slater_ind_anims_mac.h"\
	".\s\sounddata.h"\
	".\s\SoundScript.h"\
	".\s\specialmeter.h"\
	".\s\StatsFrontEnd.h"\
	".\s\surferdata.h"\
	".\s\SurferFrontEnd.h"\
	".\s\trick_system.h"\
	".\s\TrickBookFrontEnd.h"\
	".\s\trickdata.h"\
	".\s\trickregion.h"\
	".\s\tricks.h"\
	".\s\TutorialFrontEnd.h"\
	".\s\tutorialmanager.h"\
	".\s\VOEngine.h"\
	".\s\wave.h"\
	".\s\wavebreak.txt"\
	".\s\wavebreakstage.txt"\
	".\s\waveemitter.txt"\
	".\s\waveenum.h"\
	".\s\wavemarker.txt"\
	".\s\waveregion.txt"\
	".\s\wavestage.txt"\
	".\stl_heap.h"\
	".\wosgc\gc_algebra.h"\
	".\wosgc\gc_alloc.h"\
	".\wosgc\gc_audio.h"\
	".\wosgc\gc_errmsg.h"\
	".\wosgc\gc_file.h"\
	".\WOSGC\gc_gamesaver.h"\
	".\wosgc\gc_graphics.h"\
	".\wosgc\gc_input.h"\
	".\wosgc\gc_math.h"\
	".\wosgc\gc_movieplayer.h"\
	".\wosgc\gc_rasterize.h"\
	".\wosgc\gc_texturemgr.h"\
	".\wosgc\gc_timer.h"\
	".\wosgc\ngl_fixedstr.h"\
	".\wosgc\ngl_gc.h"\
	".\wosgc\ngl_version.h"\
	".\wosgc\nvl_gc.h"\
	".\WOSPS2\gas.h"\
	".\WOSPS2\ps2_algebra.h"\
	".\WOSPS2\ps2_alloc.h"\
	".\WOSPS2\ps2_audio.h"\
	".\WOSPS2\ps2_devopts.h"\
	".\WOSPS2\ps2_errmsg.h"\
	".\WOSPS2\ps2_file.h"\
	".\WOSPS2\ps2_gamesaver.h"\
	".\WOSPS2\ps2_graphics.h"\
	".\WOSPS2\ps2_input.h"\
	".\WOSPS2\ps2_m2vplayer.h"\
	".\WOSPS2\ps2_math.h"\
	".\WOSPS2\ps2_movieplayer.h"\
	".\WOSPS2\ps2_rasterize.h"\
	".\WOSPS2\ps2_texturemgr.h"\
	".\WOSPS2\ps2_timer.h"\
	".\WOSPS2\ps2_types.h"\
	".\wosxb\pstring.h"\
	".\wosxb\xb_algebra.h"\
	".\wosxb\xb_alloc.h"\
	".\wosxb\xb_audio.h"\
	".\wosxb\xb_errmsg.h"\
	".\wosxb\xb_file.h"\
	".\WOSXB\xb_gamesaver.h"\
	".\wosxb\xb_graphics.h"\
	".\wosxb\xb_input.h"\
	".\wosxb\xb_math.h"\
	".\wosxb\xb_movieplayer.h"\
	".\WOSXB\xb_portDVDCache.h"\
	".\wosxb\xb_rasterize.h"\
	".\wosxb\xb_string.h"\
	".\wosxb\xb_texturemgr.h"\
	".\wosxb\xb_timer.h"\
	".\wosxb\xb_types.h"\
	
# SUBTRACT BASE CPP /YX /Yc /Yu
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

DEP_CPP_FILES=\
	"..\..\ngl\ps2\ngl_fixedstr.h"\
	"..\..\ngl\ps2\ngl_ps2.h"\
	"..\..\ngl\ps2\tim2.h"\
	"..\..\NSL\common\nl.h"\
	"..\..\NSL\common\nsl.h"\
	"..\..\NSL\common\ProjectOptions.h"\
	"..\..\NSL\gamecube\nl_gc.h"\
	"..\..\NSL\ps2\fifo_queue.cpp"\
	"..\..\NSL\ps2\fifo_queue.h"\
	"..\..\NSL\PS2\gas.h"\
	"..\..\NSL\PS2\gas_utility.h"\
	"..\..\NSL\PS2\nl_ps2.h"\
	"..\..\NSL\PS2\nsl_ps2.h"\
	"..\..\NSL\XBOX\nl_xbox.h"\
	"..\..\NSL\XBOX\nsl_xbox.h"\
	"..\..\NSL\XBOX\nsl_xbox_ext.h"\
	"..\..\NSL\XBOX\nsl_xbox_source.h"\
	"..\..\nsl\xbox\nsl_xbox_streams.h"\
	"..\..\NVL\ps2\nvl_ps2.h"\
	"..\..\NVL\ps2\nvlMPEG_ps2.h"\
	"..\..\NVL\ps2\nvlstream_ps2.h"\
	".\aggvertbuf.h"\
	".\ai_actions.cpp"\
	".\ai_actions.h"\
	".\ai_constants.h"\
	".\ai_goals.cpp"\
	".\ai_goals.h"\
	".\ai_goals_mac.h"\
	".\ai_interface.cpp"\
	".\ai_interface.h"\
	".\ai_locomotion.cpp"\
	".\ai_locomotion.h"\
	".\ai_locomotion_direct.cpp"\
	".\ai_locomotion_direct.h"\
	".\ai_polypath.cpp"\
	".\ai_polypath.h"\
	".\ai_polypath_cell.cpp"\
	".\ai_polypath_cell.h"\
	".\ai_polypath_heap.h"\
	".\ai_voice.cpp"\
	".\ai_voice.h"\
	".\AIController.cpp"\
	".\AIController.h"\
	".\algebra.h"\
	".\anim.h"\
	".\anim_flavor.h"\
	".\anim_flavors.h"\
	".\anim_ids.h"\
	".\animation_interface.h"\
	".\app.h"\
	".\archalloc.h"\
	".\attribute_template.h"\
	".\avltree.h"\
	".\b_spline.h"\
	".\beam.h"\
	".\beam_signals.h"\
	".\billboard.h"\
	".\bone.h"\
	".\bound.h"\
	".\box_trigger_interface.h"\
	".\camera.h"\
	".\capsule.h"\
	".\cface.h"\
	".\character_hard_attribs.h"\
	".\chunkfile.h"\
	".\colgeom.h"\
	".\collide.h"\
	".\colmesh.h"\
	".\color.h"\
	".\commands.h"\
	".\conglom.h"\
	".\constants.h"\
	".\controller.h"\
	".\convex_box.h"\
	".\custom_stl.h"\
	".\debug.h"\
	".\debug_render.h"\
	".\devoptflags.h"\
	".\devoptints.h"\
	".\devoptstrs.h"\
	".\entflavor.h"\
	".\entity.h"\
	".\entity_anim.h"\
	".\entity_hard_attribs.h"\
	".\entity_interface.h"\
	".\entity_maker.h"\
	".\entity_signals.h"\
	".\entityflags.h"\
	".\entityid.h"\
	".\errorcontext.h"\
	".\face.h"\
	".\faceflags.h"\
	".\fast_vector.h"\
	".\file.h"\
	".\file_finder.h"\
	".\filespec.h"\
	".\frame_info.h"\
	".\game.h"\
	".\game_info_vars.h"\
	".\game_process.h"\
	".\gcglobals.h"\
	".\generator.h"\
	".\geomgr.h"\
	".\global.h"\
	".\global_signals.h"\
	".\graph.h"\
	".\guidance_sys.h"\
	".\hard_attrib_interface.h"\
	".\hull.h"\
	".\hwaudio.h"\
	".\hwmath.h"\
	".\hwmovieplayer.h"\
	".\hwrasterize.h"\
	".\hyperplane.h"\
	".\ini_parser.h"\
	".\inputmgr.h"\
	".\instance.h"\
	".\item.h"\
	".\item_signals.h"\
	".\joystick.h"\
	".\ks\kellyslater_ai_goals.h"\
	".\ks\ksheaps.h"\
	".\ks\wipeoutdata.h"\
	".\kshooks.h"\
	".\ksnsl.h"\
	".\ksnvl.h"\
	".\light.h"\
	".\linear_anim.h"\
	".\link_interface.h"\
	".\map_e.h"\
	".\material.h"\
	".\matfac.h"\
	".\maxskinbones.h"\
	".\meshrefs.h"\
	".\mic.h"\
	".\mobject.h"\
	".\mustash.h"\
	".\ngl.h"\
	".\osalloc.h"\
	".\osassert.h"\
	".\osdevopts.h"\
	".\oserrmsg.h"\
	".\osfile.h"\
	".\ostimer.h"\
	".\path.h"\
	".\pc_port.h"\
	".\physical_interface.h"\
	".\plane.h"\
	".\platform_defines.h"\
	".\pmesh.h"\
	".\po.h"\
	".\po_anim.h"\
	".\polytube.h"\
	".\portal.h"\
	".\profiler.h"\
	".\projconst.h"\
	".\project.h"\
	".\pstring.h"\
	".\random.h"\
	".\rect.h"\
	".\refptr.h"\
	".\region.h"\
	".\region_graph.h"\
	".\render_data.h"\
	".\renderflav.h"\
	".\scene_anim.h"\
	".\script_lib_mfg.h"\
	".\script_library_class.h"\
	".\script_mfg_signals.h"\
	".\script_object.h"\
	".\signal_anim.h"\
	".\signals.h"\
	".\simple_classes.h"\
	".\SimpleAssert.h"\
	".\singleton.h"\
	".\so_data_block.h"\
	".\sound_group.h"\
	".\sound_interface.h"\
	".\sphere.h"\
	".\stashes.h"\
	".\staticmem.h"\
	".\stl_adapter.h"\
	".\stringx.h"\
	".\terrain.h"\
	".\text_font.h"\
	".\textfile.h"\
	".\time_interface.h"\
	".\timer.h"\
	".\txtcoord.h"\
	".\types.h"\
	".\usefulmath.h"\
	".\users.h"\
	".\vert.h"\
	".\visrep.h"\
	".\vm_executable.h"\
	".\vm_symbol.h"\
	".\vm_symbol_list.h"\
	".\warnlvl.h"\
	".\wds.h"\
	".\wedge.h"\
	".\xbglobals.h"\
	
NODEP_CPP_FILES=\
	"..\..\NSL\pc\nl_pc.h"\
	"..\..\NSL\pc\nsl_pc_ext.h"\
	".\algo.h"\
	".\defalloc.h"\
	".\ekernel.h"\
	".\eregs.h"\
	".\estruct.h"\
	".\etypes.h"\
	".\gc\nsl_gc.h"\
	".\gc_arammgr.h"\
	".\graphics.h"\
	".\hwosmks\am_audio.h"\
	".\hwosmks\mks_devopts.h"\
	".\hwosmks\mks_errmsg.h"\
	".\hwosmks\mks_movieplayer.h"\
	".\hwosmks\set5_rasterize.h"\
	".\hwosmks\set5_timer.h"\
	".\hwosmks\sh4_math.h"\
	".\hwosmks\sy_alloc.h"\
	".\hwosmks\sy_file.h"\
	".\hwosnglpc\algebra.h"\
	".\hwosnglpc\pc_algebra.h"\
	".\hwosnull\null_alloc.h"\
	".\hwosnull\null_audio.h"\
	".\hwosnull\null_devopts.h"\
	".\hwosnull\null_errmsg.h"\
	".\hwosnull\null_file.h"\
	".\hwosnull\null_math.h"\
	".\hwosnull\null_movieplayer.h"\
	".\hwosnull\null_rasterize.h"\
	".\hwosnull\null_timer.h"\
	".\hwospc\d3d_rasterize.h"\
	".\hwospc\ds_audio.h"\
	".\hwospc\pc_algebra.h"\
	".\hwospc\pc_movieplayer.h"\
	".\hwospc\w32_alloc.h"\
	".\hwospc\w32_devopts.h"\
	".\hwospc\w32_errmsg.h"\
	".\hwospc\w32_file.h"\
	".\hwospc\w32_timer.h"\
	".\hwospc\x86_math.h"\
	".\hwosxb\ngl_xbox.h"\
	".\hwosxb\nvl_xbox.h"\
	".\ibcdvd.h"\
	".\ibdma.h"\
	".\ibgraph.h"\
	".\ibipu.h"\
	".\ibmc.h"\
	".\ibmpeg.h"\
	".\ibpad.h"\
	".\ibpc.h"\
	".\ibusbkb.h"\
	".\ibvu0.h"\
	".\ifdev.h"\
	".\LipSync_Anim.h"\
	".\ngl_xbox.h"\
	".\nsl.h"\
	".\nvl_gc.h"\
	".\nvl_xbox.h"\
	".\nvlstream_gc.h"\
	".\s\AccompFrontEnd.h"\
	".\s\andersen_ind_anims_mac.h"\
	".\s\beach.h"\
	".\s\beachdata.h"\
	".\s\BeachFrontEnd.h"\
	".\s\board.h"\
	".\s\board_anims_mac.h"\
	".\s\boarddata.h"\
	".\s\BoardFrontEnd.h"\
	".\s\career.h"\
	".\s\careerdata.h"\
	".\s\carrol_ind_anims_mac.h"\
	".\s\challenge.h"\
	".\s\challenge_icon.h"\
	".\s\challenge_photo.h"\
	".\s\cheat.h"\
	".\s\CheatFrontEnd.h"\
	".\s\compressedphoto.h"\
	".\s\coords.h"\
	".\s\curren_ind_anims_mac.h"\
	".\s\eventmanager.h"\
	".\s\ExtrasFrontEnd.h"\
	".\s\FEAnim.h"\
	".\s\FEEntityManager.h"\
	".\s\FEMenu.h"\
	".\s\FEPanel.h"\
	".\s\fletcher_ind_anims_mac.h"\
	".\s\floatobj.h"\
	".\s\frankenreiter_ind_anims_mac.h"\
	".\s\FrontEndManager.h"\
	".\s\FrontEndMenus.h"\
	".\s\GameData.h"\
	".\s\GlobalData.h"\
	".\s\globaltextenum.h"\
	".\s\GraphicalMenuSystem.h"\
	".\s\HighScoreFrontEnd.h"\
	".\s\igo_widget.h"\
	".\s\igo_widget_analogclock.h"\
	".\s\igo_widget_balance.h"\
	".\s\igo_widget_camera.h"\
	".\s\igo_widget_fanmeter.h"\
	".\s\igo_widget_iconcount.h"\
	".\s\igo_widget_iconradar.h"\
	".\s\igo_widget_meterchallenge.h"\
	".\s\igo_widget_objectalert.h"\
	".\s\igo_widget_photo.h"\
	".\s\igo_widget_replay.h"\
	".\s\igo_widget_simple.h"\
	".\s\igo_widget_skillchallenge.h"\
	".\s\igo_widget_specialmeter.h"\
	".\s\igo_widget_splitclock.h"\
	".\s\igo_widget_splitmeter.h"\
	".\s\igo_widget_splitscore.h"\
	".\s\igo_widget_splitter.h"\
	".\s\igo_widget_timeattack.h"\
	".\s\igo_widget_waveindicator.h"\
	".\s\IGOFrontEnd.h"\
	".\s\igohintmanager.h"\
	".\s\igoiconmanager.h"\
	".\s\igolearn_new_trickmanager.h"\
	".\s\ik_object.h"\
	".\s\irons_ind_anims_mac.h"\
	".\s\judge.h"\
	".\s\kellyslater_controller.h"\
	".\s\kellyslater_shared_anims_mac.h"\
	".\s\kellyslater_states_mac.h"\
	".\s\ks_camera.h"\
	".\s\ksdbmenu.h"\
	".\s\ksngl.h"\
	".\s\ksreplay.h"\
	".\s\LogbookFrontEnd.h"\
	".\s\machado_ind_anims_mac.h"\
	".\s\MainFrontEnd.h"\
	".\s\Map.h"\
	".\s\MCDetectFrontEnd.h"\
	".\s\menu.h"\
	".\s\menusys.h"\
	".\s\mode.h"\
	".\s\mode_headtohead.h"\
	".\s\mode_meterattack.h"\
	".\s\mode_push.h"\
	".\s\mode_timeattack.h"\
	".\s\MultiFrontEnd.h"\
	".\s\MusicMan.h"\
	".\s\ode.h"\
	".\s\osGameSaver.h"\
	".\s\PAL60FrontEnd.h"\
	".\s\PathData.h"\
	".\s\PhotoFrontEnd.h"\
	".\s\physics.h"\
	".\s\player.h"\
	".\s\replay.h"\
	".\s\robb_ind_anims_mac.h"\
	".\s\rumbleManager.h"\
	".\s\SaveLoadFrontEnd.h"\
	".\s\scoringmanager.h"\
	".\s\slater_ind_anims_mac.h"\
	".\s\sounddata.h"\
	".\s\SoundScript.h"\
	".\s\specialmeter.h"\
	".\s\StatsFrontEnd.h"\
	".\s\surferdata.h"\
	".\s\SurferFrontEnd.h"\
	".\s\trick_system.h"\
	".\s\TrickBookFrontEnd.h"\
	".\s\trickdata.h"\
	".\s\trickregion.h"\
	".\s\tricks.h"\
	".\s\tutorialdata.h"\
	".\s\TutorialFrontEnd.h"\
	".\s\tutorialmanager.h"\
	".\s\VOEngine.h"\
	".\s\wave.h"\
	".\s\wavebreak.txt"\
	".\s\wavebreakstage.txt"\
	".\s\waveemitter.txt"\
	".\s\waveenum.h"\
	".\s\wavemarker.txt"\
	".\s\waveregion.txt"\
	".\s\wavestage.txt"\
	".\stl_heap.h"\
	".\wosgc\gc_algebra.h"\
	".\wosgc\gc_alloc.h"\
	".\wosgc\gc_audio.h"\
	".\wosgc\gc_errmsg.h"\
	".\wosgc\gc_file.h"\
	".\WOSGC\gc_gamesaver.h"\
	".\wosgc\gc_graphics.h"\
	".\wosgc\gc_input.h"\
	".\wosgc\gc_math.h"\
	".\wosgc\gc_movieplayer.h"\
	".\wosgc\gc_rasterize.h"\
	".\wosgc\gc_texturemgr.h"\
	".\wosgc\gc_timer.h"\
	".\wosgc\ngl_fixedstr.h"\
	".\wosgc\ngl_gc.h"\
	".\wosgc\ngl_version.h"\
	".\wosgc\nvl_gc.h"\
	".\WOSPS2\gas.h"\
	".\WOSPS2\ps2_algebra.h"\
	".\WOSPS2\ps2_alloc.h"\
	".\WOSPS2\ps2_audio.h"\
	".\WOSPS2\ps2_devopts.h"\
	".\WOSPS2\ps2_errmsg.h"\
	".\WOSPS2\ps2_file.h"\
	".\WOSPS2\ps2_gamesaver.h"\
	".\WOSPS2\ps2_graphics.h"\
	".\WOSPS2\ps2_input.h"\
	".\WOSPS2\ps2_m2vplayer.h"\
	".\WOSPS2\ps2_math.h"\
	".\WOSPS2\ps2_movieplayer.h"\
	".\WOSPS2\ps2_rasterize.h"\
	".\WOSPS2\ps2_texturemgr.h"\
	".\WOSPS2\ps2_timer.h"\
	".\WOSPS2\ps2_types.h"\
	".\wosxb\pstring.h"\
	".\wosxb\xb_algebra.h"\
	".\wosxb\xb_alloc.h"\
	".\wosxb\xb_audio.h"\
	".\wosxb\xb_errmsg.h"\
	".\wosxb\xb_file.h"\
	".\WOSXB\xb_gamesaver.h"\
	".\wosxb\xb_graphics.h"\
	".\wosxb\xb_input.h"\
	".\wosxb\xb_math.h"\
	".\wosxb\xb_movieplayer.h"\
	".\WOSXB\xb_portDVDCache.h"\
	".\wosxb\xb_rasterize.h"\
	".\wosxb\xb_string.h"\
	".\wosxb\xb_texturemgr.h"\
	".\wosxb\xb_timer.h"\
	".\wosxb\xb_types.h"\
	
# SUBTRACT BASE CPP /YX /Yc /Yu
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\files_anim.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

DEP_CPP_FILES_=\
	"..\..\ngl\ps2\ngl_fixedstr.h"\
	"..\..\ngl\ps2\ngl_ps2.h"\
	"..\..\ngl\ps2\tim2.h"\
	"..\..\NSL\common\nl.h"\
	"..\..\NSL\common\nsl.h"\
	"..\..\NSL\common\ProjectOptions.h"\
	"..\..\NSL\gamecube\nl_gc.h"\
	"..\..\NSL\ps2\fifo_queue.cpp"\
	"..\..\NSL\ps2\fifo_queue.h"\
	"..\..\NSL\PS2\gas.h"\
	"..\..\NSL\PS2\gas_utility.h"\
	"..\..\NSL\PS2\nl_ps2.h"\
	"..\..\NSL\PS2\nsl_ps2.h"\
	"..\..\NSL\XBOX\nl_xbox.h"\
	"..\..\NSL\XBOX\nsl_xbox.h"\
	"..\..\NSL\XBOX\nsl_xbox_ext.h"\
	"..\..\NSL\XBOX\nsl_xbox_source.h"\
	"..\..\nsl\xbox\nsl_xbox_streams.h"\
	"..\..\NVL\ps2\nvl_ps2.h"\
	"..\..\NVL\ps2\nvlMPEG_ps2.h"\
	"..\..\NVL\ps2\nvlstream_ps2.h"\
	".\aggvertbuf.h"\
	".\ai_constants.h"\
	".\ai_interface.h"\
	".\ai_locomotion.h"\
	".\ai_polypath.h"\
	".\ai_polypath_cell.h"\
	".\ai_polypath_heap.h"\
	".\algebra.h"\
	".\anim.h"\
	".\anim_flavor.cpp"\
	".\anim_flavor.h"\
	".\anim_flavors.h"\
	".\anim_ids.h"\
	".\anim_maker.cpp"\
	".\anim_maker.h"\
	".\anim_user.cpp"\
	".\app.h"\
	".\archalloc.h"\
	".\avltree.h"\
	".\beam.h"\
	".\beam_signals.h"\
	".\billboard.h"\
	".\bone.h"\
	".\bound.h"\
	".\box_trigger_interface.h"\
	".\camera.h"\
	".\capsule.h"\
	".\cface.h"\
	".\chunkfile.h"\
	".\colgeom.h"\
	".\colmesh.h"\
	".\color.h"\
	".\commands.h"\
	".\conglom.h"\
	".\constants.h"\
	".\controller.h"\
	".\convex_box.h"\
	".\custom_stl.h"\
	".\debug.h"\
	".\devoptflags.h"\
	".\devoptints.h"\
	".\devoptstrs.h"\
	".\entflavor.h"\
	".\entity.h"\
	".\entity_anim.cpp"\
	".\entity_anim.h"\
	".\entity_interface.h"\
	".\entity_maker.h"\
	".\entity_signals.h"\
	".\entityflags.h"\
	".\entityid.h"\
	".\errorcontext.h"\
	".\face.h"\
	".\faceflags.h"\
	".\fast_vector.h"\
	".\file.h"\
	".\file_finder.h"\
	".\filespec.h"\
	".\frame_info.h"\
	".\game.h"\
	".\game_info_vars.h"\
	".\game_process.h"\
	".\gcglobals.h"\
	".\generator.h"\
	".\geomgr.h"\
	".\global.h"\
	".\global_signals.h"\
	".\graph.h"\
	".\guidance_sys.h"\
	".\hull.h"\
	".\hwmath.h"\
	".\hwmovieplayer.h"\
	".\hwrasterize.h"\
	".\hyperplane.h"\
	".\ini_parser.h"\
	".\inputmgr.h"\
	".\instance.h"\
	".\item.h"\
	".\item_signals.h"\
	".\joystick.h"\
	".\ks\aligned.h"\
	".\ks\ksheaps.h"\
	".\kshooks.h"\
	".\ksnsl.h"\
	".\ksnvl.h"\
	".\light.h"\
	".\linear_anim.h"\
	".\link_interface.h"\
	".\map_e.h"\
	".\marker.h"\
	".\material.h"\
	".\matfac.h"\
	".\maxskinbones.h"\
	".\meshrefs.h"\
	".\mic.h"\
	".\mobject.h"\
	".\mustash.h"\
	".\ngl.h"\
	".\osalloc.h"\
	".\osassert.h"\
	".\osdevopts.h"\
	".\oserrmsg.h"\
	".\osfile.h"\
	".\ostimer.h"\
	".\particle.h"\
	".\path.h"\
	".\pc_port.h"\
	".\physical_interface.h"\
	".\plane.h"\
	".\platform_defines.h"\
	".\pmesh.h"\
	".\po.h"\
	".\po_anim.cpp"\
	".\po_anim.h"\
	".\portal.h"\
	".\profiler.h"\
	".\projconst.h"\
	".\project.h"\
	".\pstring.h"\
	".\quatcomp.h"\
	".\random.h"\
	".\rect.h"\
	".\refptr.h"\
	".\region.h"\
	".\region_graph.h"\
	".\render_data.h"\
	".\renderflav.h"\
	".\scene_anim.cpp"\
	".\scene_anim.h"\
	".\script_lib_mfg.h"\
	".\script_library_class.h"\
	".\script_mfg_signals.h"\
	".\script_object.h"\
	".\signal_anim.cpp"\
	".\signal_anim.h"\
	".\signals.h"\
	".\SimpleAssert.h"\
	".\singleton.h"\
	".\so_data_block.h"\
	".\sphere.h"\
	".\stashes.h"\
	".\staticmem.h"\
	".\stl_adapter.h"\
	".\stringx.h"\
	".\terrain.h"\
	".\text_font.h"\
	".\textfile.h"\
	".\timer.h"\
	".\txtcoord.h"\
	".\types.h"\
	".\usefulmath.h"\
	".\users.h"\
	".\vert.h"\
	".\visrep.h"\
	".\vm_executable.h"\
	".\vm_symbol.h"\
	".\vm_symbol_list.h"\
	".\warnlvl.h"\
	".\wds.h"\
	".\wedge.h"\
	".\widget.h"\
	".\widget_entity.h"\
	".\xbglobals.h"\
	{$(INCLUDE)}"eekernel.h"\
	{$(INCLUDE)}"eeregs.h"\
	{$(INCLUDE)}"eestruct.h"\
	{$(INCLUDE)}"eetypes.h"\
	{$(INCLUDE)}"hwosgc\gc_algebra.h"\
	{$(INCLUDE)}"hwosgc\gc_alloc.h"\
	{$(INCLUDE)}"hwosgc\gc_errmsg.h"\
	{$(INCLUDE)}"hwosgc\gc_file.h"\
	{$(INCLUDE)}"HWOSGC\gc_gamesaver.h"\
	{$(INCLUDE)}"hwosgc\gc_graphics.h"\
	{$(INCLUDE)}"hwosgc\gc_input.h"\
	{$(INCLUDE)}"hwosgc\gc_math.h"\
	{$(INCLUDE)}"hwosgc\gc_movieplayer.h"\
	{$(INCLUDE)}"hwosgc\gc_rasterize.h"\
	{$(INCLUDE)}"hwosgc\gc_texturemgr.h"\
	{$(INCLUDE)}"hwosgc\gc_timer.h"\
	{$(INCLUDE)}"hwosgc\ngl_fixedstr.h"\
	{$(INCLUDE)}"hwosgc\ngl_gc.h"\
	{$(INCLUDE)}"hwosgc\ngl_version.h"\
	{$(INCLUDE)}"hwosgc\nvl_gc.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_algebra.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_alloc.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_devopts.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_errmsg.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_file.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_gamesaver.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_graphics.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_input.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_m2vplayer.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_math.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_movieplayer.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_rasterize.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_texturemgr.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_timer.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_types.h"\
	{$(INCLUDE)}"hwosxb\xb_algebra.h"\
	{$(INCLUDE)}"hwosxb\xb_alloc.h"\
	{$(INCLUDE)}"hwosxb\xb_errmsg.h"\
	{$(INCLUDE)}"hwosxb\xb_file.h"\
	{$(INCLUDE)}"HWOSXB\xb_gamesaver.h"\
	{$(INCLUDE)}"hwosxb\xb_graphics.h"\
	{$(INCLUDE)}"hwosxb\xb_input.h"\
	{$(INCLUDE)}"hwosxb\xb_math.h"\
	{$(INCLUDE)}"hwosxb\xb_movieplayer.h"\
	{$(INCLUDE)}"HWOSXB\xb_portDVDCache.h"\
	{$(INCLUDE)}"hwosxb\xb_rasterize.h"\
	{$(INCLUDE)}"hwosxb\xb_string.h"\
	{$(INCLUDE)}"hwosxb\xb_texturemgr.h"\
	{$(INCLUDE)}"hwosxb\xb_timer.h"\
	{$(INCLUDE)}"hwosxb\xb_types.h"\
	{$(INCLUDE)}"ks\AccompFrontEnd.h"\
	{$(INCLUDE)}"ks\andersen_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\beach.h"\
	{$(INCLUDE)}"ks\beachdata.h"\
	{$(INCLUDE)}"ks\BeachFrontEnd.h"\
	{$(INCLUDE)}"ks\board.h"\
	{$(INCLUDE)}"ks\board_anims_mac.h"\
	{$(INCLUDE)}"ks\boarddata.h"\
	{$(INCLUDE)}"ks\BoardFrontEnd.h"\
	{$(INCLUDE)}"ks\career.h"\
	{$(INCLUDE)}"ks\careerdata.h"\
	{$(INCLUDE)}"ks\carrol_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\challenge.h"\
	{$(INCLUDE)}"ks\challenge_icon.h"\
	{$(INCLUDE)}"ks\challenge_photo.h"\
	{$(INCLUDE)}"ks\cheat.h"\
	{$(INCLUDE)}"ks\CheatFrontEnd.h"\
	{$(INCLUDE)}"ks\compressedphoto.h"\
	{$(INCLUDE)}"ks\coords.h"\
	{$(INCLUDE)}"ks\curren_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\eventmanager.h"\
	{$(INCLUDE)}"ks\ExtrasFrontEnd.h"\
	{$(INCLUDE)}"ks\FEAnim.h"\
	{$(INCLUDE)}"ks\FEEntityManager.h"\
	{$(INCLUDE)}"ks\FEMenu.h"\
	{$(INCLUDE)}"ks\FEPanel.h"\
	{$(INCLUDE)}"ks\fletcher_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\floatobj.h"\
	{$(INCLUDE)}"ks\frankenreiter_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\FrontEndManager.h"\
	{$(INCLUDE)}"ks\FrontEndMenus.h"\
	{$(INCLUDE)}"ks\GameData.h"\
	{$(INCLUDE)}"ks\GlobalData.h"\
	{$(INCLUDE)}"ks\globaltextenum.h"\
	{$(INCLUDE)}"ks\GraphicalMenuSystem.h"\
	{$(INCLUDE)}"ks\HighScoreFrontEnd.h"\
	{$(INCLUDE)}"ks\igo_widget.h"\
	{$(INCLUDE)}"ks\igo_widget_analogclock.h"\
	{$(INCLUDE)}"ks\igo_widget_balance.h"\
	{$(INCLUDE)}"ks\igo_widget_camera.h"\
	{$(INCLUDE)}"ks\igo_widget_fanmeter.h"\
	{$(INCLUDE)}"ks\igo_widget_grid.h"\
	{$(INCLUDE)}"ks\igo_widget_iconcount.h"\
	{$(INCLUDE)}"ks\igo_widget_iconradar.h"\
	{$(INCLUDE)}"ks\igo_widget_meterchallenge.h"\
	{$(INCLUDE)}"ks\igo_widget_objectalert.h"\
	{$(INCLUDE)}"ks\igo_widget_photo.h"\
	{$(INCLUDE)}"ks\igo_widget_replay.h"\
	{$(INCLUDE)}"ks\igo_widget_simple.h"\
	{$(INCLUDE)}"ks\igo_widget_skillchallenge.h"\
	{$(INCLUDE)}"ks\igo_widget_specialmeter.h"\
	{$(INCLUDE)}"ks\igo_widget_splitclock.h"\
	{$(INCLUDE)}"ks\igo_widget_splitmeter.h"\
	{$(INCLUDE)}"ks\igo_widget_splitscore.h"\
	{$(INCLUDE)}"ks\igo_widget_splitter.h"\
	{$(INCLUDE)}"ks\igo_widget_timeattack.h"\
	{$(INCLUDE)}"ks\igo_widget_waveindicator.h"\
	{$(INCLUDE)}"ks\IGOFrontEnd.h"\
	{$(INCLUDE)}"ks\igohintmanager.h"\
	{$(INCLUDE)}"ks\igoiconmanager.h"\
	{$(INCLUDE)}"ks\igolearn_new_trickmanager.h"\
	{$(INCLUDE)}"ks\ik_object.h"\
	{$(INCLUDE)}"ks\irons_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\judge.h"\
	{$(INCLUDE)}"ks\kellyslater_controller.h"\
	{$(INCLUDE)}"ks\kellyslater_shared_anims_mac.h"\
	{$(INCLUDE)}"ks\kellyslater_states_mac.h"\
	{$(INCLUDE)}"ks\ks_camera.h"\
	{$(INCLUDE)}"ks\ksdbmenu.h"\
	{$(INCLUDE)}"ks\ksngl.h"\
	{$(INCLUDE)}"ks\ksreplay.h"\
	{$(INCLUDE)}"ks\LogbookFrontEnd.h"\
	{$(INCLUDE)}"ks\machado_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\MainFrontEnd.h"\
	{$(INCLUDE)}"ks\Map.h"\
	{$(INCLUDE)}"ks\MCDetectFrontEnd.h"\
	{$(INCLUDE)}"ks\menu.h"\
	{$(INCLUDE)}"ks\menusys.h"\
	{$(INCLUDE)}"ks\mode.h"\
	{$(INCLUDE)}"ks\mode_headtohead.h"\
	{$(INCLUDE)}"ks\mode_meterattack.h"\
	{$(INCLUDE)}"ks\mode_push.h"\
	{$(INCLUDE)}"ks\mode_timeattack.h"\
	{$(INCLUDE)}"ks\MultiFrontEnd.h"\
	{$(INCLUDE)}"ks\MusicMan.h"\
	{$(INCLUDE)}"ks\ode.h"\
	{$(INCLUDE)}"ks\osGameSaver.h"\
	{$(INCLUDE)}"ks\PAL60FrontEnd.h"\
	{$(INCLUDE)}"ks\PathData.h"\
	{$(INCLUDE)}"ks\PhotoFrontEnd.h"\
	{$(INCLUDE)}"ks\physics.h"\
	{$(INCLUDE)}"ks\player.h"\
	{$(INCLUDE)}"ks\replay.h"\
	{$(INCLUDE)}"ks\robb_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\rumbleManager.h"\
	{$(INCLUDE)}"ks\SaveLoadFrontEnd.h"\
	{$(INCLUDE)}"ks\scoringmanager.h"\
	{$(INCLUDE)}"ks\slater_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\sounddata.h"\
	{$(INCLUDE)}"ks\SoundScript.h"\
	{$(INCLUDE)}"ks\specialmeter.h"\
	{$(INCLUDE)}"ks\StatsFrontEnd.h"\
	{$(INCLUDE)}"ks\surferdata.h"\
	{$(INCLUDE)}"ks\SurferFrontEnd.h"\
	{$(INCLUDE)}"ks\trick_system.h"\
	{$(INCLUDE)}"ks\TrickBookFrontEnd.h"\
	{$(INCLUDE)}"ks\trickdata.h"\
	{$(INCLUDE)}"ks\trickregion.h"\
	{$(INCLUDE)}"ks\tricks.h"\
	{$(INCLUDE)}"ks\tutorialdata.h"\
	{$(INCLUDE)}"ks\TutorialFrontEnd.h"\
	{$(INCLUDE)}"ks\tutorialmanager.h"\
	{$(INCLUDE)}"ks\underwtr.h"\
	{$(INCLUDE)}"ks\VOEngine.h"\
	{$(INCLUDE)}"ks\wave.h"\
	{$(INCLUDE)}"ks\wavebreak.txt"\
	{$(INCLUDE)}"ks\wavebreakstage.txt"\
	{$(INCLUDE)}"ks\waveemitter.txt"\
	{$(INCLUDE)}"ks\waveenum.h"\
	{$(INCLUDE)}"ks\wavemarker.txt"\
	{$(INCLUDE)}"ks\waveregion.txt"\
	{$(INCLUDE)}"ks\wavestage.txt"\
	{$(INCLUDE)}"libcdvd.h"\
	{$(INCLUDE)}"libdma.h"\
	{$(INCLUDE)}"libgraph.h"\
	{$(INCLUDE)}"libipu.h"\
	{$(INCLUDE)}"libmc.h"\
	{$(INCLUDE)}"libmpeg.h"\
	{$(INCLUDE)}"libpad.h"\
	{$(INCLUDE)}"libpc.h"\
	{$(INCLUDE)}"libusbkb.h"\
	{$(INCLUDE)}"libvu0.h"\
	{$(INCLUDE)}"sifdev.h"\
	{$(INCLUDE)}"xgraphics.h"\
	
NODEP_CPP_FILES_=\
	"..\..\NSL\pc\nl_pc.h"\
	"..\..\NSL\pc\nsl_pc_ext.h"\
	".\algo.h"\
	".\defalloc.h"\
	".\gc_arammgr.h"\
	".\hwosmks\mks_devopts.h"\
	".\hwosmks\mks_errmsg.h"\
	".\hwosmks\mks_movieplayer.h"\
	".\hwosmks\set5_rasterize.h"\
	".\hwosmks\set5_timer.h"\
	".\hwosmks\sh4_math.h"\
	".\hwosmks\sy_alloc.h"\
	".\hwosmks\sy_file.h"\
	".\hwosnglpc\algebra.h"\
	".\hwosnglpc\pc_algebra.h"\
	".\hwosnull\null_alloc.h"\
	".\hwosnull\null_devopts.h"\
	".\hwosnull\null_errmsg.h"\
	".\hwosnull\null_file.h"\
	".\hwosnull\null_math.h"\
	".\hwosnull\null_movieplayer.h"\
	".\hwosnull\null_rasterize.h"\
	".\hwosnull\null_timer.h"\
	".\hwospc\d3d_rasterize.h"\
	".\hwospc\pc_algebra.h"\
	".\hwospc\pc_movieplayer.h"\
	".\hwospc\w32_alloc.h"\
	".\hwospc\w32_devopts.h"\
	".\hwospc\w32_errmsg.h"\
	".\hwospc\w32_file.h"\
	".\hwospc\w32_timer.h"\
	".\hwospc\x86_math.h"\
	".\hwosxb\ngl_xbox.h"\
	".\hwosxb\nvl_xbox.h"\
	".\LipSync_Anim.h"\
	".\ngl_xbox.h"\
	".\nvl_gc.h"\
	".\nvl_xbox.h"\
	".\nvlstream_gc.h"\
	".\stl_heap.h"\
	
# ADD BASE CPP /Yu
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

DEP_CPP_FILES_=\
	"..\..\ngl\ps2\ngl_fixedstr.h"\
	"..\..\ngl\ps2\ngl_ps2.h"\
	"..\..\ngl\ps2\tim2.h"\
	"..\..\NSL\common\nl.h"\
	"..\..\NSL\common\nsl.h"\
	"..\..\NSL\common\ProjectOptions.h"\
	"..\..\NSL\gamecube\nl_gc.h"\
	"..\..\NSL\ps2\fifo_queue.cpp"\
	"..\..\NSL\ps2\fifo_queue.h"\
	"..\..\NSL\PS2\gas.h"\
	"..\..\NSL\PS2\gas_utility.h"\
	"..\..\NSL\PS2\nl_ps2.h"\
	"..\..\NSL\PS2\nsl_ps2.h"\
	"..\..\NSL\XBOX\nl_xbox.h"\
	"..\..\NSL\XBOX\nsl_xbox.h"\
	"..\..\NSL\XBOX\nsl_xbox_ext.h"\
	"..\..\NSL\XBOX\nsl_xbox_source.h"\
	"..\..\nsl\xbox\nsl_xbox_streams.h"\
	"..\..\NVL\ps2\nvl_ps2.h"\
	"..\..\NVL\ps2\nvlMPEG_ps2.h"\
	"..\..\NVL\ps2\nvlstream_ps2.h"\
	".\aggvertbuf.h"\
	".\ai_constants.h"\
	".\ai_interface.h"\
	".\ai_locomotion.h"\
	".\ai_polypath.h"\
	".\ai_polypath_cell.h"\
	".\ai_polypath_heap.h"\
	".\algebra.h"\
	".\anim.h"\
	".\anim_flavor.cpp"\
	".\anim_flavor.h"\
	".\anim_flavors.h"\
	".\anim_ids.h"\
	".\anim_maker.cpp"\
	".\anim_maker.h"\
	".\anim_user.cpp"\
	".\app.h"\
	".\archalloc.h"\
	".\avltree.h"\
	".\beam.h"\
	".\beam_signals.h"\
	".\billboard.h"\
	".\bone.h"\
	".\bound.h"\
	".\box_trigger_interface.h"\
	".\camera.h"\
	".\capsule.h"\
	".\cface.h"\
	".\chunkfile.h"\
	".\colgeom.h"\
	".\colmesh.h"\
	".\color.h"\
	".\commands.h"\
	".\conglom.h"\
	".\constants.h"\
	".\controller.h"\
	".\convex_box.h"\
	".\custom_stl.h"\
	".\debug.h"\
	".\devoptflags.h"\
	".\devoptints.h"\
	".\devoptstrs.h"\
	".\entflavor.h"\
	".\entity.h"\
	".\entity_anim.cpp"\
	".\entity_anim.h"\
	".\entity_interface.h"\
	".\entity_maker.h"\
	".\entity_signals.h"\
	".\entityflags.h"\
	".\entityid.h"\
	".\errorcontext.h"\
	".\face.h"\
	".\faceflags.h"\
	".\fast_vector.h"\
	".\file.h"\
	".\file_finder.h"\
	".\filespec.h"\
	".\frame_info.h"\
	".\game.h"\
	".\game_info_vars.h"\
	".\game_process.h"\
	".\gcglobals.h"\
	".\generator.h"\
	".\geomgr.h"\
	".\global.h"\
	".\global_signals.h"\
	".\graph.h"\
	".\guidance_sys.h"\
	".\hull.h"\
	".\hwmath.h"\
	".\hwmovieplayer.h"\
	".\hwrasterize.h"\
	".\hyperplane.h"\
	".\ini_parser.h"\
	".\inputmgr.h"\
	".\instance.h"\
	".\item.h"\
	".\item_signals.h"\
	".\joystick.h"\
	".\ks\aligned.h"\
	".\ks\ksheaps.h"\
	".\kshooks.h"\
	".\ksnsl.h"\
	".\ksnvl.h"\
	".\light.h"\
	".\linear_anim.h"\
	".\link_interface.h"\
	".\map_e.h"\
	".\marker.h"\
	".\material.h"\
	".\matfac.h"\
	".\maxskinbones.h"\
	".\meshrefs.h"\
	".\mic.h"\
	".\mobject.h"\
	".\mustash.h"\
	".\ngl.h"\
	".\osalloc.h"\
	".\osassert.h"\
	".\osdevopts.h"\
	".\oserrmsg.h"\
	".\osfile.h"\
	".\ostimer.h"\
	".\particle.h"\
	".\path.h"\
	".\pc_port.h"\
	".\physical_interface.h"\
	".\plane.h"\
	".\platform_defines.h"\
	".\pmesh.h"\
	".\po.h"\
	".\po_anim.cpp"\
	".\po_anim.h"\
	".\portal.h"\
	".\profiler.h"\
	".\projconst.h"\
	".\project.h"\
	".\pstring.h"\
	".\quatcomp.h"\
	".\random.h"\
	".\rect.h"\
	".\refptr.h"\
	".\region.h"\
	".\region_graph.h"\
	".\render_data.h"\
	".\renderflav.h"\
	".\scene_anim.cpp"\
	".\scene_anim.h"\
	".\script_lib_mfg.h"\
	".\script_library_class.h"\
	".\script_mfg_signals.h"\
	".\script_object.h"\
	".\signal_anim.cpp"\
	".\signal_anim.h"\
	".\signals.h"\
	".\SimpleAssert.h"\
	".\singleton.h"\
	".\so_data_block.h"\
	".\sphere.h"\
	".\stashes.h"\
	".\staticmem.h"\
	".\stl_adapter.h"\
	".\stringx.h"\
	".\terrain.h"\
	".\text_font.h"\
	".\textfile.h"\
	".\timer.h"\
	".\txtcoord.h"\
	".\types.h"\
	".\usefulmath.h"\
	".\users.h"\
	".\vert.h"\
	".\visrep.h"\
	".\vm_executable.h"\
	".\vm_symbol.h"\
	".\vm_symbol_list.h"\
	".\warnlvl.h"\
	".\wds.h"\
	".\wedge.h"\
	".\widget.h"\
	".\widget_entity.h"\
	".\xbglobals.h"\
	
NODEP_CPP_FILES_=\
	"..\..\NSL\pc\nl_pc.h"\
	"..\..\NSL\pc\nsl_pc_ext.h"\
	"..\..\NSL\XBOX\nsl.h"\
	".\algo.h"\
	".\defalloc.h"\
	".\ekernel.h"\
	".\eregs.h"\
	".\estruct.h"\
	".\etypes.h"\
	".\gc_arammgr.h"\
	".\graphics.h"\
	".\hwosgc\nsl.h"\
	".\hwosmks\mks_devopts.h"\
	".\hwosmks\mks_errmsg.h"\
	".\hwosmks\mks_movieplayer.h"\
	".\hwosmks\set5_rasterize.h"\
	".\hwosmks\set5_timer.h"\
	".\hwosmks\sh4_math.h"\
	".\hwosmks\sy_alloc.h"\
	".\hwosmks\sy_file.h"\
	".\hwosnglpc\algebra.h"\
	".\hwosnglpc\pc_algebra.h"\
	".\hwosnull\null_alloc.h"\
	".\hwosnull\null_devopts.h"\
	".\hwosnull\null_errmsg.h"\
	".\hwosnull\null_file.h"\
	".\hwosnull\null_math.h"\
	".\hwosnull\null_movieplayer.h"\
	".\hwosnull\null_rasterize.h"\
	".\hwosnull\null_timer.h"\
	".\hwospc\d3d_rasterize.h"\
	".\hwospc\pc_algebra.h"\
	".\hwospc\pc_movieplayer.h"\
	".\hwospc\w32_alloc.h"\
	".\hwospc\w32_devopts.h"\
	".\hwospc\w32_errmsg.h"\
	".\hwospc\w32_file.h"\
	".\hwospc\w32_timer.h"\
	".\hwospc\x86_math.h"\
	".\hwosxb\ngl_xbox.h"\
	".\hwosxb\nvl_xbox.h"\
	".\ibcdvd.h"\
	".\ibdma.h"\
	".\ibgraph.h"\
	".\ibipu.h"\
	".\ibmc.h"\
	".\ibmpeg.h"\
	".\ibpad.h"\
	".\ibpc.h"\
	".\ibusbkb.h"\
	".\ibvu0.h"\
	".\ifdev.h"\
	".\LipSync_Anim.h"\
	".\ngl_xbox.h"\
	".\nvl_gc.h"\
	".\nvl_xbox.h"\
	".\nvlstream_gc.h"\
	".\s\AccompFrontEnd.h"\
	".\s\andersen_ind_anims_mac.h"\
	".\s\beach.h"\
	".\s\beachdata.h"\
	".\s\BeachFrontEnd.h"\
	".\s\board.h"\
	".\s\board_anims_mac.h"\
	".\s\boarddata.h"\
	".\s\BoardFrontEnd.h"\
	".\s\career.h"\
	".\s\careerdata.h"\
	".\s\carrol_ind_anims_mac.h"\
	".\s\challenge.h"\
	".\s\challenge_icon.h"\
	".\s\challenge_photo.h"\
	".\s\cheat.h"\
	".\s\CheatFrontEnd.h"\
	".\s\compressedphoto.h"\
	".\s\coords.h"\
	".\s\curren_ind_anims_mac.h"\
	".\s\eventmanager.h"\
	".\s\ExtrasFrontEnd.h"\
	".\s\FEAnim.h"\
	".\s\FEEntityManager.h"\
	".\s\FEMenu.h"\
	".\s\FEPanel.h"\
	".\s\fletcher_ind_anims_mac.h"\
	".\s\floatobj.h"\
	".\s\frankenreiter_ind_anims_mac.h"\
	".\s\FrontEndManager.h"\
	".\s\FrontEndMenus.h"\
	".\s\GameData.h"\
	".\s\GlobalData.h"\
	".\s\globaltextenum.h"\
	".\s\GraphicalMenuSystem.h"\
	".\s\HighScoreFrontEnd.h"\
	".\s\igo_widget.h"\
	".\s\igo_widget_analogclock.h"\
	".\s\igo_widget_balance.h"\
	".\s\igo_widget_camera.h"\
	".\s\igo_widget_fanmeter.h"\
	".\s\igo_widget_iconradar.h"\
	".\s\igo_widget_photo.h"\
	".\s\igo_widget_replay.h"\
	".\s\igo_widget_simple.h"\
	".\s\igo_widget_specialmeter.h"\
	".\s\igo_widget_splitclock.h"\
	".\s\igo_widget_splitmeter.h"\
	".\s\igo_widget_splitscore.h"\
	".\s\igo_widget_splitter.h"\
	".\s\igo_widget_timeattack.h"\
	".\s\igo_widget_waveindicator.h"\
	".\s\IGOFrontEnd.h"\
	".\s\igohintmanager.h"\
	".\s\igoiconmanager.h"\
	".\s\igolearn_new_trickmanager.h"\
	".\s\ik_object.h"\
	".\s\irons_ind_anims_mac.h"\
	".\s\judge.h"\
	".\s\kellyslater_controller.h"\
	".\s\kellyslater_shared_anims_mac.h"\
	".\s\kellyslater_states_mac.h"\
	".\s\ks_camera.h"\
	".\s\ksdbmenu.h"\
	".\s\ksngl.h"\
	".\s\ksreplay.h"\
	".\s\machado_ind_anims_mac.h"\
	".\s\MainFrontEnd.h"\
	".\s\Map.h"\
	".\s\MCDetectFrontEnd.h"\
	".\s\menu.h"\
	".\s\menusys.h"\
	".\s\mode.h"\
	".\s\mode_headtohead.h"\
	".\s\mode_meterattack.h"\
	".\s\mode_push.h"\
	".\s\mode_timeattack.h"\
	".\s\MultiFrontEnd.h"\
	".\s\MusicMan.h"\
	".\s\ode.h"\
	".\s\osGameSaver.h"\
	".\s\PAL60FrontEnd.h"\
	".\s\PathData.h"\
	".\s\PhotoFrontEnd.h"\
	".\s\physics.h"\
	".\s\player.h"\
	".\s\replay.h"\
	".\s\robb_ind_anims_mac.h"\
	".\s\rumbleManager.h"\
	".\s\SaveLoadFrontEnd.h"\
	".\s\scoringmanager.h"\
	".\s\slater_ind_anims_mac.h"\
	".\s\sounddata.h"\
	".\s\SoundScript.h"\
	".\s\specialmeter.h"\
	".\s\StatsFrontEnd.h"\
	".\s\surferdata.h"\
	".\s\SurferFrontEnd.h"\
	".\s\trick_system.h"\
	".\s\TrickBookFrontEnd.h"\
	".\s\trickdata.h"\
	".\s\trickregion.h"\
	".\s\tricks.h"\
	".\s\TutorialFrontEnd.h"\
	".\s\tutorialmanager.h"\
	".\s\VOEngine.h"\
	".\s\wave.h"\
	".\s\wavebreak.txt"\
	".\s\wavebreakstage.txt"\
	".\s\waveemitter.txt"\
	".\s\waveenum.h"\
	".\s\wavemarker.txt"\
	".\s\waveregion.txt"\
	".\s\wavestage.txt"\
	".\stl_heap.h"\
	".\wosgc\gc_algebra.h"\
	".\wosgc\gc_alloc.h"\
	".\wosgc\gc_errmsg.h"\
	".\wosgc\gc_file.h"\
	".\WOSGC\gc_gamesaver.h"\
	".\wosgc\gc_graphics.h"\
	".\wosgc\gc_input.h"\
	".\wosgc\gc_math.h"\
	".\wosgc\gc_movieplayer.h"\
	".\wosgc\gc_rasterize.h"\
	".\wosgc\gc_texturemgr.h"\
	".\wosgc\gc_timer.h"\
	".\wosgc\ngl_fixedstr.h"\
	".\wosgc\ngl_gc.h"\
	".\wosgc\ngl_version.h"\
	".\wosgc\nvl_gc.h"\
	".\WOSPS2\ps2_algebra.h"\
	".\WOSPS2\ps2_alloc.h"\
	".\WOSPS2\ps2_devopts.h"\
	".\WOSPS2\ps2_errmsg.h"\
	".\WOSPS2\ps2_file.h"\
	".\WOSPS2\ps2_gamesaver.h"\
	".\WOSPS2\ps2_graphics.h"\
	".\WOSPS2\ps2_input.h"\
	".\WOSPS2\ps2_m2vplayer.h"\
	".\WOSPS2\ps2_math.h"\
	".\WOSPS2\ps2_movieplayer.h"\
	".\WOSPS2\ps2_rasterize.h"\
	".\WOSPS2\ps2_texturemgr.h"\
	".\WOSPS2\ps2_timer.h"\
	".\WOSPS2\ps2_types.h"\
	".\wosxb\xb_algebra.h"\
	".\wosxb\xb_alloc.h"\
	".\wosxb\xb_errmsg.h"\
	".\wosxb\xb_file.h"\
	".\WOSXB\xb_gamesaver.h"\
	".\wosxb\xb_graphics.h"\
	".\wosxb\xb_input.h"\
	".\wosxb\xb_math.h"\
	".\wosxb\xb_movieplayer.h"\
	".\WOSXB\xb_portDVDCache.h"\
	".\wosxb\xb_rasterize.h"\
	".\wosxb\xb_string.h"\
	".\wosxb\xb_texturemgr.h"\
	".\wosxb\xb_timer.h"\
	".\wosxb\xb_types.h"\
	
# SUBTRACT BASE CPP /YX /Yc /Yu
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

DEP_CPP_FILES_=\
	"..\..\ngl\ps2\ngl_fixedstr.h"\
	"..\..\ngl\ps2\ngl_ps2.h"\
	"..\..\ngl\ps2\tim2.h"\
	"..\..\NSL\common\nl.h"\
	"..\..\NSL\common\nsl.h"\
	"..\..\NSL\common\ProjectOptions.h"\
	"..\..\NSL\gamecube\nl_gc.h"\
	"..\..\NSL\ps2\fifo_queue.cpp"\
	"..\..\NSL\ps2\fifo_queue.h"\
	"..\..\NSL\PS2\gas.h"\
	"..\..\NSL\PS2\gas_utility.h"\
	"..\..\NSL\PS2\nl_ps2.h"\
	"..\..\NSL\PS2\nsl_ps2.h"\
	"..\..\NSL\XBOX\nl_xbox.h"\
	"..\..\NSL\XBOX\nsl_xbox.h"\
	"..\..\NSL\XBOX\nsl_xbox_ext.h"\
	"..\..\NSL\XBOX\nsl_xbox_source.h"\
	"..\..\nsl\xbox\nsl_xbox_streams.h"\
	"..\..\NVL\ps2\nvl_ps2.h"\
	"..\..\NVL\ps2\nvlMPEG_ps2.h"\
	"..\..\NVL\ps2\nvlstream_ps2.h"\
	".\aggvertbuf.h"\
	".\ai_constants.h"\
	".\ai_interface.h"\
	".\ai_locomotion.h"\
	".\ai_polypath.h"\
	".\ai_polypath_cell.h"\
	".\ai_polypath_heap.h"\
	".\algebra.h"\
	".\anim.h"\
	".\anim_flavor.cpp"\
	".\anim_flavor.h"\
	".\anim_flavors.h"\
	".\anim_ids.h"\
	".\anim_maker.cpp"\
	".\anim_maker.h"\
	".\anim_user.cpp"\
	".\app.h"\
	".\archalloc.h"\
	".\avltree.h"\
	".\beam.h"\
	".\beam_signals.h"\
	".\billboard.h"\
	".\bone.h"\
	".\bound.h"\
	".\box_trigger_interface.h"\
	".\camera.h"\
	".\capsule.h"\
	".\cface.h"\
	".\chunkfile.h"\
	".\colgeom.h"\
	".\colmesh.h"\
	".\color.h"\
	".\commands.h"\
	".\conglom.h"\
	".\constants.h"\
	".\controller.h"\
	".\convex_box.h"\
	".\custom_stl.h"\
	".\debug.h"\
	".\devoptflags.h"\
	".\devoptints.h"\
	".\devoptstrs.h"\
	".\entflavor.h"\
	".\entity.h"\
	".\entity_anim.cpp"\
	".\entity_anim.h"\
	".\entity_interface.h"\
	".\entity_maker.h"\
	".\entity_signals.h"\
	".\entityflags.h"\
	".\entityid.h"\
	".\errorcontext.h"\
	".\face.h"\
	".\faceflags.h"\
	".\fast_vector.h"\
	".\file.h"\
	".\file_finder.h"\
	".\filespec.h"\
	".\frame_info.h"\
	".\game.h"\
	".\game_info_vars.h"\
	".\game_process.h"\
	".\gcglobals.h"\
	".\generator.h"\
	".\geomgr.h"\
	".\global.h"\
	".\global_signals.h"\
	".\graph.h"\
	".\guidance_sys.h"\
	".\hull.h"\
	".\hwmath.h"\
	".\hwmovieplayer.h"\
	".\hwrasterize.h"\
	".\hyperplane.h"\
	".\ini_parser.h"\
	".\inputmgr.h"\
	".\instance.h"\
	".\item.h"\
	".\item_signals.h"\
	".\joystick.h"\
	".\ks\aligned.h"\
	".\ks\ksheaps.h"\
	".\kshooks.h"\
	".\ksnsl.h"\
	".\ksnvl.h"\
	".\light.h"\
	".\linear_anim.h"\
	".\link_interface.h"\
	".\map_e.h"\
	".\marker.h"\
	".\material.h"\
	".\matfac.h"\
	".\maxskinbones.h"\
	".\meshrefs.h"\
	".\mic.h"\
	".\mobject.h"\
	".\mustash.h"\
	".\ngl.h"\
	".\osalloc.h"\
	".\osassert.h"\
	".\osdevopts.h"\
	".\oserrmsg.h"\
	".\osfile.h"\
	".\ostimer.h"\
	".\particle.h"\
	".\path.h"\
	".\pc_port.h"\
	".\physical_interface.h"\
	".\plane.h"\
	".\platform_defines.h"\
	".\pmesh.h"\
	".\po.h"\
	".\po_anim.cpp"\
	".\po_anim.h"\
	".\portal.h"\
	".\profiler.h"\
	".\projconst.h"\
	".\project.h"\
	".\pstring.h"\
	".\quatcomp.h"\
	".\random.h"\
	".\rect.h"\
	".\refptr.h"\
	".\region.h"\
	".\region_graph.h"\
	".\render_data.h"\
	".\renderflav.h"\
	".\scene_anim.cpp"\
	".\scene_anim.h"\
	".\script_lib_mfg.h"\
	".\script_library_class.h"\
	".\script_mfg_signals.h"\
	".\script_object.h"\
	".\signal_anim.cpp"\
	".\signal_anim.h"\
	".\signals.h"\
	".\SimpleAssert.h"\
	".\singleton.h"\
	".\so_data_block.h"\
	".\sphere.h"\
	".\stashes.h"\
	".\staticmem.h"\
	".\stl_adapter.h"\
	".\stringx.h"\
	".\terrain.h"\
	".\text_font.h"\
	".\textfile.h"\
	".\timer.h"\
	".\txtcoord.h"\
	".\types.h"\
	".\usefulmath.h"\
	".\users.h"\
	".\vert.h"\
	".\visrep.h"\
	".\vm_executable.h"\
	".\vm_symbol.h"\
	".\vm_symbol_list.h"\
	".\warnlvl.h"\
	".\wds.h"\
	".\wedge.h"\
	".\widget.h"\
	".\widget_entity.h"\
	".\xbglobals.h"\
	
NODEP_CPP_FILES_=\
	"..\..\NSL\pc\nl_pc.h"\
	"..\..\NSL\pc\nsl_pc_ext.h"\
	".\algo.h"\
	".\defalloc.h"\
	".\ekernel.h"\
	".\eregs.h"\
	".\estruct.h"\
	".\etypes.h"\
	".\gc\nsl_gc.h"\
	".\gc_arammgr.h"\
	".\graphics.h"\
	".\hwosmks\mks_devopts.h"\
	".\hwosmks\mks_errmsg.h"\
	".\hwosmks\mks_movieplayer.h"\
	".\hwosmks\set5_rasterize.h"\
	".\hwosmks\set5_timer.h"\
	".\hwosmks\sh4_math.h"\
	".\hwosmks\sy_alloc.h"\
	".\hwosmks\sy_file.h"\
	".\hwosnglpc\algebra.h"\
	".\hwosnglpc\pc_algebra.h"\
	".\hwosnull\null_alloc.h"\
	".\hwosnull\null_devopts.h"\
	".\hwosnull\null_errmsg.h"\
	".\hwosnull\null_file.h"\
	".\hwosnull\null_math.h"\
	".\hwosnull\null_movieplayer.h"\
	".\hwosnull\null_rasterize.h"\
	".\hwosnull\null_timer.h"\
	".\hwospc\d3d_rasterize.h"\
	".\hwospc\pc_algebra.h"\
	".\hwospc\pc_movieplayer.h"\
	".\hwospc\w32_alloc.h"\
	".\hwospc\w32_devopts.h"\
	".\hwospc\w32_errmsg.h"\
	".\hwospc\w32_file.h"\
	".\hwospc\w32_timer.h"\
	".\hwospc\x86_math.h"\
	".\hwosxb\ngl_xbox.h"\
	".\hwosxb\nvl_xbox.h"\
	".\ibcdvd.h"\
	".\ibdma.h"\
	".\ibgraph.h"\
	".\ibipu.h"\
	".\ibmc.h"\
	".\ibmpeg.h"\
	".\ibpad.h"\
	".\ibpc.h"\
	".\ibusbkb.h"\
	".\ibvu0.h"\
	".\ifdev.h"\
	".\LipSync_Anim.h"\
	".\ngl_xbox.h"\
	".\nsl.h"\
	".\nvl_gc.h"\
	".\nvl_xbox.h"\
	".\nvlstream_gc.h"\
	".\s\AccompFrontEnd.h"\
	".\s\andersen_ind_anims_mac.h"\
	".\s\beach.h"\
	".\s\beachdata.h"\
	".\s\BeachFrontEnd.h"\
	".\s\board.h"\
	".\s\board_anims_mac.h"\
	".\s\boarddata.h"\
	".\s\BoardFrontEnd.h"\
	".\s\career.h"\
	".\s\careerdata.h"\
	".\s\carrol_ind_anims_mac.h"\
	".\s\challenge.h"\
	".\s\challenge_icon.h"\
	".\s\challenge_photo.h"\
	".\s\cheat.h"\
	".\s\CheatFrontEnd.h"\
	".\s\compressedphoto.h"\
	".\s\coords.h"\
	".\s\curren_ind_anims_mac.h"\
	".\s\eventmanager.h"\
	".\s\ExtrasFrontEnd.h"\
	".\s\FEAnim.h"\
	".\s\FEEntityManager.h"\
	".\s\FEMenu.h"\
	".\s\FEPanel.h"\
	".\s\fletcher_ind_anims_mac.h"\
	".\s\floatobj.h"\
	".\s\frankenreiter_ind_anims_mac.h"\
	".\s\FrontEndManager.h"\
	".\s\FrontEndMenus.h"\
	".\s\GameData.h"\
	".\s\GlobalData.h"\
	".\s\globaltextenum.h"\
	".\s\GraphicalMenuSystem.h"\
	".\s\HighScoreFrontEnd.h"\
	".\s\igo_widget.h"\
	".\s\igo_widget_analogclock.h"\
	".\s\igo_widget_balance.h"\
	".\s\igo_widget_camera.h"\
	".\s\igo_widget_fanmeter.h"\
	".\s\igo_widget_iconcount.h"\
	".\s\igo_widget_iconradar.h"\
	".\s\igo_widget_meterchallenge.h"\
	".\s\igo_widget_objectalert.h"\
	".\s\igo_widget_photo.h"\
	".\s\igo_widget_replay.h"\
	".\s\igo_widget_simple.h"\
	".\s\igo_widget_skillchallenge.h"\
	".\s\igo_widget_specialmeter.h"\
	".\s\igo_widget_splitclock.h"\
	".\s\igo_widget_splitmeter.h"\
	".\s\igo_widget_splitscore.h"\
	".\s\igo_widget_splitter.h"\
	".\s\igo_widget_timeattack.h"\
	".\s\igo_widget_waveindicator.h"\
	".\s\IGOFrontEnd.h"\
	".\s\igohintmanager.h"\
	".\s\igoiconmanager.h"\
	".\s\igolearn_new_trickmanager.h"\
	".\s\ik_object.h"\
	".\s\irons_ind_anims_mac.h"\
	".\s\judge.h"\
	".\s\kellyslater_controller.h"\
	".\s\kellyslater_shared_anims_mac.h"\
	".\s\kellyslater_states_mac.h"\
	".\s\ks_camera.h"\
	".\s\ksdbmenu.h"\
	".\s\ksngl.h"\
	".\s\ksreplay.h"\
	".\s\LogbookFrontEnd.h"\
	".\s\machado_ind_anims_mac.h"\
	".\s\MainFrontEnd.h"\
	".\s\Map.h"\
	".\s\MCDetectFrontEnd.h"\
	".\s\menu.h"\
	".\s\menusys.h"\
	".\s\mode.h"\
	".\s\mode_headtohead.h"\
	".\s\mode_meterattack.h"\
	".\s\mode_push.h"\
	".\s\mode_timeattack.h"\
	".\s\MultiFrontEnd.h"\
	".\s\MusicMan.h"\
	".\s\ode.h"\
	".\s\osGameSaver.h"\
	".\s\PAL60FrontEnd.h"\
	".\s\PathData.h"\
	".\s\PhotoFrontEnd.h"\
	".\s\physics.h"\
	".\s\player.h"\
	".\s\replay.h"\
	".\s\robb_ind_anims_mac.h"\
	".\s\rumbleManager.h"\
	".\s\SaveLoadFrontEnd.h"\
	".\s\scoringmanager.h"\
	".\s\slater_ind_anims_mac.h"\
	".\s\sounddata.h"\
	".\s\SoundScript.h"\
	".\s\specialmeter.h"\
	".\s\StatsFrontEnd.h"\
	".\s\surferdata.h"\
	".\s\SurferFrontEnd.h"\
	".\s\trick_system.h"\
	".\s\TrickBookFrontEnd.h"\
	".\s\trickdata.h"\
	".\s\trickregion.h"\
	".\s\tricks.h"\
	".\s\tutorialdata.h"\
	".\s\TutorialFrontEnd.h"\
	".\s\tutorialmanager.h"\
	".\s\VOEngine.h"\
	".\s\wave.h"\
	".\s\wavebreak.txt"\
	".\s\wavebreakstage.txt"\
	".\s\waveemitter.txt"\
	".\s\waveenum.h"\
	".\s\wavemarker.txt"\
	".\s\waveregion.txt"\
	".\s\wavestage.txt"\
	".\stl_heap.h"\
	".\wosgc\gc_algebra.h"\
	".\wosgc\gc_alloc.h"\
	".\wosgc\gc_errmsg.h"\
	".\wosgc\gc_file.h"\
	".\WOSGC\gc_gamesaver.h"\
	".\wosgc\gc_graphics.h"\
	".\wosgc\gc_input.h"\
	".\wosgc\gc_math.h"\
	".\wosgc\gc_movieplayer.h"\
	".\wosgc\gc_rasterize.h"\
	".\wosgc\gc_texturemgr.h"\
	".\wosgc\gc_timer.h"\
	".\wosgc\ngl_fixedstr.h"\
	".\wosgc\ngl_gc.h"\
	".\wosgc\ngl_version.h"\
	".\wosgc\nvl_gc.h"\
	".\WOSPS2\ps2_algebra.h"\
	".\WOSPS2\ps2_alloc.h"\
	".\WOSPS2\ps2_devopts.h"\
	".\WOSPS2\ps2_errmsg.h"\
	".\WOSPS2\ps2_file.h"\
	".\WOSPS2\ps2_gamesaver.h"\
	".\WOSPS2\ps2_graphics.h"\
	".\WOSPS2\ps2_input.h"\
	".\WOSPS2\ps2_m2vplayer.h"\
	".\WOSPS2\ps2_math.h"\
	".\WOSPS2\ps2_movieplayer.h"\
	".\WOSPS2\ps2_rasterize.h"\
	".\WOSPS2\ps2_texturemgr.h"\
	".\WOSPS2\ps2_timer.h"\
	".\WOSPS2\ps2_types.h"\
	".\wosxb\xb_algebra.h"\
	".\wosxb\xb_alloc.h"\
	".\wosxb\xb_errmsg.h"\
	".\wosxb\xb_file.h"\
	".\WOSXB\xb_gamesaver.h"\
	".\wosxb\xb_graphics.h"\
	".\wosxb\xb_input.h"\
	".\wosxb\xb_math.h"\
	".\wosxb\xb_movieplayer.h"\
	".\WOSXB\xb_portDVDCache.h"\
	".\wosxb\xb_rasterize.h"\
	".\wosxb\xb_string.h"\
	".\wosxb\xb_texturemgr.h"\
	".\wosxb\xb_timer.h"\
	".\wosxb\xb_types.h"\
	
# SUBTRACT BASE CPP /YX /Yc /Yu
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\files_entity.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

DEP_CPP_FILES_E=\
	"..\..\ngl\ps2\ngl_fixedstr.h"\
	"..\..\ngl\ps2\ngl_ps2.h"\
	"..\..\ngl\ps2\tim2.h"\
	"..\..\NSL\common\nl.h"\
	"..\..\NSL\common\nsl.h"\
	"..\..\NSL\common\ProjectOptions.h"\
	"..\..\NSL\gamecube\nl_gc.h"\
	"..\..\NSL\ps2\fifo_queue.cpp"\
	"..\..\NSL\ps2\fifo_queue.h"\
	"..\..\NSL\PS2\gas.h"\
	"..\..\NSL\PS2\gas_utility.h"\
	"..\..\NSL\PS2\nl_ps2.h"\
	"..\..\NSL\PS2\nsl_ps2.h"\
	"..\..\NSL\XBOX\nl_xbox.h"\
	"..\..\NSL\XBOX\nsl_xbox.h"\
	"..\..\NSL\XBOX\nsl_xbox_ext.h"\
	"..\..\NSL\XBOX\nsl_xbox_source.h"\
	"..\..\nsl\xbox\nsl_xbox_streams.h"\
	"..\..\NVL\ps2\nvl_ps2.h"\
	"..\..\NVL\ps2\nvlMPEG_ps2.h"\
	"..\..\NVL\ps2\nvlstream_ps2.h"\
	".\aggvertbuf.h"\
	".\ai_constants.h"\
	".\ai_interface.h"\
	".\ai_locomotion.h"\
	".\ai_polypath.h"\
	".\ai_polypath_cell.h"\
	".\ai_polypath_heap.h"\
	".\algebra.h"\
	".\anim.h"\
	".\anim_flavor.h"\
	".\anim_flavors.h"\
	".\anim_ids.h"\
	".\anim_maker.h"\
	".\animation_interface.cpp"\
	".\animation_interface.h"\
	".\app.h"\
	".\archalloc.h"\
	".\attribute_template.h"\
	".\avltree.h"\
	".\b_spline.h"\
	".\beam.h"\
	".\beam_signals.h"\
	".\billboard.h"\
	".\biped_ids.h"\
	".\bone.cpp"\
	".\bone.h"\
	".\bound.h"\
	".\box_trigger_interface.cpp"\
	".\box_trigger_interface.h"\
	".\bsp_collide.h"\
	".\camera.h"\
	".\capsule.h"\
	".\cface.h"\
	".\character_hard_attribs.h"\
	".\character_soft_attribs.h"\
	".\chunkfile.h"\
	".\colgeom.h"\
	".\collide.h"\
	".\colmesh.h"\
	".\color.h"\
	".\commands.h"\
	".\conglom.h"\
	".\console.h"\
	".\constants.h"\
	".\controller.h"\
	".\convex_box.h"\
	".\custom_stl.h"\
	".\debug.h"\
	".\debug_render.h"\
	".\devoptflags.h"\
	".\devoptints.h"\
	".\devoptstrs.h"\
	".\dropshadow.h"\
	".\element.h"\
	".\entflavor.h"\
	".\entity.cpp"\
	".\entity.h"\
	".\entity_anim.h"\
	".\entity_hard_attribs.h"\
	".\entity_interface.h"\
	".\entity_maker.h"\
	".\entity_signals.h"\
	".\entity_soft_attribs.h"\
	".\entityflags.h"\
	".\entityid.h"\
	".\errorcontext.h"\
	".\face.h"\
	".\faceflags.h"\
	".\fast_vector.h"\
	".\file.h"\
	".\file_finder.h"\
	".\filespec.h"\
	".\fogmgr.h"\
	".\forceflags.h"\
	".\frame_info.h"\
	".\game.h"\
	".\game_info_vars.h"\
	".\game_process.h"\
	".\gcglobals.h"\
	".\generator.h"\
	".\geomgr.h"\
	".\global.h"\
	".\global_signals.h"\
	".\graph.h"\
	".\guidance_sys.h"\
	".\hard_attrib_interface.cpp"\
	".\hard_attrib_interface.h"\
	".\hull.h"\
	".\hwaudio.h"\
	".\hwmath.h"\
	".\hwmovieplayer.h"\
	".\hwrasterize.h"\
	".\hyperplane.h"\
	".\ini_parser.h"\
	".\inputmgr.h"\
	".\instance.h"\
	".\iri.h"\
	".\item.h"\
	".\item_signals.h"\
	".\joint.h"\
	".\joystick.h"\
	".\keyboard.h"\
	".\ks\ksheaps.h"\
	".\kshooks.h"\
	".\ksnsl.h"\
	".\ksnvl.h"\
	".\light.h"\
	".\lightmgr.h"\
	".\linear_anim.h"\
	".\link_interface.cpp"\
	".\link_interface.h"\
	".\map_e.h"\
	".\material.h"\
	".\matfac.h"\
	".\maxskinbones.h"\
	".\mbi.h"\
	".\meshrefs.h"\
	".\mic.h"\
	".\mobject.h"\
	".\msgboard.h"\
	".\mustash.h"\
	".\ngl.h"\
	".\ngl_support.h"\
	".\osalloc.h"\
	".\osassert.h"\
	".\osdevopts.h"\
	".\oserrmsg.h"\
	".\osfile.h"\
	".\ostimer.h"\
	".\owner_interface.cpp"\
	".\owner_interface.h"\
	".\path.h"\
	".\pc_port.h"\
	".\physical_interface.cpp"\
	".\physical_interface.h"\
	".\plane.h"\
	".\platform_defines.h"\
	".\pmesh.h"\
	".\po.h"\
	".\po_anim.h"\
	".\polytube.cpp"\
	".\polytube.h"\
	".\portal.h"\
	".\profiler.h"\
	".\projconst.h"\
	".\project.h"\
	".\pstring.h"\
	".\random.h"\
	".\rect.h"\
	".\refptr.h"\
	".\region.h"\
	".\region_graph.h"\
	".\render_data.h"\
	".\render_interface.cpp"\
	".\render_interface.h"\
	".\renderflav.h"\
	".\scene_anim.h"\
	".\script_data_interface.cpp"\
	".\script_data_interface.h"\
	".\script_lib_mfg.h"\
	".\script_library_class.h"\
	".\script_mfg_signals.h"\
	".\script_object.h"\
	".\signal_anim.h"\
	".\signals.h"\
	".\SimpleAssert.h"\
	".\singleton.h"\
	".\skeleton_interface.cpp"\
	".\skeleton_interface.h"\
	".\slave_interface.cpp"\
	".\slave_interface.h"\
	".\so_data_block.h"\
	".\soft_attrib_interface.cpp"\
	".\soft_attrib_interface.h"\
	".\sound_group.h"\
	".\sound_interface.cpp"\
	".\sound_interface.h"\
	".\sphere.h"\
	".\stashes.h"\
	".\staticmem.h"\
	".\stl_adapter.h"\
	".\stringx.h"\
	".\terrain.h"\
	".\text_font.h"\
	".\textfile.h"\
	".\time_interface.cpp"\
	".\time_interface.h"\
	".\timer.h"\
	".\txtcoord.h"\
	".\types.h"\
	".\usefulmath.h"\
	".\users.h"\
	".\vert.h"\
	".\vertwork.h"\
	".\visrep.h"\
	".\vm_executable.h"\
	".\vm_stack.h"\
	".\vm_symbol.h"\
	".\vm_symbol_list.h"\
	".\vm_thread.h"\
	".\warnlvl.h"\
	".\wds.h"\
	".\wedge.h"\
	".\widget.h"\
	".\widget_entity.h"\
	".\xbglobals.h"\
	{$(INCLUDE)}"eekernel.h"\
	{$(INCLUDE)}"eeregs.h"\
	{$(INCLUDE)}"eestruct.h"\
	{$(INCLUDE)}"eetypes.h"\
	{$(INCLUDE)}"hwosgc\gc_algebra.h"\
	{$(INCLUDE)}"hwosgc\gc_alloc.h"\
	{$(INCLUDE)}"hwosgc\gc_audio.h"\
	{$(INCLUDE)}"hwosgc\gc_errmsg.h"\
	{$(INCLUDE)}"hwosgc\gc_file.h"\
	{$(INCLUDE)}"HWOSGC\gc_gamesaver.h"\
	{$(INCLUDE)}"hwosgc\gc_graphics.h"\
	{$(INCLUDE)}"hwosgc\gc_input.h"\
	{$(INCLUDE)}"hwosgc\gc_math.h"\
	{$(INCLUDE)}"hwosgc\gc_movieplayer.h"\
	{$(INCLUDE)}"hwosgc\gc_rasterize.h"\
	{$(INCLUDE)}"hwosgc\gc_texturemgr.h"\
	{$(INCLUDE)}"hwosgc\gc_timer.h"\
	{$(INCLUDE)}"hwosgc\ngl_fixedstr.h"\
	{$(INCLUDE)}"hwosgc\ngl_gc.h"\
	{$(INCLUDE)}"hwosgc\ngl_version.h"\
	{$(INCLUDE)}"hwosgc\nvl_gc.h"\
	{$(INCLUDE)}"HWOSPS2\gas.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_algebra.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_alloc.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_audio.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_devopts.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_errmsg.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_file.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_gamesaver.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_graphics.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_input.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_m2vplayer.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_math.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_movieplayer.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_rasterize.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_texturemgr.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_timer.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_types.h"\
	{$(INCLUDE)}"hwosxb\pstring.h"\
	{$(INCLUDE)}"hwosxb\xb_algebra.h"\
	{$(INCLUDE)}"hwosxb\xb_alloc.h"\
	{$(INCLUDE)}"hwosxb\xb_audio.h"\
	{$(INCLUDE)}"hwosxb\xb_errmsg.h"\
	{$(INCLUDE)}"hwosxb\xb_file.h"\
	{$(INCLUDE)}"HWOSXB\xb_gamesaver.h"\
	{$(INCLUDE)}"hwosxb\xb_graphics.h"\
	{$(INCLUDE)}"hwosxb\xb_input.h"\
	{$(INCLUDE)}"hwosxb\xb_math.h"\
	{$(INCLUDE)}"hwosxb\xb_movieplayer.h"\
	{$(INCLUDE)}"HWOSXB\xb_portDVDCache.h"\
	{$(INCLUDE)}"hwosxb\xb_rasterize.h"\
	{$(INCLUDE)}"hwosxb\xb_string.h"\
	{$(INCLUDE)}"hwosxb\xb_texturemgr.h"\
	{$(INCLUDE)}"hwosxb\xb_timer.h"\
	{$(INCLUDE)}"hwosxb\xb_types.h"\
	{$(INCLUDE)}"ks\AccompFrontEnd.h"\
	{$(INCLUDE)}"ks\andersen_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\beach.h"\
	{$(INCLUDE)}"ks\beachdata.h"\
	{$(INCLUDE)}"ks\BeachFrontEnd.h"\
	{$(INCLUDE)}"ks\board.h"\
	{$(INCLUDE)}"ks\board_anims_mac.h"\
	{$(INCLUDE)}"ks\boarddata.h"\
	{$(INCLUDE)}"ks\BoardFrontEnd.h"\
	{$(INCLUDE)}"ks\career.h"\
	{$(INCLUDE)}"ks\careerdata.h"\
	{$(INCLUDE)}"ks\carrol_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\challenge.h"\
	{$(INCLUDE)}"ks\challenge_icon.h"\
	{$(INCLUDE)}"ks\challenge_photo.h"\
	{$(INCLUDE)}"ks\cheat.h"\
	{$(INCLUDE)}"ks\CheatFrontEnd.h"\
	{$(INCLUDE)}"ks\compressedphoto.h"\
	{$(INCLUDE)}"ks\coords.h"\
	{$(INCLUDE)}"ks\curren_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\eventmanager.h"\
	{$(INCLUDE)}"ks\ExtrasFrontEnd.h"\
	{$(INCLUDE)}"ks\FEAnim.h"\
	{$(INCLUDE)}"ks\FEEntityManager.h"\
	{$(INCLUDE)}"ks\FEMenu.h"\
	{$(INCLUDE)}"ks\FEPanel.h"\
	{$(INCLUDE)}"ks\fletcher_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\floatobj.h"\
	{$(INCLUDE)}"ks\frankenreiter_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\FrontEndManager.h"\
	{$(INCLUDE)}"ks\FrontEndMenus.h"\
	{$(INCLUDE)}"ks\GameData.h"\
	{$(INCLUDE)}"ks\GlobalData.h"\
	{$(INCLUDE)}"ks\globaltextenum.h"\
	{$(INCLUDE)}"ks\GraphicalMenuSystem.h"\
	{$(INCLUDE)}"ks\HighScoreFrontEnd.h"\
	{$(INCLUDE)}"ks\igo_widget.h"\
	{$(INCLUDE)}"ks\igo_widget_analogclock.h"\
	{$(INCLUDE)}"ks\igo_widget_balance.h"\
	{$(INCLUDE)}"ks\igo_widget_camera.h"\
	{$(INCLUDE)}"ks\igo_widget_fanmeter.h"\
	{$(INCLUDE)}"ks\igo_widget_grid.h"\
	{$(INCLUDE)}"ks\igo_widget_iconcount.h"\
	{$(INCLUDE)}"ks\igo_widget_iconradar.h"\
	{$(INCLUDE)}"ks\igo_widget_meterchallenge.h"\
	{$(INCLUDE)}"ks\igo_widget_objectalert.h"\
	{$(INCLUDE)}"ks\igo_widget_photo.h"\
	{$(INCLUDE)}"ks\igo_widget_replay.h"\
	{$(INCLUDE)}"ks\igo_widget_simple.h"\
	{$(INCLUDE)}"ks\igo_widget_skillchallenge.h"\
	{$(INCLUDE)}"ks\igo_widget_specialmeter.h"\
	{$(INCLUDE)}"ks\igo_widget_splitclock.h"\
	{$(INCLUDE)}"ks\igo_widget_splitmeter.h"\
	{$(INCLUDE)}"ks\igo_widget_splitscore.h"\
	{$(INCLUDE)}"ks\igo_widget_splitter.h"\
	{$(INCLUDE)}"ks\igo_widget_timeattack.h"\
	{$(INCLUDE)}"ks\igo_widget_waveindicator.h"\
	{$(INCLUDE)}"ks\IGOFrontEnd.h"\
	{$(INCLUDE)}"ks\igohintmanager.h"\
	{$(INCLUDE)}"ks\igoiconmanager.h"\
	{$(INCLUDE)}"ks\igolearn_new_trickmanager.h"\
	{$(INCLUDE)}"ks\ik_object.h"\
	{$(INCLUDE)}"ks\irons_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\judge.h"\
	{$(INCLUDE)}"ks\kellyslater_controller.h"\
	{$(INCLUDE)}"ks\kellyslater_shared_anims_mac.h"\
	{$(INCLUDE)}"ks\kellyslater_states_mac.h"\
	{$(INCLUDE)}"ks\ks_camera.h"\
	{$(INCLUDE)}"ks\ksdbmenu.h"\
	{$(INCLUDE)}"ks\ksngl.h"\
	{$(INCLUDE)}"ks\ksreplay.h"\
	{$(INCLUDE)}"ks\LogbookFrontEnd.h"\
	{$(INCLUDE)}"ks\machado_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\MainFrontEnd.h"\
	{$(INCLUDE)}"ks\Map.h"\
	{$(INCLUDE)}"ks\MCDetectFrontEnd.h"\
	{$(INCLUDE)}"ks\menu.h"\
	{$(INCLUDE)}"ks\menusys.h"\
	{$(INCLUDE)}"ks\mode.h"\
	{$(INCLUDE)}"ks\mode_headtohead.h"\
	{$(INCLUDE)}"ks\mode_meterattack.h"\
	{$(INCLUDE)}"ks\mode_push.h"\
	{$(INCLUDE)}"ks\mode_timeattack.h"\
	{$(INCLUDE)}"ks\MultiFrontEnd.h"\
	{$(INCLUDE)}"ks\MusicMan.h"\
	{$(INCLUDE)}"ks\ode.h"\
	{$(INCLUDE)}"ks\osGameSaver.h"\
	{$(INCLUDE)}"ks\PAL60FrontEnd.h"\
	{$(INCLUDE)}"ks\PathData.h"\
	{$(INCLUDE)}"ks\PhotoFrontEnd.h"\
	{$(INCLUDE)}"ks\physics.h"\
	{$(INCLUDE)}"ks\player.h"\
	{$(INCLUDE)}"ks\replay.h"\
	{$(INCLUDE)}"ks\robb_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\rumbleManager.h"\
	{$(INCLUDE)}"ks\SaveLoadFrontEnd.h"\
	{$(INCLUDE)}"ks\scoringmanager.h"\
	{$(INCLUDE)}"ks\slater_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\sounddata.h"\
	{$(INCLUDE)}"ks\SoundScript.h"\
	{$(INCLUDE)}"ks\specialmeter.h"\
	{$(INCLUDE)}"ks\StatsFrontEnd.h"\
	{$(INCLUDE)}"ks\surferdata.h"\
	{$(INCLUDE)}"ks\SurferFrontEnd.h"\
	{$(INCLUDE)}"ks\trick_system.h"\
	{$(INCLUDE)}"ks\TrickBookFrontEnd.h"\
	{$(INCLUDE)}"ks\trickdata.h"\
	{$(INCLUDE)}"ks\trickregion.h"\
	{$(INCLUDE)}"ks\tricks.h"\
	{$(INCLUDE)}"ks\tutorialdata.h"\
	{$(INCLUDE)}"ks\TutorialFrontEnd.h"\
	{$(INCLUDE)}"ks\tutorialmanager.h"\
	{$(INCLUDE)}"ks\underwtr.h"\
	{$(INCLUDE)}"ks\VOEngine.h"\
	{$(INCLUDE)}"ks\wave.h"\
	{$(INCLUDE)}"ks\wavebreak.txt"\
	{$(INCLUDE)}"ks\wavebreakstage.txt"\
	{$(INCLUDE)}"ks\waveemitter.txt"\
	{$(INCLUDE)}"ks\waveenum.h"\
	{$(INCLUDE)}"ks\wavemarker.txt"\
	{$(INCLUDE)}"ks\waveregion.txt"\
	{$(INCLUDE)}"ks\wavestage.txt"\
	{$(INCLUDE)}"libcdvd.h"\
	{$(INCLUDE)}"libdma.h"\
	{$(INCLUDE)}"libgraph.h"\
	{$(INCLUDE)}"libipu.h"\
	{$(INCLUDE)}"libmc.h"\
	{$(INCLUDE)}"libmpeg.h"\
	{$(INCLUDE)}"libpad.h"\
	{$(INCLUDE)}"libpc.h"\
	{$(INCLUDE)}"libusbkb.h"\
	{$(INCLUDE)}"libvu0.h"\
	{$(INCLUDE)}"sifdev.h"\
	{$(INCLUDE)}"xgraphics.h"\
	
NODEP_CPP_FILES_E=\
	"..\..\NSL\pc\nl_pc.h"\
	"..\..\NSL\pc\nsl_pc_ext.h"\
	".\algo.h"\
	".\defalloc.h"\
	".\gc_arammgr.h"\
	".\hwosmks\am_audio.h"\
	".\hwosmks\mks_devopts.h"\
	".\hwosmks\mks_errmsg.h"\
	".\hwosmks\mks_movieplayer.h"\
	".\hwosmks\set5_rasterize.h"\
	".\hwosmks\set5_timer.h"\
	".\hwosmks\sh4_math.h"\
	".\hwosmks\sy_alloc.h"\
	".\hwosmks\sy_file.h"\
	".\hwosnglpc\algebra.h"\
	".\hwosnglpc\pc_algebra.h"\
	".\hwosnull\null_alloc.h"\
	".\hwosnull\null_audio.h"\
	".\hwosnull\null_devopts.h"\
	".\hwosnull\null_errmsg.h"\
	".\hwosnull\null_file.h"\
	".\hwosnull\null_math.h"\
	".\hwosnull\null_movieplayer.h"\
	".\hwosnull\null_rasterize.h"\
	".\hwosnull\null_timer.h"\
	".\hwospc\d3d_rasterize.h"\
	".\hwospc\ds_audio.h"\
	".\hwospc\pc_algebra.h"\
	".\hwospc\pc_movieplayer.h"\
	".\hwospc\w32_alloc.h"\
	".\hwospc\w32_devopts.h"\
	".\hwospc\w32_errmsg.h"\
	".\hwospc\w32_file.h"\
	".\hwospc\w32_timer.h"\
	".\hwospc\x86_math.h"\
	".\hwosxb\ngl_xbox.h"\
	".\hwosxb\nvl_xbox.h"\
	".\LipSync_Anim.h"\
	".\ngl_xbox.h"\
	".\nvl_gc.h"\
	".\nvl_xbox.h"\
	".\nvlstream_gc.h"\
	".\stl_heap.h"\
	
# ADD BASE CPP /Yu
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

DEP_CPP_FILES_E=\
	"..\..\ngl\ps2\ngl_fixedstr.h"\
	"..\..\ngl\ps2\ngl_ps2.h"\
	"..\..\ngl\ps2\tim2.h"\
	"..\..\NSL\common\nl.h"\
	"..\..\NSL\common\nsl.h"\
	"..\..\NSL\common\ProjectOptions.h"\
	"..\..\NSL\gamecube\nl_gc.h"\
	"..\..\NSL\ps2\fifo_queue.cpp"\
	"..\..\NSL\ps2\fifo_queue.h"\
	"..\..\NSL\PS2\gas.h"\
	"..\..\NSL\PS2\gas_utility.h"\
	"..\..\NSL\PS2\nl_ps2.h"\
	"..\..\NSL\PS2\nsl_ps2.h"\
	"..\..\NSL\XBOX\nl_xbox.h"\
	"..\..\NSL\XBOX\nsl_xbox.h"\
	"..\..\NSL\XBOX\nsl_xbox_ext.h"\
	"..\..\NSL\XBOX\nsl_xbox_source.h"\
	"..\..\nsl\xbox\nsl_xbox_streams.h"\
	"..\..\NVL\ps2\nvl_ps2.h"\
	"..\..\NVL\ps2\nvlMPEG_ps2.h"\
	"..\..\NVL\ps2\nvlstream_ps2.h"\
	".\aggvertbuf.h"\
	".\ai_constants.h"\
	".\ai_interface.h"\
	".\ai_locomotion.h"\
	".\ai_polypath.h"\
	".\ai_polypath_cell.h"\
	".\ai_polypath_heap.h"\
	".\algebra.h"\
	".\anim.h"\
	".\anim_flavor.h"\
	".\anim_flavors.h"\
	".\anim_ids.h"\
	".\anim_maker.h"\
	".\animation_interface.cpp"\
	".\animation_interface.h"\
	".\app.h"\
	".\archalloc.h"\
	".\attribute_template.h"\
	".\avltree.h"\
	".\b_spline.h"\
	".\beam.h"\
	".\beam_signals.h"\
	".\billboard.h"\
	".\biped_ids.h"\
	".\bone.cpp"\
	".\bone.h"\
	".\bound.h"\
	".\box_trigger_interface.cpp"\
	".\box_trigger_interface.h"\
	".\bsp_collide.h"\
	".\camera.h"\
	".\capsule.h"\
	".\cface.h"\
	".\character_hard_attribs.h"\
	".\character_soft_attribs.h"\
	".\chunkfile.h"\
	".\colgeom.h"\
	".\collide.h"\
	".\colmesh.h"\
	".\color.h"\
	".\commands.h"\
	".\conglom.h"\
	".\console.h"\
	".\constants.h"\
	".\controller.h"\
	".\convex_box.h"\
	".\custom_stl.h"\
	".\debug.h"\
	".\debug_render.h"\
	".\devoptflags.h"\
	".\devoptints.h"\
	".\devoptstrs.h"\
	".\dropshadow.h"\
	".\element.h"\
	".\entflavor.h"\
	".\entity.cpp"\
	".\entity.h"\
	".\entity_anim.h"\
	".\entity_hard_attribs.h"\
	".\entity_interface.h"\
	".\entity_maker.h"\
	".\entity_signals.h"\
	".\entity_soft_attribs.h"\
	".\entityflags.h"\
	".\entityid.h"\
	".\errorcontext.h"\
	".\face.h"\
	".\faceflags.h"\
	".\fast_vector.h"\
	".\file.h"\
	".\file_finder.h"\
	".\filespec.h"\
	".\fogmgr.h"\
	".\forceflags.h"\
	".\frame_info.h"\
	".\game.h"\
	".\game_info_vars.h"\
	".\game_process.h"\
	".\gcglobals.h"\
	".\generator.h"\
	".\geomgr.h"\
	".\global.h"\
	".\global_signals.h"\
	".\graph.h"\
	".\guidance_sys.h"\
	".\hard_attrib_interface.cpp"\
	".\hard_attrib_interface.h"\
	".\hull.h"\
	".\hwaudio.h"\
	".\hwmath.h"\
	".\hwmovieplayer.h"\
	".\hwrasterize.h"\
	".\hyperplane.h"\
	".\ini_parser.h"\
	".\inputmgr.h"\
	".\instance.h"\
	".\iri.h"\
	".\item.h"\
	".\item_signals.h"\
	".\joint.h"\
	".\joystick.h"\
	".\keyboard.h"\
	".\ks\ksheaps.h"\
	".\kshooks.h"\
	".\ksnsl.h"\
	".\ksnvl.h"\
	".\light.h"\
	".\lightmgr.h"\
	".\linear_anim.h"\
	".\link_interface.cpp"\
	".\link_interface.h"\
	".\map_e.h"\
	".\material.h"\
	".\matfac.h"\
	".\maxskinbones.h"\
	".\mbi.h"\
	".\meshrefs.h"\
	".\mic.h"\
	".\mobject.h"\
	".\msgboard.h"\
	".\mustash.h"\
	".\ngl.h"\
	".\ngl_support.h"\
	".\osalloc.h"\
	".\osassert.h"\
	".\osdevopts.h"\
	".\oserrmsg.h"\
	".\osfile.h"\
	".\ostimer.h"\
	".\owner_interface.cpp"\
	".\owner_interface.h"\
	".\path.h"\
	".\pc_port.h"\
	".\physical_interface.cpp"\
	".\physical_interface.h"\
	".\plane.h"\
	".\platform_defines.h"\
	".\pmesh.h"\
	".\po.h"\
	".\po_anim.h"\
	".\polytube.cpp"\
	".\polytube.h"\
	".\portal.h"\
	".\profiler.h"\
	".\projconst.h"\
	".\project.h"\
	".\pstring.h"\
	".\random.h"\
	".\rect.h"\
	".\refptr.h"\
	".\region.h"\
	".\region_graph.h"\
	".\render_data.h"\
	".\render_interface.cpp"\
	".\render_interface.h"\
	".\renderflav.h"\
	".\scene_anim.h"\
	".\script_data_interface.cpp"\
	".\script_data_interface.h"\
	".\script_lib_mfg.h"\
	".\script_library_class.h"\
	".\script_mfg_signals.h"\
	".\script_object.h"\
	".\signal_anim.h"\
	".\signals.h"\
	".\SimpleAssert.h"\
	".\singleton.h"\
	".\skeleton_interface.cpp"\
	".\skeleton_interface.h"\
	".\slave_interface.cpp"\
	".\slave_interface.h"\
	".\so_data_block.h"\
	".\soft_attrib_interface.cpp"\
	".\soft_attrib_interface.h"\
	".\sound_group.h"\
	".\sound_interface.cpp"\
	".\sound_interface.h"\
	".\sphere.h"\
	".\stashes.h"\
	".\staticmem.h"\
	".\stl_adapter.h"\
	".\stringx.h"\
	".\terrain.h"\
	".\text_font.h"\
	".\textfile.h"\
	".\time_interface.cpp"\
	".\time_interface.h"\
	".\timer.h"\
	".\txtcoord.h"\
	".\types.h"\
	".\usefulmath.h"\
	".\users.h"\
	".\vert.h"\
	".\vertwork.h"\
	".\visrep.h"\
	".\vm_executable.h"\
	".\vm_stack.h"\
	".\vm_symbol.h"\
	".\vm_symbol_list.h"\
	".\vm_thread.h"\
	".\warnlvl.h"\
	".\wds.h"\
	".\wedge.h"\
	".\widget.h"\
	".\widget_entity.h"\
	".\xbglobals.h"\
	
NODEP_CPP_FILES_E=\
	"..\..\NSL\pc\nl_pc.h"\
	"..\..\NSL\pc\nsl_pc_ext.h"\
	"..\..\NSL\XBOX\nsl.h"\
	".\algo.h"\
	".\defalloc.h"\
	".\ekernel.h"\
	".\eregs.h"\
	".\estruct.h"\
	".\etypes.h"\
	".\gc_arammgr.h"\
	".\graphics.h"\
	".\hwosgc\nsl.h"\
	".\hwosmks\am_audio.h"\
	".\hwosmks\mks_devopts.h"\
	".\hwosmks\mks_errmsg.h"\
	".\hwosmks\mks_movieplayer.h"\
	".\hwosmks\set5_rasterize.h"\
	".\hwosmks\set5_timer.h"\
	".\hwosmks\sh4_math.h"\
	".\hwosmks\sy_alloc.h"\
	".\hwosmks\sy_file.h"\
	".\hwosnglpc\algebra.h"\
	".\hwosnglpc\pc_algebra.h"\
	".\hwosnull\null_alloc.h"\
	".\hwosnull\null_audio.h"\
	".\hwosnull\null_devopts.h"\
	".\hwosnull\null_errmsg.h"\
	".\hwosnull\null_file.h"\
	".\hwosnull\null_math.h"\
	".\hwosnull\null_movieplayer.h"\
	".\hwosnull\null_rasterize.h"\
	".\hwosnull\null_timer.h"\
	".\hwospc\d3d_rasterize.h"\
	".\hwospc\ds_audio.h"\
	".\hwospc\pc_algebra.h"\
	".\hwospc\pc_movieplayer.h"\
	".\hwospc\w32_alloc.h"\
	".\hwospc\w32_devopts.h"\
	".\hwospc\w32_errmsg.h"\
	".\hwospc\w32_file.h"\
	".\hwospc\w32_timer.h"\
	".\hwospc\x86_math.h"\
	".\hwosxb\ngl_xbox.h"\
	".\hwosxb\nvl_xbox.h"\
	".\ibcdvd.h"\
	".\ibdma.h"\
	".\ibgraph.h"\
	".\ibipu.h"\
	".\ibmc.h"\
	".\ibmpeg.h"\
	".\ibpad.h"\
	".\ibpc.h"\
	".\ibusbkb.h"\
	".\ibvu0.h"\
	".\ifdev.h"\
	".\LipSync_Anim.h"\
	".\ngl_xbox.h"\
	".\nvl_gc.h"\
	".\nvl_xbox.h"\
	".\nvlstream_gc.h"\
	".\s\AccompFrontEnd.h"\
	".\s\andersen_ind_anims_mac.h"\
	".\s\beach.h"\
	".\s\beachdata.h"\
	".\s\BeachFrontEnd.h"\
	".\s\board.h"\
	".\s\board_anims_mac.h"\
	".\s\boarddata.h"\
	".\s\BoardFrontEnd.h"\
	".\s\career.h"\
	".\s\careerdata.h"\
	".\s\carrol_ind_anims_mac.h"\
	".\s\challenge.h"\
	".\s\challenge_icon.h"\
	".\s\challenge_photo.h"\
	".\s\cheat.h"\
	".\s\CheatFrontEnd.h"\
	".\s\compressedphoto.h"\
	".\s\coords.h"\
	".\s\curren_ind_anims_mac.h"\
	".\s\eventmanager.h"\
	".\s\ExtrasFrontEnd.h"\
	".\s\FEAnim.h"\
	".\s\FEEntityManager.h"\
	".\s\FEMenu.h"\
	".\s\FEPanel.h"\
	".\s\fletcher_ind_anims_mac.h"\
	".\s\floatobj.h"\
	".\s\frankenreiter_ind_anims_mac.h"\
	".\s\FrontEndManager.h"\
	".\s\FrontEndMenus.h"\
	".\s\GameData.h"\
	".\s\GlobalData.h"\
	".\s\globaltextenum.h"\
	".\s\GraphicalMenuSystem.h"\
	".\s\HighScoreFrontEnd.h"\
	".\s\igo_widget.h"\
	".\s\igo_widget_analogclock.h"\
	".\s\igo_widget_balance.h"\
	".\s\igo_widget_camera.h"\
	".\s\igo_widget_fanmeter.h"\
	".\s\igo_widget_iconradar.h"\
	".\s\igo_widget_photo.h"\
	".\s\igo_widget_replay.h"\
	".\s\igo_widget_simple.h"\
	".\s\igo_widget_specialmeter.h"\
	".\s\igo_widget_splitclock.h"\
	".\s\igo_widget_splitmeter.h"\
	".\s\igo_widget_splitscore.h"\
	".\s\igo_widget_splitter.h"\
	".\s\igo_widget_timeattack.h"\
	".\s\igo_widget_waveindicator.h"\
	".\s\IGOFrontEnd.h"\
	".\s\igohintmanager.h"\
	".\s\igoiconmanager.h"\
	".\s\igolearn_new_trickmanager.h"\
	".\s\ik_object.h"\
	".\s\irons_ind_anims_mac.h"\
	".\s\judge.h"\
	".\s\kellyslater_controller.h"\
	".\s\kellyslater_shared_anims_mac.h"\
	".\s\kellyslater_states_mac.h"\
	".\s\ks_camera.h"\
	".\s\ksdbmenu.h"\
	".\s\ksngl.h"\
	".\s\ksreplay.h"\
	".\s\machado_ind_anims_mac.h"\
	".\s\MainFrontEnd.h"\
	".\s\Map.h"\
	".\s\MCDetectFrontEnd.h"\
	".\s\menu.h"\
	".\s\menusys.h"\
	".\s\mode.h"\
	".\s\mode_headtohead.h"\
	".\s\mode_meterattack.h"\
	".\s\mode_push.h"\
	".\s\mode_timeattack.h"\
	".\s\MultiFrontEnd.h"\
	".\s\MusicMan.h"\
	".\s\ode.h"\
	".\s\osGameSaver.h"\
	".\s\PAL60FrontEnd.h"\
	".\s\PathData.h"\
	".\s\PhotoFrontEnd.h"\
	".\s\physics.h"\
	".\s\player.h"\
	".\s\replay.h"\
	".\s\robb_ind_anims_mac.h"\
	".\s\rumbleManager.h"\
	".\s\SaveLoadFrontEnd.h"\
	".\s\scoringmanager.h"\
	".\s\slater_ind_anims_mac.h"\
	".\s\sounddata.h"\
	".\s\SoundScript.h"\
	".\s\specialmeter.h"\
	".\s\StatsFrontEnd.h"\
	".\s\surferdata.h"\
	".\s\SurferFrontEnd.h"\
	".\s\trick_system.h"\
	".\s\TrickBookFrontEnd.h"\
	".\s\trickdata.h"\
	".\s\trickregion.h"\
	".\s\tricks.h"\
	".\s\TutorialFrontEnd.h"\
	".\s\tutorialmanager.h"\
	".\s\VOEngine.h"\
	".\s\wave.h"\
	".\s\wavebreak.txt"\
	".\s\wavebreakstage.txt"\
	".\s\waveemitter.txt"\
	".\s\waveenum.h"\
	".\s\wavemarker.txt"\
	".\s\waveregion.txt"\
	".\s\wavestage.txt"\
	".\stl_heap.h"\
	".\wosgc\gc_algebra.h"\
	".\wosgc\gc_alloc.h"\
	".\wosgc\gc_audio.h"\
	".\wosgc\gc_errmsg.h"\
	".\wosgc\gc_file.h"\
	".\WOSGC\gc_gamesaver.h"\
	".\wosgc\gc_graphics.h"\
	".\wosgc\gc_input.h"\
	".\wosgc\gc_math.h"\
	".\wosgc\gc_movieplayer.h"\
	".\wosgc\gc_rasterize.h"\
	".\wosgc\gc_texturemgr.h"\
	".\wosgc\gc_timer.h"\
	".\wosgc\ngl_fixedstr.h"\
	".\wosgc\ngl_gc.h"\
	".\wosgc\ngl_version.h"\
	".\wosgc\nvl_gc.h"\
	".\WOSPS2\gas.h"\
	".\WOSPS2\ps2_algebra.h"\
	".\WOSPS2\ps2_alloc.h"\
	".\WOSPS2\ps2_audio.h"\
	".\WOSPS2\ps2_devopts.h"\
	".\WOSPS2\ps2_errmsg.h"\
	".\WOSPS2\ps2_file.h"\
	".\WOSPS2\ps2_gamesaver.h"\
	".\WOSPS2\ps2_graphics.h"\
	".\WOSPS2\ps2_input.h"\
	".\WOSPS2\ps2_m2vplayer.h"\
	".\WOSPS2\ps2_math.h"\
	".\WOSPS2\ps2_movieplayer.h"\
	".\WOSPS2\ps2_rasterize.h"\
	".\WOSPS2\ps2_texturemgr.h"\
	".\WOSPS2\ps2_timer.h"\
	".\WOSPS2\ps2_types.h"\
	".\wosxb\pstring.h"\
	".\wosxb\xb_algebra.h"\
	".\wosxb\xb_alloc.h"\
	".\wosxb\xb_audio.h"\
	".\wosxb\xb_errmsg.h"\
	".\wosxb\xb_file.h"\
	".\WOSXB\xb_gamesaver.h"\
	".\wosxb\xb_graphics.h"\
	".\wosxb\xb_input.h"\
	".\wosxb\xb_math.h"\
	".\wosxb\xb_movieplayer.h"\
	".\WOSXB\xb_portDVDCache.h"\
	".\wosxb\xb_rasterize.h"\
	".\wosxb\xb_string.h"\
	".\wosxb\xb_texturemgr.h"\
	".\wosxb\xb_timer.h"\
	".\wosxb\xb_types.h"\
	
# SUBTRACT BASE CPP /YX /Yc /Yu
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

DEP_CPP_FILES_E=\
	"..\..\ngl\ps2\ngl_fixedstr.h"\
	"..\..\ngl\ps2\ngl_ps2.h"\
	"..\..\ngl\ps2\tim2.h"\
	"..\..\NSL\common\nl.h"\
	"..\..\NSL\common\nsl.h"\
	"..\..\NSL\common\ProjectOptions.h"\
	"..\..\NSL\gamecube\nl_gc.h"\
	"..\..\NSL\ps2\fifo_queue.cpp"\
	"..\..\NSL\ps2\fifo_queue.h"\
	"..\..\NSL\PS2\gas.h"\
	"..\..\NSL\PS2\gas_utility.h"\
	"..\..\NSL\PS2\nl_ps2.h"\
	"..\..\NSL\PS2\nsl_ps2.h"\
	"..\..\NSL\XBOX\nl_xbox.h"\
	"..\..\NSL\XBOX\nsl_xbox.h"\
	"..\..\NSL\XBOX\nsl_xbox_ext.h"\
	"..\..\NSL\XBOX\nsl_xbox_source.h"\
	"..\..\nsl\xbox\nsl_xbox_streams.h"\
	"..\..\NVL\ps2\nvl_ps2.h"\
	"..\..\NVL\ps2\nvlMPEG_ps2.h"\
	"..\..\NVL\ps2\nvlstream_ps2.h"\
	".\aggvertbuf.h"\
	".\ai_constants.h"\
	".\ai_interface.h"\
	".\ai_locomotion.h"\
	".\ai_polypath.h"\
	".\ai_polypath_cell.h"\
	".\ai_polypath_heap.h"\
	".\algebra.h"\
	".\anim.h"\
	".\anim_flavor.h"\
	".\anim_flavors.h"\
	".\anim_ids.h"\
	".\anim_maker.h"\
	".\animation_interface.cpp"\
	".\animation_interface.h"\
	".\app.h"\
	".\archalloc.h"\
	".\attribute_template.h"\
	".\avltree.h"\
	".\b_spline.h"\
	".\beam.h"\
	".\beam_signals.h"\
	".\billboard.h"\
	".\biped_ids.h"\
	".\bone.cpp"\
	".\bone.h"\
	".\bound.h"\
	".\box_trigger_interface.cpp"\
	".\box_trigger_interface.h"\
	".\bsp_collide.h"\
	".\camera.h"\
	".\capsule.h"\
	".\cface.h"\
	".\character_hard_attribs.h"\
	".\character_soft_attribs.h"\
	".\chunkfile.h"\
	".\colgeom.h"\
	".\collide.h"\
	".\colmesh.h"\
	".\color.h"\
	".\commands.h"\
	".\conglom.h"\
	".\console.h"\
	".\constants.h"\
	".\controller.h"\
	".\convex_box.h"\
	".\custom_stl.h"\
	".\debug.h"\
	".\debug_render.h"\
	".\devoptflags.h"\
	".\devoptints.h"\
	".\devoptstrs.h"\
	".\dropshadow.h"\
	".\element.h"\
	".\entflavor.h"\
	".\entity.cpp"\
	".\entity.h"\
	".\entity_anim.h"\
	".\entity_hard_attribs.h"\
	".\entity_interface.h"\
	".\entity_maker.h"\
	".\entity_signals.h"\
	".\entity_soft_attribs.h"\
	".\entityflags.h"\
	".\entityid.h"\
	".\errorcontext.h"\
	".\face.h"\
	".\faceflags.h"\
	".\fast_vector.h"\
	".\file.h"\
	".\file_finder.h"\
	".\filespec.h"\
	".\fogmgr.h"\
	".\forceflags.h"\
	".\frame_info.h"\
	".\game.h"\
	".\game_info_vars.h"\
	".\game_process.h"\
	".\gcglobals.h"\
	".\generator.h"\
	".\geomgr.h"\
	".\global.h"\
	".\global_signals.h"\
	".\graph.h"\
	".\guidance_sys.h"\
	".\hard_attrib_interface.cpp"\
	".\hard_attrib_interface.h"\
	".\hull.h"\
	".\hwaudio.h"\
	".\hwmath.h"\
	".\hwmovieplayer.h"\
	".\hwrasterize.h"\
	".\hyperplane.h"\
	".\ini_parser.h"\
	".\inputmgr.h"\
	".\instance.h"\
	".\iri.h"\
	".\item.h"\
	".\item_signals.h"\
	".\joint.h"\
	".\joystick.h"\
	".\keyboard.h"\
	".\ks\ksheaps.h"\
	".\kshooks.h"\
	".\ksnsl.h"\
	".\ksnvl.h"\
	".\light.h"\
	".\lightmgr.h"\
	".\linear_anim.h"\
	".\link_interface.cpp"\
	".\link_interface.h"\
	".\map_e.h"\
	".\material.h"\
	".\matfac.h"\
	".\maxskinbones.h"\
	".\mbi.h"\
	".\meshrefs.h"\
	".\mic.h"\
	".\mobject.h"\
	".\msgboard.h"\
	".\mustash.h"\
	".\ngl.h"\
	".\ngl_support.h"\
	".\osalloc.h"\
	".\osassert.h"\
	".\osdevopts.h"\
	".\oserrmsg.h"\
	".\osfile.h"\
	".\ostimer.h"\
	".\owner_interface.cpp"\
	".\owner_interface.h"\
	".\path.h"\
	".\pc_port.h"\
	".\physical_interface.cpp"\
	".\physical_interface.h"\
	".\plane.h"\
	".\platform_defines.h"\
	".\pmesh.h"\
	".\po.h"\
	".\po_anim.h"\
	".\polytube.cpp"\
	".\polytube.h"\
	".\portal.h"\
	".\profiler.h"\
	".\projconst.h"\
	".\project.h"\
	".\pstring.h"\
	".\random.h"\
	".\rect.h"\
	".\refptr.h"\
	".\region.h"\
	".\region_graph.h"\
	".\render_data.h"\
	".\render_interface.cpp"\
	".\render_interface.h"\
	".\renderflav.h"\
	".\scene_anim.h"\
	".\script_data_interface.cpp"\
	".\script_data_interface.h"\
	".\script_lib_mfg.h"\
	".\script_library_class.h"\
	".\script_mfg_signals.h"\
	".\script_object.h"\
	".\signal_anim.h"\
	".\signals.h"\
	".\SimpleAssert.h"\
	".\singleton.h"\
	".\skeleton_interface.cpp"\
	".\skeleton_interface.h"\
	".\slave_interface.cpp"\
	".\slave_interface.h"\
	".\so_data_block.h"\
	".\soft_attrib_interface.cpp"\
	".\soft_attrib_interface.h"\
	".\sound_group.h"\
	".\sound_interface.cpp"\
	".\sound_interface.h"\
	".\sphere.h"\
	".\stashes.h"\
	".\staticmem.h"\
	".\stl_adapter.h"\
	".\stringx.h"\
	".\terrain.h"\
	".\text_font.h"\
	".\textfile.h"\
	".\time_interface.cpp"\
	".\time_interface.h"\
	".\timer.h"\
	".\txtcoord.h"\
	".\types.h"\
	".\usefulmath.h"\
	".\users.h"\
	".\vert.h"\
	".\vertwork.h"\
	".\visrep.h"\
	".\vm_executable.h"\
	".\vm_stack.h"\
	".\vm_symbol.h"\
	".\vm_symbol_list.h"\
	".\vm_thread.h"\
	".\warnlvl.h"\
	".\wds.h"\
	".\wedge.h"\
	".\widget.h"\
	".\widget_entity.h"\
	".\xbglobals.h"\
	
NODEP_CPP_FILES_E=\
	"..\..\NSL\pc\nl_pc.h"\
	"..\..\NSL\pc\nsl_pc_ext.h"\
	".\algo.h"\
	".\defalloc.h"\
	".\ekernel.h"\
	".\eregs.h"\
	".\estruct.h"\
	".\etypes.h"\
	".\gc\nsl_gc.h"\
	".\gc_arammgr.h"\
	".\graphics.h"\
	".\hwosmks\am_audio.h"\
	".\hwosmks\mks_devopts.h"\
	".\hwosmks\mks_errmsg.h"\
	".\hwosmks\mks_movieplayer.h"\
	".\hwosmks\set5_rasterize.h"\
	".\hwosmks\set5_timer.h"\
	".\hwosmks\sh4_math.h"\
	".\hwosmks\sy_alloc.h"\
	".\hwosmks\sy_file.h"\
	".\hwosnglpc\algebra.h"\
	".\hwosnglpc\pc_algebra.h"\
	".\hwosnull\null_alloc.h"\
	".\hwosnull\null_audio.h"\
	".\hwosnull\null_devopts.h"\
	".\hwosnull\null_errmsg.h"\
	".\hwosnull\null_file.h"\
	".\hwosnull\null_math.h"\
	".\hwosnull\null_movieplayer.h"\
	".\hwosnull\null_rasterize.h"\
	".\hwosnull\null_timer.h"\
	".\hwospc\d3d_rasterize.h"\
	".\hwospc\ds_audio.h"\
	".\hwospc\pc_algebra.h"\
	".\hwospc\pc_movieplayer.h"\
	".\hwospc\w32_alloc.h"\
	".\hwospc\w32_devopts.h"\
	".\hwospc\w32_errmsg.h"\
	".\hwospc\w32_file.h"\
	".\hwospc\w32_timer.h"\
	".\hwospc\x86_math.h"\
	".\hwosxb\ngl_xbox.h"\
	".\hwosxb\nvl_xbox.h"\
	".\ibcdvd.h"\
	".\ibdma.h"\
	".\ibgraph.h"\
	".\ibipu.h"\
	".\ibmc.h"\
	".\ibmpeg.h"\
	".\ibpad.h"\
	".\ibpc.h"\
	".\ibusbkb.h"\
	".\ibvu0.h"\
	".\ifdev.h"\
	".\LipSync_Anim.h"\
	".\ngl_xbox.h"\
	".\nsl.h"\
	".\nvl_gc.h"\
	".\nvl_xbox.h"\
	".\nvlstream_gc.h"\
	".\s\AccompFrontEnd.h"\
	".\s\andersen_ind_anims_mac.h"\
	".\s\beach.h"\
	".\s\beachdata.h"\
	".\s\BeachFrontEnd.h"\
	".\s\board.h"\
	".\s\board_anims_mac.h"\
	".\s\boarddata.h"\
	".\s\BoardFrontEnd.h"\
	".\s\career.h"\
	".\s\careerdata.h"\
	".\s\carrol_ind_anims_mac.h"\
	".\s\challenge.h"\
	".\s\challenge_icon.h"\
	".\s\challenge_photo.h"\
	".\s\cheat.h"\
	".\s\CheatFrontEnd.h"\
	".\s\compressedphoto.h"\
	".\s\coords.h"\
	".\s\curren_ind_anims_mac.h"\
	".\s\eventmanager.h"\
	".\s\ExtrasFrontEnd.h"\
	".\s\FEAnim.h"\
	".\s\FEEntityManager.h"\
	".\s\FEMenu.h"\
	".\s\FEPanel.h"\
	".\s\fletcher_ind_anims_mac.h"\
	".\s\floatobj.h"\
	".\s\frankenreiter_ind_anims_mac.h"\
	".\s\FrontEndManager.h"\
	".\s\FrontEndMenus.h"\
	".\s\GameData.h"\
	".\s\GlobalData.h"\
	".\s\globaltextenum.h"\
	".\s\GraphicalMenuSystem.h"\
	".\s\HighScoreFrontEnd.h"\
	".\s\igo_widget.h"\
	".\s\igo_widget_analogclock.h"\
	".\s\igo_widget_balance.h"\
	".\s\igo_widget_camera.h"\
	".\s\igo_widget_fanmeter.h"\
	".\s\igo_widget_iconcount.h"\
	".\s\igo_widget_iconradar.h"\
	".\s\igo_widget_meterchallenge.h"\
	".\s\igo_widget_objectalert.h"\
	".\s\igo_widget_photo.h"\
	".\s\igo_widget_replay.h"\
	".\s\igo_widget_simple.h"\
	".\s\igo_widget_skillchallenge.h"\
	".\s\igo_widget_specialmeter.h"\
	".\s\igo_widget_splitclock.h"\
	".\s\igo_widget_splitmeter.h"\
	".\s\igo_widget_splitscore.h"\
	".\s\igo_widget_splitter.h"\
	".\s\igo_widget_timeattack.h"\
	".\s\igo_widget_waveindicator.h"\
	".\s\IGOFrontEnd.h"\
	".\s\igohintmanager.h"\
	".\s\igoiconmanager.h"\
	".\s\igolearn_new_trickmanager.h"\
	".\s\ik_object.h"\
	".\s\irons_ind_anims_mac.h"\
	".\s\judge.h"\
	".\s\kellyslater_controller.h"\
	".\s\kellyslater_shared_anims_mac.h"\
	".\s\kellyslater_states_mac.h"\
	".\s\ks_camera.h"\
	".\s\ksdbmenu.h"\
	".\s\ksngl.h"\
	".\s\ksreplay.h"\
	".\s\LogbookFrontEnd.h"\
	".\s\machado_ind_anims_mac.h"\
	".\s\MainFrontEnd.h"\
	".\s\Map.h"\
	".\s\MCDetectFrontEnd.h"\
	".\s\menu.h"\
	".\s\menusys.h"\
	".\s\mode.h"\
	".\s\mode_headtohead.h"\
	".\s\mode_meterattack.h"\
	".\s\mode_push.h"\
	".\s\mode_timeattack.h"\
	".\s\MultiFrontEnd.h"\
	".\s\MusicMan.h"\
	".\s\ode.h"\
	".\s\osGameSaver.h"\
	".\s\PAL60FrontEnd.h"\
	".\s\PathData.h"\
	".\s\PhotoFrontEnd.h"\
	".\s\physics.h"\
	".\s\player.h"\
	".\s\replay.h"\
	".\s\robb_ind_anims_mac.h"\
	".\s\rumbleManager.h"\
	".\s\SaveLoadFrontEnd.h"\
	".\s\scoringmanager.h"\
	".\s\slater_ind_anims_mac.h"\
	".\s\sounddata.h"\
	".\s\SoundScript.h"\
	".\s\specialmeter.h"\
	".\s\StatsFrontEnd.h"\
	".\s\surferdata.h"\
	".\s\SurferFrontEnd.h"\
	".\s\trick_system.h"\
	".\s\TrickBookFrontEnd.h"\
	".\s\trickdata.h"\
	".\s\trickregion.h"\
	".\s\tricks.h"\
	".\s\tutorialdata.h"\
	".\s\TutorialFrontEnd.h"\
	".\s\tutorialmanager.h"\
	".\s\VOEngine.h"\
	".\s\wave.h"\
	".\s\wavebreak.txt"\
	".\s\wavebreakstage.txt"\
	".\s\waveemitter.txt"\
	".\s\waveenum.h"\
	".\s\wavemarker.txt"\
	".\s\waveregion.txt"\
	".\s\wavestage.txt"\
	".\stl_heap.h"\
	".\wosgc\gc_algebra.h"\
	".\wosgc\gc_alloc.h"\
	".\wosgc\gc_audio.h"\
	".\wosgc\gc_errmsg.h"\
	".\wosgc\gc_file.h"\
	".\WOSGC\gc_gamesaver.h"\
	".\wosgc\gc_graphics.h"\
	".\wosgc\gc_input.h"\
	".\wosgc\gc_math.h"\
	".\wosgc\gc_movieplayer.h"\
	".\wosgc\gc_rasterize.h"\
	".\wosgc\gc_texturemgr.h"\
	".\wosgc\gc_timer.h"\
	".\wosgc\ngl_fixedstr.h"\
	".\wosgc\ngl_gc.h"\
	".\wosgc\ngl_version.h"\
	".\wosgc\nvl_gc.h"\
	".\WOSPS2\gas.h"\
	".\WOSPS2\ps2_algebra.h"\
	".\WOSPS2\ps2_alloc.h"\
	".\WOSPS2\ps2_audio.h"\
	".\WOSPS2\ps2_devopts.h"\
	".\WOSPS2\ps2_errmsg.h"\
	".\WOSPS2\ps2_file.h"\
	".\WOSPS2\ps2_gamesaver.h"\
	".\WOSPS2\ps2_graphics.h"\
	".\WOSPS2\ps2_input.h"\
	".\WOSPS2\ps2_m2vplayer.h"\
	".\WOSPS2\ps2_math.h"\
	".\WOSPS2\ps2_movieplayer.h"\
	".\WOSPS2\ps2_rasterize.h"\
	".\WOSPS2\ps2_texturemgr.h"\
	".\WOSPS2\ps2_timer.h"\
	".\WOSPS2\ps2_types.h"\
	".\wosxb\pstring.h"\
	".\wosxb\xb_algebra.h"\
	".\wosxb\xb_alloc.h"\
	".\wosxb\xb_audio.h"\
	".\wosxb\xb_errmsg.h"\
	".\wosxb\xb_file.h"\
	".\WOSXB\xb_gamesaver.h"\
	".\wosxb\xb_graphics.h"\
	".\wosxb\xb_input.h"\
	".\wosxb\xb_math.h"\
	".\wosxb\xb_movieplayer.h"\
	".\WOSXB\xb_portDVDCache.h"\
	".\wosxb\xb_rasterize.h"\
	".\wosxb\xb_string.h"\
	".\wosxb\xb_texturemgr.h"\
	".\wosxb\xb_timer.h"\
	".\wosxb\xb_types.h"\
	
# SUBTRACT BASE CPP /YX /Yc /Yu
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\files_frontend.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

DEP_CPP_FILES_F=\
	"..\..\ngl\ps2\ngl_fixedstr.h"\
	"..\..\ngl\ps2\ngl_instbank.h"\
	"..\..\ngl\ps2\ngl_ps2.h"\
	"..\..\ngl\ps2\tim2.h"\
	"..\..\NSL\common\nl.h"\
	"..\..\NSL\common\nsl.h"\
	"..\..\NSL\common\ProjectOptions.h"\
	"..\..\NSL\gamecube\nl_gc.h"\
	"..\..\NSL\ps2\fifo_queue.cpp"\
	"..\..\NSL\ps2\fifo_queue.h"\
	"..\..\NSL\PS2\gas.h"\
	"..\..\NSL\PS2\gas_utility.h"\
	"..\..\NSL\PS2\nl_ps2.h"\
	"..\..\NSL\PS2\nsl_ps2.h"\
	"..\..\NSL\XBOX\nl_xbox.h"\
	"..\..\NSL\XBOX\nsl_xbox.h"\
	"..\..\NSL\XBOX\nsl_xbox_ext.h"\
	"..\..\NSL\XBOX\nsl_xbox_source.h"\
	"..\..\nsl\xbox\nsl_xbox_streams.h"\
	"..\..\NVL\ps2\nvl_ps2.h"\
	"..\..\NVL\ps2\nvlMPEG_ps2.h"\
	"..\..\NVL\ps2\nvlstream_ps2.h"\
	".\aggvertbuf.h"\
	".\algebra.h"\
	".\anim.h"\
	".\anim_flavor.h"\
	".\anim_flavors.h"\
	".\anim_ids.h"\
	".\app.h"\
	".\archalloc.h"\
	".\avltree.h"\
	".\beam.h"\
	".\beam_signals.h"\
	".\billboard.h"\
	".\bone.h"\
	".\bound.h"\
	".\box_trigger_interface.h"\
	".\camera.h"\
	".\capsule.h"\
	".\cface.h"\
	".\chunkfile.h"\
	".\colgeom.h"\
	".\colmesh.h"\
	".\color.h"\
	".\commands.h"\
	".\conglom.h"\
	".\constants.h"\
	".\controller.h"\
	".\convex_box.h"\
	".\custom_stl.h"\
	".\debug.h"\
	".\devoptflags.h"\
	".\devoptints.h"\
	".\devoptstrs.h"\
	".\entflavor.h"\
	".\entity.h"\
	".\entity_anim.h"\
	".\entity_interface.h"\
	".\entity_maker.h"\
	".\entity_signals.h"\
	".\entityflags.h"\
	".\entityid.h"\
	".\errorcontext.h"\
	".\face.h"\
	".\faceflags.h"\
	".\fast_vector.h"\
	".\file.h"\
	".\file_finder.h"\
	".\filespec.h"\
	".\frame_info.h"\
	".\game.h"\
	".\game_info_vars.h"\
	".\game_process.h"\
	".\gcglobals.h"\
	".\generator.h"\
	".\geomgr.h"\
	".\global.h"\
	".\global_signals.h"\
	".\graph.h"\
	".\heap.h"\
	".\heaptype.h"\
	".\hull.h"\
	".\hwmath.h"\
	".\hwmovieplayer.h"\
	".\hwrasterize.h"\
	".\hyperplane.h"\
	".\ini_parser.h"\
	".\inputmgr.h"\
	".\instance.h"\
	".\item.h"\
	".\item_signals.h"\
	".\joystick.h"\
	".\ks\AccompFrontEnd.cpp"\
	".\ks\BeachFrontEnd.cpp"\
	".\ks\boarddata.cpp"\
	".\ks\BoardFrontEnd.cpp"\
	".\ks\BoardManager.h"\
	".\ks\CheatFrontEnd.cpp"\
	".\ks\DemoMode.h"\
	".\ks\ExtrasFrontEnd.cpp"\
	".\ks\FEAnim.cpp"\
	".\ks\FEEntityManager.cpp"\
	".\ks\FEMenu.cpp"\
	".\ks\FEPanel.cpp"\
	".\ks\FrontEndManager.cpp"\
	".\ks\FrontEndMenus.cpp"\
	".\ks\GameData.cpp"\
	".\ks\globaltextenum.cpp"\
	".\ks\GraphicalMenuSystem.cpp"\
	".\ks\HighScoreFrontEnd.cpp"\
	".\ks\igo_widget.cpp"\
	".\ks\igo_widget_analogclock.cpp"\
	".\ks\igo_widget_balance.cpp"\
	".\ks\igo_widget_camera.cpp"\
	".\ks\igo_widget_fanmeter.cpp"\
	".\ks\igo_widget_grid.cpp"\
	".\ks\igo_widget_iconcount.cpp"\
	".\ks\igo_widget_iconradar.cpp"\
	".\ks\igo_widget_meterchallenge.cpp"\
	".\ks\igo_widget_objectalert.cpp"\
	".\ks\igo_widget_photo.cpp"\
	".\ks\igo_widget_replay.cpp"\
	".\ks\igo_widget_simple.cpp"\
	".\ks\igo_widget_skillchallenge.cpp"\
	".\ks\igo_widget_specialmeter.cpp"\
	".\ks\igo_widget_splitclock.cpp"\
	".\ks\igo_widget_splitmeter.cpp"\
	".\ks\igo_widget_splitscore.cpp"\
	".\ks\igo_widget_splitter.cpp"\
	".\ks\igo_widget_timeattack.cpp"\
	".\ks\igo_widget_waveindicator.cpp"\
	".\ks\IGOFrontEnd.cpp"\
	".\ks\igohintmanager.cpp"\
	".\ks\igoiconmanager.cpp"\
	".\ks\igolearn_new_trickmanager.cpp"\
	".\ks\ksfx.h"\
	".\ks\ksheaps.h"\
	".\ks\LogbookFrontEnd.cpp"\
	".\ks\MainFrontEnd.cpp"\
	".\ks\Map.cpp"\
	".\ks\MCDetectFrontEnd.cpp"\
	".\ks\MultiFrontEnd.cpp"\
	".\ks\PAL60FrontEnd.cpp"\
	".\ks\PhotoFrontEnd.cpp"\
	".\ks\PlaylistMenu.cpp"\
	".\ks\PlaylistMenu.h"\
	".\ks\SaveLoadFrontEnd.cpp"\
	".\ks\StatsFrontEnd.cpp"\
	".\ks\SurferFrontEnd.cpp"\
	".\ks\text_parser.h"\
	".\ks\TrickBookFrontEnd.cpp"\
	".\ks\TutorialFrontEnd.cpp"\
	".\ks\tutorialmanager.cpp"\
	".\ks\wavesound.h"\
	".\kshooks.h"\
	".\ksnsl.h"\
	".\ksnvl.h"\
	".\light.h"\
	".\lightmgr.h"\
	".\linear_anim.h"\
	".\link_interface.h"\
	".\map_e.h"\
	".\material.h"\
	".\matfac.h"\
	".\maxskinbones.h"\
	".\meshrefs.h"\
	".\mic.h"\
	".\mobject.h"\
	".\mustash.h"\
	".\ngl.h"\
	".\osalloc.h"\
	".\osassert.h"\
	".\osdevopts.h"\
	".\oserrmsg.h"\
	".\osfile.h"\
	".\ostimer.h"\
	".\path.h"\
	".\pc_port.h"\
	".\plane.h"\
	".\platform_defines.h"\
	".\pmesh.h"\
	".\po.h"\
	".\po_anim.h"\
	".\portal.h"\
	".\profiler.h"\
	".\projconst.h"\
	".\project.h"\
	".\pstring.h"\
	".\random.h"\
	".\rect.h"\
	".\refptr.h"\
	".\region.h"\
	".\region_graph.h"\
	".\render_data.h"\
	".\renderflav.h"\
	".\scene_anim.h"\
	".\script_lib_mfg.h"\
	".\script_library_class.h"\
	".\script_mfg_signals.h"\
	".\script_object.h"\
	".\semaphores.h"\
	".\signal_anim.h"\
	".\signals.h"\
	".\SimpleAssert.h"\
	".\singleton.h"\
	".\so_data_block.h"\
	".\sphere.h"\
	".\stashes.h"\
	".\staticmem.h"\
	".\stl_adapter.h"\
	".\stringx.h"\
	".\terrain.h"\
	".\text_font.h"\
	".\textfile.h"\
	".\timer.h"\
	".\txtcoord.h"\
	".\types.h"\
	".\usefulmath.h"\
	".\users.h"\
	".\vert.h"\
	".\visrep.h"\
	".\vm_executable.h"\
	".\vm_symbol.h"\
	".\vm_symbol_list.h"\
	".\warnlvl.h"\
	".\wds.h"\
	".\wedge.h"\
	".\xbglobals.h"\
	{$(INCLUDE)}"eekernel.h"\
	{$(INCLUDE)}"eeregs.h"\
	{$(INCLUDE)}"eestruct.h"\
	{$(INCLUDE)}"eetypes.h"\
	{$(INCLUDE)}"hwosgc\gc_algebra.h"\
	{$(INCLUDE)}"hwosgc\gc_alloc.h"\
	{$(INCLUDE)}"hwosgc\gc_errmsg.h"\
	{$(INCLUDE)}"hwosgc\gc_file.h"\
	{$(INCLUDE)}"HWOSGC\gc_gamesaver.h"\
	{$(INCLUDE)}"hwosgc\gc_graphics.h"\
	{$(INCLUDE)}"hwosgc\gc_input.h"\
	{$(INCLUDE)}"hwosgc\gc_math.h"\
	{$(INCLUDE)}"hwosgc\gc_movieplayer.h"\
	{$(INCLUDE)}"hwosgc\gc_rasterize.h"\
	{$(INCLUDE)}"hwosgc\gc_texturemgr.h"\
	{$(INCLUDE)}"hwosgc\gc_timer.h"\
	{$(INCLUDE)}"hwosgc\ngl_fixedstr.h"\
	{$(INCLUDE)}"hwosgc\ngl_gc.h"\
	{$(INCLUDE)}"hwosgc\ngl_version.h"\
	{$(INCLUDE)}"hwosgc\nvl_gc.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_algebra.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_alloc.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_devopts.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_errmsg.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_file.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_gamesaver.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_graphics.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_input.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_m2vplayer.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_math.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_movieplayer.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_rasterize.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_texturemgr.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_timer.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_types.h"\
	{$(INCLUDE)}"hwosxb\xb_algebra.h"\
	{$(INCLUDE)}"hwosxb\xb_alloc.h"\
	{$(INCLUDE)}"hwosxb\xb_errmsg.h"\
	{$(INCLUDE)}"hwosxb\xb_file.h"\
	{$(INCLUDE)}"HWOSXB\xb_gamesaver.h"\
	{$(INCLUDE)}"hwosxb\xb_graphics.h"\
	{$(INCLUDE)}"hwosxb\xb_input.h"\
	{$(INCLUDE)}"hwosxb\xb_math.h"\
	{$(INCLUDE)}"hwosxb\xb_movieplayer.h"\
	{$(INCLUDE)}"HWOSXB\xb_particle.h"\
	{$(INCLUDE)}"HWOSXB\xb_portDVDCache.h"\
	{$(INCLUDE)}"hwosxb\xb_rasterize.h"\
	{$(INCLUDE)}"HWOSXB\xb_SaveLoadFrontEnd.cpp"\
	{$(INCLUDE)}"hwosxb\xb_string.h"\
	{$(INCLUDE)}"hwosxb\xb_texturemgr.h"\
	{$(INCLUDE)}"hwosxb\xb_timer.h"\
	{$(INCLUDE)}"hwosxb\xb_types.h"\
	{$(INCLUDE)}"ks\AccompFrontEnd.h"\
	{$(INCLUDE)}"ks\andersen_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\beach.h"\
	{$(INCLUDE)}"ks\beachdata.h"\
	{$(INCLUDE)}"ks\BeachFrontEnd.h"\
	{$(INCLUDE)}"ks\board.h"\
	{$(INCLUDE)}"ks\board_anims_mac.h"\
	{$(INCLUDE)}"ks\boarddata.h"\
	{$(INCLUDE)}"ks\BoardFrontEnd.h"\
	{$(INCLUDE)}"ks\career.h"\
	{$(INCLUDE)}"ks\careerdata.h"\
	{$(INCLUDE)}"ks\carrol_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\challenge.h"\
	{$(INCLUDE)}"ks\challenge_icon.h"\
	{$(INCLUDE)}"ks\challenge_photo.h"\
	{$(INCLUDE)}"ks\cheat.h"\
	{$(INCLUDE)}"ks\CheatFrontEnd.h"\
	{$(INCLUDE)}"ks\compressedphoto.h"\
	{$(INCLUDE)}"ks\coords.h"\
	{$(INCLUDE)}"ks\curren_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\eventmanager.h"\
	{$(INCLUDE)}"ks\ExtrasFrontEnd.h"\
	{$(INCLUDE)}"ks\FEAnim.h"\
	{$(INCLUDE)}"ks\FEEntityManager.h"\
	{$(INCLUDE)}"ks\FEMenu.h"\
	{$(INCLUDE)}"ks\FEPanel.h"\
	{$(INCLUDE)}"ks\fletcher_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\floatobj.h"\
	{$(INCLUDE)}"ks\frankenreiter_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\FrontEndManager.h"\
	{$(INCLUDE)}"ks\FrontEndMenus.h"\
	{$(INCLUDE)}"ks\GameData.h"\
	{$(INCLUDE)}"ks\GlobalData.h"\
	{$(INCLUDE)}"ks\globaltextenum.h"\
	{$(INCLUDE)}"ks\GraphicalMenuSystem.h"\
	{$(INCLUDE)}"ks\HighScoreFrontEnd.h"\
	{$(INCLUDE)}"ks\igo_widget.h"\
	{$(INCLUDE)}"ks\igo_widget_analogclock.h"\
	{$(INCLUDE)}"ks\igo_widget_balance.h"\
	{$(INCLUDE)}"ks\igo_widget_camera.h"\
	{$(INCLUDE)}"ks\igo_widget_fanmeter.h"\
	{$(INCLUDE)}"ks\igo_widget_grid.h"\
	{$(INCLUDE)}"ks\igo_widget_iconcount.h"\
	{$(INCLUDE)}"ks\igo_widget_iconradar.h"\
	{$(INCLUDE)}"ks\igo_widget_meterchallenge.h"\
	{$(INCLUDE)}"ks\igo_widget_objectalert.h"\
	{$(INCLUDE)}"ks\igo_widget_photo.h"\
	{$(INCLUDE)}"ks\igo_widget_replay.h"\
	{$(INCLUDE)}"ks\igo_widget_simple.h"\
	{$(INCLUDE)}"ks\igo_widget_skillchallenge.h"\
	{$(INCLUDE)}"ks\igo_widget_specialmeter.h"\
	{$(INCLUDE)}"ks\igo_widget_splitclock.h"\
	{$(INCLUDE)}"ks\igo_widget_splitmeter.h"\
	{$(INCLUDE)}"ks\igo_widget_splitscore.h"\
	{$(INCLUDE)}"ks\igo_widget_splitter.h"\
	{$(INCLUDE)}"ks\igo_widget_timeattack.h"\
	{$(INCLUDE)}"ks\igo_widget_waveindicator.h"\
	{$(INCLUDE)}"ks\IGOFrontEnd.h"\
	{$(INCLUDE)}"ks\igohintmanager.h"\
	{$(INCLUDE)}"ks\igoiconmanager.h"\
	{$(INCLUDE)}"ks\igolearn_new_trickmanager.h"\
	{$(INCLUDE)}"ks\ik_object.h"\
	{$(INCLUDE)}"ks\irons_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\judge.h"\
	{$(INCLUDE)}"ks\kellyslater_controller.h"\
	{$(INCLUDE)}"ks\kellyslater_shared_anims_mac.h"\
	{$(INCLUDE)}"ks\kellyslater_states_mac.h"\
	{$(INCLUDE)}"ks\ks_camera.h"\
	{$(INCLUDE)}"ks\ksdbmenu.h"\
	{$(INCLUDE)}"ks\ksngl.h"\
	{$(INCLUDE)}"ks\ksreplay.h"\
	{$(INCLUDE)}"ks\LogbookFrontEnd.h"\
	{$(INCLUDE)}"ks\machado_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\MainFrontEnd.h"\
	{$(INCLUDE)}"ks\Map.h"\
	{$(INCLUDE)}"ks\MCDetectFrontEnd.h"\
	{$(INCLUDE)}"ks\menu.h"\
	{$(INCLUDE)}"ks\menusys.h"\
	{$(INCLUDE)}"ks\mode.h"\
	{$(INCLUDE)}"ks\mode_headtohead.h"\
	{$(INCLUDE)}"ks\mode_meterattack.h"\
	{$(INCLUDE)}"ks\mode_push.h"\
	{$(INCLUDE)}"ks\mode_timeattack.h"\
	{$(INCLUDE)}"ks\MultiFrontEnd.h"\
	{$(INCLUDE)}"ks\MusicMan.h"\
	{$(INCLUDE)}"ks\ode.h"\
	{$(INCLUDE)}"ks\osGameSaver.h"\
	{$(INCLUDE)}"ks\osparticle.h"\
	{$(INCLUDE)}"ks\PAL60FrontEnd.h"\
	{$(INCLUDE)}"ks\PathData.h"\
	{$(INCLUDE)}"ks\PhotoFrontEnd.h"\
	{$(INCLUDE)}"ks\physics.h"\
	{$(INCLUDE)}"ks\player.h"\
	{$(INCLUDE)}"ks\replay.h"\
	{$(INCLUDE)}"ks\robb_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\rumbleManager.h"\
	{$(INCLUDE)}"ks\SaveLoadFrontEnd.h"\
	{$(INCLUDE)}"ks\scoringmanager.h"\
	{$(INCLUDE)}"ks\slater_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\sounddata.h"\
	{$(INCLUDE)}"ks\SoundScript.h"\
	{$(INCLUDE)}"ks\specialmeter.h"\
	{$(INCLUDE)}"ks\StatsFrontEnd.h"\
	{$(INCLUDE)}"ks\surferdata.h"\
	{$(INCLUDE)}"ks\SurferFrontEnd.h"\
	{$(INCLUDE)}"ks\trick_system.h"\
	{$(INCLUDE)}"ks\TrickBookFrontEnd.h"\
	{$(INCLUDE)}"ks\trickdata.h"\
	{$(INCLUDE)}"ks\trickregion.h"\
	{$(INCLUDE)}"ks\tricks.h"\
	{$(INCLUDE)}"ks\tutorialdata.h"\
	{$(INCLUDE)}"ks\TutorialFrontEnd.h"\
	{$(INCLUDE)}"ks\tutorialmanager.h"\
	{$(INCLUDE)}"ks\underwtr.h"\
	{$(INCLUDE)}"ks\unlock_manager.h"\
	{$(INCLUDE)}"ks\VOEngine.h"\
	{$(INCLUDE)}"ks\wave.h"\
	{$(INCLUDE)}"ks\wavebreak.txt"\
	{$(INCLUDE)}"ks\wavebreakstage.txt"\
	{$(INCLUDE)}"ks\waveemitter.txt"\
	{$(INCLUDE)}"ks\waveenum.h"\
	{$(INCLUDE)}"ks\wavemarker.txt"\
	{$(INCLUDE)}"ks\waveregion.txt"\
	{$(INCLUDE)}"ks\wavestage.txt"\
	{$(INCLUDE)}"libcdvd.h"\
	{$(INCLUDE)}"libdma.h"\
	{$(INCLUDE)}"libgraph.h"\
	{$(INCLUDE)}"libipu.h"\
	{$(INCLUDE)}"libmc.h"\
	{$(INCLUDE)}"libmpeg.h"\
	{$(INCLUDE)}"libpad.h"\
	{$(INCLUDE)}"libpc.h"\
	{$(INCLUDE)}"libscf.h"\
	{$(INCLUDE)}"libusbkb.h"\
	{$(INCLUDE)}"libvu0.h"\
	{$(INCLUDE)}"sifcmd.h"\
	{$(INCLUDE)}"sifdev.h"\
	{$(INCLUDE)}"xgraphics.h"\
	
NODEP_CPP_FILES_F=\
	"..\..\NSL\pc\nl_pc.h"\
	"..\..\NSL\pc\nsl_pc_ext.h"\
	".\defalloc.h"\
	".\gc_arammgr.h"\
	".\hwosmks\mks_devopts.h"\
	".\hwosmks\mks_errmsg.h"\
	".\hwosmks\mks_movieplayer.h"\
	".\hwosmks\set5_rasterize.h"\
	".\hwosmks\set5_timer.h"\
	".\hwosmks\sh4_math.h"\
	".\hwosmks\sy_alloc.h"\
	".\hwosmks\sy_file.h"\
	".\hwosnglpc\algebra.h"\
	".\hwosnglpc\pc_algebra.h"\
	".\hwosnull\null_alloc.h"\
	".\hwosnull\null_devopts.h"\
	".\hwosnull\null_errmsg.h"\
	".\hwosnull\null_file.h"\
	".\hwosnull\null_math.h"\
	".\hwosnull\null_movieplayer.h"\
	".\hwosnull\null_rasterize.h"\
	".\hwosnull\null_timer.h"\
	".\hwospc\d3d_rasterize.h"\
	".\hwospc\pc_algebra.h"\
	".\hwospc\pc_movieplayer.h"\
	".\hwospc\w32_alloc.h"\
	".\hwospc\w32_devopts.h"\
	".\hwospc\w32_errmsg.h"\
	".\hwospc\w32_file.h"\
	".\hwospc\w32_timer.h"\
	".\hwospc\x86_math.h"\
	".\hwosxb\ngl_xbox.h"\
	".\hwosxb\nvl_xbox.h"\
	".\ks\gc_algebra.h"\
	".\LipSync_Anim.h"\
	".\ngl_xbox.h"\
	".\nvl_gc.h"\
	".\nvl_xbox.h"\
	".\nvlstream_gc.h"\
	
# ADD BASE CPP /Yu
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

DEP_CPP_FILES_F=\
	"..\..\ngl\ps2\ngl_fixedstr.h"\
	"..\..\ngl\ps2\ngl_instbank.h"\
	"..\..\ngl\ps2\ngl_ps2.h"\
	"..\..\ngl\ps2\tim2.h"\
	"..\..\NGL\XBOX\ngl_FixedStr.h"\
	"..\..\NGL\XBOX\ngl_types.h"\
	"..\..\NGL\XBOX\ngl_xbox.h"\
	".\ks\AccompFrontEnd.cpp"\
	".\ks\BeachFrontEnd.cpp"\
	".\ks\boarddata.cpp"\
	".\ks\BoardFrontEnd.cpp"\
	".\ks\BoardManager.h"\
	".\ks\CheatFrontEnd.cpp"\
	".\ks\DemoMode.h"\
	".\ks\ExtrasFrontEnd.cpp"\
	".\ks\FEAnim.cpp"\
	".\ks\FEEntityManager.cpp"\
	".\ks\FEMenu.cpp"\
	".\ks\FEPanel.cpp"\
	".\ks\FrontEndManager.cpp"\
	".\ks\FrontEndMenus.cpp"\
	".\ks\GameData.cpp"\
	".\ks\globaltextenum.cpp"\
	".\ks\GraphicalMenuSystem.cpp"\
	".\ks\HighScoreFrontEnd.cpp"\
	".\ks\igo_widget.cpp"\
	".\ks\igo_widget_analogclock.cpp"\
	".\ks\igo_widget_balance.cpp"\
	".\ks\igo_widget_camera.cpp"\
	".\ks\igo_widget_fanmeter.cpp"\
	".\ks\igo_widget_iconradar.cpp"\
	".\ks\igo_widget_photo.cpp"\
	".\ks\igo_widget_replay.cpp"\
	".\ks\igo_widget_simple.cpp"\
	".\ks\igo_widget_specialmeter.cpp"\
	".\ks\igo_widget_splitclock.cpp"\
	".\ks\igo_widget_splitmeter.cpp"\
	".\ks\igo_widget_splitscore.cpp"\
	".\ks\igo_widget_splitter.cpp"\
	".\ks\igo_widget_timeattack.cpp"\
	".\ks\igo_widget_waveindicator.cpp"\
	".\ks\IGOFrontEnd.cpp"\
	".\ks\igohintmanager.cpp"\
	".\ks\igoiconmanager.cpp"\
	".\ks\igolearn_new_trickmanager.cpp"\
	".\ks\ksfx.h"\
	".\ks\MainFrontEnd.cpp"\
	".\ks\Map.cpp"\
	".\ks\MCDetectFrontEnd.cpp"\
	".\ks\MultiFrontEnd.cpp"\
	".\ks\PAL60FrontEnd.cpp"\
	".\ks\PhotoFrontEnd.cpp"\
	".\ks\PlaylistMenu.cpp"\
	".\ks\PlaylistMenu.h"\
	".\ks\SaveLoadFrontEnd.cpp"\
	".\ks\StatsFrontEnd.cpp"\
	".\ks\SurferFrontEnd.cpp"\
	".\ks\text_parser.h"\
	".\ks\TrickBookFrontEnd.cpp"\
	".\ks\TutorialFrontEnd.cpp"\
	".\ks\tutorialmanager.cpp"\
	".\ks\wavesound.h"\
	
NODEP_CPP_FILES_F=\
	"..\..\ngl\common\ngl.h"\
	"..\..\ngl\common\types.h"\
	".\ekernel.h"\
	".\eregs.h"\
	".\estruct.h"\
	".\etypes.h"\
	".\ibcdvd.h"\
	".\ibdma.h"\
	".\ibgraph.h"\
	".\ibscf.h"\
	".\ibvu0.h"\
	".\ks\algebra.h"\
	".\ks\app.h"\
	".\ks\billboard.h"\
	".\ks\camera.h"\
	".\ks\colmesh.h"\
	".\ks\commands.h"\
	".\ks\conglom.h"\
	".\ks\controller.h"\
	".\ks\entity.h"\
	".\ks\entity_anim.h"\
	".\ks\entity_maker.h"\
	".\ks\entityflags.h"\
	".\ks\file_finder.h"\
	".\ks\game.h"\
	".\ks\gc_algebra.h"\
	".\ks\geomgr.h"\
	".\ks\global.h"\
	".\ks\heap.h"\
	".\ks\hwmovieplayer.h"\
	".\ks\hwosgc\gc_GameSaver.h"\
	".\ks\hwosgc\gc_input.h"\
	".\ks\hwosps2\ps2_GameSaver.h"\
	".\ks\hwosps2\ps2_input.h"\
	".\ks\hwosps2\ps2_m2vplayer.h"\
	".\ks\hwosxb\xb_GameSaver.h"\
	".\ks\hwosxb\xb_input.h"\
	".\ks\hwosxb\xb_particle.h"\
	".\ks\HWOSXB\xb_SaveLoadFrontEnd.cpp"\
	".\ks\hwrasterize.h"\
	".\ks\ini_parser.h"\
	".\ks\inputmgr.h"\
	".\ks\joystick.h"\
	".\ks\kshooks.h"\
	".\ks\ksnsl.h"\
	".\ks\ksnvl.h"\
	".\ks\lightmgr.h"\
	".\ks\mustash.h"\
	".\ks\osdevopts.h"\
	".\ks\pmesh.h"\
	".\ks\po.h"\
	".\ks\profiler.h"\
	".\ks\random.h"\
	".\ks\refptr.h"\
	".\ks\scene_anim.h"\
	".\ks\semaphores.h"\
	".\ks\singleton.h"\
	".\ks\stringx.h"\
	".\ks\text_font.h"\
	".\ks\timer.h"\
	".\ks\wds.h"\
	".\ks\xbglobals.h"\
	".\s\AccompFrontEnd.h"\
	".\s\andersen_ind_anims_mac.h"\
	".\s\beach.h"\
	".\s\beachdata.h"\
	".\s\BeachFrontEnd.h"\
	".\s\board.h"\
	".\s\board_anims_mac.h"\
	".\s\boarddata.h"\
	".\s\BoardFrontEnd.h"\
	".\s\career.h"\
	".\s\careerdata.h"\
	".\s\carrol_ind_anims_mac.h"\
	".\s\challenge.h"\
	".\s\challenge_icon.h"\
	".\s\challenge_photo.h"\
	".\s\cheat.h"\
	".\s\CheatFrontEnd.h"\
	".\s\compressedphoto.h"\
	".\s\coords.h"\
	".\s\curren_ind_anims_mac.h"\
	".\s\eventmanager.h"\
	".\s\ExtrasFrontEnd.h"\
	".\s\FEAnim.h"\
	".\s\FEEntityManager.h"\
	".\s\FEMenu.h"\
	".\s\FEPanel.h"\
	".\s\fletcher_ind_anims_mac.h"\
	".\s\floatobj.h"\
	".\s\frankenreiter_ind_anims_mac.h"\
	".\s\FrontEndManager.h"\
	".\s\FrontEndMenus.h"\
	".\s\GameData.h"\
	".\s\GlobalData.h"\
	".\s\globaltextenum.h"\
	".\s\GraphicalMenuSystem.h"\
	".\s\HighScoreFrontEnd.h"\
	".\s\igo_widget.h"\
	".\s\igo_widget_analogclock.h"\
	".\s\igo_widget_balance.h"\
	".\s\igo_widget_camera.h"\
	".\s\igo_widget_fanmeter.h"\
	".\s\igo_widget_iconradar.h"\
	".\s\igo_widget_photo.h"\
	".\s\igo_widget_replay.h"\
	".\s\igo_widget_simple.h"\
	".\s\igo_widget_specialmeter.h"\
	".\s\igo_widget_splitclock.h"\
	".\s\igo_widget_splitmeter.h"\
	".\s\igo_widget_splitscore.h"\
	".\s\igo_widget_splitter.h"\
	".\s\igo_widget_timeattack.h"\
	".\s\igo_widget_waveindicator.h"\
	".\s\IGOFrontEnd.h"\
	".\s\igohintmanager.h"\
	".\s\igoiconmanager.h"\
	".\s\igolearn_new_trickmanager.h"\
	".\s\ik_object.h"\
	".\s\irons_ind_anims_mac.h"\
	".\s\judge.h"\
	".\s\kellyslater_controller.h"\
	".\s\kellyslater_shared_anims_mac.h"\
	".\s\kellyslater_states_mac.h"\
	".\s\ks_camera.h"\
	".\s\ksdbmenu.h"\
	".\s\ksngl.h"\
	".\s\ksreplay.h"\
	".\s\machado_ind_anims_mac.h"\
	".\s\MainFrontEnd.h"\
	".\s\Map.h"\
	".\s\MCDetectFrontEnd.h"\
	".\s\menu.h"\
	".\s\menusys.h"\
	".\s\MultiFrontEnd.h"\
	".\s\MusicMan.h"\
	".\s\ode.h"\
	".\s\osGameSaver.h"\
	".\s\osparticle.h"\
	".\s\PAL60FrontEnd.h"\
	".\s\PathData.h"\
	".\s\PhotoFrontEnd.h"\
	".\s\physics.h"\
	".\s\player.h"\
	".\s\replay.h"\
	".\s\robb_ind_anims_mac.h"\
	".\s\rumbleManager.h"\
	".\s\SaveLoadFrontEnd.h"\
	".\s\scoringmanager.h"\
	".\s\slater_ind_anims_mac.h"\
	".\s\sounddata.h"\
	".\s\SoundScript.h"\
	".\s\specialmeter.h"\
	".\s\StatsFrontEnd.h"\
	".\s\surferdata.h"\
	".\s\SurferFrontEnd.h"\
	".\s\trick_system.h"\
	".\s\TrickBookFrontEnd.h"\
	".\s\trickdata.h"\
	".\s\trickregion.h"\
	".\s\tricks.h"\
	".\s\tutorialdata.h"\
	".\s\TutorialFrontEnd.h"\
	".\s\tutorialmanager.h"\
	".\s\VOEngine.h"\
	".\s\wave.h"\
	".\s\wavebreak.txt"\
	".\s\wavebreakstage.txt"\
	".\s\waveemitter.txt"\
	".\s\waveenum.h"\
	".\s\wavemarker.txt"\
	".\s\waveregion.txt"\
	".\s\wavestage.txt"\
	
# SUBTRACT BASE CPP /YX /Yc /Yu
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

DEP_CPP_FILES_F=\
	"..\..\ngl\ps2\ngl_fixedstr.h"\
	"..\..\ngl\ps2\ngl_instbank.h"\
	"..\..\ngl\ps2\ngl_ps2.h"\
	"..\..\ngl\ps2\tim2.h"\
	".\ks\AccompFrontEnd.cpp"\
	".\ks\BeachFrontEnd.cpp"\
	".\ks\boarddata.cpp"\
	".\ks\BoardFrontEnd.cpp"\
	".\ks\BoardManager.h"\
	".\ks\CheatFrontEnd.cpp"\
	".\ks\DemoMode.h"\
	".\ks\ExtrasFrontEnd.cpp"\
	".\ks\FEAnim.cpp"\
	".\ks\FEEntityManager.cpp"\
	".\ks\FEMenu.cpp"\
	".\ks\FEPanel.cpp"\
	".\ks\FrontEndManager.cpp"\
	".\ks\FrontEndMenus.cpp"\
	".\ks\GameData.cpp"\
	".\ks\globaltextenum.cpp"\
	".\ks\GraphicalMenuSystem.cpp"\
	".\ks\HighScoreFrontEnd.cpp"\
	".\ks\igo_widget.cpp"\
	".\ks\igo_widget_analogclock.cpp"\
	".\ks\igo_widget_balance.cpp"\
	".\ks\igo_widget_camera.cpp"\
	".\ks\igo_widget_fanmeter.cpp"\
	".\ks\igo_widget_iconcount.cpp"\
	".\ks\igo_widget_iconradar.cpp"\
	".\ks\igo_widget_meterchallenge.cpp"\
	".\ks\igo_widget_objectalert.cpp"\
	".\ks\igo_widget_photo.cpp"\
	".\ks\igo_widget_replay.cpp"\
	".\ks\igo_widget_simple.cpp"\
	".\ks\igo_widget_skillchallenge.cpp"\
	".\ks\igo_widget_specialmeter.cpp"\
	".\ks\igo_widget_splitclock.cpp"\
	".\ks\igo_widget_splitmeter.cpp"\
	".\ks\igo_widget_splitscore.cpp"\
	".\ks\igo_widget_splitter.cpp"\
	".\ks\igo_widget_timeattack.cpp"\
	".\ks\igo_widget_waveindicator.cpp"\
	".\ks\IGOFrontEnd.cpp"\
	".\ks\igohintmanager.cpp"\
	".\ks\igoiconmanager.cpp"\
	".\ks\igolearn_new_trickmanager.cpp"\
	".\ks\ksfx.h"\
	".\ks\LogbookFrontEnd.cpp"\
	".\ks\MainFrontEnd.cpp"\
	".\ks\Map.cpp"\
	".\ks\MCDetectFrontEnd.cpp"\
	".\ks\MultiFrontEnd.cpp"\
	".\ks\PAL60FrontEnd.cpp"\
	".\ks\PhotoFrontEnd.cpp"\
	".\ks\PlaylistMenu.cpp"\
	".\ks\PlaylistMenu.h"\
	".\ks\SaveLoadFrontEnd.cpp"\
	".\ks\StatsFrontEnd.cpp"\
	".\ks\SurferFrontEnd.cpp"\
	".\ks\text_parser.h"\
	".\ks\TrickBookFrontEnd.cpp"\
	".\ks\TutorialFrontEnd.cpp"\
	".\ks\tutorialmanager.cpp"\
	".\ks\wavesound.h"\
	
NODEP_CPP_FILES_F=\
	".\ekernel.h"\
	".\eregs.h"\
	".\estruct.h"\
	".\etypes.h"\
	".\ibcdvd.h"\
	".\ibdma.h"\
	".\ibgraph.h"\
	".\ibscf.h"\
	".\ibvu0.h"\
	".\ifcmd.h"\
	".\ks\algebra.h"\
	".\ks\app.h"\
	".\ks\billboard.h"\
	".\ks\camera.h"\
	".\ks\colmesh.h"\
	".\ks\commands.h"\
	".\ks\conglom.h"\
	".\ks\controller.h"\
	".\ks\entity.h"\
	".\ks\entity_anim.h"\
	".\ks\entity_maker.h"\
	".\ks\entityflags.h"\
	".\ks\file_finder.h"\
	".\ks\game.h"\
	".\ks\gc_algebra.h"\
	".\ks\geomgr.h"\
	".\ks\global.h"\
	".\ks\heap.h"\
	".\ks\hwmovieplayer.h"\
	".\ks\hwosgc\gc_GameSaver.h"\
	".\ks\hwosgc\gc_input.h"\
	".\ks\hwosps2\ps2_GameSaver.h"\
	".\ks\hwosps2\ps2_input.h"\
	".\ks\hwosps2\ps2_m2vplayer.h"\
	".\ks\hwosxb\xb_GameSaver.h"\
	".\ks\hwosxb\xb_input.h"\
	".\ks\hwosxb\xb_particle.h"\
	".\ks\HWOSXB\xb_SaveLoadFrontEnd.cpp"\
	".\ks\hwrasterize.h"\
	".\ks\ini_parser.h"\
	".\ks\inputmgr.h"\
	".\ks\joystick.h"\
	".\ks\kshooks.h"\
	".\ks\ksnsl.h"\
	".\ks\ksnvl.h"\
	".\ks\lightmgr.h"\
	".\ks\mustash.h"\
	".\ks\ngl.h"\
	".\ks\osdevopts.h"\
	".\ks\pmesh.h"\
	".\ks\po.h"\
	".\ks\profiler.h"\
	".\ks\random.h"\
	".\ks\refptr.h"\
	".\ks\scene_anim.h"\
	".\ks\semaphores.h"\
	".\ks\singleton.h"\
	".\ks\stringx.h"\
	".\ks\text_font.h"\
	".\ks\timer.h"\
	".\ks\types.h"\
	".\ks\wds.h"\
	".\ks\xbglobals.h"\
	".\s\AccompFrontEnd.h"\
	".\s\andersen_ind_anims_mac.h"\
	".\s\beach.h"\
	".\s\beachdata.h"\
	".\s\BeachFrontEnd.h"\
	".\s\board.h"\
	".\s\board_anims_mac.h"\
	".\s\boarddata.h"\
	".\s\BoardFrontEnd.h"\
	".\s\career.h"\
	".\s\careerdata.h"\
	".\s\carrol_ind_anims_mac.h"\
	".\s\challenge.h"\
	".\s\challenge_icon.h"\
	".\s\challenge_photo.h"\
	".\s\cheat.h"\
	".\s\CheatFrontEnd.h"\
	".\s\compressedphoto.h"\
	".\s\coords.h"\
	".\s\curren_ind_anims_mac.h"\
	".\s\eventmanager.h"\
	".\s\ExtrasFrontEnd.h"\
	".\s\FEAnim.h"\
	".\s\FEEntityManager.h"\
	".\s\FEMenu.h"\
	".\s\FEPanel.h"\
	".\s\fletcher_ind_anims_mac.h"\
	".\s\floatobj.h"\
	".\s\frankenreiter_ind_anims_mac.h"\
	".\s\FrontEndManager.h"\
	".\s\FrontEndMenus.h"\
	".\s\GameData.h"\
	".\s\GlobalData.h"\
	".\s\globaltextenum.h"\
	".\s\GraphicalMenuSystem.h"\
	".\s\HighScoreFrontEnd.h"\
	".\s\igo_widget.h"\
	".\s\igo_widget_analogclock.h"\
	".\s\igo_widget_balance.h"\
	".\s\igo_widget_camera.h"\
	".\s\igo_widget_fanmeter.h"\
	".\s\igo_widget_iconcount.h"\
	".\s\igo_widget_iconradar.h"\
	".\s\igo_widget_meterchallenge.h"\
	".\s\igo_widget_objectalert.h"\
	".\s\igo_widget_photo.h"\
	".\s\igo_widget_replay.h"\
	".\s\igo_widget_simple.h"\
	".\s\igo_widget_skillchallenge.h"\
	".\s\igo_widget_specialmeter.h"\
	".\s\igo_widget_splitclock.h"\
	".\s\igo_widget_splitmeter.h"\
	".\s\igo_widget_splitscore.h"\
	".\s\igo_widget_splitter.h"\
	".\s\igo_widget_timeattack.h"\
	".\s\igo_widget_waveindicator.h"\
	".\s\IGOFrontEnd.h"\
	".\s\igohintmanager.h"\
	".\s\igoiconmanager.h"\
	".\s\igolearn_new_trickmanager.h"\
	".\s\ik_object.h"\
	".\s\irons_ind_anims_mac.h"\
	".\s\judge.h"\
	".\s\kellyslater_controller.h"\
	".\s\kellyslater_shared_anims_mac.h"\
	".\s\kellyslater_states_mac.h"\
	".\s\ks_camera.h"\
	".\s\ksdbmenu.h"\
	".\s\ksngl.h"\
	".\s\ksreplay.h"\
	".\s\LogbookFrontEnd.h"\
	".\s\machado_ind_anims_mac.h"\
	".\s\MainFrontEnd.h"\
	".\s\Map.h"\
	".\s\MCDetectFrontEnd.h"\
	".\s\menu.h"\
	".\s\menusys.h"\
	".\s\MultiFrontEnd.h"\
	".\s\MusicMan.h"\
	".\s\ode.h"\
	".\s\osGameSaver.h"\
	".\s\osparticle.h"\
	".\s\PAL60FrontEnd.h"\
	".\s\PathData.h"\
	".\s\PhotoFrontEnd.h"\
	".\s\physics.h"\
	".\s\player.h"\
	".\s\replay.h"\
	".\s\robb_ind_anims_mac.h"\
	".\s\rumbleManager.h"\
	".\s\SaveLoadFrontEnd.h"\
	".\s\scoringmanager.h"\
	".\s\slater_ind_anims_mac.h"\
	".\s\sounddata.h"\
	".\s\SoundScript.h"\
	".\s\specialmeter.h"\
	".\s\StatsFrontEnd.h"\
	".\s\surferdata.h"\
	".\s\SurferFrontEnd.h"\
	".\s\trick_system.h"\
	".\s\TrickBookFrontEnd.h"\
	".\s\trickdata.h"\
	".\s\trickregion.h"\
	".\s\tricks.h"\
	".\s\tutorialdata.h"\
	".\s\TutorialFrontEnd.h"\
	".\s\tutorialmanager.h"\
	".\s\VOEngine.h"\
	".\s\wave.h"\
	".\s\wavebreak.txt"\
	".\s\wavebreakstage.txt"\
	".\s\waveemitter.txt"\
	".\s\waveenum.h"\
	".\s\wavemarker.txt"\
	".\s\waveregion.txt"\
	".\s\wavestage.txt"\
	
# SUBTRACT BASE CPP /YX /Yc /Yu
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\files_hwosgc.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\files_hwosps2.cpp
DEP_CPP_FILES_H=\
	"..\..\NGL\ps2\libsn.h"\
	"..\..\ngl\ps2\matrix.h"\
	"..\..\ngl\ps2\matrix_common.h"\
	"..\..\ngl\ps2\ngl_dma.h"\
	"..\..\ngl\ps2\ngl_fixedstr.h"\
	"..\..\ngl\ps2\ngl_instbank.h"\
	"..\..\ngl\ps2\ngl_ps2.h"\
	"..\..\ngl\ps2\ngl_ps2_internal.h"\
	"..\..\NGL\ps2\ngl_vudefs.h"\
	"..\..\ngl\ps2\tim2.h"\
	"..\..\ngl\ps2\vector.h"\
	"..\..\ngl\ps2\vector_common.h"\
	"..\..\NSL\common\nl.h"\
	"..\..\NSL\common\nsl.h"\
	"..\..\NSL\common\ProjectOptions.h"\
	"..\..\NSL\gamecube\nl_gc.h"\
	"..\..\NSL\ps2\fifo_queue.cpp"\
	"..\..\NSL\ps2\fifo_queue.h"\
	"..\..\NSL\PS2\gas.h"\
	"..\..\NSL\PS2\gas_utility.h"\
	"..\..\NSL\PS2\nl_ps2.h"\
	"..\..\NSL\PS2\nsl_ps2.h"\
	"..\..\NSL\XBOX\nl_xbox.h"\
	"..\..\NSL\XBOX\nsl_xbox.h"\
	"..\..\NSL\XBOX\nsl_xbox_ext.h"\
	"..\..\NSL\XBOX\nsl_xbox_source.h"\
	"..\..\nsl\xbox\nsl_xbox_streams.h"\
	"..\..\NVL\ps2\nvl_ps2.h"\
	"..\..\NVL\ps2\nvlMPEG_ps2.h"\
	"..\..\NVL\ps2\nvlstream_ps2.h"\
	".\acostable.txt"\
	".\aggvertbuf.h"\
	".\algebra.h"\
	".\anim.h"\
	".\anim_flavor.h"\
	".\anim_flavors.h"\
	".\anim_ids.h"\
	".\app.h"\
	".\archalloc.h"\
	".\avltree.h"\
	".\beam.h"\
	".\beam_signals.h"\
	".\billboard.h"\
	".\bitplane.h"\
	".\bone.h"\
	".\bound.h"\
	".\box_trigger_interface.h"\
	".\camera.h"\
	".\capsule.h"\
	".\cface.h"\
	".\chunkfile.h"\
	".\colgeom.h"\
	".\colmesh.h"\
	".\color.h"\
	".\commands.h"\
	".\conglom.h"\
	".\constants.h"\
	".\controller.h"\
	".\convex_box.h"\
	".\custom_stl.h"\
	".\debug.h"\
	".\devoptflags.h"\
	".\devoptints.h"\
	".\devoptstrs.h"\
	".\entflavor.h"\
	".\entity.h"\
	".\entity_anim.h"\
	".\entity_interface.h"\
	".\entity_maker.h"\
	".\entity_signals.h"\
	".\entityflags.h"\
	".\entityid.h"\
	".\errorcontext.h"\
	".\face.h"\
	".\faceflags.h"\
	".\fast_vector.h"\
	".\file.h"\
	".\file_finder.h"\
	".\filespec.h"\
	".\frame_info.h"\
	".\game.h"\
	".\game_info.h"\
	".\game_info_vars.h"\
	".\game_process.h"\
	".\gcglobals.h"\
	".\generator.h"\
	".\geomgr.h"\
	".\global.h"\
	".\global_signals.h"\
	".\graph.h"\
	".\hull.h"\
	".\hwmath.h"\
	".\hwmovieplayer.h"\
	".\hwosgc\gc_storage.h"\
	".\HWOSPS2\ps2_algebra.cpp"\
	".\HWOSPS2\ps2_alloc.cpp"\
	".\HWOSPS2\ps2_audio.cpp"\
	".\HWOSPS2\ps2_errmsg.cpp"\
	".\HWOSPS2\ps2_file.cpp"\
	".\HWOSPS2\ps2_gamesaver.cpp"\
	".\HWOSPS2\ps2_graphics.cpp"\
	".\HWOSPS2\ps2_input.cpp"\
	".\HWOSPS2\ps2_m2vplayer.c"\
	".\HWOSPS2\ps2_math.cpp"\
	".\HWOSPS2\ps2_movie_audiodec.c"\
	".\HWOSPS2\ps2_movie_disp.c"\
	".\HWOSPS2\ps2_movie_read.c"\
	".\HWOSPS2\ps2_movie_readbuf.c"\
	".\HWOSPS2\ps2_movie_strfile.c"\
	".\HWOSPS2\ps2_movie_util.c"\
	".\HWOSPS2\ps2_movie_vibuf.c"\
	".\HWOSPS2\ps2_movie_videodec.c"\
	".\HWOSPS2\ps2_movie_vobuf.c"\
	".\HWOSPS2\ps2_movieplayer.cpp"\
	".\HWOSPS2\ps2_rasterize.cpp"\
	".\HWOSPS2\ps2_storage.cpp"\
	".\HWOSPS2\ps2_storage.h"\
	".\HWOSPS2\ps2_texturemgr.cpp"\
	".\HWOSPS2\ps2_timer.cpp"\
	".\HWOSPS2\ps2_waverender.cpp"\
	".\hwosxb\xb_storage.h"\
	".\hwrasterize.h"\
	".\hyperplane.h"\
	".\ini_parser.h"\
	".\inputmgr.h"\
	".\instance.h"\
	".\item.h"\
	".\item_signals.h"\
	".\joystick.h"\
	".\keyboard.h"\
	".\ks\ksheaps.h"\
	".\ks\waverendermenu.cpp"\
	".\kshooks.h"\
	".\ksnsl.h"\
	".\ksnvl.h"\
	".\light.h"\
	".\linear_anim.h"\
	".\link_interface.h"\
	".\map_e.h"\
	".\material.h"\
	".\matfac.h"\
	".\maxskinbones.h"\
	".\meshrefs.h"\
	".\mic.h"\
	".\mobject.h"\
	".\mustash.h"\
	".\ngl.h"\
	".\osalloc.h"\
	".\osassert.h"\
	".\osdevopts.h"\
	".\oserrmsg.h"\
	".\osfile.h"\
	".\osstorage.h"\
	".\ostimer.h"\
	".\path.h"\
	".\pc_port.h"\
	".\plane.h"\
	".\platform_defines.h"\
	".\pmesh.h"\
	".\po.h"\
	".\po_anim.h"\
	".\portal.h"\
	".\profiler.h"\
	".\projconst.h"\
	".\project.h"\
	".\ps2main.cpp"\
	".\pstring.h"\
	".\random.h"\
	".\rect.h"\
	".\refptr.h"\
	".\region.h"\
	".\region_graph.h"\
	".\render_data.h"\
	".\renderflav.h"\
	".\scene_anim.h"\
	".\script_lib_mfg.h"\
	".\script_library_class.h"\
	".\script_mfg_signals.h"\
	".\script_object.h"\
	".\semaphores.h"\
	".\signal_anim.h"\
	".\signals.h"\
	".\SimpleAssert.h"\
	".\singleton.h"\
	".\sintable.txt"\
	".\so_data_block.h"\
	".\sphere.h"\
	".\stashes.h"\
	".\staticmem.h"\
	".\stl_adapter.h"\
	".\stringx.h"\
	".\terrain.h"\
	".\text_font.h"\
	".\textfile.h"\
	".\timer.h"\
	".\txtcoord.h"\
	".\types.h"\
	".\usefulmath.h"\
	".\users.h"\
	".\vert.h"\
	".\vertwork.h"\
	".\visrep.h"\
	".\vm_executable.h"\
	".\vm_symbol.h"\
	".\vm_symbol_list.h"\
	".\warnlvl.h"\
	".\wds.h"\
	".\wedge.h"\
	".\xbglobals.h"\
	{$(INCLUDE)}"devfont.h"\
	{$(INCLUDE)}"eekernel.h"\
	{$(INCLUDE)}"eeregs.h"\
	{$(INCLUDE)}"eestruct.h"\
	{$(INCLUDE)}"eetypes.h"\
	{$(INCLUDE)}"hwosgc\gc_algebra.h"\
	{$(INCLUDE)}"hwosgc\gc_alloc.h"\
	{$(INCLUDE)}"hwosgc\gc_errmsg.h"\
	{$(INCLUDE)}"hwosgc\gc_file.h"\
	{$(INCLUDE)}"hwosgc\gc_graphics.h"\
	{$(INCLUDE)}"hwosgc\gc_math.h"\
	{$(INCLUDE)}"hwosgc\gc_movieplayer.h"\
	{$(INCLUDE)}"hwosgc\gc_rasterize.h"\
	{$(INCLUDE)}"hwosgc\gc_texturemgr.h"\
	{$(INCLUDE)}"hwosgc\gc_timer.h"\
	{$(INCLUDE)}"hwosgc\ngl_fixedstr.h"\
	{$(INCLUDE)}"hwosgc\ngl_gc.h"\
	{$(INCLUDE)}"hwosgc\ngl_version.h"\
	{$(INCLUDE)}"hwosgc\nvl_gc.h"\
	{$(INCLUDE)}"HWOSPS2\gas.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_algebra.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_alloc.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_audio.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_devopts.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_errmsg.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_file.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_gamesaver.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_graphics.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_input.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_m2vplayer.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_math.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_movie_audiodec.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_movie_defs.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_movie_disp.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_movie_readbuf.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_movie_strfile.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_movie_vibuf.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_movie_videodec.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_movie_vobuf.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_movieplayer.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_rasterize.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_texturemgr.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_timer.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_types.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_waverender.h"\
	{$(INCLUDE)}"hwosxb\xb_algebra.h"\
	{$(INCLUDE)}"hwosxb\xb_alloc.h"\
	{$(INCLUDE)}"hwosxb\xb_errmsg.h"\
	{$(INCLUDE)}"hwosxb\xb_file.h"\
	{$(INCLUDE)}"hwosxb\xb_graphics.h"\
	{$(INCLUDE)}"hwosxb\xb_math.h"\
	{$(INCLUDE)}"hwosxb\xb_movieplayer.h"\
	{$(INCLUDE)}"HWOSXB\xb_portDVDCache.h"\
	{$(INCLUDE)}"hwosxb\xb_rasterize.h"\
	{$(INCLUDE)}"hwosxb\xb_string.h"\
	{$(INCLUDE)}"hwosxb\xb_texturemgr.h"\
	{$(INCLUDE)}"hwosxb\xb_timer.h"\
	{$(INCLUDE)}"ks\andersen_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\beachdata.h"\
	{$(INCLUDE)}"ks\board.h"\
	{$(INCLUDE)}"ks\board_anims_mac.h"\
	{$(INCLUDE)}"ks\boarddata.h"\
	{$(INCLUDE)}"ks\career.h"\
	{$(INCLUDE)}"ks\careerdata.h"\
	{$(INCLUDE)}"ks\carrol_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\cheat.h"\
	{$(INCLUDE)}"ks\compressedphoto.h"\
	{$(INCLUDE)}"ks\coords.h"\
	{$(INCLUDE)}"ks\curren_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\eventmanager.h"\
	{$(INCLUDE)}"ks\FEAnim.h"\
	{$(INCLUDE)}"ks\FEMenu.h"\
	{$(INCLUDE)}"ks\FEPanel.h"\
	{$(INCLUDE)}"ks\fletcher_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\floatobj.h"\
	{$(INCLUDE)}"ks\frankenreiter_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\GameData.h"\
	{$(INCLUDE)}"ks\GlobalData.h"\
	{$(INCLUDE)}"ks\globaltextenum.h"\
	{$(INCLUDE)}"ks\HighScoreFrontEnd.h"\
	{$(INCLUDE)}"ks\ik_object.h"\
	{$(INCLUDE)}"ks\irons_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\kellyslater_controller.h"\
	{$(INCLUDE)}"ks\kellyslater_shared_anims_mac.h"\
	{$(INCLUDE)}"ks\kellyslater_states_mac.h"\
	{$(INCLUDE)}"ks\ks_camera.h"\
	{$(INCLUDE)}"ks\ksngl.h"\
	{$(INCLUDE)}"ks\ksreplay.h"\
	{$(INCLUDE)}"ks\machado_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\menu.h"\
	{$(INCLUDE)}"ks\menusys.h"\
	{$(INCLUDE)}"ks\mode.h"\
	{$(INCLUDE)}"ks\mode_headtohead.h"\
	{$(INCLUDE)}"ks\mode_meterattack.h"\
	{$(INCLUDE)}"ks\mode_push.h"\
	{$(INCLUDE)}"ks\mode_timeattack.h"\
	{$(INCLUDE)}"ks\MusicMan.h"\
	{$(INCLUDE)}"ks\ode.h"\
	{$(INCLUDE)}"ks\physics.h"\
	{$(INCLUDE)}"ks\player.h"\
	{$(INCLUDE)}"ks\replay.h"\
	{$(INCLUDE)}"ks\robb_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\scoringmanager.h"\
	{$(INCLUDE)}"ks\slater_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\sounddata.h"\
	{$(INCLUDE)}"ks\SoundScript.h"\
	{$(INCLUDE)}"ks\specialmeter.h"\
	{$(INCLUDE)}"ks\surferdata.h"\
	{$(INCLUDE)}"ks\trick_system.h"\
	{$(INCLUDE)}"ks\trickdata.h"\
	{$(INCLUDE)}"ks\trickregion.h"\
	{$(INCLUDE)}"ks\tricks.h"\
	{$(INCLUDE)}"ks\underwtr.h"\
	{$(INCLUDE)}"ks\VOEngine.h"\
	{$(INCLUDE)}"ks\wave.h"\
	{$(INCLUDE)}"ks\wavebreak.txt"\
	{$(INCLUDE)}"ks\wavebreakstage.txt"\
	{$(INCLUDE)}"ks\waveemitter.txt"\
	{$(INCLUDE)}"ks\waveenum.h"\
	{$(INCLUDE)}"ks\wavemarker.txt"\
	{$(INCLUDE)}"ks\waveregion.txt"\
	{$(INCLUDE)}"ks\wavestage.txt"\
	{$(INCLUDE)}"libcdvd.h"\
	{$(INCLUDE)}"libdma.h"\
	{$(INCLUDE)}"libdmapk.h"\
	{$(INCLUDE)}"libgifpk.h"\
	{$(INCLUDE)}"libgraph.h"\
	{$(INCLUDE)}"libipu.h"\
	{$(INCLUDE)}"libmc.h"\
	{$(INCLUDE)}"libmpeg.h"\
	{$(INCLUDE)}"libpad.h"\
	{$(INCLUDE)}"libpc.h"\
	{$(INCLUDE)}"libpkt.h"\
	{$(INCLUDE)}"libsdr.h"\
	{$(INCLUDE)}"libusbkb.h"\
	{$(INCLUDE)}"libvifpk.h"\
	{$(INCLUDE)}"libvu0.h"\
	{$(INCLUDE)}"sdmacro.h"\
	{$(INCLUDE)}"sdrcmd.h"\
	{$(INCLUDE)}"sif.h"\
	{$(INCLUDE)}"sifcmd.h"\
	{$(INCLUDE)}"sifdev.h"\
	{$(INCLUDE)}"sifrpc.h"\
	{$(INCLUDE)}"sntty.h"\
	{$(INCLUDE)}"xgraphics.h"\
	
NODEP_CPP_FILES_H=\
	"..\..\NSL\pc\nl_pc.h"\
	"..\..\NSL\pc\nsl_pc_ext.h"\
	".\defalloc.h"\
	".\gc_arammgr.h"\
	".\hwosmks\mks_devopts.h"\
	".\hwosmks\mks_errmsg.h"\
	".\hwosmks\mks_movieplayer.h"\
	".\hwosmks\set5_rasterize.h"\
	".\hwosmks\set5_timer.h"\
	".\hwosmks\sh4_math.h"\
	".\hwosmks\sy_alloc.h"\
	".\hwosmks\sy_file.h"\
	".\hwosmks\vmu_storage.h"\
	".\hwosnglpc\algebra.h"\
	".\hwosnglpc\pc_algebra.h"\
	".\hwosnull\null_alloc.h"\
	".\hwosnull\null_devopts.h"\
	".\hwosnull\null_errmsg.h"\
	".\hwosnull\null_file.h"\
	".\hwosnull\null_math.h"\
	".\hwosnull\null_movieplayer.h"\
	".\hwosnull\null_rasterize.h"\
	".\hwosnull\null_storage.h"\
	".\hwosnull\null_timer.h"\
	".\hwospc\d3d_rasterize.h"\
	".\hwospc\pc_algebra.h"\
	".\hwospc\pc_movieplayer.h"\
	".\hwospc\w32_alloc.h"\
	".\hwospc\w32_devopts.h"\
	".\hwospc\w32_errmsg.h"\
	".\hwospc\w32_file.h"\
	".\hwospc\w32_storage.h"\
	".\hwospc\w32_timer.h"\
	".\hwospc\x86_math.h"\
	".\hwosxb\ngl_xbox.h"\
	".\hwosxb\nvl_xbox.h"\
	".\LipSync_Anim.h"\
	".\ngl_xbox.h"\
	".\nvl_gc.h"\
	".\nvl_xbox.h"\
	".\nvlstream_gc.h"\
	

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\files_hwosxb.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\files_kellyslater.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

DEP_CPP_FILES_K=\
	"..\..\ngl\ps2\ngl_fixedstr.h"\
	"..\..\ngl\ps2\ngl_ps2.h"\
	"..\..\ngl\ps2\tim2.h"\
	"..\..\NSL\common\nl.h"\
	"..\..\NSL\common\nsl.h"\
	"..\..\NSL\common\ProjectOptions.h"\
	"..\..\NSL\gamecube\nl_gc.h"\
	"..\..\NSL\ps2\fifo_queue.cpp"\
	"..\..\NSL\ps2\fifo_queue.h"\
	"..\..\NSL\PS2\gas.h"\
	"..\..\NSL\PS2\gas_utility.h"\
	"..\..\NSL\PS2\nl_ps2.h"\
	"..\..\NSL\PS2\nsl_ps2.h"\
	"..\..\NSL\XBOX\nl_xbox.h"\
	"..\..\NSL\XBOX\nsl_xbox.h"\
	"..\..\NSL\XBOX\nsl_xbox_ext.h"\
	"..\..\NSL\XBOX\nsl_xbox_source.h"\
	"..\..\nsl\xbox\nsl_xbox_streams.h"\
	"..\..\NVL\ps2\nvl_ps2.h"\
	"..\..\NVL\ps2\nvlMPEG_ps2.h"\
	"..\..\NVL\ps2\nvlstream_ps2.h"\
	".\aggvertbuf.h"\
	".\ai_actions.h"\
	".\ai_constants.h"\
	".\ai_goals.h"\
	".\ai_interface.h"\
	".\ai_locomotion.h"\
	".\ai_polypath.h"\
	".\ai_polypath_cell.h"\
	".\ai_polypath_heap.h"\
	".\algebra.h"\
	".\anim.h"\
	".\anim_flavor.h"\
	".\anim_flavors.h"\
	".\anim_ids.h"\
	".\animation_interface.h"\
	".\app.h"\
	".\archalloc.h"\
	".\avltree.h"\
	".\beam.h"\
	".\beam_signals.h"\
	".\billboard.h"\
	".\blendmodes.h"\
	".\bone.h"\
	".\bound.h"\
	".\box_trigger_interface.h"\
	".\camera.h"\
	".\capsule.h"\
	".\cface.h"\
	".\chunkfile.h"\
	".\colgeom.h"\
	".\collide.h"\
	".\colmesh.h"\
	".\color.h"\
	".\commands.h"\
	".\conglom.h"\
	".\console.h"\
	".\constants.h"\
	".\controller.h"\
	".\convex_box.h"\
	".\custom_stl.h"\
	".\debug.h"\
	".\debug_render.h"\
	".\devoptflags.h"\
	".\devoptints.h"\
	".\devoptstrs.h"\
	".\entflavor.h"\
	".\entity.h"\
	".\entity_anim.h"\
	".\entity_interface.h"\
	".\entity_maker.h"\
	".\entity_signals.h"\
	".\entityflags.h"\
	".\entityid.h"\
	".\errorcontext.h"\
	".\face.h"\
	".\faceflags.h"\
	".\fast_vector.h"\
	".\file.h"\
	".\file_finder.h"\
	".\filespec.h"\
	".\fogmgr.h"\
	".\frame_info.h"\
	".\game.h"\
	".\game_info.h"\
	".\game_info_vars.h"\
	".\game_process.h"\
	".\gcglobals.h"\
	".\generator.h"\
	".\geomgr.h"\
	".\global.h"\
	".\global_signals.h"\
	".\graph.h"\
	".\guidance_sys.h"\
	".\hull.h"\
	".\hwaudio.h"\
	".\hwmath.h"\
	".\hwmovieplayer.h"\
	".\hwrasterize.h"\
	".\hyperplane.h"\
	".\ini_parser.h"\
	".\inputmgr.h"\
	".\instance.h"\
	".\interface.h"\
	".\item.h"\
	".\item_signals.h"\
	".\joystick.h"\
	".\keyboard.h"\
	".\ks\beach.cpp"\
	".\ks\beachdata.cpp"\
	".\ks\board.cpp"\
	".\ks\BoardManager.cpp"\
	".\ks\BoardManager.h"\
	".\ks\career.cpp"\
	".\ks\careerdata.cpp"\
	".\ks\challenge.cpp"\
	".\ks\challenge_icon.cpp"\
	".\ks\challenge_photo.cpp"\
	".\ks\cheat.cpp"\
	".\ks\combodata.cpp"\
	".\ks\combodata.h"\
	".\ks\compressedphoto.cpp"\
	".\ks\DemoMode.cpp"\
	".\ks\DemoMode.h"\
	".\ks\dxt1_codebook.cpp"\
	".\ks\dxt1_codebook.h"\
	".\ks\dxt1_gen.cpp"\
	".\ks\dxt1_gen.h"\
	".\ks\dxt1_imagedxt1.cpp"\
	".\ks\dxt1_imagedxt1.h"\
	".\ks\dxt1_table.h"\
	".\ks\floatobj.cpp"\
	".\ks\ik_object.cpp"\
	".\ks\judge.cpp"\
	".\ks\kellyslater_ai_goals.cpp"\
	".\ks\kellyslater_ai_goals.h"\
	".\ks\kellyslater_controller.cpp"\
	".\ks\kellyslater_main.cpp"\
	".\ks\ks_camera.cpp"\
	".\ks\ksdbmenu.cpp"\
	".\ks\ksfx.h"\
	".\ks\ksheaps.h"\
	".\ks\ksreplay.cpp"\
	".\ks\menu.cpp"\
	".\ks\menungl.cpp"\
	".\ks\menusys.cpp"\
	".\ks\MusicMan.cpp"\
	".\ks\ode.cpp"\
	".\ks\physics.cpp"\
	".\ks\player.cpp"\
	".\ks\rumbleManager.cpp"\
	".\ks\scoringmanager.cpp"\
	".\ks\SFXEngine.cpp"\
	".\ks\SFXEngine.h"\
	".\ks\sounddata.cpp"\
	".\ks\specialmeter.cpp"\
	".\ks\spline.cpp"\
	".\ks\surferdata.cpp"\
	".\ks\text_parser.cpp"\
	".\ks\text_parser.h"\
	".\ks\trail.h"\
	".\ks\trick_system.cpp"\
	".\ks\trickdata.cpp"\
	".\ks\tricks.cpp"\
	".\ks\tutorialdata.cpp"\
	".\ks\VOEngine.cpp"\
	".\ks\water.h"\
	".\ks\wavedata.h"\
	".\ks\wavesound.cpp"\
	".\ks\wavesound.h"\
	".\ks\wavetex.h"\
	".\ks\wipeoutdata.cpp"\
	".\ks\wipeoutdata.h"\
	".\kshooks.h"\
	".\ksnsl.h"\
	".\ksnvl.h"\
	".\light.h"\
	".\lightmgr.h"\
	".\linear_anim.h"\
	".\link_interface.h"\
	".\map_e.h"\
	".\material.h"\
	".\matfac.h"\
	".\maxskinbones.h"\
	".\menusound.h"\
	".\meshrefs.h"\
	".\mic.h"\
	".\mobject.h"\
	".\mouse.h"\
	".\msgboard.h"\
	".\mustash.h"\
	".\ngl.h"\
	".\osalloc.h"\
	".\osassert.h"\
	".\osdevopts.h"\
	".\oserrmsg.h"\
	".\osfile.h"\
	".\ostimer.h"\
	".\path.h"\
	".\pc_port.h"\
	".\physical_interface.h"\
	".\plane.h"\
	".\platform_defines.h"\
	".\pmesh.h"\
	".\po.h"\
	".\po_anim.h"\
	".\portal.h"\
	".\profiler.h"\
	".\projconst.h"\
	".\project.h"\
	".\pstring.h"\
	".\random.h"\
	".\rect.h"\
	".\refptr.h"\
	".\region.h"\
	".\region_graph.h"\
	".\render_data.h"\
	".\renderflav.h"\
	".\scene_anim.h"\
	".\script_access.h"\
	".\script_lib_mfg.h"\
	".\script_library_class.h"\
	".\script_mfg_signals.h"\
	".\script_object.h"\
	".\signal_anim.h"\
	".\signals.h"\
	".\simple_classes.h"\
	".\SimpleAssert.h"\
	".\singleton.h"\
	".\so_data_block.h"\
	".\sphere.h"\
	".\stashes.h"\
	".\staticmem.h"\
	".\stl_adapter.h"\
	".\stringx.h"\
	".\switch_obj.h"\
	".\switch_obj_signals.h"\
	".\terrain.h"\
	".\text_font.h"\
	".\textfile.h"\
	".\time_interface.h"\
	".\timer.h"\
	".\txtcoord.h"\
	".\types.h"\
	".\usefulmath.h"\
	".\users.h"\
	".\vert.h"\
	".\visrep.h"\
	".\vm_executable.h"\
	".\vm_symbol.h"\
	".\vm_symbol_list.h"\
	".\warnlvl.h"\
	".\wds.h"\
	".\wedge.h"\
	".\widget.h"\
	".\xbglobals.h"\
	{$(INCLUDE)}"eekernel.h"\
	{$(INCLUDE)}"eeregs.h"\
	{$(INCLUDE)}"eestruct.h"\
	{$(INCLUDE)}"eetypes.h"\
	{$(INCLUDE)}"hwosgc\gc_algebra.h"\
	{$(INCLUDE)}"hwosgc\gc_alloc.h"\
	{$(INCLUDE)}"hwosgc\gc_audio.h"\
	{$(INCLUDE)}"hwosgc\gc_errmsg.h"\
	{$(INCLUDE)}"hwosgc\gc_file.h"\
	{$(INCLUDE)}"HWOSGC\gc_gamesaver.h"\
	{$(INCLUDE)}"hwosgc\gc_graphics.h"\
	{$(INCLUDE)}"hwosgc\gc_input.h"\
	{$(INCLUDE)}"hwosgc\gc_math.h"\
	{$(INCLUDE)}"hwosgc\gc_movieplayer.h"\
	{$(INCLUDE)}"hwosgc\gc_rasterize.h"\
	{$(INCLUDE)}"hwosgc\gc_texturemgr.h"\
	{$(INCLUDE)}"hwosgc\gc_timer.h"\
	{$(INCLUDE)}"hwosgc\ngl_fixedstr.h"\
	{$(INCLUDE)}"hwosgc\ngl_gc.h"\
	{$(INCLUDE)}"hwosgc\ngl_version.h"\
	{$(INCLUDE)}"hwosgc\nvl_gc.h"\
	{$(INCLUDE)}"HWOSPS2\gas.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_algebra.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_alloc.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_audio.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_devopts.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_errmsg.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_file.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_gamesaver.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_graphics.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_input.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_m2vplayer.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_math.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_movieplayer.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_rasterize.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_texturemgr.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_timer.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_types.h"\
	{$(INCLUDE)}"hwosxb\pstring.h"\
	{$(INCLUDE)}"hwosxb\xb_algebra.h"\
	{$(INCLUDE)}"hwosxb\xb_alloc.h"\
	{$(INCLUDE)}"hwosxb\xb_audio.h"\
	{$(INCLUDE)}"hwosxb\xb_errmsg.h"\
	{$(INCLUDE)}"hwosxb\xb_file.h"\
	{$(INCLUDE)}"HWOSXB\xb_gamesaver.h"\
	{$(INCLUDE)}"hwosxb\xb_graphics.h"\
	{$(INCLUDE)}"hwosxb\xb_input.h"\
	{$(INCLUDE)}"hwosxb\xb_math.h"\
	{$(INCLUDE)}"hwosxb\xb_movieplayer.h"\
	{$(INCLUDE)}"HWOSXB\xb_particle.h"\
	{$(INCLUDE)}"HWOSXB\xb_portDVDCache.h"\
	{$(INCLUDE)}"hwosxb\xb_rasterize.h"\
	{$(INCLUDE)}"hwosxb\xb_string.h"\
	{$(INCLUDE)}"hwosxb\xb_texturemgr.h"\
	{$(INCLUDE)}"hwosxb\xb_timer.h"\
	{$(INCLUDE)}"hwosxb\xb_types.h"\
	{$(INCLUDE)}"ks\AccompFrontEnd.h"\
	{$(INCLUDE)}"ks\andersen_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\beach.h"\
	{$(INCLUDE)}"ks\beachdata.h"\
	{$(INCLUDE)}"ks\BeachFrontEnd.h"\
	{$(INCLUDE)}"ks\blur.h"\
	{$(INCLUDE)}"ks\board.h"\
	{$(INCLUDE)}"ks\board_anims_mac.h"\
	{$(INCLUDE)}"ks\boarddata.h"\
	{$(INCLUDE)}"ks\BoardFrontEnd.h"\
	{$(INCLUDE)}"ks\camera_tool.h"\
	{$(INCLUDE)}"ks\career.h"\
	{$(INCLUDE)}"ks\careerdata.h"\
	{$(INCLUDE)}"ks\carrol_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\challenge.h"\
	{$(INCLUDE)}"ks\challenge_icon.h"\
	{$(INCLUDE)}"ks\challenge_photo.h"\
	{$(INCLUDE)}"ks\cheat.h"\
	{$(INCLUDE)}"ks\CheatFrontEnd.h"\
	{$(INCLUDE)}"ks\compressedphoto.h"\
	{$(INCLUDE)}"ks\coords.h"\
	{$(INCLUDE)}"ks\curren_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\eventmanager.h"\
	{$(INCLUDE)}"ks\ExtrasFrontEnd.h"\
	{$(INCLUDE)}"ks\FEAnim.h"\
	{$(INCLUDE)}"ks\FEEntityManager.h"\
	{$(INCLUDE)}"ks\FEMenu.h"\
	{$(INCLUDE)}"ks\FEPanel.h"\
	{$(INCLUDE)}"ks\fletcher_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\floatobj.h"\
	{$(INCLUDE)}"ks\frankenreiter_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\FrontEndManager.h"\
	{$(INCLUDE)}"ks\FrontEndMenus.h"\
	{$(INCLUDE)}"ks\GameData.h"\
	{$(INCLUDE)}"ks\GlobalData.h"\
	{$(INCLUDE)}"ks\globaltextenum.h"\
	{$(INCLUDE)}"ks\GraphicalMenuSystem.h"\
	{$(INCLUDE)}"ks\HighScoreFrontEnd.h"\
	{$(INCLUDE)}"ks\igo_widget.h"\
	{$(INCLUDE)}"ks\igo_widget_analogclock.h"\
	{$(INCLUDE)}"ks\igo_widget_balance.h"\
	{$(INCLUDE)}"ks\igo_widget_camera.h"\
	{$(INCLUDE)}"ks\igo_widget_fanmeter.h"\
	{$(INCLUDE)}"ks\igo_widget_grid.h"\
	{$(INCLUDE)}"ks\igo_widget_iconcount.h"\
	{$(INCLUDE)}"ks\igo_widget_iconradar.h"\
	{$(INCLUDE)}"ks\igo_widget_meterchallenge.h"\
	{$(INCLUDE)}"ks\igo_widget_objectalert.h"\
	{$(INCLUDE)}"ks\igo_widget_photo.h"\
	{$(INCLUDE)}"ks\igo_widget_replay.h"\
	{$(INCLUDE)}"ks\igo_widget_simple.h"\
	{$(INCLUDE)}"ks\igo_widget_skillchallenge.h"\
	{$(INCLUDE)}"ks\igo_widget_specialmeter.h"\
	{$(INCLUDE)}"ks\igo_widget_splitclock.h"\
	{$(INCLUDE)}"ks\igo_widget_splitmeter.h"\
	{$(INCLUDE)}"ks\igo_widget_splitscore.h"\
	{$(INCLUDE)}"ks\igo_widget_splitter.h"\
	{$(INCLUDE)}"ks\igo_widget_timeattack.h"\
	{$(INCLUDE)}"ks\igo_widget_waveindicator.h"\
	{$(INCLUDE)}"ks\IGOFrontEnd.h"\
	{$(INCLUDE)}"ks\igohintmanager.h"\
	{$(INCLUDE)}"ks\igoiconmanager.h"\
	{$(INCLUDE)}"ks\igolearn_new_trickmanager.h"\
	{$(INCLUDE)}"ks\ik_object.h"\
	{$(INCLUDE)}"ks\irons_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\judge.h"\
	{$(INCLUDE)}"ks\kellyslater_controller.h"\
	{$(INCLUDE)}"ks\kellyslater_shared_anims_mac.h"\
	{$(INCLUDE)}"ks\kellyslater_states_mac.h"\
	{$(INCLUDE)}"ks\ks_camera.h"\
	{$(INCLUDE)}"ks\ksdbmenu.h"\
	{$(INCLUDE)}"ks\ksngl.h"\
	{$(INCLUDE)}"ks\ksreplay.h"\
	{$(INCLUDE)}"ks\LogbookFrontEnd.h"\
	{$(INCLUDE)}"ks\machado_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\MainFrontEnd.h"\
	{$(INCLUDE)}"ks\Map.h"\
	{$(INCLUDE)}"ks\MCDetectFrontEnd.h"\
	{$(INCLUDE)}"ks\menu.h"\
	{$(INCLUDE)}"ks\menusys.h"\
	{$(INCLUDE)}"ks\mode.h"\
	{$(INCLUDE)}"ks\mode_headtohead.h"\
	{$(INCLUDE)}"ks\mode_meterattack.h"\
	{$(INCLUDE)}"ks\mode_push.h"\
	{$(INCLUDE)}"ks\mode_timeattack.h"\
	{$(INCLUDE)}"ks\MultiFrontEnd.h"\
	{$(INCLUDE)}"ks\MusicMan.h"\
	{$(INCLUDE)}"ks\ode.h"\
	{$(INCLUDE)}"ks\osGameSaver.h"\
	{$(INCLUDE)}"ks\osparticle.h"\
	{$(INCLUDE)}"ks\PAL60FrontEnd.h"\
	{$(INCLUDE)}"ks\PathData.h"\
	{$(INCLUDE)}"ks\PhotoFrontEnd.h"\
	{$(INCLUDE)}"ks\physics.h"\
	{$(INCLUDE)}"ks\player.h"\
	{$(INCLUDE)}"ks\replay.h"\
	{$(INCLUDE)}"ks\robb_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\rumbleManager.h"\
	{$(INCLUDE)}"ks\SaveLoadFrontEnd.h"\
	{$(INCLUDE)}"ks\scoringmanager.h"\
	{$(INCLUDE)}"ks\simple_list.h"\
	{$(INCLUDE)}"ks\slater_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\sounddata.h"\
	{$(INCLUDE)}"ks\SoundScript.h"\
	{$(INCLUDE)}"ks\specialmeter.h"\
	{$(INCLUDE)}"ks\StatsFrontEnd.h"\
	{$(INCLUDE)}"ks\surferdata.h"\
	{$(INCLUDE)}"ks\SurferFrontEnd.h"\
	{$(INCLUDE)}"ks\trick_system.h"\
	{$(INCLUDE)}"ks\TrickBookFrontEnd.h"\
	{$(INCLUDE)}"ks\trickdata.h"\
	{$(INCLUDE)}"ks\trickregion.h"\
	{$(INCLUDE)}"ks\tricks.h"\
	{$(INCLUDE)}"ks\tutorialdata.h"\
	{$(INCLUDE)}"ks\TutorialFrontEnd.h"\
	{$(INCLUDE)}"ks\tutorialmanager.h"\
	{$(INCLUDE)}"ks\underwtr.h"\
	{$(INCLUDE)}"ks\unlock_manager.h"\
	{$(INCLUDE)}"ks\VOEngine.h"\
	{$(INCLUDE)}"ks\wave.h"\
	{$(INCLUDE)}"ks\wavebreak.txt"\
	{$(INCLUDE)}"ks\wavebreakstage.txt"\
	{$(INCLUDE)}"ks\waveemitter.txt"\
	{$(INCLUDE)}"ks\waveenum.h"\
	{$(INCLUDE)}"ks\wavemarker.txt"\
	{$(INCLUDE)}"ks\waveregion.txt"\
	{$(INCLUDE)}"ks\wavestage.txt"\
	{$(INCLUDE)}"libcdvd.h"\
	{$(INCLUDE)}"libdma.h"\
	{$(INCLUDE)}"libgraph.h"\
	{$(INCLUDE)}"libipu.h"\
	{$(INCLUDE)}"libmc.h"\
	{$(INCLUDE)}"libmpeg.h"\
	{$(INCLUDE)}"libpad.h"\
	{$(INCLUDE)}"libpc.h"\
	{$(INCLUDE)}"libusbkb.h"\
	{$(INCLUDE)}"libvu0.h"\
	{$(INCLUDE)}"sifdev.h"\
	{$(INCLUDE)}"xgraphics.h"\
	
NODEP_CPP_FILES_K=\
	"..\..\NSL\pc\nl_pc.h"\
	"..\..\NSL\pc\nsl_pc_ext.h"\
	".\algo.h"\
	".\defalloc.h"\
	".\gc_arammgr.h"\
	".\hwosmks\am_audio.h"\
	".\hwosmks\mks_devopts.h"\
	".\hwosmks\mks_errmsg.h"\
	".\hwosmks\mks_movieplayer.h"\
	".\hwosmks\set5_rasterize.h"\
	".\hwosmks\set5_timer.h"\
	".\hwosmks\sh4_math.h"\
	".\hwosmks\sy_alloc.h"\
	".\hwosmks\sy_file.h"\
	".\hwosnglpc\algebra.h"\
	".\hwosnglpc\pc_algebra.h"\
	".\hwosnull\null_alloc.h"\
	".\hwosnull\null_audio.h"\
	".\hwosnull\null_devopts.h"\
	".\hwosnull\null_errmsg.h"\
	".\hwosnull\null_file.h"\
	".\hwosnull\null_math.h"\
	".\hwosnull\null_movieplayer.h"\
	".\hwosnull\null_rasterize.h"\
	".\hwosnull\null_timer.h"\
	".\hwospc\d3d_rasterize.h"\
	".\hwospc\ds_audio.h"\
	".\hwospc\pc_algebra.h"\
	".\hwospc\pc_movieplayer.h"\
	".\hwospc\w32_alloc.h"\
	".\hwospc\w32_devopts.h"\
	".\hwospc\w32_errmsg.h"\
	".\hwospc\w32_file.h"\
	".\hwospc\w32_timer.h"\
	".\hwospc\x86_math.h"\
	".\hwosxb\ngl_xbox.h"\
	".\hwosxb\nvl_xbox.h"\
	".\LipSync_Anim.h"\
	".\ngl_xbox.h"\
	".\nvl_gc.h"\
	".\nvl_xbox.h"\
	".\nvlstream_gc.h"\
	".\stl_heap.h"\
	
# ADD BASE CPP /Yu
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

DEP_CPP_FILES_K=\
	"..\..\ngl\ps2\ngl_fixedstr.h"\
	"..\..\ngl\ps2\ngl_ps2.h"\
	"..\..\ngl\ps2\tim2.h"\
	"..\..\NGL\XBOX\ngl_FixedStr.h"\
	"..\..\NGL\XBOX\ngl_types.h"\
	"..\..\NGL\XBOX\ngl_xbox.h"\
	"..\..\NSL\common\nl.h"\
	"..\..\NSL\gamecube\nl_gc.h"\
	"..\..\NSL\PS2\nl_ps2.h"\
	"..\..\NSL\XBOX\nl_xbox.h"\
	".\ks\beach.cpp"\
	".\ks\beachdata.cpp"\
	".\ks\board.cpp"\
	".\ks\BoardManager.cpp"\
	".\ks\BoardManager.h"\
	".\ks\career.cpp"\
	".\ks\careerdata.cpp"\
	".\ks\challenge.cpp"\
	".\ks\challenge_icon.cpp"\
	".\ks\challenge_photo.cpp"\
	".\ks\cheat.cpp"\
	".\ks\combodata.cpp"\
	".\ks\combodata.h"\
	".\ks\compressedphoto.cpp"\
	".\ks\DemoMode.cpp"\
	".\ks\DemoMode.h"\
	".\ks\dxt1_codebook.cpp"\
	".\ks\dxt1_codebook.h"\
	".\ks\dxt1_gen.cpp"\
	".\ks\dxt1_gen.h"\
	".\ks\dxt1_imagedxt1.cpp"\
	".\ks\dxt1_imagedxt1.h"\
	".\ks\dxt1_table.h"\
	".\ks\floatobj.cpp"\
	".\ks\ik_object.cpp"\
	".\ks\judge.cpp"\
	".\ks\kellyslater_ai_goals.cpp"\
	".\ks\kellyslater_ai_goals.h"\
	".\ks\kellyslater_controller.cpp"\
	".\ks\kellyslater_main.cpp"\
	".\ks\ks_camera.cpp"\
	".\ks\ksdbmenu.cpp"\
	".\ks\ksfx.h"\
	".\ks\ksreplay.cpp"\
	".\ks\menu.cpp"\
	".\ks\menungl.cpp"\
	".\ks\menusys.cpp"\
	".\ks\MusicMan.cpp"\
	".\ks\ode.cpp"\
	".\ks\physics.cpp"\
	".\ks\player.cpp"\
	".\ks\rumbleManager.cpp"\
	".\ks\scoringmanager.cpp"\
	".\ks\SFXEngine.cpp"\
	".\ks\SFXEngine.h"\
	".\ks\sounddata.cpp"\
	".\ks\specialmeter.cpp"\
	".\ks\spline.cpp"\
	".\ks\surferdata.cpp"\
	".\ks\text_parser.cpp"\
	".\ks\text_parser.h"\
	".\ks\trail.h"\
	".\ks\trick_system.cpp"\
	".\ks\trickdata.cpp"\
	".\ks\tricks.cpp"\
	".\ks\tutorialdata.cpp"\
	".\ks\VOEngine.cpp"\
	".\ks\water.h"\
	".\ks\wavedata.h"\
	".\ks\wavesound.cpp"\
	".\ks\wavesound.h"\
	".\ks\wavetex.h"\
	".\ks\wipeoutdata.cpp"\
	".\ks\wipeoutdata.h"\
	
NODEP_CPP_FILES_K=\
	"..\..\ngl\common\ngl.h"\
	"..\..\ngl\common\types.h"\
	"..\..\NSL\pc\nl_pc.h"\
	".\ekernel.h"\
	".\eregs.h"\
	".\estruct.h"\
	".\etypes.h"\
	".\ibdma.h"\
	".\ibgraph.h"\
	".\ibvu0.h"\
	".\ifdev.h"\
	".\ks\ai_actions.h"\
	".\ks\ai_goals.h"\
	".\ks\ai_interface.h"\
	".\ks\ai_locomotion.h"\
	".\ks\algebra.h"\
	".\ks\animation_interface.h"\
	".\ks\app.h"\
	".\ks\billboard.h"\
	".\ks\blendmodes.h"\
	".\ks\camera.h"\
	".\ks\colgeom.h"\
	".\ks\collide.h"\
	".\ks\colmesh.h"\
	".\ks\color.h"\
	".\ks\commands.h"\
	".\ks\conglom.h"\
	".\ks\console.h"\
	".\ks\controller.h"\
	".\ks\debug_render.h"\
	".\ks\entity.h"\
	".\ks\entity_anim.h"\
	".\ks\entity_maker.h"\
	".\ks\entityflags.h"\
	".\ks\file_finder.h"\
	".\ks\fogmgr.h"\
	".\ks\game.h"\
	".\ks\game_info.h"\
	".\ks\game_process.h"\
	".\ks\geomgr.h"\
	".\ks\global.h"\
	".\ks\hwaudio.h"\
	".\ks\hwmath.h"\
	".\ks\hwosgc\gc_GameSaver.h"\
	".\ks\hwosgc\gc_input.h"\
	".\ks\hwosps2\ps2_GameSaver.h"\
	".\ks\hwosps2\ps2_input.h"\
	".\ks\hwosps2\ps2_m2vplayer.h"\
	".\ks\HWOSXB\xb_algebra.h"\
	".\ks\hwosxb\xb_GameSaver.h"\
	".\ks\hwosxb\xb_input.h"\
	".\ks\hwosxb\xb_particle.h"\
	".\ks\hwrasterize.h"\
	".\ks\ini_parser.h"\
	".\ks\inputmgr.h"\
	".\ks\interface.h"\
	".\ks\item.h"\
	".\ks\joystick.h"\
	".\ks\keyboard.h"\
	".\ks\kshooks.h"\
	".\ks\ksnsl.h"\
	".\ks\ksnvl.h"\
	".\ks\lightmgr.h"\
	".\ks\matfac.h"\
	".\ks\menusound.h"\
	".\ks\mouse.h"\
	".\ks\msgboard.h"\
	".\ks\mustash.h"\
	".\ks\osdevopts.h"\
	".\ks\physical_interface.h"\
	".\ks\po.h"\
	".\ks\profiler.h"\
	".\ks\projconst.h"\
	".\ks\random.h"\
	".\ks\refptr.h"\
	".\ks\scene_anim.h"\
	".\ks\script_access.h"\
	".\ks\singleton.h"\
	".\ks\stringx.h"\
	".\ks\switch_obj.h"\
	".\ks\terrain.h"\
	".\ks\text_font.h"\
	".\ks\time_interface.h"\
	".\ks\timer.h"\
	".\ks\wds.h"\
	".\ks\widget.h"\
	".\s\AccompFrontEnd.h"\
	".\s\andersen_ind_anims_mac.h"\
	".\s\beach.h"\
	".\s\beachdata.h"\
	".\s\BeachFrontEnd.h"\
	".\s\blur.h"\
	".\s\board.h"\
	".\s\board_anims_mac.h"\
	".\s\boarddata.h"\
	".\s\BoardFrontEnd.h"\
	".\s\camera_tool.h"\
	".\s\career.h"\
	".\s\careerdata.h"\
	".\s\carrol_ind_anims_mac.h"\
	".\s\challenge.h"\
	".\s\challenge_icon.h"\
	".\s\challenge_photo.h"\
	".\s\cheat.h"\
	".\s\CheatFrontEnd.h"\
	".\s\compressedphoto.h"\
	".\s\curren_ind_anims_mac.h"\
	".\s\eventmanager.h"\
	".\s\ExtrasFrontEnd.h"\
	".\s\FEAnim.h"\
	".\s\FEEntityManager.h"\
	".\s\FEMenu.h"\
	".\s\FEPanel.h"\
	".\s\fletcher_ind_anims_mac.h"\
	".\s\floatobj.h"\
	".\s\frankenreiter_ind_anims_mac.h"\
	".\s\FrontEndManager.h"\
	".\s\FrontEndMenus.h"\
	".\s\GameData.h"\
	".\s\GlobalData.h"\
	".\s\globaltextenum.h"\
	".\s\GraphicalMenuSystem.h"\
	".\s\HighScoreFrontEnd.h"\
	".\s\igo_widget.h"\
	".\s\igo_widget_analogclock.h"\
	".\s\igo_widget_balance.h"\
	".\s\igo_widget_camera.h"\
	".\s\igo_widget_fanmeter.h"\
	".\s\igo_widget_iconradar.h"\
	".\s\igo_widget_photo.h"\
	".\s\igo_widget_replay.h"\
	".\s\igo_widget_simple.h"\
	".\s\igo_widget_specialmeter.h"\
	".\s\igo_widget_splitclock.h"\
	".\s\igo_widget_splitmeter.h"\
	".\s\igo_widget_splitscore.h"\
	".\s\igo_widget_splitter.h"\
	".\s\igo_widget_timeattack.h"\
	".\s\igo_widget_waveindicator.h"\
	".\s\IGOFrontEnd.h"\
	".\s\igohintmanager.h"\
	".\s\igoiconmanager.h"\
	".\s\igolearn_new_trickmanager.h"\
	".\s\ik_object.h"\
	".\s\irons_ind_anims_mac.h"\
	".\s\judge.h"\
	".\s\kellyslater_controller.h"\
	".\s\kellyslater_shared_anims_mac.h"\
	".\s\kellyslater_states_mac.h"\
	".\s\ks_camera.h"\
	".\s\ksdbmenu.h"\
	".\s\ksngl.h"\
	".\s\ksreplay.h"\
	".\s\machado_ind_anims_mac.h"\
	".\s\MainFrontEnd.h"\
	".\s\Map.h"\
	".\s\MCDetectFrontEnd.h"\
	".\s\menu.h"\
	".\s\menusys.h"\
	".\s\MultiFrontEnd.h"\
	".\s\MusicMan.h"\
	".\s\ode.h"\
	".\s\osGameSaver.h"\
	".\s\osparticle.h"\
	".\s\PAL60FrontEnd.h"\
	".\s\PathData.h"\
	".\s\PhotoFrontEnd.h"\
	".\s\physics.h"\
	".\s\player.h"\
	".\s\replay.h"\
	".\s\robb_ind_anims_mac.h"\
	".\s\rumbleManager.h"\
	".\s\SaveLoadFrontEnd.h"\
	".\s\scoringmanager.h"\
	".\s\simple_list.h"\
	".\s\slater_ind_anims_mac.h"\
	".\s\sounddata.h"\
	".\s\SoundScript.h"\
	".\s\specialmeter.h"\
	".\s\StatsFrontEnd.h"\
	".\s\surferdata.h"\
	".\s\SurferFrontEnd.h"\
	".\s\trick_system.h"\
	".\s\TrickBookFrontEnd.h"\
	".\s\trickdata.h"\
	".\s\trickregion.h"\
	".\s\tricks.h"\
	".\s\tutorialdata.h"\
	".\s\TutorialFrontEnd.h"\
	".\s\tutorialmanager.h"\
	".\s\VOEngine.h"\
	".\s\wave.h"\
	".\s\wavebreak.txt"\
	".\s\wavebreakstage.txt"\
	".\s\waveemitter.txt"\
	".\s\waveenum.h"\
	".\s\wavemarker.txt"\
	".\s\waveregion.txt"\
	".\s\wavestage.txt"\
	
# SUBTRACT BASE CPP /YX /Yc /Yu
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

DEP_CPP_FILES_K=\
	"..\..\ngl\ps2\ngl_fixedstr.h"\
	"..\..\ngl\ps2\ngl_ps2.h"\
	"..\..\ngl\ps2\tim2.h"\
	"..\..\NSL\common\nl.h"\
	"..\..\NSL\gamecube\nl_gc.h"\
	"..\..\NSL\PS2\nl_ps2.h"\
	"..\..\NSL\XBOX\nl_xbox.h"\
	".\ks\beach.cpp"\
	".\ks\beachdata.cpp"\
	".\ks\board.cpp"\
	".\ks\BoardManager.cpp"\
	".\ks\BoardManager.h"\
	".\ks\career.cpp"\
	".\ks\careerdata.cpp"\
	".\ks\challenge.cpp"\
	".\ks\challenge_icon.cpp"\
	".\ks\challenge_photo.cpp"\
	".\ks\cheat.cpp"\
	".\ks\combodata.cpp"\
	".\ks\combodata.h"\
	".\ks\compressedphoto.cpp"\
	".\ks\DemoMode.cpp"\
	".\ks\DemoMode.h"\
	".\ks\dxt1_codebook.cpp"\
	".\ks\dxt1_codebook.h"\
	".\ks\dxt1_gen.cpp"\
	".\ks\dxt1_gen.h"\
	".\ks\dxt1_imagedxt1.cpp"\
	".\ks\dxt1_imagedxt1.h"\
	".\ks\dxt1_table.h"\
	".\ks\floatobj.cpp"\
	".\ks\ik_object.cpp"\
	".\ks\judge.cpp"\
	".\ks\kellyslater_ai_goals.cpp"\
	".\ks\kellyslater_ai_goals.h"\
	".\ks\kellyslater_controller.cpp"\
	".\ks\kellyslater_main.cpp"\
	".\ks\ks_camera.cpp"\
	".\ks\ksdbmenu.cpp"\
	".\ks\ksfx.h"\
	".\ks\ksreplay.cpp"\
	".\ks\menu.cpp"\
	".\ks\menungl.cpp"\
	".\ks\menusys.cpp"\
	".\ks\MusicMan.cpp"\
	".\ks\ode.cpp"\
	".\ks\physics.cpp"\
	".\ks\player.cpp"\
	".\ks\rumbleManager.cpp"\
	".\ks\scoringmanager.cpp"\
	".\ks\SFXEngine.cpp"\
	".\ks\SFXEngine.h"\
	".\ks\sounddata.cpp"\
	".\ks\specialmeter.cpp"\
	".\ks\spline.cpp"\
	".\ks\surferdata.cpp"\
	".\ks\text_parser.cpp"\
	".\ks\text_parser.h"\
	".\ks\trail.h"\
	".\ks\trick_system.cpp"\
	".\ks\trickdata.cpp"\
	".\ks\tricks.cpp"\
	".\ks\tutorialdata.cpp"\
	".\ks\VOEngine.cpp"\
	".\ks\water.h"\
	".\ks\wavedata.h"\
	".\ks\wavesound.cpp"\
	".\ks\wavesound.h"\
	".\ks\wavetex.h"\
	".\ks\wipeoutdata.cpp"\
	".\ks\wipeoutdata.h"\
	
NODEP_CPP_FILES_K=\
	"..\..\NSL\pc\nl_pc.h"\
	".\ekernel.h"\
	".\eregs.h"\
	".\estruct.h"\
	".\etypes.h"\
	".\ibdma.h"\
	".\ibgraph.h"\
	".\ibvu0.h"\
	".\ifdev.h"\
	".\ks\ai_actions.h"\
	".\ks\ai_goals.h"\
	".\ks\ai_interface.h"\
	".\ks\ai_locomotion.h"\
	".\ks\algebra.h"\
	".\ks\animation_interface.h"\
	".\ks\app.h"\
	".\ks\billboard.h"\
	".\ks\blendmodes.h"\
	".\ks\camera.h"\
	".\ks\colgeom.h"\
	".\ks\collide.h"\
	".\ks\colmesh.h"\
	".\ks\color.h"\
	".\ks\commands.h"\
	".\ks\conglom.h"\
	".\ks\console.h"\
	".\ks\controller.h"\
	".\ks\debug_render.h"\
	".\ks\entity.h"\
	".\ks\entity_anim.h"\
	".\ks\entity_maker.h"\
	".\ks\entityflags.h"\
	".\ks\file_finder.h"\
	".\ks\fogmgr.h"\
	".\ks\game.h"\
	".\ks\game_info.h"\
	".\ks\game_process.h"\
	".\ks\geomgr.h"\
	".\ks\global.h"\
	".\ks\hwaudio.h"\
	".\ks\hwmath.h"\
	".\ks\hwosgc\gc_GameSaver.h"\
	".\ks\hwosgc\gc_input.h"\
	".\ks\hwosps2\ps2_GameSaver.h"\
	".\ks\hwosps2\ps2_input.h"\
	".\ks\hwosps2\ps2_m2vplayer.h"\
	".\ks\HWOSXB\xb_algebra.h"\
	".\ks\hwosxb\xb_GameSaver.h"\
	".\ks\hwosxb\xb_input.h"\
	".\ks\hwosxb\xb_particle.h"\
	".\ks\hwrasterize.h"\
	".\ks\ini_parser.h"\
	".\ks\inputmgr.h"\
	".\ks\interface.h"\
	".\ks\item.h"\
	".\ks\joystick.h"\
	".\ks\keyboard.h"\
	".\ks\kshooks.h"\
	".\ks\ksnsl.h"\
	".\ks\ksnvl.h"\
	".\ks\lightmgr.h"\
	".\ks\matfac.h"\
	".\ks\menusound.h"\
	".\ks\mouse.h"\
	".\ks\msgboard.h"\
	".\ks\mustash.h"\
	".\ks\ngl.h"\
	".\ks\osdevopts.h"\
	".\ks\physical_interface.h"\
	".\ks\po.h"\
	".\ks\profiler.h"\
	".\ks\projconst.h"\
	".\ks\random.h"\
	".\ks\refptr.h"\
	".\ks\scene_anim.h"\
	".\ks\script_access.h"\
	".\ks\singleton.h"\
	".\ks\stringx.h"\
	".\ks\switch_obj.h"\
	".\ks\terrain.h"\
	".\ks\text_font.h"\
	".\ks\time_interface.h"\
	".\ks\timer.h"\
	".\ks\types.h"\
	".\ks\wds.h"\
	".\ks\widget.h"\
	".\s\AccompFrontEnd.h"\
	".\s\andersen_ind_anims_mac.h"\
	".\s\beach.h"\
	".\s\beachdata.h"\
	".\s\BeachFrontEnd.h"\
	".\s\blur.h"\
	".\s\board.h"\
	".\s\board_anims_mac.h"\
	".\s\boarddata.h"\
	".\s\BoardFrontEnd.h"\
	".\s\camera_tool.h"\
	".\s\career.h"\
	".\s\careerdata.h"\
	".\s\carrol_ind_anims_mac.h"\
	".\s\challenge.h"\
	".\s\challenge_icon.h"\
	".\s\challenge_photo.h"\
	".\s\cheat.h"\
	".\s\CheatFrontEnd.h"\
	".\s\compressedphoto.h"\
	".\s\curren_ind_anims_mac.h"\
	".\s\eventmanager.h"\
	".\s\ExtrasFrontEnd.h"\
	".\s\FEAnim.h"\
	".\s\FEEntityManager.h"\
	".\s\FEMenu.h"\
	".\s\FEPanel.h"\
	".\s\fletcher_ind_anims_mac.h"\
	".\s\floatobj.h"\
	".\s\frankenreiter_ind_anims_mac.h"\
	".\s\FrontEndManager.h"\
	".\s\FrontEndMenus.h"\
	".\s\GameData.h"\
	".\s\GlobalData.h"\
	".\s\globaltextenum.h"\
	".\s\GraphicalMenuSystem.h"\
	".\s\HighScoreFrontEnd.h"\
	".\s\igo_widget.h"\
	".\s\igo_widget_analogclock.h"\
	".\s\igo_widget_balance.h"\
	".\s\igo_widget_camera.h"\
	".\s\igo_widget_fanmeter.h"\
	".\s\igo_widget_iconcount.h"\
	".\s\igo_widget_iconradar.h"\
	".\s\igo_widget_meterchallenge.h"\
	".\s\igo_widget_objectalert.h"\
	".\s\igo_widget_photo.h"\
	".\s\igo_widget_replay.h"\
	".\s\igo_widget_simple.h"\
	".\s\igo_widget_skillchallenge.h"\
	".\s\igo_widget_specialmeter.h"\
	".\s\igo_widget_splitclock.h"\
	".\s\igo_widget_splitmeter.h"\
	".\s\igo_widget_splitscore.h"\
	".\s\igo_widget_splitter.h"\
	".\s\igo_widget_timeattack.h"\
	".\s\igo_widget_waveindicator.h"\
	".\s\IGOFrontEnd.h"\
	".\s\igohintmanager.h"\
	".\s\igoiconmanager.h"\
	".\s\igolearn_new_trickmanager.h"\
	".\s\ik_object.h"\
	".\s\irons_ind_anims_mac.h"\
	".\s\judge.h"\
	".\s\kellyslater_controller.h"\
	".\s\kellyslater_shared_anims_mac.h"\
	".\s\kellyslater_states_mac.h"\
	".\s\ks_camera.h"\
	".\s\ksdbmenu.h"\
	".\s\ksngl.h"\
	".\s\ksreplay.h"\
	".\s\LogbookFrontEnd.h"\
	".\s\machado_ind_anims_mac.h"\
	".\s\MainFrontEnd.h"\
	".\s\Map.h"\
	".\s\MCDetectFrontEnd.h"\
	".\s\menu.h"\
	".\s\menusys.h"\
	".\s\MultiFrontEnd.h"\
	".\s\MusicMan.h"\
	".\s\ode.h"\
	".\s\osGameSaver.h"\
	".\s\osparticle.h"\
	".\s\PAL60FrontEnd.h"\
	".\s\PathData.h"\
	".\s\PhotoFrontEnd.h"\
	".\s\physics.h"\
	".\s\player.h"\
	".\s\replay.h"\
	".\s\robb_ind_anims_mac.h"\
	".\s\rumbleManager.h"\
	".\s\SaveLoadFrontEnd.h"\
	".\s\scoringmanager.h"\
	".\s\simple_list.h"\
	".\s\slater_ind_anims_mac.h"\
	".\s\sounddata.h"\
	".\s\SoundScript.h"\
	".\s\specialmeter.h"\
	".\s\StatsFrontEnd.h"\
	".\s\surferdata.h"\
	".\s\SurferFrontEnd.h"\
	".\s\trick_system.h"\
	".\s\TrickBookFrontEnd.h"\
	".\s\trickdata.h"\
	".\s\trickregion.h"\
	".\s\tricks.h"\
	".\s\tutorialdata.h"\
	".\s\TutorialFrontEnd.h"\
	".\s\tutorialmanager.h"\
	".\s\VOEngine.h"\
	".\s\wave.h"\
	".\s\wavebreak.txt"\
	".\s\wavebreakstage.txt"\
	".\s\waveemitter.txt"\
	".\s\waveenum.h"\
	".\s\wavemarker.txt"\
	".\s\waveregion.txt"\
	".\s\wavestage.txt"\
	
# SUBTRACT BASE CPP /YX /Yc /Yu
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\files_misc1.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

DEP_CPP_FILES_M=\
	"..\..\NGL\ps2\libsn.h"\
	"..\..\ngl\ps2\matrix.h"\
	"..\..\ngl\ps2\matrix_common.h"\
	"..\..\ngl\ps2\ngl_dma.h"\
	"..\..\ngl\ps2\ngl_fixedstr.h"\
	"..\..\ngl\ps2\ngl_instbank.h"\
	"..\..\ngl\ps2\ngl_ps2.h"\
	"..\..\ngl\ps2\ngl_ps2_internal.h"\
	"..\..\NGL\ps2\ngl_vudefs.h"\
	"..\..\ngl\ps2\tim2.h"\
	"..\..\ngl\ps2\vector.h"\
	"..\..\ngl\ps2\vector_common.h"\
	"..\..\NSL\common\nl.h"\
	"..\..\NSL\common\nsl.h"\
	"..\..\NSL\common\ProjectOptions.h"\
	"..\..\NSL\gamecube\nl_gc.h"\
	"..\..\NSL\ps2\fifo_queue.cpp"\
	"..\..\NSL\ps2\fifo_queue.h"\
	"..\..\NSL\PS2\gas.h"\
	"..\..\NSL\PS2\gas_utility.h"\
	"..\..\NSL\PS2\nl_ps2.h"\
	"..\..\NSL\PS2\nsl_ps2.h"\
	"..\..\NSL\XBOX\nl_xbox.h"\
	"..\..\NSL\XBOX\nsl_xbox.h"\
	"..\..\NSL\XBOX\nsl_xbox_ext.h"\
	"..\..\NSL\XBOX\nsl_xbox_source.h"\
	"..\..\nsl\xbox\nsl_xbox_streams.h"\
	"..\..\NVL\ps2\nvl_ps2.h"\
	"..\..\NVL\ps2\nvlMPEG_ps2.h"\
	"..\..\NVL\ps2\nvlstream_ps2.h"\
	".\aggvertbuf.h"\
	".\ai_constants.h"\
	".\ai_interface.h"\
	".\ai_locomotion.h"\
	".\ai_polypath.h"\
	".\ai_polypath_cell.h"\
	".\ai_polypath_heap.h"\
	".\algebra.h"\
	".\anim.h"\
	".\anim_flavor.h"\
	".\anim_flavors.h"\
	".\anim_ids.h"\
	".\anim_maker.h"\
	".\app.h"\
	".\archalloc.cpp"\
	".\archalloc.h"\
	".\attrib.cpp"\
	".\avltree.h"\
	".\b_spline.h"\
	".\beam.cpp"\
	".\beam.h"\
	".\beam_signals.h"\
	".\billboard.h"\
	".\bone.h"\
	".\bound.h"\
	".\box_trigger_interface.h"\
	".\bsp_collide.h"\
	".\camera.h"\
	".\capsule.h"\
	".\cface.h"\
	".\chunkfile.h"\
	".\colgeom.h"\
	".\collide.h"\
	".\colmesh.h"\
	".\color.h"\
	".\commands.h"\
	".\conglom.h"\
	".\console.h"\
	".\consoleCmds.h"\
	".\consoleVars.h"\
	".\constants.h"\
	".\controller.cpp"\
	".\controller.h"\
	".\convex_box.h"\
	".\crawl_box.h"\
	".\custom_stl.h"\
	".\debug.h"\
	".\debug_render.h"\
	".\devoptflags.h"\
	".\devoptints.h"\
	".\devoptstrs.h"\
	".\element.h"\
	".\entflavor.h"\
	".\entity.h"\
	".\entity_anim.h"\
	".\entity_interface.h"\
	".\entity_maker.h"\
	".\entity_signals.h"\
	".\entityflags.h"\
	".\entityid.h"\
	".\errorcontext.h"\
	".\face.h"\
	".\faceflags.h"\
	".\fast_vector.h"\
	".\fcs.h"\
	".\file.h"\
	".\file_finder.cpp"\
	".\file_finder.h"\
	".\file_manager.h"\
	".\filespec.h"\
	".\fogmgr.h"\
	".\forceflags.h"\
	".\frame_info.h"\
	".\frustum.h"\
	".\game.cpp"\
	".\game.h"\
	".\game_info.h"\
	".\game_info_vars.h"\
	".\game_process.cpp"\
	".\game_process.h"\
	".\gamefile.cpp"\
	".\gamefile.h"\
	".\gcglobals.h"\
	".\generator.h"\
	".\geomgr.h"\
	".\global.h"\
	".\global_signals.h"\
	".\graph.h"\
	".\guidance_sys.cpp"\
	".\guidance_sys.h"\
	".\hard_attribs.h"\
	".\heap.cpp"\
	".\heap.h"\
	".\heaptype.h"\
	".\hull.h"\
	".\hwaudio.h"\
	".\hwmath.h"\
	".\hwmovieplayer.h"\
	".\hwosgc\gc_archalloc.cpp"\
	".\HWOSPS2\ps2_archalloc.cpp"\
	".\hwosxb\xb_archalloc.cpp"\
	".\hwrasterize.h"\
	".\hyperplane.h"\
	".\ini_parser.h"\
	".\inputmgr.h"\
	".\instance.h"\
	".\interface.cpp"\
	".\interface.h"\
	".\iri.h"\
	".\item.cpp"\
	".\item.h"\
	".\item_signals.h"\
	".\joystick.h"\
	".\keyboard.h"\
	".\ks\BoardManager.h"\
	".\ks\DemoMode.h"\
	".\ks\displace.h"\
	".\ks\kellyslater_main.h"\
	".\ks\ksfx.h"\
	".\ks\ksheaps.h"\
	".\ks\mode_headtohead.cpp"\
	".\ks\mode_meterattack.cpp"\
	".\ks\mode_push.cpp"\
	".\ks\mode_timeattack.cpp"\
	".\ks\refract.h"\
	".\ks\SFXEngine.h"\
	".\ks\water.h"\
	".\ks\wavedata.h"\
	".\ks\wavesound.h"\
	".\ks\wavetex.h"\
	".\kshooks.h"\
	".\ksnsl.h"\
	".\ksnvl.h"\
	".\lensflare.h"\
	".\light.h"\
	".\lightmgr.h"\
	".\linear_anim.h"\
	".\link_interface.h"\
	".\localize.h"\
	".\map_e.h"\
	".\marker.cpp"\
	".\marker.h"\
	".\material.h"\
	".\matfac.h"\
	".\maxskinbones.h"\
	".\mcs.h"\
	".\meshrefs.h"\
	".\mic.cpp"\
	".\mic.h"\
	".\mobject.h"\
	".\mouse.h"\
	".\msgboard.h"\
	".\mustash.h"\
	".\ngl.h"\
	".\ngl_support.h"\
	".\osalloc.h"\
	".\osassert.h"\
	".\osdevopts.h"\
	".\oserrmsg.h"\
	".\osfile.h"\
	".\ostimer.h"\
	".\particle.h"\
	".\particlecleaner.h"\
	".\path.h"\
	".\pc_port.h"\
	".\physical_interface.h"\
	".\plane.h"\
	".\platform_defines.h"\
	".\pmesh.h"\
	".\po.h"\
	".\po_anim.h"\
	".\polytube.h"\
	".\portal.h"\
	".\profcounters.cpp"\
	".\profiler.h"\
	".\proftimers.cpp"\
	".\projconst.h"\
	".\project.h"\
	".\pstring.h"\
	".\random.h"\
	".\rect.h"\
	".\refptr.h"\
	".\region.h"\
	".\region_graph.h"\
	".\render_data.h"\
	".\renderflav.h"\
	".\scene_anim.h"\
	".\script_access.h"\
	".\script_controller_signals.h"\
	".\script_lib_controller.h"\
	".\script_lib_mfg.h"\
	".\script_library_class.h"\
	".\script_mfg_signals.h"\
	".\script_object.h"\
	".\semaphores.h"\
	".\serialin.cpp"\
	".\signal_anim.h"\
	".\signals.h"\
	".\SimpleAssert.h"\
	".\singleton.h"\
	".\sky.cpp"\
	".\sky.h"\
	".\so_data_block.h"\
	".\soft_attribs.h"\
	".\sound_group.cpp"\
	".\sound_group.h"\
	".\sound_interface.h"\
	".\sphere.h"\
	".\stashes.h"\
	".\staticmem.h"\
	".\stl_adapter.h"\
	".\stringx.h"\
	".\switch_obj.h"\
	".\switch_obj_signals.h"\
	".\terrain.h"\
	".\text_font.h"\
	".\textfile.h"\
	".\time_interface.h"\
	".\timer.h"\
	".\tool_dialogs.cpp"\
	".\tool_dialogs.h"\
	".\trigger.cpp"\
	".\trigger.h"\
	".\trigger_signals.h"\
	".\txtcoord.h"\
	".\types.h"\
	".\usefulmath.h"\
	".\users.h"\
	".\vert.h"\
	".\visrep.h"\
	".\vm_executable.h"\
	".\vm_stack.h"\
	".\vm_symbol.h"\
	".\vm_symbol_list.h"\
	".\vm_thread.h"\
	".\vsplit.h"\
	".\warnlvl.h"\
	".\wds.cpp"\
	".\wds.h"\
	".\wedge.h"\
	".\widget.h"\
	".\widget_entity.h"\
	".\widget_script.h"\
	".\xbglobals.h"\
	{$(INCLUDE)}"eekernel.h"\
	{$(INCLUDE)}"eeregs.h"\
	{$(INCLUDE)}"eestruct.h"\
	{$(INCLUDE)}"eetypes.h"\
	{$(INCLUDE)}"hwosgc\gc_algebra.h"\
	{$(INCLUDE)}"hwosgc\gc_alloc.h"\
	{$(INCLUDE)}"hwosgc\gc_audio.h"\
	{$(INCLUDE)}"hwosgc\gc_errmsg.h"\
	{$(INCLUDE)}"hwosgc\gc_file.h"\
	{$(INCLUDE)}"HWOSGC\gc_gamesaver.h"\
	{$(INCLUDE)}"hwosgc\gc_graphics.h"\
	{$(INCLUDE)}"hwosgc\gc_input.h"\
	{$(INCLUDE)}"hwosgc\gc_math.h"\
	{$(INCLUDE)}"hwosgc\gc_movieplayer.h"\
	{$(INCLUDE)}"hwosgc\gc_rasterize.h"\
	{$(INCLUDE)}"hwosgc\gc_texturemgr.h"\
	{$(INCLUDE)}"hwosgc\gc_timer.h"\
	{$(INCLUDE)}"hwosgc\ngl_fixedstr.h"\
	{$(INCLUDE)}"hwosgc\ngl_gc.h"\
	{$(INCLUDE)}"hwosgc\ngl_version.h"\
	{$(INCLUDE)}"hwosgc\nvl_gc.h"\
	{$(INCLUDE)}"HWOSPS2\gas.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_algebra.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_alloc.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_audio.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_devopts.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_errmsg.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_file.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_gamesaver.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_graphics.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_input.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_m2vplayer.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_math.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_movieplayer.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_rasterize.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_texturemgr.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_timer.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_types.h"\
	{$(INCLUDE)}"hwosxb\pstring.h"\
	{$(INCLUDE)}"hwosxb\xb_algebra.h"\
	{$(INCLUDE)}"hwosxb\xb_alloc.h"\
	{$(INCLUDE)}"hwosxb\xb_audio.h"\
	{$(INCLUDE)}"hwosxb\xb_errmsg.h"\
	{$(INCLUDE)}"hwosxb\xb_file.h"\
	{$(INCLUDE)}"HWOSXB\xb_gamesaver.h"\
	{$(INCLUDE)}"hwosxb\xb_graphics.h"\
	{$(INCLUDE)}"hwosxb\xb_input.h"\
	{$(INCLUDE)}"hwosxb\xb_math.h"\
	{$(INCLUDE)}"hwosxb\xb_movieplayer.h"\
	{$(INCLUDE)}"HWOSXB\xb_particle.h"\
	{$(INCLUDE)}"HWOSXB\xb_portDVDCache.h"\
	{$(INCLUDE)}"hwosxb\xb_rasterize.h"\
	{$(INCLUDE)}"hwosxb\xb_string.h"\
	{$(INCLUDE)}"hwosxb\xb_texturemgr.h"\
	{$(INCLUDE)}"hwosxb\xb_timer.h"\
	{$(INCLUDE)}"hwosxb\xb_types.h"\
	{$(INCLUDE)}"ks\AccompFrontEnd.h"\
	{$(INCLUDE)}"ks\andersen_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\beach.h"\
	{$(INCLUDE)}"ks\beachdata.h"\
	{$(INCLUDE)}"ks\BeachFrontEnd.h"\
	{$(INCLUDE)}"ks\blur.h"\
	{$(INCLUDE)}"ks\board.h"\
	{$(INCLUDE)}"ks\board_anims_mac.h"\
	{$(INCLUDE)}"ks\boarddata.h"\
	{$(INCLUDE)}"ks\BoardFrontEnd.h"\
	{$(INCLUDE)}"ks\camera_tool.h"\
	{$(INCLUDE)}"ks\career.h"\
	{$(INCLUDE)}"ks\careerdata.h"\
	{$(INCLUDE)}"ks\carrol_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\challenge.h"\
	{$(INCLUDE)}"ks\challenge_icon.h"\
	{$(INCLUDE)}"ks\challenge_photo.h"\
	{$(INCLUDE)}"ks\cheat.h"\
	{$(INCLUDE)}"ks\CheatFrontEnd.h"\
	{$(INCLUDE)}"ks\compressedphoto.h"\
	{$(INCLUDE)}"ks\coords.h"\
	{$(INCLUDE)}"ks\curren_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\eventmanager.h"\
	{$(INCLUDE)}"ks\ExtrasFrontEnd.h"\
	{$(INCLUDE)}"ks\FEAnim.h"\
	{$(INCLUDE)}"ks\FEEntityManager.h"\
	{$(INCLUDE)}"ks\FEMenu.h"\
	{$(INCLUDE)}"ks\FEPanel.h"\
	{$(INCLUDE)}"ks\fletcher_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\floatobj.h"\
	{$(INCLUDE)}"ks\frankenreiter_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\FrontEndManager.h"\
	{$(INCLUDE)}"ks\FrontEndMenus.h"\
	{$(INCLUDE)}"ks\GameData.h"\
	{$(INCLUDE)}"ks\GlobalData.h"\
	{$(INCLUDE)}"ks\globaltextenum.h"\
	{$(INCLUDE)}"ks\GraphicalMenuSystem.h"\
	{$(INCLUDE)}"ks\HighScoreFrontEnd.h"\
	{$(INCLUDE)}"ks\igo_widget.h"\
	{$(INCLUDE)}"ks\igo_widget_analogclock.h"\
	{$(INCLUDE)}"ks\igo_widget_balance.h"\
	{$(INCLUDE)}"ks\igo_widget_camera.h"\
	{$(INCLUDE)}"ks\igo_widget_fanmeter.h"\
	{$(INCLUDE)}"ks\igo_widget_grid.h"\
	{$(INCLUDE)}"ks\igo_widget_iconcount.h"\
	{$(INCLUDE)}"ks\igo_widget_iconradar.h"\
	{$(INCLUDE)}"ks\igo_widget_meterchallenge.h"\
	{$(INCLUDE)}"ks\igo_widget_objectalert.h"\
	{$(INCLUDE)}"ks\igo_widget_photo.h"\
	{$(INCLUDE)}"ks\igo_widget_replay.h"\
	{$(INCLUDE)}"ks\igo_widget_simple.h"\
	{$(INCLUDE)}"ks\igo_widget_skillchallenge.h"\
	{$(INCLUDE)}"ks\igo_widget_specialmeter.h"\
	{$(INCLUDE)}"ks\igo_widget_splitclock.h"\
	{$(INCLUDE)}"ks\igo_widget_splitmeter.h"\
	{$(INCLUDE)}"ks\igo_widget_splitscore.h"\
	{$(INCLUDE)}"ks\igo_widget_splitter.h"\
	{$(INCLUDE)}"ks\igo_widget_timeattack.h"\
	{$(INCLUDE)}"ks\igo_widget_waveindicator.h"\
	{$(INCLUDE)}"ks\IGOFrontEnd.h"\
	{$(INCLUDE)}"ks\igohintmanager.h"\
	{$(INCLUDE)}"ks\igoiconmanager.h"\
	{$(INCLUDE)}"ks\igolearn_new_trickmanager.h"\
	{$(INCLUDE)}"ks\ik_object.h"\
	{$(INCLUDE)}"ks\irons_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\judge.h"\
	{$(INCLUDE)}"ks\kellyslater_controller.h"\
	{$(INCLUDE)}"ks\kellyslater_shared_anims_mac.h"\
	{$(INCLUDE)}"ks\kellyslater_states_mac.h"\
	{$(INCLUDE)}"ks\ks_camera.h"\
	{$(INCLUDE)}"ks\ksdbmenu.h"\
	{$(INCLUDE)}"ks\ksngl.h"\
	{$(INCLUDE)}"ks\ksreplay.h"\
	{$(INCLUDE)}"ks\LogbookFrontEnd.h"\
	{$(INCLUDE)}"ks\machado_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\MainFrontEnd.h"\
	{$(INCLUDE)}"ks\Map.h"\
	{$(INCLUDE)}"ks\MCDetectFrontEnd.h"\
	{$(INCLUDE)}"ks\menu.h"\
	{$(INCLUDE)}"ks\menusys.h"\
	{$(INCLUDE)}"ks\mode.h"\
	{$(INCLUDE)}"ks\mode_headtohead.h"\
	{$(INCLUDE)}"ks\mode_meterattack.h"\
	{$(INCLUDE)}"ks\mode_push.h"\
	{$(INCLUDE)}"ks\mode_timeattack.h"\
	{$(INCLUDE)}"ks\MultiFrontEnd.h"\
	{$(INCLUDE)}"ks\MusicMan.h"\
	{$(INCLUDE)}"ks\ode.h"\
	{$(INCLUDE)}"ks\osGameSaver.h"\
	{$(INCLUDE)}"ks\osparticle.h"\
	{$(INCLUDE)}"ks\PAL60FrontEnd.h"\
	{$(INCLUDE)}"ks\PathData.h"\
	{$(INCLUDE)}"ks\PhotoFrontEnd.h"\
	{$(INCLUDE)}"ks\physics.h"\
	{$(INCLUDE)}"ks\player.h"\
	{$(INCLUDE)}"ks\replay.h"\
	{$(INCLUDE)}"ks\robb_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\rumbleManager.h"\
	{$(INCLUDE)}"ks\SaveLoadFrontEnd.h"\
	{$(INCLUDE)}"ks\scoringmanager.h"\
	{$(INCLUDE)}"ks\simple_list.h"\
	{$(INCLUDE)}"ks\slater_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\sounddata.h"\
	{$(INCLUDE)}"ks\SoundScript.h"\
	{$(INCLUDE)}"ks\specialmeter.h"\
	{$(INCLUDE)}"ks\StatsFrontEnd.h"\
	{$(INCLUDE)}"ks\surferdata.h"\
	{$(INCLUDE)}"ks\SurferFrontEnd.h"\
	{$(INCLUDE)}"ks\trick_system.h"\
	{$(INCLUDE)}"ks\TrickBookFrontEnd.h"\
	{$(INCLUDE)}"ks\trickdata.h"\
	{$(INCLUDE)}"ks\trickregion.h"\
	{$(INCLUDE)}"ks\tricks.h"\
	{$(INCLUDE)}"ks\tutorialdata.h"\
	{$(INCLUDE)}"ks\TutorialFrontEnd.h"\
	{$(INCLUDE)}"ks\tutorialmanager.h"\
	{$(INCLUDE)}"ks\underwtr.h"\
	{$(INCLUDE)}"ks\unlock_manager.h"\
	{$(INCLUDE)}"ks\VOEngine.h"\
	{$(INCLUDE)}"ks\wave.h"\
	{$(INCLUDE)}"ks\wavebreak.txt"\
	{$(INCLUDE)}"ks\wavebreakstage.txt"\
	{$(INCLUDE)}"ks\waveemitter.txt"\
	{$(INCLUDE)}"ks\waveenum.h"\
	{$(INCLUDE)}"ks\wavemarker.txt"\
	{$(INCLUDE)}"ks\waveregion.txt"\
	{$(INCLUDE)}"ks\wavestage.txt"\
	{$(INCLUDE)}"libcdvd.h"\
	{$(INCLUDE)}"libdma.h"\
	{$(INCLUDE)}"libdmapk.h"\
	{$(INCLUDE)}"libgifpk.h"\
	{$(INCLUDE)}"libgraph.h"\
	{$(INCLUDE)}"libipu.h"\
	{$(INCLUDE)}"libmc.h"\
	{$(INCLUDE)}"libmpeg.h"\
	{$(INCLUDE)}"libpad.h"\
	{$(INCLUDE)}"libpc.h"\
	{$(INCLUDE)}"libpkt.h"\
	{$(INCLUDE)}"libusbkb.h"\
	{$(INCLUDE)}"libvifpk.h"\
	{$(INCLUDE)}"libvu0.h"\
	{$(INCLUDE)}"sifdev.h"\
	{$(INCLUDE)}"sifrpc.h"\
	{$(INCLUDE)}"xgraphics.h"\
	
NODEP_CPP_FILES_M=\
	"..\..\NSL\pc\nl_pc.h"\
	"..\..\NSL\pc\nsl_pc_ext.h"\
	".\algo.h"\
	".\defalloc.h"\
	".\gc_arammgr.h"\
	".\hwosmks\am_audio.h"\
	".\hwosmks\mks_devopts.h"\
	".\hwosmks\mks_errmsg.h"\
	".\hwosmks\mks_movieplayer.h"\
	".\hwosmks\set5_rasterize.h"\
	".\hwosmks\set5_timer.h"\
	".\hwosmks\sh4_math.h"\
	".\hwosmks\sy_alloc.h"\
	".\hwosmks\sy_file.h"\
	".\hwosnglpc\algebra.h"\
	".\hwosnglpc\pc_algebra.h"\
	".\hwosnull\null_alloc.h"\
	".\hwosnull\null_audio.h"\
	".\hwosnull\null_devopts.h"\
	".\hwosnull\null_errmsg.h"\
	".\hwosnull\null_file.h"\
	".\hwosnull\null_math.h"\
	".\hwosnull\null_movieplayer.h"\
	".\hwosnull\null_rasterize.h"\
	".\hwosnull\null_timer.h"\
	".\hwospc\d3d_rasterize.h"\
	".\hwospc\ds_audio.h"\
	".\hwospc\pc_algebra.h"\
	".\hwospc\pc_movieplayer.h"\
	".\hwospc\w32_alloc.h"\
	".\hwospc\w32_devopts.h"\
	".\hwospc\w32_dialog.h"\
	".\hwospc\w32_errmsg.h"\
	".\hwospc\w32_file.h"\
	".\hwospc\w32_timer.h"\
	".\hwospc\x86_math.h"\
	".\hwosxb\ngl_xbox.h"\
	".\hwosxb\nvl_xbox.h"\
	".\LipSync_Anim.h"\
	".\ngl_xbox.h"\
	".\nvl_gc.h"\
	".\nvl_xbox.h"\
	".\nvlstream_gc.h"\
	".\stl_heap.h"\
	
# ADD BASE CPP /Yu
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

DEP_CPP_FILES_M=\
	"..\..\NGL\ps2\libsn.h"\
	"..\..\ngl\ps2\matrix.h"\
	"..\..\ngl\ps2\matrix_common.h"\
	"..\..\ngl\ps2\ngl_dma.h"\
	"..\..\ngl\ps2\ngl_fixedstr.h"\
	"..\..\ngl\ps2\ngl_instbank.h"\
	"..\..\ngl\ps2\ngl_ps2.h"\
	"..\..\ngl\ps2\ngl_ps2_internal.h"\
	"..\..\NGL\ps2\ngl_vudefs.h"\
	"..\..\ngl\ps2\tim2.h"\
	"..\..\ngl\ps2\vector.h"\
	"..\..\ngl\ps2\vector_common.h"\
	"..\..\NSL\common\nl.h"\
	"..\..\NSL\common\nsl.h"\
	"..\..\NSL\common\ProjectOptions.h"\
	"..\..\NSL\gamecube\nl_gc.h"\
	"..\..\NSL\ps2\fifo_queue.cpp"\
	"..\..\NSL\ps2\fifo_queue.h"\
	"..\..\NSL\PS2\gas.h"\
	"..\..\NSL\PS2\gas_utility.h"\
	"..\..\NSL\PS2\nl_ps2.h"\
	"..\..\NSL\PS2\nsl_ps2.h"\
	"..\..\NSL\XBOX\nl_xbox.h"\
	"..\..\NSL\XBOX\nsl_xbox.h"\
	"..\..\NSL\XBOX\nsl_xbox_ext.h"\
	"..\..\NSL\XBOX\nsl_xbox_source.h"\
	"..\..\nsl\xbox\nsl_xbox_streams.h"\
	"..\..\NVL\ps2\nvl_ps2.h"\
	"..\..\NVL\ps2\nvlMPEG_ps2.h"\
	"..\..\NVL\ps2\nvlstream_ps2.h"\
	".\aggvertbuf.h"\
	".\ai_constants.h"\
	".\ai_interface.h"\
	".\ai_locomotion.h"\
	".\ai_polypath.h"\
	".\ai_polypath_cell.h"\
	".\ai_polypath_heap.h"\
	".\algebra.h"\
	".\anim.h"\
	".\anim_flavor.h"\
	".\anim_flavors.h"\
	".\anim_ids.h"\
	".\anim_maker.h"\
	".\app.h"\
	".\archalloc.cpp"\
	".\archalloc.h"\
	".\attrib.cpp"\
	".\avltree.h"\
	".\b_spline.h"\
	".\beam.cpp"\
	".\beam.h"\
	".\beam_signals.h"\
	".\billboard.h"\
	".\bone.h"\
	".\bound.h"\
	".\box_trigger_interface.h"\
	".\bsp_collide.h"\
	".\camera.h"\
	".\capsule.h"\
	".\cface.h"\
	".\chunkfile.h"\
	".\colgeom.h"\
	".\collide.h"\
	".\colmesh.h"\
	".\color.h"\
	".\commands.h"\
	".\conglom.h"\
	".\console.h"\
	".\consoleCmds.h"\
	".\consoleVars.h"\
	".\constants.h"\
	".\controller.cpp"\
	".\controller.h"\
	".\convex_box.h"\
	".\crawl_box.h"\
	".\custom_stl.h"\
	".\debug.h"\
	".\debug_render.h"\
	".\devoptflags.h"\
	".\devoptints.h"\
	".\devoptstrs.h"\
	".\element.h"\
	".\entflavor.h"\
	".\entity.h"\
	".\entity_anim.h"\
	".\entity_interface.h"\
	".\entity_maker.h"\
	".\entity_signals.h"\
	".\entityflags.h"\
	".\entityid.h"\
	".\errorcontext.h"\
	".\face.h"\
	".\faceflags.h"\
	".\fast_vector.h"\
	".\fcs.h"\
	".\file.h"\
	".\file_finder.cpp"\
	".\file_finder.h"\
	".\file_manager.h"\
	".\filespec.h"\
	".\fogmgr.h"\
	".\forceflags.h"\
	".\frame_info.h"\
	".\frustum.h"\
	".\game.cpp"\
	".\game.h"\
	".\game_info.h"\
	".\game_info_vars.h"\
	".\game_process.cpp"\
	".\game_process.h"\
	".\gamefile.cpp"\
	".\gamefile.h"\
	".\gcglobals.h"\
	".\generator.h"\
	".\geomgr.h"\
	".\global.h"\
	".\global_signals.h"\
	".\graph.h"\
	".\guidance_sys.cpp"\
	".\guidance_sys.h"\
	".\hard_attribs.h"\
	".\heap.cpp"\
	".\heap.h"\
	".\heaptype.h"\
	".\hull.h"\
	".\hwaudio.h"\
	".\hwmath.h"\
	".\hwmovieplayer.h"\
	".\hwosgc\gc_archalloc.cpp"\
	".\HWOSPS2\ps2_archalloc.cpp"\
	".\hwosxb\xb_archalloc.cpp"\
	".\hwrasterize.h"\
	".\hyperplane.h"\
	".\ini_parser.h"\
	".\inputmgr.h"\
	".\instance.h"\
	".\interface.cpp"\
	".\interface.h"\
	".\iri.h"\
	".\item.cpp"\
	".\item.h"\
	".\item_signals.h"\
	".\joystick.h"\
	".\keyboard.h"\
	".\ks\BoardManager.h"\
	".\ks\DemoMode.h"\
	".\ks\kellyslater_main.h"\
	".\ks\ksfx.h"\
	".\ks\ksheaps.h"\
	".\ks\mode_headtohead.cpp"\
	".\ks\mode_meterattack.cpp"\
	".\ks\mode_push.cpp"\
	".\ks\mode_timeattack.cpp"\
	".\ks\SFXEngine.h"\
	".\ks\water.h"\
	".\ks\wavedata.h"\
	".\ks\wavesound.h"\
	".\ks\wavetex.h"\
	".\kshooks.h"\
	".\ksnsl.h"\
	".\ksnvl.h"\
	".\lensflare.h"\
	".\light.h"\
	".\lightmgr.h"\
	".\linear_anim.h"\
	".\link_interface.h"\
	".\localize.h"\
	".\map_e.h"\
	".\marker.cpp"\
	".\marker.h"\
	".\material.h"\
	".\matfac.h"\
	".\maxskinbones.h"\
	".\mcs.h"\
	".\meshrefs.h"\
	".\mic.cpp"\
	".\mic.h"\
	".\mobject.h"\
	".\mouse.h"\
	".\msgboard.h"\
	".\mustash.h"\
	".\ngl.h"\
	".\ngl_support.h"\
	".\osalloc.h"\
	".\osassert.h"\
	".\osdevopts.h"\
	".\oserrmsg.h"\
	".\osfile.h"\
	".\ostimer.h"\
	".\particle.h"\
	".\particlecleaner.h"\
	".\path.h"\
	".\pc_port.h"\
	".\physical_interface.h"\
	".\plane.h"\
	".\platform_defines.h"\
	".\pmesh.h"\
	".\po.h"\
	".\po_anim.h"\
	".\polytube.h"\
	".\portal.h"\
	".\profcounters.cpp"\
	".\profiler.h"\
	".\proftimers.cpp"\
	".\projconst.h"\
	".\project.h"\
	".\pstring.h"\
	".\random.h"\
	".\rect.h"\
	".\refptr.h"\
	".\region.h"\
	".\region_graph.h"\
	".\render_data.h"\
	".\renderflav.h"\
	".\scene_anim.h"\
	".\script_access.h"\
	".\script_controller_signals.h"\
	".\script_lib_controller.h"\
	".\script_lib_mfg.h"\
	".\script_library_class.h"\
	".\script_mfg_signals.h"\
	".\script_object.h"\
	".\semaphores.h"\
	".\serialin.cpp"\
	".\signal_anim.h"\
	".\signals.h"\
	".\SimpleAssert.h"\
	".\singleton.h"\
	".\sky.cpp"\
	".\sky.h"\
	".\so_data_block.h"\
	".\soft_attribs.h"\
	".\sound_group.cpp"\
	".\sound_group.h"\
	".\sound_interface.h"\
	".\sphere.h"\
	".\stashes.h"\
	".\staticmem.h"\
	".\stl_adapter.h"\
	".\stringx.h"\
	".\switch_obj.h"\
	".\switch_obj_signals.h"\
	".\terrain.h"\
	".\text_font.h"\
	".\textfile.h"\
	".\time_interface.h"\
	".\timer.h"\
	".\tool_dialogs.cpp"\
	".\tool_dialogs.h"\
	".\trigger.cpp"\
	".\trigger.h"\
	".\trigger_signals.h"\
	".\txtcoord.h"\
	".\types.h"\
	".\usefulmath.h"\
	".\users.h"\
	".\vert.h"\
	".\visrep.h"\
	".\vm_executable.h"\
	".\vm_stack.h"\
	".\vm_symbol.h"\
	".\vm_symbol_list.h"\
	".\vm_thread.h"\
	".\vsplit.h"\
	".\warnlvl.h"\
	".\wds.cpp"\
	".\wds.h"\
	".\wedge.h"\
	".\widget.h"\
	".\widget_entity.h"\
	".\widget_script.h"\
	".\xbglobals.h"\
	
NODEP_CPP_FILES_M=\
	"..\..\NSL\pc\nl_pc.h"\
	"..\..\NSL\pc\nsl_pc_ext.h"\
	"..\..\NSL\XBOX\nsl.h"\
	".\algo.h"\
	".\defalloc.h"\
	".\ekernel.h"\
	".\eregs.h"\
	".\estruct.h"\
	".\etypes.h"\
	".\gc_arammgr.h"\
	".\graphics.h"\
	".\hwosgc\nsl.h"\
	".\hwosmks\am_audio.h"\
	".\hwosmks\mks_devopts.h"\
	".\hwosmks\mks_errmsg.h"\
	".\hwosmks\mks_movieplayer.h"\
	".\hwosmks\set5_rasterize.h"\
	".\hwosmks\set5_timer.h"\
	".\hwosmks\sh4_math.h"\
	".\hwosmks\sy_alloc.h"\
	".\hwosmks\sy_file.h"\
	".\hwosnglpc\algebra.h"\
	".\hwosnglpc\pc_algebra.h"\
	".\hwosnull\null_alloc.h"\
	".\hwosnull\null_audio.h"\
	".\hwosnull\null_devopts.h"\
	".\hwosnull\null_errmsg.h"\
	".\hwosnull\null_file.h"\
	".\hwosnull\null_math.h"\
	".\hwosnull\null_movieplayer.h"\
	".\hwosnull\null_rasterize.h"\
	".\hwosnull\null_timer.h"\
	".\hwospc\d3d_rasterize.h"\
	".\hwospc\ds_audio.h"\
	".\hwospc\pc_algebra.h"\
	".\hwospc\pc_movieplayer.h"\
	".\hwospc\w32_alloc.h"\
	".\hwospc\w32_devopts.h"\
	".\hwospc\w32_dialog.h"\
	".\hwospc\w32_errmsg.h"\
	".\hwospc\w32_file.h"\
	".\hwospc\w32_timer.h"\
	".\hwospc\x86_math.h"\
	".\hwosxb\ngl_xbox.h"\
	".\hwosxb\nvl_xbox.h"\
	".\ibcdvd.h"\
	".\ibdma.h"\
	".\ibdmapk.h"\
	".\ibgifpk.h"\
	".\ibgraph.h"\
	".\ibipu.h"\
	".\ibmc.h"\
	".\ibmpeg.h"\
	".\ibpad.h"\
	".\ibpc.h"\
	".\ibpkt.h"\
	".\ibusbkb.h"\
	".\ibvifpk.h"\
	".\ibvu0.h"\
	".\ifdev.h"\
	".\ifrpc.h"\
	".\ks\displace.h"\
	".\ks\refract.h"\
	".\LipSync_Anim.h"\
	".\ngl_xbox.h"\
	".\nvl_gc.h"\
	".\nvl_xbox.h"\
	".\nvlstream_gc.h"\
	".\s\AccompFrontEnd.h"\
	".\s\andersen_ind_anims_mac.h"\
	".\s\beach.h"\
	".\s\beachdata.h"\
	".\s\BeachFrontEnd.h"\
	".\s\blur.h"\
	".\s\board.h"\
	".\s\board_anims_mac.h"\
	".\s\boarddata.h"\
	".\s\BoardFrontEnd.h"\
	".\s\camera_tool.h"\
	".\s\career.h"\
	".\s\careerdata.h"\
	".\s\carrol_ind_anims_mac.h"\
	".\s\challenge.h"\
	".\s\challenge_icon.h"\
	".\s\challenge_photo.h"\
	".\s\cheat.h"\
	".\s\CheatFrontEnd.h"\
	".\s\compressedphoto.h"\
	".\s\coords.h"\
	".\s\curren_ind_anims_mac.h"\
	".\s\eventmanager.h"\
	".\s\ExtrasFrontEnd.h"\
	".\s\FEAnim.h"\
	".\s\FEEntityManager.h"\
	".\s\FEMenu.h"\
	".\s\FEPanel.h"\
	".\s\fletcher_ind_anims_mac.h"\
	".\s\floatobj.h"\
	".\s\frankenreiter_ind_anims_mac.h"\
	".\s\FrontEndManager.h"\
	".\s\FrontEndMenus.h"\
	".\s\GameData.h"\
	".\s\GlobalData.h"\
	".\s\globaltextenum.h"\
	".\s\GraphicalMenuSystem.h"\
	".\s\HighScoreFrontEnd.h"\
	".\s\igo_widget.h"\
	".\s\igo_widget_analogclock.h"\
	".\s\igo_widget_balance.h"\
	".\s\igo_widget_camera.h"\
	".\s\igo_widget_fanmeter.h"\
	".\s\igo_widget_iconradar.h"\
	".\s\igo_widget_photo.h"\
	".\s\igo_widget_replay.h"\
	".\s\igo_widget_simple.h"\
	".\s\igo_widget_specialmeter.h"\
	".\s\igo_widget_splitclock.h"\
	".\s\igo_widget_splitmeter.h"\
	".\s\igo_widget_splitscore.h"\
	".\s\igo_widget_splitter.h"\
	".\s\igo_widget_timeattack.h"\
	".\s\igo_widget_waveindicator.h"\
	".\s\IGOFrontEnd.h"\
	".\s\igohintmanager.h"\
	".\s\igoiconmanager.h"\
	".\s\igolearn_new_trickmanager.h"\
	".\s\ik_object.h"\
	".\s\irons_ind_anims_mac.h"\
	".\s\judge.h"\
	".\s\kellyslater_controller.h"\
	".\s\kellyslater_shared_anims_mac.h"\
	".\s\kellyslater_states_mac.h"\
	".\s\ks_camera.h"\
	".\s\ksdbmenu.h"\
	".\s\ksngl.h"\
	".\s\ksreplay.h"\
	".\s\machado_ind_anims_mac.h"\
	".\s\MainFrontEnd.h"\
	".\s\Map.h"\
	".\s\MCDetectFrontEnd.h"\
	".\s\menu.h"\
	".\s\menusys.h"\
	".\s\mode.h"\
	".\s\mode_headtohead.h"\
	".\s\mode_meterattack.h"\
	".\s\mode_push.h"\
	".\s\mode_timeattack.h"\
	".\s\MultiFrontEnd.h"\
	".\s\MusicMan.h"\
	".\s\ode.h"\
	".\s\osGameSaver.h"\
	".\s\osparticle.h"\
	".\s\PAL60FrontEnd.h"\
	".\s\PathData.h"\
	".\s\PhotoFrontEnd.h"\
	".\s\physics.h"\
	".\s\player.h"\
	".\s\replay.h"\
	".\s\robb_ind_anims_mac.h"\
	".\s\rumbleManager.h"\
	".\s\SaveLoadFrontEnd.h"\
	".\s\scoringmanager.h"\
	".\s\simple_list.h"\
	".\s\slater_ind_anims_mac.h"\
	".\s\sounddata.h"\
	".\s\SoundScript.h"\
	".\s\specialmeter.h"\
	".\s\StatsFrontEnd.h"\
	".\s\surferdata.h"\
	".\s\SurferFrontEnd.h"\
	".\s\trick_system.h"\
	".\s\TrickBookFrontEnd.h"\
	".\s\trickdata.h"\
	".\s\trickregion.h"\
	".\s\tricks.h"\
	".\s\TutorialFrontEnd.h"\
	".\s\tutorialmanager.h"\
	".\s\VOEngine.h"\
	".\s\wave.h"\
	".\s\wavebreak.txt"\
	".\s\wavebreakstage.txt"\
	".\s\waveemitter.txt"\
	".\s\waveenum.h"\
	".\s\wavemarker.txt"\
	".\s\waveregion.txt"\
	".\s\wavestage.txt"\
	".\stl_heap.h"\
	".\wosgc\gc_algebra.h"\
	".\wosgc\gc_alloc.h"\
	".\wosgc\gc_audio.h"\
	".\wosgc\gc_errmsg.h"\
	".\wosgc\gc_file.h"\
	".\WOSGC\gc_gamesaver.h"\
	".\wosgc\gc_graphics.h"\
	".\wosgc\gc_input.h"\
	".\wosgc\gc_math.h"\
	".\wosgc\gc_movieplayer.h"\
	".\wosgc\gc_rasterize.h"\
	".\wosgc\gc_texturemgr.h"\
	".\wosgc\gc_timer.h"\
	".\wosgc\ngl_fixedstr.h"\
	".\wosgc\ngl_gc.h"\
	".\wosgc\ngl_version.h"\
	".\wosgc\nvl_gc.h"\
	".\WOSPS2\gas.h"\
	".\WOSPS2\ps2_algebra.h"\
	".\WOSPS2\ps2_alloc.h"\
	".\WOSPS2\ps2_audio.h"\
	".\WOSPS2\ps2_devopts.h"\
	".\WOSPS2\ps2_errmsg.h"\
	".\WOSPS2\ps2_file.h"\
	".\WOSPS2\ps2_gamesaver.h"\
	".\WOSPS2\ps2_graphics.h"\
	".\WOSPS2\ps2_input.h"\
	".\WOSPS2\ps2_m2vplayer.h"\
	".\WOSPS2\ps2_math.h"\
	".\WOSPS2\ps2_movieplayer.h"\
	".\WOSPS2\ps2_rasterize.h"\
	".\WOSPS2\ps2_texturemgr.h"\
	".\WOSPS2\ps2_timer.h"\
	".\WOSPS2\ps2_types.h"\
	".\wosxb\pstring.h"\
	".\wosxb\xb_algebra.h"\
	".\wosxb\xb_alloc.h"\
	".\wosxb\xb_audio.h"\
	".\wosxb\xb_errmsg.h"\
	".\wosxb\xb_file.h"\
	".\WOSXB\xb_gamesaver.h"\
	".\wosxb\xb_graphics.h"\
	".\wosxb\xb_input.h"\
	".\wosxb\xb_math.h"\
	".\wosxb\xb_movieplayer.h"\
	".\WOSXB\xb_particle.h"\
	".\WOSXB\xb_portDVDCache.h"\
	".\wosxb\xb_rasterize.h"\
	".\wosxb\xb_string.h"\
	".\wosxb\xb_texturemgr.h"\
	".\wosxb\xb_timer.h"\
	".\wosxb\xb_types.h"\
	
# SUBTRACT BASE CPP /YX /Yc /Yu
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

DEP_CPP_FILES_M=\
	"..\..\NGL\ps2\libsn.h"\
	"..\..\ngl\ps2\matrix.h"\
	"..\..\ngl\ps2\matrix_common.h"\
	"..\..\ngl\ps2\ngl_dma.h"\
	"..\..\ngl\ps2\ngl_fixedstr.h"\
	"..\..\ngl\ps2\ngl_instbank.h"\
	"..\..\ngl\ps2\ngl_ps2.h"\
	"..\..\ngl\ps2\ngl_ps2_internal.h"\
	"..\..\NGL\ps2\ngl_vudefs.h"\
	"..\..\ngl\ps2\tim2.h"\
	"..\..\ngl\ps2\vector.h"\
	"..\..\ngl\ps2\vector_common.h"\
	"..\..\NSL\common\nl.h"\
	"..\..\NSL\common\nsl.h"\
	"..\..\NSL\common\ProjectOptions.h"\
	"..\..\NSL\gamecube\nl_gc.h"\
	"..\..\NSL\ps2\fifo_queue.cpp"\
	"..\..\NSL\ps2\fifo_queue.h"\
	"..\..\NSL\PS2\gas.h"\
	"..\..\NSL\PS2\gas_utility.h"\
	"..\..\NSL\PS2\nl_ps2.h"\
	"..\..\NSL\PS2\nsl_ps2.h"\
	"..\..\NSL\XBOX\nl_xbox.h"\
	"..\..\NSL\XBOX\nsl_xbox.h"\
	"..\..\NSL\XBOX\nsl_xbox_ext.h"\
	"..\..\NSL\XBOX\nsl_xbox_source.h"\
	"..\..\nsl\xbox\nsl_xbox_streams.h"\
	"..\..\NVL\ps2\nvl_ps2.h"\
	"..\..\NVL\ps2\nvlMPEG_ps2.h"\
	"..\..\NVL\ps2\nvlstream_ps2.h"\
	".\aggvertbuf.h"\
	".\ai_constants.h"\
	".\ai_interface.h"\
	".\ai_locomotion.h"\
	".\ai_polypath.h"\
	".\ai_polypath_cell.h"\
	".\ai_polypath_heap.h"\
	".\algebra.h"\
	".\anim.h"\
	".\anim_flavor.h"\
	".\anim_flavors.h"\
	".\anim_ids.h"\
	".\anim_maker.h"\
	".\app.h"\
	".\archalloc.cpp"\
	".\archalloc.h"\
	".\attrib.cpp"\
	".\avltree.h"\
	".\b_spline.h"\
	".\beam.cpp"\
	".\beam.h"\
	".\beam_signals.h"\
	".\billboard.h"\
	".\bone.h"\
	".\bound.h"\
	".\box_trigger_interface.h"\
	".\bsp_collide.h"\
	".\camera.h"\
	".\capsule.h"\
	".\cface.h"\
	".\chunkfile.h"\
	".\colgeom.h"\
	".\collide.h"\
	".\colmesh.h"\
	".\color.h"\
	".\commands.h"\
	".\conglom.h"\
	".\console.h"\
	".\consoleCmds.h"\
	".\consoleVars.h"\
	".\constants.h"\
	".\controller.cpp"\
	".\controller.h"\
	".\convex_box.h"\
	".\crawl_box.h"\
	".\custom_stl.h"\
	".\debug.h"\
	".\debug_render.h"\
	".\devoptflags.h"\
	".\devoptints.h"\
	".\devoptstrs.h"\
	".\element.h"\
	".\entflavor.h"\
	".\entity.h"\
	".\entity_anim.h"\
	".\entity_interface.h"\
	".\entity_maker.h"\
	".\entity_signals.h"\
	".\entityflags.h"\
	".\entityid.h"\
	".\errorcontext.h"\
	".\face.h"\
	".\faceflags.h"\
	".\fast_vector.h"\
	".\fcs.h"\
	".\file.h"\
	".\file_finder.cpp"\
	".\file_finder.h"\
	".\file_manager.h"\
	".\filespec.h"\
	".\fogmgr.h"\
	".\forceflags.h"\
	".\frame_info.h"\
	".\frustum.h"\
	".\game.cpp"\
	".\game.h"\
	".\game_info.h"\
	".\game_info_vars.h"\
	".\game_process.cpp"\
	".\game_process.h"\
	".\gamefile.cpp"\
	".\gamefile.h"\
	".\gcglobals.h"\
	".\generator.h"\
	".\geomgr.h"\
	".\global.h"\
	".\global_signals.h"\
	".\graph.h"\
	".\guidance_sys.cpp"\
	".\guidance_sys.h"\
	".\hard_attribs.h"\
	".\heap.cpp"\
	".\heap.h"\
	".\heaptype.h"\
	".\hull.h"\
	".\hwaudio.h"\
	".\hwmath.h"\
	".\hwmovieplayer.h"\
	".\hwosgc\gc_archalloc.cpp"\
	".\HWOSPS2\ps2_archalloc.cpp"\
	".\hwosxb\xb_archalloc.cpp"\
	".\hwrasterize.h"\
	".\hyperplane.h"\
	".\ini_parser.h"\
	".\inputmgr.h"\
	".\instance.h"\
	".\interface.cpp"\
	".\interface.h"\
	".\iri.h"\
	".\item.cpp"\
	".\item.h"\
	".\item_signals.h"\
	".\joystick.h"\
	".\keyboard.h"\
	".\ks\BoardManager.h"\
	".\ks\DemoMode.h"\
	".\ks\kellyslater_main.h"\
	".\ks\ksfx.h"\
	".\ks\ksheaps.h"\
	".\ks\mode_headtohead.cpp"\
	".\ks\mode_meterattack.cpp"\
	".\ks\mode_push.cpp"\
	".\ks\mode_timeattack.cpp"\
	".\ks\SFXEngine.h"\
	".\ks\water.h"\
	".\ks\wavedata.h"\
	".\ks\wavesound.h"\
	".\ks\wavetex.h"\
	".\kshooks.h"\
	".\ksnsl.h"\
	".\ksnvl.h"\
	".\lensflare.h"\
	".\light.h"\
	".\lightmgr.h"\
	".\linear_anim.h"\
	".\link_interface.h"\
	".\localize.h"\
	".\map_e.h"\
	".\marker.cpp"\
	".\marker.h"\
	".\material.h"\
	".\matfac.h"\
	".\maxskinbones.h"\
	".\mcs.h"\
	".\meshrefs.h"\
	".\mic.cpp"\
	".\mic.h"\
	".\mobject.h"\
	".\mouse.h"\
	".\msgboard.h"\
	".\mustash.h"\
	".\ngl.h"\
	".\ngl_support.h"\
	".\osalloc.h"\
	".\osassert.h"\
	".\osdevopts.h"\
	".\oserrmsg.h"\
	".\osfile.h"\
	".\ostimer.h"\
	".\particle.h"\
	".\particlecleaner.h"\
	".\path.h"\
	".\pc_port.h"\
	".\physical_interface.h"\
	".\plane.h"\
	".\platform_defines.h"\
	".\pmesh.h"\
	".\po.h"\
	".\po_anim.h"\
	".\polytube.h"\
	".\portal.h"\
	".\profcounters.cpp"\
	".\profiler.h"\
	".\proftimers.cpp"\
	".\projconst.h"\
	".\project.h"\
	".\pstring.h"\
	".\random.h"\
	".\rect.h"\
	".\refptr.h"\
	".\region.h"\
	".\region_graph.h"\
	".\render_data.h"\
	".\renderflav.h"\
	".\scene_anim.h"\
	".\script_access.h"\
	".\script_controller_signals.h"\
	".\script_lib_controller.h"\
	".\script_lib_mfg.h"\
	".\script_library_class.h"\
	".\script_mfg_signals.h"\
	".\script_object.h"\
	".\semaphores.h"\
	".\serialin.cpp"\
	".\signal_anim.h"\
	".\signals.h"\
	".\SimpleAssert.h"\
	".\singleton.h"\
	".\sky.cpp"\
	".\sky.h"\
	".\so_data_block.h"\
	".\soft_attribs.h"\
	".\sound_group.cpp"\
	".\sound_group.h"\
	".\sound_interface.h"\
	".\sphere.h"\
	".\stashes.h"\
	".\staticmem.h"\
	".\stl_adapter.h"\
	".\stringx.h"\
	".\switch_obj.h"\
	".\switch_obj_signals.h"\
	".\terrain.h"\
	".\text_font.h"\
	".\textfile.h"\
	".\time_interface.h"\
	".\timer.h"\
	".\tool_dialogs.cpp"\
	".\tool_dialogs.h"\
	".\trigger.cpp"\
	".\trigger.h"\
	".\trigger_signals.h"\
	".\txtcoord.h"\
	".\types.h"\
	".\usefulmath.h"\
	".\users.h"\
	".\vert.h"\
	".\visrep.h"\
	".\vm_executable.h"\
	".\vm_stack.h"\
	".\vm_symbol.h"\
	".\vm_symbol_list.h"\
	".\vm_thread.h"\
	".\vsplit.h"\
	".\warnlvl.h"\
	".\wds.cpp"\
	".\wds.h"\
	".\wedge.h"\
	".\widget.h"\
	".\widget_entity.h"\
	".\widget_script.h"\
	".\xbglobals.h"\
	
NODEP_CPP_FILES_M=\
	"..\..\NSL\pc\nl_pc.h"\
	"..\..\NSL\pc\nsl_pc_ext.h"\
	".\algo.h"\
	".\defalloc.h"\
	".\displace.h"\
	".\ekernel.h"\
	".\eregs.h"\
	".\estruct.h"\
	".\etypes.h"\
	".\gc\nsl_gc.h"\
	".\gc_arammgr.h"\
	".\graphics.h"\
	".\hwosmks\am_audio.h"\
	".\hwosmks\mks_devopts.h"\
	".\hwosmks\mks_errmsg.h"\
	".\hwosmks\mks_movieplayer.h"\
	".\hwosmks\set5_rasterize.h"\
	".\hwosmks\set5_timer.h"\
	".\hwosmks\sh4_math.h"\
	".\hwosmks\sy_alloc.h"\
	".\hwosmks\sy_file.h"\
	".\hwosnglpc\algebra.h"\
	".\hwosnglpc\pc_algebra.h"\
	".\hwosnull\null_alloc.h"\
	".\hwosnull\null_audio.h"\
	".\hwosnull\null_devopts.h"\
	".\hwosnull\null_errmsg.h"\
	".\hwosnull\null_file.h"\
	".\hwosnull\null_math.h"\
	".\hwosnull\null_movieplayer.h"\
	".\hwosnull\null_rasterize.h"\
	".\hwosnull\null_timer.h"\
	".\hwospc\d3d_rasterize.h"\
	".\hwospc\ds_audio.h"\
	".\hwospc\pc_algebra.h"\
	".\hwospc\pc_movieplayer.h"\
	".\hwospc\w32_alloc.h"\
	".\hwospc\w32_devopts.h"\
	".\hwospc\w32_dialog.h"\
	".\hwospc\w32_errmsg.h"\
	".\hwospc\w32_file.h"\
	".\hwospc\w32_timer.h"\
	".\hwospc\x86_math.h"\
	".\hwosxb\ngl_xbox.h"\
	".\hwosxb\nvl_xbox.h"\
	".\ibcdvd.h"\
	".\ibdma.h"\
	".\ibdmapk.h"\
	".\ibgifpk.h"\
	".\ibgraph.h"\
	".\ibipu.h"\
	".\ibmc.h"\
	".\ibmpeg.h"\
	".\ibpad.h"\
	".\ibpc.h"\
	".\ibpkt.h"\
	".\ibusbkb.h"\
	".\ibvifpk.h"\
	".\ibvu0.h"\
	".\ifdev.h"\
	".\ifrpc.h"\
	".\LipSync_Anim.h"\
	".\ngl_xbox.h"\
	".\nsl.h"\
	".\nvl_gc.h"\
	".\nvl_xbox.h"\
	".\nvlstream_gc.h"\
	".\refract.h"\
	".\s\AccompFrontEnd.h"\
	".\s\andersen_ind_anims_mac.h"\
	".\s\beach.h"\
	".\s\beachdata.h"\
	".\s\BeachFrontEnd.h"\
	".\s\blur.h"\
	".\s\board.h"\
	".\s\board_anims_mac.h"\
	".\s\boarddata.h"\
	".\s\BoardFrontEnd.h"\
	".\s\camera_tool.h"\
	".\s\career.h"\
	".\s\careerdata.h"\
	".\s\carrol_ind_anims_mac.h"\
	".\s\challenge.h"\
	".\s\challenge_icon.h"\
	".\s\challenge_photo.h"\
	".\s\cheat.h"\
	".\s\CheatFrontEnd.h"\
	".\s\compressedphoto.h"\
	".\s\coords.h"\
	".\s\curren_ind_anims_mac.h"\
	".\s\eventmanager.h"\
	".\s\ExtrasFrontEnd.h"\
	".\s\FEAnim.h"\
	".\s\FEEntityManager.h"\
	".\s\FEMenu.h"\
	".\s\FEPanel.h"\
	".\s\fletcher_ind_anims_mac.h"\
	".\s\floatobj.h"\
	".\s\frankenreiter_ind_anims_mac.h"\
	".\s\FrontEndManager.h"\
	".\s\FrontEndMenus.h"\
	".\s\GameData.h"\
	".\s\GlobalData.h"\
	".\s\globaltextenum.h"\
	".\s\GraphicalMenuSystem.h"\
	".\s\HighScoreFrontEnd.h"\
	".\s\igo_widget.h"\
	".\s\igo_widget_analogclock.h"\
	".\s\igo_widget_balance.h"\
	".\s\igo_widget_camera.h"\
	".\s\igo_widget_fanmeter.h"\
	".\s\igo_widget_iconcount.h"\
	".\s\igo_widget_iconradar.h"\
	".\s\igo_widget_meterchallenge.h"\
	".\s\igo_widget_objectalert.h"\
	".\s\igo_widget_photo.h"\
	".\s\igo_widget_replay.h"\
	".\s\igo_widget_simple.h"\
	".\s\igo_widget_skillchallenge.h"\
	".\s\igo_widget_specialmeter.h"\
	".\s\igo_widget_splitclock.h"\
	".\s\igo_widget_splitmeter.h"\
	".\s\igo_widget_splitscore.h"\
	".\s\igo_widget_splitter.h"\
	".\s\igo_widget_timeattack.h"\
	".\s\igo_widget_waveindicator.h"\
	".\s\IGOFrontEnd.h"\
	".\s\igohintmanager.h"\
	".\s\igoiconmanager.h"\
	".\s\igolearn_new_trickmanager.h"\
	".\s\ik_object.h"\
	".\s\irons_ind_anims_mac.h"\
	".\s\judge.h"\
	".\s\kellyslater_controller.h"\
	".\s\kellyslater_shared_anims_mac.h"\
	".\s\kellyslater_states_mac.h"\
	".\s\ks_camera.h"\
	".\s\ksdbmenu.h"\
	".\s\ksngl.h"\
	".\s\ksreplay.h"\
	".\s\LogbookFrontEnd.h"\
	".\s\machado_ind_anims_mac.h"\
	".\s\MainFrontEnd.h"\
	".\s\Map.h"\
	".\s\MCDetectFrontEnd.h"\
	".\s\menu.h"\
	".\s\menusys.h"\
	".\s\mode.h"\
	".\s\mode_headtohead.h"\
	".\s\mode_meterattack.h"\
	".\s\mode_push.h"\
	".\s\mode_timeattack.h"\
	".\s\MultiFrontEnd.h"\
	".\s\MusicMan.h"\
	".\s\ode.h"\
	".\s\osGameSaver.h"\
	".\s\osparticle.h"\
	".\s\PAL60FrontEnd.h"\
	".\s\PathData.h"\
	".\s\PhotoFrontEnd.h"\
	".\s\physics.h"\
	".\s\player.h"\
	".\s\replay.h"\
	".\s\robb_ind_anims_mac.h"\
	".\s\rumbleManager.h"\
	".\s\SaveLoadFrontEnd.h"\
	".\s\scoringmanager.h"\
	".\s\simple_list.h"\
	".\s\slater_ind_anims_mac.h"\
	".\s\sounddata.h"\
	".\s\SoundScript.h"\
	".\s\specialmeter.h"\
	".\s\StatsFrontEnd.h"\
	".\s\surferdata.h"\
	".\s\SurferFrontEnd.h"\
	".\s\trick_system.h"\
	".\s\TrickBookFrontEnd.h"\
	".\s\trickdata.h"\
	".\s\trickregion.h"\
	".\s\tricks.h"\
	".\s\tutorialdata.h"\
	".\s\TutorialFrontEnd.h"\
	".\s\tutorialmanager.h"\
	".\s\VOEngine.h"\
	".\s\wave.h"\
	".\s\wavebreak.txt"\
	".\s\wavebreakstage.txt"\
	".\s\waveemitter.txt"\
	".\s\waveenum.h"\
	".\s\wavemarker.txt"\
	".\s\waveregion.txt"\
	".\s\wavestage.txt"\
	".\stl_heap.h"\
	".\wosgc\gc_algebra.h"\
	".\wosgc\gc_alloc.h"\
	".\wosgc\gc_audio.h"\
	".\wosgc\gc_errmsg.h"\
	".\wosgc\gc_file.h"\
	".\WOSGC\gc_gamesaver.h"\
	".\wosgc\gc_graphics.h"\
	".\wosgc\gc_input.h"\
	".\wosgc\gc_math.h"\
	".\wosgc\gc_movieplayer.h"\
	".\wosgc\gc_rasterize.h"\
	".\wosgc\gc_texturemgr.h"\
	".\wosgc\gc_timer.h"\
	".\wosgc\ngl_fixedstr.h"\
	".\wosgc\ngl_gc.h"\
	".\wosgc\ngl_version.h"\
	".\wosgc\nvl_gc.h"\
	".\WOSPS2\gas.h"\
	".\WOSPS2\ps2_algebra.h"\
	".\WOSPS2\ps2_alloc.h"\
	".\WOSPS2\ps2_audio.h"\
	".\WOSPS2\ps2_devopts.h"\
	".\WOSPS2\ps2_errmsg.h"\
	".\WOSPS2\ps2_file.h"\
	".\WOSPS2\ps2_gamesaver.h"\
	".\WOSPS2\ps2_graphics.h"\
	".\WOSPS2\ps2_input.h"\
	".\WOSPS2\ps2_m2vplayer.h"\
	".\WOSPS2\ps2_math.h"\
	".\WOSPS2\ps2_movieplayer.h"\
	".\WOSPS2\ps2_rasterize.h"\
	".\WOSPS2\ps2_texturemgr.h"\
	".\WOSPS2\ps2_timer.h"\
	".\WOSPS2\ps2_types.h"\
	".\wosxb\pstring.h"\
	".\wosxb\xb_algebra.h"\
	".\wosxb\xb_alloc.h"\
	".\wosxb\xb_audio.h"\
	".\wosxb\xb_errmsg.h"\
	".\wosxb\xb_file.h"\
	".\WOSXB\xb_gamesaver.h"\
	".\wosxb\xb_graphics.h"\
	".\wosxb\xb_input.h"\
	".\wosxb\xb_math.h"\
	".\wosxb\xb_movieplayer.h"\
	".\WOSXB\xb_particle.h"\
	".\WOSXB\xb_portDVDCache.h"\
	".\wosxb\xb_rasterize.h"\
	".\wosxb\xb_string.h"\
	".\wosxb\xb_texturemgr.h"\
	".\wosxb\xb_timer.h"\
	".\wosxb\xb_types.h"\
	
# SUBTRACT BASE CPP /YX /Yc /Yu
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\files_misc2.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

DEP_CPP_FILES_MI=\
	"..\..\ngl\ps2\ngl_fixedstr.h"\
	"..\..\ngl\ps2\ngl_ps2.h"\
	"..\..\ngl\ps2\tim2.h"\
	"..\..\NSL\common\nl.h"\
	"..\..\NSL\common\nsl.h"\
	"..\..\NSL\common\ProjectOptions.h"\
	"..\..\NSL\gamecube\nl_gc.h"\
	"..\..\NSL\ps2\fifo_queue.cpp"\
	"..\..\NSL\ps2\fifo_queue.h"\
	"..\..\NSL\PS2\gas.h"\
	"..\..\NSL\PS2\gas_utility.h"\
	"..\..\NSL\PS2\nl_ps2.h"\
	"..\..\NSL\PS2\nsl_ps2.h"\
	"..\..\NSL\XBOX\nl_xbox.h"\
	"..\..\NSL\XBOX\nsl_xbox.h"\
	"..\..\NSL\XBOX\nsl_xbox_ext.h"\
	"..\..\NSL\XBOX\nsl_xbox_source.h"\
	"..\..\nsl\xbox\nsl_xbox_streams.h"\
	"..\..\NVL\ps2\nvl_ps2.h"\
	"..\..\NVL\ps2\nvlMPEG_ps2.h"\
	"..\..\NVL\ps2\nvlstream_ps2.h"\
	".\aggvertbuf.h"\
	".\ai_constants.h"\
	".\ai_interface.h"\
	".\ai_locomotion.h"\
	".\ai_polypath.h"\
	".\ai_polypath_cell.h"\
	".\ai_polypath_heap.h"\
	".\algebra.h"\
	".\anim.h"\
	".\anim_flavor.h"\
	".\anim_flavors.h"\
	".\anim_ids.h"\
	".\anim_maker.h"\
	".\app.h"\
	".\archalloc.h"\
	".\avltree.h"\
	".\beam.h"\
	".\beam_signals.h"\
	".\billboard.cpp"\
	".\billboard.h"\
	".\binary_tree.h"\
	".\blendmodes.h"\
	".\bone.h"\
	".\bound.h"\
	".\box_trigger_interface.h"\
	".\bp_tree.h"\
	".\bsp_collide.h"\
	".\bsp_tree.h"\
	".\camera.cpp"\
	".\camera.h"\
	".\capsule.cpp"\
	".\capsule.h"\
	".\cface.cpp"\
	".\cface.h"\
	".\chunkfile.h"\
	".\clipflags.h"\
	".\colgeom.cpp"\
	".\colgeom.h"\
	".\collide.cpp"\
	".\collide.h"\
	".\colmesh.h"\
	".\color.h"\
	".\commands.h"\
	".\conglom.h"\
	".\console.h"\
	".\constants.h"\
	".\controller.h"\
	".\convex_box.cpp"\
	".\convex_box.h"\
	".\custom_stl.h"\
	".\debug.h"\
	".\debug_render.cpp"\
	".\debug_render.h"\
	".\devoptflags.h"\
	".\devoptints.h"\
	".\devoptstrs.h"\
	".\element.cpp"\
	".\element.h"\
	".\entflavor.h"\
	".\entity.h"\
	".\entity_anim.h"\
	".\entity_interface.h"\
	".\entity_maker.h"\
	".\entity_signals.h"\
	".\entityflags.h"\
	".\entityid.h"\
	".\errorcontext.h"\
	".\face.h"\
	".\faceflags.h"\
	".\fast_vector.h"\
	".\fcs.cpp"\
	".\fcs.h"\
	".\file.h"\
	".\file_finder.h"\
	".\filespec.h"\
	".\forceflags.h"\
	".\frame_info.h"\
	".\game.h"\
	".\game_info.h"\
	".\game_info_vars.h"\
	".\game_process.h"\
	".\gcglobals.h"\
	".\generator.cpp"\
	".\generator.h"\
	".\geomgr.cpp"\
	".\geomgr.h"\
	".\global.h"\
	".\global_signals.h"\
	".\graph.h"\
	".\guidance_sys.h"\
	".\hinge.h"\
	".\hull.h"\
	".\hwmath.h"\
	".\hwmovieplayer.h"\
	".\hwrasterize.h"\
	".\hyperplane.h"\
	".\ini_parser.h"\
	".\inputmgr.h"\
	".\instance.h"\
	".\iri.h"\
	".\item.h"\
	".\item_signals.h"\
	".\joint.cpp"\
	".\joint.h"\
	".\joystick.h"\
	".\keyboard.h"\
	".\ks\camera_tool.cpp"\
	".\ks\cheatmenu.cpp"\
	".\ks\GlobalData.cpp"\
	".\ks\ksheaps.h"\
	".\ks\unlock_manager.cpp"\
	".\kshooks.h"\
	".\ksnsl.h"\
	".\ksnvl.h"\
	".\lensflare.cpp"\
	".\lensflare.h"\
	".\light.cpp"\
	".\light.h"\
	".\lightmgr.cpp"\
	".\lightmgr.h"\
	".\linear_anim.h"\
	".\link_interface.h"\
	".\localize.cpp"\
	".\localize.h"\
	".\map_e.h"\
	".\material.h"\
	".\matfac.cpp"\
	".\matfac.h"\
	".\maxiri.h"\
	".\maxskinbones.h"\
	".\mcs.cpp"\
	".\mcs.h"\
	".\menusound.cpp"\
	".\meshrefs.h"\
	".\mic.h"\
	".\mobject.h"\
	".\msgboard.cpp"\
	".\msgboard.h"\
	".\mustash.h"\
	".\ngl.h"\
	".\ngl_support.h"\
	".\osalloc.h"\
	".\osassert.h"\
	".\osdevopts.h"\
	".\oserrmsg.h"\
	".\osfile.h"\
	".\ostimer.h"\
	".\particle.cpp"\
	".\particle.h"\
	".\particlecleaner.cpp"\
	".\particlecleaner.h"\
	".\path.h"\
	".\pc_port.h"\
	".\physical_interface.h"\
	".\plane.h"\
	".\platform_defines.h"\
	".\pmesh.cpp"\
	".\pmesh.h"\
	".\po.h"\
	".\po_anim.h"\
	".\portal.cpp"\
	".\portal.h"\
	".\profiler.h"\
	".\projconst.h"\
	".\project.h"\
	".\pstring.h"\
	".\random.h"\
	".\rect.h"\
	".\refptr.h"\
	".\region.cpp"\
	".\region.h"\
	".\region_graph.h"\
	".\render_data.h"\
	".\renderflav.h"\
	".\scene_anim.h"\
	".\script_lib_mfg.h"\
	".\script_library_class.h"\
	".\script_mfg_signals.h"\
	".\script_object.h"\
	".\semaphores.cpp"\
	".\semaphores.h"\
	".\shock_mods.h"\
	".\signal_anim.h"\
	".\signals.h"\
	".\SimpleAssert.h"\
	".\singleton.h"\
	".\so_data_block.h"\
	".\sphere.h"\
	".\stashes.h"\
	".\staticmem.h"\
	".\stl_adapter.h"\
	".\stringx.h"\
	".\terrain.cpp"\
	".\terrain.h"\
	".\text_font.h"\
	".\textfile.h"\
	".\time_interface.h"\
	".\timer.h"\
	".\txtcoord.h"\
	".\types.h"\
	".\usefulmath.h"\
	".\users.h"\
	".\vector_alloc.h"\
	".\vert.h"\
	".\vertnorm.h"\
	".\vertwork.h"\
	".\visrep.cpp"\
	".\visrep.h"\
	".\vm_executable.h"\
	".\vm_stack.h"\
	".\vm_symbol.h"\
	".\vm_symbol_list.h"\
	".\vm_thread.h"\
	".\vsplit.h"\
	".\warnlvl.h"\
	".\wds.h"\
	".\wedge.h"\
	".\widget.h"\
	".\widget_entity.cpp"\
	".\widget_entity.h"\
	".\widget_script.cpp"\
	".\widget_script.h"\
	".\xbglobals.h"\
	{$(INCLUDE)}"eekernel.h"\
	{$(INCLUDE)}"eeregs.h"\
	{$(INCLUDE)}"eestruct.h"\
	{$(INCLUDE)}"eetypes.h"\
	{$(INCLUDE)}"hwosgc\gc_algebra.h"\
	{$(INCLUDE)}"hwosgc\gc_alloc.h"\
	{$(INCLUDE)}"hwosgc\gc_errmsg.h"\
	{$(INCLUDE)}"hwosgc\gc_file.h"\
	{$(INCLUDE)}"HWOSGC\gc_gamesaver.h"\
	{$(INCLUDE)}"hwosgc\gc_graphics.h"\
	{$(INCLUDE)}"hwosgc\gc_input.h"\
	{$(INCLUDE)}"hwosgc\gc_math.h"\
	{$(INCLUDE)}"hwosgc\gc_movieplayer.h"\
	{$(INCLUDE)}"hwosgc\gc_rasterize.h"\
	{$(INCLUDE)}"hwosgc\gc_texturemgr.h"\
	{$(INCLUDE)}"hwosgc\gc_timer.h"\
	{$(INCLUDE)}"hwosgc\ngl_fixedstr.h"\
	{$(INCLUDE)}"hwosgc\ngl_gc.h"\
	{$(INCLUDE)}"hwosgc\ngl_version.h"\
	{$(INCLUDE)}"hwosgc\nvl_gc.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_algebra.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_alloc.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_devopts.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_errmsg.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_file.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_gamesaver.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_graphics.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_input.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_m2vplayer.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_math.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_movieplayer.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_rasterize.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_texturemgr.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_timer.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_types.h"\
	{$(INCLUDE)}"hwosxb\xb_algebra.h"\
	{$(INCLUDE)}"hwosxb\xb_alloc.h"\
	{$(INCLUDE)}"hwosxb\xb_errmsg.h"\
	{$(INCLUDE)}"hwosxb\xb_file.h"\
	{$(INCLUDE)}"HWOSXB\xb_gamesaver.h"\
	{$(INCLUDE)}"hwosxb\xb_graphics.h"\
	{$(INCLUDE)}"hwosxb\xb_input.h"\
	{$(INCLUDE)}"hwosxb\xb_math.h"\
	{$(INCLUDE)}"hwosxb\xb_movieplayer.h"\
	{$(INCLUDE)}"HWOSXB\xb_portDVDCache.h"\
	{$(INCLUDE)}"hwosxb\xb_rasterize.h"\
	{$(INCLUDE)}"hwosxb\xb_string.h"\
	{$(INCLUDE)}"hwosxb\xb_texturemgr.h"\
	{$(INCLUDE)}"hwosxb\xb_timer.h"\
	{$(INCLUDE)}"hwosxb\xb_types.h"\
	{$(INCLUDE)}"ks\AccompFrontEnd.h"\
	{$(INCLUDE)}"ks\andersen_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\beach.h"\
	{$(INCLUDE)}"ks\beachdata.h"\
	{$(INCLUDE)}"ks\BeachFrontEnd.h"\
	{$(INCLUDE)}"ks\board.h"\
	{$(INCLUDE)}"ks\board_anims_mac.h"\
	{$(INCLUDE)}"ks\boarddata.h"\
	{$(INCLUDE)}"ks\BoardFrontEnd.h"\
	{$(INCLUDE)}"ks\camera_tool.h"\
	{$(INCLUDE)}"ks\career.h"\
	{$(INCLUDE)}"ks\careerdata.h"\
	{$(INCLUDE)}"ks\carrol_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\challenge.h"\
	{$(INCLUDE)}"ks\challenge_icon.h"\
	{$(INCLUDE)}"ks\challenge_photo.h"\
	{$(INCLUDE)}"ks\cheat.h"\
	{$(INCLUDE)}"ks\CheatFrontEnd.h"\
	{$(INCLUDE)}"ks\compressedphoto.h"\
	{$(INCLUDE)}"ks\coords.h"\
	{$(INCLUDE)}"ks\curren_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\eventmanager.h"\
	{$(INCLUDE)}"ks\ExtrasFrontEnd.h"\
	{$(INCLUDE)}"ks\FEAnim.h"\
	{$(INCLUDE)}"ks\FEEntityManager.h"\
	{$(INCLUDE)}"ks\FEMenu.h"\
	{$(INCLUDE)}"ks\FEPanel.h"\
	{$(INCLUDE)}"ks\fletcher_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\floatobj.h"\
	{$(INCLUDE)}"ks\frankenreiter_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\FrontEndManager.h"\
	{$(INCLUDE)}"ks\FrontEndMenus.h"\
	{$(INCLUDE)}"ks\GameData.h"\
	{$(INCLUDE)}"ks\GlobalData.h"\
	{$(INCLUDE)}"ks\globaltextenum.h"\
	{$(INCLUDE)}"ks\GraphicalMenuSystem.h"\
	{$(INCLUDE)}"ks\HighScoreFrontEnd.h"\
	{$(INCLUDE)}"ks\igo_widget.h"\
	{$(INCLUDE)}"ks\igo_widget_analogclock.h"\
	{$(INCLUDE)}"ks\igo_widget_balance.h"\
	{$(INCLUDE)}"ks\igo_widget_camera.h"\
	{$(INCLUDE)}"ks\igo_widget_fanmeter.h"\
	{$(INCLUDE)}"ks\igo_widget_grid.h"\
	{$(INCLUDE)}"ks\igo_widget_iconcount.h"\
	{$(INCLUDE)}"ks\igo_widget_iconradar.h"\
	{$(INCLUDE)}"ks\igo_widget_meterchallenge.h"\
	{$(INCLUDE)}"ks\igo_widget_objectalert.h"\
	{$(INCLUDE)}"ks\igo_widget_photo.h"\
	{$(INCLUDE)}"ks\igo_widget_replay.h"\
	{$(INCLUDE)}"ks\igo_widget_simple.h"\
	{$(INCLUDE)}"ks\igo_widget_skillchallenge.h"\
	{$(INCLUDE)}"ks\igo_widget_specialmeter.h"\
	{$(INCLUDE)}"ks\igo_widget_splitclock.h"\
	{$(INCLUDE)}"ks\igo_widget_splitmeter.h"\
	{$(INCLUDE)}"ks\igo_widget_splitscore.h"\
	{$(INCLUDE)}"ks\igo_widget_splitter.h"\
	{$(INCLUDE)}"ks\igo_widget_timeattack.h"\
	{$(INCLUDE)}"ks\igo_widget_waveindicator.h"\
	{$(INCLUDE)}"ks\IGOFrontEnd.h"\
	{$(INCLUDE)}"ks\igohintmanager.h"\
	{$(INCLUDE)}"ks\igoiconmanager.h"\
	{$(INCLUDE)}"ks\igolearn_new_trickmanager.h"\
	{$(INCLUDE)}"ks\ik_object.h"\
	{$(INCLUDE)}"ks\irons_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\judge.h"\
	{$(INCLUDE)}"ks\kellyslater_controller.h"\
	{$(INCLUDE)}"ks\kellyslater_shared_anims_mac.h"\
	{$(INCLUDE)}"ks\kellyslater_states_mac.h"\
	{$(INCLUDE)}"ks\ks_camera.h"\
	{$(INCLUDE)}"ks\ksdbmenu.h"\
	{$(INCLUDE)}"ks\ksngl.h"\
	{$(INCLUDE)}"ks\ksreplay.h"\
	{$(INCLUDE)}"ks\LogbookFrontEnd.h"\
	{$(INCLUDE)}"ks\machado_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\MainFrontEnd.h"\
	{$(INCLUDE)}"ks\Map.h"\
	{$(INCLUDE)}"ks\MCDetectFrontEnd.h"\
	{$(INCLUDE)}"ks\menu.h"\
	{$(INCLUDE)}"ks\menusys.h"\
	{$(INCLUDE)}"ks\mode.h"\
	{$(INCLUDE)}"ks\mode_headtohead.h"\
	{$(INCLUDE)}"ks\mode_meterattack.h"\
	{$(INCLUDE)}"ks\mode_push.h"\
	{$(INCLUDE)}"ks\mode_timeattack.h"\
	{$(INCLUDE)}"ks\MultiFrontEnd.h"\
	{$(INCLUDE)}"ks\MusicMan.h"\
	{$(INCLUDE)}"ks\ode.h"\
	{$(INCLUDE)}"ks\osGameSaver.h"\
	{$(INCLUDE)}"ks\PAL60FrontEnd.h"\
	{$(INCLUDE)}"ks\PathData.h"\
	{$(INCLUDE)}"ks\PhotoFrontEnd.h"\
	{$(INCLUDE)}"ks\physics.h"\
	{$(INCLUDE)}"ks\player.h"\
	{$(INCLUDE)}"ks\replay.h"\
	{$(INCLUDE)}"ks\robb_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\rumbleManager.h"\
	{$(INCLUDE)}"ks\SaveLoadFrontEnd.h"\
	{$(INCLUDE)}"ks\scoringmanager.h"\
	{$(INCLUDE)}"ks\simple_list.h"\
	{$(INCLUDE)}"ks\slater_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\sounddata.h"\
	{$(INCLUDE)}"ks\SoundScript.h"\
	{$(INCLUDE)}"ks\specialmeter.h"\
	{$(INCLUDE)}"ks\StatsFrontEnd.h"\
	{$(INCLUDE)}"ks\surferdata.h"\
	{$(INCLUDE)}"ks\SurferFrontEnd.h"\
	{$(INCLUDE)}"ks\trick_system.h"\
	{$(INCLUDE)}"ks\TrickBookFrontEnd.h"\
	{$(INCLUDE)}"ks\trickdata.h"\
	{$(INCLUDE)}"ks\trickregion.h"\
	{$(INCLUDE)}"ks\tricks.h"\
	{$(INCLUDE)}"ks\tutorialdata.h"\
	{$(INCLUDE)}"ks\TutorialFrontEnd.h"\
	{$(INCLUDE)}"ks\tutorialmanager.h"\
	{$(INCLUDE)}"ks\underwtr.h"\
	{$(INCLUDE)}"ks\unlock_manager.h"\
	{$(INCLUDE)}"ks\VOEngine.h"\
	{$(INCLUDE)}"ks\wave.h"\
	{$(INCLUDE)}"ks\wavebreak.txt"\
	{$(INCLUDE)}"ks\wavebreakstage.txt"\
	{$(INCLUDE)}"ks\waveemitter.txt"\
	{$(INCLUDE)}"ks\waveenum.h"\
	{$(INCLUDE)}"ks\wavemarker.txt"\
	{$(INCLUDE)}"ks\waveregion.txt"\
	{$(INCLUDE)}"ks\wavestage.txt"\
	{$(INCLUDE)}"libcdvd.h"\
	{$(INCLUDE)}"libdma.h"\
	{$(INCLUDE)}"libgraph.h"\
	{$(INCLUDE)}"libipu.h"\
	{$(INCLUDE)}"libmc.h"\
	{$(INCLUDE)}"libmpeg.h"\
	{$(INCLUDE)}"libpad.h"\
	{$(INCLUDE)}"libpc.h"\
	{$(INCLUDE)}"libusbkb.h"\
	{$(INCLUDE)}"libvu0.h"\
	{$(INCLUDE)}"sifdev.h"\
	{$(INCLUDE)}"xgraphics.h"\
	
NODEP_CPP_FILES_MI=\
	"..\..\NSL\pc\nl_pc.h"\
	"..\..\NSL\pc\nsl_pc_ext.h"\
	".\algo.h"\
	".\defalloc.h"\
	".\gc_arammgr.h"\
	".\hwosmks\mks_devopts.h"\
	".\hwosmks\mks_errmsg.h"\
	".\hwosmks\mks_movieplayer.h"\
	".\hwosmks\set5_rasterize.h"\
	".\hwosmks\set5_timer.h"\
	".\hwosmks\sh4_math.h"\
	".\hwosmks\sy_alloc.h"\
	".\hwosmks\sy_file.h"\
	".\hwosnglpc\algebra.h"\
	".\hwosnglpc\pc_algebra.h"\
	".\hwosnull\null_alloc.h"\
	".\hwosnull\null_devopts.h"\
	".\hwosnull\null_errmsg.h"\
	".\hwosnull\null_file.h"\
	".\hwosnull\null_math.h"\
	".\hwosnull\null_movieplayer.h"\
	".\hwosnull\null_rasterize.h"\
	".\hwosnull\null_timer.h"\
	".\hwospc\d3d_rasterize.h"\
	".\hwospc\pc_algebra.h"\
	".\hwospc\pc_movieplayer.h"\
	".\hwospc\w32_alloc.h"\
	".\hwospc\w32_devopts.h"\
	".\hwospc\w32_errmsg.h"\
	".\hwospc\w32_file.h"\
	".\hwospc\w32_timer.h"\
	".\hwospc\x86_math.h"\
	".\hwosxb\ngl_xbox.h"\
	".\hwosxb\nvl_xbox.h"\
	".\LipSync_Anim.h"\
	".\ngl_xbox.h"\
	".\nvl_gc.h"\
	".\nvl_xbox.h"\
	".\nvlstream_gc.h"\
	".\stl_heap.h"\
	
# ADD BASE CPP /Yu
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

DEP_CPP_FILES_MI=\
	"..\..\ngl\ps2\ngl_fixedstr.h"\
	"..\..\ngl\ps2\ngl_ps2.h"\
	"..\..\ngl\ps2\tim2.h"\
	"..\..\NSL\common\nl.h"\
	"..\..\NSL\common\nsl.h"\
	"..\..\NSL\common\ProjectOptions.h"\
	"..\..\NSL\gamecube\nl_gc.h"\
	"..\..\NSL\ps2\fifo_queue.cpp"\
	"..\..\NSL\ps2\fifo_queue.h"\
	"..\..\NSL\PS2\gas.h"\
	"..\..\NSL\PS2\gas_utility.h"\
	"..\..\NSL\PS2\nl_ps2.h"\
	"..\..\NSL\PS2\nsl_ps2.h"\
	"..\..\NSL\XBOX\nl_xbox.h"\
	"..\..\NSL\XBOX\nsl_xbox.h"\
	"..\..\NSL\XBOX\nsl_xbox_ext.h"\
	"..\..\NSL\XBOX\nsl_xbox_source.h"\
	"..\..\nsl\xbox\nsl_xbox_streams.h"\
	"..\..\NVL\ps2\nvl_ps2.h"\
	"..\..\NVL\ps2\nvlMPEG_ps2.h"\
	"..\..\NVL\ps2\nvlstream_ps2.h"\
	".\aggvertbuf.h"\
	".\ai_constants.h"\
	".\ai_interface.h"\
	".\ai_locomotion.h"\
	".\ai_polypath.h"\
	".\ai_polypath_cell.h"\
	".\ai_polypath_heap.h"\
	".\algebra.h"\
	".\anim.h"\
	".\anim_flavor.h"\
	".\anim_flavors.h"\
	".\anim_ids.h"\
	".\anim_maker.h"\
	".\app.h"\
	".\archalloc.h"\
	".\avltree.h"\
	".\beam.h"\
	".\beam_signals.h"\
	".\billboard.cpp"\
	".\billboard.h"\
	".\binary_tree.h"\
	".\blendmodes.h"\
	".\bone.h"\
	".\bound.h"\
	".\box_trigger_interface.h"\
	".\bp_tree.h"\
	".\bsp_collide.h"\
	".\bsp_tree.h"\
	".\camera.cpp"\
	".\camera.h"\
	".\capsule.cpp"\
	".\capsule.h"\
	".\cface.cpp"\
	".\cface.h"\
	".\chunkfile.h"\
	".\clipflags.h"\
	".\colgeom.cpp"\
	".\colgeom.h"\
	".\collide.cpp"\
	".\collide.h"\
	".\colmesh.h"\
	".\color.h"\
	".\commands.h"\
	".\conglom.h"\
	".\console.h"\
	".\constants.h"\
	".\controller.h"\
	".\convex_box.cpp"\
	".\convex_box.h"\
	".\custom_stl.h"\
	".\debug.h"\
	".\debug_render.cpp"\
	".\debug_render.h"\
	".\devoptflags.h"\
	".\devoptints.h"\
	".\devoptstrs.h"\
	".\element.cpp"\
	".\element.h"\
	".\entflavor.h"\
	".\entity.h"\
	".\entity_anim.h"\
	".\entity_interface.h"\
	".\entity_maker.h"\
	".\entity_signals.h"\
	".\entityflags.h"\
	".\entityid.h"\
	".\errorcontext.h"\
	".\face.h"\
	".\faceflags.h"\
	".\fast_vector.h"\
	".\fcs.cpp"\
	".\fcs.h"\
	".\file.h"\
	".\file_finder.h"\
	".\filespec.h"\
	".\forceflags.h"\
	".\frame_info.h"\
	".\game.h"\
	".\game_info.h"\
	".\game_info_vars.h"\
	".\game_process.h"\
	".\gcglobals.h"\
	".\generator.cpp"\
	".\generator.h"\
	".\geomgr.cpp"\
	".\geomgr.h"\
	".\global.h"\
	".\global_signals.h"\
	".\graph.h"\
	".\guidance_sys.h"\
	".\hinge.h"\
	".\hull.h"\
	".\hwmath.h"\
	".\hwmovieplayer.h"\
	".\hwrasterize.h"\
	".\hyperplane.h"\
	".\ini_parser.h"\
	".\inputmgr.h"\
	".\instance.h"\
	".\iri.h"\
	".\item.h"\
	".\item_signals.h"\
	".\joint.cpp"\
	".\joint.h"\
	".\joystick.h"\
	".\keyboard.h"\
	".\ks\camera_tool.cpp"\
	".\ks\cheatmenu.cpp"\
	".\ks\GlobalData.cpp"\
	".\ks\ksheaps.h"\
	".\kshooks.h"\
	".\ksnsl.h"\
	".\ksnvl.h"\
	".\lensflare.cpp"\
	".\lensflare.h"\
	".\light.cpp"\
	".\light.h"\
	".\lightmgr.cpp"\
	".\lightmgr.h"\
	".\linear_anim.h"\
	".\link_interface.h"\
	".\localize.cpp"\
	".\localize.h"\
	".\map_e.h"\
	".\material.h"\
	".\matfac.cpp"\
	".\matfac.h"\
	".\maxiri.h"\
	".\maxskinbones.h"\
	".\mcs.cpp"\
	".\mcs.h"\
	".\menusound.cpp"\
	".\meshrefs.h"\
	".\mic.h"\
	".\mobject.h"\
	".\msgboard.cpp"\
	".\msgboard.h"\
	".\mustash.h"\
	".\ngl.h"\
	".\ngl_support.h"\
	".\osalloc.h"\
	".\osassert.h"\
	".\osdevopts.h"\
	".\oserrmsg.h"\
	".\osfile.h"\
	".\ostimer.h"\
	".\particle.cpp"\
	".\particle.h"\
	".\particlecleaner.cpp"\
	".\particlecleaner.h"\
	".\path.h"\
	".\pc_port.h"\
	".\physical_interface.h"\
	".\plane.h"\
	".\platform_defines.h"\
	".\pmesh.cpp"\
	".\pmesh.h"\
	".\po.h"\
	".\po_anim.h"\
	".\portal.cpp"\
	".\portal.h"\
	".\profiler.h"\
	".\projconst.h"\
	".\project.h"\
	".\pstring.h"\
	".\random.h"\
	".\rect.h"\
	".\refptr.h"\
	".\region.cpp"\
	".\region.h"\
	".\region_graph.h"\
	".\render_data.h"\
	".\renderflav.h"\
	".\scene_anim.h"\
	".\script_lib_mfg.h"\
	".\script_library_class.h"\
	".\script_mfg_signals.h"\
	".\script_object.h"\
	".\semaphores.cpp"\
	".\semaphores.h"\
	".\shock_mods.h"\
	".\signal_anim.h"\
	".\signals.h"\
	".\SimpleAssert.h"\
	".\singleton.h"\
	".\so_data_block.h"\
	".\sphere.h"\
	".\stashes.h"\
	".\staticmem.h"\
	".\stl_adapter.h"\
	".\stringx.h"\
	".\terrain.cpp"\
	".\terrain.h"\
	".\text_font.h"\
	".\textfile.h"\
	".\time_interface.h"\
	".\timer.h"\
	".\txtcoord.h"\
	".\types.h"\
	".\usefulmath.h"\
	".\users.h"\
	".\vector_alloc.h"\
	".\vert.h"\
	".\vertnorm.h"\
	".\vertwork.h"\
	".\visrep.cpp"\
	".\visrep.h"\
	".\vm_executable.h"\
	".\vm_stack.h"\
	".\vm_symbol.h"\
	".\vm_symbol_list.h"\
	".\vm_thread.h"\
	".\vsplit.h"\
	".\warnlvl.h"\
	".\wds.h"\
	".\wedge.h"\
	".\widget.h"\
	".\widget_entity.cpp"\
	".\widget_entity.h"\
	".\widget_script.cpp"\
	".\widget_script.h"\
	".\xbglobals.h"\
	
NODEP_CPP_FILES_MI=\
	"..\..\NSL\pc\nl_pc.h"\
	"..\..\NSL\pc\nsl_pc_ext.h"\
	"..\..\NSL\XBOX\nsl.h"\
	".\algo.h"\
	".\defalloc.h"\
	".\ekernel.h"\
	".\eregs.h"\
	".\estruct.h"\
	".\etypes.h"\
	".\gc_arammgr.h"\
	".\graphics.h"\
	".\hwosgc\nsl.h"\
	".\hwosmks\mks_devopts.h"\
	".\hwosmks\mks_errmsg.h"\
	".\hwosmks\mks_movieplayer.h"\
	".\hwosmks\set5_rasterize.h"\
	".\hwosmks\set5_timer.h"\
	".\hwosmks\sh4_math.h"\
	".\hwosmks\sy_alloc.h"\
	".\hwosmks\sy_file.h"\
	".\hwosnglpc\algebra.h"\
	".\hwosnglpc\pc_algebra.h"\
	".\hwosnull\null_alloc.h"\
	".\hwosnull\null_devopts.h"\
	".\hwosnull\null_errmsg.h"\
	".\hwosnull\null_file.h"\
	".\hwosnull\null_math.h"\
	".\hwosnull\null_movieplayer.h"\
	".\hwosnull\null_rasterize.h"\
	".\hwosnull\null_timer.h"\
	".\hwospc\d3d_rasterize.h"\
	".\hwospc\pc_algebra.h"\
	".\hwospc\pc_movieplayer.h"\
	".\hwospc\w32_alloc.h"\
	".\hwospc\w32_devopts.h"\
	".\hwospc\w32_errmsg.h"\
	".\hwospc\w32_file.h"\
	".\hwospc\w32_timer.h"\
	".\hwospc\x86_math.h"\
	".\hwosxb\ngl_xbox.h"\
	".\hwosxb\nvl_xbox.h"\
	".\ibcdvd.h"\
	".\ibdma.h"\
	".\ibgraph.h"\
	".\ibipu.h"\
	".\ibmc.h"\
	".\ibmpeg.h"\
	".\ibpad.h"\
	".\ibpc.h"\
	".\ibusbkb.h"\
	".\ibvu0.h"\
	".\ifdev.h"\
	".\LipSync_Anim.h"\
	".\ngl_xbox.h"\
	".\nvl_gc.h"\
	".\nvl_xbox.h"\
	".\nvlstream_gc.h"\
	".\s\AccompFrontEnd.h"\
	".\s\andersen_ind_anims_mac.h"\
	".\s\beach.h"\
	".\s\beachdata.h"\
	".\s\BeachFrontEnd.h"\
	".\s\board.h"\
	".\s\board_anims_mac.h"\
	".\s\boarddata.h"\
	".\s\BoardFrontEnd.h"\
	".\s\camera_tool.h"\
	".\s\career.h"\
	".\s\careerdata.h"\
	".\s\carrol_ind_anims_mac.h"\
	".\s\challenge.h"\
	".\s\challenge_icon.h"\
	".\s\challenge_photo.h"\
	".\s\cheat.h"\
	".\s\CheatFrontEnd.h"\
	".\s\compressedphoto.h"\
	".\s\coords.h"\
	".\s\curren_ind_anims_mac.h"\
	".\s\eventmanager.h"\
	".\s\ExtrasFrontEnd.h"\
	".\s\FEAnim.h"\
	".\s\FEEntityManager.h"\
	".\s\FEMenu.h"\
	".\s\FEPanel.h"\
	".\s\fletcher_ind_anims_mac.h"\
	".\s\floatobj.h"\
	".\s\frankenreiter_ind_anims_mac.h"\
	".\s\FrontEndManager.h"\
	".\s\FrontEndMenus.h"\
	".\s\GameData.h"\
	".\s\GlobalData.h"\
	".\s\globaltextenum.h"\
	".\s\GraphicalMenuSystem.h"\
	".\s\HighScoreFrontEnd.h"\
	".\s\igo_widget.h"\
	".\s\igo_widget_analogclock.h"\
	".\s\igo_widget_balance.h"\
	".\s\igo_widget_camera.h"\
	".\s\igo_widget_fanmeter.h"\
	".\s\igo_widget_iconradar.h"\
	".\s\igo_widget_photo.h"\
	".\s\igo_widget_replay.h"\
	".\s\igo_widget_simple.h"\
	".\s\igo_widget_specialmeter.h"\
	".\s\igo_widget_splitclock.h"\
	".\s\igo_widget_splitmeter.h"\
	".\s\igo_widget_splitscore.h"\
	".\s\igo_widget_splitter.h"\
	".\s\igo_widget_timeattack.h"\
	".\s\igo_widget_waveindicator.h"\
	".\s\IGOFrontEnd.h"\
	".\s\igohintmanager.h"\
	".\s\igoiconmanager.h"\
	".\s\igolearn_new_trickmanager.h"\
	".\s\ik_object.h"\
	".\s\irons_ind_anims_mac.h"\
	".\s\judge.h"\
	".\s\kellyslater_controller.h"\
	".\s\kellyslater_shared_anims_mac.h"\
	".\s\kellyslater_states_mac.h"\
	".\s\ks_camera.h"\
	".\s\ksdbmenu.h"\
	".\s\ksngl.h"\
	".\s\ksreplay.h"\
	".\s\machado_ind_anims_mac.h"\
	".\s\MainFrontEnd.h"\
	".\s\Map.h"\
	".\s\MCDetectFrontEnd.h"\
	".\s\menu.h"\
	".\s\menusys.h"\
	".\s\mode.h"\
	".\s\mode_headtohead.h"\
	".\s\mode_meterattack.h"\
	".\s\mode_push.h"\
	".\s\mode_timeattack.h"\
	".\s\MultiFrontEnd.h"\
	".\s\MusicMan.h"\
	".\s\ode.h"\
	".\s\osGameSaver.h"\
	".\s\PAL60FrontEnd.h"\
	".\s\PathData.h"\
	".\s\PhotoFrontEnd.h"\
	".\s\physics.h"\
	".\s\player.h"\
	".\s\replay.h"\
	".\s\robb_ind_anims_mac.h"\
	".\s\rumbleManager.h"\
	".\s\SaveLoadFrontEnd.h"\
	".\s\scoringmanager.h"\
	".\s\simple_list.h"\
	".\s\slater_ind_anims_mac.h"\
	".\s\sounddata.h"\
	".\s\SoundScript.h"\
	".\s\specialmeter.h"\
	".\s\StatsFrontEnd.h"\
	".\s\surferdata.h"\
	".\s\SurferFrontEnd.h"\
	".\s\trick_system.h"\
	".\s\TrickBookFrontEnd.h"\
	".\s\trickdata.h"\
	".\s\trickregion.h"\
	".\s\tricks.h"\
	".\s\TutorialFrontEnd.h"\
	".\s\tutorialmanager.h"\
	".\s\VOEngine.h"\
	".\s\wave.h"\
	".\s\wavebreak.txt"\
	".\s\wavebreakstage.txt"\
	".\s\waveemitter.txt"\
	".\s\waveenum.h"\
	".\s\wavemarker.txt"\
	".\s\waveregion.txt"\
	".\s\wavestage.txt"\
	".\stl_heap.h"\
	".\wosgc\gc_algebra.h"\
	".\wosgc\gc_alloc.h"\
	".\wosgc\gc_errmsg.h"\
	".\wosgc\gc_file.h"\
	".\WOSGC\gc_gamesaver.h"\
	".\wosgc\gc_graphics.h"\
	".\wosgc\gc_input.h"\
	".\wosgc\gc_math.h"\
	".\wosgc\gc_movieplayer.h"\
	".\wosgc\gc_rasterize.h"\
	".\wosgc\gc_texturemgr.h"\
	".\wosgc\gc_timer.h"\
	".\wosgc\ngl_fixedstr.h"\
	".\wosgc\ngl_gc.h"\
	".\wosgc\ngl_version.h"\
	".\wosgc\nvl_gc.h"\
	".\WOSPS2\ps2_algebra.h"\
	".\WOSPS2\ps2_alloc.h"\
	".\WOSPS2\ps2_devopts.h"\
	".\WOSPS2\ps2_errmsg.h"\
	".\WOSPS2\ps2_file.h"\
	".\WOSPS2\ps2_gamesaver.h"\
	".\WOSPS2\ps2_graphics.h"\
	".\WOSPS2\ps2_input.h"\
	".\WOSPS2\ps2_m2vplayer.h"\
	".\WOSPS2\ps2_math.h"\
	".\WOSPS2\ps2_movieplayer.h"\
	".\WOSPS2\ps2_rasterize.h"\
	".\WOSPS2\ps2_texturemgr.h"\
	".\WOSPS2\ps2_timer.h"\
	".\WOSPS2\ps2_types.h"\
	".\wosxb\xb_algebra.h"\
	".\wosxb\xb_alloc.h"\
	".\wosxb\xb_errmsg.h"\
	".\wosxb\xb_file.h"\
	".\WOSXB\xb_gamesaver.h"\
	".\wosxb\xb_graphics.h"\
	".\wosxb\xb_input.h"\
	".\wosxb\xb_math.h"\
	".\wosxb\xb_movieplayer.h"\
	".\WOSXB\xb_portDVDCache.h"\
	".\wosxb\xb_rasterize.h"\
	".\wosxb\xb_string.h"\
	".\wosxb\xb_texturemgr.h"\
	".\wosxb\xb_timer.h"\
	".\wosxb\xb_types.h"\
	
# SUBTRACT BASE CPP /YX /Yc /Yu
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

DEP_CPP_FILES_MI=\
	"..\..\ngl\ps2\ngl_fixedstr.h"\
	"..\..\ngl\ps2\ngl_ps2.h"\
	"..\..\ngl\ps2\tim2.h"\
	"..\..\NSL\common\nl.h"\
	"..\..\NSL\common\nsl.h"\
	"..\..\NSL\common\ProjectOptions.h"\
	"..\..\NSL\gamecube\nl_gc.h"\
	"..\..\NSL\ps2\fifo_queue.cpp"\
	"..\..\NSL\ps2\fifo_queue.h"\
	"..\..\NSL\PS2\gas.h"\
	"..\..\NSL\PS2\gas_utility.h"\
	"..\..\NSL\PS2\nl_ps2.h"\
	"..\..\NSL\PS2\nsl_ps2.h"\
	"..\..\NSL\XBOX\nl_xbox.h"\
	"..\..\NSL\XBOX\nsl_xbox.h"\
	"..\..\NSL\XBOX\nsl_xbox_ext.h"\
	"..\..\NSL\XBOX\nsl_xbox_source.h"\
	"..\..\nsl\xbox\nsl_xbox_streams.h"\
	"..\..\NVL\ps2\nvl_ps2.h"\
	"..\..\NVL\ps2\nvlMPEG_ps2.h"\
	"..\..\NVL\ps2\nvlstream_ps2.h"\
	".\aggvertbuf.h"\
	".\ai_constants.h"\
	".\ai_interface.h"\
	".\ai_locomotion.h"\
	".\ai_polypath.h"\
	".\ai_polypath_cell.h"\
	".\ai_polypath_heap.h"\
	".\algebra.h"\
	".\anim.h"\
	".\anim_flavor.h"\
	".\anim_flavors.h"\
	".\anim_ids.h"\
	".\anim_maker.h"\
	".\app.h"\
	".\archalloc.h"\
	".\avltree.h"\
	".\beam.h"\
	".\beam_signals.h"\
	".\billboard.cpp"\
	".\billboard.h"\
	".\binary_tree.h"\
	".\blendmodes.h"\
	".\bone.h"\
	".\bound.h"\
	".\box_trigger_interface.h"\
	".\bp_tree.h"\
	".\bsp_collide.h"\
	".\bsp_tree.h"\
	".\camera.cpp"\
	".\camera.h"\
	".\capsule.cpp"\
	".\capsule.h"\
	".\cface.cpp"\
	".\cface.h"\
	".\chunkfile.h"\
	".\clipflags.h"\
	".\colgeom.cpp"\
	".\colgeom.h"\
	".\collide.cpp"\
	".\collide.h"\
	".\colmesh.h"\
	".\color.h"\
	".\commands.h"\
	".\conglom.h"\
	".\console.h"\
	".\constants.h"\
	".\controller.h"\
	".\convex_box.cpp"\
	".\convex_box.h"\
	".\custom_stl.h"\
	".\debug.h"\
	".\debug_render.cpp"\
	".\debug_render.h"\
	".\devoptflags.h"\
	".\devoptints.h"\
	".\devoptstrs.h"\
	".\element.cpp"\
	".\element.h"\
	".\entflavor.h"\
	".\entity.h"\
	".\entity_anim.h"\
	".\entity_interface.h"\
	".\entity_maker.h"\
	".\entity_signals.h"\
	".\entityflags.h"\
	".\entityid.h"\
	".\errorcontext.h"\
	".\face.h"\
	".\faceflags.h"\
	".\fast_vector.h"\
	".\fcs.cpp"\
	".\fcs.h"\
	".\file.h"\
	".\file_finder.h"\
	".\filespec.h"\
	".\forceflags.h"\
	".\frame_info.h"\
	".\game.h"\
	".\game_info.h"\
	".\game_info_vars.h"\
	".\game_process.h"\
	".\gcglobals.h"\
	".\generator.cpp"\
	".\generator.h"\
	".\geomgr.cpp"\
	".\geomgr.h"\
	".\global.h"\
	".\global_signals.h"\
	".\graph.h"\
	".\guidance_sys.h"\
	".\hinge.h"\
	".\hull.h"\
	".\hwmath.h"\
	".\hwmovieplayer.h"\
	".\hwrasterize.h"\
	".\hyperplane.h"\
	".\ini_parser.h"\
	".\inputmgr.h"\
	".\instance.h"\
	".\iri.h"\
	".\item.h"\
	".\item_signals.h"\
	".\joint.cpp"\
	".\joint.h"\
	".\joystick.h"\
	".\keyboard.h"\
	".\ks\camera_tool.cpp"\
	".\ks\cheatmenu.cpp"\
	".\ks\GlobalData.cpp"\
	".\ks\ksheaps.h"\
	".\kshooks.h"\
	".\ksnsl.h"\
	".\ksnvl.h"\
	".\lensflare.cpp"\
	".\lensflare.h"\
	".\light.cpp"\
	".\light.h"\
	".\lightmgr.cpp"\
	".\lightmgr.h"\
	".\linear_anim.h"\
	".\link_interface.h"\
	".\localize.cpp"\
	".\localize.h"\
	".\map_e.h"\
	".\material.h"\
	".\matfac.cpp"\
	".\matfac.h"\
	".\maxiri.h"\
	".\maxskinbones.h"\
	".\mcs.cpp"\
	".\mcs.h"\
	".\menusound.cpp"\
	".\meshrefs.h"\
	".\mic.h"\
	".\mobject.h"\
	".\msgboard.cpp"\
	".\msgboard.h"\
	".\mustash.h"\
	".\ngl.h"\
	".\ngl_support.h"\
	".\osalloc.h"\
	".\osassert.h"\
	".\osdevopts.h"\
	".\oserrmsg.h"\
	".\osfile.h"\
	".\ostimer.h"\
	".\particle.cpp"\
	".\particle.h"\
	".\particlecleaner.cpp"\
	".\particlecleaner.h"\
	".\path.h"\
	".\pc_port.h"\
	".\physical_interface.h"\
	".\plane.h"\
	".\platform_defines.h"\
	".\pmesh.cpp"\
	".\pmesh.h"\
	".\po.h"\
	".\po_anim.h"\
	".\portal.cpp"\
	".\portal.h"\
	".\profiler.h"\
	".\projconst.h"\
	".\project.h"\
	".\pstring.h"\
	".\random.h"\
	".\rect.h"\
	".\refptr.h"\
	".\region.cpp"\
	".\region.h"\
	".\region_graph.h"\
	".\render_data.h"\
	".\renderflav.h"\
	".\scene_anim.h"\
	".\script_lib_mfg.h"\
	".\script_library_class.h"\
	".\script_mfg_signals.h"\
	".\script_object.h"\
	".\semaphores.cpp"\
	".\semaphores.h"\
	".\shock_mods.h"\
	".\signal_anim.h"\
	".\signals.h"\
	".\SimpleAssert.h"\
	".\singleton.h"\
	".\so_data_block.h"\
	".\sphere.h"\
	".\stashes.h"\
	".\staticmem.h"\
	".\stl_adapter.h"\
	".\stringx.h"\
	".\terrain.cpp"\
	".\terrain.h"\
	".\text_font.h"\
	".\textfile.h"\
	".\time_interface.h"\
	".\timer.h"\
	".\txtcoord.h"\
	".\types.h"\
	".\usefulmath.h"\
	".\users.h"\
	".\vector_alloc.h"\
	".\vert.h"\
	".\vertnorm.h"\
	".\vertwork.h"\
	".\visrep.cpp"\
	".\visrep.h"\
	".\vm_executable.h"\
	".\vm_stack.h"\
	".\vm_symbol.h"\
	".\vm_symbol_list.h"\
	".\vm_thread.h"\
	".\vsplit.h"\
	".\warnlvl.h"\
	".\wds.h"\
	".\wedge.h"\
	".\widget.h"\
	".\widget_entity.cpp"\
	".\widget_entity.h"\
	".\widget_script.cpp"\
	".\widget_script.h"\
	".\xbglobals.h"\
	
NODEP_CPP_FILES_MI=\
	"..\..\NSL\pc\nl_pc.h"\
	"..\..\NSL\pc\nsl_pc_ext.h"\
	".\algo.h"\
	".\defalloc.h"\
	".\ekernel.h"\
	".\eregs.h"\
	".\estruct.h"\
	".\etypes.h"\
	".\gc\nsl_gc.h"\
	".\gc_arammgr.h"\
	".\graphics.h"\
	".\hwosmks\mks_devopts.h"\
	".\hwosmks\mks_errmsg.h"\
	".\hwosmks\mks_movieplayer.h"\
	".\hwosmks\set5_rasterize.h"\
	".\hwosmks\set5_timer.h"\
	".\hwosmks\sh4_math.h"\
	".\hwosmks\sy_alloc.h"\
	".\hwosmks\sy_file.h"\
	".\hwosnglpc\algebra.h"\
	".\hwosnglpc\pc_algebra.h"\
	".\hwosnull\null_alloc.h"\
	".\hwosnull\null_devopts.h"\
	".\hwosnull\null_errmsg.h"\
	".\hwosnull\null_file.h"\
	".\hwosnull\null_math.h"\
	".\hwosnull\null_movieplayer.h"\
	".\hwosnull\null_rasterize.h"\
	".\hwosnull\null_timer.h"\
	".\hwospc\d3d_rasterize.h"\
	".\hwospc\pc_algebra.h"\
	".\hwospc\pc_movieplayer.h"\
	".\hwospc\w32_alloc.h"\
	".\hwospc\w32_devopts.h"\
	".\hwospc\w32_errmsg.h"\
	".\hwospc\w32_file.h"\
	".\hwospc\w32_timer.h"\
	".\hwospc\x86_math.h"\
	".\hwosxb\ngl_xbox.h"\
	".\hwosxb\nvl_xbox.h"\
	".\ibcdvd.h"\
	".\ibdma.h"\
	".\ibgraph.h"\
	".\ibipu.h"\
	".\ibmc.h"\
	".\ibmpeg.h"\
	".\ibpad.h"\
	".\ibpc.h"\
	".\ibusbkb.h"\
	".\ibvu0.h"\
	".\ifdev.h"\
	".\LipSync_Anim.h"\
	".\ngl_xbox.h"\
	".\nsl.h"\
	".\nvl_gc.h"\
	".\nvl_xbox.h"\
	".\nvlstream_gc.h"\
	".\s\AccompFrontEnd.h"\
	".\s\andersen_ind_anims_mac.h"\
	".\s\beach.h"\
	".\s\beachdata.h"\
	".\s\BeachFrontEnd.h"\
	".\s\board.h"\
	".\s\board_anims_mac.h"\
	".\s\boarddata.h"\
	".\s\BoardFrontEnd.h"\
	".\s\camera_tool.h"\
	".\s\career.h"\
	".\s\careerdata.h"\
	".\s\carrol_ind_anims_mac.h"\
	".\s\challenge.h"\
	".\s\challenge_icon.h"\
	".\s\challenge_photo.h"\
	".\s\cheat.h"\
	".\s\CheatFrontEnd.h"\
	".\s\compressedphoto.h"\
	".\s\coords.h"\
	".\s\curren_ind_anims_mac.h"\
	".\s\eventmanager.h"\
	".\s\ExtrasFrontEnd.h"\
	".\s\FEAnim.h"\
	".\s\FEEntityManager.h"\
	".\s\FEMenu.h"\
	".\s\FEPanel.h"\
	".\s\fletcher_ind_anims_mac.h"\
	".\s\floatobj.h"\
	".\s\frankenreiter_ind_anims_mac.h"\
	".\s\FrontEndManager.h"\
	".\s\FrontEndMenus.h"\
	".\s\GameData.h"\
	".\s\GlobalData.h"\
	".\s\globaltextenum.h"\
	".\s\GraphicalMenuSystem.h"\
	".\s\HighScoreFrontEnd.h"\
	".\s\igo_widget.h"\
	".\s\igo_widget_analogclock.h"\
	".\s\igo_widget_balance.h"\
	".\s\igo_widget_camera.h"\
	".\s\igo_widget_fanmeter.h"\
	".\s\igo_widget_iconcount.h"\
	".\s\igo_widget_iconradar.h"\
	".\s\igo_widget_meterchallenge.h"\
	".\s\igo_widget_objectalert.h"\
	".\s\igo_widget_photo.h"\
	".\s\igo_widget_replay.h"\
	".\s\igo_widget_simple.h"\
	".\s\igo_widget_skillchallenge.h"\
	".\s\igo_widget_specialmeter.h"\
	".\s\igo_widget_splitclock.h"\
	".\s\igo_widget_splitmeter.h"\
	".\s\igo_widget_splitscore.h"\
	".\s\igo_widget_splitter.h"\
	".\s\igo_widget_timeattack.h"\
	".\s\igo_widget_waveindicator.h"\
	".\s\IGOFrontEnd.h"\
	".\s\igohintmanager.h"\
	".\s\igoiconmanager.h"\
	".\s\igolearn_new_trickmanager.h"\
	".\s\ik_object.h"\
	".\s\irons_ind_anims_mac.h"\
	".\s\judge.h"\
	".\s\kellyslater_controller.h"\
	".\s\kellyslater_shared_anims_mac.h"\
	".\s\kellyslater_states_mac.h"\
	".\s\ks_camera.h"\
	".\s\ksdbmenu.h"\
	".\s\ksngl.h"\
	".\s\ksreplay.h"\
	".\s\LogbookFrontEnd.h"\
	".\s\machado_ind_anims_mac.h"\
	".\s\MainFrontEnd.h"\
	".\s\Map.h"\
	".\s\MCDetectFrontEnd.h"\
	".\s\menu.h"\
	".\s\menusys.h"\
	".\s\mode.h"\
	".\s\mode_headtohead.h"\
	".\s\mode_meterattack.h"\
	".\s\mode_push.h"\
	".\s\mode_timeattack.h"\
	".\s\MultiFrontEnd.h"\
	".\s\MusicMan.h"\
	".\s\ode.h"\
	".\s\osGameSaver.h"\
	".\s\PAL60FrontEnd.h"\
	".\s\PathData.h"\
	".\s\PhotoFrontEnd.h"\
	".\s\physics.h"\
	".\s\player.h"\
	".\s\replay.h"\
	".\s\robb_ind_anims_mac.h"\
	".\s\rumbleManager.h"\
	".\s\SaveLoadFrontEnd.h"\
	".\s\scoringmanager.h"\
	".\s\simple_list.h"\
	".\s\slater_ind_anims_mac.h"\
	".\s\sounddata.h"\
	".\s\SoundScript.h"\
	".\s\specialmeter.h"\
	".\s\StatsFrontEnd.h"\
	".\s\surferdata.h"\
	".\s\SurferFrontEnd.h"\
	".\s\trick_system.h"\
	".\s\TrickBookFrontEnd.h"\
	".\s\trickdata.h"\
	".\s\trickregion.h"\
	".\s\tricks.h"\
	".\s\tutorialdata.h"\
	".\s\TutorialFrontEnd.h"\
	".\s\tutorialmanager.h"\
	".\s\VOEngine.h"\
	".\s\wave.h"\
	".\s\wavebreak.txt"\
	".\s\wavebreakstage.txt"\
	".\s\waveemitter.txt"\
	".\s\waveenum.h"\
	".\s\wavemarker.txt"\
	".\s\waveregion.txt"\
	".\s\wavestage.txt"\
	".\stl_heap.h"\
	".\wosgc\gc_algebra.h"\
	".\wosgc\gc_alloc.h"\
	".\wosgc\gc_errmsg.h"\
	".\wosgc\gc_file.h"\
	".\WOSGC\gc_gamesaver.h"\
	".\wosgc\gc_graphics.h"\
	".\wosgc\gc_input.h"\
	".\wosgc\gc_math.h"\
	".\wosgc\gc_movieplayer.h"\
	".\wosgc\gc_rasterize.h"\
	".\wosgc\gc_texturemgr.h"\
	".\wosgc\gc_timer.h"\
	".\wosgc\ngl_fixedstr.h"\
	".\wosgc\ngl_gc.h"\
	".\wosgc\ngl_version.h"\
	".\wosgc\nvl_gc.h"\
	".\WOSPS2\ps2_algebra.h"\
	".\WOSPS2\ps2_alloc.h"\
	".\WOSPS2\ps2_devopts.h"\
	".\WOSPS2\ps2_errmsg.h"\
	".\WOSPS2\ps2_file.h"\
	".\WOSPS2\ps2_gamesaver.h"\
	".\WOSPS2\ps2_graphics.h"\
	".\WOSPS2\ps2_input.h"\
	".\WOSPS2\ps2_m2vplayer.h"\
	".\WOSPS2\ps2_math.h"\
	".\WOSPS2\ps2_movieplayer.h"\
	".\WOSPS2\ps2_rasterize.h"\
	".\WOSPS2\ps2_texturemgr.h"\
	".\WOSPS2\ps2_timer.h"\
	".\WOSPS2\ps2_types.h"\
	".\wosxb\xb_algebra.h"\
	".\wosxb\xb_alloc.h"\
	".\wosxb\xb_errmsg.h"\
	".\wosxb\xb_file.h"\
	".\WOSXB\xb_gamesaver.h"\
	".\wosxb\xb_graphics.h"\
	".\wosxb\xb_input.h"\
	".\wosxb\xb_math.h"\
	".\wosxb\xb_movieplayer.h"\
	".\WOSXB\xb_portDVDCache.h"\
	".\wosxb\xb_rasterize.h"\
	".\wosxb\xb_string.h"\
	".\wosxb\xb_texturemgr.h"\
	".\wosxb\xb_timer.h"\
	".\wosxb\xb_types.h"\
	
# SUBTRACT BASE CPP /YX /Yc /Yu
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\files_misfits.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

DEP_CPP_FILES_MIS=\
	"..\..\ngl\ps2\ngl_fixedstr.h"\
	"..\..\ngl\ps2\ngl_ps2.h"\
	"..\..\ngl\ps2\tim2.h"\
	"..\..\NSL\common\nl.h"\
	"..\..\NSL\common\nsl.h"\
	"..\..\NSL\common\ProjectOptions.h"\
	"..\..\NSL\gamecube\nl_gc.h"\
	"..\..\NSL\ps2\fifo_queue.cpp"\
	"..\..\NSL\ps2\fifo_queue.h"\
	"..\..\NSL\PS2\gas.h"\
	"..\..\NSL\PS2\gas_utility.h"\
	"..\..\NSL\PS2\nl_ps2.h"\
	"..\..\NSL\PS2\nsl_ps2.h"\
	"..\..\NSL\XBOX\nl_xbox.h"\
	"..\..\NSL\XBOX\nsl_xbox.h"\
	"..\..\NSL\XBOX\nsl_xbox_ext.h"\
	"..\..\NSL\XBOX\nsl_xbox_source.h"\
	"..\..\nsl\xbox\nsl_xbox_streams.h"\
	"..\..\NVL\ps2\nvl_ps2.h"\
	"..\..\NVL\ps2\nvlMPEG_ps2.h"\
	"..\..\NVL\ps2\nvlstream_ps2.h"\
	".\aggvertbuf.h"\
	".\ai_constants.h"\
	".\ai_interface.h"\
	".\ai_locomotion.h"\
	".\ai_polypath.h"\
	".\ai_polypath_cell.h"\
	".\ai_polypath_heap.h"\
	".\algebra.h"\
	".\anim.h"\
	".\anim_flavor.h"\
	".\anim_flavors.h"\
	".\anim_ids.h"\
	".\app.cpp"\
	".\app.h"\
	".\archalloc.h"\
	".\attribute_template.h"\
	".\avltree.h"\
	".\b_spline.h"\
	".\beam.h"\
	".\beam_signals.h"\
	".\billboard.h"\
	".\binary_tree.h"\
	".\bone.h"\
	".\bound.h"\
	".\box_trigger_interface.h"\
	".\bp_tree.h"\
	".\camera.h"\
	".\capsule.h"\
	".\cface.h"\
	".\character_hard_attribs.h"\
	".\character_soft_attribs.h"\
	".\chunkfile.h"\
	".\colgeom.h"\
	".\collide.h"\
	".\colmesh.h"\
	".\color.h"\
	".\commands.h"\
	".\conglom.cpp"\
	".\conglom.h"\
	".\constants.h"\
	".\controller.h"\
	".\convex_box.h"\
	".\custom_stl.h"\
	".\debug.h"\
	".\debug_render.h"\
	".\devoptflags.h"\
	".\devoptints.h"\
	".\devoptstrs.h"\
	".\dropshadow.h"\
	".\element.h"\
	".\entflavor.h"\
	".\entity.h"\
	".\entity_anim.h"\
	".\entity_hard_attribs.h"\
	".\entity_interface.h"\
	".\entity_maker.cpp"\
	".\entity_maker.h"\
	".\entity_signals.h"\
	".\entity_soft_attribs.h"\
	".\entityflags.h"\
	".\entityid.h"\
	".\errorcontext.h"\
	".\face.h"\
	".\faceflags.h"\
	".\fast_vector.h"\
	".\file.h"\
	".\file_finder.h"\
	".\file_manager.cpp"\
	".\file_manager.h"\
	".\filespec.h"\
	".\fogmgr.h"\
	".\forceflags.h"\
	".\frame_info.h"\
	".\game.h"\
	".\game_info_vars.h"\
	".\game_process.h"\
	".\gcglobals.h"\
	".\generator.h"\
	".\geomgr.h"\
	".\global.h"\
	".\global_signals.h"\
	".\graph.h"\
	".\guidance_sys.h"\
	".\hard_attrib_interface.h"\
	".\hull.h"\
	".\hwaudio.h"\
	".\hwmath.h"\
	".\hwmovieplayer.h"\
	".\hwrasterize.h"\
	".\hyperplane.h"\
	".\ini_parser.cpp"\
	".\ini_parser.h"\
	".\inputmgr.h"\
	".\instance.h"\
	".\iri.h"\
	".\item.h"\
	".\item_signals.h"\
	".\joystick.h"\
	".\ks\blur.cpp"\
	".\ks\ksfx.h"\
	".\ks\ksheaps.h"\
	".\ks\menu_scoring.cpp"\
	".\ks\menudrawmisc.txt"\
	".\ks\menudrawparticle.txt"\
	".\ks\menudrawwater.txt"\
	".\ks\water.h"\
	".\ks\wavetex.h"\
	".\kshooks.h"\
	".\ksnsl.h"\
	".\ksnvl.h"\
	".\lensflare.h"\
	".\light.h"\
	".\lightmgr.h"\
	".\linear_anim.h"\
	".\link_interface.h"\
	".\map_e.h"\
	".\marker.h"\
	".\material.h"\
	".\matfac.h"\
	".\maxiri.h"\
	".\maxskinbones.h"\
	".\menudraw.cpp"\
	".\meshrefs.h"\
	".\mic.h"\
	".\mobject.h"\
	".\msgboard.h"\
	".\mustash.h"\
	".\ngl.h"\
	".\ngl_support.h"\
	".\osalloc.h"\
	".\osassert.h"\
	".\osdevopts.h"\
	".\oserrmsg.h"\
	".\osfile.h"\
	".\ostimer.h"\
	".\particle.h"\
	".\particlecleaner.h"\
	".\path.h"\
	".\pc_port.h"\
	".\physical_interface.h"\
	".\plane.h"\
	".\platform_defines.h"\
	".\pmesh.h"\
	".\po.h"\
	".\po_anim.h"\
	".\polytube.h"\
	".\portal.h"\
	".\profiler.h"\
	".\projconst.h"\
	".\project.h"\
	".\pstring.h"\
	".\random.h"\
	".\rect.h"\
	".\refptr.h"\
	".\region.h"\
	".\region_graph.h"\
	".\render_data.h"\
	".\renderflav.h"\
	".\scene_anim.h"\
	".\script_controller_signals.h"\
	".\script_lib_controller.h"\
	".\script_lib_mfg.h"\
	".\script_library_class.h"\
	".\script_mfg_signals.h"\
	".\script_object.h"\
	".\signal_anim.h"\
	".\signals.h"\
	".\SimpleAssert.h"\
	".\singleton.h"\
	".\skeleton_interface.h"\
	".\sky.h"\
	".\so_data_block.h"\
	".\soft_attrib_interface.h"\
	".\sphere.h"\
	".\stashes.h"\
	".\staticmem.h"\
	".\stl_adapter.h"\
	".\stringx.h"\
	".\terrain.h"\
	".\text_font.h"\
	".\textfile.h"\
	".\timer.cpp"\
	".\timer.h"\
	".\trigger.h"\
	".\trigger_signals.h"\
	".\txtcoord.h"\
	".\types.h"\
	".\usefulmath.h"\
	".\users.h"\
	".\vector_alloc.h"\
	".\vert.h"\
	".\vertwork.h"\
	".\visrep.h"\
	".\vm_executable.h"\
	".\vm_symbol.h"\
	".\vm_symbol_list.h"\
	".\warnlvl.h"\
	".\wds.h"\
	".\wedge.h"\
	".\widget.h"\
	".\widget_entity.h"\
	".\xbglobals.h"\
	{$(INCLUDE)}"eekernel.h"\
	{$(INCLUDE)}"eeregs.h"\
	{$(INCLUDE)}"eestruct.h"\
	{$(INCLUDE)}"eetypes.h"\
	{$(INCLUDE)}"hwosgc\gc_algebra.h"\
	{$(INCLUDE)}"hwosgc\gc_alloc.h"\
	{$(INCLUDE)}"hwosgc\gc_audio.h"\
	{$(INCLUDE)}"hwosgc\gc_errmsg.h"\
	{$(INCLUDE)}"hwosgc\gc_file.h"\
	{$(INCLUDE)}"HWOSGC\gc_gamesaver.h"\
	{$(INCLUDE)}"hwosgc\gc_graphics.h"\
	{$(INCLUDE)}"hwosgc\gc_input.h"\
	{$(INCLUDE)}"hwosgc\gc_math.h"\
	{$(INCLUDE)}"hwosgc\gc_movieplayer.h"\
	{$(INCLUDE)}"hwosgc\gc_rasterize.h"\
	{$(INCLUDE)}"hwosgc\gc_texturemgr.h"\
	{$(INCLUDE)}"hwosgc\gc_timer.h"\
	{$(INCLUDE)}"hwosgc\ngl_fixedstr.h"\
	{$(INCLUDE)}"hwosgc\ngl_gc.h"\
	{$(INCLUDE)}"hwosgc\ngl_version.h"\
	{$(INCLUDE)}"hwosgc\nvl_gc.h"\
	{$(INCLUDE)}"HWOSPS2\gas.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_algebra.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_alloc.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_audio.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_devopts.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_errmsg.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_file.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_gamesaver.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_graphics.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_input.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_m2vplayer.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_math.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_movieplayer.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_rasterize.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_texturemgr.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_timer.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_types.h"\
	{$(INCLUDE)}"hwosxb\pstring.h"\
	{$(INCLUDE)}"hwosxb\xb_algebra.h"\
	{$(INCLUDE)}"hwosxb\xb_alloc.h"\
	{$(INCLUDE)}"hwosxb\xb_audio.h"\
	{$(INCLUDE)}"hwosxb\xb_errmsg.h"\
	{$(INCLUDE)}"hwosxb\xb_file.h"\
	{$(INCLUDE)}"HWOSXB\xb_gamesaver.h"\
	{$(INCLUDE)}"hwosxb\xb_graphics.h"\
	{$(INCLUDE)}"hwosxb\xb_input.h"\
	{$(INCLUDE)}"hwosxb\xb_math.h"\
	{$(INCLUDE)}"hwosxb\xb_movieplayer.h"\
	{$(INCLUDE)}"HWOSXB\xb_particle.h"\
	{$(INCLUDE)}"HWOSXB\xb_portDVDCache.h"\
	{$(INCLUDE)}"hwosxb\xb_rasterize.h"\
	{$(INCLUDE)}"hwosxb\xb_string.h"\
	{$(INCLUDE)}"hwosxb\xb_texturemgr.h"\
	{$(INCLUDE)}"hwosxb\xb_timer.h"\
	{$(INCLUDE)}"hwosxb\xb_types.h"\
	{$(INCLUDE)}"ks\AccompFrontEnd.h"\
	{$(INCLUDE)}"ks\andersen_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\beach.h"\
	{$(INCLUDE)}"ks\beachdata.h"\
	{$(INCLUDE)}"ks\BeachFrontEnd.h"\
	{$(INCLUDE)}"ks\blur.h"\
	{$(INCLUDE)}"ks\board.h"\
	{$(INCLUDE)}"ks\board_anims_mac.h"\
	{$(INCLUDE)}"ks\boarddata.h"\
	{$(INCLUDE)}"ks\BoardFrontEnd.h"\
	{$(INCLUDE)}"ks\career.h"\
	{$(INCLUDE)}"ks\careerdata.h"\
	{$(INCLUDE)}"ks\carrol_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\challenge.h"\
	{$(INCLUDE)}"ks\challenge_icon.h"\
	{$(INCLUDE)}"ks\challenge_photo.h"\
	{$(INCLUDE)}"ks\cheat.h"\
	{$(INCLUDE)}"ks\CheatFrontEnd.h"\
	{$(INCLUDE)}"ks\compressedphoto.h"\
	{$(INCLUDE)}"ks\coords.h"\
	{$(INCLUDE)}"ks\curren_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\eventmanager.h"\
	{$(INCLUDE)}"ks\ExtrasFrontEnd.h"\
	{$(INCLUDE)}"ks\FEAnim.h"\
	{$(INCLUDE)}"ks\FEEntityManager.h"\
	{$(INCLUDE)}"ks\FEMenu.h"\
	{$(INCLUDE)}"ks\FEPanel.h"\
	{$(INCLUDE)}"ks\fletcher_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\floatobj.h"\
	{$(INCLUDE)}"ks\frankenreiter_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\FrontEndManager.h"\
	{$(INCLUDE)}"ks\FrontEndMenus.h"\
	{$(INCLUDE)}"ks\GameData.h"\
	{$(INCLUDE)}"ks\GlobalData.h"\
	{$(INCLUDE)}"ks\globaltextenum.h"\
	{$(INCLUDE)}"ks\GraphicalMenuSystem.h"\
	{$(INCLUDE)}"ks\HighScoreFrontEnd.h"\
	{$(INCLUDE)}"ks\igo_widget.h"\
	{$(INCLUDE)}"ks\igo_widget_analogclock.h"\
	{$(INCLUDE)}"ks\igo_widget_balance.h"\
	{$(INCLUDE)}"ks\igo_widget_camera.h"\
	{$(INCLUDE)}"ks\igo_widget_fanmeter.h"\
	{$(INCLUDE)}"ks\igo_widget_grid.h"\
	{$(INCLUDE)}"ks\igo_widget_iconcount.h"\
	{$(INCLUDE)}"ks\igo_widget_iconradar.h"\
	{$(INCLUDE)}"ks\igo_widget_meterchallenge.h"\
	{$(INCLUDE)}"ks\igo_widget_objectalert.h"\
	{$(INCLUDE)}"ks\igo_widget_photo.h"\
	{$(INCLUDE)}"ks\igo_widget_replay.h"\
	{$(INCLUDE)}"ks\igo_widget_simple.h"\
	{$(INCLUDE)}"ks\igo_widget_skillchallenge.h"\
	{$(INCLUDE)}"ks\igo_widget_specialmeter.h"\
	{$(INCLUDE)}"ks\igo_widget_splitclock.h"\
	{$(INCLUDE)}"ks\igo_widget_splitmeter.h"\
	{$(INCLUDE)}"ks\igo_widget_splitscore.h"\
	{$(INCLUDE)}"ks\igo_widget_splitter.h"\
	{$(INCLUDE)}"ks\igo_widget_timeattack.h"\
	{$(INCLUDE)}"ks\igo_widget_waveindicator.h"\
	{$(INCLUDE)}"ks\IGOFrontEnd.h"\
	{$(INCLUDE)}"ks\igohintmanager.h"\
	{$(INCLUDE)}"ks\igoiconmanager.h"\
	{$(INCLUDE)}"ks\igolearn_new_trickmanager.h"\
	{$(INCLUDE)}"ks\ik_object.h"\
	{$(INCLUDE)}"ks\irons_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\judge.h"\
	{$(INCLUDE)}"ks\kellyslater_controller.h"\
	{$(INCLUDE)}"ks\kellyslater_shared_anims_mac.h"\
	{$(INCLUDE)}"ks\kellyslater_states_mac.h"\
	{$(INCLUDE)}"ks\ks_camera.h"\
	{$(INCLUDE)}"ks\ksdbmenu.h"\
	{$(INCLUDE)}"ks\ksngl.h"\
	{$(INCLUDE)}"ks\ksreplay.h"\
	{$(INCLUDE)}"ks\LogbookFrontEnd.h"\
	{$(INCLUDE)}"ks\machado_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\MainFrontEnd.h"\
	{$(INCLUDE)}"ks\Map.h"\
	{$(INCLUDE)}"ks\MCDetectFrontEnd.h"\
	{$(INCLUDE)}"ks\menu.h"\
	{$(INCLUDE)}"ks\menu_scoring.h"\
	{$(INCLUDE)}"ks\menusys.h"\
	{$(INCLUDE)}"ks\mode.h"\
	{$(INCLUDE)}"ks\mode_headtohead.h"\
	{$(INCLUDE)}"ks\mode_meterattack.h"\
	{$(INCLUDE)}"ks\mode_push.h"\
	{$(INCLUDE)}"ks\mode_timeattack.h"\
	{$(INCLUDE)}"ks\MultiFrontEnd.h"\
	{$(INCLUDE)}"ks\MusicMan.h"\
	{$(INCLUDE)}"ks\ode.h"\
	{$(INCLUDE)}"ks\osGameSaver.h"\
	{$(INCLUDE)}"ks\osparticle.h"\
	{$(INCLUDE)}"ks\PAL60FrontEnd.h"\
	{$(INCLUDE)}"ks\PathData.h"\
	{$(INCLUDE)}"ks\PhotoFrontEnd.h"\
	{$(INCLUDE)}"ks\physics.h"\
	{$(INCLUDE)}"ks\player.h"\
	{$(INCLUDE)}"ks\replay.h"\
	{$(INCLUDE)}"ks\robb_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\rumbleManager.h"\
	{$(INCLUDE)}"ks\SaveLoadFrontEnd.h"\
	{$(INCLUDE)}"ks\scoringmanager.h"\
	{$(INCLUDE)}"ks\slater_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\sounddata.h"\
	{$(INCLUDE)}"ks\SoundScript.h"\
	{$(INCLUDE)}"ks\specialmeter.h"\
	{$(INCLUDE)}"ks\StatsFrontEnd.h"\
	{$(INCLUDE)}"ks\surferdata.h"\
	{$(INCLUDE)}"ks\SurferFrontEnd.h"\
	{$(INCLUDE)}"ks\trick_system.h"\
	{$(INCLUDE)}"ks\TrickBookFrontEnd.h"\
	{$(INCLUDE)}"ks\trickdata.h"\
	{$(INCLUDE)}"ks\trickregion.h"\
	{$(INCLUDE)}"ks\tricks.h"\
	{$(INCLUDE)}"ks\tutorialdata.h"\
	{$(INCLUDE)}"ks\TutorialFrontEnd.h"\
	{$(INCLUDE)}"ks\tutorialmanager.h"\
	{$(INCLUDE)}"ks\underwtr.h"\
	{$(INCLUDE)}"ks\VOEngine.h"\
	{$(INCLUDE)}"ks\wave.h"\
	{$(INCLUDE)}"ks\wavebreak.txt"\
	{$(INCLUDE)}"ks\wavebreakstage.txt"\
	{$(INCLUDE)}"ks\waveemitter.txt"\
	{$(INCLUDE)}"ks\waveenum.h"\
	{$(INCLUDE)}"ks\wavemarker.txt"\
	{$(INCLUDE)}"ks\waveregion.txt"\
	{$(INCLUDE)}"ks\wavestage.txt"\
	{$(INCLUDE)}"libcdvd.h"\
	{$(INCLUDE)}"libdma.h"\
	{$(INCLUDE)}"libgraph.h"\
	{$(INCLUDE)}"libipu.h"\
	{$(INCLUDE)}"libmc.h"\
	{$(INCLUDE)}"libmpeg.h"\
	{$(INCLUDE)}"libpad.h"\
	{$(INCLUDE)}"libpc.h"\
	{$(INCLUDE)}"libusbkb.h"\
	{$(INCLUDE)}"libvu0.h"\
	{$(INCLUDE)}"sifdev.h"\
	{$(INCLUDE)}"xgraphics.h"\
	
NODEP_CPP_FILES_MIS=\
	"..\..\NSL\pc\nl_pc.h"\
	"..\..\NSL\pc\nsl_pc_ext.h"\
	".\algo.h"\
	".\defalloc.h"\
	".\gc_arammgr.h"\
	".\hwosmks\am_audio.h"\
	".\hwosmks\mks_devopts.h"\
	".\hwosmks\mks_errmsg.h"\
	".\hwosmks\mks_movieplayer.h"\
	".\hwosmks\set5_rasterize.h"\
	".\hwosmks\set5_timer.h"\
	".\hwosmks\sh4_math.h"\
	".\hwosmks\sy_alloc.h"\
	".\hwosmks\sy_file.h"\
	".\hwosnglpc\algebra.h"\
	".\hwosnglpc\pc_algebra.h"\
	".\hwosnull\null_alloc.h"\
	".\hwosnull\null_audio.h"\
	".\hwosnull\null_devopts.h"\
	".\hwosnull\null_errmsg.h"\
	".\hwosnull\null_file.h"\
	".\hwosnull\null_input.h"\
	".\hwosnull\null_math.h"\
	".\hwosnull\null_movieplayer.h"\
	".\hwosnull\null_rasterize.h"\
	".\hwosnull\null_timer.h"\
	".\hwospc\d3d_rasterize.h"\
	".\hwospc\ds_audio.h"\
	".\hwospc\pc_algebra.h"\
	".\hwospc\pc_movieplayer.h"\
	".\hwospc\w32_alloc.h"\
	".\hwospc\w32_devopts.h"\
	".\hwospc\w32_errmsg.h"\
	".\hwospc\w32_file.h"\
	".\hwospc\w32_timer.h"\
	".\hwospc\x86_math.h"\
	".\hwosxb\ngl_xbox.h"\
	".\hwosxb\nvl_xbox.h"\
	".\LipSync_Anim.h"\
	".\ngl_xbox.h"\
	".\nvl_gc.h"\
	".\nvl_xbox.h"\
	".\nvlstream_gc.h"\
	".\stl_heap.h"\
	".\users.cpp"\
	
# ADD BASE CPP /Yu
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

DEP_CPP_FILES_MIS=\
	"..\..\ngl\ps2\ngl_fixedstr.h"\
	"..\..\ngl\ps2\ngl_ps2.h"\
	"..\..\ngl\ps2\tim2.h"\
	"..\..\NSL\common\nl.h"\
	"..\..\NSL\common\nsl.h"\
	"..\..\NSL\common\ProjectOptions.h"\
	"..\..\NSL\gamecube\nl_gc.h"\
	"..\..\NSL\ps2\fifo_queue.cpp"\
	"..\..\NSL\ps2\fifo_queue.h"\
	"..\..\NSL\PS2\gas.h"\
	"..\..\NSL\PS2\gas_utility.h"\
	"..\..\NSL\PS2\nl_ps2.h"\
	"..\..\NSL\PS2\nsl_ps2.h"\
	"..\..\NSL\XBOX\nl_xbox.h"\
	"..\..\NSL\XBOX\nsl_xbox.h"\
	"..\..\NSL\XBOX\nsl_xbox_ext.h"\
	"..\..\NSL\XBOX\nsl_xbox_source.h"\
	"..\..\nsl\xbox\nsl_xbox_streams.h"\
	"..\..\NVL\ps2\nvl_ps2.h"\
	"..\..\NVL\ps2\nvlMPEG_ps2.h"\
	"..\..\NVL\ps2\nvlstream_ps2.h"\
	".\aggvertbuf.h"\
	".\ai_constants.h"\
	".\ai_interface.h"\
	".\ai_locomotion.h"\
	".\ai_polypath.h"\
	".\ai_polypath_cell.h"\
	".\ai_polypath_heap.h"\
	".\algebra.h"\
	".\anim.h"\
	".\anim_flavor.h"\
	".\anim_flavors.h"\
	".\anim_ids.h"\
	".\app.cpp"\
	".\app.h"\
	".\archalloc.h"\
	".\attribute_template.h"\
	".\avltree.h"\
	".\b_spline.h"\
	".\beam.h"\
	".\beam_signals.h"\
	".\billboard.h"\
	".\binary_tree.h"\
	".\bone.h"\
	".\bound.h"\
	".\box_trigger_interface.h"\
	".\bp_tree.h"\
	".\camera.h"\
	".\capsule.h"\
	".\cface.h"\
	".\character_hard_attribs.h"\
	".\character_soft_attribs.h"\
	".\chunkfile.h"\
	".\colgeom.h"\
	".\collide.h"\
	".\colmesh.h"\
	".\color.h"\
	".\commands.h"\
	".\conglom.cpp"\
	".\conglom.h"\
	".\constants.h"\
	".\controller.h"\
	".\convex_box.h"\
	".\custom_stl.h"\
	".\debug.h"\
	".\debug_render.h"\
	".\devoptflags.h"\
	".\devoptints.h"\
	".\devoptstrs.h"\
	".\dropshadow.h"\
	".\element.h"\
	".\entflavor.h"\
	".\entity.h"\
	".\entity_anim.h"\
	".\entity_hard_attribs.h"\
	".\entity_interface.h"\
	".\entity_maker.cpp"\
	".\entity_maker.h"\
	".\entity_signals.h"\
	".\entity_soft_attribs.h"\
	".\entityflags.h"\
	".\entityid.h"\
	".\errorcontext.h"\
	".\face.h"\
	".\faceflags.h"\
	".\fast_vector.h"\
	".\file.h"\
	".\file_finder.h"\
	".\file_manager.cpp"\
	".\file_manager.h"\
	".\filespec.h"\
	".\fogmgr.h"\
	".\forceflags.h"\
	".\frame_info.h"\
	".\game.h"\
	".\game_info_vars.h"\
	".\game_process.h"\
	".\gcglobals.h"\
	".\generator.h"\
	".\geomgr.h"\
	".\global.h"\
	".\global_signals.h"\
	".\graph.h"\
	".\guidance_sys.h"\
	".\hard_attrib_interface.h"\
	".\hull.h"\
	".\hwaudio.h"\
	".\hwmath.h"\
	".\hwmovieplayer.h"\
	".\hwrasterize.h"\
	".\hyperplane.h"\
	".\ini_parser.cpp"\
	".\ini_parser.h"\
	".\inputmgr.h"\
	".\instance.h"\
	".\iri.h"\
	".\item.h"\
	".\item_signals.h"\
	".\joystick.h"\
	".\ks\blur.cpp"\
	".\ks\ksfx.h"\
	".\ks\ksheaps.h"\
	".\ks\menu_scoring.cpp"\
	".\ks\menudrawmisc.txt"\
	".\ks\menudrawparticle.txt"\
	".\ks\menudrawwater.txt"\
	".\ks\water.h"\
	".\ks\wavetex.h"\
	".\kshooks.h"\
	".\ksnsl.h"\
	".\ksnvl.h"\
	".\lensflare.h"\
	".\light.h"\
	".\lightmgr.h"\
	".\linear_anim.h"\
	".\link_interface.h"\
	".\map_e.h"\
	".\marker.h"\
	".\material.h"\
	".\matfac.h"\
	".\maxiri.h"\
	".\maxskinbones.h"\
	".\menudraw.cpp"\
	".\meshrefs.h"\
	".\mic.h"\
	".\mobject.h"\
	".\msgboard.h"\
	".\mustash.h"\
	".\ngl.h"\
	".\ngl_support.h"\
	".\osalloc.h"\
	".\osassert.h"\
	".\osdevopts.h"\
	".\oserrmsg.h"\
	".\osfile.h"\
	".\ostimer.h"\
	".\particle.h"\
	".\particlecleaner.h"\
	".\path.h"\
	".\pc_port.h"\
	".\physical_interface.h"\
	".\plane.h"\
	".\platform_defines.h"\
	".\pmesh.h"\
	".\po.h"\
	".\po_anim.h"\
	".\polytube.h"\
	".\portal.h"\
	".\profiler.h"\
	".\projconst.h"\
	".\project.h"\
	".\pstring.h"\
	".\random.h"\
	".\rect.h"\
	".\refptr.h"\
	".\region.h"\
	".\region_graph.h"\
	".\render_data.h"\
	".\renderflav.h"\
	".\scene_anim.h"\
	".\script_controller_signals.h"\
	".\script_lib_controller.h"\
	".\script_lib_mfg.h"\
	".\script_library_class.h"\
	".\script_mfg_signals.h"\
	".\script_object.h"\
	".\signal_anim.h"\
	".\signals.h"\
	".\SimpleAssert.h"\
	".\singleton.h"\
	".\skeleton_interface.h"\
	".\sky.h"\
	".\so_data_block.h"\
	".\soft_attrib_interface.h"\
	".\sphere.h"\
	".\stashes.h"\
	".\staticmem.h"\
	".\stl_adapter.h"\
	".\stringx.h"\
	".\terrain.h"\
	".\text_font.h"\
	".\textfile.h"\
	".\timer.cpp"\
	".\timer.h"\
	".\trigger.h"\
	".\trigger_signals.h"\
	".\txtcoord.h"\
	".\types.h"\
	".\usefulmath.h"\
	".\users.h"\
	".\vector_alloc.h"\
	".\vert.h"\
	".\vertwork.h"\
	".\visrep.h"\
	".\vm_executable.h"\
	".\vm_symbol.h"\
	".\vm_symbol_list.h"\
	".\warnlvl.h"\
	".\wds.h"\
	".\wedge.h"\
	".\widget.h"\
	".\widget_entity.h"\
	".\xbglobals.h"\
	
NODEP_CPP_FILES_MIS=\
	"..\..\NSL\pc\nl_pc.h"\
	"..\..\NSL\pc\nsl_pc_ext.h"\
	"..\..\NSL\XBOX\nsl.h"\
	".\algo.h"\
	".\defalloc.h"\
	".\ekernel.h"\
	".\eregs.h"\
	".\estruct.h"\
	".\etypes.h"\
	".\gc_arammgr.h"\
	".\graphics.h"\
	".\hwosgc\nsl.h"\
	".\hwosmks\am_audio.h"\
	".\hwosmks\mks_devopts.h"\
	".\hwosmks\mks_errmsg.h"\
	".\hwosmks\mks_movieplayer.h"\
	".\hwosmks\set5_rasterize.h"\
	".\hwosmks\set5_timer.h"\
	".\hwosmks\sh4_math.h"\
	".\hwosmks\sy_alloc.h"\
	".\hwosmks\sy_file.h"\
	".\hwosnglpc\algebra.h"\
	".\hwosnglpc\pc_algebra.h"\
	".\hwosnull\null_alloc.h"\
	".\hwosnull\null_audio.h"\
	".\hwosnull\null_devopts.h"\
	".\hwosnull\null_errmsg.h"\
	".\hwosnull\null_file.h"\
	".\hwosnull\null_input.h"\
	".\hwosnull\null_math.h"\
	".\hwosnull\null_movieplayer.h"\
	".\hwosnull\null_rasterize.h"\
	".\hwosnull\null_timer.h"\
	".\hwospc\d3d_rasterize.h"\
	".\hwospc\ds_audio.h"\
	".\hwospc\pc_algebra.h"\
	".\hwospc\pc_movieplayer.h"\
	".\hwospc\w32_alloc.h"\
	".\hwospc\w32_devopts.h"\
	".\hwospc\w32_errmsg.h"\
	".\hwospc\w32_file.h"\
	".\hwospc\w32_timer.h"\
	".\hwospc\x86_math.h"\
	".\hwosxb\ngl_xbox.h"\
	".\hwosxb\nvl_xbox.h"\
	".\ibcdvd.h"\
	".\ibdma.h"\
	".\ibgraph.h"\
	".\ibipu.h"\
	".\ibmc.h"\
	".\ibmpeg.h"\
	".\ibpad.h"\
	".\ibpc.h"\
	".\ibusbkb.h"\
	".\ibvu0.h"\
	".\ifdev.h"\
	".\LipSync_Anim.h"\
	".\ngl_xbox.h"\
	".\nvl_gc.h"\
	".\nvl_xbox.h"\
	".\nvlstream_gc.h"\
	".\s\AccompFrontEnd.h"\
	".\s\andersen_ind_anims_mac.h"\
	".\s\beach.h"\
	".\s\beachdata.h"\
	".\s\BeachFrontEnd.h"\
	".\s\blur.h"\
	".\s\board.h"\
	".\s\board_anims_mac.h"\
	".\s\boarddata.h"\
	".\s\BoardFrontEnd.h"\
	".\s\career.h"\
	".\s\careerdata.h"\
	".\s\carrol_ind_anims_mac.h"\
	".\s\challenge.h"\
	".\s\challenge_icon.h"\
	".\s\challenge_photo.h"\
	".\s\cheat.h"\
	".\s\CheatFrontEnd.h"\
	".\s\compressedphoto.h"\
	".\s\coords.h"\
	".\s\curren_ind_anims_mac.h"\
	".\s\eventmanager.h"\
	".\s\ExtrasFrontEnd.h"\
	".\s\FEAnim.h"\
	".\s\FEEntityManager.h"\
	".\s\FEMenu.h"\
	".\s\FEPanel.h"\
	".\s\fletcher_ind_anims_mac.h"\
	".\s\floatobj.h"\
	".\s\frankenreiter_ind_anims_mac.h"\
	".\s\FrontEndManager.h"\
	".\s\FrontEndMenus.h"\
	".\s\GameData.h"\
	".\s\GlobalData.h"\
	".\s\globaltextenum.h"\
	".\s\GraphicalMenuSystem.h"\
	".\s\HighScoreFrontEnd.h"\
	".\s\igo_widget.h"\
	".\s\igo_widget_analogclock.h"\
	".\s\igo_widget_balance.h"\
	".\s\igo_widget_camera.h"\
	".\s\igo_widget_fanmeter.h"\
	".\s\igo_widget_iconradar.h"\
	".\s\igo_widget_photo.h"\
	".\s\igo_widget_replay.h"\
	".\s\igo_widget_simple.h"\
	".\s\igo_widget_specialmeter.h"\
	".\s\igo_widget_splitclock.h"\
	".\s\igo_widget_splitmeter.h"\
	".\s\igo_widget_splitscore.h"\
	".\s\igo_widget_splitter.h"\
	".\s\igo_widget_timeattack.h"\
	".\s\igo_widget_waveindicator.h"\
	".\s\IGOFrontEnd.h"\
	".\s\igohintmanager.h"\
	".\s\igoiconmanager.h"\
	".\s\igolearn_new_trickmanager.h"\
	".\s\ik_object.h"\
	".\s\irons_ind_anims_mac.h"\
	".\s\judge.h"\
	".\s\kellyslater_controller.h"\
	".\s\kellyslater_shared_anims_mac.h"\
	".\s\kellyslater_states_mac.h"\
	".\s\ks_camera.h"\
	".\s\ksdbmenu.h"\
	".\s\ksngl.h"\
	".\s\ksreplay.h"\
	".\s\machado_ind_anims_mac.h"\
	".\s\MainFrontEnd.h"\
	".\s\Map.h"\
	".\s\MCDetectFrontEnd.h"\
	".\s\menu.h"\
	".\s\menu_scoring.h"\
	".\s\menusys.h"\
	".\s\mode.h"\
	".\s\mode_headtohead.h"\
	".\s\mode_meterattack.h"\
	".\s\mode_push.h"\
	".\s\mode_timeattack.h"\
	".\s\MultiFrontEnd.h"\
	".\s\MusicMan.h"\
	".\s\ode.h"\
	".\s\osGameSaver.h"\
	".\s\osparticle.h"\
	".\s\PAL60FrontEnd.h"\
	".\s\PathData.h"\
	".\s\PhotoFrontEnd.h"\
	".\s\physics.h"\
	".\s\player.h"\
	".\s\replay.h"\
	".\s\robb_ind_anims_mac.h"\
	".\s\rumbleManager.h"\
	".\s\SaveLoadFrontEnd.h"\
	".\s\scoringmanager.h"\
	".\s\slater_ind_anims_mac.h"\
	".\s\sounddata.h"\
	".\s\SoundScript.h"\
	".\s\specialmeter.h"\
	".\s\StatsFrontEnd.h"\
	".\s\surferdata.h"\
	".\s\SurferFrontEnd.h"\
	".\s\trick_system.h"\
	".\s\TrickBookFrontEnd.h"\
	".\s\trickdata.h"\
	".\s\trickregion.h"\
	".\s\tricks.h"\
	".\s\TutorialFrontEnd.h"\
	".\s\tutorialmanager.h"\
	".\s\VOEngine.h"\
	".\s\wave.h"\
	".\s\wavebreak.txt"\
	".\s\wavebreakstage.txt"\
	".\s\waveemitter.txt"\
	".\s\waveenum.h"\
	".\s\wavemarker.txt"\
	".\s\waveregion.txt"\
	".\s\wavestage.txt"\
	".\stl_heap.h"\
	".\users.cpp"\
	".\wosgc\gc_algebra.h"\
	".\wosgc\gc_alloc.h"\
	".\wosgc\gc_audio.h"\
	".\wosgc\gc_errmsg.h"\
	".\wosgc\gc_file.h"\
	".\WOSGC\gc_gamesaver.h"\
	".\wosgc\gc_graphics.h"\
	".\wosgc\gc_input.h"\
	".\wosgc\gc_math.h"\
	".\wosgc\gc_movieplayer.h"\
	".\wosgc\gc_rasterize.h"\
	".\wosgc\gc_texturemgr.h"\
	".\wosgc\gc_timer.h"\
	".\wosgc\ngl_fixedstr.h"\
	".\wosgc\ngl_gc.h"\
	".\wosgc\ngl_version.h"\
	".\wosgc\nvl_gc.h"\
	".\WOSPS2\gas.h"\
	".\WOSPS2\ps2_algebra.h"\
	".\WOSPS2\ps2_alloc.h"\
	".\WOSPS2\ps2_audio.h"\
	".\WOSPS2\ps2_devopts.h"\
	".\WOSPS2\ps2_errmsg.h"\
	".\WOSPS2\ps2_file.h"\
	".\WOSPS2\ps2_gamesaver.h"\
	".\WOSPS2\ps2_graphics.h"\
	".\WOSPS2\ps2_input.h"\
	".\WOSPS2\ps2_m2vplayer.h"\
	".\WOSPS2\ps2_math.h"\
	".\WOSPS2\ps2_movieplayer.h"\
	".\WOSPS2\ps2_rasterize.h"\
	".\WOSPS2\ps2_texturemgr.h"\
	".\WOSPS2\ps2_timer.h"\
	".\WOSPS2\ps2_types.h"\
	".\wosxb\pstring.h"\
	".\wosxb\xb_algebra.h"\
	".\wosxb\xb_alloc.h"\
	".\wosxb\xb_audio.h"\
	".\wosxb\xb_errmsg.h"\
	".\wosxb\xb_file.h"\
	".\WOSXB\xb_gamesaver.h"\
	".\wosxb\xb_graphics.h"\
	".\wosxb\xb_input.h"\
	".\wosxb\xb_math.h"\
	".\wosxb\xb_movieplayer.h"\
	".\WOSXB\xb_particle.h"\
	".\WOSXB\xb_portDVDCache.h"\
	".\wosxb\xb_rasterize.h"\
	".\wosxb\xb_string.h"\
	".\wosxb\xb_texturemgr.h"\
	".\wosxb\xb_timer.h"\
	".\wosxb\xb_types.h"\
	
# SUBTRACT BASE CPP /YX /Yc /Yu
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

DEP_CPP_FILES_MIS=\
	"..\..\ngl\ps2\ngl_fixedstr.h"\
	"..\..\ngl\ps2\ngl_ps2.h"\
	"..\..\ngl\ps2\tim2.h"\
	"..\..\NSL\common\nl.h"\
	"..\..\NSL\common\nsl.h"\
	"..\..\NSL\common\ProjectOptions.h"\
	"..\..\NSL\gamecube\nl_gc.h"\
	"..\..\NSL\ps2\fifo_queue.cpp"\
	"..\..\NSL\ps2\fifo_queue.h"\
	"..\..\NSL\PS2\gas.h"\
	"..\..\NSL\PS2\gas_utility.h"\
	"..\..\NSL\PS2\nl_ps2.h"\
	"..\..\NSL\PS2\nsl_ps2.h"\
	"..\..\NSL\XBOX\nl_xbox.h"\
	"..\..\NSL\XBOX\nsl_xbox.h"\
	"..\..\NSL\XBOX\nsl_xbox_ext.h"\
	"..\..\NSL\XBOX\nsl_xbox_source.h"\
	"..\..\nsl\xbox\nsl_xbox_streams.h"\
	"..\..\NVL\ps2\nvl_ps2.h"\
	"..\..\NVL\ps2\nvlMPEG_ps2.h"\
	"..\..\NVL\ps2\nvlstream_ps2.h"\
	".\aggvertbuf.h"\
	".\ai_constants.h"\
	".\ai_interface.h"\
	".\ai_locomotion.h"\
	".\ai_polypath.h"\
	".\ai_polypath_cell.h"\
	".\ai_polypath_heap.h"\
	".\algebra.h"\
	".\anim.h"\
	".\anim_flavor.h"\
	".\anim_flavors.h"\
	".\anim_ids.h"\
	".\app.cpp"\
	".\app.h"\
	".\archalloc.h"\
	".\attribute_template.h"\
	".\avltree.h"\
	".\b_spline.h"\
	".\beam.h"\
	".\beam_signals.h"\
	".\billboard.h"\
	".\binary_tree.h"\
	".\bone.h"\
	".\bound.h"\
	".\box_trigger_interface.h"\
	".\bp_tree.h"\
	".\camera.h"\
	".\capsule.h"\
	".\cface.h"\
	".\character_hard_attribs.h"\
	".\character_soft_attribs.h"\
	".\chunkfile.h"\
	".\colgeom.h"\
	".\collide.h"\
	".\colmesh.h"\
	".\color.h"\
	".\commands.h"\
	".\conglom.cpp"\
	".\conglom.h"\
	".\constants.h"\
	".\controller.h"\
	".\convex_box.h"\
	".\custom_stl.h"\
	".\debug.h"\
	".\debug_render.h"\
	".\devoptflags.h"\
	".\devoptints.h"\
	".\devoptstrs.h"\
	".\dropshadow.h"\
	".\element.h"\
	".\entflavor.h"\
	".\entity.h"\
	".\entity_anim.h"\
	".\entity_hard_attribs.h"\
	".\entity_interface.h"\
	".\entity_maker.cpp"\
	".\entity_maker.h"\
	".\entity_signals.h"\
	".\entity_soft_attribs.h"\
	".\entityflags.h"\
	".\entityid.h"\
	".\errorcontext.h"\
	".\face.h"\
	".\faceflags.h"\
	".\fast_vector.h"\
	".\file.h"\
	".\file_finder.h"\
	".\file_manager.cpp"\
	".\file_manager.h"\
	".\filespec.h"\
	".\fogmgr.h"\
	".\forceflags.h"\
	".\frame_info.h"\
	".\game.h"\
	".\game_info_vars.h"\
	".\game_process.h"\
	".\gcglobals.h"\
	".\generator.h"\
	".\geomgr.h"\
	".\global.h"\
	".\global_signals.h"\
	".\graph.h"\
	".\guidance_sys.h"\
	".\hard_attrib_interface.h"\
	".\hull.h"\
	".\hwaudio.h"\
	".\hwmath.h"\
	".\hwmovieplayer.h"\
	".\hwrasterize.h"\
	".\hyperplane.h"\
	".\ini_parser.cpp"\
	".\ini_parser.h"\
	".\inputmgr.h"\
	".\instance.h"\
	".\iri.h"\
	".\item.h"\
	".\item_signals.h"\
	".\joystick.h"\
	".\ks\blur.cpp"\
	".\ks\ksfx.h"\
	".\ks\ksheaps.h"\
	".\ks\menu_scoring.cpp"\
	".\ks\menudrawmisc.txt"\
	".\ks\menudrawparticle.txt"\
	".\ks\menudrawwater.txt"\
	".\ks\water.h"\
	".\ks\wavetex.h"\
	".\kshooks.h"\
	".\ksnsl.h"\
	".\ksnvl.h"\
	".\lensflare.h"\
	".\light.h"\
	".\lightmgr.h"\
	".\linear_anim.h"\
	".\link_interface.h"\
	".\map_e.h"\
	".\marker.h"\
	".\material.h"\
	".\matfac.h"\
	".\maxiri.h"\
	".\maxskinbones.h"\
	".\menudraw.cpp"\
	".\meshrefs.h"\
	".\mic.h"\
	".\mobject.h"\
	".\msgboard.h"\
	".\mustash.h"\
	".\ngl.h"\
	".\ngl_support.h"\
	".\osalloc.h"\
	".\osassert.h"\
	".\osdevopts.h"\
	".\oserrmsg.h"\
	".\osfile.h"\
	".\ostimer.h"\
	".\particle.h"\
	".\particlecleaner.h"\
	".\path.h"\
	".\pc_port.h"\
	".\physical_interface.h"\
	".\plane.h"\
	".\platform_defines.h"\
	".\pmesh.h"\
	".\po.h"\
	".\po_anim.h"\
	".\polytube.h"\
	".\portal.h"\
	".\profiler.h"\
	".\projconst.h"\
	".\project.h"\
	".\pstring.h"\
	".\random.h"\
	".\rect.h"\
	".\refptr.h"\
	".\region.h"\
	".\region_graph.h"\
	".\render_data.h"\
	".\renderflav.h"\
	".\scene_anim.h"\
	".\script_controller_signals.h"\
	".\script_lib_controller.h"\
	".\script_lib_mfg.h"\
	".\script_library_class.h"\
	".\script_mfg_signals.h"\
	".\script_object.h"\
	".\signal_anim.h"\
	".\signals.h"\
	".\SimpleAssert.h"\
	".\singleton.h"\
	".\skeleton_interface.h"\
	".\sky.h"\
	".\so_data_block.h"\
	".\soft_attrib_interface.h"\
	".\sphere.h"\
	".\stashes.h"\
	".\staticmem.h"\
	".\stl_adapter.h"\
	".\stringx.h"\
	".\terrain.h"\
	".\text_font.h"\
	".\textfile.h"\
	".\timer.cpp"\
	".\timer.h"\
	".\trigger.h"\
	".\trigger_signals.h"\
	".\txtcoord.h"\
	".\types.h"\
	".\usefulmath.h"\
	".\users.h"\
	".\vector_alloc.h"\
	".\vert.h"\
	".\vertwork.h"\
	".\visrep.h"\
	".\vm_executable.h"\
	".\vm_symbol.h"\
	".\vm_symbol_list.h"\
	".\warnlvl.h"\
	".\wds.h"\
	".\wedge.h"\
	".\widget.h"\
	".\widget_entity.h"\
	".\xbglobals.h"\
	
NODEP_CPP_FILES_MIS=\
	"..\..\NSL\pc\nl_pc.h"\
	"..\..\NSL\pc\nsl_pc_ext.h"\
	".\algo.h"\
	".\defalloc.h"\
	".\ekernel.h"\
	".\eregs.h"\
	".\estruct.h"\
	".\etypes.h"\
	".\gc\nsl_gc.h"\
	".\gc_arammgr.h"\
	".\graphics.h"\
	".\hwosmks\am_audio.h"\
	".\hwosmks\mks_devopts.h"\
	".\hwosmks\mks_errmsg.h"\
	".\hwosmks\mks_movieplayer.h"\
	".\hwosmks\set5_rasterize.h"\
	".\hwosmks\set5_timer.h"\
	".\hwosmks\sh4_math.h"\
	".\hwosmks\sy_alloc.h"\
	".\hwosmks\sy_file.h"\
	".\hwosnglpc\algebra.h"\
	".\hwosnglpc\pc_algebra.h"\
	".\hwosnull\null_alloc.h"\
	".\hwosnull\null_audio.h"\
	".\hwosnull\null_devopts.h"\
	".\hwosnull\null_errmsg.h"\
	".\hwosnull\null_file.h"\
	".\hwosnull\null_input.h"\
	".\hwosnull\null_math.h"\
	".\hwosnull\null_movieplayer.h"\
	".\hwosnull\null_rasterize.h"\
	".\hwosnull\null_timer.h"\
	".\hwospc\d3d_rasterize.h"\
	".\hwospc\ds_audio.h"\
	".\hwospc\pc_algebra.h"\
	".\hwospc\pc_movieplayer.h"\
	".\hwospc\w32_alloc.h"\
	".\hwospc\w32_devopts.h"\
	".\hwospc\w32_errmsg.h"\
	".\hwospc\w32_file.h"\
	".\hwospc\w32_timer.h"\
	".\hwospc\x86_math.h"\
	".\hwosxb\ngl_xbox.h"\
	".\hwosxb\nvl_xbox.h"\
	".\ibcdvd.h"\
	".\ibdma.h"\
	".\ibgraph.h"\
	".\ibipu.h"\
	".\ibmc.h"\
	".\ibmpeg.h"\
	".\ibpad.h"\
	".\ibpc.h"\
	".\ibusbkb.h"\
	".\ibvu0.h"\
	".\ifdev.h"\
	".\LipSync_Anim.h"\
	".\ngl_xbox.h"\
	".\nsl.h"\
	".\nvl_gc.h"\
	".\nvl_xbox.h"\
	".\nvlstream_gc.h"\
	".\s\AccompFrontEnd.h"\
	".\s\andersen_ind_anims_mac.h"\
	".\s\beach.h"\
	".\s\beachdata.h"\
	".\s\BeachFrontEnd.h"\
	".\s\blur.h"\
	".\s\board.h"\
	".\s\board_anims_mac.h"\
	".\s\boarddata.h"\
	".\s\BoardFrontEnd.h"\
	".\s\career.h"\
	".\s\careerdata.h"\
	".\s\carrol_ind_anims_mac.h"\
	".\s\challenge.h"\
	".\s\challenge_icon.h"\
	".\s\challenge_photo.h"\
	".\s\cheat.h"\
	".\s\CheatFrontEnd.h"\
	".\s\compressedphoto.h"\
	".\s\coords.h"\
	".\s\curren_ind_anims_mac.h"\
	".\s\eventmanager.h"\
	".\s\ExtrasFrontEnd.h"\
	".\s\FEAnim.h"\
	".\s\FEEntityManager.h"\
	".\s\FEMenu.h"\
	".\s\FEPanel.h"\
	".\s\fletcher_ind_anims_mac.h"\
	".\s\floatobj.h"\
	".\s\frankenreiter_ind_anims_mac.h"\
	".\s\FrontEndManager.h"\
	".\s\FrontEndMenus.h"\
	".\s\GameData.h"\
	".\s\GlobalData.h"\
	".\s\globaltextenum.h"\
	".\s\GraphicalMenuSystem.h"\
	".\s\HighScoreFrontEnd.h"\
	".\s\igo_widget.h"\
	".\s\igo_widget_analogclock.h"\
	".\s\igo_widget_balance.h"\
	".\s\igo_widget_camera.h"\
	".\s\igo_widget_fanmeter.h"\
	".\s\igo_widget_iconcount.h"\
	".\s\igo_widget_iconradar.h"\
	".\s\igo_widget_meterchallenge.h"\
	".\s\igo_widget_objectalert.h"\
	".\s\igo_widget_photo.h"\
	".\s\igo_widget_replay.h"\
	".\s\igo_widget_simple.h"\
	".\s\igo_widget_skillchallenge.h"\
	".\s\igo_widget_specialmeter.h"\
	".\s\igo_widget_splitclock.h"\
	".\s\igo_widget_splitmeter.h"\
	".\s\igo_widget_splitscore.h"\
	".\s\igo_widget_splitter.h"\
	".\s\igo_widget_timeattack.h"\
	".\s\igo_widget_waveindicator.h"\
	".\s\IGOFrontEnd.h"\
	".\s\igohintmanager.h"\
	".\s\igoiconmanager.h"\
	".\s\igolearn_new_trickmanager.h"\
	".\s\ik_object.h"\
	".\s\irons_ind_anims_mac.h"\
	".\s\judge.h"\
	".\s\kellyslater_controller.h"\
	".\s\kellyslater_shared_anims_mac.h"\
	".\s\kellyslater_states_mac.h"\
	".\s\ks_camera.h"\
	".\s\ksdbmenu.h"\
	".\s\ksngl.h"\
	".\s\ksreplay.h"\
	".\s\LogbookFrontEnd.h"\
	".\s\machado_ind_anims_mac.h"\
	".\s\MainFrontEnd.h"\
	".\s\Map.h"\
	".\s\MCDetectFrontEnd.h"\
	".\s\menu.h"\
	".\s\menu_scoring.h"\
	".\s\menusys.h"\
	".\s\mode.h"\
	".\s\mode_headtohead.h"\
	".\s\mode_meterattack.h"\
	".\s\mode_push.h"\
	".\s\mode_timeattack.h"\
	".\s\MultiFrontEnd.h"\
	".\s\MusicMan.h"\
	".\s\ode.h"\
	".\s\osGameSaver.h"\
	".\s\osparticle.h"\
	".\s\PAL60FrontEnd.h"\
	".\s\PathData.h"\
	".\s\PhotoFrontEnd.h"\
	".\s\physics.h"\
	".\s\player.h"\
	".\s\replay.h"\
	".\s\robb_ind_anims_mac.h"\
	".\s\rumbleManager.h"\
	".\s\SaveLoadFrontEnd.h"\
	".\s\scoringmanager.h"\
	".\s\slater_ind_anims_mac.h"\
	".\s\sounddata.h"\
	".\s\SoundScript.h"\
	".\s\specialmeter.h"\
	".\s\StatsFrontEnd.h"\
	".\s\surferdata.h"\
	".\s\SurferFrontEnd.h"\
	".\s\trick_system.h"\
	".\s\TrickBookFrontEnd.h"\
	".\s\trickdata.h"\
	".\s\trickregion.h"\
	".\s\tricks.h"\
	".\s\tutorialdata.h"\
	".\s\TutorialFrontEnd.h"\
	".\s\tutorialmanager.h"\
	".\s\VOEngine.h"\
	".\s\wave.h"\
	".\s\wavebreak.txt"\
	".\s\wavebreakstage.txt"\
	".\s\waveemitter.txt"\
	".\s\waveenum.h"\
	".\s\wavemarker.txt"\
	".\s\waveregion.txt"\
	".\s\wavestage.txt"\
	".\stl_heap.h"\
	".\users.cpp"\
	".\wosgc\gc_algebra.h"\
	".\wosgc\gc_alloc.h"\
	".\wosgc\gc_audio.h"\
	".\wosgc\gc_errmsg.h"\
	".\wosgc\gc_file.h"\
	".\WOSGC\gc_gamesaver.h"\
	".\wosgc\gc_graphics.h"\
	".\wosgc\gc_input.h"\
	".\wosgc\gc_math.h"\
	".\wosgc\gc_movieplayer.h"\
	".\wosgc\gc_rasterize.h"\
	".\wosgc\gc_texturemgr.h"\
	".\wosgc\gc_timer.h"\
	".\wosgc\ngl_fixedstr.h"\
	".\wosgc\ngl_gc.h"\
	".\wosgc\ngl_version.h"\
	".\wosgc\nvl_gc.h"\
	".\WOSPS2\gas.h"\
	".\WOSPS2\ps2_algebra.h"\
	".\WOSPS2\ps2_alloc.h"\
	".\WOSPS2\ps2_audio.h"\
	".\WOSPS2\ps2_devopts.h"\
	".\WOSPS2\ps2_errmsg.h"\
	".\WOSPS2\ps2_file.h"\
	".\WOSPS2\ps2_gamesaver.h"\
	".\WOSPS2\ps2_graphics.h"\
	".\WOSPS2\ps2_input.h"\
	".\WOSPS2\ps2_m2vplayer.h"\
	".\WOSPS2\ps2_math.h"\
	".\WOSPS2\ps2_movieplayer.h"\
	".\WOSPS2\ps2_rasterize.h"\
	".\WOSPS2\ps2_texturemgr.h"\
	".\WOSPS2\ps2_timer.h"\
	".\WOSPS2\ps2_types.h"\
	".\wosxb\pstring.h"\
	".\wosxb\xb_algebra.h"\
	".\wosxb\xb_alloc.h"\
	".\wosxb\xb_audio.h"\
	".\wosxb\xb_errmsg.h"\
	".\wosxb\xb_file.h"\
	".\WOSXB\xb_gamesaver.h"\
	".\wosxb\xb_graphics.h"\
	".\wosxb\xb_input.h"\
	".\wosxb\xb_math.h"\
	".\wosxb\xb_movieplayer.h"\
	".\WOSXB\xb_particle.h"\
	".\WOSXB\xb_portDVDCache.h"\
	".\wosxb\xb_rasterize.h"\
	".\wosxb\xb_string.h"\
	".\wosxb\xb_texturemgr.h"\
	".\wosxb\xb_timer.h"\
	".\wosxb\xb_types.h"\
	
# SUBTRACT BASE CPP /YX /Yc /Yu
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\files_script.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

DEP_CPP_FILES_S=\
	"..\..\ngl\ps2\ngl_fixedstr.h"\
	"..\..\ngl\ps2\ngl_ps2.h"\
	"..\..\ngl\ps2\tim2.h"\
	"..\..\NSL\common\nl.h"\
	"..\..\NSL\common\nsl.h"\
	"..\..\NSL\common\ProjectOptions.h"\
	"..\..\NSL\gamecube\nl_gc.h"\
	"..\..\NSL\ps2\fifo_queue.cpp"\
	"..\..\NSL\ps2\fifo_queue.h"\
	"..\..\NSL\PS2\gas.h"\
	"..\..\NSL\PS2\gas_utility.h"\
	"..\..\NSL\PS2\nl_ps2.h"\
	"..\..\NSL\PS2\nsl_ps2.h"\
	"..\..\NSL\XBOX\nl_xbox.h"\
	"..\..\NSL\XBOX\nsl_xbox.h"\
	"..\..\NSL\XBOX\nsl_xbox_ext.h"\
	"..\..\NSL\XBOX\nsl_xbox_source.h"\
	"..\..\nsl\xbox\nsl_xbox_streams.h"\
	"..\..\NVL\ps2\nvl_ps2.h"\
	"..\..\NVL\ps2\nvlMPEG_ps2.h"\
	"..\..\NVL\ps2\nvlstream_ps2.h"\
	".\aggvertbuf.h"\
	".\ai_constants.h"\
	".\ai_interface.h"\
	".\ai_locomotion.h"\
	".\ai_polypath.h"\
	".\ai_polypath_cell.h"\
	".\ai_polypath_heap.h"\
	".\ai_voice.h"\
	".\algebra.h"\
	".\anim.h"\
	".\anim_flavor.h"\
	".\anim_flavors.h"\
	".\anim_ids.h"\
	".\app.h"\
	".\archalloc.h"\
	".\avltree.h"\
	".\beam.h"\
	".\beam_signals.h"\
	".\billboard.h"\
	".\bone.h"\
	".\bound.h"\
	".\box_trigger_interface.h"\
	".\camera.h"\
	".\capsule.h"\
	".\cface.h"\
	".\chunkfile.h"\
	".\colgeom.h"\
	".\collide.h"\
	".\colmesh.h"\
	".\color.h"\
	".\commands.h"\
	".\conglom.h"\
	".\console.h"\
	".\constants.h"\
	".\controller.h"\
	".\convex_box.h"\
	".\custom_stl.h"\
	".\debug.h"\
	".\debug_render.h"\
	".\devoptflags.h"\
	".\devoptints.h"\
	".\devoptstrs.h"\
	".\element.h"\
	".\entflavor.h"\
	".\entity.h"\
	".\entity_anim.h"\
	".\entity_interface.h"\
	".\entity_maker.h"\
	".\entity_signals.h"\
	".\entityflags.h"\
	".\entityid.h"\
	".\errorcontext.h"\
	".\face.h"\
	".\faceflags.h"\
	".\fast_vector.h"\
	".\file.h"\
	".\file_finder.h"\
	".\filespec.h"\
	".\fogmgr.h"\
	".\frame_info.h"\
	".\game.h"\
	".\game_info_vars.h"\
	".\game_process.h"\
	".\gcglobals.h"\
	".\generator.h"\
	".\geomgr.h"\
	".\global.h"\
	".\global_signals.h"\
	".\graph.h"\
	".\guidance_sys.h"\
	".\hull.h"\
	".\hwaudio.h"\
	".\hwmath.h"\
	".\hwmovieplayer.h"\
	".\hwrasterize.h"\
	".\hyperplane.h"\
	".\ini_parser.h"\
	".\inputmgr.h"\
	".\instance.h"\
	".\interface.h"\
	".\item.h"\
	".\item_signals.h"\
	".\joystick.h"\
	".\keyboard.h"\
	".\ks\ksheaps.h"\
	".\ks\SoundScript.cpp"\
	".\kshooks.h"\
	".\ksnsl.h"\
	".\ksnvl.h"\
	".\light.h"\
	".\lightmgr.h"\
	".\linear_anim.h"\
	".\link_interface.h"\
	".\localize.h"\
	".\map_e.h"\
	".\material.h"\
	".\matfac.h"\
	".\maxskinbones.h"\
	".\meshrefs.h"\
	".\mic.h"\
	".\mobject.h"\
	".\msgboard.h"\
	".\mustash.h"\
	".\ngl.h"\
	".\osalloc.h"\
	".\osassert.h"\
	".\osdevopts.h"\
	".\oserrmsg.h"\
	".\osfile.h"\
	".\ostimer.h"\
	".\path.h"\
	".\pc_port.h"\
	".\physical_interface.h"\
	".\plane.h"\
	".\platform_defines.h"\
	".\pmesh.h"\
	".\po.h"\
	".\po_anim.h"\
	".\portal.h"\
	".\profiler.h"\
	".\projconst.h"\
	".\project.h"\
	".\pstring.h"\
	".\random.h"\
	".\rect.h"\
	".\refptr.h"\
	".\region.h"\
	".\region_graph.h"\
	".\render_data.h"\
	".\renderflav.h"\
	".\scene_anim.h"\
	".\script_access.cpp"\
	".\script_access.h"\
	".\script_controller_signals.h"\
	".\script_lib.cpp"\
	".\script_lib_anim.cpp"\
	".\script_lib_anim.h"\
	".\script_lib_beam.cpp"\
	".\script_lib_beam.h"\
	".\script_lib_controller.cpp"\
	".\script_lib_controller.h"\
	".\script_lib_entity.cpp"\
	".\script_lib_entity.h"\
	".\script_lib_entity2.cpp"\
	".\script_lib_entity_widget.cpp"\
	".\script_lib_entity_widget.h"\
	".\script_lib_item.cpp"\
	".\script_lib_item.h"\
	".\script_lib_list.cpp"\
	".\script_lib_list.h"\
	".\script_lib_mfg.cpp"\
	".\script_lib_mfg.h"\
	".\script_lib_scene_anim.cpp"\
	".\script_lib_scene_anim.h"\
	".\script_lib_signal.cpp"\
	".\script_lib_signal.h"\
	".\script_lib_sound_inst.cpp"\
	".\script_lib_sound_inst.h"\
	".\script_lib_sound_stream.cpp"\
	".\script_lib_sound_stream.h"\
	".\script_lib_trigger.cpp"\
	".\script_lib_trigger.h"\
	".\script_lib_vector3d.cpp"\
	".\script_lib_vector3d.h"\
	".\script_lib_widget.cpp"\
	".\script_lib_widget.h"\
	".\script_library_class.h"\
	".\script_mfg_signals.h"\
	".\script_object.h"\
	".\script_register.cpp"\
	".\script_register.h"\
	".\signal_anim.h"\
	".\signals.h"\
	".\simple_classes.h"\
	".\SimpleAssert.h"\
	".\singleton.h"\
	".\so_data_block.h"\
	".\sound_group.h"\
	".\sound_interface.h"\
	".\sphere.h"\
	".\stashes.h"\
	".\staticmem.h"\
	".\stl_adapter.h"\
	".\stringx.h"\
	".\terrain.h"\
	".\text_font.h"\
	".\textfile.h"\
	".\time_interface.h"\
	".\timer.h"\
	".\trigger.h"\
	".\trigger_signals.h"\
	".\txtcoord.h"\
	".\types.h"\
	".\usefulmath.h"\
	".\users.h"\
	".\vert.h"\
	".\visrep.h"\
	".\vm_executable.h"\
	".\vm_stack.h"\
	".\vm_symbol.h"\
	".\vm_symbol_list.h"\
	".\vm_thread.h"\
	".\warnlvl.h"\
	".\wds.h"\
	".\wedge.h"\
	".\widget.h"\
	".\widget_entity.h"\
	".\widget_script.h"\
	".\xbglobals.h"\
	{$(INCLUDE)}"eekernel.h"\
	{$(INCLUDE)}"eeregs.h"\
	{$(INCLUDE)}"eestruct.h"\
	{$(INCLUDE)}"eetypes.h"\
	{$(INCLUDE)}"hwosgc\gc_algebra.h"\
	{$(INCLUDE)}"hwosgc\gc_alloc.h"\
	{$(INCLUDE)}"hwosgc\gc_audio.h"\
	{$(INCLUDE)}"hwosgc\gc_errmsg.h"\
	{$(INCLUDE)}"hwosgc\gc_file.h"\
	{$(INCLUDE)}"HWOSGC\gc_gamesaver.h"\
	{$(INCLUDE)}"hwosgc\gc_graphics.h"\
	{$(INCLUDE)}"hwosgc\gc_input.h"\
	{$(INCLUDE)}"hwosgc\gc_math.h"\
	{$(INCLUDE)}"hwosgc\gc_movieplayer.h"\
	{$(INCLUDE)}"hwosgc\gc_rasterize.h"\
	{$(INCLUDE)}"hwosgc\gc_texturemgr.h"\
	{$(INCLUDE)}"hwosgc\gc_timer.h"\
	{$(INCLUDE)}"hwosgc\ngl_fixedstr.h"\
	{$(INCLUDE)}"hwosgc\ngl_gc.h"\
	{$(INCLUDE)}"hwosgc\ngl_version.h"\
	{$(INCLUDE)}"hwosgc\nvl_gc.h"\
	{$(INCLUDE)}"HWOSPS2\gas.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_algebra.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_alloc.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_audio.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_devopts.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_errmsg.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_file.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_gamesaver.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_graphics.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_input.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_m2vplayer.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_math.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_movieplayer.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_rasterize.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_texturemgr.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_timer.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_types.h"\
	{$(INCLUDE)}"hwosxb\pstring.h"\
	{$(INCLUDE)}"hwosxb\xb_algebra.h"\
	{$(INCLUDE)}"hwosxb\xb_alloc.h"\
	{$(INCLUDE)}"hwosxb\xb_audio.h"\
	{$(INCLUDE)}"hwosxb\xb_errmsg.h"\
	{$(INCLUDE)}"hwosxb\xb_file.h"\
	{$(INCLUDE)}"HWOSXB\xb_gamesaver.h"\
	{$(INCLUDE)}"hwosxb\xb_graphics.h"\
	{$(INCLUDE)}"hwosxb\xb_input.h"\
	{$(INCLUDE)}"hwosxb\xb_math.h"\
	{$(INCLUDE)}"hwosxb\xb_movieplayer.h"\
	{$(INCLUDE)}"HWOSXB\xb_portDVDCache.h"\
	{$(INCLUDE)}"hwosxb\xb_rasterize.h"\
	{$(INCLUDE)}"hwosxb\xb_string.h"\
	{$(INCLUDE)}"hwosxb\xb_texturemgr.h"\
	{$(INCLUDE)}"hwosxb\xb_timer.h"\
	{$(INCLUDE)}"hwosxb\xb_types.h"\
	{$(INCLUDE)}"ks\AccompFrontEnd.h"\
	{$(INCLUDE)}"ks\andersen_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\beach.h"\
	{$(INCLUDE)}"ks\beachdata.h"\
	{$(INCLUDE)}"ks\BeachFrontEnd.h"\
	{$(INCLUDE)}"ks\board.h"\
	{$(INCLUDE)}"ks\board_anims_mac.h"\
	{$(INCLUDE)}"ks\boarddata.h"\
	{$(INCLUDE)}"ks\BoardFrontEnd.h"\
	{$(INCLUDE)}"ks\career.h"\
	{$(INCLUDE)}"ks\careerdata.h"\
	{$(INCLUDE)}"ks\carrol_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\challenge.h"\
	{$(INCLUDE)}"ks\challenge_icon.h"\
	{$(INCLUDE)}"ks\challenge_photo.h"\
	{$(INCLUDE)}"ks\cheat.h"\
	{$(INCLUDE)}"ks\CheatFrontEnd.h"\
	{$(INCLUDE)}"ks\compressedphoto.h"\
	{$(INCLUDE)}"ks\coords.h"\
	{$(INCLUDE)}"ks\curren_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\eventmanager.h"\
	{$(INCLUDE)}"ks\ExtrasFrontEnd.h"\
	{$(INCLUDE)}"ks\FEAnim.h"\
	{$(INCLUDE)}"ks\FEEntityManager.h"\
	{$(INCLUDE)}"ks\FEMenu.h"\
	{$(INCLUDE)}"ks\FEPanel.h"\
	{$(INCLUDE)}"ks\fletcher_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\floatobj.h"\
	{$(INCLUDE)}"ks\frankenreiter_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\FrontEndManager.h"\
	{$(INCLUDE)}"ks\FrontEndMenus.h"\
	{$(INCLUDE)}"ks\GameData.h"\
	{$(INCLUDE)}"ks\GlobalData.h"\
	{$(INCLUDE)}"ks\globaltextenum.h"\
	{$(INCLUDE)}"ks\GraphicalMenuSystem.h"\
	{$(INCLUDE)}"ks\HighScoreFrontEnd.h"\
	{$(INCLUDE)}"ks\igo_widget.h"\
	{$(INCLUDE)}"ks\igo_widget_analogclock.h"\
	{$(INCLUDE)}"ks\igo_widget_balance.h"\
	{$(INCLUDE)}"ks\igo_widget_camera.h"\
	{$(INCLUDE)}"ks\igo_widget_fanmeter.h"\
	{$(INCLUDE)}"ks\igo_widget_grid.h"\
	{$(INCLUDE)}"ks\igo_widget_iconcount.h"\
	{$(INCLUDE)}"ks\igo_widget_iconradar.h"\
	{$(INCLUDE)}"ks\igo_widget_meterchallenge.h"\
	{$(INCLUDE)}"ks\igo_widget_objectalert.h"\
	{$(INCLUDE)}"ks\igo_widget_photo.h"\
	{$(INCLUDE)}"ks\igo_widget_replay.h"\
	{$(INCLUDE)}"ks\igo_widget_simple.h"\
	{$(INCLUDE)}"ks\igo_widget_skillchallenge.h"\
	{$(INCLUDE)}"ks\igo_widget_specialmeter.h"\
	{$(INCLUDE)}"ks\igo_widget_splitclock.h"\
	{$(INCLUDE)}"ks\igo_widget_splitmeter.h"\
	{$(INCLUDE)}"ks\igo_widget_splitscore.h"\
	{$(INCLUDE)}"ks\igo_widget_splitter.h"\
	{$(INCLUDE)}"ks\igo_widget_timeattack.h"\
	{$(INCLUDE)}"ks\igo_widget_waveindicator.h"\
	{$(INCLUDE)}"ks\IGOFrontEnd.h"\
	{$(INCLUDE)}"ks\igohintmanager.h"\
	{$(INCLUDE)}"ks\igoiconmanager.h"\
	{$(INCLUDE)}"ks\igolearn_new_trickmanager.h"\
	{$(INCLUDE)}"ks\ik_object.h"\
	{$(INCLUDE)}"ks\irons_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\judge.h"\
	{$(INCLUDE)}"ks\kellyslater_controller.h"\
	{$(INCLUDE)}"ks\kellyslater_shared_anims_mac.h"\
	{$(INCLUDE)}"ks\kellyslater_states_mac.h"\
	{$(INCLUDE)}"ks\ks_camera.h"\
	{$(INCLUDE)}"ks\ksdbmenu.h"\
	{$(INCLUDE)}"ks\ksngl.h"\
	{$(INCLUDE)}"ks\ksreplay.h"\
	{$(INCLUDE)}"ks\LogbookFrontEnd.h"\
	{$(INCLUDE)}"ks\machado_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\MainFrontEnd.h"\
	{$(INCLUDE)}"ks\Map.h"\
	{$(INCLUDE)}"ks\MCDetectFrontEnd.h"\
	{$(INCLUDE)}"ks\menu.h"\
	{$(INCLUDE)}"ks\menusys.h"\
	{$(INCLUDE)}"ks\mode.h"\
	{$(INCLUDE)}"ks\mode_headtohead.h"\
	{$(INCLUDE)}"ks\mode_meterattack.h"\
	{$(INCLUDE)}"ks\mode_push.h"\
	{$(INCLUDE)}"ks\mode_timeattack.h"\
	{$(INCLUDE)}"ks\MultiFrontEnd.h"\
	{$(INCLUDE)}"ks\MusicMan.h"\
	{$(INCLUDE)}"ks\ode.h"\
	{$(INCLUDE)}"ks\osGameSaver.h"\
	{$(INCLUDE)}"ks\PAL60FrontEnd.h"\
	{$(INCLUDE)}"ks\PathData.h"\
	{$(INCLUDE)}"ks\PhotoFrontEnd.h"\
	{$(INCLUDE)}"ks\physics.h"\
	{$(INCLUDE)}"ks\player.h"\
	{$(INCLUDE)}"ks\replay.h"\
	{$(INCLUDE)}"ks\robb_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\rumbleManager.h"\
	{$(INCLUDE)}"ks\SaveLoadFrontEnd.h"\
	{$(INCLUDE)}"ks\scoringmanager.h"\
	{$(INCLUDE)}"ks\slater_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\sounddata.h"\
	{$(INCLUDE)}"ks\SoundScript.h"\
	{$(INCLUDE)}"ks\specialmeter.h"\
	{$(INCLUDE)}"ks\StatsFrontEnd.h"\
	{$(INCLUDE)}"ks\surferdata.h"\
	{$(INCLUDE)}"ks\SurferFrontEnd.h"\
	{$(INCLUDE)}"ks\trick_system.h"\
	{$(INCLUDE)}"ks\TrickBookFrontEnd.h"\
	{$(INCLUDE)}"ks\trickdata.h"\
	{$(INCLUDE)}"ks\trickregion.h"\
	{$(INCLUDE)}"ks\tricks.h"\
	{$(INCLUDE)}"ks\tutorialdata.h"\
	{$(INCLUDE)}"ks\TutorialFrontEnd.h"\
	{$(INCLUDE)}"ks\tutorialmanager.h"\
	{$(INCLUDE)}"ks\underwtr.h"\
	{$(INCLUDE)}"ks\VOEngine.h"\
	{$(INCLUDE)}"ks\wave.h"\
	{$(INCLUDE)}"ks\wavebreak.txt"\
	{$(INCLUDE)}"ks\wavebreakstage.txt"\
	{$(INCLUDE)}"ks\waveemitter.txt"\
	{$(INCLUDE)}"ks\waveenum.h"\
	{$(INCLUDE)}"ks\wavemarker.txt"\
	{$(INCLUDE)}"ks\waveregion.txt"\
	{$(INCLUDE)}"ks\wavestage.txt"\
	{$(INCLUDE)}"libcdvd.h"\
	{$(INCLUDE)}"libdma.h"\
	{$(INCLUDE)}"libgraph.h"\
	{$(INCLUDE)}"libipu.h"\
	{$(INCLUDE)}"libmc.h"\
	{$(INCLUDE)}"libmpeg.h"\
	{$(INCLUDE)}"libpad.h"\
	{$(INCLUDE)}"libpc.h"\
	{$(INCLUDE)}"libusbkb.h"\
	{$(INCLUDE)}"libvu0.h"\
	{$(INCLUDE)}"sifdev.h"\
	{$(INCLUDE)}"xgraphics.h"\
	
NODEP_CPP_FILES_S=\
	"..\..\NSL\pc\nl_pc.h"\
	"..\..\NSL\pc\nsl_pc_ext.h"\
	".\algo.h"\
	".\defalloc.h"\
	".\gc_arammgr.h"\
	".\hwosmks\am_audio.h"\
	".\hwosmks\mks_devopts.h"\
	".\hwosmks\mks_errmsg.h"\
	".\hwosmks\mks_movieplayer.h"\
	".\hwosmks\set5_rasterize.h"\
	".\hwosmks\set5_timer.h"\
	".\hwosmks\sh4_math.h"\
	".\hwosmks\sy_alloc.h"\
	".\hwosmks\sy_file.h"\
	".\hwosnglpc\algebra.h"\
	".\hwosnglpc\pc_algebra.h"\
	".\hwosnull\null_alloc.h"\
	".\hwosnull\null_audio.h"\
	".\hwosnull\null_devopts.h"\
	".\hwosnull\null_errmsg.h"\
	".\hwosnull\null_file.h"\
	".\hwosnull\null_math.h"\
	".\hwosnull\null_movieplayer.h"\
	".\hwosnull\null_rasterize.h"\
	".\hwosnull\null_timer.h"\
	".\hwospc\d3d_rasterize.h"\
	".\hwospc\ds_audio.h"\
	".\hwospc\pc_algebra.h"\
	".\hwospc\pc_movieplayer.h"\
	".\hwospc\w32_alloc.h"\
	".\hwospc\w32_devopts.h"\
	".\hwospc\w32_errmsg.h"\
	".\hwospc\w32_file.h"\
	".\hwospc\w32_timer.h"\
	".\hwospc\x86_math.h"\
	".\hwosxb\ngl_xbox.h"\
	".\hwosxb\nvl_xbox.h"\
	".\LipSync_Anim.h"\
	".\ngl_xbox.h"\
	".\nvl_gc.h"\
	".\nvl_xbox.h"\
	".\nvlstream_gc.h"\
	".\stl_heap.h"\
	
# ADD BASE CPP /Yu
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

DEP_CPP_FILES_S=\
	"..\..\ngl\ps2\ngl_fixedstr.h"\
	"..\..\ngl\ps2\ngl_ps2.h"\
	"..\..\ngl\ps2\tim2.h"\
	"..\..\NSL\common\nl.h"\
	"..\..\NSL\common\nsl.h"\
	"..\..\NSL\common\ProjectOptions.h"\
	"..\..\NSL\gamecube\nl_gc.h"\
	"..\..\NSL\ps2\fifo_queue.cpp"\
	"..\..\NSL\ps2\fifo_queue.h"\
	"..\..\NSL\PS2\gas.h"\
	"..\..\NSL\PS2\gas_utility.h"\
	"..\..\NSL\PS2\nl_ps2.h"\
	"..\..\NSL\PS2\nsl_ps2.h"\
	"..\..\NSL\XBOX\nl_xbox.h"\
	"..\..\NSL\XBOX\nsl_xbox.h"\
	"..\..\NSL\XBOX\nsl_xbox_ext.h"\
	"..\..\NSL\XBOX\nsl_xbox_source.h"\
	"..\..\nsl\xbox\nsl_xbox_streams.h"\
	"..\..\NVL\ps2\nvl_ps2.h"\
	"..\..\NVL\ps2\nvlMPEG_ps2.h"\
	"..\..\NVL\ps2\nvlstream_ps2.h"\
	".\aggvertbuf.h"\
	".\ai_constants.h"\
	".\ai_interface.h"\
	".\ai_locomotion.h"\
	".\ai_polypath.h"\
	".\ai_polypath_cell.h"\
	".\ai_polypath_heap.h"\
	".\ai_voice.h"\
	".\algebra.h"\
	".\anim.h"\
	".\anim_flavor.h"\
	".\anim_flavors.h"\
	".\anim_ids.h"\
	".\app.h"\
	".\archalloc.h"\
	".\avltree.h"\
	".\beam.h"\
	".\beam_signals.h"\
	".\billboard.h"\
	".\bone.h"\
	".\bound.h"\
	".\box_trigger_interface.h"\
	".\camera.h"\
	".\capsule.h"\
	".\cface.h"\
	".\chunkfile.h"\
	".\colgeom.h"\
	".\collide.h"\
	".\colmesh.h"\
	".\color.h"\
	".\commands.h"\
	".\conglom.h"\
	".\console.h"\
	".\constants.h"\
	".\controller.h"\
	".\convex_box.h"\
	".\custom_stl.h"\
	".\debug.h"\
	".\debug_render.h"\
	".\devoptflags.h"\
	".\devoptints.h"\
	".\devoptstrs.h"\
	".\element.h"\
	".\entflavor.h"\
	".\entity.h"\
	".\entity_anim.h"\
	".\entity_interface.h"\
	".\entity_maker.h"\
	".\entity_signals.h"\
	".\entityflags.h"\
	".\entityid.h"\
	".\errorcontext.h"\
	".\face.h"\
	".\faceflags.h"\
	".\fast_vector.h"\
	".\file.h"\
	".\file_finder.h"\
	".\filespec.h"\
	".\fogmgr.h"\
	".\frame_info.h"\
	".\game.h"\
	".\game_info_vars.h"\
	".\game_process.h"\
	".\gcglobals.h"\
	".\generator.h"\
	".\geomgr.h"\
	".\global.h"\
	".\global_signals.h"\
	".\graph.h"\
	".\guidance_sys.h"\
	".\hull.h"\
	".\hwaudio.h"\
	".\hwmath.h"\
	".\hwmovieplayer.h"\
	".\hwrasterize.h"\
	".\hyperplane.h"\
	".\ini_parser.h"\
	".\inputmgr.h"\
	".\instance.h"\
	".\interface.h"\
	".\item.h"\
	".\item_signals.h"\
	".\joystick.h"\
	".\keyboard.h"\
	".\ks\ksheaps.h"\
	".\ks\SoundScript.cpp"\
	".\kshooks.h"\
	".\ksnsl.h"\
	".\ksnvl.h"\
	".\light.h"\
	".\lightmgr.h"\
	".\linear_anim.h"\
	".\link_interface.h"\
	".\localize.h"\
	".\map_e.h"\
	".\material.h"\
	".\matfac.h"\
	".\maxskinbones.h"\
	".\meshrefs.h"\
	".\mic.h"\
	".\mobject.h"\
	".\msgboard.h"\
	".\mustash.h"\
	".\ngl.h"\
	".\osalloc.h"\
	".\osassert.h"\
	".\osdevopts.h"\
	".\oserrmsg.h"\
	".\osfile.h"\
	".\ostimer.h"\
	".\path.h"\
	".\pc_port.h"\
	".\physical_interface.h"\
	".\plane.h"\
	".\platform_defines.h"\
	".\pmesh.h"\
	".\po.h"\
	".\po_anim.h"\
	".\portal.h"\
	".\profiler.h"\
	".\projconst.h"\
	".\project.h"\
	".\pstring.h"\
	".\random.h"\
	".\rect.h"\
	".\refptr.h"\
	".\region.h"\
	".\region_graph.h"\
	".\render_data.h"\
	".\renderflav.h"\
	".\scene_anim.h"\
	".\script_access.cpp"\
	".\script_access.h"\
	".\script_controller_signals.h"\
	".\script_lib.cpp"\
	".\script_lib_anim.cpp"\
	".\script_lib_anim.h"\
	".\script_lib_beam.cpp"\
	".\script_lib_beam.h"\
	".\script_lib_controller.cpp"\
	".\script_lib_controller.h"\
	".\script_lib_entity.cpp"\
	".\script_lib_entity.h"\
	".\script_lib_entity2.cpp"\
	".\script_lib_entity_widget.cpp"\
	".\script_lib_entity_widget.h"\
	".\script_lib_item.cpp"\
	".\script_lib_item.h"\
	".\script_lib_list.cpp"\
	".\script_lib_list.h"\
	".\script_lib_mfg.cpp"\
	".\script_lib_mfg.h"\
	".\script_lib_scene_anim.cpp"\
	".\script_lib_scene_anim.h"\
	".\script_lib_signal.cpp"\
	".\script_lib_signal.h"\
	".\script_lib_sound_inst.cpp"\
	".\script_lib_sound_inst.h"\
	".\script_lib_sound_stream.cpp"\
	".\script_lib_sound_stream.h"\
	".\script_lib_trigger.cpp"\
	".\script_lib_trigger.h"\
	".\script_lib_vector3d.cpp"\
	".\script_lib_vector3d.h"\
	".\script_lib_widget.cpp"\
	".\script_lib_widget.h"\
	".\script_library_class.h"\
	".\script_mfg_signals.h"\
	".\script_object.h"\
	".\script_register.cpp"\
	".\script_register.h"\
	".\signal_anim.h"\
	".\signals.h"\
	".\simple_classes.h"\
	".\SimpleAssert.h"\
	".\singleton.h"\
	".\so_data_block.h"\
	".\sound_group.h"\
	".\sound_interface.h"\
	".\sphere.h"\
	".\stashes.h"\
	".\staticmem.h"\
	".\stl_adapter.h"\
	".\stringx.h"\
	".\terrain.h"\
	".\text_font.h"\
	".\textfile.h"\
	".\time_interface.h"\
	".\timer.h"\
	".\trigger.h"\
	".\trigger_signals.h"\
	".\txtcoord.h"\
	".\types.h"\
	".\usefulmath.h"\
	".\users.h"\
	".\vert.h"\
	".\visrep.h"\
	".\vm_executable.h"\
	".\vm_stack.h"\
	".\vm_symbol.h"\
	".\vm_symbol_list.h"\
	".\vm_thread.h"\
	".\warnlvl.h"\
	".\wds.h"\
	".\wedge.h"\
	".\widget.h"\
	".\widget_entity.h"\
	".\widget_script.h"\
	".\xbglobals.h"\
	
NODEP_CPP_FILES_S=\
	"..\..\NSL\pc\nl_pc.h"\
	"..\..\NSL\pc\nsl_pc_ext.h"\
	"..\..\NSL\XBOX\nsl.h"\
	".\algo.h"\
	".\defalloc.h"\
	".\ekernel.h"\
	".\eregs.h"\
	".\estruct.h"\
	".\etypes.h"\
	".\gc_arammgr.h"\
	".\graphics.h"\
	".\hwosgc\nsl.h"\
	".\hwosmks\am_audio.h"\
	".\hwosmks\mks_devopts.h"\
	".\hwosmks\mks_errmsg.h"\
	".\hwosmks\mks_movieplayer.h"\
	".\hwosmks\set5_rasterize.h"\
	".\hwosmks\set5_timer.h"\
	".\hwosmks\sh4_math.h"\
	".\hwosmks\sy_alloc.h"\
	".\hwosmks\sy_file.h"\
	".\hwosnglpc\algebra.h"\
	".\hwosnglpc\pc_algebra.h"\
	".\hwosnull\null_alloc.h"\
	".\hwosnull\null_audio.h"\
	".\hwosnull\null_devopts.h"\
	".\hwosnull\null_errmsg.h"\
	".\hwosnull\null_file.h"\
	".\hwosnull\null_math.h"\
	".\hwosnull\null_movieplayer.h"\
	".\hwosnull\null_rasterize.h"\
	".\hwosnull\null_timer.h"\
	".\hwospc\d3d_rasterize.h"\
	".\hwospc\ds_audio.h"\
	".\hwospc\pc_algebra.h"\
	".\hwospc\pc_movieplayer.h"\
	".\hwospc\w32_alloc.h"\
	".\hwospc\w32_devopts.h"\
	".\hwospc\w32_errmsg.h"\
	".\hwospc\w32_file.h"\
	".\hwospc\w32_timer.h"\
	".\hwospc\x86_math.h"\
	".\hwosxb\ngl_xbox.h"\
	".\hwosxb\nvl_xbox.h"\
	".\ibcdvd.h"\
	".\ibdma.h"\
	".\ibgraph.h"\
	".\ibipu.h"\
	".\ibmc.h"\
	".\ibmpeg.h"\
	".\ibpad.h"\
	".\ibpc.h"\
	".\ibusbkb.h"\
	".\ibvu0.h"\
	".\ifdev.h"\
	".\LipSync_Anim.h"\
	".\ngl_xbox.h"\
	".\nvl_gc.h"\
	".\nvl_xbox.h"\
	".\nvlstream_gc.h"\
	".\s\AccompFrontEnd.h"\
	".\s\andersen_ind_anims_mac.h"\
	".\s\beach.h"\
	".\s\beachdata.h"\
	".\s\BeachFrontEnd.h"\
	".\s\board.h"\
	".\s\board_anims_mac.h"\
	".\s\boarddata.h"\
	".\s\BoardFrontEnd.h"\
	".\s\career.h"\
	".\s\careerdata.h"\
	".\s\carrol_ind_anims_mac.h"\
	".\s\challenge.h"\
	".\s\challenge_icon.h"\
	".\s\challenge_photo.h"\
	".\s\cheat.h"\
	".\s\CheatFrontEnd.h"\
	".\s\compressedphoto.h"\
	".\s\coords.h"\
	".\s\curren_ind_anims_mac.h"\
	".\s\eventmanager.h"\
	".\s\ExtrasFrontEnd.h"\
	".\s\FEAnim.h"\
	".\s\FEEntityManager.h"\
	".\s\FEMenu.h"\
	".\s\FEPanel.h"\
	".\s\fletcher_ind_anims_mac.h"\
	".\s\floatobj.h"\
	".\s\frankenreiter_ind_anims_mac.h"\
	".\s\FrontEndManager.h"\
	".\s\FrontEndMenus.h"\
	".\s\GameData.h"\
	".\s\GlobalData.h"\
	".\s\globaltextenum.h"\
	".\s\GraphicalMenuSystem.h"\
	".\s\HighScoreFrontEnd.h"\
	".\s\igo_widget.h"\
	".\s\igo_widget_analogclock.h"\
	".\s\igo_widget_balance.h"\
	".\s\igo_widget_camera.h"\
	".\s\igo_widget_fanmeter.h"\
	".\s\igo_widget_iconradar.h"\
	".\s\igo_widget_photo.h"\
	".\s\igo_widget_replay.h"\
	".\s\igo_widget_simple.h"\
	".\s\igo_widget_specialmeter.h"\
	".\s\igo_widget_splitclock.h"\
	".\s\igo_widget_splitmeter.h"\
	".\s\igo_widget_splitscore.h"\
	".\s\igo_widget_splitter.h"\
	".\s\igo_widget_timeattack.h"\
	".\s\igo_widget_waveindicator.h"\
	".\s\IGOFrontEnd.h"\
	".\s\igohintmanager.h"\
	".\s\igoiconmanager.h"\
	".\s\igolearn_new_trickmanager.h"\
	".\s\ik_object.h"\
	".\s\irons_ind_anims_mac.h"\
	".\s\judge.h"\
	".\s\kellyslater_controller.h"\
	".\s\kellyslater_shared_anims_mac.h"\
	".\s\kellyslater_states_mac.h"\
	".\s\ks_camera.h"\
	".\s\ksdbmenu.h"\
	".\s\ksngl.h"\
	".\s\ksreplay.h"\
	".\s\machado_ind_anims_mac.h"\
	".\s\MainFrontEnd.h"\
	".\s\Map.h"\
	".\s\MCDetectFrontEnd.h"\
	".\s\menu.h"\
	".\s\menusys.h"\
	".\s\mode.h"\
	".\s\mode_headtohead.h"\
	".\s\mode_meterattack.h"\
	".\s\mode_push.h"\
	".\s\mode_timeattack.h"\
	".\s\MultiFrontEnd.h"\
	".\s\MusicMan.h"\
	".\s\ode.h"\
	".\s\osGameSaver.h"\
	".\s\PAL60FrontEnd.h"\
	".\s\PathData.h"\
	".\s\PhotoFrontEnd.h"\
	".\s\physics.h"\
	".\s\player.h"\
	".\s\replay.h"\
	".\s\robb_ind_anims_mac.h"\
	".\s\rumbleManager.h"\
	".\s\SaveLoadFrontEnd.h"\
	".\s\scoringmanager.h"\
	".\s\slater_ind_anims_mac.h"\
	".\s\sounddata.h"\
	".\s\SoundScript.h"\
	".\s\specialmeter.h"\
	".\s\StatsFrontEnd.h"\
	".\s\surferdata.h"\
	".\s\SurferFrontEnd.h"\
	".\s\trick_system.h"\
	".\s\TrickBookFrontEnd.h"\
	".\s\trickdata.h"\
	".\s\trickregion.h"\
	".\s\tricks.h"\
	".\s\TutorialFrontEnd.h"\
	".\s\tutorialmanager.h"\
	".\s\VOEngine.h"\
	".\s\wave.h"\
	".\s\wavebreak.txt"\
	".\s\wavebreakstage.txt"\
	".\s\waveemitter.txt"\
	".\s\waveenum.h"\
	".\s\wavemarker.txt"\
	".\s\waveregion.txt"\
	".\s\wavestage.txt"\
	".\stl_heap.h"\
	".\wosgc\gc_algebra.h"\
	".\wosgc\gc_alloc.h"\
	".\wosgc\gc_audio.h"\
	".\wosgc\gc_errmsg.h"\
	".\wosgc\gc_file.h"\
	".\WOSGC\gc_gamesaver.h"\
	".\wosgc\gc_graphics.h"\
	".\wosgc\gc_input.h"\
	".\wosgc\gc_math.h"\
	".\wosgc\gc_movieplayer.h"\
	".\wosgc\gc_rasterize.h"\
	".\wosgc\gc_texturemgr.h"\
	".\wosgc\gc_timer.h"\
	".\wosgc\ngl_fixedstr.h"\
	".\wosgc\ngl_gc.h"\
	".\wosgc\ngl_version.h"\
	".\wosgc\nvl_gc.h"\
	".\WOSPS2\gas.h"\
	".\WOSPS2\ps2_algebra.h"\
	".\WOSPS2\ps2_alloc.h"\
	".\WOSPS2\ps2_audio.h"\
	".\WOSPS2\ps2_devopts.h"\
	".\WOSPS2\ps2_errmsg.h"\
	".\WOSPS2\ps2_file.h"\
	".\WOSPS2\ps2_gamesaver.h"\
	".\WOSPS2\ps2_graphics.h"\
	".\WOSPS2\ps2_input.h"\
	".\WOSPS2\ps2_m2vplayer.h"\
	".\WOSPS2\ps2_math.h"\
	".\WOSPS2\ps2_movieplayer.h"\
	".\WOSPS2\ps2_rasterize.h"\
	".\WOSPS2\ps2_texturemgr.h"\
	".\WOSPS2\ps2_timer.h"\
	".\WOSPS2\ps2_types.h"\
	".\wosxb\pstring.h"\
	".\wosxb\xb_algebra.h"\
	".\wosxb\xb_alloc.h"\
	".\wosxb\xb_audio.h"\
	".\wosxb\xb_errmsg.h"\
	".\wosxb\xb_file.h"\
	".\WOSXB\xb_gamesaver.h"\
	".\wosxb\xb_graphics.h"\
	".\wosxb\xb_input.h"\
	".\wosxb\xb_math.h"\
	".\wosxb\xb_movieplayer.h"\
	".\WOSXB\xb_portDVDCache.h"\
	".\wosxb\xb_rasterize.h"\
	".\wosxb\xb_string.h"\
	".\wosxb\xb_texturemgr.h"\
	".\wosxb\xb_timer.h"\
	".\wosxb\xb_types.h"\
	
# SUBTRACT BASE CPP /YX /Yc /Yu
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

DEP_CPP_FILES_S=\
	"..\..\ngl\ps2\ngl_fixedstr.h"\
	"..\..\ngl\ps2\ngl_ps2.h"\
	"..\..\ngl\ps2\tim2.h"\
	"..\..\NSL\common\nl.h"\
	"..\..\NSL\common\nsl.h"\
	"..\..\NSL\common\ProjectOptions.h"\
	"..\..\NSL\gamecube\nl_gc.h"\
	"..\..\NSL\ps2\fifo_queue.cpp"\
	"..\..\NSL\ps2\fifo_queue.h"\
	"..\..\NSL\PS2\gas.h"\
	"..\..\NSL\PS2\gas_utility.h"\
	"..\..\NSL\PS2\nl_ps2.h"\
	"..\..\NSL\PS2\nsl_ps2.h"\
	"..\..\NSL\XBOX\nl_xbox.h"\
	"..\..\NSL\XBOX\nsl_xbox.h"\
	"..\..\NSL\XBOX\nsl_xbox_ext.h"\
	"..\..\NSL\XBOX\nsl_xbox_source.h"\
	"..\..\nsl\xbox\nsl_xbox_streams.h"\
	"..\..\NVL\ps2\nvl_ps2.h"\
	"..\..\NVL\ps2\nvlMPEG_ps2.h"\
	"..\..\NVL\ps2\nvlstream_ps2.h"\
	".\aggvertbuf.h"\
	".\ai_constants.h"\
	".\ai_interface.h"\
	".\ai_locomotion.h"\
	".\ai_polypath.h"\
	".\ai_polypath_cell.h"\
	".\ai_polypath_heap.h"\
	".\ai_voice.h"\
	".\algebra.h"\
	".\anim.h"\
	".\anim_flavor.h"\
	".\anim_flavors.h"\
	".\anim_ids.h"\
	".\app.h"\
	".\archalloc.h"\
	".\avltree.h"\
	".\beam.h"\
	".\beam_signals.h"\
	".\billboard.h"\
	".\bone.h"\
	".\bound.h"\
	".\box_trigger_interface.h"\
	".\camera.h"\
	".\capsule.h"\
	".\cface.h"\
	".\chunkfile.h"\
	".\colgeom.h"\
	".\collide.h"\
	".\colmesh.h"\
	".\color.h"\
	".\commands.h"\
	".\conglom.h"\
	".\console.h"\
	".\constants.h"\
	".\controller.h"\
	".\convex_box.h"\
	".\custom_stl.h"\
	".\debug.h"\
	".\debug_render.h"\
	".\devoptflags.h"\
	".\devoptints.h"\
	".\devoptstrs.h"\
	".\element.h"\
	".\entflavor.h"\
	".\entity.h"\
	".\entity_anim.h"\
	".\entity_interface.h"\
	".\entity_maker.h"\
	".\entity_signals.h"\
	".\entityflags.h"\
	".\entityid.h"\
	".\errorcontext.h"\
	".\face.h"\
	".\faceflags.h"\
	".\fast_vector.h"\
	".\file.h"\
	".\file_finder.h"\
	".\filespec.h"\
	".\fogmgr.h"\
	".\frame_info.h"\
	".\game.h"\
	".\game_info_vars.h"\
	".\game_process.h"\
	".\gcglobals.h"\
	".\generator.h"\
	".\geomgr.h"\
	".\global.h"\
	".\global_signals.h"\
	".\graph.h"\
	".\guidance_sys.h"\
	".\hull.h"\
	".\hwaudio.h"\
	".\hwmath.h"\
	".\hwmovieplayer.h"\
	".\hwrasterize.h"\
	".\hyperplane.h"\
	".\ini_parser.h"\
	".\inputmgr.h"\
	".\instance.h"\
	".\interface.h"\
	".\item.h"\
	".\item_signals.h"\
	".\joystick.h"\
	".\keyboard.h"\
	".\ks\ksheaps.h"\
	".\ks\SoundScript.cpp"\
	".\kshooks.h"\
	".\ksnsl.h"\
	".\ksnvl.h"\
	".\light.h"\
	".\lightmgr.h"\
	".\linear_anim.h"\
	".\link_interface.h"\
	".\localize.h"\
	".\map_e.h"\
	".\material.h"\
	".\matfac.h"\
	".\maxskinbones.h"\
	".\meshrefs.h"\
	".\mic.h"\
	".\mobject.h"\
	".\msgboard.h"\
	".\mustash.h"\
	".\ngl.h"\
	".\osalloc.h"\
	".\osassert.h"\
	".\osdevopts.h"\
	".\oserrmsg.h"\
	".\osfile.h"\
	".\ostimer.h"\
	".\path.h"\
	".\pc_port.h"\
	".\physical_interface.h"\
	".\plane.h"\
	".\platform_defines.h"\
	".\pmesh.h"\
	".\po.h"\
	".\po_anim.h"\
	".\portal.h"\
	".\profiler.h"\
	".\projconst.h"\
	".\project.h"\
	".\pstring.h"\
	".\random.h"\
	".\rect.h"\
	".\refptr.h"\
	".\region.h"\
	".\region_graph.h"\
	".\render_data.h"\
	".\renderflav.h"\
	".\scene_anim.h"\
	".\script_access.cpp"\
	".\script_access.h"\
	".\script_controller_signals.h"\
	".\script_lib.cpp"\
	".\script_lib_anim.cpp"\
	".\script_lib_anim.h"\
	".\script_lib_beam.cpp"\
	".\script_lib_beam.h"\
	".\script_lib_controller.cpp"\
	".\script_lib_controller.h"\
	".\script_lib_entity.cpp"\
	".\script_lib_entity.h"\
	".\script_lib_entity2.cpp"\
	".\script_lib_entity_widget.cpp"\
	".\script_lib_entity_widget.h"\
	".\script_lib_item.cpp"\
	".\script_lib_item.h"\
	".\script_lib_list.cpp"\
	".\script_lib_list.h"\
	".\script_lib_mfg.cpp"\
	".\script_lib_mfg.h"\
	".\script_lib_scene_anim.cpp"\
	".\script_lib_scene_anim.h"\
	".\script_lib_signal.cpp"\
	".\script_lib_signal.h"\
	".\script_lib_sound_inst.cpp"\
	".\script_lib_sound_inst.h"\
	".\script_lib_sound_stream.cpp"\
	".\script_lib_sound_stream.h"\
	".\script_lib_trigger.cpp"\
	".\script_lib_trigger.h"\
	".\script_lib_vector3d.cpp"\
	".\script_lib_vector3d.h"\
	".\script_lib_widget.cpp"\
	".\script_lib_widget.h"\
	".\script_library_class.h"\
	".\script_mfg_signals.h"\
	".\script_object.h"\
	".\script_register.cpp"\
	".\script_register.h"\
	".\signal_anim.h"\
	".\signals.h"\
	".\simple_classes.h"\
	".\SimpleAssert.h"\
	".\singleton.h"\
	".\so_data_block.h"\
	".\sound_group.h"\
	".\sound_interface.h"\
	".\sphere.h"\
	".\stashes.h"\
	".\staticmem.h"\
	".\stl_adapter.h"\
	".\stringx.h"\
	".\terrain.h"\
	".\text_font.h"\
	".\textfile.h"\
	".\time_interface.h"\
	".\timer.h"\
	".\trigger.h"\
	".\trigger_signals.h"\
	".\txtcoord.h"\
	".\types.h"\
	".\usefulmath.h"\
	".\users.h"\
	".\vert.h"\
	".\visrep.h"\
	".\vm_executable.h"\
	".\vm_stack.h"\
	".\vm_symbol.h"\
	".\vm_symbol_list.h"\
	".\vm_thread.h"\
	".\warnlvl.h"\
	".\wds.h"\
	".\wedge.h"\
	".\widget.h"\
	".\widget_entity.h"\
	".\widget_script.h"\
	".\xbglobals.h"\
	
NODEP_CPP_FILES_S=\
	"..\..\NSL\pc\nl_pc.h"\
	"..\..\NSL\pc\nsl_pc_ext.h"\
	".\algo.h"\
	".\defalloc.h"\
	".\ekernel.h"\
	".\eregs.h"\
	".\estruct.h"\
	".\etypes.h"\
	".\gc\nsl_gc.h"\
	".\gc_arammgr.h"\
	".\graphics.h"\
	".\hwosmks\am_audio.h"\
	".\hwosmks\mks_devopts.h"\
	".\hwosmks\mks_errmsg.h"\
	".\hwosmks\mks_movieplayer.h"\
	".\hwosmks\set5_rasterize.h"\
	".\hwosmks\set5_timer.h"\
	".\hwosmks\sh4_math.h"\
	".\hwosmks\sy_alloc.h"\
	".\hwosmks\sy_file.h"\
	".\hwosnglpc\algebra.h"\
	".\hwosnglpc\pc_algebra.h"\
	".\hwosnull\null_alloc.h"\
	".\hwosnull\null_audio.h"\
	".\hwosnull\null_devopts.h"\
	".\hwosnull\null_errmsg.h"\
	".\hwosnull\null_file.h"\
	".\hwosnull\null_math.h"\
	".\hwosnull\null_movieplayer.h"\
	".\hwosnull\null_rasterize.h"\
	".\hwosnull\null_timer.h"\
	".\hwospc\d3d_rasterize.h"\
	".\hwospc\ds_audio.h"\
	".\hwospc\pc_algebra.h"\
	".\hwospc\pc_movieplayer.h"\
	".\hwospc\w32_alloc.h"\
	".\hwospc\w32_devopts.h"\
	".\hwospc\w32_errmsg.h"\
	".\hwospc\w32_file.h"\
	".\hwospc\w32_timer.h"\
	".\hwospc\x86_math.h"\
	".\hwosxb\ngl_xbox.h"\
	".\hwosxb\nvl_xbox.h"\
	".\ibcdvd.h"\
	".\ibdma.h"\
	".\ibgraph.h"\
	".\ibipu.h"\
	".\ibmc.h"\
	".\ibmpeg.h"\
	".\ibpad.h"\
	".\ibpc.h"\
	".\ibusbkb.h"\
	".\ibvu0.h"\
	".\ifdev.h"\
	".\LipSync_Anim.h"\
	".\ngl_xbox.h"\
	".\nsl.h"\
	".\nvl_gc.h"\
	".\nvl_xbox.h"\
	".\nvlstream_gc.h"\
	".\s\AccompFrontEnd.h"\
	".\s\andersen_ind_anims_mac.h"\
	".\s\beach.h"\
	".\s\beachdata.h"\
	".\s\BeachFrontEnd.h"\
	".\s\board.h"\
	".\s\board_anims_mac.h"\
	".\s\boarddata.h"\
	".\s\BoardFrontEnd.h"\
	".\s\career.h"\
	".\s\careerdata.h"\
	".\s\carrol_ind_anims_mac.h"\
	".\s\challenge.h"\
	".\s\challenge_icon.h"\
	".\s\challenge_photo.h"\
	".\s\cheat.h"\
	".\s\CheatFrontEnd.h"\
	".\s\compressedphoto.h"\
	".\s\coords.h"\
	".\s\curren_ind_anims_mac.h"\
	".\s\eventmanager.h"\
	".\s\ExtrasFrontEnd.h"\
	".\s\FEAnim.h"\
	".\s\FEEntityManager.h"\
	".\s\FEMenu.h"\
	".\s\FEPanel.h"\
	".\s\fletcher_ind_anims_mac.h"\
	".\s\floatobj.h"\
	".\s\frankenreiter_ind_anims_mac.h"\
	".\s\FrontEndManager.h"\
	".\s\FrontEndMenus.h"\
	".\s\GameData.h"\
	".\s\GlobalData.h"\
	".\s\globaltextenum.h"\
	".\s\GraphicalMenuSystem.h"\
	".\s\HighScoreFrontEnd.h"\
	".\s\igo_widget.h"\
	".\s\igo_widget_analogclock.h"\
	".\s\igo_widget_balance.h"\
	".\s\igo_widget_camera.h"\
	".\s\igo_widget_fanmeter.h"\
	".\s\igo_widget_iconcount.h"\
	".\s\igo_widget_iconradar.h"\
	".\s\igo_widget_meterchallenge.h"\
	".\s\igo_widget_objectalert.h"\
	".\s\igo_widget_photo.h"\
	".\s\igo_widget_replay.h"\
	".\s\igo_widget_simple.h"\
	".\s\igo_widget_skillchallenge.h"\
	".\s\igo_widget_specialmeter.h"\
	".\s\igo_widget_splitclock.h"\
	".\s\igo_widget_splitmeter.h"\
	".\s\igo_widget_splitscore.h"\
	".\s\igo_widget_splitter.h"\
	".\s\igo_widget_timeattack.h"\
	".\s\igo_widget_waveindicator.h"\
	".\s\IGOFrontEnd.h"\
	".\s\igohintmanager.h"\
	".\s\igoiconmanager.h"\
	".\s\igolearn_new_trickmanager.h"\
	".\s\ik_object.h"\
	".\s\irons_ind_anims_mac.h"\
	".\s\judge.h"\
	".\s\kellyslater_controller.h"\
	".\s\kellyslater_shared_anims_mac.h"\
	".\s\kellyslater_states_mac.h"\
	".\s\ks_camera.h"\
	".\s\ksdbmenu.h"\
	".\s\ksngl.h"\
	".\s\ksreplay.h"\
	".\s\LogbookFrontEnd.h"\
	".\s\machado_ind_anims_mac.h"\
	".\s\MainFrontEnd.h"\
	".\s\Map.h"\
	".\s\MCDetectFrontEnd.h"\
	".\s\menu.h"\
	".\s\menusys.h"\
	".\s\mode.h"\
	".\s\mode_headtohead.h"\
	".\s\mode_meterattack.h"\
	".\s\mode_push.h"\
	".\s\mode_timeattack.h"\
	".\s\MultiFrontEnd.h"\
	".\s\MusicMan.h"\
	".\s\ode.h"\
	".\s\osGameSaver.h"\
	".\s\PAL60FrontEnd.h"\
	".\s\PathData.h"\
	".\s\PhotoFrontEnd.h"\
	".\s\physics.h"\
	".\s\player.h"\
	".\s\replay.h"\
	".\s\robb_ind_anims_mac.h"\
	".\s\rumbleManager.h"\
	".\s\SaveLoadFrontEnd.h"\
	".\s\scoringmanager.h"\
	".\s\slater_ind_anims_mac.h"\
	".\s\sounddata.h"\
	".\s\SoundScript.h"\
	".\s\specialmeter.h"\
	".\s\StatsFrontEnd.h"\
	".\s\surferdata.h"\
	".\s\SurferFrontEnd.h"\
	".\s\trick_system.h"\
	".\s\TrickBookFrontEnd.h"\
	".\s\trickdata.h"\
	".\s\trickregion.h"\
	".\s\tricks.h"\
	".\s\tutorialdata.h"\
	".\s\TutorialFrontEnd.h"\
	".\s\tutorialmanager.h"\
	".\s\VOEngine.h"\
	".\s\wave.h"\
	".\s\wavebreak.txt"\
	".\s\wavebreakstage.txt"\
	".\s\waveemitter.txt"\
	".\s\waveenum.h"\
	".\s\wavemarker.txt"\
	".\s\waveregion.txt"\
	".\s\wavestage.txt"\
	".\stl_heap.h"\
	".\wosgc\gc_algebra.h"\
	".\wosgc\gc_alloc.h"\
	".\wosgc\gc_audio.h"\
	".\wosgc\gc_errmsg.h"\
	".\wosgc\gc_file.h"\
	".\WOSGC\gc_gamesaver.h"\
	".\wosgc\gc_graphics.h"\
	".\wosgc\gc_input.h"\
	".\wosgc\gc_math.h"\
	".\wosgc\gc_movieplayer.h"\
	".\wosgc\gc_rasterize.h"\
	".\wosgc\gc_texturemgr.h"\
	".\wosgc\gc_timer.h"\
	".\wosgc\ngl_fixedstr.h"\
	".\wosgc\ngl_gc.h"\
	".\wosgc\ngl_version.h"\
	".\wosgc\nvl_gc.h"\
	".\WOSPS2\gas.h"\
	".\WOSPS2\ps2_algebra.h"\
	".\WOSPS2\ps2_alloc.h"\
	".\WOSPS2\ps2_audio.h"\
	".\WOSPS2\ps2_devopts.h"\
	".\WOSPS2\ps2_errmsg.h"\
	".\WOSPS2\ps2_file.h"\
	".\WOSPS2\ps2_gamesaver.h"\
	".\WOSPS2\ps2_graphics.h"\
	".\WOSPS2\ps2_input.h"\
	".\WOSPS2\ps2_m2vplayer.h"\
	".\WOSPS2\ps2_math.h"\
	".\WOSPS2\ps2_movieplayer.h"\
	".\WOSPS2\ps2_rasterize.h"\
	".\WOSPS2\ps2_texturemgr.h"\
	".\WOSPS2\ps2_timer.h"\
	".\WOSPS2\ps2_types.h"\
	".\wosxb\pstring.h"\
	".\wosxb\xb_algebra.h"\
	".\wosxb\xb_alloc.h"\
	".\wosxb\xb_audio.h"\
	".\wosxb\xb_errmsg.h"\
	".\wosxb\xb_file.h"\
	".\WOSXB\xb_gamesaver.h"\
	".\wosxb\xb_graphics.h"\
	".\wosxb\xb_input.h"\
	".\wosxb\xb_math.h"\
	".\wosxb\xb_movieplayer.h"\
	".\WOSXB\xb_portDVDCache.h"\
	".\wosxb\xb_rasterize.h"\
	".\wosxb\xb_string.h"\
	".\wosxb\xb_texturemgr.h"\
	".\wosxb\xb_timer.h"\
	".\wosxb\xb_types.h"\
	
# SUBTRACT BASE CPP /YX /Yc /Yu
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\files_vsim.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

DEP_CPP_FILES_V=\
	"..\..\ngl\ps2\ngl_fixedstr.h"\
	"..\..\ngl\ps2\ngl_ps2.h"\
	"..\..\ngl\ps2\tim2.h"\
	"..\..\NSL\common\nl.h"\
	"..\..\NSL\common\nsl.h"\
	"..\..\NSL\common\ProjectOptions.h"\
	"..\..\NSL\gamecube\nl_gc.h"\
	"..\..\NSL\ps2\fifo_queue.cpp"\
	"..\..\NSL\ps2\fifo_queue.h"\
	"..\..\NSL\PS2\gas.h"\
	"..\..\NSL\PS2\gas_utility.h"\
	"..\..\NSL\PS2\nl_ps2.h"\
	"..\..\NSL\PS2\nsl_ps2.h"\
	"..\..\NSL\XBOX\nl_xbox.h"\
	"..\..\NSL\XBOX\nsl_xbox.h"\
	"..\..\NSL\XBOX\nsl_xbox_ext.h"\
	"..\..\NSL\XBOX\nsl_xbox_source.h"\
	"..\..\nsl\xbox\nsl_xbox_streams.h"\
	"..\..\NVL\ps2\nvl_ps2.h"\
	"..\..\NVL\ps2\nvlMPEG_ps2.h"\
	"..\..\NVL\ps2\nvlstream_ps2.h"\
	".\aggvertbuf.cpp"\
	".\aggvertbuf.h"\
	".\AIController.h"\
	".\algebra.h"\
	".\anim.h"\
	".\anim_flavor.h"\
	".\anim_flavors.h"\
	".\anim_ids.h"\
	".\app.h"\
	".\archalloc.h"\
	".\avltree.h"\
	".\b_spline.cpp"\
	".\b_spline.h"\
	".\beam.h"\
	".\beam_signals.h"\
	".\billboard.h"\
	".\bitplane.cpp"\
	".\bitplane.h"\
	".\blendmodes.h"\
	".\bone.h"\
	".\bound.h"\
	".\box_trigger_interface.h"\
	".\camera.h"\
	".\capsule.h"\
	".\cface.h"\
	".\chunkfile.cpp"\
	".\chunkfile.h"\
	".\colgeom.h"\
	".\collide.h"\
	".\colmesh.h"\
	".\color.cpp"\
	".\color.h"\
	".\commands.h"\
	".\conglom.h"\
	".\console.cpp"\
	".\console.h"\
	".\consoleCmds.cpp"\
	".\consoleCmds.h"\
	".\consoleVars.cpp"\
	".\consoleVars.h"\
	".\constants.h"\
	".\controller.h"\
	".\convex_box.h"\
	".\custom_stl.h"\
	".\debug.h"\
	".\debug_render.h"\
	".\debugutil.cpp"\
	".\devoptflags.h"\
	".\devoptints.h"\
	".\devoptstrs.h"\
	".\entflavor.h"\
	".\entity.h"\
	".\entity_anim.h"\
	".\entity_interface.h"\
	".\entity_maker.h"\
	".\entity_signals.h"\
	".\entityflags.h"\
	".\entityid.h"\
	".\errorcontext.cpp"\
	".\errorcontext.h"\
	".\face.h"\
	".\faceflags.h"\
	".\fast_vector.h"\
	".\file.cpp"\
	".\file.h"\
	".\file_finder.h"\
	".\filespec.cpp"\
	".\filespec.h"\
	".\fogmgr.cpp"\
	".\fogmgr.h"\
	".\forceflags.h"\
	".\frame_info.cpp"\
	".\frame_info.h"\
	".\game.h"\
	".\game_info.h"\
	".\game_info_vars.h"\
	".\game_process.h"\
	".\gcglobals.h"\
	".\generator.h"\
	".\geometry.cpp"\
	".\geometry.h"\
	".\geomgr.h"\
	".\global.h"\
	".\global_signals.h"\
	".\graph.h"\
	".\hull.cpp"\
	".\hull.h"\
	".\hwmath.h"\
	".\hwmovieplayer.h"\
	".\hwrasterize.h"\
	".\hyperplane.h"\
	".\ini_parser.h"\
	".\inputmgr.cpp"\
	".\inputmgr.h"\
	".\instance.h"\
	".\iri.h"\
	".\item.h"\
	".\item_signals.h"\
	".\joystick.h"\
	".\keyboard.h"\
	".\ks\eventmanager.cpp"\
	".\ks\ksheaps.h"\
	".\kshooks.h"\
	".\ksnsl.h"\
	".\ksnvl.h"\
	".\light.h"\
	".\linear_anim.h"\
	".\link_interface.h"\
	".\localize.h"\
	".\map_e.h"\
	".\material.cpp"\
	".\material.h"\
	".\matfac.h"\
	".\maxskinbones.h"\
	".\meshrefs.h"\
	".\mic.h"\
	".\mobject.h"\
	".\mustash.cpp"\
	".\mustash.h"\
	".\ngl.h"\
	".\ngl_support.h"\
	".\opcode_arg_t"\
	".\opcode_t"\
	".\opcodes.cpp"\
	".\opcodes.h"\
	".\osalloc.h"\
	".\osassert.cpp"\
	".\osassert.h"\
	".\osdevopts.h"\
	".\oserrmsg.h"\
	".\osfile.h"\
	".\ostimer.h"\
	".\path.cpp"\
	".\path.h"\
	".\pc_port.h"\
	".\plane.cpp"\
	".\plane.h"\
	".\platform_defines.h"\
	".\pmesh.h"\
	".\po.cpp"\
	".\po.h"\
	".\po_anim.h"\
	".\portal.h"\
	".\profiler.cpp"\
	".\profiler.h"\
	".\projconst.h"\
	".\project.h"\
	".\pstring.cpp"\
	".\pstring.h"\
	".\random.h"\
	".\rect.h"\
	".\refptr.h"\
	".\region.h"\
	".\region_graph.h"\
	".\render_data.h"\
	".\renderflav.h"\
	".\scene_anim.h"\
	".\script_lib_mfg.h"\
	".\script_library_class.cpp"\
	".\script_library_class.h"\
	".\script_mfg_signals.h"\
	".\script_object.cpp"\
	".\script_object.h"\
	".\semaphores.h"\
	".\signal.cpp"\
	".\signal_anim.h"\
	".\signals.h"\
	".\SimpleAssert.h"\
	".\singleton.cpp"\
	".\singleton.h"\
	".\sl_debugger.h"\
	".\so_data_block.cpp"\
	".\so_data_block.h"\
	".\sphere.h"\
	".\stash_support.cpp"\
	".\stash_support.h"\
	".\stashes.h"\
	".\staticmem.h"\
	".\stl_adapter.h"\
	".\stringx.cpp"\
	".\stringx.h"\
	".\terrain.h"\
	".\text_font.cpp"\
	".\text_font.h"\
	".\textfile.cpp"\
	".\textfile.h"\
	".\timer.h"\
	".\txtcoord.h"\
	".\types.h"\
	".\usefulmath.h"\
	".\users.h"\
	".\vert.h"\
	".\vertwork.h"\
	".\visrep.h"\
	".\vm_executable.cpp"\
	".\vm_executable.h"\
	".\vm_executable_vector.h"\
	".\vm_stack.cpp"\
	".\vm_stack.h"\
	".\vm_symbol.cpp"\
	".\vm_symbol.h"\
	".\vm_symbol_list.h"\
	".\vm_thread.cpp"\
	".\vm_thread.h"\
	".\warnlvl.h"\
	".\wds.h"\
	".\wedge.h"\
	".\widget.cpp"\
	".\widget.h"\
	".\widget_entity.h"\
	".\xbglobals.h"\
	".\zip_filter.cpp"\
	".\zip_filter.h"\
	".\zlib\zlib.h"\
	{$(INCLUDE)}"eekernel.h"\
	{$(INCLUDE)}"eeregs.h"\
	{$(INCLUDE)}"eestruct.h"\
	{$(INCLUDE)}"eetypes.h"\
	{$(INCLUDE)}"hwosgc\gc_algebra.h"\
	{$(INCLUDE)}"hwosgc\gc_alloc.h"\
	{$(INCLUDE)}"hwosgc\gc_errmsg.h"\
	{$(INCLUDE)}"hwosgc\gc_file.h"\
	{$(INCLUDE)}"HWOSGC\gc_gamesaver.h"\
	{$(INCLUDE)}"hwosgc\gc_graphics.h"\
	{$(INCLUDE)}"hwosgc\gc_input.h"\
	{$(INCLUDE)}"hwosgc\gc_math.h"\
	{$(INCLUDE)}"hwosgc\gc_movieplayer.h"\
	{$(INCLUDE)}"hwosgc\gc_rasterize.h"\
	{$(INCLUDE)}"hwosgc\gc_texturemgr.h"\
	{$(INCLUDE)}"hwosgc\gc_timer.h"\
	{$(INCLUDE)}"hwosgc\ngl_fixedstr.h"\
	{$(INCLUDE)}"hwosgc\ngl_gc.h"\
	{$(INCLUDE)}"hwosgc\ngl_version.h"\
	{$(INCLUDE)}"hwosgc\nvl_gc.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_algebra.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_alloc.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_devopts.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_errmsg.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_file.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_gamesaver.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_graphics.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_input.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_m2vplayer.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_math.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_movieplayer.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_rasterize.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_texturemgr.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_timer.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_types.h"\
	{$(INCLUDE)}"hwosxb\xb_algebra.h"\
	{$(INCLUDE)}"hwosxb\xb_alloc.h"\
	{$(INCLUDE)}"hwosxb\xb_errmsg.h"\
	{$(INCLUDE)}"hwosxb\xb_file.h"\
	{$(INCLUDE)}"HWOSXB\xb_gamesaver.h"\
	{$(INCLUDE)}"hwosxb\xb_graphics.h"\
	{$(INCLUDE)}"hwosxb\xb_input.h"\
	{$(INCLUDE)}"hwosxb\xb_math.h"\
	{$(INCLUDE)}"hwosxb\xb_movieplayer.h"\
	{$(INCLUDE)}"HWOSXB\xb_portDVDCache.h"\
	{$(INCLUDE)}"hwosxb\xb_rasterize.h"\
	{$(INCLUDE)}"hwosxb\xb_string.h"\
	{$(INCLUDE)}"hwosxb\xb_texturemgr.h"\
	{$(INCLUDE)}"hwosxb\xb_timer.h"\
	{$(INCLUDE)}"hwosxb\xb_types.h"\
	{$(INCLUDE)}"ks\AccompFrontEnd.h"\
	{$(INCLUDE)}"ks\andersen_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\beach.h"\
	{$(INCLUDE)}"ks\beachdata.h"\
	{$(INCLUDE)}"ks\BeachFrontEnd.h"\
	{$(INCLUDE)}"ks\board.h"\
	{$(INCLUDE)}"ks\board_anims_mac.h"\
	{$(INCLUDE)}"ks\boarddata.h"\
	{$(INCLUDE)}"ks\BoardFrontEnd.h"\
	{$(INCLUDE)}"ks\career.h"\
	{$(INCLUDE)}"ks\careerdata.h"\
	{$(INCLUDE)}"ks\carrol_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\challenge.h"\
	{$(INCLUDE)}"ks\challenge_icon.h"\
	{$(INCLUDE)}"ks\challenge_photo.h"\
	{$(INCLUDE)}"ks\cheat.h"\
	{$(INCLUDE)}"ks\CheatFrontEnd.h"\
	{$(INCLUDE)}"ks\compressedphoto.h"\
	{$(INCLUDE)}"ks\coords.h"\
	{$(INCLUDE)}"ks\curren_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\eventmanager.h"\
	{$(INCLUDE)}"ks\ExtrasFrontEnd.h"\
	{$(INCLUDE)}"ks\FEAnim.h"\
	{$(INCLUDE)}"ks\FEEntityManager.h"\
	{$(INCLUDE)}"ks\FEMenu.h"\
	{$(INCLUDE)}"ks\FEPanel.h"\
	{$(INCLUDE)}"ks\fletcher_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\floatobj.h"\
	{$(INCLUDE)}"ks\frankenreiter_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\FrontEndManager.h"\
	{$(INCLUDE)}"ks\FrontEndMenus.h"\
	{$(INCLUDE)}"ks\GameData.h"\
	{$(INCLUDE)}"ks\GlobalData.h"\
	{$(INCLUDE)}"ks\globaltextenum.h"\
	{$(INCLUDE)}"ks\GraphicalMenuSystem.h"\
	{$(INCLUDE)}"ks\HighScoreFrontEnd.h"\
	{$(INCLUDE)}"ks\igo_widget.h"\
	{$(INCLUDE)}"ks\igo_widget_analogclock.h"\
	{$(INCLUDE)}"ks\igo_widget_balance.h"\
	{$(INCLUDE)}"ks\igo_widget_camera.h"\
	{$(INCLUDE)}"ks\igo_widget_fanmeter.h"\
	{$(INCLUDE)}"ks\igo_widget_grid.h"\
	{$(INCLUDE)}"ks\igo_widget_iconcount.h"\
	{$(INCLUDE)}"ks\igo_widget_iconradar.h"\
	{$(INCLUDE)}"ks\igo_widget_meterchallenge.h"\
	{$(INCLUDE)}"ks\igo_widget_objectalert.h"\
	{$(INCLUDE)}"ks\igo_widget_photo.h"\
	{$(INCLUDE)}"ks\igo_widget_replay.h"\
	{$(INCLUDE)}"ks\igo_widget_simple.h"\
	{$(INCLUDE)}"ks\igo_widget_skillchallenge.h"\
	{$(INCLUDE)}"ks\igo_widget_specialmeter.h"\
	{$(INCLUDE)}"ks\igo_widget_splitclock.h"\
	{$(INCLUDE)}"ks\igo_widget_splitmeter.h"\
	{$(INCLUDE)}"ks\igo_widget_splitscore.h"\
	{$(INCLUDE)}"ks\igo_widget_splitter.h"\
	{$(INCLUDE)}"ks\igo_widget_timeattack.h"\
	{$(INCLUDE)}"ks\igo_widget_waveindicator.h"\
	{$(INCLUDE)}"ks\IGOFrontEnd.h"\
	{$(INCLUDE)}"ks\igohintmanager.h"\
	{$(INCLUDE)}"ks\igoiconmanager.h"\
	{$(INCLUDE)}"ks\igolearn_new_trickmanager.h"\
	{$(INCLUDE)}"ks\ik_object.h"\
	{$(INCLUDE)}"ks\irons_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\judge.h"\
	{$(INCLUDE)}"ks\kellyslater_controller.h"\
	{$(INCLUDE)}"ks\kellyslater_shared_anims_mac.h"\
	{$(INCLUDE)}"ks\kellyslater_states_mac.h"\
	{$(INCLUDE)}"ks\ks_camera.h"\
	{$(INCLUDE)}"ks\ksdbmenu.h"\
	{$(INCLUDE)}"ks\ksngl.h"\
	{$(INCLUDE)}"ks\ksreplay.h"\
	{$(INCLUDE)}"ks\LogbookFrontEnd.h"\
	{$(INCLUDE)}"ks\machado_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\MainFrontEnd.h"\
	{$(INCLUDE)}"ks\Map.h"\
	{$(INCLUDE)}"ks\MCDetectFrontEnd.h"\
	{$(INCLUDE)}"ks\menu.h"\
	{$(INCLUDE)}"ks\menusys.h"\
	{$(INCLUDE)}"ks\mode.h"\
	{$(INCLUDE)}"ks\mode_headtohead.h"\
	{$(INCLUDE)}"ks\mode_meterattack.h"\
	{$(INCLUDE)}"ks\mode_push.h"\
	{$(INCLUDE)}"ks\mode_timeattack.h"\
	{$(INCLUDE)}"ks\MultiFrontEnd.h"\
	{$(INCLUDE)}"ks\MusicMan.h"\
	{$(INCLUDE)}"ks\ode.h"\
	{$(INCLUDE)}"ks\osGameSaver.h"\
	{$(INCLUDE)}"ks\PAL60FrontEnd.h"\
	{$(INCLUDE)}"ks\PathData.h"\
	{$(INCLUDE)}"ks\PhotoFrontEnd.h"\
	{$(INCLUDE)}"ks\physics.h"\
	{$(INCLUDE)}"ks\player.h"\
	{$(INCLUDE)}"ks\replay.h"\
	{$(INCLUDE)}"ks\robb_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\rumbleManager.h"\
	{$(INCLUDE)}"ks\SaveLoadFrontEnd.h"\
	{$(INCLUDE)}"ks\scoringmanager.h"\
	{$(INCLUDE)}"ks\slater_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\sounddata.h"\
	{$(INCLUDE)}"ks\SoundScript.h"\
	{$(INCLUDE)}"ks\specialmeter.h"\
	{$(INCLUDE)}"ks\StatsFrontEnd.h"\
	{$(INCLUDE)}"ks\surferdata.h"\
	{$(INCLUDE)}"ks\SurferFrontEnd.h"\
	{$(INCLUDE)}"ks\trick_system.h"\
	{$(INCLUDE)}"ks\TrickBookFrontEnd.h"\
	{$(INCLUDE)}"ks\trickdata.h"\
	{$(INCLUDE)}"ks\trickregion.h"\
	{$(INCLUDE)}"ks\tricks.h"\
	{$(INCLUDE)}"ks\tutorialdata.h"\
	{$(INCLUDE)}"ks\TutorialFrontEnd.h"\
	{$(INCLUDE)}"ks\tutorialmanager.h"\
	{$(INCLUDE)}"ks\underwtr.h"\
	{$(INCLUDE)}"ks\VOEngine.h"\
	{$(INCLUDE)}"ks\wave.h"\
	{$(INCLUDE)}"ks\wavebreak.txt"\
	{$(INCLUDE)}"ks\wavebreakstage.txt"\
	{$(INCLUDE)}"ks\waveemitter.txt"\
	{$(INCLUDE)}"ks\waveenum.h"\
	{$(INCLUDE)}"ks\wavemarker.txt"\
	{$(INCLUDE)}"ks\waveregion.txt"\
	{$(INCLUDE)}"ks\wavestage.txt"\
	{$(INCLUDE)}"libcdvd.h"\
	{$(INCLUDE)}"libdma.h"\
	{$(INCLUDE)}"libgraph.h"\
	{$(INCLUDE)}"libipu.h"\
	{$(INCLUDE)}"libmc.h"\
	{$(INCLUDE)}"libmpeg.h"\
	{$(INCLUDE)}"libpad.h"\
	{$(INCLUDE)}"libpc.h"\
	{$(INCLUDE)}"libusbkb.h"\
	{$(INCLUDE)}"libvu0.h"\
	{$(INCLUDE)}"sifdev.h"\
	{$(INCLUDE)}"xgraphics.h"\
	{$(INCLUDE)}"zlib\zconf.h"\
	
NODEP_CPP_FILES_V=\
	"..\..\NSL\pc\nl_pc.h"\
	"..\..\NSL\pc\nsl_pc_ext.h"\
	".\defalloc.h"\
	".\gc_arammgr.h"\
	".\hwosmks\mks_devopts.h"\
	".\hwosmks\mks_errmsg.h"\
	".\hwosmks\mks_movieplayer.h"\
	".\hwosmks\set5_rasterize.h"\
	".\hwosmks\set5_timer.h"\
	".\hwosmks\sh4_math.h"\
	".\hwosmks\sy_alloc.h"\
	".\hwosmks\sy_file.h"\
	".\hwosnglpc\algebra.h"\
	".\hwosnglpc\pc_algebra.h"\
	".\hwosnull\null_alloc.h"\
	".\hwosnull\null_devopts.h"\
	".\hwosnull\null_errmsg.h"\
	".\hwosnull\null_file.h"\
	".\hwosnull\null_math.h"\
	".\hwosnull\null_movieplayer.h"\
	".\hwosnull\null_rasterize.h"\
	".\hwosnull\null_timer.h"\
	".\hwospc\d3d_rasterize.h"\
	".\hwospc\pc_algebra.h"\
	".\hwospc\pc_movieplayer.h"\
	".\hwospc\w32_alloc.h"\
	".\hwospc\w32_devopts.h"\
	".\hwospc\w32_errmsg.h"\
	".\hwospc\w32_file.h"\
	".\hwospc\w32_timer.h"\
	".\hwospc\x86_math.h"\
	".\hwosxb\ngl_xbox.h"\
	".\hwosxb\nvl_xbox.h"\
	".\LipSync_Anim.h"\
	".\ngl_xbox.h"\
	".\nvl_gc.h"\
	".\nvl_xbox.h"\
	".\nvlstream_gc.h"\
	
# ADD BASE CPP /Yu
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

DEP_CPP_FILES_V=\
	"..\..\ngl\ps2\ngl_fixedstr.h"\
	"..\..\ngl\ps2\ngl_ps2.h"\
	"..\..\ngl\ps2\tim2.h"\
	"..\..\NSL\common\nl.h"\
	"..\..\NSL\common\nsl.h"\
	"..\..\NSL\common\ProjectOptions.h"\
	"..\..\NSL\gamecube\nl_gc.h"\
	"..\..\NSL\ps2\fifo_queue.cpp"\
	"..\..\NSL\ps2\fifo_queue.h"\
	"..\..\NSL\PS2\gas.h"\
	"..\..\NSL\PS2\gas_utility.h"\
	"..\..\NSL\PS2\nl_ps2.h"\
	"..\..\NSL\PS2\nsl_ps2.h"\
	"..\..\NSL\XBOX\nl_xbox.h"\
	"..\..\NSL\XBOX\nsl_xbox.h"\
	"..\..\NSL\XBOX\nsl_xbox_ext.h"\
	"..\..\NSL\XBOX\nsl_xbox_source.h"\
	"..\..\nsl\xbox\nsl_xbox_streams.h"\
	"..\..\NVL\ps2\nvl_ps2.h"\
	"..\..\NVL\ps2\nvlMPEG_ps2.h"\
	"..\..\NVL\ps2\nvlstream_ps2.h"\
	".\aggvertbuf.cpp"\
	".\aggvertbuf.h"\
	".\AIController.h"\
	".\algebra.h"\
	".\anim.h"\
	".\anim_flavor.h"\
	".\anim_flavors.h"\
	".\anim_ids.h"\
	".\app.h"\
	".\archalloc.h"\
	".\avltree.h"\
	".\b_spline.cpp"\
	".\b_spline.h"\
	".\beam.h"\
	".\beam_signals.h"\
	".\billboard.h"\
	".\bitplane.cpp"\
	".\bitplane.h"\
	".\blendmodes.h"\
	".\bone.h"\
	".\bound.h"\
	".\box_trigger_interface.h"\
	".\camera.h"\
	".\capsule.h"\
	".\cface.h"\
	".\chunkfile.cpp"\
	".\chunkfile.h"\
	".\colgeom.h"\
	".\collide.h"\
	".\colmesh.h"\
	".\color.cpp"\
	".\color.h"\
	".\commands.h"\
	".\conglom.h"\
	".\console.cpp"\
	".\console.h"\
	".\consoleCmds.cpp"\
	".\consoleCmds.h"\
	".\consoleVars.cpp"\
	".\consoleVars.h"\
	".\constants.h"\
	".\controller.h"\
	".\convex_box.h"\
	".\custom_stl.h"\
	".\debug.h"\
	".\debug_render.h"\
	".\debugutil.cpp"\
	".\devoptflags.h"\
	".\devoptints.h"\
	".\devoptstrs.h"\
	".\entflavor.h"\
	".\entity.h"\
	".\entity_anim.h"\
	".\entity_interface.h"\
	".\entity_maker.h"\
	".\entity_signals.h"\
	".\entityflags.h"\
	".\entityid.h"\
	".\errorcontext.cpp"\
	".\errorcontext.h"\
	".\face.h"\
	".\faceflags.h"\
	".\fast_vector.h"\
	".\file.cpp"\
	".\file.h"\
	".\file_finder.h"\
	".\filespec.cpp"\
	".\filespec.h"\
	".\fogmgr.cpp"\
	".\fogmgr.h"\
	".\forceflags.h"\
	".\frame_info.cpp"\
	".\frame_info.h"\
	".\game.h"\
	".\game_info.h"\
	".\game_info_vars.h"\
	".\game_process.h"\
	".\gcglobals.h"\
	".\generator.h"\
	".\geometry.cpp"\
	".\geometry.h"\
	".\geomgr.h"\
	".\global.h"\
	".\global_signals.h"\
	".\graph.h"\
	".\hull.cpp"\
	".\hull.h"\
	".\hwmath.h"\
	".\hwmovieplayer.h"\
	".\hwrasterize.h"\
	".\hyperplane.h"\
	".\ini_parser.h"\
	".\inputmgr.cpp"\
	".\inputmgr.h"\
	".\instance.h"\
	".\iri.h"\
	".\item.h"\
	".\item_signals.h"\
	".\joystick.h"\
	".\keyboard.h"\
	".\ks\eventmanager.cpp"\
	".\ks\ksheaps.h"\
	".\kshooks.h"\
	".\ksnsl.h"\
	".\ksnvl.h"\
	".\light.h"\
	".\linear_anim.h"\
	".\link_interface.h"\
	".\localize.h"\
	".\map_e.h"\
	".\material.cpp"\
	".\material.h"\
	".\matfac.h"\
	".\maxskinbones.h"\
	".\meshrefs.h"\
	".\mic.h"\
	".\mobject.h"\
	".\mustash.cpp"\
	".\mustash.h"\
	".\ngl.h"\
	".\ngl_support.h"\
	".\opcode_arg_t"\
	".\opcode_t"\
	".\opcodes.cpp"\
	".\opcodes.h"\
	".\osalloc.h"\
	".\osassert.cpp"\
	".\osassert.h"\
	".\osdevopts.h"\
	".\oserrmsg.h"\
	".\osfile.h"\
	".\ostimer.h"\
	".\path.cpp"\
	".\path.h"\
	".\pc_port.h"\
	".\plane.cpp"\
	".\plane.h"\
	".\platform_defines.h"\
	".\pmesh.h"\
	".\po.cpp"\
	".\po.h"\
	".\po_anim.h"\
	".\portal.h"\
	".\profiler.cpp"\
	".\profiler.h"\
	".\projconst.h"\
	".\project.h"\
	".\pstring.cpp"\
	".\pstring.h"\
	".\random.h"\
	".\rect.h"\
	".\refptr.h"\
	".\region.h"\
	".\region_graph.h"\
	".\render_data.h"\
	".\renderflav.h"\
	".\scene_anim.h"\
	".\script_lib_mfg.h"\
	".\script_library_class.cpp"\
	".\script_library_class.h"\
	".\script_mfg_signals.h"\
	".\script_object.cpp"\
	".\script_object.h"\
	".\semaphores.h"\
	".\signal.cpp"\
	".\signal_anim.h"\
	".\signals.h"\
	".\SimpleAssert.h"\
	".\singleton.cpp"\
	".\singleton.h"\
	".\sl_debugger.h"\
	".\so_data_block.cpp"\
	".\so_data_block.h"\
	".\sphere.h"\
	".\stash_support.cpp"\
	".\stash_support.h"\
	".\stashes.h"\
	".\staticmem.h"\
	".\stl_adapter.h"\
	".\stringx.cpp"\
	".\stringx.h"\
	".\terrain.h"\
	".\text_font.cpp"\
	".\text_font.h"\
	".\textfile.cpp"\
	".\textfile.h"\
	".\timer.h"\
	".\txtcoord.h"\
	".\types.h"\
	".\usefulmath.h"\
	".\users.h"\
	".\vert.h"\
	".\vertwork.h"\
	".\visrep.h"\
	".\vm_executable.cpp"\
	".\vm_executable.h"\
	".\vm_executable_vector.h"\
	".\vm_stack.cpp"\
	".\vm_stack.h"\
	".\vm_symbol.cpp"\
	".\vm_symbol.h"\
	".\vm_symbol_list.h"\
	".\vm_thread.cpp"\
	".\vm_thread.h"\
	".\warnlvl.h"\
	".\wds.h"\
	".\wedge.h"\
	".\widget.cpp"\
	".\widget.h"\
	".\widget_entity.h"\
	".\xbglobals.h"\
	".\zip_filter.cpp"\
	".\zip_filter.h"\
	".\zlib\zlib.h"\
	
NODEP_CPP_FILES_V=\
	"..\..\NSL\pc\nl_pc.h"\
	"..\..\NSL\pc\nsl_pc_ext.h"\
	"..\..\NSL\XBOX\nsl.h"\
	".\defalloc.h"\
	".\ekernel.h"\
	".\eregs.h"\
	".\estruct.h"\
	".\etypes.h"\
	".\gc_arammgr.h"\
	".\graphics.h"\
	".\hwosgc\nsl.h"\
	".\hwosmks\mks_devopts.h"\
	".\hwosmks\mks_errmsg.h"\
	".\hwosmks\mks_movieplayer.h"\
	".\hwosmks\set5_rasterize.h"\
	".\hwosmks\set5_timer.h"\
	".\hwosmks\sh4_math.h"\
	".\hwosmks\sy_alloc.h"\
	".\hwosmks\sy_file.h"\
	".\hwosnglpc\algebra.h"\
	".\hwosnglpc\pc_algebra.h"\
	".\hwosnull\null_alloc.h"\
	".\hwosnull\null_devopts.h"\
	".\hwosnull\null_errmsg.h"\
	".\hwosnull\null_file.h"\
	".\hwosnull\null_math.h"\
	".\hwosnull\null_movieplayer.h"\
	".\hwosnull\null_rasterize.h"\
	".\hwosnull\null_timer.h"\
	".\hwospc\d3d_rasterize.h"\
	".\hwospc\pc_algebra.h"\
	".\hwospc\pc_movieplayer.h"\
	".\hwospc\w32_alloc.h"\
	".\hwospc\w32_devopts.h"\
	".\hwospc\w32_errmsg.h"\
	".\hwospc\w32_file.h"\
	".\hwospc\w32_timer.h"\
	".\hwospc\x86_math.h"\
	".\hwosxb\ngl_xbox.h"\
	".\hwosxb\nvl_xbox.h"\
	".\ibcdvd.h"\
	".\ibdma.h"\
	".\ibgraph.h"\
	".\ibipu.h"\
	".\ibmc.h"\
	".\ibmpeg.h"\
	".\ibpad.h"\
	".\ibpc.h"\
	".\ibusbkb.h"\
	".\ibvu0.h"\
	".\ifdev.h"\
	".\lib\zconf.h"\
	".\LipSync_Anim.h"\
	".\ngl_xbox.h"\
	".\nvl_gc.h"\
	".\nvl_xbox.h"\
	".\nvlstream_gc.h"\
	".\s\AccompFrontEnd.h"\
	".\s\andersen_ind_anims_mac.h"\
	".\s\beach.h"\
	".\s\beachdata.h"\
	".\s\BeachFrontEnd.h"\
	".\s\board.h"\
	".\s\board_anims_mac.h"\
	".\s\boarddata.h"\
	".\s\BoardFrontEnd.h"\
	".\s\career.h"\
	".\s\careerdata.h"\
	".\s\carrol_ind_anims_mac.h"\
	".\s\challenge.h"\
	".\s\challenge_icon.h"\
	".\s\challenge_photo.h"\
	".\s\cheat.h"\
	".\s\CheatFrontEnd.h"\
	".\s\compressedphoto.h"\
	".\s\coords.h"\
	".\s\curren_ind_anims_mac.h"\
	".\s\eventmanager.h"\
	".\s\ExtrasFrontEnd.h"\
	".\s\FEAnim.h"\
	".\s\FEEntityManager.h"\
	".\s\FEMenu.h"\
	".\s\FEPanel.h"\
	".\s\fletcher_ind_anims_mac.h"\
	".\s\floatobj.h"\
	".\s\frankenreiter_ind_anims_mac.h"\
	".\s\FrontEndManager.h"\
	".\s\FrontEndMenus.h"\
	".\s\GameData.h"\
	".\s\GlobalData.h"\
	".\s\globaltextenum.h"\
	".\s\GraphicalMenuSystem.h"\
	".\s\HighScoreFrontEnd.h"\
	".\s\igo_widget.h"\
	".\s\igo_widget_analogclock.h"\
	".\s\igo_widget_balance.h"\
	".\s\igo_widget_camera.h"\
	".\s\igo_widget_fanmeter.h"\
	".\s\igo_widget_iconradar.h"\
	".\s\igo_widget_photo.h"\
	".\s\igo_widget_replay.h"\
	".\s\igo_widget_simple.h"\
	".\s\igo_widget_specialmeter.h"\
	".\s\igo_widget_splitclock.h"\
	".\s\igo_widget_splitmeter.h"\
	".\s\igo_widget_splitscore.h"\
	".\s\igo_widget_splitter.h"\
	".\s\igo_widget_timeattack.h"\
	".\s\igo_widget_waveindicator.h"\
	".\s\IGOFrontEnd.h"\
	".\s\igohintmanager.h"\
	".\s\igoiconmanager.h"\
	".\s\igolearn_new_trickmanager.h"\
	".\s\ik_object.h"\
	".\s\irons_ind_anims_mac.h"\
	".\s\judge.h"\
	".\s\kellyslater_controller.h"\
	".\s\kellyslater_shared_anims_mac.h"\
	".\s\kellyslater_states_mac.h"\
	".\s\ks_camera.h"\
	".\s\ksdbmenu.h"\
	".\s\ksngl.h"\
	".\s\ksreplay.h"\
	".\s\machado_ind_anims_mac.h"\
	".\s\MainFrontEnd.h"\
	".\s\Map.h"\
	".\s\MCDetectFrontEnd.h"\
	".\s\menu.h"\
	".\s\menusys.h"\
	".\s\mode.h"\
	".\s\mode_headtohead.h"\
	".\s\mode_meterattack.h"\
	".\s\mode_push.h"\
	".\s\mode_timeattack.h"\
	".\s\MultiFrontEnd.h"\
	".\s\MusicMan.h"\
	".\s\ode.h"\
	".\s\osGameSaver.h"\
	".\s\PAL60FrontEnd.h"\
	".\s\PathData.h"\
	".\s\PhotoFrontEnd.h"\
	".\s\physics.h"\
	".\s\player.h"\
	".\s\replay.h"\
	".\s\robb_ind_anims_mac.h"\
	".\s\rumbleManager.h"\
	".\s\SaveLoadFrontEnd.h"\
	".\s\scoringmanager.h"\
	".\s\slater_ind_anims_mac.h"\
	".\s\sounddata.h"\
	".\s\SoundScript.h"\
	".\s\specialmeter.h"\
	".\s\StatsFrontEnd.h"\
	".\s\surferdata.h"\
	".\s\SurferFrontEnd.h"\
	".\s\trick_system.h"\
	".\s\TrickBookFrontEnd.h"\
	".\s\trickdata.h"\
	".\s\trickregion.h"\
	".\s\tricks.h"\
	".\s\TutorialFrontEnd.h"\
	".\s\tutorialmanager.h"\
	".\s\VOEngine.h"\
	".\s\wave.h"\
	".\s\wavebreak.txt"\
	".\s\wavebreakstage.txt"\
	".\s\waveemitter.txt"\
	".\s\waveenum.h"\
	".\s\wavemarker.txt"\
	".\s\waveregion.txt"\
	".\s\wavestage.txt"\
	".\wosgc\gc_algebra.h"\
	".\wosgc\gc_alloc.h"\
	".\wosgc\gc_errmsg.h"\
	".\wosgc\gc_file.h"\
	".\WOSGC\gc_gamesaver.h"\
	".\wosgc\gc_graphics.h"\
	".\wosgc\gc_input.h"\
	".\wosgc\gc_math.h"\
	".\wosgc\gc_movieplayer.h"\
	".\wosgc\gc_rasterize.h"\
	".\wosgc\gc_texturemgr.h"\
	".\wosgc\gc_timer.h"\
	".\wosgc\ngl_fixedstr.h"\
	".\wosgc\ngl_gc.h"\
	".\wosgc\ngl_version.h"\
	".\wosgc\nvl_gc.h"\
	".\WOSPS2\ps2_algebra.h"\
	".\WOSPS2\ps2_alloc.h"\
	".\WOSPS2\ps2_devopts.h"\
	".\WOSPS2\ps2_errmsg.h"\
	".\WOSPS2\ps2_file.h"\
	".\WOSPS2\ps2_gamesaver.h"\
	".\WOSPS2\ps2_graphics.h"\
	".\WOSPS2\ps2_input.h"\
	".\WOSPS2\ps2_m2vplayer.h"\
	".\WOSPS2\ps2_math.h"\
	".\WOSPS2\ps2_movieplayer.h"\
	".\WOSPS2\ps2_rasterize.h"\
	".\WOSPS2\ps2_texturemgr.h"\
	".\WOSPS2\ps2_timer.h"\
	".\WOSPS2\ps2_types.h"\
	".\wosxb\xb_algebra.h"\
	".\wosxb\xb_alloc.h"\
	".\wosxb\xb_errmsg.h"\
	".\wosxb\xb_file.h"\
	".\WOSXB\xb_gamesaver.h"\
	".\wosxb\xb_graphics.h"\
	".\wosxb\xb_input.h"\
	".\wosxb\xb_math.h"\
	".\wosxb\xb_movieplayer.h"\
	".\WOSXB\xb_portDVDCache.h"\
	".\wosxb\xb_rasterize.h"\
	".\wosxb\xb_string.h"\
	".\wosxb\xb_texturemgr.h"\
	".\wosxb\xb_timer.h"\
	".\wosxb\xb_types.h"\
	
# SUBTRACT BASE CPP /YX /Yc /Yu
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

DEP_CPP_FILES_V=\
	"..\..\ngl\ps2\ngl_fixedstr.h"\
	"..\..\ngl\ps2\ngl_ps2.h"\
	"..\..\ngl\ps2\tim2.h"\
	"..\..\NSL\common\nl.h"\
	"..\..\NSL\common\nsl.h"\
	"..\..\NSL\common\ProjectOptions.h"\
	"..\..\NSL\gamecube\nl_gc.h"\
	"..\..\NSL\ps2\fifo_queue.cpp"\
	"..\..\NSL\ps2\fifo_queue.h"\
	"..\..\NSL\PS2\gas.h"\
	"..\..\NSL\PS2\gas_utility.h"\
	"..\..\NSL\PS2\nl_ps2.h"\
	"..\..\NSL\PS2\nsl_ps2.h"\
	"..\..\NSL\XBOX\nl_xbox.h"\
	"..\..\NSL\XBOX\nsl_xbox.h"\
	"..\..\NSL\XBOX\nsl_xbox_ext.h"\
	"..\..\NSL\XBOX\nsl_xbox_source.h"\
	"..\..\nsl\xbox\nsl_xbox_streams.h"\
	"..\..\NVL\ps2\nvl_ps2.h"\
	"..\..\NVL\ps2\nvlMPEG_ps2.h"\
	"..\..\NVL\ps2\nvlstream_ps2.h"\
	".\aggvertbuf.cpp"\
	".\aggvertbuf.h"\
	".\AIController.h"\
	".\algebra.h"\
	".\anim.h"\
	".\anim_flavor.h"\
	".\anim_flavors.h"\
	".\anim_ids.h"\
	".\app.h"\
	".\archalloc.h"\
	".\avltree.h"\
	".\b_spline.cpp"\
	".\b_spline.h"\
	".\beam.h"\
	".\beam_signals.h"\
	".\billboard.h"\
	".\bitplane.cpp"\
	".\bitplane.h"\
	".\blendmodes.h"\
	".\bone.h"\
	".\bound.h"\
	".\box_trigger_interface.h"\
	".\camera.h"\
	".\capsule.h"\
	".\cface.h"\
	".\chunkfile.cpp"\
	".\chunkfile.h"\
	".\colgeom.h"\
	".\collide.h"\
	".\colmesh.h"\
	".\color.cpp"\
	".\color.h"\
	".\commands.h"\
	".\conglom.h"\
	".\console.cpp"\
	".\console.h"\
	".\consoleCmds.cpp"\
	".\consoleCmds.h"\
	".\consoleVars.cpp"\
	".\consoleVars.h"\
	".\constants.h"\
	".\controller.h"\
	".\convex_box.h"\
	".\custom_stl.h"\
	".\debug.h"\
	".\debug_render.h"\
	".\debugutil.cpp"\
	".\devoptflags.h"\
	".\devoptints.h"\
	".\devoptstrs.h"\
	".\entflavor.h"\
	".\entity.h"\
	".\entity_anim.h"\
	".\entity_interface.h"\
	".\entity_maker.h"\
	".\entity_signals.h"\
	".\entityflags.h"\
	".\entityid.h"\
	".\errorcontext.cpp"\
	".\errorcontext.h"\
	".\face.h"\
	".\faceflags.h"\
	".\fast_vector.h"\
	".\file.cpp"\
	".\file.h"\
	".\file_finder.h"\
	".\filespec.cpp"\
	".\filespec.h"\
	".\fogmgr.cpp"\
	".\fogmgr.h"\
	".\forceflags.h"\
	".\frame_info.cpp"\
	".\frame_info.h"\
	".\game.h"\
	".\game_info.h"\
	".\game_info_vars.h"\
	".\game_process.h"\
	".\gcglobals.h"\
	".\generator.h"\
	".\geometry.cpp"\
	".\geometry.h"\
	".\geomgr.h"\
	".\global.h"\
	".\global_signals.h"\
	".\graph.h"\
	".\hull.cpp"\
	".\hull.h"\
	".\hwmath.h"\
	".\hwmovieplayer.h"\
	".\hwrasterize.h"\
	".\hyperplane.h"\
	".\ini_parser.h"\
	".\inputmgr.cpp"\
	".\inputmgr.h"\
	".\instance.h"\
	".\iri.h"\
	".\item.h"\
	".\item_signals.h"\
	".\joystick.h"\
	".\keyboard.h"\
	".\ks\eventmanager.cpp"\
	".\ks\ksheaps.h"\
	".\kshooks.h"\
	".\ksnsl.h"\
	".\ksnvl.h"\
	".\light.h"\
	".\linear_anim.h"\
	".\link_interface.h"\
	".\localize.h"\
	".\map_e.h"\
	".\material.cpp"\
	".\material.h"\
	".\matfac.h"\
	".\maxskinbones.h"\
	".\meshrefs.h"\
	".\mic.h"\
	".\mobject.h"\
	".\mustash.cpp"\
	".\mustash.h"\
	".\ngl.h"\
	".\ngl_support.h"\
	".\opcode_arg_t"\
	".\opcode_t"\
	".\opcodes.cpp"\
	".\opcodes.h"\
	".\osalloc.h"\
	".\osassert.cpp"\
	".\osassert.h"\
	".\osdevopts.h"\
	".\oserrmsg.h"\
	".\osfile.h"\
	".\ostimer.h"\
	".\path.cpp"\
	".\path.h"\
	".\pc_port.h"\
	".\plane.cpp"\
	".\plane.h"\
	".\platform_defines.h"\
	".\pmesh.h"\
	".\po.cpp"\
	".\po.h"\
	".\po_anim.h"\
	".\portal.h"\
	".\profiler.cpp"\
	".\profiler.h"\
	".\projconst.h"\
	".\project.h"\
	".\pstring.cpp"\
	".\pstring.h"\
	".\random.h"\
	".\rect.h"\
	".\refptr.h"\
	".\region.h"\
	".\region_graph.h"\
	".\render_data.h"\
	".\renderflav.h"\
	".\scene_anim.h"\
	".\script_lib_mfg.h"\
	".\script_library_class.cpp"\
	".\script_library_class.h"\
	".\script_mfg_signals.h"\
	".\script_object.cpp"\
	".\script_object.h"\
	".\semaphores.h"\
	".\signal.cpp"\
	".\signal_anim.h"\
	".\signals.h"\
	".\SimpleAssert.h"\
	".\singleton.cpp"\
	".\singleton.h"\
	".\sl_debugger.h"\
	".\so_data_block.cpp"\
	".\so_data_block.h"\
	".\sphere.h"\
	".\stash_support.cpp"\
	".\stash_support.h"\
	".\stashes.h"\
	".\staticmem.h"\
	".\stl_adapter.h"\
	".\stringx.cpp"\
	".\stringx.h"\
	".\terrain.h"\
	".\text_font.cpp"\
	".\text_font.h"\
	".\textfile.cpp"\
	".\textfile.h"\
	".\timer.h"\
	".\txtcoord.h"\
	".\types.h"\
	".\usefulmath.h"\
	".\users.h"\
	".\vert.h"\
	".\vertwork.h"\
	".\visrep.h"\
	".\vm_executable.cpp"\
	".\vm_executable.h"\
	".\vm_executable_vector.h"\
	".\vm_stack.cpp"\
	".\vm_stack.h"\
	".\vm_symbol.cpp"\
	".\vm_symbol.h"\
	".\vm_symbol_list.h"\
	".\vm_thread.cpp"\
	".\vm_thread.h"\
	".\warnlvl.h"\
	".\wds.h"\
	".\wedge.h"\
	".\widget.cpp"\
	".\widget.h"\
	".\widget_entity.h"\
	".\xbglobals.h"\
	".\zip_filter.cpp"\
	".\zip_filter.h"\
	".\zlib\zlib.h"\
	
NODEP_CPP_FILES_V=\
	"..\..\NSL\pc\nl_pc.h"\
	"..\..\NSL\pc\nsl_pc_ext.h"\
	".\defalloc.h"\
	".\ekernel.h"\
	".\eregs.h"\
	".\estruct.h"\
	".\etypes.h"\
	".\gc\nsl_gc.h"\
	".\gc_arammgr.h"\
	".\graphics.h"\
	".\hwosmks\mks_devopts.h"\
	".\hwosmks\mks_errmsg.h"\
	".\hwosmks\mks_movieplayer.h"\
	".\hwosmks\set5_rasterize.h"\
	".\hwosmks\set5_timer.h"\
	".\hwosmks\sh4_math.h"\
	".\hwosmks\sy_alloc.h"\
	".\hwosmks\sy_file.h"\
	".\hwosnglpc\algebra.h"\
	".\hwosnglpc\pc_algebra.h"\
	".\hwosnull\null_alloc.h"\
	".\hwosnull\null_devopts.h"\
	".\hwosnull\null_errmsg.h"\
	".\hwosnull\null_file.h"\
	".\hwosnull\null_math.h"\
	".\hwosnull\null_movieplayer.h"\
	".\hwosnull\null_rasterize.h"\
	".\hwosnull\null_timer.h"\
	".\hwospc\d3d_rasterize.h"\
	".\hwospc\pc_algebra.h"\
	".\hwospc\pc_movieplayer.h"\
	".\hwospc\w32_alloc.h"\
	".\hwospc\w32_devopts.h"\
	".\hwospc\w32_errmsg.h"\
	".\hwospc\w32_file.h"\
	".\hwospc\w32_timer.h"\
	".\hwospc\x86_math.h"\
	".\hwosxb\ngl_xbox.h"\
	".\hwosxb\nvl_xbox.h"\
	".\ibcdvd.h"\
	".\ibdma.h"\
	".\ibgraph.h"\
	".\ibipu.h"\
	".\ibmc.h"\
	".\ibmpeg.h"\
	".\ibpad.h"\
	".\ibpc.h"\
	".\ibusbkb.h"\
	".\ibvu0.h"\
	".\ifdev.h"\
	".\lib\zconf.h"\
	".\LipSync_Anim.h"\
	".\ngl_xbox.h"\
	".\nsl.h"\
	".\nvl_gc.h"\
	".\nvl_xbox.h"\
	".\nvlstream_gc.h"\
	".\s\AccompFrontEnd.h"\
	".\s\andersen_ind_anims_mac.h"\
	".\s\beach.h"\
	".\s\beachdata.h"\
	".\s\BeachFrontEnd.h"\
	".\s\board.h"\
	".\s\board_anims_mac.h"\
	".\s\boarddata.h"\
	".\s\BoardFrontEnd.h"\
	".\s\career.h"\
	".\s\careerdata.h"\
	".\s\carrol_ind_anims_mac.h"\
	".\s\challenge.h"\
	".\s\challenge_icon.h"\
	".\s\challenge_photo.h"\
	".\s\cheat.h"\
	".\s\CheatFrontEnd.h"\
	".\s\compressedphoto.h"\
	".\s\coords.h"\
	".\s\curren_ind_anims_mac.h"\
	".\s\eventmanager.h"\
	".\s\ExtrasFrontEnd.h"\
	".\s\FEAnim.h"\
	".\s\FEEntityManager.h"\
	".\s\FEMenu.h"\
	".\s\FEPanel.h"\
	".\s\fletcher_ind_anims_mac.h"\
	".\s\floatobj.h"\
	".\s\frankenreiter_ind_anims_mac.h"\
	".\s\FrontEndManager.h"\
	".\s\FrontEndMenus.h"\
	".\s\GameData.h"\
	".\s\GlobalData.h"\
	".\s\globaltextenum.h"\
	".\s\GraphicalMenuSystem.h"\
	".\s\HighScoreFrontEnd.h"\
	".\s\igo_widget.h"\
	".\s\igo_widget_analogclock.h"\
	".\s\igo_widget_balance.h"\
	".\s\igo_widget_camera.h"\
	".\s\igo_widget_fanmeter.h"\
	".\s\igo_widget_iconcount.h"\
	".\s\igo_widget_iconradar.h"\
	".\s\igo_widget_meterchallenge.h"\
	".\s\igo_widget_objectalert.h"\
	".\s\igo_widget_photo.h"\
	".\s\igo_widget_replay.h"\
	".\s\igo_widget_simple.h"\
	".\s\igo_widget_skillchallenge.h"\
	".\s\igo_widget_specialmeter.h"\
	".\s\igo_widget_splitclock.h"\
	".\s\igo_widget_splitmeter.h"\
	".\s\igo_widget_splitscore.h"\
	".\s\igo_widget_splitter.h"\
	".\s\igo_widget_timeattack.h"\
	".\s\igo_widget_waveindicator.h"\
	".\s\IGOFrontEnd.h"\
	".\s\igohintmanager.h"\
	".\s\igoiconmanager.h"\
	".\s\igolearn_new_trickmanager.h"\
	".\s\ik_object.h"\
	".\s\irons_ind_anims_mac.h"\
	".\s\judge.h"\
	".\s\kellyslater_controller.h"\
	".\s\kellyslater_shared_anims_mac.h"\
	".\s\kellyslater_states_mac.h"\
	".\s\ks_camera.h"\
	".\s\ksdbmenu.h"\
	".\s\ksngl.h"\
	".\s\ksreplay.h"\
	".\s\LogbookFrontEnd.h"\
	".\s\machado_ind_anims_mac.h"\
	".\s\MainFrontEnd.h"\
	".\s\Map.h"\
	".\s\MCDetectFrontEnd.h"\
	".\s\menu.h"\
	".\s\menusys.h"\
	".\s\mode.h"\
	".\s\mode_headtohead.h"\
	".\s\mode_meterattack.h"\
	".\s\mode_push.h"\
	".\s\mode_timeattack.h"\
	".\s\MultiFrontEnd.h"\
	".\s\MusicMan.h"\
	".\s\ode.h"\
	".\s\osGameSaver.h"\
	".\s\PAL60FrontEnd.h"\
	".\s\PathData.h"\
	".\s\PhotoFrontEnd.h"\
	".\s\physics.h"\
	".\s\player.h"\
	".\s\replay.h"\
	".\s\robb_ind_anims_mac.h"\
	".\s\rumbleManager.h"\
	".\s\SaveLoadFrontEnd.h"\
	".\s\scoringmanager.h"\
	".\s\slater_ind_anims_mac.h"\
	".\s\sounddata.h"\
	".\s\SoundScript.h"\
	".\s\specialmeter.h"\
	".\s\StatsFrontEnd.h"\
	".\s\surferdata.h"\
	".\s\SurferFrontEnd.h"\
	".\s\trick_system.h"\
	".\s\TrickBookFrontEnd.h"\
	".\s\trickdata.h"\
	".\s\trickregion.h"\
	".\s\tricks.h"\
	".\s\tutorialdata.h"\
	".\s\TutorialFrontEnd.h"\
	".\s\tutorialmanager.h"\
	".\s\VOEngine.h"\
	".\s\wave.h"\
	".\s\wavebreak.txt"\
	".\s\wavebreakstage.txt"\
	".\s\waveemitter.txt"\
	".\s\waveenum.h"\
	".\s\wavemarker.txt"\
	".\s\waveregion.txt"\
	".\s\wavestage.txt"\
	".\wosgc\gc_algebra.h"\
	".\wosgc\gc_alloc.h"\
	".\wosgc\gc_errmsg.h"\
	".\wosgc\gc_file.h"\
	".\WOSGC\gc_gamesaver.h"\
	".\wosgc\gc_graphics.h"\
	".\wosgc\gc_input.h"\
	".\wosgc\gc_math.h"\
	".\wosgc\gc_movieplayer.h"\
	".\wosgc\gc_rasterize.h"\
	".\wosgc\gc_texturemgr.h"\
	".\wosgc\gc_timer.h"\
	".\wosgc\ngl_fixedstr.h"\
	".\wosgc\ngl_gc.h"\
	".\wosgc\ngl_version.h"\
	".\wosgc\nvl_gc.h"\
	".\WOSPS2\ps2_algebra.h"\
	".\WOSPS2\ps2_alloc.h"\
	".\WOSPS2\ps2_devopts.h"\
	".\WOSPS2\ps2_errmsg.h"\
	".\WOSPS2\ps2_file.h"\
	".\WOSPS2\ps2_gamesaver.h"\
	".\WOSPS2\ps2_graphics.h"\
	".\WOSPS2\ps2_input.h"\
	".\WOSPS2\ps2_m2vplayer.h"\
	".\WOSPS2\ps2_math.h"\
	".\WOSPS2\ps2_movieplayer.h"\
	".\WOSPS2\ps2_rasterize.h"\
	".\WOSPS2\ps2_texturemgr.h"\
	".\WOSPS2\ps2_timer.h"\
	".\WOSPS2\ps2_types.h"\
	".\wosxb\xb_algebra.h"\
	".\wosxb\xb_alloc.h"\
	".\wosxb\xb_errmsg.h"\
	".\wosxb\xb_file.h"\
	".\WOSXB\xb_gamesaver.h"\
	".\wosxb\xb_graphics.h"\
	".\wosxb\xb_input.h"\
	".\wosxb\xb_math.h"\
	".\wosxb\xb_movieplayer.h"\
	".\WOSXB\xb_portDVDCache.h"\
	".\wosxb\xb_rasterize.h"\
	".\wosxb\xb_string.h"\
	".\wosxb\xb_texturemgr.h"\
	".\wosxb\xb_timer.h"\
	".\wosxb\xb_types.h"\
	
# SUBTRACT BASE CPP /YX /Yc /Yu
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ks\files_wave.cpp

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

DEP_CPP_FILES_W=\
	"..\..\NGL\ps2\libsn.h"\
	"..\..\ngl\ps2\matrix.h"\
	"..\..\ngl\ps2\matrix_common.h"\
	"..\..\ngl\ps2\ngl_dma.h"\
	"..\..\ngl\ps2\ngl_fixedstr.h"\
	"..\..\ngl\ps2\ngl_instbank.h"\
	"..\..\ngl\ps2\ngl_ps2.h"\
	"..\..\ngl\ps2\ngl_ps2_internal.h"\
	"..\..\NGL\ps2\ngl_vudefs.h"\
	"..\..\ngl\ps2\tim2.h"\
	"..\..\ngl\ps2\vector.h"\
	"..\..\ngl\ps2\vector_common.h"\
	"..\..\NSL\common\nl.h"\
	"..\..\NSL\common\nsl.h"\
	"..\..\NSL\common\ProjectOptions.h"\
	"..\..\NSL\gamecube\nl_gc.h"\
	"..\..\NSL\ps2\fifo_queue.cpp"\
	"..\..\NSL\ps2\fifo_queue.h"\
	"..\..\NSL\PS2\gas.h"\
	"..\..\NSL\PS2\gas_utility.h"\
	"..\..\NSL\PS2\nl_ps2.h"\
	"..\..\NSL\PS2\nsl_ps2.h"\
	"..\..\NSL\XBOX\nl_xbox.h"\
	"..\..\NSL\XBOX\nsl_xbox.h"\
	"..\..\NSL\XBOX\nsl_xbox_ext.h"\
	"..\..\NSL\XBOX\nsl_xbox_source.h"\
	"..\..\nsl\xbox\nsl_xbox_streams.h"\
	"..\..\NVL\ps2\nvl_ps2.h"\
	"..\..\NVL\ps2\nvlMPEG_ps2.h"\
	"..\..\NVL\ps2\nvlstream_ps2.h"\
	".\aggvertbuf.h"\
	".\algebra.h"\
	".\anim.h"\
	".\anim_flavor.h"\
	".\anim_flavors.h"\
	".\anim_ids.h"\
	".\app.h"\
	".\archalloc.h"\
	".\avltree.h"\
	".\beam.h"\
	".\beam_signals.h"\
	".\billboard.h"\
	".\bone.h"\
	".\bound.h"\
	".\box_trigger_interface.h"\
	".\camera.h"\
	".\capsule.h"\
	".\cface.h"\
	".\chunkfile.h"\
	".\colgeom.h"\
	".\colmesh.h"\
	".\color.h"\
	".\commands.h"\
	".\conglom.h"\
	".\constants.h"\
	".\controller.h"\
	".\convex_box.h"\
	".\custom_stl.h"\
	".\debug.h"\
	".\devoptflags.h"\
	".\devoptints.h"\
	".\devoptstrs.h"\
	".\entflavor.h"\
	".\entity.h"\
	".\entity_anim.h"\
	".\entity_interface.h"\
	".\entity_maker.h"\
	".\entity_signals.h"\
	".\entityflags.h"\
	".\entityid.h"\
	".\errorcontext.h"\
	".\face.h"\
	".\faceflags.h"\
	".\fast_vector.h"\
	".\file.h"\
	".\file_finder.h"\
	".\filespec.h"\
	".\frame_info.h"\
	".\game.h"\
	".\game_info_vars.h"\
	".\game_process.h"\
	".\gcglobals.h"\
	".\generator.h"\
	".\geomgr.h"\
	".\global.h"\
	".\global_signals.h"\
	".\graph.h"\
	".\hull.h"\
	".\hwmath.h"\
	".\hwmovieplayer.h"\
	".\hwrasterize.h"\
	".\hyperplane.h"\
	".\ini_parser.h"\
	".\inputmgr.h"\
	".\instance.h"\
	".\item.h"\
	".\item_signals.h"\
	".\joystick.h"\
	".\ks\ksfx.cpp"\
	".\ks\ksfx.h"\
	".\ks\ksheaps.h"\
	".\ks\ksngl.cpp"\
	".\ks\ksngl_wave_gc.cpp"\
	".\ks\oswaverender.h"\
	".\ks\refract.h"\
	".\ks\SFXEngine.h"\
	".\ks\shadow.cpp"\
	".\ks\sin.txt"\
	".\ks\spline.h"\
	".\ks\trail.cpp"\
	".\ks\trail.h"\
	".\ks\underwtr.cpp"\
	".\ks\water.cpp"\
	".\ks\water.h"\
	".\ks\watercolors.h"\
	".\ks\wave.cpp"\
	".\ks\wavedata.cpp"\
	".\ks\wavedata.h"\
	".\ks\wavedebug.txt"\
	".\ks\wavemenu.cpp"\
	".\ks\wavesound.h"\
	".\ks\wavetex.cpp"\
	".\ks\wavetex.h"\
	".\ks\wavetexdebug.txt"\
	".\ks\wavetexmenu.cpp"\
	".\kshooks.h"\
	".\ksnsl.h"\
	".\ksnvl.h"\
	".\light.h"\
	".\linear_anim.h"\
	".\link_interface.h"\
	".\map_e.h"\
	".\material.h"\
	".\matfac.h"\
	".\maxskinbones.h"\
	".\meshrefs.h"\
	".\mic.h"\
	".\mobject.h"\
	".\mustash.h"\
	".\ngl.h"\
	".\osalloc.h"\
	".\osassert.h"\
	".\osdevopts.h"\
	".\oserrmsg.h"\
	".\osfile.h"\
	".\ostimer.h"\
	".\particle.h"\
	".\path.h"\
	".\pc_port.h"\
	".\plane.h"\
	".\platform_defines.h"\
	".\pmesh.h"\
	".\po.h"\
	".\po_anim.h"\
	".\portal.h"\
	".\profiler.h"\
	".\projconst.h"\
	".\project.h"\
	".\pstring.h"\
	".\random.h"\
	".\rect.h"\
	".\refptr.h"\
	".\region.h"\
	".\region_graph.h"\
	".\render_data.h"\
	".\renderflav.h"\
	".\scene_anim.h"\
	".\script_lib_mfg.h"\
	".\script_library_class.h"\
	".\script_mfg_signals.h"\
	".\script_object.h"\
	".\signal_anim.h"\
	".\signals.h"\
	".\SimpleAssert.h"\
	".\singleton.h"\
	".\so_data_block.h"\
	".\sphere.h"\
	".\stashes.h"\
	".\staticmem.h"\
	".\stl_adapter.h"\
	".\stringx.h"\
	".\terrain.h"\
	".\text_font.h"\
	".\textfile.h"\
	".\timer.h"\
	".\txtcoord.h"\
	".\types.h"\
	".\usefulmath.h"\
	".\users.h"\
	".\vert.h"\
	".\visrep.h"\
	".\vm_executable.h"\
	".\vm_symbol.h"\
	".\vm_symbol_list.h"\
	".\warnlvl.h"\
	".\wds.h"\
	".\wedge.h"\
	".\xbglobals.h"\
	{$(INCLUDE)}"eekernel.h"\
	{$(INCLUDE)}"eeregs.h"\
	{$(INCLUDE)}"eestruct.h"\
	{$(INCLUDE)}"eetypes.h"\
	{$(INCLUDE)}"hwosgc\gc_algebra.h"\
	{$(INCLUDE)}"hwosgc\gc_alloc.h"\
	{$(INCLUDE)}"hwosgc\gc_errmsg.h"\
	{$(INCLUDE)}"hwosgc\gc_file.h"\
	{$(INCLUDE)}"HWOSGC\gc_gamesaver.h"\
	{$(INCLUDE)}"hwosgc\gc_graphics.h"\
	{$(INCLUDE)}"hwosgc\gc_input.h"\
	{$(INCLUDE)}"hwosgc\gc_math.h"\
	{$(INCLUDE)}"hwosgc\gc_movieplayer.h"\
	{$(INCLUDE)}"hwosgc\gc_rasterize.h"\
	{$(INCLUDE)}"hwosgc\gc_texturemgr.h"\
	{$(INCLUDE)}"hwosgc\gc_timer.h"\
	{$(INCLUDE)}"hwosgc\ngl_fixedstr.h"\
	{$(INCLUDE)}"hwosgc\ngl_gc.h"\
	{$(INCLUDE)}"hwosgc\ngl_version.h"\
	{$(INCLUDE)}"hwosgc\nvl_gc.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_algebra.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_alloc.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_devopts.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_errmsg.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_file.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_gamesaver.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_graphics.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_input.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_m2vplayer.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_math.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_movieplayer.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_rasterize.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_texturemgr.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_timer.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_types.h"\
	{$(INCLUDE)}"HWOSPS2\ps2_waverender.h"\
	{$(INCLUDE)}"hwosxb\xb_algebra.h"\
	{$(INCLUDE)}"hwosxb\xb_alloc.h"\
	{$(INCLUDE)}"hwosxb\xb_errmsg.h"\
	{$(INCLUDE)}"hwosxb\xb_file.h"\
	{$(INCLUDE)}"HWOSXB\xb_gamesaver.h"\
	{$(INCLUDE)}"hwosxb\xb_graphics.h"\
	{$(INCLUDE)}"hwosxb\xb_input.h"\
	{$(INCLUDE)}"hwosxb\xb_math.h"\
	{$(INCLUDE)}"hwosxb\xb_movieplayer.h"\
	{$(INCLUDE)}"HWOSXB\xb_particle.h"\
	{$(INCLUDE)}"HWOSXB\xb_portDVDCache.h"\
	{$(INCLUDE)}"hwosxb\xb_rasterize.h"\
	{$(INCLUDE)}"hwosxb\xb_string.h"\
	{$(INCLUDE)}"hwosxb\xb_texturemgr.h"\
	{$(INCLUDE)}"hwosxb\xb_timer.h"\
	{$(INCLUDE)}"hwosxb\xb_types.h"\
	{$(INCLUDE)}"HWOSXB\xb_waverender.h"\
	{$(INCLUDE)}"ks\AccompFrontEnd.h"\
	{$(INCLUDE)}"ks\andersen_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\beach.h"\
	{$(INCLUDE)}"ks\beachdata.h"\
	{$(INCLUDE)}"ks\BeachFrontEnd.h"\
	{$(INCLUDE)}"ks\blur.h"\
	{$(INCLUDE)}"ks\board.h"\
	{$(INCLUDE)}"ks\board_anims_mac.h"\
	{$(INCLUDE)}"ks\boarddata.h"\
	{$(INCLUDE)}"ks\BoardFrontEnd.h"\
	{$(INCLUDE)}"ks\career.h"\
	{$(INCLUDE)}"ks\careerdata.h"\
	{$(INCLUDE)}"ks\carrol_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\challenge.h"\
	{$(INCLUDE)}"ks\challenge_icon.h"\
	{$(INCLUDE)}"ks\challenge_photo.h"\
	{$(INCLUDE)}"ks\cheat.h"\
	{$(INCLUDE)}"ks\CheatFrontEnd.h"\
	{$(INCLUDE)}"ks\compressedphoto.h"\
	{$(INCLUDE)}"ks\coords.h"\
	{$(INCLUDE)}"ks\curren_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\eventmanager.h"\
	{$(INCLUDE)}"ks\ExtrasFrontEnd.h"\
	{$(INCLUDE)}"ks\FEAnim.h"\
	{$(INCLUDE)}"ks\FEEntityManager.h"\
	{$(INCLUDE)}"ks\FEMenu.h"\
	{$(INCLUDE)}"ks\FEPanel.h"\
	{$(INCLUDE)}"ks\fletcher_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\floatobj.h"\
	{$(INCLUDE)}"ks\frankenreiter_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\FrontEndManager.h"\
	{$(INCLUDE)}"ks\FrontEndMenus.h"\
	{$(INCLUDE)}"ks\GameData.h"\
	{$(INCLUDE)}"ks\GlobalData.h"\
	{$(INCLUDE)}"ks\globaltextenum.h"\
	{$(INCLUDE)}"ks\GraphicalMenuSystem.h"\
	{$(INCLUDE)}"ks\HighScoreFrontEnd.h"\
	{$(INCLUDE)}"ks\igo_widget.h"\
	{$(INCLUDE)}"ks\igo_widget_analogclock.h"\
	{$(INCLUDE)}"ks\igo_widget_balance.h"\
	{$(INCLUDE)}"ks\igo_widget_camera.h"\
	{$(INCLUDE)}"ks\igo_widget_fanmeter.h"\
	{$(INCLUDE)}"ks\igo_widget_grid.h"\
	{$(INCLUDE)}"ks\igo_widget_iconcount.h"\
	{$(INCLUDE)}"ks\igo_widget_iconradar.h"\
	{$(INCLUDE)}"ks\igo_widget_meterchallenge.h"\
	{$(INCLUDE)}"ks\igo_widget_objectalert.h"\
	{$(INCLUDE)}"ks\igo_widget_photo.h"\
	{$(INCLUDE)}"ks\igo_widget_replay.h"\
	{$(INCLUDE)}"ks\igo_widget_simple.h"\
	{$(INCLUDE)}"ks\igo_widget_skillchallenge.h"\
	{$(INCLUDE)}"ks\igo_widget_specialmeter.h"\
	{$(INCLUDE)}"ks\igo_widget_splitclock.h"\
	{$(INCLUDE)}"ks\igo_widget_splitmeter.h"\
	{$(INCLUDE)}"ks\igo_widget_splitscore.h"\
	{$(INCLUDE)}"ks\igo_widget_splitter.h"\
	{$(INCLUDE)}"ks\igo_widget_timeattack.h"\
	{$(INCLUDE)}"ks\igo_widget_waveindicator.h"\
	{$(INCLUDE)}"ks\IGOFrontEnd.h"\
	{$(INCLUDE)}"ks\igohintmanager.h"\
	{$(INCLUDE)}"ks\igoiconmanager.h"\
	{$(INCLUDE)}"ks\igolearn_new_trickmanager.h"\
	{$(INCLUDE)}"ks\ik_object.h"\
	{$(INCLUDE)}"ks\irons_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\judge.h"\
	{$(INCLUDE)}"ks\kellyslater_controller.h"\
	{$(INCLUDE)}"ks\kellyslater_shared_anims_mac.h"\
	{$(INCLUDE)}"ks\kellyslater_states_mac.h"\
	{$(INCLUDE)}"ks\ks_camera.h"\
	{$(INCLUDE)}"ks\ksdbmenu.h"\
	{$(INCLUDE)}"ks\ksngl.h"\
	{$(INCLUDE)}"ks\ksreplay.h"\
	{$(INCLUDE)}"ks\LogbookFrontEnd.h"\
	{$(INCLUDE)}"ks\machado_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\MainFrontEnd.h"\
	{$(INCLUDE)}"ks\Map.h"\
	{$(INCLUDE)}"ks\MCDetectFrontEnd.h"\
	{$(INCLUDE)}"ks\menu.h"\
	{$(INCLUDE)}"ks\menusys.h"\
	{$(INCLUDE)}"ks\mode.h"\
	{$(INCLUDE)}"ks\mode_headtohead.h"\
	{$(INCLUDE)}"ks\mode_meterattack.h"\
	{$(INCLUDE)}"ks\mode_push.h"\
	{$(INCLUDE)}"ks\mode_timeattack.h"\
	{$(INCLUDE)}"ks\MultiFrontEnd.h"\
	{$(INCLUDE)}"ks\MusicMan.h"\
	{$(INCLUDE)}"ks\ode.h"\
	{$(INCLUDE)}"ks\osGameSaver.h"\
	{$(INCLUDE)}"ks\osparticle.h"\
	{$(INCLUDE)}"ks\PAL60FrontEnd.h"\
	{$(INCLUDE)}"ks\PathData.h"\
	{$(INCLUDE)}"ks\PhotoFrontEnd.h"\
	{$(INCLUDE)}"ks\physics.h"\
	{$(INCLUDE)}"ks\player.h"\
	{$(INCLUDE)}"ks\replay.h"\
	{$(INCLUDE)}"ks\robb_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\rumbleManager.h"\
	{$(INCLUDE)}"ks\SaveLoadFrontEnd.h"\
	{$(INCLUDE)}"ks\scoringmanager.h"\
	{$(INCLUDE)}"ks\slater_ind_anims_mac.h"\
	{$(INCLUDE)}"ks\sounddata.h"\
	{$(INCLUDE)}"ks\SoundScript.h"\
	{$(INCLUDE)}"ks\specialmeter.h"\
	{$(INCLUDE)}"ks\StatsFrontEnd.h"\
	{$(INCLUDE)}"ks\surferdata.h"\
	{$(INCLUDE)}"ks\SurferFrontEnd.h"\
	{$(INCLUDE)}"ks\trick_system.h"\
	{$(INCLUDE)}"ks\TrickBookFrontEnd.h"\
	{$(INCLUDE)}"ks\trickdata.h"\
	{$(INCLUDE)}"ks\trickregion.h"\
	{$(INCLUDE)}"ks\tricks.h"\
	{$(INCLUDE)}"ks\tutorialdata.h"\
	{$(INCLUDE)}"ks\TutorialFrontEnd.h"\
	{$(INCLUDE)}"ks\tutorialmanager.h"\
	{$(INCLUDE)}"ks\underwtr.h"\
	{$(INCLUDE)}"ks\VOEngine.h"\
	{$(INCLUDE)}"ks\wave.h"\
	{$(INCLUDE)}"ks\wavebreak.txt"\
	{$(INCLUDE)}"ks\wavebreakstage.txt"\
	{$(INCLUDE)}"ks\waveemitter.txt"\
	{$(INCLUDE)}"ks\waveenum.h"\
	{$(INCLUDE)}"ks\wavemarker.txt"\
	{$(INCLUDE)}"ks\waveregion.txt"\
	{$(INCLUDE)}"ks\wavestage.txt"\
	{$(INCLUDE)}"libcdvd.h"\
	{$(INCLUDE)}"libdma.h"\
	{$(INCLUDE)}"libdmapk.h"\
	{$(INCLUDE)}"libgifpk.h"\
	{$(INCLUDE)}"libgraph.h"\
	{$(INCLUDE)}"libipu.h"\
	{$(INCLUDE)}"libmc.h"\
	{$(INCLUDE)}"libmpeg.h"\
	{$(INCLUDE)}"libpad.h"\
	{$(INCLUDE)}"libpc.h"\
	{$(INCLUDE)}"libpkt.h"\
	{$(INCLUDE)}"libusbkb.h"\
	{$(INCLUDE)}"libvifpk.h"\
	{$(INCLUDE)}"libvu0.h"\
	{$(INCLUDE)}"sifdev.h"\
	{$(INCLUDE)}"sifrpc.h"\
	{$(INCLUDE)}"xgraphics.h"\
	
NODEP_CPP_FILES_W=\
	"..\..\NSL\pc\nl_pc.h"\
	"..\..\NSL\pc\nsl_pc_ext.h"\
	".\defalloc.h"\
	".\gc_arammgr.h"\
	".\hwosmks\mks_devopts.h"\
	".\hwosmks\mks_errmsg.h"\
	".\hwosmks\mks_movieplayer.h"\
	".\hwosmks\set5_rasterize.h"\
	".\hwosmks\set5_timer.h"\
	".\hwosmks\sh4_math.h"\
	".\hwosmks\sy_alloc.h"\
	".\hwosmks\sy_file.h"\
	".\hwosnglpc\algebra.h"\
	".\hwosnglpc\pc_algebra.h"\
	".\hwosnull\null_alloc.h"\
	".\hwosnull\null_devopts.h"\
	".\hwosnull\null_errmsg.h"\
	".\hwosnull\null_file.h"\
	".\hwosnull\null_math.h"\
	".\hwosnull\null_movieplayer.h"\
	".\hwosnull\null_rasterize.h"\
	".\hwosnull\null_timer.h"\
	".\hwospc\d3d_rasterize.h"\
	".\hwospc\pc_algebra.h"\
	".\hwospc\pc_movieplayer.h"\
	".\hwospc\w32_alloc.h"\
	".\hwospc\w32_devopts.h"\
	".\hwospc\w32_errmsg.h"\
	".\hwospc\w32_file.h"\
	".\hwospc\w32_timer.h"\
	".\hwospc\x86_math.h"\
	".\hwosxb\ngl_xbox.h"\
	".\hwosxb\nvl_xbox.h"\
	".\ks\gc_ifl.h"\
	".\ks\ngl_gc_internal.h"\
	".\ks\ngl_gc_profile_internal.h"\
	".\ks\ngl_xbox_internal.h"\
	".\ks\pSrcBits.txt"\
	".\LipSync_Anim.h"\
	".\ngl_xbox.h"\
	".\nvl_gc.h"\
	".\nvl_xbox.h"\
	".\nvlstream_gc.h"\
	
# ADD BASE CPP /Yu
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

DEP_CPP_FILES_W=\
	"..\..\NGL\ps2\libsn.h"\
	"..\..\ngl\ps2\matrix.h"\
	"..\..\ngl\ps2\matrix_common.h"\
	"..\..\ngl\ps2\ngl_dma.h"\
	"..\..\ngl\ps2\ngl_fixedstr.h"\
	"..\..\ngl\ps2\ngl_instbank.h"\
	"..\..\ngl\ps2\ngl_ps2.h"\
	"..\..\ngl\ps2\ngl_ps2_internal.h"\
	"..\..\NGL\ps2\ngl_vudefs.h"\
	"..\..\ngl\ps2\tim2.h"\
	"..\..\ngl\ps2\vector.h"\
	"..\..\ngl\ps2\vector_common.h"\
	"..\..\NGL\XBOX\ngl_FixedStr.h"\
	"..\..\NGL\XBOX\ngl_types.h"\
	"..\..\NGL\XBOX\ngl_xbox.h"\
	".\ks\ksfx.cpp"\
	".\ks\ksfx.h"\
	".\ks\ksngl.cpp"\
	".\ks\ksngl_wave_gc.cpp"\
	".\ks\oswaverender.h"\
	".\ks\SFXEngine.h"\
	".\ks\shadow.cpp"\
	".\ks\sin.txt"\
	".\ks\spline.h"\
	".\ks\trail.cpp"\
	".\ks\trail.h"\
	".\ks\underwtr.cpp"\
	".\ks\water.cpp"\
	".\ks\water.h"\
	".\ks\watercolors.h"\
	".\ks\wave.cpp"\
	".\ks\wavedata.cpp"\
	".\ks\wavedata.h"\
	".\ks\wavedebug.txt"\
	".\ks\wavemenu.cpp"\
	".\ks\wavesound.h"\
	".\ks\wavetex.cpp"\
	".\ks\wavetex.h"\
	".\ks\wavetexdebug.txt"\
	".\ks\wavetexmenu.cpp"\
	
NODEP_CPP_FILES_W=\
	"..\..\ngl\common\ngl.h"\
	"..\..\ngl\common\types.h"\
	".\ekernel.h"\
	".\eregs.h"\
	".\estruct.h"\
	".\etypes.h"\
	".\ibdma.h"\
	".\ibdmapk.h"\
	".\ibgifpk.h"\
	".\ibgraph.h"\
	".\ibpc.h"\
	".\ibpkt.h"\
	".\ibvifpk.h"\
	".\ibvu0.h"\
	".\ifdev.h"\
	".\ifrpc.h"\
	".\ks\algebra.h"\
	".\ks\app.h"\
	".\ks\billboard.h"\
	".\ks\camera.h"\
	".\ks\commands.h"\
	".\ks\conglom.h"\
	".\ks\controller.h"\
	".\ks\entity.h"\
	".\ks\entity_anim.h"\
	".\ks\entity_maker.h"\
	".\ks\entityflags.h"\
	".\ks\file_finder.h"\
	".\ks\game.h"\
	".\ks\gc_ifl.h"\
	".\ks\geomgr.h"\
	".\ks\global.h"\
	".\ks\hwosgc\gc_GameSaver.h"\
	".\ks\hwosgc\gc_input.h"\
	".\ks\hwosps2\ps2_GameSaver.h"\
	".\ks\hwosps2\ps2_input.h"\
	".\ks\hwosps2\ps2_m2vplayer.h"\
	".\ks\hwosps2\ps2_waverender.h"\
	".\ks\HWOSXB\xb_algebra.h"\
	".\ks\hwosxb\xb_GameSaver.h"\
	".\ks\hwosxb\xb_input.h"\
	".\ks\hwosxb\xb_particle.h"\
	".\ks\hwosxb\xb_waverender.h"\
	".\ks\inputmgr.h"\
	".\ks\joystick.h"\
	".\ks\kshooks.h"\
	".\ks\ksnsl.h"\
	".\ks\ksnvl.h"\
	".\ks\light.h"\
	".\ks\mustash.h"\
	".\ks\ngl_gc_internal.h"\
	".\ks\ngl_gc_profile_internal.h"\
	".\ks\ngl_xbox_internal.h"\
	".\ks\osdevopts.h"\
	".\ks\particle.h"\
	".\ks\po.h"\
	".\ks\profiler.h"\
	".\ks\projconst.h"\
	".\ks\pSrcBits.txt"\
	".\ks\random.h"\
	".\ks\refract.h"\
	".\ks\region.h"\
	".\ks\scene_anim.h"\
	".\ks\singleton.h"\
	".\ks\stringx.h"\
	".\ks\terrain.h"\
	".\ks\text_font.h"\
	".\ks\timer.h"\
	".\ks\wds.h"\
	".\s\AccompFrontEnd.h"\
	".\s\andersen_ind_anims_mac.h"\
	".\s\beach.h"\
	".\s\beachdata.h"\
	".\s\BeachFrontEnd.h"\
	".\s\blur.h"\
	".\s\board.h"\
	".\s\board_anims_mac.h"\
	".\s\boarddata.h"\
	".\s\BoardFrontEnd.h"\
	".\s\career.h"\
	".\s\careerdata.h"\
	".\s\carrol_ind_anims_mac.h"\
	".\s\challenge.h"\
	".\s\challenge_icon.h"\
	".\s\challenge_photo.h"\
	".\s\cheat.h"\
	".\s\CheatFrontEnd.h"\
	".\s\compressedphoto.h"\
	".\s\curren_ind_anims_mac.h"\
	".\s\eventmanager.h"\
	".\s\ExtrasFrontEnd.h"\
	".\s\FEAnim.h"\
	".\s\FEEntityManager.h"\
	".\s\FEMenu.h"\
	".\s\FEPanel.h"\
	".\s\fletcher_ind_anims_mac.h"\
	".\s\floatobj.h"\
	".\s\frankenreiter_ind_anims_mac.h"\
	".\s\FrontEndManager.h"\
	".\s\FrontEndMenus.h"\
	".\s\GameData.h"\
	".\s\globaltextenum.h"\
	".\s\GraphicalMenuSystem.h"\
	".\s\HighScoreFrontEnd.h"\
	".\s\igo_widget.h"\
	".\s\igo_widget_analogclock.h"\
	".\s\igo_widget_balance.h"\
	".\s\igo_widget_camera.h"\
	".\s\igo_widget_fanmeter.h"\
	".\s\igo_widget_iconradar.h"\
	".\s\igo_widget_photo.h"\
	".\s\igo_widget_replay.h"\
	".\s\igo_widget_simple.h"\
	".\s\igo_widget_specialmeter.h"\
	".\s\igo_widget_splitclock.h"\
	".\s\igo_widget_splitmeter.h"\
	".\s\igo_widget_splitscore.h"\
	".\s\igo_widget_splitter.h"\
	".\s\igo_widget_timeattack.h"\
	".\s\igo_widget_waveindicator.h"\
	".\s\IGOFrontEnd.h"\
	".\s\igohintmanager.h"\
	".\s\igoiconmanager.h"\
	".\s\igolearn_new_trickmanager.h"\
	".\s\ik_object.h"\
	".\s\irons_ind_anims_mac.h"\
	".\s\judge.h"\
	".\s\kellyslater_controller.h"\
	".\s\kellyslater_shared_anims_mac.h"\
	".\s\kellyslater_states_mac.h"\
	".\s\ks_camera.h"\
	".\s\ksdbmenu.h"\
	".\s\ksngl.h"\
	".\s\ksreplay.h"\
	".\s\machado_ind_anims_mac.h"\
	".\s\MainFrontEnd.h"\
	".\s\Map.h"\
	".\s\MCDetectFrontEnd.h"\
	".\s\menu.h"\
	".\s\menusys.h"\
	".\s\MultiFrontEnd.h"\
	".\s\MusicMan.h"\
	".\s\ode.h"\
	".\s\osGameSaver.h"\
	".\s\osparticle.h"\
	".\s\PAL60FrontEnd.h"\
	".\s\PathData.h"\
	".\s\PhotoFrontEnd.h"\
	".\s\physics.h"\
	".\s\player.h"\
	".\s\replay.h"\
	".\s\robb_ind_anims_mac.h"\
	".\s\rumbleManager.h"\
	".\s\SaveLoadFrontEnd.h"\
	".\s\scoringmanager.h"\
	".\s\slater_ind_anims_mac.h"\
	".\s\sounddata.h"\
	".\s\SoundScript.h"\
	".\s\specialmeter.h"\
	".\s\StatsFrontEnd.h"\
	".\s\surferdata.h"\
	".\s\SurferFrontEnd.h"\
	".\s\trick_system.h"\
	".\s\TrickBookFrontEnd.h"\
	".\s\trickdata.h"\
	".\s\trickregion.h"\
	".\s\tricks.h"\
	".\s\TutorialFrontEnd.h"\
	".\s\tutorialmanager.h"\
	".\s\VOEngine.h"\
	".\s\wave.h"\
	".\s\wavebreak.txt"\
	".\s\wavebreakstage.txt"\
	".\s\waveemitter.txt"\
	".\s\waveenum.h"\
	".\s\wavemarker.txt"\
	".\s\waveregion.txt"\
	".\s\wavestage.txt"\
	
# SUBTRACT BASE CPP /YX /Yc /Yu
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

DEP_CPP_FILES_W=\
	"..\..\NGL\ps2\libsn.h"\
	"..\..\ngl\ps2\matrix.h"\
	"..\..\ngl\ps2\matrix_common.h"\
	"..\..\ngl\ps2\ngl_dma.h"\
	"..\..\ngl\ps2\ngl_fixedstr.h"\
	"..\..\ngl\ps2\ngl_instbank.h"\
	"..\..\ngl\ps2\ngl_ps2.h"\
	"..\..\ngl\ps2\ngl_ps2_internal.h"\
	"..\..\NGL\ps2\ngl_vudefs.h"\
	"..\..\ngl\ps2\tim2.h"\
	"..\..\ngl\ps2\vector.h"\
	"..\..\ngl\ps2\vector_common.h"\
	".\ks\ksfx.cpp"\
	".\ks\ksfx.h"\
	".\ks\ksngl.cpp"\
	".\ks\ksngl_wave_gc.cpp"\
	".\ks\oswaverender.h"\
	".\ks\SFXEngine.h"\
	".\ks\shadow.cpp"\
	".\ks\sin.txt"\
	".\ks\spline.h"\
	".\ks\trail.cpp"\
	".\ks\trail.h"\
	".\ks\underwtr.cpp"\
	".\ks\water.cpp"\
	".\ks\water.h"\
	".\ks\watercolors.h"\
	".\ks\wave.cpp"\
	".\ks\wavedata.cpp"\
	".\ks\wavedata.h"\
	".\ks\wavedebug.txt"\
	".\ks\wavemenu.cpp"\
	".\ks\wavesound.h"\
	".\ks\wavetex.cpp"\
	".\ks\wavetex.h"\
	".\ks\wavetexdebug.txt"\
	".\ks\wavetexmenu.cpp"\
	
NODEP_CPP_FILES_W=\
	".\ekernel.h"\
	".\eregs.h"\
	".\estruct.h"\
	".\etypes.h"\
	".\ibdma.h"\
	".\ibdmapk.h"\
	".\ibgifpk.h"\
	".\ibgraph.h"\
	".\ibpc.h"\
	".\ibpkt.h"\
	".\ibvifpk.h"\
	".\ibvu0.h"\
	".\ifdev.h"\
	".\ifrpc.h"\
	".\ks\algebra.h"\
	".\ks\app.h"\
	".\ks\billboard.h"\
	".\ks\camera.h"\
	".\ks\commands.h"\
	".\ks\conglom.h"\
	".\ks\controller.h"\
	".\ks\entity.h"\
	".\ks\entity_anim.h"\
	".\ks\entity_maker.h"\
	".\ks\entityflags.h"\
	".\ks\file_finder.h"\
	".\ks\game.h"\
	".\ks\gc_ifl.h"\
	".\ks\geomgr.h"\
	".\ks\global.h"\
	".\ks\hwosgc\gc_GameSaver.h"\
	".\ks\hwosgc\gc_input.h"\
	".\ks\hwosps2\ps2_GameSaver.h"\
	".\ks\hwosps2\ps2_input.h"\
	".\ks\hwosps2\ps2_m2vplayer.h"\
	".\ks\hwosps2\ps2_waverender.h"\
	".\ks\HWOSXB\xb_algebra.h"\
	".\ks\hwosxb\xb_GameSaver.h"\
	".\ks\hwosxb\xb_input.h"\
	".\ks\hwosxb\xb_particle.h"\
	".\ks\hwosxb\xb_waverender.h"\
	".\ks\inputmgr.h"\
	".\ks\joystick.h"\
	".\ks\kshooks.h"\
	".\ks\ksnsl.h"\
	".\ks\ksnvl.h"\
	".\ks\light.h"\
	".\ks\mustash.h"\
	".\ks\ngl.h"\
	".\ks\ngl_gc_internal.h"\
	".\ks\ngl_gc_profile_internal.h"\
	".\ks\ngl_xbox_internal.h"\
	".\ks\osdevopts.h"\
	".\ks\particle.h"\
	".\ks\po.h"\
	".\ks\profiler.h"\
	".\ks\projconst.h"\
	".\ks\pSrcBits.txt"\
	".\ks\random.h"\
	".\ks\refract.h"\
	".\ks\region.h"\
	".\ks\scene_anim.h"\
	".\ks\singleton.h"\
	".\ks\stringx.h"\
	".\ks\terrain.h"\
	".\ks\text_font.h"\
	".\ks\timer.h"\
	".\ks\types.h"\
	".\ks\wds.h"\
	".\s\AccompFrontEnd.h"\
	".\s\andersen_ind_anims_mac.h"\
	".\s\beach.h"\
	".\s\beachdata.h"\
	".\s\BeachFrontEnd.h"\
	".\s\blur.h"\
	".\s\board.h"\
	".\s\board_anims_mac.h"\
	".\s\boarddata.h"\
	".\s\BoardFrontEnd.h"\
	".\s\career.h"\
	".\s\careerdata.h"\
	".\s\carrol_ind_anims_mac.h"\
	".\s\challenge.h"\
	".\s\challenge_icon.h"\
	".\s\challenge_photo.h"\
	".\s\cheat.h"\
	".\s\CheatFrontEnd.h"\
	".\s\compressedphoto.h"\
	".\s\curren_ind_anims_mac.h"\
	".\s\eventmanager.h"\
	".\s\ExtrasFrontEnd.h"\
	".\s\FEAnim.h"\
	".\s\FEEntityManager.h"\
	".\s\FEMenu.h"\
	".\s\FEPanel.h"\
	".\s\fletcher_ind_anims_mac.h"\
	".\s\floatobj.h"\
	".\s\frankenreiter_ind_anims_mac.h"\
	".\s\FrontEndManager.h"\
	".\s\FrontEndMenus.h"\
	".\s\GameData.h"\
	".\s\globaltextenum.h"\
	".\s\GraphicalMenuSystem.h"\
	".\s\HighScoreFrontEnd.h"\
	".\s\igo_widget.h"\
	".\s\igo_widget_analogclock.h"\
	".\s\igo_widget_balance.h"\
	".\s\igo_widget_camera.h"\
	".\s\igo_widget_fanmeter.h"\
	".\s\igo_widget_iconcount.h"\
	".\s\igo_widget_iconradar.h"\
	".\s\igo_widget_meterchallenge.h"\
	".\s\igo_widget_objectalert.h"\
	".\s\igo_widget_photo.h"\
	".\s\igo_widget_replay.h"\
	".\s\igo_widget_simple.h"\
	".\s\igo_widget_skillchallenge.h"\
	".\s\igo_widget_specialmeter.h"\
	".\s\igo_widget_splitclock.h"\
	".\s\igo_widget_splitmeter.h"\
	".\s\igo_widget_splitscore.h"\
	".\s\igo_widget_splitter.h"\
	".\s\igo_widget_timeattack.h"\
	".\s\igo_widget_waveindicator.h"\
	".\s\IGOFrontEnd.h"\
	".\s\igohintmanager.h"\
	".\s\igoiconmanager.h"\
	".\s\igolearn_new_trickmanager.h"\
	".\s\ik_object.h"\
	".\s\irons_ind_anims_mac.h"\
	".\s\judge.h"\
	".\s\kellyslater_controller.h"\
	".\s\kellyslater_shared_anims_mac.h"\
	".\s\kellyslater_states_mac.h"\
	".\s\ks_camera.h"\
	".\s\ksdbmenu.h"\
	".\s\ksngl.h"\
	".\s\ksreplay.h"\
	".\s\LogbookFrontEnd.h"\
	".\s\machado_ind_anims_mac.h"\
	".\s\MainFrontEnd.h"\
	".\s\Map.h"\
	".\s\MCDetectFrontEnd.h"\
	".\s\menu.h"\
	".\s\menusys.h"\
	".\s\MultiFrontEnd.h"\
	".\s\MusicMan.h"\
	".\s\ode.h"\
	".\s\osGameSaver.h"\
	".\s\osparticle.h"\
	".\s\PAL60FrontEnd.h"\
	".\s\PathData.h"\
	".\s\PhotoFrontEnd.h"\
	".\s\physics.h"\
	".\s\player.h"\
	".\s\replay.h"\
	".\s\robb_ind_anims_mac.h"\
	".\s\rumbleManager.h"\
	".\s\SaveLoadFrontEnd.h"\
	".\s\scoringmanager.h"\
	".\s\slater_ind_anims_mac.h"\
	".\s\sounddata.h"\
	".\s\SoundScript.h"\
	".\s\specialmeter.h"\
	".\s\StatsFrontEnd.h"\
	".\s\surferdata.h"\
	".\s\SurferFrontEnd.h"\
	".\s\trick_system.h"\
	".\s\TrickBookFrontEnd.h"\
	".\s\trickdata.h"\
	".\s\trickregion.h"\
	".\s\tricks.h"\
	".\s\tutorialdata.h"\
	".\s\TutorialFrontEnd.h"\
	".\s\tutorialmanager.h"\
	".\s\VOEngine.h"\
	".\s\wave.h"\
	".\s\wavebreak.txt"\
	".\s\wavebreakstage.txt"\
	".\s\waveemitter.txt"\
	".\s\waveenum.h"\
	".\s\wavemarker.txt"\
	".\s\waveregion.txt"\
	".\s\wavestage.txt"\
	
# SUBTRACT BASE CPP /YX /Yc /Yu
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

!ENDIF 

# End Source File
# End Group
# Begin Group "Build Files"

# PROP Default_Filter ".s"
# Begin Source File

SOURCE=.\app.cmd

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

# Begin Custom Build
TargetPath=\ks\bin\KellySlaterXB_Debug.xbe
InputPath=.\app.cmd

"$(TargetPath)" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	echo

# End Custom Build

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

# Begin Custom Build
TargetPath=\ks\bin\KellySlaterXB_Bootable.xbe
InputPath=.\app.cmd

"$(TargetPath)" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	echo

# End Custom Build

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# Begin Custom Build
TargetPath=\ks\bin\KellySlaterPS2_debug.elf
InputPath=.\app.cmd

"$(TargetPath)" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	echo

# End Custom Build

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# Begin Custom Build
TargetPath=\ks\bin\KellySlaterPS2_bootable.elf
InputPath=.\app.cmd

"$(TargetPath)" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	echo

# End Custom Build

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# Begin Custom Build
TargetPath=\ks\bin\KellySlaterPS2_bootable.elf
InputPath=.\app.cmd

"$(TargetPath)" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	echo

# End Custom Build

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

# Begin Custom Build
TargetPath=\ks\bin\KellySlaterXB_Bootable.xbe
InputPath=.\app.cmd

"$(TargetPath)" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	echo

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\crt0.proview.s

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\crt0.sony.s

!IF  "$(CFG)" == "KellySlater - Xbox Debug"

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Bootable"

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Bootable"

!ELSEIF  "$(CFG)" == "KellySlater - Win32 PS2 EE Final"

!ELSEIF  "$(CFG)" == "KellySlater - Xbox Final"

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
