#include "cren_math.h"

#include <immintrin.h> // simd instructions
#include <math.h>

/// @brief stores the mat4 column into the simd register
/// @param m matrix address
/// @param col witch col index within the matrix to store
/// @param value to be stored into the register
/// @details internal
static void store_mat4_column(mat4* m, int col, __m128 value) {
    float* ptr = (float*)&value;
    switch (col) {
        case 0: m->m00 = ptr[0]; m->m10 = ptr[1]; m->m20 = ptr[2]; m->m30 = ptr[3]; break;
        case 1: m->m01 = ptr[0]; m->m11 = ptr[1]; m->m21 = ptr[2]; m->m31 = ptr[3]; break;
        case 2: m->m02 = ptr[0]; m->m12 = ptr[1]; m->m22 = ptr[2]; m->m32 = ptr[3]; break;
        case 3: m->m03 = ptr[0]; m->m13 = ptr[1]; m->m23 = ptr[2]; m->m33 = ptr[3]; break;
    }
}

float2 float2_add(const float2* f0, const float2* f1) {
    return (float2){ f0->x + f1->x, f0->y + f1->y };
}

float3 float3_add(const float3* f0, const float3* f1) {
    return (float3){ f0->x + f1->x, f0->y + f1->y, f0->z + f1->z }; 
}

float4 float4_add(const float4* f0, const float4* f1) {
    __m128 regA = _mm_loadu_ps(&f0->x);
    __m128 regB = _mm_loadu_ps(&f1->x);
    __m128 regRes = _mm_add_ps(regA, regB); // SIMD addition
    float res[4];
    _mm_storeu_ps(res, regRes);
    return (float4){ res[0], res[1], res[2], res[3] };
}

float2 float2_sub(const float2* f0, const float2* f1) {
    return (float2){ f0->x - f1->x, f0->y - f1->y };
}

float3 float3_sub(const float3* f0, const float3* f1) {
    return (float3){ f0->x - f1->x, f0->y - f1->y, f0->z - f1->z };
}

float4 float4_sub(const float4* f0, const float4* f1) {
    __m128 regA = _mm_loadu_ps(&f0->x);
    __m128 regB = _mm_loadu_ps(&f1->x);
    __m128 regRes = _mm_sub_ps(regA, regB); // SIMD subtraction
    float res[4];
    _mm_storeu_ps(res, regRes);
    return (float4){ res[0], res[1], res[2], res[3] };
}

float2 float2_mul(const float2* f0, const float2* f1) {
    return (float2){ f0->x * f1->x, f0->y * f1->y };
}

float3 float3_mul(const float3* f0, const float3* f1) {
    return (float3){ f0->x * f1->x,  f0->y * f1->y, f0->z * f1->z };
}

float4 float4_mul(const float4* f0, const float4* f1) {
    __m128 regA = _mm_loadu_ps(&f0->x);
    __m128 regB = _mm_loadu_ps(&f1->x);
    __m128 regRes = _mm_mul_ps(regA, regB); // SIMD multiplication
    float res[4];
    _mm_storeu_ps(res, regRes);
    return (float4){ res[0], res[1], res[2], res[3] };
}

mat4 mat4_mul(const mat4* m0, const mat4* m1) {
    mat4 result = { 0 };

    for (int i = 0; i < 4; i++) {
        __m128 row = _mm_loadu_ps(&m0->data[i][0]); // load a row from matrix A
        for (int j = 0; j < 4; j++) {
            __m128 col = _mm_set_ps(m1->data[3][j], m1->data[2][j], m1->data[1][j], m1->data[0][j]); // load a column from matrix B
            __m128 mul = _mm_mul_ps(row, col); // multiply row by column
            __m128 sum = _mm_hadd_ps(mul, mul); // horizontal add
            sum = _mm_hadd_ps(sum, sum);
            _mm_store_ss(&result.data[i][j], sum); // store the result
        }
    }

    return result;
}

float2 float2_div(const float2* f0, const float2* f1) {
    // check for division by zero
    if (fabsf(f1->x) < EPSILON_ZERO || fabsf(f1->y) < EPSILON_ZERO) return (float2){ NAN, NAN };
    return (float2){ f0->x / f1->x, f0->y / f1->y };
}

float3 float3_div(const float3* f0, const float3* f1) {
    // check for division by zero
    if (fabsf(f1->x) < EPSILON_ZERO || fabsf(f1->y) < EPSILON_ZERO || fabsf(f1->z) < EPSILON_ZERO) return (float3){ NAN, NAN, NAN };
    return (float3){f0->x / f1->x, f0->y / f1->y, f0->z / f1->z };
}

float4 float4_div(const float4* f0, const float4* f1) {
    // check for division by zero
    if (fabsf(f1->x) < EPSILON_ZERO || fabsf(f1->y) < EPSILON_ZERO || fabsf(f1->z) < EPSILON_ZERO || fabsf(f1->w) < EPSILON_ZERO) return (float4){ NAN, NAN, NAN, NAN };

    __m128 regA = _mm_loadu_ps(&f0->x);
    __m128 regB = _mm_loadu_ps(&f1->x);
    __m128 regRes = _mm_div_ps(regA, regB); // SIMD division
    float res[4];
    _mm_storeu_ps(res, regRes);
    return (float4){ res[0], res[1], res[2], res[3] };
}

