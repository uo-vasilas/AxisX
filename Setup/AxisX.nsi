SetCompressor /SOLID /FINAL lzma
!define AXIS_PATH "..\release"

  ;Version is read from AxisX.exe
  !getdllversion "${AXIS_PATH}\AxisX.exe" AXV_
  ;tools\Release-Axis.ps1 passes the full version (e.g. 1.1.1) as AXIS_VERSION
  !ifdef AXIS_VERSION
    !define Version "${AXIS_VERSION}"
  !else
    !define Version "${AXV_1}.${AXV_2}"
  !endif
  !define XVersionX "${AXV_1}.${AXV_2}.${AXV_3}.${AXV_4}"
  !include "MUI2.nsh"
  !include "x64.nsh"
  !include "nsDialogs.nsh"
  !include "FileFunc.nsh"
  !insertmacro GetParent

  !define MULTIUSER_EXECUTIONLEVEL Highest
  ;64-bit program: all-users installs go to Program Files (not x86)
  !define MULTIUSER_USE_PROGRAMFILES64
  !define MULTIUSER_MUI

;--------------------------------
;General

  ;Name and file
  Name "AxisX - ${Version}"
  OutFile "AxisX_Setup_${Version}.exe"

  ;Icons
  !define MUI_ICON "Icons/AxisX.ico"
  !define MUI_UNICON "Icons/UnAxisX.ico"

  ;Settings
  BrandingText "AxisX Installation System"
  CRCCheck on
  ShowInstDetails show
  ShowUninstDetails show

  ;Version Info
  VIProductVersion "${XVersionX}"
  VIAddVersionKey /LANG=1033-English "ProductVersion" "${Version}"
  VIAddVersionKey /LANG=1033-English "ProductName" "AxisX GM Tool"
  VIAddVersionKey /LANG=1033-English "FileVersion" "${Version}"
  VIAddVersionKey /LANG=1033-English "FileDescription" "GameMaster Help Tool"
  VIAddVersionKey /LANG=1033-English "LegalCopyright" "GPL v2 - Axis contributors"
  VIAddVersionKey /LANG=1033-English "Comments" "This tool was designed to simplify a GameMaster's job"

;--------------------------------
;Interface Configuration

  ;Default installation folder
  !define MULTIUSER_INSTALLMODE_INSTDIR "AxisX"
  
  ;Updates: reuse the folder and install mode of the previous installation
  !define MULTIUSER_INSTALLMODE_INSTDIR_REGISTRY_KEY "SOFTWARE\Sphere\GM Tools"
  !define MULTIUSER_INSTALLMODE_INSTDIR_REGISTRY_VALUENAME "RootPath"
  !define MULTIUSER_INSTALLMODE_DEFAULT_REGISTRY_KEY "SOFTWARE\Sphere\GM Tools"
  !define MULTIUSER_INSTALLMODE_DEFAULT_REGISTRY_VALUENAME "RootPath"

  ;Install Mode Page Settings
  !define MULTIUSER_INSTALLMODE_DEFAULT_CURRENTUSER
  !define MULTIUSER_INSTALLMODEPAGE_TEXT_ALLUSERS "Install for All Users               (Administrator privileges are required)"
  !define MULTIUSER_INSTALLMODEPAGE_TEXT_CURRENTUSER "Install for this User only       (Recommended)"

  !include "MultiUser.nsh"

  ;Start Menu Page Settings
  !define MUI_STARTMENUPAGE_REGISTRY_ROOT HKCU
  !define MUI_STARTMENUPAGE_REGISTRY_KEY "SOFTWARE\AxisX" 
  !define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu"
  !define MUI_STARTMENUPAGE_TEXT_CHECKBOX "Do not add Start Menu Folder"

  !define MUI_LICENSEPAGE_TEXT_TOP "You must agree to this license before installing."

  !define MUI_HEADERIMAGE
  !define MUI_HEADERIMAGE_BITMAP "Icons\AxisX.bmp"
  !define MUI_HEADERIMAGE_UNBITMAP "Icons\AxisX.bmp"
  !define MUI_ABORTWARNING

  ;Language selection at startup (also sets the Axis X UI language)
  !define MUI_LANGDLL_WINDOWTITLE "AxisX - Language / Sprache"
  !define MUI_LANGDLL_INFO "Language for setup and AxisX:$\r$\nSprache für Installation und AxisX:"

;--------------------------------
;Variables

  Var StartMenuFolder
  Var TempInstalldir
  Var UOFolder
  Var UOFolderText
  Var UOFolderDlg

