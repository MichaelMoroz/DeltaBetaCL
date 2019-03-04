#ifdef _WIN32
#include <windows.h>
#define ERROR_MSG(x) MessageBox(nullptr, TEXT(x), TEXT("ERROR"), MB_OK);
#else
#define ERROR_MSG(x) std::cerr << x << std::endl;
#endif

//1 rendering, 2 - bloom, 3 - border find, 4 - super-resolution
#define PIPELINE_ITER 10
#define MAX_TEXTURES_PER_ITER 10
//1 - texture down-resolution, 2 - number of textures 
static int PIPELINE[PIPELINE_ITER][2] = { { 128, 3 },{ 128, 1 }, { 32, 3 }, { 32, 1 }, { 16, 3 }, { 16, 1 },{ 4, 3 }, { 4, 1 }, { 1, 3 }, { 1, 1 }, };
//3 color samples 3 normal samples


#include <GL/glew.h>
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <AntTweakBar.h>
#include <Eigen/Dense>
#include <Eigen/Geometry>
#include <string>
#include <cmath>
#include <sstream>
#include <fstream>
#include <iomanip>
//stuff

using namespace std;


template < typename T > std::string num2str(const T& n)
{
	std::ostringstream stm;
	stm <<  n;
	return stm.str();
}

//Maximal size of the IPN array for computing the SDF functions
//Used to store the ray marching models
#define MAX_SDF_ARRAY_SIZE 4096

static const string vert_glsl = "assets/shaders/vertex_shader.glsl";
static const string frag_glsl = "assets/shaders/fragment_shader.glsl";

class ShaderHandle
{
public:
	GLuint ProgramID;

	ShaderHandle()
	{}

	void LoadShaders(const string vertex_file_path, const string fragment_file_path);

	void setUniform(string name, float X, float Y);
	void setUniform(string name, float X, float Y, float Z);
	void setUniform(string name, float X);
	void setUniform(string name, int X);
	void setUniform(string name, Eigen::Matrix3f X, bool transpose);
	void setUniform(string name, Eigen::Vector3f X);

	
	GLuint getNativeHandle()
	{
		return ProgramID;
	}
};



class Engine
{
public:
	Engine(string config, string window_name);

	bool SetFOV(float fov);
	bool Update();
	bool Running();

protected:
	void LoadFromConfig(string file);
	void ManageEvents();
	void Render();
	void DrawQuad();
	void SetTW_Interface();

private:
	sf::Clock timer;
	sf::VideoMode screen_size;
	float ray_march_resolution;
	int ResX, ResY, MarchResX, MarchResY;
	sf::RenderWindow *window;
	sf::Uint32 window_style;
	bool fullscreen;
	ShaderHandle render_shader;
	GLint rshad;
	bool running;
	TwBar *stats, *settings;

	sf::Sprite render_sprite;
	sf::Texture final_texture;
	
	//Rendering pipeline
	GLuint Framebuffer[PIPELINE_ITER];
	GLuint textures[PIPELINE_ITER][MAX_TEXTURES_PER_ITER];
	GLuint textures_HID[PIPELINE_ITER][MAX_TEXTURES_PER_ITER];

	
	/*GLuint main_texture[TEXTURES_MAIN];
	GLuint buffer_texture[TEXTURES_PER_ITER*PIPELINE_ITER-1]*/;
	
	GLint  glfinal_texture;
	
	//GLuint main_texture_HID[TEXTURES_MAIN], buffer_texture_HID[TEXTURES_PER_ITER*PIPELINE_ITER];
	
	
	GLuint vertexbuffer, uvbuffer;
	
	float time;
	float fps, smoothfps;
	float phi, theta, roll;
	Eigen::Vector3f CameraPosition, CameraDirZ, CameraDirY, CameraDirX;
	Eigen::Matrix3f CameraMat;

	int iFracIter;
	float iFracScale;
	float iFracAng1;
    float iFracAng2;
	Eigen::Vector3f iFracShift;
	Eigen::Vector3f iFracCol;
	float roughness, metallic;

	Eigen::Vector3f light, light_color;
	float exposure, light_radius;
	float FOV, SuperResolutionCondition;
	float mouse_sensitivity, roll_speed, dRoll;
	float camspeed, dPos;
	enum  RenMode { NORM, NOFX, DIST, ITER, SURFNORM, POST1 };
	RenMode RenderMode;
	

	//mouse state 
	bool LMB, RMB, MMB, PLMB, PRMB, PMMB;
	sf::Vector2i mouse, dmouse;
};




