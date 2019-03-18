


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

const sf::VideoMode window_size(1024, 1024, 24);


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
	 
	sf::RenderWindow window(window_size, "Engine Demo", sf::Style::Close, settings);
	window.setFramerateLimit(60);
	window.setVerticalSyncEnabled(true);
	if (glewInit())
	{
		ErrMsg("failed to init GLEW");
	}

	sf::Image img;
	img.create(1024, 1024, sf::Color::White);
	sf::Texture test_render;
	test_render.loadFromImage(img);
	sf::Sprite spr;
	spr.setTexture(test_render);

	Engine engine(&test_render);
	window.setActive(true);

	while (window.isOpen())
	{
		window.clear(sf::Color::Black);
		engine.Update(&window);
		engine.Render();
		window.draw(spr);
		window.display();
	}
	
	return 0;
}