signtool sign ^
	/f %CODESIGN_CERT% ^
	/t "http://timestamp.verisign.com/scripts/timstamp.dll" ^
	release/*.exe release/*.dll release/plugins/*.dll

del /q mumble9000_installer\Mumble9000_install.exe
del /q Mumble9000_install.exe

%NSIS_DIR%\Bin\makensis mumble9000_installer\mumble9000.nsi

move mumble9000_installer\Mumble9000_install.exe Mumble9000.%MumbleVersion%.exe
