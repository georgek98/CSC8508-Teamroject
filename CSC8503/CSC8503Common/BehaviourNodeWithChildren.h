/*         Created By Rich Davison,
		Edited by Samuel Buzz Appleby
 *               21/01/2021
 *                170348069
 *			Behaviour Node With Children Definition			 */
#pragma once
#include "BehaviourNode.h"
#include <vector>
class BehaviourNodeWithChildren : public BehaviourNode {
public:
	BehaviourNodeWithChildren(const std::string& nodeName) : BehaviourNode(nodeName) {};
	~BehaviourNodeWithChildren() {
		for (auto& i : childNodes) {
			delete i;
		}
	}
	void AddChild(BehaviourNode* n) {
		childNodes.emplace_back(n);
	}
	void Reset() override {
		currentState = Initialise;
		for (auto& i : childNodes) {
			i->Reset();
		}
	}
protected:
	std::vector < BehaviourNode* > childNodes;
};
