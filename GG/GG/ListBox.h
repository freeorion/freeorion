//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2003-2008 T. Zachary Laine <whatwasthataddress@gmail.com>
//!  Copyright (C) 2013-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

//! @file GG/ListBox.h
//!
//! Contains the ListBox class, a control that contains rows of other controls,
//! commonly TextControls.

#ifndef _GG_ListBox_h_
#define _GG_ListBox_h_


#include <memory>
#include <set>
#include <unordered_set>
#include <boost/optional/optional.hpp>
#include <GG/AlignmentFlags.h>
#include <GG/ClrConstants.h>
#include <GG/Control.h>
#include <GG/Timer.h>


namespace GG {

class Font;
class Scroll;
class SubTexture;
class WndEvent;

/** Styles for ListBox controls. */
GG_FLAG_TYPE(ListBoxStyle);

inline constexpr ListBoxStyle LIST_NONE            (0);        ///< Default style selected.
inline constexpr ListBoxStyle LIST_VCENTER         (1 << 0);   ///< Cells are aligned with the top of the list box control.
inline constexpr ListBoxStyle LIST_TOP             (1 << 1);   ///< Cells are aligned with the top of the list box control. This is the default.
inline constexpr ListBoxStyle LIST_BOTTOM          (1 << 2);   ///< Cells are aligned with the bottom of the list box control.
inline constexpr ListBoxStyle LIST_CENTER          (1 << 3);   ///< Cells are center-aligned.
inline constexpr ListBoxStyle LIST_LEFT            (1 << 4);   ///< Cells are left-aligned. This is the default.
inline constexpr ListBoxStyle LIST_RIGHT           (1 << 5);   ///< Cells are right-aligned.
inline constexpr ListBoxStyle LIST_NOSORT          (1 << 6);   ///< List items are not sorted. Items are sorted by default.  When used with drag-and-drop, this style allows arbitrary rearrangement of list elements by dragging.
inline constexpr ListBoxStyle LIST_SORTDESCENDING  (1 << 7);   ///< Items are sorted based on item text in descending order. Ascending order is the default.
inline constexpr ListBoxStyle LIST_NOSEL           (1 << 8);   ///< No selection, dragging, or dropping allowed.  This makes the list box effectively read-only.
inline constexpr ListBoxStyle LIST_SINGLESEL       (1 << 9);   ///< Only one item at a time can be selected. By default, multiple items may be selected.
inline constexpr ListBoxStyle LIST_QUICKSEL        (1 << 10);  ///< Each click toggles an item without affecting any others; ignored when used with LIST_SINGLESEL.
inline constexpr ListBoxStyle LIST_USERDELETE      (1 << 11);  ///< Allows user to remove selected items by pressing the delete key.
inline constexpr ListBoxStyle LIST_BROWSEUPDATES   (1 << 12);  ///< Causes a signal to be emitted whenever the mouse moves over ("browses") a row.

/** \brief A flexible control that can contain rows and columns of other
    controls, even other ListBoxes.

    A ListBox consists of rows of controls, usually text or graphics.  Each
    row represents one item; rows can be added or removed, but not columns or
    individual controls (though the individual controls can be removed from a
    row by accessing it directly).  Each Row in a ListBox must have the same
    number of cells and the same cell widths as all the others.  If you add a
    row that has fewer cells than the ListBox you are adding it to, it will be
    padded with empty cells; likewise, if it has too many cells to fit into
    the Listbox, it will have cells removed.  ListBoxes are designed to be
    easy to use in common cases, and useful in uncommon cases with only a
    little work.  Adding a row to an empty ListBox will cause the ListBox to
    take on the number of columns that the row has cells, and each column will
    have an equal portion of the ListBox's width (any remainder is placed in
    the last column).  This allows you to just add rows to a ListBox without
    worrying about setting up the ListBox in any way ahead of time.  Use
    LockColWidths() to prevent empty ListBoxes from taking on a new row's
    number of columns.  To create a ListBox with user-defined widths, use the
    ctor designed for that, or call SetNumCols(), set individual widths with
    SetColWidth(), and lock the column widths with LockColWidths(). To create a
    ListBox and manually control the column widths and alignment use
    ManuallyManageColProps() and set the number of columns with SetNumCols().
    Use DefineColWidths(), DefineColAlignments() and
    DefineColStretches() to set widths and alignments from an exemplar row.

    <br>Note that Rows are stored by pointer.  If you want to move a Row from
    one ListBox to another, use GetRow() and Insert().

    <br>Note that drag-and-drop support is a key part of ListBox's
    functionality.  As such, special effort has been made to make its use as
    natural and flexible as possible.  This includes allowing arbitrary
    reordering of ListBox rows when the LIST_NOSORT is in effect.*/
class GG_API ListBox : public Control
{
public:
    /** \brief A single item in a listbox.

        A Row is primarily a container for Controls.  Each cell in a Row
        contains pointer to a Control-derived object.  As always, each such
        Control can be connected to arbitrary functionality using signals and
        slots.  During dragging and dropping, the data type associated with a
        Row (DragDropDataType()) indicates to potential drop targets what type
        of data the Row represents; the target may accept or decline the drop
        based on the data type.  Rows are stored in ListBoxes by reference,
        not value; this means that you can subclass from Row to create your
        own custom Row types.  This is the recommended method for associating
        a row with the non-GUI object that it represents.
        Note that SortKey is not virtual; this part of its interface is used
        for compile-time polymorphism -- whatever sorter is used with a Row
        subclass must know the most-derived type of the Row subclass.
        \note The margin, column alignment, and width cell data are included
        so that each Row has all the necessary information with which to
        render itself (this is primarily needed to facilitate drag-and-drop);
        these data are duplicates of the margin, alignment, and column widths
        data found in the owning ListBox, and may be overwritten by the
        ListBox at any time. */
    struct GG_API Row : public Control
    {
        /** the type of key used to sort rows */
        using SortKeyType = const std::string&;

