/*
    Copyright 2005-2008 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_ADAM_HPP
#define ADOBE_ADAM_HPP

#include <GG/adobe/config.hpp>

#include <functional>

#include <boost/utility.hpp>
#include <boost/function.hpp>

#ifdef __MWERKS__
    #pragma warn_unusedarg off
    #pragma warn_unusedvar off
#endif

#include <boost/signals.hpp>

#ifdef __MWERKS__
    #pragma warn_unusedarg reset
    #pragma warn_unusedvar reset
#endif

#include <GG/adobe/any_regular_fwd.hpp>
#include <GG/adobe/array.hpp>
#include <GG/adobe/dictionary_fwd.hpp>
#include <GG/adobe/istream.hpp>
#include <GG/adobe/move.hpp>
#include <GG/adobe/name_fwd.hpp>
#include <GG/adobe/virtual_machine.hpp>

/*

    REVISIT (sparent) : It would be best to detangle the sheet from
    the virtual machine. The way to do this is to allow for funciton
    objects to be passed instead of line positions and expression
    arrays. The function object could bind to the line_position_t and
    the array.... This would allow for easier programatic driving -
    however, one would have to construct condition objects which
    tracked dependencies and the sheet would require a get() method to
    retrieve the value of a cell.

*/

/*************************************************************************************************/

namespace adobe {

/*!
\defgroup property_model Property Model Library (Adam)
\ingroup asl_libraries

\defgroup adam_engine Property Model Engine
\ingroup property_model
*/

/*************************************************************************************************/

/*!
    \ingroup adam_engine

    \brief The fundamental data structure for the Property Model
    engine.
*/
class sheet_t : boost::noncopyable
{
 public:
    struct relation_t;
    
    typedef boost::function<void (bool)>                 monitor_invariant_t;
    typedef boost::function<void (const any_regular_t&)> monitor_value_t;
    typedef boost::function<void (const dictionary_t&)>  monitor_contributing_t;
    typedef boost::function<void (bool)>                 monitor_enabled_t;
                                                           
/*!  

  An object that maintains the function callback validity between the
  Adam engine and the client code. These are simply
  boost::signals::connection objects, and are the responsibility of
  the client to maintain. They retain the same lifespans and semantics
  as described in the boost documentation. Typically the client code
  should group together all connections between and Adam sheet and
  their client code. When the time for destroying the client code or
  the Adam sheet comes, these connections should be disconnected to
  prevent communication between objects being destroyed.
*/

    typedef boost::signals::connection                   connection_t;
     
 #if !defined(ADOBE_NO_DOCUMENTATION)
    sheet_t();
    ~sheet_t();
 #endif
 
/*!

  \param expression token stack describing a virtual machine
  expression.

  \return The value evaluated out of the expression.
*/
    any_regular_t inspect(const array_t& expression);

/*!

  \param cell the <i>input cell</i> to which the value is to be set.

  \param value the value to set the input cell to.
*/
    void set(name_t cell, const any_regular_t& value); // input cell.

/*! 

  Touch a collection of input cells denoted by a range of names
  [first, last). Touching a cell raises the priority of the cell as if
  it were <code>set</code>, without changing the value of the
  cell. The relative priority of the cells within the range is
  preserved. Touching can be used to control which cells are selected
  for derivation in a relation.

  \todo  (sparent) : I think that touch relationships will eventually need to
  be descriped within the sheet as opposed to simply being an
  imperative action for proper scripting. Also, a facility for user
  controlled touching which effects a collection of cells on any set
  operation should be added.

  \param first Start of range <code>[first, last)</code> of <i>input
  cells</i> to touch.

  \param last End of range <code>[first, last)</code> of <i>input
  cells</i> to touch.
*/
    void touch(const name_t* first, const name_t* last); // range of input cells.
    
/*!
    The get function is intended to be connected to the VM variable lookup by the client. During
    expression evaluation, triggered by initialization, reinitialize, or update the VM can call
    get() to return the value of a variable. Outside of these calls the get() function may be
    called to get the most recent ouput value of the cell cashed from the last call to update().

    \param name of cell to calculate/get the value.
*/
    any_regular_t get(name_t cell);
    
/*!

  Add an input cell to the sheet.

  \param name name of the cell being added.

  \param position position in the parse of the cell definition.

  \param initializer expression to be evaluated for the cell's
  starting value.
*/
    void add_input      (name_t name, const line_position_t& position, const array_t& initializer);

/*!

  Add an output cell to the sheet.

  \param name name of the cell being added.

  \param position position in the parse of the cell definition.

  \param expression expression to be evaluated for the cell's output
  value.
*/