int float2_equal(const float2* a, const float2* b) {
    return fabsf(a->x - b->x) < EPSILON_ZERO && fabsf(a->y - b->y) < EPSILON_ZERO;
}

int float3_equal(const float3* a, const float3* b) {
    return fabsf(a->x - b->x) < EPSILON_ZERO && fabsf(a->y - b->y) < EPSILON_ZERO && fabsf(a->z - b->z) < EPSILON_ZERO;
}

int float4_equal(const float4* a, const float4* b) {
    return fabsf(a->x - b->x) < EPSILON_ZERO && fabsf(a->y - b->y) < EPSILON_ZERO && fabsf(a->z - b->z) < EPSILON_ZERO && fabsf(a->w - b->w) < EPSILON_ZERO;
}

mat2 mat2_identity() {
    return (mat2){
        {
            { 1.0f, 0.0f },
            { 0.0f, 1.0f }
        }
    };
}

mat3 mat3_identity() {
    return (mat3){
        {
            { 1.0f, 0.0f, 0.0f },
            { 0.0f, 1.0f, 0.0f },
            { 0.0f, 0.0f, 1.0f }
        }
    };
}

mat4 mat4_identity() {
    return (mat4){ {
        {1.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 0.0f, 1.0f}
    } };
}

double d_floor(double num) {
    return floor(num);
}

double d_log2(double num) {
    return log2(num);
}

double d_sin(double num) {
    return sin(num);
}

double d_cos(double num) {
    return cos(num);
}

double d_min(double a, double b) {
    return (((a) < (b)) ? (a) : (b));
}

float fast_inverse_sqrt(float number) {
    const float threehalfs = 1.5f;
    float x2 = number * 0.5f;
    float y = number;

    // bit-level hacking
    unsigned int i = *(unsigned int*)&y;    // treat float's bits as an integer
    i = 0x5f3759df - (i >> 1);              // initial guess for newton's method
    y = *(float*)&i;                        // treat integer's bits as a float

    // one iteration of newton's method
    y = y * (threehalfs - (x2 * y * y));
    return y;
}

