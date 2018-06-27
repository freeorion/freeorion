/*
 *  GLClientAndServerBuffer.cpp
 *  FreeOrion
 *
 *  Created by Rainer Kupke on 06.02.11.
 *  Copyright 2011. All rights reserved.
 *
 */

#include <GG/GLClientAndServerBuffer.h>

namespace GG {

///////////////////////////////////////////////////////////////////////////
// GLBufferBase
///////////////////////////////////////////////////////////////////////////
GLBufferBase::GLBufferBase() :
    b_name(0)
{}

GLBufferBase::~GLBufferBase()
{ dropServerBuffer(); }

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

///////////////////////////////////////////////////////////////////////////
// GLClientAndServerBufferBase<vtype> template
///////////////////////////////////////////////////////////////////////////
template <class vtype>
GLClientAndServerBufferBase<vtype>::GLClientAndServerBufferBase(std::size_t elementsPerItem) :
    GLBufferBase(),
    b_data(),
    b_size(0),
    b_elements_per_item(elementsPerItem)
{}

template <class vtype>
std::size_t GLClientAndServerBufferBase<vtype>::size() const
{ return b_size; }

template <class vtype>
bool GLClientAndServerBufferBase<vtype>::empty() const
{ return b_size == 0; }

template <class vtype>
void GLClientAndServerBufferBase<vtype>::reserve(std::size_t num_items)
{ b_data.reserve(num_items * b_elements_per_item); }

template <class vtype>
void GLClientAndServerBufferBase<vtype>::store(vtype item)
{
    b_data.push_back(item);
    b_size=b_data.size() / b_elements_per_item;
}

template <class vtype>
void GLClientAndServerBufferBase<vtype>::store(vtype item1, vtype item2)
{
    b_data.push_back(item1);
    b_data.push_back(item2);
    b_size=b_data.size() / b_elements_per_item;
}

template <class vtype>
void GLClientAndServerBufferBase<vtype>::store(vtype item1, vtype item2, vtype item3)
{
    b_data.push_back(item1);
    b_data.push_back(item2);
    b_data.push_back(item3);
    b_size=b_data.size() / b_elements_per_item;
}

template <class vtype> 
void GLClientAndServerBufferBase<vtype>::store(vtype item1, vtype item2, vtype item3, vtype item4)
{
    b_data.push_back(item1);
    b_data.push_back(item2);
    b_data.push_back(item3);
    b_data.push_back(item4);
    b_size=b_data.size() / b_elements_per_item;
}

template <class vtype>
void GLClientAndServerBufferBase<vtype>::createServerBuffer()
{
    glGenBuffers(1, &b_name);
    if (!b_name)
        return;
    glBindBuffer(GL_ARRAY_BUFFER, b_name);
    glBufferData(GL_ARRAY_BUFFER,
                 b_data.size() * sizeof(vtype),
                 b_data.empty() ? nullptr : &b_data[0],
                 GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

template <class vtype> void GLClientAndServerBufferBase<vtype>::clear()
{
    dropServerBuffer();
    b_size = 0;
    b_data.clear();
}

///////////////////////////////////////////////////////////////////////////
// GLRGBAColorBuffer
///////////////////////////////////////////////////////////////////////////
template class GLClientAndServerBufferBase<unsigned char>;

GLRGBAColorBuffer::GLRGBAColorBuffer() :
GLClientAndServerBufferBase<unsigned char>(4)
{}

void GLRGBAColorBuffer::store(const Clr& color)
{ GLClientAndServerBufferBase::store(color.r, color.g, color.b, color.a); }

void GLRGBAColorBuffer::activate() const
{
    if (b_name) {
        glBindBuffer(GL_ARRAY_BUFFER, b_name);
        glColorPointer(4, GL_UNSIGNED_BYTE, 0, nullptr);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    } else {
        glColorPointer(4, GL_UNSIGNED_BYTE, 0, b_data.empty() ? nullptr: &b_data[0]);
    }
}

///////////////////////////////////////////////////////////////////////////
// GL2DVertexBuffer
///////////////////////////////////////////////////////////////////////////
template class GLClientAndServerBufferBase<float>;

GL2DVertexBuffer::GL2DVertexBuffer() :
    GLClientAndServerBufferBase<float>(2)
{}

void GL2DVertexBuffer::store(const Pt& pt)
{ store(pt.x, pt.y); }

void GL2DVertexBuffer::store(X x, Y y)
{ GLClientAndServerBufferBase::store(Value(x), Value(y)); }

void GL2DVertexBuffer::store(X x, float y)
{ GLClientAndServerBufferBase::store(Value(x), y); }

void GL2DVertexBuffer::store(float x, Y y)
{ GLClientAndServerBufferBase::store(x, Value(y)); }

void GL2DVertexBuffer::store(float x, float y)
{ GLClientAndServerBufferBase::store(x, y); }

void GL2DVertexBuffer::activate() const
{
    if (b_name) {
        glBindBuffer(GL_ARRAY_BUFFER, b_name);
        glVertexPointer(2, GL_FLOAT, 0, nullptr);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    } else {
        glVertexPointer(2, GL_FLOAT, 0, b_data.empty() ? nullptr: &b_data[0]);
    }
}


///////////////////////////////////////////////////////////////////////////
// GLTexCoordBuffer
///////////////////////////////////////////////////////////////////////////
GLTexCoordBuffer::GLTexCoordBuffer() :
    GLClientAndServerBufferBase<float>(2)
{}

void GLTexCoordBuffer::activate() const
{
    if (b_name) {
        glBindBuffer(GL_ARRAY_BUFFER, b_name);
        glTexCoordPointer(2, GL_FLOAT, 0, nullptr);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    } else {
        glTexCoordPointer(2, GL_FLOAT, 0, b_data.empty() ? nullptr: &b_data[0]);
    }
}


///////////////////////////////////////////////////////////////////////////
// GL3DVertexBuffer
///////////////////////////////////////////////////////////////////////////
GL3DVertexBuffer::GL3DVertexBuffer() :
    GLClientAndServerBufferBase<float>(3)
{}

void GL3DVertexBuffer::store(float x, float y, float z)
{ GLClientAndServerBufferBase::store(x, y, z); }

void GL3DVertexBuffer::activate() const
{
    if (b_name) {
        glBindBuffer(GL_ARRAY_BUFFER, b_name);
        glVertexPointer(3, GL_FLOAT, 0, nullptr);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    } else {
        glVertexPointer(3, GL_FLOAT, 0, b_data.empty() ? nullptr: &b_data[0]);
    }
}


///////////////////////////////////////////////////////////////////////////
// GLNormalBuffer
///////////////////////////////////////////////////////////////////////////
GLNormalBuffer::GLNormalBuffer() :
    GLClientAndServerBufferBase<float>(3)
{}

void GLNormalBuffer::activate() const
{
    if (b_name) {
        glBindBuffer(GL_ARRAY_BUFFER, b_name);
        glNormalPointer(GL_FLOAT, 0, nullptr);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    } else {
        glNormalPointer(GL_FLOAT, 0, b_data.empty() ? nullptr: &b_data[0]);
    }
}

}