        Row() : Row(ListBox::DEFAULT_ROW_WIDTH, ListBox::DEFAULT_ROW_HEIGHT) {}
        Row(X w, Y h);
        ~Row() = default;

        void CompleteConstruction() override;

        /** Returns the string by which this row may be sorted. */
        virtual SortKeyType SortKey(std::size_t column) const;
        auto                size() const noexcept { return m_cells.size(); };   ///< returns the number of Controls in this Row
        bool                empty() const noexcept { return m_cells.empty(); }; ///< returns true iff there are 0 Controls in this Row

        /** Returns the Control in the \a nth cell of this Row
          * \throw std::range_error throws when size() <= \a n */
        Control*       at(std::size_t n) { return m_cells.at(n).get(); }
        const Control* at(std::size_t n) const { return m_cells.at(n).get(); }

        Alignment    RowAlignment() const noexcept { return m_row_alignment; }         ///< vertical alignment of this Row
        Alignment    ColAlignment(std::size_t n) const { return m_col_alignments[n]; } ///< horizontal alignment of the Control in the \a nth cell of this Row; not range checked
        X            ColWidth(std::size_t n) const { return m_col_widths[n]; };        ///< width of the \a nth cell of this Row; not range checked
        auto         Margin() const noexcept { return m_margin; }                      ///< amount of space left between the contents of adjacent cells, in pixels
        bool         IsNormalized() const noexcept { return m_is_normalized; }

        void         Render() override {}

        void         push_back(std::shared_ptr<Control> c); ///< adds a given Control to the end of the Row; this Control becomes property of the Row
        void         clear();               ///< removes and deletes all cells in this Row
        void         resize(std::size_t n); ///< resizes the Row to have \a n cells

        void         SetCell(std::size_t n, std::shared_ptr<Control> c);        ///< sets the Control in the \a nth cell of this Row, deleting any preexisting Control; not range checked
        void         RemoveCell(std::size_t n);                                 ///< Control in the \a nth cell of this Row, and sets the contents of the cell to 0; not range checked
        void         SetRowAlignment(Alignment align);                          ///< sets the vertical alignment of this Row
        void         SetColAlignment(std::size_t n, Alignment align);           ///< sets the horizontal alignment of the Control in the \a nth cell of this Row; not range checked
        void         SetColWidth(std::size_t n, X width);                       ///< sets the width of the \a nth cell of this Row; not range checked
        void         SetColAlignments(const std::vector<Alignment>& aligns);    ///< sets the horizontal alignment of all the Controls in this Row; not range checked
        void         ClearColAlignments();                                      ///< clears the horizontal alignments of the cells in this Row
        void         SetColWidths(const std::vector<X>& widths);                ///< sets all the widths of the cells of this Row; not range checked
        void         ClearColWidths();                                          ///< clears the minimum widths of the cells of this Row.
        void         SetColStretches(const std::vector<double>& stretches);     ///< sets all column stretches.
        void         SetMargin(unsigned int margin);                            ///< sets the amount of space left between the contents of adjacent cells, in pixels
        void         SetNormalized(bool normalized);                            ///< set normalized.  Used by ListBox to track normalization.

