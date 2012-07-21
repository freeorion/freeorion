/*
    Copyright 2005-2008 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/**************************************************************************************************/

#include <GG/adobe/adam.hpp>

#include <deque>

#include <boost/bind.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/function.hpp>
#include <boost/variant.hpp>

#include <GG/adobe/algorithm/find.hpp>
#include <GG/adobe/algorithm/for_each.hpp>
#include <GG/adobe/algorithm/transform.hpp>
#include <GG/adobe/algorithm/unique.hpp>
#include <GG/adobe/algorithm/sort.hpp>
#include <GG/adobe/array.hpp>
#include <GG/adobe/dictionary.hpp>
#include <GG/adobe/name.hpp>
#include <GG/adobe/any_regular.hpp>

#include <GG/adobe/functional.hpp>
#include <GG/adobe/istream.hpp>
#include <GG/adobe/string.hpp>
#include <GG/adobe/table_index.hpp>
#include <GG/adobe/virtual_machine.hpp>

#include <GG/ExpressionWriter.h>

#ifndef NDEBUG

#include <iostream>

#endif // NDEBUG

/**************************************************************************************************/

namespace anonymous_adam_cpp { // can't instantiate templates on types from real anonymous

/**************************************************************************************************/

#ifndef NDEBUG

struct check_reentrancy
{
    check_reentrancy(bool& x) : check_m(x)
    { assert(!x && "FATAL (sparent) : Function Not Reentrant."); check_m = true; }
    ~check_reentrancy() { check_m = false; }
    
    bool& check_m;
};

#endif // NDEBUG

/**************************************************************************************************/

/*
    REVISIT (sparent) : Move to utility? This is generally useful to provide a copy for
    non-copyable types such as boost::signal<>.
*/

template <typename T> // T models default constructable
struct empty_copy : T
{
    empty_copy() : T() { }
    empty_copy(const empty_copy&) : T() { }
    empty_copy& operator=(const empty_copy&) { return *this; }
};

/**************************************************************************************************/

typedef adobe::sheet_t                              sheet_t;

// This currently establishes an upperbound on the number of cells in the sheet at 1K.
typedef std::bitset<1024>                           cell_bits_t;
typedef int                                         priority_t;

struct compare_contributing_t;

enum access_specifier_t
{
    access_input,
    access_interface_input,
    access_interface_output,
    access_output,
    access_logic,
    access_constant,
    access_invariant
};

/**************************************************************************************************/

/*
    REVISIT (sparent) : A thought - this could be packaged as a general template function for
    converting exceptions on function object calls.
*/

/*
    REVIST (sparent) : Some version of MSVC didn't like function level try blocks. Need to test.
*/

void evaluate(adobe::virtual_machine_t& machine, const adobe::line_position_t& position,
        const adobe::array_t& expression)
#ifdef BOOST_MSVC
{
#endif
try
{
    machine.evaluate(expression);
}
catch (const std::exception& error)
{
    throw adobe::stream_error_t(error, position);
}
#ifdef BOOST_MSVC
}
#endif

/**************************************************************************************************/

struct scope_count : boost::noncopyable
{
    scope_count(std::size_t& x) : value_m(x) { ++value_m; }
    ~scope_count() { --value_m; }
    
 private:
    std::size_t& value_m;
};

template <typename T>
struct scope_value_t : boost::noncopyable
{
    scope_value_t(T& x, const T& v) : value_m(x), store_m(x) { x = v; }
    ~scope_value_t() { value_m = store_m; }

 private:
    T&  value_m;
    T   store_m;
};

/**************************************************************************************************/

} // namespace anonymous_adam_cpp
using namespace anonymous_adam_cpp;

/**************************************************************************************************/

namespace adobe {

/**************************************************************************************************/

class sheet_t::implementation_t : boost::noncopyable
{
public:
    typedef sheet_t::connection_t           connection_t;

    explicit implementation_t(virtual_machine_t& machine);
    
    any_regular_t inspect(const array_t& expression);
    
    void set(name_t, const any_regular_t&); // input cell.
    void touch(const name_t*, const name_t*); // range of input cells.
    
    any_regular_t get(name_t);
    
    void add_input      (name_t, const line_position_t&, const array_t& initializer);
    void add_output     (name_t, const line_position_t&, const array_t& expression);
    void add_constant   (name_t, const line_position_t&, const array_t& initializer);
    void add_logic      (name_t, const line_position_t&, const array_t& expression);
    void add_invariant  (name_t, const line_position_t&, const array_t& expression);
    void add_interface  (name_t, bool linked, const line_position_t&, const array_t& initializer,
                                              const line_position_t&, const array_t& expression);

    void add_relation   (const line_position_t&, const array_t& conditional,
            const relation_t* first, const relation_t* last);
    connection_t    monitor_value(name_t, const monitor_value_t&); //output only

    //input only 
    connection_t    monitor_enabled(name_t, const name_t* first, const name_t* last,
                                       const monitor_enabled_t&); 

    
    connection_t    monitor_contributing(name_t, const dictionary_t&, 
                                         const monitor_contributing_t&);

    #if 0
    connection_t    monitor_invariant_contributing(name_t invariant, const monitor_invariant_t&); 
// REVISIT (sparent) : UNIMPLEMENTED
    #endif

    connection_t    monitor_invariant_dependent(name_t invariant, const monitor_invariant_t&);

    bool has_input(name_t) const;
    bool has_output(name_t) const;

    void update();

    void reinitialize();
    
    void set(const dictionary_t& dictionary); 
// set input cells to corresponding values in dictionary.
    
    dictionary_t contributing(const dictionary_t&) const; 
// all contributing values that have changed since mark
    dictionary_t contributing_to_cell(name_t) const;

    void print(std::ostream& os) const;
    
private:
    struct relation_cell_t;

    typedef vector<relation_cell_t*>    relation_index_t;
    typedef std::vector<relation_t>     relation_set_t;
    
    struct relation_cell_t
    {
        relation_cell_t(const line_position_t& position,
                const array_t& conditional, const relation_t* first,
                const relation_t* last) :
            resolved_m(false),
            position_m(position),
            conditional_m(conditional),
            terms_m(first, last)
        { }
        
        bool                    resolved_m;
        
        line_position_t         position_m;
        array_t                 conditional_m;
        relation_set_t          terms_m;
        
        // REVISIT (sparent) : There should be a function object to set members
        void clear_resolved()
        {
            resolved_m = false;
        }
    };

    struct cell_t
    {
        typedef boost::function<any_regular_t()>            calculator_t;
        
        typedef empty_copy<boost::signal<void (bool)> >                  monitor_invariant_list_t;
        typedef empty_copy<boost::signal<void (const any_regular_t&)> >  monitor_value_list_t;
        typedef empty_copy<boost::signal<void (const cell_bits_t&)> >    monitor_contributing_list_t;

