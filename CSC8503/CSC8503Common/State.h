/*			  Created By Rich Davison
*			Edited By Samuel Buzz Appleby
 *               21/01/2021
 *                170348069
 *			State Implementation		 */
#pragma once
#include <functional>
namespace NCL {
	namespace CSC8503 {
		typedef std::function <void(float)> StateUpdateFunction;
		class State {
		public:
			State() {}
			State(StateUpdateFunction someFunc) {
				func = someFunc;
			}
			void Update(float dt) {
				if (func != nullptr) {
					func(dt);
				}
			}
		protected:
			StateUpdateFunction func;
		};
	}
}