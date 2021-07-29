# Microsoft Developer Studio Project File - Name="NSLSoundTool" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=NSLSoundTool - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "NSLSoundTool.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "NSLSoundTool.mak" CFG="NSLSoundTool - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "NSLSoundTool - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "NSLSoundTool - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/NSL/working/tools/NSLSoundTool", NBBAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "NSLSoundTool - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "libsndfile-0.0.22\src\\" /I "..\..\\" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "ADPCM_TOOL" /D "NSL_SOUND_TOOL" /D "NSL_LOAD_SOURCE_BY_NAME" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 encvag.lib libsndfile-0.0.22\Win32\libsndfile\Release\libsndfile.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "NSLSoundTool - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "libsndfile-0.0.22\src\\" /I "..\..\\" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "ADPCM_TOOL" /D "NSL_SOUND_TOOL" /D "NSL_LOAD_SOURCE_BY_NAME" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 encvag.lib libsndfile-0.0.22\Win32\libsndfile\Debug\libsndfile.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "NSLSoundTool - Win32 Release"
# Name "NSLSoundTool - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\GCSoundTool.cpp
# End Source File
# Begin Source File

SOURCE=.\imaadpcm.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\nsl.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\nsl_common.cpp
# End Source File
# Begin Source File

SOURCE=.\NSLSoundTool.cpp
# End Source File
# Begin Source File

SOURCE=.\nslSoundToolUtility.cpp
# End Source File
# Begin Source File

SOURCE=.\PS2SoundTool.cpp
# End Source File
# Begin Source File

SOURCE=.\snd_parser.cpp
# End Source File
# Begin Source File

SOURCE=.\wavparse.cpp
# End Source File
# Begin Source File

SOURCE=.\XBoxSoundTool.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\encvag.h
# End Source File
# Begin Source File

SOURCE=..\HWOSPS2\gas.h
# End Source File
# Begin Source File

SOURCE=..\HWOSPS2\Gas\GasSystem.h
# End Source File
# Begin Source File

SOURCE=.\GCSoundTool.h
# End Source File
# Begin Source File

SOURCE=.\imaadpcm.h
# End Source File
# Begin Source File

SOURCE=.\NSLSoundTool.h
# End Source File
# Begin Source File

SOURCE=.\PS2SoundTool.h
# End Source File
# Begin Source File

SOURCE=.\snd_parser.h
# End Source File
# Begin Source File

SOURCE=.\wavparse.h
# End Source File
# Begin Source File

SOURCE=.\XBoxSoundTool.h
# End Source File
# End Group
# Begin Group "Misc"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\layout.txt
# End Source File
# End Group
# End Target
# End Project
