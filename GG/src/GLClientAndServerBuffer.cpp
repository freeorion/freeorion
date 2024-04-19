//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2011 Rainer Kupke
//!  Copyright (C) 2013-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

#include <GG/GLClientAndServerBuffer.h>


using namespace GG;

///////////////////////////////////////////////////////////////////////////
// GLBufferBase
///////////////////////////////////////////////////////////////////////////

void GLBufferBase::dropServerBuffer()
{
    if (b_name) {
        glDeleteBuffers(1, &b_name);
        b_name = 0;
    }
}

void GLBufferBase::harmonizeBufferType(GLBufferBase& other)
{
    if (b_name && other.b_name) return; // OK, both have server buffer

    if (b_name || other.b_name) {       // NOT OK, only one has server buffer, drop buffer
        dropServerBuffer();
        other.dropServerBuffer();
    }
}


template class GG::GLClientAndServerBufferBase<uint8_t, 4>;
template class GG::GLClientAndServerBufferBase<float, 2>;
template class GG::GLClientAndServerBufferBase<float, 3>;
template class GG::GLClientAndServerBufferBase<float, 4>;

///////////////////////////////////////////////////////////////////////////
// GLRGBAColorBuffer
///////////////////////////////////////////////////////////////////////////
void GLRGBAColorBuffer::activate() const
{
    if (b_name) {
        glBindBuffer(GL_ARRAY_BUFFER, b_name);
        glColorPointer(4, GL_UNSIGNED_BYTE, 0, nullptr);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    } else {
        glColorPointer(4, GL_UNSIGNED_BYTE, 0, b_data.empty() ? nullptr: b_data.data());
    }
}


///////////////////////////////////////////////////////////////////////////
// GL2DVertexBuffer
///////////////////////////////////////////////////////////////////////////
void GL2DVertexBuffer::activate() const
{
    if (b_name) {
        glBindBuffer(GL_ARRAY_BUFFER, b_name);
        glVertexPointer(2, GL_FLOAT, 0, nullptr);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    } else {
        glVertexPointer(2, GL_FLOAT, 0, b_data.empty() ? nullptr: b_data.data());
    }
}


///////////////////////////////////////////////////////////////////////////
// GLTexCoordBuffer
///////////////////////////////////////////////////////////////////////////
void GLTexCoordBuffer::activate() const
{
    if (b_name) {
        glBindBuffer(GL_ARRAY_BUFFER, b_name);
        glTexCoordPointer(2, GL_FLOAT, 0, nullptr);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    } else {
        glTexCoordPointer(2, GL_FLOAT, 0, b_data.empty() ? nullptr: b_data.data());
    }
}


///////////////////////////////////////////////////////////////////////////
// GL3DVertexBuffer
///////////////////////////////////////////////////////////////////////////
void GL3DVertexBuffer::activate() const
{
    if (b_name) {
        glBindBuffer(GL_ARRAY_BUFFER, b_name);
        glVertexPointer(3, GL_FLOAT, 0, nullptr);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    } else {
        glVertexPointer(3, GL_FLOAT, 0, b_data.empty() ? nullptr: b_data.data());
    }
}


///////////////////////////////////////////////////////////////////////////
// GLNormalBuffer
///////////////////////////////////////////////////////////////////////////
void GLNormalBuffer::activate() const
{
    if (b_name) {
        glBindBuffer(GL_ARRAY_BUFFER, b_name);
        glNormalPointer(GL_FLOAT, 0, nullptr);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    } else {
        glNormalPointer(GL_FLOAT, 0, b_data.empty() ? nullptr: b_data.data());
    }
}
