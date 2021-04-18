/*			Created By Rich Davison
 *			Edited By Samuel Buzz Appleby
 *               21/01/2021
 *                170348069
 *			Game Tech Renderer Implementation */
#include "GameTechRenderer.h"
#include "../../Common/Imgui/imgui_internal.h"
#include "../../Common/AudioManager.h"
#include <string.h>
#include "../../Common/Assets.h"
#include "../../Common/MeshMaterial.h"
#include "GameManager.h"
#include "../GameTech/NetworkPlayer.h"
#include "NetworkedGame.h"


using namespace NCL;
using namespace Rendering;
using namespace CSC8503;

#define SHADOWSIZE 4096
#define POST_PASSES = 10;

Matrix4 biasMatrix = Matrix4::Translation(Vector3(0.5, 0.5, 0.5)) * Matrix4::Scale(Vector3(0.5, 0.5, 0.5));


GameTechRenderer::GameTechRenderer(GameWorld& world, PxPhysicsSystem& physics) :
	OGLRenderer(*Window::GetWindow()), gameWorld(world), pXPhysics(physics)
{

	glEnable(GL_DEPTH_TEST);
	shadowShader = new OGLShader("GameTechShadowVert.glsl", "GameTechShadowFrag.glsl");
	//blurShader = new OGLShader("TexturedVertex.glsl", "processfrag.glsl");

	glGenTextures(1, &shadowTex);
	glBindTexture(GL_TEXTURE_2D, shadowTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOWSIZE, SHADOWSIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenFramebuffers(1, &shadowFBO);

	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowTex, 0);
	glDrawBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glClearColor(1, 1, 1, 1);

	//Set up the light properties
	lightColour = Vector4(1, 1, 1, 1.0f);
	lightRadius = 1000.0f;
	lightPosition = Vector3(500.0f, 2600.0f, -500);
	lightDirection = Vector3(0.0f, -1, -1.f);

	dirLight = new Light(Vector3(100.0f, 500.0f, -100.0f), Vector4(1, 1, 1, 1), 1000.0f, Vector3(0.0f, -1, -1));
	spotLights[0] = new Light(Vector3(0, 50, 0), Vector4(1, 1, 1, 1), 5.0f);
	spotLights[0]->SetDirection(Vector3(spotLights[0]->GetPosition().x, -(spotLights[0]->GetPosition().y), 0) - spotLights[0]->GetPosition().Normalised());

	spotLights[1] = new Light(Vector3(0, 50, 0), Vector4(1, 1, 1, 1), 50.0f);
	spotLights[1]->SetDirection(Vector3(spotLights[1]->GetPosition().x, -(spotLights[1]->GetPosition().y), 0) - spotLights[1]->GetPosition().Normalised());

	//Skybox!
	skyboxShader = new OGLShader("skyboxVertex.glsl", "skyboxFragment.glsl");
	skyboxMesh = new OGLMesh();
	skyboxMesh->SetVertexPositions({ Vector3(-1, 1,-1), Vector3(-1,-1,-1) , Vector3(1,-1,-1) , Vector3(1,1,-1) });
	skyboxMesh->SetVertexIndices({ 0,1,2,2,3,0 });
	skyboxMesh->UploadToGPU();

	LoadSkybox();
	levelState = UIState::MENU;
	loadingImage = (OGLTexture*)TextureLoader::LoadAPITexture("loading_screen.png");
	backgroundImage = (OGLTexture*)TextureLoader::LoadAPITexture("background.png");
	levelImages[0] = (OGLTexture*)TextureLoader::LoadAPITexture("level_1.png");
	levelImages[1] = (OGLTexture*)TextureLoader::LoadAPITexture("level_2.png");
	levelImages[2] = (OGLTexture*)TextureLoader::LoadAPITexture("level_3.png");
	levelImages[3] = (OGLTexture*)TextureLoader::LoadAPITexture("sandbox.png");
	levelImages[4] = (OGLTexture*)TextureLoader::LoadAPITexture("sandbox_grey.png");
}

