;--------------------------------
;Include Modern UI
  !include "MUI2.nsh"

;--------------------------------
;Defines
!define QLCPLUS_HOME "c:\projects\qlcplus"
!define MUI_ICON "${QLCPLUS_HOME}\resources\icons\qlcplus.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\nsis3-uninstall.ico"
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Header\nsis3-vintage.bmp"
!define MUI_HEADERIMAGE_LEFT
!define MUI_PAGE_HEADER_TEXT "Q Light Controller Plus"
!define MUI_COMPONENTSPAGE_NODESC

;--------------------------------
;General
Name "Q Light Controller Plus"
OutFile "QLC+_5.2.0.exe"
InstallDir C:\QLC+5
InstallDirRegKey HKCU "Software\qlcplus" "Install_Dir"
RequestExecutionLevel user

!define MUI_LICENSEPAGE_TEXT_TOP "Do you accept the following statement of the Apache 2.0 license?"

!insertmacro MUI_PAGE_LICENSE "${QLCPLUS_HOME}\platforms\windows\apache_2.0.txt"

;--------------------------------
; Pages
!insertmacro MUI_PAGE_COMPONENTS
Page directory
Page custom StartMenuGroupSelect "" ": Start Menu Folder"
Page instfiles

;--------------------------------
;Languages
!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "Italian"
!insertmacro MUI_LANGUAGE "German"
!insertmacro MUI_LANGUAGE "Spanish"
!insertmacro MUI_LANGUAGE "SpanishInternational"
!insertmacro MUI_LANGUAGE "Czech"
!insertmacro MUI_LANGUAGE "French"
!insertmacro MUI_LANGUAGE "Finnish"
!insertmacro MUI_LANGUAGE "Japanese"
!insertmacro MUI_LANGUAGE "Catalan"

Function StartMenuGroupSelect
	Push $R1

	StartMenu::Select /checknoshortcuts "Don't create a start menu folder" /autoadd /lastused $R0 "Q Light Controller Plus"
	Pop $R1

	StrCmp $R1 "success" success
	StrCmp $R1 "cancel" done
		; error
		MessageBox MB_OK $R1
		StrCpy $R0 "Q Light Controller Plus" # use default
		Return
	success:
	Pop $R0

	done:
	Pop $R1
FunctionEnd

;--------------------------------
; Main application section (checked & disabled)
Section "Q Light Controller Plus" SEC_MAIN
	SectionIn RO
	SetOutPath $INSTDIR

	# this part is only necessary if you used /checknoshortcuts
	StrCpy $R1 $R0 1
	StrCmp $R1 ">" skip
		CreateDirectory $SMPROGRAMS\$R0
		CreateShortCut '$SMPROGRAMS\$R0\Q Light Controller Plus.lnk' $INSTDIR\qlcplus-qml.exe

		CreateDirectory $SMPROGRAMS\$R0
		CreateShortCut '$SMPROGRAMS\$R0\Uninstall.lnk' $INSTDIR\uninstall.exe

	skip:
SectionEnd

;--------------------------------
; Optional file association section
Section "Associate .qxw and .qxf files" SEC_ASSOC
	; Per-user classes (maps to HKCR for current user)
	WriteRegStr HKCU "Software\Classes\.qxw" "" "QLightControllerPlus.Document"
	WriteRegStr HKCU "Software\Classes\QLightControllerPlus.Document" "" "Q Light Controller Plus Workspace"
	WriteRegStr HKCU "Software\Classes\QLightControllerPlus.Document\DefaultIcon" "" "$INSTDIR\qlcplus-qml.exe,0"
	WriteRegStr HKCU "Software\Classes\QLightControllerPlus.Document\shell\open\command" "" '"$INSTDIR\qlcplus-qml.exe" --open "%1"'

	WriteRegStr HKCU "Software\Classes\.qxf" "" "QLightControllerPlusFixture.Document"
	WriteRegStr HKCU "Software\Classes\QLightControllerPlusFixture.Document" "" "Q Light Controller Plus Fixture"
	WriteRegStr HKCU "Software\Classes\QLightControllerPlusFixture.Document\DefaultIcon" "" "$INSTDIR\qlcplus-qml.exe,0"
	WriteRegStr HKCU "Software\Classes\QLightControllerPlusFixture.Document\shell\open\command" "" '"$INSTDIR\qlcplus-qml.exe" --open "%1"'

	; Notify Explorer to refresh icons
	System::Call 'SHELL32::SHChangeNotify(i 0x08000000, i 0, i 0, i 0)'
SectionEnd

;--------------------------------
; File installation section
Section
	File qlcplus-qml.exe
	File *.dll
	File *.qm
	File Sample.qxw
	File /r geometryloaders
	File /r iconengines
	File /r imageformats
	File /r multimedia
	File /r platforms
	File /r qml
	File /r renderers
	File /r sceneparsers
	File /r ColorFilters
	File /r Fixtures
	File /r Gobos
	File /r InputProfiles
	File /r Meshes
	File /r MidiTemplates
	File /r ModifiersTemplates
	File /r Plugins
	File /r RGBScripts

	WriteRegStr HKCU "SOFTWARE\qlcplus" "Install_Dir" "$INSTDIR"

	WriteUninstaller $INSTDIR\uninstall.exe
SectionEnd

;--------------------------------
; Uninstallation

UninstPage uninstConfirm
UninstPage instfiles
Section "Uninstall"
	Delete $INSTDIR\uninstall.exe
	Delete $INSTDIR\qlcplus-qml.exe
	Delete $INSTDIR\*.dll
	Delete $INSTDIR\Sample.qxw
	Delete $INSTDIR\*.qm
	RMDir /r $INSTDIR\geometryloaders
	RMDir /r $INSTDIR\iconengines
	RMDir /r $INSTDIR\imageformats
	RMDir /r $INSTDIR\multimedia
	RMDir /r $INSTDIR\platforms
	RMDir /r $INSTDIR\renderers
	RMDir /r $INSTDIR\sceneparsers
	RMDir /r $INSTDIR\qml
	RMDir /r $INSTDIR\ColorFilters
	RMDir /r $INSTDIR\Fixtures
	RMDir /r $INSTDIR\Gobos
	RMDir /r $INSTDIR\InputProfiles
	RMDir /r $INSTDIR\Meshes
	RMDir /r $INSTDIR\MidiTemplates
	RMDir /r $INSTDIR\ModifiersTemplates
	RMDir /r $INSTDIR\Plugins
	RMDir /r $INSTDIR\RGBScripts

	RMDir $INSTDIR

	; Remove per-user QML cache
	RMDir /r "$LOCALAPPDATA\qlcplus"

	; Remove file associations (per-user)
	DeleteRegKey HKCU "Software\Classes\.qxw"
	DeleteRegKey HKCU "Software\Classes\QLightControllerPlus.Document"
	DeleteRegKey HKCU "Software\Classes\.qxf"
	DeleteRegKey HKCU "Software\Classes\QLightControllerPlusFixture.Document"

	; Refresh Explorer after uninstall
	System::Call 'SHELL32::SHChangeNotify(i 0x08000000, i 0, i 0, i 0)'
SectionEnd
