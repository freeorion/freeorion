/*
        Copyright 2008 Adobe Systems Incorporated
        Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
        or a copy at http://stlab.adobe.com/licenses.html)
*/

/******************************************************************************/

#ifndef ADOBE_SEQUENCE_MVC_MULDEX
#define ADOBE_SEQUENCE_MVC_MULDEX

/******************************************************************************/

#include <boost/static_assert.hpp>

#include <GG/adobe/function_pack.hpp>
#include <GG/adobe/sheet_hooks.hpp>
#include <GG/adobe/selection.hpp>
#include <GG/adobe/sequence_model.hpp>
#include <GG/adobe/zuid.hpp>

/******************************************************************************/

namespace adobe {

/******************************************************************************/
/*!
    OVERVIEW:

    This entire header handles a single problem: How do you communicate
    to other model types (specifically here, a sequence_model) when all
    you have available to you is the property model library (Adam) to
    which you can hook up your widgets? In other words, given that the
    property model library is the only 'interface' you can use to send
    and receive information to/from your widgets, how do you get data to
    those widgets that is not well-handled by the property model?
    
    A good example of this is the list widget. We have to account for
    issues with sequences that the property model library does not
    handle well. (e.g., lists with large item sets that are expensive to
    copy, sending notifications only about items in a list that have
    changed, and setting information about a subset of the items in the
    list and not the list as a whole.)
    
    The solution here is to send sequence model/view/controller commands
    _through_ the property model library. We set aside two cells in the
    PML and farm commands through them: one cell is used to send
    commands from the model to the attached views, and the other cell is
    used to send commands from the attached controllers to the model.
    
    One of the requirements placed on any solution is that the sequence
    model/view/controller components used should have no additional
    requirements placed on them in order to use the solution. In other
    words, the sequence model/view/controller components should not be
    able to tell the difference between being directly connected to
    other sequence MVC components or being connected to each other
    through a property model.

    The solution provided here follows after the hardware design of a
    multiplexer/demultiplexer pairing, commonly called a _muldex_. All
    commands from the source to the destination are multiplexed to fit
    down a single 'line' (in this case, the property model cell), and
    are demultiplexed out the other side, where they are farmed out to
    the actual command recipients.
    
    The first line, from model to view, looks something like this:
    
        sequence_model --> view_proxy --> adam_cell --> model_proxy --> sequence_view
            [model]          [mux]          [PML]          [demux]         [view]

    The first collection of components are the mux and demux of the
    above control flow diagram, along with some helper routines designed
    to wire the whole system together. The final routine of the
    collection is intended to do all the 'heavy lifting' for the client,
    so all they need specify is a sequence model, a sequence view, a
    property model sheet, a cell name within that sheet, and an
    assemblage where some heap-allocated objects may be collected for
    future deletion. Once the system is in place, view update commands
    are sent to the attached views through the property model using the
    muldex components implemented here.

    The second line, from controller to model, looks something like this:
    
        sequence_controller --> model_proxy --> adam_cell --> controller_proxy --> sequence_model
           [controller]            [mux]          [PML]            [demux]            [model]

    The second collection of components are the mux and demux of the
    above control flow diagram, along with some helper routines designed
    to wire the whole system together. The final routine of the
    collection is intended to do all the 'heavy lifting' for the client,
    so all they need specify is a sequence controller, a sequence model,
    a property model sheet, a cell name within that sheet, and an
    assemblage where some heap-allocated objects may be collected for
    future deletion. Once the system is in place, model manipulation
    commands are sent to the attached model through the property model
    using the muldex components implemented here.

    The solutions also meets the requirement that no extra burden is
    placed on the sequence MVC components that is not already asked of
    them by the sequence MVC concepts themselves.

    USAGE CASE 1:

    A tack to take, which will likely become the most common case, is
    demonstrated here. Usually when constructing the system one has the property
    model and the sequence model together at one point, and you want to bind the
    two together on the view and controller muldex lines. Later on in another
    piece of code you will have a widget and the same property model, and would
    like to bind the two together on both the view and controller muldex lines.
    In such a case there are two helper routines to call, one for the first
    instance and one for the second:

    <pre>
    adobe::sequence_model<int>  sequence_model;
    adobe::basic_sheet_t        basic_sheet;
    my_sequence_widget<int>     my_sequence_widget; // models SequenceView and SequenceController
    adobe::assemblage_t         assemblage; // to cleanup pool allocations and call other functors

    adobe::name_t               view_line_cell_name;
    adobe::name_t               controller_line_cell_name;

    // ... at one point in the code:

    if (basic_sheet.count_interface(view_line_cell_name) &&
        basic_sheet.count_interface(controller_line_cell_name))
        attach_sequence_model_to_property_model(string_sequence_model,
                                                basic_sheet,
                                                view_line_cell_name,
                                                controller_line_cell_name,
                                                assemblage);

    // ... then, in another point in the code:

    if (basic_sheet.count_interface(view_line_cell_name) &&
        basic_sheet.count_interface(controller_line_cell_name))
        attach_sequence_widget_to_property_model(my_sequence_widget,
                                                 basic_sheet,
                                                 view_line_cell_name,
                                                 controller_line_cell_name,
                                                 assemblage);

    // Now that everything is attached the system can be used as expected.

    my_sequence_widget.push_back(42); // calls proc passed to monitor_push_back
    my_sequence_widget.insert(1, 42); // calls proc passed to monitor_insert
    </pre>

    USAGE CASE 2:

    Another tack to take is binding things together on a per-line basis. Each
    muldex 'line' (the entire collection of components from one end to the
    other) can be created with a call to a single API. There are two fundamental
    muldex lines per MVC system (one model-to-view, and one
    controller-to-model), each with its own API. Here is a sampling of what the
    code might look like to hook up these two muldex lines:

    <pre>
    adobe::sequence_model<int>  sequence_model;
    adobe::basic_sheet_t        basic_sheet;
    my_sequence_view<int>       my_sequence_view; // models SequenceView
    my_sequence_controller<int> my_sequence_controller; // models SequenceController
    adobe::assemblage_t         assemblage; // to cleanup pool allocations and call other functors

    // NOTE (fbrereto) : Make sure the assemblage is last on the list, so it is
    //                   destructed first. Thus the proper disconnections are
    //                   made prior to the destruction of the individual 
    //                   components using those connections are destroyed.

    // ... add cells in sheet for the two muldex lines. There are several ways
    // to do this. Here we take the most bare-bones approach:

    adobe::any_regular_t       dictionary_initializer;
    dictionary_initializer = adobe::any_regular_t(adobe::dictionary_t());
    basic_sheet.add_interface(adobe::static_name_t("line_in"), dictionary_initializer);
    basic_sheet.add_interface(adobe::static_name_t("line_out"), dictionary_initializer);

    attach_sequence_view_muldex(sequence_model,
                                my_sequence_view,
                                basic_sheet,
                                adobe::static_name_t("line_out"),
                                assemblage);

    attach_sequence_controller_muldex(sequence_model,
                                      my_sequence_controller,
                                      basic_sheet,
                                      adobe::static_name_t("line_in"),
                                      assemblage);

    // Here, you can invoke controller commands in my_sequence_controller and
    // the corresponding muldex line will propagate the command down the sheet
    // cell to the model, which will then turn around and propagate the
    // appropriate notification to my_sequence_view via the other muldex line.

    my_sequence_controller.push_back(42); // calls proc passed to monitor_push_back
    my_sequence_controller.insert(1, 42); // calls proc passed to monitor_insert
    </pre>

    CAVEAT(S):

    The property model library (Adam) sheet implementation is not
    reentrant. This means that you will not be able to supply cells for
    both muldex lines on a single sheet, as setting one will trigger a
    model update, thus attempting to set the other before the first
    update has completed, and the reentrancy assertion will fail. The
    adobe::basic_sheet_t data structure (at the time of this writing)
    has no such restriction. From a design perspective, it is better to
    have these muldex line cells in the basic sheet instead of the
    property model sheet. The reason is that if they were in the
    property model sheet, the implication is that they are essential
    elements to the command being formed, which is not true. Since the
    line cells are temporary holding areas for the forwarding of API
    commands through the sheet-based model, they should reside in the
    layout sheet. Another reason why they belong in the layout sheet is
    simply that there should be a pair per dialog instance, as there is
    an MVC system per dialog instance, and sharing them in the property
    model does not make sense.
*/
/******************************************************************************/
#if 0
#pragma mark -
#endif
/******************************************************************************/
/*!
    sequence_model -> **sequence_view_multiplexer** -> property_model

    This structure is the glue between the sequence model and the
    property model. It models a sequence view and will receive view
    updates as such, at which point it bundles up (multiplexes, hence
    the name) all the parameters it gets into a dictionary. The
    dictionary is then piped out to the attached property model, of
    which this structure is a controller to a particular cell. The
    monitor callback routine will (under the hood) invoke an update of
    the property model, causing the recently-pushed dictionary to be
    propagated 'down the line' to the view waiting for it on the other
    side (see the sequence_view_demultiplexer_t below.)
*/
template <typename T>
struct sequence_view_multiplexer
{
    typedef T                                            value_type;
    typedef sequence_model<T>                            sequence_model_type;
    typedef typename sequence_model_type::key_type       key_type;
    typedef typename sequence_model_type::cow_value_type cow_value_type;
    typedef dictionary_t                                 model_type;
    typedef boost::function<void (const model_type&)>    proc_type;

