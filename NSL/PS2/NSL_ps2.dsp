# Microsoft Developer Studio Project File - Name="nsl_ps2" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=nsl_ps2 - Win32 PS2 EE Final
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "NSL_ps2.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "NSL_ps2.mak" CFG="nsl_ps2 - Win32 PS2 EE Final"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "nsl_ps2 - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "nsl_ps2 - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "nsl_ps2 - Win32 PS2 EE Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "nsl_ps2 - Win32 PS2 EE Release" (based on "Win32 (x86) Static Library")
!MESSAGE "nsl_ps2 - Win32 PS2 EE Bootable" (based on "Win32 (x86) Static Library")
!MESSAGE "nsl_ps2 - Win32 PS2 EE Final" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/NSL/PS2", HVOEAAAA"
# PROP Scc_LocalPath "."
CPP=snCl.exe
RSC=rc.exe

!IF  "$(CFG)" == "nsl_ps2 - Win32 Release"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\\" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=snLib.exe
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "nsl_ps2 - Win32 Debug"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\\" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=snLib.exe
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "nsl_ps2 - Win32 PS2 EE Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "nsl_ps2___Win32_PS2_EE_Debug"
# PROP BASE Intermediate_Dir "nsl_ps2___Win32_PS2_EE_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "nsl_ps2___Win32_PS2_EE_Debug"
# PROP Intermediate_Dir "nsl_ps2___Win32_PS2_EE_Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD CPP /nologo /w /W0 /Od /I "C:\usr\local\sce\ee\include" /I "C:\usr\local\sce\common\include" /I "..\\" /D "SN_TARGET_PS2" /Fo"PS2_EE_Debug/" /FD /debug -G0 -g /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /i "..\\"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=snLib.exe
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"PS2_EE_Debug\nsl.lib" /D:SN_TARGET_PS2

!ELSEIF  "$(CFG)" == "nsl_ps2 - Win32 PS2 EE Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "nsl_ps2___Win32_PS2_EE_Release"
# PROP BASE Intermediate_Dir "nsl_ps2___Win32_PS2_EE_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "nsl_ps2___Win32_PS2_EE_Release"
# PROP Intermediate_Dir "nsl_ps2___Win32_PS2_EE_Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD CPP /nologo /w /W0 /O2 /I "C:\usr\local\sce\ee\include" /I "C:\usr\local\sce\common\include" /I "..\\" /D "SN_TARGET_PS2" /Fo"PS2_EE_Release/" /FD -G0 -g /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /i "..\\"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=snLib.exe
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"PS2_EE_Release\ps2.lib" /D:SN_TARGET_PS2

!ELSEIF  "$(CFG)" == "nsl_ps2 - Win32 PS2 EE Bootable"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "nsl_ps2___Win32_PS2_EE_Bootable"
# PROP BASE Intermediate_Dir "nsl_ps2___Win32_PS2_EE_Bootable"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "nsl_ps2___Win32_PS2_EE_Bootable"
# PROP Intermediate_Dir "nsl_ps2___Win32_PS2_EE_Bootable"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /w /W0 /Od /I "C:\usr\local\sce\ee\include" /I "C:\usr\local\sce\common\include" /I "..\\" /D "SN_TARGET_PS2" /Fo"PS2_EE_Debug/" /FD /debug /c
# ADD CPP /nologo /w /W0 /O2 /I "C:\usr\local\sce\ee\include" /I "C:\usr\local\sce\common\include" /I "..\\" /D "SN_TARGET_PS2" /Fo"PS2_EE_Debug/" /FD -G0 -g /c
# ADD BASE RSC /l 0x409 /i "..\\"
# ADD RSC /l 0x409 /i "..\\"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=snLib.exe
# ADD BASE LIB32 /nologo /out:"PS2_EE_Debug\nsl.lib" /D:SN_TARGET_PS2
# ADD LIB32 /nologo /out:"PS2_EE_Bootable\nsl.lib" /D:SN_TARGET_PS2

!ELSEIF  "$(CFG)" == "nsl_ps2 - Win32 PS2 EE Final"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "nsl_ps2___Win32_PS2_EE_Final"
# PROP BASE Intermediate_Dir "nsl_ps2___Win32_PS2_EE_Final"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "PS2_EE_Final"
# PROP Intermediate_Dir "PS2_EE_Final"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /w /W0 /Od /I "C:\usr\local\sce\ee\include" /I "C:\usr\local\sce\common\include" /I "..\\" /D "SN_TARGET_PS2" /Fo"PS2_EE_Debug/" /FD /debug /c
# ADD CPP /nologo /w /W0 /O2 /I "C:\usr\local\sce\ee\include" /I "C:\usr\local\sce\common\include" /I "..\\" /D "SN_TARGET_PS2" /Fo"PS2_EE_Debug/" /FD -G0 /c
# ADD BASE RSC /l 0x409 /i "..\\"
# ADD RSC /l 0x409 /i "..\\"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=snLib.exe
# ADD BASE LIB32 /nologo /out:"PS2_EE_Bootable\nsl.lib" /D:SN_TARGET_PS2
# ADD LIB32 /nologo /out:"PS2_EE_Final\nsl.lib" /D:SN_TARGET_PS2

!ENDIF 

# Begin Target

# Name "nsl_ps2 - Win32 Release"
# Name "nsl_ps2 - Win32 Debug"
# Name "nsl_ps2 - Win32 PS2 EE Debug"
# Name "nsl_ps2 - Win32 PS2 EE Release"
# Name "nsl_ps2 - Win32 PS2 EE Bootable"
# Name "nsl_ps2 - Win32 PS2 EE Final"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\gas.h
# End Source File
# Begin Source File

SOURCE=.\gas_utility.cpp
# End Source File
# Begin Source File

SOURCE=.\gas_utility.h
# End Source File
# Begin Source File

SOURCE=.\nl_ps2.cpp
# End Source File
# Begin Source File

SOURCE=.\nl_ps2.h
# End Source File
# Begin Source File

SOURCE=.\nsl_ps2.cpp
# End Source File
# Begin Source File

SOURCE=.\nsl_ps2.h
# End Source File
# Begin Source File

SOURCE=.\nsl_ps2_emitter.cpp
# End Source File
# Begin Source File

SOURCE=.\nsl_ps2_sound.cpp
# End Source File
# Begin Source File

SOURCE=.\nsl_ps2_source.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\common\nl.h
# End Source File
# Begin Source File

SOURCE=..\common\nsl.cpp
# End Source File
# Begin Source File

SOURCE=..\common\nsl.h
# End Source File
# Begin Source File

SOURCE=..\common\nsl_common.cpp
# End Source File
# Begin Source File

SOURCE=..\common\ProjectOptions.h
# End Source File
# Begin Source File

SOURCE=.\PS2_in_VC.h
# End Source File
# End Group
# End Target
# End Project
