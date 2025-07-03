#define _CRT_SECURE_NO_WARNINGS
#include <mesh.h>

b32		
LoadMesh(mesh *Mesh,
		 char *Filename)
{
	FILE		*File;
	b32			Success = TRUE;
	array<v3>	Vertices;
	array<v3>	Normals;
	array<u32>	VertexIndices;
	array<u32>	NormalIndices;

	File = fopen(Filename, "r");
	if (File)
	{
		for (;;)
		{
			char LineHeader[128];
			s32 Res = fscanf(File, "%s", LineHeader);

			if (Res == EOF)
			{
				break;
			}

			if (strcmp(LineHeader, "v") == 0)
			{
				v3 Vertex;
				fscanf(File, "%f %f %f\n", &Vertex.x, &Vertex.y, &Vertex.z);
				Vertices.Push(Vertex);
			}
			else if (strcmp(LineHeader, "vn") == 0)
			{
				v3 Normal;
				fscanf(File, "%f %f %f\n", &Normal.x, &Normal.y, &Normal.z);
				Normals.Push(Normal);
			}
			else if (strcmp(LineHeader, "f") == 0)
			{
				u32 VertexIndex[3];
				u32 uvIndex[3];
				u32 NormalIndex[3];

				s32 Matches = fscanf(File, "%d/%d/%d %d/%d/%d %d/%d/%d\n",
						&VertexIndex[0], &uvIndex[0], &NormalIndex[0],
						&VertexIndex[1], &uvIndex[1], &NormalIndex[1],
						&VertexIndex[2], &uvIndex[2], &NormalIndex[2]);

				if (Matches != 9)
				{
					printf("failed to parse...!\r\n");
					exit(-1);
				}

				VertexIndices.Push(VertexIndex[0]);
				VertexIndices.Push(VertexIndex[1]);
				VertexIndices.Push(VertexIndex[2]);
				NormalIndices.Push(NormalIndex[0]);
				NormalIndices.Push(NormalIndex[1]);
				NormalIndices.Push(NormalIndex[2]);
			}
		}

		printf("v count: %u\n", Vertices.Len());
		printf("vn count: %u\n", Normals.Len());

		for (u32 I = 0; I < VertexIndices.Len(); I += 1)
		{
			u32 VertexIndex = VertexIndices[I] - 1;
			v3 Vertex = Vertices[VertexIndex];

			u32 NormalIndex = NormalIndices[I] - 1;
			v3 Normal = Normalize(Normals[NormalIndex]);

			if (Normal.x < 0) Normal.x *= -1;
			if (Normal.y < 0) Normal.y *= -1;
			if (Normal.z < 0) Normal.z *= -1;

			Mesh->Vertices.Push(Vertex);
			Mesh->Vertices.Push(Normal);
		}

		printf("len: %u\n", Mesh->Vertices.Len());

		/* for (u32 I = 0; I < VertexIndices.Len(); I += 1) */
		/* { */

		/* 	Mesh->Indices.Push(VertexIndex); */
		/* } */
	}
	else
	{
		printf("failed to open file..!\r\n");
		Success = FALSE;
	}

	return (Success);
}
