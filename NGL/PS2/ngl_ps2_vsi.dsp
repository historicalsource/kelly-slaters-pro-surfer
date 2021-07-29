# Microsoft Developer Studio Project File - Name="ngl_ps2" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=ngl_ps2 - Win32 PS2 EE Final
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ngl_ps2_vsi.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ngl_ps2_vsi.mak" CFG="ngl_ps2 - Win32 PS2 EE Final"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ngl_ps2 - Win32 PS2 EE Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "ngl_ps2 - Win32 PS2 EE Release" (based on "Win32 (x86) Static Library")
!MESSAGE "ngl_ps2 - Win32 PS2 EE Bootable" (based on "Win32 (x86) Static Library")
!MESSAGE "ngl_ps2 - Win32 PS2 EE Final" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/NGL/working/ps2", NAAAAAAA"
# PROP Scc_LocalPath "."
CPP=snCl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ngl_ps2 - Win32 PS2 EE Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "ngl_ps2___Win32_PS2_EE_Debug"
# PROP BASE Intermediate_Dir "ngl_ps2___Win32_PS2_EE_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "PS2_EE_Debug"
# PROP Intermediate_Dir "PS2_EE_Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD CPP /nologo /w /W0 /Od /I "../common" /D "NGL_DEBUGGING" /D "NGL_DISABLE_MIPMAPS" /D "SN_TARGET_PS2" /D "PROJECT_KELLYSLATER" /debug -G0 -mvu0-use-vf0-vf31 /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=snLib.exe
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"PS2_EE_Debug\ngl_ps2.lib" /D:SN_TARGET_PS2

!ELSEIF  "$(CFG)" == "ngl_ps2 - Win32 PS2 EE Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "ngl_ps2___Win32_PS2_EE_Release"
# PROP BASE Intermediate_Dir "ngl_ps2___Win32_PS2_EE_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "PS2_EE_Release"
# PROP Intermediate_Dir "PS2_EE_Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /O3 /GZ /c
# ADD CPP /nologo /w /W0 /O2 /I "../common" /D "NGL_FINAL" /D "SN_TARGET_PS2" /D "PROJECT_KELLYSLATER" /D "NGL_DISABLE_MIPMAPS" -g -G0 -mvu0-use-vf0-vf31 /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=snLib.exe
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"PS2_EE_Release\ngl_ps2.lib" /D:SN_TARGET_PS2

!ELSEIF  "$(CFG)" == "ngl_ps2 - Win32 PS2 EE Bootable"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "ngl_ps2___Win32_PS2_EE_Bootable"
# PROP BASE Intermediate_Dir "ngl_ps2___Win32_PS2_EE_Bootable"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "PS2_EE_Bootable"
# PROP Intermediate_Dir "PS2_EE_Bootable"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /w /W0 /I "../common" /I "../tools/midnight" /D "SN_TARGET_PS2" /D "PROJECT_KELLYSLATER" /O3 -g -G0 /c
# ADD CPP /nologo /w /W0 /O2 /I "../common" /D "NGL_FINAL" /D "SN_TARGET_PS2" /D "PROJECT_KELLYSLATER" /D "NGL_DISABLE_MIPMAPS" -g -G0 -mvu0-use-vf0-vf31 /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=snLib.exe
# ADD BASE LIB32 /nologo /D:SN_TARGET_PS2
# ADD LIB32 /nologo /out:"PS2_EE_Bootable\ngl_ps2.lib" /D:SN_TARGET_PS2

!ELSEIF  "$(CFG)" == "ngl_ps2 - Win32 PS2 EE Final"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "ngl_ps2___Win32_PS2_EE_Final0"
# PROP BASE Intermediate_Dir "ngl_ps2___Win32_PS2_EE_Final0"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "PS2_EE_Final"
# PROP Intermediate_Dir "PS2_EE_Final"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /w /W0 /O2 /I "../common" /D "NGL_FINAL" /D "SN_TARGET_PS2" /D "PROJECT_KELLYSLATER" /D "NGL_DISABLE_MIPMAPS" -g -G0 -mvu0-use-vf0-vf31 /c
# ADD CPP /nologo /w /W0 /O2 /I "../common" /D "NGL_FINAL" /D "SN_TARGET_PS2" /D "PROJECT_KELLYSLATER" /D "NGL_DISABLE_MIPMAPS" -G0 -mvu0-use-vf0-vf31 /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=snLib.exe
# ADD BASE LIB32 /nologo /out:"PS2_EE_Bootable\ngl_ps2.lib" /D:SN_TARGET_PS2
# ADD LIB32 /nologo /out:"PS2_EE_Final\ngl_ps2.lib" /D:SN_TARGET_PS2

