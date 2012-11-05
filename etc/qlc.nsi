Name "Q Light Controller"
OutFile "qlc-3.3.0.exe"
InstallDir C:\QLC
RequestExecutionLevel user
;--------------------------------
; Pages
Page directory
Page custom StartMenuGroupSelect "" ": Start Menu Folder"
Page instfiles

Function StartMenuGroupSelect
	Push $R1

	StartMenu::Select /checknoshortcuts "Don't create a start menu folder" /autoadd /lastused $R0 "Q Light Controller"
	Pop $R1

	StrCmp $R1 "success" success
	StrCmp $R1 "cancel" done
		; error
		MessageBox MB_OK $R1
		StrCpy $R0 "Q Light Controller" # use default
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
		CreateShortCut '$SMPROGRAMS\$R0\Q Light Controller.lnk' $INSTDIR\qlc.exe

		CreateDirectory $SMPROGRAMS\$R0
		CreateShortCut '$SMPROGRAMS\$R0\Fixture Definition Editor.lnk' $INSTDIR\qlc-fixtureeditor.exe

		CreateDirectory $SMPROGRAMS\$R0
		CreateShortCut '$SMPROGRAMS\$R0\Uninstall.lnk' $INSTDIR\uninstall.exe

	skip:
SectionEnd

Section
	File mingwm10.dll
	File libgcc_s_dw2-1.dll
	File qlc.exe
	File qlc-fixtureeditor.exe
    File qlcengine.dll
    File qlcui.dll
	File QtCore4.dll
	File QtGui4.dll
	File QtXml4.dll
	File QtNetwork4.dll
    File QtScript4.dll
	File Sample.qxw
	File *.qm
	File /r Documents
	File /r Fixtures
	File /r InputProfiles
	File /r Plugins

	WriteRegStr HKCR ".qxw" "" "QLightController.Document"
	WriteRegStr HKCR "QLightController.Document" "" "Q Light Controller Workspace"
	WriteRegStr HKCR "QLightController.Document\DefaultIcon" "" "$INSTDIR\qlc.exe,0"
	WriteRegStr HKCR "QLightController.Document\shell\open\command" "" '"$INSTDIR\qlc.exe" "--open %1"'

	WriteRegStr HKCR ".qxf" "" "QLightControllerFixture.Document"
	WriteRegStr HKCR "QLightControllerFixture.Document" "" "Q Light Controller Fixture"
	WriteRegStr HKCR "QLightControllerFixture.Document\DefaultIcon" "" "$INSTDIR\qlc-fixtureeditor.exe,0"
	WriteRegStr HKCR "QLightControllerFixture.Document\shell\open\command" "" '"$INSTDIR\qlc-fixtureeditor.exe" "--open %1"'

	WriteUninstaller $INSTDIR\uninstall.exe
SectionEnd

;--------------------------------
; Uninstallation

UninstPage uninstConfirm
UninstPage instfiles
Section "Uninstall"
	Delete $INSTDIR\uninstall.exe
	Delete $INSTDIR\qlc.exe
	Delete $INSTDIR\qlc-fixtureeditor.exe
    Delete $INSTDIR\qlcengine.dll
    Delete $INSTDIR\qlcui.dll
	Delete $INSTDIR\mingwm10.dll
	Delete $INSTDIR\libgcc_s_dw2-1.dll
	Delete $INSTDIR\QtCore4.dll
	Delete $INSTDIR\QtGui4.dll
	Delete $INSTDIR\QtXml4.dll
	Delete $INSTDIR\QtNetwork4.dll
    Delete $INSTDIR\QtScript4.dll
	Delete $INSTDIR\Sample.qxw
	Delete $INSTDIR\*.qm
	RMDir /r $INSTDIR\Documents
	RMDir /r $INSTDIR\Fixtures
	RMDir /r $INSTDIR\InputProfiles
	RMDir /r $INSTDIR\Plugins

	RMDir $INSTDIR

	DeleteRegKey HKCR ".qxw"
	DeleteRegKey HKCR "QLightController.Document"

	DeleteRegKey HKCR ".qxf"
	DeleteRegKey HKCR "QLightControllerFixture.Document"
SectionEnd
