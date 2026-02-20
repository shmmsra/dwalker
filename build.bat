call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat" -arch=x64
cd "c:\Users\Administrator\Documents\AGAgents\dwalker\build"
"C:\Program Files\CMake\bin\cmake.exe" ..
"C:\Program Files\CMake\bin\cmake.exe" --build . --config Release
