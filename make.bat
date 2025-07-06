@echo off

set CPP_INCLUDE=..\include
set CPP_FLAGS=/W4 /O2 /Zi /MP /I %CPP_INCLUDE% /FC /nologo /wd4201 /EHsc
set CPP_SOURCE=..\source\build.cpp

pushd build\

cl %CPP_FLAGS% /D SIMD_WIDTH=1 /arch:AVX /Fe"main_1x" %CPP_SOURCE% user32.lib gdi32.lib
REM cl %CPP_FLAGS% /D SIMD_WIDTH=4 /arch:AVX /Fe"main_4x" %CPP_SOURCE% user32.lib gdi32.lib
REM cl %CPP_FLAGS% /D SIMD_WIDTH=8 /arch:AVX /Fe"main_8x" %CPP_SOURCE% user32.lib gdi32.lib

REM clang++ ..\source\build.cpp -O2 -g -I ..\include -D SIMD_WIDTH=1 -mavx -march=haswell -luser32.lib -lgdi32.lib -o main_1x.exe -Wno-endif-labels -fdiagnostics-absolute-paths
REM clang++ ..\source\build.cpp -O2 -g -I ..\include -D SIMD_WIDTH=4 -mavx -march=haswell -luser32.lib -lgdi32.lib -o main_4x.exe -Wno-endif-labels -fdiagnostics-absolute-paths
REM clang++ ..\source\build.cpp -O2 -g -I ..\include -D SIMD_WIDTH=8 -mavx -march=haswell -luser32.lib -lgdi32.lib -o main_8x.exe -Wno-endif-labels -fdiagnostics-absolute-paths

popd

