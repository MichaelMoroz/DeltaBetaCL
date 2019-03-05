#pragma once

//Useful stuff

#include <GL/glew.h>
#include <cstdlib>
#include <ctime>
#include <string>
#include <sstream>


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