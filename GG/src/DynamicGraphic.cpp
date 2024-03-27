//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2000-2003 Tony Casale.
//!  Copyright (C) 2003-2008 T. Zachary Laine <whatwasthataddress@gmail.com>
//!  Copyright (C) 2013-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

#include <cmath>
#include <GG/ClrConstants.h>
#include <GG/DrawUtil.h>
#include <GG/DynamicGraphic.h>
#include <GG/GUI.h>
#include <GG/Texture.h>


using namespace GG;

namespace {

struct SignalEcho
{
    SignalEcho(const std::string& name) : m_name(name) {}
    void operator()(std::size_t index)
    { std::cerr << "GG SIGNAL : " << m_name << "(index=" << index << ")" << std::endl; }
    std::string m_name;
};

constexpr double DEFAULT_FPS = 15.0;

}

DynamicGraphic::DynamicGraphic(X x, Y y, X w, Y h, bool loop, X frame_width, Y frame_height,
                               unsigned int margin, std::vector<std::shared_ptr<Texture>> textures,
                               Flags<GraphicStyle> style, std::size_t frames,
                               Flags<WndFlag> flags) :
    Control(x, y, w, h, flags),
    m_margin(margin),
    m_frame_width(frame_width),
    m_frame_height(frame_height),
    m_FPS(DEFAULT_FPS),
    m_looping(loop),
    m_first_frame_time(INVALID_TIME),
    m_last_frame_time(INVALID_TIME),
    m_style(style)
{
    ValidateStyle();
    SetColor(CLR_WHITE);
    AddFrames(std::move(textures), frames);
    m_last_frame_idx = m_frames - 1;

    if (INSTRUMENT_ALL_SIGNALS) {
        StoppedSignal.connect(SignalEcho("DynamicGraphic::StoppedSignal"));
        //EndFrameSignal.connect(SignalEcho("DynamicGraphic::EndFrameSignal"));
    }
}

std::size_t DynamicGraphic::Frames() const       
{ return m_frames; }

bool DynamicGraphic::Playing() const
{ return m_playing; }

bool DynamicGraphic::Looping() const
{ return m_looping; }

double DynamicGraphic::FPS() const
{ return m_FPS; }

std::size_t DynamicGraphic::FrameIndex() const
{ return m_curr_frame; }

unsigned int DynamicGraphic::TimeIndex() const
{ return m_last_frame_time; }

std::size_t DynamicGraphic::StartFrame() const
{ return m_first_frame_idx; }

std::size_t DynamicGraphic::EndFrame() const
{ return m_last_frame_idx; }

unsigned int DynamicGraphic::Margin() const
{ return m_margin; }

X DynamicGraphic::FrameWidth() const
{ return m_frame_width; }

Y DynamicGraphic::FrameHeight() const
{ return m_frame_height; }

Flags<GraphicStyle> DynamicGraphic::Style() const
{ return m_style; }

