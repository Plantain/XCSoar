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

#include "Persist.hpp"
#include "LogFile.hpp"
#include "LocalPath.hpp"
#include "SettingsComputer.hpp"
#include "Atmosphere.h"
#include "UtilsSystem.hpp"
#include "Logger/Logger.hpp"
#include "GlideComputer.hpp"
#include "Protection.hpp"
#include "Components.hpp"
#include "Asset.hpp"
#include "Task/ProtectedTaskManager.hpp"
#include "OS/FileUtil.hpp"

#include <stdio.h>

#include <algorithm>

using std::min;
using std::max;

static TCHAR szCalculationsPersistFileName[MAX_PATH];
static TCHAR szCalculationsPersistDirectory[MAX_PATH];

/**
 * Deletes the persistent memory file
 */
void
DeleteCalculationsPersist(void)
{
  File::Delete(szCalculationsPersistFileName);
}

/**
 * Loads calculated values from the persistent memory file
 * @param Calculated DERIVED_INFO the values should be loaded into
 */
void
LoadCalculationsPersist(DERIVED_INFO *Calculated)
{

  return; // do nothing, this is broken for CommonStats

  // Get the persistent memory filename
  if (szCalculationsPersistFileName[0] == 0) {
    if (is_altair()) {
      LocalPath(szCalculationsPersistFileName, _T("persist/xcsoar-persist.log"));
      LocalPath(szCalculationsPersistDirectory, _T("persist"));
    } else {
      LocalPath(szCalculationsPersistFileName, _T("xcsoar-persist.log"));
      _tcscpy(szCalculationsPersistDirectory, GetPrimaryDataPath());
    }
  }

  // Debug Log
  LogStartUp(_T("LoadCalculationsPersist"));

  unsigned sizein;

  // Try to open the persistent memory file
  FILE *file = _tfopen(szCalculationsPersistFileName, _T("rb"));
  if (file == NULL) {
    LogStartUp(_T("LoadCalculationsPersist file not found"));
    return;
  }

  fread(&sizein, sizeof(sizein), 1, file);
  if (sizein != sizeof(*Calculated)) {
    fclose(file);
    return;
  }

  // Read persistent memory into Calculated
  fread(Calculated, sizeof(*Calculated), 1, file);

  fread(&sizein, sizeof(sizein), 1, file);
  if (sizein != sizeof(glide_computer.GetFlightStats())) {
    glide_computer.ResetFlight();
    fclose(file);
    return;
  }

  // Read persistent memory into FlightStats
  fread(&glide_computer.GetFlightStats(), sizeof(glide_computer.GetFlightStats()), 1, file);

  /// @todo persistence for OLC data

  fread(&sizein, sizeof(sizein), 1, file);
  if (sizein != 4 * sizeof(double)) {
    fclose(file);
    return;
  }
  GlidePolar polar = protected_task_manager.get_glide_polar();

  double MACCREADY = polar.get_mc();
  double BUGS = polar.get_bugs();
  double BALLAST = polar.get_ballast();

  // Read persistent memory into MacCready, QNH, bugs, ballast and temperature
  fread(&MACCREADY, sizeof(double), 1, file);
  fread(&BUGS, sizeof(double), 1, file);
  fread(&BALLAST, sizeof(double), 1, file);
  fread(&CuSonde::maxGroundTemperature,
      sizeof(CuSonde::maxGroundTemperature), 1, file);

  //    ReadFile(hFile,&CRUISE_EFFICIENCY,
  //             size,&dwBytesWritten,(OVERLAPPED*)NULL);

  MACCREADY = min(10.0, max(MACCREADY, 0.0));
  BUGS = min(1.0, max(BUGS, 0.0));
  BALLAST = min(1.0, max(BALLAST, 0.0));
  //   CRUISE_EFFICIENCY = min(1.5, max(CRUISE_EFFICIENCY,0.75));

  polar.set_mc(fixed(MACCREADY));
  polar.set_bugs(fixed(BUGS));
  polar.set_ballast(fixed(BALLAST));
  protected_task_manager.set_glide_polar(polar);

  LogStartUp(_T("LoadCalculationsPersist OK"));

  fclose(file);
}

/**
 * Saves the calculated values to the persistent memory file
 * @param gps_info The basic data
 * @param Calculated The calculated data
 */
void
SaveCalculationsPersist(const NMEA_INFO &gps_info,
    const DERIVED_INFO &Calculated)
{
  unsigned size;

  logger.LoggerClearFreeSpace(gps_info);

  if (FindFreeSpace(szCalculationsPersistDirectory) < MINFREESTORAGE) {
    if (!logger.LoggerClearFreeSpace(gps_info)) {
      LogStartUp(_T("SaveCalculationsPersist insufficient storage"));
      return;
    } else {
      LogStartUp(_T("SaveCalculationsPersist cleared logs to free storage"));
    }
  }

  LogStartUp(_T("SaveCalculationsPersist"));

  FILE *file = _tfopen(szCalculationsPersistFileName, _T("wb"));

  if (file == NULL) {
    LogStartUp(_T("SaveCalculationsPersist can't create file"));
    return;
  }

  size = sizeof(DERIVED_INFO);
  fwrite(&size, sizeof(size), 1, file);
  fwrite(&Calculated, size, 1, file);

  size = sizeof(FlightStatistics);
  fwrite(&size, sizeof(size), 1, file);
  fwrite(&glide_computer.GetFlightStats(), size, 1, file);

  /// @todo persistence for OLC data

  GlidePolar polar = protected_task_manager.get_glide_polar();
  double MACCREADY = polar.get_mc();
  double BUGS = polar.get_bugs();
  double BALLAST = polar.get_ballast();

  size = sizeof(double)*4;
  fwrite(&size, sizeof(size), 1, file);
  fwrite(&MACCREADY, sizeof(MACCREADY), 1, file);
  fwrite(&BUGS, sizeof(BUGS), 1, file);
  fwrite(&BALLAST, sizeof(BALLAST), 1, file);
  fwrite(&CuSonde::maxGroundTemperature,
      sizeof(CuSonde::maxGroundTemperature), 1, file);

  //    WriteFile(hFile,&CRUISE_EFFICIENCY,
  //              size,&dwBytesWritten,(OVERLAPPED*)NULL);

  LogStartUp(_T("SaveCalculationsPersist ok"));

  fclose(file);
}
