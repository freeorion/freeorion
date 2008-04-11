// -*- C++ -*-
uniform sampler2D gas_texture;

varying float diffuse;
varying vec2 tex_coord;
varying float position_dot_product;
varying float surface_dot_product;
varying float below_surface_dot_product;

const float TERMINATOR = 0.3;
const float AMBIENT = 0.05;

void main()
{
    vec3 texture_color = texture2D(gas_texture, tex_coord).rgb;
    vec3 daytime = texture_color * max(diffuse, AMBIENT);
    vec3 color = daytime;
    if (diffuse < -TERMINATOR)
        color = texture_color * AMBIENT;
    if (abs(diffuse) < TERMINATOR) {
        vec3 nighttime = texture_color * AMBIENT;
        color = mix(nighttime, daytime, (diffuse + TERMINATOR) / (2.0 * TERMINATOR));
    }

    float alpha_factor = 1.0;
    if (surface_dot_product < position_dot_product)
        alpha_factor *= smoothstep(surface_dot_product, below_surface_dot_product, position_dot_product);
    gl_FragColor = vec4(color, alpha_factor);
}