void DynamicGraphic::Render()
{
    if (m_curr_texture < m_textures.size() && m_curr_subtexture < m_textures[m_curr_texture].frames) {
        bool send_stopped_signal = false;
        bool send_end_frame_signal = false;

        // advance frames
        std::size_t initial_frame_idx = (0.0 <= m_FPS ? m_first_frame_idx : m_last_frame_idx);
        std::size_t final_frame_idx =   (0.0 <= m_FPS ? m_last_frame_idx : m_first_frame_idx);
        if (m_playing) {
            if (m_first_frame_time == INVALID_TIME) {
                m_last_frame_time = m_first_frame_time = GUI::GetGUI()->Ticks();
                if (m_FPS)
                    m_first_frame_time -= static_cast<unsigned int>(1000.0 / m_FPS * m_curr_frame);
            } else {
                std::size_t old_frame = m_curr_frame;
                std::size_t curr_time = GUI::GetGUI()->Ticks();
                SetFrameIndex(initial_frame_idx +
                              static_cast<std::size_t>((curr_time - m_first_frame_time) * m_FPS / 1000.0) %
                              (m_last_frame_idx - m_first_frame_idx + 1));

                // determine whether the final frame was passed
                std::size_t frames_passed =
                    static_cast<std::size_t>((curr_time - m_last_frame_time) * m_FPS / 1000.0);
                if (m_frames <= frames_passed ||
                    (0.0 <= m_FPS ? m_curr_frame < old_frame : old_frame < m_curr_frame)) {
                    send_end_frame_signal = true;
                    // if looping isn't allowed, stop at the last frame
                    if (!m_looping) {
                        m_playing = false;
                        m_first_frame_time = INVALID_TIME;
                        SetFrameIndex(final_frame_idx);
                        send_stopped_signal = true;
                    }
                }
                m_last_frame_time = static_cast<decltype(m_last_frame_time)>(curr_time);
            }
        }

        // render current frame
        Clr color_to_use = Disabled() ? DisabledColor(Color()) : Color();
        glColor(color_to_use);

        const int INT_MARGIN = m_margin;
        std::size_t cols = static_cast<std::size_t>(
            m_textures[m_curr_texture].texture->DefaultWidth() / (m_frame_width + INT_MARGIN));
        X x = static_cast<int>(m_curr_subtexture % cols) * (m_frame_width + INT_MARGIN) + INT_MARGIN;
        Y y = static_cast<int>(m_curr_subtexture / cols) * (m_frame_height + INT_MARGIN) + INT_MARGIN;
        SubTexture st(m_textures[m_curr_texture].texture, x, y, x + m_frame_width, y + m_frame_height);

        Pt ul = UpperLeft(), lr = LowerRight();
        Pt window_sz(lr - ul);
        Pt graphic_sz(m_frame_width, m_frame_height);
        Pt pt1, pt2(graphic_sz); // (unscaled) default graphic size
        if (m_style & GRAPHIC_FITGRAPHIC) {
            if (m_style & GRAPHIC_PROPSCALE) {
                float scale_x = Value(window_sz.x) * 1.0f / Value(graphic_sz.x);
                float scale_y = Value(window_sz.y) * 1.0f / Value(graphic_sz.y);
                float scale = std::min(scale_x, scale_y);
                pt2.x = ToX(graphic_sz.x * scale);
                pt2.y = ToY(graphic_sz.y * scale);
            } else {
                pt2 = window_sz;
            }
        } else if (m_style & GRAPHIC_SHRINKFIT) {
            if (m_style & GRAPHIC_PROPSCALE) {
                const double scale_x = (graphic_sz.x > window_sz.x) ? window_sz.x / (graphic_sz.x * 1.0) : 1.0;
                const double scale_y = (graphic_sz.y > window_sz.y) ? window_sz.y / (graphic_sz.y * 1.0) : 1.0;
                const double scale = std::min(scale_x, scale_y);
                pt2.x = ToX(graphic_sz.x * scale);
                pt2.y = ToY(graphic_sz.y * scale);
            } else {
                pt2 = window_sz;
            }
        }

        X x_shift = X0;
        if (m_style & GRAPHIC_LEFT) {
            x_shift = ul.x;
        } else if (m_style & GRAPHIC_CENTER) {
            x_shift = ul.x + (window_sz.x - (pt2.x - pt1.x)) / 2;
        } else { // m_style & GRAPHIC_RIGHT
            x_shift = lr.x - (pt2.x - pt1.x);
        }
        pt1.x += x_shift;
        pt2.x += x_shift;

        Y y_shift = Y0;
        if (m_style & GRAPHIC_TOP) {
            y_shift = ul.y;
        } else if (m_style & GRAPHIC_VCENTER) {
            y_shift = ul.y + (window_sz.y - (pt2.y - pt1.y)) / 2;
        } else { // m_style & GRAPHIC_BOTTOM
            y_shift = lr.y - (pt2.y - pt1.y);
        }
        pt1.y += y_shift;
        pt2.y += y_shift;

        st.OrthoBlit(pt1, pt2);

        if (send_end_frame_signal)
            EndFrameSignal(final_frame_idx);
        if (send_stopped_signal)
            StoppedSignal(m_curr_frame);
    }
}

void DynamicGraphic::AddFrames(const Texture* texture, std::size_t frames)
{
    std::size_t frames_in_texture = FramesInTexture(texture);
    if (!frames_in_texture)
        throw CannotAddFrame("DynamicGraphic::AddFrames : attempted to add frames from a Texture too small for even one frame");

    FrameSet fs;
    fs.texture.reset(texture);
    fs.frames = std::min(frames_in_texture, std::max<std::size_t>(frames, 1));
    m_frames += fs.frames;
    m_textures.push_back(std::move(fs));
}

