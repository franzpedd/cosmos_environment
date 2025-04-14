#include "scene/components.h"

#include "core/logger.h"
#include "scene/entity.h"

namespace Cosmos
{
	IDComponent::IDComponent()
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

	//glm::mat4 rot = glm::toMat4(glm::quat(rotation));
	//glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), translation) * rot * glm::scale(glm::mat4(1.0f), scale);
	//return modelMatrix;
	const mat4 TransformComponent::GetTransform()
	{
		mat4 rotationMat;
		Quat q = yaw_pitch_roll(rotation.y, rotation.x, rotation.z);
		quat_to_mat4(&q, &rotationMat);

		mat4 translationMat;
		mat4_translate(&translationMat, translation.x, translation.y, translation.z);

		mat4 scaleMat;
		mat4_scale(&scaleMat, scale.x, scale.y, scale.z);

		mat4 temp = mat4_mul(&translationMat, &rotationMat);
		return mat4_mul(&temp, &scaleMat);
	}
}