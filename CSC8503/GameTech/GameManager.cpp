#include "GameManager.h"
#include "../CSC8503Common/Coin.h"
#include "../CSC8503Common/LongJump.h"
#include "../CSC8503Common/SpeedPower.h"
Win32Code::Win32Window* GameManager::window = nullptr;

PxPhysicsSystem* GameManager::pXPhysics = nullptr;
GameWorld* GameManager::world = nullptr;
GameTechRenderer* GameManager::renderer = nullptr;
AudioManager* GameManager::audioManager = nullptr;
Obstacles* GameManager::obstacles = nullptr;


OGLMesh* GameManager::charMeshA = nullptr;
OGLMesh* GameManager::charMeshB = nullptr;
OGLMesh* GameManager::enemyMesh = nullptr;
OGLMesh* GameManager::bonusMesh = nullptr;
OGLMesh* GameManager::capsuleMesh = nullptr;
OGLMesh* GameManager::cylinderMesh = nullptr;
OGLMesh* GameManager::cubeMesh = nullptr;
OGLMesh* GameManager::sphereMesh = nullptr;
OGLMesh* GameManager::pbodyMesh = nullptr;

OGLTexture* GameManager::basicTex = nullptr;
OGLTexture* GameManager::floorTex = nullptr;
OGLTexture* GameManager::lavaTex = nullptr;
OGLTexture* GameManager::iceTex = nullptr;
OGLTexture* GameManager::trampolineTex = nullptr;
OGLTexture* GameManager::obstacleTex = nullptr;
OGLTexture* GameManager::woodenTex = nullptr;
OGLTexture* GameManager::wallTex2 = nullptr;
OGLTexture* GameManager::finishTex = nullptr;
OGLTexture* GameManager::menuTex = nullptr;
OGLTexture* GameManager::plainTex = nullptr;
OGLTexture* GameManager::wallTex = nullptr;
OGLTexture* GameManager::dogeTex = nullptr;
OGLTexture* GameManager::redTex = nullptr;
OGLTexture* GameManager::pBodyTex = nullptr;
OGLTexture* GameManager::platformWallTex = nullptr;




OGLTexture* GameManager::pinkTex = nullptr;
OGLTexture* GameManager::metalTex = nullptr;
OGLTexture* GameManager::greyTex = nullptr;
OGLTexture* GameManager::blackTex = nullptr;
OGLTexture* GameManager::fallingTileTex = nullptr;


OGLShader* GameManager::basicShader = nullptr;
OGLShader* GameManager::toonShader = nullptr;
OGLShader* GameManager::outlineShader = nullptr;

CameraState GameManager::camState = CameraState::FREE;

//GameObject* GameManager::lockedObject = nullptr;
GameObject* GameManager::selectionObject = nullptr;

LevelState GameManager::levelState = LevelState::LEVEL1;

MeshMaterial* GameManager::pBodyMat = nullptr;
PlayerObject* GameManager::player = nullptr;

void GameManager::Create(PxPhysicsSystem* p, GameWorld* w, AudioManager* a)
{
	pXPhysics = p;
	world = w;
	renderer = new GameTechRenderer(*GameManager::GetWorld(), *pXPhysics);
	Debug::SetRenderer(renderer);
	renderer->InitGUI(window->GetHandle());
	audioManager = a;
	obstacles = new Obstacles();
}

