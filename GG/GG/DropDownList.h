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

/** \file DropDownList.h \brief Contains the DropDownList class, a control
    that displays a current selection, and allows the user to select one of
    several options from a list that drops down when the control is
    clicked. */

#ifndef _GG_DropDownList_h_
#define _GG_DropDownList_h_

#include <GG/ListBox.h>
#include <GG/GLClientAndServerBuffer.h>

#include <memory>


class ModalListPicker;

namespace GG {

/** \brief Displays a single choice, and allows the user to select items from
    a drop-down list.

    DropDownList is similar to GG::ListBox, but has significant restrictions
    over the functionality of GG::ListBox.  Specifically, all list items must
    have the same height, and there is no dragging or dropping.  Though any
    Control-derived object may be placed in an item cell, the items are only
    interactive in the drop-down list; the currently selected item is
    displayed only.  Most of the ListBox interface is duplicated in
    DropDownList.  Though you can still set the alignment, etc. of individual
    rows, as in ListBox, the currently-selected row will have the same
    alignment, etc. when displayed in the control in its unopened state.  Note
    that this may look quite ugly.

    On selection DropDownList emits one of two signals, SelChangedSignal or
    SelChangedWhileDroppedSignal.  SelChangedWhileDroppedSignal is emitted
    when the selection changes while running a ModalEventPump to display the
    drop down list and handle its events.

    SelChangedSignal will also be emitted when the drop down list closes if
    the selected item changed.
*/
class GG_API DropDownList : public Control
{
public:
    /** This is a single item in a dropdown list. \see See GG::ListBox for details.*/
    typedef ListBox::Row Row;

    typedef ListBox::iterator iterator;
    typedef ListBox::const_iterator const_iterator;

    /** \name Signal Types */ ///@{
    /** emitted when a new item is selected; will be end() when no item is
      * selected */
    typedef boost::signals2::signal<void (iterator)>   SelChangedSignalType;

    /** Signal \a true when drop down opens and false when it closes.*/
    typedef boost::signals2::signal<void (bool)>       DropDownOpenedSignalType;
    //@}

    /** \name Structors */ ///@{
    /** basic ctor.  DropDownList retains ownership of \a lb, if it is non-null. */
    DropDownList(size_t num_shown_elements, Clr color);
    ~DropDownList();
    //@}

    /** \name Accessors */ ///@{
    iterator        CurrentItem() const;            ///< returns the currently selected list item (returns end() if none is selected)
    std::size_t     CurrentItemIndex() const;       ///< returns the position of the currently selected list item within the list (returns -1 if none is selected)

    std::size_t     IteratorToIndex(iterator it) const;     ///< returns the position of \a it within the list (returns -1 if \a it == end())
    iterator        IndexToIterator(std::size_t n) const;   ///< returns an iterator to the row in position \a n (returns end() if \a n is an invalid index)

    bool            Empty() const;                  ///< returns true when the list is empty

    const_iterator  begin() const;                  ///< returns an iterator to the first list row
    const_iterator  end() const;                    ///< returns an iterator to the imaginary row one past the last

    const Row&      GetRow(std::size_t n) const;    ///< returns a const reference to the row at index \a n; not range-checked.  \note This function is O(n).
    bool            Selected(iterator it) const;    ///< returns true if row \a it is selected
    bool            Selected(std::size_t n) const;  ///< returns true if row at position \a n is selected
    Clr             InteriorColor() const;          ///< returns the color painted into the client area of the control

    Y               DropHeight() const;             ///< returns the height of the drop-down list
    bool            Dropped() const;                ///< Return true if the drop down list is open.

    /** Returns the style flags of the list \see GG::ListBoxStyle */
    Flags<ListBoxStyle> Style() const;

    std::size_t     NumRows() const;          ///< returns the total number of items in the list
    std::size_t     NumCols() const;          ///< returns the total number of columns in each list item

    /** Returns the index of the column used to sort items, when sorting is
        enabled.  \note The sort column is not range checked when it is set by
        the user; it may be >= NumCols(). */
    std::size_t     SortCol() const;

    X               ColWidth(std::size_t n) const;     ///< returns the width of column \a n in pixels; not range-checked
    Alignment       ColAlignment(std::size_t n) const; ///< returns the alignment of column \a n; must be LIST_LEFT, LIST_CENTER, or LIST_RIGHT; not range-checked
    Alignment       RowAlignment(iterator it) const;   ///< returns the alignment of row \a n; must be LIST_TOP, LIST_VCENTER, or LIST_BOTTOM; not range-checked

    Pt ClientUpperLeft() const override;
    Pt ClientLowerRight() const override;

    /** Return the width of the displayed row.  Override this function if the displayed row is a
        different width than the client width.*/
    virtual GG::X DisplayedRowWidth() const;

    /** Return the width of the dropped row.  Override this function if the dropped row is a
        different width than the client width.*/
    virtual GG::X DroppedRowWidth() const;

