/*
	Meshes
*/

#include "meshes.h"

using namespace std;

namespace ts
{
	void generateCubeMesh(Vector halfextents, vector<Index>& indices, vector<Vector>& vertices,vector<Vector>& texcoords)
	{
		float x = halfextents.x() * 2;
		float y = halfextents.y() * 2;
		float z = halfextents.z() * 2;

		indices = vector<Index>{
			0,2,1,
			0,3,2,

			1,2,6,
			6,5,1,

			4,5,6,
			6,7,4,

			2,3,6,
			6,3,7,

			0,7,3,
			0,4,7,

			0,1,5,
			0,5,4
		};
		
		texcoords = vector<Vector>{

			{ 0, 1 },
			{ 1, 1 },
			{ 1, 0 },
			{ 0, 0 },

			// { 0, 1 },
			// { 1, 1 },
			// { 1, 0 },
			// { 0, 0 },
			
			{ 1, 1 },
			{ 0, 1 },
			{ 0, 0 },
			{ 1, 0 },
		};

		vertices = vector<Vector>{
			{ -x, -y, -z, }, // 0
			{  x, -y, -z, }, // 1
			{  x,  y, -z, }, // 2
			{ -x,  y, -z, }, // 3
			{ -x, -y,  z, }, // 4
			{  x, -y,  z, }, // 5
			{  x,  y,  z, }, // 6
			{ -x,  y,  z  }, // 7
		};
	}
}
