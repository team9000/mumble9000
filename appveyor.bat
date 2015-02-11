SET MumbleVersion=1.1.%APPVEYOR_BUILD_NUMBER%

SET TOOLCHAIN=%~dp0\toolchain

SET VSVER=10.0
SET LIB=
REM SET QT_DIR=C:\Qt\Qt5.2.1\5.2.1\msvc2010
SET JOM_DIR=%TOOLCHAIN%\jom
SET VLD_DIR=%TOOLCHAIN%\vld
SET MYSQL_DIR=%TOOLCHAIN%\mysql
SET NASM_DIR=%TOOLCHAIN%\nasm
SET ICE_DIR=%TOOLCHAIN%\ice
SET PROTOBUF_DIR=%TOOLCHAIN%\protobuf
SET LIBSNDFILE_DIR=%TOOLCHAIN%\libsndfile
SET OPENSSL_DIR=%TOOLCHAIN%\openssl
SET BOOST_DIR=%TOOLCHAIN%\boost
SET GITBIN_DIR=%TOOLCHAIN%\bin
SET PERLBIN_DIR=%TOOLCHAIN%\perl\bin
SET ZLIB_DIR=%TOOLCHAIN%\zlib
SET DEBUGTOOLS_DIR=%TOOLCHAIN%\WDK\bin
SET CODESIGN_CERT=d:\jenkins\secure\code.p12
SET NSIS_DIR=%TOOLCHAIN%\nsis

REM SET QMAKESPEC=%QT_DIR%\mkspecs\win32-msvc2010

REM SET DXSDK_DIR=%TOOLCHAIN%\dxsdk\
REM SET WindowsSDKDir=%TOOLCHAIN%\win8sdk
REM CALL "%WindowsSDKDir%\bin\SetEnv.cmd"
REM SET PATH=%TOOLCHAIN%\win8sdk\bin\x86;C:\Windows\Microsoft.NET\Framework\v4.0.30319;%GITBIN_DIR%;%PERLBIN_DIR%;%QT_DIR%\bin;%JOM_DIR%;%OPENSSL_DIR%\bin;%LIBSNDFILE_DIR%\bin;%MYSQL_DIR%\lib;%ICE_DIR%\bin\vc100;%PROTOBUF_DIR%\vsprojects\Release;%NASM_DIR%;%VLD_DIR%\bin;%PATH%

rd /s /q release
mkdir release

qmake ^
	CONFIG-=sse2 ^
	CONFIG+=no-plugins CONFIG+=no-asio CONFIG+=no-g15 ^
	CONFIG+=no-bonjour CONFIG+=no-server ^
	CONFIG+=packaged -recursive
jom clean
jom -j4 release



signtool sign ^
	/f %CODESIGN_CERT% ^
	/t "http://timestamp.verisign.com/scripts/timstamp.dll" ^
	release/*.exe release/*.dll release/plugins/*.dll

del /q mumble9000_installer\Mumble9000_install.exe
del /q Mumble9000_install.exe

%NSIS_DIR%\Bin\makensis mumble9000_installer\mumble9000.nsi

move mumble9000_installer\Mumble9000_install.exe Mumble9000.%MumbleVersion%.exe

signtool sign ^
	/f %CODESIGN_CERT% ^
	/t "http://timestamp.verisign.com/scripts/timstamp.dll" ^
	Mumble9000.%MumbleVersion%.exe
