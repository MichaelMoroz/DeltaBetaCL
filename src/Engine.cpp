
#include "Engine.h"

float sqr(float x)
{
	return x*x;
}

void Engine::ManageEvents()
{
	sf::Event event;

	//Update camera matrix
	CameraMat = Eigen::Matrix3f::Identity();
	CameraMat = Eigen::AngleAxisf(phi, Eigen::Vector3f::UnitY())*CameraMat;
	CameraMat = Eigen::AngleAxisf(-theta, Eigen::Vector3f::UnitZ())*CameraMat;
	CameraMat = Eigen::AngleAxisf(roll, Eigen::Vector3f::UnitX())*CameraMat;

	CameraDirX = { CameraMat(0,0), CameraMat(0,1), CameraMat(0,2) };
	CameraDirY = { CameraMat(1,0), CameraMat(1,1), CameraMat(1,2) };
	CameraDirZ = { CameraMat(2,0), CameraMat(2,1), CameraMat(2,2) };

	//mouse handler, to make sure we dont do camera moving stuff when the mouse is over the tweakbar
	bool handl = 0;

	dPos = camspeed / fps;
	dRoll = roll_speed / fps;

	dmouse = sf::Mouse::getPosition(*window) - mouse;
	mouse = sf::Mouse::getPosition(*window);
	PRMB = RMB;
	PLMB = LMB;
	PMMB = MMB;
	LMB = sf::Mouse::isButtonPressed(sf::Mouse::Left);
	RMB = sf::Mouse::isButtonPressed(sf::Mouse::Right);
	MMB = sf::Mouse::isButtonPressed(sf::Mouse::Middle);

	if (sqr(dmouse.x) + sqr(dmouse.y) > 0)
		handl = handl || TwMouseMotion(mouse.x, mouse.y);

	if (LMB && PLMB == 0)
	{
		handl = handl || TwMouseButton(TW_MOUSE_PRESSED, TW_MOUSE_LEFT);
	}
	if (LMB == 0 && PLMB)
	{
		handl = handl || TwMouseButton(TW_MOUSE_RELEASED, TW_MOUSE_LEFT);
	}

	if (MMB && PMMB == 0)
	{
		handl = handl || TwMouseButton(TW_MOUSE_PRESSED, TW_MOUSE_MIDDLE);
	}
	if (MMB == 0 && PMMB)
	{

		handl = handl || TwMouseButton(TW_MOUSE_RELEASED, TW_MOUSE_MIDDLE);
	}

	if (RMB && PRMB == 0)
	{
		handl = handl || TwMouseButton(TW_MOUSE_PRESSED, TW_MOUSE_RIGHT);
	}
	if (RMB == 0 && PRMB)
	{
		handl = handl || TwMouseButton(TW_MOUSE_RELEASED, TW_MOUSE_RIGHT);
	}


	if (LMB)
	{
		if (sqr(dmouse.x) + sqr(dmouse.y)<3600 && !handl)
		{
			phi += dmouse.x*mouse_sensitivity;
			theta += dmouse.y*mouse_sensitivity;
		}
	}
	else
	{
		dmouse = sf::Vector2i(0, 0);
	}

	

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
	{
		roll += dRoll;
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::E))
	{
		roll -= dRoll;
	}

	//keyboard stuff

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
	{
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))
		{
			theta += dRoll;
		}
		else
		{
			CameraPosition += CameraDirX.normalized()*dPos;
		}
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
	{
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))
		{
			phi -= dRoll;
		}
		else
		{
			CameraPosition -= CameraDirZ.normalized()*dPos;
		}
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
	{
		
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))
		{
			phi += dRoll;
		}
		else
		{
			CameraPosition += CameraDirZ.normalized()*dPos;
		}
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
	{
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))
		{
			theta -= dRoll;
		}
		else
		{
			CameraPosition -= CameraDirX.normalized()*dPos;
		}
	}


	while (window->pollEvent(event))
	{
		if (event.type == sf::Event::Closed) {
			window->close();
			running = false;
			break;
		}
		else if (event.type == sf::Event::KeyPressed) {
			const sf::Keyboard::Key keycode = event.key.code;
			if (event.key.code < 0 || event.key.code >= sf::Keyboard::KeyCount) { continue; }

			if (keycode == sf::Keyboard::Escape) {
				window->close();
				running = 0;
			}
			
			if (keycode == sf::Keyboard::R) {

			}
		}

		if (event.type == sf::Event::KeyReleased) {

		}

		if (event.type == sf::Event::MouseWheelScrolled) {
			camspeed = camspeed*(1+0.05*event.mouseWheelScroll.delta);
		}
	}
	

}

