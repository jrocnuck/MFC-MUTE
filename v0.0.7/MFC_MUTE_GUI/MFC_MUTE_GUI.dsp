# Microsoft Developer Studio Project File - Name="MFC_MUTE_GUI" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=MFC_MUTE_GUI - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "MFC_MUTE_GUI.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "MFC_MUTE_GUI.mak" CFG="MFC_MUTE_GUI - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "MFC_MUTE_GUI - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "MFC_MUTE_GUI - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "MFC_MUTE_GUI - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MTd /W3 /GX /Od /I ".\\" /I "..\MFC_MUTECORE_SOURCE\\" /I "..\MFC_MUTECORE_SOURCE\minorGems\\" /I "..\MFC_MUTECORE_SOURCE\minorGems\io\file\win32\\" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "WIN_32" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 ws2_32.lib Shlwapi.lib /nologo /subsystem:windows /incremental:yes /machine:I386
# SUBTRACT LINK32 /profile /map

!ELSEIF  "$(CFG)" == "MFC_MUTE_GUI - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I ".\\" /I "..\MFC_MUTECORE_SOURCE\\" /I "..\MFC_MUTECORE_SOURCE\minorGems\\" /I "..\MFC_MUTECORE_SOURCE\minorGems\io\file\win32\\" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "WIN_32" /FD /GZ /c
# SUBTRACT CPP /Fr /YX /Yc /Yu
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ws2_32.lib Shlwapi.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# SUBTRACT LINK32 /profile

!ENDIF 

# Begin Target

# Name "MFC_MUTE_GUI - Win32 Release"
# Name "MFC_MUTE_GUI - Win32 Debug"
# Begin Group "GUI Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "ResizableLib"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ResizableLib\ResizableDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\ResizableLib\ResizableFormView.cpp
# End Source File
# Begin Source File

SOURCE=.\ResizableLib\ResizableFrame.cpp
# End Source File
# Begin Source File

SOURCE=.\ResizableLib\ResizableGrip.cpp
# End Source File
# Begin Source File

SOURCE=.\ResizableLib\ResizableLayout.cpp
# End Source File
# Begin Source File

SOURCE=.\ResizableLib\ResizableMDIChild.cpp
# End Source File
# Begin Source File

SOURCE=.\ResizableLib\ResizableMDIFrame.cpp
# End Source File
# Begin Source File

SOURCE=.\ResizableLib\ResizableMinMax.cpp
# End Source File
# Begin Source File

SOURCE=.\ResizableLib\ResizablePage.cpp
# End Source File
# Begin Source File

SOURCE=.\ResizableLib\ResizablePageEx.cpp
# End Source File
# Begin Source File

SOURCE=.\ResizableLib\ResizableSheet.cpp
# End Source File
# Begin Source File

SOURCE=.\ResizableLib\ResizableSheetEx.cpp
# End Source File
# Begin Source File

SOURCE=.\ResizableLib\ResizableState.cpp
# End Source File
# End Group
# Begin Group "GUI Controls"

# PROP Default_Filter ""
# Begin Group "CxImage"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CxImage\xfile.h
# End Source File
# Begin Source File

SOURCE=.\CxImage\ximabmp.cpp
# End Source File
# Begin Source File

SOURCE=.\CxImage\ximabmp.h
# End Source File
# Begin Source File

SOURCE=.\CxImage\ximadefs.h
# End Source File
# Begin Source File

SOURCE=.\CxImage\ximage.cpp
# End Source File
# Begin Source File

SOURCE=.\CxImage\ximage.h
# End Source File
# Begin Source File

SOURCE=.\CxImage\ximaiter.h
# End Source File
# Begin Source File

SOURCE=.\CxImage\ximalpha.cpp
# End Source File
# Begin Source File

SOURCE=.\CxImage\ximapal.cpp
# End Source File
# Begin Source File

SOURCE=.\CxImage\ximatran.cpp
# End Source File
# Begin Source File

SOURCE=.\CxImage\ximawbmp.cpp
# End Source File
# Begin Source File

SOURCE=.\CxImage\ximawbmp.h
# End Source File
# Begin Source File

SOURCE=.\CxImage\ximawnd.cpp
# End Source File
# Begin Source File

