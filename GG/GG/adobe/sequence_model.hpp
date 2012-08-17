/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/******************************************************************************/

#ifndef ADOBE_SEQUENCE_MODEL_HPP
#define ADOBE_SEQUENCE_MODEL_HPP

/******************************************************************************/

#include <list>
#ifdef ADOBE_STD_SERIALIZATION
    #include <iostream>
#endif

#include <boost/bind.hpp>
#include <boost/next_prior.hpp>
#include <boost/operators.hpp>
#include <boost/static_assert.hpp>

#include <GG/adobe/algorithm/copy.hpp>
#include <GG/adobe/algorithm/find.hpp>
#include <GG/adobe/algorithm/for_each.hpp>
#include <GG/adobe/copy_on_write.hpp>
#include <GG/adobe/poly_sequence_controller.hpp>
#include <GG/adobe/poly_sequence_view.hpp>
#include <GG/adobe/poly_sequence_model.hpp>
#include <GG/adobe/sequence_model_fwd.hpp>
#include <GG/adobe/typeinfo.hpp>
#include <GG/adobe/vector.hpp>

/******************************************************************************/

namespace adobe {

/******************************************************************************/

template <typename T>
class sequence_model
{
public:
    /// value_type for the sequence_model
    typedef T                                          value_type;
    /// size_type for the sequence_model
    typedef std::size_t                                size_type;
    /// cow_value_type for the sequence_model
    typedef copy_on_write<value_type>                  cow_value_type;
    /// key_type for the sequence_model
    typedef sequence_key<T>                            key_type;
    /// view_type for the sequence_model
    typedef typename poly_sequence_view<T>::type       poly_sequence_view_type;
    /// view_type for the sequence_model
    typedef typename poly_sequence_controller<T>::type poly_sequence_controller_type;

    BOOST_STATIC_ASSERT((sizeof(key_type) == sizeof(void*)));

    static bool interface_requires_std_rtti()
    { return adobe::type_info<T>().requires_std_rtti(); }

    static cow_value_type at(key_type key)
    { return *key.value_m; }

    void push_back(const value_type& value)
    {
        storage_m.push_back(cow_value_type(value));

        for_each(view_set_m, boost::bind(&poly_sequence_view_type::extend,
                                         _1,
                                         key_type::nkey,
                                         key_type(*--storage_m.end()),
                                         boost::cref(storage_m.back())));
    }

    void set(key_type key, const value_type& value)
    {
        if (!is_valid(key))
            return;

        storage_iterator iter(iterator_for(key));

        *iter = cow_value_type(value);

        for_each(view_set_m, boost::bind(&poly_sequence_view_type::refresh,
                                         _1,
                                         key,
                                         boost::cref(*iter)));
    }

    void insert_set(key_type before, const vector<value_type>& value_set)
    {
        storage_iterator before_iterator(iterator_for(before));
        vector<key_type> extend_key_set;

        for (typename vector<value_type>::const_iterator iter(value_set.begin()),
             last(value_set.end()); iter != last; ++iter)
        {
            storage_iterator result(storage_m.insert(before_iterator, cow_value_type(*iter)));

            extend_key_set.push_back(key_type(*result));
        }

        for_each(view_set_m, boost::bind(&poly_sequence_view_type::extend_set,
                                         _1,
                                         before,
                                         extend_key_set));
    }

    void insert(key_type before, const value_type& value)
    {
        storage_iterator result(storage_m.insert(iterator_for(before),
                                                 cow_value_type(value)));

        for_each(view_set_m, boost::bind(&poly_sequence_view_type::extend,
                                         _1,
                                         before,
                                         key_type(*result),
                                         boost::cref(*result)));
    }

    void erase(const vector<key_type>& key_set)
    {
        typedef typename vector<key_type>::const_iterator const_iterator;

        for (const_iterator iter(key_set.begin()), last(key_set.end());
             iter != last; ++iter)
            storage_m.erase(iterator_for(*iter));

        for_each(view_set_m, boost::bind(&poly_sequence_view_type::erase,
                                         _1,
                                         boost::cref(key_set)));
    }

