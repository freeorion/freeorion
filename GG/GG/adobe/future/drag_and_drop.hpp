/*
    Copyright 2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/**************************************************************************************************/

#ifndef ADOBE_DRAG_AND_DROP_HPP
#define ADOBE_DRAG_AND_DROP_HPP

/**************************************************************************************************/

#include <GG/adobe/config.hpp>

#include <GG/adobe/future/drag_and_drop_fwd.hpp>
#include <GG/adobe/future/platform_drag_and_drop_data.hpp>

#include <iterator>
#include <set>
#include <typeinfo>

#include <boost/cstdint.hpp>

/**************************************************************************************************/

namespace adobe {

/**************************************************************************************************/
/*!
    The drag and drop system is responsible for
        - being aware of drag-and-drop events
        - knowing what types ('flavors') of data the client to which it is bound can accept
        - proritizing which flavors are desired above others
        - converting the flavored-data to a specific target type representation
        - notifying the client of the newly dropped representative data
*/
template <typename SourceType, typename TargetType>
class drag_and_drop_handler
{
public:
    typedef SourceType                                                    source_type;
    typedef TargetType                                                    target_type;
    typedef drag_and_drop_handler_platform_data<source_type, target_type> platform_data_type;
    typedef boost::function<void (const target_type&)>                    client_callback_proc_t;

    drag_and_drop_handler()
    {
        data_m.init();
    }

    template <typename InputIterator>
    drag_and_drop_handler(InputIterator first, InputIterator last) :
        data_m(first, last)
    {
        data_m.init();
    }

    ~drag_and_drop_handler()
    {
        data_m.detach();
    }

    template <typename Client>
    inline void attach(const Client& client, const client_callback_proc_t& proc)
    {
        data_m.attach(client, proc);
    }

    inline void insert_flavor(boost::uint32_t flavor)
    {
        data_m.insert_flavor(flavor);
    }

    inline void erase_flavor(boost::uint32_t flavor)
    {
        data_m.erase_flavor(flavor);
    }

    platform_data_type data_m;
};

/**************************************************************************************************/

} // namespace adobe

/**************************************************************************************************/

// ADOBE_DRAG_AND_DROP_HPP
#endif

/**************************************************************************************************/
