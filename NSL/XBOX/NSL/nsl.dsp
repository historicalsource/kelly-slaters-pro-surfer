# Microsoft Developer Studio Project File - Name="nsl" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** DO NOT EDIT **

# TARGTYPE "Xbox Static Library" 0x0b04

CFG=nsl - Xbox Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "nsl.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "nsl.mak" CFG="nsl - Xbox Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "nsl - Xbox Release" (based on "Xbox Static Library")
!MESSAGE "nsl - Xbox Debug" (based on "Xbox Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe

!IF  "$(CFG)" == "nsl - Xbox Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "_XBOX" /D "NDEBUG" /YX /FD /G6 /Zvc6 /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "_XBOX" /D "NDEBUG" /YX /FD /G6 /Zvc6 /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "nsl - Xbox Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_XBOX" /D "_DEBUG" /YX /FD /G6 /Zvc6 /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /I "-I." /D "WIN32" /D "_XBOX" /D "_DEBUG" /YX /FD /G6 /Zvc6 /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "nsl - Xbox Release"
# Name "nsl - Xbox Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\nsl_xbox.cpp

!IF  "$(CFG)" == "nsl - Xbox Release"

!ELSEIF  "$(CFG)" == "nsl - Xbox Debug"

# ADD CPP /I "."
# SUBTRACT CPP /I "-I."

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\global.h
# End Source File
# Begin Source File

SOURCE=..\nl_xbox.h
# End Source File
# Begin Source File

SOURCE=..\nsl_flags_mac.h
# End Source File
# Begin Source File

SOURCE=..\nsl_params_mac.h
# End Source File
# Begin Source File

SOURCE=..\nsl_xbox.h
# End Source File
# End Group
# Begin Group "deps"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\common\nl.h
# End Source File
# Begin Source File

SOURCE=..\..\common\nsl.h
# End Source File
# Begin Source File

SOURCE=..\..\common\nsl_flags_mac.h
# End Source File
# Begin Source File

SOURCE=..\..\common\nsl_params_mac.h
# End Source File
# Begin Source File

SOURCE=..\..\common\pstring.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\pstring.h
# End Source File
# End Group
# End Target
# End Project