    /** The selection change signal while not running the modal drop down box.
        This will also signal an event when the drop list closes if the selection changed.
     */
    mutable SelChangedSignalType SelChangedSignal;
    /** The selection change signal while running the modal drop down box.*/
    mutable SelChangedSignalType SelChangedWhileDroppedSignal;

    DropDownOpenedSignalType DropDownOpenedSignal;
    //@}

    /** \name Mutators */ ///@{
    void PreRender() override;
    void Render() override;
    /** Resizes the control, ensuring the proper height is maintained based on
        the list's row height. */
    void SizeMove(const Pt& ul, const Pt& lr) override;
    void SetColor(Clr c) override;

    /** Insertion sorts \a row into a sorted list, or inserts into an unsorted
        list before \a it; returns index of insertion point.  This Row becomes
        the property of the DropDownList and should not be deleted or inserted
        into any other DropDownLists */
    iterator Insert(std::shared_ptr<Row> row, iterator it);

    /** Insertion sorts \a row into a sorted list, or inserts into an unsorted
        list at the end of the list; returns index of insertion point.  This
        Row becomes the property of the DropDownList and should not be deleted
        or inserted into any other DropDownLists */
    iterator Insert(std::shared_ptr<Row> row);

    /** Insertion sorts \a rows into a sorted list, or inserts into an unsorted
        list before \a it. The Rows become the property of this DropDownList. */
    void Insert(const std::vector<std::shared_ptr<Row>>& rows, iterator it);

    /** Insertion sorts \a rows into sorted list, or inserts into an unsorted
        list at the end of the list. The Rows become the property of thiis
        DropDownList. */
    void Insert(const std::vector<std::shared_ptr<Row>>& rows);

    std::shared_ptr<Row> Erase(iterator it, bool signal = false); ///< removes and returns \a it from the list, or 0 if no such row exists
    void Clear();                        ///< empties the list

    iterator begin();                    ///< returns an iterator to the first list row
    iterator end();                      ///< returns an iterator to the imaginary row one past the last one

    Row& GetRow(std::size_t n);          ///< returns a reference to the Row at row index \a n; not range-checked.  \note This function is O(n).

    void Select(iterator it);            ///< selects row-item \a it in the list
    void Select(std::size_t n);          ///< selects row-item \a it in the list

    void SetInteriorColor(Clr c);        ///< sets the color painted into the client area of the control

    /** sets the style flags for the list to \a s (invalidates currently
        selected item). \see GG::ListBoxStyle */
    void SetStyle(Flags<ListBoxStyle> s);

    void SetNumCols(std::size_t n);      ///< sets the number of columns in each list item to \a n; if no column widths exist before this call, proportional widths are calulated and set, otherwise no column widths are set
    void SetSortCol(std::size_t n);      ///< sets the index of the column used to sort rows when sorting is enabled (invalidates currently selected item); not range-checked
    void SetColWidth(std::size_t n, X w);///< sets the width of column \n to \a w; not range-checked

    /** Fixes the column widths; by default, an empty list will take on the
        number of columns of its first added row. \note The number of columns
        and their widths may still be set via SetNumCols() and SetColWidth()
        after this function has been called. */
    void LockColWidths();

    /** Allows the number of columns to be determined by the first row added
        to an empty ListBox */
    void UnLockColWidths();

    /** Set ListBox to stop managing column widths and alignment.  The number of columns must be
        set with SetColWidth(), but widths of individual rows columns or the header will not be
        managed by ListBox. */
    void ManuallyManageColProps();

    void SetColAlignment(std::size_t n, Alignment align); ///< sets the alignment of column \a n to \a align; not range-checked
    void SetRowAlignment(iterator it, Alignment align);   ///< sets the alignment of the Row at row index \a n to \a align; not range-checked

    /** Sets the stretch of column \a n to \a stretch; not range-checked */
    void SetColStretch(std::size_t n, double stretch);

    /** Sets whether to normalize rows when inserted (true) or leave them as
      * they are. */
    void NormalizeRowsOnInsert(bool enable = true);

    /** Set the drop down list to only mouse scroll if it is dropped. */
    void SetOnlyMouseScrollWhenDropped(bool enable);
    //@}

protected:
    /** \name Mutators */ ///@{
    void LButtonDown(const Pt& pt, Flags<ModKey> mod_keys) override;
    void KeyPress(Key key, std::uint32_t key_code_point, Flags<ModKey> mod_keys) override;
    void MouseWheel(const Pt& pt, int move, Flags<ModKey> mod_keys) override;

    ListBox*        LB();                ///< returns the ListBox used to render the selected row and the popup list

    virtual void InitBuffer();
    virtual void RenderDisplayedRow();

    GL2DVertexBuffer    m_buffer;
    //@}

private:
    const ListBox*  LB() const;

    const std::shared_ptr<ModalListPicker> m_modal_picker;
};

} // namespace GG

#endif