void GameManager::LoadAssets()
{
	auto loadFunc = [](const string& name, OGLMesh** into)
	{
		*into = new OGLMesh(name);
		(*into)->SetPrimitiveType(GeometryPrimitive::Triangles);
		(*into)->UploadToGPU();
	};

	loadFunc("cube.msh", &cubeMesh);
	loadFunc("sphere.msh", &sphereMesh);
	loadFunc("Male1.msh", &charMeshA);
	loadFunc("courier.msh", &charMeshB);
	loadFunc("security.msh", &enemyMesh);
	loadFunc("coin.msh", &bonusMesh);
	loadFunc("capsule.msh", &capsuleMesh);
	loadFunc("cylinder.msh", &cylinderMesh);
	loadFunc("pbody.msh", &pbodyMesh);

	basicTex = (OGLTexture*)TextureLoader::LoadAPITexture("checkerboard.png");
	obstacleTex = (OGLTexture*)TextureLoader::LoadAPITexture("obstacle.png");
	floorTex = (OGLTexture*)TextureLoader::LoadAPITexture("platform.png");
	lavaTex = (OGLTexture*)TextureLoader::LoadAPITexture("lava.png");
	trampolineTex = (OGLTexture*)TextureLoader::LoadAPITexture("trampoline.png");
	iceTex = (OGLTexture*)TextureLoader::LoadAPITexture("ice2.png");
	woodenTex = (OGLTexture*)TextureLoader::LoadAPITexture("woodTex.png");
	finishTex = (OGLTexture*)TextureLoader::LoadAPITexture("finish.png");
	menuTex = (OGLTexture*)TextureLoader::LoadAPITexture("menu.png");
	plainTex = (OGLTexture*)TextureLoader::LoadAPITexture("plain.png");
	wallTex = (OGLTexture*)TextureLoader::LoadAPITexture("wall.png");
	dogeTex = (OGLTexture*)TextureLoader::LoadAPITexture("doge.png");
	pBodyTex = (OGLTexture*)TextureLoader::LoadAPITexture("pbody.png");
	platformWallTex = (OGLTexture*)TextureLoader::LoadAPITexture("platformWall.png");

	wallTex2 = (OGLTexture*)TextureLoader::LoadAPITexture("wall2.png");

	pinkTex = (OGLTexture*)TextureLoader::LoadAPITexture("pink.png");
	metalTex = (OGLTexture*)TextureLoader::LoadAPITexture("metal.png");
	greyTex = (OGLTexture*)TextureLoader::LoadAPITexture("cannonBall.png");
	blackTex = (OGLTexture*)TextureLoader::LoadAPITexture("black.png");
	fallingTileTex = (OGLTexture*)TextureLoader::LoadAPITexture("crackedice.png");


	redTex = (OGLTexture*)TextureLoader::LoadAPITexture("red.png");
	pBodyMat = new MeshMaterial("pbody.mat");

	//renderer->SetTextureRepeating(trampolineTex,false);

	basicShader = new OGLShader("GameTechVert.glsl", "GameTechFrag.glsl");
	toonShader = new OGLShader("ToonShaderVertex.glsl", "ToonShaderFragment.glsl");
	outlineShader = new OGLShader("ToonShaderVertex.glsl", "ToonOutlineFragment.glsl");
}

void GameManager::ResetMenu()
{
	levelState = LevelState::LEVEL1;
	selectionObject = nullptr;
	renderer->SetSelectionObject(nullptr);
	renderer->SetLockedObject(nullptr);
}

GameManager::~GameManager()
{
	delete pXPhysics;
	delete world;
	delete renderer;
	delete audioManager;

	delete capsuleMesh;
	delete cubeMesh;
	delete sphereMesh;
	delete charMeshA;
	delete charMeshB;
	delete enemyMesh;
	delete bonusMesh;
	delete basicTex;
	delete obstacleTex;
	delete floorTex;
	delete lavaTex;
	delete trampolineTex;
	delete iceTex;
	delete woodenTex;
	delete finishTex;
	delete menuTex;
	delete plainTex;
	delete wallTex;
	delete wallTex2;
	delete dogeTex;
	delete pBodyTex;
	delete platformWallTex;
	delete redTex;


	delete pinkTex;
	delete metalTex;
	delete greyTex;
	delete blackTex;
	delete fallingTileTex;

	delete basicShader;
	delete toonShader;
	delete outlineShader;
}

GameObject* GameManager::AddPxCubeToWorld(const PxTransform& t, const PxVec3 halfSizes, float density, float friction, float elasticity)
{
	GameObject* cube = new GameObject("Cube");

	PxRigidDynamic* body = pXPhysics->GetGPhysics()->createRigidDynamic(t);
	PxMaterial* newMat = pXPhysics->GetGPhysics()->createMaterial(friction, friction, elasticity);
	PxRigidActorExt::createExclusiveShape(*body, PxBoxGeometry(halfSizes.x, halfSizes.y, halfSizes.z), *newMat);
	PxRigidBodyExt::updateMassAndInertia(*body, density);
	cube->SetPhysicsObject(new PhysXObject(body, newMat));
	pXPhysics->GetGScene()->addActor(*body);

	cube->GetTransform().SetScale(halfSizes * 2);
	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, redTex, toonShader));
	world->AddGameObject(cube);

	return cube;
}

