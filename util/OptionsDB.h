#ifndef _OptionsDB_h_
#define _OptionsDB_h_

#include "Export.h"
#include "Logger.h"
#include "OptionValidators.h"

#include <boost/any.hpp>
#include <boost/container/flat_set.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/signals2/signal.hpp>

#include <functional>
#include <map>
#include <unordered_map>

class OptionsDB;
class XMLDoc;
class XMLElement;

/////////////////////////////////////////////
// Free Functions
/////////////////////////////////////////////

//! The function signature for functions that add Options to the OptionsDB (void (OptionsDB&))
typedef std::function<void (OptionsDB&)> OptionsDBFn;

/** adds \a function to a vector of pointers to functions that add Options to
  * the OptionsDB.  This function returns a boolean so that it can be used to
  * declare a dummy static variable that causes \a function to be registered as
  * a side effect (e.g. at file scope:
  * "bool unused_bool = RegisterOption(&foo)"). */
FO_COMMON_API bool RegisterOptions(OptionsDBFn function);

/** returns the single instance of the OptionsDB class */
FO_COMMON_API OptionsDB& GetOptionsDB();

template<typename T> struct is_unique_ptr : std::false_type {};
template<typename T> struct is_unique_ptr<std::unique_ptr<T>> : std::true_type {};
template<typename T> constexpr bool is_unique_ptr_v = is_unique_ptr<T>::value;


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
    /** emitted when an option has changed */
    using OptionChangedSignalType = boost::signals2::signal<void ()>;

    struct FO_COMMON_API Option {
        Option(char short_name_, std::string name_, boost::any value_,
               boost::any default_value_, std::string description_,
               std::unique_ptr<ValidatorBase>&& validator_, bool storable_, bool flag_,
               bool recognized_, std::string section = std::string());
        Option(Option&& rhs) = default;
        Option(const Option& rhs) = delete;
        virtual ~Option();
        Option& operator=(Option&& rhs) = default;
        Option& operator=(const Option& rhs) = delete;

        // non-defaulted comparisons to only consider names
        [[nodiscard]] bool operator<(const Option& rhs) const noexcept { return name < rhs.name; };
        [[nodiscard]] bool operator==(const Option& rhs) const noexcept { return name == rhs.name; };

        // SetFromValue returns true if this->value is successfully changed
        template <typename T>
        bool SetFromValue(T&& value_);

        // SetFromString returns true if this->value is successfully changed
        bool SetFromString(std::string_view str);

        // SetToDefault returns true if this->value is successfully changed
        bool SetToDefault();

        [[nodiscard]] std::string ValueToString() const;
        [[nodiscard]] std::string DefaultValueToString() const;
        [[nodiscard]] bool        ValueIsDefault() const;

        using StringSet = boost::container::flat_set<std::string>;

        std::string     name;               ///< the name of the option
        boost::any      value;              ///< the value of the option
        boost::any      default_value;      ///< the default value of the option
        std::string     description;        ///< a desription of the option
        StringSet       sections;           ///< sections this option should display under

        // A validator for the option.  Flags have no validators; lexical_cast boolean conversions are done for them.
        std::unique_ptr<ValidatorBase> validator;
        std::unique_ptr<OptionChangedSignalType> option_changed_sig; // pointer so that references are valid even if this Option is moved

        char            short_name{0};      ///< the one character abbreviation of the option
        bool            storable = false;   ///< whether this option can be stored in an XML config file for use across multiple runs
        bool            flag = false;
        bool            recognized = false; ///< whether this option has been registered before being specified via an XML input, unrecognized options can't be parsed (don't know their type) but are stored in case they are later registered with Add()
    };

private:
    using ContainerType = std::vector<Option>;

    auto find_option(const std::string_view name) const
    { return std::find_if(m_options.begin(), m_options.end(), [name](const auto& o) { return o.name == name; }); }
    auto find_option(const std::string_view name)
    { return std::find_if(m_options.begin(), m_options.end(), [name](const auto& o) { return o.name == name; }); }

