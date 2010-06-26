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

#include "Dialogs/Internal.hpp"
#include "Screen/Fonts.hpp"
#include "Screen/Layout.hpp"
#include "Engine/Math/fixed.hpp"
#include "MainWindow.hpp"
#include "Compiler.h"

static WndForm *wf = NULL;
static WndOwnerDrawFrame *draw = NULL;

/** The Lift database */
fixed lift_db[36];
fixed max_lift = fixed_one;
/**
 * A low-pass filtered turnrate to prevent
 * from switching the turn direction to frequently
 */
fixed filtered_turn_rate = fixed_zero;
fixed last_fix = fixed_zero;
Angle last_heading;

POINT mid;
int radius;

/**
 * Returns whether we are flying a left turn
 * @return True if left turn, False if right turn
 */
static bool
is_left_turn()
{
  return (filtered_turn_rate < fixed_zero);
}

/**
 * This updates the low-pass filter for the turnrate to prevent
 * from switching the turn direction too frequently
 * @param new_turnrate The current turnrate
 */
static void
update_turnrate(fixed new_turnrate)
{
  if (filtered_turn_rate == fixed_zero)
    filtered_turn_rate = new_turnrate;
  else
    filtered_turn_rate = filtered_turn_rate * fixed(0.8)
                         + new_turnrate * fixed(0.2);
}

static unsigned
heading_to_index(Angle &heading)
{
  return std::max(fixed_zero, std::min(fixed(35),
         abs((heading.value_degrees() + fixed(5)) / fixed(10))));
}

static void
search_new_max()
{
  max_lift = fixed_one;
  for (unsigned i = 0; i < 36; i++) {
    max_lift = std::max(max_lift, lift_db[i]);
  }
  max_lift = 5;
}

static void
clear_lift()
{
  for (unsigned i = 0; i < 36; i++) {
    lift_db[i] = fixed_zero;
  }
}

/**
 * Updates the lift database
 * @param heading Current heading
 * @param lift Current lift
 */
static void
update_lift(Angle heading, fixed lift)
{
  if (last_heading.value_degrees() == fixed_zero) {
    unsigned index = heading_to_index(heading);
    lift_db[index] = lift;
    return;
  }

  Angle heading_step = Angle::degrees(fixed(10));

  for (Angle h = last_heading; is_left_turn() ==
      negative((heading - h).as_delta().value_degrees());
      (is_left_turn() ? h -= heading_step : h += heading_step)) {
    unsigned index = heading_to_index(h);
    lift_db[index] = lift;
    if (lift_db[index] == fixed_zero)
      lift_db[index] = lift;
    else
      lift_db[index] = (lift_db[index] + lift) / fixed_two;
  }

  search_new_max();
}

/**
 * Updates the calculations
 * @param basic Current NMEA_INFO struct
 */
static void
update(const NMEA_INFO &basic)
{
  if (basic.Time <= last_fix)
    return;

  update_turnrate(basic.TurnRateWind);
  update_lift(basic.Heading, basic.TotalEnergyVario);

  last_heading = basic.Heading;
  last_fix = basic.Time;

  draw->invalidate();
}

/**
 * On dialog timer -> call update()
 */
static int
OnTimerNotify(gcc_unused WindowControl * Sender)
{
  if (XCSoarInterface::Calculated().Circling)
    update(XCSoarInterface::Basic());

  return 0;
}

static int
scale_radius(fixed lift)
{
  return std::max(0, (int)((lift + max_lift)
                           / (max_lift * fixed_two) * fixed(radius)));
}

static void
PaintBackground(Canvas &canvas)
{
  canvas.clear(Brush(Color::WHITE));
  canvas.hollow_brush();

  Pen hpBackground1(1, Color(0x55, 0x55, 0x55));
  Pen hpBackground2(Pen::DASH, 1, Color(0x55, 0x55, 0x55));
  canvas.select(hpBackground1);
  canvas.circle(mid.x, mid.y, 100);
  canvas.circle(mid.x, mid.y, scale_radius(fixed_zero));
  canvas.select(hpBackground2);
  canvas.circle(mid.x, mid.y, scale_radius(max_lift));
  canvas.circle(mid.x, mid.y, scale_radius(-max_lift));
}

