/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#include <GG/adobe/future/resources.hpp>

#include <GG/adobe/string.hpp>

#include <boost/filesystem/operations.hpp>

#include <stdexcept>
#include <vector>

/****************************************************************************************************/

namespace {

/****************************************************************************************************/

typedef std::vector<boost::filesystem::path> stack_type;

/****************************************************************************************************/

stack_type& root_path()
{
    static stack_type root_s;

    return root_s;
}

/****************************************************************************************************/

} // namespace

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

void push_resource_root_path(const boost::filesystem::path& root)
{
    root_path().push_back(root);
}

/****************************************************************************************************/

void pop_resource_root_path()
{
    root_path().pop_back();
}

/****************************************************************************************************/

boost::filesystem::path find_resource(const boost::filesystem::path& name)
{
    stack_type&                  stack(root_path());
    stack_type::reverse_iterator iter(stack.rbegin());
    stack_type::reverse_iterator last(stack.rend());

    for (; iter != last; ++iter)
    {
        boost::filesystem::path candidate(*iter / name);

        if (boost::filesystem::exists(candidate))
            return candidate;
    }

    std::string err;

    err << "Could not locate resource \"" << name.string() << "\" in any resource path.";

    throw std::runtime_error(err);
}

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/
