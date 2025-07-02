@echo off

set CPP_INCLUDE=..\include
set CPP_FLAGS=/W4 /Zi /MP /I %CPP_INCLUDE% /FC /nologo /wd4201 /EHsc /Fe"main" /D SIMD_WIDTH=4 /arch:AVX
set CPP_SOURCE=..\source\*.cpp

pushd build\

cl %CPP_FLAGS% %CPP_SOURCE% user32.lib gdi32.lib

REM clang++ ..\source\*.cpp -I ..\include -D SIMD_WIDTH=4 -mavx -march=haswell -luser32.lib -lgdi32.lib -O2 -g -o main.exe -Wno-endif-labels -fdiagnostics-absolute-paths

popd

