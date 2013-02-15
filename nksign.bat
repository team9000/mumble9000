signtool sign ^
	/f %CODESIGN_CERT% ^
	/t "http://timestamp.verisign.com/scripts/timstamp.dll" ^
	release/*.exe release/*.dll release/plugins/*.dll
