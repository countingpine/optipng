# Microsoft Developer Studio Project File - Name="zlib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=zlib - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "zlib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "zlib.mak" CFG="zlib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "zlib - Win32 LIB Release" (based on "Win32 (x86) Static Library")
!MESSAGE "zlib - Win32 LIB Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "zlib - Win32 LIB ASM Release" (based on "Win32 (x86) Static Library")
!MESSAGE "zlib - Win32 LIB ASM Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "zlib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "zlib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "zlib - Win32 LIB Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "zlib___Win32_LIB_Release"
# PROP BASE Intermediate_Dir "zlib___Win32_LIB_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Win32_LIB_Release"
# PROP Intermediate_Dir "Win32_LIB_Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /O2 /D "WIN32" /D "_CRT_SECURE_NO_WARNINGS" /D "_CRT_NONSTDC_NO_WARNINGS" /D "NDEBUG" /FD /c
# SUBTRACT BASE CPP /YX /Yc /Yu
# ADD CPP /nologo /MD /W3 /O2 /D "WIN32" /D "_CRT_SECURE_NO_WARNINGS" /D "_CRT_NONSTDC_NO_WARNINGS" /D "NDEBUG" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "zlib - Win32 LIB Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "zlib___Win32_LIB_Debug"
# PROP BASE Intermediate_Dir "zlib___Win32_LIB_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Win32_LIB_Debug"
# PROP Intermediate_Dir "Win32_LIB_Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /ZI /Od /D "WIN32" /D "_CRT_SECURE_NO_WARNINGS" /D "_CRT_NONSTDC_NO_WARNINGS" /D "_DEBUG" /FD /GZ /c
# SUBTRACT BASE CPP /YX /Yc /Yu
# ADD CPP /nologo /MDd /W3 /Gm /ZI /Od /D "WIN32" /D "_CRT_SECURE_NO_WARNINGS" /D "_CRT_NONSTDC_NO_WARNINGS" /D "_DEBUG" /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Win32_LIB_Debug\zlibd.lib"

!ELSEIF  "$(CFG)" == "zlib - Win32 LIB ASM Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "zlib___Win32_LIB_ASM_Release"
# PROP BASE Intermediate_Dir "zlib___Win32_LIB_ASM_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Win32_LIB_ASM_Release"
# PROP Intermediate_Dir "Win32_LIB_ASM_Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /O2 /D "WIN32" /D "_CRT_SECURE_NO_WARNINGS" /D "_CRT_NONSTDC_NO_WARNINGS" /D "NDEBUG" /FD /c
# SUBTRACT BASE CPP /YX /Yc /Yu
# ADD CPP /nologo /MD /W3 /O2 /D "WIN32" /D "_CRT_SECURE_NO_WARNINGS" /D "_CRT_NONSTDC_NO_WARNINGS" /D "NDEBUG" /D "ASMV" /D "ASMINF" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "zlib - Win32 LIB ASM Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "zlib___Win32_LIB_ASM_Debug"
# PROP BASE Intermediate_Dir "zlib___Win32_LIB_ASM_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Win32_LIB_ASM_Debug"
# PROP Intermediate_Dir "Win32_LIB_ASM_Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /ZI /Od /D "WIN32" /D "_CRT_SECURE_NO_WARNINGS" /D "_CRT_NONSTDC_NO_WARNINGS" /D "_DEBUG" /FD /GZ /c
# SUBTRACT BASE CPP /YX /Yc /Yu
# ADD CPP /nologo /MDd /W3 /Gm /ZI /Od /D "WIN32" /D "_CRT_SECURE_NO_WARNINGS" /D "_CRT_NONSTDC_NO_WARNINGS" /D "_DEBUG" /D "ASMV" /D "ASMINF" /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Win32_LIB_ASM_Debug\zlibd.lib"

!ELSEIF  "$(CFG)" == "zlib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /O2 /D "WIN32" /D "_CRT_SECURE_NO_WARNINGS" /D "_CRT_NONSTDC_NO_WARNINGS" /D "NDEBUG" /FD /c
# SUBTRACT BASE CPP /YX /Yc /Yu
# ADD CPP /nologo /MD /W3 /O2 /D "WIN32" /D "_CRT_SECURE_NO_WARNINGS" /D "_CRT_NONSTDC_NO_WARNINGS" /D "NDEBUG" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "zlib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /ZI /Od /D "WIN32" /D "_CRT_SECURE_NO_WARNINGS" /D "_CRT_NONSTDC_NO_WARNINGS" /D "_DEBUG" /FD /GZ /c
# SUBTRACT BASE CPP /YX /Yc /Yu
# ADD CPP /nologo /MDd /W3 /Gm /ZI /Od /D "WIN32" /D "_CRT_SECURE_NO_WARNINGS" /D "_CRT_NONSTDC_NO_WARNINGS" /D "_DEBUG" /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "zlib - Win32 LIB Release"
# Name "zlib - Win32 LIB Debug"
# Name "zlib - Win32 LIB ASM Release"
# Name "zlib - Win32 LIB ASM Debug"
# Name "zlib - Win32 Release"
# Name "zlib - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\lib\zlib\adler32.c
# End Source File
# Begin Source File

SOURCE=..\..\lib\zlib\compress.c
# End Source File
# Begin Source File

SOURCE=..\..\lib\zlib\crc32.c
# End Source File
# Begin Source File

