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

#ifndef XCSOAR_IO_DATA_FILE_HPP
#define XCSOAR_IO_DATA_FILE_HPP

#include "Source.hpp"
#include "ConvertLineReader.hpp"

class TextWriter;

/**
 * Opens a file from the data directory.
 *
 * @param name the file name relative to the data directory
 * @return a Source which must be deleted by the caller; NULL if an
 * error occurred opening the file
 */
Source<char> *
OpenDataFile(const TCHAR *name);

/**
 * Opens a text file from the data directory.
 *
 * @param name the file name relative to the data directory
 * @param cs the character set of the input file
 * @return a TLineReader which must be deleted by the caller; NULL if
 * an error occurred opening the file
 */
TLineReader *
OpenDataTextFile(const TCHAR *name,
                 ConvertLineReader::charset cs=ConvertLineReader::UTF8);

/**
 * Creates a text file in the data directory.  If the file already
 * exists, it is truncated, unless "append" is true.
 */
TextWriter *
CreateDataTextFile(const TCHAR *name, bool append=false);

#endif
