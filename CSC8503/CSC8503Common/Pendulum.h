#pragma once
#include "GameObject.h"
#include "tweeny/tweeny.h"
#include "../Common/Maths.h"
namespace NCL
{
	namespace CSC8503
	{
		class Pendulum :public GameObject
		{
		public:
			Pendulum(float timeToSwing, bool isSwingingLeft = true);
			~Pendulum() {
				delete &tween;
			}
			void FixedUpdate(float fixedDT) override;

		private:
			float speed;
			float angle;
			bool isSwingingLeft;
			tweeny::tween<float> tween;
		};
	}
}
