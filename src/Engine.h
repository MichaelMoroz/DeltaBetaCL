#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include<Utilities.h>
#include<Camera.h>
#include<World.h>
#include<CLRender.h>

#ifdef _WIN32
#include <windows.h>
#define ErrMsg(x) MessageBox(nullptr, TEXT(x), TEXT("ERROR"), MB_OK);
#else
#define ErrMsg(x) std::cerr << x << std::endl;
#endif

static const string config = "engine.cfg";
static const string vert_glsl = "shaders/vertex_shader.glsl";
static const string frag_glsl = "shaders/fragment_shader.glsl";
static const string kernel_cl = "OpenCL/kernel.c";
static const string kernel_depth = "first_pass_render";
static const string kernel_texturing = "";
static const string kernel_post = "";

class Engine
{
private:
	sf::RenderWindow *window;
	int width, height;
	int MRRMlvl, MRRMsc;
	float mouse_sensitivity, camera_speed;
	bool debug;

	bool all_keys[sf::Keyboard::KeyCount];
	OpenCL *CL;
	World world;
	CLRender *depth, *texturing, *post_processing;
	float time, fps, smoothfps;
	sf::Vector2f prev_mouse;
	sf::Clock timer;
	sf::Texture texture;
	sf::Sprite spr;

public:
	Engine(int W, int H);
	vec2 GetRenderSize();
	void SetRenderingTexture(sf::Texture texture);
	void Update();
	bool Running();
	bool Render();
	~Engine();

protected:
	void LoadFromConfig(string file);
	void Draw();

};




