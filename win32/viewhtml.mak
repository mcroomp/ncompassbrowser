# Microsoft Visual C++ Generated NMAKE File, Format Version 2.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

!IF "$(CFG)" == ""
CFG=Win32 Debug
!MESSAGE No configuration specified.  Defaulting to Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "Win32 Debug" && "$(CFG)" != "Win32 Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "VIEWHTML.MAK" CFG="Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

################################################################################
# Begin Project
# PROP Target_Last_Scanned "Win32 Debug"
MTL=MkTypLib.exe
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "WinDebug"
# PROP BASE Intermediate_Dir "WinDebug"
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "WinDebug"
# PROP Intermediate_Dir "WinDebug"
OUTDIR=.\WinDebug
INTDIR=.\WinDebug

ALL : MTL_TLBS .\WinDebug\VIEWHTML.exe .\WinDebug\VIEWHTML.bsc

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 

MTL_TLBS : .\WinDebug\viewhtml.tlb
# ADD BASE CPP /nologo /MD /W3 /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_AFXDLL" /FR /Yu"stdafx.h" /c
# ADD CPP /nologo /MT /W3 /GX /Zi /Od /I "p:\viewhtml\cross_p\inc" /I "p:\viewhtml\win32\inc" /I "p:\viewhtml\jpeg\inc" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "USES_OLE_CONTROLS" /D "USES_INTERNET" /FR /Yu"cross_p.h" /c
CPP_PROJ=/nologo /MT /W3 /GX /Zi /Od /I "p:\viewhtml\cross_p\inc" /I\
 "p:\viewhtml\win32\inc" /I "p:\viewhtml\jpeg\inc" /D "_DEBUG" /D "WIN32" /D\
 "_WINDOWS" /D "_MBCS" /D "USES_OLE_CONTROLS" /D "USES_INTERNET" /FR$(INTDIR)/\
 /Fp$(OUTDIR)/"VIEWHTML.pch" /Yu"cross_p.h" /Fo$(INTDIR)/\
 /Fd$(OUTDIR)/"VIEWHTML.pdb" /c 
CPP_OBJS=.\WinDebug/
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /i "p:\viewhtml\win32\inc" /i "p:\viewhtml\cross_p\inc" /d "_DEBUG"
RSC_PROJ=/l 0x409 /fo$(INTDIR)/"VIEWHTML.res" /i "p:\viewhtml\win32\inc" /i\
 "p:\viewhtml\cross_p\inc" /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"VIEWHTML.bsc" 
BSC32_SBRS= \
	.\WinDebug\STDAFX.SBR \
	.\WinDebug\viewhtml.sbr \
	.\WinDebug\mainfrm.sbr \
	.\WinDebug\viewhdoc.sbr \
	.\WinDebug\viewhvw.sbr \
	.\WinDebug\CNTRITEM.SBR \
	.\WinDebug\CNTLITEM.SBR \
	.\WinDebug\INCTLDLG.SBR \
	.\WinDebug\DMEMFILE.SBR \
	.\WinDebug\DOPENURL.SBR \
	.\WinDebug\BITBLT.SBR \
	.\WinDebug\DDSKFILE.SBR \
	.\WinDebug\protocol.sbr \
	.\WinDebug\bookmark.sbr \
	.\WinDebug\MUTEX.SBR \
	.\WinDebug\dlgpref.sbr \
	.\WinDebug\tsocket.sbr \
	.\WinDebug\smtp.sbr \
	.\WinDebug\PICTURE.SBR \
	.\WinDebug\FMTHTML.SBR \
	.\WinDebug\BIGSTR.SBR \
	.\WinDebug\CONTAIN.SBR \
	.\WinDebug\TBLSIZE.SBR \
	.\WinDebug\MIMELOAD.SBR \
	.\WinDebug\READGIF.SBR \
	.\WinDebug\READJPEG.SBR \
	.\WinDebug\DYNLOAD.SBR \
	.\WinDebug\READHTML.SBR \
	.\WinDebug\JDMASTER.sbr \
	.\WinDebug\JMEMNOBS.sbr \
	.\WinDebug\JIDCTFST.sbr \
	.\WinDebug\JUTILS.sbr \
	.\WinDebug\JMEMMGR.sbr \
	.\WinDebug\JDAPI.sbr \
	.\WinDebug\JDDCTMGR.sbr \
	.\WinDebug\JQUANT1.sbr \
	.\WinDebug\JDHUFF.sbr \
	.\WinDebug\JDMAINCT.sbr \
	.\WinDebug\JDSAMPLE.sbr \
	.\WinDebug\JERROR.sbr \
	.\WinDebug\JDCOEFCT.sbr \
	.\WinDebug\JDPOSTCT.sbr \
	.\WinDebug\JDMARKER.sbr \
	.\WinDebug\JDMERGE.sbr \
	.\WinDebug\JDCOLOR.sbr \
	.\WinDebug\JCOMAPI.sbr

.\WinDebug\VIEWHTML.bsc : $(OUTDIR)  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 /NOLOGO /SUBSYSTEM:windows /DEBUG /MACHINE:I386
# SUBTRACT BASE LINK32 /PDB:none
# ADD LINK32 nafxcwd.lib version.lib /NOLOGO /ENTRY:"WinMainCRTStartup" /SUBSYSTEM:windows /DEBUG /MACHINE:I386 /FORCE
# SUBTRACT LINK32 /PDB:none
LINK32_FLAGS=nafxcwd.lib version.lib /NOLOGO /ENTRY:"WinMainCRTStartup"\
 /SUBSYSTEM:windows /INCREMENTAL:yes /PDB:$(OUTDIR)/"VIEWHTML.pdb" /DEBUG\
 /MACHINE:I386 /FORCE /OUT:$(OUTDIR)/"VIEWHTML.exe" 
DEF_FILE=
LINK32_OBJS= \
	.\WinDebug\STDAFX.OBJ \
	.\WinDebug\viewhtml.obj \
	.\WinDebug\mainfrm.obj \
	.\WinDebug\viewhdoc.obj \
	.\WinDebug\viewhvw.obj \
	.\WinDebug\CNTRITEM.OBJ \
	.\WinDebug\VIEWHTML.res \
	.\WinDebug\CNTLITEM.OBJ \
	.\WinDebug\INCTLDLG.OBJ \
	.\WinDebug\DMEMFILE.OBJ \
	.\WinDebug\DOPENURL.OBJ \
	.\WinDebug\BITBLT.OBJ \
	.\WinDebug\DDSKFILE.OBJ \
	.\WinDebug\protocol.obj \
	.\WinDebug\bookmark.obj \
	.\WinDebug\MUTEX.OBJ \
	.\WinDebug\dlgpref.obj \
	.\WinDebug\tsocket.obj \
	.\WinDebug\smtp.obj \
	.\WinDebug\PICTURE.OBJ \
	.\WinDebug\FMTHTML.OBJ \
	.\WinDebug\BIGSTR.OBJ \
	.\WinDebug\CONTAIN.OBJ \
	.\WinDebug\TBLSIZE.OBJ \
	.\WinDebug\MIMELOAD.OBJ \
	.\WinDebug\READGIF.OBJ \
	.\WinDebug\READJPEG.OBJ \
	.\WinDebug\DYNLOAD.OBJ \
	.\WinDebug\READHTML.OBJ \
	.\WinDebug\JDMASTER.obj \
	.\WinDebug\JMEMNOBS.obj \
	.\WinDebug\JIDCTFST.obj \
	.\WinDebug\JUTILS.obj \
	.\WinDebug\JMEMMGR.obj \
	.\WinDebug\JDAPI.obj \
	.\WinDebug\JDDCTMGR.obj \
	.\WinDebug\JQUANT1.obj \
	.\WinDebug\JDHUFF.obj \
	.\WinDebug\JDMAINCT.obj \
	.\WinDebug\JDSAMPLE.obj \
	.\WinDebug\JERROR.obj \
	.\WinDebug\JDCOEFCT.obj \
	.\WinDebug\JDPOSTCT.obj \
	.\WinDebug\JDMARKER.obj \
	.\WinDebug\JDMERGE.obj \
	.\WinDebug\JDCOLOR.obj \
	.\WinDebug\JCOMAPI.obj

.\WinDebug\VIEWHTML.exe : $(OUTDIR)  $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "WinRel"
# PROP BASE Intermediate_Dir "WinRel"
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "WinRel"
# PROP Intermediate_Dir "WinRel"
OUTDIR=.\WinRel
INTDIR=.\WinRel

