/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/**************************************************************************************************/
#if 0 // TODO
#include <GG/adobe/future/drag_and_drop.hpp>

#include <utility>
#include <map>

#include <GG/adobe/algorithm/sort.hpp>
#include <GG/adobe/algorithm/unique.hpp>

/**************************************************************************************************/

namespace {

/**************************************************************************************************/

typedef std::pair<const std::type_info*, adobe::dnd_flavor_extractor_proc_t> extractor_data_type;
typedef std::multimap<boost::uint32_t, extractor_data_type>                  extractor_set_t;

/**************************************************************************************************/

typedef std::pair<const std::type_info*, adobe::poly_drag_and_drop_converter_t> converter_data_type;
typedef std::multimap<boost::uint32_t, converter_data_type>                     converter_set_t;

/**************************************************************************************************/

extractor_set_t& extractor_set()
{
    static extractor_set_t extractor_set_s;

    return extractor_set_s;
}

/**************************************************************************************************/

converter_set_t& converter_set()
{
    static converter_set_t converter_set_s;

    return converter_set_s;
}

/**************************************************************************************************/

extractor_set_t::const_iterator find_extractor(boost::uint32_t       flavor,
                                               const std::type_info& target_type_info)
{
    typedef std::pair<extractor_set_t::const_iterator, extractor_set_t::const_iterator> range_type;

    range_type range(extractor_set().equal_range(flavor));

    while (range.first != range.second)
    {
        const std::type_info* cur(range.first->second.first); 

        if (cur == &target_type_info)
            break;

        ++range.first;
    }

    return range.first == range.second ? extractor_set().end() : range.first;
}

/**************************************************************************************************/

converter_set_t::const_iterator find_converter(boost::uint32_t       flavor,
                                               const std::type_info& target_type_info)
{
    typedef std::pair<converter_set_t::const_iterator, converter_set_t::const_iterator> range_type;

    range_type range(converter_set().equal_range(flavor));

    while (range.first != range.second)
    {
        const std::type_info* cur(range.first->second.first); 

        if (cur == &target_type_info)
            break;

        ++range.first;
    }

    return range.first == range.second ? converter_set().end() : range.first;
}

/**************************************************************************************************/

} // namespace

/**************************************************************************************************/

namespace adobe {

/**************************************************************************************************/

const boost::uint32_t flavor_file          = 'hfs ';
const boost::uint32_t flavor_file_url      = 'furl';
const boost::uint32_t flavor_future_file   = 'fssP';
const boost::uint32_t flavor_image         = 'PICT';
const boost::uint32_t flavor_movie         = 'moov';
const boost::uint32_t flavor_sound         = 'snd ';
const boost::uint32_t flavor_text          = 'TEXT';
const boost::uint32_t flavor_text_style    = 'styl';
const boost::uint32_t flavor_unicode       = 'utxt';
const boost::uint32_t flavor_unicode_style = 'ustl';

const boost::uint32_t flavor_invalid = static_cast<boost::uint32_t>(-1);

/**************************************************************************************************/

void register_dnd_converter(boost::uint32_t                       flavor,
                            const std::type_info&                 target_type_info,
                            const poly_drag_and_drop_converter_t& converter)
{
    converter_set().insert(converter_set_t::value_type(flavor,
                                                       converter_data_type(&target_type_info,
                                                                           converter)));
}

bool is_dnd_converter_registered(boost::uint32_t flavor, const std::type_info& target_type_info)
{
    return find_converter(flavor, target_type_info) != converter_set().end();
}

any_regular_t dnd_converter_invoke(boost::uint32_t       flavor,
                                   const std::type_info& target_type_info,
                                   const any_regular_t&  raw_value)
{
    converter_set_t::const_iterator found(find_converter(flavor, target_type_info));

    if (found == converter_set().end())
        throw std::runtime_error("Converter not found");

    const poly_drag_and_drop_converter_t& converter(found->second.second);

    return converter.convert(raw_value);
}

std::vector<boost::uint32_t> registered_flavor_set()
{
    std::vector<boost::uint32_t> result;

    converter_set_t::iterator first(converter_set().begin());
    converter_set_t::iterator last(converter_set().end());

    while (first != last)
    {
        result.push_back(first->first);

        ++first;
    }

    adobe::sort(result);

    result.erase(adobe::unique(result), result.end());

    return result;
}

/**************************************************************************************************/

void register_dnd_extractor(boost::uint32_t                    flavor,
                            const std::type_info&              target_type_info,
                            const dnd_flavor_extractor_proc_t& extractor)
{
    extractor_set().insert(extractor_set_t::value_type(flavor,
                                                       extractor_data_type(&target_type_info,
                                                                           extractor)));
}

bool is_dnd_extractor_registered(boost::uint32_t flavor, const std::type_info& target_type_info)
{
    return find_extractor(flavor, target_type_info) != extractor_set().end();
}

any_regular_t dnd_extractor_invoke(boost::uint32_t       flavor,
                                   const std::type_info& target_type_info,
                                   const dictionary_t&   parameters)
{
    extractor_set_t::const_iterator found(find_extractor(flavor, target_type_info));

    if (found == extractor_set().end())
        throw std::runtime_error("Extractor not found");

    const dnd_flavor_extractor_proc_t& extractor(found->second.second);

    return extractor(parameters);
}

/**************************************************************************************************/

} // namespace adobe

/**************************************************************************************************/
#endif
