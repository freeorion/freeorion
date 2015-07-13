#include "MarkupBox.h"

#include "CUIControls.h"

#include "../util/Logger.h"

#include <GG/StaticGraphic.h>
#include <GG/MultiEdit.h>
#include <GG/WndEvent.h>
#include <GG/DrawUtil.h>
#include <GG/StyleFactory.h>

namespace {
    static const GG::X          SCROLL_WIDTH(14);                       // width of scrollbar in MarkupBox
    static const int            EDGE_PAD(3);                            // padding on edges of MarkupBox that limits size of MarkupSurface
    static const GG::Y          VERY_FAR_DOWN_LAYOUT_SURFACE(2 << 15);  // maximum length in pixels of surface on which to render marked up text

    static const std::string    EMPTY_STRING("");
    static const std::string    IMAGE_TAG("img");
    static const std::string    HEADING_TAG("h");
    static bool                 markup_tags_registered = false;

    /** Ensures GG::Font knows about markup tags. */
    void RegisterMarkupTags() {
        if (markup_tags_registered)
            return;
        GG::Font::RegisterKnownTag(IMAGE_TAG);      // indicates image should be placed in markup text.  optional right or left flags indicate alignment; if absent, image appear in text
        GG::Font::RegisterKnownTag(HEADING_TAG);    // indicates enclosed text should be rendered as a heading: in different size or style of font
        markup_tags_registered = true;
    }