    void clear()
    {
        storage_m.clear();

        for_each(view_set_m, boost::bind(&poly_sequence_view_type::clear,
                                         _1));
    }

    /*!
        Used to attach a poly_sequence_view_type to the sequence_model.
        Views into the model are notified of model changes through the
        concept requirements spelled out by the SequenceView concept.
    */
    void attach_view(poly_sequence_view_type& view);
    /*!
        Detaches the requested view from the sequence_model.
    */
    void detach_view(poly_sequence_view_type& view);

    /*!
        Used to attach a poly_sequence_controller_type to the
        sequence_model. Controllers of the model send model change
        notifications through the concept requirements spelled out by
        the SequenceController concept.
    */
    void attach_controller(poly_sequence_controller_type& controller);
    /*!
        Detaches the requested controller from the sequence_model.
    */
    void detach_controller(poly_sequence_controller_type& controller);

private:
    #ifndef ADOBE_NO_DOCUMENTATION

    typedef std::list<cow_value_type>       storage_type;
    typedef typename storage_type::iterator storage_iterator;

    static bool is_valid(key_type k)
    { return k != key_type::nkey; }

    size_type size() const { return storage_m.size(); }
    bool      empty() const { return storage_m.empty(); }

    storage_iterator iterator_for(key_type key) const
    {
        storage_type& storage(const_cast<storage_type&>(storage_m));

        if (!is_valid(key))
            return storage.end();

        // Ugh -- linear search through the list to find the key.
        //
        // REVISIT (fbrereto) : Is there any way to make this faster? Perhaps a
        //                      cache of some kind to binary-search the key set,
        //                      retrieving an iterator? Using table_index?

        for (storage_iterator iter(storage.begin()), last(storage.end());
             iter != last; ++iter)
            if (key_type(*iter) == key)
                return iter;

        return storage.end();
    }

    typedef vector<poly_sequence_view_type*>       view_set_t;
    typedef vector<poly_sequence_controller_type*> controller_set_t;

    storage_type                           storage_m;
    view_set_t                             view_set_m;
    controller_set_t                       controller_set_m;
    auto_ptr<typename poly_sequence_model<T>::type> poly_m;

    // ADOBE_NO_DOCUMENTATION
    #endif
};

/******************************************************************************/

template <typename T>
void sequence_model<T>::attach_view(poly_sequence_view_type& view)
{
        typename view_set_t::iterator found(adobe::find(view_set_m, &view));

    if (found != view_set_m.end())
        return;

    view_set_m.push_back(&view);

    view.clear();

    if (empty())
        return;

    // tell the view everything possible about the current state of the model

    vector<key_type> extend_key_set;

    for (storage_iterator iter(storage_m.begin()), last(storage_m.end());
         iter != last; ++iter)
        extend_key_set.push_back(key_type(*iter));

    view.extend_set(key_type::nkey, extend_key_set);
}

/******************************************************************************/

template <typename T>
void sequence_model<T>::detach_view(poly_sequence_view_type& view)
{
        typename view_set_t::iterator found(adobe::find(view_set_m, &view));

    if (found == view_set_m.end())
        return;

    view_set_m.erase(found);
}

/******************************************************************************/

template <typename T>
void sequence_model<T>::attach_controller(poly_sequence_controller_type& controller)
{
        typename controller_set_t::iterator found(adobe::find(controller_set_m, &controller));

    if (found != controller_set_m.end())
        return;

    controller_set_m.push_back(&controller);

    if (poly_m.get() == 0)
        poly_m.reset(new typename poly_sequence_model<T>::type(boost::ref(*this)));

    controller.monitor_sequence(*poly_m);
}

/******************************************************************************/

template <typename T>
void sequence_model<T>::detach_controller(poly_sequence_controller_type& controller)
{
        typename controller_set_t::iterator found(adobe::find(controller_set_m, &controller));

    if (found == controller_set_m.end())
        return;

    controller_set_m.erase(found);
}

/******************************************************************************/

} // namespace adobe

/******************************************************************************/
// ADOBE_SEQUENCE_MODEL_HPP
#endif

/******************************************************************************/
