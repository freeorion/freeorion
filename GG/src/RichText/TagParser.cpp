//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2015 Mitten-O
//!  Copyright (C) 2016-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

#include <boost/regex.hpp>
#include "TagParser.h"


using namespace GG;

/**
* @brief The private parser implementation.
*/
namespace ParseTagsImpl {
    std::string::const_iterator ParseTagsImpl(const std::string::const_iterator start,
                                              const std::string::const_iterator end,
                                              std::vector<RichTextTag>* tags);

    // Helper. Return true if str is a prefix of the string (start..end) or vice versa.
    bool StartsWith(const std::string::const_iterator start,
                    const std::string::const_iterator end,
                    std::string_view str) noexcept
    {
        auto current = start;
        auto str_current = str.begin();

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

    constexpr std::string_view reset_tag{"<reset>"};
    static_assert(reset_tag.substr(1u, 5u) == Font::RESET_TAG);

    /** @brief Parses until an end tag for \a tag
      * @return The position just after the end tag. Throws if not found. */
    std::basic_string<char>::const_iterator FinishTag(
        std::string tag,
        std::string parameters,
        const std::string::const_iterator start,
        const std::string::const_iterator end,
        std::vector<RichTextTag>* tags)
    {
        // Use ParseTagsImpl to get the beginning of the first unmatched end tag.
        // We are interested only in the first level tags, so don't pass the vector to populate.
        const auto current = ParseTagsImpl(start, end, nullptr);

        // It is an error if the end tag is not found.
        if (current == end) {
            std::string error = "Error parsing rich text tags: expected end tag:" + tag + " got end of string.";
            throw std::runtime_error(error);
        }
        // ParseTagsImpl should have dropped us off just before the end of our tag.
        const std::string end_tag = "</" + tag + ">";

        if (StartsWith(current, end, end_tag)) {
            // A tag was successfully fully read. Add it to tags, if we got one.
            if (tags)
                tags->emplace_back(std::move(tag), std::move(parameters),
                                   std::string(start, current));

            // Continue after the tag.
            return current + end_tag.length();

        } else if (StartsWith(current, end, reset_tag)) {
            if (tags)
                tags->emplace_back(std::move(tag), "", std::string(start, current));

            // Continue without advancing
            return current;

        } else [[unlikely]] {
            // The end tag eas not the expected end tag.
            std::string rest_prefix(current, std::min(current + 20, end));
            // The rest prefix is likely to be a wrong end tag, but no worries, the rendering
            // tag interpreter ignores unpaired end tags so it will display fine.
            std::string error = "Error parsing rich text tags: expected end tag: </" + tag +
                "> but instead got: \"" + rest_prefix + "...\"";
            throw std::runtime_error(error);
        }
    }

    //! Parses tags until the first unmatched close tag, or the end.
    //! \return The position before the first unmatched closing tag or the end.
    std::string::const_iterator ParseTagsImpl(const std::string::const_iterator start,
                                              const std::string::const_iterator end,
                                              std::vector<RichTextTag>* tags)
    {
        std::string::const_iterator current = start;
        boost::match_results<std::string::const_iterator> match;
        boost::match_flag_type flags = boost::match_default;

        // The regular expression for matching begin and end tags.
        // Also extracts parameters from start tags.
        typedef boost::basic_regex<char, boost::regex_traits<char>> regex;
        const static regex tag("<(?<begin_tag>\\w+)( "
                                "(?<params>[^>]+))?>|</"
                                "(?<end_tag>\\w+)>");

        // Find all tags on this nesting level.
        while (boost::regex_search(current, end, match, tag, flags)) {
            //Found a new tag. Recurse if begin tag, return if end tag.
            const boost::ssub_match& begin_match = match["begin_tag"];
            const boost::ssub_match& end_match = match["end_tag"];

            if (end_match.matched) {
                //std::cout << "end: " << std::string_view{end_match.first, end_match.second} << std::endl;
                // An end tag encountered. Stop parsing here.
                return current + match.position();

            } else if (begin_match.matched) {
                //std::cout << "beg: " << std::string_view{begin_match.first, begin_match.second} << std::endl;
                // A new tag begins. Finish current plaintext tag if it is non-empty.
                if (tags && current != current + match.position()) {
                    tags->emplace_back(RichText::PLAINTEXT_TAG, "",
                                       std::string(current, current + match.position()));
                }

                if (std::string_view{begin_match.first, begin_match.second} == Font::RESET_TAG) [[unlikely]] {
                    if (!tags)
                        return current + match.position();
                    else
                        current += match.position() + match.length();

                } else {
                    // Recurse to the next nesting level.
                    current = FinishTag(begin_match, match["params"],
                                        current + match.position() + match.length(), end, tags);
                }

            } else {
                // This really shouldn't happen.
                std::string error = "Error parsing rich text tags: match not begin or end tag:" + match.str();
                throw std::runtime_error(error);
            }
        }

        // All done with tags, output final plaintext tag if non-empty.
        if (tags && current != end)
            tags->emplace_back(RichText::PLAINTEXT_TAG, "", std::string(current, end));

        // Did not find any more start tags. Return end happily.
        return end;
    }


    /// Wraps a tag in a plaintext tag.
    RichTextTag WrapInPlaintext(const RichTextTag& tag)
    { return RichTextTag(RichText::PLAINTEXT_TAG, "", tag.ToString()); }

    // Creates a tag for displaying an error.
    RichTextTag CreateErrorTag(const char* what)
    {
        return RichTextTag(RichText::PLAINTEXT_TAG, "",
                           std::string("<rgba 255 0 0 255>") + what + "</rgba>");
    }

    /// Add tag to tags, unless both it and the back of tags are plaintext, in which case combines them.
    void AddWithPlaintextSquashing(std::vector<RichTextTag>& tags, RichTextTag tag)
    {
        // Just add any non-plaintext tag, or if the last tag isn't plaintext
        if (tag.tag != RichText::PLAINTEXT_TAG || tags.size() < 1 || tags.back().tag != RichText::PLAINTEXT_TAG) {
            tags.emplace_back(std::move(tag));
        } else {
            // Squash adjacent plaintext.
            tags.back().content += tag.content;
        }
    }
}

// The public ParseTags.
std::vector<RichTextTag> TagParser::ParseTags(const std::string& text, std::set<std::string> known_tags)
{
    // Parses \a text into tags. All text is considered part of some tag,
    // text outside known tags will be put in plaintext tags.
    std::vector<RichTextTag> tags;

    try {
        // Parse all text into tags.
        ParseTagsImpl::ParseTagsImpl(text.begin(), text.end(), std::addressof(tags));
    } catch (const std::exception& ex) {
        // If an error was encountered, display it in the text box.
        tags.clear();
        tags.emplace_back(ParseTagsImpl::CreateErrorTag(ex.what()));
        return tags;
    }

    // Filter out unregistered tags.
    std::vector<RichTextTag> relevant_tags;
    relevant_tags.reserve(tags.size());
    for (RichTextTag& tag : tags) {
        if (known_tags.count(tag.tag))
            ParseTagsImpl::AddWithPlaintextSquashing(relevant_tags, std::move(tag));
        else
            ParseTagsImpl::AddWithPlaintextSquashing(relevant_tags, ParseTagsImpl::WrapInPlaintext(tag));
    }
    return relevant_tags;
}
