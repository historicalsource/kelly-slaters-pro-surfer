# Microsoft Developer Studio Project File - Name="nslSoundManager" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=nslSoundManager - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "nslSoundManager.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "nslSoundManager.mak" CFG="nslSoundManager - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "nslSoundManager - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "nslSoundManager - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "nslSoundManager - Win32 Release"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "nslSoundManager - Win32 Debug"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /Yu"stdafx.h" /FD /GZ  /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /GZ   /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "nslSoundManager - Win32 Release"
# Name "nslSoundManager - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\font.cpp
# End Source File
# Begin Source File

SOURCE=.\node.cpp
# End Source File
# Begin Source File

SOURCE=.\nodes.cpp
# End Source File
# Begin Source File

SOURCE=.\nslSoundManager.cpp
# End Source File
# Begin Source File

SOURCE=.\nslSoundManager.rc
# End Source File
# Begin Source File

SOURCE=.\nslSoundManagerDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\oleobject.cpp
# End Source File
# Begin Source File

SOURCE=.\oleobjects.cpp
# End Source File
# Begin Source File

SOURCE=.\picture.cpp
# End Source File
# Begin Source File

SOURCE=.\richtext.cpp
# End Source File
# Begin Source File

SOURCE=.\slider.cpp
# End Source File
# Begin Source File

SOURCE=.\SoundFileManager.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\treeview.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\font.h
# End Source File
# Begin Source File

SOURCE=.\node.h
# End Source File
# Begin Source File

SOURCE=.\nodes.h
# End Source File
# Begin Source File

SOURCE=.\nslSoundManager.h
# End Source File
# Begin Source File

SOURCE=.\nslSoundManagerDlg.h
# End Source File
# Begin Source File

SOURCE=.\oleobject.h
# End Source File
# Begin Source File

SOURCE=.\oleobjects.h
# End Source File
# Begin Source File

SOURCE=.\picture.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\richtext.h
# End Source File
# Begin Source File

SOURCE=.\slider.h
# End Source File
# Begin Source File

SOURCE=.\SoundFileManager.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\treeview.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\idr_voic.ico
# End Source File
# Begin Source File

SOURCE=.\res\nslSoundManager.ico
# End Source File
# Begin Source File

SOURCE=.\res\nslSoundManager.rc2
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
# Section nslSoundManager : {BEF6E003-A874-101A-8BBA-00AA00300CAB}
# 	2:5:Class:COleFont
# 	2:10:HeaderFile:font.h
# 	2:8:ImplFile:font.cpp
# End Section
# Section nslSoundManager : {C74190B7-8589-11D1-B16A-00C0F0283628}
# 	2:5:Class:CNodes
# 	2:10:HeaderFile:nodes.h
# 	2:8:ImplFile:nodes.cpp
# End Section
# Section nslSoundManager : {3B7C8860-D78F-101B-B9B5-04021C009402}
# 	2:21:DefaultSinkHeaderFile:richtext.h
# 	2:16:DefaultSinkClass:CRichText
# End Section
# Section nslSoundManager : {F08DF952-8592-11D1-B16A-00C0F0283628}
# 	2:5:Class:CSlider
# 	2:10:HeaderFile:slider.h
# 	2:8:ImplFile:slider.cpp
# End Section
# Section nslSoundManager : {7BF80981-BF32-101A-8BBB-00AA00300CAB}
# 	2:5:Class:CPicture
# 	2:10:HeaderFile:picture.h
# 	2:8:ImplFile:picture.cpp
# End Section
# Section nslSoundManager : {C74190B6-8589-11D1-B16A-00C0F0283628}
# 	2:21:DefaultSinkHeaderFile:treeview.h
# 	2:16:DefaultSinkClass:CTreeView1
# End Section
# Section nslSoundManager : {ED117630-4090-11CF-8981-00AA00688B10}
# 	2:5:Class:COLEObject
# 	2:10:HeaderFile:oleobject.h
# 	2:8:ImplFile:oleobject.cpp
# End Section
# Section nslSoundManager : {F08DF954-8592-11D1-B16A-00C0F0283628}
# 	2:21:DefaultSinkHeaderFile:slider.h
# 	2:16:DefaultSinkClass:CSlider
# End Section
# Section nslSoundManager : {C74190B4-8589-11D1-B16A-00C0F0283628}
# 	2:5:Class:CTreeView1
# 	2:10:HeaderFile:treeview.h
# 	2:8:ImplFile:treeview.cpp
# End Section
# Section nslSoundManager : {E9A5593C-CAB0-11D1-8C0B-0000F8754DA1}
# 	2:5:Class:CRichText
# 	2:10:HeaderFile:richtext.h
# 	2:8:ImplFile:richtext.cpp
# End Section
# Section nslSoundManager : {859321D0-3FD1-11CF-8981-00AA00688B10}
# 	2:5:Class:COLEObjects
# 	2:10:HeaderFile:oleobjects.h
# 	2:8:ImplFile:oleobjects.cpp
# End Section
# Section nslSoundManager : {C74190B8-8589-11D1-B16A-00C0F0283628}
# 	2:5:Class:CNode
# 	2:10:HeaderFile:node.h
# 	2:8:ImplFile:node.cpp
# End Section