    void refresh(key_type key, cow_value_type value)
    {
        if (proc_m == false)
            return;

        dictionary_t command;

        command[static_name_t("command")] = any_regular_t(static_name_t("refresh"));
        command[static_name_t("key")] = any_regular_t(key);
        command[static_name_t("value")] = any_regular_t(value);

        send(command);
    }

    void extend(key_type before, key_type key, cow_value_type value)
    {
        if (proc_m == false)
            return;

        dictionary_t command;

        command[static_name_t("command")] = any_regular_t(static_name_t("extend"));
        command[static_name_t("before")] = any_regular_t(before);
        command[static_name_t("key")] = any_regular_t(key);
        command[static_name_t("value")] = any_regular_t(value);

        send(command);
    }

    void extend_set(key_type before, const vector<key_type>& key_set)
    {
        if (proc_m == false)
            return;

        dictionary_t command;

        command[static_name_t("command")] = any_regular_t(static_name_t("extend_set"));
        command[static_name_t("before")] = any_regular_t(before);
        command[static_name_t("key_set")] = any_regular_t(key_set);

        send(command);
    }

    void erase(const vector<key_type>& key_set)
    {
        if (proc_m == false)
            return;

        dictionary_t command;

        command[static_name_t("command")] = any_regular_t(static_name_t("erase"));
        command[static_name_t("key_set")] = any_regular_t(key_set);

        send(command);
    }

