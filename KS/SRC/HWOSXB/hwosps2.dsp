# Microsoft Developer Studio Project File - Name="hwosps2" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=hwosps2 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "hwosps2.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "hwosps2.mak" CFG="hwosps2 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "hwosps2 - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "hwosps2 - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "hwosps2 - Win32 Fast Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "hwosps2 - Win32 Bootable" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "hwosps2 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /Zp4 /W3 /Zd /Ox /Ot /Oa /Og /Oi /Ob2 /I "..\vsim" /I "..\\" /I "..\..\dxinclude" /I "..\..\sgi_stl" /I "..\lpng102" /I "..\lpng102\zlib" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "BUILD_RELEASE" /YX"global.h" /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "hwosps2 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /G6 /Zp4 /W3 /Gm /ZI /Od /I "..\vsim" /I "..\\" /I "..\..\dxinclude" /I "..\..\sgi_stl" /I "..\lpng102" /I "..\lpng102\zlib" /D "_DEBUG" /D "BUILD_DEBUG" /D "TARGET_ps2" /Fr /YX"global.h" /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "hwosps2 - Win32 Fast Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Fast Debug"
# PROP BASE Intermediate_Dir "Fast Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "FastDebug"
# PROP Intermediate_Dir "FastDebug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /G6 /Zp4 /MLd /W3 /Gm /Zi /O2 /I "..\vsim" /I "..\\" /I "..\..\dxinclude" /I "..\..\sgi_stl" /I "..\lpng102" /I "..\lpng102\zlib" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "BUILD_FASTDEBUG" /Fr /YX"global.h" /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "hwosps2 - Win32 Bootable"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Bootable"
# PROP BASE Intermediate_Dir "Bootable"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Bootable"
# PROP Intermediate_Dir "Bootable"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /G6 /Zp4 /W3 /Ox /Ot /Oa /Og /Oi /Ob2 /I "..\vsim" /I "..\\" /I "..\..\dxinclude" /I "..\..\sgi_stl" /I "..\lpng102" /I "..\lpng102\zlib" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "BUILD_BOOTABLE" /YX"global.h" /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "hwosps2 - Win32 Release"
# Name "hwosps2 - Win32 Debug"
# Name "hwosps2 - Win32 Fast Debug"
# Name "hwosps2 - Win32 Bootable"
# Begin Group "File Module"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ps2_file.cpp
# End Source File
# Begin Source File

SOURCE=.\ps2_file.h
# End Source File
# Begin Source File

SOURCE=.\ps2_storage.cpp
# End Source File
# Begin Source File

SOURCE=.\ps2_storage.h
# End Source File
# End Group
# Begin Group "Input Module"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ps2_input.cpp
# End Source File
# Begin Source File

SOURCE=.\ps2_input.h
# End Source File
# End Group
# Begin Group "General Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ps2_alloc.cpp
# End Source File
# Begin Source File

SOURCE=.\ps2_alloc.h
# End Source File
# Begin Source File

SOURCE=.\ps2_devopts.cpp
# End Source File
# Begin Source File

SOURCE=.\ps2_devopts.h
# End Source File
# Begin Source File

SOURCE=.\ps2_errmsg.cpp
# End Source File
# Begin Source File

SOURCE=.\ps2_errmsg.h
# End Source File
# Begin Source File

SOURCE=.\ps2_math.h
# End Source File
# Begin Source File

SOURCE=.\ps2_timer.cpp
# End Source File
# Begin Source File

SOURCE=.\ps2_timer.h
# End Source File
# Begin Source File

SOURCE=.\ps2_xform.h
# End Source File
# End Group
# Begin Group "Graphics Module"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\krender.cpp
# End Source File
# Begin Source File

SOURCE=.\krender.h
# End Source File
# Begin Source File

SOURCE=.\krvu1.dsm
# End Source File
# Begin Source File

SOURCE=.\ps2_graphics.cpp
# End Source File
# Begin Source File

SOURCE=.\ps2_graphics.h
# End Source File
# Begin Source File

SOURCE=.\ps2_rasterize.cpp
# End Source File
# Begin Source File

SOURCE=.\ps2_rasterize.h
# End Source File
# Begin Source File

SOURCE=.\ps2_texturemgr.cpp
# End Source File
# Begin Source File

SOURCE=.\ps2_texturemgr.h
# End Source File
# End Group
# Begin Group "Sound Module"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ps2_audio.cpp
# End Source File
# Begin Source File

SOURCE=.\ps2_audio.h
# End Source File
# End Group
# Begin Group "Movie Player"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ps2_movie_audiodec.c
# End Source File
# Begin Source File

SOURCE=.\ps2_movie_audiodec.h
# End Source File
# Begin Source File

SOURCE=.\ps2_movie_defs.h
# End Source File
# Begin Source File

SOURCE=.\ps2_movie_disp.c
# End Source File
# Begin Source File

SOURCE=.\ps2_movie_disp.h
# End Source File
# Begin Source File

SOURCE=.\ps2_movie_read.c
# End Source File
# Begin Source File

SOURCE=.\ps2_movie_readbuf.c
# End Source File
# Begin Source File

SOURCE=.\ps2_movie_readbuf.h
# End Source File
# Begin Source File

SOURCE=.\ps2_movie_strfile.c
# End Source File
# Begin Source File

SOURCE=.\ps2_movie_strfile.h
# End Source File
# Begin Source File

SOURCE=.\ps2_movie_util.c
# End Source File
# Begin Source File

SOURCE=.\ps2_movie_vibuf.c
# End Source File
# Begin Source File

SOURCE=.\ps2_movie_vibuf.h
# End Source File
# Begin Source File

SOURCE=.\ps2_movie_videodec.c
# End Source File
# Begin Source File

SOURCE=.\ps2_movie_videodec.h
# End Source File
# Begin Source File

SOURCE=.\ps2_movie_vobuf.c
# End Source File
# Begin Source File

SOURCE=.\ps2_movie_vobuf.h
# End Source File
# Begin Source File

SOURCE=.\ps2_movieplayer.cpp
# End Source File
# Begin Source File

SOURCE=.\ps2_movieplayer.h
# End Source File
# End Group
# End Target
# End Project
