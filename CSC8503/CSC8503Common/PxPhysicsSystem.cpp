#include "PxPhysicsSystem.h"
#include <iostream>
using namespace NCL;
using namespace NCL::CSC8503;
PxFilterFlags ContactReportFilterShader(PxFilterObjectAttributes attributes0, PxFilterData filterData0, PxFilterObjectAttributes attributes1, 
	PxFilterData filterData1, PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize) {
	PX_UNUSED(attributes0);
	PX_UNUSED(attributes1);
	PX_UNUSED(filterData0);
	PX_UNUSED(filterData1);
	PX_UNUSED(constantBlockSize);
	PX_UNUSED(constantBlock);
	if (PxFilterObjectIsTrigger(attributes0) || PxFilterObjectIsTrigger(attributes1)) {
		pairFlags = PxPairFlag::eTRIGGER_DEFAULT;
		return PxFilterFlag::eDEFAULT;
	}
	// all initial and persisting reports for everything, with per-point data
	pairFlags = PxPairFlag::eSOLVE_CONTACT | PxPairFlag::eDETECT_DISCRETE_CONTACT | PxPairFlag::eDETECT_CCD_CONTACT
		| PxPairFlag::eNOTIFY_TOUCH_FOUND | PxPairFlag::eNOTIFY_TOUCH_LOST /*| PxPairFlag::eNOTIFY_TOUCH_PERSISTS*/
		| PxPairFlag::eTRIGGER_DEFAULT;

	return PxFilterFlags();
}

PxPhysicsSystem::PxPhysicsSystem() {

	gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);

	gPvd = PxCreatePvd(*gFoundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
	gPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);

	gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(), true, gPvd);

	PxSceneDesc sceneDesc = PxSceneDesc(gPhysics->getTolerancesScale());

	sceneDesc.flags |= PxSceneFlag::eENABLE_CCD;		
	sceneDesc.gravity = PxVec3(0.0f, GRAVITTY, 0.0f);
	gDispatcher = PxDefaultCpuDispatcherCreate(2);
	sceneDesc.cpuDispatcher = gDispatcher;
	sceneDesc.filterShader = ContactReportFilterShader;
	sceneDesc.simulationEventCallback = new NCL::CSC8503::GameWorld;
	gScene = gPhysics->createScene(sceneDesc);

	PxPvdSceneClient* pvdClient = gScene->getScenePvdClient();
	if (pvdClient) {
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}

	gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.6f);
}

PxPhysicsSystem::~PxPhysicsSystem() {
	PX_RELEASE(gScene);
	PX_RELEASE(gDispatcher);
	PX_RELEASE(gPhysics);
	if (gPvd) {
		PxPvdTransport* transport = gPvd->getTransport();
		PX_RELEASE(transport);
		PX_RELEASE(gPvd);
	}
	PX_RELEASE(gMaterial);
	PX_RELEASE(gFoundation);
}

void PxPhysicsSystem::ResetPhysics() {
	PxU32 nbActors = gScene->getNbActors(PxActorTypeFlag::eRIGID_DYNAMIC);
	PxActor** actors = new PxActor * [nbActors];
	gScene->getActors(PxActorTypeFlag::eRIGID_DYNAMIC, actors, nbActors);
	while (nbActors--) {
		gScene->removeActor(*actors[nbActors]);
	}

	nbActors = gScene->getNbActors(PxActorTypeFlag::eRIGID_STATIC);
	actors = new PxActor * [nbActors];
	gScene->getActors(PxActorTypeFlag::eRIGID_STATIC, actors, nbActors);
	while (nbActors--) {
		gScene->removeActor(*actors[nbActors]);
	}
}

void PxPhysicsSystem::StepPhysics(float fixedDeltaTime) {
	gScene->simulate(fixedDeltaTime);
	gScene->fetchResults(true);
}




