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

#ifndef GAUGE_VARIO_H
#define GAUGE_VARIO_H

#include "Screen/BufferWindow.hpp"
#include "Screen/Bitmap.hpp"
#include "InstrumentBlackboard.hpp"

class ContainerWindow;
class UnitSymbol;

typedef struct {
  bool    InitDone;
  int     maxLabelSize;
  RECT    recBkg;
  POINT   orgText;
  fixed lastValue;
  TCHAR   lastText[32];
  const UnitSymbol *last_unit_symbol;
} DrawInfo_t;

class GaugeVario:
  public BufferWindow,
  public InstrumentBlackboard
{
private:
  int nlength0, nlength1, nwidth, nline;
  int xoffset;
  int yoffset;

  Brush redBrush;
  Brush blueBrush;
  Brush yellowBrush;
  Brush greenBrush;
  Pen redPen;
  Pen bluePen;
  Pen yellowPen;
  Pen greenPen;
  Pen redThickPen;
  Pen blueThickPen;
  Pen blankThickPen;

  Bitmap hBitmapClimb;

  POINT *polys;
  POINT *lines;

  int gmax;
  bool dirty;
  Bitmap hDrawBitMap;
  DrawInfo_t diValueTop;
  DrawInfo_t diValueMiddle;
  DrawInfo_t diValueBottom;
  DrawInfo_t diLabelTop;
  DrawInfo_t diLabelMiddle;
  DrawInfo_t diLabelBottom;
  const UnitSymbol *unit_symbol;

public:
  GaugeVario(ContainerWindow &parent,
             int left, int top, unsigned width, unsigned height,
             const WindowStyle style=WindowStyle());
  ~GaugeVario();

  void Render();

private:
  void RenderZero(Canvas &canvas);
  void RenderValue(Canvas &canvas, int x, int y,
                   DrawInfo_t *diValue, DrawInfo_t *diLabel,
                   fixed Value, const TCHAR *Label);
  void RenderSpeedToFly(Canvas &canvas, int x, int y);
  void RenderBallast(Canvas &canvas);
  void RenderBugs(Canvas &canvas);
  int  ValueToNeedlePos(fixed Value);
  void RenderNeedle(Canvas &canvas, int i, bool average, bool clear);
  void RenderVarioLine(Canvas &canvas, int i, int sink, bool clear);
  void RenderClimb(Canvas &canvas);

  void MakePolygon(const int i);
  void MakeAllPolygons();
  POINT* getPolygon(const int i);
};

#endif
