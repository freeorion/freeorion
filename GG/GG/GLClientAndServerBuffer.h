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
    [[nodiscard]] std::size_t size() const noexcept { return b_data.size() / b_elements_per_item; }
    [[nodiscard]] bool        empty() const noexcept { return b_data.empty(); }

    // pre-allocate space for item data
    void reserve(std::size_t num_items) { b_data.reserve(num_items * b_elements_per_item); }
    auto capacity() noexcept { return b_data.capacity(); };
    auto raw_size() noexcept { return b_data.size(); };

protected:
    // store items, buffers usually store tuples, convenience functions
    // do not use while server buffer exists
    template <std::size_t ArrN>
    void store(std::array<vtype, N*ArrN> items)
    { b_data.insert(b_data.end(), items.begin(), items.end()); }

    template <std::size_t M = N, std::enable_if_t<M == 1>* = nullptr>
    void store(vtype item)
    {
        static_assert(b_elements_per_item == 1);
        b_data.push_back(item);
    }

    template <std::size_t M = N, std::enable_if_t<M == 2>* = nullptr>
    void store(vtype item1, vtype item2)
    {
        static_assert(b_elements_per_item == 2);
        b_data.insert(b_data.end(), {item1, item2});
    }

    template <std::size_t M = N, std::enable_if_t<M == 3>* = nullptr>
    void store(vtype item1, vtype item2, vtype item3)
    {
        static_assert(b_elements_per_item == 3);
        b_data.insert(b_data.end(), {item1, item2, item3});
    }

    template <std::size_t M = N, std::enable_if_t<M == 4>* = nullptr>
    void store(vtype item1, vtype item2, vtype item3, vtype item4)
    {
        static_assert(b_elements_per_item == 4);
        b_data.insert(b_data.end(), {item1, item2, item3, item4});
    }

public:
    // try to store the buffered data in a server buffer
    void createServerBuffer(GLenum usage = GL_STATIC_DRAW) {
        if (!b_name)
            glGenBuffers(1, &b_name);
        if (!b_name)
            return;
        glBindBuffer(GL_ARRAY_BUFFER, b_name);
        glBufferData(GL_ARRAY_BUFFER,
                     b_data.size() * sizeof(vtype),
                     b_data.empty() ? nullptr : b_data.data(),
                     usage);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    // drops a server buffer if one exists, clears the client side buffer
    void clear() noexcept
    { b_data.clear(); }

protected:
    std::vector<vtype>           b_data;
    static constexpr std::size_t b_elements_per_item = N;

    // used in derived classes to activate the buffer
    // implementations should use glBindBuffer, gl...Pointer if
    // server buffer exists (b_name! = 0), just gl...Pointer otherwise
    virtual void activate() const = 0;
};

///////////////////////////////////////////////////////////////////////////
// GLRGBAColorBuffer specialized class for RGBA color values
///////////////////////////////////////////////////////////////////////////
class GG_API GLRGBAColorBuffer final : public GLClientAndServerBufferBase<uint8_t, 4>
{
public:
    using base_t = GLClientAndServerBufferBase<uint8_t, 4>;
    GLRGBAColorBuffer() = default;
    void store(const Clr color) { base_t::store(color.r, color.g, color.b, color.a); }

    // Arrn distinct colours
    template <std::size_t ArrN>
    void store(std::array<Clr, ArrN> clrs)
    {
        std::array<uint8_t, ArrN*4> data{};
        auto data_it = data.begin();
        for (auto clr : clrs) {
            *data_it++ = clr.r;
            *data_it++ = clr.g;
            *data_it++ = clr.b;
            *data_it++ = clr.a;
        }
        base_t::store<ArrN>(data);
    }

    // same colour ArrN times
    template <std::size_t ArrN>
    void store(Clr clr)
    {
        std::array<uint8_t, ArrN*4> data{};
        auto data_it = data.begin();
        for (std::size_t n = 0; n < ArrN; ++n) {
            *data_it++ = clr.r;
            *data_it++ = clr.g;
            *data_it++ = clr.b;
            *data_it++ = clr.a;
        }
        base_t::store<ArrN>(data);
    }

    // same colour N times
    void store(std::size_t N, Clr clr)
    {
        base_t::reserve(base_t::size() + N);
        for (std::size_t n = 0; n < N; ++n)
            base_t::store(clr.r, clr.g, clr.b, clr.a);
    }

    void activate() const override;
};

///////////////////////////////////////////////////////////////////////////
// GL2DVertexBuffer specialized class for 2d vertex data
///////////////////////////////////////////////////////////////////////////
class GG_API GL2DVertexBuffer final : public GLClientAndServerBufferBase<float, 2>
{
public:
    using base_t = GLClientAndServerBufferBase<float, 2>;
    GL2DVertexBuffer() = default;
    void store(const Pt pt) { store(pt.x, pt.y); }
    void store(X x, Y y) { base_t::store(static_cast<float>(Value(x)), static_cast<float>(Value(y))); }
    void store(X x, float y) { base_t::store(static_cast<float>(Value(x)), y); }
    void store(float x, Y y) { base_t::store(x, static_cast<float>(Value(y))); }
    void store(float x, float y) { base_t::store(x, y); }

    template <std::size_t ArrNx2>
    void store(std::array<float, ArrNx2> xys) { base_t::store<ArrNx2/2>(xys); }

    void activate() const override;
};

///////////////////////////////////////////////////////////////////////////
// GLTexCoordBuffer specialized class for texture coordinate data
///////////////////////////////////////////////////////////////////////////
class GG_API GLTexCoordBuffer final : public GLClientAndServerBufferBase<float, 2>
{
public:
    using base_t = GLClientAndServerBufferBase<float, 2>;
    GLTexCoordBuffer() = default;
    void store(float x, float y) { base_t::store(x, y); }
    template <std::size_t ArrNx2>
    void store(std::array<float, ArrNx2> xys) { base_t::store<ArrNx2/2>(xys); }
    void activate() const override;
};

///////////////////////////////////////////////////////////////////////////
// GL3DVertexBuffer specialized class for 3d vertex data
///////////////////////////////////////////////////////////////////////////
class GG_API GL3DVertexBuffer final : public GLClientAndServerBufferBase<float, 3>
{
public:
    using base_t = GLClientAndServerBufferBase<float, 3>;
    GL3DVertexBuffer() = default;
    void store(float x, float y, float z) { base_t::store(x, y, z); }
    template <std::size_t ArrNx3>
    void store(std::array<float, ArrNx3> xyzs) { base_t::store<ArrNx3/3>(xyzs); }
    void activate() const override;
};

///////////////////////////////////////////////////////////////////////////
// GLNormalBuffer specialized class for 3d normal data
///////////////////////////////////////////////////////////////////////////
class GG_API GLNormalBuffer final : public GLClientAndServerBufferBase<float, 3>
{
public:
    using base_t = GLClientAndServerBufferBase<float, 3>;
    GLNormalBuffer() = default;
    void store(float x, float y, float z) { base_t::store(x, y, z); }
    template <std::size_t ArrNx3>
    void store(std::array<float, ArrNx3> xyzs) { base_t::store<ArrNx3/3>(xyzs); }
    void activate() const override;
};

}


#endif
