#pragma once

//Useful stuff

#include <GL/glew.h>
#include <cstdlib>
#include <ctime>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#define ERROR_MSG(x) MessageBox(nullptr, TEXT(x), TEXT("ERROR"), MB_OK);
#else
#define ERROR_MSG(x) std::cerr << x << std::endl;
#endif


using namespace std;


class ShaderHandle
{
public:
	GLuint ProgramID;

	ShaderHandle()
	{}

	void LoadShaders(const string vertex_file_path, const string fragment_file_path);

	void setUniform(string name, float X, float Y);
	void setUniform(string name, float X, float Y, float Z);
	void setUniform(string name, float X);
	void setUniform(string name, int X);

	GLuint getNativeHandle()
	{
		return ProgramID;
	}
};

void Logger(string text)
{
	ofstream LOG("log.txt", std::ios::app);
	LOG << text << "\n";
	LOG.close();
}

//random float number between 0 and 1
float Rand()
{
	return ((float)(rand() % RAND_MAX) / (float)RAND_MAX);
}

template < typename T > std::string num2str(const T& n)
{
	std::ostringstream stm;
	stm << n;
	return stm.str();
}
