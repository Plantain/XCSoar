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
#include "GlideState.hpp"
#include <math.h>
#include "Util/Quadratic.hpp"
#include "Navigation/SpeedVector.hpp"

/**
 * Quadratic function solver for MacCready theory constraint equation
 * 
 * - document this equation!
 */
class GlideQuadratic: public Quadratic
{
public:
  /**
   * Constructor.
   *
   * @param task Task to initialse solver for
   * @param V Speed (m/s)
   *
   * @return Initialised object (not solved)
   */
  GlideQuadratic(const GlideState &task, const fixed V) :
    Quadratic(task.dwcostheta_, task.wsq_ - V * V)
  {
  }

  /**
   * Find ground speed from task and wind
   *
   * @return Ground speed during cruise (m/s)
   */
  fixed
  solve() const
  {
    if (check())
      /// @todo check this is correct for all theta
      return solution_max();

    return -fixed_one;
  }
};

fixed
GlideState::calc_ave_speed(const fixed Veff) const
{
  if (positive(EffectiveWindSpeed)) {
    // only need to solve if positive wind speed
    GlideQuadratic q(*this, Veff);
    return q.solve();
  }

  return Veff;
}

// dummy task
GlideState::GlideState(const GeoVector &vector, const fixed htarget,
                       fixed altitude, const SpeedVector wind) :
  Vector(vector),
  MinHeight(htarget),
  AltitudeDifference(altitude - MinHeight)
{
  calc_speedups(wind);
}

void
GlideState::calc_speedups(const SpeedVector wind)
{
  if (positive(wind.norm)) {
    WindDirection = wind.bearing;
    EffectiveWindSpeed = wind.norm;
    const Angle theta = Angle::radians(fixed_pi) + wind.bearing - Vector.Bearing;
    EffectiveWindAngle = theta;
    wsq_ = wind.norm * wind.norm;
    HeadWind = -wind.norm*theta.cos();
    dwcostheta_ = fixed_two * HeadWind;
  } else {
    WindDirection = Angle::native(fixed_zero);
    EffectiveWindSpeed = fixed_zero;
    EffectiveWindAngle = Angle::native(fixed_zero);
    HeadWind = fixed_zero;
    wsq_ = fixed_zero;
    dwcostheta_ = fixed_zero;
  }
}

fixed
GlideState::drifted_distance(const fixed t_cl) const
{
  if (!positive(EffectiveWindSpeed))
    return Vector.Distance;

  const Angle wd = WindDirection.Reciprocal();
  fixed sinwd, coswd;
  wd.sin_cos(sinwd, coswd);

  const Angle tb = Vector.Bearing;
  fixed sintb, costb;
  tb.sin_cos(sintb, costb);

  const fixed aw = EffectiveWindSpeed * t_cl;
  const fixed dx = Vector.Distance * sintb - aw * sinwd;
  const fixed dy = Vector.Distance * costb - aw * coswd;

  return hypot(dx, dy);

  // ??   task.Bearing = RAD_TO_DEG*(atan2(dx,dy));
}
