/* GG is a GUI for SDL and OpenGL.
   Copyright (C) 2003-2008 T. Zachary Laine

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
   
#include <GG/EveLayout.h>

#include <GG/DropDownList.h>
#include <GG/Edit.h>
#include <GG/GUI.h>
#include <GG/Layout.h>
#include <GG/Menu.h>
#include <GG/MultiEdit.h>
#include <GG/Slider.h>
#include <GG/Spin.h>
#include <GG/StyleFactory.h>
#include <GG/TabWnd.h>
#include <GG/ExpressionWriter.h>
#include <GG/adobe/adam.hpp>
#include <GG/adobe/basic_sheet.hpp>
#include <GG/adobe/dictionary.hpp>
#include <GG/adobe/eve_evaluate.hpp>
#include <GG/adobe/algorithm/sort.hpp>
#include <GG/adobe/future/widgets/headers/virtual_machine_extension.hpp>
#include <GG/adobe/future/widgets/headers/widget_tokens.hpp>

#include <boost/cast.hpp>
#include <boost/ptr_container/ptr_deque.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

#include <numeric>


using namespace GG;

#define INSTRUMENT_WINDOW_CREATION 0
#define INSTRUMENT_ADD_TO_VERTICAL 0
#define INSTRUMENT_CREATED_LAYOUT 0
#define SHOW_LAYOUTS 0

#if INSTRUMENT_WINDOW_CREATION
void verbose_dump(const adobe::array_t& array, std::size_t indent = 0);
void verbose_dump(const adobe::dictionary_t& array, std::size_t indent = 0);

void verbose_dump(const adobe::array_t& array, std::size_t indent)
{
    if (array.empty()) {
        std::cout << std::string(4 * indent, ' ') << "[]\n";
        return;
    }

    std::cout << std::string(4 * indent, ' ') << "[\n";
    ++indent;
    for (adobe::array_t::const_iterator it = array.begin(); it != array.end(); ++it) {
        const adobe::any_regular_t& any = *it;
        if (any.type_info() == adobe::type_info<adobe::array_t>()) {
            verbose_dump(any.cast<adobe::array_t>(), indent);
        } else if (any.type_info() == adobe::type_info<adobe::dictionary_t>()) {
            verbose_dump(any.cast<adobe::dictionary_t>(), indent);
        } else {
            std::cout << std::string(4 * indent, ' ')
                      << "type: " << any.type_info() << " "
                      << "value: " << any << "\n";
        }
    }
    --indent;
    std::cout << std::string(4 * indent, ' ') << "]\n";
}

void verbose_dump(const adobe::dictionary_t& dictionary, std::size_t indent)
{
    if (dictionary.empty()) {
        std::cout << std::string(4 * indent, ' ') << "{}\n";
        return;
    }

    std::cout << std::string(4 * indent, ' ') << "{\n";
    ++indent;
    for (adobe::dictionary_t::const_iterator it = dictionary.begin(); it != dictionary.end(); ++it) {
        const adobe::pair<adobe::name_t, adobe::any_regular_t>& pair = *it;
        if (pair.second.type_info() == adobe::type_info<adobe::array_t>()) {
            std::cout << std::string(4 * indent, ' ') << pair.first << ",\n";
            verbose_dump(pair.second.cast<adobe::array_t>(), indent);
        } else if (pair.second.type_info() == adobe::type_info<adobe::dictionary_t>()) {
            std::cout << std::string(4 * indent, ' ') << pair.first << ",\n";
            verbose_dump(pair.second.cast<adobe::dictionary_t>(), indent);
        } else {
            std::cout << std::string(4 * indent, ' ')
                      << "(" << pair.first << ", "
                      << "type: " << pair.second.type_info() << " "
                      << "value: " << pair.second << ")\n";
        }
    }
    --indent;
    std::cout << std::string(4 * indent, ' ') << "}\n";
}
#endif

namespace {

    StyleFactory& Factory()
    { return *GUI::GetGUI()->GetStyleFactory(); }

    boost::shared_ptr<Font> DefaultFont()
    { return Factory().DefaultFont(); }

    X CharWidth()
    { return DefaultFont()->TextExtent("W").x; }

    Y CharHeight()
    { return DefaultFont()->Lineskip(); }

    Y StandardHeight()
    { return CharHeight() * 3 / 2; }

    struct Panel :
        public Wnd
    {
        Panel(const std::string& name) :
            Wnd(X0, Y0, X1, Y1, Flags<WndFlag>()),
            m_name(name)
            {}

        const std::string& Name() const
            { return m_name; }

        std::string m_name;
    };

    // TODO: Put this into StyleFactory.
    struct Dialog :
        public Wnd
    {
        Dialog(const std::string& name, Flags<WndFlag> flags) :
            Wnd(X(10), Y(10), X(200), Y(200), flags | MODAL | DRAGABLE | INTERACTIVE),
            m_title(Factory().NewTextControl(BEVEL_OFFSET.x, BEVEL_OFFSET.y - CharHeight(), name, DefaultFont()))
            {
                AttachChild(m_title);
                const Pt MIN_SIZE(6 * CharWidth(), 2 * StandardHeight());
                SetMinSize(Pt(std::max(MIN_SIZE.x, m_title->Width() + 2 * CharWidth()), MIN_SIZE.y));
            }

        virtual Pt ClientUpperLeft() const
            { return UpperLeft() + BEVEL_OFFSET + Pt(X0, m_title->Height()); }

        virtual Pt ClientLowerRight() const
            { return LowerRight() - BEVEL_OFFSET; }

        virtual WndRegion WindowRegion(const Pt& pt) const
            {
                enum {LEFT = 0, MIDDLE = 1, RIGHT = 2};
                enum {TOP = 0, BOTTOM = 2};

                // window regions look like this:
                // 0111112
                // 3444445   // 4 is client area, 0,2,6,8 are corners
                // 3444445
                // 6777778

                int x_pos = MIDDLE;   // default & typical case is that the mouse is over the (non-border) client area
                int y_pos = MIDDLE;

                Pt ul = UpperLeft() + BEVEL_OFFSET, lr = LowerRight() - BEVEL_OFFSET;

                if (pt.x < ul.x)
                    x_pos = LEFT;
                else if (pt.x > lr.x)
                    x_pos = RIGHT;

                if (pt.y < ul.y)
                    y_pos = TOP;
                else if (pt.y > lr.y)
                    y_pos = BOTTOM;

                return (Resizable() ? WndRegion(x_pos + 3 * y_pos) : WR_NONE);
            }

        virtual void SizeMove(const Pt& ul, const Pt& lr)
            {
                Wnd::SizeMove(ul, lr);
                Pt new_title_size = Pt((LowerRight() - UpperLeft()).x - BEVEL_OFFSET.x * 2, m_title->Height());
                m_title->Resize(new_title_size);
            }

        virtual void Render()
            { BeveledRectangle(UpperLeft(), LowerRight(), CLR_GRAY, CLR_GRAY, true); }

        TextControl* m_title;

        static const int FRAME_WIDTH;
        static const Pt BEVEL_OFFSET;
    };

    const int Dialog::FRAME_WIDTH = 2;
    const Pt Dialog::BEVEL_OFFSET(X(Dialog::FRAME_WIDTH), Y(Dialog::FRAME_WIDTH));

    enum LabeledStatus {
        LABELED_CONTROL,
        UNLABELED_CONTROL
    };

    enum ContainerStatus {
        CONTAINER,
        NONCONTAINER
    };

    struct MakeWndResult
    {
        MakeWndResult(const adobe::dictionary_t& params,
                      const adobe::line_position_t& position,
                      LabeledStatus labeled_status,
                      ContainerStatus container_status) :
            m_horizontal(),
            m_vertical(),
            m_child_horizontal(),
            m_child_vertical(),
            m_default_horizontal(),
            m_default_vertical(),
            m_default_child_horizontal(),
            m_default_child_vertical(),
            m_spacing(0),
            m_indent(0),
            m_margin(5),
            m_labeled_status(labeled_status),
            m_container_status(container_status)
            { Init(params, position); }

        MakeWndResult(const adobe::dictionary_t& params,
                      const adobe::line_position_t& position,
                      adobe::name_t default_horizontal,
                      adobe::name_t default_vertical,
                      adobe::name_t default_child_horizontal,
                      adobe::name_t default_child_vertical,
                      LabeledStatus labeled_status,
                      ContainerStatus container_status) :
            m_horizontal(),
            m_vertical(),
            m_child_horizontal(),
            m_child_vertical(),
            m_default_horizontal(default_horizontal),
            m_default_vertical(default_vertical),
            m_default_child_horizontal(default_child_horizontal),
            m_default_child_vertical(default_child_vertical),
            m_spacing(0),
            m_indent(0),
            m_margin(5),
            m_labeled_status(labeled_status),
            m_container_status(container_status)
            { Init(params, position); }

        void Init(const adobe::dictionary_t& params,
                  const adobe::line_position_t& position)
            {
                get_value(params, adobe::key_horizontal, m_horizontal);
                CheckAlignment(adobe::key_horizontal, m_horizontal, position);
                get_value(params, adobe::key_vertical, m_vertical);
                CheckAlignment(adobe::key_vertical, m_vertical, position);
                bool has_child_horz = get_value(params, adobe::key_child_horizontal, m_child_horizontal);
                CheckAlignment(adobe::key_child_horizontal, m_child_horizontal, position);
                bool has_child_vert = get_value(params, adobe::key_child_vertical, m_child_vertical);
                CheckAlignment(adobe::key_child_vertical, m_child_vertical, position);
                get_value(params, adobe::key_spacing, m_spacing);
                get_value(params, adobe::key_indent, m_indent);
                get_value(params, adobe::key_margin, m_margin);

                if (m_container_status == NONCONTAINER && has_child_horz) {
                    throw adobe::stream_error_t(
                        adobe::key_child_horizontal.name_m +
                        std::string(" is not compatible with non-containers"),
                        position
                    );
                }

                if (m_container_status == NONCONTAINER && has_child_vert) {
                    throw adobe::stream_error_t(
                        adobe::key_child_vertical.name_m +
                        std::string(" is not compatible with non-containers"),
                        position
                    );
                }
            }

        void CheckAlignment(adobe::name_t key_name,
                            adobe::name_t alignment_name,
                            const adobe::line_position_t& position)
            {
                if (alignment_name &&
                    alignment_name != adobe::key_align_fill &&
                    alignment_name != adobe::key_align_top &&
                    alignment_name != adobe::key_align_bottom &&
                    alignment_name != adobe::key_align_left &&
                    alignment_name != adobe::key_align_right &&
                    alignment_name != adobe::key_align_center &&
                    alignment_name != adobe::key_align_proportional) {
                    throw adobe::stream_error_t(
                        alignment_name.c_str() +
                        std::string(" is not an allowed alignment for alignment type ") +
                        key_name.c_str(),
                        position
                    );
                }
            }

        adobe::name_t m_horizontal;
        adobe::name_t m_vertical;
        adobe::name_t m_child_horizontal;
        adobe::name_t m_child_vertical;
        adobe::name_t m_default_horizontal;
        adobe::name_t m_default_vertical;
        adobe::name_t m_default_child_horizontal;
        adobe::name_t m_default_child_vertical;
        int m_spacing;
        int m_indent;
        int m_margin;
        std::auto_ptr<Wnd> m_wnd;
        LabeledStatus m_labeled_status;
        ContainerStatus m_container_status;
    };

    Flags<Alignment> AlignmentFlags(adobe::name_t horizontal, adobe::name_t vertical)
    {
        Flags<Alignment> retval;

        if (vertical == adobe::key_align_top)
            retval |= ALIGN_TOP;
        else if (vertical == adobe::key_align_center)
            retval |= ALIGN_VCENTER;
        else if (vertical == adobe::key_align_bottom)
            retval |= ALIGN_BOTTOM;
        // TODO: else fill and else proportional

        if (horizontal == adobe::key_align_left)
            retval |= ALIGN_LEFT;
        else if (horizontal == adobe::key_align_center)
            retval |= ALIGN_CENTER;
        else if (horizontal == adobe::key_align_right)
            retval |= ALIGN_RIGHT;
        // TODO: else fill and else proportional

        return retval;
    }

    void ConditionallyApplyAlignments(adobe::name_t& existing_horizontal,
                                      adobe::name_t& existing_vertical,
                                      adobe::name_t horizontal,
                                      adobe::name_t vertical)
    {
        if (!existing_horizontal)
            existing_horizontal = horizontal;
        if (!existing_vertical)
            existing_vertical = vertical;
    }

    std::pair<adobe::name_t, adobe::name_t> Alignments(const MakeWndResult& parent, const MakeWndResult& child)
    {
        std::pair<adobe::name_t, adobe::name_t> retval(child.m_horizontal, child.m_vertical);
        ConditionallyApplyAlignments(retval.first,
                                     retval.second,
                                     parent.m_child_horizontal,
                                     parent.m_child_vertical);
        ConditionallyApplyAlignments(retval.first,
                                     retval.second,
                                     child.m_default_horizontal,
                                     child.m_default_vertical);
        ConditionallyApplyAlignments(retval.first,
                                     retval.second,
                                     parent.m_default_child_horizontal,
                                     parent.m_default_child_vertical);
        return retval;
    }

    Flags<Alignment> AlignmentFlags(const MakeWndResult& parent, const MakeWndResult& child)
    {
        std::pair<adobe::name_t, adobe::name_t> alignments = Alignments(parent, child);
        return AlignmentFlags(alignments.first, alignments.second);
    }

#if INSTRUMENT_CREATED_LAYOUT
    std::string WndTypeStr(Wnd* w)
    {
#define CASE(x) if (dynamic_cast<x*>(w)) return #x;

        using boost::lexical_cast;

        CASE(Dialog)
        else CASE(Panel)
        else CASE(OverlayWnd)
        else CASE(Button)
        else CASE(StateButton)
        else CASE(TextControl)
        else CASE(Edit)
        else CASE(StaticGraphic)
        else CASE(DropDownList)
        else CASE(RadioButtonGroup)
        else CASE(Slider)
        else CASE(TabBar)
        else CASE(MenuBar)
        else CASE(Spin<int>)
        else CASE(Spin<double>)
        else if (Layout* l = dynamic_cast<Layout*>(w)) {
            return
                lexical_cast<std::string>(l->Rows()) + "x" +
                lexical_cast<std::string>(l->Columns()) +
                " Layout";
        } else {
            return "Unknown";
        }

#undef CASE
    }

    void DumpLayout(Wnd* w, int indent = 0)
    {
        std::cout << std::string(indent * 4, ' ') << WndTypeStr(w) << " "
                  << w->RelativeUpperLeft() << " - "
                  << w->RelativeLowerRight()
                  << "\n";
        for (std::list<Wnd*>::const_iterator it = w->Children().begin(); it != w->Children().end(); ++it) {
            DumpLayout(*it, indent + 1);
        }
    }
#endif

    MakeWndResult* Make_dialog(const adobe::dictionary_t& params, const adobe::line_position_t& position)
    {
        adobe::string_t name;
        bool grow = false;

        get_value(params, adobe::key_name, name);
        get_value(params, adobe::key_grow, grow);

        std::auto_ptr<MakeWndResult> retval(new MakeWndResult(params, position, UNLABELED_CONTROL, CONTAINER));

        retval->m_wnd.reset(new Dialog(name, grow ? RESIZABLE : Flags<WndFlag>()));

        return retval.release();
    }

    MakeWndResult* Make_button(const adobe::dictionary_t& params, const adobe::line_position_t& position)
    {
        adobe::string_t name;
        bool default_ = false;
        bool cancel = false;
        adobe::name_t bind;
        adobe::string_t alt;
        adobe::name_t action;
        adobe::any_regular_t value;
        adobe::name_t bind_output;

        get_value(params, adobe::key_name, name);
        get_value(params, adobe::key_default, default_);
        get_value(params, adobe::key_cancel, cancel);
        get_value(params, adobe::key_bind, bind);
        get_value(params, adobe::key_alt_text, alt);
        get_value(params, adobe::key_action, action);
        get_value(params, adobe::key_value, value);
        get_value(params, adobe::key_bind_output, bind_output);

        // TODO bind_view ?
        // TODO bind_controller ?
        // skipping items
        // skipping modifiers

        std::auto_ptr<MakeWndResult> retval(new MakeWndResult(params,
                                                              position,
                                                              adobe::key_align_center,
                                                              adobe::name_t(),
                                                              adobe::name_t(),
                                                              adobe::name_t(),
                                                              UNLABELED_CONTROL,
                                                              NONCONTAINER));

        std::auto_ptr<Button> button(
            Factory().NewButton(X0, Y0, X1, StandardHeight(), name, DefaultFont(), CLR_GRAY)
        );
        button->SetMaxSize(Pt(button->MaxSize().x, button->Height()));
        button->SetMinSize(Pt(DefaultFont()->TextExtent(name).x + 2 * CharWidth(), button->Height()));
        retval->m_wnd = button;

        return retval.release();
    }

    MakeWndResult* Make_checkbox(const adobe::dictionary_t& params, const adobe::line_position_t& position)
    {
        adobe::string_t name;
        adobe::name_t bind;
        adobe::string_t alt;
        adobe::any_regular_t value_on;
        adobe::any_regular_t value_off;

        get_value(params, adobe::key_name, name);
        get_value(params, adobe::key_bind, bind);
        get_value(params, adobe::key_alt_text, alt);
        get_value(params, adobe::key_value, value_on);
        get_value(params, adobe::key_value, value_off);

        // TODO bind_view ?
        // TODO bind_controller ?

        std::auto_ptr<MakeWndResult> retval(new MakeWndResult(params,
                                                              position,
                                                              UNLABELED_CONTROL,
                                                              NONCONTAINER));

        std::auto_ptr<StateButton> checkbox(
            Factory().NewStateButton(X0, Y0, X1, StandardHeight(), name, DefaultFont(), FORMAT_LEFT, CLR_GRAY)
        );
        checkbox->SetMaxSize(Pt(checkbox->MaxSize().x, checkbox->Height()));
        checkbox->SetMinSize(Pt(checkbox->MinSize().x, checkbox->Height()));
        retval->m_wnd = checkbox;

        return retval.release();
    }

    MakeWndResult* Make_display_number(const adobe::dictionary_t& params,
                                       const adobe::line_position_t& position)
    {
        adobe::string_t name;
        adobe::name_t bind;
        adobe::string_t alt;
        std::size_t characters;
        adobe::array_t units;
        adobe::string_t format;

        get_value(params, adobe::key_name, name);
        get_value(params, adobe::key_bind, bind);
        get_value(params, adobe::key_alt_text, alt);
        get_value(params, adobe::key_characters, characters);
        get_value(params, adobe::key_units, units);
        get_value(params, adobe::key_format, format);

        // TODO bind_view ?
        // TODO bind_controller ?

        std::auto_ptr<MakeWndResult> retval(new MakeWndResult(params,
                                                              position,
                                                              LABELED_CONTROL,
                                                              NONCONTAINER));

        std::auto_ptr<TextControl> text_control(
            Factory().NewTextControl(X0, Y0, X1, StandardHeight(), "", DefaultFont())
        );
        text_control->SetMaxSize(Pt(text_control->MaxSize().x, text_control->Height()));
        text_control->SetMinSize(Pt(text_control->MinSize().x, text_control->Height()));

        if (!name.empty()) {
            std::auto_ptr<Layout> layout(new Layout(X0, Y0, X1, Y1, 1, 2, retval->m_margin, retval->m_margin));
#if SHOW_LAYOUTS
            layout->RenderOutline(true);
#endif
            std::auto_ptr<TextControl> label(Factory().NewTextControl(X0, Y0, name, DefaultFont()));
            label->SetMinSize(Pt(label->Width(), label->MinSize().y));
            layout->Add(label.release(), 0, 0, 1, 1, ALIGN_RIGHT);
            layout->Add(text_control.release(), 0, 1);
            retval->m_wnd = layout;
        } else {
            retval->m_wnd = text_control;
        }

        return retval.release();
    }

    MakeWndResult* Make_edit_number(const adobe::dictionary_t& params,
                                    const adobe::line_position_t& position)
    {
        adobe::string_t name;
        adobe::name_t bind;
        adobe::string_t alt;
        std::size_t digits = 5;
        adobe::string_t format;
        double min_value;
        double max_value;

        get_value(params, adobe::key_name, name);
        get_value(params, adobe::key_bind, bind);
        get_value(params, adobe::key_alt_text, alt);
        get_value(params, adobe::key_digits, digits);
        get_value(params, adobe::key_format, format);
        get_value(params, adobe::key_min_value, min_value);
        get_value(params, adobe::key_max_value, max_value);

        // TODO bind_view ?
        // TODO bind_controller ?
        // TODO units ?
        // TODO bind_units ?
        // TODO bind_group ?
        // TODO touch ?
        // skipping max_digits

        std::auto_ptr<MakeWndResult> retval(new MakeWndResult(params,
                                                              position,
                                                              LABELED_CONTROL,
                                                              NONCONTAINER));

        std::auto_ptr<Edit> edit(Factory().NewEdit(X0, Y0, X1, "", DefaultFont(), CLR_GRAY));
        edit->SetMaxSize(Pt(edit->MaxSize().x, edit->Height()));
        edit->SetMinSize(Pt(static_cast<int>(digits) * CharWidth(), edit->Height()));

        if (!name.empty()) {
            std::auto_ptr<Layout> layout(new Layout(X0, Y0, X1, Y1, 1, 2, retval->m_margin, retval->m_margin));
#if SHOW_LAYOUTS
            layout->RenderOutline(true);
#endif
            std::auto_ptr<TextControl> label(Factory().NewTextControl(X0, Y0, name, DefaultFont()));
            label->SetMinSize(Pt(label->Width(), label->MinSize().y));
            layout->Add(label.release(), 0, 0, 1, 1, ALIGN_RIGHT);
            layout->Add(edit.release(), 0, 1);
            retval->m_wnd = layout;
        } else {
            retval->m_wnd = edit;
        }

        return retval.release();
    }

    MakeWndResult* Make_edit_text(const adobe::dictionary_t& params,
                                  const adobe::line_position_t& position)
    {
        adobe::string_t name;
        adobe::name_t bind;
        adobe::string_t alt;
        std::size_t characters = 8;
        std::size_t lines = 1u;
        bool scrollable = true;

        get_value(params, adobe::key_name, name);
        get_value(params, adobe::key_bind, bind);
        get_value(params, adobe::key_alt_text, alt);
        get_value(params, adobe::key_characters, characters);
        get_value(params, adobe::key_lines, lines);
        get_value(params, adobe::key_scrollable, scrollable);

        // TODO bind_view ?
        // TODO bind_controller ?
        // TODO password ?
        // skipping max_characters
        // skipping monospaced

        std::auto_ptr<MakeWndResult> retval(new MakeWndResult(params,
                                                              position,
                                                              LABELED_CONTROL,
                                                              NONCONTAINER));

        std::auto_ptr<Wnd> edit;
        X width = static_cast<int>(characters) * CharWidth();
        if (lines <= 1u) {
            std::auto_ptr<Edit> one_line_edit(
                Factory().NewEdit(X0, Y0, width, "", DefaultFont(), CLR_GRAY)
            );
            one_line_edit->SetMaxSize(Pt(one_line_edit->MaxSize().x, one_line_edit->Height()));
            one_line_edit->SetMinSize(Pt(width, one_line_edit->Height()));
            edit = one_line_edit;
        } else {
            Y height = CharHeight() * static_cast<int>(lines);
            Flags<MultiEditStyle> style = MULTI_LINEWRAP;
            if (!scrollable)
                style |= MULTI_NO_SCROLL;
            std::auto_ptr<MultiEdit> multiline_edit(
                Factory().NewMultiEdit(X0, Y0, width, height, "", DefaultFont(), CLR_GRAY, style)
            );
            multiline_edit->SetMaxSize(Pt(multiline_edit->MaxSize().x, multiline_edit->Height()));
            multiline_edit->SetMinSize(Pt(width, multiline_edit->Height()));
            edit = multiline_edit;
        }

        if (!name.empty()) {
            std::auto_ptr<Layout> layout(new Layout(X0, Y0, X1, Y1, 1, 2, retval->m_margin, retval->m_margin));
#if SHOW_LAYOUTS
            layout->RenderOutline(true);
#endif
            std::auto_ptr<TextControl> label(Factory().NewTextControl(X0, Y0, name, DefaultFont()));
            label->SetMinSize(Pt(label->Width(), label->MinSize().y));
            layout->Add(label.release(), 0, 0, 1, 1, ALIGN_RIGHT);
            layout->Add(edit.release(), 0, 1);
            retval->m_wnd = layout;
        } else {
            retval->m_wnd = edit;
        }

        return retval.release();
    }

    MakeWndResult* Make_image(const adobe::dictionary_t& params, const adobe::line_position_t& position)
    {
        adobe::string_t image;
        adobe::name_t bind;

        get_value(params, adobe::key_image, image);
        get_value(params, adobe::key_bind, bind);

        // TODO bind_view ?
        // TODO bind_controller ?
        // TODO: grow

        std::auto_ptr<MakeWndResult> retval(new MakeWndResult(params,
                                                              position,
                                                              adobe::key_align_center,
                                                              adobe::key_align_center,
                                                              adobe::name_t(),
                                                              adobe::name_t(),
                                                              UNLABELED_CONTROL,
                                                              NONCONTAINER));

        boost::shared_ptr<Texture> texture = GG::GUI::GetGUI()->GetTexture(image);
        retval->m_wnd.reset(Factory().NewStaticGraphic(X0, Y0,
                                                       texture->DefaultWidth(), texture->DefaultHeight(),
                                                       texture));

        return retval.release();
    }

    MakeWndResult* Make_popup(const adobe::dictionary_t& params, const adobe::line_position_t& position)
    {
        adobe::string_t name;
        adobe::name_t bind;
        adobe::string_t alt;
        adobe::array_t items;

        get_value(params, adobe::key_name, name);
        get_value(params, adobe::key_bind, bind);
        get_value(params, adobe::key_alt_text, alt);
        get_value(params, adobe::key_items, items);

        // TODO bind_view ?
        // TODO bind_controller ?
        // skipping custom_item_name
        // skipping popup_bind
        // skipping popup_placement

        std::auto_ptr<MakeWndResult> retval(new MakeWndResult(params,
                                                              position,
                                                              LABELED_CONTROL,
                                                              NONCONTAINER));

        const std::size_t MAX_LINES = 10;
        Y drop_height = CharHeight() * static_cast<int>(std::min(items.size(), MAX_LINES));
        std::auto_ptr<DropDownList> drop_down_list(
            Factory().NewDropDownList(X0, Y0, X1, StandardHeight(), drop_height, CLR_GRAY)
        );

        X max_item_width = X0;
        boost::ptr_deque<ListBox::Row> rows;
        for (adobe::array_t::const_iterator it = items.begin(); it != items.end(); ++it) {
            try {
                const adobe::dictionary_t& name_value_pair = it->cast<adobe::dictionary_t>();
                adobe::string_t name;
                adobe::any_regular_t value;
                if (!get_value(name_value_pair, adobe::key_name, name))
                    throw adobe::stream_error_t("popup items must each contain a string name", position);
                if (!get_value(name_value_pair, adobe::key_value, value))
                    throw adobe::stream_error_t("popup items must each contain a value", position);
                std::auto_ptr<ListBox::Row> row(new ListBox::Row);
                row->push_back(Factory().NewTextControl(X0, Y0, name, DefaultFont()));
                max_item_width = std::max(max_item_width, DefaultFont()->TextExtent(name).x);
                rows.push_back(row);
            } catch (const adobe::bad_cast&) {
                throw adobe::stream_error_t("popup items must each be a dictionary", position);
            }
        }

        max_item_width += CharWidth() * 2;
        drop_down_list->SetMaxSize(Pt(drop_down_list->MaxSize().x, drop_down_list->Height()));
        drop_down_list->SetMinSize(Pt(max_item_width, drop_down_list->Height()));

        while (!rows.empty()) {
            drop_down_list->Insert(rows.release(rows.begin()).release());
        }

        if (!name.empty()) {
            std::auto_ptr<Layout> layout(new Layout(X0, Y0, X1, Y1, 1, 2, retval->m_margin, retval->m_margin));
#if SHOW_LAYOUTS
            layout->RenderOutline(true);
#endif
            std::auto_ptr<TextControl> label(Factory().NewTextControl(X0, Y0, name, DefaultFont()));
            label->SetMinSize(Pt(label->Width(), label->MinSize().y));
            layout->Add(label.release(), 0, 0, 1, 1, ALIGN_RIGHT);
            layout->Add(drop_down_list.release(), 0, 1);
            retval->m_wnd = layout;
        } else {
            retval->m_wnd = drop_down_list;
        }

        return retval.release();
    }

    MakeWndResult* Make_radio_button(const adobe::dictionary_t& params,
                                     const adobe::line_position_t& position)
    {
        adobe::string_t name;
        adobe::name_t bind;
        adobe::string_t alt;
        adobe::any_regular_t value;

        get_value(params, adobe::key_name, name);
        get_value(params, adobe::key_bind, bind);
        get_value(params, adobe::key_alt_text, alt);
        get_value(params, adobe::key_value, value);

        // TODO bind_view ?
        // TODO bind_controller ?
        // TODO touch ?

        std::auto_ptr<MakeWndResult> retval(new MakeWndResult(params,
                                                              position,
                                                              UNLABELED_CONTROL,
                                                              CONTAINER));

        std::auto_ptr<StateButton> radio_button(
            Factory().NewStateButton(X0, Y0, X1, StandardHeight(), name, DefaultFont(), FORMAT_LEFT,
                                     CLR_GRAY, CLR_BLACK, CLR_ZERO, SBSTYLE_3D_RADIO)
        );
        radio_button->SetMaxSize(Pt(radio_button->MaxSize().x, radio_button->Height()));
        radio_button->SetMinSize(Pt(radio_button->MinSize().x, radio_button->Height()));
        retval->m_wnd = radio_button;

        return retval.release();
    }

    MakeWndResult* Make_radio_button_group(const adobe::dictionary_t& params,
                                           const adobe::line_position_t& position)
    {
        adobe::name_t placement = adobe::key_place_column;

        get_value(params, adobe::key_placement, placement);

        if (placement == adobe::key_place_overlay) {
            throw adobe::stream_error_t(
                "place_overlay placement is not compatible with radio_button_group",
                position
            );
        }

        std::auto_ptr<MakeWndResult> retval(new MakeWndResult(params,
                                                              position,
                                                              UNLABELED_CONTROL,
                                                              NONCONTAINER));

        Orientation orientation = placement == adobe::key_place_column ? VERTICAL : HORIZONTAL;
        retval->m_wnd.reset(Factory().NewRadioButtonGroup(X0, Y0, X1, Y1, orientation));

        return retval.release();
    }

    MakeWndResult* Make_slider(const adobe::dictionary_t& params, const adobe::line_position_t& position)
    {
        adobe::name_t bind;
        adobe::string_t alt;
        adobe::dictionary_t format;
        adobe::name_t orientation = adobe::key_horizontal;
        double slider_ticks;

        get_value(params, adobe::key_bind, bind);
        get_value(params, adobe::key_alt_text, alt);
        get_value(params, adobe::key_format, format);
        get_value(params, adobe::key_orientation, orientation);
        get_value(params, adobe::key_slider_ticks, slider_ticks);

        // skipping slider_point

        Orientation orientation_ = orientation == adobe::key_vertical ? VERTICAL : HORIZONTAL;

        std::auto_ptr<MakeWndResult> retval(
            new MakeWndResult(params,
                              position,
                              orientation_ == VERTICAL ? adobe::key_align_center : adobe::name_t(),
                              orientation_ == VERTICAL ? adobe::name_t() : adobe::key_align_center,
                              adobe::name_t(),
                              adobe::name_t(),
                              UNLABELED_CONTROL,
                              NONCONTAINER)
        );

        double min = 1;
        double max = 100;
        get_value(format, adobe::key_first, min);
        get_value(format, adobe::key_last, max);
        const int TAB_WIDTH = 5;
        std::auto_ptr<Slider<double> > slider(
            Factory().NewDoubleSlider(X0, Y0,
                                      orientation_ == VERTICAL ? X(Value(StandardHeight())) : X(200),
                                      orientation_ == VERTICAL ? Y(200) : StandardHeight(),
                                      min, max, orientation_, GROOVED, CLR_GRAY, TAB_WIDTH)
        );
        slider->SetMaxSize(
            Pt(orientation_ == VERTICAL ? slider->Width() : slider->MaxSize().x,
               orientation_ == VERTICAL ? slider->MaxSize().y : slider->Height())
        );
        slider->SetMinSize(
            Pt(orientation_ == VERTICAL ? slider->Width() : slider->MinSize().x,
               orientation_ == VERTICAL ? slider->MinSize().y : slider->Height())
        );
        retval->m_wnd = slider;

        return retval.release();
    }

    MakeWndResult* Make_tab_group(const adobe::dictionary_t& params,
                                  const adobe::line_position_t& position)
    {
        adobe::string_t name; // TODO: is this useful?
        adobe::name_t bind;
        adobe::any_regular_t value;

        get_value(params, adobe::key_name, name);
        get_value(params, adobe::key_bind, bind);
        get_value(params, adobe::key_value, value);

        // TODO bind_view ?
        // TODO bind_controller ?

        std::auto_ptr<MakeWndResult> retval(new MakeWndResult(params, position, UNLABELED_CONTROL, CONTAINER));

        retval->m_wnd.reset(Factory().NewTabWnd(X0, Y0, X1, Y1, DefaultFont(), CLR_GRAY));

        return retval.release();
    }

    MakeWndResult* Make_static_text(const adobe::dictionary_t& params,
                                    const adobe::line_position_t& position)
    {
        adobe::string_t name;
        adobe::string_t alt;
        std::size_t characters = 25;
        bool wrap = true;
        adobe::name_t text_horizontal = adobe::key_align_left;
        adobe::name_t text_vertical;

        get_value(params, adobe::key_name, name);
        get_value(params, adobe::key_alt_text, alt);
        get_value(params, adobe::key_characters, characters);
        get_value(params, adobe::key_wrap, wrap);
        get_value(params, adobe::key_text_horizontal, text_horizontal);
        get_value(params, adobe::key_text_vertical, text_vertical);

        std::auto_ptr<MakeWndResult> retval(new MakeWndResult(params,
                                                              position,
                                                              UNLABELED_CONTROL,
                                                              NONCONTAINER));

        X w = static_cast<int>(characters) * CharWidth();

        Flags<TextFormat> format;

        if (wrap)
            format |= FORMAT_WORDBREAK;

        if (text_horizontal == adobe::key_align_left)
            format |= FORMAT_LEFT;
        else if (text_horizontal == adobe::key_align_center)
            format |= FORMAT_CENTER;
        else if (text_horizontal == adobe::key_align_right)
            format |= FORMAT_RIGHT;

        if (text_vertical == adobe::key_align_top)
            format |= FORMAT_TOP;
        else if (text_vertical == adobe::key_align_center)
            format |= FORMAT_VCENTER;
        else if (text_vertical == adobe::key_align_bottom)
            format |= FORMAT_BOTTOM;

        Pt extent = DefaultFont()->TextExtent(name, format, w);
        retval->m_wnd.reset(
            Factory().NewTextControl(X0, Y0, extent.x, extent.y, name, DefaultFont(), CLR_BLACK, format)
        );
        retval->m_wnd->SetMinSize(Pt(retval->m_wnd->Width(), retval->m_wnd->MinSize().y));

        return retval.release();
    }

    MakeWndResult* Make_menu_bar(const adobe::dictionary_t& params, const adobe::line_position_t& position)
    {
        // TODO

        std::auto_ptr<MakeWndResult> retval(new MakeWndResult(params,
                                                              position,
                                                              UNLABELED_CONTROL,
                                                              NONCONTAINER));

        return retval.release();
    }

    MakeWndResult* Make_int_spin(const adobe::dictionary_t& params, const adobe::line_position_t& position)
    {
        adobe::string_t name;
        adobe::name_t bind;
        adobe::string_t alt;
        std::size_t digits = 5;
        adobe::dictionary_t format;

        get_value(params, adobe::key_name, name);
        get_value(params, adobe::key_bind, bind);
        get_value(params, adobe::key_alt_text, alt);
        get_value(params, adobe::key_digits, digits);
        get_value(params, adobe::key_format, format);

        // TODO bind_view ?
        // TODO bind_controller ?
        // TODO units ?
        // TODO bind_units ?
        // TODO bind_group ?
        // TODO touch ?
        // skipping max_digits

        std::auto_ptr<MakeWndResult> retval(new MakeWndResult(params,
                                                              position,
                                                              LABELED_CONTROL,
                                                              NONCONTAINER));

        int step = 1;
        int min = 1;
        int max = 100;
        bool allow_edits = false;
        get_value(format, adobe::key_step, step);
        get_value(format, adobe::key_first, min);
        get_value(format, adobe::key_last, max);
        get_value(format, adobe::key_allow_edits, allow_edits);

        std::auto_ptr<Spin<int> > spin(
            Factory().NewIntSpin(X0, Y0, X1, 0, step, min, max, allow_edits, DefaultFont(), CLR_GRAY)
        );
        spin->SetMaxSize(Pt(spin->MaxSize().x, spin->Height()));
        spin->SetMinSize(Pt(static_cast<int>(digits) * CharWidth(), spin->Height()));

        if (!name.empty()) {
            std::auto_ptr<Layout> layout(new Layout(X0, Y0, X1, Y1, 1, 2, retval->m_margin, retval->m_margin));
#if SHOW_LAYOUTS
            layout->RenderOutline(true);
#endif
            std::auto_ptr<TextControl> label(Factory().NewTextControl(X0, Y0, name, DefaultFont()));
            label->SetMinSize(Pt(label->Width(), label->MinSize().y));
            layout->Add(label.release(), 0, 0, 1, 1, ALIGN_RIGHT);
            layout->Add(spin.release(), 0, 1);
            retval->m_wnd = layout;
        } else {
            retval->m_wnd = spin;
        }

        return retval.release();
    }

    MakeWndResult* Make_double_spin(const adobe::dictionary_t& params,
                                    const adobe::line_position_t& position)
    {
        adobe::string_t name;
        adobe::name_t bind;
        adobe::string_t alt;
        std::size_t digits = 5;
        adobe::dictionary_t format;

        get_value(params, adobe::key_name, name);
        get_value(params, adobe::key_bind, bind);
        get_value(params, adobe::key_alt_text, alt);
        get_value(params, adobe::key_digits, digits);
        get_value(params, adobe::key_format, format);

        // TODO bind_view ?
        // TODO bind_controller ?
        // TODO units ?
        // TODO bind_units ?
        // TODO bind_group ?
        // TODO touch ?
        // skipping max_digits

        std::auto_ptr<MakeWndResult> retval(new MakeWndResult(params,
                                                              position,
                                                              LABELED_CONTROL,
                                                              NONCONTAINER));

        double step_ = 1.0;
        double min_ = 1.0;
        double max_ = 100.0;
        bool allow_edits = false;
        get_value(format, adobe::key_step, step_);
        get_value(format, adobe::key_first, min_);
        get_value(format, adobe::key_last, max_);
        get_value(format, adobe::key_allow_edits, allow_edits);
        int step = static_cast<int>(step_);
        int min = static_cast<int>(min_);
        int max = static_cast<int>(max_);
        std::auto_ptr<Spin<int> > spin(
            Factory().NewIntSpin(X0, Y0, X1, 0, step, min, max, allow_edits, DefaultFont(), CLR_GRAY)
        );
        spin->SetMaxSize(Pt(spin->MaxSize().x, spin->Height()));
        spin->SetMinSize(Pt(static_cast<int>(digits) * CharWidth(), spin->Height()));

        if (!name.empty()) {
            std::auto_ptr<Layout> layout(new Layout(X0, Y0, X1, Y1, 1, 2, retval->m_margin, retval->m_margin));
#if SHOW_LAYOUTS
            layout->RenderOutline(true);
#endif
            std::auto_ptr<TextControl> label(Factory().NewTextControl(X0, Y0, name, DefaultFont()));
            label->SetMinSize(Pt(label->Width(), label->MinSize().y));
            layout->Add(label.release(), 0, 0, 1, 1, ALIGN_RIGHT);
            layout->Add(spin.release(), 0, 1);
            retval->m_wnd = layout;
        } else {
            retval->m_wnd = spin;
        }

        return retval.release();
    }

    MakeWndResult* Make_overlay(const adobe::dictionary_t& params, const adobe::line_position_t& position)
    {
        adobe::name_t placement = adobe::key_place_overlay;

        get_value(params, adobe::key_placement, placement);

        if (placement == adobe::key_place_row || placement == adobe::key_place_column) {
            std::string placement_ = adobe::key_place_overlay.name_m;
            throw adobe::stream_error_t(placement_ + " placement is not compatible with overlay",
                                        position);
        }

        std::auto_ptr<MakeWndResult> retval(new MakeWndResult(params, position, UNLABELED_CONTROL, CONTAINER));

        retval->m_wnd.reset(new OverlayWnd(X0, Y0, X1, Y1));

        return retval.release();
    }

    MakeWndResult* Make_panel(const adobe::dictionary_t& params, const adobe::line_position_t& position)
    {
        adobe::string_t name;
        adobe::name_t value;
        adobe::name_t bind;

        get_value(params, adobe::key_name, name);
        get_value(params, adobe::key_value, value);
        get_value(params, adobe::key_bind, bind);

        std::auto_ptr<MakeWndResult> retval(new MakeWndResult(params, position, UNLABELED_CONTROL, CONTAINER));

        retval->m_wnd.reset(new Panel(name));

        return retval.release();
    }

    MakeWndResult* Make_layout(adobe::name_t wnd_type,
                               const adobe::dictionary_t& params,
                               const adobe::line_position_t& position)
    {
        adobe::name_t placement = adobe::key_place_overlay;

        if (get_value(params, adobe::key_placement, placement)) {
            if (!(placement == adobe::key_place_row && wnd_type == adobe::name_row ||
                  placement == adobe::key_place_column && wnd_type == adobe::name_column)) {
                std::string placement_ = placement.c_str();
                throw adobe::stream_error_t(
                    placement_ + " placement is not compatible with " + wnd_type.c_str(),
                    position
                );
            }
        }

        std::auto_ptr<MakeWndResult> retval;
        if (wnd_type == adobe::name_column) {
            retval.reset(new MakeWndResult(params,
                                           position,
                                           adobe::name_t(),
                                           adobe::name_t(),
                                           adobe::name_t(),
                                           adobe::key_align_top,
                                           UNLABELED_CONTROL,
                                           CONTAINER));
        } else {
            retval.reset(new MakeWndResult(params,
                                           position,
                                           adobe::name_t(),
                                           adobe::name_t(),
                                           adobe::key_align_left,
                                           adobe::name_t(),
                                           UNLABELED_CONTROL,
                                           CONTAINER));
        }

        std::auto_ptr<Layout> layout(new Layout(X0, Y0, X1, Y1, 1, 1, retval->m_margin, retval->m_margin));

#if SHOW_LAYOUTS
        layout->RenderOutline(true);
#endif
        retval->m_wnd.reset(layout.release());

        return retval.release();
    }

#if INSTRUMENT_WINDOW_CREATION
    int indent = 0;
#endif

    MakeWndResult* MakeWnd(adobe::name_t wnd_type,
                           const adobe::dictionary_t& params,
                           const adobe::line_position_t& position)
    {
        using namespace adobe;

#if INSTRUMENT_WINDOW_CREATION
#define IF_CASE(x) if (wnd_type == name_##x)                            \
        {                                                               \
            std::cout << std::string(indent * 4, ' ') << "    Make_"#x"(" << wnd_type << ")\n"; \
            MakeWndResult* retval = Make_##x(params, position);         \
            std::cout << std::string(indent * 4, ' ') << "    " << retval->m_wnd.get() << " " << "\n"; \
            return retval;                                              \
        }
#else
#define IF_CASE(x) if (wnd_type == name_##x) { return Make_##x(params, position); }
#endif

        IF_CASE(dialog)
        else IF_CASE(button)
        else IF_CASE(checkbox)
        else IF_CASE(display_number)
        else IF_CASE(edit_number)
        else IF_CASE(edit_text)
        else IF_CASE(image)
        else IF_CASE(popup)
        else IF_CASE(radio_button)
        else IF_CASE(radio_button_group)
        else IF_CASE(slider)
        else IF_CASE(tab_group)
        else IF_CASE(static_text)
        else IF_CASE(menu_bar)
        else IF_CASE(int_spin)
        else IF_CASE(double_spin)
        else IF_CASE(overlay)
        else IF_CASE(panel)
        else if (wnd_type == adobe::name_row || wnd_type == adobe::name_column) {
#if INSTRUMENT_WINDOW_CREATION
            std::cout << std::string(indent * 4, ' ') << "    Make_layout(" << wnd_type << ", " << params << ", ...)\n";
            MakeWndResult* retval = Make_layout(wnd_type, params, position);
            std::cout << std::string(indent * 4, ' ') << "    " << retval->m_wnd.get() << " " << "\n";
            return retval;
#else
            return Make_layout(wnd_type, params, position);
#endif
        } else {
            std::string wnd_type_ = wnd_type.c_str();
            throw adobe::stream_error_t(wnd_type_ + " is not a supported view type", position);
        }

#undef IF_CASE
    }

    bool IsContainer(adobe::name_t wnd_type)
    {
        using namespace adobe;

        return
            wnd_type == adobe::name_dialog ||
            wnd_type == adobe::name_radio_button_group ||
            wnd_type == adobe::name_tab_group ||
            wnd_type == adobe::name_overlay ||
            wnd_type == adobe::name_panel ||
            wnd_type == adobe::name_row ||
            wnd_type == adobe::name_column;
    }

    adobe::any_regular_t VariableLookup(const adobe::basic_sheet_t& layout_sheet, adobe::name_t name)
    {
        static bool s_once = true;
        static adobe::name_t s_reflected_names[] =
            {
                adobe::key_align_left,
                adobe::key_align_right,
                adobe::key_align_top,
                adobe::key_align_bottom,
                adobe::key_align_center,
                adobe::key_align_proportional,
                adobe::key_align_fill,
                adobe::key_place_row,
                adobe::key_place_column,
                adobe::key_place_overlay
            };
        static std::pair<adobe::name_t*, adobe::name_t*> s_reflected_names_range;

        if (s_once) {
            adobe::sort(s_reflected_names);
            s_reflected_names_range.first = boost::begin(s_reflected_names);
            s_reflected_names_range.second = boost::end(s_reflected_names);
            s_once = false;
        }

        adobe::name_t* it =
            std::lower_bound(s_reflected_names_range.first, s_reflected_names_range.second, name);

        if (it != s_reflected_names_range.second && *it == name)
            return adobe::any_regular_t(name);

        return layout_sheet[name];
    }

    Alignment HorizontalAlignment(Flags<Alignment> flags)
    {
        const Flags<Alignment> HORIZONTAL_FLAGS(ALIGN_LEFT | ALIGN_CENTER | ALIGN_RIGHT);
        if ((flags & HORIZONTAL_FLAGS) == ALIGN_LEFT)
            return ALIGN_LEFT;
        else if ((flags & HORIZONTAL_FLAGS) == ALIGN_CENTER)
            return ALIGN_CENTER;
        else if ((flags & HORIZONTAL_FLAGS) == ALIGN_RIGHT)
            return ALIGN_RIGHT;
        return ALIGN_NONE;
    }

}

struct EveLayout::Impl
{
    Impl(adobe::sheet_t& sheet) :
        m_sheet(sheet),
        m_current_nested_view(0),
        m_wnd(0)
        { m_lookup.attach_to(m_evaluator); }

    ~Impl()
        { delete m_wnd; }

    adobe::dictionary_t Contributing() const
        { return m_sheet.contributing(); }

    void Print(std::ostream& os) const
        {
            os << "layout name_ignored\n"
               << "{\n";
            for (std::size_t i = 0; i < m_added_cells.size(); ++i) {
                const AddedCellSet& cell_set = m_added_cells[i];
                if (i)
                    os << '\n';
                switch (cell_set.m_access) {
                case adobe::eve_callback_suite_t::constant_k: os << "constant:\n"; break;
                case adobe::eve_callback_suite_t::interface_k: os << "interface:\n"; break;
                }
                for (std::size_t j = 0; j < cell_set.m_cells.size(); ++j) {
                    const CellParameters& params = cell_set.m_cells[j];
                    // TODO: print detailed comment
                    os << "    " << params.m_name << " : "
                       << WriteExpression(params.m_initializer) << ";\n";
                    // TODO: print brief comment
                }
            }
            os << "    view ";
            PrintNestedView(os, m_nested_views, 1);
            os << "}\n";
        }

    adobe::eve_callback_suite_t BindCallbacks()
        {
            adobe::eve_callback_suite_t retval;
            m_evaluator.set_variable_lookup(
                boost::bind(VariableLookup, boost::cref(m_layout_sheet), _1)
            );
            retval.add_view_proc_m =
                boost::bind(&EveLayout::Impl::AddView, this, _1, _2, _3, _4, _5, _6);
            retval.add_cell_proc_m =
                boost::bind(&EveLayout::Impl::AddCell, this, _1, _2, _3, _4, _5, _6);
            return retval;
        }

    void AddCell(adobe::eve_callback_suite_t::cell_type_t type,
                 adobe::name_t name,
                 const adobe::line_position_t& position,
                 const adobe::array_t& initializer,
                 const std::string& brief,
                 const std::string& detailed)
        {
            if (m_added_cells.empty() || m_added_cells.back().m_access != type)
                m_added_cells.push_back(AddedCellSet(type));

            m_added_cells.back().m_cells.push_back(
                CellParameters(type, name, position, initializer, brief, detailed)
            );

            m_evaluator.evaluate(initializer);
            adobe::any_regular_t value(m_evaluator.back());
            m_evaluator.pop_back();

            if (type == adobe::eve_callback_suite_t::constant_k)
                m_layout_sheet.add_constant(name, value);
            else if (type == adobe::eve_callback_suite_t::interface_k)
                m_layout_sheet.add_interface(name, value);
            else
                assert(0 && "Cell type not supported");
        }

    boost::any AddView(const boost::any& parent_,
                       const adobe::line_position_t& position,
                       adobe::name_t name,
                       const adobe::array_t& parameters,
                       const std::string& brief,
                       const std::string& detailed)
        {
            ViewParameters params(parent_, position, name, parameters, brief, detailed);

            // TODO: Don't do this -- instead, just generate a unique integer
            // to allow the nesting logic to work right.  The actual creation
            // of all the Wnds in the layout will be postponed until the end.
            Wnd* parent = boost::any_cast<Wnd*>(parent_);

            if (!m_current_nested_view) {
                m_nested_views = NestedViews(params, 0);
                m_current_nested_view = &m_nested_views;
            } else {
                while (boost::any_cast<Wnd*>(m_current_nested_view->m_view_parameters.m_parent) !=
                       parent &&
                       m_current_nested_view->m_nested_view_parent) {
                    m_current_nested_view = m_current_nested_view->m_nested_view_parent;
                }
                assert(m_current_nested_view);
                bool is_container = IsContainer(name);
                if (is_container)
                    params.m_parent = ++parent;
                m_current_nested_view->m_children.push_back(NestedViews(params, m_current_nested_view));
                if (is_container)
                    m_current_nested_view = &m_current_nested_view->m_children.back();
            }

            return parent;
        }

    struct CellParameters
    {
        CellParameters(adobe::eve_callback_suite_t::cell_type_t type,
                       adobe::name_t name,
                       const adobe::line_position_t& position,
                       const adobe::array_t& initializer,
                       const std::string& brief,
                       const std::string& detailed) :
            m_type(type),
            m_name(name),
            m_position(position),
            m_initializer(initializer),
            m_brief(brief),
            m_detailed(detailed)
            {}
        adobe::eve_callback_suite_t::cell_type_t m_type;
        adobe::name_t m_name;
        adobe::line_position_t m_position;
        adobe::array_t m_initializer;
        std::string m_brief;
        std::string m_detailed;
    };

    struct AddedCellSet
    {
        AddedCellSet(adobe::eve_callback_suite_t::cell_type_t access) :
            m_access(access)
            {}
        adobe::eve_callback_suite_t::cell_type_t m_access;
        std::vector<CellParameters> m_cells;
    };

    typedef std::vector<AddedCellSet> AddedCells;

    struct ViewParameters
    {
        ViewParameters() {}
        ViewParameters(const boost::any& parent,
                       const adobe::line_position_t& position,
                       adobe::name_t name,
                       const adobe::array_t& parameters,
                       const std::string& brief,
                       const std::string& detailed) :
            m_parent(parent),
            m_position(position),
            m_name(name),
            m_parameters(parameters),
            m_brief(brief),
            m_detailed(detailed)
            {}
        boost::any m_parent;
        adobe::line_position_t m_position;
        adobe::name_t m_name;
        adobe::array_t m_parameters;
        std::string m_brief;
        std::string m_detailed;
    };

    struct NestedViews
    {
        NestedViews() :
            m_nested_view_parent(0)
            {}
        NestedViews(const ViewParameters& view_parameters, NestedViews* parent) :
            m_view_parameters(view_parameters),
            m_nested_view_parent(parent)
            {}
        ViewParameters m_view_parameters;
        NestedViews* m_nested_view_parent;
        std::vector<NestedViews> m_children;
    };

    void AddChildrenToHorizontalLayout(Layout& layout,
                                       MakeWndResult& wnd,
                                       boost::ptr_vector<MakeWndResult>& children)
    {
        for (std::size_t i = 0; i < children.size(); ++i) {
            Pt min_usable_size = children[i].m_wnd->MinUsableSize();
            layout.SetMinimumColumnWidth(i, min_usable_size.x);
            layout.Add(children[i].m_wnd.release(), 0, i, 1, 1, AlignmentFlags(wnd, children[i]));
        }
    }

    void AddChildrenToVerticalLayout(Layout& layout,
                                     MakeWndResult& wnd,
                                     boost::ptr_vector<MakeWndResult>& children)
    {
#if INSTRUMENT_ADD_TO_VERTICAL
        std::cout << "AddChildrenToVerticalLayout()\n";
#endif
        std::size_t max_columns = 1;
        std::map<Alignment, X> max_all_columns_widths;
        std::map<Alignment, std::vector<X> > max_single_column_widths;
        std::map<Alignment, std::vector<Layout*> > children_as_1x2_layouts;
        std::vector<Flags<Alignment> > child_alignments(children.size());
        for (std::size_t i = 0; i < children.size(); ++i) {
#if INSTRUMENT_ADD_TO_VERTICAL
            std::cout << "    child " << i << ":\n";
#endif
            Layout* l = 0;
            Alignment align;
            std::pair<adobe::name_t, adobe::name_t> raw_alignments = Alignments(wnd, children[i]);
#if INSTRUMENT_ADD_TO_VERTICAL
            std::cout << "        raw_align=" << raw_alignments.first << "," << raw_alignments.second << "\n";
#endif
            if ((l = dynamic_cast<Layout*>(children[i].m_wnd.get())) &&
                l->Rows() == 1u &&
                l->Columns() == 2u &&
                dynamic_cast<const TextControl*>(l->Cells()[0][0])) {
#if INSTRUMENT_ADD_TO_VERTICAL
                std::cout << "        two-column layout\n";
#endif
                if (!raw_alignments.first || raw_alignments.first == adobe::key_align_fill)
                    raw_alignments.first = adobe::key_align_left;
                child_alignments[i] = AlignmentFlags(raw_alignments.first, raw_alignments.second);
                align = HorizontalAlignment(child_alignments[i]);
#if INSTRUMENT_ADD_TO_VERTICAL
                std::cout << "        align=" << align << "\n";
#endif
                max_columns = 2;
                max_single_column_widths[align].resize(2);
                max_single_column_widths[align][0] =
                    std::max(max_single_column_widths[align][0],
                             std::max(l->Cells()[0][0]->MinSize().x, l->Cells()[0][0]->MinUsableSize().x));
                max_single_column_widths[align][1] =
                    std::max(max_single_column_widths[align][1],
                             std::max(l->Cells()[0][1]->MinSize().x, l->Cells()[0][1]->MinUsableSize().x));
#if INSTRUMENT_ADD_TO_VERTICAL
                std::cout << "        cell-size 0=" << std::max(l->Cells()[0][0]->MinSize().x, l->Cells()[0][0]->MinUsableSize().x) << "\n";
                std::cout << "        max_single_column_widths[" << align << "][0]=" << max_single_column_widths[align][0] << "\n";
                std::cout << "        cell-size 1=" << std::max(l->Cells()[0][1]->MinSize().x, l->Cells()[0][1]->MinUsableSize().x) << "\n";
                std::cout << "        max_single_column_widths[" << align << "][1]=" << max_single_column_widths[align][1] << "\n";
#endif
            } else {
#if INSTRUMENT_ADD_TO_VERTICAL
                std::cout << "        no layout\n";
#endif
                l = 0;
                child_alignments[i] = AlignmentFlags(raw_alignments.first, raw_alignments.second);
                align = HorizontalAlignment(child_alignments[i]);
#if INSTRUMENT_ADD_TO_VERTICAL
                std::cout << "        align=" << align << "\n";
#endif
            }
            max_all_columns_widths[align] =
                std::max(max_all_columns_widths[align],
                         std::max(children[i].m_wnd->MinSize().x, children[i].m_wnd->MinUsableSize().x));
#if INSTRUMENT_ADD_TO_VERTICAL
            std::cout << "    max_all_columns_widths[" << align << "]=" << max_all_columns_widths[align] << "\n";
#endif
            children_as_1x2_layouts[align].resize(children.size());
            children_as_1x2_layouts[align][i] = l;
#if INSTRUMENT_ADD_TO_VERTICAL
            std::cout << "    children_as_1x2_layouts[align][i]=" << children_as_1x2_layouts[align][i] << "\n";
#endif
        }

#if INSTRUMENT_ADD_TO_VERTICAL
        std::cout << "\n    max_columns=" << max_columns << "\n";
#endif

        max_single_column_widths[ALIGN_NONE].resize(max_columns);
        max_single_column_widths[ALIGN_LEFT].resize(max_columns);
        max_single_column_widths[ALIGN_CENTER].resize(max_columns);
        max_single_column_widths[ALIGN_RIGHT].resize(max_columns);

        for (std::size_t i = 0; i < children.size(); ++i) {
            std::pair<adobe::name_t, adobe::name_t> raw_alignments = Alignments(wnd, children[i]);
            Alignment align = HorizontalAlignment(AlignmentFlags(raw_alignments.first, raw_alignments.second));
            if (!raw_alignments.first || raw_alignments.first == adobe::key_align_fill)
                raw_alignments.first = adobe::key_align_left;
#if INSTRUMENT_ADD_TO_VERTICAL
            std::cout << "    layout.Add(child " << i << ", ..., " << child_alignments[i] << ")\n";
#endif
            layout.Add(children[i].m_wnd.release(), i, 0, 1, 1, child_alignments[i]);
        }

        const Alignment ALIGNMENTS[] = { ALIGN_NONE, ALIGN_LEFT, ALIGN_CENTER, ALIGN_RIGHT };
        const std::size_t NUM_ALIGNMENTS = sizeof(ALIGNMENTS) / sizeof(ALIGNMENTS[0]);

        for (std::size_t i = 0; i < NUM_ALIGNMENTS; ++i) {
            Alignment align = ALIGNMENTS[i];
#if INSTRUMENT_ADD_TO_VERTICAL
            std::cout << "    align " << align << ":\n";
#endif

            std::size_t num_in_this_layout = 0;
            for (std::size_t i = 0; i < children_as_1x2_layouts[align].size(); ++i) {
                if (children_as_1x2_layouts[align][i])
                    ++num_in_this_layout;
            }
#if INSTRUMENT_ADD_TO_VERTICAL
            std::cout << "    num_in_this_layout=" << num_in_this_layout;
            if (num_in_this_layout < 2)
                std::cout << " ... continue\n";
            else
                std::cout << "\n";
#endif
            if (num_in_this_layout < 2)
                continue;

            X_d summed_single_column_widths =
                1.0 * std::accumulate(max_single_column_widths[align].begin(),
                                      max_single_column_widths[align].end(),
                                      X0);
#if INSTRUMENT_ADD_TO_VERTICAL
            std::cout << "    summed_single_column_widths=" << summed_single_column_widths << "\n";
#endif
            bool all_columns_width_larger = max_all_columns_widths[align] > summed_single_column_widths;
            X_d difference = max_all_columns_widths[align] - summed_single_column_widths;
#if INSTRUMENT_ADD_TO_VERTICAL
            std::cout << "    difference=" << difference << "\n";
#endif

            X_d stretch_0 =
                summed_single_column_widths ?
                max_single_column_widths[align][0] / summed_single_column_widths :
                X_d(0.0);
            X_d min_width_0 = 1.0 * max_single_column_widths[align][0];
#if INSTRUMENT_ADD_TO_VERTICAL
            std::cout << "    stretch_0=" << stretch_0 << " min_width_0=" << min_width_0 << "\n";
#endif
            if (all_columns_width_larger)
                min_width_0 += difference * stretch_0;

#if INSTRUMENT_ADD_TO_VERTICAL
            if (all_columns_width_larger)
                std::cout << "    stretch_0=" << stretch_0 << " min_width_0=" << min_width_0 << "\n";
#endif

            X_d stretch_1;
            X_d min_width_1;
            if (1u < max_columns) {
                stretch_1 =
                    summed_single_column_widths ?
                    max_single_column_widths[align][1] / summed_single_column_widths :
                    X_d(0.0);
                min_width_1 = 1.0 * max_single_column_widths[align][1];
#if INSTRUMENT_ADD_TO_VERTICAL
                std::cout << "    stretch_1=" << stretch_1 << " min_width_1=" << min_width_1 << "\n";
#endif
                if (all_columns_width_larger)
                    min_width_1 += difference * stretch_1;
#if INSTRUMENT_ADD_TO_VERTICAL
                if (all_columns_width_larger)
                    std::cout << "    stretch_1=" << stretch_1 << " min_width_1=" << min_width_1 << "\n";
#endif
            }

            for (std::size_t i = 0; i < children_as_1x2_layouts[align].size(); ++i) {
                if (Layout* l = children_as_1x2_layouts[align][i]) {
                    l->SetMinimumColumnWidth(0, X(static_cast<int>(std::ceil(Value(min_width_0)))));
                    l->SetMinimumColumnWidth(1, X(static_cast<int>(std::ceil(Value(min_width_1)))));
                    l->SetChildAlignment(l->Cells()[0][0], ALIGN_RIGHT);
                    l->SetChildAlignment(l->Cells()[0][1], ALIGN_LEFT);
#if INSTRUMENT_ADD_TO_VERTICAL
                    std::cout << "    l->SetMinimumColumnWidth(0, =" << X(std::ceil(Value(min_width_0))) << ")\n";
                    std::cout << "    l->SetMinimumColumnWidth(1, =" << X(std::ceil(Value(min_width_1))) << ")\n";
#endif
                }
            }
        }
    }

    void AddChildren(MakeWndResult& wnd_,
                     boost::ptr_vector<MakeWndResult>& children,
                     adobe::name_t placement,
                     const ViewParameters& wnd_view_params)
    {
        using namespace adobe;

        Wnd* wnd = wnd_.m_wnd.get();

#if INSTRUMENT_WINDOW_CREATION
        std::cout << std::string(indent * 4, ' ') << "    AddChildren(children = [ ";
        for (std::size_t i = 0; i < children.size(); ++i) {
            std::cout << children[i].m_wnd.get() << " ";
        }
        std::cout << "])\n";
#endif

        if (wnd_view_params.m_name == adobe::name_dialog || wnd_view_params.m_name == adobe::name_panel) {
            if (placement == adobe::key_place_overlay) {
                std::string name = wnd_view_params.m_name.c_str();
                throw stream_error_t("place_overlay placement is not compatible with " + name,
                                     wnd_view_params.m_position);
            }
            Orientation orientation = (placement == adobe::key_place_row) ? HORIZONTAL : VERTICAL;
            std::auto_ptr<Layout>
                layout(new Layout(X0, Y0, X1, Y1,
                                  orientation == VERTICAL ? children.size() : 1,
                                  orientation == VERTICAL ? 1 : children.size(),
                                  wnd_.m_margin,
                                  wnd_.m_margin));

#if SHOW_LAYOUTS
            layout->RenderOutline(true);
#endif

            if (orientation == VERTICAL) {
                if (wnd_view_params.m_name == adobe::name_dialog && !wnd_.m_child_horizontal)
                    wnd_.m_default_child_horizontal = adobe::key_align_left;
                AddChildrenToVerticalLayout(*layout, wnd_, children);
            } else {
                if (wnd_view_params.m_name == adobe::name_dialog && !wnd_.m_child_vertical)
                    wnd_.m_default_child_vertical = adobe::key_align_top;
                AddChildrenToHorizontalLayout(*layout, wnd_, children);
            }
            wnd->SetLayout(layout.release());
        } else if (wnd_view_params.m_name == adobe::name_radio_button_group) {
            RadioButtonGroup* radio_button_group = boost::polymorphic_downcast<RadioButtonGroup*>(wnd);
            for (std::size_t i = 0; i < children.size(); ++i) {
                StateButton* state_button = dynamic_cast<StateButton*>(children[i].m_wnd.get());
                if (!state_button) {
                    throw stream_error_t("non-radio_buttons are not compatible with radio_button_group",
                                         wnd_view_params.m_position);
                }
                children[i].m_wnd.release();
                radio_button_group->AddButton(state_button);
            }
        } else if (wnd_view_params.m_name == adobe::name_tab_group) {
            TabWnd* tab_wnd = boost::polymorphic_downcast<TabWnd*>(wnd);
            for (std::size_t i = 0; i < children.size(); ++i) {
                Panel* panel = dynamic_cast<Panel*>(children[i].m_wnd.get());
                if (!panel) {
                    throw stream_error_t("non-panels are not compatible with tab_group",
                                         wnd_view_params.m_position);
                }
                if (panel->Name().empty()) {
                    throw stream_error_t("a panel used in tab_group requires a name parameter",
                                         wnd_view_params.m_position);
                }
                children[i].m_wnd.release();
                tab_wnd->AddWnd(panel, panel->Name());
            }
        } else if (wnd_view_params.m_name == adobe::name_overlay) {
            OverlayWnd* overlay = boost::polymorphic_downcast<OverlayWnd*>(wnd);
            for (std::size_t i = 0; i < children.size(); ++i) {
                if (!dynamic_cast<Panel*>(children[i].m_wnd.get())) {
                    throw stream_error_t("non-panels are not compatible with overlay",
                                         wnd_view_params.m_position);
                }
                overlay->AddWnd(children[i].m_wnd.release());
            }
        } else if (wnd_view_params.m_name == adobe::name_row) {
            Layout* layout = boost::polymorphic_downcast<Layout*>(wnd);
            layout->ResizeLayout(1, children.size());
            AddChildrenToHorizontalLayout(*layout, wnd_, children);
        } else if (wnd_view_params.m_name == adobe::name_column) {
            Layout* layout = boost::polymorphic_downcast<Layout*>(wnd);
            layout->ResizeLayout(children.size(), 1);
            AddChildrenToVerticalLayout(*layout, wnd_, children);
        } else {
            throw stream_error_t("attempted to attach children to a non-container",
                                 wnd_view_params.m_position);
        }
    }

    Wnd& Finish()
        {
            std::auto_ptr<MakeWndResult> dialog(CreateChild(m_nested_views));
            m_wnd = dialog->m_wnd.release();
            m_wnd->Resize(Pt(X1, Y1));
#if INSTRUMENT_CREATED_LAYOUT
            DumpLayout(m_wnd);
#endif
            return *m_wnd;
        }

    MakeWndResult* CreateChild(const NestedViews& nested_view)
        {
            const ViewParameters& view_params = nested_view.m_view_parameters;

#if INSTRUMENT_WINDOW_CREATION
            std::cout << std::string(indent * 4, ' ') << "CreateChild(" << view_params.m_name << ")\n";
#endif

            m_evaluator.evaluate(view_params.m_parameters);
            adobe::dictionary_t parameters(::adobe::move(m_evaluator.back().cast<adobe::dictionary_t>()));
            m_evaluator.pop_back();

#if INSTRUMENT_WINDOW_CREATION
            std::cout << std::string(indent * 4, ' ') << "    MakeWnd(" << view_params.m_name << ")\n";
#endif
            std::auto_ptr<MakeWndResult> retval(
                MakeWnd(view_params.m_name, parameters, view_params.m_position)
            );

#if INSTRUMENT_WINDOW_CREATION
            ++indent;
#endif
            boost::ptr_vector<MakeWndResult> children;
            for (std::size_t i = 0; i < nested_view.m_children.size(); ++i) {
                children.push_back(CreateChild(nested_view.m_children[i]));
            }
#if INSTRUMENT_WINDOW_CREATION
            --indent;
#endif

            assert(children.empty() || IsContainer(view_params.m_name));

            adobe::name_t placement =
                view_params.m_name == adobe::name_dialog ? adobe::key_place_row : adobe::key_place_column;
            get_value(parameters, adobe::key_placement, placement);

            if (!children.empty())
                AddChildren(*retval, children, placement, view_params);

            return retval.release();
        }

    static void PrintNestedView(std::ostream& os, const NestedViews& nested_view, unsigned int indent)
        {
            const ViewParameters& params = nested_view.m_view_parameters;
            // TODO: print detailed comment
            std::string initial_indent(indent * 4, ' ');
            if (indent == 1u)
                initial_indent.clear();
            std::string param_string = WriteExpression(params.m_parameters);
            os << initial_indent << params.m_name << '('
               << param_string.substr(1, param_string.size() - 3)
               << ')';
            if (nested_view.m_children.empty()) {
                if (indent  == 1u) {
                    os << "\n" // TODO: print brief comment
                       << "    {}\n";
                } else {
                    os << ";\n"; // TODO: print brief comment
                }
            } else {
                // TODO: print brief comment
                os << '\n'
                   << std::string(indent * 4, ' ') << "{\n";
                for (std::size_t i = 0; i < nested_view.m_children.size(); ++i) {
                    PrintNestedView(os, nested_view.m_children[i], indent + 1);
                }
                os << std::string(indent * 4, ' ') << "}\n";
            }
        }

    adobe::sheet_t& m_sheet;

    adobe::basic_sheet_t m_layout_sheet;
    adobe::virtual_machine_t m_evaluator;
    adobe::vm_lookup_t m_lookup;

    AddedCells m_added_cells;
    NestedViews m_nested_views;
    NestedViews* m_current_nested_view;

    Wnd* m_wnd;
};


////////////////////////////////////////////////////////////
// EveLayout                                              //
////////////////////////////////////////////////////////////
EveLayout::EveLayout(adobe::sheet_t& sheet) :
    m_impl(new Impl(sheet))
{}

EveLayout::~EveLayout()
{ delete m_impl; }

adobe::dictionary_t EveLayout::Contributing() const
{ return m_impl->Contributing(); }

void EveLayout::Print(std::ostream& os) const
{ m_impl->Print(os); }

adobe::eve_callback_suite_t EveLayout::BindCallbacks()
{ return m_impl->BindCallbacks(); }

void EveLayout::AddCell(adobe::eve_callback_suite_t::cell_type_t type,
                        adobe::name_t name,
                        const adobe::line_position_t& position,
                        const adobe::array_t& initializer,
                        const std::string& brief,
                        const std::string& detailed)
{ m_impl->AddCell(type, name, position, initializer, brief, detailed); }

boost::any EveLayout::AddView(const boost::any& parent,
                              const adobe::line_position_t& position,
                              adobe::name_t name,
                              const adobe::array_t& parameters,
                              const std::string& brief,
                              const std::string& detailed)
{ return m_impl->AddView(parent, position, name, parameters, brief, detailed); }

Wnd& EveLayout::Finish()
{ return m_impl->Finish(); }
