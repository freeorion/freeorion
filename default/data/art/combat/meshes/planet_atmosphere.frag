// -*- C++ -*-
uniform vec4 atmosphere_color;
uniform vec4 atmosphere_edge_color;

varying float sun_intensity;
varying float position_dot_product;
varying float above_surface_dot_product;
varying float surface_dot_product;
varying float edge_below_surface_dot_product;
varying float below_surface_dot_product;

void main()
{
    // As a first pass, add in the atmosphere glow that exists only below the
    // surface -- between sphere_radius and subsurface_radius.
    float alpha_factor_1 = atmosphere_color.w;
    alpha_factor_1 *= sun_intensity;
    if (surface_dot_product < position_dot_product)
        alpha_factor_1 *= 1.0 - smoothstep(surface_dot_product, below_surface_dot_product, position_dot_product);
    else
        alpha_factor_1 *= smoothstep(above_surface_dot_product, surface_dot_product, position_dot_product);

    // Now blend in the atmosphere glow that exists only at the edge of the planet
    // -- between edge_subsurface_radius and atmosphere_radius.  Note that this
    // may overlap the first pass, depending on the radius values used in the
    // vertex shader.
    float alpha_factor_2 = atmosphere_edge_color.w;
    alpha_factor_2 *= sun_intensity;
    if (surface_dot_product < position_dot_product)
        alpha_factor_2 *= 1.0 - smoothstep(surface_dot_product, edge_below_surface_dot_product, position_dot_product);
    else
        alpha_factor_2 *= smoothstep(above_surface_dot_product, surface_dot_product, position_dot_product);

    float alpha_sum = alpha_factor_1 + alpha_factor_2;
    float a1 = alpha_factor_1 / alpha_sum;
    float a2 = alpha_factor_2 / alpha_sum;
    gl_FragColor =
        vec4(atmosphere_color.xyz * a1 + atmosphere_edge_color.xyz * a2,
             max(alpha_factor_1, alpha_factor_2));
}

