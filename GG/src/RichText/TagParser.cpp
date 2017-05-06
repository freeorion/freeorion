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

#include "TagParser.h"


#include <regex>

namespace GG {

    RichTextTag::RichTextTag(const std::string& tag, const std::string& params_string,
                             const std::string& content)
    : tag(tag), tag_params(params_string), content(content)
    {
    }

    std::string RichTextTag::ToString() const {
        std::string str("<");
        str += tag;

        // Convert parameters to key="value".
        if (tag_params.length() != 0) {
            str += " " + tag_params;
        }

        str += ">" + content + "</" + tag + ">";

        return str;
    }


    /**
    * @brief The private parser implementation.
    */
    class TagParserImpl {
        public:
            //! Create a new parser that will consider the given tags as known, others as plaintext.
            TagParserImpl(const std::set<std::string>& known_tags) :
                m_known_tags(known_tags)
            {}

            //! Parses \a text into tags. All text is considered part of some tag, text outside known tags will be put in plaintext tags.
            std::vector<RichTextTag> ParseTags(const std::string& text)
            {
                // A vector of all tags in the text.
                std::vector<RichTextTag> tags;

                try {
                    // Parse all text into tags.
                    ParseTagsImpl(text.begin(), text.end(), &tags);
                } catch (const std::exception& ex) {
                    // If an error was encountered, display it in the text box.
                    tags.clear();
                    tags.push_back(CreateErrorTag(ex.what()));
                    return tags;
                }

                // Filter out unregistered tags.
                std::vector<RichTextTag> relevant_tags;
                for (const RichTextTag& tag : tags) {
                    if (m_known_tags.find(tag.tag) != m_known_tags.end()) {
                        AddWithPlaintextSquashing(relevant_tags, tag);
                    } else {
                        RichTextTag wrapped = WrapInPlaintext(tag);
                        AddWithPlaintextSquashing(relevant_tags, wrapped);
                    }
                }
                return relevant_tags;
            }

        private:
            std::set<std::string> m_known_tags; //! The set of tags considered to be our business.

            //! Parses tags until the first unmatched close tag, or the end.
            //! \return The position before the first unmatched closing tag or the end.
            std::string::const_iterator ParseTagsImpl(const std::string::const_iterator& start,
                                                      const std::string::const_iterator& end,
                                                      std::vector<RichTextTag>* tags)
            {
                std::string::const_iterator current = start;
                std::match_results<std::string::const_iterator> match;

                // The regular expression for matching begin and end tags. Also extracts parameters from start tags.
                typedef std::basic_regex<char, std::regex_traits<char>> regex;
                const static regex tag(R"(<(\w+)( [^>]+)?>|</(\w+)>)");
                const int begin_tag_capture = 1;
                const int params_capture = 2;
                const int end_tag_capture = 3;

                // Find all tags on this nesting level.
                while (std::regex_search(current, end, match, tag, std::regex_constants::match_default)) {
                    //Found a new tag. Recurse if begin tag, return if end tag.
                    const std::ssub_match& begin_match = match[begin_tag_capture];
                    const std::ssub_match& end_match = match[end_tag_capture];

                    if (begin_match.matched) {
                        // A new tag begins. Finish current plaintext tag if it is non-empty.
                        if (tags && current != current + match.position()) {
                            tags->push_back(
                                RichTextTag(RichText::PLAINTEXT_TAG, "",
                                            std::string(current, current + match.position())));
                        }

                        // Recurse to the next nesting level.
                        current = FinishTag(begin_match, match[params_capture],
                                            current + match.position() + match.length(), end, tags);
                    } else if (end_match.matched) {
                        // An end tag encountered. Stop parsing here.
                        return current + match.position();
                    } else {
                        // This really shouldn't happen.
                        std::stringstream error;
                        error << "Error parsing rich text tags: matches neither begin or end tag:" << match.format("[$&]");
                        throw std::runtime_error(error.str());
                    }
                }

                // All done with tags, output final plaintext tag if non-empty.
                if (tags && current != end) {
                    tags->push_back(
                        RichTextTag(RichText::PLAINTEXT_TAG, "", std::string(current, end)));
                }

                // Did not find any more start tags. Return end happily.
                return end;
            }