public:
    /** Has an option with name \a name been added to this OptionsDB? */
    bool OptionExists(std::string_view name) const {
        auto it = find_option(name);
        return it != m_options.end() && it->recognized;
    }

    /** write the optionDB's non-default state to the XML config file. */
    bool Commit(bool only_if_dirty = true, bool only_non_default = true);

    /** Write any options that are not at default value to persistent config, replacing any existing file
     *
     *  @returns bool If file was successfully written
     */
    bool CommitPersistent();

    /** validates a value for an option. throws std::runtime_error if no option
      * \a name exists.  throws bad_lexical_cast if \a value cannot be
      * converted to the type of the option \a name. */
    void Validate(std::string_view name, std::string_view value) const;


    /** returns the value of option \a name. Note that the exact type of item
      * stored in the option \a name must be known in advance.  This means that
      * Get() must be called as Get<int>("foo"), etc. */
    template <typename T>
    T Get(std::string_view name) const
    {
        auto it = find_option(name);
        if (!OptionExists(it))
            throw std::runtime_error(std::string{"OptionsDB::Get<>() : Attempted to get nonexistent option \""}.append(name).append("\"."));
        try {
            return boost::any_cast<T>(it->value);
        } catch (const boost::bad_any_cast&) {
            if constexpr (std::is_enum_v<T>) {
                try {
                    return T{boost::any_cast<std::underlying_type_t<T>>(it->value)};
                } catch (...) {}
            }
            ErrorLogger() << "bad any cast converting value option named: " << name
                          << ". Returning default value instead";

            try {
                return boost::any_cast<T>(it->default_value);
            } catch (const boost::bad_any_cast&) {
                if constexpr (std::is_enum_v<T>) {
                    try {
                        return T{boost::any_cast<std::underlying_type_t<T>>(it->default_value)};
                    } catch (...) {
                        ErrorLogger() << "bad any cast converting default value of option named: " << name
                                      << ". Returning data-type default value instead.";
                    }
                } else {
                    ErrorLogger() << "bad any cast converting default value of option named: " << name
                                  << ". Returning data-type default value instead: " << T();
                }
                return T();
            }
        }
    }

    /** returns the default value of option \a name. Note that the exact type
      * of item stored in the option \a name must be known in advance.  This
      * means that GetDefault() must be called as Get<int>("foo"), etc. */
    template <typename T>
    T GetDefault(std::string_view name) const
    {
        auto it = find_option(name);
        if (!OptionExists(it))
            throw std::runtime_error(std::string{"OptionsDB::GetDefault<>() : Attempted to get nonexistent option: "}.append(name));
        try {
            return boost::any_cast<T>(it->default_value);
        } catch (const boost::bad_any_cast&) {
            if constexpr (std::is_enum_v<T>) {
                try {
                    return T{boost::any_cast<std::underlying_type_t<T>>(it->default_value)};
                } catch (...) {}
            }
            ErrorLogger() << "bad any cast converting default value of option named: " << name << "  returning type default value instead";
            return T();
        }
    }

    bool IsDefaultValue(std::string_view name) const {
        const auto it = find_option(name);
        if (!OptionExists(it))
            throw std::runtime_error(std::string{"OptionsDB::IsDefaultValue<>() : Attempted to get nonexistent option: "}.append(name));
        return IsDefaultValue(it);
    }

    /** returns the string representation of the value of the option \a name.*/
    std::string GetValueString(std::string_view option_name) const;

    /** returns the string representation of the default value of the
      * option \a name.*/
    std::string GetDefaultValueString(std::string_view option_name) const;

    /** returns the description string for option \a option_name, or throws
      * std::runtime_error if no such Option exists. */
    const std::string& GetDescription(std::string_view option_name) const;

    /** returns the validator for option \a option_name, or throws
      * std::runtime_error if no such Option exists. */
    const ValidatorBase* GetValidator(std::string_view option_name) const;

    /** writes a usage message to \a os */
    void GetUsage(std::ostream& os, std::string_view command_line = "", bool allow_unrecognized = false) const;

    /** @brief  Saves the contents of the options DB to the @p doc XMLDoc.
     *
     * @param[in,out] doc  The document this OptionsDB should be written to.
     *      This resets the given @p doc.
     * @param[in] non_default_only Do not include options which are set to their
     *      default value, is unrecognized, or is "version.string"
     */
    void GetXML(XMLDoc& doc, bool non_default_only = false, bool include_version = true) const;

    /** find all registered Options that begin with \a prefix and store them in
      * \a ret. If \p allow_unrecognized then include unrecognized options. */
    std::vector<std::string_view> FindOptions(std::string_view prefix, bool allow_unrecognized = false) const;

    /** the option changed signal object for the given option */
    OptionChangedSignalType& OptionChangedSignal(std::string_view option);


    /** adds an Option, optionally with a custom validator */
    template <typename T>
    void Add(std::string name, std::string description, T&& default_value,
             std::unique_ptr<ValidatorBase> validator = nullptr, bool storable = true,
             std::string section = "")
    {
        auto it = find_option(name);
        boost::any value = default_value;
        if (!validator)
            validator = std::make_unique<Validator<std::decay_t<T>>>();

        // Check that this option hasn't already been registered and apply any
        // value that was specified on the command line or from a config file.
        if (it != m_options.end()) {
            if (it->recognized)
                throw std::runtime_error("OptionsDB::Add<>() : Option registered twice: " + name);

            // SetFrom[...]() sets "flag" to true for unrecognised options if they look like flags
            // (i.e. no parameter is found for the option)
            if (it->flag) {
                ErrorLogger() << "OptionsDB::Add<>() : Option " << name << " was specified on the command line or in a config file with no value, using default value.";
            } else {
                try {
                    // This option was previously specified externally but was not recognized at the time.
                    // Attempt to parse the value found there, overriding the default value passed in
                    value = validator->Validate(it->ValueToString());
                } catch (const boost::bad_lexical_cast&) {
                    ErrorLogger() << "OptionsDB::Add<>() : Option " << name
                                  << " was given the value \"" << it->ValueToString()
                                  << "\" from the command line or a config file but that value couldn't be converted to the correct type, using default value instead.";
                }
            }
        }

        Option option{static_cast<char>(0), name, std::move(value), std::forward<T>(default_value),
                      std::move(description), std::move(validator), storable, false, true,
                      std::move(section)};
        if (it != m_options.end())
            *it = std::move(option);
        else
            m_options.push_back(std::move(option));

        m_dirty = true;
    }

    template <typename T, typename V> requires(!is_unique_ptr_v<V> && !std::is_null_pointer_v<V>)
    void Add(std::string name, std::string description, T&& default_value,
             V&& validator, // validator needs to be wrapped in unique_ptr (eg. by cloning itself)
             bool storable = true, std::string section = "")
    {
        Add(std::move(name), std::move(description), std::forward<T>(default_value),
            std::forward<V>(validator).Clone(), storable, std::move(section));
    }

    template <typename T, typename V> requires(!is_unique_ptr_v<V> && !std::is_null_pointer_v<V>)
    void Add(const char* name, const char* description, T&& default_value,
             V&& validator, // validator needs to be wrapped in unique_ptr (eg. by cloning itself)
             bool storable = true, const char* section = "")
    {
        Add(name, description, std::forward<T>(default_value),
            std::forward<V>(validator).Clone(), storable, section);
    }

    /** adds an Option with an alternative one-character shortened name,
      * optionally with a custom validator */
    template <typename T>
    void Add(char short_name, std::string name, std::string description, T&& default_value,
             std::unique_ptr<ValidatorBase> validator = nullptr, bool storable = true,
             std::string section = "")
    {
        auto it = find_option(name);
        boost::any value{default_value};
        if (!validator)
            validator = std::make_unique<Validator<std::decay_t<T>>>();

        // Check that this option hasn't already been registered and apply any
        // value that was specified on the command line or from a config file.
        if (it != m_options.end()) {
            if (it->recognized)
                throw std::runtime_error("OptionsDB::Add<>() : Option registered twice: " + name);

            // SetFrom[...]() sets "flag" to true for unrecognised options if they look like flags
            // (i.e. no parameter is found for the option)
            if (it->flag) {
                ErrorLogger() << "OptionsDB::Add<>() : Option " << name
                              << " was specified on the command line or in a config file with no value, using default value.";
            } else {
                try {
                    // This option was previously specified externally but was not recognized at the time,
                    // attempt to parse the value found there
                    value = validator->Validate(it->ValueToString());
                } catch (const boost::bad_lexical_cast&) {
                    ErrorLogger() << "OptionsDB::Add<>() : Option " << name
                                  << " was given the value from the command line or a config file that cannot be converted to the correct type. Using default value instead.";
                }
            }
        }

        Option option{short_name, name, std::move(value), std::forward<T>(default_value),
                      std::move(description), std::move(validator), storable, false, true,
                      std::move(section)};
        if (it != m_options.end())
            *it = std::move(option);
        else
            m_options.push_back(std::move(option));

        m_dirty = true;
    }

    template <typename T, typename V> requires(!is_unique_ptr_v<V> && !std::is_null_pointer_v<V>)
    void Add(char short_name, std::string name, std::string description,
             T default_value, V&& validator, // validator should be wrapped in unique_ptr
             bool storable = true, std::string section = "")
    {
        Add<T>(short_name, std::move(name), std::move(description), std::move(default_value),
               std::make_unique<V>(std::move(validator)), storable, std::move(section));
    }

    /** adds a flag Option, which is treated as a boolean value with a default
      * of false.  Using the flag on the command line at all indicates that its
      * value it set to true. */
    void AddFlag(std::string name, std::string description,
                 bool storable = true, std::string section = std::string())
    {
        const auto it = find_option(name);
        bool value = false;

        // Check that this option hasn't already been registered and apply any value
        // that was specified on the command line or from a config file.
        if (it != m_options.end()) {
            if (it->recognized)
                throw std::runtime_error("OptionsDB::AddFlag<>() : Option registered twice: " + name);

            // SetFrom[...]() sets "flag" to false on unrecognised options if they don't look like flags
            // (flags have no parameter on the command line or have an empty tag in XML)
            if (!it->flag)
                ErrorLogger() << "OptionsDB::AddFlag<>() : Option " << name << " was specified with the value \""
                              << it->ValueToString() << "\", but flags should not have values assigned to them.";
            value = true; // if the flag is present at all its value is true
        }

        Option option{static_cast<char>(0), name, value, value, std::move(description),
                      std::make_unique<Validator<bool>>(), storable, true, true, std::move(section)};
        if (it != m_options.end())
            *it = std::move(option);
        else
            m_options.push_back(std::move(option));

        m_dirty = true;
    }

    /** adds an Option with an alternative one-character shortened name, which
      * is treated as a boolean value with a default of false.  Using the flag
      * on the command line at all indicates that its value it set to true. */
    void AddFlag(char short_name, std::string name, std::string description,
                 bool storable = true, std::string section = std::string())
    {
        auto it = find_option(name);
        bool value = false;
        auto validator = std::make_unique<Validator<bool>>();

        // Check that this option hasn't already been registered and apply any value
        // that was specified on the command line or from a config file.
        if (it != m_options.end()) {
            if (it->recognized)
                throw std::runtime_error("OptionsDB::AddFlag<>() : Option registered twice: " + name);

            // SetFrom[...]() sets "flag" to false on unrecognised options if they don't look like flags
            // (flags have no parameter on the command line or have an empty tag in XML)
            if (!it->flag)
                ErrorLogger() << "OptionsDB::AddFlag<>() : Option " << name << " was specified with the value \""
                              << it->ValueToString() << "\", but flags should not have values assigned to them.";
            value = true; // if the flag is present at all its value is true
        }

        Option option{short_name, name, value, value, std::move(description),
                      std::move(validator), storable, true, true, std::move(section)};
        if (it != m_options.end())
            *it = std::move(option);
        else
            m_options.push_back(std::move(option));

        m_dirty = true;
    }

    /** removes an Option */
    void Remove(std::string_view name);

    /** removes all unrecognized Options that begin with \a prefix.  A blank
      * string will remove all unrecognized Options. */
    void RemoveUnrecognized(std::string_view prefix = "");

    /** sets the value of option \a name to \a value */
    template <typename T>
    void Set(std::string_view name, T&& value)
    {
        auto it = find_option(name);
        if (!OptionExists(it))
            throw std::runtime_error("OptionsDB::Set<>() : Attempted to set nonexistent option " + std::string{name});
        m_dirty |= it->SetFromValue(std::forward<T>(value));
    }

    /** Set the default value of option @p name to @p value */
    template <typename T>
    void SetDefault(std::string_view name, T&& value) {
        auto it = find_option(name);
        if (!OptionExists(it))
            throw std::runtime_error("Attempted to set default value of nonexistent option \"" + std::string{name});
        if (it->default_value.type() != typeid(std::decay_t<T>))
            throw boost::bad_any_cast();
        it->default_value = std::forward<T>(value);
    }

    void SetToDefault(std::string_view name);

    /** if an xml file exists at \a file_path and has the same version tag as \a version, fill the
      * DB options contained in that file (read the file using XMLDoc, then fill the DB using SetFromXML)
      * if the \a version string is empty, bypass that check */
    void SetFromFile(const boost::filesystem::path& file_path, std::string_view version = "");

    /** fills some or all of the options of the DB from values passed in from
      * the command line */
    void SetFromCommandLine(const std::vector<std::string>& args);

    /** fills some or all of the options of the DB from values stored in
      * XMLDoc \a doc */
    void SetFromXML(const XMLDoc& doc);

    /** Defines an option section with a description and optionally a option predicate.
     *  @param name Name of section, typically in the form of a left side subset of an option name.
     *  @param description Stringtable key used for local description
     *  @param option_predicate Functor accepting a option name in the form of a std::string const ref and
     *                          returning a bool. Options which return true are displayed in the section for @p name */
    void AddSection(const char* name, std::string description,
                    std::function<bool (const std::string&)> option_predicate = nullptr);

