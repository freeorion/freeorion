//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2015 Mitten-O
//!  Copyright (C) 2016-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef _RichText_TagParser_h_
#define _RichText_TagParser_h_


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

    //!< The parameters as a string of key value pairs key="value".
    RichTextTag(std::string tag_, std::string params_string_, std::string content_);
    RichTextTag(RichTextTag&& rhs) = default;

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
                                              std::set<std::string> known_tags);

};

}


#endif
