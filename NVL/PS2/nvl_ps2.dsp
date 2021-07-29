# Microsoft Developer Studio Project File - Name="nvl_ps2" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=nvl_ps2 - Win32 PS2 EE Final
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "nvl_ps2.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "nvl_ps2.mak" CFG="nvl_ps2 - Win32 PS2 EE Final"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "nvl_ps2 - Win32 PS2 EE Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "nvl_ps2 - Win32 PS2 EE Release" (based on "Win32 (x86) Static Library")
!MESSAGE "nvl_ps2 - Win32 PS2 EE Bootable" (based on "Win32 (x86) Static Library")
!MESSAGE "nvl_ps2 - Win32 PS2 EE Final" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/NVL/ps2", VEXBAAAA"
# PROP Scc_LocalPath "."
CPP=snCl.exe
RSC=rc.exe

!IF  "$(CFG)" == "nvl_ps2 - Win32 PS2 EE Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "nvl_ps2___Win32_PS2_EE_Debug"
# PROP BASE Intermediate_Dir "nvl_ps2___Win32_PS2_EE_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "nvl_ps2___Win32_PS2_EE_Debug"
# PROP Intermediate_Dir "nvl_ps2___Win32_PS2_EE_Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD CPP /nologo /w /W0 /Od /D "SN_TARGET_PS2" /D "PROJECT_KELLYSLATER" /D "BUILD_DEBUG" /D "DEBUG" /Fo"PS2_EE_Debug/" /debug /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=snLib.exe
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"PS2_EE_Debug\nvl_ps2.lib" /D:SN_TARGET_PS2

!ELSEIF  "$(CFG)" == "nvl_ps2 - Win32 PS2 EE Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "nvl_ps2___Win32_PS2_EE_Release"
# PROP BASE Intermediate_Dir "nvl_ps2___Win32_PS2_EE_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "nvl_ps2___Win32_PS2_EE_Release"
# PROP Intermediate_Dir "nvl_ps2___Win32_PS2_EE_Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD CPP /nologo /w /W0 /O2 /D "SN_TARGET_PS2" /D "PROJECT_KELLYSLATER" /Fo"PS2_EE_Release/" /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=snLib.exe
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"PS2_EE_Release\nvl_ps2.lib" /D:SN_TARGET_PS2

!ELSEIF  "$(CFG)" == "nvl_ps2 - Win32 PS2 EE Bootable"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "nvl_ps2___Win32_PS2_EE_Bootable"
# PROP BASE Intermediate_Dir "nvl_ps2___Win32_PS2_EE_Bootable"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "PS2_EE_Bootable"
# PROP Intermediate_Dir "PS2_EE_Bootable"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /w /W0 /O2 /D "SN_TARGET_PS2" /Fo"PS2_EE_Bootable/" /c
# ADD CPP /nologo /w /W0 /O2 /D "BUILD_BOOTABLE" /D "SN_TARGET_PS2" /D "PROJECT_KELLYSLATER" -g -G0 /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=snLib.exe
# ADD BASE LIB32 /nologo /out:"PS2_EE_Release\nvl_ps2.lib" /D:SN_TARGET_PS2
# ADD LIB32 /nologo /D:SN_TARGET_PS2

!ELSEIF  "$(CFG)" == "nvl_ps2 - Win32 PS2 EE Final"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "nvl_ps2___Win32_PS2_EE_Final"
# PROP BASE Intermediate_Dir "nvl_ps2___Win32_PS2_EE_Final"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "PS2_EE_Final"
# PROP Intermediate_Dir "PS2_EE_Final"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /w /W0 /O2 /D "BUILD_BOOTABLE" /D "SN_TARGET_PS2" /D "PROJECT_KELLYSLATER" -g -G0 /c
# ADD CPP /nologo /w /W0 /O2 /D "BUILD_BOOTABLE" /D "SN_TARGET_PS2" /D "PROJECT_KELLYSLATER" -G0 /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=snLib.exe
# ADD BASE LIB32 /nologo /D:SN_TARGET_PS2
# ADD LIB32 /nologo /D:SN_TARGET_PS2

!ENDIF 

# Begin Target

# Name "nvl_ps2 - Win32 PS2 EE Debug"
# Name "nvl_ps2 - Win32 PS2 EE Release"
# Name "nvl_ps2 - Win32 PS2 EE Bootable"
# Name "nvl_ps2 - Win32 PS2 EE Final"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\nvl_ps2.cpp
DEP_CPP_NVL_P=\
	".\nvl_ps2.h"\
	".\nvlstream_ps2.h"\
	{$(INCLUDE)}"eekernel.h"\
	{$(INCLUDE)}"eeregs.h"\
	{$(INCLUDE)}"eetypes.h"\
	{$(INCLUDE)}"libcdvd.h"\
	{$(INCLUDE)}"libdma.h"\
	{$(INCLUDE)}"libipu.h"\
	{$(INCLUDE)}"sifdev.h"\
	
# End Source File
# Begin Source File

SOURCE=.\nvlMPEG_ps2.cpp
DEP_CPP_NVLMP=\
	".\nvlMPEG_ps2.h"\
	{$(INCLUDE)}"eekernel.h"\
	{$(INCLUDE)}"eeregs.h"\
	{$(INCLUDE)}"eestruct.h"\
	{$(INCLUDE)}"eetypes.h"\
	{$(INCLUDE)}"libcdvd.h"\
	{$(INCLUDE)}"libdma.h"\
	{$(INCLUDE)}"libdmapk.h"\
	{$(INCLUDE)}"libgifpk.h"\
	{$(INCLUDE)}"libgraph.h"\
	{$(INCLUDE)}"libipu.h"\
	{$(INCLUDE)}"libmpeg.h"\
	{$(INCLUDE)}"libpkt.h"\
	{$(INCLUDE)}"libsdr.h"\
	{$(INCLUDE)}"libvifpk.h"\
	{$(INCLUDE)}"sdmacro.h"\
	{$(INCLUDE)}"sdrcmd.h"\
	{$(INCLUDE)}"sif.h"\
	{$(INCLUDE)}"sifdev.h"\
	{$(INCLUDE)}"sifrpc.h"\
	
# End Source File
# Begin Source File

SOURCE=.\nvlstream_ps2.cpp
DEP_CPP_NVLST=\
	".\nvlstream_ps2.h"\
	{$(INCLUDE)}"eekernel.h"\
	{$(INCLUDE)}"eetypes.h"\
	{$(INCLUDE)}"libcdvd.h"\
	{$(INCLUDE)}"sifdev.h"\
	{$(INCLUDE)}"sifrpc.h"\
	
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\nvl_ps2.h
# End Source File
# Begin Source File

SOURCE=.\nvlMPEG_ps2.h
# End Source File
# Begin Source File

SOURCE=.\nvlstream_ps2.h
# End Source File
# Begin Source File

SOURCE=.\PS2_in_VC.h
# End Source File
# End Group
# End Target
# End Project