SOURCE=.\CxImage\xiofile.h
# End Source File
# Begin Source File

SOURCE=.\CxImage\xmemfile.cpp
# End Source File
# Begin Source File

SOURCE=.\CxImage\xmemfile.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\AboutCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\AboutCtrl.h
# End Source File
# Begin Source File

SOURCE=.\BCMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\BCMenu.h
# End Source File
# Begin Source File

SOURCE=.\BtnST.cpp
# End Source File
# Begin Source File

SOURCE=.\BtnST.h
# End Source File
# Begin Source File

SOURCE=.\ComboBoxBold.cpp
# End Source File
# Begin Source File

SOURCE=.\ComboBoxBold.h
# End Source File
# Begin Source File

SOURCE=.\DialogMinTrayBtn.h
# End Source File
# Begin Source File

SOURCE=.\DialogMinTrayBtn.hpp
# End Source File
# Begin Source File

SOURCE=.\ExternString.cpp
# End Source File
# Begin Source File

SOURCE=.\IconStatic.cpp
# End Source File
# Begin Source File

SOURCE=.\IconStatic.h
# End Source File
# Begin Source File

SOURCE=.\Label.cpp
# End Source File
# Begin Source File

SOURCE=.\Label.h
# End Source File
# Begin Source File

SOURCE=.\Led.cpp
# End Source File
# Begin Source File

SOURCE=.\MuleListCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\NumberEdit.cpp
# End Source File
# Begin Source File

SOURCE=.\NumberEdit.h
# End Source File
# Begin Source File

SOURCE=.\TimedDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\TimedDlg.h
# End Source File
# Begin Source File

SOURCE=.\TitleMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\VisualStylesXP.cpp
# End Source File
# Begin Source File

SOURCE=.\webbrowser2.cpp
# End Source File
# End Group
# Begin Group "App"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\GatherRandomnessDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\GenerateKeyPairDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\KeyLengthDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MFC_MUTE_GUI.cpp
# End Source File
# Begin Source File

SOURCE=.\MFC_MUTE_GUIDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MSDNIcons.cpp
# End Source File
# Begin Source File

SOURCE=.\MuteMainTabCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# End Group
# Begin Group "Connections Dialog"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\MUTEConnectionListCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\MuteConnectionsDlg.cpp
# End Source File
# End Group
# Begin Group "Search Dialog"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\MuteSearchDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MuteSearchListCtrl.cpp
# End Source File
# End Group
# Begin Group "Settings Dialog"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\MuteAboutDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MuteHelpDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MuteOptionsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MuteSettingsDlg.cpp
# End Source File
# End Group
# Begin Group "Downloads Dialog"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\MuteDownloadListCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\MuteDownloadQueue.cpp
# End Source File
# Begin Source File

SOURCE=.\MuteDownloadQueue.h
# End Source File
# Begin Source File

SOURCE=.\MuteDownloadQueueItem.cpp
# End Source File
# Begin Source File

SOURCE=.\MuteDownloadQueueItem.h
# End Source File
# Begin Source File

SOURCE=.\MuteDownloadsDlg.cpp
# End Source File
# End Group
# Begin Group "Uploads Dialog"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\MuteUploadListCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\MuteUploadsDlg.cpp
# End Source File
# End Group
# Begin Group "Shared Files Dialog"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\MuteSharedFilesDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MuteSharedFilesDlg.h
# End Source File
# Begin Source File

SOURCE=.\MuteSharedFilesListCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\MuteSharedFilesListCtrl.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\debugout.cpp
# End Source File
# Begin Source File

SOURCE=.\debugout.h
# End Source File
# Begin Source File

SOURCE=.\Utilities.cpp
# End Source File
# Begin Source File

SOURCE=.\Utilities.h
# End Source File
# Begin Source File

SOURCE=.\WinFileSysUtils.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\ExternString.h
# End Source File
# Begin Source File

SOURCE=.\GatherRandomnessDlg.h
# End Source File
# Begin Source File

SOURCE=.\GenerateKeyPairDlg.h
# End Source File
# Begin Source File

SOURCE=.\KeyLengthDlg.h
# End Source File
# Begin Source File

SOURCE=.\Led.h
# End Source File
# Begin Source File

SOURCE=.\MemDC.h
# End Source File
# Begin Source File

