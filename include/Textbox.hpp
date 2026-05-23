#include "Widget.hpp"
#include "notcurses.h"

#include <cstdint>
#include <ncpp/Plane.hh>

struct Textbox : Widget {
  std::string text;
  ncpp::Plane* plane = nullptr;
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
      unsigned int height = 7)
      : Widget(parent_pile, id, y, x, width, height) {
    this->x = x;
    this->y = y;
    this->h = height;
    this->w = width;
    this->text = button_text;
    ncplane_options local_opts = {.y = y, .x = x, .rows = h, .cols = w, .userptr = nullptr};
    plane = new ncpp::Plane(parent_pile, local_opts);
    plane->set_fg_rgb(fg_color);
    plane->set_bg_rgb(bg_color);
  }

  ~Textbox() { delete plane; }
};

void set_test(Widget& widget, std::string_view text);
void set_fg_color(Widget& widget, uint32_t color);
void set_bg_color(Widget& widget, uint32_t color);