    /** Parses text and returns array of MarkupTextBlock that can be individually interpreted as a single type of 
      * object on MarkupSurface (heading, plain text, image...) */
    std::vector<MarkupTextBlock> ParseMarkupText(const std::string& text) {
        std::vector<MarkupTextBlock> retval;

        // use GG::Font to parse string to extract determine tags applied to text
        std::vector<GG::Font::LineData> line_data;
        GG::Flags<GG::TextFormat> fmt = GG::FORMAT_NONE;
        ClientUI::GetFont()->DetermineLines(text, fmt, GG::X1, line_data);
        if (line_data.empty()) {
            DebugLogger() << "ParseMarkupText line data empty!  raw text:" << text;
            return retval;
        }

        for (std::vector<GG::Font::LineData>::const_iterator line_it = line_data.begin(); line_it != line_data.end(); ++line_it) {
            const GG::Font::LineData& line = *line_it;
            if (line.Empty())   // checks if char_data is empty
                continue;
            const std::vector<GG::Font::LineData::CharData>& char_data_vec = line.char_data;


            // scan through char_data vector, creating a new entry in retval for each block of text
            // that is delimited by a markup tag

            MarkupTextBlockType         open_text_block_type = PLAIN_TEXT_MARKUP;   // what kind of text is at current char, determined from the most recent markup tag
            std::vector<std::string>    open_text_block_params;                     // parameters specified in current text block opening tag
            unsigned int                text_block_start = 0;                       // first character in the current block of text
            unsigned int                text_block_end = char_data_vec.size() - 1;  // last character in current block of text.

            // DEBUG
            std::cout << "char vec size: " << char_data_vec.size() << std::endl << std::endl;
            // END DBUG

            // scan through text until the text block type changes.  text block type changes happen
            // when a an (open or close) markup tag is one of the tags of a CharData in the vector
            for (unsigned int i = 0; i < char_data_vec.size(); ++i) {
                // get current CharData
                const GG::Font::LineData::CharData& cur_char_data = char_data_vec[i];
                // get tags on current CharData
                const std::vector<boost::shared_ptr<GG::Font::FormattingTag> >& tags = cur_char_data.tags;

                //DEBUG
                {
                GG::StrSize char_start = cur_char_data.string_index;
                GG::StrSize char_size = cur_char_data.string_size;
                std::string char_text = std::string(text, Value(char_start), Value(char_size));
                std::cout << "ParseMarkupText char[" << i << "]: " << char_text;
                for (std::vector<boost::shared_ptr<GG::Font::FormattingTag> >::const_iterator it = tags.begin(); it != tags.end(); ++it) {
                    boost::shared_ptr<GG::Font::FormattingTag> temp_format_tag = *it;
                    std::cout << " <";
                    if (temp_format_tag->close_tag)
                        std::cout << "/";
                    std::cout << temp_format_tag->tag_name;
                    std::cout << ">";
                }
                std::cout << std::endl;
                }
                //END DEBUG


                // scan tags for markup tag
                MarkupTextBlockType current_char_type = PLAIN_TEXT_MARKUP;  // default, if no tag present in current CharData
                boost::shared_ptr<GG::Font::FormattingTag> current_char_format_tag;


                for (std::vector<boost::shared_ptr<GG::Font::FormattingTag> >::const_iterator it = tags.begin(); it != tags.end(); ++it) {
                    boost::shared_ptr<GG::Font::FormattingTag> format_tag_temp = *it;
                    // check for image tag first.  image overrides / ends any other tags
                    if (format_tag_temp->tag_name == IMAGE_TAG) {
                        current_char_type = IMAGE_MARKUP;
                        current_char_format_tag = format_tag_temp;
                        break;
                    // next check for header tag
                    } else if (format_tag_temp->tag_name == HEADING_TAG) {
                        current_char_type = HEADING_MARKUP;
                        current_char_format_tag = format_tag_temp;
                        break;
                    }
                }

                // if current char has a format tag, get the params to that tag
                std::vector<std::string> current_format_tag_params;
                if (current_char_format_tag) {

                    // DEBUG
                    if (current_char_format_tag->close_tag)
                        std::cout << "format tag </" << current_char_format_tag->tag_name << " ";
                    else
                        std::cout << "format tag <" << current_char_format_tag->tag_name << " ";
                    // END DBUG

                    std::vector<GG::Font::Substring> params_temp = current_char_format_tag->params;
                    for (std::vector<GG::Font::Substring>::const_iterator it = params_temp.begin(); it != params_temp.end(); ++it) {
                        current_format_tag_params.push_back(*it);

                        // DEBUG
                        std::cout << current_format_tag_params.back() << " ";
                        // END DEBUG
                    }

                    // DEBUG
                    std::cout << ">" << std::endl << std::endl;
                    // END DBUG
                }


                // if current CharData has a markup tag (ie. is not Plain Text), or if current CharData is
                // is the last, need to make a new MarkupTextBlock entry in retval
                if (current_char_type != PLAIN_TEXT_MARKUP || i >= char_data_vec.size() - 1) {

                    // check if this is the first character in the char data vector.  if it is, don't want to
                    // close a tag yet; just record this character's tag as the open one
                    if (i > 0) {
                        // current CharData is not first character in char data vector.
                        // rather, a tag indicates that the previous block has ended


                        // get last character of current text block
                        if (current_char_type == PLAIN_TEXT_MARKUP)
                            text_block_end = i;     // end at current character if it is the last CharData
                        else
                            text_block_end = i - 1; // end at preceeding character if closing block due to tag



                        // DEBUG
                        if (current_char_type != PLAIN_TEXT_MARKUP) {
                            if (current_char_format_tag->close_tag) {
                                std::cout << "found markup close tag!" << std::endl;
                                current_char_type = PLAIN_TEXT_MARKUP;
                            } else {
                                std::cout << "found markup tag!" << std::endl;
                            }
                        } else {
                            std::cout << "reached end of char vector of size: " << char_data_vec.size() <<
                                         " at index: " << i << std::endl;
                        }
                        // END DEBUG



                        // add new MarkupTextBlock entry to retval, as long as the entry is not empty
                        if (text_block_end >= text_block_start) {
                            // DEBUG
                            std::cout << "MarkupTextBlock text block start: " << text_block_start <<
                                         " and end: " << text_block_end << std::endl;
                            // END DEBUG

                            GG::StrSize start = char_data_vec[text_block_start].string_index;
                            GG::StrSize end = char_data_vec[text_block_end].string_index + char_data_vec[text_block_end].string_size;
                            std::string block_text_temp = std::string(text, Value(start), Value(end - start));


                            MarkupTextBlock block;
                            block.text = block_text_temp.substr(0, block_text_temp.find_last_not_of("\n") + 1); // strip trailing newlines
                            block.type = open_text_block_type;
                            block.params = open_text_block_params;

                            // DEBUG
                            std::cout << "MarkupTextBlock: " << std::endl;
                            std::cout << "text: " << block.text << std::endl;
                            std::cout << "type: " << block.type << std::endl;
                            std::cout << "params: ";
                            for (std::vector<std::string>::const_iterator pit = block.params.begin(); pit != block.params.end(); ++pit)
                                std::cout << *pit << " ";
                            std::cout << std::endl << std::endl;
                            // END DEBUG

                            retval.push_back(block);
                        }
                    }


                    // set new open tag type and start index.  if current tag is an open tag, it is the start
                    // of a new text block.  if it is a close tag, it is not part of a new block, and the new
                    // block starts at a later character
                    if (current_char_format_tag && current_char_format_tag->close_tag) {
                        text_block_start = i + 1;
                        open_text_block_type = PLAIN_TEXT_MARKUP;
                        open_text_block_params.clear();
                    } else {
                        text_block_start = i;
                        open_text_block_type = current_char_type;
                        open_text_block_params = current_format_tag_params;
                    }
                }
            }
        }

        // DEBUG
        std::cout << "retval.size(): " << retval.size() << std::endl;
        std::cout << "ParseMarkupText done!" << std::endl << std::endl;
        // END DEBUG

        return retval;
    }
}

