//StringTable.h
#ifndef _StringTable_h_
#define _StringTable_h_

#include <string>
#include <vector>
#include <fstream>
#include <boost/lexical_cast.hpp>

//! This is an implementation of a String Table for internationalization purposes.
//! The table is built from a file of the following format:<br>
//! name_of_language<br>
//! 0000 string0<br>
//! 0001 string1<br>
//! etc.<br>
//! An example file would look like:<br>
//! English<br>
//! 0000 Hello world!<br>
//! 0001 Goodbye world!<br>
//! 0002 I like FreeOrion very much.<br>
//! <br>
//! Another version of this file, would be:<br>
//! Español<br>
//! 0000 ¡Hola mundo!<br>
//! 0001 ¡Adios mundo!<br>
//! 0002 Me gusta FreeOrion mucho.<br>




class StringTable
{
public:

//! \names Structors
//!@{
    
    StringTable();  //!< default construction, uses S_DEFAULT_FILENAME
    
    //! @param filename A file containing the data for this StringTable
    StringTable(const std::string& filename);   //!< construct a StringTable from the given filename
    ~StringTable();                            //!< default destructor
    
//!@}

public:
//! \name Accessors
//!@{
    
    //! @param index The index of the string to lookup
    //! @return The string found at index in the table, or S_ERROR_STRING if it fails
    const std::string& operator[] (int index);    //!< Looks up a string at index and returns it.
    
    //! @param index The index of the string to lookup
    //! @return The string found at index in the table
    inline const std::string& String(int index) { return operator[] (index); }    //!< Interface to operator() \see StringTable::operator() 
    inline const std::string& Language() {return m_language;}    //!< Returns the language of this StringTable
    
//!@}

public:
//! \names Constants
//!@{
    static const std::string S_DEFAULT_FILENAME;    //!< the default file used if none specified
    static const std::string S_ERROR_STRING;    //!< A string that gets returned when invalid indices are used
    
//!@}

private:
//! \name Internal Functions
//!@{

    void Load();    //!< Loads the String table file from m_filename

//!@}

//! \name Data Members
//!@{

    std::string m_filename;    //!< the name of the file this StringTable was constructed with
    std::string m_language;    //!< A string containing the name of the language used
    std::vector<std::string> m_strings;  //!< The strings in the table

//!@}
    
};//StringTable

#endif