ALL : MTL_TLBS .\WinRel\VIEWHTML.exe .\WinRel\VIEWHTML.bsc

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 

MTL_TLBS : .\WinRel\viewhtml.tlb
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_AFXDLL" /FR /Yu"stdafx.h" /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "p:\viewhtml\cross_p\inc" /I "p:\viewhtml\win32\inc" /I "p:\viewhtml\jpeg\inc" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "USES_OLE_CONTROLS" /D "USES_INTERNET" /FR /Yu"cross_p.h" /c
# SUBTRACT CPP /Z<none>
CPP_PROJ=/nologo /MT /W3 /GX /O2 /I "p:\viewhtml\cross_p\inc" /I\
 "p:\viewhtml\win32\inc" /I "p:\viewhtml\jpeg\inc" /D "NDEBUG" /D "WIN32" /D\
 "_WINDOWS" /D "_MBCS" /D "USES_OLE_CONTROLS" /D "USES_INTERNET" /FR$(INTDIR)/\
 /Fp$(OUTDIR)/"VIEWHTML.pch" /Yu"cross_p.h" /Fo$(INTDIR)/ /c 
CPP_OBJS=.\WinRel/
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /i "p:\viewhtml\win32\inc" /i "p:\viewhtml\cross_p\inc" /d "NDEBUG"
RSC_PROJ=/l 0x409 /fo$(INTDIR)/"VIEWHTML.res" /i "p:\viewhtml\win32\inc" /i\
 "p:\viewhtml\cross_p\inc" /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"VIEWHTML.bsc" 
BSC32_SBRS= \
	.\WinRel\STDAFX.SBR \
	.\WinRel\viewhtml.sbr \
	.\WinRel\mainfrm.sbr \
	.\WinRel\viewhdoc.sbr \
	.\WinRel\viewhvw.sbr \
	.\WinRel\CNTRITEM.SBR \
	.\WinRel\CNTLITEM.SBR \
	.\WinRel\INCTLDLG.SBR \
	.\WinRel\DMEMFILE.SBR \
	.\WinRel\DOPENURL.SBR \
	.\WinRel\BITBLT.SBR \
	.\WinRel\DDSKFILE.SBR \
	.\WinRel\protocol.sbr \
	.\WinRel\bookmark.sbr \
	.\WinRel\MUTEX.SBR \
	.\WinRel\dlgpref.sbr \
	.\WinRel\tsocket.sbr \
	.\WinRel\smtp.sbr \
	.\WinRel\PICTURE.SBR \
	.\WinRel\FMTHTML.SBR \
	.\WinRel\BIGSTR.SBR \
	.\WinRel\CONTAIN.SBR \
	.\WinRel\TBLSIZE.SBR \
	.\WinRel\MIMELOAD.SBR \
	.\WinRel\READGIF.SBR \
	.\WinRel\READJPEG.SBR \
	.\WinRel\DYNLOAD.SBR \
	.\WinRel\READHTML.SBR \
	.\WinRel\JDMASTER.sbr \
	.\WinRel\JMEMNOBS.sbr \
	.\WinRel\JIDCTFST.sbr \
	.\WinRel\JUTILS.sbr \
	.\WinRel\JMEMMGR.sbr \
	.\WinRel\JDAPI.sbr \
	.\WinRel\JDDCTMGR.sbr \
	.\WinRel\JQUANT1.sbr \
	.\WinRel\JDHUFF.sbr \
	.\WinRel\JDMAINCT.sbr \
	.\WinRel\JDSAMPLE.sbr \
	.\WinRel\JERROR.sbr \
	.\WinRel\JDCOEFCT.sbr \
	.\WinRel\JDPOSTCT.sbr \
	.\WinRel\JDMARKER.sbr \
	.\WinRel\JDMERGE.sbr \
	.\WinRel\JDCOLOR.sbr \
	.\WinRel\JCOMAPI.sbr

.\WinRel\VIEWHTML.bsc : $(OUTDIR)  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 /NOLOGO /SUBSYSTEM:windows /MACHINE:I386
# SUBTRACT BASE LINK32 /PDB:none
# ADD LINK32 nafxcw.lib version.lib /NOLOGO /ENTRY:"WinMainCRTStartup" /SUBSYSTEM:windows /MACHINE:I386 /FORCE
# SUBTRACT LINK32 /PROFILE /DEBUG
LINK32_FLAGS=nafxcw.lib version.lib /NOLOGO /ENTRY:"WinMainCRTStartup"\
 /SUBSYSTEM:windows /INCREMENTAL:no /PDB:$(OUTDIR)/"VIEWHTML.pdb" /MACHINE:I386\
 /FORCE /OUT:$(OUTDIR)/"VIEWHTML.exe" 
DEF_FILE=
LINK32_OBJS= \
	.\WinRel\STDAFX.OBJ \
	.\WinRel\viewhtml.obj \
	.\WinRel\mainfrm.obj \
	.\WinRel\viewhdoc.obj \
	.\WinRel\viewhvw.obj \
	.\WinRel\CNTRITEM.OBJ \
	.\WinRel\VIEWHTML.res \
	.\WinRel\CNTLITEM.OBJ \
	.\WinRel\INCTLDLG.OBJ \
	.\WinRel\DMEMFILE.OBJ \
	.\WinRel\DOPENURL.OBJ \
	.\WinRel\BITBLT.OBJ \
	.\WinRel\DDSKFILE.OBJ \
	.\WinRel\protocol.obj \
	.\WinRel\bookmark.obj \
	.\WinRel\MUTEX.OBJ \
	.\WinRel\dlgpref.obj \
	.\WinRel\tsocket.obj \
	.\WinRel\smtp.obj \
	.\WinRel\PICTURE.OBJ \
	.\WinRel\FMTHTML.OBJ \
	.\WinRel\BIGSTR.OBJ \
	.\WinRel\CONTAIN.OBJ \
	.\WinRel\TBLSIZE.OBJ \
	.\WinRel\MIMELOAD.OBJ \
	.\WinRel\READGIF.OBJ \
	.\WinRel\READJPEG.OBJ \
	.\WinRel\DYNLOAD.OBJ \
	.\WinRel\READHTML.OBJ \
	.\WinRel\JDMASTER.obj \
	.\WinRel\JMEMNOBS.obj \
	.\WinRel\JIDCTFST.obj \
	.\WinRel\JUTILS.obj \
	.\WinRel\JMEMMGR.obj \
	.\WinRel\JDAPI.obj \
	.\WinRel\JDDCTMGR.obj \
	.\WinRel\JQUANT1.obj \
	.\WinRel\JDHUFF.obj \
	.\WinRel\JDMAINCT.obj \
	.\WinRel\JDSAMPLE.obj \
	.\WinRel\JERROR.obj \
	.\WinRel\JDCOEFCT.obj \
	.\WinRel\JDPOSTCT.obj \
	.\WinRel\JDMARKER.obj \
	.\WinRel\JDMERGE.obj \
	.\WinRel\JDCOLOR.obj \
	.\WinRel\JCOMAPI.obj

.\WinRel\VIEWHTML.exe : $(OUTDIR)  $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

################################################################################
# Begin Group "Source Files"

################################################################################
# Begin Source File

SOURCE=.\STDAFX.CPP
DEP_STDAF=\
	\viewhtml\CROSS_P\INC\CROSS_P.H\
	.\INC\STDAFX.H

!IF  "$(CFG)" == "Win32 Debug"

# ADD BASE CPP /Yc"stdafx.h"
# ADD CPP /Yc"cross_p.h"

.\WinDebug\STDAFX.OBJ :  $(SOURCE)  $(DEP_STDAF) $(INTDIR)
   $(CPP) /nologo /MT /W3 /GX /Zi /Od /I "p:\viewhtml\cross_p\inc" /I\
 "p:\viewhtml\win32\inc" /I "p:\viewhtml\jpeg\inc" /D "_DEBUG" /D "WIN32" /D\
 "_WINDOWS" /D "_MBCS" /D "USES_OLE_CONTROLS" /D "USES_INTERNET" /FR$(INTDIR)/\
 /Fp$(OUTDIR)/"VIEWHTML.pch" /Yc"cross_p.h" /Fo$(INTDIR)/\
 /Fd$(OUTDIR)/"VIEWHTML.pdb" /c  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Release"

# ADD BASE CPP /Yc"stdafx.h"
# ADD CPP /Yc"cross_p.h"

