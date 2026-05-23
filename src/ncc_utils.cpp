#include "notcurses.h"
#include <cstdint>
#include <ncpp/Plane.hh>

namespace myNCUT {

void fill_plane(ncpp::Plane& plane, unsigned int rgb) {
  unsigned int rows, cols;
  ncplane_dim_yx(plane, &rows, &cols);

  uint64_t channel = 0;
  ncchannels_set_bg_rgb(&channel, rgb);
  // for (int y = 0; y < rows; y++){
  //   for (int x = 0; x < cols; x++){
  ncplane_set_channels(plane, channel);
  // }

  // }
}
}
