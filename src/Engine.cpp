#include "Engine.h"

Engine::Engine(int Width, int Height) : time(0.f), mouse_sensitivity(0.0004f), camera_speed(0.075f),
TWBAR_ENABLED(false)
{
	sf::ContextSettings settings;
	settings.majorVersion = 2;
	settings.minorVersion = 0;
	sf::VideoMode window_size(Width, Height, 24);
	window = new sf::RenderWindow(window_size, "Engine Demo", sf::Style::Fullscreen, settings);
	
	if (glewInit())
	{
		ErrMsg("Failed to init GLEW");
	}
	window->setActive(true);

	memset(all_keys, false, sf::Keyboard::KeyCount);
	LMB = false; RMB = false; MMB = false;

	LoadFromConfig(config);
	sf::Image img;
	img.create(width, height, sf::Color::Red);
	texture.loadFromImage(img);

	window->setFramerateLimit(60);
	window->setVerticalSyncEnabled(vsync);

	spr.setTexture(texture);
	spr.setScale((float)window->getSize().x / GetRenderSize().x, (float)window->getSize().y / GetRenderSize().y);
	
	prev_mouse = sf::Vector2f(0.f, 0.f);

	Window_W = window->getSize().x;
	Window_H = window->getSize().y;
	

	CL = new OpenCL(kernel_cl, CL_device, !debug);

	if (CL->failed)
	{
		window->close();
		ErrMsg("OpenCL initialization failed! \n =(");
	}
	else
	{
		depth = new CLRender(kernel_depth, texture.getNativeHandle(), 1,
			texture.getSize().x, texture.getSize().y, MRRMlvl, MRRMsc, CL, debug);
		depth->UseWorldModel(&world);
	}

	SetAntTweakBar();

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
		bool handled = TwManageEvent(event);
		if (event.type == sf::Event::Closed) {
			window->close();
		}
		if (event.type == sf::Event::MouseButtonReleased)
		{

		}
		if (!handled)
		{
			if (event.type == sf::Event::MouseMoved)
			{
				sf::Vector2f mouse = sf::Vector2f(event.mouseMove.x, event.mouseMove.y);
				if (LMB)
				{
					sf::Vector2f dmouse = mouse - prev_mouse;
					if (prev_mouse.x != 0 && prev_mouse.y != 0)
					{
						world.GetCamera()->RotateX(-dmouse.x*mouse_sensitivity);
						world.GetCamera()->RotateY(dmouse.y*mouse_sensitivity);
					}
				}
				prev_mouse = mouse;
			}
			else if (event.type == sf::Event::MouseButtonPressed) {
				if (event.mouseButton.button == sf::Mouse::Left) {
					LMB = true;
				}
			}
			else if (event.type == sf::Event::MouseButtonReleased) {
				if (event.mouseButton.button == sf::Mouse::Left) {
					LMB = false;
				}
			}
			if (event.type == sf::Event::KeyPressed)
			{
				const sf::Keyboard::Key keycode = event.key.code;
				if (event.key.code < 0 || event.key.code >= sf::Keyboard::KeyCount) { continue; }

				if (keycode == sf::Keyboard::Escape) {
					window->close();
				} 
				else if (keycode == sf::Keyboard::F4) {
					TWBAR_ENABLED = !TWBAR_ENABLED;
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
	DrawAntTweakBar();
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
				case 7:
					CL_device = num;
					break;
				case 8:
					vsync = num;
					break;
				default:
					break;
				}
			}
		}
	}
}


