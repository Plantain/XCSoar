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

#include "Form/Util.hpp"
#include "Form/Form.hpp"
#include "Form/Edit.hpp"
#include "DataField/Base.hpp"
#include "Profile.hpp"

#include <assert.h>

void
LoadFormProperty(WndForm &form, const TCHAR *control_name, bool value)
{
  assert(control_name != NULL);

  WndProperty *ctl = (WndProperty *)form.FindByName(control_name);
  if (ctl == NULL)
    return;

  ctl->GetDataField()->Set(value);
  ctl->RefreshDisplay();
}

void
LoadFormProperty(WndForm &form, const TCHAR *control_name, int value)
{
  assert(control_name != NULL);

  WndProperty *ctl = (WndProperty *)form.FindByName(control_name);
  if (ctl == NULL)
    return;

  ctl->GetDataField()->SetAsInteger(value);
  ctl->RefreshDisplay();
}

void
LoadFormProperty(WndForm &form, const TCHAR *control_name, unsigned int value)
{
  assert(control_name != NULL);

  WndProperty *ctl = (WndProperty *)form.FindByName(control_name);
  if (ctl == NULL)
    return;

  ctl->GetDataField()->SetAsInteger(value);
  ctl->RefreshDisplay();
}

void
LoadFormProperty(WndForm &form, const TCHAR *control_name, fixed value)
{
  assert(control_name != NULL);

  WndProperty *ctl = (WndProperty *)form.FindByName(control_name);
  if (ctl == NULL)
    return;

  ctl->GetDataField()->Set(value);
  ctl->RefreshDisplay();
}

void
LoadFormProperty(WndForm &form, const TCHAR *control_name,
                 UnitGroup_t unit_group, int value)
{
  assert(control_name != NULL);

  WndProperty *ctl = (WndProperty *)form.FindByName(control_name);
  if (ctl == NULL)
    return;

  Units_t unit = Units::GetUserUnitByGroup(unit_group);
  DataField *df = ctl->GetDataField();
  df->SetUnits(Units::GetUnitName(unit));
  df->SetAsInteger(iround(Units::ToUserUnit(fixed(value), unit)));
  ctl->RefreshDisplay();
}

void
LoadFormProperty(WndForm &form, const TCHAR *control_name,
                 UnitGroup_t unit_group, fixed value)
{
  assert(control_name != NULL);

  WndProperty *ctl = (WndProperty *)form.FindByName(control_name);
  if (ctl == NULL)
    return;

  Units_t unit = Units::GetUserUnitByGroup(unit_group);

  DataField *df = ctl->GetDataField();
  df->SetUnits(Units::GetUnitName(unit));
  df->SetAsFloat(Units::ToUserUnit(value, unit));
  ctl->RefreshDisplay();
}

bool
SaveFormProperty(WndForm *wfm, const TCHAR *field, bool &value)
{
  WndProperty* wp = (WndProperty*)wfm->FindByName(field);
  if (wp) {
    if (value != wp->GetDataField()->GetAsBoolean()) {
      value = wp->GetDataField()->GetAsBoolean();
      return true;
    }
  }
  return false;
}

bool
SaveFormProperty(WndForm *wfm, const TCHAR *field, unsigned int &value)
{
  WndProperty* wp = (WndProperty*)wfm->FindByName(field);
  if (wp) {
    if ((int)value != wp->GetDataField()->GetAsInteger()) {
      value = wp->GetDataField()->GetAsInteger();
      return true;
    }
  }
  return false;
}

bool
SaveFormProperty(WndForm *wfm, const TCHAR *field, int &value)
{
  WndProperty* wp = (WndProperty*)wfm->FindByName(field);
  if (wp) {
    if (value != wp->GetDataField()->GetAsInteger()) {
      value = wp->GetDataField()->GetAsInteger();
      return true;
    }
  }
  return false;
}

