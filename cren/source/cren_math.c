#include "cren_math.h"

#include <math.h>

int float2_equal(float2* a, float2* b) {
    return fabsf(a->x - b->x) < EPSILON_ZERO && fabsf(a->y - b->y) < EPSILON_ZERO;
}

int float3_equal(float3* a, float3* b) {
    return fabsf(a->x - b->x) < EPSILON_ZERO && fabsf(a->y - b->y) < EPSILON_ZERO && fabsf(a->z - b->z) < EPSILON_ZERO;
}

float3 float3_add(float3 f0, float3 f1) {
    return (float3) { f0.x + f1.x, f0.y + f1.y, f0.z + f1.z };
}

float3 float3_sub(float3 f0, float3 f1) {
    return (float3) { f0.x - f1.x, f0.y - f1.y, f0.z - f1.z };
}

float3 float3_mul(float3 f0, float3 f1) {
    return (float3) { f0.x * f1.x, f0.y * f1.y, f0.z * f1.z };
}

float3 float3_scalar(float3 f, float s) {
    return  (float3) { f.x * s, f.y * s, f.z * s };
}

float3 float3_cross(float3 f0, float3 f1) {
    float3 result = { 0 };
    result.x = f0.y * f1.z - f0.z * f1.y;
    result.y = f0.z * f1.x - f0.x * f1.z;
    result.z = f0.x * f1.y - f0.y * f1.x;
    return result;
}

float3 float3_normalize(float3 f) {
    float length_squared = (f.x * f.x) + (f.y * f.y) + (f.z * f.z);

    if (length_squared == 0.0f) return (float3) { 0.0f, 0.0f, 0.0f };

    float length = sqrtf(length_squared);

    return (float3) { f.x / length, f.y / length,  f.z / length };
}

float float3_length(float3 f) {
    return sqrtf(f.x * f.x + f.y * f.y + f.z * f.z);
}

int float4_equal(const float4* a, const float4* b) {
    return fabsf(a->x - b->x) < EPSILON_ZERO && fabsf(a->y - b->y) < EPSILON_ZERO && fabsf(a->z - b->z) < EPSILON_ZERO && fabsf(a->w - b->w) < EPSILON_ZERO;
}

float4 float4_add(float4 f0, float4 f1) {
    return  (float4) { f0.x + f1.x, f0.y + f1.y, f0.z + f1.z, f0.w + f1.w };
}

float4 float4_sub(float4 f0, float4 f1) {
    return (float4) { f0.x - f1.x, f0.y - f1.y, f0.z - f1.z, f0.w - f1.w };
}

float4 float4_mul(float4 f0, float4 f1) {
    return (float4) { f0.x * f1.x, f0.y * f1.y, f0.z * f1.z, f0.w * f1.w };
}

float4 float4_scalar(float4 f, float s) {
    return (float4) { f.x * s, f.y * s, f.z * s, f.w * s };
}

float4 float4_neg(float4 f) {
    return (float4) { -f.x, -f.y, -f.z, -f.w };
}

mat4 mat4_mul(mat4 m0, mat4 m1) {
    mat4 res;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            res.data[i][j] = 0;
            for (int k = 0; k < 4; k++) {
                res.data[i][j] += m0.data[i][k] * m1.data[k][j];
            }
        }
    }
    return res;
}

mat4 mat4_onefied() {
    mat4 result = { 0 };
    result.col0 = (float4) { 1.0f, 1.0f, 1.0f, 1.0f };
    result.col1 = (float4) { 1.0f, 1.0f, 1.0f, 1.0f };
    result.col2 = (float4) { 1.0f, 1.0f, 1.0f, 1.0f };
    result.col3 = (float4) { 1.0f, 1.0f, 1.0f, 1.0f };
    return result;
}

