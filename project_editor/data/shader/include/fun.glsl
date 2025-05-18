// this holds a bunch of functions to facilitate shader-coding

// rotates the uv coordinates within the center
vec2 RotateUV(vec2 uv, float angle) {
    vec2 center = vec2(0.5, 0.5);
    uv -= center;

    float xCos = cos(angle);
    float xSin = sin(angle);
    mat2 rot = mat2(xCos, -xSin, xSin, xCos);

    uv = rot * uv;
    uv += center;
    return uv;
}

// applies the rotation, scale and translation of the uv coordinates
vec2 TransformUV(vec2 uv, vec2 uvOffset, vec2 uvScale, float angle) {
    uv = RotateUV(uv, angle);
    uv = (uv + uvOffset) * uvScale;
    return uv;
}