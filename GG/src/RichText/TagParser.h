/* GG is a GUI for SDL and OpenGL.

   Copyright (C) 2015 Mitten-O

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation; either version 2.1
   of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA

   If you do not wish to comply with the terms of the LGPL please
   contact the author as other terms are available for a fee.

   Zach Laine
   whatwasthataddress@gmail.com */

#ifndef TAGPARSER_H
#define TAGPARSER_H

#include <GG/RichText/RichText.h>

namespace GG {

/**
 * @brief A structure containing the data of a single rich text tag.
 *
 */
struct RichTextTag {
    std::string tag; //!< The name of the tag.
    std::string tag_params; //!< The possible parameters of the tag.
    std::string content; //!< The text between the tags.

    RichTextTag(
        const std::string& tag,
        const std::string& params_string, //!< The parameters as a string of key value pairs key="value".
        const std::string& content);

    //! Return the tag as a string that parses back to itself.
    std::string ToString() const;

};


/**
 * @brief Tag parser for rich text tags. Contains only static methods.
 *
 */
class TagParser
{
public:


    /**
     * @brief Parses a string of text into a vector of tags.
     *
     * All text will be wrapped in some tag, text outside tags and inside unknown tags will be
     * returned as RichText::PLAINTEXT_TAG tags.
     *
     * @param text The text to parse.
     * @param known_tags The set of tags to treat as valid tags. Other tags will be treated as plaintext.
     * @return std::vector< RichTextTag > The text inside tags.
     */
    static std::vector<RichTextTag> ParseTags(const std::string& text,
                                          const std::set<std::string>& known_tags);

};

}

#endif // TAGPARSER_H
