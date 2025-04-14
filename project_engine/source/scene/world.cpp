#include "scene/world.h"

namespace Cosmos
{
	World::World(const char* name)
		: mName(name)
	{
	}

	World::~World()
	{
	}

	void World::OnUpdate(double timestep)
	{
	}

	void World::OnRender(int stage, double interpolation)
	{
	}
}