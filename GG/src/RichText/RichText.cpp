
#include <GG/RichText/RichText.h>

#include "RichText_p.h"
#include "TagParser.h"

#include <boost/foreach.hpp>

#include <sstream>

namespace GG {

namespace {

/// Global storage for registered block tags.
// The factory obejct live for the lifetime of the process, they are never deleted.
boost::shared_ptr< RichText::BLOCK_FACTORY_MAP >& GetDefaultTagMap() {
    static boost::shared_ptr< RichText::BLOCK_FACTORY_MAP > tag_map(new RichText::BLOCK_FACTORY_MAP());
    return tag_map;
}
}


/**
 * \brief The tag to use for text without explicit tags, or inside unknown (to the rich text system) tags.
 */
const std::string RichText::PLAINTEXT_TAG = "GG_RICH_PLAIN";

/** \brief Private implementation class for rich text control.
 *
 */
class RichTextPrivate {
public:
    RichTextPrivate(RichText* q, const std::string& str, const boost::shared_ptr<Font>& font,
                    Clr color, Flags<TextFormat> format = FORMAT_NONE):
        m_owner(q),
        m_font(font),
        m_color(color),
        m_format(format),
        m_blockFactoryMap( GetDefaultTagMap() )
        m_padding( 0 )
    {
    }

    virtual ~RichTextPrivate() {}

    // Set the text to show. Parses it into tags and uses the factory map to turn them into blocks.
    void SetText(const std::string& content) {
        m_blocks.clear();
        m_owner->DeleteChildren();

        // Get a set of tags registered for rich text usage.
        std::set<std::string> known_tags = MapKeys(*m_blockFactoryMap);

        // Parse the content into a vector of tags.
        std::vector<RichTextTag> tags = TagParser::ParseTags(content, known_tags);

        // Create blocks from the tags and populate the control with them.
        PopulateBlocks( tags );
    }

    /**
     * @brief Set the whitespace around the content.
     *
     * @param pixels The number of pixels to reserve for empty space around the content.
     * @return void
     */
    void SetPadding(int pixels) {
        if( m_padding != pixels ) {
            m_padding = pixels;
            DoLayout();
        }
    }

    void SizeMove(Pt ul, Pt lr) {
        Pt original_size = m_owner->Size();
        m_owner->Control::SizeMove(ul, lr);

        // Redo layout if necessary.
        if( m_owner->Size() != original_size ){
            DoLayout();
        }
    }

    // Get the set of keys from a map.
    template<typename T, typename V>
    std::set<T> MapKeys(const std::map<T,V>& arg_map) {
        typedef typename std::map<T,V>::value_type map_value;
        std::set<T> keys;
        BOOST_FOREACH(const map_value& pair, arg_map) {
            keys.insert(pair.first);
        }
        return keys;
    }

    // Set the mapping from tags to factories that should be used to generate blocks from them.
    void SetBlockFactoryMap(boost::shared_ptr< RichText::BLOCK_FACTORY_MAP > blockFactoryMap) {
        m_blockFactoryMap = blockFactoryMap;
    }

private:
    RichText* const m_owner; //!< The public control.
    boost::shared_ptr<Font> m_font; //!< The font to use for text.
    Clr m_color; //! < The color to use for text.
    Flags<TextFormat> m_format; //!< Text format.
    boost::shared_ptr< RichText::BLOCK_FACTORY_MAP > m_blockFactoryMap; //!< A map that tells us how to generate block controls from tags.
    std::vector<BlockControl*> m_blocks; //!< The blocks generated from our content.
    int m_padding;

    // Easier access to m_blockFactoryMap
    RichText::BLOCK_FACTORY_MAP& FactoryMap() {
        return *m_blockFactoryMap;
    }

    // Create blocks from tags.
    void PopulateBlocks(const std::vector< RichTextTag >& tags) {
        // Create blocks using factories.
        BOOST_FOREACH(const RichTextTag& tag, tags) {
            BlockControl* block = FactoryMap()[tag.tag]->CreateFromTag(tag.tag, tag.tag_params, tag.content, m_font, m_color, m_format);
            m_owner->AttachChild(block);
            m_blocks.push_back(block);
        }

        DoLayout();
    }

    // Update the sizes of all blocks.
    void DoLayout() {
        X width = m_owner->ClientWidth() - X(m_padding);
        Pt pos = Pt( X(m_padding), Y(m_padding) );

        // The contract between RichText and block controls is this:
        // RichText tells them their width, and they determine their height.
        BOOST_FOREACH(BlockControl* block, m_blocks) {
            Pt size = block->SetMaxWidth(width);
            block->MoveTo(pos);
            pos.y += size.y;
        }

        Pt size(m_owner->Width(), pos.y + Y(m_padding));
        m_owner->Resize(size);
    }
};

/////////////////////////////////
/// RichText public interface //
///////////////////////////////

RichText::RichText(X x, Y y, X w, Y h, const std::string& str, const boost::shared_ptr<Font>& font,
                   Clr color, Flags<TextFormat> format,
                   Flags<WndFlag> flags):
    Control(x, y, w, h, flags),
    m_self(new RichTextPrivate(this, str, font, color, format))
{
    // Set text requires our members to be set.
    // Therefore we must call it here, not in constructor of Private.
    m_self->SetText(str);
}

RichText::~RichText()
{
    delete m_self;
}

void RichText::SetText(const std::string& str) {
    m_self->SetText(str);
}

void RichText::SetPadding(int pixels)
{
    m_self->SetPadding(pixels);
}

void RichText::Render() {

}

void RichText::SizeMove(const Pt& ul, const Pt& lr) {
    m_self->SizeMove(ul, lr);
}

void RichText::SetBlockFactoryMap(const boost::shared_ptr< BLOCK_FACTORY_MAP >& blockFactoryMap) {
    m_self->SetBlockFactoryMap( blockFactoryMap );
}

int RichText::RegisterDefaultBlock(const std::string& tag, IBlockControlFactory* factory) {
    Font::RegisterKnownTag(tag);
    (*GetDefaultTagMap())[tag] = factory;

    // Return a dummy to enable static registration.
    return 0;
}

}
