/*			Created By Rich Davison
 *			Edited By Samuel Buzz Appleby
 *               21/01/2021
 *                170348069
 *			Game World Implementation		 */
#include "GameWorld.h"
using namespace NCL;
using namespace NCL::CSC8503;
std::vector<GameObject*> GameWorld::gameObjects;

GameWorld::GameWorld() {
	mainCamera = new Camera();
	shuffleObjects		= false;
	worldIDCounter		= 0;
	totalCollisions = 0;
	debugMode = false;
}

GameWorld::~GameWorld() {
	ClearAndErase();
	delete mainCamera;
}

void GameWorld::Clear() {
	gameObjects.clear();
}

void GameWorld::ClearAndErase() {
	for (auto& i : gameObjects) {
		delete i;
	}
	Clear();
}

void GameWorld::AddGameObject(GameObject* o) {
	gameObjects.emplace_back(o);
	o->SetWorldID(worldIDCounter++);
}

void GameWorld::RemoveGameObject(GameObject* o, bool andDelete) {
	gameObjects.erase(std::remove(gameObjects.begin(), gameObjects.end(), o), gameObjects.end());
	if (andDelete)
		delete o;
}

void GameWorld::GetObjectIterators(GameObjectIterator& first, GameObjectIterator& last) const {
	first = gameObjects.begin();
	last = gameObjects.end();
}

void GameWorld::OperateOnContents(GameObjectFunc f) {
	for (GameObject* g : gameObjects) {
		f(g);
	}
}

void GameWorld::UpdateWorld(float dt) {
	int tempCols = 0;
	std::random_device rd;
	std::mt19937 g(rd());
	if (shuffleObjects)
		std::shuffle(gameObjects.begin(), gameObjects.end(), g);
	for (auto& i : gameObjects) {
		i->Update(dt);
		if(debugMode)
			Debug::DrawAxisLines(i->GetTransform().GetMatrix(), 2.0f);
		if (i->IsColliding())
			tempCols++;
		/*if (i->CanDestroy())
			RemoveGameObject(i, true);*/
	}
	totalCollisions = tempCols;
}

void GameWorld::UpdatePhysics(float fixedDeltaTime) {	
	for (auto& i : gameObjects) {
		i->FixedUpdate(fixedDeltaTime);
	}
}

GameObject* GameWorld::FindObjectFromPhysicsBody(PxActor* actor) {
	for (auto& i : gameObjects) {
		if (i->GetPhysicsObject()->GetPXActor() == actor) {
			return i;
		}
	}
	return nullptr;
}


