#ifndef StringTable_h_
#define StringTable_h_

//! @file
//!     Declares the StringTable class.

#include <boost/unordered_map.hpp>
#include <string>
#include <set>
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
//!   name implies, a multi-line string can span over multiple lines and any
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
    //! Create an empty StringTable
    StringTable() = default;

    //! Create a StringTable from the given @p filename.
    //!
    //! @param  filename
    //!     The file name of the translation file to load.
    //! @param  fallback
    //!     A StringTable that should be used look up unknown translation
    //!     entries.
    explicit StringTable(std::string filename, std::shared_ptr<const StringTable> fallback = nullptr);

    ~StringTable() = default;

    //! Returns if a translation for @p key exists. Caller should have shared (read)
    //! access to this stringtable before calling this function.
    //!
    //! @param key
    //!     The identifying key of a translation entry.
    //!
    //! @return
    //!     True iff a translation with that key exists, false otherwise.
    [[nodiscard]] bool StringExists(const std::string& key) const;
    [[nodiscard]] bool StringExists(const std::string_view key) const;
    [[nodiscard]] bool StringExists(const char* key) const;

    //! Returns if a translation for @p key exists and what that translation is, if it exists.
    //! Caller should have shared (read) access to this stringtable before calling this function.
    //!
    //! @param key
    //!     The identifying key of a translation entry.
    //!
    //! @return
    //!     pair containing true iff a translation with that key exists, false otherwise, and
    //!                     reference to the translation or to an emptry string if no translation exists
    [[nodiscard]] std::pair<bool, const std::string&> CheckGet(const std::string& key) const;
    [[nodiscard]] std::pair<bool, const std::string&> CheckGet(const std::string_view key) const;
    [[nodiscard]] std::pair<bool, const std::string&> CheckGet(const char* key) const;

    //! Returns the native language name of this StringTable.
    [[nodiscard]] const std::string& Language() const noexcept { return m_language; }

    //! Returns the translation file name this StringTable was loaded from.
    [[nodiscard]] const std::string& Filename() const noexcept { return m_filename; }

    [[nodiscard]] const auto& AllStrings() const noexcept { return m_strings; }

    //! Adds the a @p key and @p value pair to this StringTable, and returns a reference
    //! to the newly-added string. If the key already exists, it is overwritten.
    //! Caller should have exclusive (write) access to this stringtable before calling
    //! this function.
    //!
    //! @param key
    //!     The identifying key of a translation entry.
    //! @param value
    //!     The value to be stored with index key
    //!
    //! @return
    //!     A string for @p key containing value
    const std::string& Add(std::string key, std::string value);

private:
    //! Loads this StringTable from #m_filename
    //!
    //! @param fallback
    //!     A StringTable that should be used look up unknown translation
    //!     entries.
    void Load(std::shared_ptr<const StringTable> fallback = nullptr);

    //! The filename this StringTable was loaded from.
    std::string m_filename;

    //! The native language name of the StringTable translations.
    std::string m_language;

    struct hasher {
        using is_transparent = void;

        size_t operator()(const auto& key) const
        { return boost::hash_range(key.begin(), key.end()); }

        size_t operator()(const char* key) const {
            const std::string_view sv{key};
            return boost::hash_range(sv.begin(), sv.end());
        }
    };

    struct equalizer {
        using is_transparent = void;

        bool operator()(const auto& lhs, const auto& rhs) const noexcept
        { return lhs.compare(rhs) == 0; }

        bool operator()(const char* lhs, const auto& rhs) const noexcept
        { return rhs.compare(lhs) == 0; }
    };

    //! Mapping of translation entry keys to translated strings.
    boost::unordered_map<std::string, std::string, hasher, equalizer> m_strings;

    //! True if the StringTable was completely loaded and all references
    //! were successfully resolved.
    bool m_initialized = false;
};


#endif
