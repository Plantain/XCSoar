#ifndef JPC_RTC_H
#define JPC_RTC_H

#ifdef __cplusplus
extern "C" {
#endif

  void jas_rtc_SetTile(int index, int xstart, int ystart, int xend, int yend);
  bool jas_rtc_TileRequest(int index);
  bool jas_rtc_PollTiles(int viewx, int viewy);
  short* jas_rtc_GetImageBuffer(int index);
  void jas_rtc_SetLatLonBounds(double lon_min, double lon_max, double lat_min, double lat_max);
  void jas_rtc_SetSize(int width, int height);
  void jas_rtc_SetInitialised(bool val);
  bool jas_rtc_GetInitialised(void);
  short* jas_rtc_GetOverview(void);
  void jas_rtc_stepprogress(void);
  void jas_rtc_set_num_tiles(unsigned num);

#ifdef __cplusplus
}
#endif

#endif
