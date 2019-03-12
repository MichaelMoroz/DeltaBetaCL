#include <Camera.h>

void Camera::SetPosition(vec3 pos)
{
	position = pos;
}

void Camera::RotateLR(float a)
{
	//rotate around UP direction
	quat rotation = angleAxis(degrees(a), GetDirY());
	dirx = rotation*dirx;
	dirz = rotation*dirz;
}

void Camera::RotateUD(float a)
{
	//rotate around sideways direction
	quat rotation = angleAxis(degrees(a), GetDirZ());
	dirx = rotation*dirx;
	diry = rotation*diry;
}

void Camera::RotateRoll(float a)
{
	//rotate around look direction
	quat rotation = angleAxis(degrees(a), GetDirX());
	diry = rotation*diry;
	dirz = rotation*dirz;
}

void Camera::SetRotation(float a, float b, float c)
{
	vec3 Euler(a, b, c);
	quat rotation(Euler);
	dirx = rotation*quat(0, 1, 0, 0);
	diry = rotation*quat(0, 0, 1, 0);
	dirz = rotation*quat(0, 0, 0, 1);
}

void Camera::SetSmoothness(float k)
{
	smooth = k;
}

void Camera::Move(vec3 dx)
{
	position += dx;
}

void Camera::RotateX(float a)
{
}

void Camera::RotateY(float a)
{
}

void Camera::Roll(float a)
{
}

void Camera::Update(float dt)
{
}

vec3 Camera::GetPosition()
{
	return position;
}

vec3 Camera::GetDirX()
{
	return vec3(dirx.x, dirx.y, dirx.z);
}

vec3 Camera::GetDirY()
{
	return vec3(diry.x, diry.y, diry.z);
}

vec3 Camera::GetDirZ() 
{
	return vec3(dirz.x, dirz.y, dirz.z);
}

void Camera::SetMode(CameraMode mode)
{
	cur_mode = mode;
}