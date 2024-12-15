//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2003-2008 T. Zachary Laine <whatwasthataddress@gmail.com>
//!  Copyright (C) 2013-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

//! @file GG/Layout.h
//!
//! Contains the Layout class, which is used to size and align GG windows.

#ifndef _GG_Layout_h_
#define _GG_Layout_h_


#include <GG/AlignmentFlags.h>
#include <GG/Wnd.h>


namespace GG {

/** \brief An invisible Wnd subclass that arranges its child Wnds.

    A Layout consists of a grid of cells.  A cell may have at most one Wnd
    covering it, but need not contain a Wnd at all.  A Wnd may cover any
    rectangular region of cells, though they will commonly only cover one.
    The cells are arranged into rows and columns.  Most attributes of the
    layout are set for an entire row or column, but alignment is set for each
    child in the layout separately.  Rows and columns have two attributes:
    "stretch", and "min" (minimum row width/minimum column height).  Stretch
    indicates a propotional factor by which each row/column is stretched when
    the layout is resized.  For example, if the sum of the row stretch factors
    is 5, a row with a stretch of 2 will gain 2/5 of the increased space if
    the layout grows vertically, or lose 2/5 of the decreased space if the
    layout shrinks vertically.  Note that this means that rows with a stretch
    of 0 will not change size at all.  The exception to this is when all rows
    have a stretch of 0, in which case all the rows grow and shrink evenly.
    Obviously, this applies to columns as well.  The min sets a lower bound on
    the height of a row or the width of a column.  By default, no alignment
    value is set for a child in the layout.  If one is set, the child is not
    grown and shrunk when the layout is resized, if this is possible. Aligned
    children just sit there in the place they are aligned to.  If the layout
    becomes too small, aligned windows will be shrunk as necessary and if
    possible.  Note that the MinSize() and MaxSize() of a child will affect
    how much it can be stretched when the layout is resized.

    <p>Layouts are best used to arrange the children of another window, such
    as arranging the controls of a dialog box.  When used this way, the Layout
    becomes the sole child of its parent, and contains the parent's children
    as its own.  This scheme allows Layouts to be easily nested, since all
    Layouts are Wnd-derived.  Like a Control, a Layout will forward all
    MouseWheel(), Key*(), and dragged-child notification calls to its parent.
    Clicks fall through as well, since Layouts are not constructed with the
    Wnd::INTERACTIVE flag.

    <p>There are two attributes that affect the spacing of all the layout's
    child windows: border margin and cell margin.  Border margin is the space
    left around the entire layout, between the outer edges of the children in
    the layout and the layout's outer edges.  Cell margin is the space left
    between individual Wnds in the layout, but does not add to the margin
    around the outside of the layout.

    <p>A note about how layout minimum sizes are determined: <br>The border
    margin adds to the minimum size of the layout.  Further, the cell margin
    will have an effect on the minimum size of a cell, even an empty one, if
    it is \a greater than the row or column minimum for that cell.  So an
    empty layout with 5 columns, a border margin of 3, and a cell margin of 2
    will have a minumum width of 14.  This is determined as follows: 5 columns
    means 4 cell margins between the columns, so 4 * 2 = 8.  The border margin
    is added to both sides, which means the total minimum width is 8 + 2 * 3 =
    14.  Also, the minimum size of each child in the layout will affect the
    minimum sizes of the rows and columns it covers.  If a child covers more
    than one row/column, its minimum size is distributed over the rows/columns
    it covers, proportional to the stretch factor for each row/column.
    Finally, the min values and stretch factors must both be satisfied
    simultaneously.  For example, if the row mins are set to be [1 2 3] and
    the row stretch factors are set to be [1 2 3], the minimum width will be 6
    (neglecting the margins).  However, if the mins were instead set to be [4
    2 3], the stretch factors would lead to effective minimums of [4 8 12] to
    maintain proportionality, making the minimum width 24.

    \see The Wnd documentation has further information about layouts attached
    to Wnds.
*/
class GG_API Layout : public Wnd
{
public:
    Layout(X x, Y y, X w, Y h, std::size_t rows, std::size_t columns,
           unsigned int border_margin = 0, unsigned int cell_margin = INVALID_CELL_MARGIN);

    Pt               MinUsableSize() const noexcept override { return m_min_usable_size; }

