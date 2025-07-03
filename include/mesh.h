#ifndef __MESH_H__
#define __MESH_H__

#include <mg.h>
#include <stdio.h>
#include <string.h>

struct mesh
{
	array<v3>		Vertices;
	/* array<u32>		Indices; */
};

b32		LoadMesh(mesh *Mesh, char *Filename);

#endif // __MESH_H__

