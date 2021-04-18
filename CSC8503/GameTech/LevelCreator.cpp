/*			Created By Rich Davison
 *			Edited By Samuel Buzz Appleby
 *               21/01/2021
 *                170348069
 *			Tutorial Game Implementation		 */
#include "LevelCreator.h"
using namespace physx;
using namespace NCL;
using namespace CSC8503;

LevelCreator::LevelCreator()
{
	realFrames = IDEAL_FRAMES;
	fixedDeltaTime = IDEAL_DT;
	dTOffset = 0.0f;

	GameManager::Create(new PxPhysicsSystem(), new GameWorld(), new AudioManager());
	GameManager::LoadAssets();
}

void LevelCreator::ResetWorld()
{
	GameManager::GetWorld()->ClearAndErase();
	GameManager::GetObstacles()->ClearObstacles();
	GameManager::GetRenderer()->SetSelectedLevel(0);
	GameManager::SetPlayer(nullptr);
	//WorldCreator::GetPhysicsSystem()->ResetPhysics();
}

void LevelCreator::Update(float dt)
{
	GameManager::GetWorld()->UpdateWorld(dt);
	UpdateCamera(dt);
	UpdateLevel(dt);

	UpdateTimeStep(dt);
	GameManager::GetAudioManager()->UpdateAudio(GameManager::GetWorld()->GetMainCamera()->GetPosition());
	GameManager::GetRenderer()->Update(dt);
	GameManager::GetRenderer()->Render();
	Debug::FlushRenderables(dt);
}

void LevelCreator::UpdateCamera(float dt)
{
	if (GameManager::GetPlayer())
	{
		PxRigidDynamic* actor = (PxRigidDynamic*)GameManager::GetPlayer()->GetPhysicsObject()->GetPXActor();
		if (GameManager::GetWorld()->GetMainCamera()->GetState() != CameraState::FREE)
		{
			GameManager::GetWorld()->GetMainCamera()->RotateCameraWithObject(dt, GameManager::GetPlayer());
			if (GameManager::GetPlayer()->GetPhysicsObject()->GetPXActor()->is<PxRigidBody>()) actor->setAngularVelocity(PxVec3(0));
			float yaw = GameManager::GetWorld()->GetMainCamera()->GetYaw();
			yaw = Maths::DegreesToRadians(yaw);
			actor->setGlobalPose(PxTransform(actor->getGlobalPose().p, PxQuat(yaw, { 0, 1, 0 })));
			GameManager::GetWorld()->GetMainCamera()->UpdateCameraWithObject(dt, GameManager::GetPlayer());
		}
		else
		{
			actor->setLinearVelocity(PxVec3(0, 0, 0));
			actor->setAngularVelocity(PxVec3(0, 0, 0));
			if (GameManager::GetRenderer()->GetUIState() != UIState::DEBUG)
				GameManager::GetWorld()->GetMainCamera()->UpdateCamera(dt);
		}
	}

	else if (GameManager::GetRenderer()->GetUIState() != UIState::DEBUG)
		GameManager::GetWorld()->GetMainCamera()->UpdateCamera(dt);

}

void LevelCreator::UpdateTimeStep(float dt)
{
	dTOffset += dt;
	while (dTOffset >= fixedDeltaTime)
	{
		FixedUpdate(fixedDeltaTime);
		dTOffset -= fixedDeltaTime;
	}
	NCL::GameTimer t;
	t.Tick();
	float updateTime = t.GetTimeDeltaSeconds();
	if (updateTime > fixedDeltaTime)
	{
		realFrames /= 2;
		fixedDeltaTime *= 2;
	}
	else if (dt * 2 < fixedDeltaTime)
	{
		realFrames *= 2;
		fixedDeltaTime /= 2;

		if (realFrames > IDEAL_FRAMES)
		{
			realFrames = IDEAL_FRAMES;
			fixedDeltaTime = IDEAL_DT;
		}
	}
}

void LevelCreator::FixedUpdate(float dt)
{
	GameManager::GetPhysicsSystem()->StepPhysics(dt);
	GameManager::GetWorld()->UpdatePhysics(dt);
}

/* Logic for updating level 1 or level 2 */
void LevelCreator::UpdateLevel(float dt)
{
	PlayerObject* player = GameManager::GetPlayer();

	if (player && !player->HasFinished())
	{
		if (player->CheckHasFinished(GameManager::GetLevelState()))
		{
			GameManager::GetAudioManager()->PlayAudio("../../Assets/Audio/level_complete.wav");
			GameManager::GetRenderer()->SetUIState(UIState::FINISH);
		}
		UpdatePlayer(dt);
	}


	/* Enter debug mode? */
	if (Window::GetKeyboard()->KeyHeld(KeyboardKeys::C) && Window::GetKeyboard()->KeyPressed(KeyboardKeys::H)
		&& isSinglePlayer)
	{
		if (GameManager::GetRenderer()->GetUIState() != UIState::DEBUG)
			GameManager::GetRenderer()->SetUIState(UIState::DEBUG);
		else
			GameManager::GetRenderer()->SetUIState(UIState::INGAME);
		GameManager::GetWorld()->GetMainCamera()->SetState(CameraState::FREE);
		GameManager::GetWorld()->SetDebugMode(GameManager::GetRenderer()->GetUIState() == UIState::DEBUG);
		Window::GetWindow()->ShowOSPointer(GameManager::GetRenderer()->GetUIState() == UIState::DEBUG);
		Window::GetWindow()->LockMouseToWindow(GameManager::GetRenderer()->GetUIState() != UIState::DEBUG);
	}

	/* Debug mode selection */
	if (GameManager::GetRenderer()->GetUIState() == UIState::DEBUG)
	{
		SelectObject();
		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F1))
			GameManager::GetWorld()->ShuffleObjects(!GameManager::GetWorld()->GetShuffleObjects());
	}

	/* Change Camera */
	if (Window::GetKeyboard()->KeyPressed(NCL::KeyboardKeys::NUM1))
	{
		switch (GameManager::GetWorld()->GetMainCamera()->GetState())
		{
		case CameraState::THIRDPERSON:
			GameManager::GetWorld()->GetMainCamera()->SetState(CameraState::TOPDOWN);
			break;
		case CameraState::TOPDOWN:
			GameManager::GetWorld()->GetMainCamera()->SetState(CameraState::THIRDPERSON);
			break;
		}
	}

	else if (GameManager::GetSelectionObject())
	{
		DebugObjectMovement();
	}
}

void LevelCreator::UpdatePlayer(float dt)
{
	if (GameManager::GetPlayer()->GetRaycastTimer() <= 0.0f)
	{
		PxVec3 pos = PhysxConversions::GetVector3(GameManager::GetPlayer()->GetTransform().GetPosition() + PxVec3(0, 3, 0));
		PxVec3 dir = PxVec3(0, -1, 0);
		float distance = 4.0f;
		const PxU32 bufferSize = 4;        // [in] size of 'hitBuffer'
		PxRaycastHit hitBuffer[bufferSize];  // [out] User provided buffer for results
		PxRaycastBuffer buf(hitBuffer, bufferSize); // [out] Blocking and touching hits stored here

		GameManager::GetPhysicsSystem()->GetGScene()->raycast(pos, dir, distance, buf);
		GameManager::GetPlayer()->SetIsGrounded(buf.getNbTouches() > 1);

		GameManager::GetPlayer()->SetRaycastTimer(0.1f);
	}
}




/* Initialise camera to default location */
void LevelCreator::InitCamera()
{
	GameManager::GetWorld()->GetMainCamera()->SetNearPlane(10);
	GameManager::GetWorld()->GetMainCamera()->SetFarPlane(5000.0f);
	GameManager::GetWorld()->GetMainCamera()->SetPosition(Vector3(0, 50, 80));
	GameManager::GetWorld()->GetMainCamera()->SetYaw(0);
	GameManager::GetWorld()->GetMainCamera()->SetPitch(-10);
	GameManager::GetWorld()->GetMainCamera()->SetState(CameraState::FREE);
}

/* Initialise all the elements contained within the world */
void LevelCreator::InitWorld(LevelState state)
{
	isSinglePlayer = true;
	InitCamera();
	InitFloors(state);
	InitGameMusic(state);
	InitGameExamples(state);
	InitGameObstacles(state);
	GameManager::GetRenderer()->SetUIState(UIState::INGAME);
}