    void clear()
    {
        if (proc_m == false)
            return;

        dictionary_t command;

        command[static_name_t("command")] = any_regular_t(static_name_t("clear"));

        send(command);
    }

    void monitor(const proc_type& proc)
    { proc_m = proc; }

    void enable(bool) { }

private:
    void send(const adobe::dictionary_t& command)
    {
        // first we send the command
        proc_m(command);

        // then we send an empty dictionary to clear the line
        proc_m(dictionary_t());
    }

    proc_type proc_m;
};

/******************************************************************************/
/*!
    property_model -> **sequence_view_demultiplexer_t** -> my_sequence_view

    This structure is the glue between the property model and the
    client's sequence view. It will receive commands from the sequence
    model through the property model cell in the form of dictionaries,
    sent one at a time. The dictionaries are then farmed out
    (demultiplexed, hence the name) to the various sequence view APIs as
    provided by the client's implementation. The client actually using
    this piece of code really shouldn't know (or care) that this is
    under the hood - it is created by the helper routines below and
    bound to the assemblage, and then it should silently do its job.
*/
struct sequence_view_demultiplexer_t
{
    typedef dictionary_t model_type;

    /*!
        This constructor will take in anything that models the
        SequenceView concept and establishes a function pack set for the
        various sequence view routines. The routines will be accessed
        later as the display routine below receives updates from the
        property model, and the commands are farmed out to the relevant
        APIs.
    */
    template <typename T>
    explicit sequence_view_demultiplexer_t(T& sequence_view)
    {
        typedef typename T::key_type key_type;

        /*!
            For some reason adobe::function_traits isn't happy with whatever
            structure comes out of a boost::bind call, otherwise they would be
            inline in the register_function routines. As it is they are wrapped
            in a boost::function and passed, which is a viable alternative.
        */
        funnel_m.register_function(static_name_t("refresh"),
                                   boost::function<void (key_type, typename T::cow_value_type)>(boost::bind(&T::refresh, boost::ref(sequence_view), _1, _2)),
                                   static_name_t("key"),
                                   static_name_t("value"));

        funnel_m.register_function(static_name_t("extend"),
                                   boost::function<void (key_type, key_type, typename T::cow_value_type)>(boost::bind(&T::extend, boost::ref(sequence_view), _1, _2, _3)),
                                   static_name_t("before"),
                                   static_name_t("key"),
                                   static_name_t("value"));

        funnel_m.register_function(static_name_t("extend_set"),
                                   boost::function<void (key_type, const vector<key_type>&)>(boost::bind(&T::extend_set, boost::ref(sequence_view), _1, _2)),
                                   static_name_t("before"),
                                   static_name_t("key_set"));

        funnel_m.register_function(static_name_t("erase"),
                                   boost::function<void (const vector<key_type>&)>(boost::bind(&T::erase, boost::ref(sequence_view), _1)),
                                   static_name_t("key_set"));

        funnel_m.register_named0_function(static_name_t("clear"),
                                   boost::function<void ()>(boost::bind(&T::clear, boost::ref(sequence_view))));
    }