mat4 mat4_identity() {
    mat4 result = { 0 };
    result.col0 = (float4) { 1.0f, 0.0f, 0.0f, 0.0f };
    result.col1 = (float4) { 0.0f, 1.0f, 0.0f, 0.0f };
    result.col2 = (float4) { 0.0f, 0.0f, 1.0f, 0.0f };
    result.col3 = (float4) { 0.0f, 0.0f, 0.0f, 1.0f };
    return result;
}

mat4 mat4_perspectiveRH(float fov, float aspect, float near, float far, int clipSpace) {
    mat4 result = { 0 };
    float tanHalfFovY = tanf(fov / 2.0f);

    // X and Y scaling (same for both conventions)
    result.data[0][0] = 1.0f / (aspect * tanHalfFovY);
    result.data[1][1] = 1.0f / tanHalfFovY;
    result.data[2][3] = -1.0f;  // Homogeneous divide

    // Adjust depth scaling/translation based on clip space
    if (clipSpace == 0) { // Vulkan: depth [0, 1]
        result.data[2][2] = far / (near - far);	// Maps zNear -> 0, zFar -> 1
        result.data[3][2] = (far * near) / (near - far); // Translation
    }
    else { // OpenGL: depth [-1, 1]
        result.data[2][2] = -(far + near) / (far - near); // Maps zNear -> -1, zFar -> 1
        result.data[3][2] = -(2.0f * far * near) / (far - near); // Translation
    }

    return result;
}

mat4 mat4_rotate(mat4 m, float angle, float3 axis) {
    // 1. Normalize axis and compute temp values
    float3 axis_n = float3_normalize(axis);
    float c = cosf(angle);
    float s = sinf(angle);
    float3 temp = float3_scalar(axis_n, (1.0f - c));

    // 2. Construct rotation matrix (column-major)
    mat4 Rotate = { 0 };
    Rotate.data[0][0] = c + temp.x * axis_n.x;
    Rotate.data[0][1] = temp.x * axis_n.y + s * axis_n.z;
    Rotate.data[0][2] = temp.x * axis_n.z - s * axis_n.y;

    Rotate.data[1][0] = temp.y * axis_n.x - s * axis_n.z;
    Rotate.data[1][1] = c + temp.y * axis_n.y;
    Rotate.data[1][2] = temp.y * axis_n.z + s * axis_n.x;

    Rotate.data[2][0] = temp.z * axis_n.x + s * axis_n.y;
    Rotate.data[2][1] = temp.z * axis_n.y - s * axis_n.x;
    Rotate.data[2][2] = c + temp.z * axis_n.z;
    Rotate.data[3][3] = 1.0f;

    // 3. Multiply with input matrix (GLM's column-major multiply)
    mat4 Result;
    Result.col0 = float4_add(
        float4_add(
            float4_scalar(m.col0, Rotate.data[0][0]),
            float4_scalar(m.col1, Rotate.data[0][1])),
        float4_scalar(m.col2, Rotate.data[0][2]));

    Result.col1 = float4_add(
        float4_add(
            float4_scalar(m.col0, Rotate.data[1][0]),
            float4_scalar(m.col1, Rotate.data[1][1])),
        float4_scalar(m.col2, Rotate.data[1][2]));

    Result.col2 = float4_add(
        float4_add(
            float4_scalar(m.col0, Rotate.data[2][0]),
            float4_scalar(m.col1, Rotate.data[2][1])),
        float4_scalar(m.col2, Rotate.data[2][2]));

    Result.col3 = m.col3;
    return Result;
}

mat4 mat4_translate(mat4 mat, float3 dir) {
    mat4 result = mat;

    result.data[3][0] += dir.x;
    result.data[3][1] += dir.y;
    result.data[3][2] += dir.z;
    return result;
}

mat4 mat4_scale(mat4 m, float3 dim) {
    mat4 result;

    // Scale the first 3 columns (X, Y, Z axes)
    for (int i = 0; i < 4; ++i) {
        result.data[i][0] = m.data[i][0] * dim.x;  // Scale X column
        result.data[i][1] = m.data[i][1] * dim.y;  // Scale Y column
        result.data[i][2] = m.data[i][2] * dim.z;  // Scale Z column
        result.data[i][3] = m.data[i][3];           // Copy W column (no scaling)
    }

    return result;
}

