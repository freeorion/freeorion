// -*- C++ -*-
#ifndef _OptionsDB_h_
#define _OptionsDB_h_

#include <boost/serialization/shared_ptr.hpp>

#include "OptionValidators.h"
#include "XMLDoc.h"

#include <boost/any.hpp>
#include <boost/signal.hpp>

#include <iosfwd>
#include <map>


class OptionsDB;

/////////////////////////////////////////////
// Free Functions
/////////////////////////////////////////////
typedef void (*OptionsDBFn)(OptionsDB&); ///< the function signature for functions that add Options to the OptionsDB (void (OptionsDB&))

/** adds \a function to a vector of pointers to functions that add Options to
  * the OptionsDB.  This function returns a boolean so that it can be used to
  * declare a dummy static variable that causes \a function to be registered as
  * a side effect (e.g. at file scope:
  * "bool unused_bool = RegisterOption(&foo)"). */
bool RegisterOptions(OptionsDBFn function);

/** returns the single instance of the OptionsDB class */
OptionsDB& GetOptionsDB();


/////////////////////////////////////////////
// OptionsDB
/////////////////////////////////////////////
/** a database of values of arbitrarily mixed types that can be initialized
  * from an XML config file and/or the command line.  OptionsDB should be used
  * for initializing global settings for an application that should be
  * specified from the command line or from config files.  Such options might
  * be the resolution to use when running the program, the color depth, number
  * of players, etc.  The entire DB can be written out to config file, to later
  * be reloaded.  This allows runtime settings to be preserved from one
  * execution to the next, and still allows overrides of these settings from
  * the command line.
  * <br><br>OptionsDB must have its options and their types specified before
  * any values are assigned to those options.  This is because setting an
  * option in the DB requires the type of the option to be known in advance.
  * To specify the options, you may either use static initialization time or
  * normal runtime calls to Add() and AddFlag().  Note that the exact type of
  * added item must be specified with Add*(), so that subsequent calls to Get()
  * do not throw.  For instance, if you want to add an unsigned value
  * accessible as "foo", you should call: \verbatim
        Add("foo", "The number of foos.", 1u);\endverbatim
  * Making the same call as above with "1" instead of "1u" will cause a later
  * call to Get<unsigned int>("foo") to throw, since "foo" would be an int, not
  * an unsigned int.  To guard against this, you may wish to call Add() with an
  * explicit template parameterization, such as Add<unsigned int>(...).
  * <br><br>Flag options are just boolean values that are false by default.
  * Their values may be read and set normally, the same as any other boolean
  * option.  For exapmple, reading a flag's value form the DB can be done using
  * using: \verbatim
        Get<bool>(flag_name);endverbatim
  * <br><br>Adding options at static initialization time means that the options
  * specified will be available before main() is called, and yet you do not
  * have to fill main.cpp with all your option specifications.  Instead, you
  * can put them in the files in which their options are used.
  * <br><br>OptionsDB has an optional dotted notation for option names.  This
  * is important in use with XML only.  When options are specified as e.g.
  * "foo.bar1" and "foo.bar2", the resulting XML file will show them as:\verbatim
        <foo>
            <bar1>x</bar>
            <bar2>y</bar>
        </foo>\endverbatim
  * This allows options to be grouped in ways that are sensible for the
  * application.  This is only done as a convenience to the user.  It does not
  * change the way the options are treated in any way.  Note that is is
  * perfectly legal also to have an option "foo" containing a value "z" in the
  * example above.
  * <br><br>A few things should be said about the command-line version of
  * options.  All flag command-line options (specified with AddFlag()) are
  * assumed to have false as their default value.  This means that their mere
  * presence on the command line means that they indicate a value of true;
  * they need no argument.  For example, specifying "--help" on the command
  * line sets the option "help" in the DB to true, and leaving it out sets the
  * option to false.
  * <br><br>Long-form names should be preceded with "--", and the
  * single-character version should be preceded with "-".  An exception to this
  * is that multiple single-character (boolean) options may be run together
  * (e.g. "-cxvf").  Also, the last option in such a group may take an
  * argument, which must immediately follow the group, separated by a space as
  * (usual.
  * <br><br>Finally, note that std::runtime_error exceptions will be thrown any
  * time a problem occurs with an option (calling Get() for one that doesn't
  * exist, Add()ing one twice, etc.), and boost::bad_any_cast exceptions will
  * be thrown in situations in which an invalid type-conversion occurs,
  * including string-to-type, type-to-string or type-to-type as in the case of
  * Get() calls with the wrong tempate parameter.
  * \see RegisterOptions (for static-time options specification) */
