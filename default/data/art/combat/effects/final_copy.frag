// -*- C++ -*-
uniform sampler2D source;

void main()
{
    gl_FragColor = texture2D(source, gl_TexCoord[0].st);
}