;--------------------------------
;Pages

  !insertmacro MUI_PAGE_LICENSE "${AXIS_PATH}\LICENSE"
  !insertmacro MULTIUSER_PAGE_INSTALLMODE
  !insertmacro MUI_PAGE_DIRECTORY
  Page custom UOFolderPageCreate UOFolderPageLeave
  !insertmacro MUI_PAGE_STARTMENU Application $StartMenuFolder
  !insertmacro MUI_PAGE_INSTFILES
  
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES

;--------------------------------
;Languages (must precede .onInit, which shows the language selection)

 !insertmacro MUI_LANGUAGE "English"
 !insertmacro MUI_LANGUAGE "German"
 !insertmacro MUI_RESERVEFILE_LANGDLL

;--------------------------------
;UO folder page texts

  LangString UOPAGE_TITLE ${LANG_ENGLISH} "Ultima Online folder"
  LangString UOPAGE_TITLE ${LANG_GERMAN} "Ultima-Online-Ordner"
  LangString UOPAGE_SUB ${LANG_ENGLISH} "Needed for the item, map and gump previews."
  LangString UOPAGE_SUB ${LANG_GERMAN} "Wird für die Item-, Karten- und Gump-Vorschau gebraucht."
  LangString UOPAGE_TEXT ${LANG_ENGLISH} "Axis X draws item, NPC, map and gump previews from the Ultima Online data files (art, tiledata, map, gumps ... *.mul). Without a UO folder these previews stay empty.$\r$\n$\r$\nSelect the folder that contains these files, usually the folder of your UO client (with client.exe and tiledata.mul). You can leave the field empty and set it later in Settings > Paths."
  LangString UOPAGE_TEXT ${LANG_GERMAN} "Axis X zeichnet die Vorschau von Items, NPCs, Karten und Gumps aus den Ultima-Online-Datendateien (art, tiledata, map, gumps ... *.mul). Ohne UO-Ordner bleibt die Vorschau leer.$\r$\n$\r$\nWähle den Ordner, in dem diese Dateien liegen, meist der Ordner deines UO-Clients (mit client.exe und tiledata.mul). Das Feld darf leer bleiben, der Ordner lässt sich später unter Einstellungen > Pfade eintragen."
  LangString UOPAGE_BROWSE ${LANG_ENGLISH} "Browse..."
  LangString UOPAGE_BROWSE ${LANG_GERMAN} "Durchsuchen..."
  LangString UOPAGE_BROWSETITLE ${LANG_ENGLISH} "Select your Ultima Online folder"
  LangString UOPAGE_BROWSETITLE ${LANG_GERMAN} "Ultima-Online-Ordner auswählen"
  LangString UOPAGE_NODIR ${LANG_ENGLISH} "This folder does not exist."
  LangString UOPAGE_NODIR ${LANG_GERMAN} "Dieser Ordner existiert nicht."
  LangString UOPAGE_NOMUL ${LANG_ENGLISH} "tiledata.mul was not found in this folder, so the previews will probably stay empty.$\r$\n$\r$\nUse this folder anyway?"
  LangString UOPAGE_NOMUL ${LANG_GERMAN} "In diesem Ordner wurde keine tiledata.mul gefunden, die Vorschau bleibt wahrscheinlich leer.$\r$\n$\r$\nTrotzdem diesen Ordner verwenden?"

;--------------------------------
;StartUp

Function .onInit
  ;64-bit Windows only; use the 64-bit registry view
  ${IfNot} ${RunningX64}
    MessageBox MB_OK|MB_ICONSTOP "Axis X needs 64-bit Windows.$\r$\nAxis X braucht ein 64-Bit-Windows."
    Abort
  ${EndIf}
  SetRegView 64
  !insertmacro MULTIUSER_INIT
  !insertmacro MUI_LANGDLL_DISPLAY
FunctionEnd

Function un.onInit
  SetRegView 64
  StrCpy $TempInstalldir $INSTDIR
  !insertmacro MULTIUSER_UNINIT
  StrCpy $INSTDIR $TempInstalldir
  !insertmacro MUI_UNGETLANGUAGE
FunctionEnd

;--------------------------------
;UO folder page

Function UOFolderDetect
  ;Existing Axis X setting first, then the registry entry of a classic UO installation
  ReadRegStr $0 HKCU "Software\Sphere\GM Tools" "Default MulPath"
  ${If} $0 == ""
    ReadRegStr $0 HKLM "Software\Sphere\GM Tools" "Default MulPath"
  ${EndIf}
  ${If} $0 == ""
    ReadRegStr $0 HKCU "Software\Sphere\GM Tools" "Default Client"
    ${If} $0 == ""
      ReadRegStr $0 HKLM "Software\Sphere\GM Tools" "Default Client"
    ${EndIf}
    ${If} $0 == ""
      ReadRegStr $0 HKLM "SOFTWARE\Origin Worlds Online\Ultima Online\1.0" "ExePath"
    ${EndIf}
    ${If} $0 != ""
      ${GetParent} $0 $0
    ${EndIf}
  ${EndIf}
  StrCpy $UOFolder $0
  Call UOFolderTrim
