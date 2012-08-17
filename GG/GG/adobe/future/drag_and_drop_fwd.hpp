/*
    Copyright 2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/**************************************************************************************************/

#ifndef ADOBE_DRAG_AND_DROP_FWD_HPP
#define ADOBE_DRAG_AND_DROP_FWD_HPP

/**************************************************************************************************/

#include <GG/adobe/config.hpp>

#include <vector>

#include <boost/cstdint.hpp>
#include <boost/function.hpp>

#include <GG/adobe/any_regular.hpp>
#include <GG/adobe/dictionary.hpp>
#include <GG/adobe/future/poly_drag_and_drop_converter.hpp>

/**************************************************************************************************/

namespace adobe {

/**************************************************************************************************/
/*!
    Denotes commonly used "flavors" of acceptable data in the drag and drop system.
*/
extern const boost::uint32_t flavor_file_url;
extern const boost::uint32_t flavor_file;
extern const boost::uint32_t flavor_future_file;
extern const boost::uint32_t flavor_image;
extern const boost::uint32_t flavor_movie;
extern const boost::uint32_t flavor_sound;
extern const boost::uint32_t flavor_text_style;
extern const boost::uint32_t flavor_text;
extern const boost::uint32_t flavor_unicode_style;
extern const boost::uint32_t flavor_unicode;

extern const boost::uint32_t flavor_invalid;

/**************************************************************************************************/

void register_dnd_converter(boost::uint32_t                       flavor,
                            const std::type_info&                 target_type_info,
                            const poly_drag_and_drop_converter_t& converter);

template <typename T>
inline void register_dnd_converter(boost::uint32_t                      flavor,
                                   const poly_drag_and_drop_converter_t& converter)
{ register_dnd_converter(flavor, typeid(T), converter); }

bool is_dnd_converter_registered(boost::uint32_t flavor, const std::type_info& target_type_info);

template <typename T>
inline bool is_dnd_converter_registered(boost::uint32_t flavor)
{ return is_dnd_converter_registered(flavor, typeid(T)); }

/**************************************************************************************************/

any_regular_t dnd_converter_invoke(boost::uint32_t       flavor,
                                   const std::type_info& target_type_info,
                                   const any_regular_t&  raw_value);

template <typename DestType, typename SourceType>
inline DestType invoke_dnd_converter(boost::uint32_t flavor, const SourceType& raw_value)
{
    return dnd_converter_invoke(flavor,
                                typeid(DestType),
                                any_regular_t(raw_value)).cast<DestType>();
}

template <typename DestType, typename SourceType>
bool invoke_dnd_converter(boost::uint32_t flavor, const SourceType& raw_value, DestType& result)
{
    return dnd_converter_invoke(flavor,
                                typeid(DestType),
                                any_regular_t(raw_value)).cast<DestType>(result);
}

/**************************************************************************************************/

std::vector<boost::uint32_t> registered_flavor_set();

/**************************************************************************************************/

/*!
    REVISIT (fbrereto) : I am not pleased with the dictionary_t here for the argument set; it is too
    broad a specification. What it should be is an argument set of the platform-specific items
    needed to pull the raw flavor data out of the flavor in the drag and drop behavior. However such
    platform dependencies I am even less pleased with, so I consider this the lesser of two evils.
    Should a generic solution be found, it would be good to collapse this functionality into the
    DragAndDropConverterConcept definition, so we can have instances of that concept that take a
    piece of drag and drop flavor data and process it from start (raw OS stuff) to finish (data in
    the format the client is expecting, including post-extraction conversion.)
*/
typedef boost::function<any_regular_t (const dictionary_t&)> dnd_flavor_extractor_proc_t;

void register_dnd_extractor(boost::uint32_t                    flavor,
                            const std::type_info&              target_type_info,
                            const dnd_flavor_extractor_proc_t& extractor);

template <typename T>
inline void register_dnd_extractor(boost::uint32_t                    flavor,
                                   const dnd_flavor_extractor_proc_t& extractor)
{ register_dnd_extractor(flavor, typeid(T), extractor); }

bool is_dnd_extractor_registered(boost::uint32_t flavor, const std::type_info& target_type_info);

template <typename T>
inline bool is_dnd_extractor_registered(boost::uint32_t flavor)
{ return is_dnd_extractor_registered(flavor, typeid(T)); }

any_regular_t dnd_extractor_invoke(boost::uint32_t       flavor,
                                   const std::type_info& target_type_info,
                                   const dictionary_t&   drag_parameters);

template <typename DestType>
inline DestType invoke_dnd_extractor(boost::uint32_t flavor, const dictionary_t& drag_parameters)
{
    return dnd_extractor_invoke(flavor,
                                typeid(DestType),
                                drag_parameters).cast<DestType>();
}

template <typename DestType>
bool invoke_dnd_extractor(boost::uint32_t flavor, const dictionary_t& drag_parameters, DestType& result)
{
    return dnd_extractor_invoke(flavor,
                                typeid(DestType),
                                drag_parameters).cast<DestType>(result);
}

/**************************************************************************************************/

} // namespace adobe

/**************************************************************************************************/

// ADOBE_DRAG_AND_DROP_FWD_HPP
#endif

/**************************************************************************************************/
