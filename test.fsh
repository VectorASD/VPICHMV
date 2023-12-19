#version 130
#extension GL_EXT_gpu_shader4 : enable

uniform sampler2D sampler0;
uniform sampler2D sampler1;
uniform vec4 viewport;

varying highp vec2 qt_TexCoord0;
varying vec2 tc;
varying vec3 light;

out vec4 FragColor;

void main() {
        //vec3 tex_normal = texture2D(sampler1, tc).rgb;
        //tex_normal = normalize(tex_normal * 2.0 - 1.0);

        //float diff = max(dot(tex_normal, normalize(light)), 0.1);

        //vec4 diff_color = texture2D(sampler0, tc) * vec4(vec3(diff),1);

        vec2 pos = gl_FragCoord.xy / viewport.zw;

        //vec4 diff_color = vec4(pos.xy, 1, 1);
        vec4 diff_color = texture2D(sampler0, qt_TexCoord0);

        //if (int(gl_FragCoord.x / 4) % 2 == 0 && int(gl_FragCoord.y / 4) % 2 == 0) diff_color = vec4(0, 0, 1, 1);
        int x = int(gl_FragCoord.x / 1.5), y = int(gl_FragCoord.y / 1.5);
        int xy = int(x * y / 5);
        if (xy % 2 + xy % 3 == 0) diff_color = vec4(1 - diff_color.rgb, 1);

        //vec4 diff_color = vec4(qt_TexCoord0.xy, 1, 1);

        FragColor = diff_color;
}
