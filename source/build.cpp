#define _CRT_SECURE_NO_WARNINGS
#define MG_IMPL
#define STB_IMAGE_IMPLEMENTATION
#include <mg.h>
#include <bitmap.h>
#include <fixed_point.h>
#include <simd.h>
#include <renderer.h>
#include <mesh.h>

#include "../source/bitmap.cpp"
#include "../source/renderer.cpp"
#include "../source/fixed_point.cpp"
#include "../source/mesh.cpp"
#include "../source/main.cpp"

#if (SIMD_WIDTH==1)
	#include "../source/simd_1x.cpp"
#elif (SIMD_WIDTH==4)
	#include "../source/simd_4x.cpp"
#else
	#include "../source/simd_8x.cpp"
#endif

