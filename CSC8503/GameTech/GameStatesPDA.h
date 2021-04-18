#pragma once
#include "../CSC8503Common/PushdownState.h"
#include "../CSC8503Common/PushdownMachine.h"
#include "../CSC8503Common/StateMachine.h"
#include "../CSC8503Common/StateTransition.h"
#include "../CSC8503Common/State.h"
#include "../../Common/Window.h"
#include "NetworkedGame.h"
//#include "LevelCreator.h"

using namespace NCL;
using namespace CSC8503;

NetworkedGame* levelCreator = nullptr;

class Pause : public PushdownState
{
	PushdownResult OnUpdate(float dt, PushdownState** newState) override
	{
		GameManager::GetAudioManager()->UpdateAudio(GameManager::GetWorld()->GetMainCamera()->GetPosition());
		GameManager::GetRenderer()->Render();
		Debug::FlushRenderables(dt);
		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::ESCAPE) ||
			GameManager::GetRenderer()->GetUIState() == UIState::LOADING ||
			GameManager::GetRenderer()->GetUIState() == UIState::INGAME)
		{
			return PushdownResult::Pop;
		}
		return PushdownResult::NoChange;
	}
	void OnAwake() override
	{
		GameManager::GetWindow()->LockMouseToWindow(false);
		GameManager::GetWindow()->ShowOSPointer(true);
		GameManager::GetRenderer()->SetUIState(UIState::PAUSED);
	}
};

class MultiplayerPause : public PushdownState
{
	PushdownResult OnUpdate(float dt, PushdownState** newState) override
	{
		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::ESCAPE) ||
			GameManager::GetRenderer()->GetUIState() == UIState::LOADING ||
			GameManager::GetRenderer()->GetUIState() == UIState::INGAME)
		{
			return PushdownResult::Pop;
		}

		levelCreator->Update(dt);

		return PushdownResult::NoChange;
	}
	void OnAwake() override
	{
		GameManager::GetWindow()->LockMouseToWindow(false);
		GameManager::GetWindow()->ShowOSPointer(true);
		GameManager::GetRenderer()->SetUIState(UIState::PAUSED);
	}
};

class Level : public PushdownState
{
	PushdownResult OnUpdate(float dt, PushdownState** newState) override
	{
		if (GameManager::GetRenderer()->GetPreviousUIState() == UIState::INGAME)
		{
			GameManager::GetRenderer()->SetPreviousState(UIState::MENU);
			GameManager::ResetMenu();
			return PushdownResult::Pop;
		}

		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::ESCAPE))
		{
			*newState = new Pause();
			return PushdownResult::Push;
		}

		levelCreator->LevelCreator::Update(dt);
		return PushdownResult::NoChange;
	}

	void OnAwake() override
	{
		if (GameManager::GetRenderer()->GetUIState() != UIState::MENU)
		{
			GameManager::GetWindow()->LockMouseToWindow(true);
			GameManager::GetWindow()->ShowOSPointer(false);
			GameManager::GetRenderer()->SetUIState(UIState::INGAME);
		}
	}
};

class MultiplayerLevel : public PushdownState
{
	PushdownResult OnUpdate(float dt, PushdownState** newState) override
	{
		GameTechRenderer* r = GameManager::GetRenderer();

		if (GameManager::GetRenderer()->GetPreviousUIState() == UIState::INGAME)
		{
			GameManager::ResetMenu();
			GameManager::GetRenderer()->SetPreviousState(UIState::MULTIPLAYERMENU);
			return PushdownResult::Pop;
		}

		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::ESCAPE))
		{
			*newState = new MultiplayerPause();
			return PushdownResult::Push;
		}

		UIState ui = GameManager::GetRenderer()->GetUIState();

		if (Window::GetKeyboard()->KeyHeld(KeyboardKeys::TAB))
		{
			if (ui != UIState::SCOREBOARD)
			{
				prevState = ui;
				GameManager::GetRenderer()->SetUIState(UIState::SCOREBOARD);
			}
		}
		else if (ui == UIState::SCOREBOARD)
		{
			GameManager::GetRenderer()->SetUIState(prevState);
		}

		levelCreator->Update(dt);
		return PushdownResult::NoChange;
	}

	void OnAwake() override
	{
		if (GameManager::GetRenderer()->GetUIState() != UIState::MENU)
		{
			GameManager::GetWindow()->LockMouseToWindow(true);
			GameManager::GetWindow()->ShowOSPointer(false);
			GameManager::GetRenderer()->SetUIState(UIState::INGAME);
		}
	}
	UIState prevState;
};

class MainMenu : public PushdownState
{
public:
	PushdownResult OnUpdate(float dt, PushdownState** newState) override
	{
		GameManager::GetRenderer()->Render();
		GameManager::GetAudioManager()->UpdateAudio(GameManager::GetWorld()->GetMainCamera()->GetPosition());
		Debug::FlushRenderables(dt);

		if (GameManager::GetRenderer()->GetSelectedLevel())
		{
			GameManager::GetAudioManager()->StopAllSound();
			LevelState level = (LevelState)(GameManager::GetRenderer()->GetSelectedLevel() - 1);
			GameManager::SetLevelState(level);

			if (GameManager::GetRenderer()->GetPreviousUIState() == UIState::MENU)
			{
				*newState = new Level();
				levelCreator->LevelCreator::InitWorld(level);
			}
			else
			{
				*newState = new MultiplayerLevel();
				playerName = GameManager::GetRenderer()->GetPlayerName();
				levelCreator->StartAsServer(level, playerName);
			}

			return PushdownResult::Push;
		}

		switch (GameManager::GetRenderer()->GetUIState())
		{
		case UIState::QUIT:
			return PushdownResult::Pop;
			break;
		case UIState::JOINLEVEL:
			GameManager::GetAudioManager()->StopAllSound();
			*newState = new MultiplayerLevel();
			playerName = GameManager::GetRenderer()->GetPlayerName();
			IPAddress = GameManager::GetRenderer()->GetIP();
			levelCreator->StartAsClient(playerName, IPAddress);
			return PushdownResult::Push;
			break;
		}
		return PushdownResult::NoChange;
	}
private:
	string IPAddress;
	string playerName;
	void  OnAwake() override
	{
		GameManager::GetWindow()->ShowOSPointer(true);
		GameManager::GetWindow()->LockMouseToWindow(false);
		if (!levelCreator)
			levelCreator = new NetworkedGame();
		else
			levelCreator->ResetWorld();
		GameManager::GetRenderer()->SetUIState(UIState::MENU);
		GameManager::GetAudioManager()->StopAllSound();
		GameManager::GetAudioManager()->PlayAudio("../../Assets/Audio/MenuMusic.mp3", true);
	}
};


