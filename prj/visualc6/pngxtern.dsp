# Microsoft Developer Studio Project File - Name="pngxtern" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=pngxtern - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "pngxtern.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "pngxtern.mak" CFG="pngxtern - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "pngxtern - Win32 LIB Release" (based on "Win32 (x86) Static Library")
!MESSAGE "pngxtern - Win32 LIB Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "pngxtern - Win32 LIB ASM Release" (based on "Win32 (x86) Static Library")
!MESSAGE "pngxtern - Win32 LIB ASM Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "pngxtern - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "pngxtern - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "pngxtern - Win32 LIB Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "pngxtern___Win32_LIB_Release"
# PROP BASE Intermediate_Dir "pngxtern___Win32_LIB_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Win32_LIB_Release"
# PROP Intermediate_Dir "Win32_LIB_Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /O2 /D "WIN32" /D "_CRT_SECURE_NO_WARNINGS" /D "_CRT_NONSTDC_NO_WARNINGS" /D "NDEBUG" /FD /c
# SUBTRACT BASE CPP /YX /Yc /Yu
# ADD CPP /nologo /MD /W3 /O2 /I "..\zlib" /I "..\..\lib\zlib" /I "..\libpng" /I "..\..\lib\libpng" /D "WIN32" /D "_CRT_SECURE_NO_WARNINGS" /D "_CRT_NONSTDC_NO_WARNINGS" /D "NDEBUG" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /i "..\.." /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "pngxtern - Win32 LIB Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "pngxtern___Win32_LIB_Debug"
# PROP BASE Intermediate_Dir "pngxtern___Win32_LIB_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Win32_LIB_Debug"
# PROP Intermediate_Dir "Win32_LIB_Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /ZI /Od /D "WIN32" /D "_CRT_SECURE_NO_WARNINGS" /D "_CRT_NONSTDC_NO_WARNINGS" /D "_DEBUG" /FD /GZ /c
# SUBTRACT BASE CPP /YX /Yc /Yu
# ADD CPP /nologo /MDd /W3 /Gm /ZI /Od /I "..\zlib" /I "..\..\lib\zlib" /I "..\libpng" /I "..\..\lib\libpng" /D "WIN32" /D "_CRT_SECURE_NO_WARNINGS" /D "_CRT_NONSTDC_NO_WARNINGS" /D "_DEBUG" /D "DEBUG" /D PNG_DEBUG=1 /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Win32_LIB_Debug\pngxternd.lib"

!ELSEIF  "$(CFG)" == "pngxtern - Win32 LIB ASM Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "pngxtern___Win32_LIB_ASM_Release"
# PROP BASE Intermediate_Dir "pngxtern___Win32_LIB_ASM_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Win32_LIB_ASM_Release"
# PROP Intermediate_Dir "Win32_LIB_ASM_Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /O2 /D "WIN32" /D "_CRT_SECURE_NO_WARNINGS" /D "_CRT_NONSTDC_NO_WARNINGS" /D "NDEBUG" /FD /c
# SUBTRACT BASE CPP /YX /Yc /Yu
# ADD CPP /nologo /MD /W3 /O2 /I "..\zlib" /I "..\..\lib\zlib" /I "..\libpng" /I "..\..\lib\libpng" /D "WIN32" /D "_CRT_SECURE_NO_WARNINGS" /D "_CRT_NONSTDC_NO_WARNINGS" /D "NDEBUG" /D "PNG_USE_PNGVCRD" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /i "..\.." /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "pngxtern - Win32 LIB ASM Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "pngxtern___Win32_LIB_ASM_Debug"
# PROP BASE Intermediate_Dir "pngxtern___Win32_LIB_ASM_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Win32_LIB_ASM_Debug"
# PROP Intermediate_Dir "Win32_LIB_ASM_Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /ZI /Od /D "WIN32" /D "_CRT_SECURE_NO_WARNINGS" /D "_CRT_NONSTDC_NO_WARNINGS" /D "_DEBUG" /FD /GZ /c
# SUBTRACT BASE CPP /YX /Yc /Yu
# ADD CPP /nologo /MDd /W3 /Gm /ZI /Od /I "..\zlib" /I "..\..\lib\zlib" /I "..\libpng" /I "..\..\lib\libpng" /D "WIN32" /D "_CRT_SECURE_NO_WARNINGS" /D "_CRT_NONSTDC_NO_WARNINGS" /D "_DEBUG" /D "DEBUG" /D PNG_DEBUG=1 /D "PNG_USE_PNGVCRD" /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Win32_LIB_ASM_Debug\pngxternd.lib"

