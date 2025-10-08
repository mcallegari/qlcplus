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

;--------------------------------
;General
Name "Q Light Controller Plus"
OutFile "QLC+_5.0.0.exe"
InstallDir C:\QLC+5
InstallDirRegKey HKCU "Software\qlcplus" "Install_Dir"
RequestExecutionLevel user

!define MUI_LICENSEPAGE_TEXT_TOP "Do you accept the following statement of the Apache 2.0 license?"

!insertmacro MUI_PAGE_LICENSE "${QLCPLUS_HOME}\platforms\windows\apache_2.0.txt"

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

;--------------------------------
; Pages
Page directory
Page custom StartMenuGroupSelect "" ": Start Menu Folder"
Page instfiles

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

Section
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

Section
	File qlcplus-qml.exe
	File *.dll
	File *.qm
	File Sample.qxw
	File /r audio
	File /r geometryloaders
	File /r imageformats
	File /r mediaservice
	File /r platforms
	File /r printsupport
	File /r renderers
	File /r sceneparsers
	File /r Qt
	File /r Qt3D
	File /r QtMultimedia
	File /r QtQml
	File /r QtQuick
	File /r QtQuick.2
	File /r ColorFilters
	File /r Fixtures
	File /r Gobos
	File /r InputProfiles
	File /r Meshes
	File /r MidiTemplates
	File /r ModifiersTemplates
	File /r Plugins
	File /r RGBScripts

;	WriteRegStr HKCR ".qxw" "" "QLightControllerPlus.Document"
;	WriteRegStr HKCR "QLightControllerPlus.Document" "" "Q Light Controller Plus Workspace"
;	WriteRegStr HKCR "QLightControllerPlus.Document\DefaultIcon" "" "$INSTDIR\qlcplus-qml.exe,0"
;	WriteRegStr HKCR "QLightControllerPlus.Document\shell\open\command" "" '"$INSTDIR\qlcplus-qml.exe" --open "%1"'

;	WriteRegStr HKCR ".qxf" "" "QLightControllerPlusFixture.Document"
;	WriteRegStr HKCR "QLightControllerPlusFixture.Document" "" "Q Light Controller Plus Fixture"
;	WriteRegStr HKCR "QLightControllerPlusFixture.Document\DefaultIcon" "" "$INSTDIR\qlcplus-qml.exe,0"
;	WriteRegStr HKCR "QLightControllerPlusFixture.Document\shell\open\command" "" '"$INSTDIR\qlcplus-qml.exe" --open "%1"'

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
	RMDir /r $INSTDIR\audio
	RMDir /r $INSTDIR\geometryloaders
	RMDir /r $INSTDIR\imageformats
	RMDir /r $INSTDIR\mediaservice
	RMDir /r $INSTDIR\platforms
	RMDir /r $INSTDIR\printsupport
	RMDir /r $INSTDIR\sceneparsers
	RMDir /r $INSTDIR\Qt
	RMDir /r $INSTDIR\Qt3D
	RMDir /r $INSTDIR\QtMultimedia
	RMDir /r $INSTDIR\QtQml
	RMDir /r $INSTDIR\QtQuick
	RMDir /r $INSTDIR\QtQuick.2
	Delete $INSTDIR\Sample.qxw
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

;	DeleteRegKey HKCR ".qxw"
;	DeleteRegKey HKCR "QLightControllerPlus.Document"

	; This will delete all settings
;	DeleteRegKey HKCU "Software\qlcplus"
SectionEnd