void DynamicGraphic::AddFrames(std::shared_ptr<Texture> texture, std::size_t frames)
{
    std::size_t frames_in_texture = FramesInTexture(texture.get());
    if (!frames_in_texture)
        throw CannotAddFrame("DynamicGraphic::AddFrames : attempted to add frames from a Texture too small for even one frame");

    FrameSet fs;
    fs.texture = std::move(texture);
    fs.frames = std::min(frames_in_texture, std::max<std::size_t>(frames, 1));
    m_frames += fs.frames;
    m_textures.push_back(std::move(fs));
}

void DynamicGraphic::AddFrames(std::vector<std::shared_ptr<Texture>> textures, std::size_t frames)
{
    if (!textures.empty()) {
        std::size_t old_frames = m_frames;
        for (std::size_t i = 0; i < textures.size() - 1; ++i)
            AddFrames(std::move(textures[i]), ALL_FRAMES);
        AddFrames(std::move(textures.back()), m_frames - old_frames);
    }
}

void DynamicGraphic::Play()
{
    // if we're at the end of a previous playback and looping is disabled, reset the frame index to the initial frame
    if (!m_playing && !m_looping && m_curr_frame == (0.0 <= m_FPS ? m_last_frame_idx : m_first_frame_idx))
        SetFrameIndex(0.0 <= m_FPS ? m_first_frame_idx : m_last_frame_idx);
    m_playing = true;
    if (m_FPS == 0.0)
        m_FPS = DEFAULT_FPS;
}

void DynamicGraphic::Pause()
{ m_playing = false; }

void DynamicGraphic::NextFrame()
{
    if (m_curr_texture != INVALID_INDEX &&
        m_curr_subtexture != INVALID_INDEX &&
        !m_textures.empty()) {
        m_playing = false;
        if (m_curr_frame == m_last_frame_idx) { // if this is the very last frame
            if (m_looping) // only wrap around if looping is turned on
                SetFrameIndex(m_first_frame_idx);
        } else if (m_curr_subtexture == m_textures[m_curr_texture].frames - 1) {
            ++m_curr_texture;
            m_curr_subtexture = 0;
            ++m_curr_frame;
        } else {
            ++m_curr_subtexture;
            ++m_curr_frame;
        }
    }
}

void DynamicGraphic::PrevFrame()
{
    if (m_curr_texture != INVALID_INDEX &&
        m_curr_subtexture != INVALID_INDEX &&
        !m_textures.empty()) {
        m_playing = false;
        if (m_curr_frame == m_first_frame_idx) { // if this is the very first frame
            if (m_looping) // only wrap around if looping is turned on
                SetFrameIndex(m_last_frame_idx);
        } else if (!m_curr_subtexture) { // if this is the first frame in its texture
            --m_curr_texture;
            m_curr_subtexture = m_textures[m_curr_texture].frames - 1;
            --m_curr_frame;
        } else { // if this is some frame in the middle of its texture
            --m_curr_subtexture;
            --m_curr_frame;
        }
    }
}

void DynamicGraphic::Stop()
{
    m_playing = false;
    SetFrameIndex(0.0 <= m_FPS ? m_first_frame_idx : m_last_frame_idx);
}

void DynamicGraphic::Loop(bool b)
{ m_looping = b; }

void DynamicGraphic::SetFPS(double fps)
{ m_FPS = fps; }

void DynamicGraphic::SetFrameIndex(std::size_t idx)
{
    if (m_textures.empty()) { // if there are no valid texture data
        m_curr_texture = INVALID_INDEX;
        m_curr_subtexture = INVALID_INDEX;
        m_curr_frame = INVALID_INDEX;
    } else if (idx == INVALID_INDEX) { // if idx is too low
        m_curr_texture = 0;
        m_curr_subtexture = 0;
        m_curr_frame = 0;
    } else if (m_frames <= idx) { // if idx is too high
        m_curr_texture = m_textures.size() - 1;
        m_curr_subtexture = m_textures[m_curr_texture].frames - 1;
        m_curr_frame = m_frames - 1;
    } else {
        // try to use O(1) Prev- and NextFrame() if we can
        if (idx == m_curr_frame + 1 && m_curr_frame < m_last_frame_idx) {
            NextFrame();
            m_playing = true;
        } else if (idx == m_curr_frame - 1 && m_first_frame_idx < m_curr_frame) {
            PrevFrame();
            m_playing = true;
        } else { // use O(n) linear search if necessary
            m_curr_frame = idx;
            if (idx == 0) {
                m_curr_texture = 0;
                m_curr_subtexture = 0;
            } else {
                m_curr_subtexture = INVALID_INDEX;
                for (m_curr_texture = 0; m_curr_texture < m_textures.size(); ++m_curr_texture) {
                    if (m_textures[m_curr_texture].frames <= idx) {
                        idx -= m_textures[m_curr_texture].frames;
                    } else {
                        m_curr_subtexture = idx;
                        break;
                    }
                }
                assert(m_curr_subtexture != INVALID_INDEX);
            }
        }
    }
}

