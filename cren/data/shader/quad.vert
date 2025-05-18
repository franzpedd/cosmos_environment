#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_ARB_gpu_shader_int64 : enable

// includes
#include "include/fun.glsl"
#include "include/primitives.glsl"
#include "include/ubo_camera.glsl"
#include "include/ubo_sprite.glsl"
#include "include/push_constant.glsl"

// output vertex attributes
layout(location = 0) out vec2 outFragTexCoord;

// returns the correct matrix according with the locking mechanism
mat4 GetBillboardMatrix() {

    // return type
    mat4 billboard = pushConstant.model;

    // it is a billboard
    if(spriteParams.billboard == 1) {

        // params
        vec3 worldUp = vec3(0.0, 1.0, 0.0);
        vec3 cameraForward = normalize(vec3(camera.view[0][2], camera.view[1][2], camera.view[2][2]));
        vec3 cameraRight = normalize(cross(worldUp, cameraForward));
        vec3 cameraUp = normalize(cross(cameraForward, cameraRight));
        vec3 modelTranslation = pushConstant.model[3].xyz;

        // locking both axis is not a billboard, but just the mesh at the position
        if(spriteParams.lockAxis.x == 1.0 && spriteParams.lockAxis.y == 1.0) {
            return billboard;
        }

        // lock x axis
        else if(spriteParams.lockAxis.x == 1.0) {

            vec3 cameraForwardYZ = normalize(vec3(0.0, cameraForward.y, cameraForward.z));
            cameraRight = normalize(cross(cameraForwardYZ, vec3(1.0, 0.0, 0.0)));
            cameraUp = normalize(cross(cameraRight, cameraForwardYZ));
            billboard = mat4(
                vec4(cameraRight, 0.0),
                vec4(cameraUp, 0.0),
                vec4(cameraForwardYZ, 0.0),
                vec4(modelTranslation, 1.0)
            );
        }

        // lock y axis
        else if(spriteParams.lockAxis.y == 1.0) {

            vec3 cameraForwardXZ = normalize(vec3(cameraForward.x, 0.0, cameraForward.z));
            cameraRight = normalize(cross(worldUp, cameraForwardXZ));
            cameraUp = normalize(cross(cameraForwardXZ, cameraRight));
            billboard = mat4(
                vec4(cameraRight, 0.0),
                vec4(cameraUp, 0.0),
                vec4(cameraForwardXZ, 0.0),
                vec4(modelTranslation, 1.0)
            );
        }

        // not locking any axis
        else {
            billboard = mat4(
               vec4(cameraRight, 0.0),
               vec4(cameraUp, 0.0),
               vec4(cameraForward, 0.0),
               vec4(modelTranslation, 1.0));
        }
    }

    return billboard;
}

// returns the correct uv orientation if billboard is active
vec2 GetCorrectedUV()
{
    vec2 frag = SquareUVs[gl_VertexIndex];

    // we must invert the sprite to correctly face the camera if it is a billboard
    if(spriteParams.billboard == 1) 
    {
        // both locked, don't flip uv
        if(spriteParams.lockAxis.x == 1.0 && spriteParams.lockAxis.y == 1.0){
            return frag;
        }

        // x axis is locked, flip X, Y and rotate
        else if(spriteParams.lockAxis.x == 1.0) {
            vec2 uv = vec2(1.0 - SquareUVs[gl_VertexIndex].x, 1.0 - SquareUVs[gl_VertexIndex].y);
            frag = RotateUV(uv, radians(90.0));
        }

        // y axis is locked, flip X
        else if(spriteParams.lockAxis.y == 1.0) {
            frag = vec2(1.0 - SquareUVs[gl_VertexIndex].x, SquareUVs[gl_VertexIndex].y);
        }

        // not locked, flip X
        else {
            frag = vec2(1.0 - SquareUVs[gl_VertexIndex].x, SquareUVs[gl_VertexIndex].y);
        }
    }

    return frag;
}

// entrypoint
void main()
{
    gl_Position = camera.proj * camera.view * GetBillboardMatrix() * vec4(SquareVertices[gl_VertexIndex].xyz, 1.0);
    outFragTexCoord = GetCorrectedUV();
}