void LevelCreator::InitGameMusic(LevelState state)
{
	switch (state)
	{
	case LevelState::LEVEL1:
		GameManager::GetAudioManager()->PlayAudio("../../Assets/Audio/Level1Music.mp3", true);

		GameManager::GetAudioManager()->Play3DAudio("../../Assets/Audio/pendulum.mp3", PxTransform(PxVec3(0, 177, -180) * 2), true);
		GameManager::GetAudioManager()->Play3DAudio("../../Assets/Audio/pendulum.mp3", PxTransform(PxVec3(0, 177, -200) * 2), true);
		GameManager::GetAudioManager()->Play3DAudio("../../Assets/Audio/pendulum.mp3", PxTransform(PxVec3(0, 177, -220) * 2), true);

		GameManager::GetAudioManager()->Play3DAudio("../../Assets/Audio/Rotation.mp3", PxTransform(PxVec3(36.5, 152.5, -315) * 2), true);
		GameManager::GetAudioManager()->Play3DAudio("../../Assets/Audio/Rotation.mp3", PxTransform(PxVec3(-36.5, 152.5, -315) * 2), true);
		GameManager::GetAudioManager()->Play3DAudio("../../Assets/Audio/Rotation.mp3", PxTransform(PxVec3(0, 152.5, -360) * 2), true);
		GameManager::GetAudioManager()->Play3DAudio("../../Assets/Audio/Rotation.mp3", PxTransform(PxVec3(36.5, 152.5, -410) * 2), true);
		GameManager::GetAudioManager()->Play3DAudio("../../Assets/Audio/Rotation.mp3", PxTransform(PxVec3(-36.5, 152.5, -410) * 2), true);
		break;
	case LevelState::LEVEL2:
		GameManager::GetAudioManager()->PlayAudio("../../Assets/Audio/Level2Music.mp3", true);

		GameManager::GetAudioManager()->Play3DAudio("../../Assets/Audio/Rotation.mp3", PxTransform(PxVec3(-70, -98, -900) * 2), true);
		GameManager::GetAudioManager()->Play3DAudio("../../Assets/Audio/Rotation.mp3", PxTransform(PxVec3(0, -98, -900) * 2), true);
		GameManager::GetAudioManager()->Play3DAudio("../../Assets/Audio/Rotation.mp3", PxTransform(PxVec3(70, -98, -900) * 2), true);

		GameManager::GetAudioManager()->Play3DAudio("../../Assets/Audio/Rotation.mp3", PxTransform(PxVec3(-60, -38, -1180) * 2), true);
		GameManager::GetAudioManager()->Play3DAudio("../../Assets/Audio/Rotation.mp3", PxTransform(PxVec3(0, -38, -1180) * 2), true);
		GameManager::GetAudioManager()->Play3DAudio("../../Assets/Audio/Rotation.mp3", PxTransform(PxVec3(60, -38, -1180) * 2), true);
		GameManager::GetAudioManager()->Play3DAudio("../../Assets/Audio/Rotation.mp3", PxTransform(PxVec3(-30, -23, -1235) * 2), true);
		GameManager::GetAudioManager()->Play3DAudio("../../Assets/Audio/Rotation.mp3", PxTransform(PxVec3(30, -23, -1235) * 2), true);
		GameManager::GetAudioManager()->Play3DAudio("../../Assets/Audio/Rotation.mp3", PxTransform(PxVec3(-60, -3, -1290) * 2), true);
		GameManager::GetAudioManager()->Play3DAudio("../../Assets/Audio/Rotation.mp3", PxTransform(PxVec3(0, -3, -1290) * 2), true);
		GameManager::GetAudioManager()->Play3DAudio("../../Assets/Audio/Rotation.mp3", PxTransform(PxVec3(60, -3, -1290) * 2), true);

		GameManager::GetAudioManager()->Play3DAudio("../../Assets/Audio/Rotation.mp3", PxTransform(PxVec3(0, -90, -1720) * 2), true);
		break;
	case LevelState::LEVEL3:
		GameManager::GetAudioManager()->PlayAudio("../../Assets/Audio/Level3Music.mp3", true);

		GameManager::GetAudioManager()->Play3DAudio("../../Assets/Audio/Rotation.mp3", PxTransform(PxVec3(0, 760, 30)), true);
		GameManager::GetAudioManager()->Play3DAudio("../../Assets/Audio/Rotation.mp3", PxTransform(PxVec3(0, 760, -40)), true);
		GameManager::GetAudioManager()->Play3DAudio("../../Assets/Audio/Rotation.mp3", PxTransform(PxVec3(0, 760, -110)), true);
		GameManager::GetAudioManager()->Play3DAudio("../../Assets/Audio/Rotation.mp3", PxTransform(PxVec3(0, 760, -180)), true);
		GameManager::GetAudioManager()->Play3DAudio("../../Assets/Audio/Rotation.mp3", PxTransform(PxVec3(0, 760, -250)), true);

		GameManager::GetAudioManager()->Play3DAudio("../../Assets/Audio/Rotation.mp3", PxTransform(PxVec3(-80, 760, -5)), true);
		GameManager::GetAudioManager()->Play3DAudio("../../Assets/Audio/Rotation.mp3", PxTransform(PxVec3(-80, 760, -75)), true);
		GameManager::GetAudioManager()->Play3DAudio("../../Assets/Audio/Rotation.mp3", PxTransform(PxVec3(-80, 760, -145)), true);
		GameManager::GetAudioManager()->Play3DAudio("../../Assets/Audio/Rotation.mp3", PxTransform(PxVec3(-80, 760, -215)), true);

		GameManager::GetAudioManager()->Play3DAudio("../../Assets/Audio/Rotation.mp3", PxTransform(PxVec3(80, 760, -5)), true);
		GameManager::GetAudioManager()->Play3DAudio("../../Assets/Audio/Rotation.mp3", PxTransform(PxVec3(80, 760, -75)), true);
		GameManager::GetAudioManager()->Play3DAudio("../../Assets/Audio/Rotation.mp3", PxTransform(PxVec3(80, 760, -145)), true);
		GameManager::GetAudioManager()->Play3DAudio("../../Assets/Audio/Rotation.mp3", PxTransform(PxVec3(80, 760, -215)), true);

		GameManager::GetAudioManager()->Play3DAudio("../../Assets/Audio/Rotation.mp3", PxTransform(PxVec3(160, 760, 30)), true);
		GameManager::GetAudioManager()->Play3DAudio("../../Assets/Audio/Rotation.mp3", PxTransform(PxVec3(160, 760, -40)), true);
		GameManager::GetAudioManager()->Play3DAudio("../../Assets/Audio/Rotation.mp3", PxTransform(PxVec3(160, 760, -110)), true);
		GameManager::GetAudioManager()->Play3DAudio("../../Assets/Audio/Rotation.mp3", PxTransform(PxVec3(160, 760, -180)), true);
		GameManager::GetAudioManager()->Play3DAudio("../../Assets/Audio/Rotation.mp3", PxTransform(PxVec3(160, 760, -250)), true);

		GameManager::GetAudioManager()->Play3DAudio("../../Assets/Audio/Rotation.mp3", PxTransform(PxVec3(-160, 760, 30)), true);
		GameManager::GetAudioManager()->Play3DAudio("../../Assets/Audio/Rotation.mp3", PxTransform(PxVec3(-160, 760, -40)), true);
		GameManager::GetAudioManager()->Play3DAudio("../../Assets/Audio/Rotation.mp3", PxTransform(PxVec3(-160, 760, -110)), true);
		GameManager::GetAudioManager()->Play3DAudio("../../Assets/Audio/Rotation.mp3", PxTransform(PxVec3(-160, 760, -180)), true);
		GameManager::GetAudioManager()->Play3DAudio("../../Assets/Audio/Rotation.mp3", PxTransform(PxVec3(-160, 760, -250)), true);

		GameManager::GetAudioManager()->Play3DAudio("../../Assets/Audio/Rotation.mp3", PxTransform(PxVec3(0, -100, -75)), true);
		GameManager::GetAudioManager()->Play3DAudio("../../Assets/Audio/Rotation.mp3", PxTransform(PxVec3(-120, -100, -75)), true);
		GameManager::GetAudioManager()->Play3DAudio("../../Assets/Audio/Rotation.mp3", PxTransform(PxVec3(120, -100, -75)), true);
		break;
	case LevelState::SANDBOX:
		GameManager::GetAudioManager()->PlayAudio("../../Assets/Audio/SandboxMusic.mp3", true);
		break;
	}
}

void LevelCreator::InitPlayer(const PxTransform& t, const PxReal scale)
{
	GameManager::SetPlayer(GameManager::AddPxPlayerToWorld(t, scale));
	GameManager::SetSelectionObject(GameManager::GetPlayer());
	GameManager::GetWorld()->GetMainCamera()->SetState(CameraState::THIRDPERSON);
}