GameObject* GameManager::AddPxSphereToWorld(const PxTransform& t, const  PxReal radius, float density, float friction, float elasticity)
{
	GameObject* sphere = new GameObject("Sphere");

	PxRigidDynamic* body = pXPhysics->GetGPhysics()->createRigidDynamic(t);
	PxMaterial* newMat = pXPhysics->GetGPhysics()->createMaterial(friction, friction, elasticity);
	PxRigidActorExt::createExclusiveShape(*body, PxSphereGeometry(radius), *newMat);
	PxRigidBodyExt::updateMassAndInertia(*body, density);
	sphere->SetPhysicsObject(new PhysXObject(body, newMat));
	pXPhysics->GetGScene()->addActor(*body);
	sphere->GetTransform().SetScale(PxVec3(radius, radius, radius));
	sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, basicTex, outlineShader));
	world->AddGameObject(sphere);

	return sphere;
}

GameObject* GameManager::AddPxCapsuleToWorld(const PxTransform& t, const  PxReal radius, const PxReal halfHeight, float density, float friction, float elasticity)
{
	GameObject* capsule = new GameObject("Capsule");

	PxRigidDynamic* body = pXPhysics->GetGPhysics()->createRigidDynamic(t);
	PxMaterial* newMat = pXPhysics->GetGPhysics()->createMaterial(friction, friction, elasticity);
	PxRigidActorExt::createExclusiveShape(*body, PxCapsuleGeometry(radius, halfHeight), *newMat)->setLocalPose(PxTransform(PxQuat(PxHalfPi, PxVec3(0, 0, 1))));
	PxRigidBodyExt::updateMassAndInertia(*body, density);
	capsule->SetPhysicsObject(new PhysXObject(body, newMat));
	pXPhysics->GetGScene()->addActor(*body);

	capsule->GetTransform().SetScale(PxVec3(radius * 2, halfHeight * 2, radius * 2));
	capsule->SetRenderObject(new RenderObject(&capsule->GetTransform(), capsuleMesh, pinkTex, outlineShader));
	world->AddGameObject(capsule);

	return capsule;
}

GameObject* GameManager::AddPxCylinderToWorld(const PxTransform& t, const  PxReal radius, const PxReal halfHeight, float density, float friction, float elasticity)
{
	GameObject* cylinder = new GameObject("Cylinder");

	PxRigidDynamic* body = pXPhysics->GetGPhysics()->createRigidDynamic(t);
	PxMaterial* newMat = pXPhysics->GetGPhysics()->createMaterial(friction, friction, elasticity);
	PxRigidActorExt::createExclusiveShape(*body, PxCapsuleGeometry(radius, (2 * halfHeight) - (radius / 2)), *newMat)->setLocalPose(PxTransform(PxQuat(PxHalfPi, PxVec3(0, 0, 1))));
	PxRigidBodyExt::updateMassAndInertia(*body, density);
	cylinder->SetPhysicsObject(new PhysXObject(body, newMat));
	pXPhysics->GetGScene()->addActor(*body);

	cylinder->GetTransform().SetScale(PxVec3(radius * 2, halfHeight * 2, radius * 2));
	cylinder->SetRenderObject(new RenderObject(&cylinder->GetTransform(), cylinderMesh, basicTex, outlineShader));
	world->AddGameObject(cylinder);

	return cylinder;
}

void GameManager::AddBounceSticks(const PxTransform& t, const  PxReal radius, const PxReal halfHeight, float density, float friction, float elasticity)
{
	GameObject* capsule = new GameObject("BounceStick");

	PxRigidStatic* body = pXPhysics->GetGPhysics()->createRigidStatic(t);
	PxMaterial* newMat = pXPhysics->GetGPhysics()->createMaterial(friction, friction, elasticity);
	PxRigidActorExt::createExclusiveShape(*body, PxCapsuleGeometry(radius, halfHeight), *newMat)->setLocalPose(PxTransform(PxQuat(PxHalfPi, PxVec3(0, 0, 1))));
	capsule->SetPhysicsObject(new PhysXObject(body, newMat));
	pXPhysics->GetGScene()->addActor(*body);

	capsule->GetTransform().SetScale(PxVec3(radius * 2, halfHeight * 2, radius * 2));
	capsule->SetRenderObject(new RenderObject(&capsule->GetTransform(), capsuleMesh, pinkTex, outlineShader));
	world->AddGameObject(capsule);
}

