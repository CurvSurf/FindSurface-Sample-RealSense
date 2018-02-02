#include "sgeometry.h"


namespace sgeometry {
	using namespace smath;

	// Create a vertex array of sphere to be drawn in wireframe mode. (xyz only)
	// It will be a unit sphere located at the origin (0, 0, 0).
	void SphereData::generate() {
		/* reference: https://sites.google.com/site/ofauckland/examples/subdivide-a-tetrahedron-to-a-sphere */
		static std::function<void(int, int, int, int)> subdivide = [&](int i0, int i1, int i2, int level) {
			if (level-- > 0) {
				float3& p0 = vertices[i0];
				float3& p1 = vertices[i1];
				float3& p2 = vertices[i2];
				float3 p01 = Normalize(p0 + p1);
				float3 p12 = Normalize(p1 + p2);
				float3 p20 = Normalize(p2 + p0);

				vertices.push_back(p01);
				vertices.push_back(p12);
				vertices.push_back(p20);

				int i01 = int(vertices.size() - 3);
				int i12 = i01 + 1;
				int i20 = i12 + 1;

				subdivide(i0, i01, i20, level);
				subdivide(i01, i1, i12, level);
				subdivide(i01, i12, i20, level);
				subdivide(i20, i12, i2, level);
			}
			else {
				indices.push_back(i0);
				indices.push_back(i1);
				indices.push_back(i2);
			}
		};

		vertices.push_back({ 1.f, 0.f, 0.f });
		vertices.push_back({ -1.f, 0.f, 0.f });
		vertices.push_back({ 0.f, 1.f, 0.f });
		vertices.push_back({ 0.f, -1.f, 0.f });
		vertices.push_back({ 0.f, 0.f, 1.f });
		vertices.push_back({ 0.f, 0.f, -1.f });

		subdivide(2, 4, 0, level_of_details);
		subdivide(2, 0, 5, level_of_details);
		subdivide(2, 5, 1, level_of_details);
		subdivide(2, 1, 4, level_of_details);
		subdivide(3, 0, 4, level_of_details);
		subdivide(3, 5, 0, level_of_details);
		subdivide(3, 1, 5, level_of_details);
		subdivide(3, 4, 1, level_of_details);
	}

	int SphereData::LOD() { return level_of_details; }
	void SphereData::create() { create(level_of_details); }
	void SphereData::create(int LOD) { level_of_details = LOD; generate(); }

	void CylinderData::generate() {
		int RADIAL_SUBDIV = radial_subdivision;
		int LATERAL_SUBDIV = lateral_subdivision;
		
		float t = 1.f / 3.f;

		float unit_angle = PI / (RADIAL_SUBDIV / 2);
		for (int k = 0; k < RADIAL_SUBDIV; k++) {
			float angle = unit_angle*k;
			float c = cosf(angle);
			float s = sinf(angle);

			for (int s = 0; s < LATERAL_SUBDIV; s++) {
				float rs = float(s) / LATERAL_SUBDIV;
				float y = lerp(1.f, -1.f, rs);
				vertices.push_back({ c, y, s });
				int i00 = ;
				int i01;
				int i10;
				int i11;
			}
			vertices.push_back({ c, 1, s });
			vertices.push_back({ c, t, s });
			vertices.push_back({ c, -t, s });
			vertices.push_back({ c, -1, s });

			// indexing order
			// k    _ k+1			35   _  0
			// 00 |\ | 01			00 |\ | 01
			//    |_\|   			   |_\| 
			// 10 |\ | 11			10 |\ | 11
			//    |_\|   	...		   |_\| 
			// 20 |\ | 21			20 |\ | 21
			//    |_\|				   |_\|
			// 30      31			30		31
			//
			int i00 = k * 4;	int i01 = ((k + 1) % RADIAL_SUBDIV) * 4;
			int i10 = i00 + 1;	int i11 = i01 + 1;
			int i20 = i10 + 1;	int i21 = i11 + 1;
			int i30 = i20 + 1;	int i31 = i21 + 1;

			indices.push_back(i00); indices.push_back(i10); indices.push_back(i11);
			indices.push_back(i00); indices.push_back(i11); indices.push_back(i01);

			indices.push_back(i10); indices.push_back(i20); indices.push_back(i21);
			indices.push_back(i10); indices.push_back(i21); indices.push_back(i11);

			indices.push_back(i20); indices.push_back(i30); indices.push_back(i31);
			indices.push_back(i20); indices.push_back(i31); indices.push_back(i21);
		}
	}

