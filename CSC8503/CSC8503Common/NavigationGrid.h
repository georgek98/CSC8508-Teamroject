/*			Created By Rich Davison
 *			Edited By Samuel Buzz Appleby
 *               21/01/2021
 *                170348069
 *			Navigation Grid Definition		 */
#pragma once
#include "NavigationMap.h"
#include <string>
#include <stack>

namespace NCL {
	namespace CSC8503 {
		/* Define what type of character we'll come across */
		const char WALL_NODE = 'x';
		const char FLOOR_NODE = '.';
		const char ICE_NODE = '-';
		const char SPRING_NODE = '~';
		const char COIN_NODE = 'c';
		const char POWERUP_NODE = 'p';
		struct GridNode {
			GridNode* parent;

			GridNode* connected[4];
			int		  costs[4];

			Vector3	position;
			float f;
			float g;

			int type;
			bool considered;
			GridNode() {
				for (int i = 0; i < 4; ++i) {
					connected[i] = nullptr;
					costs[i] = 0;
				}
				f = 0;
				g = 0;
				type = 0;
				parent = nullptr;
				considered = false;
			}
			~GridNode() {}
		};

		class NavigationGrid : public NavigationMap	{
		public:
			NavigationGrid();
			NavigationGrid(const std::string&filename);
			~NavigationGrid();

			bool FindPath(const Vector3& from, const Vector3& to, NavigationPath& outPath, bool ignoreCosts) override;
			GridNode* GetNodes(){
				return allNodes;
			}
		protected:
			bool		NodeInList(GridNode* n, std::stack<GridNode*>& list);
			GridNode*	RemoveBestNode(std::stack<GridNode*>& list);
			float		Heuristic(GridNode* hNode, GridNode* endNode) const;
			int nodeSize;
			int gridWidth;
			int gridHeight;

			GridNode* allNodes;
		};
	}
}