SOURCE=.\MFC_MUTE_GUI.h
# End Source File
# Begin Source File

SOURCE=.\MFC_MUTE_GUIDlg.h
# End Source File
# Begin Source File

SOURCE=.\MuleListCtrl.h
# End Source File
# Begin Source File

SOURCE=.\MuteAboutDlg.h
# End Source File
# Begin Source File

SOURCE=.\MUTEConnectionListCtrl.h
# End Source File
# Begin Source File

SOURCE=.\MuteConnectionsDlg.h
# End Source File
# Begin Source File

SOURCE=.\MuteDownloadListCtrl.h
# End Source File
# Begin Source File

SOURCE=.\MuteDownloadsDlg.h
# End Source File
# Begin Source File

SOURCE=.\MuteHelpDlg.h
# End Source File
# Begin Source File

SOURCE=.\MuteMainTabCtrl.h
# End Source File
# Begin Source File

SOURCE=.\MuteOptionsDlg.h
# End Source File
# Begin Source File

SOURCE=.\MuteSearchDlg.h
# End Source File
# Begin Source File

SOURCE=.\MuteSearchListCtrl.h
# End Source File
# Begin Source File

SOURCE=.\MuteSettingsDlg.h
# End Source File
# Begin Source File

SOURCE=.\MuteUploadListCtrl.h
# End Source File
# Begin Source File

SOURCE=.\MuteUploadsDlg.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\StrParse.h
# End Source File
# Begin Source File

SOURCE=.\TitleMenu.h
# End Source File
# Begin Source File

SOURCE=.\webbrowser2.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=".\res\22-Vaio2 Network Neighborhood.ico"
# End Source File
# Begin Source File

SOURCE=".\res\ABOUT-background.bmp"
# End Source File
# Begin Source File

SOURCE=.\res\back.ico
# End Source File
# Begin Source File

SOURCE=.\res\back_ico.ico
# End Source File
# Begin Source File

SOURCE=.\res\cancel.ico
# End Source File
# Begin Source File

SOURCE=.\res\clear.ico
# End Source File
# Begin Source File

SOURCE=.\res\clear2.ico
# End Source File
# Begin Source File

SOURCE=.\res\clear_complete.ico
# End Source File
# Begin Source File

SOURCE=.\res\clear_stalled.ico
# End Source File
# Begin Source File

SOURCE=.\res\ConnectedHighHigh.ico
# End Source File
# Begin Source File

SOURCE=.\res\ConnectedLowLow.ico
# End Source File
# Begin Source File

SOURCE=.\res\ConnectedNotNot.ico
# End Source File
# Begin Source File

SOURCE=.\res\connections.ico
# End Source File
# Begin Source File

SOURCE=.\res\deleteall.ico
# End Source File
# Begin Source File

SOURCE=.\res\DeleteSelected.ico
# End Source File
# Begin Source File

SOURCE=.\res\flags\DENMARK.ICO
# End Source File
# Begin Source File

SOURCE=.\res\down.bmp
# End Source File
# Begin Source File

SOURCE=.\res\down2x.bmp
# End Source File
# Begin Source File

SOURCE=.\res\download.ico
# End Source File
# Begin Source File

SOURCE=.\res\download2.ico
# End Source File
# Begin Source File

SOURCE=.\res\download3.ico
# End Source File
# Begin Source File

SOURCE=.\res\DownloadFiles.ico
# End Source File
# Begin Source File

SOURCE=.\res\Exit.ico
# End Source File
# Begin Source File

SOURCE=.\res\exportall.ico
# End Source File
# Begin Source File

SOURCE=.\res\extern_l.ico
# End Source File
# Begin Source File

SOURCE=.\res\fileSharingMUTE.ico
# End Source File
# Begin Source File

SOURCE=.\res\flags\FRANCE.ICO
# End Source File
# Begin Source File

SOURCE=.\res\flags\GERMANY.ICO
# End Source File
# Begin Source File

SOURCE=.\res\flags\GREECE.ICO
# End Source File
# Begin Source File

SOURCE=.\res\H_point.cur
# End Source File
# Begin Source File

SOURCE=.\res\ico00001.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon7.ico
# End Source File
# Begin Source File

