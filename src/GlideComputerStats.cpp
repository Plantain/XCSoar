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

#include "GlideComputerStats.hpp"
#include "Protection.hpp"
#include "SettingsComputer.hpp"
#include "NMEA/Info.hpp"
#include "NMEA/Derived.hpp"
#include "Logger/Logger.hpp"
#include "Math/Earth.hpp"
#include "GPSClock.hpp"

GlideComputerStats::GlideComputerStats(ProtectedTaskManager &task) :
  GlideComputerBlackboard(task),
  log_clock(fixed(5)),
  stats_clock(fixed(60)) {}

void
GlideComputerStats::ResetFlight(const bool full)
{
  FastLogNum = 0;
  if (full)
    flightstats.Reset();
}

void
GlideComputerStats::StartTask()
{
  flightstats.StartTask();
}

/**
 * Logs GPS fixes for stats
 * @return True if valid fix (fix distance <= 200m), False otherwise
 */
bool
GlideComputerStats::DoLogging()
{
  /// @todo consider putting this sanity check inside Parser
  if (Distance(Basic().Location, LastBasic().Location) > fixed(200))
    // prevent bad fixes from being logged or added to OLC store
    return false;

  // log points more often in circling mode
  if (Calculated().Circling)
    log_clock.set_dt(fixed(SettingsComputer().LoggerTimeStepCircling));
  else
    log_clock.set_dt(fixed(SettingsComputer().LoggerTimeStepCruise));

  if (FastLogNum) {
    log_clock.set_dt(fixed_one);
    FastLogNum--;
  }

  if (log_clock.check_advance(Basic().Time)) {
    logger.LogPoint(Basic());
    gps_info.gps.Simulator = false; // reset for next fix (set by NMEA parsing)
  }

  if (Basic().flight.Flying) {
    if (stats_clock.check_advance(Basic().Time)) {
      flightstats.AddAltitudeTerrain(Basic().flight.FlightTime,
                                     Calculated().TerrainAlt);
      flightstats.AddAltitude(Basic().flight.FlightTime, Basic().NavAltitude);
      flightstats.AddTaskSpeed(Basic().flight.FlightTime,
                               Calculated().task_stats.total.remaining_effective.get_speed_incremental());
    }
  }

  return true;
}

void
GlideComputerStats::OnClimbBase(fixed StartAlt)
{
  flightstats.AddClimbBase(Calculated().ClimbStartTime -
                           Basic().flight.TakeOffTime, StartAlt);
}

void
GlideComputerStats::OnClimbCeiling()
{
  flightstats.AddClimbCeiling(Calculated().CruiseStartTime -
                              Basic().flight.TakeOffTime,
                              Calculated().CruiseStartAlt);
}

/**
 * This function is called when leaving a thermal and handles the
 * calculation of all related statistics
 */
void
GlideComputerStats::OnDepartedThermal()
{
  flightstats.AddThermalAverage(Calculated().LastThermalAverage);
}

void
GlideComputerStats::SetFastLogging()
{
  FastLogNum = 5;
}