void LevelCreator::InitFloors(LevelState state)
{
	//couldn't be bothered to type out the same vector every time
	PxVec3 translate = PxVec3(0, -59, -14) * 2;
	PxVec3 translate2 = PxVec3(0, 50, 0) * 2;
	Vector3 respawnSize;
	PxVec3 zone1Position, zone2Position, zone3Position, zone4Position;
	PxQuat q;



	switch (state)
	{
	case LevelState::LEVEL3:
		//Josh's second level
				//Floor 1

		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(0, -100, 75)), PxVec3(200, 1, 50), .5f, .1f, TextureState::FLOOR, Vector3(25, 10, 0));
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(0, -100, -275)), PxVec3(200, 1, 100), .5f, .1f, TextureState::FLOOR, Vector3(25, 10, 0));
		//GameManager::AddPxFloorToWorld(PxTransform(200, 100, -75), PxVec3(1, 200, 200));
		//GameManager::AddPxFloorToWorld(PxTransform(-200, 100, -75), PxVec3(1, 200, 200));
		//GameManager::AddPxFloorToWorld(PxTransform(0, 100, 125), PxVec3(200, 200, 1));
		//GameManager::AddPxFloorToWorld(PxTransform(0, 150, -275), PxVec3(200, 150, 1));
		//GameManager::AddPxFloorToWorld(PxTransform(25, -50, -275), PxVec3(175, 50, 1));
		//GameManager::AddPxFloorToWorld(PxTransform(-200, -80, -325), PxVec3(1, 20, 50));
		//GameManager::AddPxFloorToWorld(PxTransform(200, -80, -325), PxVec3(1, 20, 50));
		//GameManager::AddPxFloorToWorld(PxTransform(0, -80, -375), PxVec3(200, 20, 1));

		//Floor 2

		GameManager::AddPxFloorToWorld(PxTransform(0, 300, 150), PxVec3(200, 1, 70), .5f, .1f, TextureState::FLOOR, Vector3(25, 10, 0));
		GameManager::AddPxFloorToWorld(PxTransform(0, 300, -255), PxVec3(200, 1, 20), .5f, .1f, TextureState::FLOOR, Vector3(45, 10, 0));
		//GameManager::AddPxFloorToWorld(PxTransform(200, 500, -75), PxVec3(1, 200, 200));
		//GameManager::AddPxFloorToWorld(PxTransform(-200, 500, -75), PxVec3(1, 200, 200));
		//GameManager::AddPxFloorToWorld(PxTransform(0, 550, 125), PxVec3(200, 150, 1));
		//GameManager::AddPxFloorToWorld(PxTransform(0, 550, -275), PxVec3(200, 150, 1));
		//GameManager::AddPxFloorToWorld(PxTransform(-200, 320, 170), PxVec3(1, 20, 50));
		//GameManager::AddPxFloorToWorld(PxTransform(200, 320, 170), PxVec3(1, 20, 50));
		//GameManager::AddPxFloorToWorld(PxTransform(0, 320, 220), PxVec3(200, 20, 1));
		//GameManager::AddPxFloorToWorld(PxTransform(-25, 350, 125), PxVec3(175, 50, 1));

		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(-60, 400, 145), PxQuat(-0.5, PxVec3(0, 0, 1))), PxVec3(200, 1, 20), .5f, .1f, TextureState::FLOOR, Vector3(65, 10, 0));
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(150, 525, 145), PxQuat(0.35, PxVec3(0, 0, 1))), PxVec3(240, 1, 20), .5f, .1f, TextureState::FLOOR, Vector3(65, 10, 0));
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(-30, 600, 145), PxQuat(-0.35, PxVec3(0, 0, 1))), PxVec3(170, 1, 20), .5f, .1f, TextureState::FLOOR, Vector3(65, 10, 0));
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(110, 665, 145), PxQuat(0.35, PxVec3(0, 0, 1))), PxVec3(130, 1, 20), .5f, .1f, TextureState::FLOOR, Vector3(65, 10, 0));



		//Floor 3
		GameManager::AddPxFloorToWorld(PxTransform(0, 700, 95), PxVec3(200, 1, 32.5), .5f, .1f, TextureState::FLOOR, Vector3(25, 10, 0));
		GameManager::AddPxFloorToWorld(PxTransform(0, 700, -325), PxVec3(200, 1, 50), .5f, .1f, TextureState::FLOOR, Vector3(25, 10, 0));
		GameManager::AddPxFloorToWorld(PxTransform(-160, 700, -105), PxVec3(20, 1, 170), .5f, .1f, TextureState::FLOOR, Vector3(2, 25, 0));
		GameManager::AddPxFloorToWorld(PxTransform(-80, 700, -105), PxVec3(20, 1, 170), .5f, .1f, TextureState::FLOOR, Vector3(2, 25, 0));
		GameManager::AddPxFloorToWorld(PxTransform(0, 700, -105), PxVec3(20, 1, 170), .5f, .1f, TextureState::FLOOR, Vector3(2, 25, 0));
		GameManager::AddPxFloorToWorld(PxTransform(80, 700, -105), PxVec3(20, 1, 170), .5f, .1f, TextureState::FLOOR, Vector3(2, 25, 0));
		GameManager::AddPxFloorToWorld(PxTransform(160, 700, -105), PxVec3(20, 1, 170), .5f, .1f, TextureState::FLOOR, Vector3(2, 25, 0));
		GameManager::AddPxFloorToWorld(PxTransform(-200, 720, -325), PxVec3(1, 20, 50), .5f, .1f, TextureState::FLOOR, Vector3(25, 10, 0));
		GameManager::AddPxFloorToWorld(PxTransform(200, 720, -325), PxVec3(1, 20, 50), .5f, .1f, TextureState::FLOOR, Vector3(25, 10, 0));
		GameManager::AddPxFloorToWorld(PxTransform(0, 720, -375), PxVec3(200, 20, 1), 0.5, 0.01000000000015f, TextureState::FINISH, Vector3(10, 2, 0));


		break;
	case LevelState::LEVEL2:
		//fyi, the buffer zones go between obstacles. This is to give the player time to think so it's not just all one muscle memory dash
		//(that way, it also allows time for other players to catch up and makes each individual obstacle more chaotic, so it's a double win)
		//first we'll do the floors
		//starting zone

		//floor
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(0, 0, 0) * 2), PxVec3(200, 1, 100));
		//back wall													   
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(0, 4.5, 50) * 2), PxVec3(200, 10, 1), .5f, .1f, TextureState::WALL2, Vector3(9, 1, 0));
		//side wall left											   
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(-100, 4.5, 0) * 2), PxVec3(1, 10, 100), .5f, .1f, TextureState::WALL2, Vector3(6, 1, 0));
		//side wall right											   
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(100, 4.5, 0) * 2), PxVec3(1, 10, 100), .5f, .1f, TextureState::WALL2, Vector3(6, 1, 0));


		//OBSTACLE 0.5 - THE RAMP DOWN
		//after a short platform for players to build up some momentum, the players are sent down a ramp to build up speed
		//it should be slippery as well (possibly do some stuff with coefficients?) so that players don't have as much control

		//first ramp down (maybe some stuff to make it slippery after, it's designed to be like the total wipe out slide
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(0, -44.122, -192.85) * 2, PxQuat(Maths::DegreesToRadians(-17), PxVec3(1, 0, 0))), PxVec3(200, 1, 300), 0, 0, TextureState::ICE);
		//side wall left
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(-100, -40.3, -193.8) * 2, PxQuat(Maths::DegreesToRadians(-17), PxVec3(1, 0, 0))), PxVec3(1, 10, 305),.5f, .1f, TextureState::WALL2, Vector3(25, 1, 0));
		//side wall right
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(100, -40.3, -193.8) * 2, PxQuat(Maths::DegreesToRadians(-17), PxVec3(1, 0, 0))), PxVec3(1, 10, 305),.5f, .1f, TextureState::WALL2, Vector3(25, 1, 0));

		//buffer zone 1 (where contestants respawn on failing the first obstacle, this needs to be sorted on the individual kill plane)
		respawnSize = Vector3(180, 0, 80);
		zone1Position = PxVec3(0, -75, -384);
		//bottom kill plane
		GameManager::AddPxKillPlaneToWorld(PxTransform(PxVec3(0, -150, -325) * 2), PxVec3(500, 1, 850), zone1Position, respawnSize, false);
		//back kill plane													  
		GameManager::AddPxKillPlaneToWorld(PxTransform(PxVec3(0, 100, 100) * 2), PxVec3(500, 500, 1), zone1Position, respawnSize);

		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(0, -88, -384) * 2), PxVec3(200, 1, 100));
		//side wall left													  
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(100, -84, -384) * 2), PxVec3(1, 10, 100), .5f, .1f, TextureState::WALL2, Vector3(6, 1, 0));
		//side wall right													  
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(-100, -84, -384) * 2), PxVec3(1, 10, 100), .5f, .1f, TextureState::WALL2, Vector3(6, 1, 0));

		//OBSTACLE 1 - THE STEPPING STONE TRAMPOLINES
		//supposed to be bouncy floors/stepping stones type things, think like a combo of the trampolines from the end of splash mountain in total wipeout,
		//and the stepping stones from takeshi's castle
		//may need to add more if the jump isn't far enough
		//honestly not sure on some of these values, there may not be enough stepping stones, so we should decide if more are needed after a little testing

		//front row
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(-60, -88, -470) * 2), PxVec3(30, 1, 30));
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(0, -88, -480) * 2), PxVec3(30, 1, 30));
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(50, -88, -475) * 2), PxVec3(30, 1, 30));

		//next row														 
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(-40, -88, -550) * 2), PxVec3(30, 1, 30));
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(10, -88, -540) * 2), PxVec3(30, 1, 30));
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(60, -88, -520) * 2), PxVec3(30, 1, 30));

		//last row														 
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(-65, -88, -610) * 2), PxVec3(30, 1, 30));
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(-20, -88, -620) * 2), PxVec3(30, 1, 30));
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(20, -88, -640) * 2), PxVec3(30, 1, 30));
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(70, -88, -600) * 2), PxVec3(30, 1, 30));


		//buffer zone 2 (where contestants respawn on failing the second obstacle, this needs to be sorted on the individual kill plane)
		zone2Position = PxVec3(0, -87, -750);

		//bottom kill plane
		GameManager::AddPxKillPlaneToWorld(PxTransform(PxVec3(0, -150, -900) * 2), PxVec3(500, 1, 300), zone2Position, respawnSize, false);

		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(0, -88, -750) * 2), PxVec3(200, 1, 100));
		//side wall left													  
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(100, -78, -750) * 2), PxVec3(1, 20, 100), .5f, .1f, TextureState::WALL2, Vector3(7.5, 1.5, 0));
		//side wall right													  
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(-100, -78, -750) * 2), PxVec3(1, 20, 100), .5f, .1f, TextureState::WALL2, Vector3(7.5, 1.5, 0));

		//gate for buffer zone 2 (prevents players jumping ahead on the spinning platforms)
		//top of gate, may need to give this some height if there are problems getting onto platforms from jumping
		//have made the front walls taller to accommodate this, so only the top piece (below) needs to be adjusted
		//GameManager::AddPxFloorToWorld(PxTransform(PxVec3(0, -48, -800) * 2), PxVec3(200, 40, 1));

		//gate left														 
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(-90, -78, -800) * 2), PxVec3(20, 20, 1), .5f, .1f, TextureState::WALL2, Vector3(1.5, 1.5, 0));

		//gate left mid													 
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(-35, -78, -800) * 2), PxVec3(50, 20, 1), .5f, .1f, TextureState::WALL2, Vector3(2.5, 1.5, 0));

		//gate right mid												 
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(35, -78, -800) * 2), PxVec3(50, 20, 1), .5f, .1f, TextureState::WALL2, Vector3(2.5, 1.5, 0));

		//gate right													 
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(90, -78, -800) * 2), PxVec3(20, 20, 1), .5f, .1f, TextureState::WALL2, Vector3(1.5, 1.5, 0));

		//OBSTACLE 3 - THE SPINNING COLUMNS (WITH CANNONS EITHER SIDE)
		//I have no idea how to make these spin
		//I've added three so there's ample gaps between them to fall down, but allows more than one queue for efficiency
		//also that's what I put in the design and it looked nice
		//may need to make this a different class so it can spin, but I'm making it a floor for the prototype so we can see the level layout more easily
		//if they can't be made to spin, it might be an idea to switch these out for capsules. Should have a similar balance difficulty

		//buffer zone 3 (where contestants respawn on failing the third obstacle, this needs to be sorted on the individual kill plane)

		zone3Position = PxVec3(0, -87, -1050);
		//bottom kill plane
		GameManager::AddPxKillPlaneToWorld(PxTransform(PxVec3(0, -150, -1231) * 2), PxVec3(500, 1, 362), zone3Position, respawnSize, false);


		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(0, -88, -1050) * 2), PxVec3(200, 1, 100));
		//side wall left													   
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(100, -68, -1050) * 2), PxVec3(1, 40, 100), .5f, .1f, TextureState::WALL2, Vector3(8, 3, 0));
		//side wall right													   
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(-100, -68, -1050) * 2), PxVec3(1, 40, 100), .5f, .1f, TextureState::WALL2, Vector3(8, 3, 0));

		//this time, the gate is to stop the bowling balls hitting players in the buffer zone. 
		//could also act as a kill plane for the bowling balls
		//top of gate
		//ameManager::AddPxFloorToWorld(PxTransform(PxVec3(0, -38, -1100) * 2), PxVec3(200, 20, 1 ), .5f, .1f, TextureState::WALL2, Vector3(1.5, 1.5, 0));
																								  
		//gate left														  						
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(-90, -68, -1100) * 2), PxVec3(20, 40, 1), .5f, .1f, TextureState::WALL2, Vector3(1.5, 3, 0));
																								  
		//gate left mid													  						  
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(-35, -68, -1100) * 2), PxVec3(50, 40, 1), .5f, .1f, TextureState::WALL2, Vector3(3.2, 3, 0));
																								 
		//gate right mid												  						 
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(35, -68, -1100) * 2), PxVec3(50, 40, 1 ), .5f, .1f, TextureState::WALL2, Vector3(3.2, 3, 0));
																								 
		//gate right													  						 
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(90, -68, -1100) * 2), PxVec3(20, 40, 1 ), .5f, .1f, TextureState::WALL2, Vector3(1.5, 3, 0));

		//OBSTACLE 4 - RAMPED BOWLING ALLEY
		//so basically it's like that one bit of mario kart, but also indiana jones, takeshi's castle, and probably some other stuff
		//media tends to be surprisingly boulder centric
		//I thought it'd be fun if they were bowling balls rolling down a hill, and you were trying not to get hit 
		q = PhysxConversions::GetQuaternion(Quaternion::EulerAnglesToQuaternion(17, 0, 0));
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(0, -48, -1232) * 2, q), PxVec3(200, 1, 300), 0.1);

		//side wall left
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(-100, -48, -1225) * 2, q), PxVec3(1, 12, 310), .5f, .1f, TextureState::WALL2, Vector3(17, 1, 0));

		//side wall right
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(100, -48, -1225) * 2, q), PxVec3(1, 12, 310), .5f, .1f, TextureState::WALL2, Vector3(17, 1, 0));


		//buffer zone 4 (where contestants respawn on failing the fourth obstacle, this needs to be sorted on the individual kill plane)
		zone4Position = PxVec3(0, 56, -1411) + translate;

		//bottom kill plane
		GameManager::AddPxKillPlaneToWorld(PxTransform(PxVec3(0, -150, -1631) * 2), PxVec3(500, 1, 438), zone4Position, respawnSize, false);


		GameManager::AddPxFloorToWorld(PxTransform((PxVec3(0, 55, -1411) * 2) + translate), PxVec3(200, 1, 100));
		//side wall left
		GameManager::AddPxFloorToWorld(PxTransform((PxVec3(100, 81, -1411) * 2) + translate), PxVec3(1, 52, 100), .5f, .1f, TextureState::WALL2, Vector3(2, 4, 0));
		//side wall right
		GameManager::AddPxFloorToWorld(PxTransform((PxVec3(-100, 81, -1411) * 2) + translate), PxVec3(1, 52, 100), .5f, .1f, TextureState::WALL2, Vector3(2, 4, 0));

		GameManager::AddPxFloorToWorld(PxTransform((PxVec3(0, -88, -1630.5) * 2) + translate + translate2), PxVec3(200, 1, 339));
		//blender floor

		
		//diving boards can be used to give players an advantage in getting further into the blender
		//(if they can stay on, it's not gated to encourage players barging into each other

		//diving board left
		GameManager::AddPxFloorToWorld(PxTransform((PxVec3(-70, 45, -1495) * 2) + translate), PxVec3(20, 1, 70), 0.5, 2, TextureState::TRAMPOLINE, Vector3(1, 1, 0));

		//diving board centre
		GameManager::AddPxFloorToWorld(PxTransform((PxVec3(0, 45, -1495) * 2) + translate), PxVec3(20, 1, 70), 0.5, 2, TextureState::TRAMPOLINE, Vector3(1, 1, 0));

		//diving board right
		GameManager::AddPxFloorToWorld(PxTransform((PxVec3(70, 45, -1495) * 2) + translate), PxVec3(20, 1, 70), 0.5, 2, TextureState::TRAMPOLINE, Vector3(1,1, 0));

		//side wall left
		GameManager::AddPxFloorToWorld(PxTransform((PxVec3(-100, 35, -1630.5) * 2) + translate), PxVec3(1, 146, 339), 0.5, 0.0100000000015, TextureState::WALL2, PxVec3(15, 15, 0));

		//side wall right
		GameManager::AddPxFloorToWorld(PxTransform((PxVec3(100, 35, -1630.5) * 2) + translate), PxVec3(1, 146, 339), 0.5, 0.0100000000015, TextureState::WALL2, PxVec3(15, 15, 0));

		//back wall
		GameManager::AddPxFloorToWorld(PxTransform((PxVec3(0, 8.5, -1461) * 2) + translate), PxVec3(200, 93, 1), 0.5, 0.0100000000015, TextureState::WALL2, PxVec3(15, 15, 0));

		//should leave a gap of 20. Enough to get through, but tricky when there's a big old blender shoving everyone around
		//inspired loosely by those windmills on crazy golf
		//front wall left
		GameManager::AddPxFloorToWorld(PxTransform((PxVec3(-55, -73, -1800) * 2) + translate + translate2), PxVec3(90, 30, 1), 0.5, 0.0100000000015, TextureState::WALL2, PxVec3(6, 2, 0));

		//front wall right
		GameManager::AddPxFloorToWorld(PxTransform((PxVec3(55, -73, -1800) * 2) + translate + translate2), PxVec3(90, 30, 1), 0.5, 0.0100000000015, TextureState::WALL2, PxVec3(6, 2, 0));

		//front wall top
		GameManager::AddPxFloorToWorld(PxTransform((PxVec3(0, 0, -1800) * 2) + translate + translate2), PxVec3(200, 116, 1), 0.5, 0.0100000000015, TextureState::WALL2, PxVec3(15, 10, 0));

		//roof
		//GameManager::AddPxFloorToWorld(PxTransform((PxVec3(0, 107, -1581) * 2) + translate), PxVec3(200, 1, 439));

		//VICTORY PODIUM
		//the room you have to get in after the blender
		//press the button to win
		//(button + podium yet to be added)
		//also want a sliding door that locks players out
		//pressing the button should make all the floors drop out
		//Podium floor
		GameManager::AddPxFloorToWorld(PxTransform((PxVec3(0, -88, -1825) * 2) + translate + translate2), PxVec3(50, 1, 50));

		//side wall left
		GameManager::AddPxFloorToWorld(PxTransform((PxVec3(-25, -63, -1825) * 2) + translate + translate2), PxVec3(1, 50, 50), 0.5, 0.0100000000015, TextureState::WALL2, PxVec3(8, 8, 0));

		//side wall right
		GameManager::AddPxFloorToWorld(PxTransform((PxVec3(25, -63, -1825) * 2) + translate + translate2), PxVec3(1, 50, 50), 0.5, 0.0100000000015, TextureState::WALL2, PxVec3(5, 5, 0));

		//end wall
		//GameManager::AddPxFloorToWorld(PxTransform((PxVec3(0, -90, -1835) * 2) + translate), PxVec3(500, 90, 0), .5f, .1f, TextureState::WALL2);

		GameManager::AddPxFloorToWorld(PxTransform((PxVec3(0, -63, -1850) * 2) + translate + translate2), PxVec3(50, 50, 1), 0.5, 0.0100000000015, TextureState::FINISH, PxVec3(2, 2, 0));
		//roof
		//GameManager::AddPxFloorToWorld(PxTransform((PxVec3(0, -38, -1825) * 2) + translate + translate2), PxVec3(50, 1, 50));
		break;

	case LevelState::LEVEL1:
		//floor
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(0, 80, 40) * 2), PxVec3(100, 1, 150));
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(0, 115, -99.5) * 2), PxVec3(100, 70, 1), 0.5F, 0.1F, TextureState::WALL2, Vector3(5, 5, 1));
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(0, 90, -118) * 2), PxVec3(500, 120, 1), 0, 0.1f, TextureState::INVISIBLE);

		//Wall Trambolines		
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(0, 85, 30) * 2), PxVec3(25, 1, 25), 0.5F, 2.0f, TextureState::TRAMPOLINE, Vector3(1, 1, 1));
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(30, 95, 0) * 2), PxVec3(25, 1, 25), 0.5F, 2.0f, TextureState::TRAMPOLINE, Vector3(1, 1, 1));
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(60, 105, -30) * 2), PxVec3(25, 1, 25), 0.5F, 2.0f, TextureState::TRAMPOLINE, Vector3(1, 1, 1));
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(90, 115, -60) * 2), PxVec3(25, 1, 25), 0.5F, 2.0f, TextureState::TRAMPOLINE, Vector3(1, 1, 1));
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(60, 125, -85) * 2), PxVec3(25, 1, 25), 0.5F, 2.0f, TextureState::TRAMPOLINE, Vector3(1, 1, 1));
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(30, 135, -85) * 2), PxVec3(25, 1, 25), 0.5F, 2.0f, TextureState::TRAMPOLINE, Vector3(1, 1, 1));

		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(-30, 95, 0) * 2), PxVec3(25, 1, 25), 0.5F, 2.0f, TextureState::TRAMPOLINE, Vector3(1, 1, 1));
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(-60, 105, -30) * 2), PxVec3(25, 1, 25), 0.5F, 2.0f, TextureState::TRAMPOLINE, Vector3(1, 1, 1));
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(-90, 115, -60) * 2), PxVec3(25, 1, 25), 0.5F, 2.0f, TextureState::TRAMPOLINE, Vector3(1, 1, 1));
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(-60, 125, -85) * 2), PxVec3(25, 1, 25), 0.5F, 2.0f, TextureState::TRAMPOLINE, Vector3(1, 1, 1));
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(-30, 135, -85) * 2), PxVec3(25, 1, 25), 0.5F, 2.0f, TextureState::TRAMPOLINE, Vector3(1, 1, 1));

		//buffer zone 1 (where contestants respawn on failing the first obstacle, this needs to be sorted on the individual kill plane)
		respawnSize = Vector3(100, 0, 45);
		zone1Position = PxVec3(0, 81, 90);

		//Kill PLlanes for out of bounce 
		GameManager::AddPxKillPlaneToWorld(PxTransform(PxVec3(0, 60, 50) * 2), PxVec3(500, 1, 400), zone1Position, respawnSize, false);

		//Kill PLlanes for out of bounce 

		//2nd floor															 
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(0, 150, -124.5) * 2), PxVec3(100, 1, 50));
		//side wall left													 
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(50, 154.5, -124.5) * 2), PxVec3(1, 10, 50), 0.5F, 0.1F, TextureState::WALL2, Vector3(5,1.5,0));
		//side wall right													 
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(-50, 154.5, -124.5) * 2), PxVec3(1, 10, 50), 0.5F, 0.1F, TextureState::WALL2, Vector3(5, 1.5, 0));

		//BRIDGE 															 
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(0, 150, -199.5) * 2), PxVec3(20, 1, 100));


		//2nd Floor, after bridge											 
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(0, 150, -270) * 2), PxVec3(100, 1, 50));
		//side wall left													 
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(50, 154.5, -270) * 2), PxVec3(1, 10, 50), 0.5F, 0.1F, TextureState::WALL2, Vector3(5, 1.5, 0));
		//side wall right													 
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(-50, 154.5, -270) * 2), PxVec3(1, 10, 50), 0.5F, 0.1F, TextureState::WALL2, Vector3(5, 1.5, 0));

		//right platform													 
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(36.5, 150, -315) * 2), PxVec3(25, 1, 25));

		//left platform														 
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(-36.5, 150, -315) * 2), PxVec3(25, 1, 25));

		//Center platform													 
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(0, 150, -360) * 2), PxVec3(50, 1, 50));

		//right platform													 
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(36.5, 150, -410) * 2), PxVec3(25, 1, 25));

		//left platform														 
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(-36.5, 150, -410) * 2), PxVec3(25, 1, 25));

		//buffer zone 2
		zone2Position = PxVec3(0, 153, -112);
		GameManager::AddPxKillPlaneToWorld(PxTransform(PxVec3(0, 60.1, -200) * 2), PxVec3(500, 1, 170), zone2Position, respawnSize, false);
		//platform after blender platforms
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(0, 150, -460) * 2), PxVec3(100, 1, 50));
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(0, 99, -450) * 2), PxVec3(500, 100, 1), 0, 0.1F, TextureState::INVISIBLE);
		//side wall left
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(50, 154.5, -460) * 2), PxVec3(1, 10, 50), 0.5F, 0.1F, TextureState::WALL2, Vector3(5, 1.5, 0));
		//side wall right
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(-50, 154.5, -460) * 2), PxVec3(1, 10, 50), 0.5F, 0.1F, TextureState::WALL2, Vector3(5, 1.5, 0));

		//buffer zone 3
		zone3Position = PxVec3(0, 153, -270);
		GameManager::AddPxKillPlaneToWorld(PxTransform(PxVec3(0, 60, -375) * 2), PxVec3(500, 1, 190), zone3Position, respawnSize, false);
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(0, 99, -272) * 2), PxVec3(500, 100, 1), 0, 0.1F, TextureState::INVISIBLE);
		//slippery ramp
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(0, 120, -580) * 2,
			PhysxConversions::GetQuaternion(Quaternion::EulerAnglesToQuaternion(-17, 0, 0))), PxVec3(100, 1, 200), 0, 0, TextureState::ICE);

		//buffer zone 4
		zone4Position = PxVec3(0, 153, -460);
		GameManager::AddPxKillPlaneToWorld(PxTransform(PxVec3(0, 60.1, -700) * 2), PxVec3(500, 1, 500), zone4Position, respawnSize, false);
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(0, 99, -470) * 2), PxVec3(500, 100, 1), 0, 0.1F, TextureState::INVISIBLE);

		//Floor after ramp
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(0, 91, -825) * 2), PxVec3(100, 1, 300));
		//side wall left												  
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(50, 95.5, -825) * 2), PxVec3(1, 10, 300), 0.5F, 0.1F, TextureState::WALL2, Vector3(25, 1.5, 0));
		//side wall right												  
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(-50, 95.5, -825) * 2), PxVec3(1, 10, 300), 0.5F, 0.1F, TextureState::WALL2, Vector3(25, 1.5, 0));
		//wall at the end... final wall
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(0, 91, -977.5) * 2), PxVec3(100, 1, 5), 0.5F, 0.1F, TextureState::FINISH, Vector3(2, 1, 0));
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(-50, 115, -975) * 2), PxVec3(1, 40, 1), 0.5F, 0.1F, TextureState::WALL, Vector3(5, 15, 0));
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(50, 115, -975) * 2), PxVec3(1, 40, 1), 0.5F, 0.1F, TextureState::WALL, Vector3(5, 15, 0));
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(0, 130, -975) * 2), PxVec3(100, 7, 1), 0.5F, 0.1F, TextureState::FINISH, Vector3(2, 1, 0));

		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(0, 91, -990) * 2), PxVec3(100, 1, 20), 0.5F, 0.1F);
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(-50, 95.5, -990) * 2), PxVec3(1, 10, 30), 0.5F, 0.1F, TextureState::WALL2, Vector3(5, 2, 0));
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(50, 95.5, -990) * 2), PxVec3(1, 10, 30), 0.5F, 0.1F, TextureState::WALL2, Vector3(5, 2, 0));
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(0, 95.5, -1000) * 2), PxVec3(100, 10, 1),0.5F, 0.1F, TextureState::WALL2, Vector3(10, 2, 0));

		//Pillars
		for (int i = 0; i <= 15; i++)
		{
			GameManager::AddPxFloorToWorld(PxTransform(PxVec3(-44 + (i * 6), 98, -775) * 2), PxVec3(2, 15, 2), 0.5F, 0.1F, TextureState::WALL);
			GameManager::AddPxFloorToWorld(PxTransform(PxVec3(-44 + (i * 6), 98, -825) * 2), PxVec3(2, 15, 2), 0.5F, 0.1F, TextureState::WALL);
			GameManager::AddPxFloorToWorld(PxTransform(PxVec3(-44 + (i * 6), 98, -875) * 2), PxVec3(2, 15, 2), 0.5F, 0.1F, TextureState::WALL);
		}

		//First row of wall obstacle 
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(-47, 98, -775) * 2), PxVec3(4, 15, 2), 0.5F, 0.1F, TextureState::WALL);//side wall blocking remaining gap
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(-41, 95.75, -775) * 2), PxVec3(4, 11.5f, 2), 0.5F, 0.1F, TextureState::RED);
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(-35, 95.75, -775) * 2), PxVec3(4, 11.5f, 2), 0.5F, 0.1F, TextureState::RED);
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(-29, 95.75, -775) * 2), PxVec3(4, 11.5f, 2), 0.5F, 0.1F, TextureState::RED);
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(-17, 95.75, -775) * 2), PxVec3(4, 11.5f, 2), 0.5F, 0.1F, TextureState::RED);
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(-11, 95.75, -775) * 2), PxVec3(4, 11.5f, 2), 0.5F, 0.1F, TextureState::RED);
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(-5, 95.75, -775) * 2), PxVec3(4, 11.5f, 2), 0.5F, 0.1F, TextureState::RED);
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(1, 95.75, -775) * 2), PxVec3(4, 11.5f, 2), 0.5F, 0.1F, TextureState::RED);
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(7, 95.75, -775) * 2), PxVec3(4, 11.5f, 2), 0.5F, 0.1F, TextureState::RED);
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(13, 95.75, -775) * 2), PxVec3(4, 11.5f, 2), 0.5F, 0.1F, TextureState::RED);
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(25, 95.75, -775) * 2), PxVec3(4, 11.5f, 2), 0.5F, 0.1F, TextureState::RED);
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(31, 95.75, -775) * 2), PxVec3(4, 11.5f, 2), 0.5F, 0.1F, TextureState::RED);
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(37, 95.75, -775) * 2), PxVec3(4, 11.5f, 2), 0.5F, 0.1F, TextureState::RED);
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(48, 98, -775) * 2), PxVec3(4, 15, 2), 0.5F, 0.1F, TextureState::WALL);//side wall blocking remaining gap

		//Middle of wall obstacle
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(-47, 98, -825) * 2), PxVec3(4, 15, 2), 0.5F, 0.1F, TextureState::WALL);//side wall blocking remaining gap
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(-41, 95.75, -825) * 2), PxVec3(4, 11.5, 2), 0.5F, 0.1F, TextureState::RED);
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(-29, 95.75, -825) * 2), PxVec3(4, 11.5, 2), 0.5F, 0.1F, TextureState::RED);
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(-23, 95.75, -825) * 2), PxVec3(4, 11.5, 2), 0.5F, 0.1F, TextureState::RED);
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(-17, 95.75, -825) * 2), PxVec3(4, 11.5, 2), 0.5F, 0.1F, TextureState::RED);
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(-11, 95.75, -825) * 2), PxVec3(4, 11.5, 2), 0.5F, 0.1F, TextureState::RED);
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(-5, 95.75, -825) * 2), PxVec3(4, 11.5, 2), 0.5F, 0.1F, TextureState::RED);
		//gap in wall										  									
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(7, 95.75, -825) * 2), PxVec3(4, 11.5, 2), 0.5F, 0.1F, TextureState::RED);
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(13, 95.75, -825) * 2), PxVec3(4, 11.5, 2), 0.5F, 0.1F, TextureState::RED);
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(19, 95.75, -825) * 2), PxVec3(4, 11.5, 2), 0.5F, 0.1F, TextureState::RED);
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(25, 95.75, -825) * 2), PxVec3(4, 11.5, 2), 0.5F, 0.1F, TextureState::RED);
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(31, 95.75, -825) * 2), PxVec3(4, 11.5, 2), 0.5F, 0.1F, TextureState::RED);
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(37, 95.75, -825) * 2), PxVec3(4, 11.5, 2), 0.5F, 0.1F, TextureState::RED);
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(43, 95.75, -825) * 2), PxVec3(4, 11.5, 2), 0.5F, 0.1F, TextureState::RED);
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(48, 98, -825) * 2), PxVec3(4, 15, 2), 0.5F, 0.1F, TextureState::WALL);//side wall blocking remaining gap

		//Last row of wall obstacle 									  
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(-47, 98, -875) * 2), PxVec3(4, 15, 2), 0.5F, 0.1F, TextureState::WALL);//side wall blocking remaining gap
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(-41, 95.75, -875) * 2), PxVec3(4, 11.5, 2), 0.5F, 0.1F, TextureState::RED);
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(-35, 95.75, -875) * 2), PxVec3(4, 11.5, 2), 0.5F, 0.1F, TextureState::RED);
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(-29, 95.75, -875) * 2), PxVec3(4, 11.5, 2), 0.5F, 0.1F, TextureState::RED);
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(-23, 95.75, -875) * 2), PxVec3(4, 11.5, 2), 0.5F, 0.1F, TextureState::RED);
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(-17, 95.75, -875) * 2), PxVec3(4, 11.5, 2), 0.5F, 0.1F, TextureState::RED);
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(-11, 95.75, -875) * 2), PxVec3(4, 11.5, 2), 0.5F, 0.1F, TextureState::RED);
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(-5, 95.75, -875) * 2), PxVec3(4, 11.5, 2), 0.5F, 0.1F, TextureState::RED);
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(1, 95.75, -875) * 2), PxVec3(4, 11.5, 2), 0.5F, 0.1F, TextureState::RED);
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(7, 95.75, -875) * 2), PxVec3(4, 11.5, 2), 0.5F, 0.1F, TextureState::RED);
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(13, 95.75, -875) * 2), PxVec3(4, 11.5, 2), 0.5F, 0.1F, TextureState::RED);
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(19, 95.75, -875) * 2), PxVec3(4, 11.5, 2), 0.5F, 0.1F, TextureState::RED);
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(25, 95.75, -875) * 2), PxVec3(4, 11.5, 2), 0.5F, 0.1F, TextureState::RED);
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(31, 95.75, -875) * 2), PxVec3(4, 11.5, 2), 0.5F, 0.1F, TextureState::RED);
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(37, 95.75, -875) * 2), PxVec3(4, 11.5, 2), 0.5F, 0.1F, TextureState::RED);
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(48, 98, -875) * 2), PxVec3(4, 15, 2), 0.5F, 0.1F, TextureState::WALL);//side wall blocking remaining gap
		break;
	case LevelState::SANDBOX:
		GameManager::AddPxFloorToWorld(PxTransform(PxVec3(0, -20, 0)), PxVec3(1000, 1, 1000));
		break;
	}
}

