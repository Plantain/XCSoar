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

#include "InfoBoxes/Content/Glide.hpp"

#include "InfoBoxes/InfoBoxWindow.hpp"
#include "Interface.hpp"

#include <tchar.h>
#include <stdio.h>

void
InfoBoxContentLDInstant::Update(InfoBoxWindow &infobox)
{
  if (XCSoarInterface::Calculated().LD == fixed(999)) {
    infobox.SetInvalid();
    return;
  }

  // Set Value
  TCHAR tmp[32];
  _stprintf(tmp, _T("%2.0f"), (double)XCSoarInterface::Calculated().LD);
  infobox.SetValue(tmp);
}

void
InfoBoxContentLDCruise::Update(InfoBoxWindow &infobox)
{
  if (XCSoarInterface::Calculated().CruiseLD == fixed(999)) {
    infobox.SetInvalid();
    return;
  }

  // Set Value
  TCHAR tmp[32];
  _stprintf(tmp, _T("%2.0f"), (double)XCSoarInterface::Calculated().CruiseLD);
  infobox.SetValue(tmp);
}

void
InfoBoxContentLDAvg::Update(InfoBoxWindow &infobox)
{
  if (XCSoarInterface::Calculated().AverageLD == 0) {
    infobox.SetInvalid();
    return;
  }

  // Set Value
  if (XCSoarInterface::Calculated().AverageLD < 0)
    infobox.SetValue(_T("^^^"));
  else if (XCSoarInterface::Calculated().AverageLD >= 999)
    infobox.SetValue(_T("+++"));
  else {
    TCHAR tmp[32];
    _stprintf(tmp, _T("%2.0f"), (double)XCSoarInterface::Calculated().AverageLD);
    infobox.SetValue(tmp);
  }
}

void
InfoBoxContentLDVario::Update(InfoBoxWindow &infobox)
{
  if (XCSoarInterface::Calculated().LDvario == fixed(999) ||
      !XCSoarInterface::Basic().TotalEnergyVarioAvailable ||
      !XCSoarInterface::Basic().AirspeedAvailable) {
    infobox.SetInvalid();
    return;
  }

  // Set Value
  TCHAR tmp[32];
  _stprintf(tmp, _T("%2.0f"), (double)XCSoarInterface::Calculated().LDvario);
  infobox.SetValue(tmp);
}
