/* Copyright_License {

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

#ifndef __MD5__
#define __MD5__

#include <stdint.h>

class MD5
{
public:
  enum {
    DIGEST_LENGTH = 16,
  };

private:
  unsigned char buff512bits[64];
  uint32_t a, b, c, d;
  uint32_t h0, h1, h2, h3;
  uint32_t f, g;
  uint32_t MessageLenBits; // max message size=536,870,912 because of 32-bit length tracking (MD5 standard is 64-bits)

  void Process512(const unsigned char * s512in);

public:

  void InitKey(uint32_t h0in, uint32_t h1in, uint32_t h2in, uint32_t h3in);

  void InitDigest(void);
  void AppendString(const unsigned char *sin, int bSkipWhiteSpaceFlag); // must be NULL-terminated string!
  void Finalize(void);
  int GetDigest(char *buffer);
    //int IsWhiteSpace(char c);
  static bool IsValidIGCChar(char c);

};

#endif
