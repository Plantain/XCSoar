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

#include "Airspace/AirspaceVisibility.hpp"
#include "Airspace/AbstractAirspace.hpp"
#include "SettingsComputer.hpp"

bool
AirspaceVisible::type_visible(const AbstractAirspace& airspace) const
{
  return m_settings.DisplayAirspaces[airspace.get_type()];
}

bool
AirspaceVisible::altitude_visible(const AbstractAirspace& airspace) const
{
  /// @todo airspace visibility did use ToMSL(..., map.Calculated().TerrainAlt); 

  switch (m_settings.AltitudeMode) {

  case ALLON:
    return true;

  case CLIP:
    if (airspace.get_base_altitude() <= fixed(m_settings.ClipAltitude))
      return true;
    else
      return false;

  case AUTO:

    if ((m_altitude >= (airspace.get_base_altitude() - fixed(m_settings.AltWarningMargin)))
        && (m_altitude <= (airspace.get_top_altitude() + fixed(m_settings.AltWarningMargin))))
      return true;
    else
      return false;

  case ALLBELOW:
    if (m_altitude >= (airspace.get_base_altitude() - fixed(m_settings.AltWarningMargin)))
      return true;
    else
      return false;

  case INSIDE:
    if ((m_altitude >= airspace.get_base_altitude()) 
        && (m_altitude <= airspace.get_top_altitude()))
      return true;
    else
      return false;

  case ALLOFF:
    return false;
  }
  return true;
}