bool
SaveFormProperty(WndForm *wfm, const TCHAR *field, short &value)
{
  WndProperty* wp = (WndProperty*)wfm->FindByName(field);
  if (wp) {
    if (value != wp->GetDataField()->GetAsInteger()) {
      value = wp->GetDataField()->GetAsInteger();
      return true;
    }
  }
  return false;
}

bool
SaveFormProperty(const WndForm &form, const TCHAR *control_name,
                 bool &value, const TCHAR *registry_name)
{
  assert(control_name != NULL);
  assert(registry_name != NULL);

  const WndProperty *ctl = (const WndProperty *)form.FindByName(control_name);
  if (ctl == NULL)
    return false;

  bool new_value = ctl->GetDataField()->GetAsBoolean();
  if (new_value == value)
    return false;

  value = new_value;
  Profile::Set(registry_name, new_value);
  return true;
}

bool
SaveFormProperty(WndForm *wfm, const TCHAR *field, const TCHAR *reg,
                 bool &value)
{
  if (SaveFormProperty(wfm, field, value)) {
    Profile::Set(reg, value);
    return true;
  } else {
    return false;
  }
}

bool
SaveFormProperty(WndForm *wfm, const TCHAR *field, const TCHAR *reg,
                 unsigned int &value)
{
  if (SaveFormProperty(wfm, field, value)) {
    Profile::Set(reg, value);
    return true;
  } else {
    return false;
  }
}

bool
SaveFormProperty(WndForm *wfm, const TCHAR *field, const TCHAR *reg,
                 int &value)
{
  if (SaveFormProperty(wfm, field, value)) {
    Profile::Set(reg, value);
    return true;
  } else {
    return false;
  }
}

bool
SaveFormProperty(WndForm *wfm, const TCHAR *field, const TCHAR *reg,
                 short &value)
{
  if (SaveFormProperty(wfm, field, value)) {
    Profile::Set(reg, value);
    return true;
  } else {
    return false;
  }
}

bool
SaveFormProperty(const WndForm &form, const TCHAR *control_name,
                 UnitGroup_t unit_group, int &value,
                 const TCHAR *registry_name)
{
  assert(control_name != NULL);
  assert(registry_name != NULL);

  const WndProperty *ctl = (const WndProperty *)form.FindByName(control_name);
  if (ctl == NULL)
    return false;

  Units_t unit = Units::GetUserUnitByGroup(unit_group);
  int new_value = ctl->GetDataField()->GetAsInteger();
  new_value = iround(Units::ToSysUnit(fixed(new_value), unit));
  if (new_value == value)
    return false;

  value = new_value;
  Profile::Set(registry_name, new_value);
  return true;
}

bool
SaveFormProperty(const WndForm &form, const TCHAR *control_name,
                 UnitGroup_t unit_group, unsigned &value,
                 const TCHAR *registry_name)
{
  int value2 = value;
  if (SaveFormProperty(form, control_name, unit_group, value2,
                       registry_name)) {
    value = value2;
    return true;
  } else
    return false;
}

bool
SaveFormProperty(const WndForm &form, const TCHAR *control_name,
                 UnitGroup_t unit_group, fixed &value,
                 const TCHAR *registry_name)
{
  int value2 = value;
  if (SaveFormProperty(form, control_name, unit_group, value2,
                       registry_name)) {
    value = fixed(value2);
    return true;
  } else
    return false;
}

bool
SaveFormProperty(WndForm *wfm, const TCHAR *field, DisplayTextType_t &value)
{
  WndProperty* wp = (WndProperty*)wfm->FindByName(field);
  if (wp) {
    if ((int)value != wp->GetDataField()->GetAsInteger()) {
      value = (DisplayTextType_t)wp->GetDataField()->GetAsInteger();
      return true;
    }
  }
  return false;
}

bool
SaveFormProperty(WndForm *wfm, const TCHAR *field, const TCHAR *reg,
                         DisplayTextType_t &value) {
  if (SaveFormProperty(wfm, field, value)) {
    Profile::Set(reg, value);
    return true;
  } else {
    return false;
  }
}