        boost::signals2::signal<void(Pt, GG::Flags<GG::ModKey>)> RightClickedSignal;

    protected:
        /** Add elements to m_col_widths, m_col_stretches and m_col_alignments until they reach size nn. */
        void GrowWidthsStretchesAlignmentsTo(std::size_t nn);
        void RClick(Pt pt, GG::Flags<GG::ModKey> mod) override;

        std::vector<std::shared_ptr<Control>>   m_cells;            ///< the Controls in this Row (each may be null)
        std::vector<Alignment>                  m_col_alignments;   ///< column alignments; each is one of ALIGN_TOP, ALIGN_VCENTER, or ALIGN_BOTTOM
        std::vector<X>                          m_col_widths;       ///< column widths
        std::vector<double>                     m_col_stretches;    ///< the stretch factor of each column
        unsigned int                            m_margin = DEFAULT_MARGIN;      ///< the amount of space left between the contents of adjacent cells, in pixels
        Alignment                               m_row_alignment = ALIGN_VCENTER;///< row alignment; one of ALIGN_TOP, ALIGN_VCENTER, or ALIGN_BOTTOM
        bool                                    m_ignore_adjust_layout = false;
        bool                                    m_is_normalized = false;
    };

    typedef std::list<std::shared_ptr<Row>>::iterator iterator;
    typedef std::list<std::shared_ptr<Row>>::const_iterator const_iterator;

    struct GG_API IteratorHash
    { std::size_t operator()(const iterator it) const; };
    typedef std::unordered_set<iterator, IteratorHash> SelectionSet;

    /** emitted when the list box is cleared */
    typedef boost::signals2::signal<void ()>                                   ClearedRowsSignalType;
    /** emitted when one or more rows are selected or deselected */
    typedef boost::signals2::signal<void (SelectionSet)>                       SelRowsChangedSignalType;
    /** the signature of row-change-notification signals */
    typedef boost::signals2::signal<void (iterator)>                           RowSignalType;
    /** the signature of const row-change-notification signals */
    typedef boost::signals2::signal<void (const_iterator)>                     ConstRowSignalType;
    /** the signature of row-click-notification signals */
    typedef boost::signals2::signal<void(iterator, Pt, GG::Flags<GG::ModKey>)> RowClickSignalType;
    /** the signature of row-move-notification signals */
    typedef boost::signals2::signal<void (iterator, iterator)>                 MovedRowSignalType;

    typedef RowSignalType      BeforeInsertRowSignalType;   ///< emitted before a row is inserted into the list box
    typedef RowSignalType      AfterInsertRowSignalType;    ///< emitted after a row is inserted into the list box
    typedef RowSignalType      DroppedRowSignalType;        ///< emitted when a row is inserted into the list box via drag-and-drop
    typedef ConstRowSignalType DropRowAcceptableSignalType; ///< emitted when a row may be inserted into the list box via drag-and-drop
    typedef RowSignalType      BeforeEraseRowSignalType;    ///< emitted when a row in the listbox is erased; provides the deleted Row, and is emitted before the row is removed
    typedef RowSignalType      AfterEraseRowSignalType;     ///< emitted when a row in the listbox is erased; provides the deleted Row, and is emitted after the row is removed
    typedef RowSignalType      BrowsedRowSignalType;        ///< emitted when a row in the listbox is "browsed" (rolled over) by the cursor; provides the browsed row

    static constexpr int DEFAULT_MARGIN = 2;
    static constexpr X DEFAULT_ROW_WIDTH{50};
    static constexpr Y DEFAULT_ROW_HEIGHT{22};
    static constexpr unsigned int BORDER_THICK = 2; ///< the thickness with which to render the border of the control

    ListBox(Clr color, Clr interior = CLR_ZERO);
    ~ListBox() = default;
    void CompleteConstruction() override;

    Pt MinUsableSize() const noexcept override;
    Pt ClientUpperLeft() const noexcept override;
    Pt ClientLowerRight() const noexcept override;

    bool           Empty() const noexcept { return m_rows.empty(); }
    const_iterator begin() const noexcept { return m_rows.begin(); }
    const_iterator end() const noexcept { return m_rows.end(); }
    const Row&     GetRow(std::size_t n) const;                            ///< row at index \a n; not range-checked.  \note This function is O(n).
    iterator       Caret() const noexcept { return m_caret; }              ///< row that has the caret
    const auto&    Selections() const noexcept { return m_selections; }    ///< set row indexes that is currently selected
    bool           Selected(iterator it) const;                            ///< returns true if row \a it is selected
    Clr            InteriorColor() const noexcept { return m_int_color; }  ///< color painted into the client area of the control
    Clr            HiliteColor() const noexcept { return m_hilite_color; } ///< color behind selected line items