        cell_t(access_specifier_t specifier, name_t, const calculator_t& calculator, 
               std::size_t cell_set_pos, cell_t* = 0); // output
        cell_t(access_specifier_t specifier, name_t, any_regular_t, 
               std::size_t cell_set_pos); // constant

        cell_t(name_t, any_regular_t, std::size_t cell_set_pos); // input cell
        cell_t(name_t, bool linked, const calculator_t& init_expression, 
               std::size_t cell_set_pos); // interface cell (input)
        
        #if 0
        // compiler generated.
        cell_t(const cell_t& x);
        cell_t& operator=(const cell_t& x);
        #endif
                
        access_specifier_t                      specifier_m;
        name_t                                  name_m;
        
        calculator_t                            calculator_m;
        
        bool                                    linked_m;
        bool                                    invariant_m;

        priority_t                              priority_m; // For linked input cells only - zero otherwise

        bool                                    resolved_m; // For interface cells only - false if cell hasn't been flowed
        bool                                    evaluated_m; // true if cell has been calculated (or has no calculator).

        std::size_t                             relation_count_m;
        std::size_t                             initial_relation_count_m;

        bool                                    dirty_m;        // denotes change state_m value
        
        any_regular_t                           state_m;
        cell_bits_t                             contributing_m;
        cell_bits_t                             init_contributing_m;
        
        std::size_t                             cell_set_pos_m; // self index in sheet_t::cell_set_m
        
        calculator_t                            term_m;
        
        // For output half of interface cells this points to corresponding input half. NULL otherwise.
        cell_t*                                 interface_input_m;
        
        // For output half of interface cells this points to any possible connected relations.
        relation_index_t                        relation_index_m;

        monitor_value_list_t        monitor_value_m;
        monitor_contributing_list_t monitor_contributing_m;
        monitor_invariant_list_t    monitor_invariant_m;
        
        void calculate();
        
        void clear_dirty()
        {
            dirty_m = false;
            relation_count_m = initial_relation_count_m;
            term_m.clear();
            evaluated_m = specifier_m == access_input || specifier_m == access_constant /* || calculator_m.empty() */;
        
            /*
                REVISIT (sparent) : What exactly is the distinction between evaluated and resolved.
            */
        
            resolved_m = evaluated_m;
        }
        
        /*
            REVISIT (sparent) : A member wise implementation of swap would be better - but I'm
            going to swap this way for expendiancy since cell_t will likely change a lot when
            I get around to rewriting Adam.
        */
        // friend void swap(cell_t& x, cell_t&y) { std::swap(x, y); }
    };
    
    friend struct cell_t;
    friend struct compare_contributing_t;
    
    any_regular_t calculate_expression(const line_position_t& position, 
                                         const array_t& expression);
    
    dictionary_t contributing_set(const dictionary_t&, const cell_bits_t&) const;
    
    void initialize_one(cell_t& cell);

    void enabled_filter(const cell_bits_t& touch_set,
                           std::size_t contributing_index_pos,
                           monitor_enabled_t monitor,
                           const cell_bits_t& new_priority_accessed_bits,
                           const cell_bits_t& new_active_bits);

//    std::size_t cell_set_to_contributing(std::size_t cell_set_pos) const;
    
    priority_t name_to_priority(name_t name) const;
    void flow(cell_bits_t& priority_accessed);

/*
    NOTE (sparent) : cell_t contains boost::signal<> which is not copyable. The cells support
    limited copying until they have monitors attached - this allows them to be placed into a
    container prior to any connections being made. A deque is used rather than a vector because it 
    does not reallocate when it grows.
*/
    
    typedef std::deque<cell_t>                  cell_set_t;
    typedef std::deque<relation_cell_t>         relation_cell_set_t;
    
    typedef std::vector<name_t>                 get_stack_t;
    typedef std::vector<cell_t*>                index_vector_t;
    
    typedef hash_index<  cell_t,
                                boost::hash<name_t>,
                                equal_to,
                                mem_data_t<cell_t, const name_t> >  index_t;

    struct input_parameters_t
    {
        input_parameters_t (name_t name,
                            const line_position_t& position,
                            const array_t& initializer) :
            name_m(name),
            position_m(position),
            initializer_m(initializer)
            {}
        name_t name_m;
        line_position_t position_m;
        array_t initializer_m;
    };

    struct output_parameters_t
    {
        output_parameters_t (name_t output,
                             const line_position_t& position,
                             const array_t& expression) :
            output_m(output),
            position_m(position),
            expression_m(expression)
            {}
        name_t output_m;
        line_position_t position_m;
        array_t expression_m;
    };

    struct interface_parameters_t
    {
        interface_parameters_t (name_t name,
                                bool linked,
                                const line_position_t& position1,
                                const array_t& initializer_expression,
                                const line_position_t& position2, 
                                const array_t& expression) :
            name_m(name),
            linked_m(linked),
            position1_m(position1),
            initializer_expression_m(initializer_expression),
            position2_m(position2),
            expression_m(expression)
            {}
        name_t name_m;
        bool linked_m;
        line_position_t position1_m;
        array_t initializer_expression_m;
        line_position_t position2_m;
        array_t expression_m;
    };

    struct constant_parameters_t
    {
        constant_parameters_t (name_t name,
                               const line_position_t& position,
                               const array_t& initializer) :
            name_m(name),
            position_m(position),
            initializer_m(initializer)
            {}
        name_t name_m;
        line_position_t position_m;
        array_t initializer_m;
    };

    struct logic_parameters_t
    {
        logic_parameters_t (name_t logic,
                            const line_position_t& position,
                            const array_t& expression) :
            logic_m(logic),
            position_m(position),
            expression_m(expression)
            {}
        name_t logic_m;
        line_position_t position_m;
        array_t expression_m;
    };

    struct invariant_parameters_t
    {
        invariant_parameters_t (name_t invariant,
                                const line_position_t& position,
                                const array_t& expression) :
            invariant_m(invariant),
            position_m(position),
            expression_m(expression)
            {}
        name_t invariant_m;
        line_position_t position_m;
        array_t expression_m;
    };

    struct relation_parameters_t
    {
        relation_parameters_t (const line_position_t& position,
                               const array_t& conditional,
                               const relation_t* first,
                               const relation_t* last) :
            position_m(position),
            conditional_m(conditional),
            relations_m(first, last)
            {}
        line_position_t position_m;
        array_t conditional_m;
        std::vector<relation_t> relations_m;
    };

    typedef boost::variant<
        input_parameters_t,
        output_parameters_t,
        constant_parameters_t,
        logic_parameters_t,
        invariant_parameters_t,
        interface_parameters_t,
        relation_parameters_t
    > add_parameters_t;