float to_radians(float degrees) {
    return (float)(degrees * (EPSILON_PI / 180.0f));
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

float3 float3_normalize(const float3* f) {
    float3 result = { 0 };

    // compute the squared magnitude
    float sqrMagnitude = f->x * f->x + f->y * f->y + f->z * f->z;

    // avoid division by zero
    if (sqrMagnitude > 0.0f) {

        // compute the inverse square root of the squared magnitude
        float invMagnitude = fast_inverse_sqrt(sqrMagnitude);
    
        // normalize the vector
        result.x = f->x * invMagnitude;
        result.y = f->y * invMagnitude;
        result.z = f->z * invMagnitude;
    }
    
    return result;
}


float3 float3_cross(float3 f0, float3 f1) {
    float3 result;

    // load the vectors into SSE registers
    __m128 a_vec = _mm_set_ps(0.0f, f0.z, f0.y, f0.x); // [0, a.z, a.y, a.x]
    __m128 b_vec = _mm_set_ps(0.0f, f1.z, f1.y, f1.x); // [0, b.z, b.y, b.x]

    // shuffle components for cross product calculation
    __m128 a_yzx = _mm_shuffle_ps(a_vec, a_vec, _MM_SHUFFLE(3, 0, 2, 1)); // [a.y, a.z, a.x, 0]
    __m128 b_zxy = _mm_shuffle_ps(b_vec, b_vec, _MM_SHUFFLE(3, 1, 0, 2)); // [b.z, b.x, b.y, 0]
    __m128 a_zxy = _mm_shuffle_ps(a_vec, a_vec, _MM_SHUFFLE(3, 1, 0, 2)); // [a.z, a.x, a.y, 0]
    __m128 b_yzx = _mm_shuffle_ps(b_vec, b_vec, _MM_SHUFFLE(3, 0, 2, 1)); // [b.y, b.z, b.x, 0]

    // perform the cross product calculation
    __m128 cross_result = _mm_sub_ps(_mm_mul_ps(a_yzx, b_zxy), _mm_mul_ps(a_zxy, b_yzx));

    // store the result back into the float3 union
    _mm_store_ps(result.data, cross_result);

    return result;
}

mat4 mat4_perspectiveRH(float fovY, float aspect, float zNear, float zFar) {
    mat4 result = { 0 };
    float tanHalfFovY = tanf(fovY / 2.0f);

    result.data[0][0] = 1.0f / (aspect * tanHalfFovY);
    result.data[1][1] = 1.0f / tanHalfFovY;
    result.data[2][2] = zFar / (zNear - zFar); // right-handed Z points into the screen
    result.data[2][3] = -1.0f;
    result.data[3][2] = (zFar * zNear) / (zNear - zFar);

    return result;
}

mat4 mat4_translate(const mat4* initial, float x, float y, float z) {
    mat4 translationMatrix = { {
        {1.0f, 0.0f, 0.0f, x},
        {0.0f, 1.0f, 0.0f, y},
        {0.0f, 0.0f, 1.0f, z},
        {0.0f, 0.0f, 0.0f, 1.0f}
    } };

    return mat4_mul(initial, &translationMatrix);
}

mat4 mat4_rotate(const mat4* initial, float angle, float x, float y, float z) {
    mat4 rotationMatrix = { 0 };

    float c = cosf(angle);
    float s = sinf(angle);
    float t = 1.0f - c;

    // normalize the axis
    float length = sqrtf(x * x + y * y + z * z);
    x /= length;
    y /= length;
    z /= length;

    // compute individual scalar components for the rotation matrix columns
    // column 0: [t*x² + c, t*x*y + s*z, t*x*z - s*y, 0]
    float col0_0 = t * x * x + c;
    float col0_1 = t * x * y + s * z;
    float col0_2 = t * x * z - s * y;
    float col0_3 = 0.0f;

    // column 1: [t*x*y - s*z, t*y² + c, t*y*z + s*x, 0]
    float col1_0 = t * x * y - s * z;
    float col1_1 = t * y * y + c;
    float col1_2 = t * y * z + s * x;
    float col1_3 = 0.0f;

    // column 2: [t*x*z + s*y, t*y*z - s*x, t*z² + c, 0]
    float col2_0 = t * x * z + s * y;
    float col2_1 = t * y * z - s * x;
    float col2_2 = t * z * z + c;
    float col2_3 = 0.0f;

    // column 3: [0, 0, 0, 1]
    float col3_0 = 0.0f;
    float col3_1 = 0.0f;
    float col3_2 = 0.0f;
    float col3_3 = 1.0f;

    // pack the columns into SIMD registers
    store_mat4_column(&rotationMatrix, 0, _mm_set_ps(col0_3, col0_2, col0_1, col0_0));
    store_mat4_column(&rotationMatrix, 1, _mm_set_ps(col1_3, col1_2, col1_1, col1_0));
    store_mat4_column(&rotationMatrix, 2, _mm_set_ps(col2_3, col2_2, col2_1, col2_0));
    store_mat4_column(&rotationMatrix, 3, _mm_set_ps(col3_3, col3_2, col3_1, col3_0));

    // multiply the initial matrix by the rotation matrix
    return mat4_mul(initial, &rotationMatrix);
}

mat4 mat4_scale(const mat4* initial, float x, float y, float z) {
    mat4 scalingMatrix = { {
        {x,    0.0f, 0.0f, 0.0f},
        {0.0f, y,    0.0f, 0.0f},
        {0.0f, 0.0f, z,    0.0f},
        {0.0f, 0.0f, 0.0f, 1.0f}
    } };

    return mat4_mul(initial, &scalingMatrix);
}

Quat yaw_pitch_roll(float yaw, float pitch, float roll) {
    Quat q;
    float halfYaw = yaw * 0.5f;
    float halfPitch = pitch * 0.5f;
    float halfRoll = roll * 0.5f;

    float cosYaw = cosf(halfYaw);
    float sinYaw = sinf(halfYaw);
    float cosPitch = cosf(halfPitch);
    float sinPitch = sinf(halfPitch);
    float cosRoll = cosf(halfRoll);
    float sinRoll = sinf(halfRoll);

    q.x = sinYaw * cosPitch * sinRoll + cosYaw * sinPitch * cosRoll;
    q.y = cosYaw * sinPitch * cosRoll - sinYaw * cosPitch * sinRoll;
    q.z = cosYaw * cosPitch * sinRoll - sinYaw * sinPitch * cosRoll;
    q.w = cosYaw * cosPitch * cosRoll + sinYaw * sinPitch * sinRoll;

    return q;
}

void quat_to_mat4(const Quat* q, mat4* mat) {
    float xx = q->x * q->x;
    float xy = q->x * q->y;
    float xz = q->x * q->z;
    float xw = q->x * q->w;
    float yy = q->y * q->y;
    float yz = q->y * q->z;
    float yw = q->y * q->w;
    float zz = q->z * q->z;
    float zw = q->z * q->w;

    // column-major order
    mat->data[0][0] = 1.0f - 2.0f * (yy + zz);
    mat->data[0][1] = 2.0f * (xy + zw);
    mat->data[0][2] = 2.0f * (xz - yw);
    mat->data[0][3] = 0.0f;

    mat->data[1][0] = 2.0f * (xy - zw);
    mat->data[1][1] = 1.0f - 2.0f * (xx + zz);
    mat->data[1][2] = 2.0f * (yz + xw);
    mat->data[1][3] = 0.0f;

    mat->data[2][0] = 2.0f * (xz + yw);
    mat->data[2][1] = 2.0f * (yz - xw);
    mat->data[2][2] = 1.0f - 2.0f * (xx + yy);
    mat->data[2][3] = 0.0f;

    mat->data[3][0] = 0.0f;
    mat->data[3][1] = 0.0f;
    mat->data[3][2] = 0.0f;
    mat->data[3][3] = 1.0f;
}
