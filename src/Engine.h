#include<SFML/Graphics.hpp>
#include<Utilities.h>
#include<Camera.h>
#include<World.h>
#include<CLRender.h>

static const string config = "config.txt";
static const string vert_glsl = "shaders/vertex_shader.glsl";
static const string frag_glsl = "shaders/fragment_shader.glsl";
static const string kernel_cl = "OpenCL/kernel.c";
static const string kernel_depth = "first_pass_render";
static const string kernel_texturing = "";
static const string kernel_post = "";

class Engine
{
private:
	bool all_keys[sf::Keyboard::KeyCount];
	OpenCL *CL;
	World world;
	CLRender *depth, *texturing, *post_processing;
	float time, fps, smoothfps;
	sf::Vector2f prev_mouse;
	sf::Clock timer;

public:
	Engine(sf::Texture *texture);

	void SetRenderingTexture(sf::Texture texture);
	void Update(sf::RenderWindow *window);
	bool Render();
	~Engine();

protected:
	void LoadFromConfig(string file);
	void Draw();

};




