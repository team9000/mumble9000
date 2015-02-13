#!/bin/bash
set -e; set -u
ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
DL="1"
if [ -n "${NODL-}" ]; then DL=""; fi

VERSION="1.1.$APPVEYOR_BUILD_NUMBER"

TOOLCHAIN="$ROOT/toolchain"
TMP="$ROOT/tmp"
if [ "$DL" ]; then rm -Rf "$TOOLCHAIN"; fi
rm -Rf "$TMP"
mkdir -p "$TOOLCHAIN" "$TMP"

function download {
	local URL="$1"
	echo "Downloading $URL" >&2
	local MD5="$(echo -n "$URL" | md5sum | cut -f1 -d" ")"
	local CACHEDIR
	if [ -n "${APPVEYOR-}" ]; then
		CACHEDIR="/c/cygdrive/cache"
	else
		CACHEDIR="$ROOT/cache"
	fi
	mkdir -p "$CACHEDIR"
	local CACHEFILE="$CACHEDIR/$MD5"
	if [ -f "$CACHEFILE" ]; then
		echo "(From Cache)" >&2
		echo "$CACHEFILE"
		return
	fi
	local TMPFILE="$TMP/$$.$RANDOM"
	wget "$URL" -q -O "$TMPFILE"
	mv "$TMPFILE" "$CACHEFILE"
	echo "$CACHEFILE"
}

# Prep
INCLUDE=""
LIB=""

echo " --- SETTING UP ENVIRONMENT ---"

# Cygwin Utils
# preinstalled: git,wget,curl,perl
if [ -n "${APPVEYOR-}" ]; then
	echo "Downloading cygwin utils"
	"/cygdrive/c/cygwin/setup-x86.exe" \
		-q -n -O -R C:/cygwin -s http://cygwin.mirror.constant.com \
		-l C:/cygwin/var/cache/setup \
		-P unzip \
		>/dev/null
fi

# Visual Studio (Pre-Installed)
VS_DIR="/cygdrive/c/Program Files (x86)/Microsoft Visual Studio 12.0"
PATH="$VS_DIR/VC/bin:$PATH"
PATH="$VS_DIR/VC/vcpackages:$PATH"
PATH="/cygdrive/c/Program Files (x86)/MSBuild/12.0/Bin:$PATH"
INCLUDE="$(cygpath -w "$VS_DIR/VC/include");$INCLUDE"
LIB="$(cygpath -w "$VS_DIR/VC/lib");$LIB"

# Windows SDK (Pre-Installed)
WINSDK_DIR="/cygdrive/c/Program Files (x86)/Windows Kits/8.1"
PATH="$WINSDK_DIR/bin/x86:$PATH"
INCLUDE="$(cygpath -w "$WINSDK_DIR/Include/um");$INCLUDE"
INCLUDE="$(cygpath -w "$WINSDK_DIR/Include/shared");$INCLUDE"
LIB="$(cygpath -w "$WINSDK_DIR/Lib/winv6.3/um/x86");$LIB"

# DirectX SDK (Pre-Installed)
DXSDK_DIR="C:\\Program Files (x86)\\Microsoft DirectX SDK\\"
INCLUDE="$(cygpath -w "$DXSDK_DIR/Include");$INCLUDE"
LIB="$(cygpath -w "$DXSDK_DIR/Lib/x86");$LIB"

# Qt (Pre-Installed)
QT_DIR="/cygdrive/c/Qt/5.4/msvc2013_opengl"
PATH="$QT_DIR/bin:$PATH"

# Jom
JOM_DIR="$TOOLCHAIN/jom"
if [ "$DL" ]; then
	JOM_ZIP="$(download "http://download.qt-project.org/official_releases/jom/jom.zip")"
	unzip -qo "$JOM_ZIP" -d "$JOM_DIR"
fi
PATH="$JOM_DIR:$PATH"
chmod +x "$JOM_DIR/jom.exe"

# Protobuf Binary
PROTOBUF_BIN_DIR="$TOOLCHAIN/protobuf-bin"
if [ "$DL" ]; then
	PROTOBUF_BIN_ZIP="$(download "https://ci.sites.team9000.net/protobuf-bin.zip")"
	unzip -qo "$PROTOBUF_BIN_ZIP" -d "$PROTOBUF_BIN_DIR"
fi
chmod +x "$PROTOBUF_BIN_DIR/protoc.exe"
PATH="$PROTOBUF_BIN_DIR:$PATH"
LIB="$(cygpath -w "$PROTOBUF_BIN_DIR");$LIB"

# Protobuf Lib
PROTOBUF_LIB_DIR="$TOOLCHAIN/protobuf"
if [ "$DL" ]; then
	PROTOBUF_LIB_TAR="$(download "https://protobuf.googlecode.com/files/protobuf-2.5.0.tar.gz")"
	mkdir "$PROTOBUF_LIB_DIR"
	tar -xz --strip-components=1 -C "$PROTOBUF_LIB_DIR" -f "$PROTOBUF_LIB_TAR"
