/*			Created By Rich Davison
 *			Edited By Samuel Buzz Appleby
 *               21/01/2021
 *                170348069
 *			Navigation Grid Implementation		 */
#include "NavigationGrid.h"
#include "../../Common/Assets.h"

#include <fstream>

using namespace NCL;
using namespace CSC8503;

const int LEFT_NODE		= 0;
const int RIGHT_NODE	= 1;
const int TOP_NODE		= 2;
const int BOTTOM_NODE	= 3;

NavigationGrid::NavigationGrid()	{
	nodeSize	= 0;
	gridWidth	= 0;
	gridHeight	= 0;
	allNodes	= nullptr;
}

NavigationGrid::NavigationGrid(const std::string&filename) : NavigationGrid() {
	std::ifstream infile(Assets::DATADIR + filename);

	infile >> nodeSize;
	infile >> gridWidth;
	infile >> gridHeight;

	allNodes = new GridNode[gridWidth * gridHeight];

	for (int y = 0; y < gridHeight; ++y) {
		for (int x = 0; x < gridWidth; ++x) {
			GridNode&n = allNodes[(gridWidth * y) + x];
			char type = 0;
			infile >> type;
			n.type = type;
			n.position = Vector3((float)(x * nodeSize), 0, (float)(y * nodeSize));
		}
	}
	
	//now to build the connectivity between the nodes
	for (int y = 0; y < gridHeight; ++y) {
		for (int x = 0; x < gridWidth; ++x) {
			GridNode&n = allNodes[(gridWidth * y) + x];		

			if (y > 0) { //get the above node
				n.connected[0] = &allNodes[(gridWidth * (y - 1)) + x];
			}
			if (y < gridHeight - 1) { //get the below node
				n.connected[1] = &allNodes[(gridWidth * (y + 1)) + x];
			}
			if (x > 0) { //get left node
				n.connected[2] = &allNodes[(gridWidth * (y)) + (x - 1)];
			}
			if (x < gridWidth - 1) { //get right node
				n.connected[3] = &allNodes[(gridWidth * (y)) + (x + 1)];
			}
			for (int i = 0; i < 4; ++i) {
				/* Here we change our costs based on what we find */
				if (n.connected[i]) {
					if (n.connected[i]->type == FLOOR_NODE) {
						n.costs[i] = 10;
					}
					if (n.connected[i]->type == ICE_NODE) {
						n.costs[i] = 20;
					}
					if (n.connected[i]->type == SPRING_NODE) {
						n.costs[i] = 30;
					}
					if (n.connected[i]->type == WALL_NODE) {
						n.connected[i] = nullptr; //actually a wall, disconnect!
					}
				}
			}
		}	
	}
}

NavigationGrid::~NavigationGrid()	{
	delete[] allNodes;
}

bool NavigationGrid::FindPath(const Vector3& from, const Vector3& to, NavigationPath& outPath, bool ignoreCosts) {
	//need to work out which node 'from' sits in, and 'to' sits in
	int fromX = ((int)from.x / nodeSize);
	int fromZ = ((int)from.z / nodeSize);

	int toX = ((int)to.x / nodeSize);
	int toZ = ((int)to.z / nodeSize);

	if (fromX < 0 || fromX > gridWidth - 1 ||
		fromZ < 0 || fromZ > gridHeight - 1) {
		return false; //outside of map region!
	}

	if (toX < 0 || toX > gridWidth - 1 ||
		toZ < 0 || toZ > gridHeight - 1) {
		return false; //outside of map region!
	}

	GridNode* startNode = &allNodes[(fromZ * gridWidth) + fromX];
	GridNode* endNode	= &allNodes[(toZ * gridWidth) + toX];

	std::stack<GridNode*> openList;

	openList.push(startNode);

	startNode->f = 0;
	startNode->g = 0;
	startNode->parent = nullptr;

	GridNode* currentBestNode = nullptr;
	while (!openList.empty()) {
		currentBestNode = RemoveBestNode(openList);

		if (currentBestNode == endNode) {			//we've found the path!
			GridNode* node = endNode;
			while (node != nullptr) {
				outPath.PushWaypoint(node->position);
				node = node->parent;
			}
			return true;
		}
		else {
			for (int i = 0; i < 4; ++i) {
				GridNode* neighbour = currentBestNode->connected[i];
				if (!neighbour || neighbour->considered) { //might not be connected or has been considered
					continue;
				}

				float h = Heuristic(neighbour, endNode);
				float g = ignoreCosts ? g = currentBestNode->g + 1 : g = currentBestNode->g + currentBestNode->costs[i];
				float f = h + g;
				bool inOpen	= NodeInList(neighbour, openList);

				if (!inOpen) { //first time we've seen this neighbour
					openList.push(neighbour);
				}
				if (!inOpen || f < neighbour->f) {//might be a better route to this neighbour
					neighbour->parent = currentBestNode;
					neighbour->f = f;
					neighbour->g = g;
				}
			}
			currentBestNode->considered = true;
		}
	}
	return false; //open list emptied out with no path!
}

bool NavigationGrid::NodeInList(GridNode* n, std::stack<GridNode*>& list) {
	std::stack<GridNode*> newList = list;
	while (!newList.empty()) {
		GridNode* next = newList.top();
		newList.pop();
		if (next == n) 
			return true;
	}
	return false;
}

GridNode* NavigationGrid::RemoveBestNode(std::stack<GridNode*>& list) {
	std::stack<GridNode*> newList;
	GridNode* bestNode = list.top();
	list.pop();
	newList.push(bestNode);
	while (!list.empty()) {
		GridNode* next = list.top();
		list.pop();
		if (next->f < bestNode->f) 
			bestNode = next;
		newList.push(next);
	}
	while (!newList.empty()) {
		GridNode* next = newList.top();
		newList.pop();
		if(bestNode != next)
			list.push(next);
	}
	return bestNode;
}

float NavigationGrid::Heuristic(GridNode* hNode, GridNode* endNode) const {
	return (hNode->position - endNode->position).Length();
}