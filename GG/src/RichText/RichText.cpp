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

#include <GG/RichText/RichText.h>

#include "TagParser.h"

#include <cctype>
#include <sstream>


namespace {
    using namespace GG;

    //! Skips over white space characters. On return, \a it will point to \a end
    //! or the next non-white space character.
    void pass_space(std::string::const_iterator& it,
                    const std::string::const_iterator& end)
    {
        for (; it != end && isspace(*it); ++it) {}
    }

    //! Reads from \a it, expecting to find somthing of the form "key =".
    //! Returns key. Leaves \a it past the '=' or at \a end.
    std::string read_key(std::string::const_iterator& it,
                         const std::string::const_iterator& end)
    {
        // Move past space.
        pass_space(it, end);

        std::string key;

        while (it != end && !isspace(*it) && *it != '=') {
            key.push_back(*it);
            ++it;
        }

        // Move past space.
        pass_space(it, end);

        // Move past '='.
        if (*it == '=') {
            ++it;
        }

        return key;
    }

    //! Read from \a it, expecting a string of the form '"escape \" with \\"'.
    //! Returns the text between the quotes.
    //! \a it will be at the first character after the second " or at \a end.
    std::string read_quoted(std::string::const_iterator& it,
                            const std::string::const_iterator& end)
    {
        // Move past space.
        pass_space(it, end);

        // Move past ".
        if (*it == '"') {
            ++it;
        } else {
            std::string rest(it, end);
            throw std::runtime_error(std::string("Failed to parse string to end: ") + rest);
        }

        // Build up return value here.
        std::string value;

        // True when the last character was an escape.
        bool escaped = false;

        // Iterate until end or the closing '"'.
        while ((*it != '"' || escaped) && it != end) {
            // If not escaped, '\' escapes.
            if (!escaped && *it == '\\') {
                escaped = true;
            } else {
                value.push_back(*it);
                escaped = false;
            }

            ++it;
        }

        // it should always end pointing at a ".
        // Otherwise we exited the previous loop because we ran into the end of the string.
        if (*it != '"') {
            throw std::runtime_error("Parameter value not properly enclosed in \"");
        } else {
            // Move past the closing ".
            ++it;
            return value;
        }
    }

    //! Extracts key="value" pairs from a string to a map.
    void ExtractParameters(const std::string& params_string,
                           RichText::TAG_PARAMS& tag_params)
    {
        // Next key to be stored.
        std::string key;
        // Next value to be stored.
        std::string value;

        auto it = params_string.begin();
        const auto& end = params_string.end();

        try {
            while (it != end) {

                // Read key and equals sign.
                key = read_key(it, end);
                // Read value.
                value = read_quoted(it, end);

                // If key is valid, store value.
                if (key != "") {
                    tag_params[key] = value;
                }

                // Pass space.
                pass_space(it, end);
            }
        } catch (std::exception& ex) {
            // Store error in special parameter.
            tag_params["GG_ERROR"] = ex.what();
        }
    }
}

namespace GG {
    /**
     * \brief The tag to use for text without explicit tags, or inside unknown (to the rich text system) tags.
     */
    const std::string RichText::PLAINTEXT_TAG = "GG_RICH_PLAIN";

    /**
     * \brief Private implementation class for rich text control.
     */
    class RichTextPrivate {
    public:
        RichTextPrivate(RichText* q, const std::string& content,
                        const std::shared_ptr<Font>& font,
                        Clr color, Flags<TextFormat> format = FORMAT_NONE);

        void CompleteConstruction();

        virtual ~RichTextPrivate() {}

        // Set the text to show. Parses it into tags and uses the factory map to turn them into blocks.
        void SetText(const std::string& content);

        /**
          * @brief Set the whitespace around the content.
          *
          * @param pixels The number of pixels to reserve for empty space around the content.
          * @return void
          */
        void SetPadding(int pixels);
        void SizeMove(Pt ul, Pt lr);

        // Get the set of keys from a map.
        template <typename T, typename V>
        std::set<T> MapKeys(const std::map<T, V>& arg_map);

        // Set the mapping from tags to factories that should be used to generate blocks from them.
        void SetBlockFactoryMap(std::shared_ptr<RichText::BLOCK_FACTORY_MAP> block_factory_map)
        { m_block_factory_map = std::move(block_factory_map); }

    private:
        // Easier access to m_block_factory_map
        RichText::BLOCK_FACTORY_MAP& FactoryMap()
        { return *m_block_factory_map; }

        // Parses content into tags.
        std::vector<RichTextTag> ParseTags(const std::string& content);

        // Create blocks from tags.
        void CreateBlocks(const std::vector<RichTextTag>& tags);

        void AttachBlocks();

        // Update the sizes of all blocks.
        void DoLayout();

        RichText* const             m_owner;                //!< The public control.
        std::shared_ptr<Font>       m_font;                 //!< The font to use for text.
        Clr                         m_color;                //! < The color to use for text.
        Flags<TextFormat>           m_format;               //!< Text format.
        std::shared_ptr<RichText::BLOCK_FACTORY_MAP>
                                    m_block_factory_map;    //!< A map that tells us how to generate block controls from tags.
        std::vector<std::shared_ptr<BlockControl>>
                                    m_blocks;               //!< The blocks generated from our content.
        int                         m_padding;

    };