/* Initialises all game objects, enemies etc */
void LevelCreator::InitGameExamples(LevelState state)
{
	switch (state)
	{
	case LevelState::LEVEL3:
		InitPlayer(PxTransform(PxVec3(0, -100, 100)), 1);


		//power ups
		//floor 1
		GameManager::AddPxSpeedPower(PxTransform(PxVec3(-20, -92, -75)), 5);
		GameManager::AddPxSpeedPower(PxTransform(PxVec3(-40, -92, -75)), 5);
		GameManager::AddPxSpeedPower(PxTransform(PxVec3(20, -92, -75)), 5);
		GameManager::AddPxSpeedPower(PxTransform(PxVec3(40, -92, -75)), 5);

		GameManager::AddPxSpeedPower(PxTransform(PxVec3(0, -92, -75)), 5);
		GameManager::AddPxSpeedPower(PxTransform(PxVec3(0, -92, -35)), 5);
		GameManager::AddPxSpeedPower(PxTransform(PxVec3(0, -92, -95)), 5);
		GameManager::AddPxSpeedPower(PxTransform(PxVec3(0, -92, -115)), 5);
		GameManager::AddPxSpeedPower(PxTransform(PxVec3(0, -92, -135)), 5);

		//floor 2
		for (int z = 0; z < 4; ++z)
		{
			for (int x = 0; x < 5 - (z % 2); ++x)
			{
				GameManager::AddPxFallingTileToWorld(PxTransform(PxVec3((-180 + (20 * (z % 2))) + (x * 80), 300, 30 - (z * 80))), PxVec3(20, 1, 20));
				if (x % 2 == 0)
				{
					GameManager::AddPxLongJump(PxTransform(PxVec3((-180 + (20 * (z % 2))) + (x * 80), 308, 30 - (z * 80))), 5);
				}
				else
				{
					GameManager::AddPxCoinToWorld(PxTransform(PxVec3((-180 + (20 * (z % 2))) + (x * 80), 308, 30 - (z * 80))), 5);

				}
			}
		}

		//floor 3
		for (int i = 0; i < 4; ++i)
		{
			GameManager::AddPxSpeedPower(PxTransform(PxVec3(0, 708, -5 - (i * 70))), 5);
			GameManager::AddPxSpeedPower(PxTransform(PxVec3(160, 708, -5 - (i * 70))), 5);
			GameManager::AddPxSpeedPower(PxTransform(PxVec3(-160, 708, -5 - (i * 70))), 5);
		}
		break;
	case LevelState::LEVEL2:
		//player added to check this is all a reasonable scale
		InitPlayer(PxTransform(PxVec3(0, 10, 0)), 1);

		//power ups
		for (int i = 0; i < 4; ++i)
		{
			GameManager::AddPxSpeedPower(PxTransform(PxVec3(-55 + (i * 40), 5, -50) * 2), 3);
		}

		for (int i = 0; i < 3; ++i) {
			GameManager::AddPxSpeedPower(PxTransform(PxVec3(-70 + (i * 70), -85, -800) * 2), 5);
		}

		GameManager::AddPxLongJump(PxTransform(PxVec3(-60, -85, -470) * 2), 3);
		GameManager::AddPxCoinToWorld(PxTransform(PxVec3(0, -85, -480) * 2), 3);
		GameManager::AddPxLongJump(PxTransform(PxVec3(50, -85, -475) * 2), 3);
		GameManager::AddPxCoinToWorld(PxTransform(PxVec3(-40, -85, -550) * 2), 3);
		GameManager::AddPxLongJump(PxTransform(PxVec3(10, -85, -540) * 2), 3);
		GameManager::AddPxCoinToWorld(PxTransform(PxVec3(60, -85, -520) * 2), 3);
		GameManager::AddPxLongJump(PxTransform(PxVec3(-65, -85, -610) * 2), 3);
		GameManager::AddPxCoinToWorld(PxTransform(PxVec3(-20, -85, -620) * 2), 3);
		GameManager::AddPxLongJump(PxTransform(PxVec3(20, -85, -640) * 2), 3);
		GameManager::AddPxCoinToWorld(PxTransform(PxVec3(70, -85, -600) * 2), 3);
		break;
	case LevelState::LEVEL1:
		InitPlayer(PxTransform(PxVec3(0, 180, 150)), 1);

		GameManager::AddPxCoinToWorld(PxTransform(PxVec3(0, 89, 30) * 2), 3);
		GameManager::AddPxCoinToWorld(PxTransform(PxVec3(30, 99, 0) * 2), 3);
		GameManager::AddPxCoinToWorld(PxTransform(PxVec3(-30, 99, 0) * 2), 3);
		GameManager::AddPxCoinToWorld(PxTransform(PxVec3(60, 129, -85) * 2), 3);
		GameManager::AddPxCoinToWorld(PxTransform(PxVec3(-60, 129, -85) * 2), 3);

		//AFTER TRAMPOLINES
		GameManager::AddPxSpeedPower(PxTransform(PxVec3(0, 154, -124.5) * 2), 3);

		//Powerup before jumping part with blenders
		GameManager::AddPxLongJump(PxTransform(PxVec3(36, 154, -280) * 2), 3);
		GameManager::AddPxLongJump(PxTransform(PxVec3(-36, 154, -280) * 2), 3);
		GameManager::AddPxCoinToWorld(PxTransform(PxVec3(0, 154, -270) * 2), 3);
		GameManager::AddPxCoinToWorld(PxTransform(PxVec3(0, 154, -260) * 2), 3);

		//Slope  coins

		GameManager::AddPxCoinToWorld(PxTransform(PxVec3(22, 134, -545) * 2), 3);
		GameManager::AddPxCoinToWorld(PxTransform(PxVec3(4, 125, -575) * 2), 3);
		GameManager::AddPxCoinToWorld(PxTransform(PxVec3(-15, 110, -625) * 2), 3);

		//Toilet section powerUps/coins
		GameManager::AddPxLongJump(PxTransform(PxVec3(-30, 95, -720) * 2), 3);
		GameManager::AddPxSpeedPower(PxTransform(PxVec3(-15, 95, -720) * 2), 3);
		GameManager::AddPxCoinToWorld(PxTransform(PxVec3(0, 95, -720) * 2), 3);
		GameManager::AddPxLongJump(PxTransform(PxVec3(15, 95, -720) * 2), 3);
		GameManager::AddPxSpeedPower(PxTransform(PxVec3(30, 95, -720) * 2), 3);

		GameManager::AddPxCoinToWorld(PxTransform(PxVec3(30, 95, -800) * 2), 3);

		GameManager::AddPxCoinToWorld(PxTransform(PxVec3(-30, 95, -850) * 2), 3);
		//GameManager::AddPxFloorToWorld(PxTransform(PxVec3(0, 91, -825) * 2), PxVec3(100, 1, 300));
		break;
	case LevelState::SANDBOX:
		InitPlayer(PxTransform(PxVec3(0, 10, 100)), 1);
		GameManager::AddPxCoinToWorld(PxTransform(PxVec3(-50, -10, -100)), 3);
		GameManager::AddPxLongJump(PxTransform(PxVec3(0, -10, -100)), 3);
		GameManager::AddPxSpeedPower(PxTransform(PxVec3(50, -10, -100)), 3);
		for (int i = 0; i < 10; i++)
		{
			for (int j = -20; j < -10; j++)
			{
				GameManager::AddPxSphereToWorld(PxTransform(PxVec3(j * 10, 5, i * 10)), 2, abs((9 - i) * 10) + 0.1);
			}
		}
		for (int i = 0; i < 10; i++)
		{
			for (int j = -10; j < 0; j++)
			{
				GameManager::AddPxCubeToWorld(PxTransform(PxVec3(j * 10, 5, i * 10)), PxVec3(2, 2, 2), abs((9 - i) * 10) + 0.1);
			}
		}
		for (int i = 0; i < 10; i++)
		{
			for (int j = 0; j < 10; j++)
			{
				GameManager::AddPxCapsuleToWorld(PxTransform(PxVec3(j * 10, 5, i * 10)), 2, 2, abs((9 - i) * 10) + 0.1);
			}
		}
		for (int i = 0; i < 10; i++)
		{
			for (int j = 10; j < 20; j++)
			{
				GameManager::AddPxCylinderToWorld(PxTransform(PxVec3(j * 10, 5, i * 10)), 2, 2, abs((9 - i) * 10) + 0.1);
			}
		}
		//GameManager::AddPxEnemyToWorld(PxTransform(PxVec3(20, 20, 0)), 1);	
		break;
	}
}

