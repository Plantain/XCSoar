/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009

	M Roberts (original release)
	Robin Birch <robinb@ruffnready.co.uk>
	Samuel Gisiger <samuel.gisiger@triadis.ch>
	Jeff Goodenough <jeff@enborne.f2s.com>
	Alastair Harrison <aharrison@magic.force9.co.uk>
	Scott Penrose <scottp@dd.com.au>
	John Wharington <jwharington@gmail.com>
	Lars H <lars_hn@hotmail.com>
	Rob Dunning <rob@raspberryridgesheepfarm.com>
	Russell King <rmk@arm.linux.org.uk>
	Paolo Ventafridda <coolwind@email.it>
	Tobias Lohner <tobias@lohner-net.de>
	Mirek Jezek <mjezek@ipplc.cz>
	Max Kellermann <max@duempel.org>
	Tobias Bieniek <tobias.bieniek@gmx.de>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
*/

#include "StringUtil.hpp"
#include "Compatibility/string.h"

#include <assert.h>
#include <string.h>
#include <ctype.h>

const TCHAR *
string_after_prefix(const TCHAR *string, const TCHAR *prefix)
{
  assert(string != NULL);
  assert(prefix != NULL);

  size_t prefix_length = _tcslen(prefix);
  return _tcsncmp(string, prefix, prefix_length) == 0
    ? string + prefix_length
    : NULL;
}


const TCHAR *
string_after_prefix_ci(const TCHAR *string, const TCHAR *prefix)
{
  assert(string != NULL);
  assert(prefix != NULL);

  size_t prefix_length = _tcslen(prefix);
  return _tcsnicmp(string, prefix, prefix_length) == 0
    ? string + prefix_length
    : NULL;
}

void
TrimRight(TCHAR *p)
{
  size_t length = _tcslen(p);

  while (length > 0 && _istspace(p[length - 1]))
    --length;

  p[length] = 0;
}

TCHAR *
normalize_search_string(TCHAR *dest, const TCHAR *src)
{
  TCHAR *retval = dest;

  for (; !string_is_empty(src); ++src)
    if (_istalnum(*src))
      *dest++ = _totupper(*src);

  *dest = _T('\0');

  return retval;
}