private:
    /** is option at \a it in OptionsDB? */
    bool OptionExists(auto it) const
        requires requires (ContainerType::const_iterator cit) { cit == it; }
    { return it != m_options.end() && it->recognized; }

    /** is current value of option at \a it the default value for that option? */
    bool IsDefaultValue(auto it) const
        requires requires (ContainerType::const_iterator cit) { cit == it; }
    { return it != m_options.end() && it->ValueToString() == it->DefaultValueToString(); }

    void SetFromXMLRecursive(const XMLElement& elem, std::string_view section_name);

    /** Determine known option sections and which options each contains
     *  A special "root" section is added for determined top-level sections */
    std::unordered_map<std::string_view, std::set<std::string_view>>
        OptionsBySection(bool allow_unrecognized = false) const;

    struct OptionSection {
        OptionSection() = default;
        OptionSection(OptionSection&&) = default;
        OptionSection(auto&& name_, auto&& desc_, auto&& pred_) :
            name(std::forward<decltype(name_)>(name_)),
            description(std::forward<decltype(desc_)>(desc_)),
            option_predicate(std::forward<decltype(pred_)>(pred_))
        {}
        std::string name;
        std::string description;
        std::function<bool (const std::string&)> option_predicate;
    };

    ContainerType              m_options = [](){ ContainerType retval; retval.reserve(1000); return retval; }();
    std::vector<OptionSection> m_sections = []() { std::vector<OptionSection> retval; retval.reserve(20); return retval; }();
    bool                       m_dirty = false; //< has OptionsDB changed since last Commit()

    friend FO_COMMON_API OptionsDB& GetOptionsDB();
};

