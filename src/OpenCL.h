#pragma once
#include <omp.h>
#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <CL/cl.hpp>
#include <CL/cl_gl.h>
#include <string>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <iostream>
#ifdef _WIN32
#include <windows.h>
#define ERROR_MSG(x) MessageBox(nullptr, TEXT(x), TEXT("OpenCL Error!"), MB_OK);
#else
#define ERROR_MSG(x) std::cerr << x << std::endl;
#endif

using namespace cl;
using namespace std;

//This class wraps all the OpenCL loading, GLCL interop and code compilation
template < typename T > std::string num2str(const T& n)
{
	std::ostringstream stm;
	stm << n;
	return stm.str();
}


class OpenCL
{
public:
	Device default_device;
	cl::Context default_context;
	Program default_program;
	Platform default_platform;
	cl::CommandQueue queue;
	vector<std::size_t> group_size;
	string device_name, device_extensions;
	bool failed;

	void operator = (OpenCL A)
	{
		if (!A.failed)
		{
			default_device = A.default_device;
			default_context = A.default_context;
			default_program = A.default_program;
			default_platform = A.default_platform;
			queue = A.queue;
		}
	}


	OpenCL(string Kernel_path, int clgl_device, bool mute): failed(false)
	{
		ifstream sin(Kernel_path);

		if (!sin.is_open())
		{
			ERROR_MSG("Error opening OpenCL kernel file");
		}

		Program::Sources sources;
		string code((istreambuf_iterator<char>(sin)), istreambuf_iterator<char>());

		sources.push_back(make_pair(code.c_str(), code.length()));

		cl_int lError;
		std::string lBuffer;

		//
		// Generic OpenCL creation.
		//

		// Get platforms.
		vector<Platform> all_platforms;
		vector<Device> all_devices;

		Platform::get(&all_platforms);

		if (all_platforms.size() == 0)
		{
			ERROR_MSG("No platforms found. Check OpenCL installation!\n");
			failed = true;
		}

		if (!mute)
		{
			ERROR_MSG((num2str(all_platforms.size()) + " platforms found").c_str());
		}


		bool found_context = 0;
		
		int clgl_d_num = 0;

		for (int i = 0; i < all_platforms.size(); i++)
		{
			Platform test_platform = all_platforms[i];
			test_platform.getDevices(CL_DEVICE_TYPE_ALL, &all_devices);

			if (!mute)
			{
				string device_nam = test_platform.getInfo<CL_PLATFORM_NAME>();
				ERROR_MSG(("Platform: \n" + device_nam).c_str());
			}

			if (all_devices.size() == 0)
			{
				ERROR_MSG("No devices found. Check OpenCL installation!\n");
				failed = true;
			}

			// Create the properties for this context.
			cl_context_properties props[] =
			{
				CL_GL_CONTEXT_KHR,
				(cl_context_properties)wglGetCurrentContext(), // HGLRC handle
				CL_WGL_HDC_KHR,
				(cl_context_properties)wglGetCurrentDC(), // HDC handle
				CL_CONTEXT_PLATFORM,
				(cl_context_properties)test_platform(), 0
			};

			// Look for the compatible context.
			for (int j = 0; j < all_devices.size(); j++)
			{
				Device test_device = all_devices[j];
				cl_device_id aka = test_device();
				cl::Context test_context(clCreateContext(props, 1, &aka, NULL, NULL, &lError));
				if (lError == CL_SUCCESS)
				{
					if (!mute)
					{
						string device_nam = test_device.getInfo<CL_DEVICE_NAME>();
						ERROR_MSG(("We found a GLCL context! \n" + device_nam).c_str());
					}
					
					if (clgl_device == clgl_d_num)
					{
						
						default_context = test_context;
						default_platform = test_platform;
						default_device = test_device;
						found_context = 1;
					}
					clgl_d_num++;
				}
			}
		}

		if (clgl_d_num > 1 && !mute)
		{
			ERROR_MSG("Multiple interoperation OpenCL devices found, change the device number in config.cfg if program exit's/crashes.");
		}

		if (!found_context)
		{
			ERROR_MSG("Unable to find a compatible OpenCL device for GL-CL interoperation.");
			failed = true;
		}

		// Create a command queue.
		default_program = cl::Program(default_context, sources);
		vector<Device> dev;
		dev.push_back(default_device);
		cl_int BUILD_ERR = default_program.build(dev);
		if (BUILD_ERR != CL_SUCCESS)
		{
			//save error log to file
			ofstream inter;
			inter.open("errors.txt", ofstream::ate);
			string error_msg(default_program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(default_device));
			ERROR_MSG(("Error building kernel! Error number: "+ num2str(BUILD_ERR)+"\n" + error_msg).c_str());
			inter << "Building kernel errors: " << endl << error_msg << "\n";
			failed = true;
		}
		else
		{
			cout << "OpenCL kernel compiled successfully." << endl;
		}
		device_name = default_device.getInfo<CL_DEVICE_NAME>();
		group_size = default_device.getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>();
		device_extensions = default_device.getInfo<CL_DEVICE_EXTENSIONS>();
		queue = CommandQueue(default_context, default_device);
	}


	OpenCL()
	{

	}
};

