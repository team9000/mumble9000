rd /s /q release
mkdir release

qmake ^
	CONFIG-=sse2 ^
	CONFIG+=no-bundled-speex ^
	CONFIG+=no-plugins CONFIG+=no-asio CONFIG+=no-g15 ^
	CONFIG+=no-bonjour CONFIG+=no-server ^
	CONFIG+=packaged -recursive
nmake clean
nmake release

qmake ^
	CONFIG+=sse2 ^
	CONFIG+=no-bundled-speex ^
	CONFIG+=no-plugins CONFIG+=no-asio CONFIG+=no-g15 ^
	CONFIG+=no-bonjour CONFIG+=no-server ^
	CONFIG+=packaged -recursive
nmake release