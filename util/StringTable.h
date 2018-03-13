#ifndef StringTable__h_
#define StringTable__h_

#include <string>
#include <map>
#include <unordered_set>
#include <mutex>
#include <memory>

// HACK! StringTable is renamed to StringTable_ because freeimage defines
// a class StringTable too. If both are named identically, static linking
// won't be possible.

//! This is an implementation of a String Table for internationalization purposes.
//! The table is built from a file of the following format:<br>
//! name_of_language<br>
//! ID1<br>
//! STRING1<br><br>
//! comments are also allowed, preceded by a pound '#' sign
//! These will not be added to the table.
//! Any number of blank lines may separate strings, but an
//! identifier <b>MUST</b> be followed by its string on the next line.
//! Newlines sequences ("\n") within strings are converted to newlines when the string is loaded.
//!
//! An example:<br>
//! English<br>
//! <br>
//! WELCOME<br>
//! Welcome to FreeOrion<br>
//! <br>
//! EXIT<br>
//! Exit the program<br>
//! <br>
//! # These are comments<br>
//! # these are more comments<br>
//! TEST_ONE<br>
//! test one<br>
//! TEST TWO<br>
//! test two<br>
//! <br>
//! TEST THREE<br>
//! test three<br>
//! #<br>
//! <br>
//! <br>
//! <br>
//! TESTFOUR<br>
//! test four<br>
class StringTable_ {
public:
    //! \name Structors
    //!@{
    StringTable_();  //!< default construction, uses default stringtable filename

    //! construct a StringTable_ from the given filename
    //! @param filename A file containing the data for this StringTable_
    //! @param lookups_fallback_table A StringTable_ to be used as fallback expansions lookup
    StringTable_(const std::string& filename, std::shared_ptr<const StringTable_> lookups_fallback_table = nullptr);

    ~StringTable_();
    //!@}

    //! \name Accessors
    //!@{
    //! @param index The index of the string to lookup
    //! @return The string found at index in the table, or S_ERROR_STRING if it fails
    const std::string& operator[] (const std::string& index) const; //!< Looks up a string at index and returns it.

    //! @param index The index of the string to check for
    //! @return true iff a string exists with that index, false otherwise
    bool StringExists(const std::string& index) const;

    //! @return true iff this stringtable contain no entries
    bool Empty() const;

    //! @param index The index of the string to lookup
    //! @return The string found at index in the table
    inline const std::string& String(const std::string& index) const { return operator[] (index); } //!< Interface to operator() \see StringTable_::operator[]
    inline const std::string& Language() const { return m_language; } //!< Returns the language of this StringTable_
    inline const std::string& Filename() const { return m_filename; } //!< accessor to the filename
    //!@}

private:
    //! \name Internal Functions
    //!@{
    //! Loads the String table file from m_filename
    //! @param lookups_fallback_table A StringTable_ to be used as fallback expansions lookup
    void Load(std::shared_ptr<const StringTable_> lookups_fallback_table = nullptr);
    //!@}

    //! \name Data Members
    //!@{
    std::string                             m_filename = "";    //!< the name of the file this StringTable_ was constructed with
    std::string                             m_language = "";    //!< A string containing the name of the language used
    std::map<std::string, std::string>      m_strings;
    mutable std::unordered_set<std::string> m_error_strings;
    mutable std::mutex                      m_mutex;
    bool                                    m_initialized = false;
    //!@}
};

#endif // StringTable__h_
