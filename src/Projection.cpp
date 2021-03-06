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

#include "Projection.hpp"

#include "Math/Earth.hpp"
#include "Math/Angle.hpp"
#include "Screen/Layout.hpp"
#include "Units.hpp"

Projection::Projection() :
  PanLocation(Angle::native(fixed_zero), Angle::native(fixed_zero)),
  DisplayAngle(Angle::native(fixed_zero)),
  m_scale_meters_to_screen(fixed_zero)
{
}

/**
 * Converts screen coordinates to a GeoPoint
 * @param x x-Coordinate on the screen
 * @param y y-Coordinate on the screen
 */
GeoPoint
Projection::Screen2LonLat(int x, int y) const
{
  const FastIntegerRotation::Pair p =
    DisplayAngle.Rotate(x - Orig_Screen.x, y - Orig_Screen.y);
  const GeoPoint pg(Angle::native(fixed(p.first)*InvDrawScale),
                    Angle::native(fixed(p.second)*InvDrawScale));

  GeoPoint g;
  g.Latitude = PanLocation.Latitude - pg.Latitude;
  g.Longitude = PanLocation.Longitude
              + pg.Longitude * g.Latitude.invfastcosine();
  return g;
}

/**
 * Converts a GeoPoint to screen coordinates
 * @param g GeoPoint to convert
 */
POINT
Projection::LonLat2Screen(const GeoPoint &g) const
{
  const GeoPoint d = PanLocation-g;
  const FastIntegerRotation::Pair p =
    DisplayAngle.Rotate((int)(d.Longitude.value_native()
                              * g.Latitude.fastcosine() * DrawScale),
                        (int)(d.Latitude.value_native() * DrawScale));

  POINT sc;
  sc.x = Orig_Screen.x - p.first;
  sc.y = Orig_Screen.y + p.second;
  return sc;
}

/**
 * Converts a LatLon-based polygon to screen coordinates
 *
 * This one is optimised for long polygons.
 * @param ptin Input polygon
 * @param ptout Output polygon
 * @param n Number of points in the polygon
 * @param skip Number of corners to skip after a successful conversion
 */
void
Projection::LonLat2Screen(const GeoPoint *ptin, POINT *ptout,
                          unsigned n, unsigned skip) const
{
  static Angle lastangle(Angle::native(fixed_minus_one));
  static int cost = 1024, sint = 0;

  if (GetDisplayAngle() != lastangle) {
    lastangle = GetDisplayAngle();
    cost = lastangle.ifastcosine();
    sint = lastangle.ifastsine();
  }
  const int xxs = Orig_Screen.x * 1024 - 512;
  const int yys = Orig_Screen.y * 1024 + 512;
  const fixed mDrawScale = DrawScale;
  const GeoPoint mPan = PanLocation;
  const GeoPoint *p = ptin;
  const GeoPoint *ptend = ptin + n;

  while (p < ptend) {
    int Y = (int)((mPan.Latitude - p->Latitude).value_native() * mDrawScale);
    int X = (int)((mPan.Longitude - p->Longitude).value_native()
                     * p->Latitude.fastcosine() * mDrawScale);
    ptout->x = (xxs - X * cost + Y * sint) / 1024;
    ptout->y = (Y * cost + X * sint + yys) / 1024;
    ptout++;
    p += skip;
  }
}

/**
 * Converts a LatLon-based polygon to screen coordinates
 *
 * This one is optimised for long polygons.
 * @param ptin Input polygon
 * @param ptout Output polygon
 * @param n Number of points in the polygon
 * @param skip Number of corners to skip after a successful conversion
 */
void
Projection::LonLat2Screen(const pointObj* const ptin,
                          POINT *ptout,
                          const int n,
                          const int skip) const
{
  static Angle lastangle(Angle::native(fixed_minus_one));
  static int cost = 1024, sint = 0;

  if (GetDisplayAngle() != lastangle) {
    lastangle = GetDisplayAngle();
    cost = lastangle.ifastcosine();
    sint = lastangle.ifastsine();
  }
  const int xxs = Orig_Screen.x * 1024 - 512;
  const int yys = Orig_Screen.y * 1024 + 512;
  const fixed mDrawScale = DrawScale;
  const GeoPoint mPan = PanLocation;
  pointObj const * p = ptin;
  const pointObj* ptend = ptin + n;

  while (p < ptend) {
    const GeoPoint g = point2GeoPoint(*p);

    const int Y = (int)((mPan.Latitude - g.Latitude).value_native()
                        * mDrawScale);
    const int X = (int)((mPan.Longitude - g.Longitude).value_native()
                        * g.Latitude.fastcosine() * mDrawScale);

    ptout->x = (xxs - X * cost + Y * sint) / 1024;
    ptout->y = (Y * cost + X * sint + yys) / 1024;
    ptout++;
    p += skip;
  }
}

bool
Projection::LonLatVisible(const GeoPoint &loc) const
{
  if (loc.Longitude.value_native() > fixed(screenbounds_latlon.minx) &&
      loc.Longitude.value_native() < fixed(screenbounds_latlon.maxx) &&
      loc.Latitude.value_native() > fixed(screenbounds_latlon.miny) &&
      loc.Latitude.value_native() < fixed(screenbounds_latlon.maxy))
    return true;

  return false;
}

