# Microsoft Developer Studio Project File - Name="nsl_xbox" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** DO NOT EDIT **

# TARGTYPE "Xbox Static Library" 0x0b04

CFG=nsl_xbox - Xbox Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "nsl_xbox.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "nsl_xbox.mak" CFG="nsl_xbox - Xbox Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "nsl_xbox - Xbox Release" (based on "Xbox Static Library")
!MESSAGE "nsl_xbox - Xbox Debug" (based on "Xbox Static Library")
!MESSAGE "nsl_xbox - Xbox Bootable" (based on "Xbox Static Library")
!MESSAGE "nsl_xbox - Xbox Final" (based on "Xbox Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/NSL/xbox", HBBAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe

!IF  "$(CFG)" == "nsl_xbox - Xbox Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Xbox_Release"
# PROP Intermediate_Dir "Xbox_Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "_XBOX" /D "NDEBUG" /YX /FD /G6 /Zvc6 /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "_XBOX" /D "NDEBUG" /D "TARGET_XBOX" /D "NSL_LOAD_SOURCE_BY_ALIAS" /YX /FD /G6 /Zvc6 /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=snLib.exe
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "nsl_xbox - Xbox Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Xbox_Debug"
# PROP Intermediate_Dir "Xbox_Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_XBOX" /D "_DEBUG" /YX /FD /G6 /Zvc6 /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /I "..\common" /D "WIN32" /D "_XBOX" /D "_DEBUG" /D "TARGET_XBOX" /D "NSL_LOAD_SOURCE_BY_ALIAS" /FR /YX /FD /G6 /Ztmp /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=snLib.exe
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "nsl_xbox - Xbox Bootable"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "nsl_xbox___Xbox_Bootable"
# PROP BASE Intermediate_Dir "nsl_xbox___Xbox_Bootable"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Xbox_Bootable"
# PROP Intermediate_Dir "Xbox_Bootable"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "_XBOX" /D "NDEBUG" /D "TARGET_XBOX" /YX /FD /G6 /Zvc6 /c
# ADD CPP /nologo /W3 /GX /Zi /O2 /D "WIN32" /D "_XBOX" /D "NDEBUG" /D "TARGET_XBOX" /D "BUILD_BOOTABLE" /D "NSL_LOAD_SOURCE_BY_ALIAS" /YX /FD /G6 /Ztmp /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=snLib.exe
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "nsl_xbox - Xbox Final"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "nsl_xbox___Xbox_Final"
# PROP BASE Intermediate_Dir "nsl_xbox___Xbox_Final"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Xbox_Final"
# PROP Intermediate_Dir "Xbox_Final"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /Zi /O2 /D "WIN32" /D "_XBOX" /D "NDEBUG" /D "TARGET_XBOX" /D "BUILD_BOOTABLE" /D "NSL_LOAD_SOURCE_BY_ALIAS" /YX /FD /G6 /Ztmp /c
# ADD CPP /nologo /W3 /GX /Zi /O2 /D "WIN32" /D "_XBOX" /D "NDEBUG" /D "TARGET_XBOX" /D "BUILD_BOOTABLE" /D "NSL_LOAD_SOURCE_BY_ALIAS" /YX /FD /G6 /Ztmp /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=snLib.exe
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "nsl_xbox - Xbox Release"
# Name "nsl_xbox - Xbox Debug"
# Name "nsl_xbox - Xbox Bootable"
# Name "nsl_xbox - Xbox Final"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\common\nsl.cpp
# End Source File
# Begin Source File

SOURCE=..\common\nsl_common.cpp
# End Source File
# Begin Source File

SOURCE=.\nsl_xbox.cpp
# End Source File
# Begin Source File

SOURCE=.\nsl_xbox_emitter.cpp
# End Source File
# Begin Source File

SOURCE=.\nsl_xbox_ext.cpp
# End Source File
# Begin Source File

SOURCE=.\nsl_xbox_sound.cpp
# End Source File
# Begin Source File

SOURCE=.\nsl_xbox_source.cpp
# End Source File
# Begin Source File

SOURCE=.\nsl_xbox_streams.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\common\nl.h
# End Source File
# Begin Source File

SOURCE=.\nl_xbox.h
# End Source File
# Begin Source File

SOURCE=..\common\nsl.h
# End Source File
# Begin Source File

SOURCE=.\nsl_xbox.h
# End Source File
# Begin Source File

SOURCE=.\nsl_xbox_ext.h
# End Source File
# Begin Source File

SOURCE=.\nsl_xbox_source.h
# End Source File
# End Group
# End Target
# End Project
