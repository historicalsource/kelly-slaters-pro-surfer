# Microsoft Developer Studio Project File - Name="ngl_xbox" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** DO NOT EDIT **

# TARGTYPE "Xbox Static Library" 0x0b04

CFG=ngl_xbox - Xbox Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ngl_xbox.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ngl_xbox.mak" CFG="ngl_xbox - Xbox Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ngl_xbox - Xbox Release" (based on "Xbox Static Library")
!MESSAGE "ngl_xbox - Xbox Debug" (based on "Xbox Static Library")
!MESSAGE "ngl_xbox - Xbox Final" (based on "Xbox Static Library")
!MESSAGE "ngl_xbox - Xbox Bootable" (based on "Xbox Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/NGL/XBOX", KCCBAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe

!IF  "$(CFG)" == "ngl_xbox - Xbox Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "XBox_Release"
# PROP Intermediate_Dir "XBox_Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "_XBOX" /D "NDEBUG" /YX /FD /G6 /Zvc6 /c
# ADD CPP /nologo /Zp4 /ML /W4 /GX /Zi /O2 /D "WIN32" /D "_XBOX" /D "NDEBUG" /D "PROJECT_KELLYSLATER" /FR /YX /FD /G6 /Ztmp /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=snLib.exe
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "XBox_Debug"
# PROP Intermediate_Dir "XBox_Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_XBOX" /D "_DEBUG" /YX /FD /G6 /Zvc6 /c
# ADD CPP /nologo /Zp4 /W4 /Gm /GX /Zi /Od /Ob1 /D "WIN32" /D "_XBOX" /D "_DEBUG" /D "PROJECT_KELLYSLATER" /FR /YX /FD /G6 /Ztmp /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"Debug/ngl_xboxd.bsc"
LIB32=snLib.exe
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Final"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "xbox___Xbox_Final"
# PROP BASE Intermediate_Dir "xbox___Xbox_Final"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "XBox_Final"
# PROP Intermediate_Dir "XBox_Final"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /Zp4 /ML /W4 /GX /O2 /D "WIN32" /D "_XBOX" /D "NDEBUG" /D "NGL_BOOTABLE" /FR /YX /FD /G6 /Zvc6 /c
# ADD CPP /nologo /Zp4 /ML /W4 /GX /O2 /D "WIN32" /D "_XBOX" /D "NDEBUG" /D "PROJECT_KELLYSLATER" /D "NGL_FINAL" /FR /YX /FD /G6 /Ztmp /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo /o"Bootable/ngl_xboxb.bsc"
# ADD BSC32 /nologo /o"Bootable/ngl_xboxf.bsc"
LIB32=snLib.exe
# ADD BASE LIB32 /nologo /out:"Bootable\ngl_xboxb.lib"
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Bootable"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "xbox___Xbox_Bootable"
# PROP BASE Intermediate_Dir "xbox___Xbox_Bootable"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "XBox_Bootable"
# PROP Intermediate_Dir "XBox_Bootable"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /Zp4 /ML /W4 /GX /O2 /D "WIN32" /D "_XBOX" /D "NDEBUG" /D "NGL_FINAL" /FR /YX /FD /G6 /Zvc6 /c
# ADD CPP /nologo /Zp4 /ML /W4 /GX /Zi /O2 /D "WIN32" /D "_XBOX" /D "NDEBUG" /D "NGL_FINAL" /D "PROJECT_KELLYSLATER" /FR /YX /FD /G6 /Ztmp /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo /o"Bootable/ngl_xboxf.bsc"
# ADD BSC32 /nologo /o"Bootable/ngl_xboxf.bsc"
LIB32=snLib.exe
# ADD BASE LIB32 /nologo /out:"Bootable\ngl_xboxf.lib"
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "ngl_xbox - Xbox Release"
# Name "ngl_xbox - Xbox Debug"
# Name "ngl_xbox - Xbox Final"
# Name "ngl_xbox - Xbox Bootable"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\ngl_dds.cpp
# End Source File
# Begin Source File

SOURCE=.\ngl_InstBank.cpp
# End Source File
# Begin Source File

SOURCE=.\ngl_xbox.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\ngl_dds.h
# End Source File
# Begin Source File

SOURCE=.\ngl_FixedStr.h
# End Source File
# Begin Source File

SOURCE=.\ngl_InstBank.h
# End Source File
# Begin Source File

SOURCE=.\ngl_PixelShadersConst.h
# End Source File
# Begin Source File

SOURCE=.\ngl_PixelShadersMacro.h
# End Source File
# Begin Source File

SOURCE=.\ngl_types.h
# End Source File
# Begin Source File

SOURCE=.\ngl_version.h
# End Source File
# Begin Source File

SOURCE=.\ngl_VertexShaders.h
# End Source File
# Begin Source File

SOURCE=.\ngl_xbox.h
# End Source File
# Begin Source File

SOURCE=.\ngl_xbox_internal.h
# End Source File
# End Group
# Begin Group "PixelShaders"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\PixelShaders\ps_1proj.psh

!IF  "$(CFG)" == "ngl_xbox - Xbox Release"

USERDEP__PS_1P="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_1proj.psh
InputName=ps_1proj

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Debug"

USERDEP__PS_1P="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_1proj.psh
InputName=ps_1proj

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Final"

USERDEP__PS_1P="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_1proj.psh
InputName=ps_1proj

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Bootable"

USERDEP__PS_1P="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_1proj.psh
InputName=ps_1proj

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PixelShaders\ps_2proj.psh

!IF  "$(CFG)" == "ngl_xbox - Xbox Release"

USERDEP__PS_2P="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_2proj.psh
InputName=ps_2proj

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Debug"

USERDEP__PS_2P="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_2proj.psh
InputName=ps_2proj

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Final"

USERDEP__PS_2P="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_2proj.psh
InputName=ps_2proj

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Bootable"

USERDEP__PS_2P="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_2proj.psh
InputName=ps_2proj

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PixelShaders\ps_3proj.psh

!IF  "$(CFG)" == "ngl_xbox - Xbox Release"

USERDEP__PS_3P="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_3proj.psh
InputName=ps_3proj

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Debug"

USERDEP__PS_3P="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_3proj.psh
InputName=ps_3proj

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Final"

USERDEP__PS_3P="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_3proj.psh
InputName=ps_3proj

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Bootable"

USERDEP__PS_3P="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_3proj.psh
InputName=ps_3proj

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PixelShaders\ps_4proj.psh

!IF  "$(CFG)" == "ngl_xbox - Xbox Release"

USERDEP__PS_4P="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_4proj.psh
InputName=ps_4proj

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Debug"

USERDEP__PS_4P="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_4proj.psh
InputName=ps_4proj

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Final"

USERDEP__PS_4P="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_4proj.psh
InputName=ps_4proj

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Bootable"

USERDEP__PS_4P="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_4proj.psh
InputName=ps_4proj

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PixelShaders\ps_el_win.psh

!IF  "$(CFG)" == "ngl_xbox - Xbox Release"

# Begin Custom Build
InputPath=.\PixelShaders\ps_el_win.psh
InputName=ps_el_win

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Debug"

# Begin Custom Build
InputPath=.\PixelShaders\ps_el_win.psh
InputName=ps_el_win

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Final"

# Begin Custom Build
InputPath=.\PixelShaders\ps_el_win.psh
InputName=ps_el_win

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Bootable"

# Begin Custom Build
InputPath=.\PixelShaders\ps_el_win.psh
InputName=ps_el_win

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PixelShaders\ps_notex.psh

!IF  "$(CFG)" == "ngl_xbox - Xbox Release"

USERDEP__PS_NO="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_notex.psh
InputName=ps_notex

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Debug"

USERDEP__PS_NO="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_notex.psh
InputName=ps_notex

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Final"

USERDEP__PS_NO="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_notex.psh
InputName=ps_notex

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Bootable"

USERDEP__PS_NO="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_notex.psh
InputName=ps_notex

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PixelShaders\ps_t.psh

!IF  "$(CFG)" == "ngl_xbox - Xbox Release"

USERDEP__PS_T_="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_t.psh
InputName=ps_t

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Debug"

USERDEP__PS_T_="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_t.psh
InputName=ps_t

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Final"

USERDEP__PS_T_="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_t.psh
InputName=ps_t

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Bootable"

USERDEP__PS_T_="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_t.psh
InputName=ps_t

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PixelShaders\ps_t_mc.psh

!IF  "$(CFG)" == "ngl_xbox - Xbox Release"

USERDEP__PS_T_M="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_t_mc.psh
InputName=ps_t_mc

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Debug"

USERDEP__PS_T_M="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_t_mc.psh
InputName=ps_t_mc

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Final"

USERDEP__PS_T_M="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_t_mc.psh
InputName=ps_t_mc

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Bootable"

USERDEP__PS_T_M="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_t_mc.psh
InputName=ps_t_mc

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PixelShaders\ps_td.psh

!IF  "$(CFG)" == "ngl_xbox - Xbox Release"

USERDEP__PS_TD="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_td.psh
InputName=ps_td

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Debug"

USERDEP__PS_TD="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_td.psh
InputName=ps_td

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Final"

USERDEP__PS_TD="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_td.psh
InputName=ps_td

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Bootable"

USERDEP__PS_TD="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_td.psh
InputName=ps_td

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PixelShaders\ps_td_mc.psh

!IF  "$(CFG)" == "ngl_xbox - Xbox Release"

USERDEP__PS_TD_="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_td_mc.psh
InputName=ps_td_mc

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Debug"

USERDEP__PS_TD_="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_td_mc.psh
InputName=ps_td_mc

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Final"

USERDEP__PS_TD_="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_td_mc.psh
InputName=ps_td_mc

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Bootable"

USERDEP__PS_TD_="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_td_mc.psh
InputName=ps_td_mc

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PixelShaders\ps_tde.psh

!IF  "$(CFG)" == "ngl_xbox - Xbox Release"

USERDEP__PS_TDE="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_tde.psh
InputName=ps_tde

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Debug"

USERDEP__PS_TDE="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_tde.psh
InputName=ps_tde

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Final"

USERDEP__PS_TDE="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_tde.psh
InputName=ps_tde

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Bootable"

USERDEP__PS_TDE="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_tde.psh
InputName=ps_tde

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PixelShaders\ps_tde_mc.psh

!IF  "$(CFG)" == "ngl_xbox - Xbox Release"

USERDEP__PS_TDE_="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_tde_mc.psh
InputName=ps_tde_mc

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Debug"

USERDEP__PS_TDE_="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_tde_mc.psh
InputName=ps_tde_mc

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Final"

USERDEP__PS_TDE_="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_tde_mc.psh
InputName=ps_tde_mc

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Bootable"

USERDEP__PS_TDE_="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_tde_mc.psh
InputName=ps_tde_mc

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PixelShaders\ps_tdel.psh

!IF  "$(CFG)" == "ngl_xbox - Xbox Release"

USERDEP__PS_TDEL="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_tdel.psh
InputName=ps_tdel

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Debug"

USERDEP__PS_TDEL="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_tdel.psh
InputName=ps_tdel

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Final"

USERDEP__PS_TDEL="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_tdel.psh
InputName=ps_tdel

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Bootable"

USERDEP__PS_TDEL="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_tdel.psh
InputName=ps_tdel

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PixelShaders\ps_tdel_mc.psh

!IF  "$(CFG)" == "ngl_xbox - Xbox Release"

USERDEP__PS_TDEL_="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_tdel_mc.psh
InputName=ps_tdel_mc

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Debug"

USERDEP__PS_TDEL_="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_tdel_mc.psh
InputName=ps_tdel_mc

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Final"

USERDEP__PS_TDEL_="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_tdel_mc.psh
InputName=ps_tdel_mc

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Bootable"

USERDEP__PS_TDEL_="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_tdel_mc.psh
InputName=ps_tdel_mc

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PixelShaders\ps_tdl.psh

!IF  "$(CFG)" == "ngl_xbox - Xbox Release"

USERDEP__PS_TDL="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_tdl.psh
InputName=ps_tdl

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Debug"

USERDEP__PS_TDL="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_tdl.psh
InputName=ps_tdl

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Final"

USERDEP__PS_TDL="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_tdl.psh
InputName=ps_tdl

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Bootable"

USERDEP__PS_TDL="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_tdl.psh
InputName=ps_tdl

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PixelShaders\ps_tdl_mc.psh

!IF  "$(CFG)" == "ngl_xbox - Xbox Release"

USERDEP__PS_TDL_="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_tdl_mc.psh
InputName=ps_tdl_mc

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Debug"

USERDEP__PS_TDL_="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_tdl_mc.psh
InputName=ps_tdl_mc

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Final"

USERDEP__PS_TDL_="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_tdl_mc.psh
InputName=ps_tdl_mc

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Bootable"

USERDEP__PS_TDL_="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_tdl_mc.psh
InputName=ps_tdl_mc

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PixelShaders\ps_te.psh

!IF  "$(CFG)" == "ngl_xbox - Xbox Release"

USERDEP__PS_TE="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_te.psh
InputName=ps_te

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Debug"

USERDEP__PS_TE="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_te.psh
InputName=ps_te

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Final"

USERDEP__PS_TE="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_te.psh
InputName=ps_te

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Bootable"

USERDEP__PS_TE="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_te.psh
InputName=ps_te

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PixelShaders\ps_te_mc.psh

!IF  "$(CFG)" == "ngl_xbox - Xbox Release"

USERDEP__PS_TE_="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_te_mc.psh
InputName=ps_te_mc

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Debug"

USERDEP__PS_TE_="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_te_mc.psh
InputName=ps_te_mc

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Final"

USERDEP__PS_TE_="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_te_mc.psh
InputName=ps_te_mc

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Bootable"

USERDEP__PS_TE_="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_te_mc.psh
InputName=ps_te_mc

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PixelShaders\ps_tel.psh

!IF  "$(CFG)" == "ngl_xbox - Xbox Release"

USERDEP__PS_TEL="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_tel.psh
InputName=ps_tel

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Debug"

USERDEP__PS_TEL="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_tel.psh
InputName=ps_tel

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Final"

USERDEP__PS_TEL="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_tel.psh
InputName=ps_tel

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Bootable"

USERDEP__PS_TEL="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_tel.psh
InputName=ps_tel

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PixelShaders\ps_tel2.psh

!IF  "$(CFG)" == "ngl_xbox - Xbox Release"

USERDEP__PS_TEL2="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_tel2.psh
InputName=ps_tel2

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Debug"

USERDEP__PS_TEL2="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_tel2.psh
InputName=ps_tel2

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Final"

USERDEP__PS_TEL2="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_tel2.psh
InputName=ps_tel2

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Bootable"

USERDEP__PS_TEL2="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_tel2.psh
InputName=ps_tel2

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PixelShaders\ps_tel2_mc.psh

!IF  "$(CFG)" == "ngl_xbox - Xbox Release"

USERDEP__PS_TEL2_="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_tel2_mc.psh
InputName=ps_tel2_mc

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Debug"

USERDEP__PS_TEL2_="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_tel2_mc.psh
InputName=ps_tel2_mc

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Final"

USERDEP__PS_TEL2_="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_tel2_mc.psh
InputName=ps_tel2_mc

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Bootable"

USERDEP__PS_TEL2_="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_tel2_mc.psh
InputName=ps_tel2_mc

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PixelShaders\ps_tel3.psh

!IF  "$(CFG)" == "ngl_xbox - Xbox Release"

# Begin Custom Build
InputPath=.\PixelShaders\ps_tel3.psh
InputName=ps_tel3

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Debug"

# Begin Custom Build
InputPath=.\PixelShaders\ps_tel3.psh
InputName=ps_tel3

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Final"

# Begin Custom Build
InputPath=.\PixelShaders\ps_tel3.psh
InputName=ps_tel3

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Bootable"

# Begin Custom Build
InputPath=.\PixelShaders\ps_tel3.psh
InputName=ps_tel3

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PixelShaders\ps_tel3_mc.psh

!IF  "$(CFG)" == "ngl_xbox - Xbox Release"

# Begin Custom Build
InputPath=.\PixelShaders\ps_tel3_mc.psh
InputName=ps_tel3_mc

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Debug"

# Begin Custom Build
InputPath=.\PixelShaders\ps_tel3_mc.psh
InputName=ps_tel3_mc

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Final"

# Begin Custom Build
InputPath=.\PixelShaders\ps_tel3_mc.psh
InputName=ps_tel3_mc

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Bootable"

# Begin Custom Build
InputPath=.\PixelShaders\ps_tel3_mc.psh
InputName=ps_tel3_mc

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PixelShaders\ps_tel_mc.psh

!IF  "$(CFG)" == "ngl_xbox - Xbox Release"

USERDEP__PS_TEL_="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_tel_mc.psh
InputName=ps_tel_mc

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Debug"

USERDEP__PS_TEL_="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_tel_mc.psh
InputName=ps_tel_mc

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Final"

USERDEP__PS_TEL_="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_tel_mc.psh
InputName=ps_tel_mc

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Bootable"

USERDEP__PS_TEL_="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_tel_mc.psh
InputName=ps_tel_mc

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PixelShaders\ps_tl.psh

!IF  "$(CFG)" == "ngl_xbox - Xbox Release"

USERDEP__PS_TL="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_tl.psh
InputName=ps_tl

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Debug"

USERDEP__PS_TL="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_tl.psh
InputName=ps_tl

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Final"

USERDEP__PS_TL="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_tl.psh
InputName=ps_tl

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Bootable"

USERDEP__PS_TL="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_tl.psh
InputName=ps_tl

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PixelShaders\ps_tl2.psh

!IF  "$(CFG)" == "ngl_xbox - Xbox Release"

USERDEP__PS_TL2="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_tl2.psh
InputName=ps_tl2

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Debug"

USERDEP__PS_TL2="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_tl2.psh
InputName=ps_tl2

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Final"

USERDEP__PS_TL2="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_tl2.psh
InputName=ps_tl2

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Bootable"

USERDEP__PS_TL2="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_tl2.psh
InputName=ps_tl2

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PixelShaders\ps_tl2_mc.psh

!IF  "$(CFG)" == "ngl_xbox - Xbox Release"

USERDEP__PS_TL2_="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_tl2_mc.psh
InputName=ps_tl2_mc

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Debug"

USERDEP__PS_TL2_="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_tl2_mc.psh
InputName=ps_tl2_mc

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Final"

USERDEP__PS_TL2_="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_tl2_mc.psh
InputName=ps_tl2_mc

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Bootable"

USERDEP__PS_TL2_="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_tl2_mc.psh
InputName=ps_tl2_mc

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PixelShaders\ps_tl_mc.psh

!IF  "$(CFG)" == "ngl_xbox - Xbox Release"

USERDEP__PS_TL_="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_tl_mc.psh
InputName=ps_tl_mc

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Debug"

USERDEP__PS_TL_="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_tl_mc.psh
InputName=ps_tl_mc

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Final"

USERDEP__PS_TL_="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_tl_mc.psh
InputName=ps_tl_mc

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ELSEIF  "$(CFG)" == "ngl_xbox - Xbox Bootable"

USERDEP__PS_TL_="ngl_PixelShadersConst.h"	
# Begin Custom Build
InputPath=.\PixelShaders\ps_tl_mc.psh
InputName=ps_tl_mc

"PixelShaders\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xsasm /D XBOX /D PROJECT_KELLYSLATER /h /hname ngl_$(InputName)_opcode PixelShaders\$(InputName).psh PixelShaders\$(InputName).h

# End Custom Build

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