    auto             Rows() const noexcept { return m_cells.size(); }
    std::size_t      Columns() const;
    Flags<Alignment> ChildAlignment(const Wnd* wnd) const;          //! returns the aligment of child \a wnd.  \throw GG::Layout::NoSuchChild Throws if no such child exists.
    auto             BorderMargin() const noexcept { return m_border_margin; } //! returns the number of pixels that the layout will leave between its edges and the windows it contains
    auto             CellMargin() const noexcept { return m_cell_margin; }     //! returns the number of pixels the layout leaves between the edges of windows in adjacent cells
    float            RowStretch(std::size_t row) const;             //! returns the stretch factor for row \a row.  Note that \a row is not range-checked.
    float            ColumnStretch(std::size_t column) const;       //! returns the stretch factor for column \a column.  Note that \a column is not range-checked.
    Y                MinimumRowHeight(std::size_t row) const;       //! returns the minimum height allowed for row \a row.  Note that \a row is not range-checked.
    X                MinimumColumnWidth(std::size_t column) const;  //! returns the minimum height allowed for column \a column.  Note that \a column is not range-checked.

    std::vector<std::vector<const Wnd*>>Cells() const;              //! returns a matrix of the Wnds that can be found in each cell
    std::vector<std::vector<Rect>>      CellRects() const;          //! returns a matrix of rectangles in screen space that cover the cells in which child Wnds are placed
    std::vector<std::vector<Rect>>      RelativeCellRects() const;  //! returns a matrix of rectangles in layout client space that cover the cells in which child Wnds are placed

    //! Returns true iff this layout will render an outline of itself; this is
    //! sometimes useful for debugging purposes
    auto RenderOutline() const noexcept { return m_render_outline; }

    void StartingChildDragDrop(const Wnd* wnd, Pt offset) override;
    void CancellingChildDragDrop(const std::vector<const Wnd*>& wnds) override;
    void ChildrenDraggedAway(const std::vector<Wnd*>& wnds, const Wnd* destination) override;
    void SizeMove(Pt ul, Pt lr) override;
    void Render() override;

    //! Inserts \a w into the layout in the indicated cell, expanding the
    //! layout grid as necessary.  \throw GG::Layout::AttemptedOverwrite
    //! Throws if there is already a Wnd in the given cell.
    void Add(std::shared_ptr<Wnd> wnd, std::size_t row, std::size_t column, Flags<Alignment> alignment = ALIGN_NONE);

    //! Inserts \a w into the layout, covering the indicated cell(s),
    //! expanding the layout grid as necessary.  The num_rows and num_columns
    //! indicate how many rows and columns \a w covers, respectively.  So
    //! Add(foo, 1, 2, 2, 3) covers cells (1, 2) through (2, 4), inclusive.
    //! Note that \a num_rows and \a num_columns must be positive, though this
    //! is not checked. \throw GG::Layout::AttemptedOverwrite Throws if there
    //! is already a Wnd in one of the given cells.
    void Add(std::shared_ptr<Wnd> wnd, std::size_t row, std::size_t column,
             std::size_t num_rows, std::size_t num_columns, Flags<Alignment> alignment = ALIGN_NONE);

    //! Removes \a w from the layout, recalculating the layout as needed.
    //! Note that this causes the layout to relinquish responsibility for \a
    //! wnd's memory management.
    void Remove(Wnd* wnd);

    //! Resets children to their original sizes and detaches them, so that a
    //! removed Layout can leave the Wnds it lays out in their original
    //! configuration when it is no longer useful.
    void DetachAndResetChildren();

    //! Resizes the layout to be \a rows by \a columns.  If the layout
    //! shrinks, any contained windows are deleted.  Each of \a rows and \a
    //! columns must be greater than 0, though this is not checked.
    void ResizeLayout(std::size_t rows, std::size_t columns);

    //! Sets the aligment of child \a wnd to \a alignment.  If no such child
    //! exists, no action is taken.
    void SetChildAlignment(const Wnd* wnd, Flags<Alignment> alignment);

    //! Sets the number of pixels that the layout will leave between its edges
    //! and the windows it contains.
    void SetBorderMargin(unsigned int margin);

    //! Sets the number of pixels the layout leaves between the edges of
    //! windows in adjacent cells.
    void SetCellMargin(unsigned int margin);

    //! Sets the amount of stretching, relative to other rows, that \a row
    //! will do when the layout is resized.  0.0 indicates that the row's size
    //! will not change unless all rows have 0.0 stretch as well.  Note that
    //! \a row is not range-checked.
    void SetRowStretch(std::size_t row, float stretch);

