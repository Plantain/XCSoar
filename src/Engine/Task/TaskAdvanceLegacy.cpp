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
#include "TaskAdvanceLegacy.hpp"
#include "Tasks/BaseTask/TaskPoint.hpp"
#include "TaskPoints/StartPoint.hpp"
#include "TaskPoints/AATPoint.hpp"
#include "Tasks/BaseTask/IntermediatePoint.hpp"
#include "Navigation/Aircraft.hpp"

TaskAdvanceLegacy::TaskAdvanceLegacy():
  TaskAdvance(),
  mode(ADVANCE_AUTO)
{
}

bool 
TaskAdvanceLegacy::ready_to_advance(const TaskPoint &tp,
                                    const AIRCRAFT_STATE &state,
                                    const bool x_enter, 
                                    const bool x_exit)
{
  const bool m_mode_ready = mode_ready(tp);
  const bool m_state_ready = state_ready(tp, state, x_enter, x_exit);

  if ((mode==ADVANCE_ARM) || (mode==ADVANCE_ARMSTART)) {
    if (m_state_ready && !m_mode_ready) {
      m_request_armed = true;
    }
    if (m_armed) {
      m_request_armed = false;
    }
  }

  return m_mode_ready && m_state_ready;
}


bool
TaskAdvanceLegacy::aat_state_ready(const bool has_entered,
                                   const bool close_to_target) const
{
  if (mode==ADVANCE_AUTO) {
    return has_entered && close_to_target;
  } else {
    return has_entered;
  }
}


bool
TaskAdvanceLegacy::mode_ready(const TaskPoint &tp) const
{
  switch (mode) {
  case ADVANCE_MANUAL:
    return false;
  case ADVANCE_AUTO:
    return true;
  case ADVANCE_ARM:
    return m_armed;
  case ADVANCE_ARMSTART:
    return m_armed || tp.type != TaskPoint::START;
  };
  return false;
}



TaskAdvance::TaskAdvanceState_t 
TaskAdvanceLegacy::get_advance_state() const
{
  return TaskAdvance::MANUAL;
}
