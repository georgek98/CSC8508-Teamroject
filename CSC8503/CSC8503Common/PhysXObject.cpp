/*			Created By Rich Davison
 *			Edited By Samuel Buzz Appleby
 *               21/01/2021
 *                170348069
 *			Physics Object Implementation		 */
#include "PhysXObject.h"
using namespace NCL;
using namespace CSC8503;

PhysXObject::PhysXObject(PxRigidActor* actor, PxMaterial* m) {
	pXActor = actor;
	material = m;
	if (actor->is<PxRigidDynamic>()) {			
		PxRigidDynamic* body = (PxRigidDynamic*)actor;
		body->setRigidBodyFlag(PxRigidBodyFlag::eENABLE_CCD, true);		//Enable CCD so high v objects don't move through each other
	}
}

PhysXObject::~PhysXObject() {
	if (pXActor != NULL) {
		pXActor->release();
		//material->release();
	}
}
