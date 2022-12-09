//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2011 Rainer Kupke
//!  Copyright (C) 2013-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef _GLClientAndServerBuffer_h_
#define _GLClientAndServerBuffer_h_


#include <vector>
#include <GG/Base.h>


namespace GG {

///////////////////////////////////////////////////////////////////////////
// GLBufferBase common base class for Buffer classes
///////////////////////////////////////////////////////////////////////////
class GG_API GLBufferBase
{
public:
    GLBufferBase() = default;

    /** Required to automatically drop server buffer in case of delete. */
    virtual ~GLBufferBase() { dropServerBuffer(); }

    // use this if you want to make sure that two buffers both
    // have server buffers or not, drops the buffer for mixed cases
    void harmonizeBufferType(GLBufferBase& other);

protected:
    // drops the server buffer if one exists
    void dropServerBuffer();

    GLuint b_name = 0;
};

///////////////////////////////////////////////////////////////////////////
// GLClientAndServerBufferBase
// template class for buffers with different types of content
///////////////////////////////////////////////////////////////////////////
template <typename vtype, std::size_t N>
class GG_API GLClientAndServerBufferBase : public GLBufferBase
{
public:
    GLClientAndServerBufferBase() = default;
    [[nodiscard]] std::size_t size() const { return b_size; }
    [[nodiscard]] bool        empty() const { return b_size == 0; }

    // pre-allocate space for item data
    void reserve(std::size_t num_items) { b_data.reserve(num_items * b_elements_per_item); }

protected:
    // store items, buffers usually store tuples, convenience functions
    // do not use while server buffer exists
    template <std::size_t M = N, std::enable_if_t<M == 1>* = nullptr>
    void store(vtype item)
    {
        static_assert(b_elements_per_item == 1);
        b_data.push_back(item);
        ++b_size;
    }

    template <std::size_t M = N, std::enable_if_t<M == 2>* = nullptr>
    void store(vtype item1, vtype item2)
    {
        static_assert(b_elements_per_item == 2);
        b_data.push_back(item1);
        b_data.push_back(item2);
        ++b_size;
    }

    template <std::size_t M = N, std::enable_if_t<M == 3>* = nullptr>
    void store(vtype item1, vtype item2, vtype item3)
    {
        static_assert(b_elements_per_item == 3);
        b_data.push_back(item1);
        b_data.push_back(item2);
        b_data.push_back(item3);
        ++b_size;
    }

    template <std::size_t M = N, std::enable_if_t<M == 4>* = nullptr>
    void store(vtype item1, vtype item2, vtype item3, vtype item4)
    {
        static_assert(b_elements_per_item == 4);
        b_data.push_back(item1);
        b_data.push_back(item2);
        b_data.push_back(item3);
        b_data.push_back(item4);
        ++b_size;
    }

public:
    // try to store the buffered data in a server buffer
    void createServerBuffer() {
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

    // drops a server buffer if one exists, clears the client side buffer
    void clear() {
        dropServerBuffer();
        b_size = 0;
        b_data.clear();
    }

protected:
    std::vector<vtype>           b_data;
    std::size_t                  b_size = 0;
    static constexpr std::size_t b_elements_per_item = N;

    // used in derived classes to activate the buffer
    // implementations should use glBindBuffer, gl...Pointer if
    // server buffer exists (b_name! = 0), just gl...Pointer otherwise
    virtual void activate() const = 0;
};

///////////////////////////////////////////////////////////////////////////
// GLRGBAColorBuffer specialized class for RGBA color values
///////////////////////////////////////////////////////////////////////////
class GG_API GLRGBAColorBuffer : public GLClientAndServerBufferBase<uint8_t, 4>
{
public:
    using base_t = GLClientAndServerBufferBase<uint8_t, 4>;
    GLRGBAColorBuffer() = default;
    void store(const Clr color) { base_t::store(color.r, color.g, color.b, color.a); }
    void activate() const override;
};

///////////////////////////////////////////////////////////////////////////
// GL2DVertexBuffer specialized class for 2d vertex data
///////////////////////////////////////////////////////////////////////////
class GG_API GL2DVertexBuffer : public GLClientAndServerBufferBase<float, 2>
{
public:
    using base_t = GLClientAndServerBufferBase<float, 2>;
    GL2DVertexBuffer() = default;
    void store(const Pt pt) { store(pt.x, pt.y); }
    void store(X x, Y y) { base_t::store(static_cast<float>(Value(x)), static_cast<float>(Value(y))); }
    void store(X x, float y) { base_t::store(static_cast<float>(Value(x)), y); }
    void store(float x, Y y) { base_t::store(x, static_cast<float>(Value(y))); }
    void store(float x, float y) { base_t::store(x, y); }
    void activate() const override;
};

///////////////////////////////////////////////////////////////////////////
// GLTexCoordBuffer specialized class for texture coordinate data
///////////////////////////////////////////////////////////////////////////
class GG_API GLTexCoordBuffer : public GLClientAndServerBufferBase<float, 2>
{
public:
    using base_t = GLClientAndServerBufferBase<float, 2>;
    GLTexCoordBuffer() = default;
    void store(float x, float y) { base_t::store(x, y); }
    void activate() const override;
};

///////////////////////////////////////////////////////////////////////////
// GL3DVertexBuffer specialized class for 3d vertex data
///////////////////////////////////////////////////////////////////////////
class GG_API GL3DVertexBuffer : public GLClientAndServerBufferBase<float, 3>
{
public:
    using base_t = GLClientAndServerBufferBase<float, 3>;
    GL3DVertexBuffer() = default;
    void store(float x, float y, float z) { base_t::store(x, y, z); }
    void activate() const override;
};

///////////////////////////////////////////////////////////////////////////
// GLNormalBuffer specialized class for 3d normal data
///////////////////////////////////////////////////////////////////////////
class GG_API GLNormalBuffer : public GLClientAndServerBufferBase<float, 3>
{
public:
    using base_t = GLClientAndServerBufferBase<float, 3>;
    GLNormalBuffer() = default;
    void store(float x, float y, float z) { base_t::store(x, y, z); }
    void activate() const override;
};

}


#endif
