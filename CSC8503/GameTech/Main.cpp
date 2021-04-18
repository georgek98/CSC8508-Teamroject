/*			Created By Rich Davison
 *			Edited By Samuel Buzz Appleby
 *               21/01/2021
 *                170348069
 *				  Main File			 */
 //#include "TutorialGame.h"
#include "../../Common/Window.h"
#include "../CSC8503Common/PushdownMachine.h"
#include "GameStatesPDA.h"

#include "tweeny/tweeny.h"

void GamePushdownAutomata(Window* w);

using namespace NCL;
using namespace CSC8503;

int main(int argc, char** argv)
{

	//auto tween = tweeny::from(0, 0.0f).to(2, 2.0f).during(10000).onStep([](int i, float f) { printf("i=%d f=%f\n", i, f); return false; });
	//while (tween.progress() < 1.0f) tween.step(1);

	
	GameManager::SetWindow(new Win32Code::Win32Window("Dumb Guys!", 1280, 720, false, 100, 100));

	if (!GameManager::GetWindow()->HasInitialised())
		return -1;

	srand(time(0));
	GameManager::GetWindow()->GetTimer()->GetTimeDeltaSeconds(); //Clear the timer so we don't get a larget first dt!
	GamePushdownAutomata(GameManager::GetWindow());
	Window::DestroyGameWindow();		// After we have exited the automata (we've quit) destroy the window
	return 0;
}

/* This method drives the entire game on a pushdown automata */
void GamePushdownAutomata(Window* w)
{
	PushdownMachine machine(new MainMenu());
	while (w->UpdateWindow())
	{
		float dt = w->GetTimer()->GetTimeDeltaSeconds();
		if (dt > 0.1f)
		{
			std::cout << "Skipping large time delta" << std::endl;
			continue;
		}
		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::PRIOR))
			w->ShowConsole(true);
		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::NEXT))
			w->ShowConsole(false);
		if (!machine.Update(dt))
			return;
	}
}