GameObject* GameManager::AddPxCoinToWorld(const PxTransform& t, const PxReal radius)
{
	Coin* coin = new Coin();

	PxRigidDynamic* body = pXPhysics->GetGPhysics()->createRigidDynamic(t);
	PxMaterial* newMat = pXPhysics->GetGPhysics()->createMaterial(0, 0, 0);
	PxRigidActorExt::createExclusiveShape(*body, PxSphereGeometry(radius), *newMat);
	PxRigidBodyExt::updateMassAndInertia(*body, FLT_MIN);
	body->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
	coin->SetPhysicsObject(new PhysXObject(body, newMat));
	pXPhysics->GetGScene()->addActor(*body);

	coin->GetTransform().SetScale(PxVec3(radius / 4, radius / 4, radius / 4));
	coin->SetRenderObject(new RenderObject(&coin->GetTransform(), bonusMesh, basicTex, toonShader));
	coin->GetRenderObject()->SetColour(Debug::YELLOW);
	coin->SetInitialPos(t.p);
	world->AddGameObject(coin);
	return coin;
}

GameObject* GameManager::AddPxLongJump(const PxTransform& t, const PxReal radius)
{
	LongJump* jump = new LongJump();

	PxRigidDynamic* body = pXPhysics->GetGPhysics()->createRigidDynamic(t);
	PxMaterial* newMat = pXPhysics->GetGPhysics()->createMaterial(0, 0, 0);
	PxRigidActorExt::createExclusiveShape(*body, PxSphereGeometry(radius), *newMat);
	PxRigidBodyExt::updateMassAndInertia(*body, FLT_MIN);
	body->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
	jump->SetPhysicsObject(new PhysXObject(body, newMat));
	pXPhysics->GetGScene()->addActor(*body);

	jump->GetTransform().SetScale(PxVec3(radius / 4, radius / 4, radius / 4));
	jump->SetRenderObject(new RenderObject(&jump->GetTransform(), bonusMesh, basicTex, toonShader));
	jump->GetRenderObject()->SetColour(Debug::RED);
	jump->SetInitialPos(t.p);
	world->AddGameObject(jump);
	return jump;
}

GameObject* GameManager::AddPxSpeedPower(const PxTransform& t, const PxReal radius)
{
	SpeedPower* speed = new SpeedPower();

	PxRigidDynamic* body = pXPhysics->GetGPhysics()->createRigidDynamic(t);
	PxMaterial* newMat = pXPhysics->GetGPhysics()->createMaterial(0, 0, 0);
	PxRigidActorExt::createExclusiveShape(*body, PxSphereGeometry(radius), *newMat);
	PxRigidBodyExt::updateMassAndInertia(*body, FLT_MIN);
	body->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
	speed->SetPhysicsObject(new PhysXObject(body, newMat));
	pXPhysics->GetGScene()->addActor(*body);

	speed->GetTransform().SetScale(PxVec3(radius / 4, radius / 4, radius / 4));
	speed->SetRenderObject(new RenderObject(&speed->GetTransform(), bonusMesh, basicTex, toonShader));
	speed->GetRenderObject()->SetColour(Debug::GREEN);
	speed->SetInitialPos(t.p);
	world->AddGameObject(speed);
	return speed;
}

