/*			Created By Rich Davison
 *			Edited By Samuel Buzz Appleby
 *               21/01/2021
 *                170348069
 *			Tutorial Game definition		 */
#pragma once
#include "../CSC8503Common/PhysxConversions.h"
#include "../CSC8503Common/CollisionDetection.h"
#include "../GameTech/GameManager.h"

namespace NCL
{
	namespace CSC8503
	{
		const int IDEAL_FRAMES = 240;
		const float IDEAL_DT = 1.0f / IDEAL_FRAMES;
		class PlayerObject;
		enum class FinishType { INGAME, TIMEOUT, WIN, LOSE };
		class LevelCreator
		{
		public:
			LevelCreator();
			~LevelCreator() {}

			void ResetWorld();

			virtual void Update(float dt);
			void UpdateCamera(float dt);
			virtual void UpdateTimeStep(float dt);

			void FixedUpdate(float dt);

			void UpdateLevel(float dt);
			void UpdatePlayer(float dt);
			void InitPlayer(const PxTransform& t, const PxReal scale);
			void InitWorld(LevelState state);

		protected:
			float dTOffset;
			int realFrames;
			float fixedDeltaTime;
			bool isSinglePlayer;
			void InitCamera();

			void InitFloors(LevelState state);
			void InitGameExamples(LevelState state);
			void InitGameObstacles(LevelState state);
			void InitGameMusic(LevelState state);

			bool SelectObject();
			void DebugObjectMovement();
		};
	}
}

