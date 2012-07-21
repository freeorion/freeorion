/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_FILE_SLURP_HPP
#define ADOBE_FILE_SLURP_HPP

/*************************************************************************************************/

#include <GG/adobe/config.hpp>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>

#include <boost/noncopyable.hpp>

#include <stdexcept>

/****************************************************************************************************/

// REVISIT (fbrereto) : Eventually I'd like to get this file slurper to the point where
//                      it reads 64k blocks out of the file as it needs to, instead of
//                      slurping the file all at once.

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

template <typename T>
struct file_slurp : boost::noncopyable
{
    typedef T*          store_type;
    typedef std::size_t size_type;
    typedef T           value_type;
    typedef T*          iterator;
    typedef const T*    const_iterator;

    explicit file_slurp(const boost::filesystem::path& path) :
        contents_m(0), size_m(0), path_m(path)
    { reslurp(); }

    ~file_slurp()
    { if (contents_m) delete [] contents_m; }

    iterator begin() { return &contents_m[0]; }
    iterator end()   { return begin() + size(); }

    const_iterator begin() const { return &contents_m[0]; }
    const_iterator end() const   { return begin() + size(); }

    bool        empty() const { return size() == 0; }
    size_type   size() const  { return size_m; }

    T*          c_str() { return contents_m; }

    T*          release()
    {
        T* result(contents_m);

        contents_m = 0;

        return result;
    }

    void reslurp();

private:
    T*                      contents_m;
    size_type               size_m;
    boost::filesystem::path path_m;
};

/****************************************************************************************************/

template <typename T>
void file_slurp<T>::reslurp()
{
    if (contents_m)
        return;

    size_m = 0;

    boost::intmax_t size(boost::filesystem::file_size(path_m));

    contents_m = new T[static_cast<std::size_t>(size) + 1];

    if (size == 0)
        return;

    boost::filesystem::ifstream in(path_m, std::ios_base::in | std::ios_base::binary);

    if (!in.is_open())
#if 1
        throw std::runtime_error("file slurp: failed to open file");
#else
        return;
#endif

    in.unsetf(std::ios_base::skipws);

    // read in max 64K at a time
    std::size_t buffer_size(64*1024);

    if (size < buffer_size)
        buffer_size = static_cast<std::size_t>(size);

    while (true)
    {
        in.read(&contents_m[size_m], static_cast<std::streamsize>(buffer_size));

        std::size_t count(static_cast<std::size_t>(in.gcount()));

        size_m += count;

        if (count != buffer_size)
            break;
    }

    contents_m[size_m] = 0;
}

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/

#endif

/****************************************************************************************************/