bool
Projection::LonLat2ScreenIfVisible(const GeoPoint &loc,
                                   POINT *sc) const
{
  if (LonLatVisible(loc)) {
    *sc = LonLat2Screen(loc);
    return PointVisible(*sc);
  }

  return false;
}

bool
Projection::PointVisible(const POINT &P) const
{
  if ((P.x >= MapRect.left) &&
      (P.x <= MapRect.right) &&
      (P.y >= MapRect.top) &&
      (P.y <= MapRect.bottom))
    return true;

  return false;
}

void 
Projection::SetScaleMetersToScreen(const fixed scale_meters_to_screen)
{
  static const fixed fixed_r 
    (Angle::native(fixed(fixed_earth_r)).value_radians());

  m_scale_meters_to_screen = scale_meters_to_screen;
  DrawScale = fixed_r * m_scale_meters_to_screen;
  InvDrawScale = fixed_one / DrawScale;
}

void
Projection::UpdateScreenBounds() 
{
  screenbounds_latlon = CalculateScreenBounds(fixed_zero);
}

rectObj
Projection::CalculateScreenBounds(const fixed scale) const
{
  // compute lat lon extents of visible screen
  rectObj sb;

  if (scale >= fixed_one) {
    POINT screen_center = LonLat2Screen(PanLocation);

    sb.minx = sb.maxx = PanLocation.Longitude.value_native();
    sb.miny = sb.maxy = PanLocation.Latitude.value_native();

    int dx, dy;
    unsigned int maxsc = 0;
    dx = screen_center.x - MapRect.right;
    dy = screen_center.y - MapRect.top;
    maxsc = max(maxsc, isqrt4(dx * dx + dy * dy));
    dx = screen_center.x - MapRect.left;
    dy = screen_center.y - MapRect.top;
    maxsc = max(maxsc, isqrt4(dx * dx + dy * dy));
    dx = screen_center.x - MapRect.left;
    dy = screen_center.y - MapRect.bottom;
    maxsc = max(maxsc, isqrt4(dx * dx + dy * dy));
    dx = screen_center.x - MapRect.right;
    dy = screen_center.y - MapRect.bottom;
    maxsc = max(maxsc, isqrt4(dx * dx + dy * dy));

    for (int i = 0; i < 10; i++) {
      const Angle ang = Angle::degrees(i * fixed_360 / 10);
      POINT p;
      p.x = screen_center.x + iround(ang.fastcosine() * maxsc * scale);
      p.y = screen_center.y + iround(ang.fastsine() * maxsc * scale);
      GeoPoint g = Screen2LonLat(p.x, p.y);
      sb.minx = min((double)g.Longitude.value_native(), sb.minx);
      sb.miny = min((double)g.Latitude.value_native(), sb.miny);
      sb.maxx = max((double)g.Longitude.value_native(), sb.maxx);
      sb.maxy = max((double)g.Latitude.value_native(), sb.maxy);
    }
  } else {
    fixed xmin, xmax, ymin, ymax;
    int x, y;

    x = MapRect.left;
    y = MapRect.top;
    GeoPoint g = Screen2LonLat(x, y);
    xmin = g.Longitude.value_native();
    xmax = g.Longitude.value_native();
    ymin = g.Latitude.value_native();
    ymax = g.Latitude.value_native();

    x = MapRect.right;
    y = MapRect.top;
    g = Screen2LonLat(x, y);
    xmin = min(xmin, g.Longitude.value_native());
    xmax = max(xmax, g.Longitude.value_native());
    ymin = min(ymin, g.Latitude.value_native());
    ymax = max(ymax, g.Latitude.value_native());

    x = MapRect.right;
    y = MapRect.bottom;
    g = Screen2LonLat(x, y);
    xmin = min(xmin, g.Longitude.value_native());
    xmax = max(xmax, g.Longitude.value_native());
    ymin = min(ymin, g.Latitude.value_native());
    ymax = max(ymax, g.Latitude.value_native());

    x = MapRect.left;
    y = MapRect.bottom;
    g = Screen2LonLat(x, y);
    xmin = min(xmin, g.Longitude.value_native());
    xmax = max(xmax, g.Longitude.value_native());
    ymin = min(ymin, g.Latitude.value_native());
    ymax = max(ymax, g.Latitude.value_native());

    sb.minx = xmin;
    sb.maxx = xmax;
    sb.miny = ymin;
    sb.maxy = ymax;
  }

  return sb;
}

long
Projection::max_dimension(const RECT &rc)
{
  return std::max(rc.right - rc.left, rc.bottom - rc.top);
}

fixed
Projection::GetMapScaleUser() const
{
  fixed map_scale = fixed(GetMapResolutionFactor()) / m_scale_meters_to_screen;
  return map_scale;
}

int
Projection::GetMapResolutionFactor()
{
  return IBLSCALE(30);
}