    RichTextPrivate::RichTextPrivate(RichText* q, const std::string& content,
                                     const std::shared_ptr<Font>& font,
                                     Clr color, Flags<TextFormat> format /*= FORMAT_NONE*/) :
        m_owner(q),
        m_font(font),
        m_color(color),
        m_format(format),
        m_block_factory_map(RichText::DefaultBlockFactoryMap()),
        m_padding(0)
    {
        // Parse the content into a vector of tags.
        auto tags = ParseTags(content);

        // Create blocks from the tags and populate the control with them.
        CreateBlocks(tags);
    }

    void RichTextPrivate::CompleteConstruction()
    { AttachBlocks(); }

    void RichTextPrivate::SetText(const std::string& content)
    {
        // Parse the content into a vector of tags.
        auto tags = ParseTags(content);

        // Create blocks from the tags and populate the control with them.
        CreateBlocks(tags);
        AttachBlocks();
    }

    void RichTextPrivate::SetPadding(int pixels)
    {
        if (m_padding != pixels) {
            m_padding = pixels;
            DoLayout();
        }
    }

    void RichTextPrivate::SizeMove(Pt ul, Pt lr)
    {
        Pt original_size = m_owner->Size();
        m_owner->Control::SizeMove(ul, lr);

        // Redo layout if necessary.
        if (m_owner->Size() != original_size) {
            DoLayout();
        }
    }

    // Get the set of keys from a map.
    template <typename T, typename V>
    std::set<T> RichTextPrivate::MapKeys(const std::map<T, V>& arg_map)
    {
        std::set<T> keys;
        for (const auto& pair : arg_map) {
            keys.insert(pair.first);
        }
        return keys;
    }

    // Parses content into tags.
    std::vector<RichTextTag> RichTextPrivate::ParseTags(const std::string& content)
    {
        // Get a set of tags registered for rich text usage.
        std::set<std::string> known_tags = MapKeys(*m_block_factory_map);

        // Parse the content into a vector of tags.
        std::vector<RichTextTag> tags = TagParser::ParseTags(content, known_tags);

        return tags;
    }

    // Create blocks from tags.
    void RichTextPrivate::CreateBlocks(const std::vector<RichTextTag>& tags)
    {
        m_blocks.clear();

        // Create blocks using factories.
        for (const RichTextTag& tag : tags) {
            RichText::TAG_PARAMS params;
            // Extract the parameters from params_string to the tag_params map.
            ExtractParameters(tag.tag_params, params);

            auto block(FactoryMap()[tag.tag]->CreateFromTag(tag.tag, params, tag.content, m_font, m_color, m_format));
            if (block)
                m_blocks.push_back(block);
        }
    }

    void RichTextPrivate::AttachBlocks()
    {
        m_owner->DetachChildren();

        for (auto& block : m_blocks)
            m_owner->AttachChild(block);

        DoLayout();
    }

    // Update the sizes of all blocks.
    void RichTextPrivate::DoLayout()
    {
        X width = m_owner->ClientWidth() - X(m_padding)*2;
        Pt pos = Pt(X(m_padding), Y(m_padding));

        // The contract between RichText and block controls is this:
        // RichText tells them their width, and they determine their height.
        for (auto& block : m_blocks) {
            Pt size = block->SetMaxWidth(width);
            block->MoveTo(pos);
            pos.y += size.y;
        }

        Pt size(m_owner->Width(), pos.y + Y(m_padding));
        m_owner->Resize(size);
    }

    /////////////////////////////////
    /// RichText public interface //
    ///////////////////////////////
    RichText::RichText(X x, Y y, X w, Y h, const std::string& str,
                       const std::shared_ptr<Font>& font, Clr color,
                       Flags<TextFormat> format, Flags<WndFlag> flags) :
        Control(x, y, w, h, flags),
        m_self(new RichTextPrivate(this, str, font, color, format))
    {}

    void RichText::CompleteConstruction() {
        Control::CompleteConstruction();
        m_self->CompleteConstruction();
    }

    RichText::~RichText() {}

    void RichText::SetText(const std::string& str) { m_self->SetText(str); }

    void RichText::SetPadding(int pixels) { m_self->SetPadding(pixels); }

    void RichText::Render() {}

    void RichText::SizeMove(const Pt& ul, const Pt& lr) { m_self->SizeMove(ul, lr); }

    void RichText::SetBlockFactoryMap(const std::shared_ptr<BLOCK_FACTORY_MAP>& block_factory_map)
    { m_self->SetBlockFactoryMap(block_factory_map); }

    /// Global storage for registered block tags.
    // The factory object live for the lifetime of the process, they are never
    // deleted.
    std::shared_ptr<RichText::BLOCK_FACTORY_MAP>& RichText::DefaultBlockFactoryMap() {
        static std::shared_ptr<RichText::BLOCK_FACTORY_MAP> tag_map(
            new RichText::BLOCK_FACTORY_MAP());
        return tag_map;
    }

    int RichText::RegisterDefaultBlock(const std::string& tag,
                                       std::shared_ptr<IBlockControlFactory>&& factory)
    {
        Font::RegisterKnownTag(tag);
        (*DefaultBlockFactoryMap()) [tag] = std::move(factory);

        // Return a dummy to enable static registration.
        return 0;
    }
}
