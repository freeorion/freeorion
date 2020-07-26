//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2015 Mitten-O
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

//! @file GG/RichText/RichText.h
//!
//! Contains the RichText class that allows you to intersperse text with images.

#ifndef _GG_RichText_RichText_h_
#define _GG_RichText_RichText_h_


#include <GG/ClrConstants.h>
#include <GG/RichText/BlockControl.h>
#include <GG/TextControl.h>


namespace GG
{
class RichTextPrivate;

/** \brief A control for showing text and images.
 */
class GG_API RichText: public Control
{
public:
    typedef std::map<std::string, std::string> TAG_PARAMS;

    //! An interface for object that create block controls from tags.
    class IBlockControlFactory
    {
    public:
        virtual ~IBlockControlFactory() = default;
        //! Creates a control from the tag (with unparsed parameters) and the content between the tags.
        //! You own the returned control.
        virtual std::shared_ptr<BlockControl> CreateFromTag(const std::string& tag,
                                                            const TAG_PARAMS& params,
                                                            const std::string& content,
                                                            const std::shared_ptr<Font>& font,
                                                            const Clr& color,
                                                            Flags<TextFormat> format) = 0;
    };

    //! The type of the object where we store control factories of tags.
    typedef std::map<std::string, std::shared_ptr<IBlockControlFactory>> BLOCK_FACTORY_MAP;

    //! The special tag that is used to represent plaintext.
    // Allows you to register a custom control for displaying plaintext.
    static const std::string PLAINTEXT_TAG;

    RichText(X x, Y y, X w, Y h, const std::string& str, const std::shared_ptr<Font>& font,
             Clr color = CLR_BLACK, Flags<TextFormat> format = FORMAT_NONE,
             Flags<WndFlag> flags = NO_WND_FLAGS);

    void CompleteConstruction() override;

    ~RichText();

    /** Set the text content. */
    virtual void SetText(const std::string& str);

    /**
     * @brief Set the whitespace around the content.
     *
     * @param pixels The number of pixels to reserve for empty space around the content.
     * @return void
     */
    void SetPadding(int pixels);

    void Render() override;

    void SizeMove(const Pt& ul, const Pt& lr) override;

    //! Use this to customize the handling of tags in the text on a per-object basis.
    void SetBlockFactoryMap(const std::shared_ptr<BLOCK_FACTORY_MAP>& block_factory_map);

    //! Registers a factory in the default block factory map.
    static int RegisterDefaultBlock(const std::string& tag, std::shared_ptr<IBlockControlFactory>&& factory);

    //! Access the default block factory map.
    static std::shared_ptr<RichText::BLOCK_FACTORY_MAP>& DefaultBlockFactoryMap();

private:
    friend class RichTextPrivate;
    std::unique_ptr<RichTextPrivate> const m_self;
};

}


#endif