    //! Sets the amount of stretching, relative to other columns, that \a
    //! column will do when the layout is resized.  0.0 indicates that the
    //! column's size will not change unless all columns have 0.0 stretch as
    //! well.  Note that \a column is not range-checked.
    void SetColumnStretch(std::size_t column, float stretch);

    //! Sets the amount of stretching for columns with matching indices as
    //! \a stretches
    void SetColumnStretches(std::vector<float> stretches);

    //! Sets the minimum height of row \a row to \a height.  Note that \a row
    //! is not range-checked.
    void SetMinimumRowHeight(std::size_t row, Y height);

    //! Sets the minimum width of column \a column to \a width.  Note that \a
    //! column is not range-checked.
    void SetMinimumColumnWidth(std::size_t column, X width);

    //! Sets the minimum width of columns with matching indices as \a widths
    void SetMinimumColumnWidths(std::vector<X> widths);

    //! Set this to true if this layout should render an outline of itself;
    //! this is sometimes useful for debugging purposes.
    void RenderOutline(bool render_outline);

    //! The base class for Layout exceptions.
    GG_ABSTRACT_EXCEPTION(Exception);

    //! Thrown when a negative margin is provided.
    GG_CONCRETE_EXCEPTION(InvalidMargin, GG::Layout, Exception);

    //! Thrown when a property of a nonexistent child is requested.
    GG_CONCRETE_EXCEPTION(NoSuchChild, GG::Layout, Exception);

    //! Thrown when an internal check of calculations made by the layout
    //! algorithm fails.
    GG_CONCRETE_EXCEPTION(FailedCalculationCheck, GG::Layout, Exception);

    //! Thrown when an attempt is made to place a Wnd in a nonempty layout
    //! cell.
    GG_CONCRETE_EXCEPTION(AttemptedOverwrite, GG::Layout, Exception);

    static constexpr unsigned int INVALID_CELL_MARGIN = std::numeric_limits<unsigned int>::max();

protected:
    void MouseWheel(Pt pt, int move, Flags<ModKey> mod_keys) override;
    void KeyPress(Key key, uint32_t key_code_point, Flags<ModKey> mod_keys) override;
    void KeyRelease(Key key, uint32_t key_code_point, Flags<ModKey> mod_keys) override;

    virtual void DoLayout(Pt ul, Pt lr);

    //! Redo the layout.  This is called internally when something changes and
    //! it needs to redo the layout.
    //!
    //! Bug:  This does nothing if the size has not changed.  Fixing it to use
    //! call DoLayout() even when the size has not changed breaks all text boxes.
    virtual void RedoLayout();

    void DetachChildCore(Wnd* wnd) override;

private:
    struct GG_API RowColParams
    {
        RowColParams() = default;

        float        stretch = 0.0f;
        unsigned int min = 0;
        unsigned int effective_min = 0; //! current effective minimum size of this row or column, based on min, layout margins, and layout cell contents
        int          current_origin = 0;//! current position of top or left side
        unsigned int current_width = 0; //! current extent in downward or rightward direction
    };

    struct GG_API WndPosition
    {
        WndPosition() = default;
        WndPosition(std::size_t first_row_, std::size_t first_column_,
                    std::size_t last_row_, std::size_t last_column_,
                    Flags<Alignment> alignment_, Pt original_ul_, Pt original_size_);

        std::size_t      first_row = 0;
        std::size_t      first_column = 0;
        std::size_t      last_row = 0;
        std::size_t      last_column = 0;
        Flags<Alignment> alignment = ALIGN_NONE;
        Pt               original_ul;
        Pt               original_size;
    };

    static float TotalStretch(const std::vector<RowColParams>& params_vec);

    X      TotalMinWidth() const;
    Y      TotalMinHeight() const;
    void   ValidateAlignment(Flags<Alignment>& alignment);
    void   ChildSizeOrMinSizeChanged();

    std::vector<std::vector<std::weak_ptr<Wnd>>>  m_cells;
    unsigned int                    m_border_margin = 1;
    unsigned int                    m_cell_margin = 1;
    std::vector<RowColParams>       m_row_params;
    std::vector<RowColParams>       m_column_params;
    std::map<Wnd*, WndPosition, std::less<>> m_wnd_positions;
    Pt                              m_min_usable_size;
    bool                            m_ignore_child_resize = false;
    bool                            m_stop_resize_recursion = false;
    bool                            m_render_outline = false;

    friend class Wnd;
};

}


#endif
