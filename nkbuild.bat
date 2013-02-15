rd /s /q release
mkdir release

qmake ^
	CONFIG-=sse2 ^
	CONFIG+=no-plugins CONFIG+=no-asio CONFIG+=no-g15 ^
	CONFIG+=no-bonjour CONFIG+=no-server ^
	CONFIG+=packaged -recursive
jom clean
jom -j4 release

qmake ^
	CONFIG+=sse2 ^
	CONFIG+=no-plugins CONFIG+=no-asio CONFIG+=no-g15 ^
	CONFIG+=no-bonjour CONFIG+=no-server ^
	CONFIG+=packaged -recursive
jom -j4 release
