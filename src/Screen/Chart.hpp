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

#ifndef CHART_HPP
#define CHART_HPP

#include "Math/fixed.hpp"
#include "Math/Angle.hpp"
#include "Screen/Color.hpp"
#include "Screen/Pen.hpp"
#include "Compiler.h"

#include <windef.h>

class LeastSquares;
class Canvas;

class Chart
{
private:
  Canvas &canvas;
  RECT rc;

  Pen pens[5];

public:
  Chart(Canvas &the_canvas, const RECT the_rc);

  static const Color GROUND_COLOUR;

  enum Style {
    STYLE_BLUETHIN,
    STYLE_REDTHICK,
    STYLE_DASHGREEN,
    STYLE_MEDIUMBLACK,
    STYLE_THINDASHPAPER
  };

  void Reset();

  void DrawBarChart(const LeastSquares &lsdata);
  void DrawFilledLineGraph(const LeastSquares &lsdata, const Color thecolor);
  void DrawLineGraph(const LeastSquares &lsdata,  enum Style Style);
  void DrawTrend(const LeastSquares &lsdata, enum Style Style);
  void DrawTrendN(const LeastSquares &lsdata, enum Style Style);
  void DrawLine(const fixed xmin, const fixed ymin,
                const fixed xmax, const fixed ymax, enum Style Style);

  void ScaleYFromData(const LeastSquares &lsdata);
  void ScaleXFromData(const LeastSquares &lsdata);
  void ScaleYFromValue(const fixed val);
  void ScaleXFromValue(const fixed val);
  void ScaleMakeSquare();

  void StyleLine(const POINT l1, const POINT l2, enum Style Style);

  void ResetScale();

  static void FormatTicText(TCHAR *text, const fixed val, const fixed step);
  void DrawXGrid(const fixed tic_step, const fixed zero, enum Style Style,
                 const fixed unit_step, bool draw_units = false);
  void DrawYGrid(const fixed tic_step, const fixed zero, enum Style Style,
                 const fixed unit_step, bool draw_units = false);

  void DrawXLabel(const TCHAR *text);
  void DrawYLabel(const TCHAR *text);
  void DrawLabel(const TCHAR *text, const fixed xv, const fixed yv);
  void DrawArrow(const fixed x, const fixed y, const fixed mag,
                 const Angle angle, enum Style Style);
  void DrawNoData();

  fixed getYmin() const { return y_min; }
  fixed getYmax() const { return y_max; }
  fixed getXmin() const { return x_min; }
  fixed getXmax() const { return x_max; }

  gcc_pure
  long screenX(fixed x) const;

  gcc_pure
  long screenY(fixed y) const;

  gcc_pure
  long screenS(fixed s) const;

private:
  fixed yscale;
  fixed xscale;
  fixed y_min, x_min;
  fixed x_max, y_max;
  bool unscaled_x;
  bool unscaled_y;
};

#endif
