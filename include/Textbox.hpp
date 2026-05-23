#include "Widget.hpp"
#include "notcurses.h"

#include <cstdint>
#include <ncpp/Plane.hh>

struct Textbox : Widget {
  std::string text;
  ncpp::Plane* textbox_plane = nullptr;
  int x;
  int y;
  unsigned int w = 4;
  unsigned int h = 2;
  uint32_t bg_color = 0x000000;
  uint32_t fg_color = 0x00A83E;

  Textbox(
      ncpp::Plane* parent_pile,
      int y,
      int x,
      const std::string& id,
      std::string button_text,
      unsigned int width = 14,
      unsigned int height = 7,
      uint32_t bg_color = 0x000000,
      uint32_t fg_color = 0x00A83E)
      : Widget(parent_pile, id, y, x, width, height) {
    this->fg_color = fg_color;
    this->bg_color = bg_color;
    this->x = x;
    this->y = y;
    this->h = height;
    this->w = width;
    this->text = button_text;
    ncplane_options local_opts = {.y = y, .x = x, .rows = h, .cols = w, .userptr = nullptr};
    textbox_plane = new ncpp::Plane(parent_pile, local_opts);
    textbox_plane->set_fg_rgb(fg_color);
    textbox_plane->set_bg_rgb(bg_color);
  }

  ~Textbox() { delete textbox_plane; }
};

void set_test(Widget& widget, std::string_view text);
void set_fg_color(Widget& widget, uint32_t color);
void set_bg_color(Widget& widget, uint32_t color);