.\WinRel\STDAFX.OBJ :  $(SOURCE)  $(DEP_STDAF) $(INTDIR)
   $(CPP) /nologo /MT /W3 /GX /O2 /I "p:\viewhtml\cross_p\inc" /I\
 "p:\viewhtml\win32\inc" /I "p:\viewhtml\jpeg\inc" /D "NDEBUG" /D "WIN32" /D\
 "_WINDOWS" /D "_MBCS" /D "USES_OLE_CONTROLS" /D "USES_INTERNET" /FR$(INTDIR)/\
 /Fp$(OUTDIR)/"VIEWHTML.pch" /Yc"cross_p.h" /Fo$(INTDIR)/ /c  $(SOURCE) 

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\viewhtml.cpp
DEP_VIEWH=\
	\viewhtml\CROSS_P\INC\CROSS_P.H\
	.\INC\viewhtml.h\
	.\INC\mainfrm.h\
	.\INC\viewhdoc.h\
	.\INC\viewhvw.h\
	.\INC\DOPENURL.H\
	.\INC\dlgpref.h\
	\viewhtml\CROSS_P\INC\DYNLOAD.H\
	.\INC\protocol.h\
	.\INC\STDAFX.H\
	.\INC\bookmark.h\
	.\INC\CNTLINFO.H\
	\viewhtml\CROSS_P\INC\FMTHTML.H\
	.\INC\mutex.h\
	.\INC\tsocket.h\
	\viewhtml\CROSS_P\INC\READHTML.H\
	.\INC\DMEMFILE.H\
	\viewhtml\CROSS_P\INC\CONTAIN.H\
	\viewhtml\CROSS_P\INC\BIGSTR.H\
	\viewhtml\CROSS_P\INC\MIMELOAD.H

!IF  "$(CFG)" == "Win32 Debug"

.\WinDebug\viewhtml.obj :  $(SOURCE)  $(DEP_VIEWH) $(INTDIR)\
 .\WinDebug\STDAFX.OBJ

!ELSEIF  "$(CFG)" == "Win32 Release"

.\WinRel\viewhtml.obj :  $(SOURCE)  $(DEP_VIEWH) $(INTDIR) .\WinRel\STDAFX.OBJ

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\mainfrm.cpp
DEP_MAINF=\
	\viewhtml\CROSS_P\INC\CROSS_P.H\
	.\INC\viewhtml.h\
	.\INC\BITBLT.H\
	.\INC\mainfrm.h\
	.\INC\STDAFX.H\
	.\INC\bookmark.h

!IF  "$(CFG)" == "Win32 Debug"

.\WinDebug\mainfrm.obj :  $(SOURCE)  $(DEP_MAINF) $(INTDIR)\
 .\WinDebug\STDAFX.OBJ

!ELSEIF  "$(CFG)" == "Win32 Release"

.\WinRel\mainfrm.obj :  $(SOURCE)  $(DEP_MAINF) $(INTDIR) .\WinRel\STDAFX.OBJ

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\viewhdoc.cpp
DEP_VIEWHD=\
	\viewhtml\CROSS_P\INC\CROSS_P.H\
	.\INC\viewhtml.h\
	.\INC\mainfrm.h\
	.\INC\CNTLINFO.H\
	.\INC\viewhdoc.h\
	.\INC\viewhvw.h\
	.\INC\CNTLITEM.H\
	.\INC\CNTRITEM.H\
	.\INC\DOPENURL.H\
	.\INC\smtp.h\
	\viewhtml\CROSS_P\INC\PICTURE.H\
	.\INC\STDAFX.H\
	.\INC\bookmark.h\
	\viewhtml\CROSS_P\INC\FMTHTML.H\
	.\INC\DDSKFILE.H\
	.\INC\DMEMFILE.H\
	\viewhtml\CROSS_P\INC\MIMELOAD.H\
	.\INC\protocol.h\
	\viewhtml\CROSS_P\INC\READHTML.H\
	\viewhtml\CROSS_P\INC\DYNLOAD.H\
	\viewhtml\CROSS_P\INC\BIGSTR.H\
	.\INC\tsocket.h\
	\viewhtml\CROSS_P\INC\CONTAIN.H\
	.\INC\mutex.h

!IF  "$(CFG)" == "Win32 Debug"

.\WinDebug\viewhdoc.obj :  $(SOURCE)  $(DEP_VIEWHD) $(INTDIR)\
 .\WinDebug\STDAFX.OBJ

!ELSEIF  "$(CFG)" == "Win32 Release"

.\WinRel\viewhdoc.obj :  $(SOURCE)  $(DEP_VIEWHD) $(INTDIR) .\WinRel\STDAFX.OBJ

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\viewhvw.cpp
DEP_VIEWHV=\
	\viewhtml\CROSS_P\INC\CROSS_P.H\
	.\INC\viewhtml.h\
	\viewhtml\CROSS_P\INC\PICTURE.H\
	.\INC\BITBLT.H\
	.\INC\mainfrm.h\
	.\INC\viewhdoc.h\
	.\INC\CNTLITEM.H\
	.\INC\CNTRITEM.H\
	.\INC\viewhvw.h\
	.\INC\INCTLDLG.H\
	.\INC\STDAFX.H\
	.\INC\bookmark.h\
	\viewhtml\CROSS_P\INC\MIMELOAD.H\
	.\INC\CNTLINFO.H\
	\viewhtml\CROSS_P\INC\FMTHTML.H\
	.\INC\DDSKFILE.H\
	.\INC\DMEMFILE.H\
	\viewhtml\CROSS_P\INC\DYNLOAD.H\
	\viewhtml\CROSS_P\INC\BIGSTR.H\
	\viewhtml\CROSS_P\INC\READHTML.H\
	.\INC\mutex.h\
	\viewhtml\CROSS_P\INC\CONTAIN.H

!IF  "$(CFG)" == "Win32 Debug"

.\WinDebug\viewhvw.obj :  $(SOURCE)  $(DEP_VIEWHV) $(INTDIR)\
 .\WinDebug\STDAFX.OBJ

!ELSEIF  "$(CFG)" == "Win32 Release"

.\WinRel\viewhvw.obj :  $(SOURCE)  $(DEP_VIEWHV) $(INTDIR) .\WinRel\STDAFX.OBJ

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\CNTRITEM.CPP
DEP_CNTRI=\
	\viewhtml\CROSS_P\INC\CROSS_P.H\
	.\INC\viewhtml.h\
	.\INC\viewhdoc.h\
	.\INC\CNTLITEM.H\
	.\INC\CNTRITEM.H\
	\viewhtml\CROSS_P\INC\FMTHTML.H\
	.\INC\STDAFX.H\
	.\INC\bookmark.h\
	.\INC\CNTLINFO.H\
	.\INC\DDSKFILE.H\
	.\INC\DMEMFILE.H\
	\viewhtml\CROSS_P\INC\READHTML.H\
	\viewhtml\CROSS_P\INC\DYNLOAD.H\
	\viewhtml\CROSS_P\INC\MIMELOAD.H\
	\viewhtml\CROSS_P\INC\BIGSTR.H\
	\viewhtml\CROSS_P\INC\CONTAIN.H\
	.\INC\mutex.h

!IF  "$(CFG)" == "Win32 Debug"

.\WinDebug\CNTRITEM.OBJ :  $(SOURCE)  $(DEP_CNTRI) $(INTDIR)\
 .\WinDebug\STDAFX.OBJ

!ELSEIF  "$(CFG)" == "Win32 Release"

.\WinRel\CNTRITEM.OBJ :  $(SOURCE)  $(DEP_CNTRI) $(INTDIR) .\WinRel\STDAFX.OBJ

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\VIEWHTML.RC
DEP_VIEWHT=\
	.\RES\VIEWHTML.ICO\
	.\RES\CONTDOC.ICO\
	.\RES\BAD_GIF.ICO\
	.\RES\RED.ICO\
	.\RES\toolbar.bmp\
	.\RES\CURSOR1.CUR\
	.\RES\VIEWHTML.RC2

!IF  "$(CFG)" == "Win32 Debug"

.\WinDebug\VIEWHTML.res :  $(SOURCE)  $(DEP_VIEWHT) $(INTDIR)
   $(RSC) /l 0x409 /fo$(INTDIR)/"VIEWHTML.res" /i "p:\viewhtml\win32\inc" /i\
 "p:\viewhtml\cross_p\inc" /i "$(OUTDIR)" /d "_DEBUG"  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Release"

