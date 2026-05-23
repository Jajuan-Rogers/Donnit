#pragma once
#include "notcurses.h"
#include <functional>
#include <ncpp/Plane.hh>
#include <optional>
#include <string_view>
#include <vector>

enum class KeyEvent { KEYPRESS, MOUSE_CLICK };

struct KeyboardEvent {
  KeyEvent type;
  char32_t key;
  std::optional<std::function<void()> *> cb;
};

struct Widget {
  std::string_view id;
  int x;
  int y;
  unsigned int w;
  unsigned int h;
  uint32_t bg_color = 0x0044AA;
  uint32_t fg_color = 0xFFFFFF;
  ncplane_options *popts;
  ncpp::Plane *root_plane;
  std::vector<ncpp::Plane *> children;

  Widget(ncpp::Plane *parent_pile, std::string id, int y, int x,
         unsigned int w, unsigned int h, uint32_t bg_color = 0x000000,
         uint32_t fg_color = 0xFF0000, std::vector<ncpp::Plane *> children = {},
         ncplane_options *popts = nullptr) {
    this->id = id;
    this->x = x;
    this->y = y;
    this->w = w;
    this->h = h;
    this->bg_color = bg_color;
    this->fg_color = fg_color;
    ncplane_options local_opts{};
    if (popts == nullptr) {
      local_opts = {.y = y, .x = x, .rows = h, .cols = w, .userptr = nullptr};
      popts = &local_opts;
    }
    root_plane = new ncpp::Plane(*parent_pile, popts);
    root_plane->set_bg_rgb(this->bg_color);
    root_plane->set_fg_rgb(this->fg_color);
  }

  ~Widget() { delete root_plane; };
};
