#ifndef CREN_CAMERA_INCLUDED
#define CREN_CAMERA_INCLUDED

#include "cren_defines.h"
#include "cren_math.h"

/// @brief all types of camera supported
typedef enum {
    CAMERA_TYPE_LOOK_AT = 0,
    CAMERA_TYPE_FREE_LOOK
} CameraType;

/// @brief main camera data
typedef struct {
    CameraType type;
    int flipY;

    float fov;
	float near;
	float far;
	float aspectRatio;
	float movementSpeed;
	float rotationSpeed;
	float modifierSpeed;

	mat4 perspective;
	mat4 view;
	float3 rotation;
	float3 position;
	float3 scale;
	float3 viewPosition;
	float3 frontPosition;

	// movement
	int shouldMove;
	int modifierPressed;
	int movingForward;
	int movingBackward;
	int movingLeft;
	int movingRight;
} CRenCamera;

#ifdef __cplusplus 
extern "C" {
#endif

/// @brief creates and returns a camera
/// @param type the viewing type of the camera, currently only look-at or free-look
/// @param initialAspectRatio initial camera's aspect ratio
/// @return the camera itself
CREN_API CRenCamera cren_camera_create(CameraType type, float initialAspectRatio);

/// @brief updates the camera frame
/// @param camera cren's camera memory address
/// @param timestep interpolation between frames
CREN_API void cren_camera_update(CRenCamera* camera, double timestep);

/// @brief sets a new aspect ratio for the camera
/// @param camera camera's memory address
/// @param aspect new aspect ratio
CREN_API void cren_camera_set_aspect_ratio(CRenCamera* camera, float aspect);

/// @brief applys a translation to the camera's view, moving the camera
/// @param camera camera's memory address
/// @param deltaDir the delta vector to move the camera towards to
CREN_API void cren_camera_translate(CRenCamera* camera, float3 deltaDir);

/// @brief applys a rotation to the camera's view, rotating the camera
/// @param camera camera's memory address
/// @param deltaDir the delta vector to rotate the camera towards for
CREN_API void cren_camera_rotate(CRenCamera* camera, float3 deltaDir);

#ifdef __cplusplus 
}
#endif

#endif // CREN_CAMERA_INCLUDED