!ELSEIF  "$(CFG)" == "pngxtern - Win32 Release"

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
# ADD BASE CPP /nologo /MD /W3 /O2 /I "..\.." /I "..\..\..\zlib" /D "WIN32" /D "_CRT_SECURE_NO_WARNINGS" /D "_CRT_NONSTDC_NO_WARNINGS" /D "NDEBUG" /FD /c
# SUBTRACT BASE CPP /YX /Yc /Yu
# ADD CPP /nologo /MD /W3 /O2 /I "..\zlib" /I "..\..\lib\zlib" /I "..\libpng" /I "..\..\lib\libpng" /D "WIN32" /D "_CRT_SECURE_NO_WARNINGS" /D "_CRT_NONSTDC_NO_WARNINGS" /D "NDEBUG" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x409 /i "..\.." /d "NDEBUG"
# ADD RSC /l 0x409 /i "..\.." /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "pngxtern - Win32 Debug"

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
# ADD BASE CPP /nologo /MDd /W3 /Gm /ZI /Od /I "..\.." /I "..\..\..\zlib" /D "WIN32" /D "_CRT_SECURE_NO_WARNINGS" /D "_CRT_NONSTDC_NO_WARNINGS" /D "_DEBUG" /D "DEBUG" /D PNG_DEBUG=1 /FD /GZ /c
# SUBTRACT BASE CPP /YX /Yc /Yu
# ADD CPP /nologo /MDd /W3 /Gm /ZI /Od /I "..\zlib" /I "..\..\lib\zlib" /I "..\libpng" /I "..\..\lib\libpng" /D "WIN32" /D "_CRT_SECURE_NO_WARNINGS" /D "_CRT_NONSTDC_NO_WARNINGS" /D "_DEBUG" /D "DEBUG" /D PNG_DEBUG=1 /FD /GZ /c
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

# Name "pngxtern - Win32 LIB Release"
# Name "pngxtern - Win32 LIB Debug"
# Name "pngxtern - Win32 LIB ASM Release"
# Name "pngxtern - Win32 LIB ASM Debug"
# Name "pngxtern - Win32 Release"
# Name "pngxtern - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\lib\pngxtern\gif\gifread.c
# End Source File
# Begin Source File

SOURCE=..\..\lib\pngxtern\minitiff\minitiff.c
# End Source File
# Begin Source File

SOURCE=..\..\lib\pngxtern\pngxio.c
# End Source File
# Begin Source File

SOURCE=..\..\lib\pngxtern\pngxmem.c
# End Source File
# Begin Source File

SOURCE=..\..\lib\pngxtern\pngxrbmp.c
# End Source File
# Begin Source File

SOURCE=..\..\lib\pngxtern\pngxread.c
# End Source File
# Begin Source File

SOURCE=..\..\lib\pngxtern\pngxrgif.c
# End Source File
# Begin Source File

SOURCE=..\..\lib\pngxtern\pngxrjpg.c
# End Source File
# Begin Source File

SOURCE=..\..\lib\pngxtern\pngxrpnm.c
# End Source File
# Begin Source File

SOURCE=..\..\lib\pngxtern\pngxrtif.c
# End Source File
# Begin Source File

SOURCE=..\..\lib\pngxtern\pngxset.c
# End Source File
# Begin Source File

SOURCE=..\..\lib\pngxtern\pngxwrite.c
# End Source File
# Begin Source File

SOURCE=..\..\lib\pngxtern\pnm\pnmin.c
# End Source File
# Begin Source File

SOURCE=..\..\lib\pngxtern\pnm\pnmout.c
# End Source File
# Begin Source File

SOURCE=..\..\lib\pngxtern\pnm\pnmutil.c
# End Source File
# Begin Source File

SOURCE=..\..\lib\pngxtern\minitiff\tiffread.c
# End Source File
# Begin Source File

SOURCE=..\..\lib\pngxtern\minitiff\tiffwrite.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\lib\pngxtern\gif\gifread.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\pngxtern\minitiff\minitiff.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\pngxtern\pngx.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\pngxtern\pngxtern.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\pngxtern\pnm\pnmio.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\pngxtern\minitiff\tiffdef.h
# End Source File
# End Group
# End Target
# End Project
