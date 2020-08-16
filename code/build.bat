#echo

mkdir ..\build
pushd ..\build

call "c:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

cl -Zi d:\mario\code\win32_platform.cpp user32.lib gdi32.lib opengl32.lib				

popd		
