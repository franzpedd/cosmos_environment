#pragma once

#include "util/datafile.h"
#include <cren_math.h>
#include <string>

// forward declarations
namespace Cosmos { class Entity; }

namespace Cosmos
{
	struct IDComponent
	{
		unsigned long long id;

		// constructor
		IDComponent();

		// constructor
		IDComponent(unsigned long long initialID);

		// saves the component into a data file
		static void Serialize(Entity* entity, Datafile& dataFile);

		// loads the component into the entity from data file
		static void Deserialize(Entity* entity, Datafile& dataFile);
	};

	struct NameComponent
	{
		std::string name;

		// constructor
		NameComponent(std::string name) : name(name) {};

		// saves the component into a data file
		static void Serialize(Entity* entity, Datafile& dataFile);

		// loads the component into the entity from data file
		static void Deserialize(Entity* entity, Datafile& dataFile);
	};

	struct TransformComponent
	{
		float3 translation;
		float3 rotation;
		float3 scale;

		// constructor
		TransformComponent(float3 translation = { 0.0f, 0.0f, 0.0f }, float3 rotation = { 0.0f, 0.0f, 0.0f }, float3 scale = { 1.0f, 1.0f, 1.0f });

		// saves the component into a data file
		static void Serialize(Entity* entity, Datafile& dataFile);

		// loads the component into the entity from data file
		static void Deserialize(Entity* entity, Datafile& dataFile);

	public:

		// returns the transformation matrix
		const mat4 GetTransform();
	};
}