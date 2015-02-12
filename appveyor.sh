#!/bin/bash
set -e; set -u
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

MumbleVersion=1.1.$APPVEYOR_BUILD_NUMBER

TOOLCHAIN=$DIR/toolchain

VSVER=10.0
LIB=
QT_DIR=c:/Qt/5.4/msvc2013_opengl
JOM_DIR=$TOOLCHAIN/jom
VLD_DIR=$TOOLCHAIN/vld
MYSQL_DIR=$TOOLCHAIN/mysql
NASM_DIR=$TOOLCHAIN/nasm
ICE_DIR=$TOOLCHAIN/ice
PROTOBUF_DIR=$TOOLCHAIN/protobuf
LIBSNDFILE_DIR=$TOOLCHAIN/libsndfile
OPENSSL_DIR=$TOOLCHAIN/openssl
BOOST_DIR=$TOOLCHAIN/boost
ZLIB_DIR=$TOOLCHAIN/zlib
DEBUGTOOLS_DIR=$TOOLCHAIN/WDK/bin
CODESIGN_CERT=d:/jenkins/secure/code.p12
NSIS_DIR=$TOOLCHAIN/nsis

# QMAKESPEC=%QT_DIR%/mkspecs/win32-msvc2010

# DXSDK_DIR=$TOOLCHAIN/dxsdk/
# WindowsSDKDir=$TOOLCHAIN/win8sdk
# CALL "%WindowsSDKDir%/bin/SetEnv.cmd"
# PATH=$TOOLCHAIN/win8sdk/bin/x86;C:/Windows/Microsoft.NET/Framework/v4.0.30319;%QT_DIR%/bin;%JOM_DIR%;%OPENSSL_DIR%/bin;%LIBSNDFILE_DIR%/bin;%MYSQL_DIR%/lib;%ICE_DIR%/bin/vc100;%PROTOBUF_DIR%/vsprojects/Release;%NASM_DIR%;%VLD_DIR%/bin;%PATH%

PATH=$QT_DIR/bin

rm -Rf release
mkdir release

qmake \
	CONFIG-=sse2 \
	CONFIG+=no-plugins CONFIG+=no-asio CONFIG+=no-g15 \
	CONFIG+=no-bonjour CONFIG+=no-server \
	CONFIG+=packaged -recursive

jom clean
jom -j4 release

signtool sign \
	/f %CODESIGN_CERT% \
	/t "http://timestamp.verisign.com/scripts/timstamp.dll" \
	release/*.exe release/*.dll release/plugins/*.dll

rm -f mumble9000_installer/Mumble9000_install.exe
rm -f Mumble9000_install.exe

%NSIS_DIR%/Bin/makensis mumble9000_installer/mumble9000.nsi

signtool sign \
/f %CODESIGN_CERT% \
/t "http://timestamp.verisign.com/scripts/timstamp.dll" \
mumble9000_installer/Mumble9000_install.exe

mv mumble9000_installer/Mumble9000_install.exe Mumble9000.$MumbleVersion.exe