///ray_march_resolution lower then 1 makes a lower resolution ray marching, more than 1 is effectively MSAA
Engine::Engine(string config, string window_name)
{
	LoadFromConfig(config);
	running = true;
	sf::ContextSettings settings;
	settings.majorVersion = 2;
	settings.minorVersion = 0;

	time = 0;
	RenderMode = NORM;

	roll = 0;
	phi = 0;
	theta = 0;

	exposure = 1;
	metallic = 0.3;
	roughness = 0.3;
	light = Eigen::Vector3f(-0, -2, -8);
	light_color =  Eigen::Vector3f(23.47, 21.31, 20.79);
	light_radius = 0.5;

	iFracIter= 7;
	iFracScale= 1.8f;
	iFracAng1= -0.12f;
	iFracAng2= 0.5f;
    iFracShift = Eigen::Vector3f(-2.12f, -2.75f, 0.49f);
	iFracCol = Eigen::Vector3f(0.42f, 0.38f, 0.19f);

	//ERROR_MSG(("Resolution: " + num2str(ResX) + " " + num2str(ResY)).c_str());

	ray_march_resolution = abs(ray_march_resolution);

	MarchResX = ResX*ray_march_resolution;
	MarchResY = ResY*ray_march_resolution;
	window = new sf::RenderWindow(screen_size, window_name, window_style, settings);
	window->setFramerateLimit(60);
	SetTW_Interface();

	if (glewInit())
	{
		ERROR_MSG("Failed to initialize GLEW");
	}


	final_texture.create(ResX, ResY);

	render_sprite.setTexture(final_texture);
	render_sprite.setPosition(0, 0);
	render_sprite.setScale(1, 1);

	render_shader.LoadShaders(vert_glsl, frag_glsl);
	rshad = render_shader.getNativeHandle();

	//create the textures
	for (int i = 0; i < PIPELINE_ITER; i++)
	{
		for (int j = 0; j < PIPELINE[i][1]; j++)
		{
			glGenTextures(1, &textures[i][j]);
			glBindTexture(GL_TEXTURE_2D, textures[i][j]);
			//HDR texture
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, ResX / PIPELINE[i][0], ResY / PIPELINE[i][0], 0, GL_RGBA, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			textures_HID[i][j] = glGetUniformLocation(rshad, ("texture_" + num2str(i) + "_" + num2str(j)).c_str());
		}
	}

	

	/*for (int i = 0; i < TEXTURES_MAIN; i++)
	{
		glGenTextures(1, &main_texture[i]);
		glBindTexture(GL_TEXTURE_2D, main_texture[i]);
		//HDR texture
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, MarchResX, MarchResY, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	for (int i = 0; i < TEXTURES_PER_ITER*PIPELINE_ITER - 1; i++)
	{
		glGenTextures(1, &buffer_texture[i]);
		glBindTexture(GL_TEXTURE_2D, buffer_texture[i]);
		//HDR texture
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, MarchResX, MarchResY, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	}*/

	glfinal_texture = final_texture.getNativeHandle();



	glGenFramebuffers(PIPELINE_ITER, &Framebuffer[0]);


	for (int i = 0; i < PIPELINE_ITER; i++)
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, Framebuffer[i]);
		int att = 0;
		if (i != PIPELINE_ITER - 1)
		{
			/*
			for (int j = 0; j < TEXTURES_PER_ITER; j++)
			{
				glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + att, GL_TEXTURE_2D, buffer_texture[i+ TEXTURES_PER_ITER*j], 0);
				att++;
			}
			if (i == 0)
			{
				for (int j = 0; j < TEXTURES_MAIN; j++)
				{	
					glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + att, GL_TEXTURE_2D, main_texture[j], 0);
					att++;
				}
			}*/
			for (int j = 0; j < PIPELINE[i][1]; j++)
			{
				glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + att, GL_TEXTURE_2D, textures[i][j], 0);
				att++;
			}
		}
		else
		{
			glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, glfinal_texture, 0);
			att++;
		}

		GLuint *attachments = new GLuint[att];
		for (int j = 0; j < att; j++)
		{
			attachments[j] = GL_COLOR_ATTACHMENT0 + j;
		}

		glDrawBuffers(att, attachments);
	}


	

	/*for (int i = 0; i < TEXTURES_MAIN; i++)
	{
		main_texture_HID[i] = glGetUniformLocation(rshad, ("main_texture_" + num2str(i)).c_str());
	}

	for (int i = 0; i < TEXTURES_PER_ITER*PIPELINE_ITER - 1; i++)
	{
		buffer_texture_HID[i] = glGetUniformLocation(rshad, ("buffer_texture_" + num2str(i)).c_str());
	}*/


	//A square, needed to render the picture
	static const GLfloat g_vertex_buffer_data[] = {
		-1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f,
	};

	//texture coords on the quad
	static const GLfloat g_uv_buffer_data[] = {
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f,
	};


	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_uv_buffer_data), g_uv_buffer_data, GL_STATIC_DRAW);

	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(
		0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);

	// 2nd attribute buffer : UVs
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glVertexAttribPointer(
		1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
		2,                                // size : U+V => 2
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
	);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	CameraMat << 1, 0, 0,
				 0, 1, 0,
				 0, 0, 1;

	CameraPosition << 0, -2, 0;
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
					fullscreen = num;
					
					if (fullscreen) 
					{
						screen_size = sf::VideoMode::getDesktopMode();
						window_style = sf::Style::Fullscreen;
					}
					else 
					{
						window_style = sf::Style::Close;
					}
					break;
				case 2:
					if (fullscreen)
					{
						ResX = screen_size.width;
					}
					else
					{
						ResX = num;
					}
					break;
				case 3:
					if (fullscreen)
					{
						ResY = screen_size.height;
					}
					else
					{
						ResY = num;
						screen_size = sf::VideoMode(ResX, ResY, 24);
					}
					break;
				case 4:
					ray_march_resolution = num;
					break;
				case 5:
					FOV = num;
					break;
				case 6:
					camspeed = num;
					break;
				case 7:
					roll_speed = num;
					break;
				case 8:
					SuperResolutionCondition = num;
					break;
				case 9:
					mouse_sensitivity = num;
					break;
				default:
					break;
				}
			}
		}
	}
}