.\WinRel\VIEWHTML.res :  $(SOURCE)  $(DEP_VIEWHT) $(INTDIR)
   $(RSC) /l 0x409 /fo$(INTDIR)/"VIEWHTML.res" /i "p:\viewhtml\win32\inc" /i\
 "p:\viewhtml\cross_p\inc" /i "$(OUTDIR)" /d "NDEBUG"  $(SOURCE) 

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\viewhtml.odl

!IF  "$(CFG)" == "Win32 Debug"

.\WinDebug\viewhtml.tlb :  $(SOURCE)  $(OUTDIR)
   $(MTL) /nologo /D "_DEBUG" /tlb $(OUTDIR)/"viewhtml.tlb" /win32  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Release"

.\WinRel\viewhtml.tlb :  $(SOURCE)  $(OUTDIR)
   $(MTL) /nologo /D "NDEBUG" /tlb $(OUTDIR)/"viewhtml.tlb" /win32  $(SOURCE) 

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\CNTLITEM.CPP
DEP_CNTLI=\
	\viewhtml\CROSS_P\INC\CROSS_P.H\
	.\INC\CNTLINFO.H\
	.\INC\CNTLITEM.H\
	.\INC\viewhdoc.h\
	.\INC\DDSKFILE.H\
	.\INC\viewhtml.h\
	.\INC\OLEIMPL.H\
	.\INC\STDAFX.H\
	.\INC\DMEMFILE.H\
	\viewhtml\CROSS_P\INC\FMTHTML.H\
	\viewhtml\CROSS_P\INC\DYNLOAD.H\
	.\INC\bookmark.h\
	\viewhtml\CROSS_P\INC\MIMELOAD.H\
	\viewhtml\CROSS_P\INC\BIGSTR.H\
	\viewhtml\CROSS_P\INC\READHTML.H\
	.\INC\mutex.h\
	\viewhtml\CROSS_P\INC\CONTAIN.H

!IF  "$(CFG)" == "Win32 Debug"

.\WinDebug\CNTLITEM.OBJ :  $(SOURCE)  $(DEP_CNTLI) $(INTDIR)\
 .\WinDebug\STDAFX.OBJ

!ELSEIF  "$(CFG)" == "Win32 Release"

.\WinRel\CNTLITEM.OBJ :  $(SOURCE)  $(DEP_CNTLI) $(INTDIR) .\WinRel\STDAFX.OBJ

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\INCTLDLG.CPP
DEP_INCTL=\
	\viewhtml\CROSS_P\INC\CROSS_P.H\
	.\INC\viewhtml.h\
	.\INC\INCTLDLG.H\
	.\INC\STDAFX.H\
	.\INC\bookmark.h

!IF  "$(CFG)" == "Win32 Debug"

.\WinDebug\INCTLDLG.OBJ :  $(SOURCE)  $(DEP_INCTL) $(INTDIR)\
 .\WinDebug\STDAFX.OBJ

!ELSEIF  "$(CFG)" == "Win32 Release"

.\WinRel\INCTLDLG.OBJ :  $(SOURCE)  $(DEP_INCTL) $(INTDIR) .\WinRel\STDAFX.OBJ

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\DMEMFILE.CPP
DEP_DMEMF=\
	\viewhtml\CROSS_P\INC\CROSS_P.H\
	.\INC\DMEMFILE.H\
	.\INC\STDAFX.H\
	\viewhtml\CROSS_P\INC\DYNLOAD.H\
	\viewhtml\CROSS_P\INC\MIMELOAD.H\
	\viewhtml\CROSS_P\INC\BIGSTR.H\
	.\INC\mutex.h

!IF  "$(CFG)" == "Win32 Debug"

.\WinDebug\DMEMFILE.OBJ :  $(SOURCE)  $(DEP_DMEMF) $(INTDIR)\
 .\WinDebug\STDAFX.OBJ

!ELSEIF  "$(CFG)" == "Win32 Release"

.\WinRel\DMEMFILE.OBJ :  $(SOURCE)  $(DEP_DMEMF) $(INTDIR) .\WinRel\STDAFX.OBJ

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\DOPENURL.CPP
DEP_DOPEN=\
	\viewhtml\CROSS_P\INC\CROSS_P.H\
	.\INC\viewhtml.h\
	.\INC\DOPENURL.H\
	.\INC\STDAFX.H\
	.\INC\bookmark.h

!IF  "$(CFG)" == "Win32 Debug"

.\WinDebug\DOPENURL.OBJ :  $(SOURCE)  $(DEP_DOPEN) $(INTDIR)\
 .\WinDebug\STDAFX.OBJ

!ELSEIF  "$(CFG)" == "Win32 Release"

.\WinRel\DOPENURL.OBJ :  $(SOURCE)  $(DEP_DOPEN) $(INTDIR) .\WinRel\STDAFX.OBJ

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\BITBLT.CPP
DEP_BITBL=\
	\viewhtml\CROSS_P\INC\CROSS_P.H\
	\viewhtml\CROSS_P\INC\FMTHTML.H\
	\viewhtml\CROSS_P\INC\PICTURE.H\
	.\INC\BITBLT.H\
	.\INC\STDAFX.H\
	\viewhtml\CROSS_P\INC\READHTML.H\
	.\INC\DMEMFILE.H\
	\viewhtml\CROSS_P\INC\MIMELOAD.H\
	\viewhtml\CROSS_P\INC\CONTAIN.H\
	\viewhtml\CROSS_P\INC\BIGSTR.H\
	\viewhtml\CROSS_P\INC\DYNLOAD.H\
	.\INC\mutex.h

!IF  "$(CFG)" == "Win32 Debug"

.\WinDebug\BITBLT.OBJ :  $(SOURCE)  $(DEP_BITBL) $(INTDIR)\
 .\WinDebug\STDAFX.OBJ

!ELSEIF  "$(CFG)" == "Win32 Release"

.\WinRel\BITBLT.OBJ :  $(SOURCE)  $(DEP_BITBL) $(INTDIR) .\WinRel\STDAFX.OBJ

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\DDSKFILE.CPP
DEP_DDSKF=\
	\viewhtml\CROSS_P\INC\CROSS_P.H\
	.\INC\DDSKFILE.H\
	.\INC\viewhtml.h\
	.\INC\STDAFX.H\
	\viewhtml\CROSS_P\INC\DYNLOAD.H\
	.\INC\bookmark.h\
	.\INC\mutex.h

!IF  "$(CFG)" == "Win32 Debug"

.\WinDebug\DDSKFILE.OBJ :  $(SOURCE)  $(DEP_DDSKF) $(INTDIR)\
 .\WinDebug\STDAFX.OBJ

!ELSEIF  "$(CFG)" == "Win32 Release"

.\WinRel\DDSKFILE.OBJ :  $(SOURCE)  $(DEP_DDSKF) $(INTDIR) .\WinRel\STDAFX.OBJ

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\protocol.cpp
DEP_PROTO=\
	\viewhtml\CROSS_P\INC\CROSS_P.H\
	.\INC\viewhtml.h\
	\viewhtml\CROSS_P\INC\DYNLOAD.H\
	.\INC\protocol.h\
	.\INC\STDAFX.H\
	.\INC\bookmark.h\
	.\INC\mutex.h\
	.\INC\tsocket.h

!IF  "$(CFG)" == "Win32 Debug"

.\WinDebug\protocol.obj :  $(SOURCE)  $(DEP_PROTO) $(INTDIR)\
 .\WinDebug\STDAFX.OBJ

!ELSEIF  "$(CFG)" == "Win32 Release"

.\WinRel\protocol.obj :  $(SOURCE)  $(DEP_PROTO) $(INTDIR) .\WinRel\STDAFX.OBJ

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\bookmark.cpp
DEP_BOOKM=\
	\viewhtml\CROSS_P\INC\CROSS_P.H\
	.\INC\bookmark.h\
	.\INC\STDAFX.H

!IF  "$(CFG)" == "Win32 Debug"

.\WinDebug\bookmark.obj :  $(SOURCE)  $(DEP_BOOKM) $(INTDIR)\
 .\WinDebug\STDAFX.OBJ

!ELSEIF  "$(CFG)" == "Win32 Release"

.\WinRel\bookmark.obj :  $(SOURCE)  $(DEP_BOOKM) $(INTDIR) .\WinRel\STDAFX.OBJ

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\MUTEX.CPP
DEP_MUTEX=\
	\viewhtml\CROSS_P\INC\CROSS_P.H\
	.\INC\mutex.h\
	.\INC\STDAFX.H

