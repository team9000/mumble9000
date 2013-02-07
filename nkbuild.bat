qmake CONFIG-=sse2 CONFIG+=no-asio CONFIG+=no-g15 CONFIG+=no-bonjour CONFIG+=no-elevation CONFIG+=packaged -recursive
nmake clean
nmake release