//////////////////////////////
// MarkupBox::MarkupSurface //
//////////////////////////////
class MarkupBox::MarkupSurface : public GG::Control {
public:
    /** \name Structors */ ///@{
    /** Ctor. */
    MarkupSurface(GG::X x, GG::Y y, GG::X w, GG::Y h, const std::string& str);

    MarkupSurface();            ///< default ctor
    ~MarkupSurface();           ///< dtor
    //@}

    /** \name Accessors */ ///@{
    const std::string&  Text() const;
    //@}

    /** \name Mutators */ ///@{

    /** Removes old controls and recreates new contents of surface from passed
        \a str text. If PreserveScrollPositionOnNextTextSet() has been called,
        does not alter scroll position.  Otherwise, resets scroll position to
        top of contents. */
    void                SetText(const std::string& str);

    void                Clear();                    ///< Removes all controls from surface.
    void                Refresh();                  ///< Removes and recreates all controls on surface using existing text

    virtual void        Render();
    //@}

private:
    std::string                 m_text;

    std::vector<GG::Control*>   m_controls;     ///< GG::Controls used to display marked up text
};

MarkupBox::MarkupSurface::MarkupSurface(GG::X x, GG::Y y, GG::X w, GG::Y h, const std::string& str) :
    GG::Control(x, y, w, h),
    m_text(""),
    m_controls()
{
    SetText(str);
    SetChildClippingMode(ClipToClient);
}

MarkupBox::MarkupSurface::MarkupSurface() :
    GG::Control(),
    m_text(""),
    m_controls()
{
    SetChildClippingMode(ClipToClient);
}

MarkupBox::MarkupSurface::~MarkupSurface() {
    Clear();
}

const std::string& MarkupBox::MarkupSurface::Text() const {
    return m_text;
}

void MarkupBox::MarkupSurface::SetText(const std::string& str) {
    Clear();
    m_text = str;
    Refresh();
}

void MarkupBox::MarkupSurface::Clear() {
    for (std::vector<GG::Control*>::iterator it = m_controls.begin(); it != m_controls.end(); ++it)
        delete *it;
    m_controls.clear();
    m_text = "";
}

