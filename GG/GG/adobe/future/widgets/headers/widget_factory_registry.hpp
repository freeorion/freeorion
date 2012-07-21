/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_WIDGET_FACTORY_REGISTRY_HPP
#define ADOBE_WIDGET_FACTORY_REGISTRY_HPP

/*************************************************************************************************/

#include <GG/adobe/dictionary.hpp>
#include <GG/adobe/name.hpp>
#include <GG/adobe/layout_attributes.hpp>

#include <boost/function.hpp>
#include <boost/tuple/tuple.hpp>

#include <map>

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

struct widget_node_t;
struct factory_token_t;

/*************************************************************************************************/

class widget_factory_t
{
public:
    typedef boost::function<widget_node_t(const dictionary_t&     parameters,
                                          const widget_node_t&    parent,
                                          const factory_token_t&  token,
                                          const widget_factory_t& factory)> widget_factory_method_t;
    typedef boost::tuple<widget_factory_method_t,
                         bool,
                         layout_attributes_t> data_type;
    typedef name_t                            key_type;
    typedef std::map<key_type, data_type>     map_type;
    typedef map_type::value_type              value_type;

    void reg(name_t           name,
             const data_type& data)
    {  map_m[name] = data; }

    void reg(name_t                     name,
             widget_factory_method_t    method,
             bool                       is_container = false,
             const layout_attributes_t& layout_attributes = layout_attributes_t())
    {  map_m[name] = data_type(method, is_container, layout_attributes); }

    bool is_reg(name_t name) const
    { return find(name, nothrow()) == map_m.end(); }

    void unreg(name_t name)
    { map_m.erase(find(name, noconst())); }

    const widget_factory_method_t& method(name_t name) const
    { return find(name)->second.get<0>(); }

    bool is_container(name_t name) const
    { return find(name)->second.get<1>(); }

    const layout_attributes_t& layout_attributes(name_t name) const
    { return find(name)->second.get<2>(); }

    /*!
        This throws if \c name is not a registered widget
    */
    map_type::const_iterator find(name_t name) const
    { return find(name, noconst()); }

private:
    struct nothrow { };
    struct noconst { };

    map_type::iterator find(name_t name, nothrow) const
    { return const_cast<map_type&>(map_m).find(name); }

    /*!
        This throws if \c name is not a registered widget
    */
    map_type::iterator find(name_t name, noconst) const;

    map_type map_m;
};

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

#endif

/*************************************************************************************************/
