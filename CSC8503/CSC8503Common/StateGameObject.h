/*			 Created By Samuel Buzz Appleby
 *               21/01/2021
 *                170348069
 *			State Game Object Definition	 */
#pragma once
#include "GameObject.h"
#include "StateMachine.h"
namespace NCL {
	namespace CSC8503 {
		enum class state { IDLE, FOLLOWROUTE, FOLLOWPATH, FOLLOWOBJECT };
		class StateMachine;
		class StateGameObject : public GameObject {
		public:
			StateGameObject();
			~StateGameObject();
			virtual void Update(float dt);
			string StateToString() const;
			void Idle();
		protected:
			state currentState;
			StateMachine* stateMachine;
			State* idleState;
		};
	}
}