#ifndef _Measure_h_
#define _Measure_h_

#include <GG/PtRect.h>
#include <GG/Font.h>

namespace GG {

/** A pixel-based measure.
 * This class is intended to encapsulate hard-coded screen
 * measurements to enable scaling.  A measure is simply a pixel count,
 * presumed to be a measure of distance along an axis for the unscaled
 * display.  The measure is scaled when converted to a GG::X or
 * GG::Y.
 */
class GG_API Measure {
private:
    static std::shared_ptr<Font> s_font;
    int m_value;

    Measure(int value);

public:
    /** Create a new Measure.
     * We use a static function instead of a constructor in order to
     * avoid automatic casts from int to Measure, which could have bad
     * consequences.  This function can be removed  and replaced with
     * a constructor later, once everything has been converted to use
     * Measure.
     */
    static Measure c(int value);

    int Value() const;

#define oper(op)                                \
    Measure operator op () const;

oper(+);
oper(-);

#undef oper


#define oper(op)                                \
    Measure operator op (Measure other) const;  \
    Measure operator op (unsigned int b) const; \
    Measure operator op (int b) const;

oper(+);
oper(-);
oper(*);
oper(/);

#undef oper


#define oper(cmp)                                       \
    bool operator cmp (Measure other) const;

oper(<);
oper(>);
oper(==);

#undef oper


    static void SetFont(std::shared_ptr<Font> font);
    static X ScaleX(X value, std::shared_ptr<Font> font = s_font);
    static Y ScaleY(Y value, std::shared_ptr<Font> font = s_font);

    int ScaleX(std::shared_ptr<Font> font = s_font) const;
    int ScaleY(std::shared_ptr<Font> font = s_font) const;
    X GetX(std::shared_ptr<Font> font = s_font) const;
    Y GetY(std::shared_ptr<Font> font = s_font) const;
    operator X() const;
    operator Y() const;
};

// Useful constants
extern GG_API const Measure M0;
extern GG_API const Measure M1;
extern GG_API const Measure M2;
extern GG_API const Measure M4;
extern GG_API const Measure M5;
extern GG_API const Measure M10;

} // namespace GG


#define oper(op)                                        \
    GG_API GG::Measure operator op (unsigned int a, GG::Measure b);     \
    GG_API GG::Measure operator op (int a, GG::Measure b);

    oper(+);
    oper(-);
    oper(*);
    oper(/);

#undef oper

#endif // _Measure_h_
