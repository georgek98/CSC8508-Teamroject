#include "../../Common/Window.h"
#include "NavMeshRenderer.h"
using namespace NCL;

int main() {
	Window* w = Window::CreateGameWindow("NavMesh Tester", 1120, 768);
	w->SetConsolePosition(100, 0);
	if (!w->HasInitialised()) {
		return -1;
	}
	NavMeshRenderer* renderer = new NavMeshRenderer();
	while (w->UpdateWindow() && !Window::GetKeyboard()->KeyDown(KeyboardKeys::ESCAPE)) {
		float time = w->GetTimer()->GetTimeDeltaSeconds();
		renderer->Update(time);
		renderer->Render();
	}

	delete renderer;

	Window::DestroyGameWindow();
}