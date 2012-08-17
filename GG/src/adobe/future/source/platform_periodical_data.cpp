/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#include <GG/adobe/future/platform_periodical_data.hpp>

#include <vector>

/****************************************************************************************************/

namespace {

/****************************************************************************************************/

void timer_callback(adobe::periodical_platform_data_t& impl)
{
    assert(impl.fire_m);

    impl.fire_m();
}

/****************************************************************************************************/

} // namespace

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

periodical_platform_data_t::periodical_platform_data_t(const fire_proc_t& fire_proc,
                                                       std::size_t        millisecond_delay) :
    timer_m(new GG::Timer(millisecond_delay)),
    fire_m(fire_proc)
{ GG::Connect(timer_m->FiredSignal, boost::bind(&timer_callback, *this)); }

/****************************************************************************************************/

periodical_platform_data_t::~periodical_platform_data_t()
{}

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/