void MarkupBox::MarkupSurface::Refresh() {
    // remove old controls / contents
    for (std::vector<GG::Control*>::iterator it = m_controls.begin(); it != m_controls.end(); ++it)
        delete *it;
    m_controls.clear();

    // common resources...
    boost::shared_ptr<GG::Font> plain_font = ClientUI::GetFont();               // font for plain text
    boost::shared_ptr<GG::Font> header_font = ClientUI::GetTitleFont();         // font for headings
    GG::Flags<GG::TextFormat> format = GG::FORMAT_LEFT | GG::FORMAT_WORDBREAK;  // format for text blocks: header and plain text

    std::stack<GG::Rect> working_space_stack;   // keeps track of nested constrains on space where controls can be positioned.
    GG::Rect(GG::X0, GG::Y0, Width(), VERY_FAR_DOWN_LAYOUT_SURFACE);

    // put default element position at top left of surface
    GG::Y top = GG::Y0;
    GG::X left = GG::X0;

    std::vector<MarkupTextBlock> markup_text_blocks = ParseMarkupText(m_text);

    std::cout << "MarkupSurface::Refresh() after calling ParseMarkupText" << std::endl;
    std::cout << "size of vector of blocks: " << markup_text_blocks.size() << std::endl << std::endl;

    // DEBUG
    for (std::vector<MarkupTextBlock>::const_iterator it = markup_text_blocks.begin(); it != markup_text_blocks.end(); ++it) {
        std::cout << "Markup Text Block type: " << it->type << " params: ";
        for (std::vector<std::string>::const_iterator strit = it->params.begin(); strit != it->params.end(); ++strit)
            std:: cout << *strit << " ";
        std::cout << " text: " << it->text << std::endl << std::endl;
    }
    // END DEBUG

    // for each MarkupTextBlock, create the appropriate markup object

    // if object is an image, check if it has left or right tags.  if it does, position it
    // appropriately, and adjust available_Width and top and left to fit around image.


    for (std::vector<MarkupTextBlock>::const_iterator it = markup_text_blocks.begin(); it != markup_text_blocks.end(); ++it) {
        const MarkupTextBlock& block = *it;

        // if just text:
        GG::Control* control = 0;

        if (block.type == PLAIN_TEXT_MARKUP) {
            control = new LinkText(GG::X0, top, Width(), block.text, plain_font, format, ClientUI::TextColor());
        } else if (block.type == HEADING_MARKUP) {
            control = new LinkText(GG::X0, top, Width(), block.text, header_font, format, ClientUI::TextColor());
        } else if (block.type == IMAGE_MARKUP) {
            std::string filename = "";
            if (!block.params.empty())
                filename = block.params[0];
            boost::shared_ptr<GG::Texture> texture = ClientUI::GetTexture(ClientUI::ArtDir() / filename, true);
            control = new GG::StaticGraphic(texture, GG::GRAPHIC_FITGRAPHIC | GG::GRAPHIC_PROPSCALE);
            control->MoveTo(GG::Pt(left, top));
            control->Resize(GG::Pt(texture->Width(), texture->Height()));
        } else {
            std::cout << "unrecognized markup type" << std::endl;
            continue;
        }

        m_controls.push_back(control);
        AttachChild(control);

        control->MoveTo(GG::Pt(left, top));
        top += control->Height() + EDGE_PAD;

        std::cout << "MarkupSurface::Refresh control (x,y): " << control->Left() << ", " << control->Top() <<
                                                   " (w,h): " << control->Width() << ", " << control->Height() << std::endl;
    }

    Resize(GG::Pt(Width(), GG::Y(top + plain_font->Lineskip())));

    std::cout << std::endl << std::endl << "#############################################################" << std::endl << std::endl;
}

void MarkupBox::MarkupSurface::Render() {
    // do nothing.  controls on surface give it appearance.  background that contains surface give it a background.
}


