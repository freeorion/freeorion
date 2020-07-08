#include <GG/Measure.h>

using namespace GG;

std::shared_ptr<GG::Font> Measure::s_font;

const Measure GG::M0(Measure::c(0));
const Measure GG::M1(Measure::c(1));
const Measure GG::M2(Measure::c(2));
const Measure GG::M4(Measure::c(4));
const Measure GG::M5(Measure::c(5));
const Measure GG::M10(Measure::c(10));

Measure::Measure(int value)
    : m_value (value) {}

Measure Measure::c(int value) {
    return Measure(value);
}

int Measure::Value() const {
    return m_value;
}

void GG::Measure::SetFont(std::shared_ptr<Font> font) {
    s_font = font;
}

X Measure::ScaleX(X value, std::shared_ptr<Font> font) {
    /* Scale based on font's space width which scales linearly:

       pts 16, space width 4
       pts 24, space width 6
       pts 32, space width 8
       pts 40, space width 10

       The default font point size ("pts") is 16 and we scale based on
       that.
    */

    assert(font);

    if (font->PointSize() == 16) {
        // No need for scaling
        return value;
    }

    const X_d scale_factor =
        font->SpaceWidth() / static_cast<double>(4);
    return X(value * scale_factor);
}

Y Measure::ScaleY(Y value, std::shared_ptr<Font> font) {
    /* Scale based on font's height which scales roughly linearly:

       pts 16, height 20
       pts 24, height 30
       pts 32, height 39
       pts 40, height 49
    */

    assert(font);

    if (font->PointSize() == 16) {
        // No need for scaling
        return value;
    }

    const Y_d scale_factor =
        font->Height() / static_cast<double>(20);
    return Y(value * scale_factor);
}

int Measure::ScaleX(std::shared_ptr<Font> font) const {
    return GG::Value(GetX(font));
}

int Measure::ScaleY(std::shared_ptr<Font> font) const {
    return GG::Value(GetY(font));
}

X Measure::GetX(std::shared_ptr<Font> font) const {
    return ScaleX(X(m_value), font);
}

Y Measure::GetY(std::shared_ptr<Font> font) const {
    return ScaleY(Y(m_value), font);
}

Measure::operator X() const {
    return GetX();
}

Measure::operator Y() const {
    return GetY();
}


#define oper(cmp)                                       \
    bool Measure::operator cmp (Measure other) const {  \
        return m_value cmp other.m_value;               \
    }

oper(<);
oper(>);
oper(==);

#undef oper


#define oper(op)                                                \
    Measure Measure::operator op (Measure other) const {        \
        return Measure::c(m_value op other.m_value);            \
    }                                                           \
    Measure Measure::operator op (unsigned int b) const {       \
        return Measure::c(m_value op b);                        \
    }                                                           \
    Measure Measure::operator op (int b) const {                \
        return Measure::c(m_value op b);                        \
    }                                                           \
                                                                \
    Measure operator op (unsigned int a, Measure b) {           \
        return Measure::c(a op b.Value());                      \
    }                                                           \
    Measure operator op (int a, Measure b) {                    \
        return Measure::c(a op b.Value());                      \
    }

oper(+);
oper(-);
oper(*);
oper(/);

#undef oper


#define oper(op)                                \
    Measure Measure::operator op () const {     \
        return Measure(op m_value);             \
    }

oper(+);
oper(-);

#undef oper
