#pragma once

#include <GL/glew.h>
#include <OpenCL.h>
#include <CLFunction.h>
#include <Camera.h>
#include <glm/gtc/type_ptr.hpp>

class CLRender
{
public:
	CLRender(string name, int textures, int width,
		int height, int lvl, int scale, OpenCL *a);
	CLRender(string name, GLuint textureID, int textures, int width,
		int height, int lvl, int scale, OpenCL *a);
	  
	void Run(Camera &Cam);
	void SetInputTextures(GLuint *textures, int N);
	void UseWorldModel();

private:
	CLFunction render;
	Image2D **clImage;
	int W, H, L, S, textures;
	OpenCL *cl;
};