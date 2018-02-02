#pragma once
#include <vector>
#include <functional>
#include "smath.h"

namespace sgeometry {
	using namespace smath;

	// Create a vertex array of sphere to be drawn in wireframe mode. (xyz only)
	// It will be a unit sphere located at the origin (0, 0, 0).
	void CreateSphereVertexData(std::vector<float3>& vertices, std::vector<unsigned int>& indices, int LOD = 4);

	// Its center will be located at the origin (0, 0, 0) and its length is 2 and radius is 1.
	void CreateCylinderVertexData(std::vector<float3>& vertices, std::vector<unsigned int>& indices);

	/* about terminology: https://en.wikipedia.org/wiki/Torus */
	// Its center will be located at the origin (0, 0, 0) and mean radius is 1 and tube radius is 0.5.
	// Its axis is (0, 1, 0)
	void CreateTorusVertexData(std::vector<float3>& vertices, std::vector<unsigned int>& indices);
}