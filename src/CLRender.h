#pragma once

#include <GL/glew.h>
#include <OpenCL.h>
#include <CLFunction.h>
#include <Camera.h>
#include <World.h>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>

static const string log_file = "debug.log";

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
	World *world;
	CLFunction render;
	Image2D **clImage;
	Image2D void_image;
	int W, H, L, S, textures;
	OpenCL *cl;
	GLuint texture;
	bool debug;
};