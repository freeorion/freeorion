/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Declaration and implementation of list-based stacks
 *        (StackPure<E> and Stack<E>).
 * 
 * \author Carsten Gutwenger
 * 
 * \par License:
 * This file is part of the Open Graph Drawing Framework (OGDF).
 *
 * Copyright (C). All rights reserved.
 * See README.txt in the root directory of the OGDF installation for details.
 * 
 * \par
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * Version 2 or 3 as published by the Free Software Foundation
 * and appearing in the files LICENSE_GPL_v2.txt and
 * LICENSE_GPL_v3.txt included in the packaging of this file.
 *
 * \par
 * In addition, as a special exception, you have permission to link
 * this software with the libraries of the COIN-OR Osi project
 * (http://www.coin-or.org/projects/Osi.xml), all libraries required
 * by Osi, and all LP-solver libraries directly supported by the
 * COIN-OR Osi project, and distribute executables, as long as
 * you follow the requirements of the GNU General Public License
 * in regard to all of the software in the executable aside from these
 * third-party libraries.
 * 
 * \par
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * \par
 * You should have received a copy of the GNU General Public 
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 * 
 * \see  http://www.gnu.org/copyleft/gpl.html
 ***************************************************************/


#ifdef _MSC_VER
#pragma once
#endif

#ifndef OGDF_STACK_H
#define OGDF_STACK_H


#include <ogdf/basic/SList.h>


namespace ogdf {


////! The parameterized class \a StackPure<E> implements list-based stacks
///**
// * In contrast to Stack<E>, instances of \a StackPure<E> do not store the 
// * number of elements contained in the stack.
// */
//template<class E> class StackPure : private SListPure<E> {
//public:
//	//! Constructs an empty stack.
//	StackPure() { }
//
//	//! Constructs a stack that is a copy of \a S.
//	StackPure(const StackPure<E> &S) : SListPure<E>(S) { }
//
//	// destruction
//	~StackPure() { }
//
//	//! Returns true iff the stack is empty.
//	bool empty() const { return SListPure<E>::empty(); }
//
//	//! Returns a reference to the top element.
//	const E &top() const {
//		return SListPure<E>::front();
//	}
//
//	//! Returns a reference to the top element.
//	E &top() {
//		return SListPure<E>::front();
//	}
//
//	//! Assignment operator.
//	StackPure<E> &operator=(const StackPure<E> &S) {
//		SListPure<E>::operator=(S);
//		return *this;
//	}
//
//	//! Adds element \a x as top-most element to the stack.
//	SListIterator<E> push(const E &x) {
//		return SListPure<E>::pushFront(x);
//	}
//
//	//! Removes the top-most element from the stack and returns it.
//	E pop() {
//		E x = top();
//		SListPure<E>::popFront();
//		return x;
//	}
//
//	//! Makes the stack empty.
//	void clear() { SListPure<E>::clear(); }
//
//	const SListPure<E> &getListPure() const { return *this; }
//
//	OGDF_NEW_DELETE
//}; // class StackPure
//
//
////! The parameterized class \a Stack<E> implements list-based stacks
///**
// * In contrast to StackPure<E>, instances of \a Stack<E> store the 
// * number of elements contained in the stack.
// */
//template<class E> class Stack : private SList<E> {
//public:
//	//! Constructs an empty stack.
//	Stack() { }
//
//	//! Constructs a stack that is a copy of \a S.
//	Stack(const Stack<E> &S) : SList<E>(S) { }
//
//	// destruction
//	~Stack() { }
//
//	//! Returns true iff the stack is empty.
//	bool empty() const { return SList<E>::empty(); }
//	
//	//! Returns the number of elements contained in the stack.
//	int size() const { return SList<E>::size(); }
//
//	//! Returns a reference to the top element.
//	const E &top() const {
//		return SList<E>::front();
//	}
//
//	//! Returns a reference to the top element.
//	E &top() {
//		return SList<E>::front();
//	}
//
//	//! Assignment operator.
//	Stack<E> &operator=(const Stack<E> &S) {
//		SList<E>::operator=(S);
//		return *this;
//	}
//
//	//! Adds element \a x as top-most element to the stack.
//	SListIterator<E> push(const E &x) {
//		return SList<E>::pushFront(x);
//	}
//
//	//! Removes the top-most element from the stack and returns it.
//	E pop() {
//		E x = top();
//		SList<E>::popFront();
//		return x;
//	}
//
//	//! Makes the stack empty.
//	void clear() { SList<E>::clear(); }
//
//	const SList<E> &getList() const { return *this; }
//
//	OGDF_NEW_DELETE
//}; // class Stack
//
//
//// output
//template<class E>
//void print(ostream &os, const StackPure<E> &S, char delim = ' ')
//{ print(os,S.getListPure(),delim); }
//
//template<class E>
//void print(ostream &os, const Stack<E> &S, char delim = ' ')
//{ print(os,S.getListPure(),delim); }
//
//
//// output operator
//template<class E>
//ostream &operator<<(ostream &os, const StackPure<E> &S)
//{
//	print(os,S); return os;
//}
//
//template<class E>
//ostream &operator<<(ostream &os, const Stack<E> &S)
//{
//	print(os,S); return os;
//}
//


//! List-based stacks.
/**
 * In contrast to Stack<E>, instances of \a StackPure<E> do not store the 
 * number of elements contained in the stack.
 */
template<class E> class StackPure
{
	struct Element {
		Element(const E &x, Element *pNext) : m_next(pNext), m_x(x) { }
		Element *m_next;
		E        m_x;
		OGDF_NEW_DELETE
	};

	Element *m_head;

public:
	//! Constructs an empty stack.
	StackPure() { m_head = 0; }

	//! Constructs a stack that is a copy of \a S.
	StackPure(const StackPure<E> &S)  {
		m_head = 0;
		copy(S);
	}

	// destruction
	~StackPure() {
		clear();
	}

	//! Returns true iff the stack is empty.
	bool empty() const { return m_head == 0; }

	//! Returns a reference to the top element.
	const E &top() const {
		return m_head->m_x;
	}

	//! Returns a reference to the top element.
	E &top() {
		return m_head->m_x;
	}

	//! Assignment operator.
	StackPure<E> &operator=(const StackPure<E> &S) {
		clear();
		copy(S);
		return *this;
	}

	//! Adds element \a x as top-most element to the stack.
	void push(const E &x) {
		m_head = OGDF_NEW Element(x,m_head);
	}

	//! Removes the top-most element from the stack and returns it.
	E pop() {
		OGDF_ASSERT(m_head != 0)
		Element *pX = m_head;
		m_head = m_head->m_next;
		E x = pX->m_x;
		delete pX;
		return x;
	}

	//! Makes the stack empty.
	void clear() {
		while(m_head) {
			Element *pX = m_head;
			m_head = m_head->m_next;
			delete pX;
		}
	}

	void print(ostream &os, char delim = ' ') const
	{
		Element *pX = m_head;
		if (pX != 0) {
			os << pX->m_x;
			for(pX = pX->m_next; pX != 0; pX = pX->m_next)
				os << delim << pX->m_x;
		}
	}

private:
	void copy(const StackPure<E> &S) {
		Element **p = &m_head;
		
		for(Element *q = S.m_head; q != 0; q = q->m_next) {
			*p = OGDF_NEW Element(q->m_x,0);
			p = &(*p)->m_next;
		}
	}

	OGDF_NEW_DELETE
}; // class StackPure


//! The parameterized class \a Stack<E> implements list-based stacks
/**
 * In contrast to StackPure<E>, instances of \a Stack<E> store the 
 * number of elements contained in the stack.
 */
template<class E> class Stack : private StackPure<E>
{
	int m_count; //! The number of elements in the list.

public:
	//! Constructs an empty stack.
	Stack() { m_count = 0; }

	//! Constructs a stack that is a copy of \a S.
	Stack(const Stack<E> &S) : StackPure<E>(S) { m_count = S.m_count; }

	// destruction
	~Stack() { }

	//! Returns true iff the stack is empty.
	bool empty() const { return StackPure<E>::empty(); }
	
	//! Returns the number of elements contained in the stack.
	int size() const { return m_count; }

	//! Returns a reference to the top element.
	const E &top() const {
		return StackPure<E>::top();
	}

	//! Returns a reference to the top element.
	E &top() {
		return StackPure<E>::top();
	}

	//! Assignment operator.
	Stack<E> &operator=(const Stack<E> &S) {
		StackPure<E>::operator=(S);
		m_count = S.m_count;
		return *this;
	}

	//! Adds element \a x as top-most element to the stack.
	void push(const E &x) {
		++m_count;
		return StackPure<E>::push(x);
	}

	//! Removes the top-most element from the stack and returns it.
	E pop() {
		--m_count;
		return StackPure<E>::pop();
	}

	//! Makes the stack empty.
	void clear() {
		StackPure<E>::clear();
		m_count = 0;
	}

	void print(ostream &os, char delim = ' ') const {
		StackPure<E>::print(os,delim);
	}

	OGDF_NEW_DELETE
}; // class Stack



template<class E>
ostream &operator<<(ostream &os, const StackPure<E> &S)
{
	S.print(os);
	return os;
}


template<class E>
ostream &operator<<(ostream &os, const Stack<E> &S)
{
	S.print(os);
	return os;
}

} // end namespace ogdf


#endif