void Engine::SetTW_Interface()
{
	//TW interface
	TwInit(TW_OPENGL, NULL);
	TwWindowSize(ResX, ResY);

	stats = TwNewBar("Statistics");
	TwDefine(" GLOBAL help='DeltaBeta engine demonstration' ");



	// Change bar position
	int barPos[2] = { 16, 60 };
	TwSetParam(stats, NULL, "position", TW_PARAM_INT32, 2, &barPos);
	TwAddVarRO(stats, "FPS", TW_TYPE_FLOAT, &smoothfps, " label='FPS' ");
	TwAddVarRO(stats, "Time passed", TW_TYPE_FLOAT, &time, " label='Time passed' ");
	TwAddVarRO(stats, "CameraDirX", TW_TYPE_DIR3F, CameraDirX.data(), "");
	TwAddVarRO(stats, "CameraDirY", TW_TYPE_DIR3F, CameraDirY.data(), "");
	TwAddVarRO(stats, "CameraDirZ", TW_TYPE_DIR3F, CameraDirZ.data(), "");
	TwAddVarRO(stats, "CameraPosition", TW_TYPE_DIR3F, CameraPosition.data(), "");
	TwAddVarRO(stats, "RenderWidth", TW_TYPE_INT32, &MarchResX, "");
	TwAddVarRO(stats, "RenderHeight", TW_TYPE_INT32, &MarchResY, "");
	TwAddVarRO(stats, "WindowWidth", TW_TYPE_INT32, &ResX, "");
	TwAddVarRO(stats, "WindowHeight", TW_TYPE_INT32, &ResY, "");
	//TwAddVarRW(bar, "Camera speed", TW_TYPE_FLOAT, &sp, " min=0.005 max=0.5 step=0.005");
	//TwAddVarRW(bar, "Calculations per frame", TW_TYPE_INT32, &cpf, " min=1 max=500 step=1");
	settings = TwNewBar("Settings");
	
	TwEnumVal modeRender[] = // array used to describe the Scene::AnimMode enum values
	{
		{ 0,   "Normal" },
		{ 1,   "NoFX" },
		{ 2,   "Distance" },
		{ 3,   "Iterations" },
		{ 4,   "SurfNormal" },
		{ 5,   "PostProcessing" },
	};
	TwType modeType = TwDefineEnum("Mode", modeRender, 6);  // create a new TwType associated to the enum defined by the modeEV array


	TwAddVarRW(settings, "RenderMode", modeType, &RenderMode, " help='Change the current rendering mode.' ");
	TwAddVarRW(settings, "SuperResolution", TW_TYPE_FLOAT, &SuperResolutionCondition, "min=0 max=0.5 step=0.005");
	TwAddVarRW(settings, "FOV", TW_TYPE_FLOAT, &FOV, "min=20 max=160 step=1 help='Field of view, deg'");
	TwAddVarRW(settings, "MouseSensitivity", TW_TYPE_FLOAT, &mouse_sensitivity, "min=0.001 max=0.1 step=0.001");
	TwAddVarRW(settings, "CameraSpeed", TW_TYPE_FLOAT, &camspeed, "min=0.1 max=50 step=0.1");
	TwAddSeparator(settings, NULL, " group='Fractal parameters' ");
	
	TwAddVarRW(settings, "FractalIter", TW_TYPE_INT32, &iFracIter, "min=0 max=20  group='Fractal parameter");
	TwAddVarRW(settings, "FractalScale", TW_TYPE_FLOAT, &iFracScale, "min=0 max=5 step=0.01  group='Fractal parameter");
	TwAddVarRW(settings, "FractalAngle1", TW_TYPE_FLOAT, &iFracAng1, "min=-5 max=5 step=0.01  group='Fractal parameter");
	TwAddVarRW(settings, "FractalAngle2", TW_TYPE_FLOAT, &iFracAng2, "min=-5 max=5 step=0.01  group='Fractal parameter");
	TwAddVarRW(settings, "FractalShift", TW_TYPE_DIR3F, iFracShift.data(), " group='Fractal parameter");
	TwAddVarRW(settings, "FractalColor", TW_TYPE_COLOR3F, iFracCol.data(), " group='Fractal parameter");
	TwAddSeparator(settings, "sep1", NULL);
	TwDefine(" Settings/sep1 group='Fractal parameters' ");
	
	TwAddVarRW(settings, "Exposure", TW_TYPE_FLOAT, &exposure, "min=0.01 max=10 step=0.01");
	TwAddVarRW(settings, "Metallic", TW_TYPE_FLOAT, &metallic, "min=0 max=1 step=0.001");
	TwAddVarRW(settings, "Roughness", TW_TYPE_FLOAT, &roughness, "min=0 max=1 step=0.001");
	TwAddVarRW(settings, "LightRadius", TW_TYPE_FLOAT, &light_radius, "min=0 max=100 step=0.01");
	TwAddVarRW(settings, "LightPos", TW_TYPE_DIR3F, light.data(), "");
	TwAddVarRW(settings, "LightColor", TW_TYPE_DIR3F, light_color.data(), "");


	int barPos1[2] = { 16, 450 };

	TwSetParam(settings, NULL, "position", TW_PARAM_INT32, 2, &barPos1);

	TwDefine(" GLOBAL fontsize=3 ");
}