    void add_output     (name_t name, const line_position_t& position, const array_t& expression);


/*!

  Add a constant cell to the sheet.

  \param name name of the cell being added.

  \param position position in the parse of the cell definition.

  \param initializer expression to be evaluated for the cell's value.
*/
    void add_constant   (name_t name, const line_position_t& position, const array_t& initializer);

/*!

  Add a logic cell to the sheet.

  \param name name of the cell being added.

  \param position position in the parse of the cell definition.

  \param expression expression to be evaluated for the cell's output
  value.
*/
    void add_logic      (name_t name, const line_position_t& position, const array_t& expression);

/*!

  Add an invariant cell to the sheet.

  \param name name of the cell being added.

  \param position position in the parse of the cell definition.

  \param expression expression to be evaluated for the cell's output
  value.
*/
    void add_invariant  (name_t name, const line_position_t& position, const array_t& expression);

/*!  

  \par 
  
  Add an interface cell to the sheet. An interface cell combines all
  aspects of both the input and output cell types.

  \param name name of the cell being added.

  \param linked specifies whether or not the output value is
  automatically applied back to the input value of the cell.

  \param position position in the parse of the initializer definition.

  \param initializer expression to be evaluated for the cell's value

  \param position2 position in the parse of the expression definition.

  \param expression expression to be evaluated for the cell's output value.
*/
    void add_interface  (name_t name, bool linked, const line_position_t& position1,
                         const array_t& initializer, const line_position_t& position2, 
                         const array_t& expression);
    

/*!

  Add a relation logic cell to the sheet. When the sheet is updated,
  if the conditional evalutes to true then exactly one of the
  relations in the relation set will be evaluated. Which relation is
  executed is determined by evaluating cells based on their priority
  in the sheet until there is only a single cell remaining which must
  derived by the relationship.

  \todo (sparent) : A warning should be issued if the system is over
  constrained and no relations are evaluated but currently isn't.

  \param position position in the parse of the cell definition.

  \param conditional expression (if present) that must be evaluated to
  <code>true</code> in order for tese relations to be considered.

  \param first the first relation in the relation set.

  \param last one-past-the last relation in the relation set.
*/
    void add_relation(const line_position_t& position, const array_t& conditional,
                      const relation_t* first, const relation_t* last);


/*!
    
  Establishes a callback for a cell to be called when the value of the
  cell changes.

  \param cell the name of the cell to monitor.
  
  \param proc the \ref concept_convertible_to_function to be called
  with the new cell value.

  \return A connection_t that maintains the sheet-to-callback
  link. This link should be broken before the sheet or the client code
  are destroyed to prevent communication between destructing objects.
*/
    connection_t monitor_value(name_t name, const monitor_value_t& proc);

/*!

  Establishes a callback for a cell to be called when the contributing
  cells to the cell changes.

  \param cell the name of the cell to monitor.
  
  \param mark a "bookmark" used to indicate the start point from which
  contribution changes are to be measured. Any changes to the cell's
  contributing cells after this point are considered contributing and
  will be notified through the callback. A mark is obtained by calling
  sheet_t::contributing().

  \param proc the \ref concept_convertible_to_function to be called
  with the new contributing cell values.

  \return A connection_t that maintains the adam-to-callback
  link. This link should be broken before the Adam sheet or the client
  code are destroyed to prevent communication between destructing
  objects.
*/
    connection_t monitor_contributing(name_t cell, const dictionary_t& mark, 
                                      const monitor_contributing_t& proc);
    // output only

/*!
  Enabled status is a conservative approximation which is false if a
  change to the cell cannot affect the output of a correct sheet and
  true if it may effect the output.

  Establishes a callback to be called when an interface cell's enabled
  status changes. A cell's enablement is calculated from a cell and
  from an optional collection of cells referred to as a touch-set. The
  definition of enablement depends on several other definitions

  1) an "output" cell is a cell that appears in the output section
  of a sheet
 
  2) an "interface-output" is the ouput value of an interface cell

  3) an interface cell's "priority-accessed" is true if during the
  last update the cell's priority was accessed

  4) an interface cell's "value-contributed" is true if during the
  last update the cell's value contributed to either: a pure output
  cell's value, or if there are no pure output cells, to an
  interface-output cell.

  5) an interface cell's "active" == "priority-accessed" || 
  "value_contributed"

  6) an interface cell's "value-accessed" is true if during the last
  update the cell's value was accessed

  7) an interface cell's "enabled" is true if during the last update:
  active(cell) || (value-accessed(cell) && (for some cell x in the
  touchlist: prirority-accessed(x)))


  \param cell the name of the cell whose enabled status to monitor.
  
  \param first Start of range <code>[first, last)</code> of
  <i>interface cells</i> whose priority_accessed status will be
  monitored. If range is empty this must be NULL.

  \param last End of range <code>[first, last)</code> of <i>interface
  cells</i> whose priority_accessed status will be monitored.  If
  range is empty this must be NULL.


  \param proc the \ref concept_convertible_to_function to be called
  with the enabled state changes.

  \return A connection_t that maintains the sheet-to-callback
  link. This link should be broken before the sheet or the client code
  are destroyed to prevent communication between destructing objects.
*/
    connection_t monitor_enabled(name_t cell, const name_t* first, const name_t* last,
                                    const monitor_enabled_t& proc); // input only

