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

#include "Screen/ButtonWindow.hpp"
#include "Screen/ContainerWindow.hpp"

#ifdef ENABLE_SDL

#include "Screen/Font.hpp"

void
ButtonWindow::set(ContainerWindow &parent, const TCHAR *text, unsigned id,
                  int left, int top, unsigned width, unsigned height,
                  const ButtonWindowStyle style)
{
  reset();

  PaintWindow::set(parent, left, top, width, height, style);

  this->text = text;
  this->id = id;

  // XXX hard coded path
  const char *dejavu_ttf =
    "/usr/share/fonts/truetype/ttf-dejavu/DejaVuSansCondensed.ttf";
  font.set(dejavu_ttf, 16);
}

bool
ButtonWindow::on_mouse_down(int x, int y)
{
  down = true;
  invalidate();
  return true;
}

bool
ButtonWindow::on_mouse_up(int x, int y)
{
  if (!down)
    return true;

  down = false;
  invalidate();

  if (parent != NULL) {
    if (!on_clicked())
      parent->on_command(id, 0);
  }

  return true;
}

void
ButtonWindow::on_paint(Canvas &canvas)
{
  canvas.draw_button(get_client_rect(), down);

  canvas.select(font);
  SIZE size = canvas.text_size(text.c_str());
  canvas.text((get_width() - size.cx) / 2 + down,
              (get_height() - size.cy) / 2 + down, text.c_str());
}

#else /* !ENABLE_SDL */

#include <commctrl.h>

void
ButtonWindow::set(ContainerWindow &parent, const TCHAR *text, unsigned id,
                  int left, int top, unsigned width, unsigned height,
                  const ButtonWindowStyle style)
{
  Window::set(&parent, WC_BUTTON, text,
              left, top, width, height,
              style);

  ::SetWindowLong(hWnd, GWL_ID, id);
}

void
ButtonWindow::set(ContainerWindow &parent, const TCHAR *text,
                  int left, int top, unsigned width, unsigned height,
                  const ButtonWindowStyle style)
{
  set(parent, text, COMMAND_BOUNCE_ID,
      left, top, width, height, style);
  install_wndproc();
}

const tstring
ButtonWindow::get_text() const
{
  assert_none_locked();
  assert_thread();

  TCHAR buffer[256]; /* should be large enough for buttons */

  int length = GetWindowText(hWnd, buffer,
                             sizeof(buffer) / sizeof(buffer[0]));
  return tstring(buffer, length);
}

#endif /* !ENABLE_SDL */

bool
ButtonWindow::on_clicked()
{
  return false;
}