    /** Returns the style flags of the listbox \see GG::ListBoxStyle */
    Flags<ListBoxStyle> Style() const noexcept { return m_style; }

    const Row&      ColHeaders() const { return *m_header_row; }  ///< row containing the headings for the columns, if any.  If undefined, the returned heading Row will have size() 0.
    iterator        FirstRowShown() const noexcept { return m_first_row_shown; }  ///< first row visible in the listbox
    std::size_t     FirstColShown() const noexcept { return m_first_col_shown; }  ///< index of the first column visible in the listbox

    iterator        LastVisibleRow() const; ///< last row that could be drawn, taking into account the contents and the size of client area
    std::size_t     LastVisibleCol() const; ///< index of the last column that could be drawn, taking into account the contents and the size of client area

    std::size_t     NumRows() const noexcept { return m_rows.size(); }; ///< total number of rows in the ListBox
    std::size_t     NumCols() const noexcept { return m_num_cols; }     ///< total number of columns in the ListBox

    /** Returns true iff column widths are fixed \see LockColWidths() */
    bool            KeepColWidths() const noexcept { return m_keep_col_widths; }

    /** Return true if column width and alignment are not managed by ListBox. */
    bool            ManuallyManagingColProps() const noexcept { return !m_manage_column_props; }

    /** Returns the index of the column used to sort rows, when sorting is
        enabled.  \note The sort column is not range checked when it is set by
        the user; it may be < 0 or >= NumCols(). */
    std::size_t     SortCol() const noexcept { return m_sort_col; }

    X               ColWidth(std::size_t n) const;     ///< returns the width of column \a n in pixels; not range-checked
    Alignment       ColAlignment(std::size_t n) const; ///< returns the alignment of column \a n; must be ALIGN_LEFT, ALIGN_CENTER, or ALIGN_RIGHT; not range-checked
    double          ColStretch(std::size_t n) const;   ///< Return the stretch factor of column \a n.
    Alignment       RowAlignment(iterator it) const;   ///< returns the alignment of row \a it; must be ALIGN_TOP, ALIGN_VCENTER, or ALIGN_BOTTOM; not range-checked

    /** Returns true iff \p type is allowed to be dropped over this ListBox
        when drag-and-drop is enabled. */
    bool            AllowedDropType(const std::string& type) const;

    /** Whether the list should autoscroll when the user is attempting to drop
        an item into a location that is not currently visible. */
    bool            AutoScrollDuringDragDrops() const noexcept { return m_auto_scroll_during_drag_drops; }

    /** The thickness of the area around the border of the client area that will
        provoke an auto-scroll, if AutoScrollDuringDragDrops() returns true. */
    unsigned int    AutoScrollMargin() const noexcept { return m_auto_scroll_margin; }

    /** The number of milliseconds that elapse between row/column scrolls when
        auto-scrolling. */
    unsigned int    AutoScrollInterval() const noexcept;

    /** Return true if drops are allowed.*/
    bool            AllowingDrops() const noexcept { return m_allow_drops; }


    mutable ClearedRowsSignalType        ClearedRowsSignal;        ///< the cleared signal object for this ListBox
    mutable BeforeInsertRowSignalType    BeforeInsertRowSignal;    ///< the before insert signal object for this ListBox
    mutable AfterInsertRowSignalType     AfterInsertRowSignal;     ///< the after insert signal object for this ListBox
    mutable SelRowsChangedSignalType     SelRowsChangedSignal;     ///< the selection change signal object for this ListBox
    mutable DroppedRowSignalType         DroppedRowSignal;         ///< the dropped signal object for this ListBox
    mutable DropRowAcceptableSignalType  DropRowAcceptableSignal;  ///< the drop-acceptability signal object for this ListBox
    mutable MovedRowSignalType           MovedRowSignal;           ///< the moved signal object for this ListBox
    mutable RowClickSignalType           LeftClickedRowSignal;     ///< the left click signal object for this ListBox
    mutable RowClickSignalType           RightClickedRowSignal;    ///< the right click signal object for this ListBox
    mutable RowClickSignalType           DoubleClickedRowSignal;   ///< the double click signal object for this ListBox
    mutable BeforeEraseRowSignalType     BeforeEraseRowSignal;     ///< the before erase signal object for this ListBox
    mutable AfterEraseRowSignalType      AfterEraseRowSignal;      ///< the after erase signal object for this ListBox
    mutable BrowsedRowSignalType         BrowsedRowSignal;         ///< the browsed signal object for this ListBox

