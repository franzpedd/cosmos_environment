#ifndef CREN_MATH_INCLUDED
#define CREN_MATH_INCLUDED

#include "cren_defines.h"

#define EPSILON_ZERO 1e-6f
#define EPSILON_PI 3.14159265358979323846
#define EPSILON_FLT 1.192092896e-07F
#define EPSILON_INT_MIN (-2147483647 - 1)

typedef union {
	struct { float x, y; };
	struct { float u, v; };
	float data[2];
} float2;

typedef union {
	struct { float x, y, z; };
	struct { float r, g, b; };
	float data[3];
} float3;

typedef union {
	struct { float x, y, z, w; };
	struct { float r, g, b, a; };
	float data[4];
} float4;

typedef union {
	float data[2][2];
	struct { float m00, m01; float m10, m11; float m20, m21; float m30, m31; };
	struct { float2 col0; float2 col1; };
} mat2;

typedef union {
	float data[3][3];
	struct { float m00, m01, m02; float m10, m11, m12; float m20, m21, m22; float m30, m31, m32; };
	struct { float3 col0; float3 col1; float3 col2; };
} mat3;

typedef union {
	float data[4][4];
	struct { float m00, m01, m02, m03; float m10, m11, m12, m13; float m20, m21, m22, m23; float m30, m31, m32, m33; };
	struct { float4 col0; float4 col1; float4 col2; float4 col3; };
} mat4;

typedef union {
	float data[4];
	struct { float x, y, z, w; };
} quat;