void Engine::Render()
{
	glUseProgram(rshad);

	//send data to GPU
	render_shader.setUniform("RayMarchResolution", MarchResX, MarchResY);
	render_shader.setUniform("time", time);
	render_shader.setUniform("CameraDirMatrix", CameraMat, false);
	render_shader.setUniform("FOV", FOV);
	render_shader.setUniform("CameraPosition", CameraPosition);
	render_shader.setUniform("SuperResCond", SuperResolutionCondition);
	render_shader.setUniform("RenderMode", (int)RenderMode);
	render_shader.setUniform("MainResolution", ResX, ResY);

	render_shader.setUniform("iFracIter", (int)iFracIter);
	render_shader.setUniform("iFracScale", iFracScale);
	render_shader.setUniform("iFracAng1", iFracAng1);
	render_shader.setUniform("iFracAng2", iFracAng2);
	render_shader.setUniform("iFracShift", iFracShift[0], iFracShift[1], iFracShift[2]);
	render_shader.setUniform("iFracCol", iFracCol[0], iFracCol[1], iFracCol[2]);

	render_shader.setUniform("Exposure", exposure);
	render_shader.setUniform("metallic", metallic);
	render_shader.setUniform("roughness", roughness);
	render_shader.setUniform("LIGHT_POSITION", light[0], light[1], light[2]);
	render_shader.setUniform("LIGHT_COLOR", light_color[0], light_color[1], light_color[2]);
	render_shader.setUniform("LIGHT_RADIUS", light_radius);
	//set textures
	int a = 0;

	for (int i = 0; i < PIPELINE_ITER-1; i++)
	{
		for (int j = 0; j < PIPELINE[i][1]; j++)
		{
			glActiveTexture(GL_TEXTURE0 + a);
			glBindTexture(GL_TEXTURE_2D, textures[i][j]);
			glUniform1i(textures_HID[i][j], a);
			a++;
		}
	}

	for (int i = 0; i < PIPELINE_ITER; i++)
	{
		glViewport(0, 0, ResX / PIPELINE[i][0], ResY / PIPELINE[i][0]);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, Framebuffer[i]);
		render_shader.setUniform("RenderPipelineIter", i);
		DrawQuad();
	}


	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glUseProgram(0);
}

