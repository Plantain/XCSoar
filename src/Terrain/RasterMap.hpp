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

#ifndef XCSOAR_TERRAIN_RASTER_MAP_HPP
#define XCSOAR_TERRAIN_RASTER_MAP_HPP

#include "RasterProjection.hpp"
#include "RasterTile.hpp"
#include "Navigation/GeoPoint.hpp"
#include "Geo/BoundsRectangle.hpp"
#include "Util/NonCopyable.hpp"
#include "Compiler.h"

class RasterMap : private NonCopyable {
  static int ref_count;

  char *path;
  RasterTileCache raster_tile_cache;
  BoundsRectangle bounds;
  RasterProjection projection;

public:
  RasterMap(const char *path);
  ~RasterMap();

  bool isMapLoaded() const {
    return raster_tile_cache.GetInitialised();
  }

  gcc_pure
  bool inside(const GeoPoint &pt) const {
    return bounds.inside(pt);
  }

  gcc_pure
  GeoPoint GetMapCenter() const {
    return bounds.center();
  }

  void SetViewCenter(const GeoPoint &location);

  // accurate method
  int GetEffectivePixelSize(fixed &pixel_D,
                            const GeoPoint &location) const;

  gcc_pure
  short GetField(const GeoPoint &location) const;

 protected:
  void _ReloadJPG2000(void);
};


#endif