class OptionsDB
{
public:
    /** \name Signal Types */ //@{
    typedef boost::signal<void ()>                     OptionChangedSignalType; ///< emitted when an option has changed
    typedef boost::signal<void (const std::string&)>   OptionAddedSignalType;   ///< emitted when an option is added
    typedef boost::signal<void (const std::string&)>   OptionRemovedSignalType; ///< emitted when an option is removed
    //@}

    /** validates a value for an option. throws std::runtime_error if no option
      * \a name exists.  throws bad_lexical_cast if \a value cannot be
      * converted to the type of the option \a name. */
    void        Validate(const std::string& name, const std::string& value) const;

    /** returns the value of option \a name. Note that the exact type of item
      * stored in the option \a name must be known in advance.  This means that
      * Get() must be called as Get<int>("foo"), etc. */
    template <class T>
    T           Get(const std::string& name) const
    {
        std::map<std::string, Option>::const_iterator it = m_options.find(name);
        if (it == m_options.end())
            throw std::runtime_error("OptionsDB::Get<>() : Attempted to get nonexistent option \"" + name + "\".");
        return boost::any_cast<T>(it->second.value);
    }

    /** returns the default value of option \a name. Note that the exact type
      * of item stored in the option \a name must be known in advance.  This
      * means that GetDefault() must be called as Get<int>("foo"), etc. */
    template <class T>
    T           GetDefault(const std::string& name) const
    {
        std::map<std::string, Option>::const_iterator it = m_options.find(name);
        if (it == m_options.end())
            throw std::runtime_error("OptionsDB::GetDefault<>() : Attempted to get nonexistent option \"" + name + "\".");
        return boost::any_cast<T>(it->second.default_value);
    }

    /** returns the string representation of the value of the option \a name.*/
    std::string GetValueString(const std::string& option_name) const;

    /** returns the string representation of the default value of the
      * option \a name.*/
    std::string GetDefaultValueString(const std::string& option_name) const;

    /** returns the description string for option \a option_name, or throws
      * std::runtime_error if no such Option exists. */
    const std::string&  GetDescription(const std::string& option_name) const;

    /** returns the validator for option \a option_name, or throws
      * std::runtime_error if no such Option exists. */
    boost::shared_ptr<const ValidatorBase>  GetValidator(const std::string& option_name) const;

    /** writes a usage message to \a os */
    void        GetUsage(std::ostream& os, const std::string& command_line = "") const;

    /** returns the contents of the DB as an XMLDoc. */
    XMLDoc      GetXML() const;

    /** the option changed signal object for the given option */
    OptionChangedSignalType&    OptionChangedSignal(const std::string& option);

    mutable OptionAddedSignalType   OptionAddedSignal;   ///< the option added signal object for this DB
    mutable OptionRemovedSignalType OptionRemovedSignal; ///< the change removed signal object for this DB

    /** adds an Option, optionally with a custom validator */
    template <class T>
    void        Add(const std::string& name, const std::string& description, T default_value,
                    const ValidatorBase& validator = Validator<T>(), bool storable = true)
    {
        if (m_options.find(name) != m_options.end())
            throw std::runtime_error("OptionsDB::Add<>() : Option " + name + " was specified twice.");
        m_options[name] = Option(static_cast<char>(0), name, default_value, default_value, description, validator.Clone(), storable);
        OptionAddedSignal(name);
    }

