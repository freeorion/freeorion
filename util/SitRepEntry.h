#ifndef _SitRepEntry_h_
#define _SitRepEntry_h_

#ifndef _XMLDoc_h_
#include "XMLDoc.h"
#endif

#ifndef BOOST_LEXICAL_CAST_INCLUDED
#include <boost/lexical_cast.hpp>
#endif

#include <string>

/** a simple SitRepEntry to be displayed in the SitRep screen. */
struct SitRepEntry
{
    /** an enumeration of the types of entries */
    enum EntryType {INVALID_ENTRY_TYPE = -1,  ///< this is the EntryType for default-constructed SitRepEntrys; no others should have this type
                    MAX_INDUSTRY_HIT,
                    MAX_TECH_HIT,
                    SHIP_BUILT
                   };

    /** \name Structors */ //@{
    SitRepEntry() : type(INVALID_ENTRY_TYPE) {} ///< default ctor

    SitRepEntry(EntryType type_, const std::string& text_) : type(type_), text(text_) {} ///< basic ctor

    /** ctor that constructs a SitRepEntry object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a SitRepEntry object */
    SitRepEntry(const GG::XMLElement& elem)
    {
        if (elem.Tag() != "SitRepEntry")
            throw std::invalid_argument("Attempted to construct a SitRepEntry from an XMLElement that had a tag other than \"SitRepEntry\"");

        type = EntryType(boost::lexical_cast<int>(elem.Child("type").Attribute("value")));
        turn = boost::lexical_cast<int>(elem.Child("turn").Attribute("value"));
        text = elem.Child("text").Text();
    }
    //@}
   
    /** \name Accessors */ //@{
    /** encodes the SitRepEntry into an XML element */
    GG::XMLElement XMLEncode() const
    {
        GG::XMLElement retval("SitRepEntry");

        GG::XMLElement temp("type");
        temp.SetAttribute("value", boost::lexical_cast<std::string>(type));
        retval.AppendChild(temp);

        temp = GG::XMLElement("turn");
        temp.SetAttribute("value", boost::lexical_cast<std::string>(turn));
        retval.AppendChild(temp);

        temp = GG::XMLElement("text", text);
        retval.AppendChild(temp);

        return retval;
    }
    //@}

    EntryType   type; ///< the type of SitRep this is
    int         turn; ///< the turn for which this SitRep was generated
    std::string text; ///< the text, including hyperlinks, that describes this entry
};

#endif // _SitRepEntry_h_