!IF  "$(CFG)" == "Win32 Debug"

.\WinDebug\MUTEX.OBJ :  $(SOURCE)  $(DEP_MUTEX) $(INTDIR) .\WinDebug\STDAFX.OBJ

!ELSEIF  "$(CFG)" == "Win32 Release"

.\WinRel\MUTEX.OBJ :  $(SOURCE)  $(DEP_MUTEX) $(INTDIR) .\WinRel\STDAFX.OBJ

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\dlgpref.cpp
DEP_DLGPR=\
	\viewhtml\CROSS_P\INC\CROSS_P.H\
	.\INC\viewhtml.h\
	.\INC\dlgpref.h\
	.\INC\STDAFX.H\
	.\INC\bookmark.h

!IF  "$(CFG)" == "Win32 Debug"

.\WinDebug\dlgpref.obj :  $(SOURCE)  $(DEP_DLGPR) $(INTDIR)\
 .\WinDebug\STDAFX.OBJ

!ELSEIF  "$(CFG)" == "Win32 Release"

.\WinRel\dlgpref.obj :  $(SOURCE)  $(DEP_DLGPR) $(INTDIR) .\WinRel\STDAFX.OBJ

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\tsocket.cpp
DEP_TSOCK=\
	\viewhtml\CROSS_P\INC\CROSS_P.H\
	.\INC\tsocket.h\
	.\INC\STDAFX.H\
	.\INC\mutex.h

!IF  "$(CFG)" == "Win32 Debug"

.\WinDebug\tsocket.obj :  $(SOURCE)  $(DEP_TSOCK) $(INTDIR)\
 .\WinDebug\STDAFX.OBJ

!ELSEIF  "$(CFG)" == "Win32 Release"

.\WinRel\tsocket.obj :  $(SOURCE)  $(DEP_TSOCK) $(INTDIR) .\WinRel\STDAFX.OBJ

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\smtp.cpp
DEP_SMTP_=\
	\viewhtml\CROSS_P\INC\CROSS_P.H\
	.\INC\viewhtml.h\
	.\INC\viewhdoc.h\
	.\INC\smtp.h\
	.\INC\STDAFX.H\
	.\INC\bookmark.h\
	.\INC\CNTLINFO.H\
	\viewhtml\CROSS_P\INC\FMTHTML.H\
	\viewhtml\CROSS_P\INC\MIMELOAD.H\
	.\INC\protocol.h\
	\viewhtml\CROSS_P\INC\READHTML.H\
	.\INC\DMEMFILE.H\
	\viewhtml\CROSS_P\INC\DYNLOAD.H\
	\viewhtml\CROSS_P\INC\BIGSTR.H\
	.\INC\tsocket.h\
	\viewhtml\CROSS_P\INC\CONTAIN.H\
	.\INC\mutex.h

!IF  "$(CFG)" == "Win32 Debug"

.\WinDebug\smtp.obj :  $(SOURCE)  $(DEP_SMTP_) $(INTDIR) .\WinDebug\STDAFX.OBJ

!ELSEIF  "$(CFG)" == "Win32 Release"

.\WinRel\smtp.obj :  $(SOURCE)  $(DEP_SMTP_) $(INTDIR) .\WinRel\STDAFX.OBJ

!ENDIF 

# End Source File
# End Group
################################################################################
# Begin Group "Cross Platform Code"

################################################################################
# Begin Source File

SOURCE=\viewhtml\CROSS_P\PICTURE.CPP
DEP_PICTU=\
	\viewhtml\CROSS_P\INC\CROSS_P.H\
	.\INC\BITBLT.H\
	\viewhtml\CROSS_P\INC\FMTHTML.H\
	\viewhtml\CROSS_P\INC\PICTURE.H\
	.\INC\STDAFX.H\
	\viewhtml\CROSS_P\INC\READHTML.H\
	.\INC\DMEMFILE.H\
	\viewhtml\CROSS_P\INC\MIMELOAD.H\
	\viewhtml\CROSS_P\INC\CONTAIN.H\
	\viewhtml\CROSS_P\INC\BIGSTR.H\
	\viewhtml\CROSS_P\INC\DYNLOAD.H\
	.\INC\mutex.h

!IF  "$(CFG)" == "Win32 Debug"

.\WinDebug\PICTURE.OBJ :  $(SOURCE)  $(DEP_PICTU) $(INTDIR)\
 .\WinDebug\STDAFX.OBJ
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Release"

.\WinRel\PICTURE.OBJ :  $(SOURCE)  $(DEP_PICTU) $(INTDIR) .\WinRel\STDAFX.OBJ
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\viewhtml\CROSS_P\FMTHTML.CPP
DEP_FMTHT=\
	\viewhtml\CROSS_P\INC\CROSS_P.H\
	\viewhtml\CROSS_P\INC\FMTHTML.H\
	.\INC\STDAFX.H\
	\viewhtml\CROSS_P\INC\READHTML.H\
	.\INC\DMEMFILE.H\
	\viewhtml\CROSS_P\INC\CONTAIN.H\
	\viewhtml\CROSS_P\INC\BIGSTR.H\
	\viewhtml\CROSS_P\INC\MIMELOAD.H\
	\viewhtml\CROSS_P\INC\DYNLOAD.H\
	.\INC\mutex.h

!IF  "$(CFG)" == "Win32 Debug"

.\WinDebug\FMTHTML.OBJ :  $(SOURCE)  $(DEP_FMTHT) $(INTDIR)\
 .\WinDebug\STDAFX.OBJ
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Release"

.\WinRel\FMTHTML.OBJ :  $(SOURCE)  $(DEP_FMTHT) $(INTDIR) .\WinRel\STDAFX.OBJ
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\viewhtml\CROSS_P\BIGSTR.CPP
DEP_BIGST=\
	\viewhtml\CROSS_P\INC\CROSS_P.H\
	\viewhtml\CROSS_P\INC\BIGSTR.H\
	.\INC\mutex.h\
	.\INC\STDAFX.H

!IF  "$(CFG)" == "Win32 Debug"

.\WinDebug\BIGSTR.OBJ :  $(SOURCE)  $(DEP_BIGST) $(INTDIR)\
 .\WinDebug\STDAFX.OBJ
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Release"

.\WinRel\BIGSTR.OBJ :  $(SOURCE)  $(DEP_BIGST) $(INTDIR) .\WinRel\STDAFX.OBJ
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\viewhtml\CROSS_P\CONTAIN.CPP
DEP_CONTA=\
	\viewhtml\CROSS_P\INC\CROSS_P.H\
	\viewhtml\CROSS_P\INC\CONTAIN.H\
	.\INC\STDAFX.H

!IF  "$(CFG)" == "Win32 Debug"

.\WinDebug\CONTAIN.OBJ :  $(SOURCE)  $(DEP_CONTA) $(INTDIR)\
 .\WinDebug\STDAFX.OBJ
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Release"

.\WinRel\CONTAIN.OBJ :  $(SOURCE)  $(DEP_CONTA) $(INTDIR) .\WinRel\STDAFX.OBJ
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\viewhtml\CROSS_P\TBLSIZE.CPP
DEP_TBLSI=\
	\viewhtml\CROSS_P\INC\CROSS_P.H\
	\viewhtml\CROSS_P\INC\FMTHTML.H\
	.\INC\STDAFX.H\
	\viewhtml\CROSS_P\INC\READHTML.H\
	.\INC\DMEMFILE.H\
	\viewhtml\CROSS_P\INC\CONTAIN.H\
	\viewhtml\CROSS_P\INC\BIGSTR.H\
	\viewhtml\CROSS_P\INC\MIMELOAD.H\
	\viewhtml\CROSS_P\INC\DYNLOAD.H\
	.\INC\mutex.h

!IF  "$(CFG)" == "Win32 Debug"

.\WinDebug\TBLSIZE.OBJ :  $(SOURCE)  $(DEP_TBLSI) $(INTDIR)\
 .\WinDebug\STDAFX.OBJ
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Release"

