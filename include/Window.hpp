#pragma once

#include "ncpp/Visual.hh"
#include <cstdint>
#include <ncpp/Plane.hh>
#include <optional>
#include <span>
#include <variant>
#include <vector>

using ChildWidgets = std::span<std::variant<ncpp::Plane, ncpp::Visual>>;

struct Window {
  std::string id;
  int x;
  int y;
  int h;
  int w;
  uint32_t bg_color = 0x000000;
  uint32_t fg_color = 0xFF0000;
  ncpp::Plane *plane;
  std::optional<std::string> title;
  std::vector<ncpp::Plane*> children;

  Window(ncpp::Plane *stdplane, std::string id, int x, int y, unsigned int w,
         unsigned int h, ncplane_options *popts) {
    ncplane_options local_opts;
    if (popts == nullptr) {
      local_opts = {.y = y, .x = x, .rows = h, .cols = w, .userptr = nullptr};
      popts = &local_opts;
    }
    struct ncplane *raw_plane = ncpile_create(stdplane->get_notcurses(), popts);
    if (raw_plane == nullptr) {
      std::runtime_error{"failed to make plane for widget"};
    }
    this->plane = new ncpp::Plane(raw_plane);
  }

};

void set_window_title(Window &window, const std::string &title);
Window new_window(std::string id, int x, int y, int w, int h, uint32_t bg_color,
                  uint32_t fg_color, ncpp::Plane &stdplane,
                  std::string window_title);

bool add_child();
void move_window(Window &window, int y, int x);
void update_window(Window &window);
