


#include <OpenCL.h>
#include <Camera.h>
#include <CLFunction.h>
#include <CLRender.h>
#include <Engine.h>

#include <SFML/Graphics.hpp>
#include <sys/types.h>
#include <sys/stat.h>

#include <iostream>
#include <fstream>
#include <thread> 

#ifdef _WIN32
#include <windows.h>
#define ErrMsg(x) MessageBox(nullptr, TEXT(x), TEXT("ERROR"), MB_OK);
#else
#define ErrMsg(x) std::cerr << x << std::endl;
#endif

#include <mutex>


using namespace std;

const sf::VideoMode window_size(2560, 1440, 24);


#if defined(_WIN32)
int WinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR lpCmdLine, int nCmdShow) {
#else
int main(int argc, char *argv[]) {
#endif 
	sf::Clock timer;
	srand(time(NULL));

	sf::ContextSettings settings;
	settings.majorVersion = 2;
	settings.minorVersion = 0;
	 
	sf::RenderWindow window(window_size, "Engine Demo", sf::Style::Fullscreen, settings);
	//window.setFramerateLimit(60);
	window.setVerticalSyncEnabled(true);
	if (glewInit())
	{
		ErrMsg("Failed to init GLEW");
	}

	sf::Texture test_render;
	Engine engine(&test_render);
	window.setActive(true);

	sf::Sprite spr;
	spr.setTexture(test_render);
	spr.setScale((float)window.getSize().x / engine.GetRenderSize().x, (float)window.getSize().y / engine.GetRenderSize().y);
	while (window.isOpen())
	{
		window.clear(sf::Color::Black);
		engine.Render();
		engine.Update(window);
		window.draw(spr);
		window.display();
	}
	
	return 0;
}