.\WinRel\TBLSIZE.OBJ :  $(SOURCE)  $(DEP_TBLSI) $(INTDIR) .\WinRel\STDAFX.OBJ
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\viewhtml\CROSS_P\MIMELOAD.CPP
DEP_MIMEL=\
	\viewhtml\CROSS_P\INC\CROSS_P.H\
	\viewhtml\CROSS_P\INC\DYNLOAD.H\
	\viewhtml\CROSS_P\INC\MIMELOAD.H\
	\viewhtml\CROSS_P\INC\READGIF.H\
	\viewhtml\CROSS_P\INC\READJPEG.H\
	\viewhtml\CROSS_P\INC\READHTML.H\
	.\INC\DMEMFILE.H\
	.\INC\smtp.h\
	.\INC\STDAFX.H\
	.\INC\mutex.h\
	\viewhtml\CROSS_P\INC\BIGSTR.H\
	\viewhtml\CROSS_P\INC\PICTURE.H\
	\viewhtml\CROSS_P\INC\CONTAIN.H\
	.\INC\protocol.h\
	.\INC\tsocket.h

!IF  "$(CFG)" == "Win32 Debug"

.\WinDebug\MIMELOAD.OBJ :  $(SOURCE)  $(DEP_MIMEL) $(INTDIR)\
 .\WinDebug\STDAFX.OBJ
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Release"

.\WinRel\MIMELOAD.OBJ :  $(SOURCE)  $(DEP_MIMEL) $(INTDIR) .\WinRel\STDAFX.OBJ
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\viewhtml\CROSS_P\READGIF.CPP
DEP_READG=\
	\viewhtml\CROSS_P\INC\CROSS_P.H\
	.\INC\BITBLT.H\
	\viewhtml\CROSS_P\INC\FMTHTML.H\
	\viewhtml\CROSS_P\INC\READGIF.H\
	.\INC\STDAFX.H\
	\viewhtml\CROSS_P\INC\READHTML.H\
	.\INC\DMEMFILE.H\
	\viewhtml\CROSS_P\INC\PICTURE.H\
	\viewhtml\CROSS_P\INC\CONTAIN.H\
	\viewhtml\CROSS_P\INC\BIGSTR.H\
	\viewhtml\CROSS_P\INC\MIMELOAD.H\
	\viewhtml\CROSS_P\INC\DYNLOAD.H\
	.\INC\mutex.h

!IF  "$(CFG)" == "Win32 Debug"

.\WinDebug\READGIF.OBJ :  $(SOURCE)  $(DEP_READG) $(INTDIR)\
 .\WinDebug\STDAFX.OBJ
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Release"

.\WinRel\READGIF.OBJ :  $(SOURCE)  $(DEP_READG) $(INTDIR) .\WinRel\STDAFX.OBJ
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\viewhtml\CROSS_P\READJPEG.CPP
DEP_READJ=\
	\viewhtml\CROSS_P\INC\CROSS_P.H\
	\viewhtml\CROSS_P\INC\READJPEG.H\
	.\INC\BITBLT.H\
	\viewhtml\jpeg\INC\JPEGLIB.H\
	\viewhtml\jpeg\INC\JERROR.H\
	.\INC\STDAFX.H\
	\viewhtml\CROSS_P\INC\PICTURE.H\
	\viewhtml\jpeg\INC\jconfig.h\
	\viewhtml\jpeg\INC\JMORECFG.H\
	\viewhtml\jpeg\INC\JPEGINT.H\
	\viewhtml\CROSS_P\INC\MIMELOAD.H\
	\viewhtml\CROSS_P\INC\DYNLOAD.H\
	\viewhtml\CROSS_P\INC\BIGSTR.H\
	.\INC\mutex.h

!IF  "$(CFG)" == "Win32 Debug"

.\WinDebug\READJPEG.OBJ :  $(SOURCE)  $(DEP_READJ) $(INTDIR)\
 .\WinDebug\STDAFX.OBJ
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Release"

.\WinRel\READJPEG.OBJ :  $(SOURCE)  $(DEP_READJ) $(INTDIR) .\WinRel\STDAFX.OBJ
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\viewhtml\CROSS_P\DYNLOAD.CPP
DEP_DYNLO=\
	\viewhtml\CROSS_P\INC\CROSS_P.H\
	.\INC\viewhtml.h\
	\viewhtml\CROSS_P\INC\DYNLOAD.H\
	.\INC\protocol.h\
	.\INC\smtp.h\
	.\INC\STDAFX.H\
	.\INC\bookmark.h\
	.\INC\mutex.h\
	.\INC\tsocket.h\
	\viewhtml\CROSS_P\INC\MIMELOAD.H\
	\viewhtml\CROSS_P\INC\BIGSTR.H

!IF  "$(CFG)" == "Win32 Debug"

.\WinDebug\DYNLOAD.OBJ :  $(SOURCE)  $(DEP_DYNLO) $(INTDIR)\
 .\WinDebug\STDAFX.OBJ
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Release"

.\WinRel\DYNLOAD.OBJ :  $(SOURCE)  $(DEP_DYNLO) $(INTDIR) .\WinRel\STDAFX.OBJ
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\viewhtml\CROSS_P\READHTML.CPP
DEP_READH=\
	\viewhtml\CROSS_P\INC\CROSS_P.H\
	\viewhtml\CROSS_P\INC\READHTML.H\
	.\INC\STDAFX.H\
	\viewhtml\CROSS_P\INC\CONTAIN.H\
	\viewhtml\CROSS_P\INC\BIGSTR.H\
	\viewhtml\CROSS_P\INC\MIMELOAD.H\
	\viewhtml\CROSS_P\INC\DYNLOAD.H\
	.\INC\mutex.h

!IF  "$(CFG)" == "Win32 Debug"

.\WinDebug\READHTML.OBJ :  $(SOURCE)  $(DEP_READH) $(INTDIR)\
 .\WinDebug\STDAFX.OBJ
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Release"

.\WinRel\READHTML.OBJ :  $(SOURCE)  $(DEP_READH) $(INTDIR) .\WinRel\STDAFX.OBJ
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ENDIF 

# End Source File
# End Group
################################################################################
# Begin Group "JPEG Source"

################################################################################
# Begin Source File

SOURCE=\viewhtml\jpeg\JDMASTER.cpp
DEP_JDMAS=\
	\viewhtml\CROSS_P\INC\CROSS_P.H\
	\viewhtml\jpeg\INC\JINCLUDE.H\
	\viewhtml\jpeg\INC\JPEGLIB.H\
	.\INC\STDAFX.H\
	\viewhtml\jpeg\INC\jconfig.h\
	D:\MSVC20\INCLUDE\SYS\TYPES.H\
	\viewhtml\jpeg\INC\JMORECFG.H\
	\viewhtml\jpeg\INC\JPEGINT.H\
	\viewhtml\jpeg\INC\JERROR.H

!IF  "$(CFG)" == "Win32 Debug"

.\WinDebug\JDMASTER.obj :  $(SOURCE)  $(DEP_JDMAS) $(INTDIR)\
 .\WinDebug\STDAFX.OBJ
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Release"

.\WinRel\JDMASTER.obj :  $(SOURCE)  $(DEP_JDMAS) $(INTDIR) .\WinRel\STDAFX.OBJ
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\viewhtml\jpeg\JMEMNOBS.cpp
DEP_JMEMN=\
	\viewhtml\CROSS_P\INC\CROSS_P.H\
	\viewhtml\jpeg\INC\JINCLUDE.H\
	\viewhtml\jpeg\INC\JPEGLIB.H\
	\viewhtml\jpeg\INC\JMEMSYS.H\
	.\INC\STDAFX.H\
	\viewhtml\jpeg\INC\jconfig.h\
	D:\MSVC20\INCLUDE\SYS\TYPES.H\
	\viewhtml\jpeg\INC\JMORECFG.H\
	\viewhtml\jpeg\INC\JPEGINT.H\
	\viewhtml\jpeg\INC\JERROR.H

!IF  "$(CFG)" == "Win32 Debug"

.\WinDebug\JMEMNOBS.obj :  $(SOURCE)  $(DEP_JMEMN) $(INTDIR)\
 .\WinDebug\STDAFX.OBJ
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Release"

.\WinRel\JMEMNOBS.obj :  $(SOURCE)  $(DEP_JMEMN) $(INTDIR) .\WinRel\STDAFX.OBJ
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\viewhtml\jpeg\JIDCTFST.cpp
DEP_JIDCT=\
	\viewhtml\CROSS_P\INC\CROSS_P.H\
	\viewhtml\jpeg\INC\JINCLUDE.H\
	\viewhtml\jpeg\INC\JPEGLIB.H\
	\viewhtml\jpeg\INC\JDCT.H\
	.\INC\STDAFX.H\
	\viewhtml\jpeg\INC\jconfig.h\
	D:\MSVC20\INCLUDE\SYS\TYPES.H\
	\viewhtml\jpeg\INC\JMORECFG.H\
	\viewhtml\jpeg\INC\JPEGINT.H\
	\viewhtml\jpeg\INC\JERROR.H