    #if 0
    connection_t monitor_invariant_contributing(name_t input, const monitor_invariant_t&);
    #endif

/*!
    
  Establishes a callback for an output cell to be called when an
  invariant state changes.

  \param output the name of the output cell to which an invariant is
  bound.
  
  \param proc the \ref concept_convertible_to_function to be called
  with the invariant state change.

  \return A connection_t that maintains the adam-to-callback
  link. This link should be broken before the Adam sheet or the client
  code are destroyed to prevent communication between destructing
  objects.
*/
    connection_t monitor_invariant_dependent(name_t output, const monitor_invariant_t& proc);


/*!
    
  Predicate to determine wheter an input (or interface) cell with a
  given name exists.

  \param name the name of the cell to query

  \return Whether an input (or interface) cell with the given name
  exists.
*/
    bool has_input(name_t name) const;

/*!
    
  Predicate to determine wheter an output (or interface) cell with a
  given name exists.

  \param name the name of the cell to query

  \return Whether an output (or interface) cell with the given name
  exists.
*/
    bool has_output(name_t name) const;


/*!
    
  Updates all the Adam cells in the sheet depending on the values
  changed by calls to set(). This function will utilize the monitor
  callbacks to notify the client of any cell value changes that
  resulted from contributing values being applied to the sheet.
*/
    void update();
    
/*!
    
  Input cells are re-initialized, in sheet order, and interface cell
  initializers are re-evaluated. Priorities are updated, but no
  callbacks are triggered. Calls to reinitialize should be followed by
  calls to \ref update, just as if the cells were updated by \ref set.

*/
    void reinitialize();


/*!
    
  Sets various cells in the sheet to various values.

  \param dictionary the dictionary with the values to set in the
  sheet. The keys in the dictionary will be used to map thier values
  to cells in the sheet.
*/
    void set(const dictionary_t& dictionary); 
    
    #if 0
    dictionary_t current_mark() const;
    #endif


/*!
  \param mark A previous result from sheet_t::contributing().

  \return A dictionary with the minimal set of values such that if you
  were to call set(mark), followed by set(result), the output state of
  the sheet would be restored.
*/
    dictionary_t contributing(const dictionary_t& mark) const;

/*!
  \return A dictionary of the currently contributing cells and their
  values. Given an equivalent instance of the sheet, sheet.set(result)
  will ensure that the output of the sheet is restored.
*/

    dictionary_t contributing() const;
    
/*!
    \return A dictionary of the currently contributing cells and their values to the named cell.
*/
    dictionary_t contributing_to_cell(name_t) const;

    void print(std::ostream& os) const;

    /*
        REVISIT (fbrereto) : From a note from sparent 2007/04/13:

        "Make the adam VM public - eventually I'm going to pull it out all
        together (The property model library won't directly depend on it -
        only function object - which can be implemented with the VM)."

        This solution is a bit hackish, but is a viable intermediary solution;
        because sheet_t and its underlying implementation are noncopyable, the
        machine can be stored here and held by reference by the implementation,
        and everything is fine. Nevertheless, this is an interim solution given
        Sean's plans for the VM's relationship to sheet_t in the future.
    */
    virtual_machine_t machine_m;

 private:
    class implementation_t;
    implementation_t* object_m;
};

/*************************************************************************************************/

/*!
\ingroup adam_engine

\brief Adam support class.
*/

struct set_monitor_t : std::unary_function<any_regular_t, void>
{
    set_monitor_t(sheet_t& sheet, name_t cell_name) :
        cell_name_m(cell_name),
        sheet_m(sheet)
    { }
    
    void operator()(const any_regular_t& x)
    { sheet_m.get().set(cell_name_m, x); }
    
 private:
    name_t                               cell_name_m;
    boost::reference_wrapper<sheet_t>    sheet_m;
};

/*************************************************************************************************/

/*
    REVISIT (sparent) : line_position_t and array_t need to go in favor of a function object.
*/

/*!
\ingroup adam_engine

\brief Adam support class for related relationships.
*/
struct sheet_t::relation_t
{
    relation_t() { }
    relation_t(name_t n, line_position_t p, array_t e) :
        name_m(n),
        position_m(p),
        expression_m(::adobe::move(e))
    { }
    
    friend void swap(relation_t& x, relation_t& y)
    {
        swap(x.name_m, y.name_m);
        swap(x.position_m, y.position_m);
        swap(x.expression_m, y.expression_m);
    }
    
    relation_t(move_from<relation_t> x) :
        name_m(x.source.name_m),
        position_m(x.source.position_m),
        expression_m(::adobe::move(x.source.expression_m))
    { }
    
    relation_t& operator=(relation_t x) 
    { swap(*this, x); return *this; }


    name_t          name_m;
    line_position_t position_m;
    array_t         expression_m;
};

/*************************************************************************************************/
    
} // namespace adobe

/*************************************************************************************************/

#endif

/*************************************************************************************************/
