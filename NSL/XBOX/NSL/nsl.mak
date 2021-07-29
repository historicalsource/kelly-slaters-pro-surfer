# Microsoft Developer Studio Generated NMAKE File, Based on nsl.dsp
!IF "$(CFG)" == ""
CFG=nsl - Xbox Debug
!MESSAGE No configuration specified. Defaulting to nsl - Xbox Debug.
!ENDIF 

!IF "$(CFG)" != "nsl - Xbox Release" && "$(CFG)" != "nsl - Xbox Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
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
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe

!IF  "$(CFG)" == "nsl - Xbox Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\nsl.lib"


CLEAN :
	-@erase "$(INTDIR)\nsl_xbox.obj"
	-@erase "$(INTDIR)\pstring.obj"
	-@erase "$(INTDIR)\vc70.idb"
	-@erase "$(OUTDIR)\nsl.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "_XBOX" /D "NDEBUG" /Fp"$(INTDIR)\nsl.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /G6 /Zvc6 /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\nsl.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\nsl.lib" 
LIB32_OBJS= \
	"$(INTDIR)\nsl_xbox.obj" \
	"$(INTDIR)\pstring.obj"

"$(OUTDIR)\nsl.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "nsl - Xbox Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\nsl.lib"


CLEAN :
	-@erase "$(INTDIR)\nsl_xbox.obj"
	-@erase "$(INTDIR)\pstring.obj"
	-@erase "$(INTDIR)\vc70.idb"
	-@erase "$(INTDIR)\vc70.pdb"
	-@erase "$(OUTDIR)\nsl.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MTd /W3 /Gm /GX /Zi /Od /I "-I." /D "WIN32" /D "_XBOX" /D "_DEBUG" /Fp"$(INTDIR)\nsl.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /G6 /Zvc6 /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\nsl.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\nsl.lib" 
LIB32_OBJS= \
	"$(INTDIR)\nsl_xbox.obj" \
	"$(INTDIR)\pstring.obj"

"$(OUTDIR)\nsl.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ENDIF 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("nsl.dep")
!INCLUDE "nsl.dep"
!ELSE 
!MESSAGE Warning: cannot find "nsl.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "nsl - Xbox Release" || "$(CFG)" == "nsl - Xbox Debug"
SOURCE=..\nsl_xbox.cpp

!IF  "$(CFG)" == "nsl - Xbox Release"

CPP_SWITCHES=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "_XBOX" /D "NDEBUG" /Fp"$(INTDIR)\nsl.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /G6 /Zvc6 /c 

"$(INTDIR)\nsl_xbox.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "nsl - Xbox Debug"

CPP_SWITCHES=/nologo /MTd /W3 /Gm /GX /Zi /Od /I "." /D "WIN32" /D "_XBOX" /D "_DEBUG" /Fp"$(INTDIR)\nsl.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /G6 /Zvc6 /c 

"$(INTDIR)\nsl_xbox.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=..\..\common\pstring.cpp

"$(INTDIR)\pstring.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 

