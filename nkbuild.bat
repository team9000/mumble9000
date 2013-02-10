qmake ^
	CONFIG-=sse2 ^
	CONFIG+=no-plugins CONFIG+=no-asio CONFIG+=no-g15 ^
	CONFIG+=no-bonjour CONFIG+=no-server ^
	CONFIG+=packaged -recursive
rd /s /q release
mkdir release
nmake clean
nmake release
