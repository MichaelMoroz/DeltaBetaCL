#pragma once

#include <GL/glew.h>
#include <OpenCL.h>
#include <CLFunction.h>
#include <Camera.h>
#include <World.h>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>

static const string log_file = "debug.log";

#pragma pack(push, r1, 1)
typedef struct
{
	cl_float4 position;
	cl_float4 dirx;
	cl_float4 diry;
	cl_float4 dirz;
	cl_float2 resolution;
	cl_float2 step_resolution;
	cl_float FOV;
	cl_float focus;
	cl_float bokeh;
	cl_float exposure; 
	cl_float mblur;
	cl_float speckle;
	cl_float size;
	cl_int stepN; 
	cl_int step;
} cl_camera;
#pragma pack(pop, r1)

class CLRender
{
public:
	CLRender(string name, int textures, int width,
		int height, int lvl, int scale, OpenCL *a);
	CLRender(string name, GLuint textureID, int textures, int width,
		int height, int lvl, int scale, OpenCL *a, bool debug);
	  
	bool Run();
	void SetInputTextures(Image2D *textures, int N);
	void UseWorldModel(World *w);
	void DebugOut(string text);

private:
	void SetCamera(int i, int w, int h);

	World *world;
	CLFunction render;
	Image2D **clImage;
	Image2D void_image;
	int W, H, L, S, textures;
	OpenCL *cl;
	GLuint texture;
	bool debug;
};