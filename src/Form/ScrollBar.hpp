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

#ifndef XCSOAR_FORM_SCROLL_BAR_HPP
#define XCSOAR_FORM_SCROLL_BAR_HPP

#include <windows.h>

class Window;
class Canvas;

class ScrollBar {
protected:
  /** Whether the slider is currently being dragged */
  bool dragging;
  int drag_offset;
  /** Coordinates of the ScrollBar */
  RECT rc;
  /** Coordinates of the Slider */
  RECT rc_slider;

public:
  enum {
    SCROLLBARWIDTH_INITIAL = 24,
  };

  /** Constructor of the ScrollBar class */
  ScrollBar();

  /** Returns the width of the ScrollBar */
  int get_width() const {
    return rc.right - rc.left;
  }

  /** Returns the height of the ScrollBar */
  int get_height() const {
    return rc.bottom - rc.top;
  }

  /** Returns the height of the slider */
  int get_slider_height() const {
    return rc_slider.bottom - rc_slider.top;
  }

  /** Returns the height of the scrollable area of the ScrollBar */
  int get_netto_height() const {
    return get_height() - 2 * get_width() - 1;
  }

  /**
   * Returns the height of the visible scroll area of the ScrollBar
   * (the area thats not covered with the slider)
   */
  int get_scroll_height() const {
    return get_netto_height() - get_slider_height();
  }

  /**
   * Returns whether the ScrollBar is defined or has to be set up first
   * @return True if the ScrollBar is defined,
   * False if it has to be set up first
   */
  bool defined() const {
    return get_width() > 0;
  }

  /**
   * Returns the x-Coordinate of the ScrollBar
   * (remaining client area aside the ScrollBar)
   * @param size Size of the client area including the ScrollBar
   * @return The x-Coordinate of the ScrollBar
   */
  unsigned get_left(const SIZE size) const {
    return defined() ? rc.left : size.cx;
  }

  /**
   * Returns whether the given POINT is in the ScrollBar area
   * @param pt POINT to check
   * @return True if the given POINT is in the ScrollBar area, False otherwise
   */
  bool in(const POINT pt) const {
    return ::PtInRect(&rc, pt);
  }

  /**
   * Returns whether the given POINT is in the slider area
   * @param pt POINT to check
   * @return True if the given POINT is in the slider area, False otherwise
   */
  bool in_slider(const POINT pt) const {
    return ::PtInRect(&rc_slider, pt);
  }

  /**
   * Returns whether the given y-Coordinate is on the up arrow
   * @param y y-Coordinate to check
   * @return True if the given y-Coordinate is on the up arrow,
   * False otherwise
   */
  bool in_up_arrow(int y) const {
    return y < rc.top + get_width();
  }

  /**
   * Returns whether the given y-Coordinate is on the down arrow
   * @param y y-Coordinate to check
   * @return True if the given y-Coordinate is on the down arrow,
   * False otherwise
   */
  bool in_down_arrow(int y) const {
    return y >= rc.bottom - get_width();
  }

  /**
   * Returns whether the given y-Coordinate is above the slider area
   * @param y y-Coordinate to check
   * @return True if the given y-Coordinate is above the slider area,
   * False otherwise
   */
  bool above_slider(int y) const {
    return y < rc_slider.top;
  }

  /**
   * Returns whether the given y-Coordinate is below the slider area
   * @param y y-Coordinate to check
   * @return True if the given y-Coordinate is below the slider area,
   * False otherwise
   */
  bool below_slider(int y) const {
    return y >= rc_slider.bottom;
  }

  /**
   * Sets the size of the ScrollBar
   * (actually just the height, width is automatically set)
   * @param size Size of the Control the ScrollBar is used with
   */
  void set(const SIZE size);
  /** Resets the ScrollBar (undefines it) */
  void reset();

  /** Calculates the size and position of the slider */
  void set_slider(unsigned size, unsigned view_size, unsigned origin);
  /** Calculates the new origin out of the given y-Coordinate of the drag */
  unsigned to_origin(unsigned size, unsigned view_size, int y) const;

  /** Paints the ScollBar */
  void paint(Canvas &canvas) const;

  /**
   * Returns whether the slider is currently being dragged
   * @return True if the slider is currently being dragged, False otherwise
   */
  bool is_dragging() const { return dragging; }
  /**
   * Should be called when beginning to drag
   * (Called by WndListFrame::on_mouse_down)
   * @param w The Window object the ScrollBar is belonging to
   * @param y y-Coordinate
   */
  void drag_begin(Window *w, unsigned y);
  /**
   * Should be called when stopping to drag
   * (Called by WndListFrame::on_mouse_up)
   * @param w The Window object the ScrollBar is belonging to
   */
  void drag_end(Window *w);
  /**
   * Should be called while dragging
   * @param size Size of the Scrollbar (not pixelwise)
   * @param view_size Visible size of the Scrollbar (not pixelwise)
   * @param y y-Coordinate
   * @return "Value" of the ScrollBar
   */
  unsigned drag_move(unsigned size, unsigned view_size, int y) const;
};

#endif