void Engine::DrawQuad()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glDrawArrays(GL_QUADS, 0, 4);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	glFinish();
}

bool Engine::Update()
{
	window->clear(sf::Color::Black);
	ManageEvents();
	
	Render();

	window->draw(render_sprite);

	//Refresh tweak bar
	TwRefreshBar(stats);
	TwRefreshBar(settings);
	TwDraw();

	window->display();

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
	return true;
}

bool Engine::Running()
{
	return running;
}

bool Engine::SetFOV(float fov)
{
	FOV = fov;
	return true;
}

void LOGGERER(string text)
{
	ofstream LOG("run_log.txt", std::ios::app);
	LOG << text << "\n";
	LOG.close();
}

void ShaderHandle::LoadShaders(const string vertex_file_path, const string fragment_file_path)
{
	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if (VertexShaderStream.is_open())
	{
		std::string Line = "";
		while (getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}
	else
	{
		ERROR_MSG(("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n" + (string)vertex_file_path).c_str());
		getchar();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if (FragmentShaderStream.is_open())
	{
		std::string Line = "";
		while (getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;


	// Compile Vertex Shader
	LOGGERER("Compiling shader: " + (string)vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0)
	{
		std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		LOGGERER(&VertexShaderErrorMessage[0]);
	}


	// Compile Fragment Shader
	LOGGERER("Compiling shader:" + (string)fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0)
	{
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		LOGGERER(&FragmentShaderErrorMessage[0]);
	}



	// Link the program
	LOGGERER("Linking program!");
	ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0)
	{
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		LOGGERER(&ProgramErrorMessage[0]);
	}


	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);
}

void ShaderHandle::setUniform(string name, float X, float Y)
{
	GLuint A = glGetUniformLocation(ProgramID, name.c_str());
	glUniform2f(A, X, Y);
}

void ShaderHandle::setUniform(string name, float X, float Y, float Z)
{
	GLuint A = glGetUniformLocation(ProgramID, name.c_str());
	glUniform3f(A, X, Y, Z);
}


void ShaderHandle::setUniform(string name, float X)
{
	GLuint A = glGetUniformLocation(ProgramID, name.c_str());
	glUniform1f(A, X);
}

void ShaderHandle::setUniform(string name, int X)
{
	GLuint A = glGetUniformLocation(ProgramID, name.c_str());
	glUniform1i(A, X);
}

void ShaderHandle::setUniform(string name, Eigen::Matrix3f X, bool transpose = false)
{
	GLuint A = glGetUniformLocation(ProgramID, name.c_str());
	glUniformMatrix3fv(A, 1, transpose, X.data());
}

void ShaderHandle::setUniform(string name, Eigen::Vector3f X)
{
	GLuint A = glGetUniformLocation(ProgramID, name.c_str());
	glUniform3fv(A, 1, X.data());
}