#ifdef __cplusplus 
extern "C" {
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// float3 operations
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief checks if given two float2 are equals
/// @param a first float3
/// @param b second float3
/// @return 1 if they are equal, 0 otherwise
CREN_API int float2_equal(float2* a, float2* b);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// float3 operations
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief checks if given two float3 are equals
/// @param a first float3
/// @param b second float3
/// @return 1 if they are equal, 0 otherwise
CREN_API int float3_equal(float3* a, float3* b);

/// @brief adds two float3 together
/// @param f0 first float3
/// @param f1 second float3
/// @return a new float3 with the result
CREN_API float3 float3_add(float3 f0, float3 f1);

/// @brief subtract f1 from f0
/// @param f0 to be subtracted
/// @param f1 to subtract
/// @return a new float3 with the result
CREN_API float3 float3_sub(float3 f0, float3 f1);

/// @brief multiplies f1 and f0 together
/// @param f0 left-side multiplicator
/// @param f1 right-side multiplicator
/// @return a new float3 with the result
CREN_API float3 float3_mul(float3 f0, float3 f1);

/// @brief multiplies a float3 by a scalar
/// @param f float3 object
/// @param s scalar
/// @return the scaled float3
CREN_API float3 float3_scalar(float3 f, float s);

/// @brief performs the cross product of two float3
/// @param f0 left-side float3
/// @param f1 right-side float3
/// @return the final float3 result
CREN_API float3 float3_cross(float3 f0, float3 f1);

/// @brief normalizes the float3
/// @param f the float3 to be normalized
/// @return the normalized float3
CREN_API float3 float3_normalize(float3 f);

/// @brief gets the length of a float3
/// @param f the float3 to perform the calculation
/// @return the length
CREN_API float float3_length(float3 f);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// float4 operations
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief checks if given two float4 are equals
/// @param a first float3
/// @param b second float3
/// @return 1 if they are equal, 0 otherwise
CREN_API int float4_equal(const float4* a, const float4* b);

/// @brief adds two float4 vectors
/// @param left-side vector
/// @param right-side vector
/// @return the added vector
CREN_API float4 float4_add(float4 f0, float4 f1);

/// @brief subtract f1 from f0
/// @param f0 to be subtracted
/// @param f1 to subtract
/// @return a new float4 with the result
CREN_API float4 float4_sub(float4 f0, float4 f1);

/// @brief multiplies f1 and f0 together
/// @param f0 left-side multiplicator
/// @param f1 right-side multiplicator
/// @return a new float4 with the result
CREN_API float4 float4_mul(float4 f0, float4 f1);

/// @brief multiplies a float4 by a scalar
/// @param f float4 object
/// @param s scalar
/// @return the scaled float4
CREN_API float4 float4_scalar(float4 f, float s);

/// @brief makes a float4 negative
/// @param the float4 to be negated
/// @return the negative float4
CREN_API float4 float4_neg(float4 f);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// mat4 operations
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief performs matrix multiplication
/// @param m0 left-matrix
/// @param m1 right-matrix
/// @return the multiplied matrix
CREN_API mat4 mat4_mul(mat4 m0, mat4 m1);

/// @brief creates a mat4 with ones filled
/// @return a one-filled matrix
CREN_API mat4 mat4_onefied();

/// @brief creates an identity matrix
/// @return the identity matrix object
CREN_API mat4 mat4_identity();

/// @brief calculates a projection/perspective matrix
/// @param fov camera's field of view
/// @param aspect camera's aspect ratio
/// @param near the near plane
/// @param far the far plane
/// @param clipSpace 0:Vulkan, 1:OpenGL
CREN_API mat4 mat4_perspectiveRH(float fov, float aspect, float near, float far, int clipSpace);

/// @brief rotates a matrix-axis by an angle
/// @param mat the matrix to rotate
/// @brief angle of the rotation to be appyed
/// @brief axis wich axis to perform the rotation
CREN_API mat4 mat4_rotate(mat4 mat, float angleRadians, float3 axis);

/// @brief translates the matrix towards a direction
/// @param the matrix to apply the translation
/// @param dir direction to translate towards
CREN_API mat4 mat4_translate(mat4 mat, float3 dir);

/// @brief scales the matrix by a dimension
/// @param m the matrix to be scaled
/// @param dim 3d dimension vector
CREN_API mat4 mat4_scale(mat4 m, float3 dim);

/// @brief calculates the inverse matrix
/// @param m the base matrix
/// @returns the m matrix inversed
CREN_API mat4 mat4_inverse(mat4 m);

/// @brief returns the value ptr underneath the struct
CREN_API float* mat4_value_ptr(mat4* m);

/// @brief generates a matrix based on a quaternion
CREN_API mat4 mat4_from_quat(quat q);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// quat operations
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief generates a quaternion given euler angles
/// @param f pitch, yay and roll angles
/// @return quaternion 
CREN_API quat quat_from_euler(float3 f);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// utils
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief turns degrees into radians
/// @param degrees the angle to be transform
/// @return the radians equivalent
CREN_API float to_radians(float degrees);

/// @brief calculates the cosine of a degree angle
/// @param degree the angle
/// return the cosine value
CREN_API float f_cos(float degree);

/// @brief calculates the sine of a degree angle
/// @param degree the angle
/// return the sine value
CREN_API float f_sin(float degree);

/// @brief calculates the power b of an exponetial e
/// @param b the base
/// @param e the exponential
/// @return the value
CREN_API double d_power(double b, int e);

/// @brief rounds a number down to the nearest integer 
/// @param num number to evaluate
/// @return the rounded number
CREN_API double d_floor(double num);

/// @brief returns the base 2 logarithm of a number
/// @param num number to evaluate
/// @return the base 2 logarithm of num
CREN_API double d_log2(double num);

/// @brief returns the max value given two numbers
/// @param x value
/// @param y value
/// @return the higher value
CREN_API float f_max(const float x, const float y);

/// @brief returns the max value given two numbers
/// @param x value
/// @param y value
/// @return the lower value
CREN_API float f_min(const float x, const float y);

/// @brief returns the max value given two numbers
/// @param x value
/// @param y value
/// @return the higher value
CREN_API unsigned int uint_max(const unsigned int x, const unsigned int y);

/// @brief returns the min value given two numbers
/// @param x value
/// @param y value
/// @return the lower value
CREN_API unsigned int uint_min(const unsigned int x, const unsigned int y);

/// @brief if value x is within the regions returns it, otherwise returns the closest
/// @param x value to be checked
/// @param upper min boundary
/// @param lower max boundary
/// @return the value or it's closest boundarie
CREN_API unsigned int uint_clamp(const unsigned int x, const unsigned int upper, const unsigned int lower);

/// @brief returns the max value given two numbers
/// @param x value
/// @param y value
/// @return the higher value
CREN_API int int_max(const int x, const int y);

/// @brief returns the min value given two numbers
/// @param x value
/// @param y value
/// @return the lower value
CREN_API int int_min(const int x, const int y);

/// @brief if value x is within the regions returns it, otherwise returns the closest
/// @param x value to be checked
/// @param upper min boundary
/// @param lower max boundary
/// @return the value or it's closest boundarie
CREN_API int int_clamp(const int x, const int upper, const int lower);

#ifdef __cplusplus 
}
#endif

#endif // CREN_MATH_INCLUDED