//////////////////////////////
//        MarkupBox         //
//////////////////////////////
MarkupBox::MarkupBox(GG::X x, GG::Y y, GG::X w, GG::Y h, const std::string& str, GG::Flags<GG::WndFlag> flags) :
    GG::Control(x, y, w, h, flags),
    m_vscroll(0),
    m_surface(0)
{
    RegisterMarkupTags();
    m_surface = new MarkupSurface(GG::X0 + EDGE_PAD, GG::Y0 + EDGE_PAD, w - 2*EDGE_PAD, h - 2*EDGE_PAD, str);
    AttachChild(m_surface);
    SetChildClippingMode(ClipToClient);
    Refresh();
    AdjustScrolls();
    VScrolled(0, 0, 0, 0);
}


MarkupBox::MarkupBox() :
    GG::Control(),
    m_vscroll(0),
    m_surface(0)
{
    RegisterMarkupTags();
    m_surface = new MarkupSurface(GG::X0, GG::Y0, GG::X0, GG::Y0, "");
    AttachChild(m_surface);
    SetChildClippingMode(ClipToClient);
    Refresh();
    VScrolled(0, 0, 0, 0);
    // shouldn't need to adjust scrolls since text is empty
}

MarkupBox::~MarkupBox() {
    delete m_vscroll;
    //delete m_surface; // should be unnecessary
}

const std::string& MarkupBox::Text() const {
    if (m_surface)
        return m_surface->Text();
    else
        return EMPTY_STRING;
}

void MarkupBox::Render() {
    // Draw outline and background...
    GG::FlatRectangle(UpperLeft(), LowerRight(), ClientUI::WndColor(), ClientUI::WndOuterBorderColor(), 1);
}

void MarkupBox::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    GG::Pt old_size = Size();
    GG::Control::SizeMove(ul, lr);
    if (Size() != old_size) {
        if (m_vscroll)
            m_surface->SizeMove(GG::Pt(GG::X0 + EDGE_PAD,                   GG::Y0 + EDGE_PAD),
                                GG::Pt(Width() - SCROLL_WIDTH - 2*EDGE_PAD, Height() - 2*EDGE_PAD));
        else
            m_surface->SizeMove(GG::Pt(GG::X0 + EDGE_PAD,                   GG::Y0 + EDGE_PAD),
                                GG::Pt(Width() - 2*EDGE_PAD,                Height() - 2*EDGE_PAD));

        Refresh();
    }
}

void MarkupBox::SetText(const std::string& str) {
    // save old surface size
    GG::Pt old_surface_size = m_surface->Size();

    // update text, which might change size of surface
    m_surface->SetText(str);

    // check new surface size.  if it has changed, check if scrollbar positions and layout need changing
    if (m_surface->Size() != old_surface_size)
        AdjustScrolls();
}

void MarkupBox::Refresh() {
    m_surface->Refresh();
}

void MarkupBox::MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys) {
    std::cout << "MarkupBox::MouseWheel move: " << move << std::endl;
    if (!Disabled() && m_vscroll) {
        for (int i = 0; i < move; ++i) {
            m_vscroll->ScrollLineDecr();
            GG::SignalScroll(*m_vscroll, i == move -1);
        }
        for (int i = 0; i < -move; ++i) {
            m_vscroll->ScrollLineIncr();
            GG::SignalScroll(*m_vscroll, i == -move -1);
        }
    }
}

