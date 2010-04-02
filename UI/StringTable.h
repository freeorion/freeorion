// -*- C++ -*-
//StringTable.h
#ifndef StringTable__h_
#define StringTable__h_

#include <boost/lexical_cast.hpp>

#include <string>
#include <map>
#include <fstream>

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
//! #These are comments<br>
//! #these are more comments<br>
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
class StringTable_
{
public:
    //! \names Structors
    //!@{
    StringTable_();  //!< default construction, uses S_DEFAULT_FILENAME
    
    //! @param filename A file containing the data for this StringTable_
    StringTable_(const std::string& filename);   //!< construct a StringTable_ from the given filename
    ~StringTable_();                             //!< default destructor
    //!@}

    //! \name Accessors
    //!@{
    //! @param index The index of the string to lookup
    //! @return The string found at index in the table, or S_ERROR_STRING if it fails
    const std::string& operator[] (std::string index) const;        //!< Looks up a string at index and returns it.

    //! @param index The index of the string to check for
    //! @return true iff a string exists with that index, false otherwise
    bool StringExists(std::string index) const;                     //!< Looks up a string at index and returns if the string is present.

    //! @param index The index of the string to lookup
    //! @return The string found at index in the table
    inline const std::string& String(std::string index) const { return operator[] (index); }    //!< Interface to operator() \see StringTable_::operator[]
    inline const std::string& Language() const {return m_language;} //!< Returns the language of this StringTable_
    inline const std::string& Filename() const {return m_filename;} //!< accessor to the filename
    //!@}

    //! \names Constants
    //!@{
    static const std::string S_DEFAULT_FILENAME; //!< the default file used if none specified
    static const std::string S_ERROR_STRING;     //!< A string that gets returned when invalid indices are used
    //!@}

private:
    //! \name Internal Functions
    //!@{
    void Load();    //!< Loads the String table file from m_filename
    //!@}

    //! \name Data Members
    //!@{
    std::string m_filename;    //!< the name of the file this StringTable_ was constructed with
    std::string m_language;    //!< A string containing the name of the language used
    std::map<std::string, std::string> m_strings;  //!< The strings in the table
    //!@}
    
};

#endif // StringTable__h_
