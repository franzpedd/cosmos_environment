#include "cren_camera.h"

/// @brief updates the view projection matrix of a camera
/// @param camera pointer to the camera
static void internal_cren_camera_update_view_matrix(CRenCamera* camera) {

	// rotation
	mat4 rmat = mat4_identity();
	rmat = mat4_rotate(rmat, to_radians(camera->rotation.x * (camera->flipY ? -1.0f : 1.0f)), (float3){ 1.0f, 0.0f, 0.0f });
	rmat = mat4_rotate(rmat, to_radians(camera->rotation.y), (float3){ 0.0f, 1.0f, 0.0f });
	rmat = mat4_rotate(rmat, to_radians(camera->rotation.z), (float3){ 0.0f, 0.0f, 1.0f });

	// translation
	float3 translation = { camera->position.x, camera->position.y, camera->position.z };
	if (camera->flipY) translation.y *= -1.0f;
	mat4 tmat = mat4_identity();
	tmat = mat4_translate(mat4_identity(), translation);

	if (camera->type == CAMERA_TYPE_FREE_LOOK) {
		camera->view = mat4_mul(tmat, rmat); // my mat4 is reversed, yucks
	}

	else {
		camera->view = mat4_mul(rmat, tmat);
	}

	camera->viewPosition = float3_mul(camera->position, (float3){ -1.0f, 1.0f, -1.0f });
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

	camera.perspective = mat4_identity();
	camera.view = mat4_identity();
	camera.rotation = (float3){ 0.0f, 0.0f, 0.0f };
	camera.position = (float3){ 0.0f, 1.0f, 0.0f };
	camera.scale = (float3){ 1.0f, 1.0f, 1.0f };
	camera.viewPosition = (float3){ 0.0f, 0.0f, 0.0f };
	camera.frontPosition = (float3){ 0.0f, 0.0f, -1.0f };

	// calculate initial perspective
	camera.perspective = mat4_perspectiveRH(to_radians(camera.fov), initialAspectRatio, camera.near, camera.far, 0);

	// update initial view
	internal_cren_camera_update_view_matrix(&camera);

	return camera;
}

void cren_camera_update(CRenCamera* camera, double timestep) {
	if (!camera->shouldMove) return;

	camera->frontPosition = (float3){ 1.0f, 0.0f, 1.0f };
	camera->frontPosition.x = -f_cos(to_radians(camera->rotation.x)) * f_sin(to_radians(camera->rotation.y));
	camera->frontPosition.y = f_sin(to_radians(camera->rotation.x));
	camera->frontPosition.z = f_cos(to_radians(camera->rotation.x)) * f_cos(to_radians(camera->rotation.y));

	float3 pos = float3_normalize(camera->frontPosition);
	camera->frontPosition = pos;

	float moveSpeed = timestep * camera->movementSpeed * (camera->modifierPressed ? camera->modifierSpeed : 1.0f);
	float3 cross = float3_cross(camera->frontPosition, (float3){ 0.0f, 1.0f, 0.0f });
	float3 normalized = float3_mul(float3_normalize(cross), (float3){ moveSpeed, moveSpeed, moveSpeed });

	if (camera->movingForward) {
		camera->position = float3_add(camera->position, float3_mul(camera->frontPosition, (float3) { moveSpeed, moveSpeed, moveSpeed }));
	}

	if (camera->movingBackward) {
		camera->position = float3_sub(camera->position, float3_mul(camera->frontPosition, (float3) { moveSpeed, moveSpeed, moveSpeed }));
	}

	if (camera->movingLeft) {
		camera->position = float3_sub(camera->position, normalized);
	}

	if (camera->movingRight) {
		camera->position = float3_add(camera->position, normalized);
	}

	internal_cren_camera_update_view_matrix(camera);
	camera->frontPosition = float3_mul(camera->frontPosition, (float3){ -1.0f, 1.0f, -1.0f });
}

void cren_camera_set_aspect_ratio(CRenCamera* camera, float aspect) {
	camera->perspective = mat4_perspectiveRH(to_radians(camera->fov), aspect, camera->near, camera->far, 0);
	camera->aspectRatio = aspect;
}

void cren_camera_translate(CRenCamera* camera, float3 deltaDir) {
	camera->position = float3_add(camera->position, deltaDir);
	internal_cren_camera_update_view_matrix(camera);
}

void cren_camera_rotate(CRenCamera* camera, float3 deltaDir) {
	camera->rotation = float3_add(camera->rotation, deltaDir);
	internal_cren_camera_update_view_matrix(camera);
}