PlayerObject* GameManager::AddPxPlayerToWorld(const PxTransform& t, const PxReal scale)
{
	PlayerObject* p = new PlayerObject();

	float meshSize = 2.5f * scale;
	PxRigidDynamic* body = pXPhysics->GetGPhysics()->createRigidDynamic(t);
	PxRigidActorExt::createExclusiveShape(*body, PxCapsuleGeometry(meshSize, meshSize),
		*pXPhysics->GetGMaterial())->setLocalPose(PxTransform(PxVec3(0, meshSize * 2.f, 0), PxQuat(PxHalfPi, PxVec3(0, 0, 1))));
	PxRigidBodyExt::updateMassAndInertia(*body, 40.0f);
	p->SetPhysicsObject(new PhysXObject(body, pXPhysics->GetGMaterial()));
	pXPhysics->GetGScene()->addActor(*body);

	p->GetTransform().SetScale(PxVec3(meshSize * 2, meshSize * 2, meshSize * 2));
	p->SetRenderObject(new RenderObject(&p->GetTransform(), pbodyMesh, pBodyTex, outlineShader));
	world->AddGameObject(p);

	return p;
}

NetworkPlayer* GameManager::AddPxNetworkPlayerToWorld(const PxTransform& t, const PxReal scale, NetworkedGame* game, int playerNum)
{
	NetworkPlayer* p = new NetworkPlayer(game, playerNum);

	float meshSize = 2.5f * scale;
	PxRigidDynamic* body = pXPhysics->GetGPhysics()->createRigidDynamic(t);
	PxRigidActorExt::createExclusiveShape(*body, PxCapsuleGeometry(meshSize, meshSize),
		*pXPhysics->GetGMaterial())->setLocalPose(PxTransform(PxVec3(0, meshSize * 2.f, 0), PxQuat(PxHalfPi, PxVec3(0, 0, 1))));
	PxRigidBodyExt::updateMassAndInertia(*body, 40.0f);
	p->SetPhysicsObject(new PhysXObject(body, pXPhysics->GetGMaterial()));
	pXPhysics->GetGScene()->addActor(*body);

	p->GetTransform().SetScale(PxVec3(meshSize * 2, meshSize * 2, meshSize * 2));
	p->SetRenderObject(new RenderObject(&p->GetTransform(), pbodyMesh, pBodyTex, outlineShader));

	switch (playerNum) {
	case -1:
		p->GetRenderObject()->SetColour(Debug::RED);
		break;
	case 0:
		p->GetRenderObject()->SetColour(Debug::BLUE);
		break;
	case 1:
		p->GetRenderObject()->SetColour(Debug::GREEN);
		break;
	case 2:
		p->GetRenderObject()->SetColour(Debug::YELLOW);
		break;
	case 3:
		p->GetRenderObject()->SetColour(Debug::MAGENTA);
		break;
	}

	world->AddGameObject(p);

	return p;
}

void GameManager::AddPxEnemyToWorld(const PxTransform& t, const PxReal scale)
{
	GameObject* e = new GameObject("Enemy");

	float meshSize = MESH_SIZE * scale;
	PxRigidDynamic* body = pXPhysics->GetGPhysics()->createRigidDynamic(t);
	PxRigidActorExt::createExclusiveShape(*body, PxCapsuleGeometry(meshSize * .85f, meshSize * 0.85f),
		*pXPhysics->GetGMaterial())->setLocalPose(PxTransform(PxQuat(PxHalfPi, PxVec3(0, 0, 1))));
	PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);
	e->SetPhysicsObject(new PhysXObject(body, pXPhysics->GetGMaterial()));
	pXPhysics->GetGScene()->addActor(*body);

	e->GetTransform().SetScale(PxVec3(meshSize * 2, meshSize * 2, meshSize * 2));
	e->SetRenderObject(new RenderObject(&e->GetTransform(), enemyMesh, basicTex, outlineShader));
	e->GetRenderObject()->SetColour(Vector4(1, 0, 0, 1));
	world->AddGameObject(e);
}

void GameManager::AddPxSeeSawToWorld(const PxTransform& t, const PxVec3 halfSizes, float density, float friction, float elasticity)
{
	GameObject* cube = new GameObject("SeeSaw");

	PxRigidDynamic* body = pXPhysics->GetGPhysics()->createRigidDynamic(t);
	PxMaterial* newMat = pXPhysics->GetGPhysics()->createMaterial(friction, friction, elasticity);
	PxRigidActorExt::createExclusiveShape(*body, PxBoxGeometry(halfSizes.x, halfSizes.y, halfSizes.z), *newMat);
	PxRigidBodyExt::updateMassAndInertia(*body, density);
	cube->SetPhysicsObject(new PhysXObject(body, newMat));
	pXPhysics->GetGScene()->addActor(*body);
	PxRevoluteJoint* joint = PxRevoluteJointCreate(*pXPhysics->GetGPhysics(), body, PxTransform(PxVec3(0)), NULL, PxTransform(t.p * 2));

	cube->GetTransform().SetScale(halfSizes * 2);
	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, outlineShader));
	world->AddGameObject(cube);
}

