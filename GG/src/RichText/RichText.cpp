//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2015 Mitten-O
//!  Copyright (C) 2016-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

#include <cctype>
#include <sstream>
#include <GG/RichText/RichText.h>
#include "TagParser.h"


using namespace GG;

namespace {

constexpr bool is_space(const std::string_view::value_type c) noexcept
{ return c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v'; }

//! Skips over white space characters. On return, \a it will point to \a end
//! or the next non-white space character.
constexpr void pass_space(std::string_view::const_iterator& it, const std::string_view::const_iterator end)
{ for (; it != end && is_space(*it); ++it) {} }

constexpr auto to_address(const std::string_view::const_iterator it) noexcept
{
#if defined(__cpp_lib_to_address)
    return std::to_address(it);
#else
    return &*it;
#endif
}

constexpr auto to_string_view(const std::string_view::const_iterator start_it,
                              const std::string_view::const_iterator end_it) noexcept
{
    return (start_it != end_it) ?
        std::string_view{to_address(start_it), static_cast<std::size_t>(std::distance(start_it, end_it))} :
        std::string_view{};
}

//! Reads from \a it, expecting to find somthing of the form "key =".
//! Returns key. Leaves \a it past the '=' or at \a end.
constexpr std::string_view read_key(std::string_view::const_iterator& it,
                                    const std::string_view::const_iterator end)
{
    // Move past space.
    pass_space(it, end);

    const auto start_it{it};

    while (it != end && !is_space(*it) && *it != '=')
    { ++it; }

    std::string_view key = to_string_view(start_it, it);

    // Move past space.
    pass_space(it, end);

    // Move past '='.
    if (*it == '=')
        ++it;

    return key;
}


//! Read from \a it, expecting a string of the form '"escape \" with \\"'.
//! Returns the text between the quotes.
//! \a it will be at the first character after the second " or at \a end.
constexpr std::string_view read_quoted(std::string_view::const_iterator& it,
                                       const std::string_view::const_iterator end)
{
    // Move past space
    pass_space(it, end);

    if (*it != '"') {
        const auto rest = to_string_view(it, end);
        throw std::runtime_error(std::string("Failed to parse string to end: ").append(rest));
    }
    ++it; // Move past "

    const auto start_it{it};

    // find next "
    for (; *it != '"' && it != end; ++it) {}

    if (it == end)
        throw std::runtime_error("Parameter value not properly enclosed in \"");
    return to_string_view(start_it, it);
}

constexpr std::string_view test_string = "  test_key = \"test_value\"";
constexpr auto test_key_value = [](){
    auto it = test_string.begin();
    const auto test_key = read_key(it, test_string.cend());
    const auto test_val = read_quoted(it, test_string.end());

    return std::pair{test_key, test_val};
}();
static_assert(test_key_value.first == "test_key");
static_assert(test_key_value.second == "test_value");


//! Extracts key="value" pairs from a string to a map.
auto ExtractParameters(std::string_view params_string)
{
    RichText::TAG_PARAMS tag_params;

    auto it = params_string.begin();
    const auto end = params_string.end();

    try {
        while (it != end) {

            // Read key and equals sign.
            auto key = read_key(it, end);

            // If key is valid, store value.
            if (!key.empty())
                tag_params.emplace_back(key, read_quoted(it, end));

            // Pass space.
            pass_space(it, end);
        }
    } catch (...) {}

    return tag_params;
}
}

/**
    * \brief Private implementation class for rich text control.
    */
class GG::RichTextPrivate {
public:
    RichTextPrivate(RichText& q, const std::string& content,
                    std::shared_ptr<Font> font, Clr color,
                    Flags<TextFormat> format = FORMAT_NONE);
    virtual ~RichTextPrivate() = default;

    const auto BlockCount() noexcept { return m_blocks.size(); }

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

    // Set the mapping from tags to factories that should be used to generate blocks from them.
    void SetBlockFactoryMap(RichText::BlockFactoryMap block_factory_map) noexcept
    { m_block_factory_map = std::move(block_factory_map); }

private:
    // Parses content into tags.
    std::vector<RichTextTag> ParseTags(const std::string& content);

    // Create blocks from tags.
    void CreateBlocks(std::vector<RichTextTag> tags);

    void AttachBlocksToOwner();

    // Update the sizes of all blocks.
    void DoLayout();

    RichText&                   m_owner;                    //!< The public control.
    std::shared_ptr<Font>       m_font;                     //!< The font to use for text.
    RichText::BlockFactoryMap   m_block_factory_map;        //!< A map that tells us how to generate block controls from tags.
    std::vector<std::shared_ptr<BlockControl>>  m_blocks;   //!< The blocks generated from our content.
    int                         m_padding = 0;
    Clr                         m_color;                    //! < The color to use for text.
    Flags<TextFormat>           m_format;                   //!< Text format.

