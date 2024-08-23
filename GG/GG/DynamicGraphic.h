//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2000-2003 Tony Casale.
//!  Copyright (C) 2003-2008 T. Zachary Laine <whatwasthataddress@gmail.com>
//!  Copyright (C) 2013-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

//! @file GG/DynamicGraphic.h
//!
//! Contains the DynamicGraphic class, a control that allows display of
//! a slideshow or animated sequence of images.

#ifndef _GG_DynamicGraphic_h_
#define _GG_DynamicGraphic_h_


#include <GG/Control.h>
#include <GG/StaticGraphic.h>
#include <boost/signals2/signal.hpp>


namespace GG {

class Texture;

/** \brief A control that replays images in sequence, forwards or backwards,
    animated or one frame at a time.

    Frames of animation are stored in GG::Textures.  The frames are assumed to
    be laid out in rows from right to left, top to bottom, like text.  The
    location of each frame is calculated by DynamicGraphic; the user just
    needs to lay out the frames in the right order in the Texture(s) and give
    them to DynamicGraphic.  If a Texture is to be used that has "dead space"
    where there are no frames, that space must be at the end of the Texture,
    and the number of frames in the Texture should be supplied when the
    Texture is added.  When laying out the frames in the textures, the user
    can leave a margin between the frames and between the frames and the edge
    of the overall image, to make Texture creation and editing easier.  The
    width of this margin must be supplied to DynamicGraphic's ctor, and is
    constant once set.  The margin applies to the top and left of \a each
    image, so the margins at the right and bottom edges of the texture are
    optional.  The multiple-Texture ctor assumes that all Textures but the
    last are packed with frames; if you need to specify multiple Textures with
    dead space, construct with an empty \a textures parameter and use
    AddFrames().  Note that DynamicGraphic doesn't have "animated" in its
    name; it can replay images at any speed, and moreover it can be used as a
    sort of slideshow, and doesn't necessarily need to be animated at
    all. \note This is a situation in which the "last+1" idiom used throughout
    GG does not apply; when you set the end frame index to N, the last frame
    to be shown will be N, not N - 1. Also, while this control does not need
    to be the same size as the frames replayed within it, the size of the
    frames is taken from the size of the control when it is constructed. */
class GG_API DynamicGraphic : public Control
{
public:
    /** Emitted whenever playback ends because the last frame was reached and
        Looping() == false; the argument is the index of the last frame (may
        be the first frame, if playing in reverse).  \note Unlike most other
        signals, this one is emitted during the execution of Render(), so keep
        this in mind when processing this signal.*/
    typedef boost::signals2::signal<void (std::size_t)> StoppedSignalType;

    /** Emitted whenever the last frame of animation is reached; the argument
        is the index of the last frame (may be the first frame, if playing in
        reverse).  \note Unlike most other signals, this one is emitted during
        the execution of Render(), so keep this in mind when processing this
        signal.*/
    typedef boost::signals2::signal<void (std::size_t)> EndFrameSignalType;

    /** Ctor taking a vector of GG::Textures and the number of frames in those
        Textures.  The default \a frames value ALL_FRAMES indicates all
        possible area is considered to contain valid frames.  Regardless of
        the value of \a frames, all Textures but the last are assumed to have
        the maximum number of frames based on their sizes.  This ctor allows
        specification of a frame size different from the size of the
        DynamicGraphic's size. */
    DynamicGraphic(X x, Y y, X w, Y h, bool loop, X frame_width, Y frame_height, unsigned int margin,
                   std::vector<std::shared_ptr<Texture>> textures,
                   Flags<GraphicStyle> style = GRAPHIC_NONE, std::size_t frames = ALL_FRAMES,
                   Flags<WndFlag> flags = NO_WND_FLAGS);