    /*!
        display here models the requirements of the property model View
        concept, takes in an adobe::dictionary which is a wrapped set of
        command parameters (one of which is the name of the command
        itself). The command is immediately sent off to the function
        pack, which will tease out the necessary bits, align parameters
        to their specified locations, and fire off the underlying APIs
        to the attached sequence view.
    */
    void display(const model_type& value)
    {
        if (value == model_type())
            return;

        funnel_m(value);
    }

private:
    function_pack_t funnel_m;
};

/******************************************************************************/
/*!
    Hooks an object that models the SequenceView concept to a specified
    cell in a property model. The assemblage is necessary to pool the
    heap-allocated memory used to make the glue between your object and
    the property model. Once this routine is complete your object will
    behave as a view of the specified cell of the property model passed,
    and thus by proxy a view of the sequence attached to the other side
    of the cell specified.
*/
template <typename SequenceView, typename Sheet>
void attach_sequence_view_to_model(assemblage_t& assemblage,
                                   Sheet&        model,
                                   name_t        cell,
                                   SequenceView& sequence_view)
{
    // This line asserts that sequence_view does in fact model a SequenceView concept.
    boost::function_requires<SequenceViewConcept<SequenceView> >();

    sequence_view_demultiplexer_t* demux(new sequence_view_demultiplexer_t(sequence_view));

    assemblage_cleanup_ptr(assemblage, demux);

    attach_view_to_model(assemblage, model, cell, *demux);
}

/******************************************************************************/
/*!
    Hooks sequence model to a specified cell in a property model. The
    assemblage is necessary to pool the heap-allocated memory used to
    make the glue between the sequence model and the property model.
    Once this routine is complete the sequence model will behave as a
    controller of the specified cell of the property model passed, and
    thus by proxy a model to the sequence view attached to the other
    side of the cell specified.
*/
template <typename SequenceModel, typename Sheet>
void attach_sequence_model_view_to_model(assemblage_t&  assemblage,
                                         Sheet&         model,
                                         name_t         cell,
                                         SequenceModel& sequence_model)
{
    typedef typename SequenceModel::value_type                   value_type;
    typedef typename poly_sequence_view<value_type>::type poly_sequence_view_type;

    sequence_view_multiplexer<value_type>* mux(new sequence_view_multiplexer<value_type>());

    assemblage_cleanup_ptr(assemblage, mux);

    attach_controller_to_model(assemblage, model, cell, *mux);

    poly_sequence_view_type* poly_sequence_view(new poly_sequence_view_type(boost::ref(*mux)));

    assemblage_cleanup_ptr(assemblage, poly_sequence_view);

    sequence_model.attach_view(*poly_sequence_view);

    assemblage.cleanup(boost::bind(&SequenceModel::detach_view,
                                   boost::ref(sequence_model),
                                   boost::ref(*poly_sequence_view)));
}

/******************************************************************************/
/*!
    A helper routine that does it all: takes a sequence model, a
    sequence view, a property model and the name of a cell, and creates
    the multiplexer/demultiplexer components necessary for the sequence
    model to send commands to the sequence view via the specified cell
    in the property model. Note that if the sequence_model is already
    established as a controller to the property model cell, then all you
    need is to call attach_sequence_view_to_model above. The assemblage
    is, of course, used to pool the heap-allocated memory used to make
    all the glue necessary.
*/
template <typename SequenceModel, typename SequenceView, typename Sheet>
void attach_sequence_view_muldex(SequenceModel&       sequence_model,
                                 SequenceView&        sequence_view,
                                 Sheet&               model,
                                 name_t        cell,
                                 assemblage_t& assemblage)
{
    attach_sequence_view_to_model(assemblage,
                                  model,
                                  cell,
                                  sequence_view);

    attach_sequence_model_view_to_model(assemblage,
                                        model,
                                        cell,
                                        sequence_model);
}

/******************************************************************************/
#if 0
#pragma mark -
#endif
/******************************************************************************/
/*!
    my_sequence_controller -> **sequence_model_multiplexer** -> property_model

    This structure is the glue between the client's sequence controller
    and the property model. It models a sequence controller and will
    receive command as such, at which point it bundles up (multiplexes,
    hence the name) all the parameters it gets into a dictionary. The
    dictionary is then piped out to the attached property model, of
    which this structure is a controller to a particular cell. The
    monitor callback routine will (under the hood) invoke an update of
    the property model, causing the recently-pushed dictionary to be
    propagated 'down the line' to the acutal sequence model waiting for
    it on the other side (see the sequence_model_demultiplexer below.)
*/
template <typename T>
struct sequence_model_multiplexer
{
    typedef T                                         value_type;
    typedef adobe::sequence_key<T>                    key_type;
    typedef dictionary_t                              model_type;
    typedef boost::function<void (const model_type&)> proc_type;