    struct print_visitor :
        public boost::static_visitor<>
    {
        print_visitor(std::ostream& os) :
            os_m(os)
            {}

        void operator()(const input_parameters_t& params) const
            {
                os_m << "    " << params.name_m << " : "
                     << GG::WriteExpression(params.initializer_m) << ";\n";
            }

        void operator()(const output_parameters_t& params) const
            {
                os_m << "    " << params.output_m << " <== "
                     << GG::WriteExpression(params.expression_m) << ";\n";
            }

        void operator()(const constant_parameters_t& params) const
            {
                os_m << "    " << params.name_m << " : "
                     << GG::WriteExpression(params.initializer_m) << ";\n";
            }

        void operator()(const logic_parameters_t& params) const
            {
                os_m << "    " << params.logic_m << " <== "
                     << GG::WriteExpression(params.expression_m) << ";\n";
            }

        void operator()(const invariant_parameters_t& params) const
            {
                os_m << "    " << params.invariant_m << " <== "
                     << GG::WriteExpression(params.expression_m) << ";\n";
            }

        void operator()(const interface_parameters_t& params) const
            {
                os_m << "    ";
                if (!params.linked_m)
                    os_m << "unlink ";
                os_m << params.name_m;
                if (!params.initializer_expression_m.empty())
                    os_m << " : " << GG::WriteExpression(params.initializer_expression_m);
                if (!params.expression_m.empty())
                    os_m << "<== " << GG::WriteExpression(params.expression_m);
                os_m << ";\n";
            }

        void operator()(const relation_parameters_t& params) const
            {
                os_m << "    ";
                if (!params.conditional_m.empty())
                    os_m << "when (" << GG::WriteExpression(params.conditional_m) << ") ";
                os_m << "relate {\n";
                for (std::size_t i = 0; i < params.relations_m.size(); ++i) {
                    const relation_t& relation = params.relations_m[i];
                    os_m << "        " << relation.name_m << " <== "
                         << GG::WriteExpression(relation.expression_m) << ";\n";
                }
                os_m << "    }\n";
            }

        std::ostream& os_m;
    };

    struct added_cell_set_t
    {
        added_cell_set_t(access_specifier_t access) :
            access_m(access)
            {}
        access_specifier_t access_m;
        std::vector<add_parameters_t> added_cells_m;
    };

    typedef std::vector<added_cell_set_t> added_cells_t;

    index_t             name_index_m;
    index_t             setable_index_m; // input of interface or input;
    index_t             input_index_m;
    index_t             output_index_m;
    
    index_vector_t      invariant_index_m;

    priority_t          priority_high_m;
    priority_t          priority_low_m;

    cell_bits_t         conditional_indirect_contributing_m;
        
    virtual_machine_t&  machine_m;
    get_stack_t         get_stack_m;
    std::size_t         get_count_m;

    cell_bits_t         init_dirty_m;
    cell_bits_t         priority_accessed_m;      
    cell_bits_t         value_accessed_m; 
    cell_bits_t         active_m; 

    typedef boost::signal<void (const cell_bits_t&, const cell_bits_t&)>  
                                 monitor_enabled_list_t;
    monitor_enabled_list_t    monitor_enabled_m;
    
    cell_bits_t            accumulate_contributing_m;

    bool                   has_output_m; // true if there are any output cells.
    bool                   initialize_mode_m; // true during reinitialize call.

    added_cells_t          added_cells_m;
    
    // Actual cell storage - every thing else is index or state.
    
