!include plugins\nsProcess.nsh



!define OLDVERSIONWARNING \
  "An older version of $(^Name) was found on your system. It is recommended that you uninstall the old version before installing the new version.$\r$\n$\r$\nDo you want to uninstall the old version of $(^Name)?"
!define OLDVERSIONREMOVEERROR \
  "A problem was encountered while removing the old version of $(^Name). Please uninstall it manually using Programs and Features in Control Panel."
 
 
!define INSTALLSTATE_DEFAULT "5"
!define INSTALLLEVEL_MAXIMUM "0xFFFF"
!define INSTALLSTATE_ABSENT "2"
!define ERROR_SUCCESS "0"




!macro sharedFunctions un

Function ${un}KillExe
    ${For} $R1 1 5
		DetailPrint "Attempting to kill: $0"
		${nsProcess::KillProcess} "$0" $R0
		IntCmp $R0 603 notRunning
		DetailPrint "Failed to kill $R0"
		Sleep 100
    ${Next}

	MessageBox MB_OK "The installer was unable to stop the program. Please stop it manually and try again."
	Abort
notRunning:
FunctionEnd

Function ${un}SkipPageIfSilent
	IfSilent 0 +2
	Abort
FunctionEnd



; push msi upgradeCode into this function
Function ${un}CallMsiDeinstaller
	Pop $0

	System::Call 'MSI::MsiEnumRelatedProducts(t "$0",i0,i r0,t.r1)i.r2'
	${If} $2 = 0
		DetailPrint product:$1
	${Else}
		Goto Done
	${Endif}

	System::Call "msi::MsiQueryProductStateA(t '$1') i.r0"
	StrCmp $0 "${INSTALLSTATE_DEFAULT}" 0 Done

	MessageBox MB_YESNO|MB_ICONQUESTION "${OLDVERSIONWARNING}" \
	IDNO BreakMe

	System::Call "msi::MsiConfigureProductA(t '$1', \
	i ${INSTALLLEVEL_MAXIMUM}, i ${INSTALLSTATE_ABSENT}) i.r0"
	StrCmp $0 ${ERROR_SUCCESS} Done

	MessageBox MB_OK|MB_ICONEXCLAMATION \
	"${OLDVERSIONREMOVEERROR}"

	BreakMe:
	Abort
	Done:
FunctionEnd




!macroend

!insertmacro sharedFunctions ""
!insertmacro sharedFunctions "un."
