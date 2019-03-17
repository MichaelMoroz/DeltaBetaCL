#pragma once
//World signed distance function with bounding volume hierarchy and various global parameters
#include<Camera.h>

class World
{
public:
	World();
	void Update(float dt);
	Camera* GetCamera();

private:
	float time;
	Camera camera;
};