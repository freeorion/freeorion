#ifndef _OptionsDB_h_
#define _OptionsDB_h_

#include "Export.h"
#include "Logger.h"
#include "OptionValidators.h"

#include <boost/any.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/signals2/signal.hpp>

#include <map>
#include <unordered_set>


class OptionsDB;
class XMLDoc;
class XMLElement;

/////////////////////////////////////////////
// Free Functions
/////////////////////////////////////////////
typedef void (*OptionsDBFn)(OptionsDB&); ///< the function signature for functions that add Options to the OptionsDB (void (OptionsDB&))

/** adds \a function to a vector of pointers to functions that add Options to
  * the OptionsDB.  This function returns a boolean so that it can be used to
  * declare a dummy static variable that causes \a function to be registered as
  * a side effect (e.g. at file scope:
  * "bool unused_bool = RegisterOption(&foo)"). */
FO_COMMON_API bool RegisterOptions(OptionsDBFn function);

/** returns the single instance of the OptionsDB class */
FO_COMMON_API OptionsDB& GetOptionsDB();


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
  * they need no argument.  For example, specifying "--version" on the command
  * line sets the option "version" in the DB to true, and leaving it out sets the
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
class FO_COMMON_API OptionsDB {
public:
    /** \name Signal Types */ //@{
    /** emitted when an option has changed */
    typedef boost::signals2::signal<void ()>                   OptionChangedSignalType;
    /** emitted when an option is added */
    typedef boost::signals2::signal<void (const std::string&)> OptionAddedSignalType;
    /** emitted when an option is removed */
    typedef boost::signals2::signal<void (const std::string&)> OptionRemovedSignalType;
    //@}

    /** indicates whether an option with name \a name has been added to this
        OptionsDB. */
    bool        OptionExists(const std::string& name) const
    { return m_options.count(name) && m_options.at(name).recognized; }

    /** write back the optionDB's state to the XML config file
        if it has changed since it was last saved. */
    void        Commit();

