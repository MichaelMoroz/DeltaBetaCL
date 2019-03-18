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

	sf::Event event;
	vec3 dx = vec3(0.f);
	while (window->pollEvent(event))
	{
		if (event.type == sf::Event::Closed) {
			window->close();
			break;
		}
		else if (event.type == sf::Event::MouseButtonReleased)
		{

		}
		else if (event.type == sf::Event::MouseMoved)
		{
			sf::Vector2f mouse = sf::Vector2f(event.mouseMove.x, event.mouseMove.y);
			sf::Vector2f dmouse = mouse - prev_mouse;
			if (prev_mouse.x != 0 && prev_mouse.y != 0)
			{
				world.GetCamera()->RotateX(-dmouse.x*0.0004);
				world.GetCamera()->RotateY(dmouse.y*0.0004);
			}
			prev_mouse = mouse;
		}
		else if (event.type == sf::Event::KeyPressed) {
			const sf::Keyboard::Key keycode = event.key.code;
			if (event.key.code < 0 || event.key.code >= sf::Keyboard::KeyCount) { continue; }

			if (keycode == sf::Keyboard::Escape) {
				window->close();
			}
			
			float speed = 0.01;
			if (keycode == sf::Keyboard::W) {
				dx.x += speed;
			}
			if (keycode == sf::Keyboard::S) {
				dx.x -= speed;
			}
			if (keycode == sf::Keyboard::A) {
				dx.z += speed;
			}
			if (keycode == sf::Keyboard::A) {
				dx.z -= speed;
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
			{
				world.GetCamera()->Roll(0.001);
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::E))
			{
				world.GetCamera()->Roll(-0.001);
			}
			
		}
	}
	world.GetCamera()->Move(dx);
	world.GetCamera()->Update(0.01);
	timer.restart();
}

bool Engine::Render()
{
	depth->Run();
	CL->queue.finish();
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