    template <typename SequenceController>
    sequence_model_multiplexer(SequenceController& controller)
    {
        BOOST_STATIC_ASSERT((boost::is_same<T, typename sequence_controller_value_type<SequenceController>::type>::value));

        poly_m.reset(new typename poly_sequence_model<T>::type(boost::ref(*this)));

        controller.monitor_sequence(*poly_m);
    }

    /*
        The following are sequence_controller routines.
    */
    
    void push_back(const value_type& value)
    {
        if (proc_m == false)
            return;

        dictionary_t command;

        command[static_name_t("command")] = any_regular_t(static_name_t("push_back"));
        command[static_name_t("value")] = any_regular_t(value);

        send(command);
    }

    void set(key_type key, const value_type& value)
    {
        if (proc_m == false)
            return;

        dictionary_t command;

        command[static_name_t("command")] = any_regular_t(static_name_t("set"));
        command[static_name_t("key")] = any_regular_t(key);
        command[static_name_t("value")] = any_regular_t(value);

        send(command);
    }

    void insert(key_type before, const value_type& value)
    {
        if (proc_m == false)
            return;

        dictionary_t command;

        command[static_name_t("command")] = any_regular_t(static_name_t("insert"));
        command[static_name_t("before")] = any_regular_t(before);
        command[static_name_t("value")] = any_regular_t(value);

        send(command);
    }

    void insert_set(key_type before, const vector<value_type>& value_set)
    {
        if (proc_m == false)
            return;

        dictionary_t command;

        command[static_name_t("command")] = any_regular_t(static_name_t("insert_set"));
        command[static_name_t("before")] = any_regular_t(before);
        command[static_name_t("value_set")] = any_regular_t(value_set);

        send(command);
    }

    void erase(const vector<key_type>& key_set)
    {
        if (proc_m == false)
            return;

        dictionary_t command;

        command[static_name_t("command")] = any_regular_t(static_name_t("erase"));
        command[static_name_t("key_set")] = any_regular_t(key_set);

        send(command);
    }

    void clear()
    {
        if (proc_m == false)
            return;

        dictionary_t command;

        command[static_name_t("command")] = any_regular_t(static_name_t("clear"));

        send(command);
    }

    /*
        The following are property_model_controller routines.
    */
    
    void monitor(const proc_type& proc)
    { proc_m = proc; }

    void enable(bool) { }

private:
    void send(const adobe::dictionary_t& command)
    {
        // first we send the command
        proc_m(command);

        // then we send an empty dictionary to clear the line
        proc_m(dictionary_t());
    }

