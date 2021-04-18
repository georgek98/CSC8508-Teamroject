/*         Created By Rich Davison,
		Edited by Samuel Buzz Appleby
 *               21/01/2021
 *                170348069
 *			Behaviour Selector Definition			 */
#pragma once
#include "BehaviourNodeWithChildren.h"
class BehaviourSelector : public BehaviourNodeWithChildren {
public:
	BehaviourSelector(const std::string& nodeName) : BehaviourNodeWithChildren(nodeName) {}
	~BehaviourSelector() {}
	BehaviourState Execute(float dt) override {
		for (auto& i : childNodes) {
			BehaviourState nodeState = i->Execute(dt);
			switch (nodeState) {
			case Failure: continue;
			case Success:
			case Ongoing:
			{
				currentState = nodeState;
				return currentState;
			}
			}
		}
		return Failure;
	}
};
