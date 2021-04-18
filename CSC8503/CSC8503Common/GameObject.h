/*			  Created By Rich Davison
*			Edited By Samuel Buzz Appleby
 *               21/01/2021
 *                170348069
 *			Game Object Definition		 */
#pragma once
#include <vector>
#include "Transform.h"
#include "RenderObject.h"
#include "PhysXObject.h"

using std::vector;
namespace NCL
{
	namespace CSC8503
	{
		class NetworkObject;
		class GameObject
		{
		public:
			GameObject(string name = "");
			~GameObject();

			virtual void Update(float dt);
			virtual void FixedUpdate(float fixedDT) {}

			void SetName(string val)
			{
				name = val;
			}

			Transform& GetTransform()
			{
				return transform;
			}

			RenderObject* GetRenderObject() const
			{
				return renderObject;
			}

			PhysXObject* GetPhysicsObject() const
			{
				return physicsObject;
			}

			NetworkObject* GetNetworkObject() const
			{
				return networkObject;
			}

			virtual void SetRenderObject(RenderObject* newObject)
			{
				renderObject = newObject;
			}

			virtual void SetPhysicsObject(PhysXObject* newObject)
			{
				physicsObject = newObject;
			}

			virtual void SetNetworkObject(NetworkObject* newObject)
			{
				networkObject = newObject;
			}

			const string& GetName() const
			{
				return name;
			}

			virtual void OnCollisionBegin(GameObject* otherObject) ;

			virtual void OnCollisionEnd(GameObject* otherObject) ;

			void SetWorldID(int newID)
			{
				worldID = newID;
			}

			int GetWorldID() const
			{
				return worldID;
			}

			float GetTimeAlive() const
			{
				return timeAlive;
			}

			/*void SetGrounded(bool val)
			{
				isGrounded = val;
			}*/

			bool IsColliding() const
			{
				return isColliding;
			}

			bool CanDestroy() const {
				return canDestroy;
			}

			void SetPosition(const PxVec3& worldPos);
			void SetOrientation(const PxQuat& newOr);

		protected:
			Transform transform;

			PhysXObject* physicsObject;
			RenderObject* renderObject;
			NetworkObject* networkObject;

			int	worldID;
			string	name;
			float timeAlive;
			bool isGrounded;

			bool isColliding;
			bool canDestroy;
		};
	}
}

