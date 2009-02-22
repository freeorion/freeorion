// -*- C++ -*-
uniform sampler2D color_texture, normal_texture;

varying float diffuse;
varying vec2 tex_coord;
varying vec3 light_vec;

const float AMBIENT = 0.1;

void main()
{
    vec3 color = texture2D(color_texture, tex_coord).rgb * max(diffuse, AMBIENT);

    // renormalize to [-1, 1]
    vec3 normal = texture2D(normal_texture, tex_coord).xyz * 2.0 - 1.0;
    float normal_factor = abs(dot(normal, light_vec));
    color *= normal_factor;

    gl_FragColor = vec4(color, 1.0);
}
