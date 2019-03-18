#include "CLRender.h"


CLRender::CLRender(string name, int textures, int width, int height, int lvl, int scale, OpenCL *a) :
	cl(a), W(width), H(height), S(scale), L(lvl), world(NULL)
{
	clImage = new Image2D*[lvl];
	for (int i = 0; i < lvl; i++)
	{
		clImage[i] = new Image2D[textures];
		for (int j = 0; j < textures; j++)
		{
			int w = width*pow(0.5, lvl - i - 1);
			int h = height*pow(0.5, lvl - i - 1);

			clImage[i][j] = cl::Image2D(cl->default_context, CL_MEM_READ_WRITE,
				cl::ImageFormat(CL_RGBA, CL_FLOAT),
				w, h, 0, NULL);

		}
	}
	render.Initialize(name, cl, 1, 1, width, height);
}

CLRender::CLRender(string name, GLuint textureID, int txtr, int width, int height, int lvl, int scale, OpenCL *a) :
	cl(a), W(width), H(height), S(scale), L(lvl), textures(txtr), world(NULL)
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

bool CLRender::Run()
{
	if (world == NULL)
	{
		ERROR_MSG("No world model to render.");
		return false;
	} 
	else
	{
		//set camera parameters
		vec4 data = vec4(world->GetCamera()->GetPosition(), 0);
		render.SetArg(2 * textures, 4, glm::value_ptr(data));
		data = vec4(world->GetCamera()->GetDirX(), 0);
		render.SetArg(2 * textures + 1, 4, glm::value_ptr(data));
		data = vec4(world->GetCamera()->GetDirY(), 0);
		render.SetArg(2 * textures + 2, 4, glm::value_ptr(data));
		data = vec4(world->GetCamera()->GetDirZ(), 0);
		render.SetArg(2 * textures + 3, 4, glm::value_ptr(data));
		data = vec4(float(W), float(H), float(L), float(S));
		render.SetArg(2 * textures + 4, 4, glm::value_ptr(data));
		data = world->GetCamera()->GetCameraProperties();
		render.SetArg(2 * textures + 5, 4, glm::value_ptr(data));
		data = world->GetCamera()->GetCameraProperties2();
		render.SetArg(2 * textures + 6, 4, glm::value_ptr(data));

		//render through all resolutions
		for (int i = 0; i < L; i++)
		{
			int w = W*pow(0.5, L - i - 1);
			int h = H*pow(0.5, L - i - 1);
			render.SetRange(floor(sqrt(cl->group_size[0])), floor(sqrt(cl->group_size[0])), w, h);
			//set all current textures for this resolution
			for (int j = 0; j < textures; j++)
			{
				render.SetArg(j, clImage[i][j]);
			}
			//set all previous textures 
			for (int j = 0; j < textures; j++)
			{
				if (i == 0) // if initial step
				{
					render.SetArg(j + textures, clImage[i][j]);
				}
				else
				{
					render.SetArg(j + textures, clImage[i - 1][j]);
				}
			}
			render.SetArg(2 * textures + 7, i);
			render.RFlush();
		}
		
		return true;
	}
}

void CLRender::SetInputTextures(Image2D * textures, int N)
{

}

void CLRender::UseWorldModel(World * w)
{
	world = w;
}

