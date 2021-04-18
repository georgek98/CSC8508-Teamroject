/*			Created By Rich Davison
 *			Edited By Samuel Buzz Appleby
 *               21/01/2021
 *                170348069
 *			Physics Object Definition		 */
#pragma once
#include <ctype.h>
#include "../../include/PxPhysicsAPI.h"
#include "../../include/foundation/Px.h"
#include "../../include/foundation/PxTransform.h"
using namespace physx;
namespace NCL {
	namespace CSC8503 {
		class PhysXObject {
		public:
			PhysXObject(PxRigidActor* actor, PxMaterial* m);
			~PhysXObject();

			PxMaterial* GetMaterial() const {
				return material;
			}

			PxRigidActor* GetPXActor() const {
				return pXActor;
			}
		protected:
			PxRigidActor* pXActor;
			PxMaterial* material;
		};
	}
}

