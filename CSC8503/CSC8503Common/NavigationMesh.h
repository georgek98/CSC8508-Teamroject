/*			Created By Rich Davison
 *			Edited By Samuel Buzz Appleby
 *               21/01/2021
 *                170348069
 *			Navigation Mesh Definition		 */
#pragma once
#include "NavigationMap.h"
#include <string>
#include <vector>
namespace NCL {
	namespace CSC8503 {
		class NavigationMesh : public NavigationMap {
		public:
			NavigationMesh();
			NavigationMesh(const std::string& filename);
			~NavigationMesh();

			bool FindPath(const Vector3& from, const Vector3& to, NavigationPath& outPath, bool ignoreCosts) override;

		protected:

			struct NavTri {
				NavTri* neighbours[3];

				NavTri() {
					neighbours[0] = nullptr;
					neighbours[1] = nullptr;
					neighbours[2] = nullptr;
				}
			};

			std::vector<NavTri>		allTris;
			std::vector<Vector3>	allVerts;
			std::vector<int>		allIndices;
		};
	}
}