SOURCE=.\res\Info.ico
# End Source File
# Begin Source File

SOURCE=.\res\flags\ITALY.ICO
# End Source File
# Begin Source File

SOURCE=.\res\leds.bmp
# End Source File
# Begin Source File

SOURCE=.\res\flags\LITHUAN.ICO
# End Source File
# Begin Source File

SOURCE=.\res\logo.bmp
# End Source File
# Begin Source File

SOURCE=.\res\MESSAGE.ICO
# End Source File
# Begin Source File

SOURCE=.\res\MFC_MUTE_GUI.ico
# End Source File
# Begin Source File

SOURCE=.\MFC_MUTE_GUI.rc
# End Source File
# Begin Source File

SOURCE=.\res\MFC_MUTE_GUI.rc2
# End Source File
# Begin Source File

SOURCE=.\res\mute_hel.ico
# End Source File
# Begin Source File

SOURCE=.\res\mute_help.ico
# End Source File
# Begin Source File

SOURCE=.\res\next.ico
# End Source File
# Begin Source File

SOURCE=.\res\Open.ico
# End Source File
# Begin Source File

SOURCE=.\res\preferences2.ico
# End Source File
# Begin Source File

SOURCE=.\res\Prefs_General.ico
# End Source File
# Begin Source File

SOURCE=.\res\PriorityHigh.ico
# End Source File
# Begin Source File

SOURCE=.\res\Refresh.ico
# End Source File
# Begin Source File

SOURCE=.\res\RestoreWindow.ico
# End Source File
# Begin Source File

SOURCE=.\res\SearchResults.ico
# End Source File
# Begin Source File

SOURCE=.\res\SearchResults2.ico
# End Source File
# Begin Source File

SOURCE=.\res\serveradd.ico
# End Source File
# Begin Source File

SOURCE=.\res\ServerList.ico
# End Source File
# Begin Source File

SOURCE=.\res\ServersUpdate.ico
# End Source File
# Begin Source File

SOURCE=.\res\SharedFiles.ico
# End Source File
# Begin Source File

SOURCE=.\res\SharedFilesList.ico
# End Source File
# Begin Source File

SOURCE=.\res\slidbara.gif
# End Source File
# Begin Source File

SOURCE=.\res\flags\SPAIN.ICO
# End Source File
# Begin Source File

SOURCE=.\res\start.ico
# End Source File
# Begin Source File

SOURCE=.\res\TRASH.ICO
# End Source File
# Begin Source File

SOURCE=.\res\TRAYBUTTON_LUNA_BLUE.BMP
# End Source File
# Begin Source File

SOURCE=.\res\TRAYBUTTON_LUNA_HOMESTEAD.BMP
# End Source File
# Begin Source File

SOURCE=.\res\TRAYBUTTON_LUNA_METALLIC.BMP
# End Source File
# Begin Source File

SOURCE=.\res\flags\TURKEY.ICO
# End Source File
# Begin Source File

SOURCE=.\res\flags\U_S_A.ICO
# End Source File
# Begin Source File

SOURCE=.\res\up.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Up0down0.ico
# End Source File
# Begin Source File

SOURCE=.\res\Up0down1.ico
# End Source File
# Begin Source File

SOURCE=.\res\Up1down0.ico
# End Source File
# Begin Source File

SOURCE=.\res\Up1down1.ico
# End Source File
# Begin Source File

SOURCE=.\res\up2x.bmp
# End Source File
# Begin Source File

SOURCE=.\res\upload.ico
# End Source File
# Begin Source File

SOURCE=.\res\upload2.ico
# End Source File
# Begin Source File

SOURCE=.\res\User.ico
# End Source File
# Begin Source File

SOURCE=.\res\WebBased.ico
# End Source File
# End Group
# Begin Group "minorGems"

# PROP Default_Filter ""
# Begin Group "crypto"

# PROP Default_Filter ""
# Begin Group "hashes"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\crypto\hashes\sha1.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\crypto\hashes\sha1.h
# End Source File
# End Group
# End Group
# Begin Group "formats"