void MarkupBox::KeyPress(GG::Key key, boost::uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys) {
    if (Disabled()) {
        GG::Control::KeyPress(key, key_code_point, mod_keys);
        return;
    }

    if (!m_vscroll)
        return;

    int bottom = 0;

    switch (key) {
    case GG::GGK_UP:
        m_vscroll->ScrollLineDecr();
        GG::SignalScroll(*m_vscroll, true);
        break;
    case GG::GGK_DOWN:
        m_vscroll->ScrollLineIncr();
        GG::SignalScroll(*m_vscroll, true);
        break;
    case GG::GGK_HOME:
        m_vscroll->ScrollTo(0);
        GG::SignalScroll(*m_vscroll, true);
        break;
    case GG::GGK_END:
        bottom = Value(m_surface->Height() - Height()); // find top position so that bottom of surface is at bottom of this box
        m_vscroll->ScrollTo(std::max(0, bottom));       // but don't allow negative scroll positions, in case this height is larger than surface height
        GG::SignalScroll(*m_vscroll, true);
        break;
    case GG::GGK_PAGEUP:
        m_vscroll->ScrollPageDecr();
        GG::SignalScroll(*m_vscroll, true);
        break;
    case GG::GGK_PAGEDOWN:
        m_vscroll->ScrollPageIncr();
        GG::SignalScroll(*m_vscroll, true);
        break;
    default:
        break;
    }
}

void MarkupBox::Clear() {
    m_surface->Clear();
    VScrolled(0, 0, 0, 0);
}

void MarkupBox::AdjustScrolls() {
    if (!m_surface) {
        delete m_vscroll;   m_vscroll = 0;
        return;
    }

    GG::Y surface_height = m_surface->Height();
    GG::Y this_height = Height();
    GG::Y line_height = ClientUI::GetFont()->Lineskip() * 3;

    if (m_vscroll) {
        if (surface_height <= this_height) {
            // there is a scrollbar, but it's not needed.  Remove it.
            delete m_vscroll;   m_vscroll = 0;

            // resize surface to account for extra horizontal space due to lack of scrollbars
            m_surface->SizeMove(UpperLeft(), LowerRight());

            // redo text layout, since removing scrollbar added extra width
            m_surface->Refresh();

            // ensure surface top is at top of this control
            VScrolled(0, 0, 0, 0);

        } else {
            // there is a scrollbar already and it's still needed.  adjust scroll range
            m_vscroll->SizeScroll(0 /* min value scrollbar can take */, Value(surface_height) /* max value scrollbar can take */,
                                  static_cast<unsigned int>(Value(line_height)) /* size of one line tick of bar */,
                                  static_cast<unsigned int>(Value(this_height)) /* size of one page tick of bar */);
        }
    } else if (!m_vscroll) {
        if (surface_height <= this_height) {
            // there is no scrollbar, and no bar is needed.  Make sure surface is showing from top.
            VScrolled(0, 0, 0, 0);

        } else {
            // there is no scrollbar, but one is needed.  Create one
            boost::shared_ptr<GG::StyleFactory> style = GetStyleFactory();

            m_vscroll = style->NewMultiEditVScroll(ClientUI::TextColor(), GG::CLR_ZERO);
            m_vscroll->MoveTo(GG::Pt(Width() - SCROLL_WIDTH, GG::Y0));
            m_vscroll->Resize(GG::Pt(SCROLL_WIDTH, Height()));

            // adjust size of surface since creating a scrollbar takes up some horizontal space, which affects layout
            m_surface->SizeMove(UpperLeft(), LowerRight() - GG::Pt(SCROLL_WIDTH, GG::Y0));

            // redo layout after changing surface size
            m_surface->Refresh();

            // adjust range of scrolling and size of increments and pages after potentially resizing surface during refresh
            m_vscroll->SizeScroll(0, Value(surface_height),
                                  static_cast<unsigned int>(Value(line_height)),
                                  static_cast<unsigned int>(Value(this_height)));

            // show new scrollbar and connect it functionally to this control
            AttachChild(m_vscroll);
            GG::Connect(m_vscroll->ScrolledSignal, &MarkupBox::VScrolled, this);
        }
    }
}

void MarkupBox::VScrolled(int upper, int ignored1, int ignored2, int ignored3) {
    // move surface in opposite direction to distance scroll is scrolled in vertical direction.  Thus, scrolling down
    // reveals more of lower parts of surface, and vice versa.
    m_surface->MoveTo(GG::Pt(GG::X0, GG::Y(-upper)));
}