!ENDIF 

# Begin Target

# Name "ngl_ps2 - Win32 PS2 EE Debug"
# Name "ngl_ps2 - Win32 PS2 EE Release"
# Name "ngl_ps2 - Win32 PS2 EE Bootable"
# Name "ngl_ps2 - Win32 PS2 EE Final"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\ngl_ate.cpp
DEP_CPP_NGL_A=\
	".\ngl_ate.h"\
	".\ngl_fixedstr.h"\
	
# End Source File
# Begin Source File

SOURCE=.\ngl_dma.cpp
DEP_CPP_NGL_D=\
	".\ngl_dma.h"\
	".\ngl_fixedstr.h"\
	".\ngl_ps2.h"\
	".\tim2.h"\
	{$(INCLUDE)}"eekernel.h"\
	{$(INCLUDE)}"eeregs.h"\
	{$(INCLUDE)}"eestruct.h"\
	{$(INCLUDE)}"eetypes.h"\
	{$(INCLUDE)}"libdma.h"\
	{$(INCLUDE)}"libgraph.h"\
	{$(INCLUDE)}"libvu0.h"\
	
# End Source File
# Begin Source File

SOURCE=.\ngl_instbank.cpp
DEP_CPP_NGL_I=\
	".\ngl_fixedstr.h"\
	".\ngl_instbank.h"\
	
# End Source File
# Begin Source File

SOURCE=.\ngl_particle.dsm

!IF  "$(CFG)" == "ngl_ps2 - Win32 PS2 EE Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build
TargetDir=.\PS2_EE_Debug
InputPath=.\ngl_particle.dsm
InputName=ngl_particle

"$(TargetDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\tools\bin\m4 < $(InputPath) > $(InputPath)_1 
	ee-gcc -x c++ -E -DPROJECT_KELLYSLATER $(InputPath)_1 > $(InputPath)_2 
	ee-dvp-as -g -I/usr/local/sce/ee/include -o $(TargetDir)\$(InputName).obj $(InputPath)_2 
	del $(InputPath)_1 
	del $(InputPath)_2 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_ps2 - Win32 PS2 EE Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build
TargetDir=.\PS2_EE_Release
InputPath=.\ngl_particle.dsm
InputName=ngl_particle

"$(TargetDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\tools\bin\m4 < $(InputPath) > $(InputPath)_1 
	ee-gcc -x c++ -E -DPROJECT_KELLYSLATER $(InputPath)_1 > $(InputPath)_2 
	ee-dvp-as -g -I/usr/local/sce/ee/include -o $(TargetDir)\$(InputName).obj $(InputPath)_2 
	del $(InputPath)_1 
	del $(InputPath)_2 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_ps2 - Win32 PS2 EE Bootable"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build
TargetDir=.\PS2_EE_Bootable
InputPath=.\ngl_particle.dsm
InputName=ngl_particle

"$(TargetDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\tools\bin\m4 < $(InputPath) > $(InputPath)_1 
	ee-gcc -x c++ -E -DPROJECT_KELLYSLATER $(InputPath)_1 > $(InputPath)_2 
	ee-dvp-as -g -I/usr/local/sce/ee/include -o $(TargetDir)\$(InputName).obj $(InputPath)_2 
	del $(InputPath)_1 
	del $(InputPath)_2 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_ps2 - Win32 PS2 EE Final"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build
TargetDir=.\PS2_EE_Final
InputPath=.\ngl_particle.dsm
InputName=ngl_particle

"$(TargetDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\tools\bin\m4 < $(InputPath) > $(InputPath)_1 
	ee-gcc -x c++ -E -DPROJECT_KELLYSLATER $(InputPath)_1 > $(InputPath)_2 
	ee-dvp-as -g -I/usr/local/sce/ee/include -o $(TargetDir)\$(InputName).obj $(InputPath)_2 
	del $(InputPath)_1 
	del $(InputPath)_2 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ngl_ps2.cpp