# PROP Default_Filter ""
# Begin Group "html"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\formats\html\HTMLUtils.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\formats\html\HTMLUtils.h
# End Source File
# End Group
# Begin Group "xml"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\formats\xml\XMLUtils.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\formats\xml\XMLUtils.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\formats\encodingUtils.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\formats\encodingUtils.h
# End Source File
# End Group
# Begin Group "io"

# PROP Default_Filter ""
# Begin Group "file"

# PROP Default_Filter ""
# Begin Group "win32 No. 1"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\io\file\win32\DirectoryWin32.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\io\file\win32\dirent.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\io\file\win32\dirent.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\io\file\win32\PathWin32.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\io\file\Directory.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\io\file\File.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\io\file\FileInputStream.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\io\file\FileOutputStream.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\io\file\Path.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\io\file\UniversalFileIO.h
# End Source File
# End Group
# Begin Group "win32"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\io\win32\TypeIOWin32.cpp
# End Source File
# End Group
# End Group
# Begin Group "network"

# PROP Default_Filter ""
# Begin Group "win32 No. 2"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\network\win32\HostAddressWin32.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\network\win32\SocketClientWin32.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\network\win32\SocketServerWin32.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\network\win32\SocketWin32.cpp
# End Source File
# End Group
# Begin Group "p2pParts"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\network\p2pParts\DuplicateMessageDetector.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\network\p2pParts\DuplicateMessageDetector.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\network\p2pParts\HostCatcher.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\network\p2pParts\HostCatcher.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\network\p2pParts\MessagePerSecondLimiter.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\network\p2pParts\MessagePerSecondLimiter.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\network\p2pParts\MultipleConnectionPreventer.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\network\p2pParts\MultipleConnectionPreventer.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\network\p2pParts\OutboundChannel.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\network\p2pParts\OutboundChannel.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\network\p2pParts\protocolUtils.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\network\p2pParts\protocolUtils.h
# End Source File
# End Group
# Begin Group "web"

# PROP Default_Filter ""
# Begin Group "server"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\network\web\server\ConnectionPermissionHandler.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\network\web\server\ConnectionPermissionHandler.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\network\web\server\PageGenerator.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\network\web\server\RequestHandlingThread.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\network\web\server\RequestHandlingThread.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\network\web\server\ThreadHandlingThread.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\network\web\server\ThreadHandlingThread.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\network\web\server\WebServer.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\network\web\server\WebServer.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\network\web\MimeTyper.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\network\web\MimeTyper.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\network\web\URLUtils.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\network\web\URLUtils.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\network\web\WebClient.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\network\web\WebClient.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\network\HostAddress.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\network\LoggingSocketStream.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\network\Message.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\network\NetworkFunctionLocks.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\network\NetworkFunctionLocks.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\network\Socket.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\network\SocketClient.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\network\SocketServer.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\network\SocketStream.h
# End Source File
# End Group
# Begin Group "protocol"

# PROP Default_Filter ""
# Begin Group "p2p"

# PROP Default_Filter ""
# End Group
# End Group
# Begin Group "system"

# PROP Default_Filter ""
# Begin Group "win32 No. 3"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\system\jroc\BinarySemaphoreJROC.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\system\win32\LauncherWin32.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\system\jroc\MutexJROC.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\system\win32\ThreadWin32.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\system\win32\TimeWin32.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\system\BinarySemaphore.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\system\endian.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\system\Launcher.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\system\Semaphore.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\system\StopSignalThread.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\system\StopSignalThread.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\system\Thread.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\system\ThreadSafePrinter.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\system\Time.h
# End Source File
# End Group
# Begin Group "util"

# PROP Default_Filter ""
# Begin Group "log"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\util\log\AppLog.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\util\log\AppLog.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\util\log\FileLog.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\util\log\FileLog.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\util\log\Log.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\util\log\Log.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\util\log\PrintLog.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\util\log\PrintLog.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\util\printUtils.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\util\SettingsManager.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\util\StringBufferOutputStream.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\util\stringUtils.cpp
# End Source File
# End Group
# End Group
# Begin Group "MUTE"