GameTechRenderer::~GameTechRenderer()
{
	glDeleteTextures(1, &shadowTex);
	glDeleteTextures(2, bufferColourTex);

	glDeleteFramebuffers(1, &shadowFBO);
	glDeleteFramebuffers(1, &processFBO);

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void GameTechRenderer::LoadSkybox()
{
	string filenames[6] = {
		"/Cubemap/skyrender0004.png",
		"/Cubemap/skyrender0001.png",
		"/Cubemap/skyrender0003.png",
		"/Cubemap/skyrender0006.png",
		"/Cubemap/skyrender0002.png",
		"/Cubemap/skyrender0005.png"
	};

	int width[6] = { 0 };
	int height[6] = { 0 };
	int channels[6] = { 0 };
	int flags[6] = { 0 };

	vector<char*> texData(6, nullptr);

	for (int i = 0; i < 6; ++i)
	{
		TextureLoader::LoadTexture(filenames[i], texData[i], width[i], height[i], channels[i], flags[i]);
		if (i > 0 && (width[i] != width[0] || height[0] != height[0]))
		{
			std::cout << __FUNCTION__ << " cubemap input textures don't match in size?\n";
			return;
		}
	}
	glGenTextures(1, &skyboxTex);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTex);

	GLenum type = channels[0] == 4 ? GL_RGBA : GL_RGB;

	for (int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width[i], height[i], 0, type, GL_UNSIGNED_BYTE, texData[i]);
	}

	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void GameTechRenderer::InitGUI(HWND handle)
{
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(handle);
	ImGui_ImplOpenGL3_Init("#version 130");

	titleFont = io.Fonts->AddFontFromFileTTF("../../Assets/Fonts/JosefinSans-Bold.ttf", 50.0f);
	textFont = io.Fonts->AddFontFromFileTTF("../../Assets/Fonts/JosefinSans-Regular.ttf", 15.0f);

	window_flags = 0;
	window_flags |= ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoResize;
	window_flags |= ImGuiWindowFlags_NoCollapse;
	window_flags |= ImGuiWindowFlags_AlwaysAutoResize;
	window_flags |= ImGuiWindowFlags_NoScrollbar;
	//window_flags |= ImGuiWindowFlags_MenuBar;
	//window_flags |= ImGuiWindowFlags_NoNav;
	//window_flags |= ImGuiWindowFlags_NoBackground;
	//window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
	window_flags |= ImGuiWindowFlags_NoTitleBar;

	box_flags = 0;
	box_flags |= ImGuiWindowFlags_NoMove;
	box_flags |= ImGuiWindowFlags_NoResize;
	box_flags |= ImGuiWindowFlags_NoCollapse;
	box_flags |= ImGuiWindowFlags_AlwaysAutoResize;
	box_flags |= ImGuiWindowFlags_NoScrollbar;
}

bool GameTechRenderer::TestValidHost()
{
	std::vector<string> vect;
	std::stringstream ss(ipString);
	while (ss.good())
	{
		string substr;
		getline(ss, substr, '.');
		if (substr.size() < 1)
			return false;
		vect.push_back(substr);
	}
	return vect.size() == 4/* && portString.length() > 0 && isdigit(portString.at(0))*/;
}

void GameTechRenderer::RenderFrame()
{
	switch (GameManager::GetLevelState()) {
		case LevelState::LEVEL1:
			spotLights[0]->SetPosition(Vector3(0, 50, 100));
			break;

		case LevelState::LEVEL2:
			dirLight->SetPosition(Vector3(500, 4000, -500));
			spotLights[0]->SetPosition(Vector3(0, 200, 150));
			break;

		case LevelState::LEVEL3:
			dirLight->SetPosition(Vector3(500, 5000, -500));
			spotLights[0]->SetPosition(Vector3(0, -50, 100));
			break;
	}

	glEnable(GL_CULL_FACE);
	glClearColor(1, 1, 1, 1);
	BuildObjectList();
	SortObjectList();
	RenderShadowMap();
	RenderSkybox();
	RenderCamera();
	RenderUI();
	glDisable(GL_CULL_FACE); //Todo - text indices are going the wrong way...
}


