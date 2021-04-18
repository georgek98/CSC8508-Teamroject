#pragma once
#include "../../Plugins/OpenGLRendering/OGLMesh.h"
#include "../../Plugins/OpenGLRendering/OGLShader.h"
#include "../../Plugins/OpenGLRendering/OGLTexture.h"
#include "../../Common/TextureLoader.h"
#include "../Common/Light.h"
#include "../../Common/Win32Window.h"

#include "../CSC8503Common/GameWorld.h"
#include "../CSC8503Common/PxPhysicsSystem.h"
#include "GameTechRenderer.h"
#include "../../Common/AudioManager.h"

#include "../CSC8503Common/Cannonball.h"
#include "../CSC8503Common/Cannon.h"
#include "../CSC8503Common/KillPlane.h"
#include "../CSC8503Common/Obstacles.h"
#include "NetworkPlayer.h"
#include "../Common/MeshMaterial.h";
#include "../CSC8503Common/FallingTile.h"

#include "../CSC8503Common/Pendulum.h"

using namespace NCL;
using namespace CSC8503;
const float MESH_SIZE = 3.0f;
enum class LevelState { LEVEL1, LEVEL2, LEVEL3, SANDBOX };
enum class TextureState
{
	FLOOR, ICE, TRAMPOLINE, INVISIBLE, FINISH, WALL, RED, DOGE,
	PINK, WALL2, METAL, CANNON, CANNONBALL, WOOD
};

class GameManager
{
public:
	static void Create(PxPhysicsSystem* p, GameWorld* w, AudioManager* a);
	static void LoadAssets();
	static void ResetMenu();

	~GameManager();
	static GameObject* AddPxCubeToWorld(const PxTransform& t, const PxVec3 halfSizes, float density = 10.0f, float friction = 0.5f, float elasticity = 0.1f);
	static GameObject* AddPxSphereToWorld(const PxTransform& t, const PxReal radius, float density = 10.0f, float friction = 0.5f, float elasticity = 0.1f);
	static GameObject* AddPxCapsuleToWorld(const PxTransform& t, const PxReal radius, const PxReal halfHeight,
		float density = 10.0f, float friction = 0.5f, float elasticity = 0.1f);
	static GameObject* AddPxCylinderToWorld(const PxTransform& t, const PxReal radius, const PxReal halfHeight,
		float density = 10.0f, float friction = 0.5f, float elasticity = 0.1f);
	static void AddPxFloorToWorld(const PxTransform& t, const PxVec3 halfSizes, float friction = 0.5f, float elasticity = 0.1f, TextureState state = TextureState::FLOOR, Vector3 textureScale = Vector3(10, 10, 10));

	static void AddBounceSticks(const PxTransform& t, const PxReal radius, const PxReal halfHeight,
		float density = 10.0f, float friction = 0.5f, float elasticity = 0.1f);
	static void AddPxSeeSawToWorld(const PxTransform& t, const PxVec3 halfSizes, float density = 10.0f, float friction = 0.5f, float elasticity = 0.1f);
	static void AddPxRevolvingDoorToWorld(const PxTransform& t, const PxVec3 halfSizes, float density = 10.0f, float friction = 0.5f, float elasticity = 0.1f);

	static GameObject* AddPxCoinToWorld(const PxTransform& t, const PxReal radius);
	static GameObject* AddPxLongJump(const PxTransform& t, const PxReal radius);
	static GameObject* AddPxSpeedPower(const PxTransform& t, const PxReal radius);
	static PlayerObject* AddPxPlayerToWorld(const PxTransform& t, const PxReal scale);
	static NetworkPlayer* AddPxNetworkPlayerToWorld(const PxTransform& t, const PxReal scale, NetworkedGame* game, int playerNum);
	static void AddPxEnemyToWorld(const PxTransform& t, const PxReal scale);