!IF  "$(CFG)" == "Win32 Debug"

.\WinDebug\JIDCTFST.obj :  $(SOURCE)  $(DEP_JIDCT) $(INTDIR)\
 .\WinDebug\STDAFX.OBJ
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Release"

.\WinRel\JIDCTFST.obj :  $(SOURCE)  $(DEP_JIDCT) $(INTDIR) .\WinRel\STDAFX.OBJ
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\viewhtml\jpeg\JUTILS.cpp
DEP_JUTIL=\
	\viewhtml\CROSS_P\INC\CROSS_P.H\
	\viewhtml\jpeg\INC\JINCLUDE.H\
	\viewhtml\jpeg\INC\JPEGLIB.H\
	.\INC\STDAFX.H\
	\viewhtml\jpeg\INC\jconfig.h\
	D:\MSVC20\INCLUDE\SYS\TYPES.H\
	\viewhtml\jpeg\INC\JMORECFG.H\
	\viewhtml\jpeg\INC\JPEGINT.H\
	\viewhtml\jpeg\INC\JERROR.H

!IF  "$(CFG)" == "Win32 Debug"

.\WinDebug\JUTILS.obj :  $(SOURCE)  $(DEP_JUTIL) $(INTDIR)\
 .\WinDebug\STDAFX.OBJ
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Release"

.\WinRel\JUTILS.obj :  $(SOURCE)  $(DEP_JUTIL) $(INTDIR) .\WinRel\STDAFX.OBJ
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\viewhtml\jpeg\JMEMMGR.cpp
DEP_JMEMM=\
	\viewhtml\CROSS_P\INC\CROSS_P.H\
	\viewhtml\jpeg\INC\JINCLUDE.H\
	\viewhtml\jpeg\INC\JPEGLIB.H\
	\viewhtml\jpeg\INC\JMEMSYS.H\
	.\INC\STDAFX.H\
	\viewhtml\jpeg\INC\jconfig.h\
	D:\MSVC20\INCLUDE\SYS\TYPES.H\
	\viewhtml\jpeg\INC\JMORECFG.H\
	\viewhtml\jpeg\INC\JPEGINT.H\
	\viewhtml\jpeg\INC\JERROR.H

!IF  "$(CFG)" == "Win32 Debug"

.\WinDebug\JMEMMGR.obj :  $(SOURCE)  $(DEP_JMEMM) $(INTDIR)\
 .\WinDebug\STDAFX.OBJ
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Release"

.\WinRel\JMEMMGR.obj :  $(SOURCE)  $(DEP_JMEMM) $(INTDIR) .\WinRel\STDAFX.OBJ
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\viewhtml\jpeg\JDAPI.cpp
DEP_JDAPI=\
	\viewhtml\CROSS_P\INC\CROSS_P.H\
	\viewhtml\jpeg\INC\JINCLUDE.H\
	\viewhtml\jpeg\INC\JPEGLIB.H\
	.\INC\STDAFX.H\
	\viewhtml\jpeg\INC\jconfig.h\
	D:\MSVC20\INCLUDE\SYS\TYPES.H\
	\viewhtml\jpeg\INC\JMORECFG.H\
	\viewhtml\jpeg\INC\JPEGINT.H\
	\viewhtml\jpeg\INC\JERROR.H

!IF  "$(CFG)" == "Win32 Debug"

.\WinDebug\JDAPI.obj :  $(SOURCE)  $(DEP_JDAPI) $(INTDIR) .\WinDebug\STDAFX.OBJ
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Release"

.\WinRel\JDAPI.obj :  $(SOURCE)  $(DEP_JDAPI) $(INTDIR) .\WinRel\STDAFX.OBJ
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\viewhtml\jpeg\JDDCTMGR.cpp
DEP_JDDCT=\
	\viewhtml\CROSS_P\INC\CROSS_P.H\
	\viewhtml\jpeg\INC\JINCLUDE.H\
	\viewhtml\jpeg\INC\JPEGLIB.H\
	\viewhtml\jpeg\INC\JDCT.H\
	.\INC\STDAFX.H\
	\viewhtml\jpeg\INC\jconfig.h\
	D:\MSVC20\INCLUDE\SYS\TYPES.H\
	\viewhtml\jpeg\INC\JMORECFG.H\
	\viewhtml\jpeg\INC\JPEGINT.H\
	\viewhtml\jpeg\INC\JERROR.H

!IF  "$(CFG)" == "Win32 Debug"

.\WinDebug\JDDCTMGR.obj :  $(SOURCE)  $(DEP_JDDCT) $(INTDIR)\
 .\WinDebug\STDAFX.OBJ
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Release"

.\WinRel\JDDCTMGR.obj :  $(SOURCE)  $(DEP_JDDCT) $(INTDIR) .\WinRel\STDAFX.OBJ
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\viewhtml\jpeg\JQUANT1.cpp
DEP_JQUAN=\
	\viewhtml\CROSS_P\INC\CROSS_P.H\
	\viewhtml\jpeg\INC\JINCLUDE.H\
	\viewhtml\jpeg\INC\JPEGLIB.H\
	.\INC\STDAFX.H\
	\viewhtml\jpeg\INC\jconfig.h\
	D:\MSVC20\INCLUDE\SYS\TYPES.H\
	\viewhtml\jpeg\INC\JMORECFG.H\
	\viewhtml\jpeg\INC\JPEGINT.H\
	\viewhtml\jpeg\INC\JERROR.H

!IF  "$(CFG)" == "Win32 Debug"

.\WinDebug\JQUANT1.obj :  $(SOURCE)  $(DEP_JQUAN) $(INTDIR)\
 .\WinDebug\STDAFX.OBJ
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Release"

.\WinRel\JQUANT1.obj :  $(SOURCE)  $(DEP_JQUAN) $(INTDIR) .\WinRel\STDAFX.OBJ
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\viewhtml\jpeg\JDHUFF.cpp
DEP_JDHUF=\
	\viewhtml\CROSS_P\INC\CROSS_P.H\
	\viewhtml\jpeg\INC\JINCLUDE.H\
	\viewhtml\jpeg\INC\JPEGLIB.H\
	.\INC\STDAFX.H\
	\viewhtml\jpeg\INC\jconfig.h\
	D:\MSVC20\INCLUDE\SYS\TYPES.H\
	\viewhtml\jpeg\INC\JMORECFG.H\
	\viewhtml\jpeg\INC\JPEGINT.H\
	\viewhtml\jpeg\INC\JERROR.H

!IF  "$(CFG)" == "Win32 Debug"

.\WinDebug\JDHUFF.obj :  $(SOURCE)  $(DEP_JDHUF) $(INTDIR)\
 .\WinDebug\STDAFX.OBJ
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Release"

.\WinRel\JDHUFF.obj :  $(SOURCE)  $(DEP_JDHUF) $(INTDIR) .\WinRel\STDAFX.OBJ
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\viewhtml\jpeg\JDMAINCT.cpp
DEP_JDMAI=\
	\viewhtml\CROSS_P\INC\CROSS_P.H\
	\viewhtml\jpeg\INC\JINCLUDE.H\
	\viewhtml\jpeg\INC\JPEGLIB.H\
	.\INC\STDAFX.H\
	\viewhtml\jpeg\INC\jconfig.h\
	D:\MSVC20\INCLUDE\SYS\TYPES.H\
	\viewhtml\jpeg\INC\JMORECFG.H\
	\viewhtml\jpeg\INC\JPEGINT.H\
	\viewhtml\jpeg\INC\JERROR.H

!IF  "$(CFG)" == "Win32 Debug"

.\WinDebug\JDMAINCT.obj :  $(SOURCE)  $(DEP_JDMAI) $(INTDIR)\
 .\WinDebug\STDAFX.OBJ
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Release"

.\WinRel\JDMAINCT.obj :  $(SOURCE)  $(DEP_JDMAI) $(INTDIR) .\WinRel\STDAFX.OBJ
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\viewhtml\jpeg\JDSAMPLE.cpp
DEP_JDSAM=\
	\viewhtml\CROSS_P\INC\CROSS_P.H\
	\viewhtml\jpeg\INC\JINCLUDE.H\
	\viewhtml\jpeg\INC\JPEGLIB.H\
	.\INC\STDAFX.H\
	\viewhtml\jpeg\INC\jconfig.h\
	D:\MSVC20\INCLUDE\SYS\TYPES.H\
	\viewhtml\jpeg\INC\JMORECFG.H\
	\viewhtml\jpeg\INC\JPEGINT.H\
	\viewhtml\jpeg\INC\JERROR.H

