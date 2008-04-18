// -*- C++ -*-
uniform sampler2D scene;
uniform sampler2D pass_1;
uniform sampler2D pass_2;
uniform sampler2D pass_3;
uniform sampler2D pass_4;

const float exposure = 1.0;
const float prescale = 1.0;

void main()
{
    vec4 color = texture2D(scene, gl_TexCoord[0].st);
    vec4 passes =
        mat4(texture2D(pass_1, gl_TexCoord[0].st),
             texture2D(pass_2, gl_TexCoord[0].st),
             texture2D(pass_3, gl_TexCoord[0].st),
             texture2D(pass_4, gl_TexCoord[0].st)) *
        vec4(0.125, 0.125, 0.125, 0.33333);
    gl_FragColor = color * exposure + passes * 16.0 / prescale;
}