    std::size_t  Frames() const;      ///< returns the total number of frames in all the Textures that make up the animated sequence
    bool         Playing() const;     ///< returns true if the animation is running
    bool         Looping() const;     ///< returns true if playback is looping instead of stopping when it reaches the end
    double       FPS() const;         ///< returns the number of frames playing per second; may be positive, 0, or negative
    std::size_t  FrameIndex() const;  ///< returns the index of the currently-shown frame; INVALID_INDEX if none
    unsigned int TimeIndex() const;   ///< returns the time in ms (measured from the time of the first frame); INVALID_TIME if none

    /** Returns the index of the earliest frame to be shown during playback.
        \note when playing backwards this will be the last frame shown. */
    std::size_t  StartFrame() const;

    /** Returns the index of the latest frame to be shown during playback.
        \note when playing backwards this will be the first frame shown. */
    std::size_t  EndFrame() const;

    unsigned int Margin() const;      ///< returns the number of pixels placed between frames and between the frames and the Texture edges
    X            FrameWidth() const;  ///< returns the original width of the control (and the width of the frame images)
    Y            FrameHeight() const; ///< returns the original height of the control (and the height of the frame images)

    /** Returns the style of the DynamicGraphic \see GraphicStyle */
    Flags<GraphicStyle> Style() const;

    mutable StoppedSignalType  StoppedSignal;  ///< the stopped signal object for this DynamicGraphic
    mutable EndFrameSignalType EndFrameSignal; ///< the end-frame signal object for this DynamicGraphic

    void Render() override;

    /** Adds a set of frames from Texture \a texture to the animation.  If \a
        frames == ALL_FRAMES, the Texture is assumed to contain the maximum
        possible number of frames based on its size and the frame size.
        \warning Calling code <b>must not</b> delete \a texture; \a texture
        becomes the property of a shared_ptr inside the DynamicGraphic.
        \throw GG::DynamicGraphic::CannotAddFrame Throws if \a texture is not
        large enough to contain any frames.*/
    void AddFrames(const Texture* texture, std::size_t frames = ALL_FRAMES);

    /** Adds a set of frames from Texture \a texture to the animation.  If \a
        frames == ALL_FRAMES, the Texture is assumed to contain the maximum
        possible number of frames based on its size and the frame size.
        \throw GG::DynamicGraphic::CannotAddFrame Throws if \a texture is not
        large enough to contain any frames.*/
    void AddFrames(std::shared_ptr<Texture> texture, std::size_t frames = ALL_FRAMES);

    /** Adds a set of frames from Texture \a texture to the animation.  If \a
        frames == ALL_FRAMES, the Textures are assumed to contain the
        maximum possible number of frames based on its size and the frame
        size.  Regardless of the value of \a frames, all Textures but the last
        are assumed to have the maximum number of frames based on their sizes.
        \throw GG::DynamicGraphic::CannotAddFrame Throws if no texture in \a
        textures is large enough to contain any frames.*/
    void AddFrames(std::vector<std::shared_ptr<Texture>> textures, std::size_t frames = ALL_FRAMES);

    void Play();                    ///< starts the animation of the image
    void Pause();                   ///< stops playback without adjusting the frame index
    void NextFrame();               ///< increments the frame index by 1.  If Looping() == true and the next frame would be be past the last, the first frame is shown.  Pauses playback.
    void PrevFrame();               ///< decrements the frame index by 1.  If Looping() == true and the next frame would be be past the first, the last frame is shown.  Pauses playback.
    void Stop();                    ///< stops playback and resets the frame index to 0
    void Loop(bool b = true);       ///< turns looping of playback on or off

    /** Sets the frames per second playback speed (default is 15.0 FPS).
        Negative rates indicate reverse playback.  \note Calling SetFPS(0.0)
        is equivalent to calling Pause(). */
    void SetFPS(double fps);

    void SetFrameIndex(std::size_t idx);    ///< sets the frame index to \a idx ( value is locked to range [0, Frames()] )

    /** Sets the frame index to the frame nearest time index \a idx, where \a
        idx measures time in ms from the beginning of the animation ( value is
        locked to range [0, Frames() * FPS()) ).  \note If looping is enabled,
        the time index may be any value >= 0.0, and values will "wrap" around
        the length of a loop.  If looping is disabled, any time index \a idx
        that is later than Frames() * FPS() is mapped to the last frame. */
    void SetTimeIndex(unsigned int time);