	static void AddLightToWorld(Vector3 position, Vector3 color, float radius = 5);
	static GameObject* AddPxRotatingCubeToWorld(const PxTransform& t, const PxVec3 halfSizes, const PxVec3 rotation, float friction = 0.5f, float elasticity = 0.5, bool rotating = true, TextureState state = TextureState::WOOD, Vector3 textureScale = Vector3(2, 2, 0));
	static GameObject* AddPxRotatingCylinderToWorld(const PxTransform& t, const PxReal radius, const PxReal halfHeight, const PxVec3 rotation, float friction = 0.5f, float elasticity = 0.5f, TextureState state = TextureState::PINK, Vector3 textureScale = Vector3(2, 2, 0));
	static GameObject* AddPxPendulumToWorld(const PxTransform& t, const PxReal radius, const PxReal halfHeight, const float timeToSwing, const bool isSwingingLeft = true, float friction = 0.5f, float elasticity = 0.5f, Vector3 textureScale = Vector3(1, 5, 0));
	static Cannonball* AddPxCannonBallToWorld(const PxTransform& t, const PxReal radius = 5, const PxVec3* force = new PxVec3(0, 85000, 700000), int time = 10, float density = 10.0f, float friction = 0.5f, float elasticity = 0.1f);
	static void AddPxCannonToWorld(const PxTransform& t, const PxVec3 trajectory, const int shotTime, const int shotSize, PxVec3 translate = PxVec3(0, 100, 0));
	static void AddPxKillPlaneToWorld(const PxTransform& t, const PxVec3 halfSizes, const PxVec3 respawnCentre, Vector3 respawnSizeRange, bool hide = true);
	static GameObject* AddPxFallingTileToWorld(const PxTransform& t, const PxVec3 halfSizes, float friction = 0.5f, float elasticity = 0.f);
	static Obstacles* GetObstacles()
	{
		return obstacles;
	}
	static PxPhysicsSystem* GetPhysicsSystem()
	{
		return pXPhysics;
	}

	static GameWorld* GetWorld()
	{
		return world;
	}

	static GameTechRenderer* GetRenderer()
	{
		return renderer;
	}

	static AudioManager* GetAudioManager()
	{
		return audioManager;
	}

	static GameObject* GetSelectionObject()
	{
		return selectionObject;
	}

	static void SetSelectionObject(GameObject* val)
	{
		selectionObject = val;
		renderer->SetSelectionObject(selectionObject);
	}

	static void SetLevelState(LevelState val)
	{
		levelState = val;
	}

	static LevelState GetLevelState()
	{
		return levelState;
	}

	static void SetWindow(Win32Code::Win32Window* val)
	{
		window = val;
	}

	static Win32Code::Win32Window* GetWindow()
	{
		return window;
	}

	static void SetPlayer(PlayerObject* val)
	{
		player = val;
		renderer->SetPlayer(player);
	}

	static PlayerObject* GetPlayer()
	{
		return player;
	}

private:
	static Win32Code::Win32Window* window;

	static PxPhysicsSystem* pXPhysics;
	static GameWorld* world;
	static GameTechRenderer* renderer;
	static AudioManager* audioManager;
	static Obstacles* obstacles;

	static OGLMesh* capsuleMesh;
	static OGLMesh* cylinderMesh;
	static OGLMesh* cubeMesh;
	static OGLMesh* sphereMesh;
	static OGLMesh* charMeshA;
	static OGLMesh* charMeshB;
	static OGLMesh* enemyMesh;
	static OGLMesh* bonusMesh;
	static OGLMesh* pbodyMesh;


	static OGLTexture* basicTex;
	static OGLTexture* floorTex;
	static OGLTexture* lavaTex;
	static OGLTexture* iceTex;
	static OGLTexture* trampolineTex;
	static OGLTexture* obstacleTex;
	static OGLTexture* woodenTex;    // windmill
	static OGLTexture* finishTex;
	static OGLTexture* menuTex;
	static OGLTexture* plainTex;
	static OGLTexture* wallTex;
	static OGLTexture* wallTex2;
	static OGLTexture* dogeTex;
	static OGLTexture* pBodyTex;
	static OGLTexture* redTex;

	static OGLTexture* pinkTex; // bouncy
	static OGLTexture* metalTex; // pendulum // ping pongs
	static OGLTexture* greyTex; // cannonball
	static OGLTexture* blackTex; // cannon
	static OGLTexture* fallingTileTex; // falling tiles



	static OGLTexture* platformWallTex;

	static OGLShader* basicShader;
	static OGLShader* toonShader;
	static OGLShader* outlineShader;

	static CameraState camState;

	static GameObject* selectionObject;

	static LevelState levelState;

	static PlayerObject* player;

	static MeshMaterial* pBodyMat;
};

