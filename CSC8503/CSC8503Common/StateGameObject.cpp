/*			 Created By Samuel Buzz Appleby
 *               21/01/2021
 *                170348069
 *			State Game Object Implementation	 */
#include "StateGameObject.h"
using namespace NCL;
using namespace CSC8503;
StateGameObject::StateGameObject() {
	stateMachine = new StateMachine();
	currentState = state::IDLE;
	idleState = new State([&](float dt)->void {
		this->Idle();
	});
	stateMachine->AddState(idleState);
}

StateGameObject::~StateGameObject() {
	delete stateMachine;
}

void StateGameObject::Update(float dt) {
	stateMachine->Update(dt);
}

string StateGameObject::StateToString() const {
	switch (currentState) {
		case state::IDLE:
			return "Idling";
		case state::FOLLOWROUTE:
			return "Following Route";
		case state::FOLLOWPATH:
			return "Following A* Path";
		case state::FOLLOWOBJECT:
			return "Following Raycasted Object";
		default:
			return "State Not Known";
	}
}

void StateGameObject::Idle() {
	//GetPhysicsObject()->ClearForces();
}