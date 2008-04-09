// -*- C++ -*-
uniform vec4 atmosphere_color;

varying float sun_intensity;
varying float position_dot_product;
varying float above_surface_dot_product;
varying float surface_dot_product;
varying float below_surface_dot_product;

void main()
{
    float alpha_factor = 1.0;
    alpha_factor *= sun_intensity;
    if (surface_dot_product < position_dot_product)
        alpha_factor *= 1.0 - smoothstep(surface_dot_product, below_surface_dot_product, position_dot_product);
    else
        alpha_factor *= smoothstep(above_surface_dot_product, surface_dot_product, position_dot_product);
    gl_FragColor = vec4(atmosphere_color.xyz, atmosphere_color.w * alpha_factor);
}