    cell_set_t          cell_set_m;
    relation_cell_set_t relation_cell_set_m;
     
#ifndef NEBUG
    bool                updated_m;
    bool                check_update_reentrancy_m;
#endif
};

/**************************************************************************************************/

void 
sheet_t::implementation_t::enabled_filter(const cell_bits_t& touch_set,
                                         std::size_t contributing_index_pos,
                                         monitor_enabled_t monitor,
                                         const cell_bits_t& new_priority_accessed_bits,
                                         const cell_bits_t& 
                                             new_active_bits)
{
    cell_bits_t new_priority_accessed_touch = new_priority_accessed_bits & touch_set;
    cell_bits_t old_priority_accessed_touch = priority_accessed_m & touch_set;
    bool unchanged_priority_accessed_touch = 
            (new_priority_accessed_touch ^ old_priority_accessed_touch).none();
            
    cell_t& cell = cell_set_m[contributing_index_pos];
    
    bool active(active_m.test(contributing_index_pos));
    bool new_active(new_active_bits.test(contributing_index_pos));
    
    if (unchanged_priority_accessed_touch && (active == new_active)) return;
        
    monitor(new_active || 
               (value_accessed_m.test(cell.cell_set_pos_m) && new_priority_accessed_touch.any()));
}

/**************************************************************************************************/

/*
    REVISIT (sparent) : Need to figure out what happens if this is called on an input cell during
    initialization (before it is resolved).
*/

void sheet_t::implementation_t::cell_t::calculate()
{
    if (evaluated_m) return;
    // REVISIT (sparent) : review resolved_m resolved issue.
    //assert(resolved_m && "Cell in an invalid state?");
    
    // This is to handle conditionals which refer to cells involved in relate clauses
    if (relation_count_m) throw std::logic_error(make_string("cell ", name_m.c_str(),
            " is attached to an unresolved relate clause."));
    
    any_regular_t result = term_m.empty() ? calculator_m() : term_m();
    
    dirty_m = (result != state_m);
    state_m = ::adobe::move(result);
    evaluated_m = true;
}

/**************************************************************************************************/

sheet_t::implementation_t::cell_t::cell_t(name_t name, any_regular_t x, 
                                          std::size_t cell_set_pos) :
    specifier_m(access_input),
    name_m(name),
    invariant_m(false),
    priority_m(0),
    resolved_m(true),
    evaluated_m(true),
    relation_count_m(0),
    initial_relation_count_m(0),
    dirty_m(false),
    state_m(::adobe::move(x)),
    cell_set_pos_m(cell_set_pos),
    interface_input_m(0)
{
    init_contributing_m.set(cell_set_pos);
}

/**************************************************************************************************/

sheet_t::implementation_t::cell_t::cell_t(name_t name, bool linked, const calculator_t& initializer, 
                                          std::size_t cell_set_pos) :
    specifier_m(access_interface_input),
    name_m(name),
    calculator_m(initializer),
    linked_m(linked),
    invariant_m(false),
    priority_m(0),
    resolved_m(true),
    evaluated_m(true),
    relation_count_m(0),
    initial_relation_count_m(0),
    cell_set_pos_m(cell_set_pos),
    interface_input_m(0)
{
    contributing_m.set(cell_set_pos);
}
    
/**************************************************************************************************/    

sheet_t::implementation_t::cell_t::cell_t(access_specifier_t specifier, name_t name,
                                          const calculator_t& calculator, 
                                          std::size_t cell_set_pos, cell_t* input) :
    specifier_m(specifier),
    name_m(name),
    calculator_m(calculator),
    linked_m(false),
    invariant_m(false),
    priority_m(0),
    resolved_m(false),
    evaluated_m(calculator_m.empty()),
    relation_count_m(0),
    initial_relation_count_m(0),
    cell_set_pos_m(cell_set_pos),
    interface_input_m(input)
{ }
    
/**************************************************************************************************/    

sheet_t::implementation_t::cell_t::cell_t(access_specifier_t specifier, name_t name,
                                          any_regular_t x, std::size_t cell_set_pos) :
    specifier_m(specifier),
    name_m(name),
    linked_m(false),
    invariant_m(false),
    priority_m(0),
    resolved_m(true),
    evaluated_m(true),
    relation_count_m(0),
    initial_relation_count_m(0),
    state_m(::adobe::move(x)),
    cell_set_pos_m(cell_set_pos),
    interface_input_m(0)
{ }
    
/**************************************************************************************************/

sheet_t::sheet_t() :
    object_m(new implementation_t(machine_m))
{ }

sheet_t::~sheet_t()
{ delete object_m; }

any_regular_t sheet_t::inspect(const array_t& expression)
{ return object_m->inspect(expression); }
 
void sheet_t::set(name_t input, const any_regular_t& value)
{ object_m->set(input, value); }
    
void sheet_t::touch(const name_t* first, const name_t* last)
{ object_m->touch(first, last); }

void sheet_t::add_input(name_t input, const line_position_t& position, const array_t& initializer)
{ object_m->add_input(input, position, initializer); }

void sheet_t::add_output(name_t output, const line_position_t& position, const array_t& expression)
{ object_m->add_output(output, position, expression); }

void sheet_t::add_constant(name_t constant, const line_position_t& position, 
                           const array_t& initializer)
{ object_m->add_constant(constant, position, initializer); }

void sheet_t::add_logic(name_t logic, const line_position_t& position, const array_t& expression)
{ object_m->add_logic(logic, position, expression); }

void sheet_t::add_invariant(name_t invariant, const line_position_t& position, 
                            const array_t& expression)
{ object_m->add_invariant(invariant, position, expression); }

void sheet_t::add_interface(name_t name, bool linked, const line_position_t& position1, 
                            const array_t& initializer, const line_position_t& position2, 
                            const array_t& expression)
{ object_m->add_interface(name, linked, position1, initializer, position2, expression); }

void sheet_t::add_relation(const line_position_t& position, const array_t& conditional,
        const relation_t* first, const relation_t* last)
{ object_m->add_relation(position, conditional, first, last); }

sheet_t::connection_t sheet_t::monitor_value(name_t output, const monitor_value_t& monitor)
{ return object_m->monitor_value(output, monitor); }

sheet_t::connection_t sheet_t::monitor_contributing(name_t output, const dictionary_t& mark,
                                                    const monitor_contributing_t& monitor)
{ return object_m->monitor_contributing(output, mark, monitor); }

sheet_t::connection_t sheet_t::monitor_enabled(name_t input, const name_t* first, 
                                                  const name_t* last, 
                                                  const monitor_enabled_t& monitor)
{ return object_m->monitor_enabled(input, first, last, monitor); }

#if 0
sheet_t::connection_t sheet_t::monitor_invariant_contributing(name_t input, 
                                                              const monitor_invariant_t& monitor)
{ return object_m->monitor_invariant_contributing(input, monitor); }
#endif

sheet_t::connection_t sheet_t::monitor_invariant_dependent(name_t output, 
                                                           const monitor_invariant_t& monitor)
{ return object_m->monitor_invariant_dependent(output, monitor); }

bool sheet_t::has_input(name_t name) const
{ return object_m->has_input(name); }

bool sheet_t::has_output(name_t name) const
{ return object_m->has_output(name); }

void sheet_t::update()
{ object_m->update(); }

void sheet_t::reinitialize()
{ object_m->reinitialize(); }

void sheet_t::set(const dictionary_t& dictionary)
{ object_m->set(dictionary); }

any_regular_t sheet_t::get(name_t cell)
{ return object_m->get(cell); }

dictionary_t sheet_t::contributing(const dictionary_t& mark) const
{ return object_m->contributing(mark); }

dictionary_t sheet_t::contributing() const
{ return object_m->contributing(dictionary_t()); }

dictionary_t sheet_t::contributing_to_cell(name_t x) const
{ return object_m->contributing_to_cell(x); }

void sheet_t::print(std::ostream& os) const
{ object_m->print(os); }

/**************************************************************************************************/

sheet_t::implementation_t::implementation_t(virtual_machine_t& machine) :
    name_index_m(boost::hash<name_t>(), equal_to(), &cell_t::name_m),
    input_index_m(boost::hash<name_t>(), equal_to(), &cell_t::name_m),
    output_index_m(boost::hash<name_t>(), equal_to(), &cell_t::name_m),
    priority_high_m(0),
    priority_low_m(0),
    machine_m(machine),
    get_count_m(0),
    has_output_m(false),
    initialize_mode_m(false)
#ifndef NDEBUG
    ,
    updated_m(false),
    check_update_reentrancy_m(false)
#endif
{ }

/**************************************************************************************************/

any_regular_t sheet_t::implementation_t::inspect(const array_t& expression)
{
    machine_m.evaluate(expression);
    
    any_regular_t result = ::adobe::move(machine_m.back());
    machine_m.pop_back();
    
    return result;
}
    
/**************************************************************************************************/

void sheet_t::implementation_t::set(name_t n, const any_regular_t& v)
{
    index_t::iterator iter(input_index_m.find(n));
    if (iter == input_index_m.end())
    {
        throw std::logic_error(make_string("input cell ", n.c_str(), " does not exist."));
    }

    ++priority_high_m;
    iter->state_m = v;
    iter->priority_m = priority_high_m;
    
    // Leave contributing untouched.
    
    if (iter->specifier_m == access_input) init_dirty_m.set(iter->cell_set_pos_m);
}
    
/**************************************************************************************************/

void sheet_t::implementation_t::touch(const name_t* first, const name_t* last)
{
    // REVISIT (sparent) : This should be constrained to interface cells only.
    // REVISIT (sparent) : This logic is similar to the logic in flow and should be the same.

    // build an index of the cells to touch sorted by current priority.
    
    typedef table_index<priority_t, cell_t> priority_index_t;
    
    priority_index_t index(&cell_t::priority_m);
    
    // REVISIT (sparent) : This loop is transform
    
    while (first != last)
    {
        index_t::iterator iter(input_index_m.find(*first));
        if (iter == input_index_m.end())
        {
            throw std::logic_error(make_string("input cell ", first->c_str(), " does not exist."));
        }
        
        index.push_back(*iter);
        ++first;
    }
    
    index.sort();
    
    // Touch the cells - keeping their relative priority
    
    for (priority_index_t::iterator f(index.begin()), l(index.end()); f != l; ++f)
    {
        ++priority_high_m;
        f->priority_m = priority_high_m;
    }
}

/**************************************************************************************************/

void sheet_t::implementation_t::add_input(name_t name, const line_position_t& position,
                                          const array_t& initializer)
{
    if (added_cells_m.empty() || added_cells_m.back().access_m != access_input)
        added_cells_m.push_back(added_cell_set_t(access_input));
    added_cells_m.back().added_cells_m.push_back(input_parameters_t(name, position, initializer));

    scope_value_t<bool> scope(initialize_mode_m, true);
    
    any_regular_t initial_value;

    if (initializer.size()) initial_value = calculate_expression(position, initializer);

    cell_set_m.push_back(cell_t(name, ::adobe::move(initial_value), cell_set_m.size()));
    // REVISIT (sparent) : Non-transactional on failure.
    input_index_m.insert(cell_set_m.back());
}
    
/**************************************************************************************************/

void sheet_t::implementation_t::add_output(name_t output, const line_position_t& position,
        const array_t& expression)
{
    if (added_cells_m.empty() || added_cells_m.back().access_m != access_output)
        added_cells_m.push_back(added_cell_set_t(access_output));
    added_cells_m.back().added_cells_m.push_back(output_parameters_t(output, position, expression));

    // REVISIT (sparent) : Non-transactional on failure.
    cell_set_m.push_back(cell_t(access_output, output, 
                                boost::bind(&implementation_t::calculate_expression,
                                            boost::ref(*this), position, expression),
                                cell_set_m.size()));

    output_index_m.insert(cell_set_m.back());
    
    if (!name_index_m.insert(cell_set_m.back()).second) {
        throw stream_error_t(make_string("cell named '", output.c_str(), "'already exists."), position);
    }
    
    has_output_m = true;
}
    

/**************************************************************************************************/
// REVISIT (sparent) : Hacked glom of input/output pair.

void sheet_t::implementation_t::add_interface(name_t name, bool linked,
                                              const line_position_t& position1,
                                              const array_t& initializer_expression,
                                              const line_position_t& position2, 
                                              const array_t& expression)
{
    if (added_cells_m.empty() || added_cells_m.back().access_m != access_interface_input)
        added_cells_m.push_back(added_cell_set_t(access_interface_input));
    added_cells_m.back().added_cells_m.push_back(interface_parameters_t(name,
                                                                        linked,
                                                                        position1,
                                                                        initializer_expression,
                                                                        position2, 
                                                                        expression));

    scope_value_t<bool> scope(initialize_mode_m, true);

    if (initializer_expression.size()) {
        cell_set_m.push_back(cell_t(name, linked, boost::bind(&implementation_t::calculate_expression,
            boost::ref(*this), position1, initializer_expression), cell_set_m.size()));
    } else {
        cell_set_m.push_back(cell_t(name, linked, cell_t::calculator_t(), cell_set_m.size()));
    }

    // REVISIT (sparent) : Non-transactional on failure.
    input_index_m.insert(cell_set_m.back());

    if (initializer_expression.size())
        initialize_one(cell_set_m.back());

    if (expression.size())
    {
    // REVISIT (sparent) : Non-transactional on failure.
        cell_set_m.push_back(cell_t(access_interface_output, name, 
                                    boost::bind(&implementation_t::calculate_expression,
                                                boost::ref(*this), position2, expression),
                                    cell_set_m.size(), &cell_set_m.back()));
    }
    else
    {
        cell_set_m.push_back(cell_t(access_interface_output, name, 
                                    boost::bind(&implementation_t::get, boost::ref(*this), name),
                                    cell_set_m.size(), &cell_set_m.back()));
    }
    output_index_m.insert(cell_set_m.back());
    
    if (!name_index_m.insert(cell_set_m.back()).second) {
        throw stream_error_t(make_string("cell named '", name.c_str(), "'already exists."), position2);
    }
}
    
/**************************************************************************************************/

void sheet_t::implementation_t::add_constant(name_t name, const line_position_t& position,
        const array_t& initializer)
{
    if (added_cells_m.empty() || added_cells_m.back().access_m != access_constant)
        added_cells_m.push_back(added_cell_set_t(access_constant));
    added_cells_m.back().added_cells_m.push_back(constant_parameters_t(name, position, initializer));

    scope_value_t<bool> scope(initialize_mode_m, true);

    cell_set_m.push_back(cell_t(access_constant, name, 
                                calculate_expression(position, initializer),
                                cell_set_m.size()));
    // REVISIT (sparent) : Non-transactional on failure.
    
    if (!name_index_m.insert(cell_set_m.back()).second) {
        throw stream_error_t(make_string("cell named '", name.c_str(), "'already exists."), position);
    }
}
    
/**************************************************************************************************/

void sheet_t::implementation_t::add_logic(name_t logic, const line_position_t& position,
        const array_t& expression)
{
    if (added_cells_m.empty() || added_cells_m.back().access_m != access_logic)
        added_cells_m.push_back(added_cell_set_t(access_logic));
    added_cells_m.back().added_cells_m.push_back(logic_parameters_t(logic, position, expression));

    cell_set_m.push_back(cell_t(access_logic, logic, 
                                boost::bind(&implementation_t::calculate_expression,
                                            boost::ref(*this), position, expression),
                                cell_set_m.size()));
    
    if (!name_index_m.insert(cell_set_m.back()).second) {
        throw stream_error_t(make_string("cell named '", logic.c_str(), "'already exists."), position);
    }
}
    
/**************************************************************************************************/

void sheet_t::implementation_t::add_invariant(name_t invariant, const line_position_t& position,
        const array_t& expression)
{
    if (added_cells_m.empty() || added_cells_m.back().access_m != access_invariant)
        added_cells_m.push_back(added_cell_set_t(access_invariant));
    added_cells_m.back().added_cells_m.push_back(invariant_parameters_t(invariant, position, expression));

    // REVISIT (sparent) : Should invariants also go in name_index_m?
    cell_set_m.push_back(cell_t(access_invariant, invariant, 
                                boost::bind(&implementation_t::calculate_expression,
                                            boost::ref(*this), position, expression),
                                cell_set_m.size()));
    invariant_index_m.push_back(&cell_set_m.back());
}

/**************************************************************************************************/

void sheet_t::implementation_t::add_relation(const line_position_t& position,
        const array_t& conditional, const relation_t* first, const relation_t* last)
{
    if (added_cells_m.empty() || added_cells_m.back().access_m != access_logic)
        added_cells_m.push_back(added_cell_set_t(access_logic));
    added_cells_m.back().added_cells_m.push_back(relation_parameters_t(position, conditional, first, last));

    relation_cell_set_m.push_back(relation_cell_t(position, conditional, first, last));
    
    for (; first != last; ++first) {
        index_t::iterator p = output_index_m.find(first->name_m);
        
        if (p == output_index_m.end() || !p->interface_input_m)
            throw stream_error_t(make_string("interface cell ", first->name_m.c_str(), " does not exist."), position);
            
        p->relation_index_m.push_back(&relation_cell_set_m.back());
        ++p->initial_relation_count_m;
    }
}

/**************************************************************************************************/

any_regular_t sheet_t::implementation_t::calculate_expression(
        const line_position_t& position,
        const array_t& expression)
{
    evaluate(machine_m, position, expression);
    
    any_regular_t result = ::adobe::move(machine_m.back());
    machine_m.pop_back();
        
    return result;
}

/**************************************************************************************************/

sheet_t::connection_t 
sheet_t::implementation_t::monitor_enabled(name_t n, const name_t* first, const name_t* last,
                                              const monitor_enabled_t& monitor)
{
    assert(updated_m && "Must call sheet_t::update() prior to monitor_enabled.");
    index_t::iterator iter(input_index_m.find(n));

    if (iter == input_index_m.end()) 
        throw std::logic_error(make_string("Attempt to monitor nonexistent cell: ", n.c_str()));

    cell_bits_t touch_set;
    while(first != last) {
        index_t::iterator i(input_index_m.find(*first));
        if (i == input_index_m.end()) 
            throw std::logic_error(make_string("Attempt to monitor nonexistent cell: ", 
                                               first->c_str()));
        touch_set.set(i->cell_set_pos_m);
        ++first;
        
    }

    monitor(active_m.test(iter->cell_set_pos_m) || (touch_set & priority_accessed_m).any());

    return monitor_enabled_m.connect(boost::bind(&sheet_t::implementation_t::enabled_filter,
                                                 this, touch_set, iter->cell_set_pos_m, monitor,
                                                 _1, _2));
}

/**************************************************************************************************/
sheet_t::connection_t 
sheet_t::implementation_t::monitor_invariant_dependent(name_t n, 
                                                       const monitor_invariant_t& monitor)
{
    assert(updated_m && "Must call sheet_t::update() prior to monitor_invariant_dependent.");

    index_t::iterator iter(output_index_m.find(n));

    if (iter == output_index_m.end()) 
        throw std::logic_error(make_string("Attempt to monitor nonexistent cell: ", n.c_str()));

    monitor(iter->invariant_m);

    return iter->monitor_invariant_m.connect(monitor);
}

/**************************************************************************************************/

sheet_t::connection_t
sheet_t::implementation_t::monitor_value(name_t name, const monitor_value_t& monitor)
{
    assert(updated_m && "Must call sheet_t::update() prior to monitor_value.");
    
    cell_t* cell_ptr(NULL);

    index_t::iterator iter(output_index_m.find(name));
    bool is_output_cell(iter != output_index_m.end());
    if(is_output_cell)
    {
        cell_ptr = &*iter;
    }
    else
    {
        index_vector_t::iterator i(adobe::find_if(invariant_index_m, bind(&cell_t::name_m, _1)));
        if(i == invariant_index_m.end())
            throw std::logic_error(make_string("Attempt to monitor nonexistent cell: ", name.c_str()));
        cell_ptr = *i;
    }

    monitor(cell_ptr->state_m);

    return cell_ptr->monitor_value_m.connect(monitor);
}

/**************************************************************************************************/

sheet_t::connection_t 
sheet_t::implementation_t::monitor_contributing(name_t n, const dictionary_t& mark, 
                                                const monitor_contributing_t& monitor)
{
    assert(updated_m && "Must call sheet_t::update() prior to monitor_contributing.");

    index_t::iterator       iter(output_index_m.find(n));

    if (iter == output_index_m.end())
    {
        throw std::logic_error(make_string("Attempt to monitor nonexistent cell: ", n.c_str()));
    }

    monitor(contributing_set(mark, iter->contributing_m));

    return iter->monitor_contributing_m.connect(
            boost::bind(monitor, boost::bind(&sheet_t::implementation_t::contributing_set,
                    boost::ref(*this), mark, _1)));
}

/**************************************************************************************************/

inline bool sheet_t::implementation_t::has_input(name_t name) const
{
    return input_index_m.find(name) != input_index_m.end();
}

/**************************************************************************************************/

inline bool sheet_t::implementation_t::has_output(name_t name) const
{
    return output_index_m.find(name) != output_index_m.end();
}

/**************************************************************************************************/

priority_t sheet_t::implementation_t::name_to_priority(name_t name) const
{
     index_t::const_iterator i = input_index_m.find(name);
     assert(i != input_index_m.end() && i->specifier_m == access_interface_input
        && "interface cell not found, should not be possible - preflight in add_interface.");
     return i->priority_m;
}

/**************************************************************************************************/

void sheet_t::implementation_t::flow(cell_bits_t& priority_accessed)
{
    vector<name_t> cell_names;
    
    for (relation_cell_set_t::iterator f(relation_cell_set_m.begin()), l(relation_cell_set_m.end());
            f != l; ++f) {
        if (!f->resolved_m) {
            transform(f->terms_m, std::back_inserter(cell_names), &relation_t::name_m);
        }
    }
    
    // Make the list unique
    sort(cell_names);
    unique(cell_names);
    
    // Sort the names by the associate priority
    sort(cell_names, less(), boost::bind(&implementation_t::name_to_priority, this, _1));

    for (vector<name_t>::const_iterator f = cell_names.begin(), l = cell_names.end(); f != l; ++f) {
        index_t::const_iterator i = input_index_m.find(*f);
        assert(i != input_index_m.end() && i->specifier_m == access_interface_input
            && "interface cell not found, should not be possible - preflight in add_interface.");
        priority_accessed.set(i->cell_set_pos_m);
    }
            
    /*
    pop the top cell from the stack
    if the cell is not resolved then resolve as a contributor
    find which relations the cell contributes to if any-
        if there is only one unresolved cell on the relation then
            resolve that cell as derived
                (need to have the cell refer to the deriving term in the relation?)
            push the cell to the top of stack
    loop until stack is empty.
    */
    
    while(!cell_names.empty()) {
        name_t name = cell_names.back(); cell_names.pop_back();

        index_t::iterator i = output_index_m.find(name);
        /*
            REVSIT (sparent) : I need to make sure that only interface cells are name by the relate
            clauses prior to this.
        */
        assert(i != output_index_m.end());
        
        if (i->relation_count_m == 0) continue;
        
        i->resolved_m = true;
        
        // REVISIT (sparent) : Need an index here to find the relations quickly.
        
        for (relation_index_t::iterator f = i->relation_index_m.begin(), l = i->relation_index_m.end();
                f != l; ++f) {
                
            if ((*f)->resolved_m) continue;
                        
            --i->relation_count_m;
            
            cell_t* cell_to_resolve     = 0;
            const   relation_t* term    = 0;
            bool    at_least_one        = false;
            
            for (relation_set_t::iterator tf((*f)->terms_m.begin()), tl((*f)->terms_m.end()); tf != tl; ++tf) {
            
                index_t::iterator iter(output_index_m.find(tf->name_m));
                assert(iter != output_index_m.end());
                
                cell_t& cell(*iter);
                
                if (cell.resolved_m) continue;
                
                if (!cell_to_resolve) {
                    cell_to_resolve = &cell;
                    term = &(*tf);
                    at_least_one = true;
                } else {
                    cell_to_resolve = NULL; break;
                }
            }
            
            // REVISIT (sparent) : This is a runtime error.
            
            assert(at_least_one && "All terms of relation resolved but relation not applied.");
            
            if (!cell_to_resolve) continue;
            
            (*f)->resolved_m = true;          
            cell_to_resolve->resolved_m = true;
            cell_to_resolve->term_m = boost::bind(&implementation_t::calculate_expression,
                                            boost::ref(*this), term->position_m, term->expression_m); // cell needs to use the term for calculate.
            --cell_to_resolve->relation_count_m;
            
            // This will be a derived cell and will have a priority lower than any cell contributing to it
            
            assert(cell_to_resolve->interface_input_m && "Missing input half of interface cell.");
            if (cell_to_resolve->interface_input_m->linked_m) {
                cell_to_resolve->interface_input_m->priority_m = --priority_low_m;
            }
            
            if (cell_to_resolve->relation_count_m) cell_names.push_back(cell_to_resolve->name_m);
        }
        
        assert(i->relation_count_m == 0 && "Cell still belongs to relation but all relations resolved.");
    }
}

/**************************************************************************************************/

void sheet_t::implementation_t::update()
{
#ifndef NDEBUG
    check_reentrancy checker(check_update_reentrancy_m);
    updated_m = true;
#endif

    conditional_indirect_contributing_m.reset(); 
    
    value_accessed_m.reset();
 
    for_each(cell_set_m, &cell_t::clear_dirty);
    for_each(relation_cell_set_m, &relation_cell_t::clear_resolved);

// Solve the conditionals.

    accumulate_contributing_m.reset();
    
    for (relation_cell_set_t::iterator current_cell(relation_cell_set_m.begin()),
            last_cell(relation_cell_set_m.end()); current_cell != last_cell; ++current_cell)
    {
        if (current_cell->conditional_m.empty()) continue;
                                
        if (!calculate_expression(current_cell->position_m, current_cell->conditional_m).cast<bool>())
        {
            // remove this relation from any terms.
            for (relation_set_t::iterator current_term(current_cell->terms_m.begin()),
                     last_term(current_cell->terms_m.end()); current_term != last_term; 
                 ++current_term)
            {
                index_t::iterator iter(output_index_m.find(current_term->name_m));
                
                assert(iter != output_index_m.end() && "Cell was present when we added the relation...");
                
                --iter->relation_count_m;
            }
            current_cell->resolved_m = true;
        }
    }
        
    conditional_indirect_contributing_m = accumulate_contributing_m;
    
    cell_bits_t priority_accessed;
    
    flow(priority_accessed);
        
#ifndef NDEBUG
    for (relation_cell_set_t::iterator first(relation_cell_set_m.begin()),
            last(relation_cell_set_m.end()); first != last; ++first)
    {
        if (first->resolved_m) continue;
                

        std::clog << "(warning) relation unnecessary and ignored\n" << first->position_m;

    }
#endif

// calculate the output/interface_output cells and apply.
    
    for (index_t::const_iterator iter (output_index_m.begin()), last (output_index_m.end()); 
     iter != last; ++iter)
    {
        cell_t& cell(*iter);
        
        // REVISIT (sparent) : This is a copy/paste of get();
        
        
        if (!cell.evaluated_m) {
            accumulate_contributing_m.reset();
            
            get_stack_m.push_back(cell.name_m);
            
            cell.calculate();
            
            get_stack_m.pop_back();
            
            cell.contributing_m = accumulate_contributing_m;
            
            cell.contributing_m |= conditional_indirect_contributing_m;
        }
            
        /*
            REVISIT (sparent) : This would be slightly more efficient if I moved the link flag
            to the output side.
        */
        
        // Apply the interface output to interface inputs of linked cells.
        if (cell.interface_input_m && cell.interface_input_m->linked_m) {
            cell.interface_input_m->state_m = cell.state_m;
        }
    }
    
// Then we can check the invariants -
        
    cell_bits_t contributing;
    
    for (index_vector_t::const_iterator iter (invariant_index_m.begin()), 
             last (invariant_index_m.end()); iter != last; ++iter)
    {
        cell_t& cell(**iter);
        bool old_inv(false);
        if(!cell.monitor_value_m.empty())
            old_inv = cell.state_m.cast<bool>();
        cell.calculate();
        bool new_inv(cell.state_m.cast<bool>());
        if(!new_inv) contributing |= cell.contributing_m;
        if(old_inv != new_inv) cell.monitor_value_m(any_regular_t(new_inv));
    }
    
    
    /* REVISIT (sparent) : Shoule we report
     * conditional_indirect_contributing with the invariants? */
            
    /* REVISIT (sparent): Monitoring a value should return all of -
        value
        contributing
        invariant_dependent
        
        Otherwise the client risks getting out of sync.
    */
    
    cell_bits_t active;
    // REVIST (sparent) : input monitor should recieve priority_accessed and poison bits.

    for (index_t::const_iterator iter (output_index_m.begin()), last (output_index_m.end()); 
         iter != last; ++iter)
    {
        cell_t&         cell(*iter);
        bool            invariant ((contributing & cell.contributing_m).none());

        if (invariant != cell.invariant_m) cell.monitor_invariant_m(invariant);
        cell.invariant_m = invariant;

        if (cell.dirty_m) cell.monitor_value_m(cell.state_m);        

    
        /*
            REVISIT (sparent) : Is there any way to prune this down a
            bit? Calculating the contributing each time is expensive.
        */
    
        if (!cell.monitor_contributing_m.empty())
        {
            // REVISIT (sparent) : no need to notify if contributing didn't change...
            cell.monitor_contributing_m(cell.contributing_m);
        }

        // Build bitset of active cells:
        index_t::iterator input_iter(input_index_m.find(cell.name_m));
        const bool is_pure_output(input_iter == input_index_m.end());
        if (!is_pure_output)
        {  
            if(priority_accessed.test(input_iter->cell_set_pos_m)) 
//priority_accessed_m not updated yet, so we use priority_accessed
            {
                active.set(input_iter->cell_set_pos_m);
            }
        }
        if(is_pure_output || !has_output_m)
        {
            active |= cell.contributing_m;
        }
    }
    
    // update 
    monitor_enabled_m(priority_accessed, active);
    priority_accessed_m = priority_accessed;
    active_m = active;    
}

/**************************************************************************************************/

void sheet_t::implementation_t::initialize_one(cell_t& cell)
{
    /*
        REVISIT (sparent) : Should have more checking here - detecting cycles (and forward
        references?)
    */
    
    accumulate_contributing_m.reset();
    cell.state_m = cell.calculator_m();
    cell.priority_m = ++priority_high_m;
    cell.init_contributing_m |= accumulate_contributing_m;
}
 
/**************************************************************************************************/

void sheet_t::implementation_t::reinitialize()
{
    scope_value_t<bool> scope(initialize_mode_m, true);
    
    for (index_t::iterator f = output_index_m.begin(), l = output_index_m.end(); f != l; ++f) {
    
        if (!f->interface_input_m) continue;
        
        cell_t& cell = *f->interface_input_m;
        
        if ((init_dirty_m & cell.init_contributing_m).none()) continue;
            
        initialize_one(cell);
    }
    
    init_dirty_m.reset();
}

/**************************************************************************************************/

dictionary_t sheet_t::implementation_t::contributing(const dictionary_t& mark) const
{
    cell_bits_t contributing;
    
    for (index_t::const_iterator iter (output_index_m.begin()), last (output_index_m.end()); 
         iter != last; ++iter)
    {
// REVISIT (mmarcus) : check whether really want to exclude interface cells
        index_t::iterator input_iter(input_index_m.find(iter->name_m));
        const bool is_pure_output(input_iter == input_index_m.end());
        if(!has_output_m || is_pure_output)
            contributing |= iter->contributing_m;
    }
    
    return contributing_set(mark, contributing);
}

/**************************************************************************************************/

dictionary_t sheet_t::implementation_t::contributing_to_cell(name_t x) const
{   
    index_t::iterator iter = output_index_m.find(x);
    
    if (iter == output_index_m.end())
        throw std::logic_error(make_string("No monitorable cell: ", x.c_str()));
        
    return contributing_set(dictionary_t(), iter->contributing_m);
}

/**************************************************************************************************/

void sheet_t::implementation_t::print(std::ostream& os) const
{
    os << "sheet name_ignored\n"
       << "{\n";
    for (std::size_t i = 0; i < added_cells_m.size(); ++i) {
        const added_cell_set_t& cell_set = added_cells_m[i];
        if (i)
            os << '\n';
        switch (cell_set.access_m) {
        case access_input: os << "input:\n"; break;
        case access_interface_input: os << "interface:\n"; break;
        case access_output: os << "output:\n"; break;
        case access_logic: os << "logic:\n"; break;
        case access_constant: os << "constant:\n"; break;
        case access_invariant: os << "invariant:\n"; break;
        case access_interface_output: break;
        }
        for (std::size_t j = 0; j < cell_set.added_cells_m.size(); ++j) {
            boost::apply_visitor(print_visitor(os), cell_set.added_cells_m[j]);
        }
    }
    os << "}\n";
}

/**************************************************************************************************/
/*
    NOTE (sparent) : A mark containes a dictionary of contributing values.
    
    If a contributing value has changed or been added since the mark
    then it will be reported in the dictionary result.
    
    If a value is contributing, and the set of values which are
    contributing has changed, then the value will be reported as
    having been touched.
*/

dictionary_t sheet_t::implementation_t::contributing_set(const dictionary_t& mark,
        const cell_bits_t& contributing) const
{
    dictionary_t changed;
    dictionary_t touched;
    bool         include_touched(false);
    
    for (std::size_t index(0), last(cell_set_m.size()); index != last; ++index)
    {
        if (contributing[index])
        {
            const cell_t&           cell = cell_set_m[index];
            const name_t&           name(cell.name_m);
            const any_regular_t&    value(cell.state_m);
            bool                    priority_accessed(
                                        priority_accessed_m.test(cell.cell_set_pos_m));
            
            if (!mark.count(name))
            {
                include_touched = true;
                changed.insert(make_pair(name, value));
            } else if (get_value(mark, name) != value) changed.insert(make_pair(name, value));
            else if (priority_accessed) touched.insert(make_pair(name, value));
        }
    }
    
    if (include_touched) { changed.insert(touched.begin(), touched.end()); }
    return changed;
}

/**************************************************************************************************/

void sheet_t::implementation_t::set(const dictionary_t& dict)
{
    for (dictionary_t::const_iterator iter(dict.begin()), last(dict.end());
            iter != last; ++iter)
    {
        set(iter->first, iter->second);
    }
}

/**************************************************************************************************/

any_regular_t sheet_t::implementation_t::get(name_t variable_name)
{
    if (initialize_mode_m) {
        index_t::iterator iter(input_index_m.find(variable_name));
        
        if (iter == input_index_m.end()) {
            iter = name_index_m.find(variable_name);
            if (iter == name_index_m.end() || iter->specifier_m != access_constant) {
                throw std::logic_error(make_string("Variable ", variable_name.c_str(), 
                                                   " not found."));
            }
        }
        
        cell_t& cell = *iter;
        
        accumulate_contributing_m |= cell.init_contributing_m;
        return cell.state_m;
    }

    scope_count scope(get_count_m);
    
    /*
        REVISIT (sparent) - If we go to three pass on interface cells then the max count is
        number of cells plus number of interface cells.
    */
    
    if (get_count_m > cell_set_m.size())
    {
        throw std::logic_error(std::string("cycle detected, consider using a relate { } clause."));
    }
    
    // If the variable is on the top of the stack then we assume it
    // must refer to an input cell.
    
    if (get_stack_m.size() && (get_stack_m.back() == variable_name))
    {
        index_t::iterator iter(input_index_m.find(variable_name));
        if (iter == input_index_m.end()) 
        {
            throw std::logic_error(make_string("input variable ", variable_name.c_str(), 
                                               " not found."));
        }
        value_accessed_m.set(iter->cell_set_pos_m);
        
        accumulate_contributing_m |= iter->contributing_m;
        return iter->state_m;
    }
    
    cell_t* cell_ptr;
    
    index_t::iterator iter (name_index_m.find(variable_name));
    if (iter != name_index_m.end()) cell_ptr = &(*iter);
    else
    {
        index_t::iterator iter(input_index_m.find(variable_name));
        if (iter == input_index_m.end()) 
        {  
            throw std::logic_error(make_string("variable ", variable_name.c_str(), " not found."));
        }
        cell_ptr = &(*iter);
    }
    
    cell_t& cell(*cell_ptr);
    
    if (cell.specifier_m == access_interface_output) get_stack_m.push_back(variable_name);
    
    if (cell.specifier_m == access_interface_input) value_accessed_m.set(cell.cell_set_pos_m);
// REVISIT (sparent) : paired call should be ctor/dtor

    try
    {
        // REVISIT (sparent) : First pass getting the logic correct.
        
        if (cell.evaluated_m) accumulate_contributing_m |= cell.contributing_m;
        else {
            cell_bits_t old = accumulate_contributing_m;
            accumulate_contributing_m.reset();
            
            cell.calculate();
            
            cell.contributing_m = accumulate_contributing_m;
            accumulate_contributing_m |= old;
        }

        if(cell.specifier_m != access_input && cell.specifier_m != access_constant)
            cell.contributing_m |= conditional_indirect_contributing_m;
    }
    catch (...)
    {
        if (cell.specifier_m == access_interface_output) get_stack_m.pop_back();
        throw;
    }

    if (cell.specifier_m == access_interface_output) get_stack_m.pop_back();
    
    return cell.state_m;
}

/**************************************************************************************************/

} // namespace adobe

/**************************************************************************************************/
