set MumbleSourceDir=%~dp0
set MumbleQtDir=%QT_DIR%
set MumbleSndFileDir=%LIBSNDFILE_DIR%\bin
set MumbleNoMySQL=1
set MumbleNoIce=1
set MumbleOpenSslDir=%OPENSSL_DIR%
set MumbleZlibDir=%ZLIB_DIR%
set MumbleNoSSE2=1
set MumbleNoG15=1
set MumbleDebugToolsDir=%DEBUGTOOLS_DIR%
set MumbleMergeModuleDir=%DIR%\mergemodules
set WixToolPath=%DIR%\wix
set WixTargetsPath=%WixToolPath%\Wix.targets
set WixTasksPath=%WixToolPath%\wixtasks.dll
cd installer
::msbuild MumbleInstall.sln /p:Configuration=Release /t:Clean;Build
::perl build_installer.pl
cd ..
del Mumble.msi
move installer\bin\Release\Mumble.msi Mumble.msi