    proc_type                                       proc_m;
    auto_ptr<typename poly_sequence_model<T>::type> poly_m;
};

/******************************************************************************/
/*!
    property_model -> **sequence_model_demultiplexer** -> sequence_model

    This structure is the glue between the property model and the
    sequence model. It will receive commands from the sequence
    controller through the property model cell in the form of
    dictionaries, sent one at a time. The dictionaries are then farmed
    out (demultiplexed, hence the name) to the various sequence view
    APIs as provided by the client's implementation. The client actually
    using this piece of code really shouldn't know (or care) that this
    is under the hood - it is created by the helper routines below and
    bound to the assemblage, and then it should silently do its job.
*/
template <typename T>
struct sequence_model_demultiplexer
{
    typedef T                      value_type;
    typedef adobe::sequence_key<T> key_type;
    typedef dictionary_t           model_type;

    sequence_model_demultiplexer() :
        sequence_m(0)
    { }

    /*!
        display here models the requirements of the property model View concept,
        takes in an adobe::dictionary which is a wrapped set of command
        parameters (one of which is the name of the command itself). The command
        is immediately sent off to the function pack, which will tease out the
        necessary bits, align parameters to their specified locations, and fire
        off the underlying APIs to the attached sequence view.
    */
    void display(const model_type& value)
    {
        if (value == model_type())
            return;

        funnel_m(value);
    }