void GameManager::AddPxRevolvingDoorToWorld(const PxTransform& t, const PxVec3 halfSizes, float density, float friction, float elasticity)
{
	GameObject* cube = new GameObject("RevolvingDoor");

	PxRigidDynamic* body = pXPhysics->GetGPhysics()->createRigidDynamic(t);
	PxMaterial* newMat = pXPhysics->GetGPhysics()->createMaterial(friction, friction, elasticity);
	PxRigidActorExt::createExclusiveShape(*body, PxBoxGeometry(halfSizes.x, halfSizes.y, halfSizes.z), *newMat);
	PxRigidBodyExt::updateMassAndInertia(*body, density);
	cube->SetPhysicsObject(new PhysXObject(body, newMat));
	pXPhysics->GetGScene()->addActor(*body);
	PxD6Joint* joint = PxD6JointCreate(*pXPhysics->GetGPhysics(), body, PxTransform(PxVec3(0)), NULL, PxTransform(t.p * 2));
	joint->setMotion(PxD6Axis::eSWING1, PxD6Motion::eFREE);

	cube->GetTransform().SetScale(halfSizes * 2);
	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, outlineShader));
	world->AddGameObject(cube);
}

GameObject* GameManager::AddPxRotatingCubeToWorld(const PxTransform& t, const PxVec3 halfSizes, const PxVec3 rotation,
	float friction, float elasticity, bool rotatedRotation, TextureState state, Vector3 textureScale)
{
	GameObject* cube = new GameObject("Rotating Cube");

	PxRigidDynamic* body = pXPhysics->GetGPhysics()->createRigidDynamic(t);
	PxMaterial* newMat = pXPhysics->GetGPhysics()->createMaterial(friction, friction, elasticity);
	PxRigidActorExt::createExclusiveShape(*body, PxBoxGeometry(halfSizes.x, halfSizes.y, halfSizes.z), *newMat);
	body->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
	PxVec3 up;

	if (rotatedRotation)
	{
		int x = rotation.x;
		int y = rotation.y;
		int z = rotation.z;

		int speed = std::max(std::initializer_list<int>{x, y, z});

		Vector3 fwd = PhysxConversions::GetVector3(Quaternion(t.q) * Vector3(speed, 0, 0));
		up = PhysxConversions::GetVector3(Vector3::Cross(Vector3(speed, 0, 0), -fwd));
	}
	else
	{
		up = rotation;
	}

	body->setAngularVelocity(up);
	body->setAngularDamping(0.f);
	body->setMass(0.f);
	body->setMassSpaceInertiaTensor(PxVec3(0.f));
	cube->SetPhysicsObject(new PhysXObject(body, newMat));
	pXPhysics->GetGScene()->addActor(*body);

	cube->GetTransform().SetScale(halfSizes * 2);
	if (state == TextureState::WOOD)
	{
		cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, woodenTex, outlineShader));
	}
	else
	{
		cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, outlineShader));
	}
	cube->GetTransform().SetTextureScale(textureScale);
	world->AddGameObject(cube);
	return cube;
}