void GameTechRenderer::RenderUI()
{
	for (int i = 0; i < 5; i++) ImGui::GetIO().MouseDown[i] = false;

	int button = -1;
	if (Window::GetMouse()->ButtonDown(MouseButtons::LEFT)) button = 0;

	if (button != -1) ImGui::GetIO().MouseDown[button] = true;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplWin32_NewFrame();
	bool* showWin = new bool(false), anotherWin;

	ImGui::NewFrame();

	static float f = 0.0f;
	static int counter = 0;
	const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
	bool validName = nameString.length() > 0 && nameString != "Enter Name";
	readyToHost = validName;
	readyToJoin = TestValidHost() && validName;
	string* activeString = enterIP ? &ipString :/* enterPort ? &portString :*/ &nameString;

	switch (levelState)
	{
	case UIState::PAUSED:
		ImGui::PushFont(titleFont);
		ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + main_viewport->Size.x / 4, main_viewport->WorkPos.x + main_viewport->Size.y / 4), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(main_viewport->Size.x / 2, main_viewport->Size.y / 2), ImGuiCond_Always);
		ImGui::Begin("PAUSED", NULL, box_flags);
		if (ImGui::Button("Resume"))
		{
			levelState = UIState::INGAME;
		}
		if (ImGui::Button("Options"))
		{
			levelState = UIState::INGAMEOPTIONS;
		}
		if (ImGui::Button("Exit to Menu"))
		{
			prevState = UIState::INGAME;
			levelState = UIState::LOADING;
		}
		ImGui::PopFont();
		ImGui::End();
		break;
	case UIState::MENU:
		ImGui::SetNextWindowBgAlpha(0);
		ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x, main_viewport->WorkPos.y), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(main_viewport->Size.x, main_viewport->Size.y), ImGuiCond_Always);
		ImGui::Begin("Background", NULL, window_flags);
		ImGui::Image((void*)(intptr_t)backgroundImage->GetObjectID(), ImVec2(main_viewport->Size.x, main_viewport->Size.y));
		ImGui::End();
		ImGui::SetNextWindowBgAlpha(0);
		ImGui::PushFont(titleFont);
		ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x, main_viewport->WorkPos.y), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(main_viewport->Size.x, main_viewport->Size.y), ImGuiCond_Always);
		ImGui::Begin("Title Screen", NULL, window_flags);
		if (ImGui::Button("Single Player"))
		{
			prevState = UIState::MENU;
			levelState = UIState::MODESELECT;
		}
		if (ImGui::Button("Multiplayer"))
		{
			levelState = UIState::MULTIPLAYERMENU;
		}
		if (ImGui::Button("Options"))
		{
			levelState = UIState::OPTIONS;
		}
		if (ImGui::Button("Quit"))
		{
			levelState = UIState::QUIT;
		}
		ImGui::PopFont();
		ImGui::End();
		break;
	case UIState::OPTIONS:
		ImGui::PushFont(titleFont);
		ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x, main_viewport->WorkPos.y), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(main_viewport->Size.x, main_viewport->Size.y), ImGuiCond_Always);
		ImGui::Begin("Options", NULL, window_flags);
		ImGui::Text("VOLUME");
		ImGui::SliderInt("", &(AudioManager::GetVolume()), 0, 100);
		if (ImGui::Button("Back"))
		{
			levelState = UIState::MENU;
		}
		ImGui::PopFont();
		ImGui::End();
		break;
	case UIState::INGAMEOPTIONS:
		ImGui::PushFont(titleFont);
		ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + main_viewport->Size.x / 4, main_viewport->WorkPos.x + main_viewport->Size.y / 4), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(main_viewport->Size.x / 2, main_viewport->Size.y / 2), ImGuiCond_Always);
		ImGui::Begin("Options", NULL, box_flags);
		ImGui::Text("VOLUME");
		ImGui::SliderInt("", &(AudioManager::GetVolume()), 0, 100);
		ImGui::SetWindowFontScale(0.5);
		ImGui::TextWrapped("(Debug Mode Activated with C + H)");
		if (ImGui::Button("Back"))
		{
			levelState = UIState::PAUSED;
		}
		ImGui::PopFont();
		ImGui::End();
		break;
	case UIState::MODESELECT:
		ImGui::SetNextWindowBgAlpha(1);
		ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x, main_viewport->WorkPos.y), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(main_viewport->Size.x / 2, main_viewport->Size.y / 2), ImGuiCond_Always);
		ImGui::Begin("Level 1", NULL, window_flags);
		if (ImGui::ImageButton((void*)(intptr_t)levelImages[0]->GetObjectID(), ImVec2(main_viewport->Size.x / 2.1,
			main_viewport->Size.y / 2.1))) 
		{
			GameManager::GetRenderer()->SetUIState(UIState::LOADING);
			selectedLevel = 1;
		}
		ImGui::End();
		ImGui::SetNextWindowPos(ImVec2(main_viewport->Size.x / 2, main_viewport->WorkPos.y), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(main_viewport->Size.x / 2, main_viewport->Size.y / 2), ImGuiCond_Always);
		ImGui::Begin("Level 2", NULL, window_flags);
		if (ImGui::ImageButton((void*)(intptr_t)levelImages[1]->GetObjectID(), ImVec2(main_viewport->Size.x / 2.1,
			main_viewport->Size.y / 2.1)))
		{
			GameManager::GetRenderer()->SetUIState(UIState::LOADING);
			selectedLevel = 2;
		}
		ImGui::End();
		ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x, main_viewport->Size.y / 2), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(main_viewport->Size.x / 2, main_viewport->Size.y / 2), ImGuiCond_Always);
		ImGui::Begin("Level 3", NULL, window_flags);
		if (ImGui::ImageButton((void*)(intptr_t)levelImages[2]->GetObjectID(), ImVec2(main_viewport->Size.x / 2.1,
			main_viewport->Size.y / 2.1)))
		{
			GameManager::GetRenderer()->SetUIState(UIState::LOADING);
			selectedLevel = 3;
		}
		ImGui::End();
		ImGui::SetNextWindowPos(ImVec2(main_viewport->Size.x / 2, main_viewport->Size.y / 2), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(main_viewport->Size.x / 2, main_viewport->Size.y / 2), ImGuiCond_Always);
		ImGui::Begin("Sandbox", NULL, window_flags);
		if (prevState == UIState::MULTIPLAYERMENU)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
			ImGui::ImageButton((void*)(intptr_t)levelImages[4]->GetObjectID(), ImVec2(main_viewport->Size.x / 2.1,
				main_viewport->Size.y / 2.1));
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
		else if (ImGui::ImageButton((void*)(intptr_t)levelImages[3]->GetObjectID(), ImVec2(main_viewport->Size.x / 2.1,
			main_viewport->Size.y / 2.1)))
		{
			GameManager::GetRenderer()->SetUIState(UIState::LOADING);
			selectedLevel = 4;
		}
		ImGui::End();
		ImGui::SetNextWindowPos(ImVec2(main_viewport->Size.x / 2 - main_viewport->Size.x / 16,
			main_viewport->Size.y / 2 - main_viewport->Size.y / 16), ImGuiCond_Always);
		ImGui::Begin("Back", NULL, window_flags);
		if (ImGui::Button("Back", ImVec2(main_viewport->Size.x / 8, main_viewport->Size.y / 8)))
		{
			levelState = prevState;
		}
		ImGui::End();
		break;
	case UIState::MULTIPLAYERMENU:
		ImGui::PushFont(titleFont);
		ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x, main_viewport->WorkPos.y), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(main_viewport->Size.x, main_viewport->Size.y), ImGuiCond_Always);
		ImGui::Begin("Multiplayer Menu", NULL, window_flags);

		if (!readyToHost)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
			ImGui::Button("Host Game");
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
		else if (ImGui::Button("Host Game"))
		{
			prevState = UIState::MULTIPLAYERMENU;
			levelState = UIState::MODESELECT;
		}

		if (!readyToJoin)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
			ImGui::Button("Join Game");
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
		else if (ImGui::Button("Join Game"))
		{
			levelState = UIState::JOINLEVEL;
		}

		ImGui::Text("Player Name:");
		ImGui::SameLine();

		if (enterName)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
			ImGui::Button(nameString.c_str(), ImVec2(400, 50));
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
		else if (ImGui::Button(nameString.c_str(), ImVec2(400, 50)))
		{
			nameString.clear();
			enterName = true;
			enterIP = false;
		}

		ImGui::Text("Host IP:");
		ImGui::SameLine();

		if (enterIP)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
			ImGui::Button(ipString.c_str(), ImVec2(400, 50));
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
		else if (ImGui::Button(ipString.c_str(), ImVec2(400, 50)))
		{
			ipString.clear();
			enterName = false;
			enterIP = true;
		}

		/* Using hex to get keyboard inputs */
		for (int i = 0x30; i <= 0x39; ++i)
		{
			if (Window::GetKeyboard()->KeyPressed((KeyboardKeys)i) || Window::GetKeyboard()->KeyPressed((KeyboardKeys)(i + 48)))
			{
				activeString->append(std::to_string(i - 0x30));
			}
		}

		if (activeString == &nameString)
		{
			for (int i = 0x41; i <= 0x5A; ++i)
			{
				if (Window::GetKeyboard()->KeyPressed((KeyboardKeys)i))
				{
					char letter = (char)(97 + i - 0x41);

					if (Window::GetKeyboard()->KeyHeld(KeyboardKeys::SHIFT))
					{
						letter -= 32;
					}

					activeString->append(1, letter);
				}
			}
		}
		else if (activeString == &ipString && Window::GetKeyboard()->KeyPressed(KeyboardKeys::PERIOD))
		{
			activeString->append(".");
		}

		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::BACK) && activeString->length() > 0)
		{
			activeString->pop_back();
		}

		if (ImGui::Button("Back"))
		{
			ipString.clear();
			enterIP = false;
			enterName = false;
			levelState = UIState::MENU;
		}
		ImGui::PopFont();
		ImGui::End();
		break;
	case UIState::INGAME:
		ImGui::PushFont(textFont);
		ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 20, main_viewport->WorkPos.y + 20), ImGuiCond_Always);
		ImGui::Begin("Game Info", NULL, window_flags);
		if (levelState == UIState::DEBUG)
		{
			if (gameWorld.GetShuffleObjects())
				ImGui::Text("Shuffle Objects(F1):On");
			else
				ImGui::Text("Shuffle Objects(F1):Off");
		}
		if (player)
		{
			ImGui::Text("Coins Collected %d", player->GetCoinsCollected());
			switch (player->GetPowerUpState())
			{
			case::PowerUpState::LONGJUMP:
				ImGui::Text("Long Jump Power-Up!");
				break;
			case::PowerUpState::SPEEDPOWER:
				ImGui::Text("Speed Boost Power-Up!");
				break;
			}
		}

		ImGui::Text("FPS Average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::PopFont();
		ImGui::End();

		ImGui::PushFont(textFont);
		ImGui::SetNextWindowPos(ImVec2(main_viewport->Size.x - 250, main_viewport->WorkPos.y + 20), ImGuiCond_Always);
		ImGui::Begin("Controls", NULL, window_flags);
		ImGui::Text("Pause(ESC)");
		ImGui::Text("Move(WASD)");
		ImGui::Text("Sprint(LSHIFT)");

		switch (gameWorld.GetMainCamera()->GetState())
		{
		case CameraState::THIRDPERSON:
			ImGui::Text("Change to Topdown Camera[1]");
			break;
		case CameraState::TOPDOWN:
			ImGui::Text("Change to Thirdperson Camera[1]");
			break;
		}
		ImGui::PopFont();
		ImGui::End();
		break;
	case UIState::DEBUG:
		ImGui::PushFont(textFont);
		ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 20, main_viewport->WorkPos.y + 20), ImGuiCond_Always);
		ImGui::Begin("Game Info", NULL, window_flags);
		if (gameWorld.GetShuffleObjects())
			ImGui::Text("Shuffle Objects(F1):On");
		else
			ImGui::Text("Shuffle Objects(F1):Off");
		if (player)
			ImGui::Text("Coins Collected %d", player->GetCoinsCollected());
		ImGui::Text("FPS Average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::PopFont();
		ImGui::End();

		ImGui::PushFont(textFont);
		ImGui::SetNextWindowPos(ImVec2(main_viewport->Size.x - 250, main_viewport->WorkPos.y + 20), ImGuiCond_Always);
		ImGui::Begin("Controls", NULL, window_flags);
		ImGui::Text("Pause(ESC)");

		if (!selectionObject)
		{
			ImGui::Text("Select Object (LM Click)");
		}
		else
		{
			ImGui::Text("De-Select Object (RM Click)");
			if (selectionObject == player)
				ImGui::Text("Lock/Unlock Player (L)");
		}
		ImGui::Text("Change to play mode(Q)");

		switch (gameWorld.GetMainCamera()->GetState())
		{
		case CameraState::THIRDPERSON:
			ImGui::Text("Change to Topdown Camera[1]");
			break;
		case CameraState::TOPDOWN:
			ImGui::Text("Change to Thirdperson Camera[1]");
			break;
		}
		ImGui::PopFont();
		ImGui::End();

		if (selectionObject)
		{
			ImGui::PushFont(textFont);
			ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 20, main_viewport->WorkPos.y + (2.5 * main_viewport->Size.y / 3.5) - 20), ImGuiCond_Always);
			ImGui::Begin("Debug Information", NULL, window_flags);
			ImGui::Text("Selected Object:%s", selectionObject->GetName().c_str());
			ImGui::Text("Position:%s", Vector3(selectionObject->GetTransform().GetPosition()).ToString().c_str());
			ImGui::Text("Orientation:%s", Quaternion(selectionObject->GetTransform().GetOrientation()).ToEuler().ToString().c_str());

			if (selectionObject->GetPhysicsObject() != nullptr)
			{
				if (selectionObject->GetPhysicsObject()->GetPXActor()->is<PxRigidDynamic>())
				{
					PxRigidDynamic* body = (PxRigidDynamic*)selectionObject->GetPhysicsObject()->GetPXActor();
					ImGui::Text("Linear Velocity:%s", Vector3(body->getLinearVelocity()).ToString().c_str());
					ImGui::Text("Angular Velocity:%s", Vector3(body->getAngularVelocity()).ToString().c_str());
					ImGui::Text("Mass:%.1f", body->getMass());
				}
				else
				{
					ImGui::Text("Linear Velocity:%s", Vector3(0, 0, 0).ToString().c_str());
					ImGui::Text("Angular Velocity:%s", Vector3(0, 0, 0).ToString().c_str());
					ImGui::Text("Mass:N/A");
				}

				ImGui::Text("Friction:%.1f", selectionObject->GetPhysicsObject()->GetMaterial()->getDynamicFriction());
				ImGui::Text("Elasticity:%.1f", selectionObject->GetPhysicsObject()->GetMaterial()->getRestitution());
			}
			ImGui::PopFont();
			ImGui::End();
		}

		ImGui::PushFont(textFont);
		ImGui::SetNextWindowPos(ImVec2(main_viewport->Size.x - 250, main_viewport->WorkPos.y + (5 * main_viewport->Size.y / 6) - 20), ImGuiCond_Always);
		ImGui::Begin("PhysX Information", NULL, window_flags);
		ImGui::Text("Static Physics Objects:%d", pXPhysics.GetGScene()->getNbActors(PxActorTypeFlag::eRIGID_STATIC));
		ImGui::Text("Dynamic Physics Objects:%d", pXPhysics.GetGScene()->getNbActors(PxActorTypeFlag::eRIGID_DYNAMIC));
		ImGui::Text("Total Game Objects:%d", gameWorld.gameObjects.size());
		ImGui::Text("Current Collisions:%d", gameWorld.GetTotalCollisions());
		ImGui::PopFont();
		ImGui::End();
		break;
	case UIState::SCOREBOARD:
		ImGui::PushFont(titleFont);
		ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + main_viewport->Size.x / 4, main_viewport->WorkPos.x + main_viewport->Size.y / 4), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(main_viewport->Size.x / 2, main_viewport->Size.y / 2), ImGuiCond_Always);
		ImGui::Begin("PLAYERS", NULL, box_flags);

		if (nGame)
		{
			int levelNetworkObjectsCount = nGame->GetLevelNetworkObjectsCount();
			std::vector<NetworkObject*> networkObjects = nGame->GetNetworkObjects();

			for (int i = levelNetworkObjectsCount; i < networkObjects.size(); i++)
			{
				GameObject* g = networkObjects[i]->GetGameObject();

				if (dynamic_cast<NetworkPlayer*>(g))
				{
					NetworkPlayer* n = (NetworkPlayer*)g;

					if (n->IsConnected())
					{
						ImGui::Text(n->GetPlayerName().c_str());

						if (n->HasFinished())
						{
							ImGui::SameLine(ImGui::GetWindowWidth() - 200);
							ImGui::Text("%.2f", n->GetFinishTime());
						}
					}
				}
			}
		}
		ImGui::PopFont();
		ImGui::End();
		break;
	case UIState::FINISH:
		ImGui::PushFont(titleFont);
		ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + main_viewport->Size.x / 4, main_viewport->WorkPos.x + main_viewport->Size.y / 8), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(main_viewport->Size.x / 2, main_viewport->Size.y / 5.5), ImGuiCond_Always);
		ImGui::Begin("FINISHED!", NULL, box_flags);
		ImGui::Text("Time: %.2f", GameManager::GetPlayer()->GetFinishTime());
		ImGui::PopFont();
		ImGui::End();
		break;
	}
	if ((selectedLevel && levelState == UIState::LOADING) || levelState == UIState::JOINLEVEL) {
		ImGui::SetNextWindowBgAlpha(1);
		ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x, main_viewport->WorkPos.y), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(main_viewport->Size.x, main_viewport->Size.y), ImGuiCond_Always);
		ImGui::Begin("Background", NULL, window_flags);
		ImGui::Image((void*)(intptr_t)loadingImage->GetObjectID(), ImVec2(main_viewport->Size.x, main_viewport->Size.y));
		ImGui::End();
	}
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GameTechRenderer::BuildObjectList()
{
	activeObjects.clear();

	gameWorld.OperateOnContents(
		[&](GameObject* o)
		{
			const RenderObject* g = o->GetRenderObject();
			if (g)
			{
				activeObjects.emplace_back(g);
			}

		}
	);
}

