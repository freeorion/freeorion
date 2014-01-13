#include "i18n.h"

#include "Directories.h"
#include "Logger.h"
#include "OptionsDB.h"
#include "StringTable.h"

namespace {
    std::string GetDefaultStringTableFileName()
    { return PathString(GetResourceDir() / "stringtables" / "en.txt"); }

    std::string GetStringTableFileName() {
        std::string option_filename = GetOptionsDB().Get<std::string>("stringtable-filename");
        if (option_filename.empty())
            return GetDefaultStringTableFileName();
        else
            return option_filename;
    }

    std::map<std::string, const StringTable_*> stringtables;

    const StringTable_& GetStringTable(std::string stringtable_filename = "") {
        // get option-configured stringtable if no filename specified
        if (stringtable_filename.empty())
            stringtable_filename = GetStringTableFileName();

        // attempt to find requested stringtable...
        std::map<std::string, const StringTable_*>::const_iterator it =
            stringtables.find(stringtable_filename);
        if (it != stringtables.end())
            return *(it->second);

        // if already loaded, load, store, and return
        const StringTable_* table = new StringTable_(stringtable_filename);
        stringtables[stringtable_filename] = table;

        return *table;
    }

    const StringTable_& GetDefaultStringTable()
    { return GetStringTable(GetDefaultStringTableFileName()); }
}

void FlushLoadedStringTables()
{ stringtables.clear(); }

const std::string& UserString(const std::string& str) {
    const StringTable_& string_table = GetStringTable();
    if (string_table.StringExists(str))
        return GetStringTable().String(str);
    else
        return GetDefaultStringTable().String(str);
}

void UserStringList(const std::string& str_list, std::list<std::string>& strings) {
    std::istringstream template_stream(UserString(str_list));
    std::copy(std::istream_iterator<std::string>(template_stream),
              std::istream_iterator<std::string>(),
              std::back_inserter<std::list<std::string> >(strings));
}

boost::format FlexibleFormat(const std::string &string_to_format) {
    try {
        boost::format retval(string_to_format);
        retval.exceptions(boost::io::no_error_bits);
        return retval;
    } catch (const std::exception& e) {
        Logger().errorStream() << "FlexibleFormat caught exception when formatting: " << e.what();
    }
    boost::format retval(UserString("ERROR"));
    retval.exceptions(boost::io::no_error_bits);
    return retval;
}

const std::string& Language()
{ return GetStringTable().Language(); }

std::string RomanNumber(unsigned int n) {
    //letter pattern (N) and the associated values (V)
    static const std::string  N[] = { "M", "CM", "D", "CD", "C", "XC", "L", "XL", "X", "IX", "V", "IV", "I"};
    static const unsigned int V[] = {1000,  900, 500,  400, 100,   90,  50,   40,  10,    9,   5,    4,   1};
    unsigned int remainder = n; //remainder of the number to be written
    int i = 0;                  //pattern index
    std::string retval = "";;
    if (n == 0) return "";      //the romans didn't know there is a zero, read a book about history of the zero if you want to know more
                                //Roman numbers are written using patterns, you chosse the highest pattern lower that the number
                                //write it down, and substract it's value until you reach zero.

    // safety check to avoid very long loops
    if (n > 10000)
        return "!";

    //we start with the highest pattern and reduce the size every time it doesn't fit
    while (remainder > 0) {
        //check if number is larger than the actual pattern value
        if (remainder >= V[i]) {
            //write pattern down
            retval += N[i];
            //reduce number
            remainder -= V[i];
        } else {
            //we need the next pattern
            i++;
        }
    }
    return retval;
}
