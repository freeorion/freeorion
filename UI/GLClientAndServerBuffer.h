// -*- C++ -*-
#ifndef _GLClientAndServerBuffer_h_
#define _GLClientAndServerBuffer_h_

#ifdef _MSC_VER
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
#endif

/*
 *  GLClientAndServerBuffer.h
 *  FreeOrion
 *
 *  Created by Rainer Kupke on 06.02.11.
 *  Copyright 2011. All rights reserved.
 *
 */

// include OpenGL headers
#if defined(__APPLE__) && defined(__MACH__)
# include <OpenGL/gl.h>
#else
# include <GL/gl.h>
#endif

#include <vector>

///////////////////////////////////////////////////////////////////////////
// GLBufferBase common base class for Buffer classes
///////////////////////////////////////////////////////////////////////////

class GLBufferBase
{
public:
    GLBufferBase();             // ctor
    virtual ~GLBufferBase();    // dtor,    required to automatically drop  
                                //          server buffer in case of delete
    
    // use this if you want to make sure that two buffers both 
    // have server buffers or not, drops the buffer for mixed cases
    void harmonizeBufferType(GLBufferBase& other);
    
protected:
    // drops the server buffer if one exists
    void dropServerBuffer();
    

    GLuint      b_name;
};

///////////////////////////////////////////////////////////////////////////
// GLClientAndServerBufferBase 
// template class for buffers with different types of content
///////////////////////////////////////////////////////////////////////////

template <class vtype> 
class GLClientAndServerBufferBase : public GLBufferBase 
{
private:
    GLClientAndServerBufferBase(); // default ctor forbidden, 
                                   // buffer needs to know number 
                                   // of elements per item
public:
    GLClientAndServerBufferBase(std::size_t elementsPerItem);
    std::size_t size() const;
    
    // store items, buffers usually store tupels, convenience functions
    // do not use while server buffer exists
    void store(vtype item);                                     
    void store(vtype item1,vtype item2);                        
    void store(vtype item1,vtype item2,vtype item3);
    void store(vtype item1,vtype item2,vtype item3,vtype item4);
    
    // try to store the buffered data in a server buffer
    void createServerBuffer(void);
    
    // drops a server buffer if one exists,
    // clears the client side buffer
    void clear(void);
    
protected:
    std::vector<vtype> b_data;
    std::size_t b_size;
    std::size_t b_elementsPerItem;
    
    // used in derived classes to activate the buffer
    // implementations should use glBindBuffer, gl...Pointer if
    // server buffer exists (b_name!=0), just gl...Pointer otherwise
    virtual void activate(void) const =0;
};

///////////////////////////////////////////////////////////////////////////
// GLRGBAColorBuffer specialized class for RGBA color values
///////////////////////////////////////////////////////////////////////////

class GLRGBAColorBuffer : public GLClientAndServerBufferBase<unsigned char>
{
public:
    GLRGBAColorBuffer();
    
    void activate(void) const;
};

///////////////////////////////////////////////////////////////////////////
// GL2DVertexBuffer specialized class for 2d vertex data
///////////////////////////////////////////////////////////////////////////

class GL2DVertexBuffer : public GLClientAndServerBufferBase<float>
{
public:
    GL2DVertexBuffer();
    
    void activate(void) const;
};

///////////////////////////////////////////////////////////////////////////
// GLTexCoordBuffer specialized class for texture coordinate data
///////////////////////////////////////////////////////////////////////////

class GLTexCoordBuffer : public GLClientAndServerBufferBase<float>
{
public:
    GLTexCoordBuffer();
    
    void activate(void) const;
};

#endif