/* This method will initialise any other moveable obstacles we want */
void LevelCreator::InitGameObstacles(LevelState state)
{
	PxVec3 translate = PxVec3(0, -59, -14) * 2;
	PxVec3 translate2 = PxVec3(0, 50, 0) * 2;
	PxQuat q;

	switch (state)
	{
	case LevelState::LEVEL3:
		//Killplane
		GameManager::AddPxKillPlaneToWorld(PxTransform(PxVec3(0, -130, 0)), PxVec3(1000, 1, 1000), PxVec3(0, -50, 50), PxVec3(90, 1, 20), false);

		//Floor 1
		GameManager::AddPxRotatingCubeToWorld(PxTransform(PxVec3(0, -100, -75)), PxVec3(20, 1, 80), PxVec3(0, 2, 0), 0.5, 0.5, false, TextureState::WOOD, Vector3(1, 3, 0));



		GameManager::AddPxRotatingCubeToWorld(PxTransform(PxVec3(-120, -100, -75)), PxVec3(20, 1, 60), PxVec3(-0.25, 0, 0), 0.5, 0.5, false, TextureState::WOOD, Vector3(1, 3, 0));
		GameManager::AddPxRotatingCubeToWorld(PxTransform(PxVec3(-120, -100, -75)), PxVec3(20, 60, 1), PxVec3(-0.25, 0, 0), 0.5, 0.5, false, TextureState::WOOD, Vector3(1, 3, 0));
		GameManager::AddPxRotatingCubeToWorld(PxTransform(PxVec3(120, -100, -75)), PxVec3(20, 1, 60), PxVec3(-0.25, 0, 0), 0.5, 0.5, false, TextureState::WOOD, Vector3(1, 3, 0));
		GameManager::AddPxRotatingCubeToWorld(PxTransform(PxVec3(120, -100, -75)), PxVec3(20, 60, 1), PxVec3(-0.25, 0, 0), 0.5, 0.5, false, TextureState::WOOD, Vector3(1, 3, 0));

		//Floor 1-2 connection
		GameManager::AddBounceSticks(PxTransform(PxVec3(-160, -100, -300), PxQuat(1.5708, PxVec3(1, 0, 0))), 10, 20, 10, 0.5, 3);
		GameManager::AddBounceSticks(PxTransform(PxVec3(-80, -100, -300), PxQuat(1.5708, PxVec3(1, 0, 0))), 10, 20, 10, 0.5, 3);
		GameManager::AddBounceSticks(PxTransform(PxVec3(0, -100, -300), PxQuat(1.5708, PxVec3(1, 0, 0))), 10, 20, 10, 0.5, 3);
		GameManager::AddBounceSticks(PxTransform(PxVec3(80, -100, -300), PxQuat(1.5708, PxVec3(1, 0, 0))), 10, 20, 10, 0.5, 3);
		GameManager::AddBounceSticks(PxTransform(PxVec3(160, -100, -300), PxQuat(1.5708, PxVec3(1, 0, 0))), 10, 20, 10, 0.5, 3);

		GameManager::AddBounceSticks(PxTransform(PxVec3(-120, -20, -300), PxQuat(1.5708, PxVec3(1, 0, 0))), 10, 20, 10, 0.5, 3);
		GameManager::AddBounceSticks(PxTransform(PxVec3(40, -20, -300), PxQuat(1.5708, PxVec3(1, 0, 0))), 10, 20, 10, 0.5, 3);
		GameManager::AddBounceSticks(PxTransform(PxVec3(-40, -20, -300), PxQuat(1.5708, PxVec3(1, 0, 0))), 10, 20, 10, 0.5, 4);
		GameManager::AddBounceSticks(PxTransform(PxVec3(-120, -20, -300), PxQuat(1.5708, PxVec3(1, 0, 0))), 10, 20, 10, 0.5, 3);

		GameManager::AddBounceSticks(PxTransform(PxVec3(-160, 60, -300), PxQuat(1.5708, PxVec3(1, 0, 0))), 10, 20, 10, 0.5, 3);
		GameManager::AddBounceSticks(PxTransform(PxVec3(-80, 60, -300), PxQuat(1.5708, PxVec3(1, 0, 0))), 10, 20, 10, 0.5, 3);
		GameManager::AddBounceSticks(PxTransform(PxVec3(0, 60, -300), PxQuat(1.5708, PxVec3(1, 0, 0))), 10, 20, 10, 0.5, 3);
		GameManager::AddBounceSticks(PxTransform(PxVec3(80, 60, -300), PxQuat(1.5708, PxVec3(1, 0, 0))), 10, 20, 10, 0.5, 3);
		GameManager::AddBounceSticks(PxTransform(PxVec3(160, 60, -300), PxQuat(1.5708, PxVec3(1, 0, 0))), 10, 20, 10, 0.5, 3);


		GameManager::AddBounceSticks(PxTransform(PxVec3(-120, 140, -300), PxQuat(1.5708, PxVec3(1, 0, 0))), 10, 20, 10, 0.5, 3);
		GameManager::AddBounceSticks(PxTransform(PxVec3(40, 140, -300), PxQuat(1.5708, PxVec3(1, 0, 0))), 10, 20, 10, 0.5, 3);
		GameManager::AddBounceSticks(PxTransform(PxVec3(-40, 140, -300), PxQuat(1.5708, PxVec3(1, 0, 0))), 10, 20, 10, 0.5, 3);
		GameManager::AddBounceSticks(PxTransform(PxVec3(-120, 140, -300), PxQuat(1.5708, PxVec3(1, 0, 0))), 10, 20, 10, 0.5, 3);

		GameManager::AddBounceSticks(PxTransform(PxVec3(-160, 220, -300), PxQuat(1.5708, PxVec3(1, 0, 0))), 10, 20, 10, 0.5, 3);
		GameManager::AddBounceSticks(PxTransform(PxVec3(-80, 220, -300), PxQuat(1.5708, PxVec3(1, 0, 0))), 10, 20, 10, 0.5, 3);
		GameManager::AddBounceSticks(PxTransform(PxVec3(0, 220, -300), PxQuat(1.5708, PxVec3(1, 0, 0))), 10, 20, 10, 0.5, 3);
		GameManager::AddBounceSticks(PxTransform(PxVec3(80, 220, -300), PxQuat(1.5708, PxVec3(1, 0, 0))), 10, 20, 10, 0.5, 3);
		GameManager::AddBounceSticks(PxTransform(PxVec3(160, 220, -300), PxQuat(1.5708, PxVec3(1, 0, 0))), 10, 20, 10, 0.5, 3);

		GameManager::AddBounceSticks(PxTransform(PxVec3(-120, 300, -300), PxQuat(1.5708, PxVec3(1, 0, 0))), 10, 20, 10, 0.5, 3);
		GameManager::AddBounceSticks(PxTransform(PxVec3(40, 300, -300), PxQuat(1.5708, PxVec3(1, 0, 0))), 10, 20, 10, 0.5, 3);
		GameManager::AddBounceSticks(PxTransform(PxVec3(-40, 300, -300), PxQuat(1.5708, PxVec3(1, 0, 0))), 10, 20, 10, 0.5, 3);
		GameManager::AddBounceSticks(PxTransform(PxVec3(-120, 300, -300), PxQuat(1.5708, PxVec3(1, 0, 0))), 10, 20, 10, 0.5, 3);

		//Floor 2
		for (int z = 0; z < 4; ++z)
		{
			for (int x = 0; x < 5 - (z % 2); ++x)
			{
				GameManager::AddPxFallingTileToWorld(PxTransform(PxVec3((-180 + (20 * (z % 2))) + (x * 80), 300, 30 - (z * 80))), PxVec3(20, 1, 20));
			}
		}

		//Floor 2-3 connection
		GameManager::AddPxCannonToWorld(PxTransform(PxVec3(-180, 700, 145)), PxVec3(50, 0, 0), 15, 15, PxVec3(50, 0, 0));
		for (int i = 0; i < 30; i++)
		{
			GameManager::GetObstacles()->cannons.push_back(GameManager::AddPxCannonBallToWorld(PxTransform(PxVec3(-200, 700, 145) * 2), 10, new PxVec3(25, 0, 0), 37));
		}

		//floor 3 (pendulums)
		GameManager::AddPxPendulumToWorld(PxTransform(PxVec3(0, 760, 30)), 10, 30, 5, true, 0.5f, 2);
		GameManager::AddPxPendulumToWorld(PxTransform(PxVec3(0, 760, -40)), 10, 30, 5, true, 0.5f, 2);
		GameManager::AddPxPendulumToWorld(PxTransform(PxVec3(0, 760, -110)), 10, 30, 5, true, 0.5f, 2);
		GameManager::AddPxPendulumToWorld(PxTransform(PxVec3(0, 760, -180)), 10, 30, 5, true, 0.5f, 2);
		GameManager::AddPxPendulumToWorld(PxTransform(PxVec3(0, 760, -250)), 10, 30, 5, true, 0.5f, 2);

		GameManager::AddPxPendulumToWorld(PxTransform(PxVec3(-80, 760, -5)), 10, 30, 5, false, 0.5f, 2);
		GameManager::AddPxPendulumToWorld(PxTransform(PxVec3(-80, 760, -75)), 10, 30, 5, false, 0.5f, 2);
		GameManager::AddPxPendulumToWorld(PxTransform(PxVec3(-80, 760, -145)), 10, 30, 5, false, 0.5f, 2);
		GameManager::AddPxPendulumToWorld(PxTransform(PxVec3(-80, 760, -215)), 10, 30, 5, false, 0.5f, 2);

		GameManager::AddPxPendulumToWorld(PxTransform(PxVec3(80, 760, -5)), 10, 30, 5, false, 0.5f, 2);
		GameManager::AddPxPendulumToWorld(PxTransform(PxVec3(80, 760, -75)), 10, 30, 5, false, 0.5f, 2);
		GameManager::AddPxPendulumToWorld(PxTransform(PxVec3(80, 760, -145)), 10, 30, 5, false, 0.5f, 2);
		GameManager::AddPxPendulumToWorld(PxTransform(PxVec3(80, 760, -215)), 10, 30, 5, false, 0.5f, 2);

		GameManager::AddPxPendulumToWorld(PxTransform(PxVec3(160, 760, 30)), 10, 30, 5, true, 0.5f, 2);
		GameManager::AddPxPendulumToWorld(PxTransform(PxVec3(160, 760, -40)), 10, 30, 5, true, 0.5f, 2);
		GameManager::AddPxPendulumToWorld(PxTransform(PxVec3(160, 760, -110)), 10, 30, 5, true, 0.5f, 2);
		GameManager::AddPxPendulumToWorld(PxTransform(PxVec3(160, 760, -180)), 10, 30, 5, true, 0.5f, 2);
		GameManager::AddPxPendulumToWorld(PxTransform(PxVec3(160, 760, -250)), 10, 30, 5, true, 0.5f, 2);

		GameManager::AddPxPendulumToWorld(PxTransform(PxVec3(-160, 760, 30)), 10, 30, 5, true, 0.5f, 2);
		GameManager::AddPxPendulumToWorld(PxTransform(PxVec3(-160, 760, -40)), 10, 30, 5, true, 0.5f, 2);
		GameManager::AddPxPendulumToWorld(PxTransform(PxVec3(-160, 760, -110)), 10, 30, 5, true, 0.5f, 2);
		GameManager::AddPxPendulumToWorld(PxTransform(PxVec3(-160, 760, -180)), 10, 30, 5, true, 0.5f, 2);
		GameManager::AddPxPendulumToWorld(PxTransform(PxVec3(-160, 760, -250)), 10, 30, 5, true, 0.5f, 2);
		break;
	case LevelState::LEVEL2:

		//HAVE COMMENTED OUT THE ORIGINAL BEAMS, WILL LEAVE IN IN CASE WE DECIDE TO GO FOR STATIC ONES
		//WorldCreator::AddPxFloorToWorld(PxTransform(PxVec3(-70, -98, -900)), PxVec3(20, 20, 200));
		GameManager::AddPxRotatingCylinderToWorld(PxTransform(PxVec3(-70, -98, -900) * 2, PxQuat(Maths::DegreesToRadians(90), PxVec3(1, 0, 0))),
			20, 100, PxVec3(0, 0, 1), .5f, .5f, TextureState::METAL, Vector3(1.5, 10, 0));


		//WorldCreator::AddPxFloorToWorld(PxTransform(PxVec3(0, -98, -900      )*2), PxVec3(20, 20, 200));
		GameManager::AddPxRotatingCylinderToWorld(PxTransform(PxVec3(0, -98, -900) * 2, PxQuat(Maths::DegreesToRadians(90), PxVec3(1, 0, 0))),
			20, 100, PxVec3(0, 0, 1), .5f, .5f, TextureState::METAL, Vector3(1.5, 10, 0));
		//WorldCreator::AddPxFloorToWorld(PxTransform(PxVec3(70, -98, -900     )*2), PxVec3(20, 20, 200));
		GameManager::AddPxRotatingCylinderToWorld(PxTransform(PxVec3(70, -98, -900) * 2, PxQuat(Maths::DegreesToRadians(90), PxVec3(1, 0, 0))),
			20, 100, PxVec3(0, 0, 1), .5f, .5f, TextureState::METAL, Vector3(1.5, 10, 0));

		//cannons																
		GameManager::AddPxCannonToWorld(PxTransform(PxVec3(-150, -70, -850) * 2), PxVec3(700, -50, 0), 10, 10, PxVec3(35, 0, 0));
		GameManager::AddPxCannonToWorld(PxTransform(PxVec3(-150, -70, -900) * 2), PxVec3(700, -50, 0), 10, 10, PxVec3(35, 0, 0));
		GameManager::AddPxCannonToWorld(PxTransform(PxVec3(-150, -70, -950) * 2), PxVec3(700, -50, 0), 10, 10, PxVec3(35, 0, 0));

		GameManager::AddPxCannonToWorld(PxTransform(PxVec3(150, -70, -825) * 2), PxVec3(-700, -50, 0), 10, 10, PxVec3(-35, 0, 0));
		GameManager::AddPxCannonToWorld(PxTransform(PxVec3(150, -70, -875) * 2), PxVec3(-700, -50, 0), 10, 10, PxVec3(-35, 0, 0));
		GameManager::AddPxCannonToWorld(PxTransform(PxVec3(150, -70, -925) * 2), PxVec3(-700, -50, 0), 10, 10, PxVec3(-35, 0, 0));
		GameManager::AddPxCannonToWorld(PxTransform(PxVec3(150, -70, -975) * 2), PxVec3(-700, -50, 0), 10, 10, PxVec3(-35, 0, 0));

		//pegs as obstacles/hiding places for the bowling balls
		//row 1
		q = PhysxConversions::GetQuaternion(Quaternion::EulerAnglesToQuaternion(11, 45, 11));
		GameManager::AddPxRotatingCubeToWorld(PxTransform(PxVec3(-60, -38, -1180) * 2, q), PxVec3(10, 50, 10), PxVec3(0, 2, 0));
		GameManager::AddPxRotatingCubeToWorld(PxTransform(PxVec3(0, -38, -1180) * 2, q), PxVec3(10, 50, 10), PxVec3(0, 2, 0));
		GameManager::AddPxRotatingCubeToWorld(PxTransform(PxVec3(60, -38, -1180) * 2, q), PxVec3(10, 50, 10), PxVec3(0, 2, 0));

		//row 2
		GameManager::AddPxRotatingCubeToWorld(PxTransform(PxVec3(-30, -23, -1235) * 2, q), PxVec3(10, 50, 10), PxVec3(0, 2, 0));
		GameManager::AddPxRotatingCubeToWorld(PxTransform(PxVec3(30, -23, -1235) * 2, q), PxVec3(10, 50, 10), PxVec3(0, 2, 0));

		//row 3
		GameManager::AddPxRotatingCubeToWorld(PxTransform(PxVec3(-60, -3, -1290) * 2, q), PxVec3(10, 50, 10), PxVec3(0, 2, 0));
		GameManager::AddPxRotatingCubeToWorld(PxTransform(PxVec3(0, -3, -1290) * 2, q), PxVec3(10, 50, 10), PxVec3(0, 2, 0));
		GameManager::AddPxRotatingCubeToWorld(PxTransform(PxVec3(60, -3, -1290) * 2, q), PxVec3(10, 50, 10), PxVec3(0, 2, 0));



		//OBSTACLE 5 - THE BLENDER
		//basically, it's an enclosed space with a spinning arm at the bottom to randomise which player actually wins
		//it should be flush with the entrance to the podium room so that the door is reasonably difficult to access unless there's nobody else there
		//again, not sure how to create the arm, it's a moving object, might need another class for this
		//also, it's over a 100m drop to the blender floor, so pls don't put fall damage in blender blade
		GameManager::AddPxRotatingCylinderToWorld(PxTransform((PxVec3(0, -90, -1720) * 2) + translate + translate2, PxQuat(Maths::DegreesToRadians(90), PxVec3(1, 0, 0))), 20, 80, PxVec3(0, 2, 0));
		GameManager::AddPxCannonToWorld(PxTransform(PxVec3(-80, 100, -1351) * 2 + translate), PxVec3(1, -200, 1), 20, 20, PxVec3(0, 0, 25));
		GameManager::AddPxCannonToWorld(PxTransform(PxVec3(-40, 100, -1351) * 2 + translate), PxVec3(1, -200, 1), 20, 20, PxVec3(0, 0, 25));
		GameManager::AddPxCannonToWorld(PxTransform(PxVec3(0, 100, -1351) * 2 + translate), PxVec3(1, -200, 1), 20, 20, PxVec3(0, 0, 25));
		GameManager::AddPxCannonToWorld(PxTransform(PxVec3(40, 100, -1351) * 2 + translate), PxVec3(1, -200, 1), 20, 20, PxVec3(0, 0, 25));
		GameManager::AddPxCannonToWorld(PxTransform(PxVec3(80, 100, -1351) * 2 + translate), PxVec3(1, -200, 1), 20, 20, PxVec3(0, 0, 25));


		for (int i = 0; i < 30; i++)
		{
			GameManager::GetObstacles()->cannons.push_back(GameManager::AddPxCannonBallToWorld(PxTransform(PxVec3(50000, 5000, 5000) * 2), 20));
		}
		break;

	case LevelState::LEVEL1:
		//OBSTACLE 1
		//Rotating pillars
		GameManager::AddPxPendulumToWorld(PxTransform(PxVec3(0, 177, -180) * 2), 10, 25, 200);
		GameManager::AddPxPendulumToWorld(PxTransform(PxVec3(0, 177, -200) * 2), 10, 25, 200, false);
		GameManager::AddPxPendulumToWorld(PxTransform(PxVec3(0, 177, -220) * 2), 10, 25, 200);

		//OBSTACLE2 
		//Jumping platforms with blenders
		GameManager::AddPxRotatingCylinderToWorld(PxTransform(PxVec3(36.5, 152.5, -315) * 2, PxQuat(Maths::DegreesToRadians(90), PxVec3(0, 0, 1))),
			3, 12, PxVec3(0, 1, 0));

		GameManager::AddPxRotatingCylinderToWorld(PxTransform(PxVec3(-36.5, 152.5, -315) * 2, PxQuat(Maths::DegreesToRadians(90), PxVec3(0, 0, 1))),
			3, 12, PxVec3(0, 1, 0));

		GameManager::AddPxRotatingCylinderToWorld(PxTransform(PxVec3(0, 152.5, -360) * 2, PxQuat(Maths::DegreesToRadians(90), PxVec3(0, 0, 1))),
			3, 24, PxVec3(0, 1, 0));

		GameManager::AddPxRotatingCylinderToWorld(PxTransform(PxVec3(36.5, 152.5, -410) * 2, PxQuat(Maths::DegreesToRadians(90), PxVec3(0, 0, 1))),
			3, 12, PxVec3(0, 1, 0));

		GameManager::AddPxRotatingCylinderToWorld(PxTransform(PxVec3(-36.5, 152.5, -410) * 2, PxQuat(Maths::DegreesToRadians(90), PxVec3(0, 0, 1))),
			3, 12, PxVec3(0, 1, 0));

		//OBSTACLE 3
		//bouncing sticks on the slide 
		for (int i = 0; i <= 7; i++)
		{
			GameManager::AddBounceSticks(PxTransform(PxVec3(-35 + (i * 10), 140, -522) * 2), 2, 2, 10.0F, 0.5F, 1.0F);
			GameManager::AddBounceSticks(PxTransform(PxVec3(-35 + (i * 10), 116, -602) * 2), 2, 2, 10.0F, 0.5F, 1.0F);
		}

		for (int i = 0; i <= 8; i++)
		{
			GameManager::AddBounceSticks(PxTransform(PxVec3(-40 + (i * 10), 128, -562) * 2), 2, 2, 10.0F, 0.5F, 1.0F);
			GameManager::AddBounceSticks(PxTransform(PxVec3(-40 + (i * 10), 104, -642) * 2), 2, 2, 10.0F, 0.5F, 1.0F);
		}

		//OBSTACLE 4
		//Running through walls
		//cubes		
		/* 1st round */
		GameManager::AddPxCubeToWorld(PxTransform(PxVec3(-22, 93, -775) * 2), PxVec3(2, 2, 2), 1.0F);
		GameManager::AddPxCubeToWorld(PxTransform(PxVec3(-22, 95, -775) * 2), PxVec3(2, 2, 2), 1.0F);
		GameManager::AddPxCubeToWorld(PxTransform(PxVec3(-22, 97, -775) * 2), PxVec3(2, 2, 2), 1.0F);
		GameManager::AddPxCubeToWorld(PxTransform(PxVec3(-22, 99, -775) * 2), PxVec3(2, 2, 2), 1.0F);
		GameManager::AddPxCubeToWorld(PxTransform(PxVec3(-22, 101, -775) * 2), PxVec3(2, 2, 2), 1.0F);
		GameManager::AddPxCubeToWorld(PxTransform(PxVec3(-24, 93, -775) * 2), PxVec3(2, 2, 2), 1.0F);
		GameManager::AddPxCubeToWorld(PxTransform(PxVec3(-24, 95, -775) * 2), PxVec3(2, 2, 2), 1.0F);
		GameManager::AddPxCubeToWorld(PxTransform(PxVec3(-24, 97, -775) * 2), PxVec3(2, 2, 2), 1.0F);
		GameManager::AddPxCubeToWorld(PxTransform(PxVec3(-24, 99, -775) * 2), PxVec3(2, 2, 2), 1.0F);
		GameManager::AddPxCubeToWorld(PxTransform(PxVec3(-24, 101, -775) * 2), PxVec3(2, 2, 2), 1.0F);

		GameManager::AddPxCubeToWorld(PxTransform(PxVec3(18, 93, -775) * 2), PxVec3(2, 2, 2), 1.0F);
		GameManager::AddPxCubeToWorld(PxTransform(PxVec3(18, 95, -775) * 2), PxVec3(2, 2, 2), 1.0F);
		GameManager::AddPxCubeToWorld(PxTransform(PxVec3(18, 97, -775) * 2), PxVec3(2, 2, 2), 1.0F);
		GameManager::AddPxCubeToWorld(PxTransform(PxVec3(18, 99, -775) * 2), PxVec3(2, 2, 2), 1.0F);
		GameManager::AddPxCubeToWorld(PxTransform(PxVec3(18, 101, -775) * 2), PxVec3(2, 2, 2), 1.0F);
		GameManager::AddPxCubeToWorld(PxTransform(PxVec3(20, 93, -775) * 2), PxVec3(2, 2, 2), 1.0F);
		GameManager::AddPxCubeToWorld(PxTransform(PxVec3(20, 95, -775) * 2), PxVec3(2, 2, 2), 1.0F);
		GameManager::AddPxCubeToWorld(PxTransform(PxVec3(20, 97, -775) * 2), PxVec3(2, 2, 2), 1.0F);
		GameManager::AddPxCubeToWorld(PxTransform(PxVec3(20, 99, -775) * 2), PxVec3(2, 2, 2), 1.0F);
		GameManager::AddPxCubeToWorld(PxTransform(PxVec3(20, 101, -775) * 2), PxVec3(2, 2, 2), 1.0F);

		GameManager::AddPxCubeToWorld(PxTransform(PxVec3(42, 93, -775) * 2), PxVec3(2, 2, 2), 1.0F);
		GameManager::AddPxCubeToWorld(PxTransform(PxVec3(42, 95, -775) * 2), PxVec3(2, 2, 2), 1.0F);
		GameManager::AddPxCubeToWorld(PxTransform(PxVec3(42, 97, -775) * 2), PxVec3(2, 2, 2), 1.0F);
		GameManager::AddPxCubeToWorld(PxTransform(PxVec3(42, 99, -775) * 2), PxVec3(2, 2, 2), 1.0F);
		GameManager::AddPxCubeToWorld(PxTransform(PxVec3(42, 101, -775) * 2), PxVec3(2, 2, 2), 1.0F);
		GameManager::AddPxCubeToWorld(PxTransform(PxVec3(44, 93, -775) * 2), PxVec3(2, 2, 2), 1.0F);
		GameManager::AddPxCubeToWorld(PxTransform(PxVec3(44, 95, -775) * 2), PxVec3(2, 2, 2), 1.0F);
		GameManager::AddPxCubeToWorld(PxTransform(PxVec3(44, 97, -775) * 2), PxVec3(2, 2, 2), 1.0F);
		GameManager::AddPxCubeToWorld(PxTransform(PxVec3(44, 99, -775) * 2), PxVec3(2, 2, 2), 1.0F);
		GameManager::AddPxCubeToWorld(PxTransform(PxVec3(44, 101, -775) * 2), PxVec3(2, 2, 2), 1.0F);

		/* Second Round */
		GameManager::AddPxCubeToWorld(PxTransform(PxVec3(0, 93, -825) * 2), PxVec3(2, 2, 2), 1.0F);
		GameManager::AddPxCubeToWorld(PxTransform(PxVec3(0, 95, -825) * 2), PxVec3(2, 2, 2), 1.0F);
		GameManager::AddPxCubeToWorld(PxTransform(PxVec3(0, 97, -825) * 2), PxVec3(2, 2, 2), 1.0F);
		GameManager::AddPxCubeToWorld(PxTransform(PxVec3(0, 99, -825) * 2), PxVec3(2, 2, 2), 1.0F);
		GameManager::AddPxCubeToWorld(PxTransform(PxVec3(0, 101, -825) * 2), PxVec3(2, 2, 2), 1.0F);
		GameManager::AddPxCubeToWorld(PxTransform(PxVec3(2, 93, -825) * 2), PxVec3(2, 2, 2), 1.0F);
		GameManager::AddPxCubeToWorld(PxTransform(PxVec3(2, 95, -825) * 2), PxVec3(2, 2, 2), 1.0F);
		GameManager::AddPxCubeToWorld(PxTransform(PxVec3(2, 97, -825) * 2), PxVec3(2, 2, 2), 1.0F);
		GameManager::AddPxCubeToWorld(PxTransform(PxVec3(2, 99, -825) * 2), PxVec3(2, 2, 2), 1.0F);
		GameManager::AddPxCubeToWorld(PxTransform(PxVec3(2, 101, -825) * 2), PxVec3(2, 2, 2), 1.0F);

		GameManager::AddPxCubeToWorld(PxTransform(PxVec3(-34, 93, -825) * 2), PxVec3(2, 2, 2), 1.0F);
		GameManager::AddPxCubeToWorld(PxTransform(PxVec3(-34, 95, -825) * 2), PxVec3(2, 2, 2), 1.0F);
		GameManager::AddPxCubeToWorld(PxTransform(PxVec3(-34, 97, -825) * 2), PxVec3(2, 2, 2), 1.0F);
		GameManager::AddPxCubeToWorld(PxTransform(PxVec3(-34, 99, -825) * 2), PxVec3(2, 2, 2), 1.0F);
		GameManager::AddPxCubeToWorld(PxTransform(PxVec3(-34, 101, -825) * 2), PxVec3(2, 2, 2), 1.0F);
		GameManager::AddPxCubeToWorld(PxTransform(PxVec3(-36, 93, -825) * 2), PxVec3(2, 2, 2), 1.0F);
		GameManager::AddPxCubeToWorld(PxTransform(PxVec3(-36, 95, -825) * 2), PxVec3(2, 2, 2), 1.0F);
		GameManager::AddPxCubeToWorld(PxTransform(PxVec3(-36, 97, -825) * 2), PxVec3(2, 2, 2), 1.0F);
		GameManager::AddPxCubeToWorld(PxTransform(PxVec3(-36, 99, -825) * 2), PxVec3(2, 2, 2), 1.0F);
		GameManager::AddPxCubeToWorld(PxTransform(PxVec3(-36, 101, -825) * 2), PxVec3(2, 2, 2), 1.0F);


		/* Third Round */
		GameManager::AddPxCubeToWorld(PxTransform(PxVec3(42, 93, -875) * 2), PxVec3(2, 2, 2), 1.0F);
		GameManager::AddPxCubeToWorld(PxTransform(PxVec3(42, 95, -875) * 2), PxVec3(2, 2, 2), 1.0F);
		GameManager::AddPxCubeToWorld(PxTransform(PxVec3(42, 97, -875) * 2), PxVec3(2, 2, 2), 1.0F);
		GameManager::AddPxCubeToWorld(PxTransform(PxVec3(42, 99, -875) * 2), PxVec3(2, 2, 2), 1.0F);
		GameManager::AddPxCubeToWorld(PxTransform(PxVec3(42, 101, -875) * 2), PxVec3(2, 2, 2), 1.0F);
		GameManager::AddPxCubeToWorld(PxTransform(PxVec3(44, 93, -875) * 2), PxVec3(2, 2, 2), 1.0F);
		GameManager::AddPxCubeToWorld(PxTransform(PxVec3(44, 95, -875) * 2), PxVec3(2, 2, 2), 1.0F);
		GameManager::AddPxCubeToWorld(PxTransform(PxVec3(44, 97, -875) * 2), PxVec3(2, 2, 2), 1.0F);
		GameManager::AddPxCubeToWorld(PxTransform(PxVec3(44, 99, -875) * 2), PxVec3(2, 2, 2), 1.0F);
		GameManager::AddPxCubeToWorld(PxTransform(PxVec3(44, 101, -875) * 2), PxVec3(2, 2, 2), 1.0F);
		break;
	case LevelState::SANDBOX:
		GameManager::AddPxSphereToWorld(PxTransform(PxVec3(-20, 20, -20)), 2);
		GameManager::AddPxCubeToWorld(PxTransform(PxVec3(0, 20, -20)), PxVec3(2, 2, 2));
		GameManager::AddPxCapsuleToWorld(PxTransform(PxVec3(20, 20, -20)), 2, 2);
		break;
	}
}

