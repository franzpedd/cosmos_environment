#ifndef CREN_MATH_INCLUDED
#define CREN_MATH_INCLUDED

#define EPSILON_ZERO 1e-6f
#define EPSILON_PI 3.14159265358979323846

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
} Quat;

#ifdef __cplusplus 
extern "C" {
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Addition operation
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief adds two float2 together
/// @param f0 first float2
/// @param f1 second float2
/// @return a new float2 with the result
float2 float2_add(const float2* f0, const float2* f1);

/// @brief adds two float3 together
/// @param f0 first float3
/// @param f1 second float3
/// @return a new float3 with the result
float3 float3_add(const float3* f0, const float3* f1);

/// @brief adds two float4 together
/// @param f0 first float4
/// @param f1 second float4
/// @return a new float4 with the result
float4 float4_add(const float4* f0, const float4* f1);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Subtraction operation
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief subtract f1 from f0
/// @param f0 to be subtracted
/// @param f1 to subtract
/// @return a new float2 with the result
float2 float2_sub(const float2* f0, const float2* f1);

/// @brief subtract f1 from f0
/// @param f0 to be subtracted
/// @param f1 to subtract
/// @return a new float3 with the result
float3 float3_sub(const float3* f0, const float3* f1);

/// @brief subtract f1 from f0
/// @param f0 to be subtracted
/// @param f1 to subtract
/// @return a new float4 with the result
float4 float4_sub(const float4* f0, const float4* f1);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Multiplication operation
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief multiplies f1 and f0 together
/// @param f0 left-side multiplicator
/// @param f1 right-side multiplicator
/// @return a new float2 with the result
float2 float2_mul(const float2* f0, const float2* f1);

/// @brief multiplies f1 and f0 together
/// @param f0 left-side multiplicator
/// @param f1 right-side multiplicator
/// @return a new float3 with the result
float3 float3_mul(const float3* f0, const float3* f1);

/// @brief multiplies f1 and f0 together
/// @param f0 left-side multiplicator
/// @param f1 right-side multiplicator
/// @return a new float4 with the result
float4 float4_mul(const float4* f0, const float4* f1);

/// @brief multiplies two mat4 structures
/// @param m0 right-side multiplicator
/// @param m1 left-side multiplicator
/// @return the multiplied matrix
mat4 mat4_mul(const mat4* m0, const mat4* m1);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Division operation
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief divides f1 from f0
/// @param f0 the dividend
/// @param f1 the divisor
/// @return a new float2 with the result
float2 float2_div(const float2* f0, const float2* f1);

/// @brief divides f1 from f0
/// @param f0 the dividend
/// @param f1 the divisor
/// @return a new float3 with the result
float3 float3_div(const float3* f0, const float3* f1);

/// @brief divides f1 from f0
/// @param f0 the dividend
/// @param f1 the divisor
/// @return a new float4 with the result
float4 float4_div(const float4* f0, const float4* f1);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Equality check
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief checks if given two float2 are equals
/// @param a first float3
/// @param b second float3
/// @return 1 if they are equal, 0 otherwise
int float2_equal(const float2* a, const float2* b);

/// @brief checks if given two float3 are equals
/// @param a first float3
/// @param b second float3
/// @return 1 if they are equal, 0 otherwise
int float3_equal(const float3* a, const float3* b);

/// @brief checks if given two float4 are equals
/// @param a first float3
/// @param b second float3
/// @return 1 if they are equal, 0 otherwise
int float4_equal(const float4* a, const float4* b);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Identity
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief returns the identity matrix for a mat2
/// @return mat2 identity matrix
mat2 mat2_identity();

/// @brief returns the identity matrix for a mat3
/// @return mat3 identity matrix
mat3 mat3_identity();

/// @brief returns the identity matrix for a mat4
/// @return mat4 identity matrix
mat4 mat4_identity();

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Generic Utils
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief rounds a number down to the nearest integer 
/// @param num number to evaluate
/// @return the rounded number
double d_floor(double num);

/// @brief returns the base 2 logarithm of a number
/// @param num number to evaluate
/// @return the base 2 logarithm of num
double d_log2(double num);

/// @brief returns the sin of a radians number
/// @return the sin value
double d_sin(double num);

/// @brief returns the cos of a radians number
/// @return the cos value
double d_cos(double num);

/// @brief checks witch double is the lowwer in value
double d_min(double a, double b);

/// @brief fast inverse square root using newton's method
/// @param number the number to be inverse squared
/// @return the inverse squared number
float fast_inverse_sqrt(float number);

/// @brief turns degrees into radians
/// @param degrees the angle to be transform
/// @return the radians equivalent
float to_radians(float degrees);

/// @brief returns the max value given two numbers
/// @param x value
/// @param y value
/// @return the higher value
unsigned int uint_max(const unsigned int x, const unsigned int y);

/// @brief returns the min value given two numbers
/// @param x value
/// @param y value
/// @return the lower value
unsigned int uint_min(const unsigned int x, const unsigned int y);

/// @brief if value x is within the regions returns it, otherwise returns the closest
/// @param x value to be checked
/// @param upper min boundary
/// @param lower max boundary
/// @return the value or it's closest boundarie
unsigned int uint_clamp(const unsigned int x, const unsigned int upper, const unsigned int lower);

/// @brief returns the max value given two numbers
/// @param x value
/// @param y value
/// @return the higher value
int int_max(const int x, const int y);

/// @brief returns the min value given two numbers
/// @param x value
/// @param y value
/// @return the lower value
int int_min(const int x, const int y);

/// @brief if value x is within the regions returns it, otherwise returns the closest
/// @param x value to be checked
/// @param upper min boundary
/// @param lower max boundary
/// @return the value or it's closest boundarie
int int_clamp(const int x, const int upper, const int lower);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// float3 Utils
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief normalizes the float3
/// @param f the float3 to be normalized
/// @return the normalized float3
float3 float3_normalize(const float3* f);

/// @brief performs the cross product of two float3
/// @param f0 left-side float3
/// @param f1 right-side float3
/// @return the final float3 result
float3 float3_cross(float3 f0, float3 f1);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// mat4 Utils
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief performs a perspective/projection transformation using the right-hand rule
/// @param fovY field-of-view
/// @param aspect aspect ratio to the transformation
/// @param zNear nearest point
/// @param zFar farthes point
/// @return the transformed matrix
mat4 mat4_perspectiveRH(float fovY, float aspect, float zNear, float zFar);

/// @brief translates an initial mat4 to a x,y,z float3
/// @param initial matrix to be translated
/// @param x float3 coordinate
/// @param y float3 coordinate
/// @param z float3 coordinate
/// @return the translated matrix
mat4 mat4_translate(const mat4* initial, float x, float y, float z);

/// @brief applies a rotation to an initial mat4
/// @param initial matrix to be rotated
/// @param angle angles in degrees to rotate the matrix  
/// @param x float3 coordinate
/// @param y float3 coordinate
/// @param z float3 coordinate
/// @return the rotated matrix
mat4 mat4_rotate(const mat4* initial, float angle, float x, float y, float z);

/// @brief scales an initial mat4 by a x,y,z float3
/// @param initial matrix to be scaled
/// @param x float3 coordinate
/// @param y float3 coordinate
/// @param z float3 coordinate
/// @return the scaled matrix
mat4 mat4_scale(const mat4* initial, float x, float y, float z);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// quat Utils
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// @brief proper quaternion construction from euler angles
/// @param yaw Y euler angle
/// @param pitch X euler angle
/// @param roll Z euler angle
Quat yaw_pitch_roll(float yaw, float pitch, float roll);

/// @brief converts a quaternion to a mat4
/// @param q quaternion object
/// @param mat output mat4 object
void quat_to_mat4(const Quat* q, mat4* mat);

#ifdef __cplusplus 
}
#endif

#endif // CREN_MATH_INCLUDED