#include "ui/gizmo.h"
#include "ui/wrapper_imgui.h"

#include <imguizmo/imguizmo.h>

#include "core/renderer.h"
#include "scene/components.h"
#include "scene/entity.h"

namespace Cosmos
{
	static bool epsilon_equal(float a, float b, float epsilon) {
		return fabsf(a - b) <= epsilon;
	}

	static bool Decompose(const mat4* transform, float3* translation, float3* rotation, float3* scale) {
		mat4 localMatrix = *transform;
		constexpr float epsilon = 1e-6f;

		// normalize the matrix
		if (epsilon_equal(localMatrix.m33, 0.0f, epsilon)) return false;

		bool row0 = !epsilon_equal(localMatrix.m03, 0.0f, epsilon);
		bool row1 = !epsilon_equal(localMatrix.m13, 0.0f, epsilon);
		bool row2 = !epsilon_equal(localMatrix.m23, 0.0f, epsilon);

		if (row0 || row1 || row2) {
			localMatrix.m03 = localMatrix.m13 = localMatrix.m23 = 0.0f;
			localMatrix.m33 = 1.0f;
		}

		// handle translation
		translation->x = localMatrix.m30;
		translation->y = localMatrix.m31;
		translation->z = localMatrix.m32;

		localMatrix.m30 = localMatrix.m31 = localMatrix.m32 = 0.0f;

		// Handle scale
		float3 row[3];
		row[0] = { localMatrix.m00, localMatrix.m01, localMatrix.m02 };
		row[1] = { localMatrix.m10, localMatrix.m11, localMatrix.m12 };
		row[2] = { localMatrix.m20, localMatrix.m21, localMatrix.m22 };

		// compute scale factor and normalize rows
		scale->x = float3_length(row[0]);
		row[0] = float3_normalize(row[0]);
		scale->y = float3_length(row[1]);
		row[1] = float3_normalize(row[1]);
		scale->z = float3_length(row[2]);
		row[2] = float3_normalize(row[2]);

		// extract rotation
		rotation->y = asinf(-row[0].z);

		if (cosf(rotation->y) != 0.0f) {
			rotation->x = atan2f(row[1].z, row[2].z);
			rotation->z = atan2f(row[0].y, row[0].x);
		}

		else {
			rotation->x = atan2f(-row[2].x, row[1].y);
			rotation->z = 0.0f;
		}

		return true;
	}

	Gizmo::Gizmo(Renderer& renderer)
		: mRenderer(renderer)
	{

	}

	void Gizmo::OnUpdate(Entity* entity)
	{
		if (entity == NULL) return;
		if (!entity->HasComponent<TransformComponent>()) return;

		ImGuizmo::SetOrthographic(false);
		ImGuizmo::SetDrawlist();
		
		// viewport rect
		float vpWidth = (float)ImGui::GetWindowWidth();
		float vpHeight = (float)ImGui::GetWindowHeight();
		ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, vpWidth, vpHeight);
		
		// camera
		auto& camera = mRenderer.GetContext()->camera;
		
		mat4 view = camera.view;
		mat4 proj = mat4_perspectiveRH(to_radians(camera.fov), vpWidth / vpHeight, camera.near, camera.far, 0);
		proj.data[1][1] *= -1.0f;

		// entity
		auto& tc = entity->GetComponent<TransformComponent>();
		mat4 transform = tc.GetTransform();
		
		// snapping
		float snapValue = mMode == Gizmo::Mode::Rotate ? mSnappingValue + 5.0f : mSnappingValue;
		float snapValues[3] = { snapValue, snapValue, snapValue };
		
		// gizmos drawing
		ImGuizmo::Manipulate
		(
			mat4_value_ptr(&view),
			mat4_value_ptr(&proj),
			(ImGuizmo::OPERATION)mMode,
			ImGuizmo::MODE::LOCAL,
			mat4_value_ptr(&transform),
			nullptr,
			mSnapping ? snapValues : nullptr
		);
		
		if (ImGuizmo::IsUsing())
		{
			float3 translation, rotation, scale;
			Decompose(&transform, &translation, &rotation, &scale);
		
			float3 deltaRotation = float3_sub(rotation, tc.rotation);
			tc.translation = translation;
			tc.rotation = float3_add(tc.rotation, deltaRotation);
			tc.scale = scale;
		}
	}
}