#pragma once

#include<Utilities.h>
//This class wraps the OpenCL kernel function computation
class CLFunction
{
private:
	OpenCL* CL;
	cl::NDRange global, local;
	cl::Kernel kernel;
	string name;

public:
	void Initialize(string progname, OpenCL *C, int local_x, int local_y, int global_x, int global_y)
	{
		CL = C;
		name = progname;
		//create the kernel
		kernel = cl::Kernel(CL->default_program, progname.c_str());
		//create the minimum possible global range for this local range
		global = NDRange(ceil((float)global_x / (float)local_x)*local_x, ceil((float)global_y / (float)local_y)*local_y);
		local = NDRange(local_x, local_y);
	}

	CLFunction(string progname, OpenCL *C, int local_x, int local_y, int global_x, int global_y)
	{
		Initialize(progname, C, local_x, local_y, global_x, global_y);
	}

	CLFunction()
	{

	}

	void SetRange(int local_x, int local_y, int global_x, int global_y)
	{
		global = NDRange(ceil((float)global_x / (float)local_x)*local_x, ceil((float)global_y / (float)local_y)*local_y);
		local = NDRange(local_x, local_y);
	}

	void SetArg(int i, cl::Buffer &A)
	{
		kernel.setArg(i, A);
	}

	void SetArg(int i, cl::Image2D &A)
	{
		kernel.setArg(i, A);
	}

	void SetArg(int i, float A)
	{
		kernel.setArg(i, sizeof(float), (void*)&A);
	}


	void SetArg(int i, int A)
	{
		kernel.setArg(i, sizeof(int), (void*)&A);
	}

	void Run()
	{
		CL->queue.enqueueNDRangeKernel(kernel, cl::NullRange, global, local);
	}

	void RFinish()
	{
		CL->queue.enqueueNDRangeKernel(kernel, cl::NullRange, global, local);
		CL->queue.finish();
	}

	void RFlush()
	{
		int error = CL->queue.enqueueNDRangeKernel(kernel, cl::NullRange, global, local);
		CL->queue.flush();
		if (error != 0)
		{
			string error = "OpenCL function " + name + " error: " + num2str(error);
			ERROR_MSG(error.c_str());
		}
	}

	void RFlushCustom(cl::NDRange g, cl::NDRange l)
	{
		CL->queue.enqueueNDRangeKernel(kernel, cl::NullRange, g, l);
		CL->queue.flush();
	}

	~CLFunction()
	{

	}
};