static void
PaintPlane(Canvas &canvas)
{
  int x = mid.x + (is_left_turn() ? radius : -radius);
  int y = mid.y;

  Pen hpPlane(Layout::FastScale(2), Color(0x55, 0x55, 0x55));
  canvas.select(hpPlane);

  canvas.line(x, y - Layout::FastScale(6), x, y + Layout::FastScale(6));
  canvas.line(x + Layout::FastScale(10), y - Layout::FastScale(2),
              x - Layout::FastScale(10), y - Layout::FastScale(2));
  canvas.line(x + Layout::FastScale(4), y + Layout::FastScale(4),
              x - Layout::FastScale(4), y + Layout::FastScale(4));
}

void
convert_to_xy(Angle heading, fixed lift, int &x, int &y)
{
  Angle current_heading = XCSoarInterface::Basic().Heading;

  x = (int)((heading - current_heading).cos() * scale_radius(lift));
  y = (int)((heading - current_heading).sin() * scale_radius(lift));

  if (!is_left_turn()) {
    x *= -1;
    y *= -1;
  }

  x += mid.x;
  y += mid.y;
}

static void
PaintLiftPoint(Canvas &canvas, Angle heading, fixed lift)
{
  Brush p(Color::BLUE);
  Pen hpLift(Layout::FastScale(1), Color::BLACK);
  canvas.select(hpLift);

  int x, y;
  convert_to_xy(heading, lift, x, y);

  if (heading == XCSoarInterface::Basic().Heading)
    canvas.select(p);

  canvas.circle(x, y, 3);
}

static void
PaintLiftLine(Canvas &canvas, Angle heading1, fixed lift1,
              Angle heading2, fixed lift2)
{
  int x1, y1, x2, y2;
  convert_to_xy(heading1, lift1, x1, y1);
  convert_to_xy(heading2, lift2, x2, y2);

  Pen hpLift(Layout::FastScale(1), Color::BLACK);
  canvas.select(hpLift);

  canvas.line(x1, y1, x2, y2);
}

static void
PaintLift(Canvas &canvas)
{
  for (unsigned i = 1; i < 36; i++) {
    PaintLiftLine(canvas, Angle::degrees(fixed((i - 1) * 10)), lift_db[i - 1],
                          Angle::degrees(fixed(i * 10)), lift_db[i]);
  }
  PaintLiftLine(canvas, Angle::degrees(fixed(350)), lift_db[35],
                        Angle::degrees(fixed(0)), lift_db[0]);

  for (unsigned i = 0; i < 36; i++) {
    PaintLiftPoint(canvas, Angle::degrees(fixed(i * 10)), lift_db[i]);
  }
}

/**
 * This event is called when the Canvas needs to be repainted
 * @param Sender
 * @param canvas The Canvas to paint
 */
static void
OnPaint(WindowControl *Sender, Canvas &canvas)
{
  PaintBackground(canvas);
  PaintPlane(canvas);
  PaintLift(canvas);
}

/**
 * This event handler is called when the "Close" button is pressed
 */
static void
OnCloseClicked(gcc_unused WndButton &button)
{
  wf->SetModalResult(mrOK);
}

static CallBackTableEntry_t CallBackTable[] = {
  DeclareCallBackEntry(OnPaint),
  DeclareCallBackEntry(NULL)
};

void
dlgThermalAssistentShowModal()
{
  // Load dialog from XML
  if (Layout::landscape)
    wf = dlgLoadFromXML(CallBackTable, _T("dlgThermalAssistent_L.xml"),
        XCSoarInterface::main_window, _T("IDR_XML_THERMALASSISTENT_L"));
  else
    wf = dlgLoadFromXML(CallBackTable, _T("dlgThermalAssistent.xml"),
        XCSoarInterface::main_window, _T("IDR_XML_THERMALASSISTENT"));

  if (!wf)
    return;

  // Set canvas pointer
  draw = (WndOwnerDrawFrame *)wf->FindByName(_T("frmCanvas"));
  if (!draw)
    return;

  mid.x = draw->get_width() / 2;
  mid.y = draw->get_height() / 2;
  radius = std::min(mid.x, mid.y) * 0.9;

  // Set dialog events
  wf->SetTimerNotify(OnTimerNotify);

  // Set button events
  ((WndButton *)wf->FindByName(_T("cmdClose")))->
      SetOnClickNotify(OnCloseClicked);

  // Clear lift database
  clear_lift();
  max_lift = fixed_zero;
  filtered_turn_rate = fixed_zero;

  // Show the dialog
  wf->ShowModal();

  // After dialog closed -> delete it
  delete wf;
}
