#pragma once
#include<vector>
#include "Cannonball.h"

namespace NCL
{
	namespace CSC8503
	{

		class Obstacles
		{
		public:
			vector<Cannonball*> cannons;

			~Obstacles()
			{
				//delete cannons;
			}

			void ClearObstacles() {
				cannons.clear();
			}
		protected:

		};
	}
}