    void StartingChildDragDrop(const Wnd* wnd, Pt offset) override;
    void AcceptDrops(Pt pt, std::vector<std::shared_ptr<Wnd>> wnds, Flags<ModKey> mod_keys) override;
    void ChildrenDraggedAway(const std::vector<Wnd*>& wnds, const Wnd* destination) override;
    void PreRender() override;
    void Render() override;

    /** Resizes the control, then resizes the scrollbars as needed. */
    void SizeMove(Pt ul, Pt lr) override;

    /** Show the  list box.  If \p show_children is true then show the rows that are within the
        boundaries of the list box.*/
    void Show() override;

    void Disable(bool b = true) override;
    void SetColor(Clr c) noexcept override;

    /** Insertion sorts \a row into the ListBox if sorted, or inserts into an
        unsorted ListBox before \a it; returns insertion point.  This Row
        becomes the property of this ListBox. */
    iterator Insert(std::shared_ptr<Row> row, iterator it);

    /** Insertion sorts \a row into the ListBox if sorted, or inserts into an
        unsorted ListBox at the end of the list; returns insertion point.
        This Row becomes the property of this ListBox. */
    iterator Insert(std::shared_ptr<Row> row);

    /** Insertion sorts \a rows into the ListBox if sorted, or inserts into an
        unsorted ListBox before \a it. The Rows become the property of this
        ListBox. */
    void Insert(const std::vector<std::shared_ptr<Row>>& rows, iterator it);
    void Insert(std::vector<std::shared_ptr<Row>>&& rows, iterator it);

    /** Insertion sorts \a rows into the ListBox if sorted, or inserts into an
        unsorted ListBox at the end of the list. The Rows become the property
        of this ListBox. */
    void Insert(const std::vector<std::shared_ptr<Row>>& rows);
    void Insert(std::vector<std::shared_ptr<Row>>&& rows);

    std::shared_ptr<Row>Erase(iterator it, bool signal = false);        ///< removes and returns the row that \a it points to from the ListBox, or 0 if no such row exists

    void Clear();                                        ///< empties the ListBox
    void SelectRow(iterator it, bool signal = false);    ///< selects row \a it
    void DeselectRow(iterator it, bool signal = false);  ///< deselects row \a it
    void SelectAll(bool signal = false);                 ///< selects all rows
    void DeselectAll(bool signal = false);               ///< deselects all rows
    void SetSelections(const SelectionSet& s, bool signal = false);  ///< sets the set of selected rows to \a s

    iterator    begin() noexcept;                       ///< returns an iterator to the first list row
    iterator    end() noexcept;                         ///< returns an iterator to the imaginary row one past the last one

    Row& GetRow(std::size_t n);                         ///< returns a reference to the Row at row index \a n; not range-checked.  \note This function is O(n).

    void SetCaret(iterator it);                         ///< sets the position of the caret to \a it
    void BringRowIntoView(iterator it);                 ///< moves the scrollbars so that row \a it is visible
    void SetFirstRowShown(iterator it);                 ///< moves the scrollbars so that row \a it is the first visible

    /** Sets how much to scroll when scrolled using the mousewheel. */
    void SetVScrollWheelIncrement(unsigned int increment);
    void SetHScrollWheelIncrement(unsigned int increment);

    void SetInteriorColor(Clr c) noexcept;              ///< sets the color painted into the client area of the control
    void SetHiliteColor(Clr c) noexcept;                ///< sets the color behind selected line items

    /** sets the style flags for the ListBox to \a s. \see GG::ListBoxStyle */
    void SetStyle(Flags<ListBoxStyle> s);

    void SetColHeaders(std::shared_ptr<Row> r);         ///< sets the row used as headings for the columns; this Row becomes property of the ListBox.
    void RemoveColHeaders();                            ///< removes any columns headings set

    void SetColWidth(std::size_t n, X w);               ///< sets the width of column \n to \a w; not range-checked
    void SetNumCols(std::size_t n);                     ///< sets the number of columns in the ListBox to \a n; if no column widths exist before this call, proportional widths are calulated and set, otherwise no column widths are set
    void SetSortCol(std::size_t n);                     ///< sets the index of the column used to sort rows when sorting is enabled; not range-checked

    /** Sets the comparison function used to sort a given pair of Rows during
        row sorting.  Note that \a sort_cmp is assumed to produce an ascending
        order when used to sort; setting the LIST_SORTDESCENDING style can be
        used to produce a reverse sort. */
    void SetSortCmp(std::function<bool (const Row&, const Row&, std::size_t)> sort_cmp);

