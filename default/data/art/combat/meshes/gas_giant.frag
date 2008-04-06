// -*- C++ -*-
uniform sampler2D gas_texture;

varying float diffuse;
varying vec2 tex_coord;
varying float position_dot_product;
varying float surface_dot_product;
varying float below_surface_dot_product;

const float TERMINATOR = 0.3;
const float INV_TERMINATOR = 1.0 / (2.0 * TERMINATOR);
const float AMBIENT = 0.3;

void main()
{
    vec3 daytime = texture2D(gas_texture, tex_coord).rgb * diffuse;
    vec3 color = daytime;
    if (diffuse < -TERMINATOR)
        color = daytime * AMBIENT;
    if (abs(diffuse) < TERMINATOR) {
        vec3 nighttime = daytime * AMBIENT;
        color = mix(nighttime, daytime, (diffuse + TERMINATOR) * INV_TERMINATOR);
    }

    float alpha_factor = 1.0;
    if (surface_dot_product < position_dot_product)
        alpha_factor *= smoothstep(surface_dot_product, below_surface_dot_product, position_dot_product);
    gl_FragColor = vec4(color, alpha_factor);
}