mat4 mat4_inverse(mat4 m) {
    float coef00 = m.col2.z * m.col3.w - m.col3.z * m.col2.w;
    float coef02 = m.col1.z * m.col3.w - m.col3.z * m.col1.w;
    float coef03 = m.col1.z * m.col2.w - m.col2.z * m.col1.w;

    float coef04 = m.col2.y * m.col3.w - m.col3.y * m.col2.w;
    float coef06 = m.col1.y * m.col3.w - m.col3.y * m.col1.w;
    float coef07 = m.col1.y * m.col2.w - m.col2.y * m.col1.w;

    float coef08 = m.col2.y * m.col3.z - m.col3.y * m.col2.z;
    float coef10 = m.col1.y * m.col3.z - m.col3.y * m.col1.z;
    float coef11 = m.col1.y * m.col2.z - m.col2.y * m.col1.z;

    float coef12 = m.col2.x * m.col3.w - m.col3.x * m.col2.w;
    float coef14 = m.col1.x * m.col3.w - m.col3.x * m.col1.w;
    float coef15 = m.col1.x * m.col2.w - m.col2.x * m.col1.w;

    float coef16 = m.col2.x * m.col3.z - m.col3.x * m.col2.z;
    float coef18 = m.col1.x * m.col3.z - m.col3.x * m.col1.z;
    float coef19 = m.col1.x * m.col2.z - m.col2.x * m.col1.z;

    float coef20 = m.col2.x * m.col3.y - m.col3.x * m.col2.y;
    float coef22 = m.col1.x * m.col3.y - m.col3.x * m.col1.y;
    float coef23 = m.col1.x * m.col2.y - m.col2.x * m.col1.y;

    float4 fac0 = { coef00, coef00, coef02, coef03 };
    float4 fac1 = { coef04, coef04, coef06, coef07 };
    float4 fac2 = { coef08, coef08, coef10, coef11 };
    float4 fac3 = { coef12, coef12, coef14, coef15 };
    float4 fac4 = { coef16, coef16, coef18, coef19 };
    float4 fac5 = { coef20, coef20, coef22, coef23 };

    float4 vec0 = { m.col1.x, m.col0.x, m.col0.x, m.col0.x };
    float4 vec1 = { m.col1.y, m.col0.y, m.col0.y, m.col0.y };
    float4 vec2 = { m.col1.z, m.col0.z, m.col0.z, m.col0.z };
    float4 vec3 = { m.col1.w, m.col0.w, m.col0.w, m.col0.w };

    float4 inv0 = { fac1.x * vec2.x - fac2.x * vec3.x,
                 fac1.y * vec2.y - fac2.y * vec3.y,
                 fac1.z * vec2.z - fac2.z * vec3.z,
                 fac1.w * vec2.w - fac2.w * vec3.w };
    float4 inv1 = { fac0.x * vec2.x - fac3.x * vec3.x,
                 fac0.y * vec2.y - fac3.y * vec3.y,
                 fac0.z * vec2.z - fac3.z * vec3.z,
                 fac0.w * vec2.w - fac3.w * vec3.w };
    float4 inv2 = { fac0.x * vec1.x - fac4.x * vec3.x,
                 fac0.y * vec1.y - fac4.y * vec3.y,
                 fac0.z * vec1.z - fac4.z * vec3.z,
                 fac0.w * vec1.w - fac4.w * vec3.w };
    float4 inv3 = { fac0.x * vec0.x - fac5.x * vec3.x,
                 fac0.y * vec0.y - fac5.y * vec3.y,
                 fac0.z * vec0.z - fac5.z * vec3.z,
                 fac0.w * vec0.w - fac5.w * vec3.w };

    float det = m.col0.x * inv0.x - m.col1.x * inv0.y + m.col2.x * inv0.z - m.col3.x * inv0.w;
    if (fabsf(det) <= EPSILON_FLT) {
        return mat4_identity();  // return zero matrix on failure (GLM behavior)
    }

    float inv_det = 1.0f / det;
    mat4 result;
    result.col0 = (float4) { inv0.x * inv_det, -inv0.y * inv_det,  inv0.z * inv_det, -inv0.w * inv_det };
    result.col1 = (float4) { -inv1.x * inv_det,  inv1.y * inv_det, -inv1.z * inv_det,  inv1.w * inv_det };
    result.col2 = (float4) { inv2.x * inv_det, -inv2.y * inv_det,  inv2.z * inv_det, -inv2.w * inv_det };
    result.col3 = (float4) { -inv3.x * inv_det,  inv3.y * inv_det, -inv3.z * inv_det,  inv3.w * inv_det };
    return result;
}

