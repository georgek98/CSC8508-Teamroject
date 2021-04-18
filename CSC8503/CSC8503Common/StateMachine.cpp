/*			  Created By Rich Davison
*			Edited By Samuel Buzz Appleby
 *               21/01/2021
 *                170348069
 *			State Machine Implementation		 */
#include "StateMachine.h"
using namespace NCL::CSC8503;
StateMachine::StateMachine() {
	activeState = nullptr;
}
StateMachine ::~StateMachine() {
	for (auto& i : allStates) {
		delete i;
	}
	for (auto& i : allTransitions) {
		delete i.second;
	}
}
void StateMachine::AddState(State* s) {
	allStates.emplace_back(s);
	if (activeState == nullptr) {
		activeState = s; // make this the default entry state !
	}
}
void StateMachine::AddTransition(StateTransition * t) {
	allTransitions.insert(std::make_pair(t->GetSourceState(), t));
}
void StateMachine::Update(float dt) {
	if (activeState) {
		activeState->Update(dt);
		// Get the transition set starting from this state node ;
		std::pair<TransitionIterator, TransitionIterator> range = allTransitions.equal_range(activeState);
		// Iterate through them all
		for (auto& i = range.first; i != range.second; ++i) {
			if (i->second->CanTransition()) { // some transition is true !
				State * newState = i->second->GetDestinationState();
				activeState = newState;
			}
		}
	}
}