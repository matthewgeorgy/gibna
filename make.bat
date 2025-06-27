@echo off

set CPP_INCLUDE=..\include
set CPP_FLAGS=/W4 /Zi /MP /I %CPP_INCLUDE% /FC /nologo /wd4201 /EHsc /Fe"main"
set CPP_SOURCE=..\source\*.cpp

pushd build\

cl %CPP_FLAGS% %CPP_SOURCE% user32.lib gdi32.lib

popd