float* mat4_value_ptr(mat4* m) {
    return &(m->data[0][0]); // equivalent to &(m->m00)
}

mat4 mat4_from_quat(quat q) {
    float x = q.x, y = q.y, z = q.z, w = q.w;
    float x2 = x + x, y2 = y + y, z2 = z + z;
    float xx = x * x2, xy = x * y2, xz = x * z2;
    float yy = y * y2, yz = y * z2, zz = z * z2;
    float wx = w * x2, wy = w * y2, wz = w * z2;

    mat4 m = mat4_identity();
    m.m00 = 1.0f - (yy + zz);
    m.m01 = xy + wz;
    m.m02 = xz - wy;
    m.m10 = xy - wz;
    m.m11 = 1.0f - (xx + zz);
    m.m12 = yz + wx;
    m.m20 = xz + wy;
    m.m21 = yz - wx;
    m.m22 = 1.0f - (xx + yy);
    return m;
}

quat quat_from_euler(float3 f) {
    float cy = cosf(f.y * 0.5f);
    float sy = sinf(f.y * 0.5f);
    float cp = cosf(f.x * 0.5f);
    float sp = sinf(f.x * 0.5f);
    float cr = cosf(f.z * 0.5f);
    float sr = sinf(f.z * 0.5f);

    quat q;
    q.w = cr * cp * cy + sr * sp * sy;
    q.x = sr * cp * cy - cr * sp * sy;
    q.y = cr * sp * cy + sr * cp * sy;
    q.z = cr * cp * sy - sr * sp * cy;
    return q;
}

float to_radians(float degrees) {
    return (float)(degrees * (EPSILON_PI / 180.0f));
}

float f_cos(float degree) {
    return cosf(degree);
}

float f_sin(float degree) {
    return sinf(degree);
}

CREN_API double d_power(double b, int e) {

    if (e == 0) return 1;
    if (e < 0) {
        if (e == EPSILON_INT_MIN) { // avoid overflow
            return 1 / (b * d_power(b, -(e + 1)));
        }

        return 1 / d_power(b, -e);
    }

    double temp = d_power(b, e / 2);
    return (e % 2 == 0) ? temp * temp : b * temp * temp;
}

double d_floor(double num) {
    return floor(num);
}

double d_log2(double num) {
    return log2(num);
}

float f_max(const float x, const float y) {
    return (x < y) ? y : x;
}

float f_min(const float x, const float y) {
    return (y < x) ? y : x;
}

unsigned int uint_max(const unsigned int x, const unsigned int y) {
    return (x < y) ? y : x;
}

unsigned int uint_min(const unsigned int x, const unsigned int y) {
    return (y < x) ? y : x;
}

unsigned int uint_clamp(const unsigned int x, const unsigned int upper, const unsigned int lower) {
    return uint_min(upper, uint_max(x, lower));
}

int int_max(const int x, const int y) {
    return (x < y) ? y : x;
}

int int_min(const int x, const int y) {
    return (y < x) ? y : x;
}

int int_clamp(const int x, const int upper, const int lower) {
    return int_min(upper, int_max(x, lower));
}