// -*- C++ -*-
#ifndef _CUISpin_h_
#define _CUISpin_h_

#ifndef _CUIControls_h_
#include "CUIControls.h"
#endif

#ifndef _GGSpin_h_
#include "GGSpin.h"
#endif

/** a FreeOrion Spin control */
template <class T> class CUISpin : public GG::Spin<T>
{
public:
    typedef typename GG::Spin<T>::ValueType ValueType;

    /** \name Structors */ //@{
    CUISpin(int x, int y, int w, T value, T step, T min, T max, bool edits) :
        GG::Spin<T>(x, y, w, value, step, min, max, edits, ClientUI::FONT, ClientUI::PTS, ClientUI::CTRL_BORDER_COLOR, 
                    ClientUI::TEXT_COLOR, GG::CLR_ZERO, new CUIArrowButton(0, 0, 1, 1, SHAPE_UP, ClientUI::DROP_DOWN_LIST_ARROW_COLOR), 
                    new CUIArrowButton(0, 0, 1, 1, SHAPE_DOWN, ClientUI::DROP_DOWN_LIST_ARROW_COLOR))
    {
    }

    /** ctor that constructs an CUISpin object from an XMLElement. \throw std::invalid_argument May throw 
        std::invalid_argument if \a elem does not encode a CUISpin object*/
    CUISpin(const GG::XMLElement& elem) : 
        GG::Spin<T>(elem.Child(GG::Spin<T>::XMLTypeName()))
    {
        if (elem.Tag() != XMLTypeName())
            throw std::invalid_argument("Attempted to construct a " + XMLTypeName() + " from an XMLElement that had a tag other than \"" + XMLTypeName() + "\"");
    }
    //@}

    /** \name Accessors */ //@{
    /** constructs an XMLElement from a CUISpin object*/
    virtual GG::XMLElement XMLEncode() const
    {
        GG::XMLElement retval(XMLTypeName());
        retval.AppendChild(GG::Spin<T>::XMLEncode());
        return retval;
    }
    //@}

    /** \name Mutators */ //@{
    virtual bool Render()
    {
        GG::Clr color_to_use = Disabled() ? DisabledColor(Color()) : Color();
        GG::Clr int_color_to_use = Disabled() ? DisabledColor(InteriorColor()) : InteriorColor();
        GG::Pt ul = UpperLeft(), lr = LowerRight();
        FlatRectangle(ul.x, ul.y, lr.x, lr.y, int_color_to_use, color_to_use, 1);
        if (!EditsAllowed()) {
            GetEdit()->OffsetMove(UpperLeft());
            GetEdit()->Render();
            GetEdit()->OffsetMove(-UpperLeft());
        }
        UpButton()->OffsetMove(UpperLeft());
        DownButton()->OffsetMove(UpperLeft());
        UpButton()->Render();
        DownButton()->Render();
        UpButton()->OffsetMove(-UpperLeft());
        DownButton()->OffsetMove(-UpperLeft());
        return true;
    }

    virtual void SizeMove(int x1, int y1, int x2, int y2)
    {
        Wnd::SizeMove(x1, y1, x2, y2);
        const int BORDER_THICK = 1;
        const int BUTTON_MARGIN = 3;
        const int BN_HEIGHT = (Height() - 3 * BUTTON_MARGIN) / 2;
        const int BN_WIDTH = BN_HEIGHT;
        const int BN_X_POS = Width() - BN_WIDTH - BORDER_THICK;
        GetEdit()->SizeMove(0, 0, BN_X_POS, Height());
        UpButton()->SizeMove(BN_X_POS, BORDER_THICK,
                             BN_X_POS + BN_WIDTH, BUTTON_MARGIN + BN_HEIGHT);
        DownButton()->SizeMove(BN_X_POS, BUTTON_MARGIN + BN_HEIGHT + BUTTON_MARGIN,
                               BN_X_POS + BN_WIDTH, BUTTON_MARGIN + BN_HEIGHT + BUTTON_MARGIN + BN_HEIGHT);
    }
    //@}

    /** returns a std::string representing this CUISpin's exact type, including the type of its data, to aid with automatic
        XML saving and loading*/
    static std::string XMLTypeName()
    {
        std::string retval = "CUISpin_";
        retval += typeid(ValueType).name();
        retval += "_";
        return retval;
    }
};

#endif // _CUISpin_h_
