#include "CLRender.h"


CLRender::CLRender(string name, int textures, int width, int height, int lvl, int scale, OpenCL *a):
	cl(a), W(width), H(height), S(scale), L(lvl)
{
	clImage = new Image2D*[lvl];
	for (int i = 0; i < lvl; i++)
	{
		clImage[i] = new Image2D[textures];
		for (int j = 0; j < textures; j++)
		{
			int w = width*pow(0.5,lvl-i-1);
			int h = height*pow(0.5, lvl - i - 1);
			
			clImage[i][j] = cl::Image2D(cl->default_context, CL_MEM_READ_WRITE,
										cl::ImageFormat(CL_RGBA, CL_FLOAT),
										w, h, 0, NULL);
				
		}
	}
	render.Initialize(name, cl, 1, 1, width, height);
}

CLRender::CLRender(string name, GLuint textureID, int textures, int width, int height, int lvl, int scale, OpenCL *a):
	cl(a), W(width), H(height), S(scale), L(lvl)
{
	clImage = new Image2D*[lvl];
	for (int i = 0; i < lvl; i++)
	{
		clImage[i] = new Image2D[textures];
		for (int j = 0; j < textures; j++)
		{
			int w = width*pow(0.5, lvl - i - 1);
			int h = height*pow(0.5, lvl - i - 1);
			if (j == 0 && i == lvl - 1)
			{
				//Create Interoperation texture
				int lError = 0;
				cl_mem texture_cl = clCreateFromGLTexture(cl->default_context(), CL_MEM_WRITE_ONLY, GL_TEXTURE_2D, 0, textureID, &lError);
				clImage[i][j] = cl::Image2D(texture_cl);
			}
			else
			{
				clImage[i][j] = cl::Image2D(cl->default_context, CL_MEM_READ_WRITE,
											cl::ImageFormat(CL_RGBA, CL_FLOAT),
											w, h, 0, NULL);
			}
		}
	}

	render.Initialize(name, cl, 1, 1, width, height);
}

void CLRender::Run(Camera & Cam)
{
	//set camera parameters
	//render.SetArg(textures,)

	//render through all resolutions
	for (int i = 0; i < L; i++)
	{
		int w = W*pow(0.5, L - i - 1);
		int h = H*pow(0.5, L - i - 1);
		render.SetRange(1, 1, w, h);
		//set all textures for this resolution
		for (int j = 0; j < textures; j++)
		{
			render.SetArg(j, clImage[i][j]);
		}
		//set all previous textures
		render.RFlush();
	}
}

void CLRender::SetInputTextures(GLuint * textures, int N)
{
}

void CLRender::UseWorldModel()
{
}