FunctionEnd

Function UOFolderTrim
  StrCpy $0 $UOFolder 1 -1
  ${If} $0 == "\"
    StrLen $0 $UOFolder
    IntOp $0 $0 - 1
    StrCpy $UOFolder $UOFolder $0
  ${EndIf}
FunctionEnd

Function UOFolderPageCreate
  !insertmacro MUI_HEADER_TEXT "$(UOPAGE_TITLE)" "$(UOPAGE_SUB)"
  ${If} $UOFolder == ""
    Call UOFolderDetect
  ${EndIf}
  nsDialogs::Create 1018
  Pop $UOFolderDlg
  ${If} $UOFolderDlg == error
    Abort
  ${EndIf}
  ${NSD_CreateLabel} 0 0 100% 70u "$(UOPAGE_TEXT)"
  Pop $0
  ${NSD_CreateText} 0 78u 78% 12u "$UOFolder"
  Pop $UOFolderText
  ${NSD_CreateButton} 80% 77u 20% 14u "$(UOPAGE_BROWSE)"
  Pop $0
  ${NSD_OnClick} $0 UOFolderBrowse
  nsDialogs::Show
FunctionEnd

Function UOFolderBrowse
  ${NSD_GetText} $UOFolderText $0
  nsDialogs::SelectFolderDialog "$(UOPAGE_BROWSETITLE)" "$0"
  Pop $0
  ${If} $0 != error
    ${NSD_SetText} $UOFolderText $0
  ${EndIf}
FunctionEnd

Function UOFolderPageLeave
  ${NSD_GetText} $UOFolderText $UOFolder
  Call UOFolderTrim
  ${If} $UOFolder == ""
    Return
  ${EndIf}
  ${IfNot} ${FileExists} "$UOFolder\*.*"
    MessageBox MB_OK|MB_ICONEXCLAMATION "$(UOPAGE_NODIR)"
    Abort
  ${EndIf}
  ${IfNot} ${FileExists} "$UOFolder\tiledata.mul"
    MessageBox MB_YESNO|MB_ICONQUESTION "$(UOPAGE_NOMUL)" IDYES +2
    Abort
  ${EndIf}
FunctionEnd

;Stores the chosen UO folder where Axis X reads it (Settings > Paths)
Function UOFolderSave
  ${If} $UOFolder == ""
    Return
  ${EndIf}
  ${If} ${FileExists} "$UOFolder\client.exe"
    StrCpy $1 1
  ${Else}
    StrCpy $1 0
  ${EndIf}
  ${If} $MultiUser.InstallMode == "AllUsers"
    WriteRegStr HKLM "Software\Sphere\GM Tools" "Default MulPath" "$UOFolder\"
    ${If} $1 == 1
      WriteRegStr HKLM "Software\Sphere\GM Tools" "Default Client" "$UOFolder\client.exe"
    ${Else}
      WriteRegDWORD HKLM "Software\Sphere\GM Tools" "SameAsClient" 0
    ${EndIf}
  ${Else}
    WriteRegStr HKCU "Software\Sphere\GM Tools" "Default MulPath" "$UOFolder\"
    ${If} $1 == 1
      WriteRegStr HKCU "Software\Sphere\GM Tools" "Default Client" "$UOFolder\client.exe"
    ${Else}
      WriteRegDWORD HKCU "Software\Sphere\GM Tools" "SameAsClient" 0
    ${EndIf}
  ${EndIf}
FunctionEnd
;--------------------------------
;Installer Sections

