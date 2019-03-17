#include "World.h"

World::World()
{
}

void World::Update(float dt)
{
	camera.Update(dt);
}

Camera * World::GetCamera()
{
	return &camera;
}