void GameTechRenderer::SortObjectList()
{
	//Who cares!
}

void GameTechRenderer::RenderShadowMap()
{
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glViewport(0, 0, SHADOWSIZE, SHADOWSIZE);

	glCullFace(GL_FRONT);

	BindShader(shadowShader);
	int mvpLocation = glGetUniformLocation(shadowShader->GetProgramID(), "mvpMatrix");

	Matrix4 shadowViewMatrix = Matrix4::BuildViewMatrix(dirLight->GetPosition(), Vector3(0, 0, 0), Vector3(0, 1, 0));
	Matrix4 shadowProjMatrix = Matrix4::Perspective(100.0f, 6000.0f, 1, 45.0f);

	Matrix4 mvMatrix = shadowProjMatrix * shadowViewMatrix;

	shadowMatrix = biasMatrix * mvMatrix; //we'll use this one later on

	for (const auto& i : activeObjects)
	{
		Matrix4 modelMatrix = (*i).GetTransform()->GetMatrix();
		Matrix4 mvpMatrix = mvMatrix * modelMatrix;
		glUniformMatrix4fv(mvpLocation, 1, false, (float*)&mvpMatrix);


		BindMesh((*i).GetMesh());
		int layerCount = (*i).GetMesh()->GetSubMeshCount();

		for (int i = 0; i < layerCount; ++i)
		{
			DrawBoundMesh(i);
		}
	}
	glViewport(0, 0, currentWidth, currentHeight);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glCullFace(GL_BACK);
}