/* If in debug mode we can select an object with the cursor, displaying its properties and allowing us to take control */
bool LevelCreator::SelectObject()
{
	if (Window::GetMouse()->ButtonDown(NCL::MouseButtons::LEFT))
	{
		PxVec3 pos = PhysxConversions::GetVector3(GameManager::GetWorld()->GetMainCamera()->GetPosition());
		PxVec3 dir = PhysxConversions::GetVector3(CollisionDetection::GetMouseDirection(*GameManager::GetWorld()->GetMainCamera()));
		float distance = 10000.0f;
		PxRaycastBuffer hit;

		if (GameManager::GetPhysicsSystem()->GetGScene()->raycast(pos, dir, distance, hit))
		{
			if (GameManager::GetSelectionObject() && GameManager::GetSelectionObject()->GetRenderObject())
			{
				GameManager::GetSelectionObject()->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
			}

			GameManager::SetSelectionObject(GameManager::GetWorld()->FindObjectFromPhysicsBody(hit.block.actor));

			if (GameManager::GetSelectionObject() == GameManager::GetPlayer())
			{
				GameManager::GetWorld()->GetMainCamera()->SetState(CameraState::THIRDPERSON);
				GameManager::GetRenderer()->SetUIState(UIState::INGAME);
				GameManager::GetWorld()->SetDebugMode(GameManager::GetRenderer()->GetUIState() == UIState::DEBUG);
				Window::GetWindow()->ShowOSPointer(false);
				Window::GetWindow()->LockMouseToWindow(true);
			}
			else if (GameManager::GetSelectionObject()->GetRenderObject())
				GameManager::GetSelectionObject()->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));

			return true;
		}
		return false;
	}

	if (Window::GetMouse()->ButtonDown(NCL::MouseButtons::RIGHT))
	{
		if (GameManager::GetSelectionObject())
		{
			if (GameManager::GetSelectionObject()->GetRenderObject())
				GameManager::GetSelectionObject()->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
			GameManager::SetSelectionObject(nullptr);
		}
	}
	return false;
}

/* If we've selected an object, we can manipulate it with some key presses */
void LevelCreator::DebugObjectMovement()
{
	if (GameManager::GetSelectionObject()->GetPhysicsObject()->GetPXActor()->is<PxRigidDynamic>())
	{
		PxRigidDynamic* body = (PxRigidDynamic*)GameManager::GetSelectionObject()->GetPhysicsObject()->GetPXActor();

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::LEFT))
			body->addTorque(PxVec3(-10, 0, 0), PxForceMode::eIMPULSE);
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT))
			body->addTorque(PxVec3(10, 0, 0), PxForceMode::eIMPULSE);
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP))
			body->addTorque(PxVec3(0, 0, -10), PxForceMode::eIMPULSE);
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN))
			body->addTorque(PxVec3(0, 0, 10), PxForceMode::eIMPULSE);
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM2))
			body->addTorque(PxVec3(0, -10, 0), PxForceMode::eIMPULSE);
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM8))
			body->addTorque(PxVec3(0, 10, 0), PxForceMode::eIMPULSE);
	}
}