Section "Default Section" SecDefault

  SetOutPath "$INSTDIR"  
  File "${AXIS_PATH}\AxisX.exe"
  File "${AXIS_PATH}\AxisX.chm"
  File "${AXIS_PATH}\AxisX.ini"
  File "${AXIS_PATH}\DoorWiz.ini"
  File "${AXIS_PATH}\LightWiz.ini"
  File "${AXIS_PATH}\changelog.txt"
  File "${AXIS_PATH}\hoglocs.scp"
  SetOutPath "$INSTDIR\Language"
  File "${AXIS_PATH}\Language\*.*"
  ;Server profile folder: user folder for per-user installs, next to the exe for all users
  ${if} $MultiUser.InstallMode == "AllUsers"
    StrCpy $2 "$INSTDIR\Profile"
  ${else}
    StrCpy $2 "$LOCALAPPDATA\AxisX\Profile"
  ${endif}
  SetOutPath "$2"
  File "${AXIS_PATH}\Profile\LIESMICH.txt"
  ;Server profiles are only included with makensis /DWITH_PROFILE (taken from the builder's user folder)
  !ifdef WITH_PROFILE
    File /nonfatal "$%LOCALAPPDATA%\AxisX\Profile\*.scp"
  !endif
  SetOutPath "$INSTDIR"

  ;UI language from the installer language selection
  ${if} $LANGUAGE == ${LANG_GERMAN}
    StrCpy $1 "deu"
  ${else}
    StrCpy $1 "eng"
  ${endif}
  ; AxisX.exe is statically linked - no VC/MFC runtime files needed.
  

  ${if} $MultiUser.InstallMode == "AllUsers"
    ;Store installation folder
    WriteRegStr HKLM "Software\Sphere\GM Tools" "RootPath" "$INSTDIR"
    WriteRegStr HKLM "Software\Sphere\GM Tools" "Language" "$1"
    WriteIniStr "$INSTDIR\AxisX.ini" "SETTINGS Default" "RegInstallation" "Machine"

    ;Store Uninstallation Info
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AxisX" "DisplayName" "AxisX (remove only)"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\AxisX" "UninstallString" '"$INSTDIR\AxisX_uninst.exe"'
  ${else}
    ;Store installation folder
    WriteRegStr HKCU "Software\Sphere\GM Tools" "RootPath" "$INSTDIR"
    WriteRegStr HKCU "Software\Sphere\GM Tools" "Language" "$1"
    WriteIniStr "$INSTDIR\AxisX.ini" "SETTINGS Default" "RegInstallation" "User"

    ;Store Uninstallation Info
    WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\AxisX" "DisplayName" "AxisX (remove only)"
    WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\AxisX" "UninstallString" '"$INSTDIR\AxisX_uninst.exe"'
  ${endif}
  
  Call UOFolderSave

  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
    
    ;Create shortcuts
    CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk" "$INSTDIR\AxisX_uninst.exe"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\AxisX.lnk" "$INSTDIR\AxisX.exe"
  
  !insertmacro MUI_STARTMENU_WRITE_END

  CreateShortCut "$DESKTOP\AxisX.lnk" "$INSTDIR\AxisX.exe"

  DeleteRegKey HKCU "SOFTWARE\AxisX"

  ;Create uninstaller
  WriteUninstaller "$INSTDIR\AxisX_uninst.exe"

SectionEnd

;--------------------------------
;Uninstaller Section

Section "Uninstall"

ReadIniStr $0 "$TempInstalldir\AxisX.ini" "SETTINGS Default" "RegInstallation"
  ${if} $0 == "Machine"
    Call un.MultiUser.InstallMode.AllUsers
  ${else}
    Call un.MultiUser.InstallMode.CurrentUser
  ${endif}

Delete "$TempInstalldir\AxisX.exe"
Delete "$TempInstalldir\AxisX.chm"
Delete "$TempInstalldir\AxisX.ini"
Delete "$TempInstalldir\DoorWiz.ini"
Delete "$TempInstalldir\LightWiz.ini"
Delete "$TempInstalldir\changelog.txt"
Delete "$TempInstalldir\hoglocs.scp"
Delete "$DESKTOP\AxisX.lnk"
RMDir /r "$TempInstalldir\Language"
;Remove only the readme; server profiles (*.scp) stay
Delete "$TempInstalldir\Profile\LIESMICH.txt"
RMDir "$TempInstalldir\Profile"
Delete "$LOCALAPPDATA\AxisX\Profile\LIESMICH.txt"
RMDir "$LOCALAPPDATA\AxisX\Profile"
Delete "$TempInstalldir\AxisX_uninst.exe"
  
  !insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuFolder
  StrCmp $StartMenuFolder "" NO_SHORTCUTS  
  Delete "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\AxisX.lnk"
  RMDir "$SMPROGRAMS\$StartMenuFolder"
  NO_SHORTCUTS:

MessageBox MB_YESNO "Do you want to remove your AxisX settings?" IDNO Skip
  ${if} $0 == "Machine"
    DeleteRegKey HKLM "SOFTWARE\Sphere\GM Tools"
    DeleteRegKey HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\AxisX"
  ${else}
    DeleteRegKey HKCU "SOFTWARE\Sphere\GM Tools"
    DeleteRegKey HKCU "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\AxisX"
  ${endif}
skip:

SectionEnd