!IF  "$(CFG)" == "Win32 Debug"

.\WinDebug\JDSAMPLE.obj :  $(SOURCE)  $(DEP_JDSAM) $(INTDIR)\
 .\WinDebug\STDAFX.OBJ
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Release"

.\WinRel\JDSAMPLE.obj :  $(SOURCE)  $(DEP_JDSAM) $(INTDIR) .\WinRel\STDAFX.OBJ
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\viewhtml\jpeg\JERROR.cpp
DEP_JERRO=\
	\viewhtml\CROSS_P\INC\CROSS_P.H\
	\viewhtml\jpeg\INC\JINCLUDE.H\
	\viewhtml\jpeg\INC\JPEGLIB.H\
	\viewhtml\jpeg\INC\JVERSION.H\
	\viewhtml\jpeg\INC\JERROR.H\
	.\INC\STDAFX.H\
	\viewhtml\jpeg\INC\jconfig.h\
	D:\MSVC20\INCLUDE\SYS\TYPES.H\
	\viewhtml\jpeg\INC\JMORECFG.H\
	\viewhtml\jpeg\INC\JPEGINT.H

!IF  "$(CFG)" == "Win32 Debug"

.\WinDebug\JERROR.obj :  $(SOURCE)  $(DEP_JERRO) $(INTDIR)\
 .\WinDebug\STDAFX.OBJ
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Release"

.\WinRel\JERROR.obj :  $(SOURCE)  $(DEP_JERRO) $(INTDIR) .\WinRel\STDAFX.OBJ
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\viewhtml\jpeg\JDCOEFCT.cpp
DEP_JDCOE=\
	\viewhtml\CROSS_P\INC\CROSS_P.H\
	\viewhtml\jpeg\INC\JINCLUDE.H\
	\viewhtml\jpeg\INC\JPEGLIB.H\
	.\INC\STDAFX.H\
	\viewhtml\jpeg\INC\jconfig.h\
	D:\MSVC20\INCLUDE\SYS\TYPES.H\
	\viewhtml\jpeg\INC\JMORECFG.H\
	\viewhtml\jpeg\INC\JPEGINT.H\
	\viewhtml\jpeg\INC\JERROR.H

!IF  "$(CFG)" == "Win32 Debug"

.\WinDebug\JDCOEFCT.obj :  $(SOURCE)  $(DEP_JDCOE) $(INTDIR)\
 .\WinDebug\STDAFX.OBJ
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Release"

.\WinRel\JDCOEFCT.obj :  $(SOURCE)  $(DEP_JDCOE) $(INTDIR) .\WinRel\STDAFX.OBJ
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\viewhtml\jpeg\JDPOSTCT.cpp
DEP_JDPOS=\
	\viewhtml\CROSS_P\INC\CROSS_P.H\
	\viewhtml\jpeg\INC\JINCLUDE.H\
	\viewhtml\jpeg\INC\JPEGLIB.H\
	.\INC\STDAFX.H\
	\viewhtml\jpeg\INC\jconfig.h\
	D:\MSVC20\INCLUDE\SYS\TYPES.H\
	\viewhtml\jpeg\INC\JMORECFG.H\
	\viewhtml\jpeg\INC\JPEGINT.H\
	\viewhtml\jpeg\INC\JERROR.H

!IF  "$(CFG)" == "Win32 Debug"

.\WinDebug\JDPOSTCT.obj :  $(SOURCE)  $(DEP_JDPOS) $(INTDIR)\
 .\WinDebug\STDAFX.OBJ
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Release"

.\WinRel\JDPOSTCT.obj :  $(SOURCE)  $(DEP_JDPOS) $(INTDIR) .\WinRel\STDAFX.OBJ
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\viewhtml\jpeg\JDMARKER.cpp
DEP_JDMAR=\
	\viewhtml\CROSS_P\INC\CROSS_P.H\
	\viewhtml\jpeg\INC\JINCLUDE.H\
	\viewhtml\jpeg\INC\JPEGLIB.H\
	.\INC\STDAFX.H\
	\viewhtml\jpeg\INC\jconfig.h\
	D:\MSVC20\INCLUDE\SYS\TYPES.H\
	\viewhtml\jpeg\INC\JMORECFG.H\
	\viewhtml\jpeg\INC\JPEGINT.H\
	\viewhtml\jpeg\INC\JERROR.H

!IF  "$(CFG)" == "Win32 Debug"

.\WinDebug\JDMARKER.obj :  $(SOURCE)  $(DEP_JDMAR) $(INTDIR)\
 .\WinDebug\STDAFX.OBJ
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Release"

.\WinRel\JDMARKER.obj :  $(SOURCE)  $(DEP_JDMAR) $(INTDIR) .\WinRel\STDAFX.OBJ
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\viewhtml\jpeg\JDMERGE.cpp
DEP_JDMER=\
	\viewhtml\CROSS_P\INC\CROSS_P.H\
	\viewhtml\jpeg\INC\JINCLUDE.H\
	\viewhtml\jpeg\INC\JPEGLIB.H\
	.\INC\STDAFX.H\
	\viewhtml\jpeg\INC\jconfig.h\
	D:\MSVC20\INCLUDE\SYS\TYPES.H\
	\viewhtml\jpeg\INC\JMORECFG.H\
	\viewhtml\jpeg\INC\JPEGINT.H\
	\viewhtml\jpeg\INC\JERROR.H

!IF  "$(CFG)" == "Win32 Debug"

.\WinDebug\JDMERGE.obj :  $(SOURCE)  $(DEP_JDMER) $(INTDIR)\
 .\WinDebug\STDAFX.OBJ
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Release"

.\WinRel\JDMERGE.obj :  $(SOURCE)  $(DEP_JDMER) $(INTDIR) .\WinRel\STDAFX.OBJ
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\viewhtml\jpeg\JDCOLOR.cpp
DEP_JDCOL=\
	\viewhtml\CROSS_P\INC\CROSS_P.H\
	\viewhtml\jpeg\INC\JINCLUDE.H\
	\viewhtml\jpeg\INC\JPEGLIB.H\
	.\INC\STDAFX.H\
	\viewhtml\jpeg\INC\jconfig.h\
	D:\MSVC20\INCLUDE\SYS\TYPES.H\
	\viewhtml\jpeg\INC\JMORECFG.H\
	\viewhtml\jpeg\INC\JPEGINT.H\
	\viewhtml\jpeg\INC\JERROR.H

!IF  "$(CFG)" == "Win32 Debug"

.\WinDebug\JDCOLOR.obj :  $(SOURCE)  $(DEP_JDCOL) $(INTDIR)\
 .\WinDebug\STDAFX.OBJ
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Release"

.\WinRel\JDCOLOR.obj :  $(SOURCE)  $(DEP_JDCOL) $(INTDIR) .\WinRel\STDAFX.OBJ
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\viewhtml\jpeg\JCOMAPI.cpp
DEP_JCOMA=\
	\viewhtml\CROSS_P\INC\CROSS_P.H\
	\viewhtml\jpeg\INC\JINCLUDE.H\
	\viewhtml\jpeg\INC\JPEGLIB.H\
	.\INC\STDAFX.H\
	\viewhtml\jpeg\INC\jconfig.h\
	D:\MSVC20\INCLUDE\SYS\TYPES.H\
	\viewhtml\jpeg\INC\JMORECFG.H\
	\viewhtml\jpeg\INC\JPEGINT.H\
	\viewhtml\jpeg\INC\JERROR.H

!IF  "$(CFG)" == "Win32 Debug"

.\WinDebug\JCOMAPI.obj :  $(SOURCE)  $(DEP_JCOMA) $(INTDIR)\
 .\WinDebug\STDAFX.OBJ
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ELSEIF  "$(CFG)" == "Win32 Release"

.\WinRel\JCOMAPI.obj :  $(SOURCE)  $(DEP_JCOMA) $(INTDIR) .\WinRel\STDAFX.OBJ
   $(CPP) $(CPP_PROJ)  $(SOURCE) 

!ENDIF 

# End Source File
# End Group
# End Project
################################################################################