template <typename T>
bool OptionsDB::Option::SetFromValue(T&& value_) {
    if constexpr (!std::is_same_v<std::decay_t<decltype(value_)>, std::string> &&
                  std::is_convertible_v<decltype(value_), std::string>)
    {
        return SetFromValue(std::string(value_));

    } else if (value.type() != typeid(std::decay_t<T>)) {
        DebugLogger() << "OptionsDB::Option::SetFromValue expected type " << value.type().name()
                      << " but got value of type " << typeid(T).name();
    }

    bool changed = false;

    try {
        if (flag) {
            changed = (std::to_string(boost::any_cast<bool>(value))
                    != std::to_string(boost::any_cast<bool>(value_)));
        } else if (validator) {
            changed = validator->String(value) != validator->String(value_);
        } else {
            throw std::runtime_error("Option::SetFromValue called with no Validator set");
        }
    } catch (const std::exception& e) {
        ErrorLogger() << "Exception thrown when validating while setting option " << name
                      << " : " << e.what();
        changed = true;
    }

    if (changed) {
        value = std::forward<T>(value_);
        (*option_changed_sig)();
    }
    return changed;
}

// needed because std::vector<std::string> is not streamable
template <>
FO_COMMON_API std::vector<std::string> OptionsDB::Get<std::vector<std::string>>(std::string_view name) const;

#endif
