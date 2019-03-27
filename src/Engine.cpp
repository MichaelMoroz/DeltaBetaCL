#include "Engine.h"

Engine::Engine(int Width, int Height): time(0.f), mouse_sensitivity(0.0004f), camera_speed(0.075f)
{
	sf::ContextSettings settings;
	settings.majorVersion = 2;
	settings.minorVersion = 0;
	sf::VideoMode window_size(Width, Height, 24);
	window = new sf::RenderWindow(window_size, "Engine Demo", sf::Style::Fullscreen, settings);
	//window.setFramerateLimit(60);
	window->setVerticalSyncEnabled(true);
	if (glewInit())
	{
		ErrMsg("Failed to init GLEW");
	}
	window->setActive(true);

	memset(all_keys, false, sf::Keyboard::KeyCount);
	LoadFromConfig(config);
	sf::Image img;
	img.create(width, height, sf::Color::Red);
	texture.loadFromImage(img);

	spr.setTexture(texture);
	spr.setScale((float)window->getSize().x / GetRenderSize().x, (float)window->getSize().y / GetRenderSize().y);
	

	prev_mouse = sf::Vector2f(0.f, 0.f);
	CL = new OpenCL(kernel_cl);

	if (CL->failed)
	{
		window->close();
	}

	depth = new CLRender(kernel_depth, texture.getNativeHandle(), 1, 
		texture.getSize().x, texture.getSize().y, MRRMlvl, MRRMsc, CL, debug);
	depth->UseWorldModel(&world);
}

vec2 Engine::GetRenderSize()
{
	return vec2(width, height);
}

void Engine::SetRenderingTexture(sf::Texture texture)
{
}

void Engine::Update()
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
	while (window->pollEvent(event))
	{
		if (event.type == sf::Event::Closed) {
			window->close();
		}
		if (event.type == sf::Event::MouseButtonReleased)
		{

		}
		if (event.type == sf::Event::MouseMoved)
		{
			sf::Vector2f mouse = sf::Vector2f(event.mouseMove.x, event.mouseMove.y);
			sf::Vector2f dmouse = mouse - prev_mouse;
			if (prev_mouse.x != 0 && prev_mouse.y != 0)
			{
				world.GetCamera()->RotateX(-dmouse.x*mouse_sensitivity);
				world.GetCamera()->RotateY(dmouse.y*mouse_sensitivity);
			}
			prev_mouse = mouse;
		}
		if (event.type == sf::Event::KeyPressed)
		{
			const sf::Keyboard::Key keycode = event.key.code;
			if (event.key.code < 0 || event.key.code >= sf::Keyboard::KeyCount) { continue; }

			if (keycode == sf::Keyboard::Escape) {
				window->close();
			}
		
			all_keys[keycode] = true;
		}
		else if (event.type == sf::Event::KeyReleased) 
		{
			const sf::Keyboard::Key keycode = event.key.code;
			if (event.key.code < 0 || event.key.code >= sf::Keyboard::KeyCount) { continue; }
			all_keys[keycode] = false;
		}
		else if (event.type = sf::Event::MouseWheelScrolled)
		{
			camera_speed *= 1 + (float)event.mouseWheelScroll.delta*0.1f;
		}
	}

	vec3 dx = vec3(0.f);
	if (all_keys[sf::Keyboard::W]) {
		dx.x += camera_speed;
	}
	if (all_keys[sf::Keyboard::S]) {
		dx.x -= camera_speed;
	}
	if (all_keys[sf::Keyboard::A]) {
		dx.z -= camera_speed;
	}
	if (all_keys[sf::Keyboard::D]) {
		dx.z += camera_speed;
	}
	if (all_keys[sf::Keyboard::Q]) {
		world.GetCamera()->Roll(0.001);
	}
	if (all_keys[sf::Keyboard::E]) {
		world.GetCamera()->Roll(-0.001);
	}

	world.GetCamera()->Move(dx);
	world.GetCamera()->Update(dt*0.6);
	timer.restart();
	if (debug) window->close();
}

bool Engine::Running()
{
	return window->isOpen();
}

bool Engine::Render()
{
	window->clear(sf::Color::Black);
	depth->Run();
	CL->queue.finish();
	window->draw(spr);
	window->display();
	return true;
}

Engine::~Engine()
{
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
					width = num;
					break;
				case 2:
					height = num;
					break;
				case 3:
					world.GetCamera()->SetFOV(num);
					break;
				case 4:
					MRRMlvl = num;
					break;
				case 5:
					MRRMsc = num;
					break;
				case 6:
					debug = num;
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
