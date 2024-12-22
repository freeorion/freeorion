#ifndef _CUIDrawUtil_h_
#define _CUIDrawUtil_h_

#include <GG/Clr.h>
#include <GG/PtRect.h>
#include <GG/GLClientAndServerBuffer.h>

#include <memory>


/** adjusts the intensity of the color up or down by \a amount units per color
  * channel; leaves alpha unchanged if \a jointly_capped is true then, if the
  * original \a amount would out any of the rgb channel values above 255,
  * \a amount for all channels is reduced so that the highest resulting rgb
  * channel value is 255. */
[[nodiscard]] GG::Clr AdjustBrightness(GG::Clr color, int amount, bool jointly_capped=false);

/** adjusts the intensity of the color up or down by multiplying the non-alpa
  * channels by \a amount if \a jointly_capped is true then, if the original
  * \a amount would out any of the rgb channel values above 255,
  * \a amount for all channels is reduced so that the highest resulting rgb
  * channel value is 255. */
[[nodiscard]] GG::Clr AdjustBrightness(GG::Clr color, double amount, bool jointly_capped=false);

/** returns fully opaque (max alpha channel) version of the color */
[[nodiscard]] GG::Clr OpaqueColor(GG::Clr color);

/** Stores verticies in CCW order that outline a rectangle
 * @param[out] buffer Buffer to store verticies
 * @param[in] area Outside corners of rectangle
 * @param[in] border_thickness Thickness of border for each corner, drawn
 *              towards the interior of @p area.
 */
void BufferStoreRectangle(GG::GL2DVertexBuffer& buffer, GG::Rect area, GG::Rect border_thickness);

/** Using immediate GL calls, renders a rectangle whose upper left and lower
  * right corners are angled.  If \a upper_left_angled == false, the upper left
  * corner is drawn as a normal corner */
void AngledCornerRectangle(const GG::Pt ul, const GG::Pt lr, GG::Clr color, GG::Clr border,
                           int angle_offset, int thick, bool upper_left_angled = true,
                           bool lower_right_angled = true, bool draw_bottom = true);

/** Stores, in \a buffer verticies in CCW order that outline a rectangle
  * with upper left and lower right corners are angled. */
void BufferStoreAngledCornerRectangleVertices(GG::GL2DVertexBuffer& buffer, const GG::Pt ul, const GG::Pt lr,
                                              int angle_offset, bool upper_left_angled = true,
                                              bool lower_right_angled = true, bool connect_bottom_line = true);

/** returns true iff \a pt falls within \a rect, with the missing bits of the
  * angled corners not catching the point. If \a upper_left_angled == false,
  * the upper left corner is treated as a normal corner. */
bool InAngledCornerRect(const GG::Pt pt, const GG::Pt ul, const GG::Pt lr, int angle_offset,
                        bool upper_left_angled = true, bool lower_right_angled = true) noexcept;

/** the orientations used to render some shapes used in the UI; the orientations
  * usually refer to the direction in which the shape is pointing */
enum class ShapeOrientation : uint8_t {UP, DOWN, LEFT, RIGHT};

/** renders a triangle of arbitrary size and shape, having an optional 1-pixel-thick border */
void Triangle(const GG::Pt pt1, const GG::Pt pt2, const GG::Pt pt3, GG::Clr color, bool border = true);

/** returns true iff \a pt lies within the triangle described by the other parameters */
bool InTriangle(const GG::Pt pt, const GG::Pt pt1, const GG::Pt pt2, const GG::Pt pt3) noexcept;

/** renders a triangle with two equal-length sides, oriented in the desired
  * direction.  The triangle will have a base length of one of
  * (<i>x2</i> - <i>x1</i>) and (<i>y2</i> - <i>y1</i>),
  * depending on \a orientation, and a height of the other. */
void IsoscelesTriangle(const GG::Pt ul, const GG::Pt lr, ShapeOrientation orientation,
                       GG::Clr color, bool border = true);

/** Stores, in \a buffer, vertices in CCW order that outline an isosceles triangle. */
void BufferStoreIsoscelesTriangle(GG::GL2DVertexBuffer& buffer, const GG::Pt ul,
                                  const GG::Pt lr, ShapeOrientation orientation);

/** returns true iff \a pt falls within the isosceles triangle described by the other parameters */
bool InIsoscelesTriangle(const GG::Pt pt, const GG::Pt ul, const GG::Pt lr,
                         ShapeOrientation orientation) noexcept;

/** Draws a filled portion of a circle when \a filled_shape is true or an
  * unfilled portion when \a filled_shape is false. */
void CircleArc(const GG::Pt ul, const GG::Pt lr, double theta1, double theta2, bool filled_shape);

/** Draws a filled or unfilled portions of a circle with every other segment
  * skipped, with a total of \a segments segments. */
void CircleArcSegments(const GG::Pt ul, const GG::Pt lr, int segments, bool filled_shape);

/** Stores, in \a buffer vertices in CCW order that outline a circular arc or
  * \a num_slices indicates how many triangles to use to compose the circle, or
  * if 0, indicates that the function should auto-determine how many slices.
  * if \a fan is true, the vertices are stored as a triangle fan- compatible
  * list, while false indicates a triangle list (where each triangle has all
  * 3 vertices specified explicitly. \a filled_shape indicates that a vertex
  * at the centre of the circle should be present and shared with all triangles.*/
void BufferStoreCircleArcVertices(GG::GL2DVertexBuffer& buffer, const GG::Pt ul,
                                  const GG::Pt lr, double theta1, double theta2,
                                  bool filled_shape = false,
                                  int num_slices = 0, bool fan = true);

/** Draws a rectangle whose corners are rounded with radius \a radius as
  * indicated by the \a *_round parameters.  If \a fill is true, the resulting
  * rectangle is solid; it is drawn in outline otherwise. */
void PartlyRoundedRect(const GG::Pt ul, const GG::Pt lr, int radius, bool ur_round,
                       bool ul_round, bool ll_round, bool lr_round, bool fill);

/** Stores, in \a buffer verticies in CCW order that outline rectangle with
  * corners rounded with radius \a radrius as determined by the \a *_round
  * parameters. */
void BufferStorePartlyRoundedRectVertices(GG::GL2DVertexBuffer& buffer, const GG::Pt ul,
                                          const GG::Pt lr, int radius, bool ur_round,
                                          bool ul_round, bool ll_round, bool lr_round);

/** ScanlineRenderer renders scanlines in circular/square/arbitrary areas.
    It loads the scanline shader on first use.
    There is only expected to by one ScanlineRenderer per compilation unit so
    that the shader is compiled once.

    Bracketing OpenGL primitives with StartUsing() and StopUsing() will render
    scanlines in that arbitrary area.*/
class ScanlineRenderer {
public:
    ScanlineRenderer();
    ~ScanlineRenderer();

    /** Draw scanlines in the circular area bounded by \p ul and \p lr.*/
    void RenderCircle(const GG::Pt ul, const GG::Pt lr);

    /** Draw scanlines in the square area bounded by \p ul and \p lr.*/
    void RenderRectangle(const GG::Pt ul, const GG::Pt lr);

    /** Changes the color used to draw the scanlines. Set color before calling StartUsing()*/
    void SetColor(GG::Clr clr) noexcept;

    /** Start using ScanlineRenderer to draw arbitrary shapes with the scanline
        shader program.*/
    void StartUsing();

    /** Stop using ScanlineRenderer to draw arbitrary shapes with the scanline
        shader program.*/
    void StopUsing();

private:
    class Impl;

    std::unique_ptr<Impl> const m_impl;
};


#endif
