/*			  Created By Rich Davison
*			Edited By Samuel Buzz Appleby
 *               21/01/2021
 *                170348069
 *			Game World Definition		 */
#pragma once
#include <set>
#include <vector>
#include <functional>
#include "GameObject.h"
#include "../../Common/Camera.h"
#include <algorithm>
#include <random>
#include "Debug.h"
#include "../../Common/Camera.h"
#include "../../Common/Vector2.h"
#include "../../Common/Vector3.h"
#include "../../Common/TextureLoader.h"
#include "../../Common/Matrix4.h"
#include "../../Common/Light.h"
namespace NCL
{
	class Camera;
	namespace CSC8503
	{
		class GameObject;
		typedef std::function<void(GameObject*)> GameObjectFunc;
		typedef std::vector<GameObject*>::const_iterator GameObjectIterator;
		class GameWorld : public PxSimulationEventCallback
		{
		public:
			GameWorld();
			~GameWorld();

			/* PhysX callback methods */
			void onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs) override
			{
				PxActor* actor1 = pairHeader.actors[0];
				GameObject* obj1 = FindObjectFromPhysicsBody(actor1);

				PxActor* actor2 = pairHeader.actors[1];
				GameObject* obj2 = FindObjectFromPhysicsBody(actor2);

				if (obj1 && obj2)
				{
					for (PxU32 i = 0; i < nbPairs; i++)
					{
						const PxContactPair& cp = pairs[i];

						if (cp.events & PxPairFlag::eNOTIFY_TOUCH_FOUND)
						{
							obj1->OnCollisionBegin(obj2);
							obj2->OnCollisionBegin(obj1);
						}
						if (cp.events & PxPairFlag::eNOTIFY_TOUCH_LOST)
						{
							obj1->OnCollisionEnd(obj2);
							obj2->OnCollisionEnd(obj1);
						}
					}
				}

			}
			void onConstraintBreak(PxConstraintInfo* constraints, PxU32 count) {}
			void onWake(PxActor** actors, PxU32 count) {}
			void onSleep(PxActor** actors, PxU32 count) {}
			void onTrigger(PxTriggerPair* pairs, PxU32 count) {}
			void onAdvance(const PxRigidBody* const* bodyBuffer, const PxTransform* poseBuffer, const PxU32 count) {}

			void Clear();
			void ClearAndErase();

			void AddGameObject(GameObject* o);
			void RemoveGameObject(GameObject* o, bool andDelete = false);

			Camera* GetMainCamera() const
			{
				return mainCamera;
			}

			void ShuffleObjects(bool state)
			{
				shuffleObjects = state;
			}

			virtual void UpdateWorld(float dt);

			void UpdatePhysics(float fixedDeltaTime);

			static GameObject* FindObjectFromPhysicsBody(PxActor* actor);

			void OperateOnContents(GameObjectFunc f);

			void GetObjectIterators(GameObjectIterator& first, GameObjectIterator& last) const;

			bool GetShuffleObjects() const
			{
				return shuffleObjects;
			}
			static std::vector<GameObject*> gameObjects;

			int GetTotalCollisions() const
			{
				return totalCollisions;
			}

			//void IncreamentLightCount()
			//{
			//	++lightCount;
			//}
			//int GetLightCount()
			//{
			//	return lightCount;
			//}

			//void AddLight(Light& l)
			//{
			//	//lights[lightCount] = l;
			//}
			void SetDebugMode(bool val)
			{
				debugMode = val;
			}

		protected:
			Camera* mainCamera;
			bool	shuffleObjects;
			int		worldIDCounter;
			int totalCollisions;
			int     lightCount;

			bool debugMode;
		};
	}
}