    /** Write any options that are not at default value to persistent config, replacing any existing file
     *
     *  @returns bool If file was successfully written
     */
    bool        CommitPersistent();

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
        auto it = m_options.find(name);
        if (!OptionExists(it))
            throw std::runtime_error("OptionsDB::Get<>() : Attempted to get nonexistent option \"" + name + "\".");
        return boost::any_cast<T>(it->second.value);
    }

    /** returns the default value of option \a name. Note that the exact type
      * of item stored in the option \a name must be known in advance.  This
      * means that GetDefault() must be called as Get<int>("foo"), etc. */
    template <class T>
    T           GetDefault(const std::string& name) const
    {
        auto it = m_options.find(name);
        if (!OptionExists(it))
            throw std::runtime_error("OptionsDB::GetDefault<>() : Attempted to get nonexistent option \"" + name + "\".");
        return boost::any_cast<T>(it->second.default_value);
    }

    bool        IsDefaultValue(const std::string& name) const {
        auto it = m_options.find(name);
        if (!OptionExists(it))
            throw std::runtime_error("OptionsDB::IsDefaultValue<>() : Attempted to get nonexistent option \"" + name + "\".");
        return IsDefaultValue(it);
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
    std::shared_ptr<const ValidatorBase> GetValidator(const std::string& option_name) const;

    /** writes a usage message to \a os */
    void        GetUsage(std::ostream& os, const std::string& command_line = "", bool allow_unrecognized = false) const;

    /** @brief  Saves the contents of the options DB to the @p doc XMLDoc.
     *
     * @param[in,out] doc  The document this OptionsDB should be written to.
     *      This resets the given @p doc.
     * @param[in] non_default_only Do not include options which are set to their
     *      default value, is unrecognized, or is "version.string"
     */
    void        GetXML(XMLDoc& doc, bool non_default_only = false) const;

    /** find all registered Options that begin with \a prefix and store them in
      * \a ret. If \p allow_unrecognized then include unrecognized options. */
    void        FindOptions(std::set<std::string>& ret, const std::string& prefix, bool allow_unrecognized = false) const;

    /** the option changed signal object for the given option */
    OptionChangedSignalType&        OptionChangedSignal(const std::string& option);

    mutable OptionAddedSignalType   OptionAddedSignal;   ///< the option added signal object for this DB
    mutable OptionRemovedSignalType OptionRemovedSignal; ///< the change removed signal object for this DB

    /** adds an Option, optionally with a custom validator */
    template <class T>
    void        Add(const std::string& name, const std::string& description, T default_value,
                    const ValidatorBase& validator = Validator<T>(), bool storable = true,
                    const std::string& section = std::string())
    {
        auto it = m_options.find(name);
        boost::any value = default_value;
        // Check that this option hasn't already been registered and apply any value that was specified on the command line or from a config file.
        if (it != m_options.end()) {
            if (it->second.recognized)
                throw std::runtime_error("OptionsDB::Add<>() : Option " + name + " was registered twice.");
            if (it->second.flag) { // SetFrom[...]() sets "flag" to true for unrecognised options if they look like flags (i.e. no parameter is found for the option)
                ErrorLogger() << "OptionsDB::Add<>() : Option " << name << " was specified on the command line or in a config file with no value, using default value.";
            } else {
                try {
                    // This option was previously specified externally but was not recognized at the time, attempt to parse the value found there
                    value = validator.Validate(it->second.ValueToString());
                } catch (boost::bad_lexical_cast&) {
                    ErrorLogger() << "OptionsDB::Add<>() : Option " << name << " was given the value \"" << it->second.ValueToString() << "\" from the command line or a config file but that value couldn't be converted to the correct type, using default value instead.";
                }
            }
        }
        m_options[name] = Option(static_cast<char>(0), name, value, default_value,
                                 description, validator.Clone(), storable, false, true, section);
        m_dirty = true;
        OptionAddedSignal(name);
    }

    /** adds an Option with an alternative one-character shortened name,
      * optionally with a custom validator */
    template <class T>
    void        Add(char short_name, const std::string& name, const std::string& description, T default_value,
                    const ValidatorBase& validator = Validator<T>(), bool storable = true,
                    const std::string& section = std::string())
    {
        auto it = m_options.find(name);
        boost::any value = default_value;
        // Check that this option hasn't already been registered and apply any value that was specified on the command line or from a config file.
        if (it != m_options.end()) {
            if (it->second.recognized)
                throw std::runtime_error("OptionsDB::Add<>() : Option " + name + " was registered twice.");
            if (it->second.flag) { // SetFrom[...]() sets "flag" to true for unrecognised options if they look like flags (i.e. no parameter is found for the option)
                ErrorLogger() << "OptionsDB::Add<>() : Option " << name << " was specified on the command line or in a config file with no value, using default value.";
            } else {
                try {
                    // This option was previously specified externally but was not recognized at the time, attempt to parse the value found there
                    value = validator.Validate(it->second.ValueToString());
                } catch (boost::bad_lexical_cast&) {
                    ErrorLogger() << "OptionsDB::Add<>() : Option " << name << " was given the value \"" << it->second.ValueToString() << "\" from the command line or a config file but that value couldn't be converted to the correct type, using default value instead.";
                }
            }
        }
        m_options[name] = Option(short_name, name, value, default_value, description,
                                 validator.Clone(), storable, false, true, section);
        m_dirty = true;
        OptionAddedSignal(name);
    }

    /** adds a flag Option, which is treated as a boolean value with a default
      * of false.  Using the flag on the command line at all indicates that its
      * value it set to true. */
    void        AddFlag(const std::string& name, const std::string& description,
                        bool storable = true, const std::string& section = std::string())
    {
        auto it = m_options.find(name);
        bool value = false;
        // Check that this option hasn't already been registered and apply any value that was specified on the command line or from a config file.
        if (it != m_options.end()) {
            if (it->second.recognized)
                throw std::runtime_error("OptionsDB::AddFlag<>() : Option " + name + " was registered twice.");
            if (!it->second.flag) // SetFrom[...]() sets "flag" to false on unrecognised options if they don't look like flags (flags have no parameter on the command line or have an empty tag in XML)
                ErrorLogger() << "OptionsDB::AddFlag<>() : Option " << name << " was specified with the value \"" << it->second.ValueToString() << "\", but flags should not have values assigned to them.";
            value = true; // if the flag is present at all its value is true
        }
        m_options[name] = Option(static_cast<char>(0), name, value,
                                 boost::lexical_cast<std::string>(false),
                                 description, nullptr, storable, true, true, section);
        m_dirty = true;
        OptionAddedSignal(name);
    }

    /** adds an Option with an alternative one-character shortened name, which
      * is treated as a boolean value with a default of false.  Using the flag
      * on the command line at all indicates that its value it set to true. */
    void        AddFlag(char short_name, const std::string& name,
                        const std::string& description, bool storable = true,
                        const std::string& section = std::string())
    {
        auto it = m_options.find(name);
        bool value = false;
        // Check that this option hasn't already been registered and apply any value that was specified on the command line or from a config file.
        if (it != m_options.end()) {
            if (it->second.recognized)
                throw std::runtime_error("OptionsDB::AddFlag<>() : Option " + name + " was registered twice.");
            if (!it->second.flag) // SetFrom[...]() sets "flag" to false on unrecognised options if they don't look like flags (flags have no parameter on the command line or have an empty tag in XML)
                ErrorLogger() << "OptionsDB::AddFlag<>() : Option " << name << " was specified with the value \"" << it->second.ValueToString() << "\", but flags should not have values assigned to them.";
            value = true; // if the flag is present at all its value is true
        }
        m_options[name] = Option(short_name, name, value,
                                 boost::lexical_cast<std::string>(false),
                                 description, nullptr, storable, true, true, section);
        m_dirty = true;
        OptionAddedSignal(name);
    }

    /** removes an Option */
    void        Remove(const std::string& name);

    /** removes all unrecognized Options that begin with \a prefix.  A blank
      * string will remove all unrecognized Options. */
    void        RemoveUnrecognized(const std::string& prefix = "");

    /** sets the value of option \a name to \a value */
    template <class T>
    void        Set(const std::string& name, const T& value)
    {
        auto it = m_options.find(name);
        if (!OptionExists(it))
            throw std::runtime_error("OptionsDB::Set<>() : Attempted to set nonexistent option \"" + name + "\".");
        m_dirty |= it->second.SetFromValue(value);
    }

    /** Set the default value of option @p name to @p value */
    template <class T>
    void        SetDefault(const std::string& name, const T& value) {
        std::map<std::string, Option>::iterator it = m_options.find(name);
        if (!OptionExists(it))
            throw std::runtime_error("Attempted to set default value of nonexistent option \"" + name + "\".");
        if (it->second.default_value.type() != typeid(T))
            throw boost::bad_any_cast();
        it->second.default_value = value;
    }

    /** if an xml file exists at \a file_path and has the same version tag as \a version, fill the
      * DB options contained in that file (read the file using XMLDoc, then fill the DB using SetFromXML)
      * if the \a version string is empty, bypass that check */
    void        SetFromFile(const boost::filesystem::path& file_path,
                            const std::string& version = "");

    /** fills some or all of the options of the DB from values passed in from
      * the command line */
    void        SetFromCommandLine(const std::vector<std::string>& args);

    /** fills some or all of the options of the DB from values stored in
      * XMLDoc \a doc */
    void        SetFromXML(const XMLDoc& doc);

    struct FO_COMMON_API Option {
        Option();
        Option(char short_name_, const std::string& name_, const boost::any& value_,
               const boost::any& default_value_, const std::string& description_,
               const ValidatorBase *validator_, bool storable_, bool flag_, bool recognized_,
               const std::string& section = std::string());

        // SetFromValue returns true if this->value is successfully changed
        template <typename T>
        bool            SetFromValue(const T& value_);
        // SetFromString returns true if this->value is successfully changed
        bool            SetFromString(const std::string& str);
        // SetToDefault returns true if this->value is successfully changed
        bool            SetToDefault();
        std::string     ValueToString() const;
        std::string     DefaultValueToString() const;
        bool            ValueIsDefault() const;

        std::string     name;           ///< the name of the option
        char            short_name;     ///< the one character abbreviation of the option
        boost::any      value;          ///< the value of the option
        boost::any      default_value;  ///< the default value of the option
        std::string     description;    ///< a desription of the option
        std::unordered_set<std::string> sections; ///< sections this option should display under

        /** A validator for the option.  Flags have no validators; lexical_cast
            boolean conversions are done for them. */
        std::shared_ptr<const ValidatorBase> validator;

        bool            storable;       ///< whether this option can be stored in an XML config file for use across multiple runs
        bool            flag;
        bool            recognized;     ///< whether this option has been registered before being specified via an XML input, unrecognized options can't be parsed (don't know their type) but are stored in case they are later registered with Add()

        mutable std::shared_ptr<boost::signals2::signal<void ()>> option_changed_sig_ptr;

        static std::map<char, std::string> short_names;   ///< the master list of abbreviated option names, and their corresponding long-form names
    };

    struct FO_COMMON_API OptionSection {
        OptionSection();
        OptionSection(const std::string& name_, const std::string& description_,
                      std::function<bool (const std::string&)> option_predicate_);

        std::string name;
        std::string description;
        std::function<bool (const std::string&)> option_predicate = nullptr;
    };

    /** Defines an option section with a description and optionally a option predicate.
     *  @param name Name of section, typically in the form of a left side subset of an option name.
     *  @param description Stringtable key used for local description
     *  @param option_predicate Functor accepting a option name in the form of a std::string const ref and
     *                          returning a bool. Options which return true are displayed in the section for @p name */
    void AddSection(const std::string& name, const std::string& description,
                    std::function<bool (const std::string&)> option_predicate = nullptr);

private:
    /** indicates whether the option referenced by \a it has been added to this
        OptionsDB.  Overloaded for convenient use within other OptionsDB
        functions */
    bool        OptionExists(std::map<std::string, Option>::const_iterator it) const
    { return it != m_options.end() && it->second.recognized; }

    /** indicates whether the current value of the option references by \a is
        the default value for that option */
    bool        IsDefaultValue(std::map<std::string, Option>::const_iterator it) const
    { return it != m_options.end() && it->second.ValueToString() == it->second.DefaultValueToString(); }

    OptionsDB();

    void        SetFromXMLRecursive(const XMLElement& elem, const std::string& section_name);

    /** Determine known option sections and which options each contains
     *  A special "root" section is added for determined top-level sections */
    std::unordered_map<std::string, std::set<std::string>> OptionsBySection(bool allow_unrecognized = false) const;

    std::map<std::string, Option>   m_options;
    std::unordered_map<std::string, OptionSection> m_sections;
    static OptionsDB*               s_options_db;
    bool                            m_dirty; //< has OptionsDB changed since last Commit()

    friend FO_COMMON_API OptionsDB& GetOptionsDB();
};

template <typename T>
bool OptionsDB::Option::SetFromValue(const T& value_) {
    if (value.type() != typeid(T))
        throw boost::bad_any_cast();

    bool changed = false;

    if (!flag) {
        changed = validator->String(value) != validator->String(value_);
    } else {
        changed = (boost::lexical_cast<std::string>(boost::any_cast<bool>(value))
                   != boost::lexical_cast<std::string>(boost::any_cast<bool>(value_)));
    }

    if (changed) {
        value = value_;
        (*option_changed_sig_ptr)();
    }
    return changed;
}



#endif // _OptionsDB_h_
