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

#ifndef FLARM_TRAFFIC_WINDOW_H
#define FLARM_TRAFFIC_WINDOW_H

#include "Screen/PaintWindow.hpp"
#include "FLARM/State.hpp"
#include "SettingsComputer.hpp"

/**
 * A Window which renders FLARM traffic.
 */
class FlarmTrafficWindow : public PaintWindow {
protected:
  Brush hbWarning;
  Brush hbAlarm;
  Brush hbSelection;
  Brush hbTeam;

  Pen hpWarning;
  Pen hpAlarm;
  Pen hpStandard;
  Pen hpPassive;
  Pen hpSelection;

  unsigned zoom;
  int selection;
  int warning;
  POINT radar_mid;
  SIZE radar_size;
  POINT sc[FLARM_STATE::FLARM_MAX_TRAFFIC];

  Angle direction;
  FLARM_STATE data;
  SETTINGS_TEAMCODE settings;

public:
  int side_display_type;

public:
  FlarmTrafficWindow();

public:
  bool WarningMode() const;

  const FLARM_TRAFFIC *GetTarget() const {
    return selection >= 0 && data.FLARM_Traffic[selection].defined()
      ? &data.FLARM_Traffic[selection]
      : NULL;
  }

  void SetTarget(int i);
  void NextTarget();
  void PrevTarget();
  void SelectNearTarget(int x, int y);

protected:
  static double GetZoomDistance(unsigned zoom);

  void GetZoomDistanceString(TCHAR* str1, TCHAR* str2, unsigned size) const;
  double RangeScale(double d) const;

  void UpdateSelector();
  void UpdateWarnings();
  void Update(Angle new_direction, const FLARM_STATE &new_data,
              const SETTINGS_TEAMCODE &new_settings);
  void PaintTrafficInfo(Canvas &canvas) const;
  void PaintRadarNoTraffic(Canvas &canvas) const;
  void PaintRadarTarget(Canvas &canvas, const FLARM_TRAFFIC &traffic,
                        unsigned i);
  void PaintRadarTraffic(Canvas &canvas);
  void PaintRadarPlane(Canvas &canvas) const;
  void PaintRadarBackground(Canvas &canvas) const;

protected:
  virtual bool on_create();
  virtual bool on_resize(unsigned width, unsigned height);
  virtual void on_paint(Canvas &canvas);
};

#endif