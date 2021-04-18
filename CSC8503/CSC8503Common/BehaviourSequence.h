/*         Created By Rich Davison,
		Edited by Samuel Buzz Appleby
 *               21/01/2021
 *                170348069
 *			Behaviour Sequence Definition			 */
#pragma once
#include "BehaviourNodeWithChildren.h"
class BehaviourSequence : public BehaviourNodeWithChildren {
public:
	BehaviourSequence(const std::string& nodeName) : BehaviourNodeWithChildren(nodeName) {}
	~BehaviourSequence() {}
	BehaviourState Execute(float dt) override {
		for (auto& i : childNodes) {
			BehaviourState nodeState = i->Execute(dt);
			switch (nodeState) {
			case Success: continue;
			case Failure:
			case Ongoing:
			{
				currentState = nodeState;
				return nodeState;
			}
			}
		}
		return Success;
	}
};