    /** Fixes the column widths; by default, an empty ListBox will take on the
        number of columns of its first added row. \note The number of columns
        and their widths may still be set via SetNumCols() and SetColWidth()
        after this function has been called. */
    void LockColWidths();

    /** Allows the number of columns to be determined by the first row added to
        an empty ListBox */
    void UnLockColWidths();

    /** Set ListBox to stop managing column widths and alignment.  The number of columns must be
        set with SetColWidth(), but widths of individual rows columns or the header will not be
        managed by ListBox. */
    void ManuallyManageColProps();

    /** Sets the alignment of column \a n to \a align; not range-checked */
    void SetColAlignment(std::size_t n, Alignment align);

    /** Sets the stretch of column \a n to \a stretch; not range-checked */
    void SetColStretch(std::size_t n, double stretch);

    /** Sets the alignment of row \a it to \a align; not range-checked */
    void SetRowAlignment(iterator it, Alignment align);

    /** Sets whether to normalize rows when inserted (true) or leave them as
      * they are. */
    void NormalizeRowsOnInsert(bool enable = true);

    /** Sets the column widths from an exemplar \p row.*/
    void DefineColWidths(const Row& row);

    /** Sets the column alignments from an exemplar \p row.*/
    void DefineColAlignments(const Row& row);

    /** Sets the column alignments from an exemplar \p row.*/
    void DefineColStretches(const Row& row);

    /** Sets whether to add padding at the end of the scrolls when the ListBox is
     *  bigger than the client area, so that any row can be scrolled all the way to
     *  the top (true), or only use as much space as it needs. */
    void AddPaddingAtEnd(bool enable = true);

    /** Allow drops if \p allow is true.*/
    void AllowDrops(bool allow);

    /** Allow all drop types if \p allow is true.*/
    void AllowAllDropTypes(bool allow);

    /** Allows Rows with data type \a str to be dropped over this ListBox when
        drag-and-drop is enabled. \note Passing "" enables all drop types. */
    void AllowDropType(std::string str);
    void AllowDropType(std::string_view str) { AllowDropType(std::string{str}); }
    void AllowDropType(const char* str) { AllowDropType(std::string{str}); }

    /** Set this to determine whether the list should autoscroll when the user
        is attempting to drop an item into a location that is not currently
        visible. */
    void AutoScrollDuringDragDrops(bool auto_scroll);

    /** Sets the thickness of the area around the border of the client area that
        will provoke an auto-scroll, if AutoScrollDuringDragDrops() returns
        true. */
    void SetAutoScrollMargin(unsigned int margin);

    /** Sets the number of milliseconds that elapse between row/column scrolls
        when auto-scrolling. */
    void SetAutoScrollInterval(unsigned int interval);

protected:
    X               RightMargin() const noexcept;  ///< space skipped at right of client area for vertical scroll bar
    Y               BottomMargin() const noexcept; ///< space skipped at bottom of client area for horizontal scroll bar
    unsigned int    CellMargin() const noexcept { return m_cell_margin; }; ///< the number of pixels left between the contents of each cell and the cell boundary

    iterator        RowUnderPt(Pt pt) const; ///< returns row under pt, if any; value must be checked (i.e. it may be end())

    iterator        OldSelRow() const noexcept { return m_old_sel_row; };     ///< last row that was selected with a left-button mouse-down
    iterator        OldRDownRow() const noexcept { return m_old_rdown_row; }; ///< last row that was selected with a right-button mouse-down
    iterator        LClickRow() const noexcept { return m_lclick_row; }       ///< last row that was left-clicked
    iterator        RClickRow() const noexcept { return m_rclick_row; }       ///< last row that was right-clicked

    bool            AutoScrollingUp() const noexcept { return m_auto_scrolling_up; }       ///< returns true iff the list is being autoscrolled up due to drag-and-drop
    bool            AutoScrollingDown() const noexcept { return m_auto_scrolling_down; }   ///< returns true iff the list is being autoscrolled down due to drag-and-drop
    bool            AutoScrollingLeft() const noexcept { return m_auto_scrolling_left; }   ///< returns true iff the list is being autoscrolled left due to drag-and-drop
    bool            AutoScrollingRight() const noexcept { return m_auto_scrolling_right; } ///< returns true iff the list is being autoscrolled right due to drag-and-drop

