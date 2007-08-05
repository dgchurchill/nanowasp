; Nanowasp Installer

;--------------------------------
;Include Modern UI

  !include "MUI.nsh"

;--------------------------------
;General

  ;Name and file
  Name "Nanowasp"
  OutFile "nanowasp.exe"

  ;Default installation folder
  InstallDir "$PROGRAMFILES\Nanowasp"
  
  ;Get installation folder from registry if available
  InstallDirRegKey HKCU "Software\Nanowasp" ""

  ;Vista redirects $SMPROGRAMS to all users without this
  RequestExecutionLevel admin

;--------------------------------
;Variables

  Var MUI_TEMP
  Var STARTMENU_FOLDER

;--------------------------------
;Interface Settings

  !define MUI_ABORTWARNING

;--------------------------------
;Pages

  !insertmacro MUI_PAGE_DIRECTORY
  
  ;Start Menu Folder Page Configuration
  !define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKCU" 
  !define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\Nanowasp" 
  !define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"
  
  !insertmacro MUI_PAGE_STARTMENU Application $STARTMENU_FOLDER
  
  !insertmacro MUI_PAGE_INSTFILES
  
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES

;--------------------------------
;Languages
 
  !insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Installer Sections

Section "Nanowasp" SecNanowasp

  DetailPrint "Installing Visual C++ Runtime"
  SetOutPath "$TEMP"
  File "NonRelease\wix\VCCRT.msi"
  ExecWait 'MsiExec.exe /q /i $TEMP\VCCRT.msi'
  Delete "$TEMP\VCCRT.msi"


  SetOutPath "$INSTDIR"
  File "Release\nanowasp.exe"
  File "Libs\libdsk-1.1.12\win32vc6\Release\libdsk.dll"
  File "Microbee.xml"
  File "license.txt"
  File "Utils\cpmtools-2.6\cpmls.exe"
  File "Utils\cpmtools-2.6\cpmcp.exe"
  File "Utils\cpmtools-2.6\cpmrm.exe"
  File "Utils\cpmtools-2.6\diskdefs"
  
  SetOutPath "$INSTDIR\Data"
  ; Just to create Data folder
  
  SetOutPath "$INSTDIR\Images"
  File "Images\nanowasp-logo.png"

  ; Restore output path so the shortcut gets the correct working dir
  SetOutPath "$INSTDIR"

  
  ;Store installation folder
  WriteRegStr HKCU "Software\Nanowasp" "" $INSTDIR
  
  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"
  
  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
    
    ;Create shortcuts
    CreateDirectory "$SMPROGRAMS\$STARTMENU_FOLDER"
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Nanowasp.lnk" "$INSTDIR\nanowasp.exe"
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Uninstall Nanowasp.lnk" "$INSTDIR\Uninstall.exe"
  
  !insertmacro MUI_STARTMENU_WRITE_END

SectionEnd

;--------------------------------
;Uninstaller Section

Section "Uninstall"

  ;ADD YOUR OWN FILES HERE...
  Delete "$INSTDIR\nanowasp.exe"
  Delete "$INSTDIR\libdsk.dll"
  Delete "$INSTDIR\Microbee.xml"
  Delete "$INSTDIR\license.txt"
  Delete "$INSTDIR\cpmls.exe"
  Delete "$INSTDIR\cpmcp.exe"
  Delete "$INSTDIR\cpmrm.exe"
  Delete "$INSTDIR\diskdefs"
  
  RMDir "$INSTDIR\Data"
  
  Delete "$INSTDIR\Images\nanowasp-logo.png"
  RMDir "$INSTDIR\Images"

  Delete "$INSTDIR\Uninstall.exe"

  RMDir "$INSTDIR"
  
  !insertmacro MUI_STARTMENU_GETFOLDER Application $MUI_TEMP
    
  Delete "$SMPROGRAMS\$MUI_TEMP\Nanowasp.lnk"
  Delete "$SMPROGRAMS\$MUI_TEMP\Uninstall Nanowasp.lnk"
  
  ;Delete empty start menu parent diretories
  StrCpy $MUI_TEMP "$SMPROGRAMS\$MUI_TEMP"
 
  startMenuDeleteLoop:
	ClearErrors
    RMDir $MUI_TEMP
    GetFullPathName $MUI_TEMP "$MUI_TEMP\.."
    
    IfErrors startMenuDeleteLoopDone
  
    StrCmp $MUI_TEMP $SMPROGRAMS startMenuDeleteLoopDone startMenuDeleteLoop
  startMenuDeleteLoopDone:

  DeleteRegKey /ifempty HKCU "Software\Nanowasp"

SectionEnd