            // Helper. Return true if str is a prefix of the string (start..end) or vice versa.
            bool StartsWith(const std::string::const_iterator& start,
                            const std::string::const_iterator& end, const std::string& str)
            {
                std::string::const_iterator current = start;
                std::string::const_iterator str_current = str.begin();

                while (current != end && str_current != str.end()) {
                    if (*current != *str_current) {
                        return false;
                    } else {
                        ++current;
                        ++str_current;
                    }
                }

                return true;
            }


            /**
             * @brief Parses until finds an end tag for \a tag.
             *
             * @return The position just after the end tag. Throws if not found.
             */
            std::basic_string<char>::const_iterator FinishTag(
                const std::string& tag, const std::string& parameters,
                const std::string::const_iterator& start, const std::string::const_iterator& end,
                std::vector<RichTextTag>* tags)
            {
                // Use ParseTagsImpl to get the the beginning of the first unmatched end tag.
                // We are interested only in the first level tags, so don't pass the vector to populate.
                std::string::const_iterator current = ParseTagsImpl(start, end, nullptr);

                // It is an error if the end tag is not found.
                if (current == end) {
                    std::stringstream error;
                    error << "Error parsing rich text tags: expected end tag:" << tag << " got end of string.";
                    throw std::runtime_error(error.str());
                } else {
                    // ParseTagsImpl should have dropped us off just before the end of our tag.
                    std::string end_tag = std::string() + "</" + tag + ">";

                    if (StartsWith(current, end, end_tag)) {
                        // A tag was successfully fully read. Add it to tags, if we got one.
                        if (tags) {
                            tags->push_back(
                                RichTextTag(tag, parameters, std::string(start, current)));
                        }

                        // Continue after the tag.
                        return current + end_tag.length();
                    } else {
                        // The end tag eas not the expected end tag.
                        std::stringstream error;
                        std::string rest_prefix(current, std::min(current + 20, end));

                        // The rest prefix is likely to be a wrong end tag, but no worries, the rendering tag interpreter ignores
                        // unpaired end tags so it will display fine.
                        error << "Error parsing rich text tags: expected end tag:" << tag << " got: \"" << rest_prefix << "...\"";
                        throw std::runtime_error(error.str());
                    }
                }
            }

            /// Wraps a tag in a plaintext tag.
            RichTextTag WrapInPlaintext(const RichTextTag& tag)
            {
                return RichTextTag(RichText::PLAINTEXT_TAG, "", tag.ToString());
            }

            // Creates a tag for displaying an error.
            RichTextTag CreateErrorTag(const char* what)
            {
                return RichTextTag(RichText::PLAINTEXT_TAG, "",
                                   std::string("<rgba 255 0 0 255>") + what + "</rgba>");
            }

            /// Add tag to tags, unless both it and the back of tags are plaintext, in which case combines them.
            void AddWithPlaintextSquashing(std::vector<RichTextTag>& tags, const RichTextTag& tag)
            {
                // Just add any non-plaintext tag, or if the last tag isn't plaintext
                if (tag.tag != RichText::PLAINTEXT_TAG || tags.size() < 1 || tags.back().tag != RichText::PLAINTEXT_TAG) {
                    tags.push_back(tag);
                } else {
                    // Squash adjacent plaintext.
                    tags.back().content += tag.content;
                }
            }

    };

    // The public ParseTags.
    std::vector<RichTextTag> TagParser::ParseTags(const std::string& text,
                                                  const std::set<std::string>& known_tags)
    {
        // Create a parser and parse using it.
        TagParserImpl parser(known_tags);
        return parser.ParseTags(text);
    }

}
