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


#include <SFML/Graphics.hpp>
#include <Utilities.h>
#include <SFML/OpenGL.hpp>
#include <AntTweakBar.h>
#include <glm/glm.hpp>
#include <OpenCL.h>
#include <CLFunction.h>
#include <cmath>
#include <fstream>
#include <iomanip>
//stuff

static const string vert_glsl = "shaders/vertex_shader.glsl";
static const string frag_glsl = "shaders/fragment_shader.glsl";
static const string kernel_cl = "OpenCL/kernel.c";


class Engine
{
public:
	Engine(string config, GLint render_texture, int width, int height);

	bool SetFOV(float fov);
	bool Render();

protected:
	void LoadFromConfig(string file);
	void DrawQuad();

private:
	int ResX, ResY;
	ShaderHandle render_shader;
	GLint rshad;

	GLint  glfinal_texture;

	GLuint vertexbuffer, uvbuffer;
	
	float time;
	float fps, smoothfps;
	float phi, theta, roll;
	glm::vec3 CameraPosition, CameraDirZ, CameraDirY, CameraDirX;
	glm::mat3 CameraMat;

	int iFracIter;
	float iFracScale;
	float iFracAng1;
    float iFracAng2;
	glm::vec3 iFracShift;
	glm::vec3 iFracCol;
	float roughness, metallic;

	glm::vec3 light, light_color;
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




