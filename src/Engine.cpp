#include "Engine.h"

Engine::Engine(sf::Texture *texture): time(0.f)
{
	//LoadFromConfig(config);
	prev_mouse = sf::Vector2f(0.f, 0.f);
	CL = new OpenCL(kernel_cl);
	depth = new CLRender(kernel_depth, texture->getNativeHandle(), 1, texture->getSize().x, texture->getSize().y, 1, 1, CL);
	depth->UseWorldModel(&world);
}

void Engine::SetRenderingTexture(sf::Texture texture)
{
}

void Engine::Update(sf::RenderWindow *window)
{
	float dt = timer.getElapsedTime().asSeconds();
	time += dt;

	sf::Event event;
	while (window->pollEvent(event))
	{
		if (event.type == sf::Event::Closed) {
			window->close();
			break;
		}
		else if (event.type == sf::Event::KeyPressed) {
			const sf::Keyboard::Key keycode = event.key.code;
			if (event.key.code < 0 || event.key.code >= sf::Keyboard::KeyCount) { continue; }

			if (keycode == sf::Keyboard::Escape) {
				window->close();
			}

		}
		else if (event.type == sf::Event::MouseButtonReleased)
		{

		}
		else if (event.type == sf::Event::MouseMoved)
		{
			sf::Vector2f mouse = sf::Vector2f(event.mouseButton.x, event.mouseButton.y);
			sf::Vector2f dmouse = mouse - prev_mouse;

			world.GetCamera()->RotateX(dmouse.x);
			world.GetCamera()->RotateY(dmouse.y);

			prev_mouse = mouse;
		}
	}
	
	timer.restart();
}

bool Engine::Render()
{
	depth->Run();
	return true;
}

void Engine::LoadFromConfig(string file)
{
	int increment = 0;
	ifstream config;
	config.open(file);
	if (config.fail())
	{
		ERROR_MSG("Failed to open the configuration file");
	}
	string line;
	while (getline(config, line))
	{
		if (line.substr(0, 1) != "#")
		{
			increment++;
			istringstream iss(line);
			float num;
			while ((iss >> num))
			{
				switch (increment)
				{
				case 1:
					world.GetCamera()->SetFOV(num);
					break;
				default:
					break;
				}
			}
		}
	}
}

void Engine::Draw()
{
}
