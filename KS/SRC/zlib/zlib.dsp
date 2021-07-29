# Microsoft Developer Studio Project File - Name="zlib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=zlib - Win32 PS2 EE Bootable
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "zlib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "zlib.mak" CFG="zlib - Win32 PS2 EE Bootable"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "zlib - Win32 PS2 EE Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "zlib - Win32 PS2 EE Release" (based on "Win32 (x86) Static Library")
!MESSAGE "zlib - Win32 PS2 EE Bootable" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/ks/src/zlib", UGIBAAAA"
# PROP Scc_LocalPath "."
CPP=snCl.exe
RSC=rc.exe

!IF  "$(CFG)" == "zlib - Win32 PS2 EE Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "PS2_EE_Debug"
# PROP BASE Intermediate_Dir "PS2_EE_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "PS2_EE_Debug"
# PROP Intermediate_Dir "PS2_EE_Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Od /D "SN_TARGET_PS2" /debug -G0 /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=snLib.exe
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"PS2_EE_Debug\libz_ps2.lib" /D:SN_TARGET_PS2

!ELSEIF  "$(CFG)" == "zlib - Win32 PS2 EE Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "PS2_EE_Release"
# PROP BASE Intermediate_Dir "PS2_EE_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "PS2_EE_Release"
# PROP Intermediate_Dir "PS2_EE_Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /O2 /D "SN_TARGET_PS2" -G0 -g /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=snLib.exe
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"PS2_EE_Release\libz_ps2.lib" /D:SN_TARGET_PS2

!ELSEIF  "$(CFG)" == "zlib - Win32 PS2 EE Bootable"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "PS2_EE_Bootable"
# PROP BASE Intermediate_Dir "PS2_EE_Bootable"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "PS2_EE_Bootable"
# PROP Intermediate_Dir "PS2_EE_Bootable"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /O2 /D "SN_TARGET_PS2" /c
# ADD CPP /nologo /W3 /D "SN_TARGET_PS2" -G0 -g /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=snLib.exe
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"PS2_EE_Bootable\libz_ps2.lib" /D:SN_TARGET_PS2

!ENDIF 

# Begin Target

# Name "zlib - Win32 PS2 EE Debug"
# Name "zlib - Win32 PS2 EE Release"
# Name "zlib - Win32 PS2 EE Bootable"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\adler32.c
DEP_CPP_ADLER=\
	".\zconf.h"\
	".\zlib.h"\
	
# End Source File
# Begin Source File

SOURCE=.\compress.c
DEP_CPP_COMPR=\
	".\zconf.h"\
	".\zlib.h"\
	
# End Source File
# Begin Source File

SOURCE=.\crc32.c
DEP_CPP_CRC32=\
	".\zconf.h"\
	".\zlib.h"\
	
# End Source File
# Begin Source File

SOURCE=.\deflate.c
DEP_CPP_DEFLA=\
	".\deflate.h"\
	".\zconf.h"\
	".\zlib.h"\
	".\zutil.h"\
	
# End Source File
# Begin Source File

SOURCE=.\gzio.c
DEP_CPP_GZIO_=\
	".\zconf.h"\
	".\zlib.h"\
	".\zutil.h"\
	
# End Source File
# Begin Source File

SOURCE=.\infblock.c
DEP_CPP_INFBL=\
	".\infblock.h"\
	".\infcodes.h"\
	".\inftrees.h"\
	".\infutil.h"\
	".\zconf.h"\
	".\zlib.h"\
	".\zutil.h"\
	
# End Source File
# Begin Source File

SOURCE=.\infcodes.c
DEP_CPP_INFCO=\
	".\infblock.h"\
	".\infcodes.h"\
	".\inffast.h"\
	".\inftrees.h"\
	".\infutil.h"\
	".\zconf.h"\
	".\zlib.h"\
	".\zutil.h"\
	
# End Source File
# Begin Source File

SOURCE=.\inffast.c
DEP_CPP_INFFA=\
	".\infblock.h"\
	".\infcodes.h"\
	".\inffast.h"\
	".\inftrees.h"\
	".\infutil.h"\
	".\zconf.h"\
	".\zlib.h"\
	".\zutil.h"\
	
# End Source File
# Begin Source File

SOURCE=.\inflate.c
DEP_CPP_INFLA=\
	".\infblock.h"\
	".\zconf.h"\
	".\zlib.h"\
	".\zutil.h"\
	
# End Source File
# Begin Source File

SOURCE=.\inftrees.c
DEP_CPP_INFTR=\
	".\inffixed.h"\
	".\inftrees.h"\
	".\zconf.h"\
	".\zlib.h"\
	".\zutil.h"\
	
# End Source File
# Begin Source File

SOURCE=.\infutil.c
DEP_CPP_INFUT=\
	".\infblock.h"\
	".\infcodes.h"\
	".\inftrees.h"\
	".\infutil.h"\
	".\zconf.h"\
	".\zlib.h"\
	".\zutil.h"\
	
# End Source File
# Begin Source File

SOURCE=.\maketree.c
DEP_CPP_MAKET=\
	".\inftrees.h"\
	".\zconf.h"\
	".\zlib.h"\
	".\zutil.h"\
	
# End Source File
# Begin Source File

SOURCE=.\trees.c
DEP_CPP_TREES=\
	".\deflate.h"\
	".\trees.h"\
	".\zconf.h"\
	".\zlib.h"\
	".\zutil.h"\
	
# End Source File
# Begin Source File

SOURCE=.\uncompr.c
DEP_CPP_UNCOM=\
	".\zconf.h"\
	".\zlib.h"\
	
# End Source File
# Begin Source File

SOURCE=.\zutil.c
DEP_CPP_ZUTIL=\
	".\zconf.h"\
	".\zlib.h"\
	".\zutil.h"\
	
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\deflate.h
# End Source File
# Begin Source File

SOURCE=.\infblock.h
# End Source File
# Begin Source File

SOURCE=.\infcodes.h
# End Source File
# Begin Source File

SOURCE=.\inffast.h
# End Source File
# Begin Source File

SOURCE=.\inffixed.h
# End Source File
# Begin Source File

SOURCE=.\inftrees.h
# End Source File
# Begin Source File

SOURCE=.\infutil.h
# End Source File
# Begin Source File

SOURCE=.\trees.h
# End Source File
# Begin Source File

SOURCE=.\zconf.h
# End Source File
# Begin Source File

SOURCE=.\zlib.h
# End Source File
# Begin Source File

SOURCE=.\zutil.h
# End Source File
# End Group
# End Target
# End Project
