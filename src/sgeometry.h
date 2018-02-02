#pragma once
#include <vector>
#include <functional>
#include "smath.h"

namespace sgeometry {
	using namespace smath;

	// Create a vertex array of sphere to be drawn in wireframe mode. (xyz only)
	// It will be a unit sphere located at the origin (0, 0, 0).
	struct GeometryData {
		std::vector<float3> vertices;
		std::vector<unsigned int> indices;

		virtual void create() = 0;
	};

	class SphereData : GeometryData {
		int level_of_details = 3;
		void generate();

	public:
		int LOD();
		void create();
		void create(int LOD);
	};
	
	class CylinderData : GeometryData {
		int radial_subdivision = 10;
		int lateral_subdivision = 3;
		void generate();

	public:
		int RSUB();
		int LSUB();
		void create();
		void create(int RSUB, int LSUB);
	};
	// It's center will be located at the origin (0, 0, 0) and its length is 2 and radius is 1.
	void CreateCylinderVertexData(std::vector<float3>& vertices, std::vector<unsigned int>& indices);

	/* about terminology: https://en.wikipedia.org/wiki/Torus */
	// It's center will be located at the origin (0, 0, 0) and mean radius is 1 and tube radius is 0.5.
	void CreateTorusVertexData(std::vector<float3>& vertices, std::vector<unsigned int>& indices, int TSUB = 36, int PSUB = 10);
}