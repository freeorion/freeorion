// -*- C++ -*-
/** Copyright 2004 T. Zachary Laine 
    Permission is granted to use this file by anyone for any purpose, without restriction. */

#ifndef _GLState_h_
#define _GLState_h_

#if defined(_MSC_VER)
#include <windows.h>
#endif
#include <GL/gl.h>

#include <string>
#include <sstream>

// overloaded glGet template
template <typename T> void GetVars(GLenum pname, T* arr);

// overloaded glGet template specializations
template <> inline void GetVars<GLboolean>(GLenum pname, GLboolean* arr) 
{
    glGetBooleanv(pname, arr);
}
template <> inline void GetVars<GLdouble>(GLenum pname, GLdouble* arr) 
{
    glGetDoublev(pname, arr);
}
template <> inline void GetVars<GLfloat>(GLenum pname, GLfloat* arr) 
{
    glGetFloatv(pname, arr);
}
template <> inline void GetVars<GLint>(GLenum pname, GLint* arr) 
{
    glGetIntegerv(pname, arr);
}

template <typename T> void DumpVal(std::stringstream& stream, T val)
{
    stream << val;
}

template <> inline void DumpVal<GLboolean>(std::stringstream& stream, GLboolean val)
{
    stream << (val == GL_FALSE ? "GL_FALSE" : "GL_TRUE");
}

// macros to generate code for the declaration and use of arrays that hold the current GL state
#define DECLARE_VARS(type, pname, sz) type m_ ## pname [ sz ];

#define GET_VARS(type, pname, sz) GetVars< type >( pname , m_ ## pname);

