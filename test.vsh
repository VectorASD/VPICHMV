#version 130

//attribute highp vec4 qt_Vertex;
//attribute highp vec2 qt_MultiTexCoord0;
//attribute float qt_LOL[20];

in vec4 Position;
in vec2 TexCoord;
in vec3 Normal;
in vec3 Tangent;
in vec3 Binormal;

const vec3 lightPos = vec3(100,100,100);

uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ModelMatrix;

out vec2 tc;
out vec3 light;
out highp vec2 qt_TexCoord0;

void main() {
    vec3 T = normalize(vec3(ModelMatrix * vec4(Tangent,  0.0)));
    vec3 B = normalize(vec3(ModelMatrix * vec4(Binormal, 0.0)));
    vec3 N = normalize(vec3(ModelMatrix * vec4(Normal,   0.0)));
    mat3 TBN = transpose(mat3(T, B, N));
    vec4 wp = ModelMatrix * Position;

    light = TBN * (lightPos - wp.xyz);
    tc = Tangent.xy;

    gl_Position = ProjectionMatrix * ViewMatrix * wp;
    qt_TexCoord0 = vec2(gl_MultiTexCoord0.xy);
}
