#include "Widget.hpp"
#include <cstdint>
#include <ncpp/Plane.hh>

struct Textbox {
  Widget m_base;
  ncpp::Plane *textbox_plane = nullptr;
  int *const x;
  int *const y;
  unsigned int *const w;
  unsigned int *const h;

  Textbox(Widget &m_base, int *const x, int *const y, unsigned int *const w,
          unsigned int *const h)
      : m_base(m_base), x(x), y(y), w(w), h(h) {};
};

void set_test(Widget &widget, std::string_view text);
void set_fg_color(Widget &widget, uint32_t color);
void set_bg_color(Widget &widget, uint32_t color);
