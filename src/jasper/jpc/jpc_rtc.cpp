#include "jasper/jpc_rtc.h"
#include "Terrain/RasterTile.hpp"
#include "ProgressGlue.hpp"

RasterTileCache *raster_tile_current = 0;

static void StepProgressDialog(void) {
  ProgressGlue::Step();
}

static void SetNumTiles(unsigned num) {
  ProgressGlue::SetStep(1000 / (num + MAX_ACTIVE_TILES));
}

extern "C" {

  void jas_rtc_stepprogress(void) {
    StepProgressDialog();
  }
  void jas_rtc_set_num_tiles(unsigned num) {
    SetNumTiles(num);
  }

  void jas_rtc_SetTile(int index,
                       int xstart, int ystart,
                       int xend, int yend) {
    raster_tile_current->SetTile(index, xstart, ystart, xend, yend);
  }

  bool jas_rtc_TileRequest(int index) {
    return raster_tile_current->TileRequest(index);
  }

  short* jas_rtc_GetImageBuffer(int index) {
    return raster_tile_current->GetImageBuffer(index);
  }

  void jas_rtc_SetLatLonBounds(double lon_min, double lon_max,
                               double lat_min, double lat_max) {
    raster_tile_current->SetLatLonBounds(lon_min, lon_max, lat_min, lat_max);
  }

  void jas_rtc_SetSize(int width,
                       int height) {
    raster_tile_current->SetSize(width, height);
  }

  void jas_rtc_SetInitialised(bool val) {
    raster_tile_current->SetInitialised(val);
  }

  /*
  bool jas_rtc_GetInitialised(void) {
    return raster_tile_current->GetInitialised();
  }
  */

  short* jas_rtc_GetOverview(void) {
    return raster_tile_current->GetOverview();
  }
};
