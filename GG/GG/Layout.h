// -*- C++ -*-
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

/** \file Layout.h \brief Contains the Layout class, which is used to size and
    align GG windows. */

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
    /** \name Structors */ ///@{
    /** Ctor. */
    Layout(X x, Y y, X w, Y h, std::size_t rows, std::size_t columns,
           unsigned int border_margin = 0, unsigned int cell_margin = INVALID_CELL_MARGIN);
    //@}

    /** \name Accessors */ ///@{
    Pt MinUsableSize() const override;

    std::size_t      Rows() const;                             ///< returns the number of rows in the layout
    std::size_t      Columns() const;                          ///< returns the number of columns in the layout
    Flags<Alignment> ChildAlignment(const Wnd* wnd) const;     ///< returns the aligment of child \a wnd.  \throw GG::Layout::NoSuchChild Throws if no such child exists.
    unsigned int     BorderMargin() const;                     ///< returns the number of pixels that the layout will leave between its edges and the windows it contains
    unsigned int     CellMargin() const;                       ///< returns the number of pixels the layout leaves between the edges of windows in adjacent cells
    double           RowStretch(std::size_t row) const;        ///< returns the stretch factor for row \a row.  Note that \a row is not range-checked.
    double           ColumnStretch(std::size_t column) const;  ///< returns the stretch factor for column \a column.  Note that \a column is not range-checked.
    Y                MinimumRowHeight(std::size_t row) const;  ///< returns the minimum height allowed for row \a row.  Note that \a row is not range-checked.
    X                MinimumColumnWidth(std::size_t column) const; ///< returns the minimum height allowed for column \a column.  Note that \a column is not range-checked.
    std::vector<std::vector<const Wnd*>>
                     Cells() const;                            ///< returns a matrix of the Wnds that can be found in each cell
    std::vector<std::vector<Rect>>
                     CellRects() const;                        ///< returns a matrix of rectangles in screen space that cover the cells in which child Wnds are placed
    std::vector<std::vector<Rect>>
                     RelativeCellRects() const;                ///< returns a matrix of rectangles in layout client space that cover the cells in which child Wnds are placed

    /** Returns true iff this layout will render an outline of itself; this is
        sometimes useful for debugging purposes */
    bool   RenderOutline() const;

    /** Returns the outline color used to render this layout (this is only
        used if RenderOutline() returns true).  This is sometimes useful for
        debugging purposes. */
    Clr    OutlineColor() const;
    //@}

    /** \name Mutators */ ///@{
    void StartingChildDragDrop(const Wnd* wnd, const Pt& offset) override;
    void CancellingChildDragDrop(const std::vector<const Wnd*>& wnds) override;
    void ChildrenDraggedAway(const std::vector<Wnd*>& wnds, const Wnd* destination) override;
    void SizeMove(const Pt& ul, const Pt& lr) override;
    void Render() override;

    /** Inserts \a w into the layout in the indicated cell, expanding the
        layout grid as necessary.  \throw GG::Layout::AttemptedOverwrite
        Throws if there is already a Wnd in the given cell. */
    void Add(std::shared_ptr<Wnd> wnd, std::size_t row, std::size_t column, Flags<Alignment> alignment = ALIGN_NONE);

    /** Inserts \a w into the layout, covering the indicated cell(s),
        expanding the layout grid as necessary.  The num_rows and num_columns
        indicate how many rows and columns \a w covers, respectively.  So
        Add(foo, 1, 2, 2, 3) covers cells (1, 2) through (2, 4), inclusive.
        Note that \a num_rows and \a num_columns must be positive, though this
        is not checked. \throw GG::Layout::AttemptedOverwrite Throws if there
        is already a Wnd in one of the given cells. */
    void Add(std::shared_ptr<Wnd> wnd, std::size_t row, std::size_t column, std::size_t num_rows, std::size_t num_columns, Flags<Alignment> alignment = ALIGN_NONE);

    /** Removes \a w from the layout, recalculating the layout as needed.
        Note that this causes the layout to relinquish responsibility for \a
        wnd's memory management. */
    void Remove(Wnd* wnd);

    /** Resets children to their original sizes and detaches them, so that a
        removed Layout can leave the Wnds it lays out in their original
        configuration when it is no longer useful. */
    void DetachAndResetChildren();

    /** Resizes the layout to be \a rows by \a columns.  If the layout
        shrinks, any contained windows are deleted.  Each of \a rows and \a
        columns must be greater than 0, though this is not checked. */
    void ResizeLayout(std::size_t rows, std::size_t columns);

    /** Sets the aligment of child \a wnd to \a alignment.  If no such child
        exists, no action is taken. */
    void SetChildAlignment(const Wnd* wnd, Flags<Alignment> alignment);

    /** Sets the number of pixels that the layout will leave between its edges
        and the windows it contains */
    void SetBorderMargin(unsigned int margin);

    /** Sets the number of pixels the layout leaves between the edges of
        windows in adjacent cells */
    void SetCellMargin(unsigned int margin);

    /** Sets the amount of stretching, relative to other rows, that \a row
        will do when the layout is resized.  0.0 indicates that the row's size
        will not change unless all rows have 0.0 stretch as well.  Note that
        \a row is not range-checked. */
    void SetRowStretch(std::size_t row, double stretch);

    /** Sets the amount of stretching, relative to other columns, that \a
        column will do when the layout is resized.  0.0 indicates that the
        column's size will not change unless all columns have 0.0 stretch as
        well.  Note that \a column is not range-checked. */
    void SetColumnStretch(std::size_t column, double stretch);

    /** Sets the minimum height of row \a row to \a height.  Note that \a row
        is not range-checked. */
    void SetMinimumRowHeight(std::size_t row, Y height);

    /** Sets the minimum width of column \a column to \a width.  Note that \a
        column is not range-checked. */
    void SetMinimumColumnWidth(std::size_t column, X width);

    /** Set this to true if this layout should render an outline of itself;
        this is sometimes useful for debugging purposes */
    void RenderOutline(bool render_outline);

    /** Sets the outline color used to render this layout (this is only used
        if RenderOutline() returns true).  This is sometimes useful for
        debugging purposes. */
    void SetOutlineColor(Clr color);
    //@}

    /** \name Exceptions */ ///@{
    /** The base class for Layout exceptions. */
    GG_ABSTRACT_EXCEPTION(Exception);

    /** Thrown when a negative margin is provided. */
    GG_CONCRETE_EXCEPTION(InvalidMargin, GG::Layout, Exception);

    /** Thrown when a property of a nonexistent child is requested. */
    GG_CONCRETE_EXCEPTION(NoSuchChild, GG::Layout, Exception);

    /** Thrown when an internal check of calculations made by the layout
        algorithm fails. */
    GG_CONCRETE_EXCEPTION(FailedCalculationCheck, GG::Layout, Exception);

    /** Thrown when an attempt is made to place a Wnd in a nonempty layout
        cell. */
    GG_CONCRETE_EXCEPTION(AttemptedOverwrite, GG::Layout, Exception);
    //@}

    static const unsigned int INVALID_CELL_MARGIN;