GameObject* GameManager::AddPxRotatingCylinderToWorld(const PxTransform& t, const PxReal radius, const PxReal halfHeight, const PxVec3 rotation, float friction, float elasticity, TextureState state, Vector3 textureScale)
{
	GameObject* cylinder = new GameObject("RotatingCylinder");

	PxRigidDynamic* body = pXPhysics->GetGPhysics()->createRigidDynamic(t);
	PxMaterial* newMat = pXPhysics->GetGPhysics()->createMaterial(friction, friction, elasticity);
	PxRigidActorExt::createExclusiveShape(*body, PxCapsuleGeometry(radius, (2 * halfHeight) - (radius / 2)), *newMat)->setLocalPose(PxTransform(PxQuat(PxHalfPi, PxVec3(0, 0, 1))));
	body->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
	body->setAngularVelocity(rotation);
	body->setAngularDamping(0.f);
	body->setMass(0.f);
	body->setMassSpaceInertiaTensor(PxVec3(0.f));
	cylinder->SetPhysicsObject(new PhysXObject(body, newMat));
	pXPhysics->GetGScene()->addActor(*body);

	cylinder->GetTransform().SetTextureScale(textureScale);
	cylinder->GetTransform().SetScale(PxVec3(radius * 2, halfHeight * 2, radius * 2));
	if (state == TextureState::PINK)	{

		cylinder->SetRenderObject(new RenderObject(&cylinder->GetTransform(), cylinderMesh, pinkTex, toonShader));
	}
	else if(state == TextureState::METAL)
	{
		cylinder->SetRenderObject(new RenderObject(&cylinder->GetTransform(), cylinderMesh, metalTex, toonShader));
	}
	else
	{
		cylinder->SetRenderObject(new RenderObject(&cylinder->GetTransform(), cylinderMesh, basicTex, toonShader));
	}
	world->AddGameObject(cylinder);

	return cylinder;
}

GameObject* GameManager::AddPxPendulumToWorld(const PxTransform& t, const PxReal radius, const PxReal halfHeight, const float timeToSwing, const bool isSwingingLeft, float friction, float elasticity, Vector3 textureScale)
{
	Pendulum* pendulum = new Pendulum(timeToSwing, isSwingingLeft);

	PxRigidDynamic* body = pXPhysics->GetGPhysics()->createRigidDynamic(t);
	PxMaterial* newMat = pXPhysics->GetGPhysics()->createMaterial(friction, friction, elasticity);
	PxRigidActorExt::createExclusiveShape(*body, PxCapsuleGeometry(radius, (2 * halfHeight)), *newMat)->setLocalPose(PxTransform(PxQuat(PxHalfPi, PxVec3(0, 0, 1))));
	body->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
	body->setAngularDamping(0.f);
	body->setMass(0.f);
	body->setMassSpaceInertiaTensor(PxVec3(0.f));
	pendulum->SetPhysicsObject(new PhysXObject(body, newMat));
	pXPhysics->GetGScene()->addActor(*body);

	pendulum->GetTransform().SetScale(PxVec3(radius * 2, halfHeight * 2, radius * 2));
	pendulum->SetRenderObject(new RenderObject(&pendulum->GetTransform(), cylinderMesh, metalTex, toonShader));
	pendulum->GetTransform().SetTextureScale(textureScale);
	world->AddGameObject(pendulum);
	return pendulum;
}



void GameManager::AddPxFloorToWorld(const PxTransform& t, const PxVec3 halfSizes, float friction, float elasticity, TextureState state, Vector3 textureScale)
{
	GameObject* floor = new GameObject("Floor");

	PxRigidStatic* body = pXPhysics->GetGPhysics()->createRigidStatic(t);
	PxMaterial* newMat = pXPhysics->GetGPhysics()->createMaterial(friction, friction, elasticity);
	PxRigidActorExt::createExclusiveShape(*body, PxBoxGeometry(halfSizes.x, halfSizes.y, halfSizes.z), *newMat);
	floor->SetPhysicsObject(new PhysXObject(body, newMat));
	pXPhysics->GetGScene()->addActor(*body);
	floor->GetTransform().SetScale(halfSizes * 2);
	floor->GetTransform().SetTextureScale(textureScale);
	switch (state)
	{
	case TextureState::FLOOR:
		floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, floorTex, toonShader));
		break;
	case TextureState::WALL:
		floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, platformWallTex, toonShader));
		break;
	case TextureState::ICE:
		floor->SetName("Ice");
		floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, iceTex, toonShader));
		break;
	case TextureState::TRAMPOLINE:
		floor->SetName("Trampoline");
		floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, trampolineTex, toonShader));
		break;
	case TextureState::FINISH:
		floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, finishTex, toonShader));
		break;
	case TextureState::RED:
		floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, redTex, toonShader));
		break;
	case TextureState::DOGE:
		floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, dogeTex, toonShader));
		break;
	case TextureState::WALL2:
		floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, wallTex2, toonShader));
		break;
	case TextureState::INVISIBLE:
		break;

	}
	world->AddGameObject(floor);
}

