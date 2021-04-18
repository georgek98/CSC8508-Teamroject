/*			  Created By Rich Davison
*			Edited By Samuel Buzz Appleby
 *               21/01/2021
 *                170348069
 *			State Machine Definition		 */
#include <map>
#include <vector>
#include "State.h"
#include "StateTransition.h"
namespace NCL {
	namespace CSC8503 {
		class State; // Predeclare the classes we need
		class StateTransition;
		// Typedefs to make life easier !
		typedef std::multimap<State*, StateTransition*> TransitionContainer;
		typedef TransitionContainer::iterator TransitionIterator;
		class StateMachine {
			public:
				StateMachine();
				~StateMachine();
				void AddState(State * s);
				void AddTransition(StateTransition * t);
				void Update(float dt);
			protected:
				State* activeState;
				std::vector<State*> allStates;
				TransitionContainer allTransitions;
		};
	}
}
