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

#include "MOFile.hpp"

#include <assert.h>

MOFile::MOFile(const TCHAR *path)
  :mapping(path), count(0) {
  if (mapping.error())
    return;

  const struct mo_header *header = (const struct mo_header *)mapping.data();
  if (mapping.size() < sizeof(*header))
    return;

  if (header->magic == 0x950412de)
    native_byte_order = true;
  else if (header->magic == 0xde120495)
    native_byte_order = false;
  else
    /* invalid magic */
    return;

  unsigned n = import_uint32(header->num_strings);
  if (n >= 0x100000)
    return;

  strings.resize_discard(n);

  const struct mo_table_entry *entry = (const struct mo_table_entry *)
    mapping.at(import_uint32(header->original_table_offset));
  for (unsigned i = 0; i < n; ++i) {
    strings[i].original = get_string(entry++);
    if (strings[i].original == NULL)
      return;
  }

  entry = (const struct mo_table_entry *)
    mapping.at(import_uint32(header->translation_table_offset));
  for (unsigned i = 0; i < n; ++i) {
    strings[i].translation = get_string(entry++);
    if (strings[i].translation == NULL)
      return;
  }

  count = n;
}

const char *
MOFile::lookup(const char *p) const
{
  assert(p != NULL);

  for (unsigned i = 0; i < count; ++i)
    if (strcmp(strings[i].original, p) == 0)
      return strings[i].translation;

  return NULL;
}

const char *
MOFile::get_string(const struct mo_table_entry *entry) const
{
  unsigned length = import_uint32(entry->length);
  unsigned offset = import_uint32(entry->offset);

  if (offset >= mapping.size() || length >= mapping.size() ||
      (offset + length) >= mapping.size())
    /* overflow */
    return NULL;

  const char *p = (const char *)mapping.at(offset);
  if (p[length] != 0 || strlen(p) != length)
    /* invalid string */
    return NULL;

  return p;
}