Cannonball* GameManager::AddPxCannonBallToWorld(const PxTransform& t, const  PxReal radius, const PxVec3* force, int time, float density, float friction, float elasticity)
{
	Cannonball* cannonBall = new Cannonball(time);
	PxRigidDynamic* body = pXPhysics->GetGPhysics()->createRigidDynamic(t);
	PxMaterial* newMat = pXPhysics->GetGPhysics()->createMaterial(friction, friction, elasticity);
	PxRigidActorExt::createExclusiveShape(*body, PxSphereGeometry(radius), *newMat);
	PxRigidBodyExt::updateMassAndInertia(*body, density);
	cannonBall->SetPhysicsObject(new PhysXObject(body, newMat));
	pXPhysics->GetGScene()->addActor(*body);
	cannonBall->GetTransform().SetScale(PxVec3(radius, radius, radius));
	cannonBall->SetRenderObject(new RenderObject(&cannonBall->GetTransform(), sphereMesh, greyTex, outlineShader));
	world->AddGameObject(cannonBall);
	return cannonBall;
}

void GameManager::AddPxCannonToWorld(const PxTransform& t, const PxVec3 trajectory, const int shotTime, const int shotSize, const PxVec3 translate)
{
	Cannon* cannon = new Cannon(trajectory, shotTime, shotSize, translate);

	PxRigidStatic* body = pXPhysics->GetGPhysics()->createRigidStatic(t);
	PxRigidActorExt::createExclusiveShape(*body, PxBoxGeometry(30, 15, 15), *pXPhysics->GetGMaterial());
	cannon->SetPhysicsObject(new PhysXObject(body, pXPhysics->GetGMaterial()));
	pXPhysics->GetGScene()->addActor(*body);
	cannon->GetTransform().SetScale(PxVec3(30, 15, 15) * 2);
	cannon->SetRenderObject(new RenderObject(&cannon->GetTransform(), cubeMesh, blackTex, toonShader));
	world->AddGameObject(cannon);
}

void GameManager::AddPxKillPlaneToWorld(const PxTransform& t, const PxVec3 halfSizes, const PxVec3 respawnCentre, Vector3 respawnSizeRange, bool hide)
{
	KillPlane* cube = new KillPlane(respawnCentre, respawnSizeRange);

	PxRigidStatic* body = pXPhysics->GetGPhysics()->createRigidStatic(t);
	PxRigidActorExt::createExclusiveShape(*body, PxBoxGeometry(halfSizes.x, halfSizes.y, halfSizes.z), *pXPhysics->GetGMaterial());
	cube->SetPhysicsObject(new PhysXObject(body, pXPhysics->GetGMaterial()));
	pXPhysics->GetGScene()->addActor(*body);

	cube->GetTransform().SetScale(halfSizes * 2);
	if (!hide)
	{
		cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, lavaTex, toonShader));
		cube->GetTransform().SetTextureScale(Vector3(halfSizes.x, halfSizes.z, halfSizes.y) / 100.f);
	}
	world->AddGameObject(cube);
}

GameObject* GameManager::AddPxFallingTileToWorld(const PxTransform& t, const PxVec3 halfSizes, float friction, float elasticity)
{
	FallingTile* cube = new FallingTile("Floor", 1, 0, t.p);

	PxRigidDynamic* body = pXPhysics->GetGPhysics()->createRigidDynamic(t);
	PxMaterial* newMat = pXPhysics->GetGPhysics()->createMaterial(friction, friction, elasticity);
	PxRigidActorExt::createExclusiveShape(*body, PxBoxGeometry(halfSizes.x, halfSizes.y, halfSizes.z), *newMat);
	body->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
	body->setAngularVelocity(PxVec3(0));
	body->setAngularDamping(0.f);
	body->setMass(0.f);
	body->setMassSpaceInertiaTensor(PxVec3(0.f));
	cube->SetPhysicsObject(new PhysXObject(body, newMat));
	pXPhysics->GetGScene()->addActor(*body);

	cube->GetTransform().SetScale(halfSizes * 2);
	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, fallingTileTex, toonShader));
	world->AddGameObject(cube);
	return cube;
}