    void KeyPress(Key key, uint32_t key_code_point, Flags<ModKey> mod_keys) override;
    void MouseWheel(Pt pt, int move, Flags<ModKey> mod_keys) override;
    void DragDropEnter(Pt pt, std::map<const Wnd*, bool>& drop_wnds_acceptable, Flags<ModKey> mod_keys) override;
    void DragDropHere(Pt pt, std::map<const Wnd*, bool>& drop_wnds_acceptable, Flags<ModKey> mod_keys) override;
    void DragDropLeave() override;
    void CancellingChildDragDrop(const std::vector<const Wnd*>& wnds) override;
    void TimerFiring(unsigned int ticks, Timer* timer) override;

    bool EventFilter(Wnd* w, const WndEvent& event) override;

    /** Define the number of columns, the column widths and alignment from \p row.*/
    /** Insertion sorts into list, or inserts into an unsorted list before
        \a it; returns insertion point. */
    iterator Insert(std::shared_ptr<Row> row, iterator it, bool dropped);

    /** Insertion sorts into list, or inserts into an unsorted list before
        \a it; returns insertion point. */
    void Insert(const std::vector<std::shared_ptr<Row>>& rows, iterator it, bool dropped);
    void Insert(std::vector<std::shared_ptr<Row>>&& rows, iterator it, bool dropped);

    std::shared_ptr<Row> Erase(iterator it, bool removing_duplicate, bool signal); ///< erases the row at index \a idx, handling it as a duplicate removal (such as for drag-and-drops within a single ListBox) if indicated
    void BringCaretIntoView();  ///< makes sure caret is visible when scrolling occurs due to keystrokes etc.
    void ResetAutoScrollVars(); ///< resets all variables related to auto-scroll to their initial values
    void Resort();              ///< performs a full resort of all rows, using the current sort functor.
    Row& ColHeaders();          ///< returns the row containing the headings for the columns, if any.  If undefined, the returned heading Row will have size() 0. non-const for derivers

    /** creates, destroys, or resizes scrolls to reflect size of data in listbox. \p force_scroll
        forces the scroll bar to be added.*/
    void AdjustScrolls(bool adjust_for_resize, std::pair<bool, bool> force_hv = {false, false});

    void DropsAcceptable(DropsAcceptableIter first, DropsAcceptableIter last,
                         Pt pt, Flags<ModKey> mod_keys) const override;
    void HandleRowRightClicked(Pt pt, Flags<ModKey> mod);

private:
    /** Show only rows that are within the visible list box area and hide all others.  If
        \p do_prerender is true then prerender the visible rows.  Return true if prerender
        resulted in any visible row changing size. */
    bool        ShowVisibleRows(bool do_prerender);
    void        ConnectSignals();
    void        ValidateStyle();                                        ///< reconciles inconsistencies in the style flags
    void        VScrolled(int tab_low, int tab_high, int low, int high);///< signals from the vertical scroll bar are caught here
    void        HScrolled(int tab_low, int tab_high, int low, int high);///< signals from the horizontal scroll bar are caught here
    void        ClickAtRow(iterator it, Flags<ModKey> mod_keys);        ///< handles to a mouse-click or spacebar-click on \a it, modified by \a keys
    void        NormalizeRow(Row* row) const;                           ///< adjusts a Row so that it has the same number of cells as other rows, and that each cell has the correct width and alignment
    iterator    FirstRowShownWhenBottomIs(iterator bottom_row);         ///< Returns the first row shown when the last row shown is \a bottom_row
    std::size_t FirstColShownWhenRightIs(std::size_t right_col, X client_width); ///< Returns the index of the first column shown when the last column shown is \a right_col

    struct SelectionCache;
    /** Cache the selected, clicked and last browsed rows.*/
    std::shared_ptr<SelectionCache> CacheSelections();
    /** Restore cached selected, clicked and last browsed rows.*/
    void RestoreCachedSelections(const SelectionCache& cache);

    /** Return the client size excluding the scroll bar sizes, in order to determine if scroll bars
        are needed. This is a private function that is a component of AdjustScrolls.*/
    Pt ClientSizeExcludingScrolls() const;

    /** Return a pair of optional X and/or Y dimensions of the scollable area iff vscroll and/or
        hscroll are required. If scrollbars are needed, the scrollable extent will be larger than the
        client size.  If a scrollbar is not required in some dimension return boost::none
        for that dimension. \p maybe_client_size might contain a precalculated client size.

        This is a private function that is a component of AdjustScrolls. */
    std::pair<boost::optional<X>, boost::optional<Y>>
        CheckIfScrollsRequired(std::pair<bool, bool> force_scrolls = {false, false},
                               const boost::optional<Pt>& maybe_client_size = boost::none) const;