#define COMPARE_VARS(type, pname, sz)                                                           \
    {for (int i = 0; i < sz; ++i) {                                                             \
        if ( m_ ## pname [i] != rhs.m_ ## pname [i] )                                           \
            return false;                                                                       \
    }}

#define DUMP_VARS(type, pname, sz)                                                              \
    name = # pname ;                                                                            \
    stream << name << " = ";                                                                    \
    if (sz == 1) {                                                                              \
        DumpVal(stream, m_ ## pname [0]);                                                       \
        stream << "\n";                                                                         \
    } else if (sz == 16) {                                                                      \
        stream << "[ ";                                                                         \
        for (int j = 0; j < 4 ; ++j) {                                                          \
            for (int i = 0; i < 4 ; ++i) {                                                      \
                DumpVal(stream, m_ ## pname [j * 4 + i]);                                       \
                stream << " ";                                                                  \
            }                                                                                   \
            if (j < 3)                                                                          \
                stream << "\n" << std::string(name.size() + 5, ' ');                            \
        }                                                                                       \
        stream << "]\n";                                                                        \
    } else {                                                                                    \
        stream << "[ ";                                                                         \
        for (int i = 0; i < sz ; ++i) {                                                         \
            DumpVal(stream, m_ ## pname [i]);                                                   \
            stream << " ";                                                                      \
        }                                                                                       \
        stream << "]\n";                                                                        \
    }

#define DIFF_VARS(type, pname, sz)                                                              \
    {for (int i = 0; i < sz; ++i) {                                                             \
        if ( m_ ## pname [i] != rhs.m_ ## pname [i]) {                                          \
            stream << # pname                                                                   \
                   << (sz > 1 ? ("[" + boost::lexical_cast<std::string>(i) + "]") : "")         \
                   << " changed from ";                                                         \
            DumpVal(stream, m_ ## pname [i]);                                                   \
            stream << " to ";                                                                   \
            DumpVal(stream, rhs.m_ ## pname [i]);                                               \
            stream << "\n";                                                                     \
        }                                                                                       \
    }}

#define USE_VARLIST(usage) \
    usage (GLint, GL_ACCUM_ALPHA_BITS, 1) \
    usage (GLint, GL_ACCUM_BLUE_BITS, 1) \
    usage (GLdouble, GL_ACCUM_CLEAR_VALUE, 4) \
    usage (GLint, GL_ACCUM_GREEN_BITS, 1) \
    usage (GLint, GL_ACCUM_RED_BITS, 1) \
    usage (GLdouble, GL_ALPHA_BIAS, 1) \
    usage (GLint, GL_ALPHA_BITS, 1) \
    usage (GLdouble, GL_ALPHA_SCALE, 1) \
    usage (GLboolean, GL_ALPHA_TEST, 1) \
    usage (GLint, GL_ALPHA_TEST_FUNC, 1) \
    usage (GLdouble, GL_ALPHA_TEST_REF, 1) \
    usage (GLint, GL_ATTRIB_STACK_DEPTH, 1) \
    usage (GLboolean, GL_AUTO_NORMAL, 1) \
    usage (GLint, GL_AUX_BUFFERS, 1) \
    usage (GLboolean, GL_BLEND, 1) \
    usage (GLint, GL_BLEND_DST, 1) \
    usage (GLint, GL_BLEND_SRC, 1) \
    usage (GLdouble, GL_BLUE_BIAS, 1) \
    usage (GLint, GL_BLUE_BITS, 1) \
    usage (GLdouble, GL_BLUE_SCALE, 1) \
    usage (GLint, GL_CLIENT_ATTRIB_STACK_DEPTH, 1) \
    usage (GLboolean, GL_CLIP_PLANE0, 1) \
    usage (GLboolean, GL_CLIP_PLANE1, 1) \
    usage (GLboolean, GL_CLIP_PLANE2, 1) \
    usage (GLboolean, GL_CLIP_PLANE3, 1) \
    usage (GLboolean, GL_COLOR_ARRAY, 1) \
    usage (GLint, GL_COLOR_ARRAY_SIZE, 1) \
    usage (GLint, GL_COLOR_ARRAY_STRIDE, 1) \
    usage (GLint, GL_COLOR_ARRAY_TYPE, 1) \
    usage (GLdouble, GL_COLOR_CLEAR_VALUE, 4) \
    usage (GLboolean, GL_COLOR_LOGIC_OP, 1) \
    usage (GLboolean, GL_COLOR_MATERIAL, 1) \
    usage (GLint, GL_COLOR_MATERIAL_FACE, 1) \
    usage (GLint, GL_COLOR_MATERIAL_PARAMETER, 1) \
    usage (GLboolean, GL_COLOR_WRITEMASK, 4) \
    usage (GLboolean, GL_CULL_FACE, 1) \
    usage (GLint, GL_CULL_FACE_MODE, 1) \
    usage (GLdouble, GL_CURRENT_COLOR, 4) \
    usage (GLint, GL_CURRENT_INDEX, 1) \
    usage (GLdouble, GL_CURRENT_NORMAL, 3) \
    usage (GLdouble, GL_CURRENT_RASTER_COLOR, 4) \
    usage (GLdouble, GL_CURRENT_RASTER_DISTANCE, 1) \
    usage (GLint, GL_CURRENT_RASTER_INDEX, 1) \
    usage (GLdouble, GL_CURRENT_RASTER_POSITION, 4) \
    usage (GLboolean, GL_CURRENT_RASTER_POSITION_VALID, 1) \
    usage (GLdouble, GL_CURRENT_RASTER_TEXTURE_COORDS, 4) \
    usage (GLdouble, GL_CURRENT_TEXTURE_COORDS, 4) \
    usage (GLdouble, GL_DEPTH_BIAS, 1) \
    usage (GLint, GL_DEPTH_BITS, 1) \
    usage (GLdouble, GL_DEPTH_CLEAR_VALUE, 1) \
    usage (GLint, GL_DEPTH_FUNC, 1) \
    usage (GLdouble, GL_DEPTH_RANGE, 2) \
    usage (GLdouble, GL_DEPTH_SCALE, 1) \
    usage (GLboolean, GL_DEPTH_TEST, 1) \
    usage (GLboolean, GL_DEPTH_WRITEMASK, 1) \
    usage (GLboolean, GL_DITHER, 1) \
    usage (GLboolean, GL_DOUBLEBUFFER, 1) \
    usage (GLint, GL_DRAW_BUFFER, 1) \
    usage (GLboolean, GL_EDGE_FLAG, 1) \
    usage (GLboolean, GL_EDGE_FLAG_ARRAY, 1) \
    usage (GLint, GL_EDGE_FLAG_ARRAY_STRIDE, 1) \
    usage (GLboolean, GL_FOG, 1) \
    usage (GLdouble, GL_FOG_COLOR, 4) \
    usage (GLint, GL_FOG_DENSITY, 1) \
    usage (GLdouble, GL_FOG_END, 1) \
    usage (GLint, GL_FOG_HINT, 1) \
    usage (GLint, GL_FOG_INDEX, 1) \
    usage (GLint, GL_FOG_MODE, 1) \
    usage (GLdouble, GL_FOG_START, 1) \
    usage (GLint, GL_FRONT_FACE, 1) \
    usage (GLdouble, GL_GREEN_BIAS, 1) \
    usage (GLint, GL_GREEN_BITS, 1) \
    usage (GLdouble, GL_GREEN_SCALE, 1) \
    usage (GLboolean, GL_INDEX_ARRAY, 1) \
    usage (GLint, GL_INDEX_ARRAY_STRIDE, 1) \
    usage (GLint, GL_INDEX_ARRAY_TYPE, 1) \
    usage (GLint, GL_INDEX_BITS, 1) \
    usage (GLdouble, GL_INDEX_CLEAR_VALUE, 1) \
    usage (GLboolean, GL_INDEX_LOGIC_OP, 1) \
    usage (GLboolean, GL_INDEX_MODE, 1) \
    usage (GLint, GL_INDEX_OFFSET, 1) \
    usage (GLint, GL_INDEX_SHIFT, 1) \
    usage (GLint, GL_INDEX_WRITEMASK, 1) \
    usage (GLboolean, GL_LIGHT0, 1) \
    usage (GLboolean, GL_LIGHT1, 1) \
    usage (GLboolean, GL_LIGHT2, 1) \
    usage (GLboolean, GL_LIGHT3, 1) \
    usage (GLboolean, GL_LIGHT4, 1) \
    usage (GLboolean, GL_LIGHT5, 1) \
    usage (GLboolean, GL_LIGHT6, 1) \
    usage (GLboolean, GL_LIGHT7, 1) \
    usage (GLboolean, GL_LIGHTING, 1) \
    usage (GLdouble, GL_LIGHT_MODEL_AMBIENT, 4) \
    usage (GLboolean, GL_LIGHT_MODEL_LOCAL_VIEWER, 1) \
    usage (GLboolean, GL_LIGHT_MODEL_TWO_SIDE, 1) \
    usage (GLboolean, GL_LINE_SMOOTH, 1) \
    usage (GLint, GL_LINE_SMOOTH_HINT, 1) \
    usage (GLboolean, GL_LINE_STIPPLE, 1) \
    usage (GLint, GL_LINE_STIPPLE_PATTERN, 1) \
    usage (GLint, GL_LINE_STIPPLE_REPEAT, 1) \
    usage (GLdouble, GL_LINE_WIDTH, 1) \
    usage (GLdouble, GL_LINE_WIDTH_GRANULARITY, 1) \
    usage (GLdouble, GL_LINE_WIDTH_RANGE, 2) \
    usage (GLint, GL_LIST_BASE, 1) \
    usage (GLint, GL_LIST_INDEX, 1) \
    usage (GLint, GL_LIST_MODE, 1) \
    usage (GLint, GL_LOGIC_OP_MODE, 1) \
    usage (GLboolean, GL_MAP1_COLOR_4, 1) \
    usage (GLdouble, GL_MAP1_GRID_DOMAIN, 2) \
    usage (GLint, GL_MAP1_GRID_SEGMENTS, 1) \
    usage (GLboolean, GL_MAP1_INDEX, 1) \
    usage (GLboolean, GL_MAP1_NORMAL, 1) \
    usage (GLboolean, GL_MAP1_TEXTURE_COORD_1, 1) \
    usage (GLboolean, GL_MAP1_TEXTURE_COORD_2, 1) \
    usage (GLboolean, GL_MAP1_TEXTURE_COORD_3, 1) \
    usage (GLboolean, GL_MAP1_TEXTURE_COORD_4, 1) \
    usage (GLboolean, GL_MAP1_VERTEX_3, 1) \
    usage (GLboolean, GL_MAP1_VERTEX_4, 1) \
    usage (GLboolean, GL_MAP2_COLOR_4, 1) \
    usage (GLdouble, GL_MAP2_GRID_DOMAIN, 4) \
    usage (GLint, GL_MAP2_GRID_SEGMENTS, 2) \
    usage (GLboolean, GL_MAP2_INDEX, 1) \
    usage (GLboolean, GL_MAP2_NORMAL, 1) \
    usage (GLboolean, GL_MAP2_TEXTURE_COORD_1, 1) \
    usage (GLboolean, GL_MAP2_TEXTURE_COORD_2, 1) \
    usage (GLboolean, GL_MAP2_TEXTURE_COORD_3, 1) \
    usage (GLboolean, GL_MAP2_TEXTURE_COORD_4, 1) \
    usage (GLboolean, GL_MAP2_VERTEX_3, 1) \
    usage (GLboolean, GL_MAP2_VERTEX_4, 1) \
    usage (GLboolean, GL_MAP_COLOR, 1) \
    usage (GLboolean, GL_MAP_STENCIL, 1) \
    usage (GLint, GL_MATRIX_MODE, 1) \
    usage (GLdouble, GL_MODELVIEW_MATRIX, 16) \
    usage (GLint, GL_MODELVIEW_STACK_DEPTH, 1) \
    usage (GLint, GL_NAME_STACK_DEPTH, 1) \
    usage (GLboolean, GL_NORMAL_ARRAY, 1) \
    usage (GLint, GL_NORMAL_ARRAY_STRIDE, 1) \
    usage (GLint, GL_NORMAL_ARRAY_TYPE, 1) \
    usage (GLboolean, GL_NORMALIZE, 1) \
    usage (GLint, GL_PACK_ALIGNMENT, 1) \
    usage (GLboolean, GL_PACK_LSB_FIRST, 1) \
    usage (GLint, GL_PACK_ROW_LENGTH, 1) \
    usage (GLint, GL_PACK_SKIP_PIXELS, 1) \
    usage (GLint, GL_PACK_SKIP_ROWS, 1) \
    usage (GLboolean, GL_PACK_SWAP_BYTES, 1) \
    usage (GLint, GL_PERSPECTIVE_CORRECTION_HINT, 1) \
    usage (GLint, GL_PIXEL_MAP_A_TO_A_SIZE, 1) \
    usage (GLint, GL_PIXEL_MAP_B_TO_B_SIZE, 1) \
    usage (GLint, GL_PIXEL_MAP_G_TO_G_SIZE, 1) \
    usage (GLint, GL_PIXEL_MAP_I_TO_A_SIZE, 1) \
    usage (GLint, GL_PIXEL_MAP_I_TO_B_SIZE, 1) \
    usage (GLint, GL_PIXEL_MAP_I_TO_G_SIZE, 1) \
    usage (GLint, GL_PIXEL_MAP_I_TO_I_SIZE, 1) \
    usage (GLint, GL_PIXEL_MAP_I_TO_R_SIZE, 1) \
    usage (GLint, GL_PIXEL_MAP_R_TO_R_SIZE, 1) \
    usage (GLint, GL_PIXEL_MAP_S_TO_S_SIZE, 1) \
    usage (GLdouble, GL_POINT_SIZE, 1) \
    usage (GLdouble, GL_POINT_SIZE_GRANULARITY, 1) \
    usage (GLdouble, GL_POINT_SIZE_RANGE, 2) \
    usage (GLboolean, GL_POINT_SMOOTH, 1) \
    usage (GLint, GL_POINT_SMOOTH_HINT, 1) \
    usage (GLint, GL_POLYGON_MODE, 1) \
    usage (GLdouble, GL_POLYGON_OFFSET_FACTOR, 1) \
    usage (GLdouble, GL_POLYGON_OFFSET_UNITS, 1) \
    usage (GLboolean, GL_POLYGON_OFFSET_FILL, 1) \
    usage (GLboolean, GL_POLYGON_OFFSET_LINE, 1) \
    usage (GLboolean, GL_POLYGON_OFFSET_POINT, 1) \
    usage (GLboolean, GL_POLYGON_SMOOTH, 1) \
    usage (GLint, GL_POLYGON_SMOOTH_HINT, 1) \
    usage (GLboolean, GL_POLYGON_STIPPLE, 1) \
    usage (GLdouble, GL_PROJECTION_MATRIX, 16) \
    usage (GLint, GL_PROJECTION_STACK_DEPTH, 1) \
    usage (GLint, GL_READ_BUFFER, 1) \
    usage (GLdouble, GL_RED_BIAS, 1) \
    usage (GLint, GL_RED_BITS, 1) \
    usage (GLdouble, GL_RED_SCALE, 1) \
    usage (GLint, GL_RENDER_MODE, 1) \
    usage (GLboolean, GL_RGBA_MODE, 1) \
    usage (GLdouble, GL_SCISSOR_BOX, 4) \
    usage (GLboolean, GL_SCISSOR_TEST, 1) \
    usage (GLint, GL_SHADE_MODEL, 1) \
    usage (GLint, GL_STENCIL_BITS, 1) \
    usage (GLint, GL_STENCIL_CLEAR_VALUE, 1) \
    usage (GLint, GL_STENCIL_FAIL, 1) \
    usage (GLint, GL_STENCIL_FUNC, 1) \
    usage (GLint, GL_STENCIL_PASS_DEPTH_FAIL, 1) \
    usage (GLint, GL_STENCIL_PASS_DEPTH_PASS, 1) \
    usage (GLdouble, GL_STENCIL_REF, 1) \
    usage (GLboolean, GL_STENCIL_TEST, 1) \
    usage (GLint, GL_STENCIL_VALUE_MASK, 1) \
    usage (GLint, GL_STENCIL_WRITEMASK, 1) \
    usage (GLboolean, GL_STEREO, 1) \
    usage (GLint, GL_SUBPIXEL_BITS, 1) \
    usage (GLboolean, GL_TEXTURE_1D, 1) \
    usage (GLboolean, GL_TEXTURE_2D, 1) \
    usage (GLboolean, GL_TEXTURE_COORD_ARRAY, 1) \
    usage (GLint, GL_TEXTURE_COORD_ARRAY_SIZE, 1) \
    usage (GLint, GL_TEXTURE_COORD_ARRAY_STRIDE, 1) \
    usage (GLint, GL_TEXTURE_COORD_ARRAY_TYPE, 1) \
    usage (GLboolean, GL_TEXTURE_GEN_Q, 1) \
    usage (GLboolean, GL_TEXTURE_GEN_R, 1) \
    usage (GLboolean, GL_TEXTURE_GEN_S, 1) \
    usage (GLboolean, GL_TEXTURE_GEN_T, 1) \
    usage (GLdouble, GL_TEXTURE_MATRIX, 16) \
    usage (GLint, GL_TEXTURE_STACK_DEPTH, 1) \
    usage (GLint, GL_UNPACK_ALIGNMENT, 1) \
    usage (GLboolean, GL_UNPACK_LSB_FIRST, 1) \
    usage (GLint, GL_UNPACK_ROW_LENGTH, 1) \
    usage (GLint, GL_UNPACK_SKIP_PIXELS, 1) \
    usage (GLint, GL_UNPACK_SKIP_ROWS, 1) \
    usage (GLboolean, GL_UNPACK_SWAP_BYTES, 1) \
    usage (GLboolean, GL_VERTEX_ARRAY, 1) \
    usage (GLint, GL_VERTEX_ARRAY_SIZE, 1) \
    usage (GLint, GL_VERTEX_ARRAY_STRIDE, 1) \
    usage (GLint, GL_VERTEX_ARRAY_TYPE, 1) \
    usage (GLint, GL_VIEWPORT, 4) \
    usage (GLdouble, GL_ZOOM_X, 1) \
    usage (GLdouble, GL_ZOOM_Y, 1)


/** takes a snapshot of the GL state for comparison to other such snapshots. */
class GLStateComparator
{
public:
    /** default ctor. */
    GLStateComparator()
    {
    }

    /** ctor. Note that results are undefined if this ctor is used before GL is initialized. */
    GLStateComparator(bool grab_state)
    {
        if (grab_state)
            GrabState();
    }

    /** produces a list of the stored GL state values. */
    std::string Dump() const
    {
        std::stringstream stream;
        std::string name;
        USE_VARLIST(DUMP_VARS)
        return stream.str();
    }

    /** produces a list changes from this object to \a rhs. */
    std::string Diff(const GLStateComparator& rhs) const
    {
        std::stringstream stream;
        USE_VARLIST(DIFF_VARS)
        return stream.str();
    }

    /** returns true iff the values stored in this object are identical to those stored in \a rhs. */
    bool operator==(const GLStateComparator& rhs) const
    {
        USE_VARLIST(COMPARE_VARS)
        return true;
    }

    /** returns true iff the values stored in this object differ from those stored in \a rhs. */
    bool operator!=(const GLStateComparator& rhs) const
    {
        return !(*this == rhs);
    }

    /** takes and stores a snapshot of the GL state.  Note that results are undefined if this function 
        is called before GL is initialized. */
    void GrabState()
    {
        USE_VARLIST(GET_VARS)
    }

private:
    USE_VARLIST(DECLARE_VARS)
};

#undef DECLARE_VARS
#undef GET_VARS
#undef DUMP_VARS
#undef DIFF_VARS
#undef USE_VARLIST

#endif // _GLState_h_