# PROP Default_Filter ""
# Begin Group "common"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\common\AESDecryptor.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\common\AESDecryptor.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\common\AESEncryptor.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\common\AESEncryptor.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\common\CryptoUtils.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\common\CryptoUtils.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\common\SimpleWebHostCache.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\common\SimpleWebHostCache.h
# End Source File
# End Group
# Begin Group "cryptoPP"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\crypto\aes.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\crypto\algebra.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\crypto\algebra.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\crypto\asn.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\crypto\asn.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\crypto\cbc.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\crypto\cbc.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\crypto\config.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\crypto\cryptlib.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\crypto\cryptlib.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\crypto\default.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\crypto\default.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\crypto\des.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\crypto\des.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\crypto\dessp.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\crypto\eprecomp.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\crypto\eprecomp.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\crypto\files.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\crypto\files.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\crypto\filters.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\crypto\filters.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\crypto\hex.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\crypto\hex.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\crypto\hmac.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\crypto\integer.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\crypto\integer.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\crypto\iterhash.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\crypto\iterhash.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\crypto\mdc.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\crypto\misc.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\crypto\misc.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\crypto\modarith.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\crypto\modes.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\crypto\modes.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\crypto\mqueue.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\crypto\mqueue.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\crypto\nbtheory.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\crypto\nbtheory.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\crypto\oaep.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\crypto\oaep.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\crypto\oids.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\crypto\pch.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\crypto\pch.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\crypto\pkcspad.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\crypto\pkcspad.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\crypto\pubkey.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\crypto\pubkey.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\crypto\queue.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\crypto\queue.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\crypto\randpool.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\crypto\randpool.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\crypto\rdtables.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\crypto\rijndael.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\crypto\rijndael.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\crypto\rng.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\crypto\rng.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\crypto\rsa.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\crypto\rsa.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\crypto\sha.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\crypto\sha.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\crypto\smartptr.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\crypto\words.h
# End Source File
# End Group
# Begin Group "layers"

# PROP Default_Filter ""
# Begin Group "fileTransfer"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\layers\fileTransfer\fileTransmitter.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\layers\fileTransfer\fileTransmitter.h
# End Source File
# End Group
# Begin Group "messageRouting"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\layers\messageRouting\ChannelReceivingThread.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\layers\messageRouting\ChannelReceivingThread.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\layers\messageRouting\ChannelReceivingThreadManager.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\layers\messageRouting\ChannelReceivingThreadManager.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\layers\messageRouting\ConnectionMaintainer.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\layers\messageRouting\ConnectionMaintainer.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\layers\messageRouting\LocalAddressReceiver.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\layers\messageRouting\LocalAddressReceiver.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\layers\messageRouting\MessageIDTracker.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\layers\messageRouting\MessageIDTracker.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\layers\messageRouting\messageRouter.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\layers\messageRouting\messageRouter.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\layers\messageRouting\OutboundChannelManager.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\layers\messageRouting\OutboundChannelManager.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\layers\messageRouting\ServerThread.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\layers\messageRouting\ServerThread.h
# End Source File
# End Group
# Begin Group "pointToPoint"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\layers\pointToPoint\pointToPointCommunicator.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\layers\pointToPoint\pointToPointCommunicator.h
# End Source File
# End Group
# Begin Group "secureStream"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\layers\secureStream\SecureInputStream.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\layers\secureStream\SecureInputStream.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\layers\secureStream\SecureOutputStream.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\layers\secureStream\SecureOutputStream.h
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\layers\secureStream\SecureStreamFactory.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\layers\secureStream\SecureStreamFactory.h
# End Source File
# End Group
# End Group
# Begin Group "otherApps"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\otherApps\fileSharing\fileShare.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\otherApps\fileSharing\fileShare.h
# End Source File
# End Group
# Begin Group "UI"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\otherApps\fileSharing\userInterface\common\formatUtils.cpp
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\MUTE\otherApps\fileSharing\userInterface\common\formatUtils.h
# End Source File
# End Group
# End Group
# Begin Source File

SOURCE=".\FUTURE THINGS TO ADD to MUTE MFC GUI.txt"
# End Source File
# Begin Source File

SOURCE=".\HOW TO BUILD - VISUAL STUDIO 6.0.TXT"
# End Source File
# Begin Source File

SOURCE=.\res\MUTE_MFC_GUI.manifest
# End Source File
# Begin Source File

SOURCE=..\MFC_MUTECORE_SOURCE\minorGems\system\pthreads\pthreadVSE.lib
# End Source File
# End Target
# End Project