    /** Add vscroll and/or hscroll if \p required_total_extents the x andor y dimension exists. The
        value of \p required_total_extents is the full x and y dimensions of the underlying ListBox
        requiring the scrollbar as calculated in CheckIfScrollsRequired.  Return a pair of bools
        indicating if vscroll and/or hscroll was added and/or removed.  \p maybe_client_size might
        contain a precalculated client size as calculated in ClientSizeExcludingScrolls.

        This is a private function that is a component of AdjustScrolls. */
    std::pair<bool, bool> AddOrRemoveScrolls(const std::pair<boost::optional<X>, boost::optional<Y>>& required_total_extents,
                                             const boost::optional<Pt>& maybe_client_size = boost::none);

    /// m_rows is mutable to enable returning end() from const functions in constant time.
    mutable std::list<std::shared_ptr<Row>> m_rows;             ///< line item data

    std::shared_ptr<Scroll> m_vscroll;          ///< vertical scroll bar on right
    std::shared_ptr<Scroll> m_hscroll;          ///< horizontal scroll bar at bottom
    unsigned int            m_vscroll_wheel_scroll_increment = 0;
    unsigned int            m_hscroll_wheel_scroll_increment = 0;

    iterator        m_caret = m_rows.end();             ///< the item currently selected, or the last item selected by the user 
    SelectionSet    m_selections;                       ///< vector of indexes of selected items
    iterator        m_old_sel_row = m_rows.end();       ///< used to make sure clicks end on the same row where they started
    bool            m_old_sel_row_selected = false;     ///< set to true if m_old_sel_row was selected at the point at which it was designated
    iterator        m_old_rdown_row = m_rows.end();     ///< the row that most recently recieved a right button down message
    iterator        m_lclick_row = m_rows.end();        ///< the row most recently left-clicked
    iterator        m_rclick_row = m_rows.end();        ///< the row most recently right-clicked
    iterator        m_last_row_browsed = m_rows.end();  ///< the last row sent out as having been browsed (used to prevent duplicate browse signals)

    GG::Pt          m_first_row_offset = {X(BORDER_THICK), Y(BORDER_THICK)};///< scrolled offset of the first row.
    iterator        m_first_row_shown = m_rows.end();   ///< index of row at top of visible area (always begin() for non-empty ListBox with LIST_NOSCROLL set)
    std::size_t     m_first_col_shown = 0;              ///< like above, but index of column at left
    std::size_t     m_num_cols = 1;                     ///< the number of columns
    std::vector<X>  m_col_widths;                       ///< the width of each of the columns goes here

    std::vector<Alignment>  m_col_alignments;               ///< the horizontal alignment of each of the columns goes here
    std::vector<double>     m_col_stretches;                ///< the stretch factor of each column
    unsigned int            m_cell_margin = DEFAULT_MARGIN; ///< the amount of space left between each edge of the cell and its contents, in pixels

    Clr                     m_int_color;                ///< color painted into the client area of the control
    Clr                     m_hilite_color = CLR_SHADOW;///< color behind selected line items
    Flags<ListBoxStyle>     m_style = LIST_NONE;        ///< composed of ListBoxStyles enums (see GUIBase.h)

    std::shared_ptr<Row>    m_header_row = Wnd::Create<Row>();  ///< row of header text/graphics
    std::size_t             m_sort_col = 0;                     ///< the index of the column data used to sort the list

    using sort_func_t = std::function<bool (const Row&, const Row&, std::size_t)>;
    sort_func_t             m_sort_cmp;                 ///< the predicate used to sort the values in the m_sort_col column of two rows

    /** Set to boost::none to allow all types.  Otherwise each string is an allowed type.*/
    boost::optional<std::unordered_set<std::string>> m_allowed_drop_types = boost::none;

    Timer           m_auto_scroll_timer{250};
    unsigned int    m_auto_scroll_margin = 8;

    bool            m_keep_col_widths = false;  ///< should we keep the column widths, once set?
    bool            m_allow_drops = false;      ///< are we accepting drops
    bool            m_auto_scroll_during_drag_drops = true;
    bool            m_auto_scrolling_up = false;
    bool            m_auto_scrolling_down = false;
    bool            m_auto_scrolling_left = false;
    bool            m_auto_scrolling_right = false;

    bool            m_normalize_rows_on_insert = true;
    bool            m_manage_column_props = true;
    bool            m_add_padding_at_end = true;

    friend class DropDownList; ///< allow complete access to DropDownList, which relies on ListBox to do its rendering
};

}

#endif
