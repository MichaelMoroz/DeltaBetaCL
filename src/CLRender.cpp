#include "CLRender.h"


CLRender::CLRender(string name, int textures, int width, int height, int lvl, int scale, OpenCL *a) :
	cl(a), W(width), H(height), S(scale), L(lvl), world(NULL), texture(NULL)
{
	clImage = new Image2D*[lvl];
	for (int i = 0; i < lvl; i++)
	{
		clImage[i] = new Image2D[textures];
		int w = glm::max(128.f, floor(width*pow(1.f / S, lvl - i - 1)));
		int h = glm::max(128.f, floor(height*pow(1.f / S, lvl - i - 1)));

		for (int j = 0; j < textures; j++)
		{
			cl_int err = 0;
			static const cl_image_format format = { CL_RGBA, CL_FLOAT };
			cl_mem img = clCreateImage2D(cl->default_context(),
				CL_MEM_READ_WRITE, &format, w, h, 0, NULL, &err);
			clImage[i][j] = cl::Image2D(img);
			DebugOut("OpenCL texture error: " + getOpenCLError(err));
		}

		if (i == 0)
		{
			//void image for initial step
			cl_int err = 0;
			static const cl_image_format format = { CL_RGBA, CL_FLOAT };
			cl_mem img = clCreateImage2D(cl->default_context(),
				CL_MEM_READ_WRITE, &format, w, h, 0, NULL, &err);
			DebugOut("Void texture error: " + getOpenCLError(err));
			void_image = cl::Image2D(img);
		}
	}
	render.Initialize(name, cl, 1, 1, width, height);
}

CLRender::CLRender(string name, GLuint textureID, int txtr, int width, int height, int lvl, int scale, OpenCL *a, bool d) :
	cl(a), W(width), H(height), S(scale), L(lvl), textures(txtr), world(NULL), texture(textureID), debug(d)
{
	clImage = new Image2D*[lvl];
	for (int i = 0; i < lvl; i++)
	{
		clImage[i] = new Image2D[textures];
		int w = glm::max(128.f, floor(width*pow(1.f / S, lvl - i - 1)));
		int h = glm::max(128.f, floor(height*pow(1.f / S, lvl - i - 1)));
		for (int j = 0; j < textures; j++)
		{
			
			if (j == 0 && i == lvl - 1)
			{
				//Create Interoperation texture
				int lError = 0;
				cl_mem texture_cl = clCreateFromGLTexture2D(cl->default_context(), CL_MEM_WRITE_ONLY, GL_TEXTURE_2D, 0, textureID, &lError);
				DebugOut("Interoperation texture error: " + getOpenCLError(lError));
				clImage[i][j] = cl::Image2D(texture_cl);
			}
			else
			{
				cl_int err = 0;
				static const cl_image_format format = { CL_RGBA, CL_FLOAT };
				cl_mem img = clCreateImage2D(cl->default_context(),
					CL_MEM_READ_WRITE, &format, w, h, 0, NULL, &err);
				clImage[i][j] = cl::Image2D(img);
				DebugOut("OpenCL texture error: " + getOpenCLError(err));
			}
			
		}

		if (i == 0)
		{
			//void image for initial step
			cl_int err = 0;
			static const cl_image_format format = { CL_RGBA, CL_FLOAT };
			cl_mem img = clCreateImage2D(cl->default_context(),
				CL_MEM_READ_WRITE, &format, w, h, 0, NULL, &err);
			DebugOut("Void texture error: " + getOpenCLError(err));
			void_image = cl::Image2D(img);
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
		if (texture != NULL)
		{
			cl_int err = clEnqueueAcquireGLObjects(cl->queue(), 1, &clImage[L - 1][0](), 0, 0, 0);
			DebugOut("Acquire GL Objects error:" + getOpenCLError(err));
		}
		//set camera parameters
		/*vec4 data = vec4(world->GetCamera()->GetPosition(), 0);
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
		render.SetArg(2 * textures + 6, 4, glm::value_ptr(data));*/

		//render through all resolutions
		for (int i = 0; i < L; i++)
		{
			//minimal size 128
			int w = glm::max(128.f, floor(W*pow(1.f / S, L - i - 1)));
			int h = glm::max(128.f, floor(H*pow(1.f / S, L - i - 1)));
			//use maximal possible group size
			render.SetRange(floor(sqrt(cl->group_size[0])), floor(sqrt(cl->group_size[0])), w, h);

			//set all previous textures 
			for (int j = 0; j < textures; j++)
			{
				if (i == 0) // if initial step
				{
					render.SetArg(j, void_image);
				}
				else
				{
					render.SetArg(j, clImage[i - 1][j]);
				}
			}

			//set all current textures for this resolution
			for (int j = 0; j < textures; j++)
			{
				render.SetArg(j + textures, clImage[i][j]);
			}
		
			SetCamera(i, w, h);
			//render.SetArg(2 * textures + 7, i);
			cl_int err = render.RFlush();
			DebugOut("Kernel execution error:" + getOpenCLError(err) + ", lvl = " + num2str(i));
		}
		
		if (texture != NULL)
		{
			cl_int err = clEnqueueReleaseGLObjects(cl->queue(), 1, &clImage[L - 1][0](), 0, 0, 0);
			DebugOut("Release GL Objects error:" + getOpenCLError(err));
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

void CLRender::DebugOut(string text)
{
	if (debug)
	{
		ofstream file(log_file, ofstream::app);
		file << text << endl;
		file.close();
	}
}

cl_float4 vec3_to_float4(vec3 A)
{
	cl_float4 B;
	B.x = A.x;
	B.y = A.y;
	B.z = A.z;
	B.w = 0.f;
	return B;
}

cl_float2 tofloat2(float x, float y)
{
	cl_float2 B;
	B.x = x;
	B.y = y;
	return B;
}

void CLRender::SetCamera(int i, int w, int h)
{
	cl_camera cam;
	cam.position = vec3_to_float4(world->GetCamera()->GetPosition());
	cam.dirx = vec3_to_float4(world->GetCamera()->GetDirX()); 
	cam.diry = vec3_to_float4(world->GetCamera()->GetDirY());
	cam.dirz = vec3_to_float4(world->GetCamera()->GetDirZ());
	cam.resolution = tofloat2(W, H);
	cam.step_resolution = tofloat2(w,h);
	vec4 P1 = world->GetCamera()->GetCameraProperties();
	vec4 P2 = world->GetCamera()->GetCameraProperties2();
	cam.FOV = P1.x; 
	cam.focus = P1.z;
	cam.bokeh = P1.w;
	cam.exposure = P1.y;
	cam.mblur = P2.y;
	cam.speckle = P2.z;
	cam.size = P2.x;
	cam.stepN = L;
	cam.step = i;

	render.SetArg(2*textures, cam);
}

