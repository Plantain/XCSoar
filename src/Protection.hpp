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

#ifndef XCSOAR_PROTECTION_HPP
#define XCSOAR_PROTECTION_HPP

#include "Thread/Trigger.hpp"
#include "Thread/Mutex.hpp"

extern Mutex mutexBlackboard;

void TriggerGPSUpdate();
void TriggerVarioUpdate();
void CreateCalculationThread(void);

// changed only in config or by user interface
// used in settings dialog
extern bool DevicePortChanged;
extern bool AirspaceFileChanged;
extern bool WaypointFileChanged;
extern bool TerrainFileChanged;
extern bool AirfieldFileChanged;
extern bool TopologyFileChanged;
extern bool PolarFileChanged;
extern bool LanguageFileChanged;
extern bool StatusFileChanged;
extern bool InputFileChanged;
extern bool MapFileChanged;

extern Trigger closeTriggerEvent;
extern Trigger globalRunningEvent;
extern Trigger airspaceWarningEvent;
extern Trigger targetManipEvent;
extern Trigger triggerClimbEvent;

/**
 * Suspend all threads which have unprotected access to shared data.
 * Call this before doing write operations on shared data.
 */
void
SuspendAllThreads();

/**
 * Resume all threads suspended by SuspendAllThreads().
 */
void
ResumeAllThreads();

class ScopeSuspendAllThreads {
public:
  ScopeSuspendAllThreads() { SuspendAllThreads(); }
  ~ScopeSuspendAllThreads() { ResumeAllThreads(); }
};

#endif

