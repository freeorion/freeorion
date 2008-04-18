// -*- C++ -*-
uniform sampler2D scene;
uniform sampler2D glow_geometry;

const float exposure = 1.0;
const float prescale = 1.0;
const float gloom = 0.95;

// Set this to adjust the brightness of glowing objects.  The higher you set this,
// the more objects will glow, but they may also show more banding artifacts.
const float glow_amplification_factor = 2.0;

void main()
{
    gl_FragColor = texture2D(scene, gl_TexCoord[0].st);
    float glow_alpha = texture2D(glow_geometry, gl_TexCoord[0].st).a;
    gl_FragColor.rgb *= (glow_alpha * glow_alpha * glow_amplification_factor + 1.0);
    gl_FragColor = gl_FragColor * exposure - gloom;
    gl_FragColor *= prescale;
    gl_FragColor = max(gl_FragColor, vec4(0.0));
}