protected:
    /** \name Mutators */ ///@{
    void MouseWheel(const Pt& pt, int move, Flags<ModKey> mod_keys) override;
    void KeyPress(Key key, std::uint32_t key_code_point, Flags<ModKey> mod_keys) override;
    void KeyRelease(Key key, std::uint32_t key_code_point, Flags<ModKey> mod_keys) override;

    virtual void DoLayout(Pt ul, Pt lr);

    /** Redo the layout.  This is called internally when something changes and
        it needs to redo the layout.

        Bug:  This does nothing if the size has not changed.  Fixing it to use
        call DoLayout() even when the size has not changed breaks all text boxes.
    */
    virtual void RedoLayout();
    //@}

private:
    struct GG_API RowColParams
    {
        RowColParams();

        double       stretch;
        unsigned int min;
        unsigned int effective_min;   ///< current effective minimum size of this row or column, based on min, layout margins, and layout cell contents
        int          current_origin;  ///< current position of top or left side
        unsigned int current_width;   ///< current extent in downward or rightward direction
    };

    struct GG_API WndPosition
    {
        WndPosition();
        WndPosition(std::size_t first_row_, std::size_t first_column_,
                    std::size_t last_row_, std::size_t last_column_,
                    Flags<Alignment> alignment_, const Pt& original_ul_, const Pt& original_size_);

        std::size_t      first_row;
        std::size_t      first_column;
        std::size_t      last_row;
        std::size_t      last_column;
        Flags<Alignment> alignment;
        Pt               original_ul;
        Pt               original_size;
    };

    double TotalStretch(const std::vector<RowColParams>& params_vec) const;
    X      TotalMinWidth() const;
    Y      TotalMinHeight() const;
    void   ValidateAlignment(Flags<Alignment>& alignment);
    void   ChildSizeOrMinSizeChanged();

    std::vector<std::vector<std::weak_ptr<Wnd>>>  m_cells;
    unsigned int                    m_border_margin;
    unsigned int                    m_cell_margin;
    std::vector<RowColParams>       m_row_params;
    std::vector<RowColParams>       m_column_params;
    std::map<Wnd*, WndPosition>     m_wnd_positions;
    Pt                              m_min_usable_size;
    bool                            m_ignore_child_resize;
    bool                            m_stop_resize_recursion;
    bool                            m_render_outline;
    Clr                             m_outline_color;

    friend class Wnd;
};

} // namespace GG

#endif
