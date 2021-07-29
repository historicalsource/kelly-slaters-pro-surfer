# Microsoft Developer Studio Project File - Name="ngl_ps2" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) External Target" 0x0106

CFG=ngl_ps2 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ngl_ps2.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ngl_ps2.mak" CFG="ngl_ps2 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ngl_ps2 - Win32 Release" (based on "Win32 (x86) External Target")
!MESSAGE "ngl_ps2 - Win32 Debug" (based on "Win32 (x86) External Target")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/NGL/working/ps2", NAAAAAAA"
# PROP Scc_LocalPath "."

!IF  "$(CFG)" == "ngl_ps2 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Cmd_Line "NMAKE /f ngl_ps2.mak"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "ngl_ps2.exe"
# PROP BASE Bsc_Name "ngl_ps2.bsc"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Cmd_Line "makeps2.bat release"
# PROP Rebuild_Opt ""
# PROP Target_File "ngl_ps2.a"
# PROP Bsc_Name ""
# PROP Target_Dir ""

!ELSEIF  "$(CFG)" == "ngl_ps2 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Cmd_Line "NMAKE /f ngl_ps2.mak"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "ngl_ps2.exe"
# PROP BASE Bsc_Name "ngl_ps2.bsc"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Cmd_Line "\ngl\working\ps2\makeps2.bat debug"
# PROP Rebuild_Opt ""
# PROP Target_File "ngl_ps2.a"
# PROP Bsc_Name ""
# PROP Target_Dir ""

!ENDIF 

# Begin Target

# Name "ngl_ps2 - Win32 Release"
# Name "ngl_ps2 - Win32 Debug"

!IF  "$(CFG)" == "ngl_ps2 - Win32 Release"

!ELSEIF  "$(CFG)" == "ngl_ps2 - Win32 Debug"

!ENDIF 

# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\ngl_ate.cpp
# End Source File
# Begin Source File

SOURCE=.\ngl_dma.cpp
# End Source File
# Begin Source File

SOURCE=.\ngl_font8x12.inc
# End Source File
# Begin Source File

SOURCE=.\ngl_instbank.cpp
# End Source File
# Begin Source File

SOURCE=.\ngl_particle.dsm
# End Source File
# Begin Source File

SOURCE=.\ngl_ps2.cpp
# End Source File
# Begin Source File

SOURCE=.\ngl_ps2_asm.s
# End Source File
# Begin Source File

SOURCE=.\ngl_radiusmesh.inc
# End Source File
# Begin Source File

SOURCE=.\ngl_spheremesh.inc
# End Source File
# Begin Source File

SOURCE=.\ngl_vu1.dsm
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\ngl_ate.h
# End Source File
# Begin Source File

SOURCE=.\ngl_dma.h
# End Source File
# Begin Source File

SOURCE=.\ngl_fixedstr.h
# End Source File
# Begin Source File

SOURCE=.\ngl_instbank.h
# End Source File
# Begin Source File

SOURCE=.\ngl_ps2.h
# End Source File
# Begin Source File

SOURCE=.\ngl_ps2_internal.h
# End Source File
# Begin Source File

SOURCE=.\ngl_vudefs.h
# End Source File
# Begin Source File

SOURCE=.\tim2.h
# End Source File
# End Group
# Begin Group "Makefiles"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\MakePS2.bat
# End Source File
# Begin Source File

SOURCE=.\NGLPS2.mak
# End Source File
# End Group
# End Target
# End Project
