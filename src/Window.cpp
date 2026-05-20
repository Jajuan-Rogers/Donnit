#include "Window.hpp"
#include "ncpp/Visual.hh"
#include <cstdint>
#include <ncpp/Plane.hh>
#include <string>

void set_window_title(Window &window, const std::string &title) {
  if (window.title) {
    int title_placement =
        (window.plane->get_dim_x() / 2) - (window.title->length() / 2);
    window.plane->putstr(0, title_placement, window.title->c_str());
  }
}

static void fill_window(Window &window, const char *c) {
  int row_count = window.plane->get_dim_y();
  int col_count = window.plane->get_dim_x();
  for (int row = 1; row < row_count; row++)
    for (int col = 1; col < col_count; col++) {
      window.plane->putstr(row, col, c);
    };
}

Window new_window(std::string id, int x, int y, int w, int h, uint32_t bg_color,
                  uint32_t fg_color, ncpp::Plane &stdplane, std::string title) {

  ncplane_options plane_opts = {.y = 2,
                                .x = 2,
                                .rows = 20,
                                .cols = 40,
                                .userptr = nullptr,
                                .name = id.c_str(),
                                .resizecb = nullptr,
                                .flags = 0};

  ncpp::Plane *plane = new ncpp::Plane(stdplane, plane_opts);
  plane->set_bg_rgb(bg_color);
  plane->set_fg_rgb(fg_color);
  if (title.empty())
    return Window{id, x, y, w, h, bg_color, fg_color, plane};

  Window window = Window{id, x, y, w, h, bg_color, fg_color, plane, title};
  set_window_title(window, title);
  const char* c = "-";
  //NOTE: debug code delete l8r
  fill_window(window, c);
  return window;
}

void move_window(Window &window, int x, int y) {
  window.x = x;
  window.y = y;
  window.plane->move(window.x, window.y);
}
