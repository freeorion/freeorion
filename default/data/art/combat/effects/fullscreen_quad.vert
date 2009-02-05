// -*- C++ -*-
void main()
{
    vec2 pos = sign(gl_Vertex.xy);
    gl_TexCoord[0].st = (vec2(pos.x, -pos.y) + 1.0) / 2.0;
    gl_Position = ftransform();
}