DEP_CPP_NGL_P=\
	".\matrix.h"\
	".\matrix_common.h"\
	".\ngl_ate.h"\
	".\ngl_dma.h"\
	".\ngl_fixedstr.h"\
	".\ngl_instbank.h"\
	".\ngl_ps2.h"\
	".\ngl_ps2_internal.h"\
	".\ngl_radiusmesh.inc"\
	".\ngl_spheremesh.inc"\
	".\ngl_sysfont_fdf.inc"\
	".\ngl_sysfont_tm2.inc"\
	".\ngl_version.h"\
	".\ngl_vudefs.h"\
	".\tim2.h"\
	".\vector.h"\
	".\vector_common.h"\
	{$(INCLUDE)}"eekernel.h"\
	{$(INCLUDE)}"eeregs.h"\
	{$(INCLUDE)}"eestruct.h"\
	{$(INCLUDE)}"eetypes.h"\
	{$(INCLUDE)}"libdma.h"\
	{$(INCLUDE)}"libdmapk.h"\
	{$(INCLUDE)}"libgifpk.h"\
	{$(INCLUDE)}"libgraph.h"\
	{$(INCLUDE)}"libpc.h"\
	{$(INCLUDE)}"libpkt.h"\
	{$(INCLUDE)}"libsn.h"\
	{$(INCLUDE)}"libvifpk.h"\
	{$(INCLUDE)}"libvu0.h"\
	{$(INCLUDE)}"sifdev.h"\
	{$(INCLUDE)}"sifrpc.h"\
	
NODEP_CPP_NGL_P=\
	".\global.h"\
	".\profiler.h"\
	
# End Source File
# Begin Source File

SOURCE=.\ngl_vu1.dsm

!IF  "$(CFG)" == "ngl_ps2 - Win32 PS2 EE Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build
IntDir=.\PS2_EE_Debug
InputPath=.\ngl_vu1.dsm
InputName=ngl_vu1

"$(IntDir)\$(InputName).o" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ee-gcc -x c++ -E -DPROJECT_KELLYSLATER $(InputPath) | C:\usr\local\sce\ee\gcc\bin\ps2dvpas -sn -g -o $(IntDir)\$(InputName).o -I /usr/local/sce/ee/include --

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_ps2 - Win32 PS2 EE Release"

# PROP Intermediate_Dir "PS2_EE_Release"
# PROP Ignore_Default_Tool 1
# Begin Custom Build
IntDir=.\PS2_EE_Release
InputPath=.\ngl_vu1.dsm
InputName=ngl_vu1

"$(IntDir)\$(InputName).o" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ee-gcc -x c++ -E -DPROJECT_KELLYSLATER $(InputPath) | C:\usr\local\sce\ee\gcc\bin\ps2dvpas -sn -g -o $(IntDir)\$(InputName).o -I /usr/local/sce/ee/include --

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_ps2 - Win32 PS2 EE Bootable"

# PROP BASE Intermediate_Dir "PS2_EE_Release"
# PROP BASE Ignore_Default_Tool 1
# PROP Intermediate_Dir "PS2_EE_Release"
# PROP Ignore_Default_Tool 1
# Begin Custom Build
IntDir=.\PS2_EE_Release
InputPath=.\ngl_vu1.dsm
InputName=ngl_vu1

"$(IntDir)\$(InputName).o" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ee-gcc -x c++ -E -DPROJECT_KELLYSLATER $(InputPath) | C:\usr\local\sce\ee\gcc\bin\ps2dvpas -sn -g -o $(IntDir)\$(InputName).o -I /usr/local/sce/ee/include --

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_ps2 - Win32 PS2 EE Final"

# PROP BASE Intermediate_Dir "PS2_EE_Release"
# PROP BASE Ignore_Default_Tool 1
# PROP Intermediate_Dir "PS2_EE_Release"
# PROP Ignore_Default_Tool 1
# Begin Custom Build
IntDir=.\PS2_EE_Release
InputPath=.\ngl_vu1.dsm
InputName=ngl_vu1

"$(IntDir)\$(InputName).o" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ee-gcc -x c++ -E -DPROJECT_KELLYSLATER $(InputPath) | C:\usr\local\sce\ee\gcc\bin\ps2dvpas -sn -g -o $(IntDir)\$(InputName).o -I /usr/local/sce/ee/include --

# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\libsn.h
# End Source File
# Begin Source File

SOURCE=.\matrix.h
# End Source File
# Begin Source File

SOURCE=.\matrix_common.h
# End Source File
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

SOURCE=.\PS2_in_VC.h
# End Source File
# Begin Source File

SOURCE=.\tim2.h
# End Source File
# Begin Source File

SOURCE=.\vector.h
# End Source File
# Begin Source File

SOURCE=.\vector_common.h
# End Source File
# End Group
# Begin Group "Inc Files"

# PROP Default_Filter ".inc"
# Begin Source File

SOURCE=.\ngl_font8x12.inc
# End Source File
# Begin Source File

SOURCE=.\ngl_radiusmesh.inc
# End Source File
# Begin Source File

SOURCE=.\ngl_spheremesh.inc
# End Source File
# End Group
# End Target
# End Project