    friend class RichText;
};

RichTextPrivate::RichTextPrivate(RichText& q, const std::string& content,
                                 std::shared_ptr<Font> font,
                                 Clr color, Flags<TextFormat> format) :
    m_owner(q),
    m_font(std::move(font)),
    m_block_factory_map(RichText::DefaultBlockFactoryMap()),
    m_color(color),
    m_format(format)
{
    // Parse the content into a vector of tags and create
    // blocks from the tags and populate the control with them.
    CreateBlocks(ParseTags(content));
    // can CreateBlocks here but not AttachBlocksToOwner, since CreateBlocks doesn't
    // depend on m_owner, but AttachBlocksToOwner will modify the passed-in reference
    // but this constructor is called during RichText's constructor, and so the
    // passed in reference isn't fully constructed yet
}

void RichTextPrivate::SetText(const std::string& content)
{
    // Parse the content into a vector of tags and create
    // blocks from the tags and populate the control with them.
    CreateBlocks(ParseTags(content));
    AttachBlocksToOwner();
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
    const Pt original_size = m_owner.Size();

    m_owner.Control::SizeMove(ul, lr);

    // Redo layout if necessary.
    if (m_owner.Size() != original_size)
        DoLayout();
}

namespace {
    // Get the set of keys from a map.
    template <typename T, typename V, typename C>
    std::set<T> MapKeys(const std::map<T, V, C>& arg_map)
    {
        std::set<T> keys;
        for ([[maybe_unused]] auto& [key, val] : arg_map) {
            (void)val;  // quiet unused varianle warning
            keys.insert(key);
        }
        return keys;
    }
}

std::vector<RichTextTag> RichTextPrivate::ParseTags(const std::string& content)
{
    // Get a set of tags registered for rich text usage,
    // and parse the content into a vector of tags.
    std::set<std::string> keys;
    std::transform(m_block_factory_map.begin(), m_block_factory_map.end(),
                   std::inserter(keys, keys.end()),
                   [](const auto entry) { return std::string{entry.first}; });
    return TagParser::ParseTags(content, std::move(keys));
}

// Create blocks from tags.
void RichTextPrivate::CreateBlocks(std::vector<RichTextTag> tags)
{
    m_blocks.clear();
    m_blocks.reserve(tags.size());

    // Create blocks using factories.
    for (RichTextTag& tag : tags) {
        const auto params = ExtractParameters(tag.tag_params);
        const auto it = std::find_if(m_block_factory_map.begin(), m_block_factory_map.end(),
                                     [tag = std::string_view{tag.tag}](const auto& key_fac)
                                     { return key_fac.first == tag; });
        if (it == m_block_factory_map.end())
            continue;
        const auto* fac = it->second.get();
        if (!fac)
            continue;
        if (auto block = fac->CreateFromTag(params, std::move(tag.content), m_font, m_color, m_format))
            m_blocks.push_back(std::move(block));
    }
}

void RichTextPrivate::AttachBlocksToOwner()
{
    m_owner.DetachChildren();

    for (const auto& block : m_blocks)
        m_owner.AttachChild(block);

    DoLayout();
}

// Update the sizes of all blocks.
void RichTextPrivate::DoLayout()
{
    X width = m_owner.ClientWidth() - X(m_padding)*2;
    Pt pos = Pt(X(m_padding), Y(m_padding));

    // The contract between RichText and block controls is this:
    // RichText tells them their width, and they determine their height.
    for (auto& block : m_blocks) {
        Pt size = block->SetMaxWidth(width);
        block->MoveTo(pos);
        pos.y += size.y;
    }

    Pt size(m_owner.Width(), pos.y + Y(m_padding));
    m_owner.Resize(size);
}

/////////////////////////////////
/// RichText public interface //
///////////////////////////////
RichText::RichText(X x, Y y, X w, Y h, const std::string& str,
                    const std::shared_ptr<Font>& font, Clr color,
                    Flags<TextFormat> format, Flags<WndFlag> flags) :
    Control(x, y, w, h, flags),
    m_self(std::make_unique<RichTextPrivate>(*this, str, font, color, format))
{
    SetName("RichText (" + std::to_string(str.size()) + "): \"" + str.substr(0, 16)
            + "\" blocks: " + std::to_string(m_self->BlockCount()));
    SetChildClippingMode(Wnd::ChildClippingMode::ClipToAncestorClient);
}

void RichText::CompleteConstruction() {
    Control::CompleteConstruction();
    m_self->AttachBlocksToOwner();
    SetName("RichText blocks: " + std::to_string(m_self->BlockCount()));
}

RichText::~RichText() = default;

void RichText::SetText(const std::string& str)
{
    std::string name_start = "RichText (" + std::to_string(str.size()) + "): \"" + str.substr(0, 16) + "\" blocks: ";
    m_self->SetText(str);
    SetName(name_start + std::to_string(m_self->BlockCount()));
}

void RichText::SetPadding(int pixels) { m_self->SetPadding(pixels); }

void RichText::SizeMove(Pt ul, Pt lr) { m_self->SizeMove(ul, lr); }

void RichText::SetBlockFactoryMap(BlockFactoryMap block_factory_map)
{ m_self->SetBlockFactoryMap(std::move(block_factory_map)); }

namespace {
    RichText::BlockFactoryMap& defaultBlockFactoryMapSingleton()
    {
        static RichText::BlockFactoryMap default_block_factories{};
        return default_block_factories;
    }
}

const RichText::BlockFactoryMap& RichText::DefaultBlockFactoryMap()
{ return defaultBlockFactoryMapSingleton(); }

int RichText::RegisterDefaultBlock(std::string_view tag, std::shared_ptr<IBlockControlFactory> factory)
{
    Font::RegisterKnownTags({tag});
    defaultBlockFactoryMapSingleton().emplace_back(tag, std::move(factory));

    // Return a dummy to enable static registration.
    return 0;
}
