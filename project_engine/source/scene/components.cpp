#include "scene/components.h"

#include "core/logger.h"
#include "scene/entity.h"

namespace Cosmos
{
	IDComponent::IDComponent()
		: id(0)
	{
		COSMOS_LOG(LogSeverity::Todo, "Create id");
	}

	IDComponent::IDComponent(unsigned long long initialID)
	{
		id = initialID;
	}

	void IDComponent::Serialize(Entity* entity, Datafile& dataFile)
	{
		if (entity->HasComponent<IDComponent>()) {
			std::string uuid = std::to_string(entity->GetComponent<IDComponent>().id);
			dataFile[uuid]["ID"].SetString(uuid);
		}
	}

	void IDComponent::Deserialize(Entity* entity, Datafile& dataFile)
	{
		if (dataFile.Exists("ID")) {
			std::string id = dataFile["ID"].GetString();
			entity->AddComponent<IDComponent>();
			entity->GetComponent<IDComponent>().id = std::stoull(id, 0, 10);
		}
	}

	void NameComponent::Serialize(Entity* entity, Datafile& dataFile)
	{
		if (entity->HasComponent<NameComponent>()) {
			std::string uuid = std::to_string(entity->GetComponent<IDComponent>().id);
			dataFile[uuid]["Name"].SetString(entity->GetComponent<NameComponent>().name);
		}
	}

	void NameComponent::Deserialize(Entity* entity, Datafile& dataFile)
	{
		if (dataFile.Exists("Name")) {
			std::string name = dataFile["Name"].GetString();
			entity->AddComponent<NameComponent>(name);
		}
	}

	TransformComponent::TransformComponent(float3 translation, float3 rotation, float3 scale)
		: translation(translation), rotation(rotation), scale(scale)
	{
	}

	void TransformComponent::Serialize(Entity* entity, Datafile& dataFile)
	{
		if (entity->HasComponent<TransformComponent>()) {
			std::string uuid = std::to_string(entity->GetComponent<IDComponent>().id);
			auto& component = entity->GetComponent<TransformComponent>();
			auto& place = dataFile[uuid]["Transform"];

			place["Translation"]["X"].SetDouble(component.translation.x);
			place["Translation"]["Y"].SetDouble(component.translation.y);
			place["Translation"]["Z"].SetDouble(component.translation.z);

			place["Rotation"]["X"].SetDouble(component.rotation.x);
			place["Rotation"]["Y"].SetDouble(component.rotation.y);
			place["Rotation"]["Z"].SetDouble(component.rotation.z);

			place["Scale"]["X"].SetDouble(component.scale.x);
			place["Scale"]["Y"].SetDouble(component.scale.y);
			place["Scale"]["Z"].SetDouble(component.scale.z);
		}
	}

	void TransformComponent::Deserialize(Entity* entity, Datafile& dataFile)
	{
		if (dataFile.Exists("Transform")) {
			entity->AddComponent<TransformComponent>();
			auto& component = entity->GetComponent<TransformComponent>();

			auto& dataT = dataFile["Transform"]["Translation"];
			component.translation = { (float)dataT["X"].GetDouble(), (float)dataT["Y"].GetDouble(), (float)dataT["Z"].GetDouble() };

			auto& dataR = dataFile["Transform"]["Rotation"];
			component.rotation = { (float)dataR["X"].GetDouble(), (float)dataR["Y"].GetDouble(), (float)dataR["Z"].GetDouble() };

			auto& dataS = dataFile["Transform"]["Scale"];
			component.scale = { (float)dataS["X"].GetDouble(), (float)dataS["Y"].GetDouble(), (float)dataS["Z"].GetDouble() };
		}
	}

	mat4 TransformComponent::GetTransform()
	{
		quat q = quat_from_euler({ to_radians(rotation.x), to_radians(rotation.y), to_radians(rotation.z) });
		mat4 rmat = mat4_from_quat(q);
		mat4 smat = mat4_scale(mat4_identity(), { scale.x, scale.y, scale.z });
		mat4 tmat = mat4_translate(mat4_identity(), { translation.x, translation.y, translation.z });

		return mat4_mul(smat, mat4_mul(rmat, tmat)); // scale × (rotate × translate) // (Row-Major)
	}
}