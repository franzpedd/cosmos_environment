#include "cren_camera.h"

/// @brief updates the view projection matrix of a camera
/// @param camera pointer to the camera
static void internal_cren_camera_update_view_matrix(CRenCamera* camera) { 
	// rotation matrix
	mat4 rotationMat = mat4_identity();
	rotationMat = mat4_rotate(&rotationMat, to_radians(camera->rotation.x * (camera->flipY == 1 ? -1.0f : 1.0f)), 1.0f, 0.0f, 0.0f);
	rotationMat = mat4_rotate(&rotationMat, to_radians(camera->rotation.y), 0.0f, 1.0f, 0.0f);
	rotationMat = mat4_rotate(&rotationMat, to_radians(camera->rotation.z), 0.0f, 0.0f, 1.0f);
	
	// translation matrix
	mat4 translationMat = mat4_identity();
	translationMat = mat4_translate(&translationMat, camera->position.x, (camera->flipY == 1 ? -1.0f : 1.0f) * camera->position.y, camera->position.z);
	
	// scale matrix
	mat4 scaleMat = mat4_identity();
	scaleMat = mat4_scale(&scaleMat, camera->scale.x, camera->scale.y, camera->scale.z);
	
	// combine matrices based on camera type
	if (camera->type == CAMERA_TYPE_FREE_LOOK) {
		// free-look camera: Scale > Rotate > Translate
		camera->view = mat4_mul(&translationMat, &rotationMat);
		camera->view = mat4_mul(&camera->view, &scaleMat);
	}
	else if(camera->type == CAMERA_TYPE_LOOK_AT) {
		// look-at: Scale > Translate > Rotate
		camera->view = mat4_mul(&rotationMat, &translationMat);
		camera->view = mat4_mul(&camera->view, &scaleMat);
	}
	
	camera->viewPosition.x *= -1.0f;
	camera->viewPosition.y *=  1.0f;
	camera->viewPosition.z *= -1.0f;
}

CRenCamera cren_camera_create(CameraType type, float initialAspectRatio) {
	CRenCamera camera = { 0 };
	camera.type = type;
	camera.flipY = 1; // only vulkan for now, therefore this will always be true
	camera.fov = 45.0f;
	camera.near = 0.1f;
	camera.far = 256.0f;
	camera.aspectRatio = initialAspectRatio;
	camera.movementSpeed = 1.0f;
	camera.rotationSpeed = 1.0f;
	camera.modifierSpeed = 2.5f;

	// calculate initial perspective
	camera.perspective = mat4_perspectiveRH(to_radians(camera.fov), initialAspectRatio, camera.near, camera.far);
	camera.perspective.data[1][1] *= -1.0f;

	// update initial view
	internal_cren_camera_update_view_matrix(&camera);

	return camera;
}

void cren_camera_update(CRenCamera* camera, double timestep) {
	if (!camera->shouldMove) return;

	float3 front = { 1.0f, 0.0f, 1.0f };
	front.x = (float)-d_cos(to_radians(camera->rotation.x)) * (float)d_sin(to_radians(camera->rotation.y));
	front.y = (float)d_sin(to_radians(camera->rotation.x));
	front.z = (float)d_cos(to_radians(camera->rotation.x)) * (float)d_cos(to_radians(camera->rotation.y));
	camera->frontPosition = float3_normalize(&front);

	float moveSpeed = (float)timestep * camera->movementSpeed * (camera->modifierPressed ? camera->modifierSpeed : 1.0f);

	if (camera->movingForward) {
		
		camera->position.x += (camera->frontPosition.x * moveSpeed);
		camera->position.y += (camera->frontPosition.y * moveSpeed);
		camera->position.z += (camera->frontPosition.z * moveSpeed);
	}

	if (camera->movingBackward) {
		camera->position.x -= (camera->frontPosition.x * moveSpeed);
		camera->position.y -= (camera->frontPosition.y * moveSpeed);
		camera->position.z -= (camera->frontPosition.z * moveSpeed);
	}

	if (camera->movingLeft) {
		const float3 dir = { 0.0f, 1.0f, 0.0f };
		float3 cross = float3_cross(camera->frontPosition, dir);
		float3 normalized = float3_normalize(&cross);
		camera->position.x -= normalized.x * moveSpeed;
		camera->position.y -= normalized.y * moveSpeed;
		camera->position.z -= normalized.z * moveSpeed;
	}

	if (camera->movingRight) {
		const float3 dir = { 0.0f, 1.0f, 0.0f };
		float3 cross = float3_cross(camera->frontPosition, dir);
		float3 normalized = float3_normalize(&cross);
		camera->position.x += normalized.x * moveSpeed;
		camera->position.y += normalized.y * moveSpeed;
		camera->position.z += normalized.z * moveSpeed;
	}

	internal_cren_camera_update_view_matrix(camera);

	camera->frontPosition.x *= -1.0f;
	camera->frontPosition.y *=  1.0f;
	camera->frontPosition.z *= -1.0f;
}

void cren_camera_set_aspect_ratio(CRenCamera* camera, float aspect) {
	camera->perspective = mat4_perspectiveRH(to_radians(camera->fov), aspect, camera->near, camera->far);

	if (camera->flipY) {
		camera->perspective.data[1][1] *= -1.0f;
	}

	camera->aspectRatio = aspect;
}

void cren_camera_translate(CRenCamera* camera, float3 deltaDir) {
	camera->position.x += deltaDir.x;
	camera->position.y += deltaDir.y;
	camera->position.z += deltaDir.z;
	internal_cren_camera_update_view_matrix(camera);
}

void cren_camera_rotate(CRenCamera* camera, float3 deltaDir) {
	camera->rotation.x += deltaDir.x;
	camera->rotation.y += deltaDir.y;
	camera->rotation.z += deltaDir.z;
	internal_cren_camera_update_view_matrix(camera);
}
