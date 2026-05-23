#pragma once

#include "Widget.hpp"
#include "ncpp/Visual.hh"
#include "notcurses.h"

#include <cstdint>
#include <ncpp/NotCurses.hh>
#include <ncpp/Plane.hh>
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
  ncpp::Plane* window_pile = nullptr;  // the pil
  std::string title;
  std::vector<Widget*> children;       // storage for arbitrary planes that are also on the pile

  Window(
      ncpp::NotCurses& nc,
      std::string id,
      std::string title,
      int y,
      int x,
      unsigned int w,
      unsigned int h,
      ncplane_options* popts = nullptr,
      uint32_t bg_color = 0x000000,
      uint32_t fg_color = 0xFF0000) {
    this->x = x;
    this->y = y;
    this->w = w;
    this->h = h;
    this->bg_color = bg_color;
    this->fg_color = fg_color;
    ncplane_options local_opts {};
    if (popts == nullptr) {
      local_opts = {.y = y, .x = x, .rows = h, .cols = w, .userptr = nullptr};
      popts = &local_opts;
    }
    struct ncplane* pile = ncpile_create(nc, popts);
    window_pile = new ncpp::Plane(pile);
  }

  ~Window() {
    for (Widget* widget : children) {
      delete widget;
    }
    delete window_pile;
  }
};

bool add_child(Window& window, Widget& child);
void move_window(Window& window, int y, int x);
void update_window(Window& window);
