set MumbleSourceDir=%~dp0
set MumbleQtDir=%QT_DIR%
set MumbleSndFileDir=%LIBSNDFILE_DIR%\bin
set MumbleNoMySQL=1
set MumbleNoIce=1
set MumbleOpenSslDir=%OPENSSL_DIR%
set MumbleZlibDir=%ZLIB_DIR%
set MumbleNoSSE2=
set MumbleNoG15=1
set MumbleDebugToolsDir=%DEBUGTOOLS_DIR%
set MumbleMergeModuleDir=%TOOLCHAIN%\mergemodules
set WixToolPath=%TOOLCHAIN%\wix
set WixTargetsPath=%WixToolPath%\Wix.targets
set WixTasksPath=%WixToolPath%\wixtasks.dll

signtool sign ^
	/f %CODESIGN_CERT% ^
	/t "http://timestamp.verisign.com/scripts/timstamp.dll" ^
	release/*.exe release/*.dll release/plugins/*.dll

cd installer
msbuild MumbleInstall.sln /p:Configuration=Release /t:Clean;Build
perl build_installer.pl
cd ..
del /q Mumble.msi
del /q Mumble*.msi
move installer\bin\Release\Mumble.msi Mumble9000.%MumbleVersion%.msi

signtool sign ^
	/f %CODESIGN_CERT% ^
	/t "http://timestamp.verisign.com/scripts/timstamp.dll" ^
	Mumble9000.%MumbleVersion%.msi
