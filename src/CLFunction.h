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
		cl_int arg_error = kernel.setArg(i, A);
		if (arg_error != CL_SUCCESS)
		{
			string err = "OpenCL setArg " + num2str(i) + " error: " + getOpenCLError(arg_error);
			ERROR_MSG(err.c_str());
		}
	}

	void SetArg(int i, cl::Image2D &A)
	{
		cl_int arg_error = kernel.setArg(i, A);
		if (arg_error != CL_SUCCESS)
		{
			string err = "OpenCL setArg " + num2str(i) + " error: " + getOpenCLError(arg_error);
			ERROR_MSG(err.c_str());
		}
	}

	//floatN
	void SetArg(int i, int N, float *A)
	{	
		cl_int arg_error = kernel.setArg(i, sizeof(float)*N, A);
		if (arg_error != CL_SUCCESS)
		{
			string err = "OpenCL setArg " + num2str(i) + " error: " + getOpenCLError(arg_error);
			ERROR_MSG(err.c_str());
		}
	}

	template<class E> void SetArg(int i, E& A)
	{
		int size = sizeof(E);
		cl_int arg_error = kernel.setArg(i, size, (void*)&A);
		if (arg_error != CL_SUCCESS)
		{
			string err = "OpenCL setArg " + num2str(i) + " error: " + getOpenCLError(arg_error);
			ERROR_MSG(err.c_str());
		}
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

	int RFlush()
	{
		cl_int error = CL->queue.enqueueNDRangeKernel(kernel, cl::NullRange, global, local);
		CL->queue.flush();
		if (error != 0)
		{
			string err = string("OpenCL function \"") + name + string("\" error: ") + getOpenCLError(error);
			ERROR_MSG(err.c_str());
		}
		return error;
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