fi
INCLUDE="$(cygpath -w "$PROTOBUF_LIB_DIR/src");$INCLUDE"

# Openssl Source
OPENSSL_SRC_DIR="$TOOLCHAIN/openssl-src"
if [ "$DL" ]; then
	OPENSSL_SRC_TAR="$(download "https://www.openssl.org/source/openssl-1.0.2.tar.gz")"
	mkdir "$OPENSSL_SRC_DIR"
	tar -xz --strip-components=1 -C "$OPENSSL_SRC_DIR" -f "$OPENSSL_SRC_TAR"
	# retar it and dereference the symlinks :(
	tar -ch -C "$OPENSSL_SRC_DIR" -f "$TMP/openssl.tar" .
	rm -Rf "$OPENSSL_SRC_DIR"
	mkdir "$OPENSSL_SRC_DIR"
	tar -x -C "$OPENSSL_SRC_DIR" -f "$TMP/openssl.tar"
fi
INCLUDE="$(cygpath -w "$OPENSSL_SRC_DIR/include");$INCLUDE"

# Openssl Lib
OPENSSL_LIB_DIR="$TOOLCHAIN/openssl-lib"
if [ "$DL" ]; then
	OPENSSL_LIB_ZIP="$(download "https://ci.sites.team9000.net/openssl-lib-b.zip")"
	unzip -qo "$OPENSSL_LIB_ZIP" -d "$OPENSSL_LIB_DIR"
fi
LIB="$(cygpath -w "$OPENSSL_LIB_DIR");$LIB"

# Sndfile
SNDFILE_DIR="$TOOLCHAIN/sndfile"
if [ "$DL" ]; then
	SNDFILE_ZIP="$(download "https://ci.sites.team9000.net/libsndfile.zip")"
	unzip -qo "$SNDFILE_ZIP" -d "$SNDFILE_DIR"
fi
INCLUDE="$(cygpath -w "$SNDFILE_DIR/include");$INCLUDE"
LIB="$(cygpath -w "$SNDFILE_DIR/lib");$LIB"

# Boost (Pre-Installed boost_1_56_0-msvc-12.0-32.exe)
BOOST_DIR="/cygdrive/c/Libraries/boost"
INCLUDE="$INCLUDE;$(cygpath -w "$BOOST_DIR")"
LIB="$LIB;$(cygpath -w "$BOOST_DIR/lib32-msvc-12.0")"

# NSIS
NSIS_DIR="$TOOLCHAIN/nsis"
if [ "$DL" ]; then
	NSIS_ZIP="$(download "https://ci.sites.team9000.net/NSIS.zip")"
	unzip -qo "$NSIS_ZIP" -d "$NSIS_DIR"
fi
NSIS_DIR=("$NSIS_DIR"/*)
chmod +x "$NSIS_DIR/makensis.exe"
chmod +x "$NSIS_DIR/Bin/makensis.exe"
chmod +x "$NSIS_DIR/Bin/zlib1.dll"
PATH="$NSIS_DIR/Bin:$PATH"

rm -Rf release
mkdir release

if [ -n "${APPVEYOR-}" ]; then
	echo "Initing git submodules"
	git submodule update --init --recursive
fi

export INCLUDE
export LIB
export DXSDK_DIR

echo " --- QMAKE ---"

MumbleVersion="$VERSION" \
qmake \
	CONFIG-=sse2 \
	CONFIG+=no-plugins CONFIG+=no-asio CONFIG+=no-g15 \
	CONFIG+=no-bonjour CONFIG+=no-server \
	CONFIG+=no-elevation \
	CONFIG+=packaged -recursive

echo " --- COMPILE ---"

jom clean
jom -j8 release

echo " --- SIGNING ---"

#signtool sign \
#/f %CODESIGN_CERT% \
#/t "http://timestamp.verisign.com/scripts/timstamp.dll" \
#release/*.exe release/*.dll release/plugins/*.dll

rm -f mumble9000_installer/Mumble9000_install.exe
rm -f Mumble9000_install.exe

echo " --- BUILDING INSTALLER ---"



LIBSNDFILE_DIR="$(cygpath -w "$SNDFILE_DIR")" \
QT_DIR="$(cygpath -w "$QT_DIR")" \
OPENSSL_DIR="$(cygpath -w "$OPENSSL_LIB_DIR")" \
VS_DIR="$(cygpath -w "$VS_DIR")" \
makensis mumble9000_installer/mumble9000.nsi

echo " --- SIGNING INSTALLER ---"

#signtool sign \
#/f %CODESIGN_CERT% \
#/t "http://timestamp.verisign.com/scripts/timstamp.dll" \
#mumble9000_installer/Mumble9000_install.exe

mv mumble9000_installer/Mumble9000_install.exe "Mumble9000.$VERSION.exe"