    /** adds an Option with an alternative one-character shortened name,
      * optionally with a custom validator */
    template <class T>
    void        Add(char short_name, const std::string& name, const std::string& description, T default_value,
                    const ValidatorBase& validator = Validator<T>(), bool storable = true)
    {
        if (m_options.find(name) != m_options.end())
            throw std::runtime_error("OptionsDB::Add<>() : Option " + name + " was specified twice.");
        m_options[name] = Option(short_name, name, default_value, default_value, description, validator.Clone(), storable);
        OptionAddedSignal(name);
    }

    /** adds a flag Option, which is treated as a boolean value with a default
      * of false.  Using the flag on the command line at all indicates that its
      * value it set to true. */
    void        AddFlag(const std::string& name, const std::string& description, bool storable = true)
    {
        if (m_options.find(name) != m_options.end())
            throw std::runtime_error("OptionsDB::AddFlag<>() : Option " + name + " was specified twice.");
        m_options[name] = Option(static_cast<char>(0), name, false, boost::lexical_cast<std::string>(false),
                                 description, 0, storable);
        OptionAddedSignal(name);
    }

    /** adds an Option with an alternative one-character shortened name, which
      * is treated as a boolean value with a default of false.  Using the flag
      * on the command line at all indicates that its value it set to true. */
    void        AddFlag(char short_name, const std::string& name, const std::string& description, bool storable = true)
    {
        if (m_options.find(name) != m_options.end())
            throw std::runtime_error("OptionsDB::AddFlag<>() : Option " + name + " was specified twice.");
        m_options[name] = Option(short_name, name, false, boost::lexical_cast<std::string>(false),
                                 description, 0, storable);
        OptionAddedSignal(name);
    }

    /** removes an Option */
    void        Remove(const std::string& name);

    /** sets the value of option \a name to \a value */
    template <class T>
    void        Set(const std::string& name, const T& value)
    {
        std::map<std::string, Option>::iterator it = m_options.find(name);
        if (it == m_options.end())
            throw std::runtime_error("OptionsDB::Set<>() : Attempted to set nonexistent option \"" + name + "\".");
        if (it->second.value.type() != typeid(T))
            throw boost::bad_any_cast();
        it->second.value = value;
        (*it->second.option_changed_sig_ptr)();
    }

    /** fills some or all of the options of the DB from values passed in from
      * the command line */
    void        SetFromCommandLine(int argc, char* argv[]);

    /** fills some or all of the options of the DB from values stored in
      * XMLDoc \a doc */
    void        SetFromXML(const XMLDoc& doc);

private:
    struct Option
    {
        Option();
        Option(char short_name_, const std::string& name_, const boost::any& value_,
               const boost::any& default_value_, const std::string& description_,
               const ValidatorBase *validator_, bool storable_);

        void            SetFromString(const std::string& str);
        std::string     ValueToString() const;
        std::string     DefaultValueToString() const;

        std::string     name;           ///< the name of the option
        char            short_name;     ///< the one character abbreviation of the option
        boost::any      value;          ///< the value of the option
        boost::any      default_value;  ///< the default value of the option
        std::string     description;    ///< a desription of the option
        boost::shared_ptr<const ValidatorBase>
                        validator;      ///< a validator for the option. Flags have no validators; lexical_cast boolean conversions are done for them.
        bool            storable;       ///< whether this option can be stored in an XML config file for use across multiple runs

        mutable boost::shared_ptr<boost::signal<void ()> > option_changed_sig_ptr;

        static std::map<char, std::string> short_names;   ///< the master list of abbreviated option names, and their corresponding long-form names
    };

    OptionsDB();

    void        SetFromXMLRecursive(const XMLElement& elem, const std::string& section_name);

    std::map<std::string, Option>   m_options;

    static OptionsDB*               s_options_db;

    friend OptionsDB& GetOptionsDB();
};

#endif // _OptionsDB_h_
