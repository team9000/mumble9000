@echo off
SET DIR=d:\mumble
SET VSVER=10.0
SET LIB=
SET QT_DIR=%DIR%\qt
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
CALL "%TOOLCHAIN%\env.bat"
CALL "%DXSDK_DIR%\Utilities\bin\dx_setenv.cmd" x86
CALL "%TOOLCHAIN%\VC\vcvarsall.bat" x86
SET PATH=%QT_DIR%\bin;%OPENSSL_DIR%\bin;%LIBSNDFILE_DIR%\bin;%MYSQL_DIR%\lib;%ICE_DIR%\bin\vc100;%PROTOBUF_DIR%;%NASM_DIR%;%VLD_DIR%\bin;%PATH%

TITLE Mumble Development Environment
