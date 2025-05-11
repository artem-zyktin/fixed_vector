set PREMAKE=tools/premake/bin/win/premake5.exe
set makefile=makefiles/premake5.lua
set generator=vs2022

"%PREMAKE%" --file="%makefile%" "%generator%"