	int CylinderData::RSUB() { return radial_subdivision; }
	int CylinderData::LSUB() { return lateral_subdivision; }
	void CylinderData::create() { create(radial_subdivision, lateral_subdivision); }
	void CylinderData::create(int RSUB, int LSUB) { radial_subdivision = RSUB; lateral_subdivision = LSUB; generate(); }

	
	// It's center will be located at the origin (0, 0, 0) and its length is 2 and radius is 1.
	void CreateCylinderVertexData(std::vector<float3>& vertices, std::vector<unsigned int>& indices) {

		
	}

	/* about terminology: https://en.wikipedia.org/wiki/Torus */
	// It's center will be located at the origin (0, 0, 0) and mean radius is 1 and tube radius is 0.5.
	void CreateTorusVertexData(std::vector<float3>& vertices, std::vector<unsigned int>& indices, int TSUB, int PSUB) {

		int TOROIDAL_SUBDIV = TSUB;
		int POLOIDAL_SUBDIV = PSUB;
		//vertices.clear(); vertices.reserve(TOROIDAL_SUBDIV*POLOIDAL_SUBDIV);
		//indices.clear(); indices.reserve(TOROIDAL_SUBDIV*POLOIDAL_SUBDIV * 6);
		//unsigned int base_index = unsigned int(vertices.size());

		float toroidal_unit_angle = PI / (TOROIDAL_SUBDIV / 2);
		float poloidal_unit_angle = PI / (POLOIDAL_SUBDIV / 2);

		float mean_radius = 1.0f;
		float tube_radius = 0.5f;
		for (int k = 0; k < TOROIDAL_SUBDIV; k++) {

			float phi = toroidal_unit_angle*float(k);
			float cphi = cosf(phi);
			float sphi = sinf(phi);

			for (int s = 0; s < POLOIDAL_SUBDIV; s++) {

				float theta = poloidal_unit_angle*float(s);
				float ctheta = cosf(theta);
				float stheta = sinf(theta);

				vertices.push_back({
					(mean_radius + tube_radius*ctheta)*cphi,
					tube_radius*stheta,
					-(mean_radius + tube_radius*ctheta)*sphi
				});

				// indexing order
				//		k    _  k+1
				//	s	00 |\ | 10
				//	s+1	01 |_\| 11
				//
				int i0_ = k*POLOIDAL_SUBDIV;
				int i1_ = ((k + 1) % TOROIDAL_SUBDIV)*POLOIDAL_SUBDIV;
				int i_0 = s;
				int i_1 = (s + 1) % POLOIDAL_SUBDIV;
				int i00 = i0_ + i_0;
				int i01 = i0_ + i_1;
				int i10 = i1_ + i_0;
				int i11 = i1_ + i_1;
				//int i10 = i00 - s + (s + 1) % POLOIDAL_SUBDIV;
				//int i01 = ((k + 1) % TOROIDAL_SUBDIV)*POLOIDAL_SUBDIV + s;
				//int i11 = i01 - s + (s + 1) % POLOIDAL_SUBDIV;

				indices.push_back(i00); indices.push_back(i01); indices.push_back(i11);
				indices.push_back(i00); indices.push_back(i11); indices.push_back(i10);
			}
		}
	}
}