void Engine::SetAntTweakBar()
{
	//TW interface
	TwInit(TW_OPENGL, NULL);
	TwWindowSize(Window_W, Window_H);

	stats = TwNewBar("Statistics");
	TwDefine(" GLOBAL help='Marble Marcher mod by Michael Moroz' ");

	// Change bar position
	int barPos[2] = { 16, 60 };
	TwSetParam(stats, NULL, "position", TW_PARAM_INT32, 2, &barPos);


	TwAddVarRO(stats, "FPS", TW_TYPE_FLOAT, &fps, " label='FPS' ");
	TwAddVarRO(stats, "Render width", TW_TYPE_INT32, &width, "");
	TwAddVarRO(stats, "Render height", TW_TYPE_INT32, &height, "");
	TwAddVarRO(stats, "MRRM level", TW_TYPE_INT32, &MRRMlvl, "");
	TwAddVarRO(stats, "MRRM scale", TW_TYPE_INT32, &MRRMsc, "");
	TwAddButton(stats, "Info1.1", NULL, NULL, (" label='Rendering device: " + CL->device_name + "' ").c_str());
	
	settings = TwNewBar("Settings");
	TwAddVarRW(settings, "Mouse sensitivity", TW_TYPE_FLOAT, &mouse_sensitivity, "min=0.000 max=0.5 step=0.001");
	TwAddVarRW(settings, "Camera speed", TW_TYPE_FLOAT, &camera_speed, "min=0.000 max=5 step=0.001");

	int barPos1[2] = { 16, 450 };

	TwSetParam(settings, NULL, "position", TW_PARAM_INT32, 2, &barPos1);

	TwDefine("Statistics size='300 200' "); // resize bar
	TwDefine("GLOBAL fontsize=3 ");
	TwDefine("Settings color='255 128 0' alpha=210");
	TwDefine("Statistics color='0 128 255' alpha=210");
}

void Engine::DrawAntTweakBar()
{
	//Refresh tweak bar
	if (TWBAR_ENABLED)
	{
		TwRefreshBar(stats);
		TwRefreshBar(settings);
		TwDraw();
	}
}

bool Engine::TwManageEvent(sf::Event &event)
{
	if (TWBAR_ENABLED)
	{
		bool released = event.type == sf::Event::MouseButtonReleased;
		bool moved = event.type == sf::Event::MouseMoved;
		bool LMB = event.mouseButton.button == sf::Mouse::Left;
		bool RMB = event.mouseButton.button == sf::Mouse::Right;
		bool MMB = event.mouseButton.button == sf::Mouse::Middle;

		bool handl = 0;

		if (moved)
		{
			sf::Vector2i mouse = sf::Vector2i(event.mouseMove.x, event.mouseMove.y);
			handl = handl || TwMouseMotion(mouse.x, mouse.y);
		}

		if (LMB && !released)
		{
			handl = handl || TwMouseButton(TW_MOUSE_PRESSED, TW_MOUSE_LEFT);
		}
		if (LMB && released)
		{
			handl = handl || TwMouseButton(TW_MOUSE_RELEASED, TW_MOUSE_LEFT);
		}

		if (MMB && !released)
		{
			handl = handl || TwMouseButton(TW_MOUSE_PRESSED, TW_MOUSE_MIDDLE);
		}
		if (MMB && released)
		{
			handl = handl || TwMouseButton(TW_MOUSE_RELEASED, TW_MOUSE_MIDDLE);
		}

		if (RMB && !released)
		{
			handl = handl || TwMouseButton(TW_MOUSE_PRESSED, TW_MOUSE_RIGHT);
		}
		if (RMB && released)
		{
			handl = handl || TwMouseButton(TW_MOUSE_RELEASED, TW_MOUSE_RIGHT);
		}

		if (RMB && released)
		{
			handl = handl || TwMouseButton(TW_MOUSE_RELEASED, TW_MOUSE_RIGHT);
		}

		bool keypress = event.type == sf::Event::KeyPressed;
		bool keyrelease = event.type == sf::Event::MouseButtonReleased;
		int keycode = event.key.code;

		if (keypress)
		{
			handl = handl || TwKeyPressed(TW_KEY_F1 + keycode - sf::Keyboard::F1, TW_KMOD_NONE);
		}

		return handl;
	}
	else
	{
		return 0;
	}
}


