/*         Created By Rich Davison,
		Edited by Samuel Buzz Appleby
 *               21/01/2021
 *                170348069
 *			Behaviour Action Definition			 */
#pragma once
#include "BehaviourNode.h"
#include <functional>
typedef std::function <BehaviourState(float, BehaviourState)> BehaviourActionFunc;
class BehaviourAction : public BehaviourNode {
public:
	BehaviourAction(const std::string& nodeName, BehaviourActionFunc f) : BehaviourNode(nodeName) {
		function = f; // sets our custom function !
	}
	BehaviourState Execute(float dt) override {
		currentState = function(dt, currentState); // calls it!
		return currentState;
	}
protected:
	BehaviourActionFunc function;
};
