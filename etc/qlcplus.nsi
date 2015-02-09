;--------------------------------
;Include Modern UI
  !include "MUI2.nsh"

;--------------------------------
;Defines
!define QLCPLUS_HOME "c:\Qt\qlcplus"
!define MUI_ICON "${QLCPLUS_HOME}\resources\icons\qlcplus.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Header\win.bmp"
!define MUI_HEADERIMAGE_LEFT
!define MUI_PAGE_HEADER_TEXT "Q Light Controller Plus"

;--------------------------------
;General
Name "Q Light Controller Plus"
OutFile "QLC+_4.8.5.exe"
InstallDir C:\QLC+
InstallDirRegKey HKCU "Software\qlcplus" "Install_Dir"
RequestExecutionLevel user

;--------------------------------
;Languages
!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "Italian"
!insertmacro MUI_LANGUAGE "German"
!insertmacro MUI_LANGUAGE "Spanish"
!insertmacro MUI_LANGUAGE "SpanishInternational"

!define MUI_LICENSEPAGE_TEXT_TOP "Do you accept the following statement of the Apache 2.0 license ?"

!insertmacro MUI_PAGE_LICENSE "${QLCPLUS_HOME}\etc\apache_2.0.txt"

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
		CreateShortCut '$SMPROGRAMS\$R0\Q Light Controller Plus.lnk' $INSTDIR\qlcplus.exe

		CreateDirectory $SMPROGRAMS\$R0
		CreateShortCut '$SMPROGRAMS\$R0\Fixture Definition Editor.lnk' $INSTDIR\qlcplus-fixtureeditor.exe

		CreateDirectory $SMPROGRAMS\$R0
		CreateShortCut '$SMPROGRAMS\$R0\Uninstall.lnk' $INSTDIR\uninstall.exe

	skip:
SectionEnd

Section
	File mingwm10.dll
	File libgcc_s_dw2-1.dll
	File libmad-0.dll
	File libogg-0.dll
	File libFLAC-8.dll
	File libvorbis-0.dll
	File libvorbisenc-2.dll
	File libsndfile-1.dll
	File libfftw3-3.dll
	File libstdc++-6.dll
	File pthreadGC2.dll
	File qlcplus.exe
	File qlcplus-fixtureeditor.exe
	File qlcplusengine.dll
	File qlcplusui.dll
	File qlcpluswebaccess.dll
	File QtCore4.dll
	File QtGui4.dll
	File QtXml4.dll
	File QtNetwork4.dll
	File QtScript4.dll
	File Sample.qxw
	File *.qm
	File /r Documents
	File /r Fixtures
	File /r Gobos
	File /r InputProfiles
	File /r MidiTemplates
	File /r ModifiersTemplates
	File /r Plugins
	File /r RGBScripts
	File /r Web

	WriteRegStr HKCR ".qxw" "" "QLightControllerPlus.Document"
	WriteRegStr HKCR "QLightControllerPlus.Document" "" "Q Light Controller Plus Workspace"
	WriteRegStr HKCR "QLightControllerPlus.Document\DefaultIcon" "" "$INSTDIR\qlcplus.exe,0"
	WriteRegStr HKCR "QLightControllerPlus.Document\shell\open\command" "" '"$INSTDIR\qlcplus.exe" --open "%1"'

	WriteRegStr HKCR ".qxf" "" "QLightControllerPlusFixture.Document"
	WriteRegStr HKCR "QLightControllerFixturePlus.Document" "" "Q Light Controller Plus Fixture"
	WriteRegStr HKCR "QLightControllerFixturePlus.Document\DefaultIcon" "" "$INSTDIR\qlcplus-fixtureeditor.exe,0"
	WriteRegStr HKCR "QLightControllerFixturePlus.Document\shell\open\command" "" '"$INSTDIR\qlcplus-fixtureeditor.exe" --open "%1"'

	WriteRegStr HKCU "SOFTWARE\qlcplus" "Install_Dir" "$INSTDIR"

	WriteUninstaller $INSTDIR\uninstall.exe
SectionEnd

;--------------------------------
; Uninstallation

UninstPage uninstConfirm
UninstPage instfiles
Section "Uninstall"
	Delete $INSTDIR\uninstall.exe
	Delete $INSTDIR\qlcplus.exe
	Delete $INSTDIR\qlcplus-fixtureeditor.exe
	Delete $INSTDIR\qlcplusengine.dll
	Delete $INSTDIR\qlcplusui.dll
	Delete $INSTDIR\qlcpluswebaccess.dll
	Delete $INSTDIR\mingwm10.dll
	Delete $INSTDIR\libgcc_s_dw2-1.dll
	Delete $INSTDIR\libmad-0.dll
	Delete $INSTDIR\libogg-0.dll
	Delete $INSTDIR\libFLAC-8.dll
	Delete $INSTDIR\libvorbis-0.dll
	Delete $INSTDIR\libvorbisenc-2.dll
	Delete $INSTDIR\libsndfile-1.dll
	Delete $INSTDIR\libfftw3-3.dll
	Delete $INSTDIR\libstdc++-6.dll
	Delete $INSTDIR\pthreadGC2.dll
	Delete $INSTDIR\QtCore4.dll
	Delete $INSTDIR\QtGui4.dll
	Delete $INSTDIR\QtXml4.dll
	Delete $INSTDIR\QtNetwork4.dll
	Delete $INSTDIR\QtScript4.dll
	Delete $INSTDIR\Sample.qxw
	Delete $INSTDIR\*.qm
	RMDir /r $INSTDIR\Documents
	RMDir /r $INSTDIR\Fixtures
	RMDir /r $INSTDIR\Gobos
	RMDir /r $INSTDIR\InputProfiles
	RMDir /r $INSTDIR\MidiTemplates
	RMDir /r $INSTDIR\ModifiersTemplates
	RMDir /r $INSTDIR\Plugins
	RMDir /r $INSTDIR\RGBScripts
	RMDir /r $INSTDIR\Web

	RMDir $INSTDIR

	DeleteRegKey HKCR ".qxw"
	DeleteRegKey HKCR "QLightControllerPlus.Document"

	DeleteRegKey HKCR ".qxf"
	DeleteRegKey HKCR "QLightControllerPlusFixture.Document"

	; This will delete all settings
	DeleteRegKey HKCU "Software\qlcplus"
SectionEnd
