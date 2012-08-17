/* 
 * Some (broken) DevIL libs require a `_vsnprintf'-function instead of
 * vsnprintf, so we'll wrap it
 */
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifndef HAVE__VSNPRINTF

#include <stdio.h>
#include <stdarg.h>

int _vsnprintf(char* str, size_t size, const char* format, va_list ap)
{
    return vsnprintf(str, size, format, ap);
}

#endif