void DynamicGraphic::SetTimeIndex(unsigned int time)
{
    std::size_t initial_frame_idx = 0.0 <= m_FPS ? m_first_frame_idx : m_last_frame_idx;
    std::size_t final_frame_idx = 0.0 <= m_FPS ? m_last_frame_idx : m_first_frame_idx;
    std::size_t frames_in_sequence = (m_last_frame_idx - m_first_frame_idx + 1);
    if (time == INVALID_TIME)
        SetFrameIndex(initial_frame_idx);
    else if (frames_in_sequence * m_FPS <= time && !m_looping)
        SetFrameIndex(final_frame_idx);
    else
        SetFrameIndex(initial_frame_idx + static_cast<unsigned int>(time * m_FPS / 1000.0) % frames_in_sequence);
}

void DynamicGraphic::SetStartFrame(std::size_t idx)
{
    if (idx == INVALID_INDEX)
        m_first_frame_idx = 0;
    else if (m_frames <= idx)
        m_first_frame_idx = m_frames - 1;
    else
        m_first_frame_idx = idx;

    if (m_curr_frame < m_first_frame_idx)
        SetFrameIndex(m_first_frame_idx);
}

void DynamicGraphic::SetEndFrame(std::size_t idx)
{
    if (idx == INVALID_INDEX)
        m_last_frame_idx = 0;
    else if (m_frames <= idx)
        m_last_frame_idx = m_frames - 1;
    else
        m_last_frame_idx = idx;

    if (m_last_frame_idx < m_curr_frame)
        SetFrameIndex(m_last_frame_idx);
}

void DynamicGraphic::SetStyle(Flags<GraphicStyle> style)
{
    m_style = style;
    ValidateStyle();
}

std::size_t DynamicGraphic::FramesInTexture(const Texture* t) const
{
    const int INT_MARGIN = m_margin;
    std::size_t cols = t->DefaultWidth() / (m_frame_width + INT_MARGIN);
    std::size_t rows = t->DefaultHeight() / (m_frame_height + INT_MARGIN);
    return cols * rows;
}

std::size_t DynamicGraphic::CurrentTexture() const
{ return m_curr_texture; }

std::size_t DynamicGraphic::CurrentSubTexture() const
{ return m_curr_texture; }

unsigned int DynamicGraphic::FirstFrameTime() const
{ return m_first_frame_time; }

unsigned int DynamicGraphic::LastFrameTime() const
{ return m_last_frame_time; }

void DynamicGraphic::ValidateStyle()
{
    int dup_ct = 0;   // duplication count
    if (m_style & GRAPHIC_LEFT) ++dup_ct;
    if (m_style & GRAPHIC_RIGHT) ++dup_ct;
    if (m_style & GRAPHIC_CENTER) ++dup_ct;
    if (dup_ct != 1) {   // exactly one must be picked; when none or multiples are picked, use GRAPHIC_CENTER by default
        m_style &= ~(GRAPHIC_RIGHT | GRAPHIC_LEFT);
        m_style |= GRAPHIC_CENTER;
    }
    dup_ct = 0;
    if (m_style & GRAPHIC_TOP) ++dup_ct;
    if (m_style & GRAPHIC_BOTTOM) ++dup_ct;
    if (m_style & GRAPHIC_VCENTER) ++dup_ct;
    if (dup_ct != 1) {   // exactly one must be picked; when none or multiples are picked, use GRAPHIC_VCENTER by default
        m_style &= ~(GRAPHIC_TOP | GRAPHIC_BOTTOM);
        m_style |= GRAPHIC_VCENTER;
    }
    dup_ct = 0;
    if (m_style & GRAPHIC_FITGRAPHIC) ++dup_ct;
    if (m_style & GRAPHIC_SHRINKFIT) ++dup_ct;
    if (dup_ct > 1) {   // mo more than one may be picked; when both are picked, use GRAPHIC_SHRINKFIT by default
        m_style &= ~GRAPHIC_FITGRAPHIC;
        m_style |= GRAPHIC_SHRINKFIT;
    }
}