void GameTechRenderer::RenderSkybox()
{
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);

	float screenAspect = (float)currentWidth / (float)currentHeight;
	Matrix4 viewMatrix = gameWorld.GetMainCamera()->BuildViewMatrix();
	Matrix4 projMatrix = gameWorld.GetMainCamera()->BuildProjectionMatrix(screenAspect);

	BindShader(skyboxShader);

	int projLocation = glGetUniformLocation(skyboxShader->GetProgramID(), "projMatrix");
	int viewLocation = glGetUniformLocation(skyboxShader->GetProgramID(), "viewMatrix");
	int texLocation = glGetUniformLocation(skyboxShader->GetProgramID(), "cubeTex");

	glUniformMatrix4fv(projLocation, 1, false, (float*)&projMatrix);
	glUniformMatrix4fv(viewLocation, 1, false, (float*)&viewMatrix);

	glUniform1i(texLocation, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTex);

	BindMesh(skyboxMesh);
	DrawBoundMesh();

	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
}

void GameTechRenderer::RenderCamera()
{
	float screenAspect = (float)currentWidth / (float)currentHeight;
	Matrix4 viewMatrix = gameWorld.GetMainCamera()->BuildViewMatrix();
	Matrix4 projMatrix = gameWorld.GetMainCamera()->BuildProjectionMatrix(screenAspect);
	//Matrix4 tex

	OGLShader* activeShader = nullptr;
	int projLocation = 0;
	int viewLocation = 0;
	int modelLocation = 0;
	int colourLocation = 0;
	int hasVColLocation = 0;
	int hasTexLocation = 0;
	int shadowLocation = 0;
	int textureMatrixLocation = 0;
	//int 

	int lightPosLocation = 0;
	int lightDirLocation = 0;
	int lightColourLocation = 0;
	int lightRadiusLocation = 0;

	int cameraLocation = 0;

	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, shadowTex);

	for (const auto& i : activeObjects)
	{
		OGLShader* shader = (OGLShader*)(*i).GetShader();
		BindShader(shader);

		if (activeShader != shader)
		{
			projLocation = glGetUniformLocation(shader->GetProgramID(), "projMatrix");
			viewLocation = glGetUniformLocation(shader->GetProgramID(), "viewMatrix");
			modelLocation = glGetUniformLocation(shader->GetProgramID(), "modelMatrix");
			shadowLocation = glGetUniformLocation(shader->GetProgramID(), "shadowMatrix");
			textureMatrixLocation = glGetUniformLocation(shader->GetProgramID(), "textureMatrix");
			colourLocation = glGetUniformLocation(shader->GetProgramID(), "objectColour");
			hasVColLocation = glGetUniformLocation(shader->GetProgramID(), "hasVertexColours");
			hasTexLocation = glGetUniformLocation(shader->GetProgramID(), "hasTexture");

			lightPosLocation = glGetUniformLocation(shader->GetProgramID(), "lightPos");
			lightColourLocation = glGetUniformLocation(shader->GetProgramID(), "lightColour");
			lightRadiusLocation = glGetUniformLocation(shader->GetProgramID(), "lightRadius");

			glUniformMatrix4fv(projLocation, 1, false, (float*)&projMatrix);
			glUniformMatrix4fv(viewLocation, 1, false, (float*)&viewMatrix);
			
			cameraLocation = glGetUniformLocation(shader->GetProgramID(), "cameraPos");
			Vector3 pos = gameWorld.GetMainCamera()->GetPosition();
			glUniform3fv(cameraLocation, 1, (float*)&pos);

			Vector4 col = dirLight->GetColour();
			glUniform3fv(glGetUniformLocation(shader->GetProgramID(), "dirLight.lightPos"), 1, (float*)&col);
			glUniform4fv(glGetUniformLocation(shader->GetProgramID(), "dirLight.lightColour"), 1, (float*)&col);
			glUniform1f(glGetUniformLocation(shader->GetProgramID(), "dirLight.lightRadius"), dirLight->GetRadius());

			for (int i = 0; i < std::size(spotLights); i++) {
				Vector4 c = spotLights[i]->GetColour();
				Vector3 p = spotLights[i]->GetPosition();
				Vector3 d = spotLights[i]->GetDirection();
				glUniform3fv(glGetUniformLocation(shader->GetProgramID(), ("spotLights[" + std::to_string(i) + "].lightPos").c_str()), 1, (float*)&p);
				glUniform4fv(glGetUniformLocation(shader->GetProgramID(), ("spotLights["+std::to_string(i)+"].lightColour").c_str()), 1, (float*)&c);
				glUniform1f(glGetUniformLocation(shader->GetProgramID(), ("spotLights["+std::to_string(i)+"].lightRadius").c_str()), spotLights[i]->GetRadius());
				glUniform3fv(glGetUniformLocation(shader->GetProgramID(), ("spotLights[" + std::to_string(i) + "].spotDir").c_str()), 1, (float*)&d);
			}
			Vector3 p1 = dirLight->GetPosition();
			glUniform3fv(lightPosLocation, 1, (float*)&p1);
			glUniform4fv(lightColourLocation, 1, (float*)&col);
			glUniform1f(lightRadiusLocation, dirLight->GetRadius());

			int shadowTexLocation = glGetUniformLocation(shader->GetProgramID(), "shadowTex");
			glUniform1i(shadowTexLocation, 1);

			activeShader = shader;
		}

		modelMatrix = (*i).GetTransform()->GetMatrix();
		glUniformMatrix4fv(modelLocation, 1, false, (float*)&modelMatrix);

		Matrix4 fullShadowMat = shadowMatrix * modelMatrix;
		glUniformMatrix4fv(shadowLocation, 1, false, (float*)&fullShadowMat);

		Matrix4 textureMatrix = (*i).GetTransform()->GetTextureMatrix();
		glUniformMatrix4fv(textureMatrixLocation, 1, false, (float*)&textureMatrix);


		Vector4 col = i->GetColour();
		glUniform4fv(colourLocation, 1, (float*)&col);

		glUniform1i(hasVColLocation, !(*i).GetMesh()->GetColourData().empty());

		glUniform1i(hasTexLocation, (OGLTexture*)(*i).GetDefaultTexture() ? 1 : 0);

		BindMesh((*i).GetMesh());
		int layerCount = (*i).GetMesh()->GetSubMeshCount();

		vector<GLuint> mats;

		BindTextureToShader((OGLTexture*)(*i).GetDefaultTexture(), "mainTex", 0);

		if ((*i).GetMeshMaterial() != nullptr)
		{
			for (int j = 0; j < layerCount; j++)
			{
				const MeshMaterialEntry* matEntry =
					(*i).GetMeshMaterial()->GetMaterialForLayer(j);

				mats.emplace_back(playerTex);
				//BindTextureToShader((OGLTexture*)mats[j], "mainTex", 0);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, mats[j]);
				DrawSubMesh(j);
			}
			//for (int j = 0; j < layerCount; j++)
			//{
				//glActiveTexture(GL_TEXTURE0);
				//glBindTexture(GL_TEXTURE_2D, mats[j]);
				//BindTextureToShader((OGLTexture*)playerTex, "mainTex", 0);
				//DrawSubMesh(j);
			//}
		}
		else
		{
			for (int i = 0; i < layerCount; ++i)
			{
				DrawBoundMesh(i);
			}
		}
		//BindTextureToShader()

		//for (int j = 0; j < layerCount; ++j)
		//{
			//DrawBoundMesh(j);
		//}
		//}

	}
}

Matrix4 GameTechRenderer::SetupDebugLineMatrix()	const
{
	float screenAspect = (float)currentWidth / (float)currentHeight;
	Matrix4 viewMatrix = gameWorld.GetMainCamera()->BuildViewMatrix();
	Matrix4 projMatrix = gameWorld.GetMainCamera()->BuildProjectionMatrix(screenAspect);

	return projMatrix * viewMatrix;
}

Matrix4 GameTechRenderer::SetupDebugStringMatrix()	const
{
	return Matrix4::Orthographic(-1, 1.0f, 100, 0, 0, 100);
}
