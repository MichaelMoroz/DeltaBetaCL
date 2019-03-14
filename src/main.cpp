


#include <OpenCL.h>
#include <Camera.h>
#include <CLFunction.h>
#include <CLRender.h>

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
	if (glewInit())
	{
		std::cerr << "failed to init GLEW" << std::endl;
	}



	//Load opencl code and compile it
	OpenCL CL("OpenCL\\kernel.c");
	sf::Image img;
	img.create(1024, 512, sf::Color::White);
	sf::Texture test_render;
	test_render.loadFromImage(img);
	Camera camera;
	CLRender RND("first_pass_render", test_render.getNativeHandle(), 1, 1024, 512, 1, 1, &CL);
	sf::Sprite spr;
	spr.setTexture(test_render);

	RND.Run(camera);
	float fps = 0, smoothfps = 0, time = 0;
	while (window.isOpen())
	{
		
		window.clear(sf::Color::Black);
		sf::Event event;
		
		auto LMB = sf::Mouse::isButtonPressed(sf::Mouse::Left);
		auto RMB = sf::Mouse::isButtonPressed(sf::Mouse::Right);
		auto MMB = sf::Mouse::isButtonPressed(sf::Mouse::Middle);

		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed) {
				window.close();
				break;
			}
			else if (event.type == sf::Event::KeyPressed) {
				const sf::Keyboard::Key keycode = event.key.code;
				if (event.key.code < 0 || event.key.code >= sf::Keyboard::KeyCount) { continue; }

				if (keycode == sf::Keyboard::Escape) {
					window.close();
				}

			}

		}

		window.draw(spr);
		window.display();

		//fps and time calculations
		float dt = timer.getElapsedTime().asSeconds();
		fps = 1 / dt;
		if (time == 0)
		{
			smoothfps = fps;
		}
		else
		{
			smoothfps = smoothfps*0.95 + 0.05*fps;
		}
		time += dt;

		timer.restart();
	}
	
	return 0;
}