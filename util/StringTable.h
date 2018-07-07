#ifndef StringTable_h_
#define StringTable_h_

//! @file
//!     Declares the StringTable class.

#include <string>
#include <map>
#include <unordered_set>
#include <mutex>
#include <memory>

//! Provides a key based translation string lookup with fallback.
//!
//! StringTable is a map to look up translations based on key representing a
//! translation entry.  When a translation for a key is not found the
//! StringTable delegates the lookup to another fallback StringTable, which is
//! filled with entries from another language.  When no fallback StringTable
//! contains the requested translation entry an error string is returned,
//! pointing out the missing translation entry.
//!
//! StringTables are loaded from text files which contain the translation
//! entries for a specific language.  Those translation files are named like
//!
//! ```{.txt}
//! <LANG ISO 639-1>.txt
//! ```
//!
//! where <LANG ISO 639-1> is the two letter language identifier for a language
//! according to ISO 639-1 (e.g English=en, Russian=ru, German=de, ...).
//!
//! The format of those translation files consists of newline separated entries
//! where:
//!
//! * The first line is the native language name.
//! * Linse starting with a hash sign `#` are considered comments and are
//!   ignored when loading the file.
//! * Empty lines are ignored when loading the file.
//! * The translation entries consist of an key followed by a newline, followed
//!   by the translated text followed by a newline.
//! * An identifer must be unique within an translation file.
//! * Identifiers consist of the any latin character, digits or an underscore.
//!   Spaces or other characters are not allowed.
//! * The translated text is a valid UTF-8 based string, which will either be
//!   terminated by a newline, trimming of any whitespace at the begining or end
//!   of the translated string or a multi-line string.
//! * A multi-line string starts and ends with three single quotes `'''`. As the
//!   name implies a multi-line string can span over multiple lines and any
//!   whitespace inside the string will be preseved.
//!
//! A minimal example translation file for the english language `en.txt` should
//! look like:
//!
//! ```{.txt}
//! English
//! # This line is a comment, the line above is the native language name.
//!
//! # The two lines below are an translation entry consisting of an identifer
//! # and a single line of translated text.
//! A_SINGLE_LINE
//! This is the translated text associated with A_SINGLE_LINE
//!
//! # The line below is a translation entry with multiple lines of translated
//! # text.
//! A_MULTI_LINE
//! '''This text
//! spans over
//! multiple lines.
//!     The whitespace before this line is preseved, same goes for the newlines
//! after this line up to the triple single quotes.
//!
//!
//! '''
//!
//! # This translation entry doesn't preserve whitespace
//! SINGLE_LINE_WS_TRIM
//!          This text is intended in the translation file, but the whitespace will be trimmed.
//!
//! # This translation entry preseves whitespace
//! SINGLE_LINE_WS_PRESERVE
//! '''      This text will keep its leading and trailing whitespace   '''
//! ```
//!
//! StringTables implement multiple string substitution mechanism.
//!
//! The first substitution mechanism replaces references to a translation entry
//! with the text of another translation entry.  The syntax for a reference
//! consist of two opening square brackets `[[`, the referenced key and two
//! closing brackets `]]`.  For example:
//!
//! ```{.txt}
//! REFERENCE
//! I am the replaced text
//!
//! TRANSLATION_ENTRY
//! This translation embeds another translation right here > [[REFERENCE]]
//! ```
//!
//! would return
//!
//! ```{.txt}
//! This translation embeds another translaion right here > I am the replaced text
//! ```
//!
//! The second substitution mechanism replaces typed references with type links
//! and the translation text.  The syntax for a typed reference consists of two
//! opening square brackets `[[`, a type key, a space, the referenced key and
//! two closing brackets `]]``.  For example:
//!
//! ```{.txt}
//! REFERENCE
//! I am referenced
//!
//! TRANSLATION_ENTRY
//! This translation links to [[foo_type REFERENCE]]
//! ```
//!
//! would return
//!
//! ```{.txt}
//! This translation links to <foo_type REFERENCE>I am referenced</foo_type>
//! ```
class StringTable {
public:
    //! Create a StringTable instance by loading translations from the default
    //! translation file.
    StringTable();

    //! Create a StringTable from the given @p filename.
    //!
    //! @param  filename
    //!     The file name of the translation file to load.
    //! @param  fallback
    //!     A StringTable that should be used look up unknown translation
    //!     entries.
    StringTable(const std::string& filename, std::shared_ptr<const StringTable> fallback = nullptr);

    ~StringTable();

    //! Returns a translation for @p key.
    //!
    //! @param key
    //!     The identifying key of a translation entry.
    //!
    //! @return
    //!     The translation for @p key or S_ERROR_STRING if no translation was
    //!     found.
    const std::string& operator[] (const std::string& key) const;

    //! Returns if a translation for @p key exists.
    //!
    //! @param key
    //!     The identifying key of a translation entry.
    //!
    //! @return
    //!     True iff a translation with that key exists, false otherwise.
    bool StringExists(const std::string& key) const;

    //! Returns the native language name of this StringTable.
    inline const std::string& Language() const
    { return m_language; }

    //! Returns the translation file name this StringTable was loaded from.
    inline const std::string& Filename() const
    { return m_filename; }

private:
    //! Loads this StringTable from #m_filename
    //!
    //! @param fallback
    //!     A StringTable that should be used look up unknown translation
    //!     entries.
    void Load(std::shared_ptr<const StringTable> fallback = nullptr);

    //! The filename this StringTable was loaded from.
    std::string m_filename = "";

    //! The native language name of the StringTable translations.
    std::string m_language = "";

    //! Mapping of translation entry keys to translated strings.
    std::map<std::string, std::string> m_strings;

    //! Cache for missing translation keys to ensure the returned error
    //! reference string is not destroyed due local scope.
    mutable std::unordered_set<std::string> m_error_strings;

    //! Ensure that multithreaded access to a StringTable is done in an orderly
    //! fashion.
    mutable std::mutex m_mutex;

    //! True if the StringTable was completely loaded and all references
    //! were successfully resolved.
    bool m_initialized = false;
};

#endif // StringTable_h_
