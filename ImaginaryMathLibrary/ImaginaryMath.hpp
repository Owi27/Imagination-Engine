namespace Math
{
    struct vec2
    {
        float x, y;

        vec2();
        vec2(float);
        vec2(float, float);
    };

    struct vec3
    {
        float x, y, z;

        vec3();
        vec3(float);
        vec3(float, float, float);
        vec3(float, vec2);
        vec3(vec2, float);
    };

    struct vec4
    {
        float x, y, z, a;

        vec4();
        vec4(float);
        vec4(float, float, float, float);
        vec4(float, float, vec2);
        vec4(float, vec2, float);
        vec4(vec2, float, float);
        vec4(vec2, vec2);
        vec4(float, vec3);
        vec4(vec3, float);
    };

    struct mat2
    {
        vec2 row1, row2;

        mat2();

    };
}