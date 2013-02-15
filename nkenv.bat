@echo off

SET MumbleVersion=1.0.7

SET DIR=d:\mumble
SET VSVER=10.0
SET LIB=
SET QT_DIR=%DIR%\qtmumble
SET MUMBLE_DIR=%DIR%\mumble
SET VLD_DIR=%DIR%\vld
SET MYSQL_DIR=%DIR%\mysql
SET NASM_DIR=%DIR%\nasm
SET ICE_DIR=%DIR%\ice
SET PROTOBUF_DIR=%DIR%\protobuf
SET LIBSNDFILE_DIR=%DIR%\libsndfile
SET OPENSSL_DIR=%DIR%\openssl
SET BOOST_DIR=%DIR%\boost
SET TOOLCHAIN=d:\chrome\win_toolchain
SET GITBIN_DIR=d:\chrome\depot_tools
SET PERLBIN_DIR=%DIR%\perl\bin
SET ZLIB_DIR=%DIR%\zlib
SET DEBUGTOOLS_DIR=%TOOLCHAIN%\WDK\bin
SET CODESIGN_CERT=d:\jenkins\secure\code.p12

SET OLDDIR=%CD%
CALL "%TOOLCHAIN%\env.bat"
::Set Lib=%DXSDK_DIR%\Lib\x86;%Lib%
::Set Include=%DXSDK_DIR%\Include;%Include%
SET DXSDK_DIR=%DXSDK_DIR%\
CALL "%TOOLCHAIN%\VC\vcvarsall.bat" x86
SET PATH=%TOOLCHAIN%\win8sdk\bin\x86;%GITBIN_DIR%;%PERLBIN_DIR%;%QT_DIR%\bin;%OPENSSL_DIR%\bin;%LIBSNDFILE_DIR%\bin;%MYSQL_DIR%\lib;%ICE_DIR%\bin\vc100;%PROTOBUF_DIR%\vsprojects\Release;%NASM_DIR%;%VLD_DIR%\bin;%PATH%

chdir /d %OLDDIR%
TITLE Mumble Development Environment