    /** Sets the index of the first frame to be shown during playback ( value
        is locked to range [0, Frames()] ).  \note when playing backwards this
        will be the last frame shown. */
    void SetStartFrame(std::size_t idx);

    /** Sets the index of the last frame to be shown during playback ( value
        is locked to range [0, Frames()] ).  \note when playing backwards this
        will be the first frame shown. */
    void  SetEndFrame(std::size_t idx);

    /** Sets the style flags, and perfroms sanity checking \see
        GraphicStyle */
    void  SetStyle(Flags<GraphicStyle> style);

    /** The base class for DynamicGraphic exceptions. */
    GG_ABSTRACT_EXCEPTION(Exception);

    /** Thrown when an attempt is made to add a frame to a DynamicGraphic from
        a texture smaller than a single frame. */
    GG_CONCRETE_EXCEPTION(CannotAddFrame, GG::DynamicGraphic, Exception);

    static constexpr std::size_t  ALL_FRAMES = std::numeric_limits<std::size_t>::max();
    static constexpr std::size_t  INVALID_INDEX = std::numeric_limits<std::size_t>::max();
    static constexpr unsigned int INVALID_TIME = std::numeric_limits<unsigned int>::max();

protected:
    struct FrameSet
    {
        std::shared_ptr<const Texture> texture; ///< the texture with the frames in it
        std::size_t frames = 0;                 ///< the number of frames in this texture
    };

    std::size_t FramesInTexture(const Texture* t) const;   ///< returns the maximum number of frames that could be stored in \a t given the size of the control and Margin()
    auto& Textures() const noexcept { return m_textures; } ///< returns the shared_ptrs to texture objects with all animation frames

    std::size_t  CurrentTexture() const;    ///< returns the current Texture being shown (part of it, anyway); INVALID_INDEX if none
    std::size_t  CurrentSubTexture() const; ///< returns the current frame being shown within Texture number CurrTexture(); INVALID_INDEX if none
    unsigned int FirstFrameTime() const;    ///< returns the time index in ms that the first frame in the sequence was shown during the current playback; INVALID_TIME if none
    unsigned int LastFrameTime() const;     ///< returns the time index in ms of the most recent frame shown (should be m_curr_frame); INVALID_TIME if none

    const unsigned int m_margin;            ///< the number of pixels placed between frames and between the frames and the Texture edges
    const X            m_frame_width;       ///< the width of each frame 
    const Y            m_frame_height;      ///< the height of each frame 

private:
    void ValidateStyle();             ///< ensures that the style flags are consistent

    std::vector<FrameSet> m_textures; ///< shared_ptrs to texture objects with all animation frames

    double       m_FPS = 1.0;           ///< current rate of playback in FPS
    bool         m_playing = true;      ///< set to true if playback is happening
    bool         m_looping = false;     ///< set to true if the playback should start over when it reaches the end
    std::size_t  m_curr_texture = 0;    ///< the current Texture being shown (part of it, anyway); INVALID_INDEX if none
    std::size_t  m_curr_subtexture = 0; ///< the current frame being shown within Texture number \a m_curr_texture; INVALID_INDEX if none
    std::size_t  m_frames = 0;          ///< the total number of frames in the animation
    std::size_t  m_curr_frame = 0;      ///< the current absolute frame being shown; INVALID_INDEX if none
    unsigned int m_first_frame_time = 0;///< the time index in ms that the first frame in the sequence was shown during the current playback; INVALID_TIME if none
    unsigned int m_last_frame_time = 0; ///< the time index in ms of the most recent frame shown (should be m_curr_frame); INVALID_TIME if none
    std::size_t  m_first_frame_idx = 0; ///< the index of the first frame shown during playback, usually 0
    std::size_t  m_last_frame_idx = 0;  ///< the index of the last frame shown during playback. usually m_frames - 1

    Flags<GraphicStyle> m_style;
};

}


#endif
