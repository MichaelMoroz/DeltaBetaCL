#pragma once
#include <omp.h>
#include <CL/cl.hpp>
#include <string>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <iostream>

using namespace cl;
using namespace std;

//This class wraps all the OpenCL loading, GLCL interop and code compilation

class OpenCL
{
public:
	Device default_device;
	cl::Context default_context;
	Program default_program;
	Platform default_platform;
	cl::CommandQueue queue;

	void operator = (OpenCL A)
	{
		default_device = A.default_device;
		default_context = A.default_context;
		default_program = A.default_program;
		default_platform = A.default_platform;
		queue = A.queue;
	}

	OpenCL(string Kernel_path, int device_n, int platform_n)
	{
		ifstream sin(Kernel_path);

		if (!sin.is_open())
		{
			cout << "Error opening kernel" << endl;
		}

		Program::Sources sources;
		string code((istreambuf_iterator<char>(sin)), istreambuf_iterator<char>());

		sources.push_back(make_pair(code.c_str(), code.length()));

		vector<Platform> all_platforms;
		vector<Device> all_devices;
		vector<Device> dev;

		Platform::get(&all_platforms);

		if (all_platforms.size() == 0)
		{
			cout << "No platforms found. Check OpenCL installation!\n";

		}


		default_platform = all_platforms[platform_n];
		default_platform.getDevices(CL_DEVICE_TYPE_ALL, &all_devices);
		if (all_devices.size() == 0)
		{
			cout << "No devices found. Check OpenCL installation!\n";

		}


		default_device = all_devices[device_n];
		cl::Context context(default_device);
		default_context = context;
		default_program = cl::Program(context, sources);
		dev.push_back(default_device);
		char* options = ""/*"-cl-mad-enable -cl-fast-relaxed-math"*/;
		if (default_program.build(dev,options) != CL_SUCCESS)
		{
			ofstream inter;
			inter.open("errors.txt", ofstream::ate);
			cout << "Error building kernel: " << endl << default_program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(default_device) << "\n";
			inter << "Error building kernel: " << endl << default_program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(default_device) << "\n";
			system("pause");
		}
		else
		{
			cout << "OpenCL kernel compiled successfully." << endl;
		}

		queue = CommandQueue(default_context, default_device);
	}

	OpenCL(string Kernel_path)
	{
		ifstream sin(Kernel_path);

		if (!sin.is_open())
		{
			cout << "Error opening kernel" << endl;
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
		vector<Device> dev;

		Platform::get(&all_platforms);

		if (all_platforms.size() == 0)
		{
			cout << "No platforms found. Check OpenCL installation!\n";
		}

		bool found_context = 0;

		for (int i = 0; i < all_platforms.size(); i++)
		{
			default_platform = all_platforms[i];
			default_platform.getDevices(CL_DEVICE_TYPE_ALL, &all_devices);

			if (all_devices.size() == 0)
			{
				cout << "No devices found. Check OpenCL installation!\n";
			}

			// Create the properties for this context.
			cl_context_properties props[] =
			{
				CL_GL_CONTEXT_KHR,
				(cl_context_properties)wglGetCurrentContext(), // HGLRC handle
				CL_WGL_HDC_KHR,
				(cl_context_properties)wglGetCurrentDC(), // HDC handle
				CL_CONTEXT_PLATFORM,
				(cl_context_properties)default_platform(), 0
			};



			// Look for the compatible context.
			for (int j = 0; j < all_devices.size(); j++)
			{
				default_device = all_devices[j];
				cl_device_id aka = default_device();
				cl::Context test_context(clCreateContext(props, 1, &aka, NULL, NULL, &lError));
				if (lError == CL_SUCCESS)
				{
					cout << "We found the GLCL context." << endl;

					default_context = test_context;
					found_context = 1;
					break;
				}
			}
		}

		if (!found_context)
		{
			std::cerr << "Unable to find a compatible OpenCL device." << std::endl;
		}

		// Create a command queue.
		default_program = cl::Program(default_context, sources);
		dev.push_back(default_device);
		if (default_program.build(dev) != CL_SUCCESS)
		{
			ofstream inter;
			inter.open("errors.txt", ofstream::ate);
			cout << "Error building kernel: " << endl << default_program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(default_device) << "\n";
			inter << "Error building kernel: " << endl << default_program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(default_device) << "\n";
			system("pause");
		}
		else
		{
			cout << "OpenCL kernel compiled successfully." << endl << endl;
		}

		queue = CommandQueue(default_context, default_device);
	}


	OpenCL()
	{

	}
};

