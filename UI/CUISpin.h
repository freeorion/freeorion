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
    /** \name Structors */ //@{
    CUISpin(int x, int y, int w, T value, T step, T min, T max, bool edits) :
        Spin(x, y, w, value, step, min, max, edits, ClientUI::FONT, ClientUI::PTS, ClientUI::CTRL_BORDER_COLOR, 
             ClientUI::TEXT_COLOR, GG::CLR_ZERO, new CUIArrowButton(0, 0, 1, 1, ClientUI::CTRL_BORDER_COLOR), 
             new CUIArrowButton(0, 0, 1, 1, ClientUI::CTRL_BORDER_COLOR))
    {
    }

    /** ctor that constructs an CUISpin object from an XMLElement. \throw std::invalid_argument May throw 
        std::invalid_argument if \a elem does not encode a CUISpin object*/
    CUISpin(const XMLElement& elem) : 
        Spin(elem.Child(Spin<T>::XMLTypeName()))
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
        retval.AppendChild(Spin::XMLEncode());
        return retval;
    }
    //@}

    /** \name Mutators */ //@{
    virtual bool Render()
    {
        Clr color_to_use = Disabled() ? DisabledColor(Color()) : Color();
        Clr int_color_to_use = Disabled() ? DisabledColor(InteriorColor()) : InteriorColor();
        Pt ul = UpperLeft(), lr = LowerRight();
        FlatRectangle(ul.x, ul.y, lr.x, lr.y, int_color_to_use, color_to_use, 1);
        if (!m_editable) {
            m_edit->OffsetMove(UpperLeft());
            m_edit->Render();
            m_edit->OffsetMove(-UpperLeft());
        }
        m_up_bn->OffsetMove(UpperLeft());
        m_dn_bn->OffsetMove(UpperLeft());
        m_up_bn->Render();
        m_dn_bn->Render();
        m_up_bn->OffsetMove(-UpperLeft());
        m_dn_bn->OffsetMove(-UpperLeft());
        return true;
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