    void monitor_sequence(typename poly_sequence_model<T>::type& sequence)
    {
        sequence_m = &sequence;

        funnel_m.register_function(static_name_t("push_back"),
                                   boost::function<void (const value_type&)>(boost::bind(&poly_sequence_model<T>::type::push_back, boost::ref(*sequence_m), _1)),
                                   static_name_t("value"));

        funnel_m.register_function(static_name_t("set"),
                                   boost::function<void (key_type pos, const value_type&)>(boost::bind(&poly_sequence_model<T>::type::set, boost::ref(*sequence_m), _1, _2)),
                                   static_name_t("key"),
                                   static_name_t("value"));

        funnel_m.register_function(static_name_t("insert"),
                                   boost::function<void (key_type pos, const value_type&)>(boost::bind(&poly_sequence_model<T>::type::insert, boost::ref(*sequence_m), _1, _2)),
                                   static_name_t("before"),
                                   static_name_t("value"));

        funnel_m.register_function(static_name_t("insert_set"),
                                   boost::function<void (key_type pos, const vector<value_type>&)>(boost::bind(&poly_sequence_model<T>::type::insert_set, boost::ref(*sequence_m), _1, _2)),
                                   static_name_t("before"),
                                   static_name_t("value_set"));

        funnel_m.register_function(static_name_t("erase"),
                                   boost::function<void (const vector<key_type>& selection)>(boost::bind(&poly_sequence_model<T>::type::erase, boost::ref(*sequence_m), _1)),
                                   static_name_t("key_set"));

        funnel_m.register_named0_function(static_name_t("clear"),
                                   boost::function<void ()>(boost::bind(&poly_sequence_model<T>::type::clear, boost::ref(*sequence_m))));
    }

private:
    typename poly_sequence_model<T>::type* sequence_m;
    function_pack_t                        funnel_m;
};

/******************************************************************************/
/*!
    Hooks an object that models the SequenceController concept to a
    specified cell in a property model. The assemblage is necessary to
    pool the heap-allocated memory used to make the glue between your
    object and the property model. Once this routine is complete your
    object will behave as a controller of the specified cell of the
    property model passed, and thus by proxy a controller of the
    sequence attached to the other side of the cell specified.
*/
template <typename SequenceController, typename Sheet>
void attach_sequence_controller_to_model(assemblage_t&       assemblage,
                                         Sheet&              model,
                                         name_t              cell,
                                         SequenceController& sequence_controller)
{
    // This line asserts that sequence_controller does in fact model a SequenceController concept.
    boost::function_requires<SequenceControllerConcept<SequenceController> >();

    typedef typename sequence_controller_value_type<SequenceController>::type value_type;
    typedef typename poly_sequence_controller<value_type>::type               poly_sequence_controller_type;

    sequence_model_multiplexer<value_type>* mux(new sequence_model_multiplexer<value_type>(sequence_controller));

    assemblage_cleanup_ptr(assemblage, mux);

    attach_controller_to_model(assemblage, model, cell, *mux);
}

/******************************************************************************/
/*!
    Hooks sequence model to a specified cell in a property model. The
    assemblage is necessary to pool the heap-allocated memory used to
    make the glue between the sequence model and the property model.
    Once this routine is complete the sequence model will behave as a
    view of the specified cell of the property model passed, and thus by
    proxy a model of the sequence controller attached to the other side
    of the cell specified.
*/
template <typename SequenceModel, typename Sheet>
void attach_sequence_model_controller_to_model(assemblage_t&  assemblage,
                                               Sheet&         model,
                                               name_t         cell,
                                               SequenceModel& sequence_model)
{
    typedef typename SequenceModel::value_type                  value_type;
    typedef typename poly_sequence_controller<value_type>::type poly_sequence_controller_type;

    sequence_model_demultiplexer<value_type>* demux(new sequence_model_demultiplexer<value_type>());

    assemblage_cleanup_ptr(assemblage, demux);

    poly_sequence_controller_type* poly_sequence_controller(new poly_sequence_controller_type(boost::ref(*demux)));

    assemblage_cleanup_ptr(assemblage, poly_sequence_controller);

    sequence_model.attach_controller(*poly_sequence_controller);

    assemblage.cleanup(boost::bind(&SequenceModel::detach_controller,
                                   boost::ref(sequence_model),
                                   boost::ref(*poly_sequence_controller)));

    attach_view_to_model(assemblage, model, cell, *demux);
}

/******************************************************************************/
/*!
    A helper routine that does it all: takes a sequence model, a
    sequence controller, a property model and the name of a cell, and
    creates the multiplexer/demultiplexer components necessary for the
    sequence controller to send commands to the sequence model via the
    specified cell in the property model. Note that if the
    sequence_model is already established as a view to the property
    model cell, then all you need is to call
    attach_sequence_controller_to_model above. The assemblage is, of
    course, used to pool the heap-allocated memory used to make all the
    glue necessary.
*/
template <typename SequenceModel, typename SequenceController, typename Sheet>
void attach_sequence_controller_muldex(SequenceModel&      sequence_model,
                                       SequenceController& sequence_controller,
                                       Sheet&              model,
                                       name_t              cell,
                                       assemblage_t&       assemblage)
{
    attach_sequence_controller_to_model(assemblage,
                                        model,
                                        cell,
                                        sequence_controller);

    attach_sequence_model_controller_to_model(assemblage,
                                              model,
                                              cell,
                                              sequence_model);
}

/******************************************************************************/
/*!
    Another helper routine that takes things from a more logical tack. Usually
    one has a sequence_model on one side of the property model and would like
    to hook it up to both the view and controller lines in one fell swoop. This
    routine does that.
*/
template <typename SequenceModel, typename Sheet>
void attach_sequence_model_to_property_model(SequenceModel& sequence_model,
                                             Sheet&         model,
                                             name_t         view_line_cell,
                                             name_t         controller_line_cell,
                                             assemblage_t&  assemblage)
{
    attach_sequence_model_controller_to_model(assemblage,
                                              model,
                                              controller_line_cell,
                                              sequence_model);

    attach_sequence_model_view_to_model(assemblage,
                                        model,
                                        view_line_cell,
                                        sequence_model);
}

/******************************************************************************/
/*!
    Another helper routine that takes things from a more logical tack. Usually
    one has a sequence_model on one side of the property model and would like
    to hook it up to both the view and controller lines in one fell swoop. This
    routine does that.
*/
template <typename SequenceWidget, typename Sheet>
void attach_sequence_widget_to_property_model(SequenceWidget& sequence_widget,
                                              Sheet&          model,
                                              name_t          view_line_cell,
                                              name_t          controller_line_cell,
                                              assemblage_t&   assemblage)
{
    attach_sequence_controller_to_model(assemblage,
                                        model,
                                        controller_line_cell,
                                        sequence_widget);

    attach_sequence_view_to_model(assemblage,
                                  model,
                                  view_line_cell,
                                  sequence_widget);
}

/******************************************************************************/

} // namespace adobe

/******************************************************************************/
// ADOBE_SEQUENCE_MVC_MULDEX
#endif
/******************************************************************************/
