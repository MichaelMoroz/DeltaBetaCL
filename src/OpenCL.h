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


template<class gen_int> string getOpenCLError(gen_int error)
{
	cl_int ERR = error;
	switch (ERR) {
	// run-time and JIT compiler errors
	case 0: return "CL_SUCCESS";
	case -1: return "CL_DEVICE_NOT_FOUND";
	case -2: return "CL_DEVICE_NOT_AVAILABLE";
	case -3: return "CL_COMPILER_NOT_AVAILABLE";
	case -4: return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
	case -5: return "CL_OUT_OF_RESOURCES";
	case -6: return "CL_OUT_OF_HOST_MEMORY";
	case -7: return "CL_PROFILING_INFO_NOT_AVAILABLE";
	case -8: return "CL_MEM_COPY_OVERLAP";
	case -9: return "CL_IMAGE_FORMAT_MISMATCH";
	case -10: return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
	case -11: return "CL_BUILD_PROGRAM_FAILURE";
	case -12: return "CL_MAP_FAILURE";
	case -13: return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
	case -14: return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
	case -15: return "CL_COMPILE_PROGRAM_FAILURE";
	case -16: return "CL_LINKER_NOT_AVAILABLE";
	case -17: return "CL_LINK_PROGRAM_FAILURE";
	case -18: return "CL_DEVICE_PARTITION_FAILED";
	case -19: return "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";

	// compile-time errors
	case -30: return "CL_INVALID_VALUE";
	case -31: return "CL_INVALID_DEVICE_TYPE";
	case -32: return "CL_INVALID_PLATFORM";
	case -33: return "CL_INVALID_DEVICE";
	case -34: return "CL_INVALID_CONTEXT";
	case -35: return "CL_INVALID_QUEUE_PROPERTIES";
	case -36: return "CL_INVALID_COMMAND_QUEUE";
	case -37: return "CL_INVALID_HOST_PTR";
	case -38: return "CL_INVALID_MEM_OBJECT";
	case -39: return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
	case -40: return "CL_INVALID_IMAGE_SIZE";
	case -41: return "CL_INVALID_SAMPLER";
	case -42: return "CL_INVALID_BINARY";
	case -43: return "CL_INVALID_BUILD_OPTIONS";
	case -44: return "CL_INVALID_PROGRAM";
	case -45: return "CL_INVALID_PROGRAM_EXECUTABLE";
	case -46: return "CL_INVALID_KERNEL_NAME";
	case -47: return "CL_INVALID_KERNEL_DEFINITION";
	case -48: return "CL_INVALID_KERNEL";
	case -49: return "CL_INVALID_ARG_INDEX";
	case -50: return "CL_INVALID_ARG_VALUE";
	case -51: return "CL_INVALID_ARG_SIZE";
	case -52: return "CL_INVALID_KERNEL_ARGS";
	case -53: return "CL_INVALID_WORK_DIMENSION";
	case -54: return "CL_INVALID_WORK_GROUP_SIZE";
	case -55: return "CL_INVALID_WORK_ITEM_SIZE";
	case -56: return "CL_INVALID_GLOBAL_OFFSET";
	case -57: return "CL_INVALID_EVENT_WAIT_LIST";
	case -58: return "CL_INVALID_EVENT";
	case -59: return "CL_INVALID_OPERATION";
	case -60: return "CL_INVALID_GL_OBJECT";
	case -61: return "CL_INVALID_BUFFER_SIZE";
	case -62: return "CL_INVALID_MIP_LEVEL";
	case -63: return "CL_INVALID_GLOBAL_WORK_SIZE";
	case -64: return "CL_INVALID_PROPERTY";
	case -65: return "CL_INVALID_IMAGE_DESCRIPTOR";
	case -66: return "CL_INVALID_COMPILER_OPTIONS";
	case -67: return "CL_INVALID_LINKER_OPTIONS";
	case -68: return "CL_INVALID_DEVICE_PARTITION_COUNT";

	// extension errors
	case -1000: return "CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR";
	case -1001: return "CL_PLATFORM_NOT_FOUND_KHR";
	case -1002: return "CL_INVALID_D3D10_DEVICE_KHR";
	case -1003: return "CL_INVALID_D3D10_RESOURCE_KHR";
	case -1004: return "CL_D3D10_RESOURCE_ALREADY_ACQUIRED_KHR";
	case -1005: return "CL_D3D10_RESOURCE_NOT_ACQUIRED_KHR";
	default: return "Unknown OpenCL error";
	}
}

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
			ERROR_MSG(("Error building kernel! " + getOpenCLError(BUILD_ERR) + "\n" + error_msg).c_str());
			inter << "Building kernel errors: " << endl << error_msg << "\n";
			failed = true;
		}
		else if(!mute)
		{
			ERROR_MSG("OpenCL kernel compiled successfully.");
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

