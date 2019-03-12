#pragma once

#include <glm\glm.hpp>
#include <glm\gtc\quaternion.hpp>

using namespace glm;

class Camera
{
public:
	enum CameraMode
	{
		Free,
		ThirdPerson,
		FirstPerson
	};

	Camera(): alpha(0), beta(0), gamma(0), cur_mode(Free)
	{
		dirx = quat(0, 1, 0, 0);
		diry = quat(0, 0, 1, 0);
		dirz = quat(0, 0, 0, 1);
		velocity = vec3(0);
		position = vec3(0);
	}

	void SetPosition(vec3 pos);
	void RotateLR(float a);
	void RotateUD(float a);
	void RotateRoll(float a);
	void SetRotation(float a, float b, float c);
	void SetSmoothness(float k);
	void SetMode(CameraMode mode);
	void SetRadius(float r);
	void LookAt(vec3 pos);

	vec3 GetPosition();
	vec3 GetDirX();
	vec3 GetDirY();
	vec3 GetDirZ();

	void Shift(vec3 dx);
	void Move(vec3 dv);
	void RotateX(float a);
	void RotateY(float a);
	void Roll(float a);

	void Update(float dt);
private:
	CameraMode cur_mode;
	//camera position 
	vec3 position;
	// camera directions
	quat dirx, diry, dirz;

	//camera inertial speed
	vec3 velocity;

	//camera direction inertial angular velocities
	float alpha, beta, gamma;
	
	// 3 person view radius
	float radius;

	//movement smoothness parameter
	float smooth;
};