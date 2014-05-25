!addplugindir plugins
!include MUI2.nsh
!include myutil.nsh
!include plugins\RefreshShellIcons.nsh

!define PRETTYNAME "Mumble9000"
!define PATHNAME "Mumble9000"
!define EXENAME "mumble9000"







Name "${PRETTYNAME}"
RequestExecutionLevel user
InstallDir "$LOCALAPPDATA\${PATHNAME}\Application"
!ifndef OutFile
 !define OutFile "${PATHNAME}_install.exe"
!endif
OutFile "${OutFile}"


; Pages
!define MUI_ABORTWARNING
!define MUI_COMPONENTSPAGE_NODESC
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_UNPAGE_INSTFILES



Section

	strcpy $0 "mumble.exe"
	Call KillExe
	strcpy $0 "${EXENAME}.exe"
	Call KillExe

	Push "{B0EEFCC7-8A9c-4471-AB10-CBD35BE3161D}"
	call CallMsiDeinstaller

	; write shortcuts
	SetShellVarContext 'current'
	IfFileExists "$INSTDIR\${EXENAME}.exe" NoShortcuts
		CreateShortCut "$SMPROGRAMS\${PRETTYNAME}.lnk" "$INSTDIR\${EXENAME}.exe"
		CreateShortCut "$DESKTOP\${PRETTYNAME}.lnk" "$INSTDIR\${EXENAME}.exe"
	NoShortcuts:

	SetAutoClose true
	SetOutPath "$INSTDIR"
	WriteUninstaller "uninstall.exe"
	File "/oname=${EXENAME}.exe" "..\release\mumble.exe"
	File "..\release\*.dll"
	File "$%LIBSNDFILE_DIR%\bin\libsndfile-1.dll"
	File "$%DEBUGTOOLS_DIR%\dbghelp.dll"
	File "$%QT_DIR%\bin\icudt51.dll"
	File "$%QT_DIR%\bin\icuin51.dll"
	File "$%QT_DIR%\bin\icuuc51.dll"
	File "$%QT_DIR%\bin\libEGL.dll"
	File "$%QT_DIR%\bin\libGLESv2.dll"
	File "$%QT_DIR%\bin\Qt5Core.dll"
	File "$%QT_DIR%\bin\Qt5Gui.dll"
	File "$%QT_DIR%\bin\Qt5Network.dll"
	File "$%QT_DIR%\bin\Qt5Sql.dll"
	File "$%QT_DIR%\bin\Qt5Svg.dll"
	File "$%QT_DIR%\bin\Qt5Widgets.dll"
	File "$%QT_DIR%\bin\Qt5Xml.dll"
	File "$%OPENSSL_DIR%\bin\libeay32.dll"
	File "$%OPENSSL_DIR%\bin\ssleay32.dll"
	File "$%ZLIB_DIR%\zlib1.dll"
	SetOutPath "$INSTDIR\accessible"
	File "$%QT_DIR%\plugins\accessible\qtaccessiblewidgets.dll"
	SetOutPath "$INSTDIR\iconengines"
	File "$%QT_DIR%\plugins\iconengines\qsvgicon.dll"
	SetOutPath "$INSTDIR\imageformats"
	File "$%QT_DIR%\plugins\imageformats\qgif.dll"
	File "$%QT_DIR%\plugins\imageformats\qico.dll"
	File "$%QT_DIR%\plugins\imageformats\qjpeg.dll"
	File "$%QT_DIR%\plugins\imageformats\qmng.dll"
	File "$%QT_DIR%\plugins\imageformats\qsvg.dll"
	File "$%QT_DIR%\plugins\imageformats\qtiff.dll"
	SetOutPath "$INSTDIR\platforms"
	File "$%QT_DIR%\plugins\platforms\qwindows.dll"
	SetOutPath "$INSTDIR\sqldrivers"
	File "$%QT_DIR%\plugins\sqldrivers\qsqlite.dll"

	; Write uninstaller
	DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PATHNAME}"
	WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PATHNAME}" "DisplayName" "${PRETTYNAME}"
	WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PATHNAME}" "DisplayIcon" "$INSTDIR\${EXENAME}.exe"
	WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PATHNAME}" "UninstallString" "$INSTDIR\uninstall.exe"
	WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PATHNAME}" "InstallLocation" "$INSTDIR"
	WriteRegDWORD HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PATHNAME}" "NoModify" 0x1
	WriteRegDWORD HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PATHNAME}" "NoRepair" 0x1
SectionEnd
Function .onInstSuccess
	;Call RefreshShellIcons
	Exec '"$WINDIR\explorer.exe" "$INSTDIR\${EXENAME}.exe"'
	
	; If we quit before the main window opens, it doesn't
	; get brought to the front on windows 8 :(
	;Sleep 1000
FunctionEnd

;--------------------------------
; Uninstaller

Section "Uninstall"
	IfFileExists "$INSTDIR\${EXENAME}.exe" +3
		MessageBox MB_OK "The uninstaller was unable to find the installed application."
		Abort

	strcpy $0 "${EXENAME}.exe"
	Call un.KillExe

	SetAutoClose true

	RMDir /r "$INSTDIR"

	SetShellVarContext 'current'
	Delete "$SMPROGRAMS\${PRETTYNAME}.lnk"
	Delete "$SMPROGRAMS\Startup\${PRETTYNAME}.lnk"
	Delete "$DESKTOP\${PRETTYNAME}.lnk"

	DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PATHNAME}"
SectionEnd

;------------------------------
; This must be at the end
!insertmacro MUI_LANGUAGE "English"