SOURCE=..\..\lib\zlib\deflate.c
# End Source File
# Begin Source File

SOURCE=..\..\lib\zlib\gzclose.c
# End Source File
# Begin Source File

SOURCE=..\..\lib\zlib\gzlib.c
# End Source File
# Begin Source File

SOURCE=..\..\lib\zlib\gzread.c
# End Source File
# Begin Source File

SOURCE=..\..\lib\zlib\gzwrite.c
# End Source File
# Begin Source File

SOURCE=..\..\lib\zlib\infback.c
# End Source File
# Begin Source File

SOURCE=..\..\lib\zlib\inffast.c
# End Source File
# Begin Source File

SOURCE=..\..\lib\zlib\inflate.c
# End Source File
# Begin Source File

SOURCE=..\..\lib\zlib\inftrees.c
# End Source File
# Begin Source File

SOURCE=..\..\lib\zlib\trees.c
# End Source File
# Begin Source File

SOURCE=..\..\lib\zlib\uncompr.c
# End Source File
# Begin Source File

SOURCE=..\..\lib\zlib\win32\zlib.def

!IF  "$(CFG)" == "zlib - Win32 LIB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zlib - Win32 LIB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zlib - Win32 LIB ASM Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zlib - Win32 LIB ASM Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zlib - Win32 Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zlib - Win32 Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\lib\zlib\zutil.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\lib\zlib\crc32.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\zlib\deflate.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\zlib\inffast.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\zlib\inffixed.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\zlib\inflate.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\zlib\inftrees.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\zlib\trees.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\zlib\zconf.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\zlib\zlib.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\zlib\zutil.h
# End Source File
# End Group
# Begin Group "Assembler Files (Unsupported)"

# PROP Default_Filter "asm;obj;c;cpp;cxx;h;hpp;hxx"
# Begin Source File

SOURCE=..\..\lib\zlib\contrib\masmx86\gvmat32.asm

!IF  "$(CFG)" == "zlib - Win32 LIB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zlib - Win32 LIB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zlib - Win32 LIB ASM Release"

# Begin Custom Build - Assembling...
IntDir=.\Win32_LIB_ASM_Release
InputPath=..\..\lib\zlib\contrib\masmx86\gvmat32.asm
InputName=gvmat32

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml.exe /nologo /c /coff /Cx /Fo"$(IntDir)\$(InputName).obj" "$(InputPath)"

# End Custom Build

!ELSEIF  "$(CFG)" == "zlib - Win32 LIB ASM Debug"

# Begin Custom Build - Assembling...
IntDir=.\Win32_LIB_ASM_Debug
InputPath=..\..\lib\zlib\contrib\masmx86\gvmat32.asm
InputName=gvmat32

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml.exe /nologo /c /coff /Cx /Zi /Fo"$(IntDir)\$(InputName).obj" "$(InputPath)"

# End Custom Build

!ELSEIF  "$(CFG)" == "zlib - Win32 Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zlib - Win32 Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\lib\zlib\contrib\masmx86\gvmat32c.c

!IF  "$(CFG)" == "zlib - Win32 LIB Release"

# PROP Exclude_From_Build 1
# ADD BASE CPP /I "..\.."
# ADD CPP /I "..\.." /I "..\..\lib\zlib"

!ELSEIF  "$(CFG)" == "zlib - Win32 LIB Debug"

# PROP Exclude_From_Build 1
# ADD BASE CPP /I "..\.."
# ADD CPP /I "..\.." /I "..\..\lib\zlib"

!ELSEIF  "$(CFG)" == "zlib - Win32 LIB ASM Release"

# ADD BASE CPP /I "..\.."
# ADD CPP /I "..\.." /I "..\..\lib\zlib"

!ELSEIF  "$(CFG)" == "zlib - Win32 LIB ASM Debug"

# ADD BASE CPP /I "..\.."
# ADD CPP /I "..\.." /I "..\..\lib\zlib"

!ELSEIF  "$(CFG)" == "zlib - Win32 Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1
# ADD BASE CPP /I "..\.."
# ADD CPP /I "..\.." /I "..\..\lib\zlib"

!ELSEIF  "$(CFG)" == "zlib - Win32 Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1
# ADD BASE CPP /I "..\.."
# ADD CPP /I "..\.." /I "..\..\lib\zlib"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\lib\zlib\contrib\masmx86\inffas32.asm

!IF  "$(CFG)" == "zlib - Win32 LIB Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zlib - Win32 LIB Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zlib - Win32 LIB ASM Release"

# Begin Custom Build - Assembling...
IntDir=.\Win32_LIB_ASM_Release
InputPath=..\..\lib\zlib\contrib\masmx86\inffas32.asm
InputName=inffas32

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml.exe /nologo /c /coff /Cx /Fo"$(IntDir)\$(InputName).obj" "$(InputPath)"

# End Custom Build

!ELSEIF  "$(CFG)" == "zlib - Win32 LIB ASM Debug"

# Begin Custom Build - Assembling...
IntDir=.\Win32_LIB_ASM_Debug
InputPath=..\..\lib\zlib\contrib\masmx86\inffas32.asm
InputName=inffas32

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml.exe /nologo /c /coff /Cx /Zi /Fo"$(IntDir)\$(InputName).obj" "$(InputPath)"

# End Custom Build

!ELSEIF  "$(CFG)" == "zlib - Win32 Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zlib - Win32 Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
