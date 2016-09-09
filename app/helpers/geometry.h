/*
	Meshes
*/

#pragma once

#include <vector>
#include <tscore/maths.h>
#include <tsgraphics/indexbuffer.h>

namespace ts
{
	void generateCubeMesh(Vector halfextents, std::vector<Index>& indices, std::vector<Vector>& vertices, std::vector<Vector>& texcoords);
}
