/*			Created By Rich Davison
 *			Edited By Samuel Buzz Appleby
 *               21/01/2021
 *                170348069
 *			Game Tech Renderer Definition		 */
#pragma once
#include "../../Plugins/OpenGLRendering/OGLRenderer.h"
#include "../../Plugins/OpenGLRendering/OGLShader.h"
#include "../../Plugins/OpenGLRendering/OGLTexture.h"
#include "../../Plugins/OpenGLRendering/OGLMesh.h"
#include "../CSC8503Common/GameWorld.h"
#include "../../Common/Imgui/imgui_impl_opengl3.h"
#include "../../Common/Imgui/imgui_impl_win32.h"
#include "../CSC8503Common/PxPhysicsSystem.h"
#include "../../Common/Imgui/imgui_internal.h"
#include "../../Common/AudioManager.h"
#include <sstream>
#include "../CSC8503Common/PlayerObject.h"

namespace NCL {
	class Maths::Vector3;
	class Maths::Vector4;
	namespace CSC8503
	{
		class RenderObject;
		class NetworkedGame;
		enum class UIState { PAUSED, MENU, OPTIONS, MODESELECT, MULTIPLAYERMENU, JOINLEVEL, 
			INGAME, INGAMEOPTIONS, QUIT, DEBUG, SCOREBOARD, FINISH, LOADING };

		const int NUM_SPOTLIGHTS = 2;

		class GameTechRenderer : public OGLRenderer
		{
		public:
			GameTechRenderer(GameWorld& world, PxPhysicsSystem& physics);
			~GameTechRenderer();
			void InitGUI(HWND handle);
			bool TestValidHost();

			void SetUIState(UIState val)
			{
				levelState = val;
			}

			UIState GetUIState()
			{
				return levelState;
			}

			void SetSelectionObject(GameObject* val)
			{
				selectionObject = val;
			}

			void SetLockedObject(GameObject* val)
			{
				lockedObject = val;
			}

			string GetIP() const
			{
				return ipString;
			}

			string GetPlayerName() const {
				return nameString;
			}

			void SetPlayer(PlayerObject* val) {
				player = val;
			}

			void SetSelectedLevel(int val) {
				selectedLevel = val;
			}

			int GetSelectedLevel() const {
				return selectedLevel;
			}
			GLuint playerTex;

			void SetNetworkedGame(NetworkedGame* game) {
				nGame = game;
			}

			void SetPreviousState(UIState state) {
				prevState = state;
			}

			UIState GetPreviousUIState() const
			{
				return prevState;
			}
			void SetLightPosition(Vector3 lPos)
			{
				lightPosition = lPos;
			}


		protected:
			void RenderFrame()	override;

			Matrix4 SetupDebugLineMatrix()	const override;
			Matrix4 SetupDebugStringMatrix()const override;

			OGLShader* defaultShader;

			GameWorld& gameWorld;
			PxPhysicsSystem& pXPhysics;
			PlayerObject* player;
			NetworkedGame* nGame;

			void RenderUI();

			void BuildObjectList();
			void SortObjectList();
			void RenderShadowMap();
			void RenderCamera();
			void RenderSkybox();

			//void RenderPostProcessing();
			//void PresentScene();

			void LoadSkybox();

			vector<const RenderObject*> activeObjects;

			OGLShader* skyboxShader;
			OGLMesh* skyboxMesh;
			GLuint		skyboxTex;

			//shadow mapping things
			OGLShader*  shadowShader;
			GLuint		shadowTex;
			GLuint		shadowFBO;

			//Matrix things
			Matrix4		projMatrix;		//Projection matrix
			Matrix4		modelMatrix;	//Model matrix. NOT MODELVIEW
			Matrix4		viewMatrix;		//View matrix
			Matrix4		textureMatrix;	//Texture matrix
			Matrix4     shadowMatrix;

			Vector4		lightColour;
			float		lightRadius;
			Vector3		lightPosition;
			Vector3		lightDirection;

			ImFont* textFont;
			ImFont* titleFont;

			ImGuiWindowFlags window_flags;
			ImGuiWindowFlags box_flags;
			OGLTexture* levelImages[5];
			OGLTexture* backgroundImage;
			OGLTexture* loadingImage;

			UIState levelState;
			UIState prevState;
			GameObject* lockedObject;
			GameObject* selectionObject;

			int selectedLevel = 0;
			bool readyToJoin = false;
			bool readyToHost = false;

			string ipString = "Enter IP";
			string nameString = "Enter Name";

			bool enterName = false;
			bool enterIP = false;
			bool enterPort = false;

			Light* dirLight;
			Light* spotLights[NUM_SPOTLIGHTS];

			bool postProcessing;
			bool blurEffect;
			bool edgeEffect;

			OGLShader* blurShader;
			GLuint processFBO;
			GLuint bufferColourTex[2];
			GLuint bufferDepthTex;
		};
	}
}

