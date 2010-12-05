/*
 * $Revision: 2027 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2010-09-01 11:55:17 +0200 (Wed, 01 Sep 2010) $ 
 ***************************************************************/
 
/** \file
 * \brief Implementation of class String
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



#include <ogdf/basic/String.h>
#include <ogdf/basic/Hashing.h>
#include <stdarg.h>
#include <string.h>


namespace ogdf {


char String::s_pBuffer[OGDF_STRING_BUFFER_SIZE];


String::String()
{
	m_pChar = new char[1];
	if (m_pChar == 0) OGDF_THROW(InsufficientMemoryException);

	m_pChar[0] = 0;
	m_length = 0;
}


String::String(const char c)
{
    m_length = 1;
	m_pChar = new char[2];
	if (m_pChar == 0) OGDF_THROW(InsufficientMemoryException);

    m_pChar[0] = c;
    m_pChar[1] = '\0';
}


String::String(const char *str)
{
	m_length = strlen(str);
	m_pChar = new char[m_length+1];
	if (m_pChar == 0) OGDF_THROW(InsufficientMemoryException);

	ogdf::strcpy(m_pChar,m_length+1,str);
}


String::String(size_t maxLen, const char *str)
{
    m_length = maxLen;
	m_pChar = new char[m_length+1];
	if (m_pChar == 0) OGDF_THROW(InsufficientMemoryException);

	ogdf::strncpy(m_pChar, m_length+1, str, m_length);
    m_pChar[m_length] = '\0';
}

/*
String::String(const char *format, ...)
{
	va_list argList;
	va_start(argList,format);

	m_length = vsprintf(s_pBuffer,format,argList);
	m_pChar = new char[m_length+1];
	if (m_pChar == 0) OGDF_THROW(InsufficientMemoryException);

	strcpy(m_pChar,s_pBuffer);
}
*/

String::String(const String &str)
{
	m_length = str.m_length;
	m_pChar = new char[m_length+1];
	if (m_pChar == 0) OGDF_THROW(InsufficientMemoryException);

	ogdf::strcpy(m_pChar,m_length+1,str.m_pChar);
}


String::~String()
{
	delete [] m_pChar;
}


String &String::operator =(const String &str)
{
	if (&str == this) return *this;

	delete [] m_pChar;

	m_length = str.m_length;
	m_pChar = new char[m_length+1];
	if (m_pChar == 0) OGDF_THROW(InsufficientMemoryException);

	ogdf::strcpy(m_pChar,m_length+1,str.m_pChar);

	return *this;
}


String &String::operator =(const char *str)
{
	delete [] m_pChar;

	m_length = strlen(str);
	m_pChar = new char[m_length+1];
	if (m_pChar == 0) OGDF_THROW(InsufficientMemoryException);

	ogdf::strcpy(m_pChar,m_length+1,str);

	return *this;
}


String &String::operator +=(const String &str)
{
	size_t oldLength = m_length;
	char *pOldChar = m_pChar;

	m_length += str.m_length;
	m_pChar = new char[m_length+1];
	if (m_pChar == 0) {
		delete [] pOldChar;
		OGDF_THROW(InsufficientMemoryException);
	}
	
	ogdf::strcpy(m_pChar,m_length+1,pOldChar);
	ogdf::strcpy(m_pChar+oldLength,m_length+1-oldLength,str.m_pChar);

	delete [] pOldChar;

	return *this;
}


void String::sprintf(const char *format, ...)
{
	delete [] m_pChar;

	va_list argList;
	va_start(argList,format);

	m_length = ogdf::vsprintf(s_pBuffer,OGDF_STRING_BUFFER_SIZE,format,argList);
	m_pChar = new char[m_length+1];
	if (m_pChar == 0) OGDF_THROW(InsufficientMemoryException);

	ogdf::strcpy(m_pChar,m_length+1,s_pBuffer);
}


int String::compare (const String &x, const String &y)
{
	return strcmp(x.m_pChar, y.m_pChar);
}


istream& operator>>(istream& is, String &str)
{
	is >> String::s_pBuffer;
	str = String::s_pBuffer;
	return is;
}

int DefHashFunc<String>::hash(const String &key) const
{
	int hashValue = 0;
	const char *pChar = key.cstr();

	while (*pChar)
		hashValue += int(*pChar++);

	return hashValue;
}


} // end namespace ogdf
