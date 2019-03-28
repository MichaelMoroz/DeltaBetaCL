#include <Engine.h>

#include <iostream>
#include <fstream>

using namespace std;

#if defined(_WIN32)
int WinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR lpCmdLine, int nCmdShow) {
#else
int main(int argc, char *argv[]) {
#endif 
	srand(time(NULL));

	Engine engine(1920, 1080);

	while (engine.Running())
	{
		engine.Render();
		engine.Update();	
	}
	
	return 0;
}